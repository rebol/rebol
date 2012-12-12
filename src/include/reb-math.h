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
**  Summary: Math related definitions
**  Module:  reb-math.h
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

// Decimal number formatting specifications:
typedef struct Reb_Deci_Spec {
	REBDEC dec;		// number to form
	REBINT len;		// # of digits requested
	REBCHR *out;	// result: string of digits (no point or sign)
	REBINT point;	// result: position of decimal point
	REBINT sign;	// result: sign of number
} REBDCS;
