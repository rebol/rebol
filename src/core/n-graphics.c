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
**  Module:  n-graphics.c
**  Summary: native functions for graphical objects
**  Section: natives
**  Author:  Carl Sassenrath
**  Notes:
**     Obsolete: Graphics is now implemented as an R3 extension.
**
***********************************************************************/

#include "sys-core.h"


#ifdef MOVED_TO_EXTENSION
/***********************************************************************
**
*/	REBFLG Find_Gob_Tree(REBGOB *gob, REBGOB *tgob)
/*
**		Scan a gob tree for the target gob. Return true or false.
**		Gob MUST have a pane (check for it first).
**
***********************************************************************/
{
	REBGOB **gp;
	REBINT n;

	gp = GOB_HEAD(gob);

	for (n = GOB_TAIL(gob); n > 0; n--, gp++) {
		if (*gp == tgob) return TRUE;
		if (GOB_PANE(*gp) && Find_Gob_Tree(*gp, tgob)) return TRUE;
	}

	return FALSE;
}

/***********************************************************************
**
*/	REBNATIVE(show)
/*
***********************************************************************/
{
#ifndef NO_GRAPHICS
	REBVAL *val = D_ARG(1);
	REBVAL *val2;
	REBVAL *head;
	REBINT dsp; // temporarily store it on stack

	if (IS_GOB(val)) OS_SHOW_GOB(VAL_GOB(val));
	else if (IS_BLOCK(val)) {
		dsp = DSP + 1;

		// Reduce all words and paths of the GOB type.
		// THIS STAYS ON THE STACK (because stack is not used)
		Reduce_Type_Stack(VAL_SERIES(val), VAL_INDEX(val), REB_GOB);

		head = DS_GET(dsp);

		// Optimize: remove any gobs that are in subgobs.
		// Set all gobs in block as showable:
		for (val = head; NOT_END(val); val++) {
			if (IS_GOB(val)) VAL_SET_OPT(val, OPTS_TEMP);
		}

		// Foreach gob in block, scan for it in all gobs:
		for (val = head; NOT_END(val); val++) {
			if (IS_GOB(val)
				&& GOB_PANE(VAL_GOB(val))
				&& VAL_GET_OPT(val, OPTS_TEMP)
			) {
				for (val2 = head; NOT_END(val2); val2++) {
					if (val != val2
						&& IS_GOB(val2)
						&& VAL_GET_OPT(val2, OPTS_TEMP)
						&& Find_Gob_Tree(VAL_GOB(val), VAL_GOB(val2))
					) {
						VAL_CLR_OPT(val2, OPTS_TEMP); // do not show it
						// break;  // Keep going, can be: show [A B C A] (rare)
					}
				}
			}
		}
		// Show those that are left:
		for (val = head; NOT_END(val); val++) {
			if (VAL_GET_OPT(val, OPTS_TEMP)) {
				OS_SHOW_GOB(VAL_GOB(val));
				VAL_CLR_OPT(val, OPTS_TEMP);
			}
//			else {
//				Print("No-show: %r", val);
//			}
		}
		DSP = dsp - 1; // reset stack
	}
	else OS_SHOW_GOB(0);
#endif

	return R_ARG1;
}


/***********************************************************************
**
*/	REBNATIVE (size_text)
/*
***********************************************************************/
{
#ifndef NO_GRAPHICS
	REBGOB *gob = VAL_GOB(D_ARG(1));
	REBPAR xy;

	if (!IS_GOB_TEXT(gob) && !IS_GOB_STRING(gob)) Trap_Arg(D_ARG(1)); //!!! better error

	OS_SIZE_TEXT(gob, &xy);

	VAL_SET(D_RET, REB_PAIR);
	VAL_PAIR(D_RET) = xy;
#endif
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE (caret_to_offset)
/*
***********************************************************************/
{
#ifndef NO_GRAPHICS
	REBGOB *gob = VAL_GOB(D_ARG(1));
	REBVAL *arg1 = D_ARG(2);
	REBVAL *arg2 = D_ARG(3);
	REBPAR xy;
	REBINT element;
	REBINT position;

	if (!IS_GOB_TEXT(gob) && !IS_GOB_STRING(gob)) Trap_Arg(D_ARG(1)); //!!! better error

	if (IS_INTEGER(arg1))
		element = Int32(arg1);
	else if (IS_BLOCK(arg1))
		element = VAL_INDEX(arg1);

	if (IS_INTEGER(arg2))
		position = Int32(arg2);
	else if (IS_STRING(arg2))
		position = VAL_INDEX(arg2);

	if (element < 0) Trap_Arg(arg1);
	if (position < 0) Trap_Arg(arg2);

	OS_CARET_TO_OFFSET(gob, &xy, element, position);

	VAL_SET(D_RET, REB_PAIR);
	VAL_PAIR(D_RET) = xy;
#endif
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE (offset_to_caret)
/*
***********************************************************************/
{
#ifndef NO_GRAPHICS
	REBGOB *gob = VAL_GOB(D_ARG(1));
	REBPAR xy = VAL_PAIR(D_ARG(2));
	REBINT element;
	REBINT position;
	REBSER *ser;
	REBVAL *val;

	if (!IS_GOB_TEXT(gob) && !IS_GOB_STRING(gob)) Trap_Arg(D_ARG(1)); //!!! better error

	OS_OFFSET_TO_CARET(gob, xy, &element, &position);

	if (IS_GOB_STRING(gob)) {
		Set_Series(REB_STRING, D_RET, GOB_CONTENT(gob));
		VAL_INDEX(D_RET) = position;
	} else {
		ser = Copy_Block(GOB_CONTENT(gob), 0);
		Set_Series(REB_BLOCK, D_RET, ser);
		if ((REBCNT)element < ser->tail) {
			VAL_INDEX(D_RET) = element;
			val = BLK_SKIP(ser, element);
			if (IS_WORD(val)) {
				val = Get_Var(val);
				*BLK_SKIP(ser, element) = *val;
				val = BLK_SKIP(ser, element);
			}
			if (ANY_STR(val)) {
				VAL_INDEX(val) = position;
				return R_RET;
			}
		}
		return R_NONE;
	}
#endif
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(draw)
/*
***********************************************************************/
{
#ifndef NO_GRAPHICS
	REBVAL *arg = D_ARG(1);
	REBVAL *image;
	REBINT err;
	REBSER *ser;

	if (IS_IMAGE(arg)) image = arg;
	else image = Make_Image(VAL_PAIR_X(arg), VAL_PAIR_Y(arg));

	*D_RET = *image;
	if (err = OS_DRAW_IMAGE(VAL_SERIES(image), ser = At_Head(D_ARG(2)))) {
		Trap_Word(RE_DIALECT, SYM_DRAW, BLK_SKIP(ser, (-err)-1));
	}
#endif
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(effect)
/*
***********************************************************************/
{
#ifndef NO_GRAPHICS
	REBVAL *arg = D_ARG(1);
	REBVAL *image;
	REBINT err;
	REBSER *ser;

	if (IS_IMAGE(arg)) image = arg;
	else image = Make_Image(VAL_PAIR_X(arg), VAL_PAIR_Y(arg));

	*D_RET = *image;
	if (err = OS_EFFECT_IMAGE(VAL_SERIES(image), ser = At_Head(D_ARG(2)))) {
		Trap_Word(RE_DIALECT, SYM_DRAW, BLK_SKIP(ser, (-err)-1));
	}
#endif
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(cursor)
/*
***********************************************************************/
{
#ifndef NO_GRAPHICS
	REBVAL *arg = D_ARG(1);
	REBINT n = 0;

	if (IS_INTEGER(arg)) n = Int32(arg);
	else if (IS_NONE(arg)) n = 0;
	else n = -1;

	OS_CURSOR_IMAGE(n, (n < 0) ? VAL_SERIES(arg) : 0);
#endif
	return R_UNSET;
}
#endif

/***********************************************************************
**
*/	void Trap_Image()
/*
***********************************************************************/
{
	Trap0(RE_BAD_MEDIA);
}
