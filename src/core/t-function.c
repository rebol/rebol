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
**  Module:  t-function.c
**  Summary: function related datatypes
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

static REBOOL Same_Func(REBVAL *val, REBVAL *arg)
{
	if (VAL_TYPE(val) == VAL_TYPE(arg) &&
		VAL_FUNC_SPEC(val) == VAL_FUNC_SPEC(arg) &&
		VAL_FUNC_ARGS(val) == VAL_FUNC_ARGS(arg) &&
		VAL_FUNC_CODE(val) == VAL_FUNC_CODE(arg)) return TRUE;
	return FALSE;
}


/***********************************************************************
**
*/	REBINT CT_Function(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	if (mode >= 0) return Same_Func(a, b);
	return -1;
}


/***********************************************************************
**
*/	REBSER *As_Typesets(REBSER *types)
/*
***********************************************************************/
{
	REBVAL *val;

	types = Copy_Block(types, 1);
	for (val = BLK_HEAD(types); NOT_END(val); val++) {
		SET_TYPE(val, REB_TYPESET);
	}
	return types;
}


/***********************************************************************
**
*/	REBFLG MT_Function(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	return Make_Function(type, out, data);
}


/***********************************************************************
**
*/	REBTYPE(Function)
/*
***********************************************************************/
{
	REBVAL *value = D_ARG(1);
	REBVAL *arg = D_ARG(2);
	REBCNT type = VAL_TYPE(value);
	REBCNT n;

	switch (action) {

	case A_MAKE:
	case A_TO:
		// make function! [[args] [body]]
		if (IS_DATATYPE(value)) {
			n = VAL_DATATYPE(value);
			if (Make_Function(n, value, arg)) break;
			Trap_Make(n, arg);
		}

		// make :func []
		// make :func [[args]]
		// make :func [* [body]]
		if (ANY_FUNC(value)) {
			if (!IS_BLOCK(arg)) goto bad_arg;
			if (!ANY_FUNC(value)) goto bad_arg;
			if (!Copy_Function(value, arg)) goto bad_arg;
			break;
		}
		if (!IS_NONE(arg)) goto bad_arg;
		// fall thru...
	case A_COPY:
		Copy_Function(value, 0);
		break;

	case A_REFLECT:
		n = What_Reflector(arg); // zero on error
		switch (n) {
		case OF_WORDS:
			//if (type == REB_CLOSURE)
			Set_Block(value, List_Func_Words(value));
			//else
			//	Set_Block(value, List_Func_Words(value));
			break;
		case OF_BODY:
of_type:
			switch (type) {
			case REB_FUNCTION:
			case REB_CLOSURE:
				Set_Block(value, Clone_Block(VAL_FUNC_BODY(value)));
				Unbind_Block(VAL_BLK(value), TRUE);
				break;
			case REB_NATIVE:
			case REB_COMMAND:
			case REB_ACTION:
				SET_NONE(value);
				break;
			case REB_OP:
				type = VAL_GET_EXT(value); // internal datatype
				goto of_type;
			}
			break;
		case OF_SPEC:
			Set_Block(value, Clone_Block(VAL_FUNC_SPEC(value)));
			Unbind_Block(VAL_BLK(value), TRUE);
			break;
		case OF_TYPES:
			Set_Block(value, As_Typesets(VAL_FUNC_ARGS(value)));
			break;
		case OF_TITLE:
			arg = BLK_HEAD(VAL_FUNC_SPEC(value));
			for (; NOT_END(arg) && !IS_STRING(arg) && !IS_WORD(arg); arg++);
			if (!IS_STRING(arg)) return R_NONE;
			Set_String(value, Copy_Series(VAL_SERIES(arg)));
			break;
		default:
		bad_arg:
			Trap_Reflect(type, arg);
		}
		break;

	default: Trap_Action(type, action);
	}

	DS_RET_VALUE(value);
	return R_RET;
}
