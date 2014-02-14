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
**  Module:  t-event.c
**  Summary: event datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**    Events are kept compact in order to fit into normal 128 bit
**    values cells. This provides high performance for high frequency
**    events and also good memory efficiency using standard series.
**
***********************************************************************/

#include "sys-core.h"
#include "reb-evtypes.h"
#include "reb-net.h"


/***********************************************************************
**
*/	REBINT CT_Event(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	REBINT diff = Cmp_Event(a, b);
	if (mode >=0) return diff == 0;
	return -1;
}


/***********************************************************************
**
*/	REBINT Cmp_Event(REBVAL *t1, REBVAL *t2)
/*
**	Given two events, compare them.
**
***********************************************************************/
{
	REBINT	diff;

	if (
		   (diff = VAL_EVENT_MODEL(t1) - VAL_EVENT_MODEL(t2))
		|| (diff = VAL_EVENT_TYPE(t1) - VAL_EVENT_TYPE(t2))
		|| (diff = VAL_EVENT_XY(t1) - VAL_EVENT_XY(t2))
	) return diff;

	return 0;
}


/***********************************************************************
**
*/	static REBFLG Set_Event_Var(REBVAL *value, REBVAL *word, REBVAL *val)
/*
***********************************************************************/
{
	REBVAL *arg;
	REBINT n;
	REBCNT w;

	switch (VAL_WORD_CANON(word)) {

	case SYM_TYPE:
		if (!IS_WORD(val) && !IS_LIT_WORD(val)) return FALSE;
		arg = Get_System(SYS_VIEW, VIEW_EVENT_TYPES);
		if (IS_BLOCK(arg)) {
			w = VAL_WORD_CANON(val);
			for (n = 0, arg = VAL_BLK(arg); NOT_END(arg); arg++, n++) {
				if (IS_WORD(arg) && VAL_WORD_CANON(arg) == w) {
					VAL_EVENT_TYPE(value) = n;
					return TRUE;
				}
			}
			Trap_Arg(val);
		}
		return FALSE;

	case SYM_PORT:
		if (IS_PORT(val)) {
			VAL_EVENT_MODEL(value) = EVM_PORT;
			VAL_EVENT_SER(value) = VAL_PORT(val);
		}
		else if (IS_OBJECT(val)) {
			VAL_EVENT_MODEL(value) = EVM_OBJECT;
			VAL_EVENT_SER(value) = VAL_OBJ_FRAME(val);
		}
		else if (IS_NONE(val)) {
			VAL_EVENT_MODEL(value) = EVM_GUI;
		} else return FALSE;
		break;

	case SYM_WINDOW:
	case SYM_GOB:
		if (IS_GOB(val)) {
			VAL_EVENT_MODEL(value) = EVM_GUI;
			VAL_EVENT_SER(value) = VAL_GOB(val);
			break;
		}
		return FALSE; 

	case SYM_OFFSET:
		if (IS_PAIR(val)) {
			SET_EVENT_XY(value, Float_Int16(VAL_PAIR_X(val)), Float_Int16(VAL_PAIR_Y(val)));
		}
		else return FALSE;
		break;

	case SYM_KEY:
		//VAL_EVENT_TYPE(value) != EVT_KEY && VAL_EVENT_TYPE(value) != EVT_KEY_UP)
		VAL_EVENT_MODEL(value) = EVM_GUI;
		if (IS_CHAR(val)) {
			VAL_EVENT_DATA(value) = VAL_CHAR(val);
		}
		else if (IS_LIT_WORD(val) || IS_WORD(val)) {
			arg = Get_System(SYS_VIEW, VIEW_EVENT_KEYS);
			if (IS_BLOCK(arg)) {
				arg = VAL_BLK_DATA(arg);
				for (n = VAL_INDEX(arg); NOT_END(arg); n++, arg++) {
					if (IS_WORD(arg) && VAL_WORD_CANON(arg) == VAL_WORD_CANON(val)) {
						VAL_EVENT_DATA(value) = (n+1) << 16;
						break;
					}
				}
				if (IS_END(arg)) return FALSE;
				break;
			}
			return FALSE;
		}
		else return FALSE;
		break;

	case SYM_CODE:
		if (IS_INTEGER(val)) {
			VAL_EVENT_DATA(value) = VAL_INT32(val);
		}
		else return FALSE;
		break;

	default:
			return FALSE;
	}

	return TRUE;
}


/***********************************************************************
**
*/	static void Set_Event_Vars(REBVAL *evt, REBVAL *blk)
/*
***********************************************************************/
{
	REBVAL *var;
	REBVAL *val;

	while (NOT_END(blk)) {
		var = blk++;
		val = blk++;
		if (IS_END(val)) val = NONE_VALUE;
		else val = Get_Simple_Value(val);
		if (!Set_Event_Var(evt, var, val)) Trap2(RE_BAD_FIELD_SET, var, Of_Type(val));
	}
}


/***********************************************************************
**
*/	static REBFLG Get_Event_Var(REBVAL *value, REBCNT sym, REBVAL *val)
/*
***********************************************************************/
{
	REBVAL *arg;
	REBREQ *req;
	REBINT n;
	REBSER *ser;

	switch (sym) {

	case SYM_TYPE:
		if (VAL_EVENT_TYPE(value) == 0) goto is_none;
		arg = Get_System(SYS_VIEW, VIEW_EVENT_TYPES);
		if (IS_BLOCK(arg) && VAL_TAIL(arg) >= EVT_MAX) {
			*val = *VAL_BLK_SKIP(arg, VAL_EVENT_TYPE(value));
			break;
		}
		return FALSE;

	case SYM_PORT:
		// Most events are for the GUI:
		if (IS_EVENT_MODEL(value, EVM_GUI)) {
			*val = *Get_System(SYS_VIEW, VIEW_EVENT_PORT);
		}
		// Event holds a port:
		else if (IS_EVENT_MODEL(value, EVM_PORT)) {
			SET_PORT(val, VAL_EVENT_SER(value));
		}
		// Event holds an object:
		else if (IS_EVENT_MODEL(value, EVM_OBJECT)) {
			SET_OBJECT(val, VAL_EVENT_SER(value));
		}
		else if (IS_EVENT_MODEL(value, EVM_CALLBACK)) {
			*val = *Get_System(SYS_PORTS, PORTS_CALLBACK);
		}
		else {
			// assumes EVM_DEVICE
			// Event holds the IO-Request, which has the PORT:
			req = VAL_EVENT_REQ(value);
			if (!req || !req->port) goto is_none;
			SET_PORT(val, (REBSER*)(req->port));
		}
		break;

	case SYM_WINDOW:
	case SYM_GOB:
		if (IS_EVENT_MODEL(value, EVM_GUI)) {
			if (VAL_EVENT_SER(value)) {
				SET_GOB(val, VAL_EVENT_SER(value));
				break;
			}
		}
		return FALSE;

	case SYM_OFFSET:
		if (VAL_EVENT_TYPE(value) == EVT_KEY || VAL_EVENT_TYPE(value) == EVT_KEY_UP)
			goto is_none;
		VAL_SET(val, REB_PAIR);
		VAL_PAIR_X(val) = (REBD32)VAL_EVENT_X(value);
		VAL_PAIR_Y(val) = (REBD32)VAL_EVENT_Y(value);
		break;

	case SYM_KEY:
		if (VAL_EVENT_TYPE(value) != EVT_KEY && VAL_EVENT_TYPE(value) != EVT_KEY_UP)
			goto is_none;
		n = VAL_EVENT_DATA(value); // key-words in top 16, chars in lower 16
		if (n & 0xffff0000) {
			arg = Get_System(SYS_VIEW, VIEW_EVENT_KEYS);
			n = (n >> 16) - 1;
			if (IS_BLOCK(arg) && n < (REBINT)VAL_TAIL(arg)) {
				*val = *VAL_BLK_SKIP(arg, n);
				break;
			}
			return FALSE;
		}
		SET_CHAR(val, n);
		break;

	case SYM_FLAGS:
		if (VAL_EVENT_FLAGS(value) & (1<<EVF_DOUBLE | 1<<EVF_CONTROL | 1<<EVF_SHIFT)) {
			ser = Make_Block(3);
			if (GET_FLAG(VAL_EVENT_FLAGS(value), EVF_DOUBLE)) {
				arg = Append_Value(ser);
				Init_Word(arg, SYM_DOUBLE);
			}
			if (GET_FLAG(VAL_EVENT_FLAGS(value), EVF_CONTROL)) {
				arg = Append_Value(ser);
				Init_Word(arg, SYM_CONTROL);
			}
			if (GET_FLAG(VAL_EVENT_FLAGS(value), EVF_SHIFT)) {
				arg = Append_Value(ser);
				Init_Word(arg, SYM_SHIFT);
			}
			Set_Block(val, ser);
		} else SET_NONE(val);
		break;

	case SYM_CODE:
		if (VAL_EVENT_TYPE(value) != EVT_KEY && VAL_EVENT_TYPE(value) != EVT_KEY_UP)
			goto is_none;
		n = VAL_EVENT_DATA(value); // key-words in top 16, chars in lower 16
		SET_INTEGER(val, n);
		break;

	case SYM_DATA:
		// Event holds a file string:
		if (VAL_EVENT_TYPE(value) != EVT_DROP_FILE) goto is_none;
		if (!GET_FLAG(VAL_EVENT_FLAGS(value), EVF_COPIED)) {
			void *str = VAL_EVENT_SER(value);
			VAL_EVENT_SER(value) = Copy_Bytes(str, -1);
			SET_FLAG(VAL_EVENT_FLAGS(value), EVF_COPIED);
			OS_FREE(str);
		}
		Set_Series(REB_FILE, val, VAL_EVENT_SER(value));
		break;

	default:
		return FALSE;
	}

	return TRUE;

is_none:
	SET_NONE(val);
	return TRUE;
}


/***********************************************************************
**
*/	REBFLG MT_Event(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	if (IS_BLOCK(data)) {
		CLEARS(out);
		Set_Event_Vars(out, VAL_BLK_DATA(data));
		VAL_SET(out, REB_EVENT);
		return TRUE;
	}

	return FALSE;
}


/***********************************************************************
**
*/	REBINT PD_Event(REBPVS *pvs)
/*
***********************************************************************/
{
	if (IS_WORD(pvs->select)) {
		if (pvs->setval == 0 || NOT_END(pvs->path+1)) {
			if (!Get_Event_Var(pvs->value, VAL_WORD_CANON(pvs->select), pvs->store)) return PE_BAD_SELECT;
			return PE_USE;
		} else {
			if (!Set_Event_Var(pvs->value, pvs->select, pvs->setval)) return PE_BAD_SET;
			return PE_OK;
		}
	}
	return PE_BAD_SELECT;
}


/***********************************************************************
**
*/	REBTYPE(Event)
/*
***********************************************************************/
{
	REBVAL *value;
	REBVAL *arg;

	value = D_ARG(1);
	arg = D_ARG(2);

	if (action == A_MAKE) {
		// Clone an existing event?
		if (IS_EVENT(value)) return R_ARG1;
		else if (IS_DATATYPE(value)) {
			if (IS_EVENT(arg)) return R_ARG2;
			//Trap_Make(REB_EVENT, value);
			VAL_SET(D_RET, REB_EVENT);
			CLEARS(&(D_RET->data.event));
		}
		else
is_arg_error:
			Trap_Types(RE_EXPECT_VAL, REB_EVENT, VAL_TYPE(arg));

		// Initialize GOB from block:
		if (IS_BLOCK(arg)) Set_Event_Vars(D_RET, VAL_BLK_DATA(arg));
		else goto is_arg_error;
	}
	else Trap_Action(REB_EVENT, action);

	return R_RET;
}

#ifdef ndef
//	case A_PATH:
		if (IS_WORD(arg)) {
			switch (VAL_WORD_CANON(arg)) {
			case SYM_TYPE:    index = EF_TYPE; break;
			case SYM_PORT:	  index = EF_PORT; break;
			case SYM_KEY:     index = EF_KEY; break;
			case SYM_OFFSET:  index = EF_OFFSET; break;
			case SYM_MODE:	  index = EF_MODE; break;
			case SYM_TIME:    index = EF_TIME; break;
//!!! return these as options flags, not refinements.
//			case SYM_SHIFT:   index = EF_SHIFT; break;
//			case SYM_CONTROL: index = EF_CONTROL; break;
//			case SYM_DOUBLE_CLICK: index = EF_DCLICK; break;
			default: Trap1(RE_INVALID_PATH, arg);
			}
			goto pick_it;
		}
		else if (!IS_INTEGER(arg))
			Trap1(RE_INVALID_PATH, arg);
		// fall thru


	case A_PICK:
		index = num = Get_Num_Arg(arg);
		if (num > 0) index--;
		if (num == 0 || index < 0 || index > EF_DCLICK) {
			if (action == A_POKE) Trap_Range(arg);
			goto is_none;
		}
pick_it:
		switch(index) {
		case EF_TYPE:
			if (VAL_EVENT_TYPE(value) == 0) goto is_none;
			arg = Get_System(SYS_VIEW, VIEW_EVENT_TYPES);
			if (IS_BLOCK(arg) && VAL_TAIL(arg) >= EVT_MAX) {
				*D_RET = *VAL_BLK_SKIP(arg, VAL_EVENT_TYPE(value));
				return R_RET;
			}
			return R_NONE;

		case EF_PORT:
			// Most events are for the GUI:
			if (GET_FLAG(VAL_EVENT_FLAGS(value), EVF_NO_REQ))
				*D_RET = *Get_System(SYS_VIEW, VIEW_EVENT_PORT);
			else {
				req = VAL_EVENT_REQ(value);
				if (!req || !req->port) goto is_none;
				SET_PORT(D_RET, (REBSER*)(req->port));
			}
			return R_RET;

		case EF_KEY:
			if (VAL_EVENT_TYPE(value) != EVT_KEY) goto is_none;
			if (VAL_EVENT_FLAGS(value)) {  // !!!!!!!!!!!!! needs mask
				VAL_SET(D_RET, REB_CHAR);
				VAL_CHAR(D_RET) = VAL_EVENT_KEY(value) & 0xff;
			} else
				Init_Word(D_RET, VAL_EVENT_XY(value));
			return R_RET;

		case EF_OFFSET:
			VAL_SET(D_RET, REB_PAIR);
			VAL_PAIR_X(D_RET) = VAL_EVENT_X(value);
			VAL_PAIR_Y(D_RET) = VAL_EVENT_Y(value);
			return R_RET;

		case EF_TIME:
			VAL_SET(D_RET, REB_INTEGER);
//!!			VAL_INT64(D_RET) = VAL_EVENT_TIME(value);
			return R_RET;

		case EF_SHIFT:
			VAL_SET(D_RET, REB_LOGIC);
			VAL_LOGIC(D_RET) = GET_FLAG(VAL_EVENT_FLAGS(value), EVF_SHIFT) != 0;
			return R_RET;

		case EF_CONTROL:
			VAL_SET(D_RET, REB_LOGIC);
			VAL_LOGIC(D_RET) = GET_FLAG(VAL_EVENT_FLAGS(value), EVF_CONTROL) != 0;
			return R_RET;

		case EF_DCLICK:
			VAL_SET(D_RET, REB_LOGIC);
			VAL_LOGIC(D_RET) = GET_FLAG(VAL_EVENT_FLAGS(value), EVF_DOUBLE) != 0;
			return R_RET;

/*			case EF_FACE:
			{
				REBWIN	*wp;
				if (!IS_BLOCK(BLK_HEAD(Windows) + VAL_EVENT_WIN(value))) return R_RET None_Value;
				wp = (REBWIN *)VAL_BLK(BLK_HEAD(Windows) + VAL_EVENT_WIN(value));
				*D_RET = wp->masterFace;
				return R_RET;
			}
*/
		}
		break;

// These are used to map symbols to event field cases:
enum rebol_event_fields {
	EF_TYPE,
	EF_KEY,
	EF_OFFSET,
	EF_TIME,
	EF_SHIFT,	// Keep these? !!!
	EF_CONTROL,
	EF_DCLICK,
	EF_PORT,
	EF_MODE,
};

#endif


/***********************************************************************
**
*/	 void Mold_Event(REBVAL *value, REB_MOLD *mold)
/*
***********************************************************************/
{
	REBVAL val;
	REBCNT field;
	REBCNT fields[] = {
		SYM_TYPE, SYM_PORT, SYM_GOB, SYM_OFFSET, SYM_KEY,
		SYM_FLAGS, SYM_CODE, SYM_DATA, 0
	};

	Pre_Mold(value, mold);
	Append_Byte(mold->series, '[');
	mold->indent++;

	for (field = 0; fields[field]; field++) {
		Get_Event_Var(value, fields[field], &val);
		if (!IS_NONE(&val)) {
			New_Indented_Line(mold);
			Append_UTF8(mold->series, Get_Sym_Name(fields[field]), -1);
			Append_Bytes(mold->series, ": ");
			if (IS_WORD(&val)) Append_Byte(mold->series, '\'');
			Mold_Value(mold, &val, TRUE);
		}
	}

	mold->indent--;
	New_Indented_Line(mold);
	Append_Byte(mold->series, ']');

	End_Mold(mold);
}

