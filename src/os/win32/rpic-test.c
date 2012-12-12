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
**  rpic-test.c - Test for REBOL Plug-In Component
**
***********************************************************************/

//#include <windows.h>

#define ODD_INT_64

#include "reb-c.h"
#include "reb-plugin.h"
#include "plugin-types.h"

#define REBSER void
#include "rpi-lib.h"

const char *init_block =
	"REBOL [\n"
		"Title: {Example plugin}\n"
		"Name: example\n"
		"Type: plugin\n"
		"Exports: [map-words cmd1 cmd2 cmd2i cmd2d cmdw cmds cmdb cmdbl]\n"
	"]\n"
	"map-words: command [words [block!]]\n"
	"cmd1: command [a]\n"
	"cmd2: command [a b]\n"
	"cmd2i: command [a [integer!] b [integer!]]\n"
	"cmd2d: command [a [decimal!] b [decimal!]]\n"
	"cmdw: command [w]\n"
	"cmds: command [str [string!] index [integer!]]\n"
	"cmdb: command [blk [block!] index [integer!]]\n"
	"cmdbl: command [blk [block!]]\n"
;

RPIEXT const char *RPI_Init(int opts, RPILIB *lib) {
	RPI = lib;
	if (lib->version == RPI_VERSION) return init_block;
	return 0;
}

RPIEXT int RPI_Quit(int opts) {
	return 0;
}

u32 *word_ids = 0;

RPIEXT int RPI_Call(int cmd, RPIFRM *frm) {
	switch (cmd) {
	case 0:
		word_ids = RPI_MAP_WORDS(RPA_SERIES(frm,1));
		return RPR_TRUE;
	case 1:
		RPA_INT64(frm,1) = -RPA_INT64(frm,1);
		break;
	case 2:
	case 3:
		RPA_INT64(frm,1) = RPA_INT64(frm, 1) + RPA_INT64(frm, 2);
		break;
	case 4:
		RPA_DEC64(frm,1) = RPA_DEC64(frm, 1) + RPA_DEC64(frm, 2);
		break;
	case 5:
		RPA_INT64(frm,1) = RPI_FIND_WORD(word_ids, RPA_WORD(frm,1));
		RPA_TYPE(frm,1) = RPT_INTEGER;
		break;
	case 6:
		RPA_INT64(frm,1) = RPI_GET_CHAR(RPA_SERIES(frm,1), (u32)RPA_INT64(frm,2)-1);
		RPA_TYPE(frm,1) = RPT_INTEGER;
		break;
	case 7:
		RPA_TYPE(frm,1) = RPI_GET_VALUE(RPA_SERIES(frm,1), (u32)RPA_INT64(frm,2)-1, &RPA_ARG(frm, 1));
		break;
	case 8:
		RPA_INT64(frm,1) = RPI_SERIES_INFO(RPA_SERIES(frm,1), RPI_INFO_TAIL);
		RPA_TYPE(frm,1) = RPT_INTEGER;
		break;
	}
	return RPR_VALUE;
}
