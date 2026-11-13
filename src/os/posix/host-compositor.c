/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
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
**  Title: Posix Compositor abstraction layer API.
**  Author: Oldes
**  File:  host-compositor.c
**  Purpose: Provides simple gob compositor code for Posix.
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

#include <math.h> //for floor()
#ifdef DEBUG_DRAW_REGIONS
#include <stdlib.h> //for rand()
#endif
#include "reb-host.h"
#include "host-lib.h"
#include "reb-compositor.h"

//***** Externs *****

//***** Macros *****

//#define GOB_HWIN(gob)	((HWND)OS_Find_Window(gob))

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
	return NULL;
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
//	ReleaseDC(GOB_HWIN(ctx->wind_gob), ctx->wind_DC);
//	DeleteDC(ctx->back_DC);
//	DeleteObject(ctx->back_buffer);
//	DeleteObject(ctx->win_clip);
//	DeleteObject(ctx->new_clip);
//	DeleteObject(ctx->old_clip);
//	OS_Free(ctx);
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

}


/***********************************************************************
**
*/  REBSER* OS_Gob_To_Image(REBGOB *gob)
/*
**		Render gob into an image.
**
***********************************************************************/
{
	return NULL;
}


/***********************************************************************
**
*/ void OS_Blit_Window(REBCMP* ctx)
/*
**	Blit window content on the screen.
**
***********************************************************************/
{
}


/***********************************************************************
**
*/	void OS_Blit_Gob_Color(REBGOB *gob, REBCMP* ctx, REBXYI abs_oft, REBXYI clip_oft, REBXYI clip_siz)
/*
**		Fill color rectangle, a pixel at a time.
**
***********************************************************************/
{

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

}


