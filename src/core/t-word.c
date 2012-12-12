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
**  Module:  t-word.c
**  Summary: word related datatypes
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

#ifdef not_used
/***********************************************************************
**
**	REBFLG MT_Word(REBVAL *out, REBVAL *data, REBCNT type)
**
***********************************************************************/
{
	if (!IS_WORD(data)) return FALSE;
	*out = *data;
	VAL_SET(out, type);
	return TRUE;
}
#endif


/***********************************************************************
**
*/	REBINT CT_Word(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	REBINT e;
	REBINT diff;
	if (mode >= 0) {
		e = VAL_WORD_CANON(a) == VAL_WORD_CANON(b);
		if (mode == 1) e &= VAL_WORD_INDEX(a) == VAL_WORD_INDEX(b)
			&& VAL_WORD_FRAME(a) == VAL_WORD_FRAME(b);
		else if (mode >= 2) {
			e = (VAL_WORD_SYM(a) == VAL_WORD_SYM(b) &&
				VAL_WORD_INDEX(a) == VAL_WORD_INDEX(b) &&
				VAL_WORD_FRAME(a) == VAL_WORD_FRAME(b));
		}
	} else {
		diff = Compare_Word(a, b, FALSE);
		if (mode == -1) e = diff >= 0;
		else e = diff > 0;
	}
	return e;
}


/***********************************************************************
**
*/	REBTYPE(Word)
/*
***********************************************************************/
{
	REBVAL *val = D_ARG(1);
	REBVAL *arg = D_ARG(2);
	REBCNT type = VAL_TYPE(val);
	REBINT diff;
	REBCNT sym;

	switch (action) {
	case A_LENGTHQ:
		diff = LEN_BYTES(Get_Sym_Name(VAL_WORD_SYM(val)));
		if (type != REB_WORD) diff++;
		DS_Ret_Int(diff);
		break;

	case A_MAKE:
	case A_TO:
		// TO word! ...
		if (type == REB_DATATYPE) type = (REBCNT)VAL_DATATYPE(val);
		if (ANY_WORD(arg)) {
			VAL_SET(arg, type);
			return R_ARG2;
		}
		else {
			if (IS_STRING(arg)) {
				REBYTE *bp;
				REBCNT len;
				// Set sym. Rest is set below.
				bp = Qualify_String(arg, 255, &len, TRUE);
				if (type == REB_ISSUE) sym = Scan_Issue(bp, len);
				else sym = Scan_Word(bp, len);
				if (!sym) Trap1(RE_BAD_CHAR, arg);
			}
			else if (IS_CHAR(arg)) {
				REBYTE buf[8];
				sym = Encode_UTF8_Char(&buf[0], VAL_CHAR(arg)); //returns length
				sym = Scan_Word(&buf[0], sym);
				if (!sym) Trap1(RE_BAD_CHAR, arg);
			}
			else if (IS_DATATYPE(arg)) {
				sym = VAL_DATATYPE(arg)+1;
			}
			else if (IS_LOGIC(arg)) {
				sym = IS_TRUE(arg) ? SYM_TRUE : SYM_FALSE;
			}
			else Trap_Types(RE_EXPECT_VAL, REB_WORD, VAL_TYPE(arg));
			Set_Word(D_RET, sym, 0, 0);
			VAL_SET(D_RET, type);
		}
		break;

	default:
		Trap_Action(type, action);
	}

	return R_RET;
}
