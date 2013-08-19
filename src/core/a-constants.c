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
**  Module:  a-constants.c
**  Summary: special global constants and strings
**  Section: environment
**  Author:  Carl Sassenrath
**  Notes:
**     Very few strings should be located here. Most strings are
**     put in the compressed embedded boot image. That saves space,
**     reduces tampering, and allows UTF8 encoding. See ../boot dir.
**
***********************************************************************/

#include "sys-core.h"

#define BP (REBYTE*)

const REBYTE Str_Banner[] = "REBOL 3 %d.%d.%d.%d.%d";

const char Str_REBOL[] = "REBOL";

const REBYTE * Str_Stack_Misaligned = {
	BP("!! Stack misaligned: %d")
};

const REBYTE * const Crash_Msgs[] = {
	BP"REBOL System Error",
	BP"boot failure",
	BP"internal problem",
	BP"assertion failed",
	BP"invalid datatype %d",
	BP"unspecific",
	BP"\n\nProgram terminated abnormally.\nThis should never happen.\nPlease contact www.REBOL.com with details."
};

const REBYTE * const Str_Dump[] = {
	BP"%s Series %x \"%s\": wide: %2d size: %6d bias: %d tail: %d rest: %d flags: %x"
};

const REBYTE * Hex_Digits = BP"0123456789ABCDEF";

const REBYTE * const Bad_Ptr = BP"#[BAD-PTR]";

const REBYTE * const Esc_Names[] = {
	// Must match enum REBOL_Esc_Codes!
	BP"line",
	BP"tab",
	BP"page",
	BP"escape",
	BP"esc",
	BP"back",
	BP"del",
	BP"null"
};

const REBYTE Esc_Codes[] = {
	// Must match enum REBOL_Esc_Codes!
	10,
	9,
	12,
	27,
	27,
	8,
	127,
	0
};

const REBYTE Month_Lengths[12] = {
	31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

const REBYTE * const Month_Names[12] = {
	BP"January",	BP"February", BP"March",
	BP"April",	BP"May",		 BP"June",
	BP"July",	BP"August",	 BP"September",
	BP"October",	BP"November", BP"December"
};

