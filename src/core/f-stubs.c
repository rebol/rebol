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
**  Module:  f-stubs.c
**  Summary: miscellaneous little functions
**  Section: functional
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "sys-deci-funcs.h"

/***********************************************************************
**
*/	void Long_To_Bytes(REBYTE *out, REBCNT in)
/*
***********************************************************************/
{
	out[0] = (REBYTE) in;
	out[1] = (REBYTE)(in >> 8);
	out[2] = (REBYTE)(in >> 16);
	out[3] = (REBYTE)(in >> 24);
}


/***********************************************************************
**
*/	REBCNT Bytes_To_Long(REBYTE const *in)
/*
***********************************************************************/
{
	return (REBCNT) in[0]          // & 0xFF
	     | (REBCNT) (in[1] <<  8)  // & 0xFF00;
	     | (REBCNT) (in[2] << 16)  // & 0xFF0000;
	     | (REBCNT) (in[3] << 24); // & 0xFF000000;
}


/***********************************************************************
**
*/	REBCNT Find_Int(REBINT *array, REBINT num)
/*
***********************************************************************/
{
	REBCNT n;

	for (n = 0; array[n] && array[n] != num; n++);
	if (array[n]) return n;
	return NOT_FOUND;
}


/***********************************************************************
**
*/	REBINT Get_Num_Arg(REBVAL *val)
/*
**		Get the amount to skip or pick.
**		Allow multiple types. Throw error if not valid.
**		Note that the result is one-based.
**
***********************************************************************/
{
	REBINT n;

	if (IS_INTEGER(val)) {
		if (VAL_INT64(val) > (i64)MAX_I32 || VAL_INT64(val) < (i64)MIN_I32)
			Trap_Range(val);
		n = VAL_INT32(val);
	}
	else if (IS_DECIMAL(val) || IS_PERCENT(val)) {
		if (VAL_DECIMAL(val) > MAX_I32 || VAL_DECIMAL(val) < MIN_I32)
			Trap_Range(val);
		n = (REBINT)VAL_DECIMAL(val);
	}
	else if (IS_LOGIC(val)) n = (VAL_LOGIC(val) ? 1 : 2);
	else Trap_Arg(val);

	return n;
}


/***********************************************************************
**
*/	REBINT Float_Int16(REBD32 f)
/*
***********************************************************************/
{
	if (fabs(f) > (REBD32)(0x7FFF)) {
		DS_PUSH_DECIMAL(f);
		Trap_Range(DS_TOP);
	}
	return (REBINT)f;
}


/***********************************************************************
**
*/	REBINT Int32(REBVAL *val)
/*
***********************************************************************/
{
	REBINT n = 0;

	if (IS_DECIMAL(val)) {
		if (VAL_DECIMAL(val) > MAX_I32 || VAL_DECIMAL(val) < MIN_I32)
			Trap_Range(val);
		n = (REBINT)VAL_DECIMAL(val);
	} else {
		if (VAL_INT64(val) > (i64)MAX_I32 || VAL_INT64(val) < (i64)MIN_I32)
			Trap_Range(val);
		n = VAL_INT32(val);
	}

	return n;
}


/***********************************************************************
**
*/	REBINT Int32s(REBVAL *val, REBINT sign)
/*
**		Get integer as positive, negative 32 bit value.
**		Sign field can be
**			0: >= 0
**			1: >  0
**		   -1: <  0
**
***********************************************************************/
{
	REBINT n = 0;

	if (IS_DECIMAL(val)) {
		if (VAL_DECIMAL(val) > MAX_I32 || VAL_DECIMAL(val) < MIN_I32)
			Trap_Range(val);

		n = (REBINT)VAL_DECIMAL(val);
	} else {
		if (VAL_INT64(val) > (i64)MAX_I32 || VAL_INT64(val) < (i64)MIN_I32)
			Trap_Range(val);

		n = VAL_INT32(val);
	}

	// More efficient to use positive sense:
	if (
		(sign == 0 && n >= 0) ||
		(sign >  0 && n >  0) ||
		(sign <  0 && n <  0)
	)
		return n;

	Trap_Range(val);
	return 0;
}


/***********************************************************************
**
*/	REBI64 Int64(REBVAL *val)
/*
***********************************************************************/
{
	if (IS_INTEGER(val)) return VAL_INT64(val);
	if (IS_DECIMAL(val) || IS_PERCENT(val)) return (REBI64)VAL_DECIMAL(val);
	if (IS_MONEY(val)) return deci_to_int(VAL_DECI(val));
	Trap_Arg(val);
	return 0;
}


/***********************************************************************
**
*/	REBDEC Dec64(REBVAL *val)
/*
***********************************************************************/
{
	if (IS_DECIMAL(val) || IS_PERCENT(val)) return VAL_DECIMAL(val);
	if (IS_INTEGER(val)) return (REBDEC)VAL_INT64(val);
	if (IS_MONEY(val)) return deci_to_decimal(VAL_DECI(val));
	Trap_Arg(val);
	return 0;
}


/***********************************************************************
**
*/	REBI64 Int64s(REBVAL *val, REBINT sign)
/*
**		Get integer as positive, negative 64 bit value.
**		Sign field can be
**			0: >= 0
**			1: >  0
**		   -1: <  0
**
***********************************************************************/
{
	REBI64 n;

	if (IS_DECIMAL(val)) {
		if (VAL_DECIMAL(val) > MAX_I64 || VAL_DECIMAL(val) < MIN_I64)
			Trap_Range(val);
		n = (REBI64)VAL_DECIMAL(val);
	} else {
		n = VAL_INT64(val);
	}

	// More efficient to use positive sense:
	if (
		(sign == 0 && n >= 0) ||
		(sign >  0 && n >  0) ||
		(sign <  0 && n <  0)
	)
		return n;

	Trap_Range(val);
	DEAD_END;
}


/***********************************************************************
**
*/	REBINT Int8u(REBVAL *val)
/*
***********************************************************************/
{
	if (VAL_INT64(val) > (i64)255 || VAL_INT64(val) < (i64)0) Trap_Range(val);
	return VAL_INT32(val);
}


/***********************************************************************
**
*/	REBCNT Find_Refines(REBVAL *ds, REBCNT mask)
/*
**		Scans the stack for function refinements that have been
**		specified in the mask (each as a bit) and are being used.
**
***********************************************************************/
{
	REBINT n;
	REBCNT result = 0;
	REBINT len = DS_ARGC;

	for (n = 0; n < len; n++) {
		if ((mask & (1 << n) && D_REF(n+1)))
			result |= 1 << n;
	}
	return result;
}


/***********************************************************************
**
*/	void Set_Datatype(REBVAL *value, REBINT n)
/*
***********************************************************************/
{
	*value = *BLK_SKIP(Lib_Context, n+1);
}


/***********************************************************************
**
*/  REBVAL *Get_Type(REBCNT index)
/*
**      Returns the specified datatype value from the system context.
**		The datatypes are all at the head of the context.
**
***********************************************************************/
{
	ASSERT(index < SERIES_TAIL(Lib_Context), RP_BAD_OBJ_INDEX);
	return FRM_VALUES(Lib_Context) + index + 1;
}


/***********************************************************************
**
*/  REBVAL *Of_Type(REBVAL *value)
/*
**      Returns the datatype value for the given value.
**		The datatypes are all at the head of the context.
**
***********************************************************************/
{
	return FRM_VALUES(Lib_Context) + VAL_TYPE(value) + 1;
}


/***********************************************************************
**
*/  REBINT Get_Type_Sym(REBCNT type)
/*
**      Returns the datatype word for the given type number.
**
***********************************************************************/
{
	return FRM_WORD_SYM(Lib_Context, type + 1);
}


/***********************************************************************
**
*/  REBVAL *Get_Type_Word(REBCNT type)
/*
**      Returns the datatype word for the given type number.
**
***********************************************************************/
{
	return FRM_WORD(Lib_Context, type + 1);
}


/***********************************************************************
**
*/  REBYTE *Get_Field_Name(REBSER *obj, REBCNT index)
/*
**      Get the name of a field of an object.
**
***********************************************************************/
{
	ASSERT1(index < SERIES_TAIL(obj), RP_BAD_OBJ_INDEX);
	return Get_Sym_Name(FRM_WORD_SYM(obj, index));
}


/***********************************************************************
**
*/  REBVAL *Get_Field(REBSER *obj, REBCNT index)
/*
**      Get an instance variable from an object series.
**
***********************************************************************/
{
	ASSERT1(index < SERIES_TAIL(obj), RP_BAD_OBJ_INDEX);
	return FRM_VALUES(obj) + index;
}


/***********************************************************************
**
*/  REBVAL *Get_Object(REBVAL *objval, REBCNT index)
/*
**      Get an instance variable from an object value.
**
***********************************************************************/
{
	REBSER *obj = VAL_OBJ_FRAME(objval);
	ASSERT1(IS_FRAME(BLK_HEAD(obj)), RP_BAD_OBJ_FRAME);
	ASSERT1(index < SERIES_TAIL(obj), RP_BAD_OBJ_INDEX);
	return FRM_VALUES(obj) + index;
}


/***********************************************************************
**
*/  REBVAL *In_Object(REBSER *base, ...)
/*
**      Get value from nested list of objects. List is null terminated.
**		Returns object value, else returns 0 if not found.
**
***********************************************************************/
{
	REBVAL *obj = 0;
	REBCNT n;
	va_list args;

	va_start(args, base);
	while (NZ(n = va_arg(args, REBCNT))) {
		if (n >= SERIES_TAIL(base)) return 0;
		obj = OFV(base, n);
		if (!IS_OBJECT(obj)) return 0;
		base = VAL_OBJ_FRAME(obj);
	}
	va_end(args);

	return obj;
}


/***********************************************************************
**
*/  REBVAL *Get_System(REBCNT i1, REBCNT i2)
/*
**      Return a second level object field of the system object.
**
***********************************************************************/
{
	REBVAL *obj;

	obj = VAL_OBJ_VALUES(ROOT_SYSTEM) + i1;
	if (!i2) return obj;
	ASSERT1(IS_OBJECT(obj), RP_BAD_OBJ_INDEX);
	return Get_Field(VAL_OBJ_FRAME(obj), i2);
}


/***********************************************************************
**
*/  REBINT Get_System_Int(REBCNT i1, REBCNT i2, REBINT default_int)
/*
**      Get an integer from system object.
**
***********************************************************************/
{
	REBVAL *val = Get_System(i1, i2);
	if (IS_INTEGER(val)) return VAL_INT32(val);
	return default_int;
}


/***********************************************************************
**
*/  REBSER *Make_Std_Object(REBCNT index)
/*
***********************************************************************/
{
	return CLONE_OBJECT(VAL_OBJ_FRAME(Get_System(SYS_STANDARD, index)));
}


/***********************************************************************
**
*/  void Set_Object_Values(REBSER *obj, REBVAL *vals)
/*
***********************************************************************/
{
	REBVAL *value;

	for (value = FRM_VALUES(obj) + 1; NOT_END(value); value++) { // skip self
		if (IS_END(vals)) SET_NONE(value);
		else *value = *vals++;
	}
}


/***********************************************************************
**
*/	void Set_Series(REBINT type, REBVAL *value, REBSER *series)
/*
**		Common function.
**
***********************************************************************/
{
	VAL_SET(value, type);
	VAL_SERIES(value) = series;
	VAL_INDEX(value) = 0;
	VAL_SERIES_SIDE(value) = 0;
}


/***********************************************************************
**
*/	void Set_Block(REBVAL *value, REBSER *series)
/*
**		Common function.
**
***********************************************************************/
{
	VAL_SET(value, REB_BLOCK);
	VAL_SERIES(value) = series;
	VAL_INDEX(value) = 0;
	VAL_SERIES_SIDE(value) = 0;
}


/***********************************************************************
**
*/	void Set_Block_Index(REBVAL *value, REBSER *series, REBCNT index)
/*
**		Common function.
**
***********************************************************************/
{
	VAL_SET(value, REB_BLOCK);
	VAL_SERIES(value) = series;
	VAL_INDEX(value) = index;
	VAL_SERIES_SIDE(value) = 0;
}


/***********************************************************************
**
*/	void Set_String(REBVAL *value, REBSER *series)
/*
**		Common function.
**
***********************************************************************/
{
	VAL_SET(value, REB_STRING);
	VAL_SERIES(value) = series;
	VAL_INDEX(value) = 0;
	VAL_SERIES_SIDE(value) = 0;
}


/***********************************************************************
**
*/	void Set_Binary(REBVAL *value, REBSER *series)
/*
**		Common function.
**
***********************************************************************/
{
	VAL_SET(value, REB_BINARY);
	VAL_SERIES(value) = series;
	VAL_INDEX(value) = 0;
	VAL_SERIES_SIDE(value) = 0;
}


/***********************************************************************
**
*/	void Set_Tuple(REBVAL *value, REBYTE *bytes, REBCNT len)
/*
***********************************************************************/
{
	REBYTE *bp;

	VAL_SET(value, REB_TUPLE);
	VAL_TUPLE_LEN(value) = (REBYTE)len;
	for (bp = VAL_TUPLE(value); len > 0; len--)
		*bp++ = *bytes++;
}


/***********************************************************************
**
*/	void Set_Object(REBVAL *value, REBSER *series)
/*
***********************************************************************/
{
	VAL_SET(value, REB_OBJECT);
	VAL_OBJ_FRAME(value) = series;
}


/***********************************************************************
**
*/	REBCNT Val_Series_Len(REBVAL *value)
/*
**		Get length of series, but avoid negative values.
**
***********************************************************************/
{
	if (VAL_INDEX(value) >= VAL_TAIL(value)) return 0;
	return VAL_TAIL(value) - VAL_INDEX(value);
}


/***********************************************************************
**
*/	REBCNT Val_Byte_Len(REBVAL *value)
/*
**		Get length of series in bytes.
**
***********************************************************************/
{
	if (VAL_INDEX(value) >= VAL_TAIL(value)) return 0;
	return (VAL_TAIL(value) - VAL_INDEX(value)) * SERIES_WIDE(VAL_SERIES(value));
}


/***********************************************************************
**
*/	REBFLG Get_Logic_Arg(REBVAL *arg)
/*
***********************************************************************/
{
	if (IS_NONE(arg)) return 0;
	if (IS_INTEGER(arg)) return (VAL_INT64(arg) != 0);
	if (IS_LOGIC(arg)) return (VAL_LOGIC(arg) != 0);
	if (IS_DECIMAL(arg) || IS_PERCENT(arg)) return (VAL_DECIMAL(arg) != 0.0);
	Trap_Arg(arg);
	DEAD_END;
}



#ifdef ndef
/***********************************************************************
**
*/	 REBCNT Get_Part_Length(REBVAL *bval, REBVAL *eval)
/*
**		Determine the length of a /PART value.
**		If /PART value is an integer just use it.
**		If it is a series and it is the same series as the first,
**		use the difference between the two indices.
**
**		If the length ends up negative, back up the index as much
**		as possible. If backed up over the head, adjust the length.
**
**		Note: This one does not handle list datatypes.
**
***********************************************************************/
{
	REBINT	len;
	REBCNT	tail;

	if (IS_INTEGER(eval) || IS_DECIMAL(eval)) {
		len = Int32(eval);
		if (IS_SCALAR(bval) && VAL_TYPE(bval) != REB_PORT)
			Trap1(RE_INVALID_PART, bval);
	}
	else if (
		(
			// IF normal series and self referencing:
			VAL_TYPE(eval) >= REB_STRING &&
			VAL_TYPE(eval) <= REB_BLOCK &&
			VAL_TYPE(bval) == VAL_TYPE(eval) &&
			VAL_SERIES(bval) == VAL_SERIES(eval)
		) || (
			// OR IF it is a port:
			IS_PORT(bval) && IS_PORT(eval) &&
			VAL_OBJ_FRAME(bval) == VAL_OBJ_FRAME(eval)
		)
	)
		len = (REBINT)VAL_INDEX(eval) - (REBINT)VAL_INDEX(bval);
	else
		Trap1(RE_INVALID_PART, eval);
/* !!!!
	if (IS_PORT(bval)) {
		PORT_STATE_OBJ	*port;

		port = VAL_PORT(&VAL_PSP(bval)->state);
		if (PORT_FLAG(port) & PF_DIRECT)
			tail = 0x7fffffff;
		else
			tail = PORT_TAIL(VAL_PORT(&VAL_PSP(bval)->state));
	}
	else
*/		tail = VAL_TAIL(bval);

	if (len < 0) {
		len = -len;
		if (len > (REBINT)VAL_INDEX(bval))
			len = (REBINT)VAL_INDEX(bval);
		VAL_INDEX(bval) -= (REBCNT)len;
	}
	else if (!IS_INTEGER(eval) && (len + VAL_INDEX(bval)) > tail)
		len = (REBINT)(tail - VAL_INDEX(bval));

	return (REBCNT)len;
}
#endif


/***********************************************************************
**
*/	 REBINT Partial1(REBVAL *sval, REBVAL *lval)
/*
**		Process the /part (or /skip) and other length modifying
**		arguments.
**
***********************************************************************/
{
	REBI64 len;
	REBINT maxlen;
	REBINT is_ser = ANY_SERIES(sval);

	// If lval = NONE, use the current len of the target value:
	if (IS_NONE(lval)) {
		if (!is_ser) return 1;
		if (VAL_INDEX(sval) >= VAL_TAIL(sval)) return 0;
		return (VAL_TAIL(sval) - VAL_INDEX(sval));
	}
	if (IS_INTEGER(lval) || IS_DECIMAL(lval)) len = Int32(lval);
	else {
		if (is_ser && VAL_TYPE(sval) == VAL_TYPE(lval) && VAL_SERIES(sval) == VAL_SERIES(lval))
			len = (REBINT)VAL_INDEX(lval) - (REBINT)VAL_INDEX(sval);
		else
			Trap1(RE_INVALID_PART, lval);

	}

	if (is_ser) {
		// Restrict length to the size available:
		if (len >= 0) {
			maxlen = (REBINT)VAL_LEN(sval);
			if (len > maxlen) len = maxlen;
		} else {
			len = -len;
			if (len > (REBINT)VAL_INDEX(sval)) len = (REBINT)VAL_INDEX(sval);
			VAL_INDEX(sval) -= (REBCNT)len;
		}
	}

	return (REBINT)len;
}


/***********************************************************************
**
*/	 REBINT Partial(REBVAL *aval, REBVAL *bval, REBVAL *lval, REBFLG flag)
/*
**		Args:
**			aval: target value
**			bval: argument to modify target (optional)
**			lval: length value (or none)
**
**		Determine the length of a /PART value. It can be:
**			1. integer or decimal
**			2. relative to A value (bval is null)
**			3. relative to B value
**
**		Flag: indicates special treatment for CHANGE. As in:
**			CHANGE/part "abcde" "xy" 3 => "xyde"
**
**		NOTE: Can modify the value's index!
**		The result can be negative. ???
**
***********************************************************************/
{
	REBVAL *val;
	REBINT len;
	REBINT maxlen;

	// If lval = NONE, use the current len of the target value:
	if (IS_NONE(lval)) {
		val = (bval && ANY_SERIES(bval)) ? bval : aval;
		if (VAL_INDEX(val) >= VAL_TAIL(val)) return 0;
		return (VAL_TAIL(val) - VAL_INDEX(val));
	}

	if (IS_INTEGER(lval)) {
		len = Int32(lval);
		val = flag ? aval : bval;
	}

	else if (IS_DECIMAL(lval)) {
		len = Int32(lval);
		val = bval;
	}

	else {
		// So, lval must be relative to aval or bval series:
		if (VAL_TYPE(aval) == VAL_TYPE(lval) && VAL_SERIES(aval) == VAL_SERIES(lval))
			val = aval;
		else if (bval && VAL_TYPE(bval) == VAL_TYPE(lval) && VAL_SERIES(bval) == VAL_SERIES(lval))
			val = bval;
		else
			Trap1(RE_INVALID_PART, lval);

		len = (REBINT)VAL_INDEX(lval) - (REBINT)VAL_INDEX(val);
	}

	if (!val) val = aval;

	// Restrict length to the size available:
	if (len >= 0) {
		maxlen = (REBINT)VAL_LEN(val);
		if (len > maxlen) len = maxlen;
	} else {
		len = -len;
		if (len > (REBINT)VAL_INDEX(val)) len = (REBINT)VAL_INDEX(val);
		VAL_INDEX(val) -= (REBCNT)len;
//		if ((-len) > (REBINT)VAL_INDEX(val)) len = -(REBINT)VAL_INDEX(val);
	}

	return len;
}


#ifdef ndef
/***********************************************************************
**
*/	void Define_File_Global(REBYTE *name, REBYTE *content)
/*
**		Util function used in startup.
**
***********************************************************************/
{
	REBCNT sym = Make_Word(name, 0);
	REBSER *str = Make_CStr(content);
	REBVAL *value;

	value = Append_Frame(Main_Context, 0, sym);
	SET_STR_TYPE(REB_FILE, value, str);
}

#endif

/***********************************************************************
**
*/	int Clip_Int(int val, int mini, int maxi)
/*
***********************************************************************/
{
	if (val < mini) val = mini;
	else if (val > maxi) val = maxi;
	return val;
}

/***********************************************************************
**
*/	void memswapl(void *m1, void *m2, size_t len)
/*
**		For long integer memory units, not chars. It is assumed that
**		the len is an exact modulo of long.
**
***********************************************************************/
{
	long t, *a, *b;

	a = m1;
	b = m2;
	len /= sizeof(long);
	while (len--) {
		t = *b;
		*b++ = *a;
		*a++ = t;
	}
}


/***********************************************************************
**
*/	i64 Add_Max(int type, i64 n, i64 m, i64 maxi)
/*
***********************************************************************/
{
	i64 r = n + m;
	if (r < -maxi || r > maxi) {
		if (type) Trap1(RE_TYPE_LIMIT, Get_Type(type));
		r = r > 0 ? maxi : -maxi;
	}
	return r;
}


/***********************************************************************
**
*/	int Mul_Max(int type, i64 n, i64 m, i64 maxi)
/*
***********************************************************************/
{
	i64 r = n * m;
	if (r < -maxi || r > maxi) Trap1(RE_TYPE_LIMIT, Get_Type(type));
	return (int)r;
}


/***********************************************************************
**
*/	REBVAL *Make_OS_Error()
/*
***********************************************************************/
{
	REBCHR str[100];

	OS_FORM_ERROR(0, str, 100);
	Set_String(DS_RETURN, Copy_OS_Str(str, LEN_STR(str)));
	return DS_RETURN;
}


/***********************************************************************
**
*/	REBSER *At_Head(REBVAL *value)
/*
**		Return the series for a value, but if it has an index
**		offset, return a copy of the series from that position.
**		Useful for functions that do not accept index offsets.
**
***********************************************************************/
{
	REBCNT len;
	REBSER *ser;
	REBSER *src = VAL_SERIES(value);
	REBCNT wide;

	if (VAL_INDEX(value) == 0) return src;

	len = VAL_LEN(value);
	wide = SERIES_WIDE(src);
	ser = Make_Series(len, wide, FALSE);

	memcpy(ser->data, src->data + (VAL_INDEX(value) * wide), len * wide);
	ser->tail = len;

	return ser;
}


/***********************************************************************
**
*/	REBSER *Collect_Set_Words(REBVAL *val)
/*
**		Scan a block, collecting all of its SET words as a block.
**
***********************************************************************/
{
	REBCNT cnt = 0;
	REBVAL *val2 = val;
	REBSER *ser;

	for (; NOT_END(val); val++) if (IS_SET_WORD(val)) cnt++;
	val = val2;

	ser = Make_Block(cnt);
	val2 = BLK_HEAD(ser);
	for (; NOT_END(val); val++) {
		if (IS_SET_WORD(val)) Init_Word(val2++, VAL_WORD_SYM(val));
	}
	SET_END(val2);
	SERIES_TAIL(ser) = cnt;

	return ser;
}


/***********************************************************************
**
*/	REBINT What_Reflector(REBVAL *word)
/*
***********************************************************************/
{
	if (IS_WORD(word)) {
		switch (VAL_WORD_SYM(word)) {
		case SYM_SPEC:   return OF_SPEC;
		case SYM_BODY:   return OF_BODY;
		case SYM_WORDS:  return OF_WORDS;
		case SYM_VALUES: return OF_VALUES;
		case SYM_TYPES:  return OF_TYPES;
		case SYM_TITLE:  return OF_TITLE;
		}
	}
	return 0;
}
