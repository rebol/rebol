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
**  Module:  t-object.c
**  Summary: object datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"


static REBOOL Same_Object(REBVAL *val, REBVAL *arg)
{
	if (
		VAL_TYPE(arg) == VAL_TYPE(val) &&
		//VAL_OBJ_SPEC(val) == VAL_OBJ_SPEC(arg) &&
		VAL_OBJ_FRAME(val) == VAL_OBJ_FRAME(arg)
	) return TRUE;
	return FALSE;
}


static REBOOL Equal_Object(REBVAL *val, REBVAL *arg)
{
	REBSER *f1;
	REBSER *f2;
	REBSER *w1;
	REBSER *w2;
	REBINT n;

	if (VAL_TYPE(arg) != VAL_TYPE(val)) return FALSE;

	f1 = VAL_OBJ_FRAME(val);
	f2 = VAL_OBJ_FRAME(arg);
	if (f1 == f2) return TRUE;
	if (f1->tail != f2->tail) return FALSE;

	w1 = FRM_WORD_SERIES(f1);
	w2 = FRM_WORD_SERIES(f2);
	if (w1->tail != w2->tail) return FALSE;

	// Compare each entry:
	for (n = 1; n < (REBINT)(f1->tail); n++) {
		if (Cmp_Value(BLK_SKIP(w1, n), BLK_SKIP(w2, n), FALSE)) return FALSE;
		// Use Compare_Values();
		if (Cmp_Value(BLK_SKIP(f1, n), BLK_SKIP(f2, n), FALSE)) return FALSE;
	}

	return TRUE;
}

static void Append_Obj(REBSER *obj, REBVAL *arg)
{
	REBCNT i;
	REBCNT len = 0;
	REBVAL *val;
	REBVAL *start = arg;

	// Can be a word:
	if (ANY_WORD(arg)) {
		if (!Find_Word_Index(obj, VAL_WORD_SYM(arg), TRUE)) {
			if (VAL_WORD_CANON(arg) == SYM_SELF) Trap0(RE_SELF_PROTECTED);
			Expand_Frame(obj, 1, 1); // copy word table also
			Append_Frame(obj, 0, VAL_WORD_SYM(arg));
			// val is UNSET
		}
		return;
	}

	if (!IS_BLOCK(arg)) Trap_Arg(arg);

	// Verify word/value argument block:
	for (arg = VAL_BLK_DATA(arg); NOT_END(arg); arg += 2) {

		if (!IS_WORD(arg) && !IS_SET_WORD(arg)) Trap_Arg(arg);

		if (NZ(i = Find_Word_Index(obj, VAL_WORD_SYM(arg), TRUE))) {
			// Just change the value, do not append it.
			val = FRM_VALUE(obj, i);
			if (GET_FLAGS(VAL_OPTS(FRM_WORD(obj, i)), OPTS_HIDE, OPTS_LOCK)) { 
				// Back out... reset any prior flags:
				for (; arg != VAL_BLK_DATA(start); arg -= 2) VAL_CLR_OPT(arg, OPTS_TEMP);
				if (VAL_PROTECTED(FRM_WORD(obj, i))) Trap1(RE_LOCKED_WORD, FRM_WORD(obj, i));
				Trap0(RE_HIDDEN);
			}
			// Problem above: what about prior OPTS_FLAGS? Ok to leave them as is?
			if (IS_END(arg+1)) SET_NONE(val);
			else *val = arg[1];
			VAL_SET_OPT(arg, OPTS_TEMP);
		} else {
			if (VAL_WORD_CANON(arg) == SYM_SELF) Trap0(RE_SELF_PROTECTED);
			len++;
			// was: Trap1(RE_DUP_VARS, arg);
		}
	
		if (IS_END(arg+1)) break; // fix bug#708
	}

	// Append new values to end of frame (if necessary):
	if (len > 0) {
		Expand_Frame(obj, len, 1); // copy word table also
		for (arg = VAL_BLK_DATA(start); NOT_END(arg); arg += 2) {
			if (VAL_GET_OPT(arg, OPTS_TEMP)) VAL_CLR_OPT(arg, OPTS_TEMP);
			else {
				val = Append_Frame(obj, 0, VAL_WORD_SYM(arg));
				if (IS_END(arg+1)) {
					SET_NONE(val);
					break;
				}
				else *val = arg[1];
			}
		}
	}
}

static REBSER *Trim_Object(REBSER *obj)
{
	REBVAL *val;
	REBINT cnt = 0;
	REBSER *nobj;
	REBVAL *nval;
	REBVAL *word;
	REBVAL *nwrd;

	word = FRM_WORDS(obj)+1;
	for (val = FRM_VALUES(obj)+1; NOT_END(val); val++, word++) {
		if (VAL_TYPE(val) > REB_NONE && !VAL_GET_OPT(word, OPTS_HIDE))
			cnt++;
	}

	nobj = Make_Frame(cnt);
	nval = FRM_VALUES(nobj)+1;
	word = FRM_WORDS(obj)+1;
	nwrd = FRM_WORDS(nobj)+1;
	for (val = FRM_VALUES(obj)+1; NOT_END(val); val++, word++) {
		if (VAL_TYPE(val) > REB_NONE && !VAL_GET_OPT(word, OPTS_HIDE)) {
			*nval++ = *val;
			*nwrd++ = *word;
		}
	}
	SET_END(nval);
	SET_END(nwrd);
	SERIES_TAIL(nobj) = cnt+1;
	SERIES_TAIL(FRM_WORD_SERIES(nobj)) = cnt+1;

	return nobj;
}


/***********************************************************************
**
*/	REBINT CT_Object(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	if (mode < 0) return -1;
	if (mode == 3) return Same_Object(a, b);
	return Equal_Object(a, b);
}


/***********************************************************************
**
*/	REBINT CT_Frame(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	if (mode < 0) return -1;
	return VAL_SERIES(a) == VAL_SERIES(b);
}



/***********************************************************************
**
*/	REBFLG MT_Object(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	if (!IS_BLOCK(data)) return FALSE;
	VAL_OBJ_FRAME(out) = Construct_Object(0, VAL_BLK_DATA(data), 0);
	VAL_SET(out, type);
	if (type == REB_ERROR) {
		Make_Error_Object(out, out);
	}
	return TRUE;
}


/***********************************************************************
**
*/	REBINT PD_Object(REBPVS *pvs)
/*
***********************************************************************/
{
	REBINT n = 0;

	if (!VAL_OBJ_FRAME(pvs->value)) {
		return PE_NONE; // Error objects may not have a frame.
	}

	if (IS_WORD(pvs->select)) {
		n = Find_Word_Index(VAL_OBJ_FRAME(pvs->value), VAL_WORD_SYM(pvs->select), FALSE);
	}
//	else if (IS_INTEGER(pvs->select)) {
//		n = Int32s(pvs->select, 1);
//	}
	else return PE_BAD_SELECT;

	if (n <= 0 || (REBCNT)n >= SERIES_TAIL(VAL_OBJ_FRAME(pvs->value)))
		return PE_BAD_SELECT;

	if (pvs->setval && IS_END(pvs->path+1) && VAL_PROTECTED(VAL_FRM_WORD(pvs->value, n)))
		Trap1(RE_LOCKED_WORD, pvs->select);

	pvs->value = VAL_OBJ_VALUES(pvs->value) + n;
	return PE_SET;
	// if setval, check PROTECT mode!!!
	// VAL_FLAGS((VAL_OBJ_VALUES(value) + n)) &= ~FLAGS_CLEAN;
}


/***********************************************************************
**
*/	REBTYPE(Object)
/*
**		Handles object! and error! datatypes.
**
***********************************************************************/
{
	REBVAL *value = D_ARG(1);
	REBVAL *arg = D_ARG(2);
	REBINT n;
	REBVAL *val;
	REBSER *obj, *src_obj;
	REBCNT type = 0;

	switch (action) {

	case A_MAKE:
		// make object! | error! | module! | task!
		if (IS_DATATYPE(value)) {

			type = VAL_DATATYPE(value); // target type

			if (IS_BLOCK(arg)) {

				// make object! [init]
				if (type == REB_OBJECT) {
					obj = Make_Object(0, VAL_BLK_DATA(arg));
					SET_OBJECT(ds, obj); // GC save
					arg = Do_Bind_Block(obj, arg); // GC-OK
					if (THROWN(arg)) {
						DS_RET_VALUE(arg);
						return R_RET;
					}
					break; // returns obj
				}

				if (type == REB_MODULE) {
					*value = *Make_Module(arg);
					type = 0;
				//	VAL_MOD_BODY(value) = VAL_SERIES(arg);
				//	VAL_SET(value, REB_MODULE); // GC protected
				//	DO_BLK(arg);
					break; // returns value
				}

				// make task! [init]
				if (type == REB_TASK) {
					// Does it include a spec?
					if (IS_BLOCK(VAL_BLK(arg))) {
						arg = VAL_BLK(arg);
						if (!IS_BLOCK(arg+1)) Trap_Make(REB_TASK, value);
						obj = Make_Module_Spec(arg);
						VAL_MOD_BODY(value) = VAL_SERIES(arg+1);
					} else {
						obj = Make_Module_Spec(0);
						VAL_MOD_BODY(value) = VAL_SERIES(arg);
					}
					break; // returns obj
				}
			}

			// make error! [....]
			if (type == REB_ERROR) {
				Make_Error_Object(arg, value); // arg is block/string, returns value
				type = 0;
				break; // returns value
			}

			// make object! 10
			if (IS_NUMBER(arg)) {
				n = Int32s(arg, 0);
				obj = Make_Frame(n);
				break; // returns obj
			}

			// make object! map!
			if (IS_MAP(arg)) {
				obj = Map_To_Object(VAL_SERIES(arg));
				break; // returns obj
			}

			//if (IS_NONE(arg)) {obj = Make_Frame(0); break;}

			Trap_Make(type, arg);
		}

		// make parent-object ....
		if (IS_OBJECT(value)) {
			type = REB_OBJECT;
			src_obj  = VAL_OBJ_FRAME(value);

			// make parent none | []
			if (IS_NONE(arg) || (IS_BLOCK(arg) && IS_EMPTY(arg))) {
				obj = Copy_Block_Values(src_obj, 0, SERIES_TAIL(src_obj), TS_CLONE);
				Rebind_Frame(src_obj, obj);
				break;	// returns obj
			}

			// make parent [...]
			if (IS_BLOCK(arg)) {
				obj = Make_Object(src_obj, VAL_BLK_DATA(arg));
				Rebind_Frame(src_obj, obj);
				SET_OBJECT(ds, obj);
				arg = Do_Bind_Block(obj, arg); // GC-OK
				if (THROWN(arg)) {
					DS_RET_VALUE(arg);
					return R_RET;
				}
				break; // returns obj
			}

			// make parent-object object
			if (IS_OBJECT(arg)) {
				obj = Merge_Frames(src_obj, VAL_OBJ_FRAME(arg));
				break; // returns obj
			}
		}
		Trap_Make(VAL_TYPE(value), value);

	case A_TO:
		// special conversions to object! | error! | module!
		if (IS_DATATYPE(value)) {
			type = VAL_DATATYPE(value);
			if (type == REB_ERROR) {
				Make_Error_Object(arg, value); // arg is block/string, returns value
				type = 0; // type already set
				break; // returns value
			}
			else if (type == REB_OBJECT) {
				if (IS_ERROR(arg)) {
					if (VAL_ERR_NUM(arg) < 100) Trap_Arg(arg);
					obj = VAL_ERR_OBJECT(arg);
					break; // returns obj
				}
			}
			else if (type == REB_MODULE) {
				if (!IS_BLOCK(arg) || IS_EMPTY(arg)) Trap_Make(REB_MODULE, arg);
				val = VAL_BLK_DATA(arg); // module spec
				if (!IS_OBJECT(val)) Trap_Arg(val);
				obj = VAL_OBJ_FRAME(val);
				val++; // module object
				if (!IS_OBJECT(val)) Trap_Arg(val);
				VAL_MOD_SPEC(val) = obj;
				*value = *val;
				VAL_SET(value, REB_MODULE);
				type = 0; // type already set
				break; // returns value
			}
		}
		else type = VAL_TYPE(value);
		Trap_Make(type, arg);

	case A_APPEND:
		TRAP_PROTECT(VAL_SERIES(value));
		if (IS_OBJECT(value)) {
			Append_Obj(VAL_OBJ_FRAME(value), arg);
			return R_ARG1;
		}
		else
			Trap_Action(VAL_TYPE(value), action); // !!! needs better error

	case A_LENGTHQ:
		if (IS_OBJECT(value)) {
			DS_RET_INT(SERIES_TAIL(VAL_OBJ_FRAME(value))-1);
			return R_RET;
		}
		Trap_Action(VAL_TYPE(value), action);

	case A_COPY:
		// Note: words are not copied and bindings not changed!
	{
		REBU64 types = 0;
		if (D_REF(ARG_COPY_PART)) Trap0(RE_BAD_REFINES);
		if (D_REF(ARG_COPY_DEEP)) {
			types |= CP_DEEP | (D_REF(ARG_COPY_TYPES) ? 0 : TS_STD_SERIES);
		}
		if D_REF(ARG_COPY_TYPES) {
			arg = D_ARG(ARG_COPY_KINDS);
			if (IS_DATATYPE(arg)) types |= TYPESET(VAL_DATATYPE(arg));
			else types |= VAL_TYPESET(arg);
		}
		VAL_OBJ_FRAME(value) = obj = Copy_Block(VAL_OBJ_FRAME(value), 0);
		if (types != 0) Copy_Deep_Values(obj, 1, SERIES_TAIL(obj), types);
		break; // returns value
	}
	case A_SELECT:
	case A_FIND:
		n = 0;
		if (IS_WORD(arg))
			n = Find_Word_Index(VAL_OBJ_FRAME(value), VAL_WORD_SYM(arg), FALSE);

		if (n <= 0 || (REBCNT)n >= SERIES_TAIL(VAL_OBJ_FRAME(value)))
			return R_NONE;

		if (action == A_FIND) goto is_true;

		value = VAL_OBJ_VALUES(value) + n;
		break;

	case A_REFLECT:
		action = What_Reflector(arg); // zero on error
		if (action == OF_SPEC) {
			if (!VAL_MOD_SPEC(value)) return R_NONE;
			VAL_OBJ_FRAME(value) = VAL_MOD_SPEC(value);
			VAL_SET(value, REB_OBJECT);
			break;
		}
		// Adjust for compatibility with PICK:
		if (action == OF_VALUES) action = 2;
		else if (action == OF_BODY) action = 3;
		if (action < 1 || action > 3) Trap_Reflect(VAL_TYPE(value), arg);
#ifdef obsolete
		goto reflect;

	case A_PICK:
		action = Get_Num_Arg(arg); // integer, decimal, logic
		if (action < 1 || action > 3) Trap_Arg(arg);
		if (action < 3) action |= 4;  // add SELF to list
reflect:
#endif
		if (THROWN(value)) Trap0(RE_THROW_USAGE);
		Set_Block(value, Make_Object_Block(VAL_OBJ_FRAME(value), action));
		break;

	case A_TRIM:
		if (Find_Refines(ds, ALL_TRIM_REFS)) Trap0(RE_BAD_REFINES); // none allowed
		type = VAL_TYPE(value);
		obj = Trim_Object(VAL_OBJ_FRAME(value));
		break;

	case A_TAILQ:
		if (IS_OBJECT(value)) {
			SET_LOGIC(DS_RETURN, SERIES_TAIL(VAL_OBJ_FRAME(value)) <= 1);
			return R_RET;
		}
		Trap_Action(VAL_TYPE(value), action);

	default:
		Trap_Action(VAL_TYPE(value), action);
	}

	if (type) {
		VAL_SET(value, type);
		VAL_OBJ_FRAME(value) = obj;
	}

	DS_RET_VALUE(value);
	return R_RET;

is_true:
	return R_TRUE;
}


/***********************************************************************
**
*/	REBINT PD_Frame(REBPVS *pvs)
/*
**		pvs->value points to the first value in frame (SELF).
**
***********************************************************************/
{
	REBCNT sym;
	REBCNT s;
	REBVAL *word;
	REBVAL *val;

	if (IS_WORD(pvs->select)) {
		sym = VAL_WORD_SYM(pvs->select);
		s = SYMBOL_TO_CANON(sym);
		word = BLK_SKIP(VAL_FRM_WORDS(pvs->value), 1);
		for (val = pvs->value + 1; NOT_END(val); val++, word++) {
			if (sym == VAL_BIND_SYM(word) || s == VAL_BIND_CANON(word)) {
				if (VAL_GET_OPT(word, OPTS_HIDE)) break;
				if (VAL_PROTECTED(word)) Trap1(RE_LOCKED_WORD, word);
				pvs->value = val;
				return PE_SET;
			}
		}
	}
	return PE_BAD_SELECT;
}


/***********************************************************************
**
*/	REBTYPE(Frame)
/*
***********************************************************************/
{
	switch (action) {
	case A_MAKE:
	case A_TO:
		Trap_Make(REB_FRAME, D_ARG(2));
	}

	return R_ARG1;
}


#ifdef later
/***********************************************************************
**
**	Get_Obj_Mods -- return a block of modified words from an object
**
***********************************************************************/
REBVAL *Get_Obj_Mods(REBFRM *frame, REBVAL **inter_block)
{
	REBVAL *obj  = D_ARG(1);
	REBVAL *words, *val;
	REBFRM *frm  = VAL_OBJ_FRAME(obj);
	REBSER *ser  = Make_Block(2);
	REBOOL clear = D_REF(2);
	//DISABLE_GC;

	val   = BLK_HEAD(frm->values);
	words = BLK_HEAD(frm->words);
	for (; NOT_END(val); val++, words++)
		if (!(VAL_FLAGS(val) & FLAGS_CLEAN)) {
			Append_Val(ser, words);
			if (clear) VAL_FLAGS(val) |= FLAGS_CLEAN;
		}
	if (!STR_LEN(ser)) {
		ENABLE_GC;
		goto is_none;
	}

	Bind_Block(frm, BLK_HEAD(ser), FALSE);
	VAL_SERIES(Temp_Blk_Value) = ser;
	//ENABLE_GC;
	return Temp_Blk_Value;
}
#endif
