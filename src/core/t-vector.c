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
**  Module:  t-vector.c
**  Summary: vector datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

REBU64 f_to_u64(float n) {
	union {
		REBU64 u;
		REBDEC d;
	} t;
	t.d = n;
	return t.u;
}
		

REBU64 get_vect(REBCNT bits, REBYTE *data, REBCNT n)
{
	switch (bits) {
	case VTSI08:
		return (REBI64) ((i8*)data)[n];

	case VTSI16:
		return (REBI64) ((i16*)data)[n];

	case VTSI32:
		return (REBI64) ((i32*)data)[n];

	case VTSI64:
		return (REBI64) ((i64*)data)[n];

	case VTUI08:
		return (REBU64) ((u8*)data)[n];

	case VTUI16:
		return (REBU64) ((u16*)data)[n];

	case VTUI32:
		return (REBU64) ((u32*)data)[n];

	case VTUI64:
		return (REBU64) ((i64*)data)[n];

	case VTSF08:
	case VTSF16:
	case VTSF32:
		return f_to_u64(((float*)data)[n]);
	
	case VTSF64:
		return ((REBU64*)data)[n];
	}

	return 0;
}

void set_vect(REBCNT bits, REBYTE *data, REBCNT n, REBI64 i, REBDEC f) {
	switch (bits) {

	case VTSI08:
		((i8*)data)[n] = (i8)i;
		break;

	case VTSI16:
		((i16*)data)[n] = (i16)i;
		break;

	case VTSI32:
		((i32*)data)[n] = (i32)i;
		break;

	case VTSI64:
		((i64*)data)[n] = (i64)i;
		break;

	case VTUI08:
		((u8*)data)[n] = (u8)i;
		break;

	case VTUI16:
		((u16*)data)[n] = (u16)i;
		break;

	case VTUI32:
		((u32*)data)[n] = (u32)i;
		break;

	case VTUI64:
		((i64*)data)[n] = (u64)i;
		break;

	case VTSF08:
	case VTSF16:
	case VTSF32:
		((float*)data)[n] = (float)f;
		break;

	case VTSF64:
		((double*)data)[n] = f;
		break;
	}
}


void Set_Vector_Row(REBSER *ser, REBVAL *blk)
{
//	REBCNT idx = VAL_INDEX(blk);
	REBCNT len = VAL_LEN(blk);
	REBVAL *val;
	REBCNT n = 0;
	REBCNT bits = VECT_TYPE(ser);
	REBI64 i = 0;
	REBDEC f = 0;

	if (IS_BLOCK(blk)) {
		val = VAL_BLK_DATA(blk);

		for (; NOT_END(val); val++) {
			if (IS_INTEGER(val) || IS_CHAR(val)) {
				i = VAL_INT64(val);
				if (bits > VTUI64) f = (REBDEC)(i);
			}
			else if (IS_DECIMAL(val)) {
				f = VAL_DECIMAL(val);
				if (bits <= VTUI64) i = (REBINT)(f);
			}
			else Trap_Arg(val);
			//if (n >= ser->tail) Expand_Vector(ser);
			set_vect(bits, ser->data, n++, i, f);
		}
	}
	else {
#ifdef old_code
		REBYTE *data = VAL_BIN_DATA(blk);
		for (; len > 0; len--, idx++) {
			set_vect(bits, ser->data, n++, (REBI64)(data[idx]), f);
		}
#else
		REBCNT bytes = ser->tail * SERIES_WIDE(ser); //TODO: review! Wide is max 256 bytes!!!
		if (len > bytes) len = bytes;
		COPY_MEM(ser->data, VAL_BIN_DATA(blk), len);
#endif
	}
}


/***********************************************************************
**
*/	static REBOOL Query_Vector_Field(REBSER *vect, REBCNT field, REBVAL *ret)
/*
**		Set a value with file data according specified mode
**
***********************************************************************/
{
	switch (field) {
	case SYM_TYPE:
		Init_Word(ret, (VECT_TYPE(vect) >= VTSF08) ? SYM_DECIMAL_TYPE : SYM_INTEGER_TYPE);
		break;
	case SYM_SIZE:
		SET_INTEGER(ret, VECT_BIT_SIZE(VECT_TYPE(vect)));
		break;
	case SYM_LENGTH:
		SET_INTEGER(ret, vect->tail);
		break;
	case SYM_SIGNED:
		SET_LOGIC(ret, !(VECT_TYPE(vect) >= VTUI08 && VECT_TYPE(vect) <= VTUI64));
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


/***********************************************************************
**
*/	REBSER *Make_Vector_Block(REBVAL *vect)
/*
**		Convert a vector to a block.
**
***********************************************************************/
{
	REBCNT len = VAL_LEN(vect);
	REBYTE *data = VAL_SERIES(vect)->data;
	REBCNT type = VECT_TYPE(VAL_SERIES(vect));
	REBSER *ser = Make_Block(len);
	REBCNT n;
	REBVAL *val = NULL;

	if (len > 0) {
		val = BLK_HEAD(ser);
		for (n = VAL_INDEX(vect); n < VAL_TAIL(vect); n++, val++) {
			VAL_SET(val, (type >= VTSF08) ? REB_DECIMAL : REB_INTEGER);
			VAL_INT64(val) = get_vect(type, data, n); // can be int or decimal
		}
		SET_END(val);
	}

	ser->tail = len;

	return ser;
}

#ifndef EXCLUDE_VECTOR_MATH
/***********************************************************************
**
*/	REBVAL* Math_Op_Vector(REBVAL *v1, REBVAL *v2, REBCNT action)
/*
**		Do basic math operation on a vector
**
***********************************************************************/
{
	REBSER *vect = NULL;
	REBYTE *data;
	REBCNT bits;
	REBCNT len;

	REBVAL *left;
	REBVAL *right;

	REBI64 i = 0;
	REBDEC f = 0;
	REBCNT n = 0;

	if (IS_VECTOR(v1) && IS_NUMBER(v2)) {
		left = v1;
		right = v2;
	} else if (IS_VECTOR(v2) && IS_NUMBER(v1)) {
		left = v2;
		right = v1;
	} else {
		Trap_Action(VAL_TYPE(v1), action);
		return NULL;
	}
	vect = VAL_SERIES(left);
	bits = VECT_TYPE(vect);
	data = vect->data;

	if (IS_INTEGER(right)) {
		i = VAL_INT64(right);
		f = (REBDEC)i;
	} else {
		f = VAL_DECIMAL(right);
		i = (REBI64)f;
	}

	n = VAL_INDEX(left);
	len = n + VAL_LEN(left);

	switch (action) {
		case A_ADD:
			switch (bits) {
			case VTSI08: for (; n<len; n++) ( (i8*)data)[n] += ( i8)i; break;
			case VTSI16: for (; n<len; n++) ((i16*)data)[n] += (i16)i; break;
			case VTSI32: for (; n<len; n++) ((i32*)data)[n] += (i32)i; break;
			case VTSI64: for (; n<len; n++) ((i64*)data)[n] += (i64)i; break;
			case VTUI08: for (; n<len; n++) (( u8*)data)[n] += ( u8)i; break;
			case VTUI16: for (; n<len; n++)	((u16*)data)[n] += (u16)i; break;
			case VTUI32: for (; n<len; n++) ((u32*)data)[n] += (u32)i; break;
			case VTUI64: for (; n<len; n++)	((i64*)data)[n] += (u64)i; break;
			case VTSF08:
			case VTSF16:
			case VTSF32: for (; n<len; n++) (( float*)data)[n] += (float)f; break;
			case VTSF64: for (; n<len; n++) ((double*)data)[n] += f; break;
			}
			break;
		case A_SUBTRACT:
			switch (bits) {
			case VTSI08: for (; n<len; n++) (( i8*)data)[n] -= ( i8)i; break;
			case VTSI16: for (; n<len; n++) ((i16*)data)[n] -= (i16)i; break;
			case VTSI32: for (; n<len; n++) ((i32*)data)[n] -= (i32)i; break;
			case VTSI64: for (; n<len; n++) ((i64*)data)[n] -= (i64)i; break;
			case VTUI08: for (; n<len; n++) (( u8*)data)[n] -= ( u8)i; break;
			case VTUI16: for (; n<len; n++)	((u16*)data)[n] -= (u16)i; break;
			case VTUI32: for (; n<len; n++) ((u32*)data)[n] -= (u32)i; break;
			case VTUI64: for (; n<len; n++)	((i64*)data)[n] -= (u64)i; break;
			case VTSF08:
			case VTSF16:
			case VTSF32: for (; n<len; n++) (( float*)data)[n] -= (float)f; break;
			case VTSF64: for (; n<len; n++) ((double*)data)[n] -= f; break;
			}
			break;
		case A_MULTIPLY:
			switch (bits) {
			case VTSI08: for (; n<len; n++) (( i8*)data)[n] *= ( i8)i; break;
			case VTSI16: for (; n<len; n++) ((i16*)data)[n] *= (i16)i; break;
			case VTSI32: for (; n<len; n++) ((i32*)data)[n] *= (i32)i; break;
			case VTSI64: for (; n<len; n++) ((i64*)data)[n] *= (i64)i; break;
			case VTUI08: for (; n<len; n++) (( u8*)data)[n] *= ( u8)i; break;
			case VTUI16: for (; n<len; n++)	((u16*)data)[n] *= (u16)i; break;
			case VTUI32: for (; n<len; n++) ((u32*)data)[n] *= (u32)i; break;
			case VTUI64: for (; n<len; n++)	((i64*)data)[n] *= (u64)i; break;
			case VTSF08:
			case VTSF16:
			case VTSF32: for (; n<len; n++) (( float*)data)[n] *= (float)f; break;
			case VTSF64: for (; n<len; n++) ((double*)data)[n] *= f; break;
			}
			break;
		case A_DIVIDE:
			if (i == 0) Trap0(RE_ZERO_DIVIDE);
			switch (bits) {
			case VTSI08: for (; n<len; n++) (( i8*)data)[n] /= ( i8)i; break;
			case VTSI16: for (; n<len; n++) ((i16*)data)[n] /= (i16)i; break;
			case VTSI32: for (; n<len; n++) ((i32*)data)[n] /= (i32)i; break;
			case VTSI64: for (; n<len; n++) ((i64*)data)[n] /= (i64)i; break;
			case VTUI08: for (; n<len; n++) (( u8*)data)[n] /= ( u8)i; break;
			case VTUI16: for (; n<len; n++)	((u16*)data)[n] /= (u16)i; break;
			case VTUI32: for (; n<len; n++) ((u32*)data)[n] /= (u32)i; break;
			case VTUI64: for (; n<len; n++)	((i64*)data)[n] /= (u64)i; break;
			case VTSF08:
			case VTSF16:
			case VTSF32: for (; n<len; n++) (( float*)data)[n] /= (float)f; break;
			case VTSF64: for (; n<len; n++) ((double*)data)[n] /= f; break;
			}
			break;
	}
	return left;
}
#endif

/***********************************************************************
**
*/	REBINT Compare_Vector(REBVAL *v1, REBVAL *v2)
/*
***********************************************************************/
{
	REBCNT l1 = VAL_LEN(v1);
	REBCNT l2 = VAL_LEN(v2);
	REBCNT len = MIN(l1, l2);
	REBCNT n;
	REBU64 i1 = 0;
	REBU64 i2 = 0;
	REBYTE *d1 = VAL_SERIES(v1)->data;
	REBYTE *d2 = VAL_SERIES(v2)->data;
	REBCNT b1 = VECT_TYPE(VAL_SERIES(v1));
	REBCNT b2 = VECT_TYPE(VAL_SERIES(v2));

	if (
		(b1 >= VTSF08 && b2 < VTSF08)
		|| (b2 >= VTSF08 && b1 < VTSF08)
	) Trap0(RE_NOT_SAME_TYPE);

	for (n = 0; n < len; n++) {
		i1 = get_vect(b1, d1, n + VAL_INDEX(v1));
		i2 = get_vect(b2, d2, n + VAL_INDEX(v2));
		if (i1 != i2) break;
	}

	if (n != len) {
		if (i1 > i2) return 1;
		return -1;
	}

	return l1 - l2;
}


/***********************************************************************
**
*/	void Shuffle_Vector(REBVAL *vect, REBFLG secure)
/*
***********************************************************************/
{
	REBCNT n;
	REBCNT k;
	REBU64 swap;
	REBYTE *data = VAL_SERIES(vect)->data;
	REBCNT type = VECT_TYPE(VAL_SERIES(vect));
	REBCNT idx = VAL_INDEX(vect);

	// We can do it as INTS, because we just deal with the bits:
	if (type == VTSF32) type = VTUI32;
	else if (type == VTSF64) type = VTUI64;

	for (n = VAL_LEN(vect); n > 1;) {
		k = idx + (REBCNT)Random_Int(secure) % n;
		n--;
		swap = get_vect(type, data, k);
		set_vect(type, data, k, get_vect(type, data, n + idx), 0);
		set_vect(type, data, n + idx, swap, 0);
	}
}


/***********************************************************************
**
*/	void Set_Vector_Value(REBVAL *var, REBSER *series, REBCNT index)
/*
***********************************************************************/
{
	REBYTE *data = series->data;
	REBCNT bits = VECT_TYPE(series);

	var->data.integer = get_vect(bits, data, index);
	if (bits >= VTSF08) SET_TYPE(var, REB_DECIMAL);
	else SET_TYPE(var, REB_INTEGER);
}


/***********************************************************************
**
*/	REBSER *Make_Vector(REBINT type, REBINT sign, REBINT dims, REBINT bits, REBINT size)
/*
**		type: the datatype
**		sign: signed or unsigned
**		dims: number of dimensions
**		bits: number of bits per unit (8, 16, 32, 64)
**		size: number of values
**
***********************************************************************/
{
	REBCNT len;
	REBSER *ser;

	//printf("MAKE_VECTOR=> type: %i sign: %i dims: %i bits: %i size: %i\n", type, sign, dims, bits, size);

	len = size * dims;
	if (len > 0x7fffffff) return 0;
	ser = Make_Series(len+1, bits/8, TRUE); // !!! can width help extend the len?
	LABEL_SERIES(ser, "make vector");
	CLEAR(ser->data, len*bits/8);
	ser->tail = len;  // !!! another way to do it?

	// Store info about the vector (could be moved to flags if necessary):
	ser->size = (dims << 8) | (type << 3) | (sign << 2) | (bits == 64 ? 3 : bits >> 4); // there are only 2 bits to store the info

	return ser;
}

/***********************************************************************
**
*/	REBVAL *Make_Vector_Spec(REBVAL *bp, REBVAL *value)
/*
**	Make a vector from a block spec.
**
**     make vector! [integer! 32 100]
**     make vector! [decimal! 64 100]
**     make vector! [unsigned integer! 32]
**     Fields:
**          signed:     signed, unsigned
**    		datatypes:  integer, decimal
**    		dimensions: 1 - N
**    		bitsize:    1, 8, 16, 32, 64
**    		size:       integer units
**    		init:		block of values
**
***********************************************************************/
{
	REBINT type = -1; // 0 = int,    1 = float
	REBINT sign = -1; // 0 = signed, 1 = unsigned
	REBINT dims = 1;
	REBINT bits = 32;
	REBCNT size = 0;
	REBSER *vect;
	REBVAL *iblk = 0;

	// SIGNED / UNSIGNED
	if (IS_WORD(bp)) {
		switch (VAL_WORD_CANON(bp)) {
		case SYM_I8X:
		case SYM_INT8X:   type = 0; sign = 0; bits =  8; bp++; goto size_spec;
		case SYM_U8X:
		case SYM_BYTEX:
		case SYM_UINT8X:  type = 0; sign = 1; bits =  8; bp++; goto size_spec;
		case SYM_I16X:
		case SYM_INT16X:  type = 0; sign = 0; bits = 16; bp++; goto size_spec;
		case SYM_U16X:
		case SYM_UINT16X: type = 0; sign = 1; bits = 16; bp++; goto size_spec;
		case SYM_I32X:
		case SYM_INT32X:  type = 0; sign = 0; bits = 32; bp++; goto size_spec;
		case SYM_U32X:
		case SYM_UINT32X: type = 0; sign = 1; bits = 32; bp++; goto size_spec;
		case SYM_I64X:
		case SYM_INT64X:  type = 0; sign = 0; bits = 64; bp++; goto size_spec;
		case SYM_U64X:
		case SYM_UINT64X: type = 0; sign = 1; bits = 64; bp++; goto size_spec;
		case SYM_F32X:
		case SYM_FLOATX:  type = 1; sign = 0; bits = 32; bp++; goto size_spec;
		case SYM_F64X:
		case SYM_DOUBLEX: type = 1; sign = 0; bits = 64; bp++; goto size_spec;

		case SYM_UNSIGNED: sign = 1; bp++; break;
		case SYM_SIGNED:   sign = 0; bp++; break;
		}
	}

	// INTEGER! or DECIMAL!
	if (IS_WORD(bp)) {
		if (VAL_WORD_CANON(bp) == (REB_INTEGER+1)) // integer! symbol
			type = 0;
		else if (VAL_WORD_CANON(bp) == (REB_DECIMAL+1)) { // decimal! symbol
			type = 1;
			if (sign > 0) return 0;
		}
		else return 0;
		bp++;
	}

	if (type < 0) type = 0;
	if (sign < 0) sign = 0;

	// BITS
	if (IS_INTEGER(bp)) {
		bits = Int32(bp);
		if (
			(bits == 32 || bits == 64)
			||
			(type == 0 && (bits == 8 || bits == 16))
		) bp++;
		else return 0;
	} else return 0;

size_spec:

	// SIZE
	if (IS_INTEGER(bp)) {
		size = Int32(bp);
		if (size < 0) return 0;
		bp++;
	}

	// Initial data:
	if (IS_BLOCK(bp) || IS_BINARY(bp)) {
		REBCNT len = VAL_LEN(bp);
		if (IS_BINARY(bp)) len /= (bits >> 3);
		if (len > size && size == 0) size = len;
		iblk = bp;
		bp++;
	}

	// Index offset:
	if (IS_INTEGER(bp)) {
		VAL_INDEX(value) = (Int32s(bp, 1) - 1);
		bp++;
	}
	else VAL_INDEX(value) = 0;

	if (NOT_END(bp)) return 0;

	vect = Make_Vector(type, sign, dims, bits, size);
	if (!vect) return 0;

	if (iblk) Set_Vector_Row(vect, iblk);

	SET_TYPE(value, REB_VECTOR);
	VAL_SERIES(value) = vect;
	// index set earlier

	return value;
}


/***********************************************************************
**
*/	REBFLG MT_Vector(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	if (Make_Vector_Spec(data, out)) return TRUE;
	return FALSE;
}


/***********************************************************************
**
*/	REBINT CT_Vector(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	REBINT num;

	if (mode == 3)
		return VAL_SERIES(a) == VAL_SERIES(b) && VAL_INDEX(a) == VAL_INDEX(b);

	num = Compare_Vector(a, b);
	if (mode >= 0) return (num == 0);
	if (mode == -1) return (num >= 0);
	return (num > 0);
}


/***********************************************************************
**
*/	REBINT PD_Vector(REBPVS *pvs)
/*
***********************************************************************/
{
	REBVAL *sel = pvs->select;
	REBVAL *val = pvs->value;
	REBVAL *set = pvs->setval;
	REBSER *vect = VAL_SERIES(val);
	REBINT bits = VECT_TYPE(vect);
	REBINT n;
	//REBINT dims;
	
	REBYTE *vp;
	REBI64 i = 0;
	REBDEC f = 0.0;

	if (IS_INTEGER(sel) || IS_DECIMAL(sel)) {
		n = Int32(sel);
		if (n == 0) return (pvs->setval) ? PE_BAD_RANGE : PE_NONE; // allow PICK with zero index but not for POKE
		if (n < 0) n++;
	} else if (IS_WORD(sel)) {
		if (set == 0) {
			val = pvs->value = pvs->store;
			if(!Query_Vector_Field(vect, VAL_WORD_CANON(sel), val)) return PE_BAD_SELECT;
			return PE_OK;
		} else
			return PE_BAD_SET;
	} else  return PE_BAD_SELECT;

	n += VAL_INDEX(val);
	vect = VAL_SERIES(val);
	vp   = vect->data;
	
	//dims = vect->size >> 8;

	if (pvs->setval == 0) {

		// Check range:
		if (n <= 0 || (REBCNT)n > vect->tail) return PE_NONE;

		// Get element value:
		pvs->store->data.integer = get_vect(bits, vp, n-1); // 64 bits
		if (bits < VTSF08) {
			SET_TYPE(pvs->store, REB_INTEGER);
		} else {
			SET_TYPE(pvs->store, REB_DECIMAL);
		}

		return PE_USE;
	}

	//--- Set Value...
	TRAP_PROTECT(vect);

	if (n <= 0 || (REBCNT)n > vect->tail) return PE_BAD_RANGE;

	if (IS_INTEGER(set)) {
		i = VAL_INT64(set);
		if (bits > VTUI64) f = (REBDEC)(i);
	}
	else if (IS_DECIMAL(set)) {
		f = VAL_DECIMAL(set);
		if (bits <= VTUI64) i = (REBINT)(f);
	}
	else return PE_BAD_SET;

	set_vect(bits, vp, n-1, i, f);

	return PE_OK;
}


static void reverse_vector(REBVAL *value, REBCNT len)
{
	REBCNT n;
	REBCNT m;
	REBINT width = VAL_VEC_WIDTH(value);

	if (width == 1) {
		REBYTE *bp = VAL_BIN_DATA(value);
		REBYTE c1;
		for (n = 0, m = len-1; n < len / 2; n++, m--) {
			c1 = bp[n];
			bp[n] = bp[m];
			bp[m] = c1;
		}
	}
	else if (width == 2) {
		REBUNI *up = VAL_UNI_DATA(value);
		REBUNI c2;
		for (n = 0, m = len-1; n < len / 2; n++, m--) {
			c2 = up[n];
			up[n] = up[m];
			up[m] = c2;
		}
	}
	else if (width == 4) {
		REBCNT *i4 = (REBCNT*)VAL_DATA(value);
		REBCNT c4;
		for (n = 0, m = len-1; n < len / 2; n++, m--) {
			c4 = i4[n];
			i4[n] = i4[m];
			i4[m] = c4;
		}
	}
	else if (width == 8) {
		REBU64 *i8 = (REBU64*)VAL_DATA(value);
		REBU64 c8;
		for (n = 0, m = len-1; n < len / 2; n++, m--) {
			c8 = i8[n];
			i8[n] = i8[m];
			i8[m] = c8;
		}
	}
}


/***********************************************************************
**
*/	REBTYPE(Vector)
/*
***********************************************************************/
{
	REBVAL *value = D_ARG(1);
	REBVAL *arg = D_ARG(2);
	REBINT type;
	REBCNT size, bits;
	REBSER *vect;
	REBSER *ser;
	REBSER *blk;
	REBVAL *val;
	REBINT	len;

	type = Do_Series_Action(action, value, arg);
	if (type >= 0) return type;

	vect = VAL_SERIES(value); // not valid for MAKE or TO

	// Check must be in this order (to avoid checking a non-series value);
	if (action >= A_TAKE && action <= A_SORT && IS_PROTECT_SERIES(vect))
		Trap0(RE_PROTECTED);

	switch (action) {

	case A_PICK:
		Pick_Path(value, arg, 0);
		return R_TOS;

	case A_POKE:
		Pick_Path(value, arg, D_ARG(3));
		return R_ARG3;

#ifndef EXCLUDE_VECTOR_MATH
	case A_ADD:
	case A_SUBTRACT:
	case A_MULTIPLY:
	case A_DIVIDE:
		Math_Op_Vector(value, arg, action);
		break;
#endif

	case A_MAKE:
		// We only allow MAKE VECTOR! ...
		if (!IS_DATATYPE(value)) goto bad_make;

		// CASE: make vector! 100
		if (IS_INTEGER(arg) || IS_DECIMAL(arg)) {
			size = Int32s(arg, 0);
			if (size < 0) goto bad_make;
			ser = Make_Vector(0, 0, 1, 32, size);
			SET_VECTOR(value, ser);
			break;
		}
//		if (IS_NONE(arg)) {
//			ser = Make_Vector(0, 0, 1, 32, 0);
//			SET_VECTOR(value, ser);
//			break;
//		}
		// fall thru

	case A_TO:
		// CASE: make vector! [...]
		if (IS_BLOCK(arg) && Make_Vector_Spec(VAL_BLK_DATA(arg), value)) break;
		goto bad_make;

	case A_LENGTHQ:
		//bits = 1 << (vect->size & 3);
		SET_INTEGER(D_RET, vect->tail);
		return R_RET;

	case A_COPY:
		len = Partial(value, 0, D_ARG(3), 0); // Can modify value index.
		ser = Copy_Series_Part(vect, VAL_INDEX(value), len);
		ser->size = vect->size; // attributes
		SET_VECTOR(value, ser);
		break;

	case A_REVERSE:
		len = Partial(value, 0, D_ARG(3), 0);
		if (len > 0) reverse_vector(value, len);
		break;
			
	case A_RANDOM:
		if (D_REF(2) || D_REF(4)) Trap0(RE_BAD_REFINES); // /seed /only
		Shuffle_Vector(value, D_REF(3));
		return R_ARG1;

	case A_REFLECT:
		bits = VECT_TYPE(vect);
		if (SYM_SPEC == VAL_WORD_SYM(D_ARG(2))) {
			blk = Make_Block(4);
			if (bits >= VTUI08 && bits <= VTUI64) Init_Word(Append_Value(blk), SYM_UNSIGNED);
			Query_Vector_Field(vect, SYM_TYPE, Append_Value(blk));
			Query_Vector_Field(vect, SYM_SIZE, Append_Value(blk));
			Query_Vector_Field(vect, SYM_LENGTH, Append_Value(blk));
			Set_Series(REB_BLOCK, value, blk);
		} else {
			if(!Query_Vector_Field(vect, VAL_WORD_SYM(D_ARG(2)), value))
				Trap_Reflect(VAL_TYPE(value), D_ARG(2));
		}
		break;

	case A_QUERY:
		bits = VECT_TYPE(vect);
		REBVAL *spec = Get_System(SYS_STANDARD, STD_VECTOR_INFO);
		if (!IS_OBJECT(spec)) Trap_Arg(spec);
		if (D_REF(2)) { // query/mode refinement
			REBVAL *field = D_ARG(3);
			if(IS_WORD(field)) {
				if (!Query_Vector_Field(vect, VAL_WORD_SYM(field), value))
					Trap_Reflect(VAL_TYPE(value), field); // better error?
			}
			else if (IS_BLOCK(field)) {
				REBSER *values = Make_Block(2 * BLK_LEN(VAL_SERIES(field)));
				REBVAL *word = VAL_BLK_DATA(field);
				for (; NOT_END(word); word++) {
					if (ANY_WORD(word)) {
						if (IS_SET_WORD(word)) {
							// keep the set-word in result
							val = Append_Value(values);
							*val = *word;
							VAL_SET_LINE(val);
						}
						val = Append_Value(values);
						if (!Query_Vector_Field(vect, VAL_WORD_SYM(word), val))
							Trap1(RE_INVALID_ARG, word);
					}
					else  Trap1(RE_INVALID_ARG, word);
				}
				Set_Series(REB_BLOCK, value, values);
			}
			else {
				Set_Block(D_RET, Get_Object_Words(spec));
				return R_RET;
			}
		} else {
			REBSER *obj = CLONE_OBJECT(VAL_OBJ_FRAME(spec));
			Query_Vector_Field(vect, SYM_SIGNED, OFV(obj, STD_VECTOR_INFO_SIGNED));
			Query_Vector_Field(vect, SYM_TYPE,   OFV(obj, STD_VECTOR_INFO_TYPE));
			Query_Vector_Field(vect, SYM_SIZE,   OFV(obj, STD_VECTOR_INFO_SIZE));
			Query_Vector_Field(vect, SYM_LENGTH, OFV(obj, STD_VECTOR_INFO_LENGTH));
			SET_OBJECT(value, obj);
		}
		break;

	default:
		Trap_Action(VAL_TYPE(value), action);
	}

	*D_RET = *value;
	return R_RET;

bad_make:
	Trap_Make(REB_VECTOR, arg);
	DEAD_END;
}


/***********************************************************************
**
*/	void Mold_Vector(REBVAL *value, REB_MOLD *mold, REBFLG molded)
/*
***********************************************************************/
{
	REBSER *vect = VAL_SERIES(value);
	REBYTE *data = vect->data;
	REBCNT bits  = VECT_TYPE(vect);
//	REBCNT dims  = vect->size >> 8;
	REBCNT len;
	REBCNT n;
	REBCNT c;
	union {REBU64 i; REBDEC d;} v;
	REBYTE buf[32];
	REBYTE l;
	REBOOL indented = !GET_MOPT(mold, MOPT_INDENT);

	if (GET_MOPT(mold, MOPT_MOLD_ALL)) {
		len = VAL_TAIL(value);
		n = 0;
	} else {
		len = VAL_LEN(value);
		n = VAL_INDEX(value);
	}

	if (molded) {
		REBCNT type = (bits >= VTSF08) ? REB_DECIMAL : REB_INTEGER;
		Pre_Mold(value, mold);
		if (!GET_MOPT(mold, MOPT_MOLD_ALL)) Append_Byte(mold->series, '[');
		if (bits >= VTUI08 && bits <= VTUI64) Append_Bytes(mold->series, "unsigned ");
		Emit(mold, "N I I [", type+1, VECT_BIT_SIZE(bits), len);
		if (indented && len > 10) {
			mold->indent++;
			New_Indented_Line(mold);
		}
		CHECK_MOLD_LIMIT(mold, len);
	}

	c = 0;
	for (; n < vect->tail; n++) {
		if (MOLD_HAS_LIMIT(mold) && MOLD_OVER_LIMIT(mold)) return;
		v.i = get_vect(bits, data, n);
		if (bits < VTSF08) {
			l = Emit_Integer(buf, v.i);
		} else {
			l = Emit_Decimal(buf, v.d, 0, '.', mold->digits);
		}
		Append_Bytes_Len(mold->series, buf, l);
		if (indented && (++c > 9) && (n+1 < vect->tail)) {
			New_Indented_Line(mold);
			c = 0;
		}
		else
			Append_Byte(mold->series, ' '); 
	}

	if (len) mold->series->tail--; // remove final space

	if (molded) {
		if (indented && len > 10) {
			mold->indent--;
			New_Indented_Line(mold);
		}
		Append_Byte(mold->series, ']');
		if (!GET_MOPT(mold, MOPT_MOLD_ALL)) {
			Append_Byte(mold->series, ']');
		}
		else {
			Post_Mold(value, mold);
		}
	}
}
