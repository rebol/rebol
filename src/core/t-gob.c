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
**  Module:  t-gob.c
**  Summary: graphical object datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

void Trap_Temp(void) {Trap0(501);} //!!! temp trap function

const REBCNT Gob_Flag_Words[] = {
	SYM_RESIZE,      GOBF_RESIZE,
	SYM_NO_TITLE,    GOBF_NO_TITLE,
	SYM_NO_BORDER,   GOBF_NO_BORDER,
	SYM_DROPABLE,    GOBF_DROPABLE,
	SYM_TRANSPARENT, GOBF_TRANSPARENT,
	SYM_POPUP,       GOBF_POPUP,
	SYM_MODAL,       GOBF_MODAL,
	SYM_ON_TOP,      GOBF_ON_TOP,
	SYM_HIDDEN,      GOBF_HIDDEN,
	0, 0
};


/***********************************************************************
**
*/	REBINT CT_Gob(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	if (mode >= 0)
		return VAL_GOB(a) == VAL_GOB(b) && VAL_GOB_INDEX(a) == VAL_GOB_INDEX(b);
	return -1;
}

/***********************************************************************
**
*/  REBGOB *Make_Gob(void)
/*
**      Allocate a new GOB.
**
***********************************************************************/
{
	REBGOB *gob = Make_Node(GOB_POOL);
	CLEAR(gob, sizeof(REBGOB));
	GOB_W(gob) = 100;
	GOB_H(gob) = 100;
	USE_GOB(gob);
	if ((GC_Ballast -= Mem_Pools[GOB_POOL].wide) <= 0) SET_SIGNAL(SIG_RECYCLE);
	return gob;
}


/***********************************************************************
**
*/  REBINT Cmp_Gob(REBVAL *g1, REBVAL *g2)
/*
***********************************************************************/
{
	REBINT n;

	n = VAL_GOB(g2) - VAL_GOB(g1);
	if (n != 0) return n;
	n = VAL_GOB_INDEX(g2) - VAL_GOB_INDEX(g1);
	if (n != 0) return n;
	return 0;
}


/***********************************************************************
**
*/	static REBFLG Set_Pair(REBXYF *pair, REBVAL *val)
/*
***********************************************************************/
{
	if (IS_PAIR(val)) {
		pair->x = VAL_PAIR_X(val);
		pair->y = VAL_PAIR_Y(val);
	}
	else if (IS_INTEGER(val)) {
		pair->x = pair->y = (REBD32)VAL_INT64(val);
	}
	else if (IS_DECIMAL(val)) {
		pair->x = pair->y = (REBD32)VAL_DECIMAL(val);
	}
	else
		return FALSE;

	return TRUE;
}


/***********************************************************************
**
*/	static REBCNT Find_Gob(REBGOB *gob, REBGOB *target)
/*
**		Find a target GOB within the pane of another gob.
**		Return the index, or a -1 if not found.
**
***********************************************************************/
{
	REBCNT len;
	REBCNT n;
	REBGOB **ptr;

	if (GOB_PANE(gob)) {
		len = GOB_TAIL(gob);
		ptr = GOB_HEAD(gob);
		for (n = 0; n < len; n++, ptr++)
			if (*ptr == target) return n;
	}
	return NOT_FOUND;
}


/***********************************************************************
**
*/	static void Detach_Gob(REBGOB *gob)
/*
**		Remove a gob value from its parent.
**		Done normally in advance of inserting gobs into new parent.
**
***********************************************************************/
{
	REBGOB *par;
	REBINT i;

	par = GOB_PARENT(gob);
	if (par && GOB_PANE(par) && (i = Find_Gob(par, gob)) != NOT_FOUND) {
		Remove_Series(GOB_PANE(par), i, 1);
	}
	GOB_PARENT(gob) = 0;
}


/***********************************************************************
**
*/	static void Insert_Gobs(REBGOB *gob, REBVAL *arg, REBCNT index, REBCNT len, REBFLG change)
/*
**		Insert one or more gobs into a pane at the given index.
**		If index >= tail, an append occurs. Each gob has its parent
**		gob field set. (Call Detach_Gobs() before inserting.)
**
***********************************************************************/
{
	REBGOB **ptr;
	REBCNT n, count;
	REBVAL *val, *sarg;
	REBINT i;

	// Verify they are gobs:
	sarg = arg;
	for (n = count = 0; n < len; n++, val++) {
		val = arg++;
		if (IS_WORD(val)) val = Get_Var(val);
		if (IS_GOB(val)) {
			count++;
			if (GOB_PARENT(VAL_GOB(val))) {
				// Check if inserting into same parent:
				i = -1;
				if (GOB_PARENT(VAL_GOB(val)) == gob) {
					i = Find_Gob(gob, VAL_GOB(val));
					if (i > 0 && i == (REBINT)index-1) { // a no-op
						SET_GOB_STATE(VAL_GOB(val), GOBS_NEW);
						return;
					}
				}
				Detach_Gob(VAL_GOB(val));
				if ((REBINT)index > i) index--;
			}
		}
	}
	arg = sarg;

	// Create or expand the pane series:
	if (!GOB_PANE(gob)) {
		GOB_PANE(gob) = Make_Series(count, sizeof(REBGOB*), 0);
		LABEL_SERIES(GOB_PANE(gob), "gob pane");
		GOB_TAIL(gob) = count;
		index = 0;
	}
	else {
		if (change) {
			if (index + count > GOB_TAIL(gob)) {
				EXPAND_SERIES_TAIL(GOB_PANE(gob), index + count - GOB_TAIL(gob));
			}
		} else {
			Expand_Series(GOB_PANE(gob), index, count);
			if (index >= GOB_TAIL(gob)) index = GOB_TAIL(gob)-1;
		}
	}

	ptr = GOB_SKIP(gob, index);
	for (n = 0; n < len; n++) {
		val = arg++;
		if (IS_WORD(val)) val = Get_Var(val);
		if (IS_GOB(val)) {
			if GOB_PARENT(VAL_GOB(val)) Trap_Temp();
			*ptr++ = VAL_GOB(val);
			GOB_PARENT(VAL_GOB(val)) = gob;
			SET_GOB_STATE(VAL_GOB(val), GOBS_NEW);
		}
	}
}


/***********************************************************************
**
*/	static void Remove_Gobs(REBGOB *gob, REBCNT index, REBCNT len)
/*
**		Remove one or more gobs from a pane at the given index.
**
***********************************************************************/
{
	REBGOB **ptr;
	REBCNT n;

	ptr = GOB_SKIP(gob, index);
	for (n = 0; n < len; n++, ptr++) {
		GOB_PARENT(*ptr) = 0;
	}

	Remove_Series(GOB_PANE(gob), index, len);
}


/***********************************************************************
**
*/	static REBSER *Pane_To_Block(REBGOB *gob, REBCNT index, REBINT len)
/*
**		Convert pane list of gob pointers to a block of GOB!s.
**
***********************************************************************/
{
	REBSER *ser;
	REBGOB **gp;
	REBVAL *val;

	if (len == -1 || (len + index) > GOB_TAIL(gob)) len = GOB_TAIL(gob) - index;
	if (len < 0) len = 0;

	ser = Make_Block(len);
	ser->tail = len;
	val = BLK_HEAD(ser);
	gp = GOB_HEAD(gob);
	for (; len > 0; len--, val++, gp++) {
		SET_GOB(val, *gp);
	}
	SET_END(val);

	return ser;
}


/***********************************************************************
**
*/	static REBSER *Flags_To_Block(REBGOB *gob)
/*
***********************************************************************/
{
	REBSER *ser;
	REBVAL *val;
	REBINT i;

	ser = Make_Block(3);

	for (i = 0; Gob_Flag_Words[i]; i += 2) {
		if (GET_GOB_FLAG(gob, Gob_Flag_Words[i+1])) {
			val = Append_Value(ser);
			Init_Word(val, Gob_Flag_Words[i]);
		}
	}

	return ser;
}


/***********************************************************************
**
*/	static void Set_Gob_Flag(REBGOB *gob, REBVAL *word)
/*
***********************************************************************/
{
	REBINT i;

	for (i = 0; Gob_Flag_Words[i]; i += 2) {
		if (VAL_WORD_CANON(word) == Gob_Flag_Words[i]) {
			SET_GOB_FLAG(gob, Gob_Flag_Words[i+1]);
			break;
		}
	}
}


/***********************************************************************
**
*/	static REBFLG Set_GOB_Var(REBGOB *gob, REBVAL *word, REBVAL *val)
/*
***********************************************************************/
{
	switch (VAL_WORD_CANON(word)) {
	case SYM_OFFSET:
		return Set_Pair(&(gob->offset), val);

	case SYM_SIZE:
		return Set_Pair(&gob->size, val);

	case SYM_IMAGE:
		CLR_GOB_OPAQUE(gob);
		if (IS_IMAGE(val)) {
			SET_GOB_TYPE(gob, GOBT_IMAGE);
			GOB_W(gob) = (REBD32)VAL_IMAGE_WIDE(val);
			GOB_H(gob) = (REBD32)VAL_IMAGE_HIGH(val);
			GOB_CONTENT(gob) = VAL_SERIES(val);
//			if (!VAL_IMAGE_TRANSP(val)) SET_GOB_OPAQUE(gob);
		}
		else if (IS_NONE(val)) SET_GOB_TYPE(gob, GOBT_NONE);
		else return FALSE;
		break;

	case SYM_DRAW:
		CLR_GOB_OPAQUE(gob);
		if (IS_BLOCK(val)) {
			SET_GOB_TYPE(gob, GOBT_DRAW);
			GOB_CONTENT(gob) = VAL_SERIES(val);
		}
		else if (IS_NONE(val)) SET_GOB_TYPE(gob, GOBT_NONE);
		else return FALSE;
		break;

	case SYM_TEXT:
		CLR_GOB_OPAQUE(gob);
		if (IS_BLOCK(val)) {
			SET_GOB_TYPE(gob, GOBT_TEXT);
			GOB_CONTENT(gob) = VAL_SERIES(val);
		}
		else if (IS_STRING(val)) {
			SET_GOB_TYPE(gob, GOBT_STRING);
			GOB_CONTENT(gob) = VAL_SERIES(val);
		}
		else if (IS_NONE(val)) SET_GOB_TYPE(gob, GOBT_NONE);
		else return FALSE;
		break;

	case SYM_EFFECT:
		CLR_GOB_OPAQUE(gob);
		if (IS_BLOCK(val)) {
			SET_GOB_TYPE(gob, GOBT_EFFECT);
			GOB_CONTENT(gob) = VAL_SERIES(val);
		}
		else if (IS_NONE(val)) SET_GOB_TYPE(gob, GOBT_NONE);
		else return FALSE;
		break;

	case SYM_COLOR:
		CLR_GOB_OPAQUE(gob);
		if (IS_TUPLE(val)) {
			SET_GOB_TYPE(gob, GOBT_COLOR);
			Set_Pixel_Tuple((REBYTE*)&GOB_CONTENT(gob), val);
			if (VAL_TUPLE_LEN(val) < 4 || VAL_TUPLE(val)[3] == 0)
				SET_GOB_OPAQUE(gob);
		}
		else if (IS_NONE(val)) SET_GOB_TYPE(gob, GOBT_NONE);
		break;

	case SYM_PANE:
		if (GOB_PANE(gob)) Clear_Series(GOB_PANE(gob));
		if (IS_BLOCK(val))
			Insert_Gobs(gob, VAL_BLK_DATA(val), 0, VAL_BLK_LEN(val), 0);
		else if (IS_GOB(val))
			Insert_Gobs(gob, val, 0, 1, 0);
		else if (IS_NONE(val))
			gob->pane = 0;
		else
			return FALSE;
		break;

	case SYM_ALPHA:
		GOB_ALPHA(gob) = Clip_Int(Int32(val), 0, 255);
		break;

	case SYM_DATA:
		SET_GOB_DTYPE(gob, GOBD_NONE);
		if (IS_OBJECT(val)) {
			SET_GOB_DTYPE(gob, GOBD_OBJECT);
			SET_GOB_DATA(gob, VAL_OBJ_FRAME(val));
		}
		else if (IS_BLOCK(val)) {
			SET_GOB_DTYPE(gob, GOBD_BLOCK);
			SET_GOB_DATA(gob, VAL_SERIES(val));
		}
		else if (IS_STRING(val)) {
			SET_GOB_DTYPE(gob, GOBD_STRING);
			SET_GOB_DATA(gob, VAL_SERIES(val));
		}
		else if (IS_BINARY(val)) {
			SET_GOB_DTYPE(gob, GOBD_BINARY);
			SET_GOB_DATA(gob, VAL_SERIES(val));
		}
		else if (IS_INTEGER(val)) {
			SET_GOB_DTYPE(gob, GOBD_INTEGER);
			SET_GOB_DATA(gob, (void*)VAL_INT32(val));
		}
		else if (IS_NONE(val))
			SET_GOB_TYPE(gob, GOBT_NONE);
		else return FALSE;
		break;

	case SYM_FLAGS:
		if (IS_WORD(val)) Set_Gob_Flag(gob, val);
		else if (IS_BLOCK(val)) {
			gob->flags = 0;
			for (val = VAL_BLK(val); NOT_END(val); val++) {
				if (IS_WORD(val)) Set_Gob_Flag(gob, val);
			}
		}
		break;

	case SYM_OWNER:
		if (IS_GOB(val))
			GOB_TMP_OWNER(gob) = VAL_GOB(val);
		else
			return FALSE;
		break;

	default:
			return FALSE;
	}
	return TRUE;
}


/***********************************************************************
**
*/	static REBFLG Get_GOB_Var(REBGOB *gob, REBVAL *word, REBVAL *val)
/*
***********************************************************************/
{
	switch (VAL_WORD_CANON(word)) {

	case SYM_OFFSET:
		SET_PAIR(val, GOB_X(gob), GOB_Y(gob));
		break;

	case SYM_SIZE:
		SET_PAIR(val, GOB_W(gob), GOB_H(gob));
		break;

	case SYM_IMAGE:
		if (GOB_TYPE(gob) == GOBT_IMAGE) {
			// image
		}
		else goto is_none;
		break;

	case SYM_DRAW:
		if (GOB_TYPE(gob) == GOBT_DRAW) {
			Set_Block(val, GOB_CONTENT(gob)); // Note: compiler optimizes SET_BLOCKs below
		}
		else goto is_none;
		break;

	case SYM_TEXT:
		if (GOB_TYPE(gob) == GOBT_TEXT) {
			Set_Block(val, GOB_CONTENT(gob));
		}
		else if (GOB_TYPE(gob) == GOBT_STRING) {
			Set_String(val, GOB_CONTENT(gob));
		}
		else goto is_none;
		break;

	case SYM_EFFECT:
		if (GOB_TYPE(gob) == GOBT_EFFECT) {
			Set_Block(val, GOB_CONTENT(gob));
		}
		else goto is_none;
		break;

	case SYM_COLOR:
		if (GOB_TYPE(gob) == GOBT_COLOR) {
			Set_Tuple_Pixel((REBYTE*)&GOB_CONTENT(gob), val);
		}
		else goto is_none;
		break;

	case SYM_ALPHA:
		SET_INTEGER(val, GOB_ALPHA(gob));
		break;

	case SYM_PANE:
		if (GOB_PANE(gob))
			Set_Block(val, Pane_To_Block(gob, 0, -1));
		else
			Set_Block(val, Make_Block(0));
		break;

	case SYM_PARENT:
		if (GOB_PARENT(gob)) {
			SET_GOB(val, GOB_PARENT(gob));
		}
		else
is_none:
			SET_NONE(val);
		break;

	case SYM_DATA:
		if (GOB_DTYPE(gob) == GOBD_OBJECT) {
			SET_OBJECT(val, GOB_DATA(gob));
		}
		else if (GOB_DTYPE(gob) == GOBD_BLOCK) {
			Set_Block(val, GOB_DATA(gob));
		}
		else if (GOB_DTYPE(gob) == GOBD_STRING) {
			Set_String(val, GOB_DATA(gob));
		}
		else if (GOB_DTYPE(gob) == GOBD_BINARY) {
			SET_BINARY(val, GOB_DATA(gob));
		}
		else if (GOB_DTYPE(gob) == GOBD_INTEGER) {
			SET_INTEGER(val, (REBINT)GOB_DATA(gob));
		}
		else goto is_none;
		break;

	case SYM_FLAGS:
		Set_Block(val, Flags_To_Block(gob));
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/***********************************************************************
**
*/	static void Set_GOB_Vars(REBGOB *gob, REBVAL *blk)
/*
***********************************************************************/
{
	REBVAL *var;
	REBVAL *val;

	while (NOT_END(blk)) {
		var = blk++;
		val = blk++;
		if (!IS_SET_WORD(var)) Trap2(RE_EXPECT_VAL, Get_Type(REB_SET_WORD), Of_Type(var));
		if (IS_END(val) || IS_UNSET(val) || IS_SET_WORD(val))
			Trap1(RE_NEED_VALUE, var);
		val = Get_Simple_Value(val);
		if (!Set_GOB_Var(gob, var, val)) Trap2(RE_BAD_FIELD_SET, var, Of_Type(val));
	}
}


/***********************************************************************
**
*/	REBSER *Gob_To_Block(REBGOB *gob)
/*
**		Used by MOLD to create a block.
**
***********************************************************************/
{
	REBSER *ser = Make_Block(10);
	REBVAL *val;
	REBINT words[6] = {SYM_OFFSET, SYM_SIZE, SYM_ALPHA, 0};
	REBVAL *vals[6];
	REBINT n = 0;
	REBVAL *val1;
	REBCNT sym;

	for (n = 0; words[n]; n++) {
		val = Append_Value(ser);
		Init_Word(val, words[n]);
		VAL_SET(val, REB_SET_WORD);
		vals[n] = Append_Value(ser);
	}

	SET_PAIR(vals[0], GOB_X(gob), GOB_Y(gob));
	SET_PAIR(vals[1], GOB_W(gob), GOB_H(gob));
	SET_INTEGER(vals[2], GOB_ALPHA(gob));

	if (!GOB_TYPE(gob)) return ser;

	if (GOB_CONTENT(gob)) {
		val1 = Append_Value(ser);
		val = Append_Value(ser);
		switch (GOB_TYPE(gob)) {
		case GOBT_COLOR:
			sym = SYM_COLOR;
			break;
		case GOBT_IMAGE:
			sym = SYM_IMAGE;
			break;
		case GOBT_STRING:
		case GOBT_TEXT:
			sym = SYM_TEXT;
			break;
		case GOBT_DRAW:
			sym = SYM_DRAW;
			break;
		case GOBT_EFFECT:
			sym = SYM_EFFECT;
			break;
		}
		Init_Word(val1, sym);
		VAL_SET(val1, REB_SET_WORD);
		Get_GOB_Var(gob, val1, val);
	}

	return ser;
}


/***********************************************************************
**
*/	REBFLG MT_Gob(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	REBGOB *ngob;

	if (IS_BLOCK(data)) {
		ngob = Make_Gob();
		Set_GOB_Vars(ngob, VAL_BLK_DATA(data));
		SET_GOB(out, ngob);
		return TRUE;
	}

	return FALSE;
}


/***********************************************************************
**
*/	REBINT PD_Gob(REBPVS *pvs)
/*
***********************************************************************/
{
	REBGOB *gob = VAL_GOB(pvs->value);
	REBCNT index;
	REBCNT tail;

	if (IS_WORD(pvs->select)) {
		if (pvs->setval == 0 || NOT_END(pvs->path+1)) {
			if (!Get_GOB_Var(gob, pvs->select, pvs->store)) return PE_BAD_SELECT;
			// Check for SIZE/X: types of cases:
			if (pvs->setval && IS_PAIR(pvs->store)) {
				REBVAL *sel = pvs->select;
				pvs->value = pvs->store;
				Next_Path(pvs); // sets value in pvs->store
				Set_GOB_Var(gob, sel, pvs->store); // write it back to gob
			}
			return PE_USE;
		} else {
			if (!Set_GOB_Var(gob, pvs->select, pvs->setval)) return PE_BAD_SET;
			return PE_OK;
		}
	}
	if (IS_INTEGER(pvs->select)) {
		if (!GOB_PANE(gob)) return PE_NONE;
		tail = GOB_PANE(gob) ? GOB_TAIL(gob) : 0;
		index = VAL_GOB_INDEX(pvs->value);
		index += Int32(pvs->select) - 1;
		if (index >= tail) return PE_NONE;
		gob = *GOB_SKIP(gob, index);
		index = 0;
		VAL_SET(pvs->store, REB_GOB);
		VAL_GOB(pvs->store) = gob;
		VAL_GOB_INDEX(pvs->store) = 0;
		return PE_USE;
	}
	return PE_BAD_SELECT;
}


/***********************************************************************
**
*/	REBTYPE(Gob)
/*
***********************************************************************/
{
	REBVAL *val;
	REBVAL *arg;
	REBGOB *gob;
	REBGOB *ngob;
	REBCNT index;
	REBCNT tail;
	REBCNT len;

	arg = D_ARG(2);
	val = D_RET;
	*val = *D_ARG(1);
	gob = 0;

	if (IS_GOB(val)) {
		gob = VAL_GOB(val);
		index = VAL_GOB_INDEX(val);
		tail = GOB_PANE(gob) ? GOB_TAIL(gob) : 0;
	}

	// unary actions
	switch(action) {

	case A_MAKE:
		ngob = Make_Gob();
		val = D_ARG(1);

		// Clone an existing GOB:
		if (IS_GOB(val)) {	// local variable "gob" is valid
			*ngob = *gob;	// Copy all values
			ngob->pane = 0;
			ngob->parent = 0;
		}
		else if (!IS_DATATYPE(val)) goto is_arg_error;

		// Initialize GOB from block:
		if (IS_BLOCK(arg)) {
			Set_GOB_Vars(ngob, VAL_BLK_DATA(arg));
		}
		// Merge GOB provided as argument:
		else if (IS_GOB(arg)) {
			*ngob = *VAL_GOB(arg);
			ngob->pane = 0;
			ngob->parent = 0;
		}
		else if (IS_PAIR(arg)) {
			ngob->size.x = VAL_PAIR_X(arg);
			ngob->size.y = VAL_PAIR_Y(arg);
		}
		else
			Trap_Make(REB_GOB, arg);
		// Allow NONE as argument:
//		else if (!IS_NONE(arg))
//			goto is_arg_error;
		SET_GOB(DS_RETURN, ngob);
		break;

	case A_PICK:
		if (!IS_NUMBER(arg) && !IS_NONE(arg)) Trap_Arg(arg);
		if (!GOB_PANE(gob)) goto is_none;
		index += Get_Num_Arg(arg) - 1;
		if (index >= tail) goto is_none;
		gob = *GOB_SKIP(gob, index);
		index = 0;
		goto set_index;

	case A_POKE:
		index += Get_Num_Arg(arg) - 1;
		arg = D_ARG(3);
	case A_CHANGE:
		if (!IS_GOB(arg)) goto is_arg_error;
		if (!GOB_PANE(gob) || index >= tail) Trap0(RE_PAST_END);
		if (action == A_CHANGE && (DS_REF(AN_PART) || DS_REF(AN_ONLY) || DS_REF(AN_DUP))) Trap0(RE_NOT_DONE);
		Insert_Gobs(gob, arg, index, 1, 0);
		//ngob = *GOB_SKIP(gob, index);
		//GOB_PARENT(ngob) = 0;
		//*GOB_SKIP(gob, index) = VAL_GOB(arg);
		if (action == A_POKE) {
			*DS_RETURN = *arg;
			return R_RET;
		}
		index++;
		goto set_index;

	case A_APPEND:
		index = tail;
	case A_INSERT:
		if (DS_REF(AN_PART) || DS_REF(AN_ONLY) || DS_REF(AN_DUP)) Trap0(RE_NOT_DONE);
		if (IS_GOB(arg)) len = 1;
		else if (IS_BLOCK(arg)) {
			len = VAL_BLK_LEN(arg);
			arg = VAL_BLK_DATA(arg);
		}
		else goto is_arg_error;;
		Insert_Gobs(gob, arg, index, len, 0);
		break;

	case A_CLEAR:
		if (tail > index) Remove_Gobs(gob, index, tail - index);
		break;

	case A_REMOVE:
		// /PART length
		len = D_REF(2) ? Get_Num_Arg(D_ARG(3)) : 1;
		if (index + len > tail) len = tail - index;
		if (index < tail && len != 0) Remove_Gobs(gob, index, len);
		break;

	case A_TAKE:
		len = D_REF(2) ? Get_Num_Arg(D_ARG(3)) : 1;
		if (index + len > tail) len = tail - index;
		if (index < 0 || index >= tail) goto is_none;
		if (!D_REF(2)) { // just one value
			VAL_SET(val, REB_GOB);
			VAL_GOB(val) = *GOB_SKIP(gob, index);
			VAL_GOB_INDEX(val) = 0;
			Remove_Gobs(gob, index, 1);
			return R_RET;
		} else {
			Set_Block(D_RET, Pane_To_Block(gob, index, len));
			Remove_Gobs(gob, index, len);
		}
		return R_RET;

	case A_NEXT:
		if (index < tail) index++;
		goto set_index;

	case A_BACK:
		if (index > 0) index--;
		goto set_index;

	case A_AT:
		index--;
	case A_SKIP:
		index += VAL_INT32(arg);
		goto set_index;

	case A_HEAD:
		index = 0;
		goto set_index;

	case A_TAIL:
		index = tail;
		goto set_index;

	case A_HEADQ:
		if (index == 0) goto is_true;
		goto is_false;

	case A_TAILQ:
		if (index >= tail) goto is_true;
		goto is_false;

	case A_PASTQ:
		if (index > tail) goto is_true;
		goto is_false;

	case A_INDEXQ:
		SET_INTEGER(val, index+1);
		break;

	case A_LENGTHQ:
		index = (tail > index) ? tail - index : 0;
		SET_INTEGER(val, index);
		break;

	case A_FIND:
		if (IS_GOB(arg)) {
			index = Find_Gob(gob, VAL_GOB(arg));
			if (index == NOT_FOUND) goto is_none;
			goto set_index;
		}
		goto is_none;

    case A_REVERSE:
		for (index = 0; index < tail/2; index++) {
			ngob = *GOB_SKIP(gob, tail-index-1);
			*GOB_SKIP(gob, tail-index-1) = *GOB_SKIP(gob, index);
			*GOB_SKIP(gob, index) = ngob;
		}
		return R_ARG1;

	default:
		Trap_Action(REB_GOB, action);
	}
	return R_RET;

set_index:
	VAL_SET(val, REB_GOB);
	VAL_GOB(val) = gob;
	VAL_GOB_INDEX(val) = index;
	return R_RET;

is_none:
	return R_NONE;

is_arg_error:
	Trap_Types(RE_EXPECT_VAL, REB_GOB, VAL_TYPE(arg));

is_false:
	return R_FALSE;

is_true:
	return R_TRUE;
}
