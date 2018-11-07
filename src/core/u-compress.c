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
**  Module:  u-compress.c
**  Summary: interface to zlib and or optional lzma compression
**  Section: utility
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "sys-zlib.h"
#ifdef USE_LZMA
#include "sys-lzma.h"
#endif // USE_LZMA



/*
 *  This number represents the top file size that,
 *  if the data is random, will produce a larger output
 *  file than input.  The number is really a bit smaller
 *  but we like to be safe. -- SN
 */
#define STERLINGS_MAGIC_NUMBER      10000

/*
 *  This number represents the largest that a small file that expands
 *  on compression can expand.  The actual value is closer to
 *  500 bytes but why take chances? -- SN
 */
#define STERLINGS_MAGIC_FIX         1024

/*
 *  The why_compress_constant is here to satisfy the condition that
 *  somebody might just try compressing some big file that is already well
 *  compressed (or expands for some other wild reason).  So we allocate
 *  a compression buffer a bit larger than the original file size.
 *  10% is overkill for really large files so some other limit might
 *  be a good idea.
*/
#define WHY_COMPRESS_CONSTANT       0.1

/***********************************************************************
**
*/  REBSER *Compress(REBSER *input, REBINT index, REBCNT len, REBFLG use_crc)
/*
**      Compress a binary (only).
**		data
**		/part
**		length
**		/crc32
**
**      Note: If the file length is "small", it can't overrun on
**      compression too much so we use our magic numbers; otherwise,
**      we'll just be safe by a percentage of the file size.  This may
**      be a bit much, though.
**
***********************************************************************/
{
	uLongf size;
	REBSER *output;
	REBINT err;
	REBYTE out_size[sizeof(REBCNT)];

	size = len + (len > STERLINGS_MAGIC_NUMBER ? len / 10 + 12 : STERLINGS_MAGIC_FIX);
	output = Make_Binary(size);

	//DISABLE_GC;	// !!! why??
	// dest, dest-len, src, src-len, level
	err = Z_compress2(BIN_HEAD(output), (uLongf*)&size, BIN_HEAD(input) + index, len, use_crc);
	if (err) {
		if (err == Z_MEM_ERROR) Trap0(RE_NO_MEMORY);
		SET_INTEGER(DS_RETURN, err);
		Trap1(RE_BAD_PRESS, DS_RETURN); //!!!provide error string descriptions
	}
	SET_STR_END(output, size);
	SERIES_TAIL(output) = size;
	REBCNT_To_Bytes(out_size, (REBCNT)len); // Tag the size to the end.
	Append_Series(output, (REBYTE*)out_size, sizeof(REBCNT));
	if (SERIES_AVAIL(output) > 1024) // Is there wasted space?
		output = Copy_Series(output); // Trim it down if too big. !!! Revisit this based on mem alloc alg.
	//ENABLE_GC;

	return output;
}


/***********************************************************************
**
*/  REBSER *Decompress(REBSER *input, REBCNT index, REBINT len, REBCNT limit, REBFLG use_crc)
/*
**      Decompress a binary (only).
**
***********************************************************************/
{
	uLongf size;
	REBSER *output;
	REBINT err;

	if (len < 0 || (index + len > BIN_LEN(input))) len = BIN_LEN(input) - index;

	// Get the size from the end and make the output buffer that size.
	if (len <= 4) Trap0(RE_PAST_END); // !!! better msg needed
	size = Bytes_To_REBCNT(BIN_SKIP(input, len) - sizeof(REBCNT));

	if (limit && size > limit) Trap_Num(RE_SIZE_LIMIT, size); 

	output = Make_Binary(size);

	//DISABLE_GC;
	err = Z_uncompress(BIN_HEAD(output), (uLongf*)&size, BIN_HEAD(input) + index, len, use_crc);
	if (err) {
		if (PG_Boot_Phase < 2) return 0;
		if (err == Z_MEM_ERROR) Trap0(RE_NO_MEMORY);
		SET_INTEGER(DS_RETURN, err);
		Trap1(RE_BAD_PRESS, DS_RETURN); //!!!provide error string descriptions
	}
	SET_STR_END(output, size);
	SERIES_TAIL(output) = size;
	//ENABLE_GC;
	return output;
}

#ifdef USE_LZMA

static void *SzAlloc(ISzAllocPtr p, size_t size) { UNUSED(p); return malloc(size); }
static void SzFree(ISzAllocPtr p, void *address) { UNUSED(p); free(address); }
const ISzAlloc g_Alloc = { SzAlloc, SzFree };

/***********************************************************************
**
*/  REBSER *CompressLzma(REBSER *input, REBINT index, REBCNT len)
/*
**      Compress a binary (only) using LZMA compression.
**		data
**		/part
**		length
**
***********************************************************************/
{
	REBU64  size;
	REBU64  size_in = len;
	REBSER *output;
	REBINT  err;
	REBYTE *dest;
	REBYTE  out_size[sizeof(REBCNT)];

	//@@ are these Sterling's magic numbers correct for LZMA too?
	size = LZMA_PROPS_SIZE + size_in + (size_in > STERLINGS_MAGIC_NUMBER ? size_in / 10 + 12 : STERLINGS_MAGIC_FIX);
	output = Make_Binary(size);

	// so far hardcoded LZMA encoder properties... it would be nice to be able specify these by user if needed.
	CLzmaEncProps props;
	LzmaEncProps_Init(&props);
	props.level = 5;
	props.dictSize = 0; // use default value
	props.lc = -1; // -1 = default value
	props.lp = -1;
	props.pb = -1;
	props.fb = -1;
	props.numThreads = -1;
	// Possible values:
	//	int level, /* 0 <= level <= 9, default = 5 */
	//	unsigned dictSize, /* use (1 << N) or (3 << N). 4 KB < dictSize <= 128 MB */
	//	int lc, /* 0 <= lc <= 8, default = 3  */
	//	int lp, /* 0 <= lp <= 4, default = 0  */
	//	int pb, /* 0 <= pb <= 4, default = 2  */
	//	int fb,  /* 5 <= fb <= 273, default = 32 */
	//	int numThreads /* 1 or 2, default = 2 */

	dest = BIN_HEAD(output);

	/* header: 5 bytes of LZMA properties */
	REBU64 headerSize = LZMA_PROPS_SIZE;
	size -= headerSize;

	err = LzmaEncode(dest + headerSize, (SizeT*)&size, BIN_HEAD(input) + index, (SizeT)len, &props, dest, (SizeT*)&headerSize, 0,
		((ICompressProgress *)0), &g_Alloc, &g_Alloc);
	//printf("lzmaencode res: %i size: %u headerSize: %u\n", err, size, headerSize);
	if (err) {
		if (err == SZ_ERROR_MEM) Trap0(RE_NO_MEMORY);
		SET_INTEGER(DS_RETURN, err);
		Trap1(RE_BAD_PRESS, DS_RETURN); //!!!provide error string descriptions
	}
	size += headerSize;
	//SET_STR_END(output, size);
	SERIES_TAIL(output) = size;
	REBCNT_To_Bytes(out_size, (REBCNT)len); // Tag the size to the end.
	Append_Series(output, (REBYTE*)out_size, sizeof(REBCNT));
	if (SERIES_AVAIL(output) > 1024) // Is there wasted space?
		output = Copy_Series(output); // Trim it down if too big. !!! Revisit this based on mem alloc alg.
	return output;
}

/***********************************************************************
**
*/  REBSER *DecompressLzma(REBSER *input, REBCNT index, REBINT len, REBCNT limit)
/*
**      Decompress a binary (only).
**
***********************************************************************/
{
	REBU64 size;
	REBU64 unpackSize;
	REBSER *output;
	REBINT err;
	REBYTE *dest;
	REBYTE *src = BIN_HEAD(input) + index;
	REBU64 headerSize = LZMA_PROPS_SIZE;
	ELzmaStatus status = 0;

	if (len < 0 || (index + len > BIN_LEN(input))) len = BIN_LEN(input) - index;
	if (len < 9) Trap0(RE_PAST_END); // !!! better msg needed
	size = cast(REBU64, len - LZMA_PROPS_SIZE); // don't include size of properties

	// Get the uncompressed size from the end.
	unpackSize = cast(REBU64, Bytes_To_REBCNT(BIN_SKIP(input, len) - sizeof(REBCNT)));
	if(limit > 0 && unpackSize > limit) unpackSize = limit;

	output = Make_Binary(unpackSize);
	dest = BIN_HEAD(output);

	err = LzmaDecode(dest, (SizeT*)&unpackSize, src + LZMA_PROPS_SIZE, (SizeT*)&size, src, headerSize, LZMA_FINISH_ANY, &status, &g_Alloc);
	//printf("lzmadecode res: %i status: %i size: %u\n", err, status, size);

	if (err) {
		if (err == SZ_ERROR_MEM) Trap0(RE_NO_MEMORY);
		SET_INTEGER(DS_RETURN, err);
		Trap1(RE_BAD_PRESS, DS_RETURN); //!!!provide error string descriptions
	}
	SET_STR_END(output, unpackSize);
	SERIES_TAIL(output) = unpackSize;
	return output;
}

#endif //USE_LZMA