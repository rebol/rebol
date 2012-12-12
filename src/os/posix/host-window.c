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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

#include "reb-host.h"
#include "host-lib.h"
#include "rebol-lib.h"


//**********************************************************************
//** OSAL Library Functions ********************************************
//**********************************************************************

/***********************************************************************
**
*/	void OS_Init_Graphics(REBGOB *gob)
/*
**  Initialize graphics subsystem. Store Gob_Root.
**
***********************************************************************/
{
}


/***********************************************************************
**
*/  void OS_GUI_Metrics(REBOL_OS_METRICS *met)
/*
**      Provide info about the hosting GUI.
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
	return 0;
}


/***********************************************************************
**
*/  void OS_Map_Gob(REBGOB **gob, REBPAR *xy, REBOOL inner)
/*
**      Map GOB and offset to inner or outer GOB and offset.
**
***********************************************************************/
{
}


/***********************************************************************
**
*/  REBINT OS_Size_Text(REBGOB *gob, REBPAR *size)
/*
**      Return the area size of the text.
**
***********************************************************************/
{
	return 0;
}


/***********************************************************************
**
*/  REBINT OS_Offset_To_Caret(REBGOB *gob, REBPAR xy, REBINT *element, REBINT *position)
/*
**      Return the element and position for a given offset pair.
**
***********************************************************************/
{
	return 0;
}


/***********************************************************************
**
*/  REBINT OS_Caret_To_Offset(REBGOB *gob, REBPAR *xy, REBINT element, REBINT position)
/*
**      Return the offset pair for a given element and position.
**
***********************************************************************/
{
	return 0;
}


/***********************************************************************
**
*/  REBINT OS_Gob_To_Image(REBSER *image, REBGOB *gob)
/*
**      Render gob into an image.
**		Clip to keep render inside the image provided.
**
***********************************************************************/
{
	return 0;
}


/***********************************************************************
**
*/  REBINT OS_Draw_Image(REBSER *image, REBSER *block)
/*
**      Render DRAW dialect into an image.
**		Clip to keep render inside the image provided.
**
***********************************************************************/
{
	return 0;
}


/***********************************************************************
**
*/  REBINT OS_Effect_Image(REBSER *image, REBSER *block)
/*
**      Render EFFECT dialect into an image.
**		Clip to keep render inside the image provided.
**
***********************************************************************/
{
	return 0;
}

/***********************************************************************
**
*/  void OS_Cursor_Image(REBINT n, REBSER *image)
/*
***********************************************************************/
{
}
