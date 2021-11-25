/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2021 Rebol Open Source Contributors
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
**  Module:  t-handle.c
**  Summary: handle datatype
**  Section: datatypes
**  Author:  Oldes
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

/***********************************************************************
**
*/	REBINT Cmp_Handle(REBVAL *a, REBVAL *b)
/*
***********************************************************************/
{
	REBYTE *sp;
	REBYTE *tp;
	if (IS_CONTEXT_HANDLE(a)) {
		if (IS_CONTEXT_HANDLE(b)) {
			if (VAL_HANDLE_SYM(a) != VAL_HANDLE_SYM(b)) {
				// comparing 2 context handles of different types
				sp = VAL_HANDLE_NAME(a);
				tp = VAL_HANDLE_NAME(b);
				return Compare_UTF8(sp, tp, (REBCNT)LEN_BYTES(tp)) + 2;
			}
		}
		else {
			// comparing context-handle with data-handle
			return -1;
		}
	}
	else if (IS_CONTEXT_HANDLE(b)) {
		// comparing data-handle with context-handle
		return 1;
	}
	// same ctx-handle types or both data handles
	return (VAL_HANDLE_I32(a) - VAL_HANDLE_I32(b));
}

/***********************************************************************
**
*/	REBINT CT_Handle(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	REBINT diff;
	if (mode > 0) {
		return ((VAL_HANDLE_FLAGS(a) == VAL_HANDLE_FLAGS(b))
			&&  (VAL_HANDLE_DATA(a)  == VAL_HANDLE_DATA(b)));
	}
	else if (mode == 0) {
		return (IS_CONTEXT_HANDLE(a) && IS_CONTEXT_HANDLE(b)
			&& (VAL_HANDLE_SYM(a) == VAL_HANDLE_SYM(b)));
	}
	else {
		diff = Cmp_Handle(a, b);
		if (mode == -1) return (diff >= 0);
		return (diff > 0);
	}
}


/***********************************************************************
**
*/	REBFLG MT_Handle(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	return FALSE;
}


/***********************************************************************
**
*/	REBINT PD_Handle(REBPVS *pvs)
/*
***********************************************************************/
{
	REBVAL *data = pvs->value;
	REBVAL *arg = pvs->select;
	REBVAL *val = pvs->setval;
	REBINT sym = 0;

	if (!IS_HANDLE(data)) return PE_BAD_ARGUMENT;
	if (!ANY_WORD(arg)) return PE_BAD_SELECT;

	sym = VAL_WORD_CANON(arg);

	if (val == 0) {
		// onle get-path is allowed for handles and only /type value so far
		if (sym != SYM_TYPE) return PE_BAD_SELECT;
		if (IS_CONTEXT_HANDLE(data)) {
			val = pvs->store;
			Set_Word(val, VAL_HANDLE_SYM(data), NULL, 0);
			return PE_USE;
		}
		// for the data handles, return NONE
		return PE_NONE;
	}
	else {
		// changing handle's type is not allowed
		return PE_BAD_SET;
	}
}


/***********************************************************************
**
*/	static REBOOL Query_Handle_Field(REBVAL *data, REBVAL *select, REBVAL *ret)
/*
**		Set a value with handle data according specified mode
**
***********************************************************************/
{
	REBPVS pvs;
	pvs.value = data;
	pvs.select = select;
	pvs.setval = 0;
	pvs.store = ret;

	return (PE_BAD_SELECT > PD_Handle(&pvs));
}


/***********************************************************************
**
*/	REBTYPE(Handle)
/*
**
***********************************************************************/
{
	REBVAL *val = D_ARG(1);
	REBVAL *spec;
	REBINT num;

	switch (action) {
	case A_REFLECT:
		*D_ARG(3) = *D_ARG(2);
		// continue..
	case A_QUERY:
		//TODO: this code could be made resusable with other types!
		spec = Get_System(SYS_STANDARD, STD_HANDLE_INFO);
		if (!IS_OBJECT(spec)) Trap_Arg(spec);
		if (D_REF(2)) { // query/mode refinement
			REBVAL *field = D_ARG(3);
			if (IS_WORD(field)) {
				switch (VAL_WORD_CANON(field)) {
				case SYM_WORDS:
					Set_Block(D_RET, Get_Object_Words(spec));
					return R_RET;
				case SYM_SPEC:
					return R_ARG1;
				}
				if (!Query_Handle_Field(val, field, D_RET))
					Trap_Reflect(VAL_TYPE(val), field); // better error?
			}
			else if (IS_BLOCK(field)) {
				REBVAL *out = D_RET;
				REBSER *values = Make_Block(2 * BLK_LEN(VAL_SERIES(field)));
				REBVAL *word = VAL_BLK_DATA(field);
				for (; NOT_END(word); word++) {
					if (ANY_WORD(word)) {
						if (IS_SET_WORD(word)) {
							// keep the set-word in result
							out = Append_Value(values);
							*out = *word;
							VAL_SET_LINE(out);
						}
						out = Append_Value(values);
						if (!Query_Handle_Field(val, word, out))
							Trap1(RE_INVALID_ARG, word);
					}
					else  Trap1(RE_INVALID_ARG, word);
				}
				Set_Series(REB_BLOCK, D_RET, values);
			}
			else {
				Set_Block(D_RET, Get_Object_Words(spec));
			}
		}
		else {
			REBSER *obj = CLONE_OBJECT(VAL_OBJ_FRAME(spec));
			REBSER *words = VAL_OBJ_WORDS(spec);
			REBVAL *word = BLK_HEAD(words);
			for (num = 0; NOT_END(word); word++, num++) {
				Query_Handle_Field(val, word, OFV(obj, num));
			}
			SET_OBJECT(D_RET, obj);
		}
		return R_RET;

	default:
		Trap_Action(VAL_TYPE(val), action);
	}

	return R_RET;
}
