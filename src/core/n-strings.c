/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2026 Rebol Open Source Contributors
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

#ifdef INCLUDE_DEFLATE
#define ADLER32_FUNC libdeflate_adler32
#else
#define ADLER32_FUNC z_adler32_z
#endif

REBCNT ADLER32_FUNC(REBCNT adler, REBYTE *buf, REBCNT len);
REBSER *Make_Binary_BE64(REBVAL *arg);

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
#ifdef INCLUDE_SHA3
	{SHA3_224, SHA3_224_Starts,   SHA3_Update,   SHA3_224_Finish,   SHA3_CtxSize, SYM_SHA3_224, 28, 64},
	{SHA3_256, SHA3_256_Starts,   SHA3_Update,   SHA3_256_Finish,   SHA3_CtxSize, SYM_SHA3_256, 32, 64},
	{SHA3_384, SHA3_384_Starts,   SHA3_Update,   SHA3_384_Finish,   SHA3_CtxSize, SYM_SHA3_384, 48, 128},
	{SHA3_512, SHA3_512_Starts,   SHA3_Update,   SHA3_512_Finish,   SHA3_CtxSize, SYM_SHA3_512, 64, 128},
#endif
#ifdef INCLUDE_XXHASH
	{HashXXH3,   XXH3_Starts,   XXH3_Update,   XXH3_Finish,   XXH3_CtxSize,   SYM_XXH3,    8, 64},
	{HashXXH32,  XXH32_Starts,  XXH32_Update,  XXH32_Finish,  XXH32_CtxSize,  SYM_XXH32,   4, 64},
	{HashXXH64,  XXH64_Starts,  XXH64_Update,  XXH64_Finish,  XXH64_CtxSize,  SYM_XXH64,   8, 64},
	{HashXXH128, XXH128_Starts, XXH128_Update, XXH128_Finish, XXH128_CtxSize, SYM_XXH128, 16, 64},
#endif
	{0}

};


/***********************************************************************
**
*/	REBOOL Message_Digest(REBYTE *output, REBYTE *input, REBCNT length, REBCNT method, REBCNT *olen)
/*
***********************************************************************/
{
	switch (method) {
	case SYM_MD5:
		MD5(input, length, output);
		*olen = 16;
		break;
	case SYM_SHA1:
		SHA1(input, length, output);
		*olen = 20;
		break;
	case SYM_SHA256:
		SHA256(input, length, output);
		*olen = 32;
		break;
	case SYM_SHA224:
		SHA224(input, length, output);
		*olen = 28;
		break;
	case SYM_SHA512:
		SHA512(input, length, output);
		*olen = 64;
		break;
#ifdef INCLUDE_SHA384
	case SYM_SHA384:
		SHA384(input, length, output);
		*olen = 48;
		break;
#endif
#ifdef INCLUDE_SHA3
	case SYM_SHA3_224:
		SHA3_224(input, length, output);
		*olen = 28;
		break;
	case SYM_SHA3_256:
		SHA3_256(input, length, output);
		*olen = 32;
		break;
	case SYM_SHA3_384:
		SHA3_384(input, length, output);
		*olen = 48;
		break;
	case SYM_SHA3_512:
		SHA3_512(input, length, output);
		*olen = 64;
		break;
#endif
#ifdef INCLUDE_RIPEMD160
	case SYM_RIPEMD160:
		RIPEMD160(input, length, output);
		*olen = 20;
		break;
#endif
#ifdef INCLUDE_MD4
	case SYM_MD4:
		MD4(input, length, output);
		*olen = 16;
		break;
#endif
#ifdef INCLUDE_XXHASH
	case SYM_XXH3:
		HashXXH3(input, length, output);
		*olen = 8;
		break;
	case SYM_XXH32:
		HashXXH32(input, length, output);
		*olen = 4;
		break;
	case SYM_XXH64:
		HashXXH64(input, length, output);
		*olen = 8;
		break;
	case SYM_XXH128:
		HashXXH128(input, length, output);
		*olen = 16;
		break;
#endif
	default:
		return FALSE;
	}
	return TRUE;
}


/***********************************************************************
**
*/	REBNATIVE(ajoin)
/*
***********************************************************************/
{
	REBSER *str;
	REBCNT type = VAL_TYPE(VAL_BLK_DATA(D_ARG(1)));
	REBVAL *delimiter = D_REF(2) ? D_ARG(3) : NULL;

	str = Form_Reduce(VAL_SERIES(D_ARG(1)), VAL_INDEX(D_ARG(1)), delimiter, D_REF(4));
	if (!str) return R_TOS;

	// Use result string-like type based on first value, except tag!
	if (type < REB_STRING || type >= REB_TAG) type = REB_STRING;

	//  Using DS_RETURN not D_RET (stack modified)
	VAL_SET(DS_RETURN, type);
	VAL_SERIES(DS_RETURN) = str;
	VAL_INDEX(DS_RETURN) = 0;
	VAL_SERIES_SIDE(DS_RETURN) = 0;

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
//		data [binary! string! file!] {If string, it will be UTF8 encoded. File is dispatched to file-checksum function.}
//		method [word!] {One of `system/catalog/checksums` and HASH}
//		/with {Extra value for HMAC key or hash table size; not compatible with TCP/CRC24/CRC32/ADLER32 methods.}
//		 spec [any-string! binary! integer!] {String or binary for MD5/SHA* HMAC key, integer for hash table size.}
//		/part {Limits to a given length}
//		 length
//	]
***********************************************************************/
{
	REBVAL *data   = D_ARG(ARG_CHECKSUM_DATA);
	REBVAL *method = D_ARG(ARG_CHECKSUM_METHOD);
	REBVAL *spec   = D_ARG(ARG_CHECKSUM_SPEC);
	REBINT sym;
	REBCNT sum = 0;
	REBLEN i = 0;
	REBLEN j;
	REBSER *digest;
	REBCNT len, keylen;
	REBYTE *bin;
	REBYTE *keycp;


	len = Partial1(data, D_ARG(ARG_CHECKSUM_LENGTH));
	sym = VAL_WORD_CANON(method);

	if (IS_BINARY(data) || IS_STRING(data)) {
		bin = VAL_BIN_DATA(data);
	}
	else {
		// Dispatch file to file-checksum function...
		REBVAL *func = Find_Word_Value(Lib_Context, SYM_FILE_CHECKSUM);
		if (func && IS_FUNCTION(func) && sym > SYM_CRC32 && sym <= SYM_SHA3_512) {
			if (D_REF(ARG_CHECKSUM_WITH) || D_REF(ARG_CHECKSUM_PART))
				Trap0(RE_BAD_REFINES);

			// Build block to evaluate: [file-checksum data method]
			// Where method must be converted to lit-word first...
			VAL_TYPE(method) = REB_LIT_WORD;
			REBSER *code = Make_Block(3);
			BLK_HEAD(code)[0] = *func;
			BLK_HEAD(code)[1] = *data;
			BLK_HEAD(code)[2] = *method;
			SERIES_TAIL(code) = 3; // It is important to mark the tail of the block!
			Do_Next(code, 0, 0);
			*DS_RETURN = *DS_TOP;
			return R_RET;
		}
		// in case that file-checksum is not a function or is used not yet implemented method...
		Trap0(RE_FEATURE_NA);
		return R_NONE; // some compilers don't understand, that we stop evaluation here:/
	}
	
	if (sym > SYM_CRC32 && sym <= SYM_SHA3_512) {
		// O: could be optimized using index computed from `sym`
		// find matching digest:
		for (i = 0; i < sizeof(digests) / sizeof(digests[0]); i++) {

			if (digests[i].index == sym) {

				digest = Make_Series(digests[i].len, 1, FALSE);
				LABEL_SERIES(digest, "checksum digest");

				if (D_REF(ARG_CHECKSUM_WITH)) {	// HMAC
					if (IS_INTEGER(spec))
						Trap1(RE_BAD_REFINE, D_ARG(ARG_CHECKSUM_SPEC));

					// UTF-8 encoded...
					keycp = VAL_BIN_DATA(spec);
					keylen = VAL_LEN(spec);

					REBYTE tmpdigest[128];		// Size must be max of all digest[].len;
					REBYTE ipad[128],opad[128];	// Size must be max of all digest[].hmacblock;
					void *ctx = Make_CMem(digests[i].ctxsize());

					REBCNT blocklen = digests[i].hmacblock;

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
					digests[i].update(ctx, bin, len);
					digests[i].final(ctx, tmpdigest);
					digests[i].init(ctx);
					digests[i].update(ctx,opad,blocklen);
					digests[i].update(ctx,tmpdigest,digests[i].len);
					digests[i].final(ctx,BIN_HEAD(digest));

					Free_Mem(ctx, digests[i].ctxsize());

				} else {
					digests[i].digest(bin, len, BIN_HEAD(digest));
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
		sum = (sym == SYM_CRC32) ? CRC32(bin, len) : ADLER32_FUNC(0x00000001L, bin, len);
	}
	else if (sym == SYM_HASH) {  // /hash
		if(!D_REF(ARG_CHECKSUM_WITH)) Trap0(RE_MISSING_ARG);
		if (!IS_INTEGER(spec)) Trap1(RE_BAD_REFINE, D_ARG(ARG_CHECKSUM_SPEC));
		sum = (REBCNT)MAX(1, VAL_INT64(spec)); // size of the hash table
		sum = Hash_Value(data) % sum;
	}
	else if (sym == SYM_CRC24) {
		sum = Compute_CRC24(bin, len);
	}
	else if (sym == SYM_TCP) {
		sum = Compute_IPC(bin, len);
	}
	else {
		Trap_Arg(D_ARG(ARG_CHECKSUM_METHOD));
	}

	DS_RET_INT(sum);
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
		// Just a guess at size:
		frame = Make_Block(10);		// Use a std BUF_?
		Set_Block(D_RET, frame);	// Keep safe

		// !issue! Is this what we really want here?
		Scan_Net_Header(frame, VAL_DATA(value));
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
	REBVAL *arg = D_ARG(1);
	REBINT base = VAL_INT32(D_ARG(2));
	REBVAL *part = D_ARG(5);
	REBCNT len = 0;

	if (D_REF(4)) {
		len = Partial(arg, 0, part, 0); // Can modify value index.
		if (len == 0) {
			Set_Binary(D_RET, Make_Binary(0));
			return R_RET;
		}
	}
	else {
		len = VAL_LEN(arg);
	}
	if (!Decode_Binary(D_RET, VAL_DATA(arg), len, base, 0, D_REF(3)))
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
	REBVAL *arg = D_ARG(1);
	REBINT base = VAL_INT32(D_ARG(2));
	REBCNT limit = NO_LIMIT;
	REBVAL *part = D_ARG(5);
	REBOOL brk = !D_REF(6);

	if (IS_INTEGER(arg)) {
		if (VAL_INT64(arg) != 0) {
			// Convert integer to binary...
			SET_BINARY(arg, Make_Binary_BE64(arg));
			// ...and trim leading zeros...
			REBCNT i = 0;
			while (i <= 7 && VAL_BIN_HEAD(arg)[i] == 0) { ++i; }
			VAL_INDEX(arg) = i;
		}
		else {
			// Special case for e.g.: enbase 0 16 ;== "00"
			SET_BINARY(arg, Make_Binary(1));
			VAL_TAIL(arg) = 1;
		}
	}
	if (D_REF(4)) {
		limit = Partial(arg, 0, part, 0); // Can modify value index.
		if (limit == 0) {
			Set_String(D_RET, Make_Binary(0));
			return R_RET;
		}
	}

	switch (base) {
	case 64:
		ser = Encode_Base64(arg, 0, limit, brk, D_REF(3));
		break;
	case 16:
		ser = Encode_Base16(arg, 0, limit, brk);
		break;
	case 2:
		ser = Encode_Base2(arg, 0, limit, brk);
		break;
	case 85:
#ifdef INCLUDE_BASE85
		ser = Encode_Base85(arg, 0, limit, FALSE);
#else
		Trap0(RE_FEATURE_NA);
#endif
		break;
	case 36:
#ifdef INCLUDE_BASE36
		ser = Encode_Base36(arg, 0, limit, FALSE);
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
*/	REBSER* Dehex_String(REBYTE* input, REBLEN length, const REBCHR escape_char, REBFLG as_uri)
/*
***********************************************************************/
{
	REBU32 n;
	REBYTE* bp, * dp;
	REBI64 len = (REBI64)length;
	const REBCHR space_char = escape_char == '=' ? '_' : '+';

	bp = input;
	dp = Reset_Buffer(BUF_SCAN, len + 1); // count also the terminating null byte

	for (; len > 0; len--) {
		if (*bp == escape_char && len > 2 && Scan_Hex2(bp + 1, &n)) {
			*dp++ = (REBYTE)n;
			bp += 3;
			len -= 2;
		}
		else if (*bp == space_char && as_uri) {
			*dp++ = ' ';
			bp++;
		}
		else {
			*dp++ = *bp++;
		}
	}
	*dp = 0;
	return Copy_String(BUF_SCAN, 0, AS_REBLEN(dp - BIN_HEAD(BUF_SCAN)));
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
//	REBOOL as_uri      = D_REF(4);
	REBSER *ser;

	const REBCHR escape_char = D_REF(2) ? VAL_CHAR(D_ARG(3)) : '%';

	ASSERT1(VAL_BYTE_SIZE(arg), RP_BAD_SIZE);
	ser = Dehex_String(VAL_BIN_DATA(arg), VAL_LEN(arg), escape_char, D_REF(4));
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
	REBOOL  no_space   = D_REF(6);
	REBYTE encoded[4];
	REBCNT n, encoded_size;
	REBSER *ser;
	const REBCHR escape_char = D_REF(2) ? VAL_CHAR(D_ARG(3)) : '%';
	const REBCHR space_char = escape_char == '=' ? '_' : '+';

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
	REBYTE *dp = Reset_Buffer(BUF_SCAN, 12 * VAL_LEN(arg));

	if (VAL_BYTE_SIZE(arg)) {
		// byte size is 1, so the series should not contain chars with value over 0x80
		// see: https://github.com/Oldes/Rebol3/commit/aa939ba41fd71efb9f0a97f85993c95129c7e515

		REBYTE *bp = VAL_BIN_DATA(arg);
		REBYTE *ep = VAL_BIN_TAIL(arg);		

		while (bp < ep) {
			REBYTE c = bp[0];
			bp++;
			if (no_space) {
				if (c == ' ') {
					*dp++ = space_char;
					continue;
				}
				if (c == space_char) goto escaped_ascii;
			}
			if (Check_Bit_Cased(VAL_SERIES(val_bitset), c)) {
				*dp++ = c;
				continue;
			}
		escaped_ascii:
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
			if (no_space) {
				if (c == ' ') {
					*dp++ = space_char;
					continue;
				}
				if (c == space_char) {
					encoded_size = Encode_UTF8_Char(encoded, c);
					goto escaped_uni;
				}
			}
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
		escaped_uni:
			for (n = 0; n < encoded_size; ++n) {
				*dp++ = escape_char;
				*dp++ = Hex_Digits[(encoded[n] & 0xf0) >> 4];
				*dp++ = Hex_Digits[ encoded[n] & 0xf];
			}
		}
	}
	*dp = 0;
	ser = Copy_String(BUF_SCAN, 0, AS_REBLEN(dp - BIN_HEAD(BUF_SCAN)));
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
	ASSERT1(VAL_BYTE_SIZE(val), RP_BAD_SIZE);
	ser = Entab_Bytes(VAL_BIN(val), VAL_INDEX(val), len, tabsize);
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
	ASSERT1(VAL_BYTE_SIZE(val), RP_BAD_SIZE);
	ser = Detab_Bytes(VAL_BIN(val), VAL_INDEX(val), len, tabsize);
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
	REBCNT len;
//	REBSER *series;
	REBYTE buffer[MAX_TUPLE*2+4];  // largest value possible
	REBYTE *buf = &buffer[0];

	if (D_REF(2)) {	// /size
		//@@ https://github.com/Oldes/Rebol-issues/issues/127
		if (VAL_INT64(D_ARG(3)) <= 0 || VAL_UNT64(D_ARG(3)) > MAX_U32)
			Trap_Arg(D_ARG(3));
		len = VAL_UNT32(D_ARG(3));
	}
	else if (IS_CHAR(arg)) {
		len = VAL_UNT32(arg);
		if (len <= 0xFF) len = 2;
		else if (len <= 0xFFFF) len = 4;
		else if (len <= 0xFFFFFF) len = 6;
		else len = 8;
	}
	else {
		len = NO_LIMIT;
	}

	if (IS_INTEGER(arg) || IS_CHAR(arg)) {
		if (len == NO_LIMIT || len > MAX_HEX_LEN) len = MAX_HEX_LEN;
		Form_Hex_Pad(buf, VAL_INT64(arg), len);
	}
	else if (IS_TUPLE(arg)) {
		REBCNT n;
		if (len == NO_LIMIT || len > 2 * MAX_TUPLE || len > 2 * VAL_TUPLE_LEN(arg))
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
	REBFLG surrogates;

	bp = UTF8_Check(VAL_BIN_DATA(arg), VAL_LEN(arg), &surrogates);
	if (bp == 0) return R_NONE;

	VAL_INDEX(arg) = AS_REBLEN(bp - VAL_BIN_HEAD(arg));
	return R_ARG1;
}


/***********************************************************************
**
*/	REBNATIVE(split_lines)
/*
**  Note: It should be part of the new `split` native in the future!
**
***********************************************************************/
{
	REBVAL *arg = D_ARG(1);

	Set_Block(D_RET, Split_Lines(arg));
	return R_RET;
}
