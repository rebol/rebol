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
#include "reb-ext.h" // includes copy of ext-types.h

extern const REBYTE Reb_To_RXT[REB_MAX];
extern RXIARG Value_To_RXI(REBVAL *val); // f-extension.c
extern void RXI_To_Value(REBVAL *val, RXIARG arg, REBCNT type); // f-extension.


/***********************************************************************
**
*/	REBINT Cmp_Handle(REBVAL *a, REBVAL *b)
/*
**		Ordering for SORT and for the comparison actions.
**
**		Returns <0, 0 or >0, and 0 only for the SAME handle. The order
**		between two different ones is arbitrary but total and stable,
**		which is all a sort asks for: context handles first, then by
**		type name, then by the address of the thing they hold.
**
***********************************************************************/
{
	REBINT  diff;
	REBUPT  pa, pb;

	// A context handle and a data handle are never interleaved.
	if (IS_CONTEXT_HANDLE(a) != IS_CONTEXT_HANDLE(b))
		return IS_CONTEXT_HANDLE(a) ? -1 : 1;

	// Two context handles of different types sort by type NAME, so that
	// a sorted block groups by type and reads the way a person expects.
	//
	// Compare_UTF8 answers on a scale of its own, and the sign reads the
	// OPPOSITE way round from a subtraction: -1 means s1 > s2. Its
	// header gives the conversion, and both halves are needed here -
	//
	//     -3  s1 < s2, really different   -> +2
	//     -1  s1 > s2, really different   -> +2
	//      1  s1 < s2, differs by case    -> -2
	//      3  s1 > s2, differs by case    -> -2
	//      0  identical
	//
	// The case range cannot arise while type names are canon symbols -
	// two spellings differing only in case are one symbol, and the test
	// above has already established the symbols differ - but converting
	// it correctly costs one line and removes the question.
	if (IS_CONTEXT_HANDLE(a) && VAL_HANDLE_SYM(a) != VAL_HANDLE_SYM(b)) {
		REBYTE* sp = VAL_HANDLE_NAME(a);
		REBYTE* tp = VAL_HANDLE_NAME(b);
		diff = Compare_UTF8(sp, tp, (REBCNT)LEN_BYTES(tp));
		if (diff < 0) return diff + 2;
		if (diff > 0) return diff - 2;
		// Identical spelling under two symbols: fall through, so the two
		// are still ordered rather than reported equal.
	}

	// Same type, or both data handles: the address decides.
	//
	// The WHOLE pointer. VAL_HANDLE_I32 is an int view of the same union
	// - half a pointer on a 64 bit build - and subtracting two of them
	// overflows for addresses far enough apart, which is a sign flip
	// rather than a wrong order. Compared, never subtracted.
	pa = (REBUPT)VAL_HANDLE_DATA(a);
	pb = (REBUPT)VAL_HANDLE_DATA(b);
	if (pa != pb) return (pa < pb) ? -1 : 1;

	// Identical payloads under different names are still different
	// handles - and this is the last thing left to separate them.
	if (VAL_HANDLE_SYM(a) != VAL_HANDLE_SYM(b))
		return (VAL_HANDLE_SYM(a) < VAL_HANDLE_SYM(b)) ? -1 : 1;

	return 0;
}


/***********************************************************************
**
*/	REBINT CT_Handle(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	REBINT diff;

	if (mode >= 0) {
		// EQUAL? and STRICT-EQUAL? are the same question for a handle.
		// There is no loose form: no case, no encoding, no index into a
		// series - nothing two distinct handles could differ in and
		// still be the same value. So `=` means what `==` means, and
		// FIND, SELECT and UNIQUE do the right thing by consequence.
		if (IS_CONTEXT_HANDLE(a) || IS_CONTEXT_HANDLE(b)) {
			if (!IS_CONTEXT_HANDLE(a) || !IS_CONTEXT_HANDLE(b)) return 0;

			// The context alone. Equal contexts have equal types, and
			// the flags are deliberately NOT consulted: they carry
			// HANDLE_CONTEXT_MARKED, which the collector owns, so two
			// values naming this one handle can hold different
			// snapshots of it.
			return (VAL_HANDLE_CTX(a) == VAL_HANDLE_CTX(b));
		}

		// A data handle is what it points at, what kind of pointer that
		// is, and what it is called. The bookkeeping bits are masked out
		// here for the same reason as above.
		return (VAL_HANDLE_DATA(a) == VAL_HANDLE_DATA(b))
			&& (VAL_HANDLE_SYM(a) == VAL_HANDLE_SYM(b))
			&& ((VAL_HANDLE_FLAGS(a) & HANDLE_VALUE_FLAGS)
				== (VAL_HANDLE_FLAGS(b) & HANDLE_VALUE_FLAGS));
	}

	diff = Cmp_Handle(a, b);
	if (mode == -1) return (diff >= 0);
	return (diff > 0);
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

	if (IS_CONTEXT_HANDLE(data) && IS_USED_HOB(VAL_HANDLE_CTX(data))) {
		RXIARG xarg;
		REBCNT type;
		REBCNT idx = VAL_HANDLE_CTX(data)->index;
		REBHSP spec = PG_Handles[idx];
		if (val == 0) {
			if (spec.get_path) {
				if (PE_USE == spec.get_path(VAL_HANDLE_CTX(data), sym, &type, &xarg)) {
					RXI_To_Value(pvs->store, xarg, type);
					return PE_USE;
				}
			}
			// A word this handle does not know is an error, as before -
			// except TYPE, which is answered below for every handle
			// rather than only for a live context one.
			if (sym != SYM_TYPE) return PE_BAD_SELECT;
		} else {
			if (spec.set_path) {
				type = Reb_To_RXT[VAL_TYPE(val)];
				xarg = Value_To_RXI(val);
				return spec.set_path(VAL_HANDLE_CTX(data), sym, &type, &xarg);
			}
		}
	}
 
	// The type comes out of the value itself, so this is reachable for a
	// context handle whose get_path refused the word, for one whose
	// context has been released, and for a data handle which never had a
	// context at all. It is the same answer MOLD prints.
	if (val == 0 && sym == SYM_TYPE) {
		Set_Word(pvs->store, VAL_HANDLE_SYM(data), NULL, 0);
		return PE_USE;
	}
 
	// for the data handles, return NONE on get
	return NZ(val) ? PE_BAD_SET : PE_NONE;
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
		REBVAL *field = D_ARG(ARG_QUERY_FIELD);
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
					if (!IS_GET_WORD(word)) {
						// keep the set-word in result
						out = Append_Value(values);
						*out = *word;
						VAL_TYPE(out) = REB_SET_WORD;
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
		else if (IS_NONE(field)){
			Set_Block(D_RET, Get_Object_Words(spec));
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
