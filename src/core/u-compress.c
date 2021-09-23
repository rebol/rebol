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
	stream.avail_out = size;
	stream.next_out = BIN_HEAD(output);

	err = deflate(&stream, Z_FINISH);
	//printf("deflate err: %i  stream.total_out: %i .avail_out: %i\n", err, stream.total_out, stream.avail_out);
	
	if (err != Z_STREAM_END)
		Trap_ZStream_Error(&stream, err, TRUE);

	SET_STR_END(output, stream.total_out);
	SERIES_TAIL(output) = stream.total_out;

	if((windowBits & 16) != 16) { // Not GZIP
		// Tag the size to the end. Only when not using GZIP envelope.
		REBYTE out_size[sizeof(REBCNT)];
		REBCNT_To_Bytes(out_size, (REBCNT)in_len);
		Append_Series(output, (REBYTE*)out_size, sizeof(REBCNT));
	}
	
	if (SERIES_AVAIL(output) > 1024) // Is there wasted space?
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
	if (limit > 0) {
		size = limit;
	} else if (windowBits < 0) {
		// limit was not specified, but data are supposed to be raw DEFLATE data
		// max teoretic DEFLATE ration is 1032:1, but that is quite unrealistic
		// it will be more around 3:1 or 4:1, so 10:1 could be enough for automatic setup.
		size = 10 * (REBCNT)len; //@@ fix me, if you don't agree with above claim
	} else {
		// Get the uncompressed size from last 4 source data bytes.
		if (len < 4) Trap0(RE_PAST_END); // !!! better msg needed
		size = cast(REBU64, Bytes_To_REBCNT(BIN_SKIP(input, index + len) - sizeof(REBCNT)));
		if (size > (uLongf)len * 14) Trap_Num(RE_SIZE_LIMIT, size); // check for a realistic limit
	}

	output = Make_Binary(size);

	z_stream stream;
	stream.zalloc = &zalloc; // fail() cleans up automatically, see notes
	stream.zfree = &zfree;
	stream.opaque = NULL; // passed to zalloc and zfree, not needed currently
	stream.total_out = 0;

	stream.avail_in = len;
	stream.next_in = cast(const Bytef*, BIN_HEAD(input) + index);
	
	err = inflateInit2(&stream, windowBits);
	if (err != Z_OK) Trap_ZStream_Error(&stream, err, FALSE);
	
	stream.avail_out = size;
	stream.next_out = BIN_HEAD(output);

	for(;;) {
		err = inflate(&stream, Z_NO_FLUSH);
		if (err == Z_STREAM_END || stream.total_out == size)
			break; // Finished. (and buffer was big enough)
		//printf("err: %i size: %i avail_out: %i total_out: %i\n", err, size, stream.avail_out, stream.total_out);
		if(err != Z_OK) Trap_ZStream_Error(&stream, err, FALSE);
		//@@: may need to resize the destination buffer! But...
		//@@: so far let's expect that size is always correct
		//@@: and introduce self expanding buffers in compression port implementation
	}
	//printf("total_out: %i\n", stream.total_out);
	inflateEnd(&stream);

	SET_STR_END(output, stream.total_out);
	SERIES_TAIL(output) = stream.total_out;

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


#ifdef INCLUDE_PNG_FILTER
#ifdef INCLUDE_PNG_CODEC
int paeth_predictor(int a, int b, int c);
#else
#define int_abs(a) (((a)<0)?(-(a)):(a))
static int paeth_predictor(int a, int b, int c) {
	int p, pa, pb, pc;

	p = a + b - c;
	pa = int_abs(p - a);
	pb = int_abs(p - b);
	pc = int_abs(p - c);
	if ((pa <= pb) && (pa <= pc))
		return a;
	else if (pb <= pc)
		return b;
	return c;
}
#endif

enum PNG_Filter_Types {
	PNG_FILTER_SUB = 1,
	PNG_FILTER_UP,
	PNG_FILTER_AVERAGE,
	PNG_FILTER_PAETH
};

static REBYTE get_png_filter_type(REBVAL* val) {
	if (IS_WORD(val)) {
		switch (VAL_WORD_SYM(val)) {
		case SYM_SUB:     return PNG_FILTER_SUB;
		case SYM_UP:      return PNG_FILTER_UP;
		case SYM_AVERAGE: return PNG_FILTER_AVERAGE;
		case SYM_PAETH:   return PNG_FILTER_PAETH;
		}
	}
	else if (IS_INTEGER(val)) {
		return MIN(PNG_FILTER_PAETH, MAX(0, VAL_INT32(val)));
	}
	Trap1(RE_INVALID_ARG, val);
}

// See: https://www.rfc-editor.org/rfc/rfc2083.html#page-31
//      https://en.wikipedia.org/wiki/Portable_Network_Graphics#Filtering
/***********************************************************************
**
*/	REBNATIVE(filter)
/*
//	filter: native [
//		"PNG delta filter"
//		data   [binary!] "Input"
//		width  [number!] "Scanline width"
//		type   [integer! word!] "1..4 or one of: [sub up average paeth]"
//		/skip bpp [integer!] "Bytes per pixel"
//	]
***********************************************************************/
{
	REBVAL *val_data  = D_ARG(1);
	REBVAL *val_width = D_ARG(2);
	REBVAL *val_type  = D_ARG(3);
	REBOOL  ref_skip  = D_REF(4);
	REBVAL *val_bpp   = D_ARG(5);

	REBSER *ser;
	REBYTE *bin = VAL_BIN_DATA(val_data);
	REBYTE *scan, *prev, *temp, *out;
	REBCNT r, c, rows, bytes;
	REBINT width = AS_INT32(val_width);
	REBYTE filter = get_png_filter_type(val_type);
	REBCNT bpp = ref_skip ? VAL_INT32(val_bpp) : 1;

	bytes = VAL_LEN(val_data);

	if (width <= 1 || width > bytes)
		Trap1(RE_INVALID_ARG, val_width);
	if (bpp < 1 || bpp > width)
		Trap1(RE_INVALID_ARG, val_bpp);

	rows = bytes / width;
	ser  = Make_Binary(bytes);
	out  = BIN_DATA(ser);

	temp = malloc(width);
	if (!temp) {
		Trap0(RE_NO_MEMORY);
		return R_NONE;
	}
	memset(temp, 0, width);

	prev = temp;
	for (r = 0; r < rows; r++) {
		scan = bin + (r * width);
		out  = BIN_SKIP(ser, r * width);

		switch (filter) {
		case PNG_FILTER_SUB:
			for (c = 0; c < bpp; c++)
				out[c] = scan[c];
			for (c = bpp; c < width; c++)
				out[c] = scan[c] - scan[c - bpp];
			break;
		case PNG_FILTER_UP:
			for (c = 0; c < width; c++)
				out[c] = scan[c] - prev[c];
			break;
		case PNG_FILTER_AVERAGE:
			for (c = 0; c < bpp; c++)
				out[c] = scan[c] - (prev[c] >> 1);
			for (c = bpp; c < width; c++)
				out[c] = scan[c] - ((scan[c - bpp] + prev[c]) >> 1) & 0xFF;
			break;
		case PNG_FILTER_PAETH:
			for (c = 0; c < bpp; c++)
				out[c] = scan[c] - prev[c];
			for (c = bpp; c < width; c++)
				out[c] = scan[c] - paeth_predictor(scan[c - bpp], prev[c], prev[c - bpp]);
			break;
		}
		prev = scan;
	}
	free(temp);
	SET_BINARY(D_RET, ser);
	VAL_TAIL(D_RET) = bytes;
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(unfilter)
/*
//	unfilter: native [
//		"Reversed PNG delta filter"
//		data  [binary!]  "Input"
//		width [number!]  "Scanline width (not counting the type byte)"
//		/as   "Filter type. If not used, type is decoded from first byte on each line."
//		 type [integer! word!] "1..4 or one of: [sub up average paeth]"
//		/skip
//		 bpp [integer!]  "Bytes per pixel"
//	]
***********************************************************************/
{
	REBVAL *val_data  = D_ARG(1);
	REBVAL *val_width = D_ARG(2);
	REBOOL  ref_as    = D_REF(3);
	REBVAL *val_type  = D_ARG(4);
	REBOOL  ref_skip  = D_REF(5);
	REBVAL *val_bpp   = D_ARG(6);

	REBSER *ser;
	REBYTE *bin = VAL_BIN_DATA(val_data);
	REBINT width = AS_INT32(val_width);
	REBCNT r, c, rows;
	REBYTE *scan, *prev, *temp, *out;
	REBYTE filter;
	REBCNT bytes = VAL_LEN(val_data);
	REBCNT bpp = ref_skip ? VAL_INT32(val_bpp) : 1;

	if (!ref_as) width++;
	if (width <= 1 || width > bytes)
		Trap1(RE_INVALID_ARG, val_width);
	if (bpp < 1 || bpp > width)
		Trap1(RE_INVALID_ARG, val_bpp);

	rows = ceil(bytes / width);
	ser  = Make_Binary(bytes);
	out  = BIN_DATA(ser);

	if (ref_as)
		filter = get_png_filter_type(val_type);

	temp = malloc(width);
	if (!temp) {
		Trap0(RE_NO_MEMORY);
		return R_NONE;
	}
	memset(temp, 0, width);

	if (!ref_as) width--;

	prev = temp;
	for (r = 0; r < rows; r++) {
		if (ref_as) {
			scan = bin + r * width;
			out = BIN_SKIP(ser, r * width);
		} else {
			scan = bin + r * (width + 1);
			out = BIN_SKIP(ser, r * (width));
			filter = scan[0];
			scan++;
		}
		
		switch (filter) {
		case PNG_FILTER_SUB:
			for (c = 0; c < bpp; c++)
				out[c] = scan[c];
			for (c = bpp; c < width; c++)
				out[c] = scan[c] + out[c - bpp];
			break;
		case PNG_FILTER_UP:
			for (c = 0; c < width; c++)
				out[c] = scan[c] + prev[c];
			break;
		case PNG_FILTER_AVERAGE:
			for (c = 0; c < bpp; c++)
				out[c] = scan[c] + (prev[c] >> 1) & 0xFF;
			for (c = bpp; c < width; c++)
				out[c] = scan[c] + ((out[c - bpp] + prev[c]) >> 1) & 0xFF;
			break;
		case PNG_FILTER_PAETH:
			for (c = 0; c < bpp; c++)
				out[c] = scan[c] + prev[c];
			for (c = bpp; c < width; c++)
				out[c] = scan[c] + paeth_predictor(out[c - bpp], prev[c], prev[c - bpp]);
			break;
		}
		prev = out;
	}
	free(temp);
	SET_BINARY(D_RET, ser);
	VAL_TAIL(D_RET) = ref_as ? bytes : bytes - rows;
	return R_RET;
}
#endif //INCLUDE_PNG_FILTER
