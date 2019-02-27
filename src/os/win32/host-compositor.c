/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
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
**  Title: Win32 GDI based Compositor abstraction layer API.
**  Author: Richard Smolak
**  File:  host-compositor.c
**  Purpose: Provides simple gob compositor code for Windows OS.
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

#define WINVER 0x0500  //for AlphaBlend()
#define _WIN32_WINNT 0x0500  //for DC_BRUSH
#include <windows.h>

#include <math.h>	//for floor()
#include <stdlib.h> //for rand()
#include "reb-host.h"
#include "host-lib.h"

//***** Externs *****
extern HWND Find_Window(REBGOB *gob);

//***** Macros *****

#define GOB_HWIN(gob)	((HWND)Find_Window(gob))

//***** Locals *****

static REBXYF Zero_Pair = {0, 0};

typedef struct compositor_ctx {
	REBYTE *Window_Buffer;
	REBXYI winBufSize;	
	REBGOB *Win_Gob;
	REBGOB *Root_Gob;
	HDC winDC;
	HBITMAP Back_Buffer;
	HDC backDC;
	BITMAPINFO bmpInfo;
	HRGN Win_Clip;
	HRGN New_Clip;
	HRGN Old_Clip;	
	REBXYF absOffset;
	HBRUSH DCbrush;
} REBCMP_CTX;

/***********************************************************************
**
*/ REBYTE* rebcmp_get_buffer(REBCMP_CTX* ctx)
/*
**	Provide pointer to window compositing buffer.
**  Return NULL if buffer not available of call failed.
**
**  NOTE: The buffer may be "locked" during this call on some platforms.
**        Always call rebcmp_release_buffer() to be sure it is released.
**
***********************************************************************/
{
	return ctx->Window_Buffer;
}

/***********************************************************************
**
*/ void rebcmp_release_buffer(REBCMP_CTX* ctx)
/*
**	Release the window compositing buffer acquired by rebcmp_get_buffer().
**
**  NOTE: this call can be "no-op" on platforms that don't need locking.
**
***********************************************************************/
{
}

/***********************************************************************
**
*/ REBOOL rebcmp_resize_buffer(REBCMP_CTX* ctx, REBGOB* winGob)
/*
**	Resize the window compositing buffer.
**
**  Returns TRUE if buffer size was really changed, otherwise FALSE.
**
***********************************************************************/
{
	//check if window size really changed or buffer needs to be created
	if ((GOB_LOG_W(winGob) != GOB_WO(winGob)) || (GOB_LOG_H(winGob) != GOB_HO(winGob)) || ctx->Back_Buffer == 0) {
		HBITMAP new_buffer;
		HDC newDC;
		REBYTE *new_bytes;
		REBINT w = GOB_LOG_W_INT(winGob);
		REBINT h = GOB_LOG_H_INT(winGob);
		RECT lprc = {0,0,w,h};
		
		///set window size in bitmapinfo struct
		ctx->bmpInfo.bmiHeader.biWidth = w;
		ctx->bmpInfo.bmiHeader.biHeight = -h;

		//create new window backbuffer and DC
		new_buffer = CreateDIBSection(ctx->winDC, &ctx->bmpInfo, DIB_RGB_COLORS,(VOID **)&new_bytes, 0, 0);
		newDC = CreateCompatibleDC(ctx->winDC);

		//update the buffer size values
		ctx->winBufSize.x = w;
		ctx->winBufSize.y = h;
		
		//select new DC with back buffer and delete old DC(to prevent leak)
		DeleteObject((HBITMAP)SelectObject(newDC, new_buffer));

		//fill the background color
		SetDCBrushColor(newDC, RGB(200,200,200));		
		FillRect(newDC,&lprc, ctx->DCbrush);
		
		if (ctx->backDC != 0) {
/*		
			//copy the current content
			BitBlt(
				newDC,
				0, 0,
				w, h,
				ctx->backDC,
				0, 0,
				SRCCOPY
			);
*/			
			//cleanup of previously used objects
			DeleteObject(ctx->Back_Buffer);
			DeleteDC(ctx->backDC);
		}
		
		//make the new buffer actual
		ctx->Back_Buffer = new_buffer;
		ctx->backDC = newDC;
		ctx->Window_Buffer = new_bytes;

		//set window clip region
//		SetRectRgn(ctx->Win_Clip, 0, 0, w, h);
//		SelectClipRgn(ctx->backDC, ctx->Win_Clip);

		//update old gob area
		GOB_XO(winGob) = GOB_LOG_X(winGob);
		GOB_YO(winGob) = GOB_LOG_Y(winGob);
		GOB_WO(winGob) = GOB_LOG_W(winGob);
		GOB_HO(winGob) = GOB_LOG_H(winGob);
		return TRUE;
	}
	return FALSE;
}

/***********************************************************************
**
*/ void* rebcmp_create(REBGOB* rootGob, REBGOB* gob)
/*
**	Create new Compositor instance.
**
***********************************************************************/
{
	//new compositor struct
	REBCMP_CTX *ctx = (REBCMP_CTX*)OS_Make(sizeof(REBCMP_CTX));
	
	//bitmapinfo struct
	CLEARS(&ctx->bmpInfo);
    ctx->bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    ctx->bmpInfo.bmiHeader.biPlanes = 1;
    ctx->bmpInfo.bmiHeader.biBitCount = 32;
    ctx->bmpInfo.bmiHeader.biCompression = BI_RGB;        

	//shortcuts
	ctx->Root_Gob = rootGob;
	ctx->Win_Gob = gob;
	ctx->winDC = GetDC(GOB_HWIN(gob));
	ctx->backDC = 0;
	ctx->Back_Buffer = 0;

	//custom color brush
	ctx->DCbrush = GetStockObject(DC_BRUSH);
	
	//initialize clipping regions
	ctx->Win_Clip = CreateRectRgn(0, 0, GOB_LOG_W_INT(gob), GOB_LOG_H_INT(gob));
	ctx->New_Clip = CreateRectRgn(0, 0, 0, 0);
	ctx->Old_Clip = CreateRectRgn(0, 0, 0, 0);	
	
	//call resize to init rest
	rebcmp_resize_buffer(ctx, gob);

	return ctx;
}

/***********************************************************************
**
*/ void rebcmp_destroy(REBCMP_CTX* ctx)
/*
**	Destroy existing Compositor instance.
**
***********************************************************************/
{
	//do cleanup
	ReleaseDC(GOB_HWIN(ctx->Win_Gob), ctx->winDC );
	DeleteDC(ctx->backDC);
	DeleteObject(ctx->Back_Buffer);
	DeleteObject(ctx->Win_Clip);
	DeleteObject(ctx->New_Clip);
	DeleteObject(ctx->Old_Clip);
	OS_Free(ctx);
}

/***********************************************************************
**
*/ static void process_gobs(REBCMP_CTX* ctx, REBGOB* gob)
/*
**	Recursively process and compose gob and its children.
**
** NOTE: this function is used internally by rebcmp_compose() call only.
**
***********************************************************************/
{
	REBINT x = ROUND_TO_INT(ctx->absOffset.x);
	REBINT y = ROUND_TO_INT(ctx->absOffset.y);
	REBINT intersection_result;
	RECT gob_clip;

	if (GET_GOB_STATE(gob, GOBS_NEW)){
		//reset old-offset and old-size if newly added
		GOB_XO(gob) = GOB_LOG_X(gob);
		GOB_YO(gob) = GOB_LOG_Y(gob);
		GOB_WO(gob) = GOB_LOG_W(gob);
		GOB_HO(gob) = GOB_LOG_H(gob);

		CLR_GOB_STATE(gob, GOBS_NEW);
	}

//	RL_Print("oft: %dx%d siz: %dx%d abs_oft: %dx%d \n", GOB_X_INT(gob), GOB_Y_INT(gob), GOB_W_INT(gob), GOB_H_INT(gob), x, y);

	//intersect gob dimensions with actual window clip region
	SetRectRgn(ctx->Win_Clip, x, y, x + GOB_LOG_W_INT(gob), y + GOB_LOG_H_INT(gob));
	intersection_result = ExtSelectClipRgn(ctx->backDC, ctx->Win_Clip, RGN_AND);


	GetClipBox(ctx->backDC, &gob_clip);
//	RL_Print("clip: %dx%d %dx%d\n", gob_clip.left, gob_clip.top, gob_clip.right, gob_clip.bottom);
	
	if (intersection_result != NULLREGION)
	{
//		RL_Print("clip OK %d %d\n", r, GOB_TYPE(gob));
		
//		if (!GET_GOB_FLAG(gob, GOBF_WINDOW))
		//render GOB content
		REBXYI offset = {x,y};
		REBXYI top_left = {gob_clip.left, gob_clip.top};
		REBXYI bottom_right = {gob_clip.right, gob_clip.bottom};
		switch (GOB_TYPE(gob)) {
			case GOBT_COLOR:
//					RL_Print("draw color gob %dx%d\n", x, y);
				rebdrw_gob_color(gob, ctx->Window_Buffer, ctx->winBufSize, offset, top_left, bottom_right);
				break;
			
			case GOBT_IMAGE:
//				RL_Print("draw image gob\n");
				rebdrw_gob_image(gob, ctx->Window_Buffer, ctx->winBufSize, offset, top_left, bottom_right);
				break;

			case GOBT_DRAW:
				rebdrw_gob_draw(gob, ctx->Window_Buffer ,ctx->winBufSize,  offset, top_left, bottom_right);
				break;

			case GOBT_TEXT:
			case GOBT_STRING:
				rt_gob_text(gob, ctx->Window_Buffer ,ctx->winBufSize,ctx->absOffset, top_left, bottom_right);
				break;
				
			case GOBT_EFFECT:
				break;
		}
/*
		//draw clip region frame (for debugging)
		GetClipRgn(ctx->backDC, ctx->Win_Clip); //copy the actual region to Win_Clip
		SetDCBrushColor(ctx->backDC, RGB(rand() % 256, rand() % 256, rand() % 256));
		FrameRgn(ctx->backDC, ctx->Win_Clip, ctx->DCbrush, 2, 2);	
*/

		//recursively process sub GOBs
		if (GOB_PANE(gob)) {
			REBINT n;
//			RECT parent_clip;		
			REBINT len = GOB_TAIL(gob);
			REBGOB **gp = GOB_HEAD(gob);
			
			//store clip region coords
//			GetClipBox(ctx->backDC, &parent_clip);

			for (n = 0; n < len; n++, gp++) {
				REBINT g_x = GOB_LOG_X(*gp);
				REBINT g_y = GOB_LOG_Y(*gp);

				//restore the parent clip region
//				SetRectRgn(ctx->Win_Clip, parent_clip.left, parent_clip.top, parent_clip.right, parent_clip.bottom);
				SetRectRgn(ctx->Win_Clip, gob_clip.left, gob_clip.top, gob_clip.right, gob_clip.bottom);
				SelectClipRgn(ctx->backDC, ctx->Win_Clip);
				
				ctx->absOffset.x += g_x;
				ctx->absOffset.y += g_y;
				
				process_gobs(ctx, *gp);

				ctx->absOffset.x -= g_x;
				ctx->absOffset.y -= g_y;
			}
		}
	} //else {RL_Print("invisible!\n");}
}

/***********************************************************************
**
*/ void rebcmp_compose(REBCMP_CTX* ctx, REBGOB* winGob, REBGOB* gob, REBOOL only)
/*
**	Compose content of the specified gob. Main compositing function.
**
**  If the ONLY arg is TRUE then the specified gob area will be
**  rendered to the buffer at 0x0 offset.(used by TO-IMAGE)
**
***********************************************************************/
{
	REBINT max_depth = 1000; // avoid infinite loops
	REBINT intersection_result;
	REBD32 abs_x;
	REBD32 abs_y;
	REBD32 abs_ox;
	REBD32 abs_oy;
	REBGOB* parent_gob = gob;
	RECT gob_clip;

//	RL_Print("COMPOSE %d %d\n", GetDeviceCaps(ctx->backDC, SHADEBLENDCAPS), GetDeviceCaps(ctx->winDC, SHADEBLENDCAPS));
	
	abs_x = 0;
	abs_y = 0;

	//reset clip region to window area
	SetRectRgn(ctx->Win_Clip, 0, 0, GOB_LOG_W_INT(winGob), GOB_LOG_H_INT(winGob));
	SelectClipRgn(ctx->backDC, ctx->Win_Clip);

	//the offset is shifted to render given gob at offset 0x0 (used by TO-IMAGE)
	if (only){
		abs_x = 0;
		abs_y = 0;
	} else {
		//calculate absolute offset of the gob
		while (GOB_PARENT(parent_gob) && (max_depth-- > 0) && !GET_GOB_FLAG(parent_gob, GOBF_WINDOW))
		{
			abs_x += GOB_LOG_X(parent_gob);
			abs_y += GOB_LOG_Y(parent_gob);
			parent_gob = GOB_PARENT(parent_gob);
		}
	}

	ctx->absOffset.x = 0;
	ctx->absOffset.y = 0;
	
	if (!GET_GOB_STATE(gob, GOBS_NEW)){
		//calculate absolute old offset of the gob
		abs_ox = abs_x + (GOB_XO(gob) - GOB_LOG_X(gob));
		abs_oy = abs_y + (GOB_YO(gob) - GOB_LOG_Y(gob));
		
//		RL_Print("OLD: %dx%d %dx%d\n",(REBINT)abs_ox, (REBINT)abs_oy, (REBINT)abs_ox + GOB_WO_INT(gob), (REBINT)abs_oy + GOB_HO_INT(gob));
		
		//set region with old gob location and dimensions
		SetRectRgn(ctx->Old_Clip, (REBINT)abs_ox, (REBINT)abs_oy, (REBINT)abs_ox + GOB_WO_INT(gob), (REBINT)abs_oy + GOB_HO_INT(gob));
	}
	
//	RL_Print("NEW: %dx%d %dx%d\n",(REBINT)abs_x, (REBINT)abs_y, (REBINT)abs_x + GOB_W_INT(gob), (REBINT)abs_y + GOB_H_INT(gob));
	
	//Create union of "new" and "old" gob location
	SetRectRgn(ctx->New_Clip, (REBINT)abs_x, (REBINT)abs_y, (REBINT)abs_x + GOB_LOG_W_INT(gob), (REBINT)abs_y + GOB_LOG_H_INT(gob));
	CombineRgn(ctx->Win_Clip, ctx->Old_Clip, ctx->New_Clip, RGN_OR);

	
	//intersect resulting region with window clip region
	intersection_result = ExtSelectClipRgn(ctx->backDC, ctx->Win_Clip, RGN_AND);

	GetClipBox(ctx->backDC, &gob_clip);
//	RL_Print("old+new clip: %dx%d %dx%d\n", gob_clip.left, gob_clip.top, gob_clip.right, gob_clip.bottom);
	
	if (intersection_result != NULLREGION)
		//redraw gobs
		process_gobs(ctx, only ? gob : winGob);
	
	//update old GOB area
	GOB_XO(gob) = GOB_LOG_X(gob);
	GOB_YO(gob) = GOB_LOG_Y(gob);
	GOB_WO(gob) = GOB_LOG_W(gob);
	GOB_HO(gob) = GOB_LOG_H(gob);
}

/***********************************************************************
**
*/ void rebcmp_blit(REBCMP_CTX* ctx)
/*
**	Blit window content on the screen.
**
***********************************************************************/
{
	BitBlt(
		ctx->winDC,
		0, 0,
		GOB_LOG_W_INT(ctx->Win_Gob), GOB_LOG_H_INT(ctx->Win_Gob),
		ctx->backDC,
		0, 0,
		SRCCOPY
	);
}
