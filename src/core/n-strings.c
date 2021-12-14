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
**  Module:  n-strings.c
**  Summary: native functions for strings
**  Section: natives
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "sys-scan.h"
#include "sys-deci-funcs.h"
#include "sys-checksum.h"

REBCNT z_adler32_z(REBCNT adler, REBYTE *buf, REBCNT len);



// Table of hash functions and parameters:
static struct digest {
	REBYTE *(*digest)(REBYTE *, REBCNT, REBYTE *);
	void (*init)(void *);
	void (*update)(void *, REBYTE *, REBCNT);
	void (*final)(void *, REBYTE *);
	int (*ctxsize)(void);
	REBINT index;
	REBINT len;
	REBINT hmacblock;
} digests[] = {

	{MD5,       MD5_Starts,    MD5_Update,    MD5_Finish,    MD5_CtxSize, SYM_MD5,    16, 64},
	{SHA1,     SHA1_Starts,   SHA1_Update,   SHA1_Finish,   SHA1_CtxSize, SYM_SHA1,   20, 64},
	{SHA256, SHA256_Starts, SHA256_Update, SHA256_Finish, SHA256_CtxSize, SYM_SHA256, 32, 64},
#ifdef INCLUDE_SHA224
	{SHA224, SHA224_Starts, SHA256_Update, SHA256_Finish, SHA256_CtxSize, SYM_SHA224, 28, 64},
#endif
#ifdef INCLUDE_SHA384
	{SHA384, SHA384_Starts, SHA384_Update, SHA384_Finish, SHA384_CtxSize, SYM_SHA384, 48, 128},
#endif
	{SHA512, SHA512_Starts, SHA512_Update, SHA512_Finish, SHA512_CtxSize, SYM_SHA512, 64, 128},
#ifdef INCLUDE_RIPEMD160
	{RIPEMD160, RIPEMD160_Starts, RIPEMD160_Update, RIPEMD160_Finish, RIPEMD160_CtxSize, SYM_RIPEMD160, 20, 64},
#endif
#ifdef INCLUDE_MD4
	{MD4, MD4_Starts, MD4_Update, MD4_Finish, MD4_CtxSize, SYM_MD4, 16, 64},
#endif
	{0}

};


/***********************************************************************
**
*/	REBNATIVE(ajoin)
/*
***********************************************************************/
{
	REBSER *str;

	str = Form_Reduce(VAL_SERIES(D_ARG(1)), VAL_INDEX(D_ARG(1)));
	if (!str) return R_TOS;

	Set_String(DS_RETURN, str); // not D_RET (stack modified)

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(as_binary)
/*
***********************************************************************/
{
	Trap0(RE_DEPRECATED);
//	*D_RET = *D_ARG(1);
//	VAL_SET(D_RET, REB_BINARY);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(as_string)
/*
***********************************************************************/
{
	Trap0(RE_DEPRECATED);
//	*D_RET = *D_ARG(1);
//	VAL_SET(D_RET, REB_STRING);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(checksum)
/*
//	checksum: native [
//		{Computes a checksum, CRC, hash, or HMAC.}
//		data [binary! string!] {If string, it will be UTF8 encoded}
//		method [word!] {One of `system/catalog/checksums` and HASH}
//		/with {Extra value for HMAC key or hash table size; not compatible with TCP/CRC24/CRC32/ADLER32 methods.}
//		 spec [any-string! binary! integer!] {String or binary for MD5/SHA* HMAC key, integer for hash table size.}
//		/part {Limits to a given length}
//		 length
//	]
***********************************************************************/
{
	REBVAL *arg  = D_ARG(ARG_CHECKSUM_DATA);
	REBINT sym   = VAL_WORD_CANON(D_ARG(ARG_CHECKSUM_METHOD));
	REBVAL *spec = D_ARG(ARG_CHECKSUM_SPEC);
	REBINT sum;
	REBINT i = 0;
	REBINT j;
	REBSER *digest, *ser;
	REBCNT len, keylen;
	REBYTE *data;
	REBYTE *keycp;


	len = Partial1(arg, D_ARG(ARG_CHECKSUM_LENGTH));

	if (IS_STRING(arg)) {
		ser = Encode_UTF8_Value(arg, len, 0);
		data = SERIES_DATA(ser);
		len = SERIES_LEN(ser) - 1;
	}
	else {
		data = VAL_BIN_DATA(arg);
	}

	if (sym > SYM_CRC32 && sym <= SYM_SHA512) {
		// O: could be optimized using index computed from `sym`
		// find matching digest:
		for (i = 0; i < sizeof(digests) / sizeof(digests[0]); i++) {

			if (digests[i].index == sym) {

				digest = Make_Series(digests[i].len, 1, FALSE);
				LABEL_SERIES(digest, "checksum digest");

				if (D_REF(ARG_CHECKSUM_WITH)) {	// HMAC
					if (IS_INTEGER(spec))
						Trap1(RE_BAD_REFINE, D_ARG(ARG_CHECKSUM_SPEC));

					if (IS_BINARY(spec)) {
						keycp = VAL_BIN_DATA(spec);
						keylen = VAL_LEN(spec);
					}
					else {
						// normalize to UTF8 first
						ser = Encode_UTF8_Value(spec, VAL_LEN(spec), 0);
						keycp = SERIES_DATA(ser);
						keylen = SERIES_LEN(ser) - 1;
					}
					REBYTE tmpdigest[128];		// Size must be max of all digest[].len;
					REBYTE ipad[128],opad[128];	// Size must be max of all digest[].hmacblock;
					void *ctx = Make_Mem(digests[i].ctxsize());

					int blocklen = digests[i].hmacblock;

					if (keylen > blocklen) {
						digests[i].digest(keycp,keylen,tmpdigest);
						keycp = tmpdigest;
						keylen = digests[i].len;
					}

					memset(ipad, 0, blocklen);
					memset(opad, 0, blocklen);
					memcpy(ipad, keycp, keylen);
					memcpy(opad, keycp, keylen);

					for (j = 0; j < blocklen; j++) {
						ipad[j]^=0x36;
						opad[j]^=0x5c;
					}

					digests[i].init(ctx);
					digests[i].update(ctx,ipad,blocklen);
					digests[i].update(ctx, data, len);
					digests[i].final(ctx, tmpdigest);
					digests[i].init(ctx);
					digests[i].update(ctx,opad,blocklen);
					digests[i].update(ctx,tmpdigest,digests[i].len);
					digests[i].final(ctx,BIN_HEAD(digest));

					Free_Mem(ctx, digests[i].ctxsize());

				} else {
					digests[i].digest(data, len, BIN_HEAD(digest));
				}

				SERIES_TAIL(digest) = digests[i].len;
				Set_Series(REB_BINARY, DS_RETURN, digest);

				return 0;
			}
		}
		// used correct name, but diggest was not found (excluded from build)
		Trap0(RE_FEATURE_NA);
	}

	if (D_REF(ARG_CHECKSUM_WITH) && ((sym > SYM_HASH && sym <= SYM_CRC32) || sym == SYM_TCP))
		Trap0(RE_BAD_REFINES);

	if (sym == SYM_CRC32 || sym == SYM_ADLER32) {
		i = (sym == SYM_CRC32) ? CRC32(data, len) : z_adler32_z(0x00000001L, data, len);
	}
	else if (sym == SYM_HASH) {  // /hash
		if(!D_REF(ARG_CHECKSUM_WITH)) Trap0(RE_MISSING_ARG);
		if (!IS_INTEGER(spec)) Trap1(RE_BAD_REFINE, D_ARG(ARG_CHECKSUM_SPEC));
		sum = VAL_INT32(spec); // size of the hash table
		if (sum <= 1) sum = 1;
		i = Hash_String(data, len) % sum;
	}
	else if (sym == SYM_CRC24) {
		i = Compute_CRC24(data, len);
	}
	else if (sym == SYM_TCP) {
		i = Compute_IPC(data, len);
	}
	else {
		Trap_Arg(D_ARG(ARG_CHECKSUM_METHOD));
	}

	DS_RET_INT(i);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(compress)
/*
//	compress: native [
//		{Compresses data.}
//		data [binary! string!] {If string, it will be UTF8 encoded}
//		method [word!] "zlib deflate gzip lzma"
//		/part length {Length of source data}
//		/level lvl [integer!] {Compression level 0-9}
//	]
***********************************************************************/
{
	REBVAL *data     = D_ARG(1);
	REBINT  method   = VAL_WORD_CANON(D_ARG(2));
//	REBOOL ref_part  = D_REF(3);
	REBVAL *length   = D_ARG(4);
	REBOOL ref_level = D_REF(5);
	REBVAL *level    = D_ARG(6);

	REBSER *ser;
	REBCNT index;
	REBCNT len;
	REBINT windowBits = MAX_WBITS;

	len = Partial1(data, length);
	ser = Prep_Bin_Str(data, &index, &len); // result may be a SHARED BUFFER!

	switch (method) {
	case SYM_ZLIB:
	zlib_compress:
		Set_Binary(D_RET, CompressZlib(ser, index, (REBINT)len, ref_level ? VAL_INT32(level) : -1, windowBits));
		break;
	
	case SYM_DEFLATE:
		windowBits = -windowBits;
		goto zlib_compress;
	
	case SYM_GZIP:
		windowBits |= 16;
		goto zlib_compress;

	case SYM_LZMA:
#ifdef INCLUDE_LZMA
		Set_Binary(D_RET, CompressLzma(ser, index, (REBINT)len, ref_level ? VAL_INT32(level) : -1));
#else
		Trap0(RE_FEATURE_NA);
#endif
		break;
	case SYM_CRUSH:
#ifdef INCLUDE_CRUSH
		Set_Binary(D_RET, CompressCrush(ser, index, (REBINT)len, ref_level ? VAL_INT32(level) : 2));
#else
		Trap0(RE_FEATURE_NA);
#endif
		break;
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
//		method [word!] "zlib deflate gzip lzma" 
//		/part "Limits source data to a given length or position"
//			length [number! series!] {Length of compressed data (must match end marker)}
//		/size
//			bytes [integer!] {Number of uncompressed bytes.}
]
***********************************************************************/
{
	REBVAL *data    = D_ARG(1);
	REBINT  method  = VAL_WORD_CANON(D_ARG(2));
//	REBOOL ref_part = D_REF(3);
	REBVAL *length  = D_ARG(4);
	REBOOL ref_size = D_REF(5);
	REBVAL *size    = D_ARG(6);

	REBCNT limit = 0;
	REBCNT len;
	REBINT windowBits = MAX_WBITS;

	len = Partial1(data, length);

	if (ref_size) limit = (REBCNT)Int32s(size, 1); // /limit size

	switch (method) {
	case SYM_ZLIB:
	zlib_decompress:
		Set_Binary(D_RET, DecompressZlib(VAL_SERIES(data), VAL_INDEX(data), (REBINT)len, limit, windowBits));
		break;

	case SYM_DEFLATE:
		windowBits = -windowBits;
		goto zlib_decompress;

	case SYM_GZIP:
		windowBits |= 16;
		goto zlib_decompress;

	case SYM_LZMA:
#ifdef INCLUDE_LZMA
		Set_Binary(D_RET, DecompressLzma(VAL_SERIES(data), VAL_INDEX(data), (REBINT)len, limit));
#else
		Trap0(RE_FEATURE_NA);
#endif
		break;
	case SYM_CRUSH:
#ifdef INCLUDE_CRUSH
		Set_Binary(D_RET, DecompressCrush(VAL_SERIES(data), VAL_INDEX(data), (REBINT)len, limit));
#else
		Trap0(RE_FEATURE_NA);
#endif
		break;
	default:
		Trap1(RE_INVALID_ARG, D_ARG(2));
	}

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(construct)
/*
***********************************************************************/
{
	REBVAL *value = D_ARG(1);
	REBSER *parent = 0;
	REBSER *frame;

	if (IS_STRING(value) || IS_BINARY(value)) {
		REBCNT index;

		// Just a guess at size:
		frame = Make_Block(10);		// Use a std BUF_?
		Set_Block(D_RET, frame);	// Keep safe

		// Convert string if necessary. Store back for safety.
		VAL_SERIES(value) = Prep_Bin_Str(value, &index, 0);

		// !issue! Is this what we really want here?
		Scan_Net_Header(frame, VAL_BIN(value) + index);
		value = D_RET;
	}

	if (D_REF(2)) parent = VAL_OBJ_FRAME(D_ARG(3));

	frame = Construct_Object(parent, VAL_BLK_DATA(value), D_REF(4));
	SET_OBJECT(D_RET, frame);

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(debase)
/*
**		Converts a binary base representation string to binary.
**		Input is a STRING, but BINARY is also accepted.
**		BINARY is returned. We don't know the encoding.
**
***********************************************************************/
{
	REBINT base = VAL_INT32(D_ARG(2));
	REBSER *ser;
	REBCNT index;
	REBCNT len = 0;

	if (D_REF(4)) {
		if (VAL_INT64(D_ARG(5)) > (i64)MAX_I32) {
			len = MAX_I32;
		} else if (VAL_INT64(D_ARG(5)) <= 0) {
			Set_Binary(D_RET, Make_Binary(0));
			return R_RET;
		} else
			len = VAL_INT32(D_ARG(5));
	}

	ser = Prep_Bin_Str(D_ARG(1), &index, &len); // result may be a SHARED BUFFER!

	if (!Decode_Binary(D_RET, BIN_SKIP(ser, index), len, base, 0, D_REF(3)))
 		Trap1(RE_INVALID_DATA, D_ARG(1));

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(enbase)
/*
**		Converts a binary to a binary base representation STRING.
**		Input is BINARY or STRING (UTF8 encoded).
**
***********************************************************************/
{
	REBSER *ser = NULL;
	REBCNT index;
	REBVAL *arg = D_ARG(1);
	REBINT base = VAL_INT32(D_ARG(2));
	REBCNT limit = NO_LIMIT;

	if (D_REF(4)) {
		if (VAL_INT64(D_ARG(5)) > (i64)MAX_I32) {
			limit = MAX_I32;
		} else if (VAL_INT64(D_ARG(5)) <= 0) {
			Set_String(D_RET, Make_Binary(0));
			return R_RET;
		} else
			limit = VAL_INT32(D_ARG(5));
	}

	Set_Binary(arg, Prep_Bin_Str(arg, &index, (limit == NO_LIMIT) ? 0 : &limit)); // may be SHARED buffer
	VAL_INDEX(arg) = index;

	switch (base) {
	case 64:
		ser = Encode_Base64(arg, 0, limit, FALSE, D_REF(3));
		break;
	case 16:
		ser = Encode_Base16(arg, 0, limit, FALSE);
		break;
	case 2:
		ser = Encode_Base2(arg, 0, limit, FALSE);
		break;
	case 85:
#ifdef INCLUDE_BASE85
		ser = Encode_Base85(arg, 0, limit, FALSE);
#else
		Trap0(RE_FEATURE_NA);
#endif
		
		break;
	default:
		Trap_Arg(D_ARG(2));
	}

	Set_String(D_RET, ser);

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(decloak)
/*
**		Input is BINARY only. Modifies input.
**
***********************************************************************/
{
	REBVAL *data = D_ARG(1);
	REBVAL *key  = D_ARG(2);

	if (IS_PROTECT_SERIES(VAL_SERIES(data))) Trap0(RE_PROTECTED);

	if (!Cloak(TRUE, VAL_BIN_DATA(data), VAL_LEN(data), (REBYTE*)key, 0, D_REF(3)))
		Trap_Arg(key);

	return R_ARG1;
}


/***********************************************************************
**
*/	REBNATIVE(encloak)
/*
**		Input is BINARY only. Modifies input.
**
***********************************************************************/
{
	REBVAL *data = D_ARG(1);
	REBVAL *key  = D_ARG(2);
	
	if (IS_PROTECT_SERIES(VAL_SERIES(data))) Trap0(RE_PROTECTED);

	if (!Cloak(FALSE, VAL_BIN_DATA(data), VAL_LEN(data), (REBYTE*)key, 0, D_REF(3)))
		Trap_Arg(key);

	return R_ARG1;
}


/***********************************************************************
**
*/	REBNATIVE(dehex)
/*
**		Works for any string.
**
***********************************************************************/
{
	REBVAL *arg        = D_ARG(1);
//	REBOOL  ref_escape = D_REF(2);
//	REBVAL *val_escape = D_ARG(3);
	REBOOL as_url      = D_REF(4);
	REBINT len = (REBINT)VAL_LEN(arg); // due to len -= 2 below
	REBUNI n;
	REBSER *ser;

	const REBCHR escape_char = D_REF(2) ? VAL_CHAR(D_ARG(3)) : '%';

	if (VAL_BYTE_SIZE(arg)) {
		REBYTE *bp = VAL_BIN_DATA(arg);
		REBYTE *dp = Reset_Buffer(BUF_FORM, len);

		for (; len > 0; len--) {
			if (*bp == escape_char && len > 2 && Scan_Hex2(bp+1, &n, FALSE)) {
				*dp++ = (REBYTE)n;
				bp += 3;
				len -= 2;
			} else {
				*dp++ = *bp++;
			}
		}

		*dp = 0;
		ser = Copy_String(BUF_FORM, 0, dp - BIN_HEAD(BUF_FORM));
	}
	else {
		REBUNI *up = VAL_UNI_DATA(arg);
		REBUNI *dp = (REBUNI*)Reset_Buffer(BUF_MOLD, len);

		for (; len > 0; len--) {
			if (*up == escape_char && len > 2 && Scan_Hex2((REBYTE*)(up+1), &n, TRUE)) {
				*dp++ = (REBUNI)n;
				up += 3;
				len -= 2;
			} else {
				*dp++ = *up++;
			}
		}

		*dp = 0;
		ser = Copy_String(BUF_MOLD, 0, dp - UNI_HEAD(BUF_MOLD));
	}

	Set_Series(VAL_TYPE(arg), D_RET, ser);

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(enhex)
/*
**		Works for any string.
**		Compatible with https://tc39.es/ecma262/#sec-encodeuri-uri
**		If source is unicode (wide) string, result is ASCII.
**			value [any-string! binary!] {The string to encode}
**			/escape char [char!] {Can be used to change the default escape char #"%"}
**			/unescaped set [bitset!] {Can be used to specify, which chars can be unescaped}
**
**
***********************************************************************/
{
	REBVAL *arg = D_ARG(1);
//	REBOOL  ref_escape = D_REF(2);
//	REBVAL *val_escape = D_ARG(3);
	REBOOL  ref_bitset = D_REF(4);
	REBVAL *val_bitset = D_ARG(5);
	REBYTE encoded[4];
	REBCNT n, encoded_size;
	REBSER *ser;
	const REBCHR escape_char = D_REF(2) ? VAL_CHAR(D_ARG(3)) : '%';

	if (!ref_bitset) {
		// use bitset value from system/catalog/bitsets
		// use URI bitset when value is file or url
		// else use URI_COMPONENT
		val_bitset = Get_Object(
			Get_System(SYS_CATALOG, CAT_BITSETS),
			(IS_URL(arg) || IS_FILE(arg)) ? CAT_BITSETS_URI : CAT_BITSETS_URI_COMPONENT
		);
	}

	// using FORM buffer for intermediate conversion;
	// counting with the worst scenario, where each single codepoint
	// might need 4 bytes of UTF-8 data (%XX%XX%XX%XX)
	REBYTE *dp = Reset_Buffer(BUF_FORM, 12 * VAL_LEN(arg));

	if (VAL_BYTE_SIZE(arg)) {
		// byte size is 1, so the series should not contain chars with value over 0x80
		// see: https://github.com/Oldes/Rebol3/commit/aa939ba41fd71efb9f0a97f85993c95129c7e515

		REBYTE *bp = VAL_BIN_DATA(arg);
		REBYTE *ep = VAL_BIN_TAIL(arg);		

		while (bp < ep) {
			REBYTE c = bp[0];
			bp++;
			if (Check_Bit_Cased(VAL_SERIES(val_bitset), c)) {
				*dp++ = c;
				continue;
			}
			*dp++ = escape_char;
			*dp++ = Hex_Digits[(c & 0xf0) >> 4];
			*dp++ = Hex_Digits[ c & 0xf];
		}
	}
	else { // UNICODE variant
		REBUNI *up = VAL_UNI_DATA(arg);
		REBUNI *ep = (REBUNI*)VAL_UNI_TAIL(arg);
		
		while (up < ep) {
			REBUNI c = up[0];
			up++;

			if (c >= 0x80) {// all non-ASCII characters *must* be percent encoded
				encoded_size = Encode_UTF8_Char(encoded, c);
			} else {
				if (Check_Bit_Cased(VAL_SERIES(val_bitset), c)) {
					*dp++ = (REBYTE)c;
					continue;
				}
				encoded[0] = cast(REBYTE, c);
				encoded_size = 1;
			}
			for (n = 0; n < encoded_size; ++n) {
				*dp++ = escape_char;
				*dp++ = Hex_Digits[(encoded[n] & 0xf0) >> 4];
				*dp++ = Hex_Digits[ encoded[n] & 0xf];
			}
		}
	}
	*dp = 0;
	ser = Copy_String(BUF_FORM, 0, dp - BIN_HEAD(BUF_FORM));
	Set_Series(VAL_TYPE(arg), D_RET, ser);

	return R_RET;
}


/***********************************************************************
**
*/  REBNATIVE(deline)
/*
**		Convert CR or CRLF strings to just LF strings.
**
***********************************************************************/
{
	REBVAL *val = D_ARG(1);
	REBINT len = VAL_LEN(val);

	if (IS_PROTECT_SERIES(VAL_SERIES(val))) Trap0(RE_PROTECTED);

	if (D_REF(2)) { //lines
		Set_Block(D_RET, Split_Lines(val));
		return R_RET;
	}

	Replace_CRLF_to_LF(val, len);

	return R_ARG1;
}


/***********************************************************************
**
*/  REBNATIVE(enline)
/*
**		Convert LF to CRLF or native format.
**
***********************************************************************/
{
	REBVAL *val = D_ARG(1);
	REBSER *ser = VAL_SERIES(val);

	if (IS_PROTECT_SERIES(VAL_SERIES(val))) Trap0(RE_PROTECTED);
	if (IS_BLOCK(val)) Trap0(RE_NOT_DONE);

	if (SERIES_TAIL(ser)) {
#ifdef TO_WINDOWS
		if (VAL_BYTE_SIZE(val))
			Replace_LF_To_CRLF_Bytes(ser, VAL_INDEX(val), VAL_LEN(val));
		else
			Replace_LF_To_CRLF_Uni(ser, VAL_INDEX(val), VAL_LEN(val));
#else
		Replace_CRLF_to_LF(val, VAL_LEN(val));
#endif
	}

	return R_ARG1;
}


/***********************************************************************
**
*/  REBNATIVE(entab)
/*
**		Modifies input.
**
***********************************************************************/
{
	REBVAL *val = D_ARG(1);
	REBINT tabsize = TAB_SIZE;
	REBSER *ser;
	REBCNT len = VAL_LEN(val);

	if (D_REF(2)) tabsize = Int32s(D_ARG(3), 1);

	// Set up the copy buffer:
	if (VAL_BYTE_SIZE(val))
		ser = Entab_Bytes(VAL_BIN(val), VAL_INDEX(val), len, tabsize);
	else
		ser = Entab_Unicode(VAL_UNI(val), VAL_INDEX(val), len, tabsize);

	Set_Series(VAL_TYPE(val), D_RET, ser);
	
	return R_RET;
}


/***********************************************************************
**
*/  REBNATIVE(detab)
/*
***********************************************************************/
{
	REBVAL *val = D_ARG(1);
	REBINT tabsize = TAB_SIZE;
	REBSER *ser;
	REBCNT len = VAL_LEN(val);

	if (D_REF(2)) tabsize = Int32s(D_ARG(3), 1);

	// Set up the copy buffer:
	if (VAL_BYTE_SIZE(val))
		ser = Detab_Bytes(VAL_BIN(val), VAL_INDEX(val), len, tabsize);
	else
		ser = Detab_Unicode(VAL_UNI(val), VAL_INDEX(val), len, tabsize);

	Set_Series(VAL_TYPE(val), D_RET, ser);
	
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(lowercase)
/*
***********************************************************************/
{
	Change_Case(ds, D_ARG(1), D_ARG(3), FALSE);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(uppercase)
/*
***********************************************************************/
{
	Change_Case(ds, D_ARG(1), D_ARG(3), TRUE);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(to_hex)
/*
***********************************************************************/
{
	REBVAL *arg = D_ARG(1);
	REBINT len;
//	REBSER *series;
	REBYTE buffer[MAX_TUPLE*2+4];  // largest value possible
	REBYTE *buf;

#ifdef removed
	if (IS_INTEGER(arg)) len = MAX_HEX_LEN;
	else if (IS_TUPLE(arg)) {
		len = VAL_TUPLE_LEN(arg);
		if (len < 3) len = 3;
		len *= 2;
	}
	else Trap_Arg(arg);

	else if (IS_DECIMAL(arg)) len = MAX_HEX_LEN;
	else if (IS_MONEY(arg)) len = 24;
	else if (IS_CHAR(arg)) len = (VAL_CHAR(arg) > 0x7f) ? 4 : 2;
#endif

	buf = &buffer[0];

	len = -1;
	if (D_REF(2)) {	// /size
		//@@ https://github.com/Oldes/Rebol-issues/issues/127
		len = (REBINT) VAL_INT64(D_ARG(3));
		if (len <= 0) Trap_Arg(D_ARG(3));
	}
	if (IS_INTEGER(arg)) { // || IS_DECIMAL(arg)) {
		if (len < 0 || len > MAX_HEX_LEN) len = MAX_HEX_LEN;
		Form_Hex_Pad(buf, VAL_INT64(arg), len);
	}
	else if (IS_TUPLE(arg)) {
		REBINT n;
		if (len < 0 || len > 2 * MAX_TUPLE || len > 2 * VAL_TUPLE_LEN(arg))
			len = 2 * VAL_TUPLE_LEN(arg);
		for (n = 0; n < VAL_TUPLE_LEN(arg); n++)
			buf = Form_Hex2(buf, VAL_TUPLE(arg)[n]);
		for (; n < 3; n++)
			buf = Form_Hex2(buf, 0);
		*buf = 0;
	}
	else Trap_Arg(arg);

#ifdef removed
	else if (IS_CHAR(arg)) {
		REBSER *ser = Make_Binary(6);
		ser->tail = xEncode_UTF8_Char(BIN_HEAD(ser), VAL_CHAR(arg));
		for (len = 0; len < (signed)(ser->tail); len++)
			buf = Form_Hex2(buf, *BIN_SKIP(ser, len));
		len = ser->tail * 2;
		//Form_Hex_Pad(buf, VAL_CHAR(arg), len);
	}
	else if (IS_MONEY(arg)) {
		REBYTE tmp[12];
		deci_to_binary(&tmp[0], VAL_DECI(arg));
		for (len = 0; len < 12; len++)
			buf = Form_Hex2(buf, tmp[len]);
		len = 24;
	}
#endif

//	SERIES_TAIL(series) = len;
//	Set_Series(REB_ISSUE, D_RET, series);
	Init_Word(D_RET, Scan_Issue(&buffer[0], len));
	VAL_SET(D_RET, REB_ISSUE);

	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(find_script)
/*
***********************************************************************/
{
	REBVAL *arg = D_ARG(1);
	REBINT n;

	n = What_UTF(VAL_BIN_DATA(arg), VAL_LEN(arg));

	if (n != 0 && n != 8) return R_NONE;  // UTF8 only

	if (n == 8) VAL_INDEX(arg) += 3;  // BOM8 length

	n = Scan_Header(VAL_BIN_DATA(arg), VAL_LEN(arg)); // returns offset

	if (n == -1) return R_NONE;

	VAL_INDEX(arg) += n;

	return R_ARG1;
}


/***********************************************************************
**
*/	REBNATIVE(utfq)
/*
***********************************************************************/
{
	REBINT utf = What_UTF(VAL_BIN_DATA(D_ARG(1)), VAL_LEN(D_ARG(1)));
	DS_RET_INT(utf);
	return R_RET;
}


/***********************************************************************
**
*/	REBNATIVE(invalid_utfq)
/*
***********************************************************************/
{
	REBVAL *arg = D_ARG(1);
	const REBYTE *bp;

	bp = Check_UTF8(VAL_BIN_DATA(arg), VAL_LEN(arg));
	if (bp == 0) return R_NONE;

	VAL_INDEX(arg) = bp - VAL_BIN_HEAD(arg);
	return R_ARG1;
}
