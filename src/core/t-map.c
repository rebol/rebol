/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2023 Rebol Open Source Developers
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

// Use following define to speed map creation a little bit as the key words
// will not be normalized to set-words and or words when using words-of

//#define DO_NOT_NORMALIZE_MAP_KEYS

// with this define you would get:
//	[a b:]     = words-of make map! [a 1 b: 2]
//	[a 1 b: 2] = body-of  make map! [a 1 b: 2]
//
// else:
//	[a b]       = words-of make map! [a 1 b: 2]
//	[a: 1 b: 2] = body-of  make map! [a 1 b: 2]


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
**		Hash series is also created.
**
***********************************************************************/
{
	REBSER *blk = Make_Block(size*2);
	REBSER *ser = 0;

	ser = Make_Hash_Array(size);

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
	//o: use fallback hash value, if key is not a hashable type, instead of an error
	//o: https://github.com/Oldes/Rebol-issues/issues/1765
	if (!hash) hash = 3 * (len/5); //Trap_Type(key);

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
			if (VAL_TYPE(val) == VAL_TYPE(key) && 0 == Cmp_Value(key, val, cased)) return hash;
			hash += skip;
			if (hash >= len) hash -= len;
		}
	}

	// Append new value the target series:
	if (mode > 1) {
		hashes[hash] = (SERIES_TAIL(series) / wide) +1;
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
		key = Find_Key(series, series->series, val, 2, TRUE, 0);
		hashes[key] = n/2+1;
	}
}


/***********************************************************************
**
*/	static REBCNT Find_Entry(REBSER *series, REBVAL *key, REBVAL *val, REBOOL cased)
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
	REBCNT n;
	REBVAL *set;

	if (IS_NONE(key) || hser == NULL) return 0;

	// Get hash table, expand it if needed:
	if (series->tail > hser->tail/2) {
		Expand_Hash(hser); // modifies size value
		Rehash_Hash(series);
	}

	hash = Find_Key(series, hser, key, 2, cased, 0);
	hashes = (REBCNT*)hser->data;
	n = hashes[hash];

	// Just a GET of value:
	if (!val) return n;

	// Must set the value:
	if (n) {  // re-set it:
		set = BLK_SKIP(series, ((n-1)*2)); // find the key
		VAL_CLR_OPT(set++, OPTS_HIDE);     // clear HIDE flag in case it was removed key; change to value position
		*set = *val;                       // set the value
		return n;
	}

	// Create new entry:
#ifndef DO_NOT_NORMALIZE_MAP_KEYS
	// append key
	if(ANY_WORD(key) && VAL_TYPE(key) != REB_SET_WORD) {
		// Normalize the KEY (word) to be a SET-WORD
		set = Append_Value(series);
		*set = *key;
		VAL_SET(set, REB_SET_WORD);
	} else if (ANY_BINSTR(key) && !IS_LOCK_SERIES(VAL_SERIES(key))) {
		// copy the key if it is any-series (except when it is permanently locked)
		set = Append_Value(series);
		VAL_SERIES(set) = Copy_String(VAL_SERIES(key), VAL_INDEX(key), VAL_LEN(key));
		VAL_TYPE(set) = VAL_TYPE(key);
	} else {
		Append_Val(series, key);
	}
#else
	if (ANY_BINSTR(key) && !IS_LOCK_SERIES(VAL_SERIES(key))) {
		// copy the key if it is any-series (except when it is permanently locked)
		set = Append_Value(series);
		VAL_SERIES(set) = Copy_String(VAL_SERIES(key), VAL_INDEX(key), VAL_LEN(key));
		VAL_TYPE(set) = VAL_TYPE(key);
	} else {
		Append_Val(series, key);
	}
#endif
	// append value
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
		if (!VAL_MAP_REMOVED(v)) c++; // count only not removed values
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

	if (pvs->setval) TRAP_PROTECT(VAL_SERIES(data));

	if (IS_END(pvs->path+1)) val = pvs->setval;
	if (IS_NONE(pvs->select)) return PE_NONE;

	// O: No type limit enymore!
	// O: https://github.com/Oldes/Rebol-issues/issues/2421
	//if (!ANY_WORD(pvs->select) && !ANY_BINSTR(pvs->select) &&
	//	!IS_INTEGER(pvs->select) && !IS_CHAR(pvs->select))
	//	return PE_BAD_SELECT;

	n = Find_Entry(VAL_SERIES(data), pvs->select, val, FALSE);

	if (!n) return PE_NONE;

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
	len >>= 1; // part must be number of key/value pairs
	for (n = 0; n < len && NOT_END(val) && NOT_END(val+1); val += 2, n++) {
		Find_Entry(ser, val, val+1, TRUE);
	}
}

/***********************************************************************
**
*/	REBSER *Copy_Map(REBVAL *val, REBU64 types)
/*
**		Copy given map.
**
***********************************************************************/
{
	REBSER *series;
	series = Make_Map(VAL_BLK_LEN(val) / 2);
	//COPY_BLK_PART(series, VAL_BLK_DATA(data), n);
	Append_Map(series, val, UNKNOWN);
	if (types != 0) Copy_Deep_Values(series, 0, SERIES_TAIL(series), types);
	Rehash_Hash(series);
	return series;
}

/***********************************************************************
**
*/	REBFLG MT_Map(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	REBCNT n;
//	REBSER *series;

	if (!IS_BLOCK(data) && !IS_MAP(data)) return FALSE;

	n = VAL_BLK_LEN(data);
	if (n & 1) return FALSE;

	Set_Series(REB_MAP, out, Copy_Map(data, type));

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
		if (!VAL_MAP_REMOVED(val)) cnt++; // must not be removed
	}

	// Copy entries to new block:
	blk = Make_Block(cnt * ((what == 0) ? 2 : 1));
	out = BLK_HEAD(blk);
	for (val = BLK_HEAD(mapser); NOT_END(val) && NOT_END(val+1); val += 2) {
		if (!VAL_MAP_REMOVED(val)) {
			if (what < 0) {
				// words-of
				*out++ = val[0];
#ifndef DO_NOT_NORMALIZE_MAP_KEYS
				if (ANY_WORD(val)) VAL_SET(out - 1, REB_WORD);
#endif
				continue;
			}
			else if (what == 0) {
				// body-of
				*out++ = val[0]; 
				// making unified output by forcing new-line for each key
				VAL_SET_LINE(out-1);
			}
			*out++ = val[1]; // values
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

	ser = Make_Hash_Array(size);
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
	REBINT n = 0;
	REBSER *series = VAL_SERIES(val);

	// Check must be in this order (to avoid checking a non-series value);
	if (action >= A_TAKE && action <= A_SORT && IS_PROTECT_SERIES(series))
		Trap0(RE_PROTECTED);

	switch (action) {

	case A_PICK:		// same as SELECT for MAP! datatype
	case A_SELECT:
	case A_FIND:
		n = Find_Entry(series, arg, 0, Find_Refines(ds, AM_SELECT_CASE) ? AM_FIND_CASE : 0);
		if (!n) return R_NONE;
		*D_RET = *VAL_BLK_SKIP(val, ((n-1)*2)+((action == A_FIND)?0:1));
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

	case A_PUT:
		Find_Entry(series, arg, D_ARG(3), Find_Refines(ds, AM_PUT_CASE) ? AM_FIND_CASE : 0);
		return R_ARG3;

	case A_POKE:  // CHECK all pokes!!! to be sure they check args now !!!
		Find_Entry(series, arg, D_ARG(3), FALSE);
		*D_RET = *D_ARG(3);
		break;

	case A_REMOVE:
		//O: throw an error if /part is used?
		n = Find_Entry(series, D_ARG(ARG_REMOVE_KEY_ARG), 0, TRUE);
		if (n) {
			n = (n-1)*2;
			VAL_SET_OPT(VAL_BLK_SKIP(val, n), OPTS_HIDE);
			VAL_SET(VAL_BLK_SKIP(val, n+1), REB_NONE); // set value to none (so the old one may be GCed)
		}
		return R_ARG1;

	case A_LENGTHQ:
		n = Length_Map(series);
		DS_RET_INT(n);
		break;

	case A_MAKE:
	case A_TO:
		// make map! [word val word val]
		if (IS_BLOCK(arg) || IS_PAREN(arg) || IS_MAP(arg)) {
		map_from_block:
			if (MT_Map(D_RET, arg, 0)) return R_RET;
			Trap_Arg(arg);
//		} else if (IS_NONE(arg)) {
//			n = 3; // just a start
		// make map! 10000
		} else if (IS_NUMBER(arg)) {
			if (action == A_TO) Trap_Arg(arg);
			n = Int32s(arg, 0);
		} else if (IS_OBJECT(arg)) {
			Set_Block(arg, Make_Object_Block(VAL_OBJ_FRAME(arg), 3));
			goto map_from_block;
		} else
			Trap_Make(REB_MAP, Of_Type(arg));
		// positive only
		series = Make_Map(n);
		Set_Series(REB_MAP, D_RET, series);
		break;

	case A_COPY: {
		REBU64 types = 0;
		if D_REF(ARG_COPY_TYPES) {
			arg = D_ARG(ARG_COPY_KINDS);
			if (IS_DATATYPE(arg)) types |= TYPESET(VAL_DATATYPE(arg));
			else types |= VAL_TYPESET(arg);
		}
		if (D_REF(ARG_COPY_DEEP)) {
			types |= CP_DEEP | (D_REF(ARG_COPY_TYPES) ? types : TS_DEEP_COPIED);
		}
		Set_Series(REB_MAP, D_RET, Copy_Map(val, types));
		break;
	}
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
