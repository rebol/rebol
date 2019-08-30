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
**  Title: Windowing stubs
**  File:  host-window.c
**  Purpose: Provides stub functions for windowing.
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

/* WARNING:
**     The function declarations here cannot be modified without
**     also modifying those found in the other OS host-lib files!
**     Do not even modify the argument names.
*/


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

#include "reb-host.h"
#include "host-lib.h"
//#include "rebol-lib.h"
#include <gtk/gtk.h>

#define INCLUDE_EXT_DATA
#include "host-ext-window.h"

//***** Constants *****//

#define MAX_WINDOWS 64
#define REBOL_GTK_APP_ID "org.rebol.view"
#define GOB_HWIN(gob)	(OS_Find_Window(gob))
#define GOB_COMPOSITOR(gob)	(OS_Find_Compositor(gob)) //gets handle to window's compositor

//***** Externs *****//

extern REBINT exit_loop;
extern void on_window_removed(GApplication *app, gpointer gob);
extern void on_clicked(GtkWidget *widget, gpointer gob);

//extern HINSTANCE App_Instance;		// Set by winmain function
extern void Host_Crash(char *reason);
//extern LRESULT CALLBACK REBOL_Window_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
//extern LRESULT CALLBACK REBOL_OpenGL_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//***** Locals *****//
static GtkApplication *App = NULL;
static REBOOL Registered = FALSE;		// Window has been registered
static struct gob_window *Gob_Windows;
static u32* window_ext_words;
static REBOOL Custom_Cursor = FALSE;

RL_LIB *RL; // Link back to reb-lib from embedded extensions

//***** Globals *****//

GMainContext *GTKCtx = NULL;
REBGOB *Gob_Root;				// Top level GOB (the screen)
void* Cursor = NULL;			// active cursor image object
REBXYF Zero_Pair = {0, 0};
REBXYF log_size = { 1.0, 1.0 };	 // logical pixel size measured in physical pixels (can be changed using GUI-METRIC command)
REBXYF phys_size = { 1.0, 1.0 }; // physical pixel size measured in logical pixels(reciprocal value of log_size)
REBINT window_scale;



//**********************************************************************
//** OSAL Library Functions ********************************************
//**********************************************************************

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
			//OS_Destroy_Compositor(Gob_Windows[n].compositor);
			CLEAR(&Gob_Windows[n], sizeof(Gob_Windows[n]));
			return;
		}
	}
}

static void
activate (GApplication *app, gpointer user_data)
{
	puts("GTK activate");
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
	REBINT windex;
	int x, y, w, h;

	if (!Registered) {
		//puts("gtk_application_new....");
		//App = gtk_application_new(REBOL_GTK_APP_ID, G_APPLICATION_NON_UNIQUE);
		//g_signal_connect (App, "activate", G_CALLBACK (activate), NULL);
		//gtk_init(NULL, NULL);
		return NULL;
	}

	if (App == NULL) {
		puts("Failed to init GTK!");
		return NULL;
	}
	windex = Alloc_Window(gob);
	if (windex < 0) Host_Crash("Too many windows");
	printf("OS_Open_Window.. %i\n", windex);

	CLEAR_GOB_STATE(gob);
	x = GOB_X_INT(gob);
	y = GOB_Y_INT(gob);
	w = GOB_W_INT(gob);
	h = GOB_H_INT(gob);
	SET_GOB_STATE(gob, GOBS_NEW);

	printf("pos: %ix%i size: %ix%i\n", x, y, w, h);

	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *button_box;
	GtkWidget *area;
	GtkTextBuffer *buffer;
	GtkWidget *_widget;

	printf("GTK VERSION: %i.%i.%i\n", gtk_get_major_version(), gtk_get_minor_version(), gtk_get_micro_version());

	window = gtk_application_window_new(App);
	printf("window: %p\n", window);
	gtk_window_set_title( GTK_WINDOW(window), "Window");
	gtk_window_set_default_size( GTK_WINDOW(window), w, h);

	button_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_container_add (GTK_CONTAINER (window), button_box);
	button = gtk_button_new_with_label ("Hello World");
	g_signal_connect (button, "clicked", G_CALLBACK (on_clicked), gob);
	gtk_container_add (GTK_CONTAINER (button_box), button);

	//----- area -----
	area = gtk_text_view_new();
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(area));
	_widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_size_request(_widget, 200, 400);
	gtk_container_add(_widget, area);
	gtk_container_add (GTK_CONTAINER (button_box), _widget);


	puts("set...");
	Gob_Windows[windex].win = window;

	//if (!GET_GOB_FLAG(gob, GOBF_HIDDEN)) {
		gtk_widget_show_all(window);
	//}

	exit_loop++;

	return window;




#ifdef unused


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
		RL_Get_String(GOB_CONTENT(gob), 0, (void**)&title, TRUE);
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
		Class_Name_Window,
		title,
		options,
		x, y, w, h,
		parent,
		NULL, App_Instance, NULL
	);
	if (!window) {
		Host_Crash("CreateWindow failed");
	}

	Gob_Windows[windex].win = window;
	Gob_Windows[windex].compositor = OS_Create_Compositor(Gob_Root, gob);

	if (!Default_Font) {
		LOGFONTW font;
		HTHEME *hTheme = NULL;
		HRESULT res = -1;
		if (IsThemeActive()) {
			hTheme = OpenThemeData(window, L"Window");
			if (hTheme) {
				res = GetThemeSysFont(hTheme, TMT_MSGBOXFONT, &font);
			}
		} else {
			NONCLIENTMETRICS metrics;
			ZeroMemory(&metrics, sizeof(metrics));
			metrics.cbSize = sizeof(metrics);
			res = SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);
			if (res) font = metrics.lfMessageFont;
		}
		if ( res >= 0 ) {
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
//#ifdef __LLP64__
//	SetWindowLongPtr(window, GWLP_USERDATA, (REBUPT)gob);
//#else
//	SetWindowLong(window, GWL_USERDATA, (REBUPT)gob);
//#endif

	if (!GET_GOB_FLAG(gob, GOBF_HIDDEN)) {
		if (GET_GOB_FLAG(gob, GOBF_ON_TOP)) SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		ShowWindow(window, SW_SHOWNORMAL);
		OS_Draw_Window(0, gob);              // draw not native widgets
		SendMessage(window, WM_PAINT, 0, 0); // draw native widgets
		SetForegroundWindow(window);
	}
	return window;
#endif
	return NULL;
}


/***********************************************************************
**
*/  void OS_Close_Window(REBGOB *gob)
/*
**		Close the window.
**
***********************************************************************/
{
	printf("OS_Close_Window: %p\n", gob);
	void* parent = NULL;
	if (GET_GOB_FLAG(gob, GOBF_WINDOW) && OS_Find_Window(gob)) {
		if (GET_GOB_FLAG(gob, GOBF_MODAL)) {
#ifdef unused
			parent = GetParent(GOB_HWIN(gob));
			if (parent) {
				//EnableWindow(parent, TRUE);
				//EnumWindows(EnumWindowsProc, (LPARAM)parent);
			}
#endif
		}
		//DestroyWindow(GOB_HWIN(gob)); //TODO: gtk_widget_destroy???
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
	void *compositor;
	REBOOL changed;
#ifdef unused
	compositor = GOB_COMPOSITOR(gob);
	changed = OS_Resize_Window_Buffer(compositor, gob);
	if (redraw) {
		OS_Compose_Gob(compositor, gob, gob, FALSE); // what if not actually resized?
		OS_Blit_Window(compositor);
	}
#endif
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

}

/***********************************************************************
**
*/	void OS_Draw_Window(REBGOB *wingob, REBGOB *gob)
/*
**		Refresh the GOB within the given window. If the wingob
**		is zero, then find the correct window for it.
**
***********************************************************************/
{

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
					OS_Draw_Window(0, *gp);
				}
			}
		}
		return 0;
	}
//	// Is it a native widget?
//	else if (GOBT_WIDGET == GOB_TYPE(gob)) {
//		RedrawWindow((HWND)VAL_HANDLE(GOB_WIDGET_HANDLE(gob)), NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
//	}
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
	OS_Draw_Window(0, gob);  // 0 = window parent of gob
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
#ifdef unused
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
		case W_WINDOW_BUTTON:
			class = (REBCHR*)Class_Name_Button;
			style |= WS_TABSTOP | BS_PUSHBUTTON; // | BS_DEFPUSHBUTTON;
			break;
		case W_WINDOW_CHECK:
			class = (REBCHR*)Class_Name_Button;
			style |= WS_TABSTOP | BS_AUTOCHECKBOX;
			break;
		case W_WINDOW_RADIO:
			class = (REBCHR*)Class_Name_Button;
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
#endif
	return NULL;
}


/***********************************************************************
**
*/	REBOOL OS_Get_Widget_Data(REBGOB *gob, REBVAL *ret)
/*
**	Returns data according the widget type
**
***********************************************************************/
{
#ifdef unused
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
#endif
	return FALSE;
}


/***********************************************************************
**
*/	REBOOL OS_Set_Widget_Data(REBGOB *gob, REBVAL *data)
/*
**	Returns data according the widget type
**
***********************************************************************/
{
#ifdef unused
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
#endif
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
	case SM_SCREEN_WIDTH:
		result = gdk_screen_width();
		break;
	case SM_SCREEN_HEIGHT:
		result = gdk_screen_height();
		break;
#ifdef unused
	case SM_VIRTUAL_SCREEN_WIDTH:
		result = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		break;
	case SM_VIRTUAL_SCREEN_HEIGHT:
		result = GetSystemMetrics(SM_CYVIRTUALSCREEN);
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
#endif
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
	//SetCursor((HCURSOR)cursor);
}

/***********************************************************************
**
*/	void* OS_Load_Cursor(void *cursor)
/*
**
**
***********************************************************************/
{
	//return (void*)LoadCursor(NULL, (LPCTSTR)cursor);
}

/***********************************************************************
**
*/	void OS_Destroy_Cursor(void *cursor)
/*
**
**
***********************************************************************/
{
	//DestroyCursor((HCURSOR)Cursor);
}


/***********************************************************************
**
*/	void* OS_Image_To_Cursor(REBYTE* image, REBINT width, REBINT height)
/*
**      Converts REBOL image! to Windows CURSOR
**
***********************************************************************/
{
#ifdef unused
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
#endif
	return NULL;
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
//#if 0
//			if (RXA_TYPE(frm, 3) == RXT_PAIR) {
//				log_size.x = RXA_PAIR(frm, 3).x;
//				log_size.y = RXA_PAIR(frm, 3).y;
//				phys_size.x = 1 / log_size.x;
//				phys_size.y = 1 / log_size.y;
//			}
//#endif
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
		case W_WINDOW_SCREEN_NUM:
			x = OS_Get_Metrics(SM_SCREEN_NUM, 0);
			RXA_INT64(frm, 1) = x;
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
	//return RXR_UNSET;
}


/***********************************************************************
**
*/	void Init_Windows(void)
/*
**	Initialize special variables of the graphics subsystem.
**
***********************************************************************/
{
	//puts("Init_Windows");
	RL = RL_Extend((REBYTE *)(&RX_window[0]), &RXD_Window);

	Gob_Windows = OS_Make(sizeof(struct gob_window) * (MAX_WINDOWS + 1));
	CLEAR(Gob_Windows, sizeof(struct gob_window) * (MAX_WINDOWS + 1));

	//puts("gtk_application_new....");
	App = gtk_application_new(REBOL_GTK_APP_ID, G_APPLICATION_NON_UNIQUE);
	//g_signal_connect (App, "activate", G_CALLBACK(activate), NULL);
	g_signal_connect(App,"window-removed", G_CALLBACK(on_window_removed), exit_loop);

	GTKCtx = g_main_context_default();
	g_application_register(App, NULL, NULL);
	//gtk_init(NULL, NULL);
	Registered = TRUE;

//	Cursor = LoadCursor(NULL, IDC_ARROW);
}

