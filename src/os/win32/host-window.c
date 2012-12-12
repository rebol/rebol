/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  REBOL is a trademark of REBOL Technologies
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

#include <windows.h>
#include <math.h>

#include "reb-host.h"
#include "host-lib.h"

#ifndef NO_COMPOSITOR
#include "agg-compositor.h"
#endif

//***** Constants *****

#define MAX_WINDOWS 64
#define GOB_HWIN(gob)	(Find_Window(gob))
#define GOB_COMPOSITOR(gob)	(Find_Compositor(gob)) //gets handle to window's compositor

struct gob_window {REBGOB *gob; HWND win; void* compositor;}; // Maps gob to window

//***** Externs *****

extern HINSTANCE App_Instance;		// Set by winmain function
extern void Host_Crash(char *reason);
extern LRESULT CALLBACK REBOL_Window_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern MSG Handle_Messages();

#ifdef TEMP_REMOVED
extern void* Create_RichText();
extern void* Create_Effects();
#endif

//***** Locals *****

static BOOL Registered = FALSE;		// Window has been registered
static const REBCHR *Window_Class_Name = TXT("REBOLWindow");
static struct gob_window *Gob_Windows;

REBGOB *Gob_Root;				// Top level GOB (the screen)
HCURSOR Cursor;					// active cursor image object
REBPAR Zero_Pair = {0, 0};
//void* Effects;


/***********************************************************************
**
*/	REBOOL As_OS_Str(REBSER *series, REBCHR **string)
/*
**	If necessary, convert a string series to Win32 wide-chars.
**  (Handy for GOB/TEXT handling).
**  If the string series is empty the resulting string is set to NULL
**
**  Function returns:
**      TRUE - if the resulting string needs to be deallocated by the caller code
**      FALSE - if REBOL string is used (no dealloc needed)
**
**  Note: REBOL strings are allowed to contain nulls.
**
***********************************************************************/
{
	int len, n;
	void *str;
	wchar_t *wstr;

	if ((len = RL_Get_String(series, 0, &str)) < 0) {
		// Latin1 byte string - convert to wide chars
		len = -len;
		wstr = OS_Make((len+1) * sizeof(wchar_t));
		for (n = 0; n < len; n++)
			wstr[n] = (wchar_t)((char*)str)[n];
		wstr[len] = 0;
		//note: following string needs be deallocated in the code that uses this function
		*string = (REBCHR*)wstr;
		return TRUE;
	}
	*string = (len == 0) ? NULL : str; //empty string check
	return FALSE;
}


/***********************************************************************
**
*/	void Init_Windows(void)
/*
**	Initialize special variables of the graphics subsystem.
**
***********************************************************************/
{
	Gob_Windows = OS_Make(sizeof(struct gob_window) * (MAX_WINDOWS+1));
	CLEAR(Gob_Windows, sizeof(struct gob_window) * (MAX_WINDOWS+1));

	Cursor = LoadCursor(NULL, IDC_ARROW);
}


/**********************************************************************
**
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

static REBINT Alloc_Window(REBGOB *gob) {
	int n;
	for (n = 0; n < MAX_WINDOWS; n++) {
		if (Gob_Windows[n].gob == 0) {
			Gob_Windows[n].gob = gob;
#ifndef NO_COMPOSITOR
			Gob_Windows[n].compositor = Create_Compositor(Gob_Root, gob);
//			Reb_Print("Create_Compositor %d", Gob_Windows[n].compositor);
#endif
			return n;
		}
	}
	return -1;
}

static HWND Find_Window(REBGOB *gob) {
	int n;
	for (n = 0; n < MAX_WINDOWS; n++) {
		if (Gob_Windows[n].gob == gob) return Gob_Windows[n].win;
	}
	return 0;
}

static HWND Find_Compositor(REBGOB *gob) {
	int n;
	for (n = 0; n < MAX_WINDOWS; n++) {
		if (Gob_Windows[n].gob == gob) return Gob_Windows[n].compositor;
	}
	return 0;
}

static void Free_Window(REBGOB *gob) {
	int n;
	for (n = 0; n < MAX_WINDOWS; n++) {
		if (Gob_Windows[n].gob == gob) {
#ifndef NO_COMPOSITOR
			Destroy_Compositor(Gob_Windows[n].compositor);
//			Reb_Print("Destroy_Compositor %d", Gob_Windows[n].compositor);
#endif
			Gob_Windows[n].gob = 0;
			return;
		}
	}
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
	wc.lpszClassName = Window_Class_Name;
	wc.hInstance     = App_Instance;
	wc.lpfnWndProc   = REBOL_Window_Proc;

	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.lpszMenuName  = NULL;

	wc.hIconSm = LoadImage(App_Instance, // small class icon
		MAKEINTRESOURCE(5),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_DEFAULTCOLOR
	);

	// If not already registered:
	//if (!GetClassInfo(App_Instance, Window_Class_Name, &wclass))
	//  RegisterClass(&wclass);

	if (!RegisterClassEx(&wc)) Host_Crash("Cannot register window");

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
*/  HWND Open_Window(REBGOB *gob)
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
	REBINT windex;
	HWND window;
	REBCHR *title;
	int x, y, w, h;
	HWND parent = NULL;
	REBYTE osString = FALSE;
    REBPAR metric;

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

	if (!GET_FLAGS(gob->flags, GOBF_NO_TITLE, GOBF_NO_BORDER)) {
	    metric.y = GetSystemMetrics(SM_CYCAPTION);
		options |= WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU;
		h += metric.y;
		y -= metric.y;
	}

	if (GET_GOB_FLAG(gob, GOBF_RESIZE)) {
	    metric.x = GetSystemMetrics(SM_CXSIZEFRAME);
	    metric.y = GetSystemMetrics(SM_CYSIZEFRAME);
		options |= WS_SIZEBOX | WS_BORDER;
		x -= metric.x;
		y -= metric.y;
		w += metric.x * 2;
		h += metric.y * 2;
		if (!GET_GOB_FLAG(gob, GOBF_NO_TITLE))
			options |= WS_MAXIMIZEBOX;
	}
	else if (!GET_GOB_FLAG(gob, GOBF_NO_BORDER)) {
	    metric.x = GetSystemMetrics(SM_CXFIXEDFRAME);
	    metric.y = GetSystemMetrics(SM_CYFIXEDFRAME);
		options |= WS_BORDER;
		if (!GET_GOB_FLAG(gob, GOBF_NO_TITLE)){
			x -= metric.x;
			y -= metric.y;
			w += metric.x * 2;
			h += metric.y * 2;
		}
	}

	if (IS_GOB_STRING(gob))
        osString = As_OS_Str(GOB_CONTENT(gob), (REBCHR**)&title);
    else
        title = TXT("REBOL Window");

	if (GET_GOB_FLAG(gob, GOBF_POPUP)) {
		parent = GOB_HWIN(GOB_TMP_OWNER(gob));
		if (GET_GOB_FLAG(gob, GOBF_MODAL)) {
			EnableWindow(parent, FALSE);
			EnumWindows(EnumWindowsProc, (LPARAM)parent);
		}
	}

	// Create the window:
	window = CreateWindowEx(
		WS_EX_WINDOWEDGE,
		Window_Class_Name,
		title,
		options,
		x, y, w, h,
		parent,
		NULL, App_Instance, NULL
	);

    //don't let the string leak!
    if (osString) OS_Free(title);
	if (!window) Host_Crash("CreateWindow failed");

	// Enable drag and drop
	if (GET_GOB_FLAG(gob, GOBF_DROPABLE))
		DragAcceptFiles(window, TRUE);

	Gob_Windows[windex].win = window;
	SET_GOB_FLAG(gob, GOBF_WINDOW);
	SET_GOB_STATE(gob, GOBS_OPEN);

	// Provide pointer from window back to REBOL window:
	SetWindowLong(window, GWL_USERDATA, (long)gob);

	if (!GET_GOB_FLAG(gob, GOBF_HIDDEN)) {
		if (GET_GOB_FLAG(gob, GOBF_ON_TOP)) SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		ShowWindow(window, SW_SHOWNORMAL);
		SetForegroundWindow(window);
	}

	return window;
}

/* Removed from above code -- is any of this really needed? -CS

	// Set rectangle coordinates:
	rect.left   = GOB_X(gob);
	rect.right  = rect.left + GOB_W(gob);
	rect.top    = GOB_Y(gob);
	rect.bottom = rect.top + GOB_H(gob);
	AdjustWindowRect(&rect, options, FALSE);

	// Create window (use parent if specified):
	GOB_WIN(gob) = CreateWindow(Window_Class_Name, title, options,
		rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top,
		(wparent ? GOB_WIN(wparent) : NULL), NULL, App_Instance, NULL);

	// Drain startup messages:
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
*/


/***********************************************************************
**
*/  void Close_Window(REBGOB *gob)
/*
**		Close the window.
**
***********************************************************************/
{
	HWND parent = NULL;
	if (GET_GOB_FLAG(gob, GOBF_WINDOW) && Find_Window(gob)) {
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
		Free_Window(gob);
	}
}


/***********************************************************************
**
*/	REBOOL Resize_Window(REBGOB *gob, REBOOL redraw)
/*
**		Update window parameters.
**
***********************************************************************/
{
#ifndef NO_COMPOSITOR
	void *compositor;
	REBOOL changed;
	compositor = GOB_COMPOSITOR(gob);
	changed = Resize_Window_Buffer(compositor, gob);
	if (redraw) Compose_Gob(compositor, gob, gob);
	return changed;
#else
	REBINT Draw_Window(REBGOB *wingob, REBGOB *gob);
	Draw_Window(gob, gob);
	return TRUE;
#endif
}


/***********************************************************************
**
*/	void Update_Window(REBGOB *gob)
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
		Resize_Window(gob, FALSE);

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
        osString = As_OS_Str(GOB_CONTENT(gob), (REBCHR**)&title);
		SetWindowText(window, title);
		//don't let the string leak!
        if (osString) OS_Free(title);
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
*/	void Blit_Rect(REBGOB *gob, REBPAR d, REBPAR dsize, REBYTE *src, REBPAR s, REBPAR ssize)
/*
**		This routine copies a rectangle from a PAN structure to the
**		current output device.
**
***********************************************************************/
{
	HDC         hdc;
	BITMAPINFO  BitmapInfo;
	REBINT      mode;

	if (!IS_WINDOW(gob)) return;

	hdc = GetDC(GOB_HWIN(gob));

	mode = SetStretchBltMode(hdc, COLORONCOLOR);
	BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
#ifdef NO_COMPOSITOR
	BitmapInfo.bmiHeader.biWidth = ssize.x;
	BitmapInfo.bmiHeader.biHeight = -(REBINT)dsize.y;
#else
	BitmapInfo.bmiHeader.biWidth = ROUND_TO_INT(gob->size.x);
	BitmapInfo.bmiHeader.biHeight = -ROUND_TO_INT(gob->size.y);
#endif
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	BitmapInfo.bmiHeader.biSizeImage = 0;
	BitmapInfo.bmiHeader.biXPelsPerMeter = 1;
	BitmapInfo.bmiHeader.biYPelsPerMeter = 1;
	BitmapInfo.bmiHeader.biClrUsed = 0;
	BitmapInfo.bmiHeader.biClrImportant = 0;

// 	StretchDIBits(hdc, d.x, d.y, dsize.x, dsize.y, s.x, s.y, ssize.x, ssize.y, src, &BitmapInfo, DIB_PAL_COLORS, SRCCOPY);

	//we need little transformation to get rid of StretchDIBits() quirk when src.x and src.y = 0
  	StretchDIBits(hdc,
		d.x, d.y + dsize.y - 1, dsize.x, -dsize.y,
		s.x, s.y + ssize.y + 1, ssize.x, -ssize.y,
		src, &BitmapInfo, DIB_PAL_COLORS, SRCCOPY);

//	Reb_Print("blit: %dx%d %dx%d %dx%d %dx%d" ,d.x, d.y + dsize.y - 1, dsize.x, -dsize.y,s.x, ssize.y + s.y + 1, ssize.x, -ssize.y);

	SetStretchBltMode(hdc, mode);

	ReleaseDC(GOB_HWIN(gob), hdc);
}


/***********************************************************************
**
*/	void Blit_Color(REBGOB *gob, REBPAR d, REBPAR dsize, long color)
/*
**		Fill color rectangle, a pixel at a time.
**
***********************************************************************/
{
	HDC hdc;
	long clr;
	RECT rect;

	if (!IS_WINDOW(gob)) return;

	clr = ((color >> 16) & 255) | ((color & 255) << 16) | (color & 255<<8);

	hdc = GetDC(GOB_HWIN(gob));

	rect.left   = d.x;
	rect.top    = d.y;
	rect.right  = dsize.x+d.x; // see note on FillRect
	rect.bottom = dsize.y+d.y;

	//Reb_Print("rect: %dx%d %dx%d", rect.left, rect.top, rect.right, rect.bottom);

	FillRect(hdc, &rect, CreateSolidBrush(clr)); // excludes bottom & right borders
	ReleaseDC(GOB_HWIN(gob), hdc);
}


/***********************************************************************
**
*/	REBINT Draw_Window(REBGOB *wingob, REBGOB *gob)
/*
**		Refresh the GOB within the given window. If the wingob
**		is zero, then find the correct window for it.
**
***********************************************************************/
{
	REBINT len;

#ifdef NO_COMPOSITOR
	REBINT n;
	REBGOB **gp;
	//static int nnn = 0;
#else
	void *compositor;
#endif

	if (!wingob) {
		wingob = gob;
		while (GOB_PARENT(wingob) && GOB_PARENT(wingob) != Gob_Root
			&& GOB_PARENT(wingob) != wingob) // avoid infinite loop
			wingob = GOB_PARENT(wingob);

		//check if it is really open
		if (!IS_WINDOW(wingob) || !GET_GOB_STATE(wingob, GOBS_OPEN)) return 0;
	}

//	Reb_Print("draw: %d %8x", nnn++, gob);

#ifdef NO_COMPOSITOR
	// Blit the current gob:
	if (IS_GOB_IMAGE(gob)) {
		Blit_Rect(wingob, gob->offset, gob->size, GOB_BITMAP(gob), Zero_Pair, gob->size);
	}
	else { //if (IS_GOB_COLOR(gob))
		Blit_Color(wingob, gob->offset, gob->size, (long)GOB_CONTENT(gob));
	}

	// Blit the children:
	if (GOB_PANE(gob)) {
		len = GOB_TAIL(gob);
		gp = GOB_HEAD(gob);
		for (n = 0; n < len; n++, gp++)
			Draw_Window(wingob, *gp);
	}
	return 0;
#else
	//render and blit the GOB
	compositor = GOB_COMPOSITOR(wingob);
	len = Compose_Gob(compositor, wingob, gob);
	return len;
#endif
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
	REBGOB *gob;
	REBPAR size;

	gob = (REBGOB *)GetWindowLong(window, GWL_USERDATA);

	if (gob) {

		BeginPaint(window, (LPPAINTSTRUCT) &ps);

#ifdef NO_COMPOSITOR
		Draw_Window(gob, gob);
#else
		size.x = ROUND_TO_INT(gob->size.x);
		size.y = ROUND_TO_INT(gob->size.y);
		Blit_Rect(gob, Zero_Pair, size, Get_Window_Buffer(GOB_COMPOSITOR(gob)), Zero_Pair, size);
#endif

		EndPaint(window, (LPPAINTSTRUCT) &ps);
	}
}


/***********************************************************************
**
*/  REBINT Show_Gob(REBGOB *gob)
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
					Close_Window(g);
			}
		}

		// Open any new windows:
		if (GOB_PANE(Gob_Root)) {
			gp = GOB_HEAD(Gob_Root);
			for (n = GOB_TAIL(Gob_Root)-1; n >= 0; n--, gp++) {
				if (!GET_GOB_FLAG(*gp, GOBF_WINDOW))
					Open_Window(*gp);
					Draw_Window(0, *gp);
			}
		}
		return 0;
	}
	// Is it a window gob that needs to be closed?
	else if (!GOB_PARENT(gob) && GET_GOB_FLAG(gob, GOBF_WINDOW)) {
		Close_Window(gob);
		return 0;
	}
	// Is it a window gob that needs to be opened or refreshed?
	else if (GOB_PARENT(gob) == Gob_Root) {
		if (!GET_GOB_FLAG(gob, GOBF_WINDOW))
			Open_Window(gob);
		else
			Update_Window(gob); // Problem! We may not want this all the time.
	}

	// Otherwise, composite and referesh the gob or all gobs:
	return Draw_Window(0, gob);  // 0 = window parent of gob
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

	Main_Event_Window = Open_Window(&Main_Window_GOB);
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
