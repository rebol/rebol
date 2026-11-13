/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2025 Rebol Open Source Contributors
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
#include "reb-ext-handler.h"
#ifdef INCLUDE_LZMA
#include "sys-lzma.h"
#endif // INCLUDE_LZMA


// Registry entry structure
typedef struct {
	REBINT          sym;    // Symbol ID for the method
	COMPRESS_FUNC   encode; // Compress function pointer
	DECOMPRESS_FUNC decode; // Decompress function pointer
} COMPRESS_METHOD;

// Dynamic registry (used to register compression methods on runtime)
#define COMPRESS_METHOD_SIZE 16
static COMPRESS_METHOD *compress_registry; // allocated in Init_Compression
static REBCNT compress_method_count = 0;
static REBCNT compress_method_size = COMPRESS_METHOD_SIZE;


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


#ifdef INCLUDE_DEFLATE
#include "deflate/libdeflate.h"

static void* DelfateAlloc(size_t size) { return Make_Managed_Mem(0, size); }
static void DelfateFree(void* address) { Free_Managed_Mem(0, address); }

static const struct libdeflate_options options = {
	.sizeof_options = sizeof(options),
	.malloc_func = DelfateAlloc,
	.free_func = DelfateFree,
};

typedef enum {
	DEFLATE_MODE_DEFLATE,
	DEFLATE_MODE_ZLIB,
	DEFLATE_MODE_GZIP
} deflate_mode_t;

typedef size_t(*compress_fn_t)(
	struct libdeflate_compressor* ctx,
	const void* in, size_t in_size,
	void* out, size_t out_size
	);

typedef size_t(*compress_bound_fn_t)(
	struct libdeflate_compressor* ctx,
	size_t in_size
	);

typedef enum libdeflate_result(*decompress_fn_t)(
	struct libdeflate_decompressor* ctx,
	const void* in, size_t in_size,
	void* out, size_t out_size,
	size_t* actual_out
	);

typedef enum libdeflate_result(*decompress_ex_fn_t)(
	struct libdeflate_decompressor* ctx,
	const void* in, size_t in_size,
	void* out, size_t out_size,
	size_t* in_consumed,
	size_t* out_consumed
	);

typedef struct {
	compress_fn_t       compress;
	compress_bound_fn_t compress_bound;
	decompress_fn_t     decompress;
	decompress_ex_fn_t  decompress_ex;
} deflate_handlers_t;

static const deflate_handlers_t handlers[] = {
	[DEFLATE_MODE_DEFLATE] = {
		.compress = libdeflate_deflate_compress,
		.compress_bound = libdeflate_deflate_compress_bound,
		.decompress = libdeflate_deflate_decompress,
		.decompress_ex = libdeflate_deflate_decompress_ex
	},
	[DEFLATE_MODE_ZLIB] = {
		.compress = libdeflate_zlib_compress,
		.compress_bound = libdeflate_zlib_compress_bound,
		.decompress = libdeflate_zlib_decompress,
		.decompress_ex = libdeflate_zlib_decompress_ex
	},
	[DEFLATE_MODE_GZIP] = {
		.compress = libdeflate_gzip_compress,
		.compress_bound = libdeflate_gzip_compress_bound,
		.decompress = libdeflate_gzip_decompress,
		.decompress_ex = libdeflate_gzip_decompress_ex
	}
};

int CompressCommonDeflate(deflate_mode_t mode, const REBYTE* input, REBLEN len, REBCNT level, REBSER** output, REBINT* error) {
	struct libdeflate_compressor* ctx;
	const deflate_handlers_t* h;

	if (mode < 0 || mode > 3) return FALSE;
	h = &handlers[mode];
	if (!h->compress || !h->compress_bound) return FALSE;

	if (level > 12) level = 12;
	ctx = libdeflate_alloc_compressor_ex(level, &options);
	if (!ctx) return FALSE;

	size_t size = h->compress_bound(ctx, len);
	if (size == 0 || size > MAX_I32) {
		libdeflate_free_compressor(ctx);
		return FALSE;
	}
	*output = Make_Binary((REBLEN)size);

	size = h->compress(ctx, input, len, BIN_HEAD(*output), SERIES_REST(*output));

	libdeflate_free_compressor(ctx);

	if (size == 0) return FALSE;
	SERIES_TAIL(*output) = (REBLEN)size;
	return TRUE;
}

/***********************************************************************
**
*/  int CompressDeflate(const REBYTE* input, REBLEN len, REBCNT level, REBSER** output, REBINT* error)
/*
**      Compress a binary using Deflate.
**
***********************************************************************/
{
	return CompressCommonDeflate(DEFLATE_MODE_DEFLATE, input, len, level, output, error);
}
/***********************************************************************
**
*/  int CompressGzip(const REBYTE* input, REBLEN len, REBCNT level, REBSER** output, REBINT* error)
/*
**      Compress a binary using Deflate with gzip wrapper.
**
***********************************************************************/
{
	return CompressCommonDeflate(DEFLATE_MODE_GZIP, input, len, level, output, error);
}
/***********************************************************************
**
*/  int CompressZlib(const REBYTE* input, REBLEN len, REBCNT level, REBSER** output, REBINT* error)
/*
**      Compress a binary using Deflate with zlib wrapper.
**
***********************************************************************/
{
	return CompressCommonDeflate(DEFLATE_MODE_ZLIB, input, len, level, output, error);
}

int DecompressCommonDeflate(deflate_mode_t mode, const REBYTE* input, REBLEN len, REBLEN limit, REBSER** output, REBINT* error) {
	struct libdeflate_decompressor* ctx;
	int result;
	REBU64 out_len;
	size_t out_bytes = 0;
	size_t in_bytes = 0;
	const deflate_handlers_t* h = &handlers[mode];

	if (mode >= sizeof(handlers) / sizeof(handlers[0]) || !h->decompress) return FALSE;

	ctx = libdeflate_alloc_decompressor_ex(&options);
	if (!ctx) return FALSE;

	out_len = (limit != NO_LIMIT) ? limit : len << 2;

	if (out_len == 0) {
		*output = Make_Binary(1);
		libdeflate_free_decompressor(ctx);
		return TRUE;
	}
	if (out_len > MAX_I32) out_len = MAX_I32;
	*output = Make_Binary((REBLEN)out_len);

	if (limit != NO_LIMIT) {
		out_bytes = limit;
		result = h->decompress(ctx, input, len, BIN_HEAD(*output), out_bytes, NULL);
	}
	else {
	retry:
		result = h->decompress_ex(ctx, input, len, BIN_HEAD(*output), SERIES_REST(*output), &in_bytes, &out_bytes);
		if (result == LIBDEFLATE_INSUFFICIENT_SPACE) {
			SERIES_TAIL(*output) = SERIES_REST(*output);
			Expand_Series(*output, AT_TAIL, len >> 1);
			goto retry;
		}
	}

	if (result > 0) {
		*error = result;
		libdeflate_free_decompressor(ctx);
		return FALSE;
	}
	if (limit != NO_LIMIT && out_bytes > limit) out_bytes = limit;
	libdeflate_free_decompressor(ctx);
	SERIES_TAIL(*output) = (REBLEN)out_bytes;
	return TRUE;
}

/***********************************************************************
**
*/  int DecompressDeflate(const REBYTE* input, REBLEN len, REBCNT level, REBSER** output, REBINT* error)
/*
**      Decompress a binary using Deflate.
**
***********************************************************************/
{
	return DecompressCommonDeflate(DEFLATE_MODE_DEFLATE, input, len, level, output, error);
}
/***********************************************************************
**
*/  int DecompressGzip(const REBYTE* input, REBLEN len, REBCNT level, REBSER** output, REBINT* error)
/*
**      Decompress a binary using Deflate with gzip wrapper.
**
***********************************************************************/
{
	return DecompressCommonDeflate(DEFLATE_MODE_GZIP, input, len, level, output, error);
}
/***********************************************************************
**
*/  int DecompressZlib(const REBYTE* input, REBLEN len, REBCNT level, REBSER** output, REBINT* error)
/*
**      Decompress a binary using Deflate with zlib wrapper.
**
***********************************************************************/
{
	return DecompressCommonDeflate(DEFLATE_MODE_ZLIB, input, len, level, output, error);
}

#endif //INCLUDE_DEFLATE


#ifdef INCLUDE_DEPRECATED_ZLIB
#include "sys-zlib.h"
static void *zalloc(void *opaque, unsigned nr, unsigned size) {
	return Make_Managed_Mem(opaque, nr*size);
}

static void zfree(void *opaque, void *address) {
	Free_Managed_Mem(opaque, address);
}

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
*/  REBSER *CompressZlibDeprecated(REBSER *input, REBINT index, REBCNT in_len, REBINT level, REBINT windowBits)
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
*/  REBSER *DecompressZlibDeprecated(REBSER *input, REBCNT index, REBINT len, REBCNT limit, REBINT windowBits)
/*
**      Decompress a binary (only).
**
***********************************************************************/
{
	uLongf size;
	REBSER *output;
	REBINT err;

	if (len < 0 || (index + len > BIN_LEN(input))) len = BIN_LEN(input) - index;
	size = (limit != NO_LIMIT) ? limit : (uLongf)len * 3;

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
			stream.avail_out = SERIES_REST(output) - stream.total_out - 1;
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
#endif

#ifdef INCLUDE_LZMA

static void *SzAlloc(ISzAllocPtr p, size_t size) { return Make_Managed_Mem((void*)p, size); }
static void SzFree(ISzAllocPtr p, void *address) { Free_Managed_Mem((void *)p, address); }
static const ISzAlloc g_Alloc = { SzAlloc, SzFree };

/***********************************************************************
**
*/  int CompressLzma(const REBYTE* input, REBLEN len, REBCNT level, REBSER** output, int* error)
/*
**      Compress a binary using LZMA compression.
**
***********************************************************************/
{
	REBU64  size;
	REBYTE *dest;
	REBYTE  out_size[sizeof(REBCNT)];

	level = (level == UNKNOWN) ? 5 : MIN(9, level);

	//@@ are these Sterling's magic numbers correct for LZMA too?
	size = LZMA_PROPS_SIZE + len + (len > STERLINGS_MAGIC_NUMBER ? len / 10 + 12 : STERLINGS_MAGIC_FIX);
	*output = Make_Binary(size);

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

	dest = BIN_HEAD(*output);

	/* header: 5 bytes of LZMA properties */
	REBU64 headerSize = LZMA_PROPS_SIZE;
	size -= headerSize;

	*error = LzmaEncode(dest + headerSize, (SizeT*)&size, input, (SizeT)len, &props, dest, (SizeT*)&headerSize, 0,
		((ICompressProgress *)0), &g_Alloc, &g_Alloc);
	//printf("lzmaencode res: %i size: %u headerSize: %u\n", err, size, headerSize);
	if (*error) return FALSE;
	size += headerSize;

	SERIES_TAIL(*output) = size;
	REBCNT_To_Bytes(out_size, (REBCNT)len); // Tag the size to the end.
	Append_Series(*output, (REBYTE*)out_size, sizeof(REBCNT));
	if (SERIES_AVAIL(*output) > 16384)  // Is there wasted space?
		*output = Copy_Series(*output); // Trim it down if too big. !!! Revisit this based on mem alloc alg.
	return TRUE;
}

/***********************************************************************
**
*/  int DecompressLzma(const REBYTE* input, REBLEN len, REBLEN limit, REBSER** output, int* error)
/*
**      Decompress a binary using LZMA.
**
***********************************************************************/
{
	REBU64 size;
	REBU64 destLen;
	REBYTE *dest;
	REBU64 headerSize = LZMA_PROPS_SIZE;
	ELzmaStatus status = 0;

	if (len < 9) Trap0(RE_PAST_END); // !!! better msg needed
	size = cast(REBU64, len - LZMA_PROPS_SIZE); // don't include size of properties

	if(limit != NO_LIMIT) {
		destLen = limit;
	} else {
		// Get the uncompressed size from last 4 source data bytes.
		destLen = cast(REBU64, Bytes_To_REBCNT(input + len - sizeof(REBCNT)));
	}

	*output = Make_Binary(destLen);
	dest = BIN_HEAD(*output);

	*error = LzmaDecode(dest, (SizeT*)&destLen, input + LZMA_PROPS_SIZE, (SizeT*)&size, input, headerSize, LZMA_FINISH_ANY, &status, &g_Alloc);
	//printf("lzmadecode res: %i status: %i size: %u\n", err, status, size);

	if (*error) return FALSE;
	SERIES_TAIL(*output) = destLen;
	return TRUE;
}

#endif //INCLUDE_LZMA


#ifdef INCLUDE_BROTLI
#include "brotli/encode.h"
#include "brotli/decode.h"
#include "brotli/types.h"

/* Default brotli_alloc_func */
void* BrotliDefaultAllocFunc(void* opaque, size_t size) {
	return Make_Managed_Mem(opaque, size);
}

/* Default brotli_free_func */
void BrotliDefaultFreeFunc(void* opaque, void* address) {
	Free_Managed_Mem(opaque, address);
}

// Using global de/encoder state, because Rebol may throw an error
// and so left the state unreleased...
static BrotliEncoderState* BrotliEncoder = NULL;
static BrotliDecoderState* BrotliDecoder = NULL;

/***********************************************************************
**
*/  int CompressBrotli(const REBYTE* input, REBLEN len, REBCNT level, REBSER** output, int* error)
/*
**      Compress a binary using Brotli.
**
***********************************************************************/
{
	size_t available_out = 0;
	size_t available_in = len;
	size_t total_out = 0;
	size_t max_size;
	BROTLI_BOOL res;
	REBYTE* bin;
	
	error = 0;

	if (BrotliDecoder) {
		BrotliEncoderDestroyInstance(BrotliEncoder);
		BrotliEncoder = NULL;
	}
	BrotliEncoder = BrotliEncoderCreateInstance(BrotliDefaultAllocFunc, BrotliDefaultFreeFunc, NULL);
	if (!BrotliEncoder) return 0;
	
	// Set compression quality (0-11)
	if (level == NO_LIMIT) level = 6;
	BrotliEncoderSetParameter(BrotliEncoder, BROTLI_PARAM_QUALITY, MAX(0, MIN(11, level)));
	max_size = BrotliEncoderMaxCompressedSize(len);
	*output = Make_Binary((REBLEN)max_size);
	
	bin = BIN_TAIL(*output);
	available_out = SERIES_REST(*output);
	// compress..
	res = BrotliEncoderCompressStream(
		BrotliEncoder, BROTLI_OPERATION_FINISH,
		&available_in, &input,
		&available_out, &bin, &total_out);
	// cleanup..
	BrotliEncoderDestroyInstance(BrotliEncoder);
	BrotliEncoder = NULL;
	SERIES_TAIL(*output) = (REBLEN)total_out;
	return res;
}

/***********************************************************************
**
*/  int DecompressBrotli(const REBYTE* input, REBLEN len, REBLEN limit, REBSER** output, int* error)
/*
**      Decompress a binary using Brotli.
**
***********************************************************************/
{
	BROTLI_BOOL res;

	if (BrotliDecoder)
		BrotliDecoderDestroyInstance(BrotliDecoder); // in case that there was a Rebol error in previous call

	BrotliDecoder = BrotliDecoderCreateInstance(BrotliDefaultAllocFunc, BrotliDefaultFreeFunc, NULL);
	if (!BrotliDecoder) {
		//trace("Failed to create the Brotli decoder!");
		return FALSE;
	}

	REBU64 out_len = (limit != NO_LIMIT) ? limit : len * 2;
	*output = Make_Binary(out_len);

	size_t availableIn = len;
	size_t availableOut = SERIES_AVAIL(*output);
	size_t totalOut = 0;
	uint8_t* nextOut = BIN_HEAD(*output);

	while (1) {
		res = BrotliDecoderDecompressStream(BrotliDecoder, &availableIn, (const uint8_t**)&input, &availableOut, &nextOut, &totalOut);
		if (res == BROTLI_DECODER_RESULT_ERROR || res == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT) goto error;

		if (BrotliDecoderIsFinished(BrotliDecoder) || (limit != NO_LIMIT && totalOut > limit)) {
			break;  // Decompression finished
		}

		// If the output buffer is full, resize it
		if (availableOut == 0 && (limit == NO_LIMIT || limit < totalOut)) {
			SERIES_TAIL(*output) = (REBLEN)totalOut;
			Expand_Series(*output, AT_TAIL, out_len); //@@ May throw an error and so the decoder would not be released!
			SERIES_TAIL(*output) = (REBLEN)totalOut;
			nextOut = BIN_HEAD(*output) + totalOut;  // Move the output pointer to the correct position
			availableOut = SERIES_AVAIL(*output);
		}
	}

	BrotliDecoderDestroyInstance(BrotliDecoder);
	BrotliDecoder = NULL;

	if (limit != NO_LIMIT && totalOut > limit) totalOut = limit;

	SET_STR_END(*output, totalOut);
	SERIES_TAIL(*output) = (REBLEN)totalOut;
	return TRUE;

error:
	*error = BrotliDecoderGetErrorCode(BrotliDecoder);
	BrotliDecoderDestroyInstance(BrotliDecoder);
	BrotliDecoder = NULL;
	return FALSE;
}

#endif //INCLUDE_BROTLI

#ifdef INCLUDE_LZ4 // https://github.com/lz4/lz4
#define LZ4_USER_MEMORY_FUNCTIONS
void* LZ4_malloc(size_t size) {
	return Make_Managed_Mem(NULL, size);
}
void* LZ4_calloc(size_t nmemb, size_t size) {
	return Make_Managed_CMem(nmemb, size);
}
void LZ4_free(void* address) {
	Free_Managed_Mem(NULL, address);
}
#include "lz4/lz4hc.h"
/***********************************************************************
**
*/  int CompressLz4(const REBYTE* input, REBLEN len, REBCNT level, REBSER** output, int* error)
/*
**      Compress a binary using LZ4.
**
***********************************************************************/
{
	int result = LZ4_compressBound(len);
	if (result <= 0) return FALSE;
	*output = Make_Binary(result);
	result = LZ4_compress_HC(
		input, BIN_HEAD(*output),
		len, SERIES_REST(*output),
		MAX(1, MIN(LZ4HC_CLEVEL_MAX, (int)level)));
	if (result <= 0) return FALSE;
	SERIES_TAIL(*output) = result;
	return TRUE;
}

/***********************************************************************
**
*/  int DecompressLz4(const REBYTE* input, REBLEN len, REBLEN limit, REBSER** output, int* error)
/*
**      Decompress a binary using LZ4.
**
***********************************************************************/
{
	*output = Make_Binary((limit != NO_LIMIT) ? limit : len * 3);
	int result = LZ4_decompress_safe(input, BIN_HEAD(*output), len, SERIES_REST(*output));
	if (result <= 0) return FALSE;
	SERIES_TAIL(*output) = result;
	return TRUE;
}

#endif //INCLUDE_LZ4


#ifdef INCLUDE_LZAV // https://github.com/avaneev/lzav
#undef LZAV_DEF_MALLOC
#define LZAV_MALLOC( s, T ) (T*) Make_Managed_Mem(NULL, s)
#define LZAV_FREE( p ) Free_Managed_Mem(NULL, p )

#include "sys-lzav.h" 
/***********************************************************************
**
*/  int CompressLzav(const REBYTE* input, REBLEN len, REBCNT level, REBSER** output, int* error)
/*
**      Compress a binary using LZAV.
**
***********************************************************************/
{
	error = 0;
	int out_bytes = lzav_compress_bound(len);
	if (out_bytes <= 0) return 0;
	*output = Make_Binary(out_bytes);
	out_bytes = lzav_compress_default(input, BIN_HEAD(*output), (const int)len, SERIES_REST(*output));
	if (out_bytes == 0) return FALSE;
	SERIES_TAIL(*output) = out_bytes;
	return TRUE;
}

/***********************************************************************
**
*/  int DecompressLzav(const REBYTE* input, REBLEN len, REBLEN limit, REBSER** output, int* error)
/*
**      Decompress a binary using LZAV.
**
***********************************************************************/
{
	if (limit == NO_LIMIT) return 0; // LZMA requires exact destination size!
	*output = Make_Binary(limit);
	*error = lzav_decompress(input, BIN_HEAD(*output), (const int)len, (const int)limit);
	if (*error < 0) return FALSE;
	SERIES_TAIL(*output) = limit;
	return TRUE;

}
#endif //INCLUDE_LZAV


/***********************************************************************
**
*/	REBOOL Register_Compress_Method(REBINT sym, void* encode, void* decode)
/*
**		Register a new compression method
**
***********************************************************************/
{
    if (compress_method_count >= compress_method_size) {
		compress_method_size += 8;
		void* new_registry = Make_Clear_Mem(compress_method_size, sizeof(COMPRESS_METHOD));
		if (new_registry == NULL) return FALSE;
		COPY_MEM(new_registry, compress_registry, compress_method_count * sizeof(COMPRESS_METHOD));
		Free_Mem(compress_registry, compress_method_count * sizeof(COMPRESS_METHOD));
		compress_registry = (COMPRESS_METHOD*)new_registry;
    }
    
    // Check if already registered
    for (REBCNT i = 0; i < compress_method_count; i++) {
        if (compress_registry[i].sym == sym) {
            // Update existing handlers
            compress_registry[i].encode = (COMPRESS_FUNC)encode;
			compress_registry[i].decode = (DECOMPRESS_FUNC)decode;
            return TRUE;
        }
    }
    
    // Add new entry
    compress_registry[compress_method_count].sym = sym;
    compress_registry[compress_method_count].encode = (COMPRESS_FUNC)encode;
	compress_registry[compress_method_count].decode = (DECOMPRESS_FUNC)decode;
    compress_method_count++;

	// Append new name to system/catalog/compressions
	REBVAL val;
	REBVAL* blk = Get_System(SYS_CATALOG, CAT_COMPRESSIONS);
	Init_Word(&val, sym);
	Append_Val(VAL_SERIES(blk), &val);

    return TRUE;
}

// Find compression handler
static COMPRESS_FUNC Find_Compress_Handler(REBINT sym) {
	for (REBCNT i = 0; i < compress_method_count; i++) {
		if (compress_registry[i].sym == sym) {
			return compress_registry[i].encode;
		}
	}
	return NULL;
}
// Find decompression handler
static DECOMPRESS_FUNC Find_Decompress_Handler(REBINT sym) {
	for (REBCNT i = 0; i < compress_method_count; i++) {
		if (compress_registry[i].sym == sym) {
			return compress_registry[i].decode;
		}
	}
	return NULL;
}



/***********************************************************************
**
*/	REBNATIVE(compress)
/*
//	compress: native [
//		{Compresses data.}
//		data [binary! string!] {If string, it will be UTF8 encoded}
//		method [word!] {One of `system/catalog/compressions`}
//		/part length {Length of source data}
//		/level lvl [integer!] {Compression level 0-9}
//	]
***********************************************************************/
{
	REBVAL* data = D_ARG(1);
	REBINT  method = VAL_WORD_CANON(D_ARG(2));
	//	REBOOL ref_part  = D_REF(3);
	REBVAL* length = D_ARG(4);
	REBOOL ref_level = D_REF(5);

	REBSER* ser = VAL_SERIES(data);
	REBCNT len = Partial1(data, length); // May modify the index!
	REBCNT index = VAL_INDEX(data);
#ifdef INCLUDE_DEPRECATED_ZLIB
	REBINT windowBits = MAX_WBITS;
#endif
	REBCNT level = ref_level ? VAL_UNT32(D_ARG(6)) : UNKNOWN;
	
	// Try to find registered handler
	COMPRESS_FUNC encoder = Find_Compress_Handler(method);
	if (encoder) {
		REBSER* out = NULL;
		REBINT  err = 0;
		int res = encoder(BIN_SKIP(ser, index), len, level, &out, &err);
		if ( res && out) {
			Set_Binary(D_RET, out);
			return R_RET;
		}
		else {
			if (out) Free_Series(out);
			SET_INTEGER(DS_RETURN, err);
			Trap1(RE_BAD_PRESS, DS_RETURN); //!!!provide error string descriptions
		}
	}

	switch (method) {
#ifdef INCLUDE_DEPRECATED_ZLIB
	case SYM_ZLIB:
	zlib_compress:
		Set_Binary(D_RET, CompressZlibDeprecated(ser, index, (REBINT)len, level, windowBits));
		break;

	case SYM_DEFLATE:
		windowBits = -windowBits;
		goto zlib_compress;

	case SYM_GZIP:
		windowBits |= 16;
		goto zlib_compress;
#endif
	default:
		Trap1(RE_INVALID_ARG, D_ARG(2));
	}

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(decompress)
/*
//	decompress: native [
//		{Decompresses data.}
//		data [binary!] {Source data to decompress}
//		method [word!] {One of `system/catalog/compressions`}
//		/part "Limits source data to a given length or position"
//			length [number! series!] {Length of compressed data (must match end marker)}
//		/size
//			bytes [integer!] {Number of uncompressed bytes.}
]
***********************************************************************/
{
	REBVAL* data = D_ARG(1);
	REBINT  method = VAL_WORD_CANON(D_ARG(2));
	//	REBOOL ref_part = D_REF(3);
	REBVAL* length = D_ARG(4);
	REBOOL ref_size = D_REF(5);
	REBVAL* size = D_ARG(6);

	REBCNT limit = NO_LIMIT;
	REBCNT len;
	REBINT windowBits = MAX_WBITS;

	len = Partial1(data, length);

	if (ref_size) limit = (REBCNT)Int32s(size, 1); // /limit size

	// Try to find registered handler
	DECOMPRESS_FUNC decode = Find_Decompress_Handler(method);
	if (decode) {
		REBSER* out = NULL;
		REBINT  err = 0;
		REBYTE* inp = BIN_SKIP(VAL_SERIES(data), VAL_INDEX(data));
		if (decode(inp, len, limit, &out, &err) && out) {
			Set_Binary(D_RET, out);
			return R_RET;
		}
		else {
			if (out) Free_Series(out);
			SET_INTEGER(DS_RETURN, err);
			Trap1(RE_BAD_PRESS, DS_RETURN); //!!!provide error string descriptions
		}
	}
	switch (method) {
#ifdef INCLUDE_DEPRECATED_ZLIB
	case SYM_ZLIB:
	zlib_decompress:
		Set_Binary(D_RET, DecompressZlibDeprecated(VAL_SERIES(data), VAL_INDEX(data), (REBINT)len, limit, windowBits));
		break;

	case SYM_DEFLATE:
		windowBits = -windowBits;
		goto zlib_decompress;

	case SYM_GZIP:
		windowBits |= 16;
		goto zlib_decompress;
#endif
	default:
		Trap1(RE_INVALID_ARG, D_ARG(2));
	}

	return R_RET;
}

/***********************************************************************
**
*/	void Init_Compression(void)
/*
**		Fill system/catalog/compressions with optional compressions
**
***********************************************************************/
{
	compress_registry = Make_Clear_Mem(COMPRESS_METHOD_SIZE, sizeof(COMPRESS_METHOD));

#ifdef INCLUDE_DEFLATE
	Register_Compress_Method(SYM_DEFLATE, CompressDeflate, DecompressDeflate);
	Register_Compress_Method(SYM_ZLIB, CompressZlib, DecompressZlib);
	Register_Compress_Method(SYM_GZIP, CompressGzip, DecompressGzip);
#endif
#ifdef INCLUDE_BROTLI
	Register_Compress_Method(SYM_BR, CompressBrotli, DecompressBrotli);
#endif
#ifdef INCLUDE_CRUSH
	Register_Compress_Method(SYM_CRUSH, CompressCrush, DecompressCrush);
#endif
#ifdef INCLUDE_LZ4
	Register_Compress_Method(SYM_LZ4, CompressLz4, DecompressLz4);
#endif
#ifdef INCLUDE_LZAV
	Register_Compress_Method(SYM_LZAV, CompressLzav, DecompressLzav);
#endif
#ifdef INCLUDE_LZMA
	Register_Compress_Method(SYM_LZMA, CompressLzma, DecompressLzma);
#endif
#ifdef INCLUDE_LZW
	Register_Compress_Method(SYM_LZW, CompressLzw, DecompressLzw);
#endif

}

/***********************************************************************
**
*/	void Dispose_Compression(void)
/*
**		Release allocated compression memory.
**
***********************************************************************/
{
	if (compress_registry) {
		Free_Mem(compress_registry, compress_method_size * sizeof(COMPRESS_METHOD));
	}
}