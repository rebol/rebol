/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2023 Rebol Open Source Developers
**  REBOL is a trademark of REBOL Technologies
**
**  Additional code modifications and improvements Copyright 2012 Saphirion AG
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
**  Title: Windowing Event Handler
**  Author: Carl Sassenrath, Richard Smolak
**  Purpose: This code handles windowing related events.
**	Related: host-window.c, dev-event.c
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

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0501        // this is needed to be able use SPI_GETWHEELSCROLLLINES etc.
#endif

#include <windows.h>
#include <commctrl.h>	// For WM_MOUSELEAVE event
#include <zmouse.h>
#include <math.h>	//for floor()

//OpenGL
#pragma comment(lib, "opengl32")
#include <gl/gl.h>

//-- Not currently used:
//#include <windowsx.h>
//#include <mmsystem.h>
//#include <winuser.h>

#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wparam) ((short)HIWORD (wparam))
#endif

#include "reb-host.h"
#include "host-lib.h"

extern HINSTANCE App_Instance;		// Set by winmain function
extern HWND      Focused_Window;

#define IS_LAYERED(hwnd) ((WS_EX_LAYERED & GetWindowLongPtr(hwnd, GWL_EXSTYLE)) > 0)
#define GOB_FROM_HWND(hwnd) (REBGOB *)GetWindowLongPtr(hwnd, GWLP_USERDATA)



//***** Externs *****

extern HCURSOR Cursor;
extern void Done_Device(int handle, int error);
extern void Paint_Window(HWND window);
extern const WORD Key_To_Event[]; // in dev-stdio.c



/***********************************************************************
**
**	Local Functions
**
***********************************************************************/

static void Add_Event_XY(REBGOB *gob, REBINT id, REBINT xy, REBINT flags)
{
	REBEVT evt;

	evt.type  = id;
	evt.flags = (u8) (flags | (1<<EVF_HAS_XY));
	evt.model = EVM_GUI;
	evt.data  = xy;
	evt.ser = (void*)gob;

	RL_Event(&evt);	// returns 0 if queue is full
}

static void Add_Event_Key(REBGOB *gob, REBINT id, REBINT key, REBINT flags)
{
	REBEVT evt;

	evt.type  = id;
	evt.flags = flags;
	evt.model = EVM_GUI;
	evt.data  = key;
	evt.ser = (void*)gob;

	RL_Event(&evt);	// returns 0 if queue is full
}

static void Add_Event_Widget(HWND widget, REBINT id, REBINT flags)
{
	REBEVT evt;
	REBGOB *gob;
	
	gob = GOB_FROM_HWND(widget);

	// fields are throwing CHANGE event during its creation
	// when there is no GWLP_USERDATA yet available!
	if (!gob) return; // ... so ignore it in such a case.

	evt.type = id;
	evt.flags = 0; //(u8)(flags | (1 << EVF_HAS_XY));
	evt.model = EVM_GUI;
	evt.data = 0;
	evt.ser = (void*)gob;

	RL_Event(&evt);	// returns 0 if queue is full
}

static void Add_File_Events(REBGOB *gob, REBINT flags, HDROP drop)
{
	REBEVT evt;
	REBINT num;
	REBINT len;
	REBINT i;
	REBCHR buf[MAX_PATH] = {0};
	POINT xy;
	
	//Get the mouse position
	DragQueryPoint(drop, &xy);

	evt.type  = EVT_DROP_FILE;
	evt.flags = (u8) (flags | (1<<EVF_HAS_XY) | (1<<EVF_COPIED) | (1<<EVF_HAS_DATA));
	evt.model = EVM_GUI;
	// reporting XY as an absolute position, so the target gob can be located using map-event from Gob_Root
	evt.data = (xy.x + GOB_X_INT(gob)) | (xy.y + GOB_Y_INT(gob))<<16;

	num = DragQueryFile(drop, -1, NULL, 0);

	for (i = 0; i < num; i++){
		// NOTE: originaly the buffer was made using OS_Make and freed when not needed
		// but for some reason it was causing a crash! Use GlobalAlloc/GlobalLock?
		len = DragQueryFile(drop, i, buf, MAX_PATH-1);
		// printf("DROP: %d %ls\n", len, buf);
		evt.ser = RL_Encode_UTF8_String(buf, len, TRUE, 0);
		if (!RL_Event(&evt)) break;	// queue is full
	}
	DragFinish(drop);
}

static REBINT Check_Modifiers(REBINT flags)
{
	if (GetKeyState(VK_SHIFT) < 0) flags |= (1<<EVF_SHIFT);
	if (GetKeyState(VK_CONTROL) < 0) flags |= (1<<EVF_CONTROL);
	return flags;
}

static int modal_timer_id;
static void onModalBlock(
  HWND Arg1,
  UINT Arg2,
  UINT_PTR Arg3,
  DWORD Arg4
) {
	//puts("o");
	//Add_Event_XY(gob, EVT_RESIZE, xy, 0);
}

/***********************************************************************
**
*/	LRESULT CALLBACK REBOL_Base_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
/*
**		A Base_Proc() message handler. Simply translate Windows
**		messages into a generic form that REBOL processes.
**
***********************************************************************/
{
	LPARAM xy = lParam;
	REBGOB *gob;

	// Handle message:
	switch(msg)
	{
	case WM_MOUSEACTIVATE:
		if (IS_LAYERED(hwnd)) {
			SetForegroundWindow(GetParent(hwnd));
			return 3; // do not make it activated when click it
		}
		break;
	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		return 0;
	case WM_LBUTTONUP:
		ReleaseCapture();
		return 0;
	case WM_ERASEBKGND:
		return TRUE; // drawing in WM_PAINT to avoid flickering
	case WM_SIZE:
		gob = GOB_FROM_HWND(hwnd);
		//TODO: finish me!
		break;
	}
	return 0;
}

/***********************************************************************
**
*/	LRESULT CALLBACK REBOL_Window_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
/*
**		A Window_Proc() message handler. Simply translate Windows
**		messages into a generic form that REBOL processes.
**
***********************************************************************/
{
	LPARAM xy = lParam;
	REBGOB *gob;
	REBCNT flags = 0;
	REBCNT i;
	REBCNT mw_num_lines;  // mouse wheel lines setting
	// In order to trace resizing, we need these state variables. It is
	// assumed that we are not re-entrant during this operation. In Win32
	// resizing is a modal loop and prevents it being a problem.
	static LPARAM last_xy = 0;
	static REBINT mode = 0;
	//SCROLLINFO si;

	gob = GOB_FROM_HWND(hwnd);

	//printf("REBOL_Window_Proc - msg: %0X wParam: %0X lParam: %0X gob: %0Xh\n", msg, wParam, lParam, gob);

	// Not a REBOL window (or early creation):
	if (!gob || !IS_WINDOW(gob)) {
		switch(msg) {
			case WM_PAINT:
				Paint_Window(hwnd);
				break;
			case WM_CLOSE:
				DestroyWindow(hwnd);
				break;
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			//case WM_GETMINMAXINFO: printf("WM_GETMINMAXINFO\n");
			default:
				// Default processing that we do not care about:
				return DefWindowProc(hwnd, msg, wParam, xy);
		}
		return 0;
	}

	// Handle message:
	switch(msg)
	{
		case WM_PAINT:
		case WM_DISPLAYCHANGE:
			Paint_Window(hwnd);
			return FALSE;

		case WM_MOUSEMOVE:
			Add_Event_XY(gob, EVT_MOVE, xy, flags);
			//if (!(WIN_FLAGS(wp) & WINDOW_TRACK_LEAVE))
			//	Track_Mouse_Leave(wp);
			return FALSE;

		case WM_ERASEBKGND:
			//if (!IS_WINDOW(gob))
				return TRUE;
			break;

		case WM_SIZE:
		
		//case WM_SIZING:
			RL_Print("SIZE %d\n", mode);
			if (wParam == SIZE_MINIMIZED) {
				//Invalidate the size but not win buffer
				gob->old_size.x = 0;
				gob->old_size.y = 0;
				Add_Event_XY(gob, EVT_MINIMIZE, xy, flags);
			} else {
				gob->size.x = (i16)LOWORD(xy);
				gob->size.y = (i16)HIWORD(xy);
				last_xy = xy;
				OS_Resize_Window(gob, TRUE);
				if (mode) {
					//Resize and redraw the window buffer (when resize dragging)
					mode = EVT_RESIZE;
					break;
				} else {
					//Resize only the window buffer (when win gob size changed by REBOL code or using min/max buttons)
					if (!OS_Resize_Window(gob, (wParam == SIZE_RESTORED))){
						//size has been changed programatically - return only 'resize event
						Add_Event_XY(gob, EVT_RESIZE, xy, flags);
						break;
					}
				}
				//Otherwise send combo of 'resize + maximize/restore events
				if (wParam == SIZE_MAXIMIZED) i = EVT_MAXIMIZE;
				else if (wParam == SIZE_RESTORED) i = EVT_RESTORE;
				else i = 0;
				Add_Event_XY(gob, EVT_RESIZE, xy, flags);
				if (i) Add_Event_XY(gob, i, xy, flags);
			}
			return FALSE;

		case WM_MOVE:
			// Minimize and maximize call this w/o mode set.
			gob->offset.x = (i16)LOWORD(xy);
			gob->offset.y = (i16)HIWORD(xy);
			last_xy = xy;
			if (mode) mode = EVT_OFFSET;
			else Add_Event_XY(gob, EVT_OFFSET, xy, flags);
			return FALSE;

		case WM_HSCROLL:
		case WM_VSCROLL:
			if (lParam == 0) {
				// standard scroll bar
				Add_Event_Widget((HWND)wParam, EVT_SCROLL, flags);
			} else {
				Add_Event_Widget((HWND)lParam, EVT_CHANGE, flags);
			}
			return 0;

		case WM_ENTERSIZEMOVE:
			mode = -1; // possible to ENTER and EXIT w/o SIZE change
			//modal_timer_id = SetTimer(hwnd, 0, USER_TIMER_MINIMUM , (TIMERPROC)NULL);
			break;

		case WM_EXITSIZEMOVE:
			if (mode > 0) 
				Add_Event_XY(gob, mode, last_xy, flags);
			//else
			//	KillTimer(hwnd, modal_timer_id);
			mode = 0;
			
			break;

		case WM_MOUSELEAVE:
			// Get cursor position, not the one given in message:
			//GetCursorPos(&x_y);
			//ScreenToClient(hwnd, &x_y);
			//xy = (x_y.y << 16) + (x_y.x & 0xffff);
			Add_Event_XY(gob, EVT_MOVE, xy, flags);
			// WIN_FLAGS(wp) &= ~WINDOW_TRACK_LEAVE;
			break;

		case WM_MOUSEWHEEL:
			SystemParametersInfo(SPI_GETWHEELSCROLLLINES,0, &mw_num_lines, 0);
			if (LOWORD(wParam) == MK_CONTROL || mw_num_lines > WHEEL_DELTA) {
				Add_Event_XY(gob, EVT_SCROLL_PAGE, (GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA) << 16, flags);
			} else {
				Add_Event_XY(gob, EVT_SCROLL_LINE, ((GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA) << 16) * mw_num_lines, flags);
			}
			break;

		case WM_TIMER:
			//printf("t");
			Add_Event_XY(gob, EVT_TIME, xy, flags);
			break;

		case WM_SETCURSOR:
			if (LOWORD(xy) == 1) {
				SetCursor(Cursor);
				return TRUE;
			}
			break;

		case WM_LBUTTONDBLCLK:
			SET_FLAG(flags, EVF_DOUBLE);
		case WM_LBUTTONDOWN:
			//if (!WIN_CAPTURED(wp)) {
			flags = Check_Modifiers(flags);
			Add_Event_XY(gob, EVT_DOWN, xy, flags);
			SetFocus(hwnd);
			SetCapture(hwnd);
			//WIN_CAPTURED(wp) = EVT_BTN1_UP;
			break;

		case WM_LBUTTONUP:
			//if (WIN_CAPTURED(wp) == EVT_BTN1_UP) {
			flags = Check_Modifiers(flags);
			Add_Event_XY(gob, EVT_UP, xy, flags);
			ReleaseCapture();
			//WIN_CAPTURED(wp) = 0;
			break;

		case WM_RBUTTONDBLCLK:
			SET_FLAG(flags, EVF_DOUBLE);
		case WM_RBUTTONDOWN:
			//if (!WIN_CAPTURED(wp)) {
			flags = Check_Modifiers(flags);
			Add_Event_XY(gob, EVT_ALT_DOWN, xy, flags);
			SetFocus(hwnd);
			SetCapture(hwnd);
			//WIN_CAPTURED(wp) = EVT_BTN2_UP;
			break;

		case WM_RBUTTONUP:
			//if (WIN_CAPTURED(wp) == EVT_BTN2_UP) {
			flags = Check_Modifiers(flags);
			Add_Event_XY(gob, EVT_ALT_UP, xy, flags);
			ReleaseCapture();
			//WIN_CAPTURED(wp) = 0;
			break;

		case WM_MBUTTONDBLCLK:
			SET_FLAG(flags, EVF_DOUBLE);
		case WM_MBUTTONDOWN:
			//if (!WIN_CAPTURED(wp)) {
			flags = Check_Modifiers(flags);
			Add_Event_XY(gob, EVT_AUX_DOWN, xy, flags);
			SetFocus(hwnd);
			SetCapture(hwnd);
			break;

		case WM_MBUTTONUP:
			//if (WIN_CAPTURED(wp) == EVT_BTN2_UP) {
			flags = Check_Modifiers(flags);
			Add_Event_XY(gob, EVT_AUX_UP, xy, flags);
			ReleaseCapture();
			break;

		case WM_KEYDOWN:
			// Note: key repeat may cause multiple downs before an up.
		case WM_KEYUP:
			flags = Check_Modifiers(flags);
			for (i = 0; Key_To_Event[i] && wParam > Key_To_Event[i]; i += 2);
			if (wParam == Key_To_Event[i])
				Add_Event_Key(gob, (msg==WM_KEYDOWN) ? EVT_CONTROL : EVT_CONTROL_UP, Key_To_Event[i+1], flags);
			break;

		case WM_CHAR:
			flags = Check_Modifiers(flags);
#ifdef OS_WIDE_CHAR
			i = wParam;
#else
			i = wParam & 0xff;
#endif
			//if (i == 127) i = 8; // Windows weirdness of converting ctrl-backspace to delete
			Add_Event_Key(gob, EVT_KEY, i, flags);
			break;

		case WM_COMMAND:
			//printf("\nWM_COMMAND wParam: %04X %04X %u  lParam: %08X \n", LOWORD(wParam), HIWORD(wParam), HIWORD(wParam),  lParam);
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				Add_Event_Widget((HWND)lParam, EVT_CHANGE, flags);
				break;
			//case EN_UPDATE:
			//		GetScrollInfo((HWND)lParam, SB_HORZ, &si);
			//		//ShowScrollBar((HWND)lParam, SB_HORZ, si.nPos > 0);
			//		//printf("EN h: %d\n", si.nPos);
			//	break;
			case BN_CLICKED:
				Add_Event_Widget((HWND)lParam, EVT_CLICK, flags);
				break;
			case EN_SETFOCUS:
				Add_Event_Widget((HWND)lParam, EVT_FOCUS, flags);
				break;
			case EN_KILLFOCUS:
				Add_Event_Widget((HWND)lParam, EVT_UNFOCUS, flags);
				break;
			}
			break;
		
		case WM_DROPFILES:
			Add_File_Events(gob, flags, (HDROP)wParam);
			return 0;

		case WM_CTLCOLORSTATIC:
			//SetTextColor((HDC)wParam, RGB(0, 0, 0));
			//SetBkMode((HDC)wParam, COLOR_BACKGROUND);
			//return (LRESULT)GetStockObject(COLOR_WINDOW);
			break;

		case WM_CTLCOLORBTN:
			//printf("\nWM_CTLCOLORBTN wParam: %08X lParam: %08X \n", wParam, lParam);
			//SetBkMode((HDC)wParam, COLOR_WINDOW);
			//SetBkColor((HDC)lParam, RGB(0, 0, 0));
			//return (LRESULT)GetStockObject(COLOR_WINDOW);
			break;

		case WM_NOTIFY:
			//printf("\nWM_NOTIFY wParam: %04X %04X %u  lParam: %08X \n", LOWORD(wParam), HIWORD(wParam), HIWORD(wParam), lParam);
			switch (((LPNMHDR)lParam)->code) {
			case DTN_DATETIMECHANGE:
				Add_Event_Widget(((LPNMHDR)lParam)->hwndFrom, EVT_CHANGE, flags);
				return 0;
			}
			break;

		case WM_GETMINMAXINFO:
			//printf("WM_GETMINMAXINFO\n");
			break;

		case WM_SETFOCUS:
			//puts("WM_SETFOCUS");
			Focused_Window = hwnd;
			break;

		case WM_CLOSE:
			Add_Event_XY(gob, EVT_CLOSE, xy, flags);
			OS_Close_Window(gob);	// Needs to be removed - should be done by REBOL event handling
//			DestroyWindow(hwnd);// This is done in OS_Close_Window()
			return 0;



		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_NCDESTROY:
			//TODO: notify native widgets?
			//puts("WM_NCDESTROY");
			break;

	}
	return DefWindowProc(hwnd, msg, wParam, xy);
}


/***********************************************************************
**
*/	LRESULT CALLBACK REBOL_OpenGL_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
/*
**		An OpenGL message handler.
**
***********************************************************************/
{
	REBGOB *gob;
	REBCNT flags = 0;
	HDC hdc;
	HGLRC hglrc;
	PIXELFORMATDESCRIPTOR pfd;
	PAINTSTRUCT ps;
	
	gob = GOB_FROM_HWND(hwnd);

	//if(msg != WM_PAINT)
	//	printf("OpenGL_Proc - msg: %0X wParam: %0X lParam: %0X gob: %0Xh\n", msg, wParam, lParam, gob);
	
	// Handle message:
	switch (msg)
	{
	case WM_PAINT: 
		hdc = BeginPaint(hwnd, &ps);
		//TODO: retrive context from gob's handle
		//hglrc = wglGetCurrentContext();
		//wglMakeCurrent(hdc, hglrc);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBegin(GL_TRIANGLES);
			glColor3ub(255, 0, 0);    glVertex2d(-0.75, -0.75);
			glColor3ub(0, 255, 0);    glVertex2d(0.0, 0.75);
			glColor3ub(0, 0, 255);    glVertex2d(0.75, -0.75);
			glEnd();
			glFlush();

			SwapBuffers(hdc);
		//	wglMakeCurrent(hdc, 0);
			EndPaint(hwnd, &ps);
		return FALSE;
	case WM_ERASEBKGND:
		// for testing purposes, set random background so far...
		glClearColor(((float)rand()/(float)(RAND_MAX))*1.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		break;
	case WM_SIZE:
		glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_CREATE:
		hdc = GetDC(hwnd);
		ZeroMemory(&pfd, sizeof(pfd));
		PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
			PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
			32,                   // Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,                   // Number of bits for the depthbuffer
			8,                    // Number of bits for the stencilbuffer
			0,                    // Number of Aux buffers in the framebuffer.
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};
		int pf = ChoosePixelFormat(hdc, &pfd);
		if (!pf) {
			RL_Print("Could not find a pixel format.. OpenGL cannot create its context.\n");
			return FALSE;
		}
		SetPixelFormat(hdc, pf, &pfd);
		hglrc = wglCreateContext(hdc);
		if (hglrc) {
			wglMakeCurrent(hdc, hglrc);
		}
		else {
			RL_Print("Failed to create OpenGL context!\n");
			return FALSE;
		}
		RL_Print("OPENGL CONTEXT CREATED!\n");
		RL_Print("Version %s\n", glGetString(GL_VERSION));
		return FALSE;

	case WM_DESTROY:
		hglrc = wglGetCurrentContext();
		wglDeleteContext(hglrc);
		return FALSE;
	}

	return REBOL_Window_Proc(hwnd, msg, wParam, lParam);
}
