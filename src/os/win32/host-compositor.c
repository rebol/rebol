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
**	Copyright 2019 Oldes
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
**  Author: Oldes, Richard Smolak
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
#ifndef WINVER
#define WINVER 0x0500  //for AlphaBlend()
#define _WIN32_WINNT 0x0500  //for DC_BRUSH
#endif
#include <windows.h>

#include <math.h> //for floor()
#ifdef DEBUG_DRAW_REGIONS
#include <stdlib.h> //for rand()
#endif
#include "reb-host.h"
#include "host-lib.h"
#include "reb-compositor.h"

//***** Externs *****

//***** Macros *****

#define GOB_HWIN(gob)	((HWND)OS_Find_Window(gob))

//***** Locals *****

static REBXYF Zero_Pair = {0, 0};

/***********************************************************************
**
*/ void* OS_Create_Compositor(REBGOB* rootGob, REBGOB* gob)
/*
**	Create new Compositor instance.
**
***********************************************************************/
{
	//new compositor struct
	REBCMP *ctx = (REBCMP*)OS_Make(sizeof(REBCMP));

	//bitmapinfo struct
	CLEARS(&ctx->bmpInfo);
	ctx->bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	ctx->bmpInfo.bmiHeader.biPlanes = 1;
	ctx->bmpInfo.bmiHeader.biBitCount = 32;
	ctx->bmpInfo.bmiHeader.biCompression = BI_RGB;

	//shortcuts
	ctx->root_gob = rootGob;
	ctx->wind_gob = gob;
	ctx->wind_DC = GetDC(GOB_HWIN(gob));
	ctx->back_DC = 0;
	ctx->back_buffer = 0;

	//custom color brush
	ctx->brush_DC = GetStockObject(DC_BRUSH);

	//initialize clipping regions
	ctx->win_clip = CreateRectRgn(0, 0, GOB_LOG_W_INT(gob), GOB_LOG_H_INT(gob));
	ctx->new_clip = CreateRectRgn(0, 0, 0, 0);
	ctx->old_clip = CreateRectRgn(0, 0, 0, 0);

	ctx->win_rect.left = 0;
	ctx->win_rect.top = 0;
	ctx->win_rect.right = GOB_LOG_W_INT(gob);
	ctx->win_rect.bottom = GOB_LOG_H_INT(gob);

	//call resize to init rest
	OS_Resize_Window_Buffer(ctx, gob);

	return ctx;
}


/***********************************************************************
**
*/ void OS_Destroy_Compositor(REBCMP* ctx)
/*
**	Destroy existing Compositor instance.
**
***********************************************************************/
{
	//do cleanup
	ReleaseDC(GOB_HWIN(ctx->wind_gob), ctx->wind_DC);
	DeleteDC(ctx->back_DC);
	DeleteObject(ctx->back_buffer);
	DeleteObject(ctx->win_clip);
	DeleteObject(ctx->new_clip);
	DeleteObject(ctx->old_clip);
	OS_Free(ctx);
}


/***********************************************************************
**
*/ REBYTE* OS_Get_Window_Buffer(REBCMP* ctx)
/*
**	Provide pointer to window compositing buffer.
**  Return NULL if buffer not available of call failed.
**
**  NOTE: The buffer may be "locked" during this call on some platforms.
**        Always call Os_Release_Window_Buffer(ctx) to be sure it is released.
**
***********************************************************************/
{
	return ctx->wind_buffer;
}

/***********************************************************************
**
*/ void OS_Release_Window_Buffer(REBCMP* ctx)
/*
**	Release the window compositing buffer acquired by Os_Get_Window_Buffer().
**
**  NOTE: this call can be "no-op" on platforms that don't need locking.
**
***********************************************************************/
{
}


/***********************************************************************
**
*/ REBOOL OS_Resize_Window_Buffer(REBCMP* ctx, REBGOB* winGob)
/*
**	Resize the window compositing buffer.
**
**  Returns TRUE if buffer size was really changed, otherwise FALSE.
**
***********************************************************************/
{
	//check if window size really changed or buffer needs to be created
	if ((GOB_LOG_W(winGob) != GOB_WO(winGob)) || (GOB_LOG_H(winGob) != GOB_HO(winGob)) || ctx->back_buffer == 0) {
		HBITMAP new_buffer;
		HDC     new_DC;
		REBYTE *new_bytes;
		REBINT  w = GOB_LOG_W_INT(winGob);
		REBINT  h = GOB_LOG_H_INT(winGob);
		
		//set window size in bitmapinfo struct
		ctx->bmpInfo.bmiHeader.biWidth = w;
		ctx->bmpInfo.bmiHeader.biHeight = -h;

		//create new window backbuffer and DC
		new_buffer = CreateDIBSection(ctx->wind_DC, &ctx->bmpInfo, DIB_RGB_COLORS,(VOID **)&new_bytes, 0, 0);
		new_DC = CreateCompatibleDC(ctx->wind_DC);

		//update the buffer size values
		ctx->wind_size.x = w;
		ctx->wind_size.y = h;
		
		//select new DC with back buffer and delete old DC(to prevent leak)
		DeleteObject((HBITMAP)SelectObject(new_DC, new_buffer));

		//fill the background color
		SetDCBrushColor(new_DC, RGB(240,240,240));		
		FillRect(new_DC, &ctx->win_rect, ctx->brush_DC);
		
		if (ctx->back_DC != 0) {
/*		
			//copy the current content
			BitBlt(
				new_DC,
				0, 0,
				w, h,
				ctx->back_DC,
				0, 0,
				SRCCOPY
			);
*/			
			//cleanup of previously used objects
			DeleteObject(ctx->back_buffer);
			DeleteDC(ctx->back_DC);
		}
		
		//make the new buffer actual
		ctx->back_buffer = new_buffer;
		ctx->back_DC = new_DC;
		ctx->wind_buffer = new_bytes;

		//set window clip region
//		SetRectRgn(ctx->win_clip, 0, 0, w, h);
//		SelectClipRgn(ctx->back_DC, ctx->win_clip);

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
*/ static void process_gobs(REBCMP* ctx, REBGOB* gob)
/*
**	Recursively process and compose gob and its children.
**
**  NOTE: this function is used internally by OS_Compose_Gob() call only.
**
***********************************************************************/
{
	REBINT x = ROUND_TO_INT(ctx->abs_offset.x);
	REBINT y = ROUND_TO_INT(ctx->abs_offset.y);
	REBINT intersection_result;
	RECT gob_clip;

	if (GET_GOB_STATE(gob, GOBS_NEW)){
		//reset old-offset and old-size if newly added
		GOB_XO(gob) = GOB_LOG_X(gob);
		GOB_YO(gob) = GOB_LOG_Y(gob);
		GOB_WO(gob) = GOB_LOG_W(gob);
		GOB_HO(gob) = GOB_LOG_H(gob);

#ifdef HAS_WIDGET_GOB
		if (GOB_TYPE(gob) == GOBT_WIDGET) {
			OS_Init_Gob_Widget(ctx, gob);
		}
#endif

		CLR_GOB_STATE(gob, GOBS_NEW);
	}

	//RL_Print("type: %d oft: %dx%d siz: %dx%d abs_oft: %dx%d \n", GOB_TYPE(gob), GOB_X_INT(gob), GOB_Y_INT(gob), GOB_W_INT(gob), GOB_H_INT(gob), x, y);

	//intersect gob dimensions with actual window clip region
	SetRectRgn(ctx->win_clip, x, y, x + GOB_LOG_W_INT(gob), y + GOB_LOG_H_INT(gob));
	intersection_result = ExtSelectClipRgn(ctx->back_DC, ctx->win_clip, RGN_AND);


	GetClipBox(ctx->back_DC, &gob_clip);
	//RL_Print("clip: %dx%d %dx%d\n", gob_clip.left, gob_clip.top, gob_clip.right, gob_clip.bottom);
	
	if (intersection_result != NULLREGION)
	{
		//RL_Print("clip OK %d %d\n", intersection_result, GOB_TYPE(gob));
		
		//render GOB content
		REBXYI offset = {x,y};
		REBXYI top_left = {gob_clip.left, gob_clip.top};
		REBXYI bottom_right = {gob_clip.right, gob_clip.bottom};
		switch (GOB_TYPE(gob)) {
			case GOBT_COLOR:
				//RL_Print("draw color gob %dx%d\n", x, y);
				OS_Blit_Gob_Color(gob, ctx, offset, top_left, bottom_right);
				break;
			
			case GOBT_IMAGE:
				//RL_Print("draw image gob\n");
				OS_Blit_Gob_Image(gob, ctx, offset, gob_clip.top, gob_clip.left, gob_clip.bottom, gob_clip.right);
				break;
	
#ifdef HAS_WIDGET_GOB
			case GOBT_WIDGET:
				//RL_Print("draw widget\n");
				break;
#endif

			case GOBT_DRAW:
				//not implemented
				break;

			case GOBT_TEXT:
			case GOBT_STRING:
				//not implemented
				break;
				
			case GOBT_EFFECT:
				//not implemented
				break;
		}
		
#ifdef DEBUG_DRAW_REGIONS
		//draw clip region frame (for debugging)
		GetClipRgn(ctx->back_DC, ctx->win_clip); //copy the actual region to win_clip
		SetDCBrushColor(ctx->back_DC, RGB(rand() % 256, rand() % 256, rand() % 256));
		FrameRgn(ctx->back_DC, ctx->win_clip, ctx->brush_DC, 1, 1);	
#endif

		//recursively process sub GOBs
		if (GOB_PANE(gob)) {
			REBINT n;
//			RECT parent_clip;		
			REBINT len = GOB_TAIL(gob);
			REBGOB **gp = GOB_HEAD(gob);
			
			//store clip region coords
//			GetClipBox(ctx->back_DC, &parent_clip);

			for (n = 0; n < len; n++, gp++) {
				REBINT g_x = GOB_LOG_X(*gp);
				REBINT g_y = GOB_LOG_Y(*gp);

				//restore the parent clip region
//				SetRectRgn(ctx->win_clip, parent_clip.left, parent_clip.top, parent_clip.right, parent_clip.bottom);
				SetRectRgn(ctx->win_clip, gob_clip.left, gob_clip.top, gob_clip.right, gob_clip.bottom);
				SelectClipRgn(ctx->back_DC, ctx->win_clip);
				
				ctx->abs_offset.x += g_x;
				ctx->abs_offset.y += g_y;
				
				process_gobs(ctx, *gp);

				ctx->abs_offset.x -= g_x;
				ctx->abs_offset.y -= g_y;
			}
		}
	} //else {RL_Print("invisible!\n");}
}

/***********************************************************************
**
*/ void OS_Compose_Gob(REBCMP* ctx, REBGOB* winGob, REBGOB* gob, REBOOL only)
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

	//RL_Print("OS_Compose_Gob only: %d\n", only);
	
	abs_x = 0;
	abs_y = 0;

	//reset clip region to window area
	SetRectRgn(ctx->win_clip, 0, 0, GOB_LOG_W_INT(winGob), GOB_LOG_H_INT(winGob));
	SelectClipRgn(ctx->back_DC, ctx->win_clip);

	//the offset is shifted to render given gob at offset 0x0 (used by TO-IMAGE)
	if (only) {
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

	ctx->abs_offset.x = 0;
	ctx->abs_offset.y = 0;
	
	if (!GET_GOB_STATE(gob, GOBS_NEW)) {
		//calculate absolute old offset of the gob
		abs_ox = abs_x + (GOB_XO(gob) - GOB_LOG_X(gob));
		abs_oy = abs_y + (GOB_YO(gob) - GOB_LOG_Y(gob));
		
		//RL_Print("OLD: %dx%d %dx%d\n",(REBINT)abs_ox, (REBINT)abs_oy, (REBINT)abs_ox + GOB_WO_INT(gob), (REBINT)abs_oy + GOB_HO_INT(gob));
		
		//set region with old gob location and dimensions
		SetRectRgn(ctx->old_clip, (REBINT)abs_ox, (REBINT)abs_oy, (REBINT)abs_ox + GOB_WO_INT(gob), (REBINT)abs_oy + GOB_HO_INT(gob));
	}
	
	//RL_Print("NEW: %dx%d %dx%d\n",(REBINT)abs_x, (REBINT)abs_y, (REBINT)abs_x + GOB_W_INT(gob), (REBINT)abs_y + GOB_H_INT(gob));
	
	//Create union of "new" and "old" gob location
	SetRectRgn(ctx->new_clip, (REBINT)abs_x, (REBINT)abs_y, (REBINT)abs_x + GOB_LOG_W_INT(gob), (REBINT)abs_y + GOB_LOG_H_INT(gob));
	CombineRgn(ctx->win_clip, ctx->old_clip, ctx->new_clip, RGN_OR);

	
	//intersect resulting region with window clip region
	intersection_result = ExtSelectClipRgn(ctx->back_DC, ctx->win_clip, RGN_AND);

	GetClipBox(ctx->back_DC, &gob_clip);
	//RL_Print("old+new clip: %dx%d %dx%d\n", gob_clip.left, gob_clip.top, gob_clip.right, gob_clip.bottom);
	
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
*/  REBSER* OS_Gob_To_Image(REBGOB *gob)
/*
**		Render gob into an image.
**
***********************************************************************/
{
	REBINT w, h;
	REBSER *img;
	REBCMP *cmp;

	w = GOB_LOG_W_INT(gob);
	h = GOB_LOG_H_INT(gob);
	img = (REBSER*)RL_MAKE_IMAGE(w, h);

	cmp = OS_Create_Compositor(Gob_Root, gob);
	OS_Compose_Gob(cmp, gob, gob, TRUE);

	//copy the composed result to image
	memcpy((REBYTE *)RL_SERIES(img, RXI_SER_DATA), OS_Get_Window_Buffer(cmp), w * h * 4);

	OS_Release_Window_Buffer(cmp);
	OS_Destroy_Compositor(cmp);

	return img;
}


/***********************************************************************
**
*/ void OS_Blit_Window(REBCMP* ctx)
/*
**	Blit window content on the screen.
**
***********************************************************************/
{
	BitBlt(
		ctx->wind_DC,
		0, 0,
		GOB_LOG_W_INT(ctx->wind_gob), GOB_LOG_H_INT(ctx->wind_gob),
		ctx->back_DC,
		0, 0,
		SRCCOPY
	);
}


/***********************************************************************
**
*/	void OS_Blit_Gob_Color(REBGOB *gob, REBCMP* ctx, REBXYI abs_oft, REBXYI clip_oft, REBXYI clip_siz)
/*
**		Fill color rectangle, a pixel at a time.
**
***********************************************************************/
{
	COLORREF rgb;
	RECT rect;

	//if (!gob || GOB_TYPE(gob) != GOBT_COLOR) return;
	//if (!IS_WINDOW(gob)) return;

	REBYTE* color = (REBYTE*)&GOB_CONTENT(gob);
	rgb = color[C_R] | (color[C_G] << 8) | (color[C_B] << 16);

	rect.left   = clip_oft.x;
	rect.top    = clip_oft.y;
	rect.right  = clip_siz.x + clip_oft.x;
	rect.bottom = clip_siz.y + clip_oft.y;

	//RL_Print("OS_Blit_Gob_Color: %dx%d %dx%d\n", rect.left, rect.top, rect.right, rect.bottom);

	FillRect(ctx->back_DC, &rect, CreateSolidBrush(rgb));
}

/***********************************************************************
**
*/	void OS_Blit_Gob_Image(REBGOB *gob, REBCMP* ctx, REBXYI abs_oft, REBINT top, REBINT left, REBINT bottom, REBINT right)
/*
**		This routine copies a rectangle from a PAN structure to the
**		current output device.
**
***********************************************************************/
{
	//if (!gob || GOB_TYPE(gob) != GOBT_IMAGE || !GOB_CONTENT(gob)) return;

	REBSER     *img = (REBSER*)GOB_CONTENT(gob);
	HDC         hdc = ctx->back_DC;
//	BITMAPINFO  BitmapInfo = ctx->bmpInfo;
//	REBINT      mode;
	REBINT      src_siz_x = IMG_WIDE(img); // real image size
	REBINT      src_siz_y = IMG_HIGH(img);

#define alphaBlend
#ifdef alphaBlend
	BLENDFUNCTION bf;      // structure for alpha blending 
	HBITMAP hbitmap;       // bitmap handle 
	BITMAPINFO bmi;        // bitmap header 
	// zero the memory for the bitmap info 
    ZeroMemory(&bmi, sizeof(BITMAPINFO));

	// setup bitmap info  
    // set the bitmap width and height to 60% of the width and height of each of the three horizontal areas. Later on, the blending will occur in the center of each of the three areas. 
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = src_siz_x;
    bmi.bmiHeader.biHeight = -src_siz_y;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;         // four 8-bit components 
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = src_siz_x * src_siz_y * 4;

	REBYTE *pixels = IMG_DATA(img); 
	//printf("p>: %x\n", ((int*)IMG_DATA(img))[0]);

	REBYTE *pvBits;          // pointer to DIB section 
	REBYTE *p;

	// create a DC for our bitmap -- the source DC for AlphaBlend 
	HDC hdcbmp = CreateCompatibleDC(hdc);
	//hbitmap = CreateDIBitmap(hdcbmp, &bmi.bmiHeader, CBM_INIT, IMG_DATA(img), &bmi, DIB_RGB_COLORS);
	//if (hbitmap == NULL) {
	//	puts("faild to init bmp");
	//	return;
	//}
	//HBITMAP hbitmap   = CreateCompatibleBitmap(hdc, src_siz_x, src_siz_y);
	   

	// create our DIB section and select the bitmap into the dc 
    hbitmap = CreateDIBSection(hdcbmp, &bmi, DIB_RGB_COLORS, &pvBits , NULL, 0x0);
 	SelectObject(hdcbmp, hbitmap);

//	memcpy(pvBits, pixels, src_siz_x * src_siz_y * 4 );
	p = pvBits;
	//printf("p0: %x\n", ((int*)p)[0]);
	for( int y = 0; y < src_siz_y; y++ ) {
      for( int x = 0; x < src_siz_x; x++ ) {
         p[0] = (BYTE)((DWORD)pixels[0] * pixels[3] / 255);
         p[1] = (BYTE)((DWORD)pixels[1] * pixels[3] / 255);
         p[2] = (BYTE)((DWORD)pixels[2] * pixels[3] / 255);
		 p[3] = (BYTE)pixels[3];
         p += 4;
		 pixels += 4;
      }
   }
	//printf("p1: %x\n", ((int*)pvBits)[0]);

	bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 0xFF; //0x7f;  // half of 0xff = 50% transparency 
    bf.AlphaFormat = AC_SRC_ALPHA; //0;     // ignore source alpha channel 

	//RL_Print("AlphaBlend: %dx%d %dx%d\n", left, top, src_siz_x, src_siz_y);

	if (!AlphaBlend(hdc, left, top,
		src_siz_x, src_siz_y,
		hdcbmp, 0, 0, src_siz_x, src_siz_y, bf)) {
		RL_Print("alphaBlend failed!\n");
	}
	
	// do cleanup 
    DeleteObject(hbitmap);
    DeleteDC(hdcbmp);

#else

	BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BitmapInfo.bmiHeader.biWidth = src_siz_x;
	BitmapInfo.bmiHeader.biHeight = -src_siz_y;
	
	mode = SetStretchBltMode(hdc, COLORONCOLOR); // returns previous mode
	StretchDIBits(
		hdc,
		left, top,
		//right - left, bottom - top,
		src_siz_x, src_siz_y,
		0, 0, // always at 0x0 so far; should we support image atlases?
		src_siz_x, src_siz_y,
		GOB_BITMAP(gob),
		&BitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY
	);
	SetStretchBltMode(hdc, mode);
#endif
}


