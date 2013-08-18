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

#define	SET_VECTOR(v,s) VAL_SERIES(v)=(s), VAL_INDEX(v)=0, VAL_SET(v, REB_VECTOR)

// Encoding Format:
//		stored in series->size for now
//		[d d d d   d d d d   0 0 0 0   t s b b]

// Encoding identifiers:
enum {
	VTSI08 = 0,
	VTSI16,
	VTSI32,
	VTSI64,

	VTUI08,
	VTUI16,
	VTUI32,
	VTUI64,

	VTSF08,		// not used
	VTSF16,		// not used
	VTSF32,
	VTSF64,
};

#define VECT_TYPE(s) ((s)->size & 0xff)

static REBCNT bit_sizes[4] = {8, 16, 32, 64};

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
		return (REBI64) ((char*)data)[n];

	case VTSI16:
		return (REBI64) ((short*)data)[n];

	case VTSI32:
		return (REBI64) ((long*)data)[n];

	case VTSI64:
		return (REBI64) ((i64*)data)[n];

	case VTUI08:
		return (REBU64) ((unsigned char*)data)[n];

	case VTUI16:
		return (REBU64) ((unsigned short*)data)[n];

	case VTUI32:
		return (REBU64) ((unsigned long*)data)[n];

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
		((char*)data)[n] = (char)i;
		break;

	case VTSI16:
		((short*)data)[n] = (short)i;
		break;

	case VTSI32:
		((long*)data)[n] = (long)i;
		break;

	case VTSI64:
		((i64*)data)[n] = (i64)i;
		break;

	case VTUI08:
		((unsigned char*)data)[n] = (unsigned char)i;
		break;

	case VTUI16:
		((unsigned short*)data)[n] = (unsigned short)i;
		break;

	case VTUI32:
		((unsigned long*)data)[n] = (unsigned long)i;
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
	REBCNT idx = VAL_INDEX(blk);
	REBCNT len = VAL_LEN(blk);
	REBVAL *val;
	REBCNT n = 0;
	REBCNT bits = VECT_TYPE(ser);
	REBI64 i = 0;
	REBDEC f = 0;

	if (IS_BLOCK(blk)) {
		val = VAL_BLK_DATA(blk);

		for (; NOT_END(val); val++) {
			if (IS_INTEGER(val)) {
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
		REBYTE *data = VAL_BIN_DATA(blk);
		for (; len > 0; len--, idx++) {
			set_vect(bits, ser->data, n++, (REBI64)(data[idx]), f);
		}
	}
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
	REBVAL *val;

	if (len > 0) {
		val = BLK_HEAD(ser);
		for (n = VAL_INDEX(vect); n < VAL_TAIL(vect); n++, val++) {
			VAL_SET(val, (type >= VTSF08) ? REB_DECIMAL : REB_INTEGER);
			VAL_INT64(val) = get_vect(type, data, n); // can be int or decimal
		}
	}

	SET_END(val);
	ser->tail = len;

	return ser;
}


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
	REBU64 i1;
	REBU64 i2;
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
**		size: size of array ?
**
***********************************************************************/
{
	REBCNT len;
	REBSER *ser;

	len = size * dims;
	if (len > 0x7fffffff) return 0;
	ser = Make_Series(len+1, bits/8, TRUE); // !!! can width help extend the len?
	LABEL_SERIES(ser, "make vector");
	CLEAR(ser->data, len*bits/8);
	ser->tail = len;  // !!! another way to do it?

	// Store info about the vector (could be moved to flags if necessary):
	switch (bits) {
	case  8: bits = 0; break;
	case 16: bits = 1; break;
	case 32: bits = 2; break;
	case 64: bits = 3; break;
	}
	ser->size = (dims << 8) | (type << 3) | (sign << 2) | bits;

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
	REBCNT size = 1;
	REBSER *vect;
	REBVAL *iblk = 0;

	// UNSIGNED
	if (IS_WORD(bp) && VAL_WORD_CANON(bp) == SYM_UNSIGNED) { 
		sign = 1;
		bp++;
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

	// SIZE
	if (IS_INTEGER(bp)) {
		size = Int32(bp);
		if (size < 0) return 0;
		bp++;
	}

	// Initial data:
	if (IS_BLOCK(bp) || IS_BINARY(bp)) {
		REBCNT len = VAL_LEN(bp);
		if (IS_BINARY(bp) && type == 1) return 0;
		if (len > size) size = len;
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
	REBINT n = Compare_Vector(a, b);  // needs to be expanded for equality
	if (mode >= 0) {
		return n == 0;
	}
	if (mode == -1) return n >= 0;
	return n > 0;
}


/***********************************************************************
**
*/	REBINT PD_Vector(REBPVS *pvs)
/*
***********************************************************************/
{
	REBSER *vect;
	REBINT n;
	REBINT dims;
	REBINT bits;
	REBYTE *vp;
	REBI64 i;
	REBDEC f;

	if (IS_INTEGER(pvs->select) || IS_DECIMAL(pvs->select))
		n = Int32(pvs->select);
	else return PE_BAD_SELECT;

	n += VAL_INDEX(pvs->value);
	vect = VAL_SERIES(pvs->value);
	vp   = vect->data;
	bits = VECT_TYPE(vect);
	dims = vect->size >> 8;

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

	if (IS_INTEGER(pvs->setval)) {
		i = VAL_INT64(pvs->setval);
		if (bits > VTUI64) f = (REBDEC)(i);
	}
	else if (IS_DECIMAL(pvs->setval)) {
		f = VAL_DECIMAL(pvs->setval);
		if (bits <= VTUI64) i = (REBINT)(f);
	}
	else return PE_BAD_SET;

	set_vect(bits, vp, n-1, i, f);

	return PE_OK;
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
	REBCNT size;
	REBSER *vect;
	REBSER *ser;

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
		ser = Copy_Series(vect);
		ser->size = vect->size; // attributes
		SET_VECTOR(value, ser);
		break;

	case A_RANDOM:
		if (D_REF(2) || D_REF(4)) Trap0(RE_BAD_REFINES); // /seed /only
		Shuffle_Vector(value, D_REF(3));
		return R_ARG1;

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
		Emit(mold, "N I I [", type+1, bit_sizes[bits & 3], len);
		if (len) New_Indented_Line(mold);
	}

	c = 0;
	for (; n < vect->tail; n++) {
		v.i = get_vect(bits, data, n);
		if (bits < VTSF08) {
			l = Emit_Integer(buf, v.i);
		} else {
			l = Emit_Decimal(buf, v.d, 0, '.', mold->digits);
		}
		Append_Bytes_Len(mold->series, buf, l);

		if ((++c > 7) && (n+1 < vect->tail)) {
			New_Indented_Line(mold);
			c = 0;
		}
		else
			Append_Byte(mold->series, ' '); 
	}

	if (len) mold->series->tail--; // remove final space

	if (molded) {
		if (len) New_Indented_Line(mold);
		Append_Byte(mold->series, ']');
		if (!GET_MOPT(mold, MOPT_MOLD_ALL)) {
			Append_Byte(mold->series, ']');
		}
		else {
			Post_Mold(value, mold);
		}
	}
}
