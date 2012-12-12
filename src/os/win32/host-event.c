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
**  Title: Windowing Event Handler
**  Author: Carl Sassenrath
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

//-- Not currently used:
//#include <windowsx.h>
//#include <mmsystem.h>
//#include <winuser.h>

#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wparam) ((short)HIWORD (wparam))
#endif

#include "reb-host.h"
#include "host-lib.h"

//***** Constants *****

// Virtual key conversion table, sorted by first column.
const REBCNT Key_To_Event[] = {
		VK_PRIOR,	EVK_PAGE_UP,
		VK_NEXT,	EVK_PAGE_DOWN,
		VK_END,		EVK_END,
		VK_HOME,	EVK_HOME,
		VK_LEFT,	EVK_LEFT,
		VK_UP,		EVK_UP,
		VK_RIGHT,	EVK_RIGHT,
		VK_DOWN,	EVK_DOWN,
		VK_INSERT,	EVK_INSERT,
		VK_DELETE,	EVK_DELETE,
		VK_F1,		EVK_F1,
		VK_F2,		EVK_F2,
		VK_F3,		EVK_F3,
		VK_F4,		EVK_F4,
		VK_F5,		EVK_F5,
		VK_F6,		EVK_F6,
		VK_F7,		EVK_F7,
		VK_F8,		EVK_F8,
		VK_F9,		EVK_F9,
		VK_F10,		EVK_F10,
		VK_F11,		EVK_F11,
		VK_F12,		EVK_F12,
		0x7fffffff,	0
};

//***** Externs *****

extern HCURSOR Cursor;
extern void Done_Device(int handle, int error);
extern void Paint_Window(HWND window);
extern void Close_Window(REBGOB *gob);
extern REBOOL Resize_Window(REBGOB *gob, REBOOL redraw);


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

static void Add_File_Events(REBGOB *gob, REBINT flags, HDROP drop)
{
	REBEVT evt;
	REBINT num;
	REBINT len;
	REBINT i;
	REBCHR* buf;
	POINT xy;

	//Get the mouse position
	DragQueryPoint(drop, &xy);

	evt.type  = EVT_DROP_FILE;
	evt.flags = (u8) (flags | (1<<EVF_HAS_XY));
	evt.model = EVM_GUI;
	evt.data = xy.x | xy.y<<16;


	num = DragQueryFile(drop, -1, NULL, 0);

	for (i = 0; i < num; i++){
		len = DragQueryFile(drop, i, NULL, 0);
		buf = OS_Make(len+1);
		DragQueryFile(drop, i, buf, len+1);
		//Reb_Print("DROP: %s", buf);
		buf[len] = 0;
		// ?! convert to REBOL format? E.g.: evt.ser = OS_To_REBOL_File(buf, &len);
		OS_Free(buf);
		if (!RL_Event(&evt)) break;	// queue is full
	}
}

static Check_Modifiers(REBINT flags)
{
	if (GetKeyState(VK_SHIFT) < 0) flags |= (1<<EVF_SHIFT);
	if (GetKeyState(VK_CONTROL) < 0) flags |= (1<<EVF_CONTROL);
	return flags;
}


/***********************************************************************
**
*/	LRESULT CALLBACK REBOL_Window_Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM xy)
/*
**		A Window_Proc() message handler. Simply translate Windows
**		messages into a generic form that REBOL processes.
**
***********************************************************************/
{
	REBGOB *gob;
	REBCNT flags = 0;
	REBCNT i;
	REBCNT mw_num_lines;  // mouse wheel lines setting
	// In order to trace resizing, we need these state variables. It is
	// assumed that we are not re-entrant during this operation. In Win32
	// resizing is a modal loop and prevents it being a problem.
	static LPARAM last_xy = 0;
	static REBINT mode = 0;

	gob = (REBGOB *)GetWindowLong(hwnd, GWL_USERDATA);

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
			Paint_Window(hwnd);
			break;

		case WM_MOUSEMOVE:
			Add_Event_XY(gob, EVT_MOVE, xy, flags);
			//if (!(WIN_FLAGS(wp) & WINDOW_TRACK_LEAVE))
			//	Track_Mouse_Leave(wp);
			break;

		case WM_SIZE:
//			RL_Print("SIZE %d\n", mode);
			if (wParam == SIZE_MINIMIZED) {
				//Invalidate the size but not win buffer
				gob->old_size.x = 0;
				gob->old_size.y = 0;
				Add_Event_XY(gob, EVT_MINIMIZE, xy, flags);
			} else {
				gob->size.x = (i16)LOWORD(xy);
				gob->size.y = (i16)HIWORD(xy);
				last_xy = xy;
				if (mode) {
					//Resize and redraw the window buffer (when resize dragging)
					Resize_Window(gob, TRUE);
					mode = EVT_RESIZE;
					break;
				} else {
					//Resize only the window buffer (when win gob size changed by REBOL code or using min/max buttons)
					if (!Resize_Window(gob, FALSE)){
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
			break;

		case WM_MOVE:
			// Minimize and maximize call this w/o mode set.
			gob->offset.x = (i16)LOWORD(xy);
			gob->offset.y = (i16)HIWORD(xy);
			last_xy = xy;
			if (mode) mode = EVT_OFFSET;
			else Add_Event_XY(gob, EVT_OFFSET, xy, flags);
			break;

		case WM_ENTERSIZEMOVE:
			mode = -1; // possible to ENTER and EXIT w/o SIZE change
			break;

		case WM_EXITSIZEMOVE:
			if (mode > 0) Add_Event_XY(gob, mode, last_xy, flags);
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
			//Add_Event_XY(gob, EVT_TIME, xy, flags);
			break;

		case WM_SETCURSOR:
			if (LOWORD(xy) == 1) {
				SetCursor(Cursor);
				return TRUE;
			} else goto default_case;

		case WM_LBUTTONDBLCLK:
			SET_FLAG(flags, EVF_DOUBLE);
		case WM_LBUTTONDOWN:
			//if (!WIN_CAPTURED(wp)) {
			flags = Check_Modifiers(flags);
			Add_Event_XY(gob, EVT_DOWN, xy, flags);
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
				Add_Event_Key(gob, (msg==WM_KEYDOWN) ? EVT_KEY : EVT_KEY_UP, Key_To_Event[i+1] << 16, flags);
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

		case WM_DROPFILES:
			Add_File_Events(gob, flags, (HDROP)wParam);
			break;

		case WM_CLOSE:
			Add_Event_XY(gob, EVT_CLOSE, xy, flags);
			Close_Window(gob);	// Needs to be removed - should be done by REBOL event handling
//			DestroyWindow(hwnd);// This is done in Close_Window()
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
		default_case:
			return DefWindowProc(hwnd, msg, wParam, xy);
	}
	return 0;
}
