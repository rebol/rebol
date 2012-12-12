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
**  Module:  b-main.c
**  Summary: skip
**  Section: bootstrap
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/
#include "sys-core.h"

static REBARGS Main_Args;	// Not multi-threaded

/***********************************************************************
**
*/	char *Prompt_User()
/*
***********************************************************************/
{
	char *text;

	Prin("DSP: %d Mem: %d >> ", DSP, PG_Mem_Usage);
	text = Input_Str();
	if (*text == '\n') exit(0);
	return text;
}


/***********************************************************************
**
*/	int main(int argc, char **argv)
/*
***********************************************************************/
{
	char *cmd;

	// Parse command line arguments. Done early. May affect REBOL boot.
	Parse_Args(argc, argv, &Main_Args);

	Print_Str("REBOL 3.0\n");

	REBOL_Init(&Main_Args);

	// Evaluate user input:
	while (TRUE) {
		cmd = Prompt_User();
		REBOL_Do_String(cmd);
		if (!IS_UNSET(DS_TOP)) {
			//if (DSP > 0) {
				if (!IS_ERROR(DS_TOP)) {
					Prin("== ");
					Print_Value(DS_TOP, 0, TRUE);
				} else
					Print_Value(DS_TOP, 0, FALSE);
			//}
		}
		//DS_DROP; // result
	}

	return 0;
}
