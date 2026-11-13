/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2026 Rebol Open Source Developers
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
**  Module:  n-request-color.c
**  Summary: native request-color function
**  Section: natives
**  Author:  Oldes
**	Notes:
**		So far it is used only on Windows. On macOS it's implementented
**		as Rebol mezzanine function in file mezz-osx-dialogs.reb
***********************************************************************/

#include "sys-core.h"

/***********************************************************************
**
*/	REBNATIVE(request_color)
/*
//	request-color: native [
//		{Asks user to select a color.}
//		/default  "Default RGB color"
//		 color    [tuple!]
//	]
//
***********************************************************************/
{
#ifdef TO_WINDOWS
	static REBYTE color[4];
	if (D_REF(1)) {
		COPY_MEM(color, VAL_TUPLE(D_ARG(2)), 4);
	}
	if (OS_Request_Color((REBCNT*)&color)) {
		VAL_SET(DS_RETURN, REB_TUPLE);
		VAL_TUPLE_LEN(DS_RETURN) = 3;
		COPY_MEM(VAL_TUPLE(DS_RETURN), &color, 4);
		return R_RET;
	}
	else return R_NONE;
#else
	Trap0(RE_FEATURE_NA);
	return R_NONE;
#endif
}