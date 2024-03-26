/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2024 Rebol Open Source Contributors
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
**  Module:  n-hash.c
**  Summary: hashing values
**  Section: natives
**  Author:  Oldes
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "sys-xxhash.h"

//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#define MURMUR_HASH_3_X86_32_C1		0xcc9e2d51
#define MURMUR_HASH_3_X86_32_C2		0x1b873593

//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here
FORCE_INLINE REBCNT getblock32(const REBCNT* p, int i) {
	return p[i];
}
/*-----------------------------------------------------------------------------
// Block mix - mix the key block, combine with hash block, mix the hash block,
// repeat. */

FORCE_INLINE void bmix(REBCNT* h1, REBCNT* k1)
{
	*k1 *= MURMUR_HASH_3_X86_32_C1;
	*k1 = ROTL32(*k1, 15);
	*k1 *= MURMUR_HASH_3_X86_32_C2;
	*h1 ^= *k1;
	*h1 = ROTL32(*h1, 13); *h1 = *h1 * 5 + 0xe6546b64;
}
//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche
FORCE_INLINE REBCNT fmix32(REBCNT h) {
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}
//-----------------------------------------------------------------------------
REBCNT MurmurHash3_x86_32(const void* key, int len, REBCNT seed)
{
	const REBYTE* data = (const REBYTE*)key;
	const int nblocks = len / 4;

	REBCNT h1 = seed;

	//----------
	// body
	const REBCNT* blocks = (const REBCNT*)(data + nblocks * 4);

	for (int i = -nblocks; i; i++)
	{
		REBCNT k1 = getblock32(blocks, i);
		bmix(&h1, &k1);
	}

	//----------
	// tail
	const REBYTE* tail = (const REBYTE*)(data + nblocks * 4);

	REBCNT k1 = 0;

	switch (len & 3) {
	case 3: k1 ^= tail[2] << 16;
	case 2: k1 ^= tail[1] << 8;
	case 1: k1 ^= tail[0];
		k1 *= MURMUR_HASH_3_X86_32_C1;
		k1  = ROTL32(k1, 16);
		k1 *= MURMUR_HASH_3_X86_32_C2;
		h1 ^= k1;
	};

	//----------
	// finalization
	h1 ^= len;
	h1 = fmix32(h1);

	return h1;
}


/***********************************************************************
**
*/	REBNATIVE(hash)
/*
//	hash: native [
//		{Computes a hash value from any Rebol value. This number may change between different Rebol versions!}
//		value [any-type!]
//	]
***********************************************************************/
{
	DS_RET_INT(Hash_Value(D_ARG(1)) & 0xffffffff);
	return R_RET;
}

FORCE_INLINE
/***********************************************************************
**
*/	REBCNT Hash_Probe(REBCNT hash, REBCNT idx, REBCNT len)
/*
**		Calculates the index based on a quadratic probing approach.
**
**
***********************************************************************/
{
	//return ((hash % len) + idx) % len; // linear
	return ((hash % len) + 1 * idx + 3 * idx * idx) % len; // quadratric
	//return ((hash % len) + idx * (1 + (hash % (len - 1)))) % len; // double hashing
}

/***********************************************************************
**
*/	REBCNT Hash_Value(REBVAL* val)
/*
**		Return a case insensitive hash value for any Rebol value.
**
**
***********************************************************************/
{
	REBCNT ret;
	if (ANY_WORD(val))
		return VAL_WORD_CANON(val); //plain value seems to be faster than: fmix32(VAL_WORD_CANON(val));
	if (ANY_STR(val))
		return Hash_String_Value(val) ^ VAL_TYPE(val);
	if (ANY_BLOCK(val))
		return Hash_Block_Value(val);

	switch (VAL_TYPE(val)) {
	case REB_BINARY:
		return Hash_Binary(VAL_BIN_DATA(val), VAL_BIN_LEN(val));

	case REB_LOGIC:
		return 2 + VAL_LOGIC(val);

	case REB_INTEGER:
	case REB_DECIMAL: // depends on INT64 sharing the DEC64 bits
		return fmix32((REBCNT)(VAL_INT64(val) >> 32) ^ ((REBCNT)VAL_INT64(val)));

	case REB_CHAR: // hashed always as lowercased
		if ((ret = VAL_CHAR(val)) < UNICODE_CASES) ret = LO_CASE(ret);
		return ret = (((REBCNT)ret * 506832829L) >> 18);

	case REB_MONEY:
		return VAL_ALL_BITS(val)[0] ^ VAL_ALL_BITS(val)[1] ^ VAL_ALL_BITS(val)[2];

	case REB_TIME:
	case REB_DATE:
		ret = (REBCNT)(VAL_TIME(val) ^ (VAL_TIME(val) / SEC_SEC));
		if (IS_DATE(val)) ret ^= VAL_DATE(val).bits;
		return ret;

	case REB_TUPLE:
		if (VAL_TUPLE_LEN(val) <= 4)
			return fmix32(VAL_UNT32(val));
		// else..
		return Hash_Binary(VAL_TUPLE(val), VAL_TUPLE_LEN(val));

	case REB_PAIR:
		return VAL_ALL_BITS(val)[0] ^ VAL_ALL_BITS(val)[1];

	case REB_DATATYPE:
		return CRC_Word(Get_Sym_Name(VAL_DATATYPE(val) + 1), UNKNOWN);

//	case REB_HANDLE:
//		return (REBCNT)VAL_HANDLE_I32(val); //TODO: this is not valid for context handles!
	}
	// all other types... 
	return fmix32(VAL_TYPE(val));
}

/***********************************************************************
**
*/	REBCNT Hash_Block_Value(REBVAL *block)
/*
**		Return a case sensitive hash value for the block.
**
***********************************************************************/
{
	REBCNT k1, h1 = 0;
	REBVAL *data = VAL_BLK_DATA(block);
	REBLEN len = VAL_LEN(block);

	k1 = VAL_TYPE(block);
	bmix(&h1, &k1);

	while (NZ(VAL_TYPE(data))) {
		k1 = Hash_Value(data++);
		bmix(&h1, &k1);
	}
	h1 ^= len;
	h1 = fmix32(h1);

	return h1;
}

/***********************************************************************
**
*/	REBCNT Hash_String_Value(REBVAL* val)
/*
**		Return a case insensitive hash value for the string value.
**
***********************************************************************/
{
	REBYTE* bin;
	REBUNI* uni;
	REBLEN  len;
	REBCNT  n, ch;
	REBCNT  hash = 0;

	if (BYTE_SIZE(VAL_SERIES(val))) {
		bin = VAL_BIN_DATA(val);
		len = VAL_TAIL(val) - VAL_INDEX(val);
		for (n = 0; n < len; n++) {
			ch = (REBCNT)LO_CASE(*bin++);
			bmix(&hash, &ch);
		}
	}
	else {
		uni = VAL_UNI_DATA(val);
		len = Val_Series_Len(val);
		for (n = 0; n < len; n++) {
			ch = (REBCNT)(*uni++);
			if (ch < UNICODE_CASES)
				ch = (REBCNT)LO_CASE(ch);
			bmix(&hash, &ch);
		}
	}
	hash ^= len;
	hash = fmix32(hash);
	return hash;
}

FORCE_INLINE
/***********************************************************************
**
*/	REBCNT Hash_Binary(const REBYTE* bin, REBCNT len)
/*
**		Return a case sensitive hash value for the binary.
**
***********************************************************************/
{
	return MurmurHash3_x86_32(bin, len, 0);
	//return XXH64(bin, len, 0);

}


/***********************************************************************
**
*/	REBCNT Hash_Word(const REBYTE* str, REBLEN len)
/*
**		Return a case insensitive hash value for the UTF-8 encoded string.
**
***********************************************************************/
{
	REBCNT ch;
	REBCNT hash=0;

	if (len == UNKNOWN) len = LEN_BYTES(str);

	for (; len > 0; str++, len--) {
		ch = *str;
		if (ch > 127) {
			ch = Decode_UTF8_Char(&str, &len); // mods str, ulen
			if (!ch) Trap0(RE_INVALID_CHARS);
		}
		if (ch < UNICODE_CASES) ch = LO_CASE(ch);
		bmix(&hash, &ch);
	}
	hash ^= len;
	hash = fmix32(hash);
	return hash;
}

/***********************************************************************
**
*/	REBSER* Make_Hash_Array(REBCNT len)
/*
***********************************************************************/
{
	REBCNT n;
	REBSER* ser;

	n = Get_Hash_Prime(len * 2); // best when 2X # of keys
	if (!n) Trap_Num(RE_SIZE_LIMIT, len);

	ser = Make_Series(n + 1, sizeof(REBCNT), FALSE);
	LABEL_SERIES(ser, "make hash array");
	Clear_Series(ser);
	ser->tail = n;

	return ser;
}


/***********************************************************************
**
*/	REBSER* Hash_Block(REBVAL* block, REBCNT cased)
/*
**		Hash ALL values of a block. Return hash array series.
**		Used for SET logic (unique, union, etc.)
**
**		Note: hash array contents (indexes) are 1-based!
**
***********************************************************************/
{
	REBCNT n;
	REBCNT key;
	REBSER* hser;
	REBCNT* hashes;
	REBSER* series = VAL_SERIES(block);

	// Create the hash array (integer indexes):
	hser = Make_Hash_Array(VAL_LEN(block));
	hashes = (REBCNT*)hser->data;

	for (n = VAL_INDEX(block); n < series->tail; n++) {
		key = Find_Key(series, hser, BLK_SKIP(series, n), 1, cased, 0);
		hashes[key] = n + 1;
	}

	return hser;
}

