/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2023 Rebol Open Source Developers
**  REBOL is a trademark of REBOL Technologies
**
**  Additional code modifications and improvements:
**	Copyright 2012-2018 Saphirion AG & Atronix
**	Copyright 2019-2023 Oldes
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Title: Windowing Interface for Win32
**  Author: Carl Sassenrath
**  Purpose: Provides the API used by REBOL for window management.
**	Related: host-event.c, host-graphics.c
**
************************************************************************
**
**  NOTE to PROGRAMMERS:
**
**    1. Keep code clear and simple.
**    2. Document unusual code, reasoning, or gotchas.
**    3. Use same style for code, vars, indent(4), comments, etc.
**    4. Keep in mind Linux, OS X, BSD, big/little endian CPUs.
**    5. Test everything, then test it again.
**
***********************************************************************/

#ifndef WINVER
#define WINVER 0x0501        // this is needed to be able use WINDOWINFO struct etc.
#endif

/* Forces the use of Visual Styles if compiling with VisualStudio */
#ifdef _MSC_VER
#pragma comment(linker,"\"/manifestdependency:type='win32' \
	name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
	processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif


#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h> // used for setting visual defaults
#include <vssym32.h> // --//--
#include <math.h>
#include <versionhelpers.h> // for OS version detection

//#include <stdio.h> // used for debuging traces

#undef IS_ERROR // Windows is using this macro name too, we don't need their version

#include "reb-host.h"
#include "host-lib.h"
#include "reb-compositor.h"
#include "reb-types.h"

#define INCLUDE_EXT_DATA
#include "host-ext-window.h"

//***** Extra data stored with hWnd struct ******//
//typedef struct reb_wnd_extra {
//	// nothing yet
//} WNDEXTRA;


//***** DPI ******//
#ifndef DPI_ENUMS_DECLARED
typedef enum PROCESSDPI_AwareNESS
{
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEMDPI_Aware = 1,
	PROCESS_PER_MONITORDPI_Aware = 2
} PROCESSDPI_AwareNESS;
typedef enum MONITOR_DPI_TYPE {
	MDT_EFFECTIVE_DPI = 0,
	MDT_ANGULAR_DPI = 1,
	MDT_RAW_DPI = 2,
	MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;
#endif /*DPI_ENUMS_DECLARED*/

typedef BOOL(WINAPI * SETPROCESSDPIAWARE_T)(void);
typedef HRESULT(WINAPI * SETPROCESSDPIAWARENESS_T)(PROCESSDPI_AwareNESS);
typedef HRESULT(WINAPI * GETDPIFORMONITOR_T)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);

//***** Constants *****//

#define MAX_WINDOWS 64
#define GOB_HWIN(gob)	(OS_Find_Window(gob))
#define GOB_COMPOSITOR(gob)	(OS_Find_Compositor(gob)) //gets handle to window's compositor

//***** Externs *****//

extern HINSTANCE App_Instance;		// Set by winmain function
extern void Host_Crash(char *reason);
extern LRESULT CALLBACK REBOL_Window_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK REBOL_OpenGL_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK REBOL_Base_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//***** Locals *****//

static REBOOL Registered = FALSE;		// Window has been registered
static const REBCHR *Class_Name_Window   = TXT("RebWindow");
static const REBCHR *Class_Name_Base     = TXT("RebBase");
//static const REBCHR *Class_Name_Button   = TXT("RebButton");
static const REBCHR *Class_Name_ComboBox = TXT("RebCombo");
static struct gob_window *Gob_Windows;
static REBOOL DPI_Aware = FALSE;
static REBOOL Custom_Cursor = FALSE;
static HFONT Default_Font = NULL;
static REBOOL Windows8_And_Newer = FALSE;

static u32* window_ext_words;

extern RL_LIB *RL; // Link back to reb-lib from embedded extensions

//***** Globals *****//

REBGOB *Gob_Root;				// Top level GOB (the screen)
HCURSOR Cursor;					// active cursor image object
REBXYF Zero_Pair = {0, 0};
REBXYF log_size = { 1.0, 1.0 };	 // logical pixel size measured in physical pixels (can be changed using GUI-METRIC command)
REBXYF phys_size = { 1.0, 1.0 }; // physical pixel size measured in logical pixels(reciprocal value of log_size)
REBINT window_scale;

//***** Forwards *****//

static REBCNT Get_Widget_Text(HWND widget, REBVAL *text);
void Paint_Window(HWND window);

/***********************************************************************
**
*/  static void Init_DPI_Awareness(void) 
/*
**      Initialize DPI awareness if available
**
**		Based on code from https://github.com/floooh/sokol
**
***********************************************************************/
{
	SETPROCESSDPIAWARE_T     SetProcessDPIAwareFunc;
	SETPROCESSDPIAWARENESS_T SetProcessDPIAwarenessFunc;
	GETDPIFORMONITOR_T       GetDpiForMonitorFunc;
	HINSTANCE user32 = LoadLibraryA("user32.dll");
	HINSTANCE shcore = LoadLibraryA("shcore.dll");

	if (!user32 || !shcore) return;
	
	SetProcessDPIAwareFunc = (SETPROCESSDPIAWARE_T)GetProcAddress(user32, "SetProcessDPIAware");
	SetProcessDPIAwarenessFunc = (SETPROCESSDPIAWARENESS_T)GetProcAddress(shcore, "SetProcessDpiAwareness");
	GetDpiForMonitorFunc = (GETDPIFORMONITOR_T)GetProcAddress(shcore, "GetDpiForMonitor");

	if (SetProcessDPIAwarenessFunc) {
		SetProcessDPIAwarenessFunc(PROCESS_SYSTEMDPI_Aware);
		DPI_Aware = TRUE;
	}
	else if (SetProcessDPIAwareFunc) {
		SetProcessDPIAwareFunc();
		DPI_Aware = TRUE;
	}
	/* get dpi scale factor for main monitor */
	if (GetDpiForMonitorFunc && DPI_Aware) {
		POINT pt = { 1, 1 };
		HMONITOR hm = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		UINT dpix, dpiy;
		HRESULT hr = GetDpiForMonitorFunc(hm, MDT_EFFECTIVE_DPI, &dpix, &dpiy);
		UNUSED(hr);
		/* clamp window scale to an integer factor */
		window_scale = (int)((float)dpix / 96.0f);
	}
	else {
		window_scale = 1;
	}
	FreeLibrary(user32);
	FreeLibrary(shcore);
}

/**********************************************************************
**
*/	REBINT Alloc_Window(REBGOB *gob)
/*
**	Window Allocator
**
**	The window handle is not stored in the gob to avoid wasting
**	memory or creating too many exceptions in the gob.parent field.
**	Instead, we store gob and window pointers in an array that we
**	scan when required.
**
**	This code below is not optimial, but works ok because:
**		1) there are usually very few windows open
**		2) window functions are not called often
**		2) window events are mapped directly to gobs
**
**********************************************************************/
{
	int n;
	for (n = 0; n < MAX_WINDOWS; n++) {
		if (Gob_Windows[n].gob == 0) {
			Gob_Windows[n].gob = gob;
			// note: don't create compositor here,
			// it will be done when its class is registered
			return n;
		}
	}
	return -1;
}

/***********************************************************************
**
*/	void* OS_Find_Window(REBGOB *gob)
/*
**	Return window handle of given gob.
**
***********************************************************************/
{
	int n;
	for (n = 0; n < MAX_WINDOWS; n++) {
		if (Gob_Windows[n].gob == gob) return Gob_Windows[n].win;
	}
	return 0;
}

/***********************************************************************
**
*/	void* OS_Find_Compositor(REBGOB *gob)
/*
**	Return compositor handle of given gob.
**
***********************************************************************/
{
	int n;
	for (n = 0; n < MAX_WINDOWS; n++) {
		if (Gob_Windows[n].gob == gob) return Gob_Windows[n].compositor;
	}
	return 0;
}

/***********************************************************************
**
*/	REBGOB* OS_Get_Gob_Root()
/*
**	Return gob root.
**	Needed to map-event when event does not hold gob, but just data and 
**	compiled as a standalone library (without host part)
**
***********************************************************************/
{
	return Gob_Root;
}

/***********************************************************************
**
*/	void OS_Free_Window(REBGOB *gob)
/*
**	Release the Gob_Windows slot used by given gob.
**
***********************************************************************/
{
	int n;
	for (n = 0; n < MAX_WINDOWS; n++) {
		if (Gob_Windows[n].gob == gob) {
			OS_Destroy_Compositor(Gob_Windows[n].compositor);
			CLEAR(&Gob_Windows[n], sizeof(Gob_Windows[n]));
			return;
		}
	}
}


/***********************************************************************
**
*/  static void Make_Subclass(const REBCHR *new_class, const REBCHR *old_class, WNDPROC *proc, REBOOL system)
/*
**      Register super class
**
***********************************************************************/
{
	HINSTANCE hInstance = system ? NULL : App_Instance;
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));

	if(!GetClassInfoEx(hInstance, old_class, &wcex)) {
		RL_Print("Failed to get old class info!\n");
	}
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.cbWndExtra = 0; //sizeof(WNDEXTRA);
	wcex.lpszClassName = new_class;
	wcex.hInstance = App_Instance;
	if(!RegisterClassEx(&wcex))
		Host_Crash("Cannot register sub-class");
}


/***********************************************************************
**
*/  static void Register_Window()
/*
**      Register the window class.
**
**      Note: Executed in OS_Init code.
**
***********************************************************************/
{
	WNDCLASSEX wc;

	wc.cbSize        = sizeof(wc);
	wc.lpszClassName = Class_Name_Window;
	wc.hInstance     = App_Instance;
	wc.lpfnWndProc   = REBOL_Window_Proc;

	wc.hIcon         = LoadIcon(App_Instance, MAKEINTRESOURCE(101));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0; //sizeof(WNDEXTRA);
	wc.lpszMenuName  = NULL;

	wc.hIconSm = LoadImage(App_Instance, // small class icon
		MAKEINTRESOURCE(101),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_DEFAULTCOLOR
	);

	if (!RegisterClassEx(&wc)) Host_Crash("Cannot register window");

	wc.lpfnWndProc = REBOL_OpenGL_Proc;
	wc.lpszClassName = TXT("RebOpenGL");
	if (!RegisterClassEx(&wc)) RL_Print("Failed to register OpenGL class\n");

	wc.lpfnWndProc = REBOL_Base_Proc;
	wc.lpszClassName = TXT("RebBase");
	if (!RegisterClassEx(&wc)) RL_Print("Failed to register Base class\n");

	//Make_Subclass(Class_Name_Button, TEXT("BUTTON"), NULL, TRUE);

	Registered = TRUE;
}


/***********************************************************************
**
*/  BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
/*
**		Callback function which enables/disables events in windows
**		specified by lParam
**
**		This function is used by Win API EnumWindows() call.
**
***********************************************************************/
{
	if (GetParent(hwnd) == (HWND)lParam){
		if (IsWindowEnabled(hwnd)){
			EnableWindow(hwnd, FALSE);
		} else {
			EnableWindow(hwnd, TRUE);
		}
	}

	return TRUE;
}


/***********************************************************************
**
*/  void* OS_Open_Window(REBGOB *gob)
/*
**      Initialize the graphics window.
**
**      Note: This function is used by embedded windows as well.
**      Such as those inserted into web browser output.
**
**		The window handle is returned, but not expected to be used
**		other than for debugging conditions.
**
***********************************************************************/
{
	REBINT options;
	REBINT ws_flags;
	REBINT windex;
	HWND window;
	REBCHR *title;
	int x, y, w, h;
	HWND parent = NULL;
	REBYTE osString = FALSE;
	RECT rect;

	if (!Registered) Register_Window();

	windex = Alloc_Window(gob);
	if (windex < 0) Host_Crash("Too many windows");

	CLEAR_GOB_STATE(gob);
	x = GOB_X_INT(gob);
	y = GOB_Y_INT(gob);
	w = GOB_W_INT(gob);
	h = GOB_H_INT(gob);

	SET_GOB_STATE(gob, GOBS_NEW);

	// Setup window options:

	options = WS_POPUP;
	ws_flags = 0;

	if (!GET_FLAGS(gob->flags, GOBF_NO_TITLE, GOBF_NO_BORDER)) {
		options |= WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU;
	}

	if (GET_GOB_FLAG(gob, GOBF_RESIZE)) {
		options |= WS_SIZEBOX | WS_BORDER;
		if (!GET_GOB_FLAG(gob, GOBF_NO_TITLE))
			options |= WS_MAXIMIZEBOX;
	}
	else if (!GET_GOB_FLAG(gob, GOBF_NO_BORDER)) {
		options |= WS_BORDER;
	}

	if (IS_GOB_STRING(gob))
		RL_Get_String(GOB_CONTENT(gob), 0, (void**)&title, TRUE);
    else
        title = TXT("REBOL Window");

	if (GET_GOB_FLAG(gob, GOBF_POPUP)) {
		ws_flags |= WS_EX_TOOLWINDOW;
		parent = GOB_HWIN(GOB_TMP_OWNER(gob));
		if (GET_GOB_FLAG(gob, GOBF_MODAL)) {
			EnableWindow(parent, FALSE);
			EnumWindows(EnumWindowsProc, (LPARAM)parent);
		}
	}
	if (GET_GOB_FLAG(gob, GOBF_ON_TOP)) {
		ws_flags |= WS_EX_TOPMOST;
	}
	//if (!GET_GOB_FLAG(gob, GOBF_NO_BORDER)) {
	//	ws_flags |= WS_EX_WINDOWEDGE;
	//}

	if (GOB_ALPHA(gob) < 255) {
		//puts("semi-transparent window");
		ws_flags |= WS_EX_LAYERED;
	}

	/* QUESTION:
	** When opening window at position 0x0, should it be position of the content,
	** or of the top-left window corner? I'm choosing the second variant for now (oldes).
	*/
	rect.left = 0;
	rect.top = 0;
	rect.right = w;
	rect.bottom = h;

	AdjustWindowRectEx(&rect, options, FALSE, ws_flags); // not DPI aware!

	//printf("%i %i %i %i\n", rect.left, rect.top, rect.right, rect.bottom);

	x += rect.left;     // else window will be placed at position, where shadow on the left side is fully visible
	w = rect.right - rect.left; // update the size so client area has the requested size + window borders
	h = rect.bottom - rect.top;
	//----------------------------------------------------

	// Create the window:
	window = CreateWindowEx(
		ws_flags, 
		Class_Name_Window,
		title,
		options,
		x, y, w, h,
		parent,
		NULL, App_Instance, NULL
	);
	if (!window) {
		Host_Crash("CreateWindow failed");
		return NULL; // silent compiler's warnings
	}

	Gob_Windows[windex].win = window;
	Gob_Windows[windex].compositor = OS_Create_Compositor(Gob_Root, gob);

	if (!Default_Font) {
		LOGFONTW font;
		HTHEME *hTheme = NULL;
		HRESULT err = E_FAIL;
		if (IsThemeActive()) {
			hTheme = OpenThemeData(window, L"Window");
			if (hTheme) {
				err = GetThemeSysFont(hTheme, TMT_MSGBOXFONT, &font);
			}
		}
		if (err != S_OK) {
			NONCLIENTMETRICS metrics;
			ZeroMemory(&metrics, sizeof(metrics));
			metrics.cbSize = sizeof(metrics);
			if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0)) {
				font = metrics.lfMessageFont;
				err = S_OK;
			}
		}
		if (err == S_OK) {
			Default_Font = CreateFontIndirect(&font);
		}

		if (hTheme) CloseThemeData(hTheme);
		if (!Default_Font) Default_Font = GetStockObject(DEFAULT_GUI_FONT);
	}

	// Enable drag and drop
	if (GET_GOB_FLAG(gob, GOBF_DROPABLE))
		DragAcceptFiles(window, TRUE);

	SET_GOB_FLAG(gob, GOBF_WINDOW);
	SET_GOB_FLAG(gob, GOBF_ACTIVE);
	SET_GOB_STATE(gob, GOBS_OPEN);

	// Provide pointer from window back to REBOL window:
	SetWindowLongPtr(window, GWLP_USERDATA, (REBUPT)gob);

	if (GOB_ALPHA(gob) < 255) {
		SetLayeredWindowAttributes(window, 0, GOB_ALPHA(gob), LWA_ALPHA);
	}

	if (!GET_GOB_FLAG(gob, GOBF_HIDDEN)) {
		if (GET_GOB_FLAG(gob, GOBF_ON_TOP)) SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		ShowWindow(window, SW_SHOWNORMAL);
		//UpdateWindow(window);
		RedrawWindow(window, NULL, NULL, RDW_UPDATENOW); 
		SetForegroundWindow(window);
	}
	return window;
}


/***********************************************************************
**
*/  void OS_Close_Window(REBGOB *gob)
/*
**		Close the window.
**
***********************************************************************/
{
	HWND parent = NULL;
	if (GET_GOB_FLAG(gob, GOBF_WINDOW) && OS_Find_Window(gob)) {
		if (GET_GOB_FLAG(gob, GOBF_MODAL)) {
			parent = GetParent(GOB_HWIN(gob));
			if (parent) {
				EnableWindow(parent, TRUE);
				EnumWindows(EnumWindowsProc, (LPARAM)parent);
			}
		}
		DestroyWindow(GOB_HWIN(gob));
		CLR_GOB_FLAG(gob, GOBF_WINDOW);
		CLEAR_GOB_STATE(gob); // set here or in the destory?
		OS_Free_Window(gob);
	}
}


/***********************************************************************
**
*/	REBOOL OS_Resize_Window(REBGOB *gob, REBOOL redraw)
/*
**		Update window parameters.
**
***********************************************************************/
{
	REBCMP *compositor;
	REBOOL changed;
	compositor = GOB_COMPOSITOR(gob);
	compositor->win_rect.right = GOB_LOG_W_INT(gob);
	compositor->win_rect.bottom = GOB_LOG_H_INT(gob);
	changed = OS_Resize_Window_Buffer(compositor, gob);
	//InvalidateRect()
	//if (redraw) {
	//	OS_Compose_Gob(compositor, gob, gob, FALSE); // what if not actually resized?
	//	OS_Blit_Window(compositor);
	//}
	return changed;
}


/***********************************************************************
**
*/	void OS_Update_Window(REBGOB *gob)
/*
**		Update window parameters.
**
***********************************************************************/
{
	RECT r;
	REBCNT opts = 0;
	HWND window;
	WINDOWINFO wi;
	REBCHR *title;
	REBYTE osString = FALSE;

	wi.cbSize = sizeof(WINDOWINFO);

	if (!IS_WINDOW(gob)) return;

	window = GOB_HWIN(gob);

	if (GOB_X(gob) == GOB_XO(gob) && GOB_Y(gob) == GOB_YO(gob))
		opts |= SWP_NOMOVE;

	if (GOB_W(gob) == GOB_WO(gob) && GOB_H(gob) == GOB_HO(gob))
		opts |= SWP_NOSIZE;
	else
		//Resize window and/or buffer in case win size changed programatically
		OS_Resize_Window(gob, FALSE);

	//Get the new window size together with borders, tilebar etc.
	GetWindowInfo(window, &wi);
	r.left   = GOB_X_INT(gob);
	r.right  = r.left + GOB_W_INT(gob);
	r.top    = GOB_Y_INT(gob);
	r.bottom = r.top + GOB_H_INT(gob);
	AdjustWindowRect(&r, wi.dwStyle, FALSE);

	//Set the new size
	SetWindowPos(window, 0, r.left, r.top, r.right - r.left, r.bottom - r.top, opts | SWP_NOZORDER);

	//if (opts)
//		SetWindowPos(window, 0, GOB_X(gob), GOB_Y(gob), GOB_W(gob), GOB_H(gob), opts | SWP_NOZORDER);

	if (IS_GOB_STRING(gob)){
		RL_Get_String(GOB_CONTENT(gob), 0, (void**)&title, TRUE);
		SetWindowText(window, title);
    }

	/*
	switch (arg) {
		case 0: arg = SW_MINIMIZE; break;
		case 1: arg = SW_RESTORE;  break;
		case 2: arg = SW_MAXIMIZE; break;
	}

	ShowWindow(window, arg);

	SetForegroundWindow(window);
	*/
}

/***********************************************************************
**
*/	void OS_Draw_Window(REBGOB *wingob, REBGOB *gob, REBOOL invalidate)
/*
**		Refresh the GOB within the given window. If the wingob
**		is zero, then find the correct window for it.
**
***********************************************************************/
{
	//REBCMP* compositor;
	if (!wingob) {
		wingob = gob;
		while (GOB_PARENT(wingob) && GOB_PARENT(wingob) != Gob_Root
			&& GOB_PARENT(wingob) != wingob) // avoid infinite loop
		{
			wingob = GOB_PARENT(wingob);
		}
		//check if it is really open
		if (!IS_WINDOW(wingob) || !GET_GOB_STATE(wingob, GOBS_OPEN)) {
			return;
		}
	}
	
	//if (invalidate) {
	//	RECT rect;
	//	rect.left   = GOB_X_INT(gob);
	//	rect.right  = rect.left + GOB_W_INT(gob);
	//	rect.top    = GOB_Y_INT(gob);
	//	rect.bottom = rect.top + GOB_H_INT(gob);
	//	InvalidateRect(GOB_HWIN(wingob), NULL, FALSE);
	//	UpdateWindow(GOB_HWIN(wingob));
	//} else {
		//render and blit the GOB
	//	compositor = GOB_COMPOSITOR(wingob);
	//	OS_Compose_Gob(compositor, wingob, gob, FALSE);
	//}

	InvalidateRect(GOB_HWIN(wingob), NULL, FALSE);
	//	UpdateWindow(GOB_HWIN(wingob));
}


/***********************************************************************
**
*/	void Paint_Window(HWND window)
/*
**		Repaint the window by redrawing all the gobs.
**		It just blits the whole window buffer.
**
***********************************************************************/
{
	PAINTSTRUCT ps;
	REBGOB *wingob;
	REBCMP* cmp;

	wingob = (REBGOB *)GetWindowLongPtr(window, GWLP_USERDATA);

	if (wingob) {
		BeginPaint(window, (LPPAINTSTRUCT) &ps);

		cmp = GOB_COMPOSITOR(wingob);

		//printf("PS: %f %f %f %f - %x %x\n", ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom, ps.hdc, cmp->wind_DC);

		

		//printf("erase %i %i %i %i\n", cmp->win_rect.left, cmp->win_rect.top, cmp->win_rect.right, cmp->win_rect.bottom);
		//HBRUSH TransperrantBrush = CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION)); //(HBRUSH)GetStockObject(NULL_BRUSH); //
		//SetBkMode(cmp->back_DC, OPAQUE);
		//FillRect(cmp->back_DC, &cmp->win_rect, TransperrantBrush);
		//FillRect(cmp->wind_DC, &cmp->win_rect, TransperrantBrush);
		//DeleteObject(TransperrantBrush);

		BitBlt( cmp->wind_DC, 0, 0, GOB_LOG_W_INT(cmp->wind_gob), GOB_LOG_H_INT(cmp->wind_gob), cmp->wind_DC, 0, 0, SRCERASE); 
//		BitBlt( cmp->back_DC, 0, 0, GOB_LOG_W_INT(cmp->wind_gob), GOB_LOG_H_INT(cmp->wind_gob), cmp->back_DC, 0, 0, SRCERASE); 

		FillRect(cmp->back_DC, &cmp->win_rect, cmp->brush_DC);
		
		OS_Compose_Gob(cmp, wingob, wingob, FALSE);

		//OS_Blit_Window(compositor);
		
		BitBlt(
			cmp->wind_DC,
			0, 0,
			GOB_LOG_W_INT(cmp->wind_gob), GOB_LOG_H_INT(cmp->wind_gob),
			cmp->back_DC,
			0, 0,
			SRCCOPY
		);
		
		EndPaint(window, (LPPAINTSTRUCT) &ps);
	}
}


/***********************************************************************
**
*/  REBINT OS_Show_Gob(REBGOB *gob)
/*
**	Notes:
**		1.	Can be called with NONE (0), Gob_Root (All), or a
**			specific gob to open, close, or refresh.
**
**		2.	A new window will be in Gob_Root/pane but will not
**			have GOBF_WINDOW set.
**
**		3.	A closed window will have no PARENT and will not be
**			in the Gob_Root/pane but will have GOBF_WINDOW set.
**
***********************************************************************/
{
	REBINT n;
	REBGOB *g;
	REBGOB **gp;

	if (!gob) return 0;

	// Are we asked to open/close/refresh all windows?
	if (gob == Gob_Root) {  // show none, and show screen-gob

		// Remove any closed windows:
		for (n = 0; n < MAX_WINDOWS; n++) {
			if (g = Gob_Windows[n].gob) {
				if (!GOB_PARENT(g) && GET_GOB_FLAG(g, GOBF_WINDOW))
					OS_Close_Window(g);
			}
		}

		// Open any new windows:
		if (GOB_PANE(Gob_Root)) {
			gp = GOB_HEAD(Gob_Root);
			for (n = GOB_TAIL(Gob_Root)-1; n >= 0; n--, gp++) {
				if (!GET_GOB_FLAG(*gp, GOBF_WINDOW)) {
					OS_Open_Window(*gp);
					OS_Draw_Window(0, *gp, FALSE);
				}
			}
		}
		return 0;
	}
	// Is it a native widget?
	else if (GOBT_WIDGET == GOB_TYPE(gob)) {
		RedrawWindow((HWND)VAL_HANDLE(GOB_WIDGET_HANDLE(gob)), NULL, NULL, RDW_INVALIDATE);// | RDW_ERASE);
	}
	// Is it a window gob that needs to be closed?
	else if (!GOB_PARENT(gob) && GET_GOB_FLAG(gob, GOBF_WINDOW)) {
		OS_Close_Window(gob);
		return 0;
	}
	// Is it a window gob that needs to be opened or refreshed?
	else if (GOB_PARENT(gob) == Gob_Root) {
		if (!GET_GOB_FLAG(gob, GOBF_WINDOW))
			OS_Open_Window(gob);
		else
			OS_Update_Window(gob); // Problem! We may not want this all the time.
	}

	// Otherwise, composite and referesh the gob or all gobs:
	OS_Draw_Window(0, gob, TRUE);  // 0 = window parent of gob
	
	return 0;
}


/***********************************************************************
**
*/  void* OS_Init_Gob_Widget(REBCMP *ctx, REBGOB *gob)
/*
**      Creates native GUI widget
**
***********************************************************************/
{
	HWND hWnd;
	REBCHR *class;
	REBCHR *text = NULL;
	REBI64 value_i64 = 0;
	REBDEC value_dec = 0.0;
	REBINT range;
	REBOOL vertical;
	REBFLG  style = WS_CHILD | WS_VISIBLE;  // common flags
	REBFLG xstyle = 0;
	REBVAL *hndl  = GOB_WIDGET_HANDLE(gob);
	REBVAL *type  = GOB_WIDGET_TYPE(gob);
	REBVAL *spec  = GOB_WIDGET_SPEC(gob);

	if (!IS_HANDLE(hndl) || !IS_BLOCK(spec)) return NULL;
	REBVAL *val   = BLK_HEAD(VAL_SERIES(spec));

	if (IS_WORD(val) || IS_LIT_WORD(val)) { // ...where the first value is the type of the widget
		SET_INTEGER(type, (REBI64)RL_Find_Word(window_ext_words, val->data.word.sym));
	} else {
		return NULL;
	}

	while (!IS_END(++val)) {
		if (IS_STRING(val)) {
			RL_Get_String(VAL_SERIES(val), 0, (void**)&text, TRUE);
		} else if (IS_INTEGER(val)) {
			value_i64 = VAL_INT64(val);
		} else if (IS_DECIMAL(val)) {
			value_dec = VAL_DECIMAL(val);
		}
	}

	switch (VAL_INT64(type)) {
		//case W_WINDOW_BASE:

		//	break;
		case W_WINDOW_BUTTON:
			class = TEXT("BUTTON"); //(REBCHR*)Class_Name_Button;
			style |= WS_TABSTOP | BS_PUSHBUTTON; // | BS_DEFPUSHBUTTON;
			break;
		case W_WINDOW_CHECK:
			class = TEXT("BUTTON"); //(REBCHR*)Class_Name_Button;
			style |= WS_TABSTOP | BS_AUTOCHECKBOX;
			break;
		case W_WINDOW_RADIO:
			class = TEXT("BUTTON"); //(REBCHR*)Class_Name_Button;
			style |= WS_TABSTOP | BS_AUTORADIOBUTTON;
			break;
		case W_WINDOW_FIELD:
			class = TXT("edit");
			style  |= ES_LEFT | ES_AUTOHSCROLL | ES_NOHIDESEL;
			xstyle |= WS_EX_CLIENTEDGE;
			break;
		case W_WINDOW_AREA:
			class = TXT("edit");
			style  |= ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL
				   | WS_HSCROLL | WS_VSCROLL | ES_NOHIDESEL;
			xstyle |= WS_EX_CLIENTEDGE;
			break;
		case W_WINDOW_TEXT:
			class = TXT("STATIC");
			style |= SS_SIMPLE;
			break;
		case W_WINDOW_TEXT_LIST:
			class = TXT("ListBox");
			style  |= LBS_NOTIFY | WS_HSCROLL | WS_VSCROLL | LBS_NOINTEGRALHEIGHT;
			xstyle |= WS_EX_CLIENTEDGE;
			break;
		case W_WINDOW_PROGRESS:
			class = TXT("msctls_progress32");
			if (gob->size.y > gob->size.x) style |= PBS_VERTICAL;
			break;
		case W_WINDOW_SLIDER:
			class = TXT("msctls_trackbar32");
			vertical = gob->size.y > gob->size.x;
			if (vertical) style |= TBS_VERT | TBS_DOWNISLEFT;
			break;
		case W_WINDOW_DATE_TIME:
			class = TXT("SysDateTimePick32");
			vertical = gob->size.y > gob->size.x;
			style |= WS_BORDER; //| DTS_SHOWNONE;
			break;
		case W_WINDOW_GROUP_BOX:
			class = TXT("BUTTON");
			style |= BS_GROUPBOX;
			break;
		case W_WINDOW_OPENGL:
			class = TXT("RebOpenGL");
			style |= CS_OWNDC;
			break;
		default:
			//RL_Print("unknown widget name");
			return NULL;
	}
	
	hWnd = CreateWindowEx(
		xstyle,
		class,
		text,
		style,
		gob->offset.x, gob->offset.y,       // position
		gob->size.x, gob->size.y,           // size
		WindowFromDC(ctx->wind_DC),         // parent
		(HMENU)0,
		App_Instance, NULL);

	VAL_HANDLE(hndl) = (ANYFUNC)hWnd;
	SendMessage(hWnd, WM_SETFONT, (WPARAM)Default_Font, 0);
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)gob);

	//printf("======== NEW widget: %08Xh for gob: %08Xh\n", hWnd, gob);

	switch (VAL_INT64(type)) {
	case W_WINDOW_BUTTON:

		//SendMessage(hWnd, BCM_FIRST + 0x0009, 0, text);
		//Button_SetNote(hWnd, text);
		break;
	case W_WINDOW_PROGRESS:
		SendMessage(hWnd, PBM_SETPOS, value_i64, 0);
		break;
	case W_WINDOW_SLIDER:
		range = ROUND_TO_INT(vertical ? gob->size.y : gob->size.x);
		SendMessage(hWnd, TBM_SETRANGE, 0, (LPARAM)MAKELONG(0, range));
		SendMessage(hWnd, TBM_SETPAGESIZE, 0, (LPARAM)4);
		//SendMessage(hWnd, TBM_SETSEL, 0, (LPARAM)MAKELONG(0, range));
		SendMessage(hWnd, TBM_SETPOS, 1, (LPARAM)(value_dec * (REBDEC)range));
		RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
		break;
	}

	return hWnd;
}


/***********************************************************************
**
*/	REBOOL OS_Get_Widget_Data(REBGOB *gob, REBVAL *ret)
/*
**	Returns data according the widget type
**
***********************************************************************/
{
	REBVAL *hndl = GOB_WIDGET_HANDLE(gob);
	REBVAL *type = GOB_WIDGET_TYPE(gob);
	REBVAL *data;

	HWND hWnd = (HWND)VAL_HANDLE(hndl);
	LRESULT res;
	REBDEC range;
	REBCNT count;
	SYSTEMTIME dat;

	//printf("OS_Get_Widget_Data type: %d\n", VAL_INT64(type));

	switch (VAL_INT64(type)) {

	case W_WINDOW_PROGRESS:
		res = SendMessage(hWnd, PBM_GETPOS, 0, 0);
		SET_INTEGER(ret, (REBU64)res);
		return TRUE;

	case W_WINDOW_SLIDER:
		range = (REBDEC)SendMessage(hWnd, TBM_GETRANGEMAX, 0, 0);
		if (range > 0) {
			res = SendMessage(hWnd, TBM_GETPOS, 0, 0);
			range = (REBDEC)res / range;
		}
		if (gob->size.y > gob->size.x) range = 1 - range;
		SET_DECIMAL(ret, range);
		return TRUE;

	case W_WINDOW_FIELD:
	case W_WINDOW_AREA:
		data = GOB_WIDGET_DATA(gob);
		count = Get_Widget_Text(hWnd, data);
		SET_STRING(ret, VAL_SERIES(data));
		VAL_TAIL(ret) = count;
		return TRUE;

	case W_WINDOW_DATE_TIME:
		data = GOB_WIDGET_DATA(gob);
		res = SendMessage(hWnd, DTM_GETSYSTEMTIME, 0, (LPARAM)&dat);
		if (GDT_VALID == res) {
			VAL_SET(data, REB_DATE);
			VAL_YEAR(data) = dat.wYear;
			VAL_MONTH(data) = dat.wMonth;
			VAL_DAY(data) = dat.wDay;
			VAL_ZONE(data) = 0;
			VAL_TIME(data) = NO_TIME;
			//VAL_TIME(data) = TIME_SEC(dat.wHour * 3600 + dat.wMinute * 60 + dat.wSecond) + 1000000 * dat.wMilliseconds;
			*ret = *data;
			return TRUE;
		}
		break;
	}
	//@@ throw an error or return NONE when unhandled type?
	SET_NONE(ret);
	return TRUE;
	//return FALSE;
}


/***********************************************************************
**
*/	REBOOL OS_Set_Widget_Data(REBGOB *gob, REBVAL *data)
/*
**	Returns data according the widget type
**
***********************************************************************/
{
	REBVAL *hndl = GOB_WIDGET_HANDLE(gob);
	REBVAL *type = GOB_WIDGET_TYPE(gob);
	
	HWND hWnd = (HWND)VAL_HANDLE(hndl);

	switch (VAL_INT64(type)) {
		case W_WINDOW_PROGRESS:
			if (IS_INTEGER(data)) {
				SendMessage(hWnd, PBM_SETPOS, VAL_INT32(data), 0);
				return TRUE;
			}
			break;
		case W_WINDOW_CHECK:
		case W_WINDOW_RADIO:
			if (IS_LOGIC(data)) {
				SendMessage(hWnd, BM_SETCHECK, VAL_LOGIC(data), 0);
				return TRUE;
			}
			break;
	}
	return FALSE;
}


/***********************************************************************
**
*/	REBD32 OS_Get_Metrics(METRIC_TYPE type, REBINT display)
/*
**	Provide OS specific UI related information.
**
***********************************************************************/
{
	REBD32 result = 0;
	switch (type) {
	case SM_VIRTUAL_SCREEN_WIDTH:
		result = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		break;
	case SM_VIRTUAL_SCREEN_HEIGHT:
		result = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		break;
	case SM_SCREEN_WIDTH:
		result = GetSystemMetrics(SM_CXSCREEN);
		break;
	case SM_SCREEN_HEIGHT:
		result = GetSystemMetrics(SM_CYSCREEN);
		break;
	case SM_WORK_WIDTH:
	{
		RECT rect;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
		result = rect.right;
	}
	break;
	case SM_WORK_HEIGHT:
	{
		RECT rect;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
		result = rect.bottom;
	}
	break;
	case SM_TITLE_HEIGHT:
		result = GetSystemMetrics(SM_CYCAPTION);
		break;
	case SM_SCREEN_DPI_X:
	{
		HDC hDC = GetDC(NULL);
		result = GetDeviceCaps(hDC, LOGPIXELSX);
		ReleaseDC(NULL, hDC);
	}
	break;
	case SM_SCREEN_DPI_Y:
	{
		HDC hDC = GetDC(NULL);
		result = GetDeviceCaps(hDC, LOGPIXELSY);
		ReleaseDC(NULL, hDC);
	}
	break;
	case SM_BORDER_WIDTH:
		result = GetSystemMetrics(SM_CXSIZEFRAME);
		break;
	case SM_BORDER_HEIGHT:
		result = GetSystemMetrics(SM_CYSIZEFRAME);
		break;
	case SM_BORDER_FIXED_WIDTH:
		result = GetSystemMetrics(SM_CXFIXEDFRAME);
		break;
	case SM_BORDER_FIXED_HEIGHT:
		result = GetSystemMetrics(SM_CYFIXEDFRAME);
		break;
	case SM_WINDOW_MIN_WIDTH:
		result = GetSystemMetrics(SM_CXMIN);
		break;
	case SM_WINDOW_MIN_HEIGHT:
		result = GetSystemMetrics(SM_CYMIN);
		break;
	case SM_WORK_X:
	{
		RECT rect;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
		result = rect.left;
	}
	break;
	case SM_WORK_Y:
	{
		RECT rect;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
		result = rect.top;
	}
	break;
	}
	return result;
}


/***********************************************************************
**
*/	void OS_Set_Cursor(void *cursor)
/*
**
**
***********************************************************************/
{
	SetCursor((HCURSOR)cursor);
}

/***********************************************************************
**
*/	void* OS_Load_Cursor(void *cursor)
/*
**
**
***********************************************************************/
{
	return (void*)LoadCursor(NULL, (LPCTSTR)cursor);
}

/***********************************************************************
**
*/	void OS_Destroy_Cursor(void *cursor)
/*
**
**
***********************************************************************/
{
	DestroyCursor((HCURSOR)Cursor);
}


/***********************************************************************
**
*/	void* OS_Image_To_Cursor(REBYTE* image, REBINT width, REBINT height)
/*
**      Converts REBOL image! to Windows CURSOR
**
***********************************************************************/
{
	int xHotspot = 0;
	int yHotspot = 0;

	HICON result = NULL;
	HBITMAP hSourceBitmap;
	BITMAPINFO  BitmapInfo;
	ICONINFO iconinfo;

    //Get the system display DC
    HDC hDC = GetDC(NULL);

	//Create DIB
	unsigned char* ppvBits;
	int bmlen = width * height * 4;
	int i;

	BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BitmapInfo.bmiHeader.biWidth = width;
	BitmapInfo.bmiHeader.biHeight = -(signed)height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	BitmapInfo.bmiHeader.biSizeImage = 0;
	BitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	BitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	BitmapInfo.bmiHeader.biClrUsed = 0;
	BitmapInfo.bmiHeader.biClrImportant = 0;

	hSourceBitmap = CreateDIBSection(hDC, &BitmapInfo, DIB_RGB_COLORS, (void**)&ppvBits, NULL, 0);
	if (!hSourceBitmap) return NULL;

	//Release the system display DC
    ReleaseDC(NULL, hDC);

	//Copy the image content to DIB
	COPY_MEM(ppvBits, image, bmlen);

	//Invert alphachannel from the REBOL format
	for (i = 3;i < bmlen;i+=4){
		ppvBits[i] ^= 0xff;
	}

	//Create the cursor using the masks and the hotspot values provided
	iconinfo.fIcon		= FALSE;
	iconinfo.xHotspot	= xHotspot;
	iconinfo.yHotspot	= yHotspot;
	iconinfo.hbmMask	= hSourceBitmap;
	iconinfo.hbmColor	= hSourceBitmap;

	result = CreateIconIndirect(&iconinfo);

	DeleteObject(hSourceBitmap);

	return result;
}

//**********************************************************************
//** Window commands! dipatcher **************************************
//**********************************************************************


/***********************************************************************
**
*/	RXIEXT int RXD_Window(int cmd, RXIFRM *frm, REBCEC *data)
/*
**		Window command extension dispatcher.
**
***********************************************************************/
{
	REBINT display = 0;
	switch (cmd) {
	case CMD_WINDOW_SHOW:
	{
		if (RXA_TYPE(frm, 1) == RXT_GOB) {
			REBGOB* gob = (REBGOB*)RXA_SERIES(frm, 1);
			OS_Show_Gob(gob);
		}
		return RXR_VALUE;
	}
	case CMD_WINDOW_GUI_METRIC:
	{

		REBD32 x, y;
		u32 w = RL_Find_Word(window_ext_words, RXA_WORD(frm, 1));

		if (RXA_TYPE(frm, 5) == RXT_INTEGER) {
			display = RXA_INT32(frm, 5);
		}

		switch (w)
		{
		case W_WINDOW_SCREEN_SIZE:
			x = PHYS_COORD_X(OS_Get_Metrics(SM_SCREEN_WIDTH, display));
			y = PHYS_COORD_Y(OS_Get_Metrics(SM_SCREEN_HEIGHT, display));
			break;

		case W_WINDOW_LOG_SIZE:
#if 0
			if (RXA_TYPE(frm, 3) == RXT_PAIR) {
				log_size.x = RXA_PAIR(frm, 3).x;
				log_size.y = RXA_PAIR(frm, 3).y;
				phys_size.x = 1 / log_size.x;
				phys_size.y = 1 / log_size.y;
			}
#endif
			x = log_size.x;
			y = log_size.y;
			break;

		case W_WINDOW_PHYS_SIZE:
			x = phys_size.x;
			y = phys_size.y;
			break;

		case W_WINDOW_TITLE_SIZE:
			x = 0;
			y = PHYS_COORD_Y(OS_Get_Metrics(SM_TITLE_HEIGHT, display));
			break;

		case W_WINDOW_BORDER_SIZE:
			x = OS_Get_Metrics(SM_BORDER_WIDTH, display);
			y = OS_Get_Metrics(SM_BORDER_HEIGHT, display);
			break;

		case W_WINDOW_BORDER_FIXED:
			x = OS_Get_Metrics(SM_BORDER_FIXED_WIDTH, display);
			y = OS_Get_Metrics(SM_BORDER_FIXED_HEIGHT, display);
			break;

		case W_WINDOW_WINDOW_MIN_SIZE:
			x = OS_Get_Metrics(SM_WINDOW_MIN_WIDTH, display);
			y = OS_Get_Metrics(SM_WINDOW_MIN_HEIGHT, display);
			break;

		case W_WINDOW_WORK_ORIGIN:
			x = OS_Get_Metrics(SM_WORK_X, display);
			y = OS_Get_Metrics(SM_WORK_Y, display);
			break;

		case W_WINDOW_WORK_SIZE:
			x = PHYS_COORD_X(OS_Get_Metrics(SM_WORK_WIDTH, display));
			y = PHYS_COORD_Y(OS_Get_Metrics(SM_WORK_HEIGHT, display));
			break;

		case W_WINDOW_SCREEN_DPI:
			x = OS_Get_Metrics(SM_SCREEN_DPI_X, display);
			y = OS_Get_Metrics(SM_SCREEN_DPI_Y, display);
			break;

		case W_WINDOW_SCREEN_ORIGIN:
			x = OS_Get_Metrics(SM_SCREEN_X, display);
			y = OS_Get_Metrics(SM_SCREEN_Y, display);
			break;

		case W_WINDOW_SCREENS:
			// get number of display monitors
			// NOTE: it shoould be provided in some better way!
			RXA_INT64(frm, 1) = GetSystemMetrics(SM_CMONITORS);
			RXA_TYPE(frm, 1) = RXT_INTEGER;
			return RXR_VALUE;
		default:
			return RXR_BAD_ARGS;
		}

		if (w) {
			RXA_PAIR(frm, 1).x = x;
			RXA_PAIR(frm, 1).y = y;
			RXA_TYPE(frm, 1) = RXT_PAIR;
		}
		else {
			RXA_TYPE(frm, 1) = RXT_NONE;
		}
		return RXR_VALUE;

	}
	break;

	case CMD_WINDOW_INIT_TOP_WINDOW:
		Gob_Root = (REBGOB*)RXA_SERIES(frm, 1); // system/view/screen-gob
		Gob_Root->parent = NULL;
		Gob_Root->size.x = OS_Get_Metrics(SM_SCREEN_WIDTH, display);
		Gob_Root->size.y = OS_Get_Metrics(SM_SCREEN_HEIGHT, display);
#ifdef unused
		//Initialize text rendering context
		if (Rich_Text) Destroy_RichText(Rich_Text);
		Rich_Text = Create_RichText();
#endif
		break;

	case CMD_WINDOW_INIT_WORDS:
		window_ext_words = RL_Map_Words(RXA_SERIES(frm, 1));
		break;

	case CMD_WINDOW_CURSOR:
	{
		REBUPT n = 0;
		REBYTE *image = NULL;

		if (RXA_TYPE(frm, 1) == RXT_IMAGE) {
			image = RXA_IMAGE_BITS(frm, 1);
		}
		else {
			n = RXA_INT64(frm, 1);
		}

		if (Custom_Cursor) {
			//Destroy cursor object only if it is a custom image
			OS_Destroy_Cursor(Cursor);
			Custom_Cursor = FALSE;
		}

		if (n > 0)
			Cursor = OS_Load_Cursor((void*)n);
		else if (image) {
			Cursor = OS_Image_To_Cursor(image, RXA_IMAGE_WIDTH(frm, 1), RXA_IMAGE_HEIGHT(frm, 1));
			Custom_Cursor = TRUE;
		}
		else
			Cursor = NULL;

		OS_Set_Cursor(Cursor);
	}
	break;

	default:
		return RXR_NO_COMMAND;
	}
	return RXR_UNSET;
}


/***********************************************************************
**
*/	void Init_Windows(void)
/*
**	Initialize special variables of the graphics subsystem.
**
***********************************************************************/
{
	RL = RL_Extend((REBYTE *)(&RX_window[0]), &RXD_Window);

	Gob_Windows = OS_Make(sizeof(struct gob_window) * (MAX_WINDOWS + 1));
	CLEAR(Gob_Windows, sizeof(struct gob_window) * (MAX_WINDOWS + 1));
	Cursor = LoadCursor(NULL, IDC_ARROW);
	Init_DPI_Awareness();

	Windows8_And_Newer = IsWindows8OrGreater();
	
	if (IsWindowsXPOrGreater()) {
		// Enable visual styles (not for Win2000)
		INITCOMMONCONTROLSEX InitCtrlEx;
		InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
		InitCtrlEx.dwICC = ICC_STANDARD_CLASSES
						 | ICC_LINK_CLASS
						 | ICC_UPDOWN_CLASS
						 | ICC_LISTVIEW_CLASSES
						 | ICC_PROGRESS_CLASS
						 | ICC_BAR_CLASSES
						 | ICC_DATE_CLASSES;
		if (!InitCommonControlsEx(&InitCtrlEx)) {
			RL_Print("Could not initialize common controls!\n");
		}
	}
}

/***********************************************************************
**
*/	void Dispose_Windows(void)
/*
***********************************************************************/
{
	OS_Free(Gob_Windows);
	OS_Free(window_ext_words);
	Gob_Windows = NULL;
	window_ext_words = NULL;
}


/**************** helpers ********************/

static REBCNT Get_Widget_Text(HWND widget, REBVAL *text)
{
	REBCNT count = SendMessage(widget, WM_GETTEXTLENGTH, -1, 0);
	
	if (!IS_STRING(text)) {
		SET_STRING(text, RL_Make_String(count, TRUE));
	}
	else if (count > VAL_REST(text)) {
		RL_Expand_Series(VAL_SERIES(text), VAL_TAIL(text), count - VAL_REST(text));
	}
	SendMessage(widget, WM_GETTEXT, count + 1, (LPARAM)VAL_BIN(text));
	return count;
}



#ifdef NOT_USED_BUT_MAYBE_LATER

HWND   Main_Event_Window;
REBGOB Main_Window_GOB;

/***********************************************************************
**
xx	void Init_Event_Window(void)
/*
**		We need to do this in order to get certain events even
**		if there is no window is open (async DSN for example).
**
***********************************************************************/
{
	if (Main_Event_Window) return;

	SET_GOB_FLAG(&Main_Window_GOB, GOBF_HIDDEN);

	Main_Event_Window = OS_Open_Window(&Main_Window_GOB);
}


/***********************************************************************
**
xx	static REBINT Find_Gob(REBGOB *gob, REBGOB *target)
/*
**		Find a target GOB within the pane of another gob.
**		Return the index, or a -1 if not found.
**
***********************************************************************/
{
	REBINT len;
	REBINT n;
	REBGOB **ptr;

	len = GOB_TAIL(gob);
	ptr = GOB_HEAD(gob);
	for (n = 0; n < len; n++, ptr++)
		if (*ptr == target) return n;

	return -1;
}


/***********************************************************************
**
xx	void OS_Get_Window_Size(REBGOB *gob, REBINT *w, REBINT *h)
/*
**	Get the window size.
**
***********************************************************************/
{
	RECT r;

	if (!IS_WINDOW(gob)) return;

	GetClientRect(GOB_HWIN(gob), &r);
	*w = r.right;
	*h = r.bottom;
}


/***********************************************************************
**
xx  void OS_Scroll_Bits(REBGOB *gob, REBINT x, REBINT y, REBINT w, REBINT h, REBINT dx, REBINT dy)
/*
***********************************************************************/
{
	HDC hdc;
	RECT r={x,y,x+w,y+h};
	HRGN updatergn,oldrgn,newrgn;

	if (!IS_WINDOW(gob)) return;

	updatergn = CreateRectRgn(0,0,0,0);
	hdc = GetDC(GOB_HWIN(gob));
	ScrollDC(hdc,dx,dy,&r,0,updatergn,0);
	ReleaseDC(GOB_HWIN(gob),hdc);
	oldrgn = CreateRectRgn(x,y,x+w,y+h);
	newrgn = CreateRectRgn(x+dx,y+dy,x+w+dx,y+h+dy);
	CombineRgn(oldrgn,oldrgn,newrgn,RGN_DIFF);
	if (NULLREGION != CombineRgn(updatergn,updatergn,oldrgn,RGN_DIFF))
		InvalidateRgn(GOB_HWIN(gob),updatergn,FALSE);
	DeleteObject(newrgn);
	DeleteObject(oldrgn);
	DeleteObject(updatergn);
}

#endif
