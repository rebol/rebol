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
**  Module:  t-map.c
**  Summary: map datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/
/*
	A map is a SERIES that can also include a hash table for faster lookup.

	The hashing method used here is the same as that used for the
	REBOL symbol table, with the exception that this method must
	also store the value of the symbol (not just its word).

	The structure of the series header for a map is the	same as other
	series, except that the opt series field is	a pointer to a REBCNT
	series, the hash table.

	The hash table is an array of REBCNT integers that are index values
	into the map series. NOTE: They are one-based to avoid 0 which is an
	empty slot.

	Each value in the map consists of a word followed by its value.

	These functions are also used hashing SET operations (e.g. UNION).

	The series/tail / 2 is the number of values stored.

	The hash-series/tail is a prime number that is use for computing
	slots in the hash table.
*/

#include "sys-core.h"

#define MIN_DICT 8 // size to switch to hashing


/***********************************************************************
**
*/	REBINT CT_Map(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	if (mode < 0) return -1;
	if (mode == 3) return VAL_SERIES(a) == VAL_SERIES(b);
	return 0 == Cmp_Block(a, b, 0);
}


/***********************************************************************
**
*/	static REBSER *Make_Map(REBINT size)
/*
**		Makes a MAP block (that holds both keys and values).
**		Size is the number of key-value pairs.
**		If size >= MIN_DICT, then a hash series is also created.
**
***********************************************************************/
{
	REBSER *blk = Make_Block(size*2);
	REBSER *ser = 0;

	if (size >= MIN_DICT) ser = Make_Hash_Array(size);

	blk->series = ser;

	return blk;
}


/***********************************************************************
**
*/	REBINT Find_Key(REBSER *series, REBSER *hser, REBVAL *key, REBINT wide, REBCNT cased, REBYTE mode)
/*
**		Returns hash index (either the match or the new one).
**		A return of zero is valid (as a hash index);
**
**		Wide: width of record (normally 2, a key and a value).
**
**		Modes:
**			0 - search, return hash if found or not
**			1 - search, return hash, else return -1 if not
**			2 - search, return hash, else append value and return -1
**
***********************************************************************/
{
	REBCNT *hashes;
	REBCNT skip;
	REBCNT hash;
	REBCNT len;
	REBCNT n;
	REBVAL *val;

	// Compute hash for value:
	len = hser->tail;
	hash = Hash_Value(key, len);
	if (!hash) Trap_Type(key);

	// Determine skip and first index:
	skip  = (len == 0) ? 0 : (hash & 0x0000FFFF) % len;
	if (skip == 0) skip = 1;
	hash = (len == 0) ? 0 : (hash & 0x00FFFF00) % len;

	// Scan hash table for match:
	hashes = (REBCNT*)hser->data;
	if (ANY_WORD(key)) {
		while (NZ(n = hashes[hash])) {
			val = BLK_SKIP(series, (n-1) * wide);
			if (
				ANY_WORD(val) &&
				(VAL_WORD_SYM(key) == VAL_BIND_SYM(val) ||
				(!cased && VAL_WORD_CANON(key) == VAL_BIND_CANON(val)))
			) return hash;
			hash += skip;
			if (hash >= len) hash -= len;
		}
	}
	else if (ANY_BINSTR(key)) {
		while (NZ(n = hashes[hash])) {
			val = BLK_SKIP(series, (n-1) * wide);
			if (
				VAL_TYPE(val) == VAL_TYPE(key)
				&& 0 == Compare_String_Vals(key, val, (REBOOL)(!IS_BINARY(key) && !cased))
			) return hash;
			hash += skip;
			if (hash >= len) hash -= len;
		}
	} else {
		while (NZ(n = hashes[hash])) {
			val = BLK_SKIP(series, (n-1) * wide);
			if (VAL_TYPE(val) == VAL_TYPE(key) && 0 == Cmp_Value(key, val, !cased)) return hash;
			hash += skip;
			if (hash >= len) hash -= len;
		}
	}

	// Append new value the target series:
	if (mode > 1) {
		hashes[hash] = SERIES_TAIL(series)+1;
		//Debug_Num("hash:", hashes[hash]);
		Append_Series(series, (REBYTE*)key, wide);
		//Dump_Series(series, "hash");
	}

	return (mode > 0) ? -1 : hash;
}


/***********************************************************************
**
*/	static void Rehash_Hash(REBSER *series)
/*
**		Recompute the entire hash table. Table must be large enough.
**
***********************************************************************/
{
	REBVAL *val;
	REBCNT n;
	REBCNT key;
	REBCNT *hashes;

	if (!series->series) return;

	hashes = (REBCNT*)(series->series->data);

	val = BLK_HEAD(series);
	for (n = 0; n < series->tail; n += 2, val += 2) {
		key = Find_Key(series, series->series, val, 2, 0, 0);
		hashes[key] = n/2+1;
	}
}


/***********************************************************************
**
*/	static REBCNT Find_Entry(REBSER *series, REBVAL *key, REBVAL *val)
/*
**		Try to find the entry in the map. If not found
**		and val is SET, create the entry and store the key and
**		val.
**
**		RETURNS: the index to the VALUE or zero if there is none.
**
***********************************************************************/
{
	REBSER *hser = series->series; // can be null
	REBCNT *hashes;
	REBCNT hash;
	REBVAL *v;
	REBCNT n;

	if (IS_NONE(key)) return 0;

	// We may not be large enough yet for the hash table to
	// be worthwhile, so just do a linear search:
	if (!hser) {
		if (series->tail < MIN_DICT*2) {
			v = BLK_HEAD(series);
			if (ANY_WORD(key)) {
				for (n = 0; n < series->tail; n += 2, v += 2) {
					if (ANY_WORD(v) && SAME_SYM(key, v)) {
						if (val) *++v = *val;
						return n/2+1;
					}
				}
			}
			else if (ANY_BINSTR(key)) {
				for (n = 0; n < series->tail; n += 2, v += 2) {
					if (VAL_TYPE(key) == VAL_TYPE(v) && 0 == Compare_String_Vals(key, v, (REBOOL)!IS_BINARY(v))) {
						if (val) {
							*++v = *val;
//							VAL_SERIES(v) = Copy_Series_Value(val);
//							VAL_INDEX(v) = 0;
						}
						return n/2+1;
					}
				}
			}
			else if (IS_INTEGER(key)) {
				for (n = 0; n < series->tail; n += 2, v += 2) {
					if (IS_INTEGER(v) && VAL_INT64(key) == VAL_INT64(v)) {
						if (val) *++v = *val;
						return n/2+1;
					}
				}
			}
			else if (IS_CHAR(key)) {
				for (n = 0; n < series->tail; n += 2, v += 2) {
					if (IS_CHAR(v) && VAL_CHAR(key) == VAL_CHAR(v)) {
						if (val) *++v = *val;
						return n/2+1;
					}
				}
			}
			else Trap_Type(key);

			if (!val) return 0;
			Append_Val(series, key);
			Append_Val(series, val); // no Copy_Series_Value(val) on strings
			return series->tail/2;
		}

		// Add hash table:
		//Print("hash added %d", series->tail);
		series->series = hser = Make_Hash_Array(series->tail);
		Rehash_Hash(series);
	}

	// Get hash table, expand it if needed:
	if (series->tail > hser->tail/2) {
		Expand_Hash(hser); // modifies size value
		Rehash_Hash(series);
	}

	hash = Find_Key(series, hser, key, 2, 0, 0);
	hashes = (REBCNT*)hser->data;
	n = hashes[hash];

	// Just a GET of value:
	if (!val) return n;

	// Must set the value:
	if (n) {  // re-set it:
		*BLK_SKIP(series, ((n-1)*2)+1) = *val; // set it
		return n;
	}

	// Create new entry:
	Append_Val(series, key);
	Append_Val(series, val);  // no Copy_Series_Value(val) on strings

	return (hashes[hash] = series->tail/2);
}


/***********************************************************************
**
*/	REBINT Length_Map(REBSER *series)
/*
***********************************************************************/
{
	REBCNT n, c = 0;
	REBVAL *v = BLK_HEAD(series);

	for (n = 0; n < series->tail; n += 2, v += 2) {
		if (!IS_NONE(v+1)) c++; // must have non-none value
	}

	return c;
}


/***********************************************************************
**
*/	REBINT PD_Map(REBPVS *pvs)
/*
***********************************************************************/
{
	REBVAL *data = pvs->value;
	REBVAL *val = 0;
	REBINT n = 0;

	if (IS_END(pvs->path+1)) val = pvs->setval;
	if (IS_NONE(pvs->select)) return PE_NONE;

	if (!ANY_WORD(pvs->select) && !ANY_BINSTR(pvs->select) &&
		!IS_INTEGER(pvs->select) && !IS_CHAR(pvs->select))
		return PE_BAD_SELECT;

	n = Find_Entry(VAL_SERIES(data), pvs->select, val);

	if (!n) return PE_NONE;

	TRAP_PROTECT(VAL_SERIES(data));
	pvs->value = VAL_BLK_SKIP(data, ((n-1)*2)+1);
	return PE_OK;
}


/***********************************************************************
**
*/	static void Append_Map(REBSER *ser, REBVAL *arg, REBCNT len)
/*
***********************************************************************/
{
	REBVAL *val;
	REBCNT n;

	val = VAL_BLK_DATA(arg);
	for (n = 0; n < len && NOT_END(val) && NOT_END(val+1); val += 2, n += 2) {
		Find_Entry(ser, val, val+1);
	}
}


/***********************************************************************
**
*/	REBFLG MT_Map(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	REBCNT n;
	REBSER *series;

	if (!IS_BLOCK(data) && !IS_MAP(data)) return FALSE;

	n = VAL_BLK_LEN(data);
	if (n & 1) return FALSE;

	series = Make_Map(n/2);

	//COPY_BLK_PART(series, VAL_BLK_DATA(data), n);
	Append_Map(series, data, UNKNOWN);

	Rehash_Hash(series);

	Set_Series(REB_MAP, out, series);

	return TRUE;
}


/***********************************************************************
**
*/	REBSER *Map_To_Block(REBSER *mapser, REBINT what)
/*
**		mapser = series of the map
**		what: -1 - words, +1 - values, 0 -both
**
***********************************************************************/
{
	REBVAL *val;
	REBCNT cnt = 0;
	REBSER *blk;
	REBVAL *out;

	// Count number of set entries:
	for (val = BLK_HEAD(mapser); NOT_END(val) && NOT_END(val+1); val += 2) {
		if (!IS_NONE(val+1)) cnt++; // must have non-none value
	}

	// Copy entries to new block:
	blk = Make_Block(cnt * ((what == 0) ? 2 : 1));
	out = BLK_HEAD(blk);
	for (val = BLK_HEAD(mapser); NOT_END(val) && NOT_END(val+1); val += 2) {
		if (!IS_NONE(val+1)) {
			if (what <= 0) *out++ = val[0];
			if (what >= 0) *out++ = val[1];
		}
	}

	SET_END(out);
	blk->tail = out - BLK_HEAD(blk);
	return blk;
}


/***********************************************************************
**
*/	void Block_As_Map(REBSER *blk)
/*
**		Convert existing block to a map.
**
***********************************************************************/
{
	REBSER *ser = 0;
	REBCNT size = SERIES_TAIL(blk);

	if (size >= MIN_DICT) ser = Make_Hash_Array(size);
	blk->series = ser;
	Rehash_Hash(blk);
}


/***********************************************************************
**
*/	REBSER *Map_To_Object(REBSER *mapser)
/*
***********************************************************************/
{
	REBVAL *val;
	REBCNT cnt = 0;
	REBSER *frame;
	REBVAL *word;
	REBVAL *mval;

	// Count number of set entries:
	for (mval = BLK_HEAD(mapser); NOT_END(mval) && NOT_END(mval+1); mval += 2) {
		if (ANY_WORD(mval) && !IS_NONE(mval+1)) cnt++;
	}

	// See Make_Frame() - cannot use it directly because no Collect_Words
	frame = Make_Frame(cnt);

	word = FRM_WORD(frame, 1);
	val  = FRM_VALUE(frame, 1);
	for (mval = BLK_HEAD(mapser); NOT_END(mval) && NOT_END(mval+1); mval += 2) {
		if (ANY_WORD(mval) && !IS_NONE(mval+1)) {
			VAL_SET(word, REB_SET_WORD);
			VAL_SET_OPT(word, OPTS_UNWORD);
			VAL_BIND_SYM(word) = VAL_WORD_SYM(mval);
			VAL_BIND_TYPESET(word) = ~((TYPESET(REB_END) | TYPESET(REB_UNSET))); // not END or UNSET
			word++;
			*val++ = mval[1];
		}
	}

	SET_END(word);
	SET_END(val);
	FRM_WORD_SERIES(frame)->tail = frame->tail = cnt + 1;

	return frame;
}


/***********************************************************************
**
*/	REBTYPE(Map)
/*
***********************************************************************/
{
	REBVAL *val = D_ARG(1);
	REBVAL *arg = D_ARG(2);
	REBINT n;
	REBSER *series = VAL_SERIES(val);

	// Check must be in this order (to avoid checking a non-series value);
	if (action >= A_TAKE && action <= A_SORT && IS_PROTECT_SERIES(series))
		Trap0(RE_PROTECTED);

	switch (action) {

	case A_PICK:		// same as SELECT for MAP! datatype
	case A_SELECT:
		n = Find_Entry(series, arg, 0);
		if (!n) return R_NONE;
		*D_RET = *VAL_BLK_SKIP(val, ((n-1)*2)+1);
		break;

	case A_INSERT:
	case A_APPEND:
		if (!IS_BLOCK(arg)) Trap_Arg(val);
		*D_RET = *val;
		if (DS_REF(AN_DUP)) {
			n = Int32(DS_ARG(AN_COUNT));
			if (n <= 0) break;
		}
		Append_Map(series, arg, Partial1(arg, D_ARG(AN_LENGTH)));
		break;

	case A_POKE:  // CHECK all pokes!!! to be sure they check args now !!!
		n = Find_Entry(series, arg, D_ARG(3));
		*D_RET = *D_ARG(3);
		break;

	case A_LENGTHQ:
		n = Length_Map(series);
		DS_RET_INT(n);
		break;

	case A_MAKE:
	case A_TO:
		// make map! [word val word val]
		if (IS_BLOCK(arg) || IS_PAREN(arg) || IS_MAP(arg)) {
			if (MT_Map(D_RET, arg, 0)) return R_RET;
			Trap_Arg(arg);
//		} else if (IS_NONE(arg)) {
//			n = 3; // just a start
		// make map! 10000
		} else if (IS_NUMBER(arg)) {
			if (action == A_TO) Trap_Arg(arg);
			n = Int32s(arg, 0);
		} else
			Trap_Make(REB_MAP, Of_Type(arg));
		// positive only
		series = Make_Map(n);
		Set_Series(REB_MAP, D_RET, series);
		break;

	case A_COPY:
		if (MT_Map(D_RET, val, 0)) return R_RET;
		Trap_Arg(val);

	case A_CLEAR:
		Clear_Series(series);
		if (series->series) Clear_Series(series->series);
		Set_Series(REB_MAP, D_RET, series);
		break;

	case A_REFLECT:
		action = What_Reflector(arg); // zero on error
		// Adjust for compatibility with PICK:
		if (action == OF_VALUES) n = 1;
		else if (action == OF_WORDS) n = -1;
		else if (action == OF_BODY) n = 0;
		else Trap_Reflect(REB_MAP, arg);
		series = Map_To_Block(series, n);
		Set_Block(D_RET, series);
		break;

	case A_TAILQ:
		return (Length_Map(series) == 0) ? R_TRUE : R_FALSE;

	default:
		Trap_Action(REB_MAP, action);
	}

	return R_RET;
}
