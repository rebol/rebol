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
**  Module:  u-compress.c
**  Summary: interface to zlib and or optional lzma compression
**  Section: utility
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "sys-zlib.h"
#ifdef INCLUDE_LZMA
#include "sys-lzma.h"
#endif // INCLUDE_LZMA

static void *zalloc(void *opaque, unsigned nr, unsigned size)
{
	UNUSED(opaque);
	void *ptr = malloc(nr * size);
	//printf("zalloc: %x - %i\n", ptr, nr * size);
	return ptr;
}

static void zfree(void *opaque, void *addr)
{
	UNUSED(opaque);
	//printf("zfree: %x\n", addr);
	free(addr);
}

//#ifdef old_Sterlings_code // used also in LZMA code at this moment
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
//#endif

void Trap_ZStream_Error(z_stream *stream, int err, REBOOL while_compression)
/*
**      Free Z_stream resources and throw Rebol error using message,
**      if available, or error code
**/
{
	REBVAL *ret = DS_RETURN;
	if(stream->msg) {
		REBSER* msg = Append_UTF8(NULL, cb_cast(stream->msg), cast(REBINT, strlen(stream->msg)));
		SET_STRING(ret, msg);
	} else {
		SET_INTEGER(ret, err);
	}
	if(while_compression) {
		deflateEnd(stream);
	} else {
		inflateEnd(stream);
	}
	Trap1(RE_BAD_PRESS, ret);
}

/***********************************************************************
**
*/  REBSER *CompressZlib(REBSER *input, REBINT index, REBCNT in_len, REBINT level, REBINT windowBits)
/*
**      Compress a binary (only).
**
***********************************************************************/
{
	uLongf size;
	REBSER *output;
	REBINT err;

	z_stream stream;
	stream.zalloc = &zalloc;
	stream.zfree = &zfree;
	stream.opaque = NULL;

	if(level < 0)
		level = Z_DEFAULT_COMPRESSION;
	else if(level > Z_BEST_COMPRESSION)
		level = Z_BEST_COMPRESSION;

	err = z_deflateInit2(&stream, level, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY);
	if (err != Z_OK) Trap_ZStream_Error(&stream, err, TRUE);

#ifdef old_Sterlings_code
	size = in_len + (in_len > STERLINGS_MAGIC_NUMBER ? in_len / 10 + 12 : STERLINGS_MAGIC_FIX);
#else
	size = 1 + deflateBound(&stream, in_len); // one more byte for trailing null byte -> SET_STR_END
#endif

	stream.avail_in = in_len;
	stream.next_in = cast(const z_Bytef*, BIN_HEAD(input) + index);

	output = Make_Binary(size);
	stream.avail_out = SERIES_AVAIL(output);
	stream.next_out = BIN_HEAD(output);

	for (;;) {
		err = deflate(&stream, Z_FINISH);
		if (err == Z_STREAM_END)
			break; // Finished or we have enough data.
		//printf("deflate err: %i  stream.total_out: %i .avail_out: %i\n", err, stream.total_out, stream.avail_out);
		if (err != Z_OK)
			Trap_ZStream_Error(&stream, err, FALSE);
		if (stream.avail_out == 0) {
			// expand output buffer...
			SERIES_TAIL(output) = stream.total_out;
			Expand_Series(output, AT_TAIL, in_len);
			stream.next_out = BIN_SKIP(output, stream.total_out);
			stream.avail_out = SERIES_REST(output) - stream.total_out;
		}
	}

	SET_STR_END(output, stream.total_out);
	SERIES_TAIL(output) = stream.total_out;
	
	if (SERIES_AVAIL(output) > 4096)  // Is there wasted space?
		output = Copy_Series(output); // Trim it down if too big. !!! Revisit this based on mem alloc alg.

	deflateEnd(&stream);
	return output;
}


/***********************************************************************
**
*/  REBSER *DecompressZlib(REBSER *input, REBCNT index, REBINT len, REBCNT limit, REBINT windowBits)
/*
**      Decompress a binary (only).
**
***********************************************************************/
{
	uLongf size;
	REBSER *output;
	REBINT err;

	if (len < 0 || (index + len > BIN_LEN(input))) len = BIN_LEN(input) - index;
	size = (limit > 0) ? limit : (uLongf)len * 3;

	output = Make_Binary(size);

	z_stream stream;
	stream.zalloc = &zalloc; // fail() cleans up automatically, see notes
	stream.zfree = &zfree;
	stream.opaque = NULL; // passed to zalloc and zfree, not needed currently
	stream.total_out = 0;

	stream.avail_in = len;
	stream.next_in = cast(const Bytef*, BIN_SKIP(input, index));
	
	err = inflateInit2(&stream, windowBits);
	if (err != Z_OK) Trap_ZStream_Error(&stream, err, FALSE);
	
	stream.avail_out = SERIES_AVAIL(output);
	stream.next_out = BIN_HEAD(output);

	for(;;) {
		err = inflate(&stream, Z_NO_FLUSH);
		if (err == Z_STREAM_END || (limit && stream.total_out >= limit))
			break; // Finished or we have enough data.
		//printf("err: %i size: %i avail_out: %i total_out: %i\n", err, size, stream.avail_out, stream.total_out);
		if (err != Z_OK) Trap_ZStream_Error(&stream, err, FALSE);
		if (stream.avail_out == 0) {
			// expand output buffer...
			SERIES_TAIL(output) = stream.total_out;
			Expand_Series(output, AT_TAIL, len);
			stream.next_out = BIN_SKIP(output, stream.total_out);
			stream.avail_out = SERIES_REST(output) - stream.total_out;
		}
	}
	//printf("total_out: %i\n", stream.total_out);
	inflateEnd(&stream);

	if (limit && stream.total_out > limit) {
		stream.total_out = limit;
	}
	SET_STR_END(output, stream.total_out);
	SERIES_TAIL(output) = stream.total_out;

	if (SERIES_AVAIL(output) > 4096) // Is there wasted space?
		output = Copy_Series(output); // Trim it down if too big. !!! Revisit this based on mem alloc alg.

	return output;
}

#ifdef INCLUDE_LZMA

static void *SzAlloc(ISzAllocPtr p, size_t size) { UNUSED(p); return malloc(size); }
static void SzFree(ISzAllocPtr p, void *address) { UNUSED(p); free(address); }
static const ISzAlloc g_Alloc = { SzAlloc, SzFree };

/***********************************************************************
**
*/  REBSER *CompressLzma(REBSER *input, REBINT index, REBCNT in_len, REBINT level)
/*
**      Compress a binary (only) using LZMA compression.
**		data
**		/part
**		length
**
***********************************************************************/
{
	REBU64  size;
	REBU64  size_in = in_len;
	REBSER *output;
	REBINT  err;
	REBYTE *dest;
	REBYTE  out_size[sizeof(REBCNT)];

	if (level < 0)
		level = 5;
	else if (level > 9)
		level = 9;

	//@@ are these Sterling's magic numbers correct for LZMA too?
	size = LZMA_PROPS_SIZE + size_in + (size_in > STERLINGS_MAGIC_NUMBER ? size_in / 10 + 12 : STERLINGS_MAGIC_FIX);
	output = Make_Binary(size);

	// so far hardcoded LZMA encoder properties... it would be nice to be able specify these by user if needed.
	CLzmaEncProps props;
	LzmaEncProps_Init(&props);
	props.level = level;
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

	err = LzmaEncode(dest + headerSize, (SizeT*)&size, BIN_HEAD(input) + index, (SizeT)in_len, &props, dest, (SizeT*)&headerSize, 0,
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
	REBCNT_To_Bytes(out_size, (REBCNT)in_len); // Tag the size to the end.
	Append_Series(output, (REBYTE*)out_size, sizeof(REBCNT));
	if (SERIES_AVAIL(output) > 1024) // Is there wasted space?
		output = Copy_Series(output); // Trim it down if too big. !!! Revisit this based on mem alloc alg.
	return output;
}

/***********************************************************************
**
*/  REBSER *DecompressLzma(REBSER *input, REBCNT index, REBINT in_len, REBCNT limit)
/*
**      Decompress a binary (only).
**
***********************************************************************/
{
	REBU64 size;
	REBU64 destLen;
	REBSER *output;
	REBINT err;
	REBYTE *dest;
	REBYTE *src = BIN_HEAD(input) + index;
	REBU64 headerSize = LZMA_PROPS_SIZE;
	ELzmaStatus status = 0;

	if (in_len < 0 || (index + in_len > BIN_LEN(input))) in_len = BIN_LEN(input) - index;
	if (in_len < 9) Trap0(RE_PAST_END); // !!! better msg needed
	size = cast(REBU64, in_len - LZMA_PROPS_SIZE); // don't include size of properties

	if(limit > 0) {
		destLen = limit;
	} else {
		// Get the uncompressed size from last 4 source data bytes.
		destLen = cast(REBU64, Bytes_To_REBCNT(BIN_SKIP(input, index + in_len) - sizeof(REBCNT)));
	}

	output = Make_Binary(destLen);
	dest = BIN_HEAD(output);

	err = LzmaDecode(dest, (SizeT*)&destLen, src + LZMA_PROPS_SIZE, (SizeT*)&size, src, headerSize, LZMA_FINISH_ANY, &status, &g_Alloc);
	//printf("lzmadecode res: %i status: %i size: %u\n", err, status, size);

	if (err) {
		if (err == SZ_ERROR_MEM) Trap0(RE_NO_MEMORY);
		SET_INTEGER(DS_RETURN, err);
		Trap1(RE_BAD_PRESS, DS_RETURN); //!!!provide error string descriptions
	}
	SET_STR_END(output, destLen);
	SERIES_TAIL(output) = destLen;
	return output;
}

#endif //INCLUDE_LZMA

