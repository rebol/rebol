/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2023 Rebol Open Source Contributors
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
**  Module:  p-crypt.c
**  Summary: Cryptography port interface
**  Section: ports
**  Author:  Oldes
**  Notes:
**
***********************************************************************/

#if !defined(REBOL_OPTIONS_FILE)
#include "opt-config.h"
#else
#include REBOL_OPTIONS_FILE
#endif

#include "sys-crypt.h"
#include "reb-net.h"

#ifdef INCLUDE_MBEDTLS

#ifndef MBEDTLS_GET_UINT32_BE
// including it here as it is defined only for private use in the mbedTLS sources
#define MBEDTLS_GET_UINT32_BE( data , offset )                  \
    (                                                           \
          ( (uint32_t) ( data )[( offset )    ] << 24 )         \
        | ( (uint32_t) ( data )[( offset ) + 1] << 16 )         \
        | ( (uint32_t) ( data )[( offset ) + 2] <<  8 )         \
        | ( (uint32_t) ( data )[( offset ) + 3]       )         \
    )
#endif

// This function was defined as `static` in the mbedtls sources, but I want to use it here as well.
extern int mbedtls_ccm_compare_tags(const unsigned char *tag1, const unsigned char *tag2, size_t tag_len);


/***********************************************************************
**
*/	static mbedtls_cipher_id_t get_cipher_id(REBCNT sym)
/*
**  Note: requires exact word order defined in boot/words.reb file
**
***********************************************************************/
{
	if (sym >= SYM_AES_128_ECB && sym <= SYM_AES_256_GCM) {
		return MBEDTLS_CIPHER_ID_AES;
	}
	if (sym >= SYM_CAMELLIA_128_ECB && sym <= SYM_CAMELLIA_256_GCM) {
		return MBEDTLS_CIPHER_ID_CAMELLIA;
	}
	if (sym >= SYM_ARIA_128_ECB && sym <= SYM_ARIA_256_GCM) {
		return MBEDTLS_CIPHER_ID_ARIA;
	}
	if (sym >= SYM_CHACHA20 && sym <= SYM_CHACHA20_POLY1305) {
		return MBEDTLS_CIPHER_ID_CHACHA20;
	}
	return MBEDTLS_CIPHER_ID_NULL;
}


static void free_crypt_cipher_context(CRYPT_CTX *ctx);


/***********************************************************************
**
*/	void crypt_context_free(void *ctx)
/*
***********************************************************************/
{
	CRYPT_CTX *crypt;
	if (ctx == NULL) return;
	crypt = (CRYPT_CTX *)ctx;
	if (crypt->buffer) {
		CLEAR(crypt->buffer->data, crypt->buffer->rest);
		Free_Series(crypt->buffer);
	}
	free_crypt_cipher_context(crypt);
	CLEARS(crypt);
}


/***********************************************************************
**
*/	static REBOOL init_crypt_key(CRYPT_CTX *ctx, REBVAL *val)
/*
***********************************************************************/
{
	REBSER *ser;
	REBCNT  len = 0;
	REBYTE *bin = NULL;
	if (val == NULL) return FALSE;
	ctx->state = CRYPT_PORT_NEEDS_INIT;
	if (IS_NONE(val)) {
		CLEAR(&ctx->key, MBEDTLS_MAX_KEY_LENGTH);
		return TRUE;
	}
	if (IS_STRING(val)) {
		ser = Encode_UTF8_Value(val, VAL_LEN(val), 0);
		len = SERIES_TAIL(ser);
		bin = BIN_HEAD(ser);
	}
	else if (IS_BINARY(val)) {
		len = VAL_LEN(val);
		bin = VAL_BIN_AT(val);
	}
	if (bin && len > 0) {
		if (len > MBEDTLS_MAX_KEY_LENGTH)
			len = MBEDTLS_MAX_KEY_LENGTH;
		CLEAR(&ctx->key, MBEDTLS_MAX_KEY_LENGTH);
		COPY_MEM(&ctx->key, bin, len);
		return TRUE;
	}
	return FALSE;
}

/***********************************************************************
**
*/	static REBOOL init_crypt_iv(CRYPT_CTX *ctx, REBVAL *val)
/*
***********************************************************************/
{
	REBCNT len;
	if (val == NULL) return FALSE;
	ctx->state = CRYPT_PORT_NEEDS_INIT;
	if (IS_NONE(val)) {
		CLEAR(&ctx->IV, MBEDTLS_MAX_IV_LENGTH);
		CLEAR(&ctx->nonce, MBEDTLS_MAX_IV_LENGTH);
		ctx->IV_len = 0;
		return TRUE;
	}
	if (IS_BINARY(val)) {
		len = VAL_LEN(val);
		if (len > 0) {
			if (len > MBEDTLS_MAX_IV_LENGTH)
				len = MBEDTLS_MAX_IV_LENGTH;
			CLEAR(&ctx->IV, MBEDTLS_MAX_IV_LENGTH);
			CLEAR(&ctx->nonce, MBEDTLS_MAX_IV_LENGTH);
			COPY_MEM(&ctx->IV, VAL_BIN_AT(val), len);
			ctx->IV_len = len;
		}
		return TRUE;
	}
	return FALSE;
}

/***********************************************************************
**
*/	static REBOOL init_crypt_direction(CRYPT_CTX *ctx, REBVAL *val)
/*
***********************************************************************/
{
	if (!val || !IS_WORD(val)) return FALSE;
	ctx->state = CRYPT_PORT_NEEDS_INIT;
	switch (VAL_WORD_CANON(val)) {
	case SYM_ENCRYPT:
		ctx->operation = MBEDTLS_ENCRYPT;
		break;
	case SYM_DECRYPT:
		ctx->operation = MBEDTLS_DECRYPT;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************
**
*/	static void free_crypt_cipher_context(CRYPT_CTX *ctx)
/*
***********************************************************************/
{
	REBCNT type = ctx->cipher_type;
	if (ctx->cipher_ctx == NULL) return;
	ctx->state = CRYPT_PORT_CLOSED;
	switch (type) {
	case SYM_AES_128_ECB:
	case SYM_AES_192_ECB:
	case SYM_AES_256_ECB:
	case SYM_AES_128_CBC:
	case SYM_AES_192_CBC:
	case SYM_AES_256_CBC:
		mbedtls_aes_free((mbedtls_aes_context *)ctx->cipher_ctx);
		break;


#ifdef MBEDTLS_CAMELLIA_C
	case SYM_CAMELLIA_128_ECB:
	case SYM_CAMELLIA_192_ECB:
	case SYM_CAMELLIA_256_ECB:
	case SYM_CAMELLIA_128_CBC:
	case SYM_CAMELLIA_192_CBC:
	case SYM_CAMELLIA_256_CBC:
		mbedtls_camellia_free((mbedtls_camellia_context *)ctx->cipher_ctx);
		break;
#endif


#ifdef MBEDTLS_ARIA_C
	case SYM_ARIA_128_ECB:
	case SYM_ARIA_192_ECB:
	case SYM_ARIA_256_ECB:
	case SYM_ARIA_128_CBC:
	case SYM_ARIA_192_CBC:
	case SYM_ARIA_256_CBC:
		mbedtls_aria_free((mbedtls_aria_context *)ctx->cipher_ctx);
		break;
#endif


#ifdef MBEDTLS_CHACHA20_C
	case SYM_CHACHA20:
		mbedtls_chacha20_free((mbedtls_chacha20_context *)ctx->cipher_ctx);
		break;
#endif


#ifdef MBEDTLS_CHACHAPOLY_C
	case SYM_CHACHA20_POLY1305:
		mbedtls_chachapoly_free((mbedtls_chachapoly_context *)ctx->cipher_ctx);
		break;
#endif
	}
	free(ctx->cipher_ctx);
	ctx->cipher_ctx = NULL;
}



/***********************************************************************
**
*/	static REBOOL init_crypt_algorithm(CRYPT_CTX *ctx, REBVAL *val)
/*
***********************************************************************/
{
	REBCNT type;

	if (!IS_WORD(val)) return FALSE;
	type = VAL_WORD_CANON(val);
	if (type != ctx->cipher_type) {
		free_crypt_cipher_context(ctx);
	}
	ctx->state = CRYPT_PORT_NEEDS_INIT;
	switch (type) {

	case SYM_AES_128_ECB:
	case SYM_AES_192_ECB:
	case SYM_AES_256_ECB:
#ifdef MBEDTLS_CIPHER_MODE_CBC
	case SYM_AES_128_CBC:
	case SYM_AES_192_CBC:
	case SYM_AES_256_CBC:
#endif
		if (ctx->cipher_ctx == NULL)
			ctx->cipher_ctx = malloc(sizeof(mbedtls_aes_context));
		mbedtls_aes_init((mbedtls_aes_context*)ctx->cipher_ctx);
		switch (type) {
		case SYM_AES_128_ECB:
		case SYM_AES_128_CBC: ctx->key_bitlen = 128; break;
		case SYM_AES_192_ECB:
		case SYM_AES_192_CBC: ctx->key_bitlen = 192; break;
		case SYM_AES_256_ECB:
		case SYM_AES_256_CBC: ctx->key_bitlen = 256; break;
		}
		ctx->cipher_block_size = 16;
		break;

#ifdef MBEDTLS_CCM_C
	case SYM_AES_128_CCM:
	case SYM_AES_192_CCM:
	case SYM_AES_256_CCM:
#ifdef MBEDTLS_CAMELLIA_C
	case SYM_CAMELLIA_128_CCM:
	case SYM_CAMELLIA_192_CCM:
	case SYM_CAMELLIA_256_CCM:
#endif
#ifdef MBEDTLS_ARIA_C
	case SYM_ARIA_128_CCM:
	case SYM_ARIA_192_CCM:
	case SYM_ARIA_256_CCM:
#endif
		if (ctx->cipher_ctx == NULL)
			ctx->cipher_ctx = malloc(sizeof(mbedtls_ccm_context));
		ctx->cipher_mode = MBEDTLS_MODE_CCM;
		mbedtls_ccm_init((mbedtls_ccm_context*)ctx->cipher_ctx);
		switch (type) {
		case SYM_AES_128_CCM:
		case SYM_ARIA_128_CCM:
		case SYM_CAMELLIA_128_CCM: ctx->key_bitlen = 128; break;
		case SYM_AES_192_CCM:
		case SYM_ARIA_192_CCM:
		case SYM_CAMELLIA_192_CCM: ctx->key_bitlen = 192; break;
		case SYM_AES_256_CCM:
		case SYM_ARIA_256_CCM:
		case SYM_CAMELLIA_256_CCM: ctx->key_bitlen = 256; break;
		}
		ctx->cipher_block_size = 0;
		break;

#endif

#ifdef MBEDTLS_GCM_C
	case SYM_AES_128_GCM:
	case SYM_AES_192_GCM:
	case SYM_AES_256_GCM:
#ifdef MBEDTLS_CAMELLIA_C
	case SYM_CAMELLIA_128_GCM:
	case SYM_CAMELLIA_192_GCM:
	case SYM_CAMELLIA_256_GCM:
#endif
#ifdef MBEDTLS_ARIA_C
	case SYM_ARIA_128_GCM:
	case SYM_ARIA_192_GCM:
	case SYM_ARIA_256_GCM:
#endif
		if (ctx->cipher_ctx == NULL)
			ctx->cipher_ctx = malloc(sizeof(mbedtls_gcm_context));
		mbedtls_gcm_init((mbedtls_gcm_context *)ctx->cipher_ctx);
		ctx->cipher_mode = MBEDTLS_MODE_GCM;
		switch (type) {
		case SYM_AES_128_GCM:
		case SYM_ARIA_128_GCM:
		case SYM_CAMELLIA_128_GCM: ctx->key_bitlen = 128; break;
		case SYM_AES_192_GCM:
		case SYM_ARIA_192_GCM:
		case SYM_CAMELLIA_192_GCM: ctx->key_bitlen = 192; break;
		case SYM_AES_256_GCM:
		case SYM_ARIA_256_GCM:
		case SYM_CAMELLIA_256_GCM: ctx->key_bitlen = 256; break;
		}
		ctx->cipher_block_size = 0;
		break;
#endif

#ifdef MBEDTLS_CAMELLIA_C
	case SYM_CAMELLIA_128_ECB:
	case SYM_CAMELLIA_192_ECB:
	case SYM_CAMELLIA_256_ECB:
#ifdef MBEDTLS_CIPHER_MODE_CBC
	case SYM_CAMELLIA_128_CBC:
	case SYM_CAMELLIA_192_CBC:
	case SYM_CAMELLIA_256_CBC:
#endif
		if (ctx->cipher_ctx == NULL)
			ctx->cipher_ctx = malloc(sizeof(mbedtls_camellia_context));
		switch (type) {
		case SYM_CAMELLIA_128_ECB:
		case SYM_CAMELLIA_128_CBC: ctx->key_bitlen = 128; break;
		case SYM_CAMELLIA_192_ECB:
		case SYM_CAMELLIA_192_CBC: ctx->key_bitlen = 192; break;
		case SYM_CAMELLIA_256_ECB:
		case SYM_CAMELLIA_256_CBC: ctx->key_bitlen = 256; break;
		}
		ctx->cipher_block_size = 16;
		break;
#endif


#ifdef MBEDTLS_ARIA_C
	case SYM_ARIA_128_ECB:
	case SYM_ARIA_192_ECB:
	case SYM_ARIA_256_ECB:
#ifdef MBEDTLS_CIPHER_MODE_CBC
	case SYM_ARIA_128_CBC:
	case SYM_ARIA_192_CBC:
	case SYM_ARIA_256_CBC:
#endif
		if (ctx->cipher_ctx == NULL)
			ctx->cipher_ctx = malloc(sizeof(mbedtls_camellia_context));
		switch (type) {
		case SYM_ARIA_128_ECB:
		case SYM_ARIA_128_CBC: ctx->key_bitlen = 128; break;
		case SYM_ARIA_192_ECB:
		case SYM_ARIA_192_CBC: ctx->key_bitlen = 192; break;
		case SYM_ARIA_256_ECB:
		case SYM_ARIA_256_CBC: ctx->key_bitlen = 256; break;
		}
		ctx->cipher_block_size = 16;
		break;
#endif


#ifdef MBEDTLS_CHACHA20_C
	case SYM_CHACHA20:
		if (ctx->cipher_ctx == NULL)
			ctx->cipher_ctx = malloc(sizeof(mbedtls_chacha20_context));
		mbedtls_chacha20_init((mbedtls_chacha20_context *)ctx->cipher_ctx);
		ctx->cipher_block_size = 16U;
		break;
#endif


#ifdef MBEDTLS_CHACHAPOLY_C
	case SYM_CHACHA20_POLY1305:
		if (ctx->cipher_ctx == NULL)
			ctx->cipher_ctx = malloc(sizeof(CHACHAPOLY_CTX));
		mbedtls_chachapoly_init((CHACHAPOLY_CTX *)ctx->cipher_ctx);
		ctx->cipher_block_size = 0;
		break;
#endif
	}
	ctx->cipher_type = type;
	return TRUE;
}


/***********************************************************************
**
*/	static REBOOL Crypt_Open(REBSER *port)
/*
***********************************************************************/
{
	REBVAL *spec;
	REBVAL *state;
	REBVAL *val;
	REBINT  err = 0;
	CRYPT_CTX *ctx;

	spec = BLK_SKIP(port, STD_PORT_SPEC);
	if (!IS_OBJECT(spec)) Trap1(RE_INVALID_SPEC, spec);

	state = BLK_SKIP(port, STD_PORT_STATE);
	MAKE_HANDLE(state, SYM_CRYPT);
	ctx = (CRYPT_CTX *)VAL_HANDLE_CONTEXT_DATA(state);
	
	if (NOT_VALID_CONTEXT_HANDLE(state, SYM_CRYPT)) {
		Trap0(RE_INVALID_HANDLE);
		return FALSE;
	}

	val = Obj_Value(spec, STD_PORT_SPEC_CRYPT_ALGORITHM);
	if (!init_crypt_algorithm(ctx, val)) { err = 1;  goto failed; }

	val = Obj_Value(spec, STD_PORT_SPEC_CRYPT_INIT_VECTOR);
	if (!init_crypt_iv(ctx, val)) { err = 1; goto failed; }
	SET_NONE(val); // as we have a copy, make it invisible from the spec

	val = Obj_Value(spec, STD_PORT_SPEC_CRYPT_KEY);
	if(!init_crypt_key(ctx, val)) { err = 1; goto failed; }
	SET_NONE(val); // as we have a copy, make it invisible from the spec

	val = Obj_Value(spec, STD_PORT_SPEC_CRYPT_DIRECTION);
	if (!init_crypt_direction(ctx, val)) { err = 1; goto failed; }

	ctx->buffer = Make_Binary(256);
	// buffer is extended when needed.
	// protected using KEEP, because it is not accesible from any real Rebol value!
	KEEP_SERIES(ctx->buffer, "crypt");

	ctx->state = CRYPT_PORT_NEEDS_INIT;

	return TRUE;

failed:
	if (IS_HANDLE(spec))
		Free_Hob(VAL_HANDLE_CTX(spec));
	if (err == 1) {
		Trap1(RE_INVALID_SPEC, spec);
	}
	Trap_Port(RE_CANNOT_OPEN, port, err);
	return FALSE;
}


/***********************************************************************
**
*/	static REBINT Crypt_Crypt(CRYPT_CTX *ctx, REBYTE *input, REBCNT len, REBCNT *olen)
/*
***********************************************************************/
{
	REBINT  err;
	REBSER *bin;
	REBCNT  blk, ofs;
	REBYTE *start = input;

	*olen = 0;

	bin = ctx->buffer;
	blk = ctx->cipher_block_size;

	// make sure, that input is not less than required block size
	// unhandled input data are stored later in the ctx->unprocessed_data
	if (blk > 0 && len < blk) return 0;

	// make space at tail if needed...
	Extend_Series(bin, len);

	switch (ctx->cipher_type) {

	case SYM_AES_128_ECB:
	case SYM_AES_192_ECB:
	case SYM_AES_256_ECB:
		for (ofs = 0; ofs <= len - blk; ofs += blk) {
			err = mbedtls_aes_crypt_ecb((mbedtls_aes_context *)ctx->cipher_ctx, ctx->operation, input, BIN_TAIL(bin));
			if (err) return err;
			SERIES_TAIL(bin) += blk;
			ctx->state = CRYPT_PORT_HAS_DATA;
			input += blk;
		}
		break;
#ifdef MBEDTLS_CIPHER_MODE_CBC
	case SYM_AES_128_CBC:
	case SYM_AES_192_CBC:
	case SYM_AES_256_CBC:
		blk = len - (len % 16);
		err = mbedtls_aes_crypt_cbc((mbedtls_aes_context *)ctx->cipher_ctx, ctx->operation, blk, ctx->IV, input, BIN_TAIL(bin));
		if (err) return err;
		SERIES_TAIL(bin) += blk;
		ctx->state = CRYPT_PORT_HAS_DATA;
		input += blk;
		break;
#endif

#ifdef MBEDTLS_CCM_C
	case SYM_AES_128_CCM:
	case SYM_AES_192_CCM:
	case SYM_AES_256_CCM:
#ifdef MBEDTLS_CAMELLIA_C
	case SYM_CAMELLIA_128_CCM:
	case SYM_CAMELLIA_192_CCM:
	case SYM_CAMELLIA_256_CCM:
#endif
#ifdef MBEDTLS_ARIA_C
	case SYM_ARIA_128_CCM:
	case SYM_ARIA_192_CCM:
	case SYM_ARIA_256_CCM:
#endif
	{
		size_t out_bytes = 0;
		REBLEN plaintext_len = len - ctx->aad_len;
		mbedtls_ccm_context* ccm = (mbedtls_ccm_context*)ctx->cipher_ctx;

		if (ctx->operation == MBEDTLS_DECRYPT) {
			plaintext_len -= ctx->tag_len;
		}

		if (plaintext_len < 0) return -0x0001;
		err = mbedtls_ccm_set_lengths(ccm, ctx->aad_len, plaintext_len, ctx->tag_len);
		if (err) return err;
		if (ctx->aad_len) {
			err = mbedtls_ccm_update_ad(ccm, input, ctx->aad_len);
			if (err) return err;
			input += ctx->aad_len;
			len   -= ctx->aad_len;
		}
		err = mbedtls_ccm_update(ccm, input, plaintext_len, BIN_TAIL(bin), len, &out_bytes);
		if (err) return err;
		SERIES_TAIL(bin) += (REBLEN)out_bytes;
		ctx->state = CRYPT_PORT_HAS_DATA;
		input += out_bytes;

		if (ctx->tag_len) {
			// compute tag
			Extend_Series(bin, ctx->tag_len);
			err = mbedtls_ccm_finish(ccm, BIN_TAIL(bin), ctx->tag_len);
			if (err) return err;
			if (ctx->operation == MBEDTLS_DECRYPT) {
				err = mbedtls_ccm_compare_tags(input, BIN_TAIL(bin), ctx->tag_len);
				if (err) return err;
				input += ctx->tag_len;
			}
			else {
				SERIES_TAIL(bin) += ctx->tag_len;
			}
		}
		else {
			err = mbedtls_ccm_finish(ccm, NULL, 0);
			if (err) return err;
		}
		ctx->state = CRYPT_PORT_HAS_DATA;
		
		break;
	}
#endif

#ifdef MBEDTLS_GCM_C
	case SYM_AES_128_GCM:
	case SYM_AES_192_GCM:
	case SYM_AES_256_GCM:
#ifdef MBEDTLS_CAMELLIA_C
	case SYM_CAMELLIA_128_GCM:
	case SYM_CAMELLIA_192_GCM:
	case SYM_CAMELLIA_256_GCM:
#endif
#ifdef MBEDTLS_ARIA_C
	case SYM_ARIA_128_GCM:
	case SYM_ARIA_192_GCM:
	case SYM_ARIA_256_GCM:
#endif
	{
		size_t out_bytes = 0;

		if (ctx->state == CRYPT_PORT_NO_DATA && ctx->aad_len) {
			if (len < ctx->aad_len) return 1;
			err = mbedtls_gcm_update_ad((mbedtls_gcm_context *)ctx->cipher_ctx, input, ctx->aad_len);
			if (err) return err;
			input += ctx->aad_len;
			len -= ctx->aad_len;
			if (len == 0) {
				ctx->state = CRYPT_PORT_HAS_DATA;
				break;
			}
		}
		err = mbedtls_gcm_update((mbedtls_gcm_context *)ctx->cipher_ctx, input, len, BIN_TAIL(bin), len, &out_bytes);
		if (err) return err;
		SERIES_TAIL(bin) += (REBLEN)out_bytes;
		ctx->state = CRYPT_PORT_HAS_DATA;
		input += out_bytes;
		break;
	}
#endif

#ifdef MBEDTLS_CAMELLIA_C
	case SYM_CAMELLIA_128_ECB:
	case SYM_CAMELLIA_192_ECB:
	case SYM_CAMELLIA_256_ECB:
		for (ofs = 0; ofs <= len - blk; ofs += blk) {
			err = mbedtls_camellia_crypt_ecb((mbedtls_camellia_context *)ctx->cipher_ctx, ctx->operation, input, BIN_TAIL(bin));
			if (err) return err;
			SERIES_TAIL(bin) += blk;
			ctx->state = CRYPT_PORT_HAS_DATA;
			input += blk;
		}
		break;
#ifdef MBEDTLS_CIPHER_MODE_CBC
	case SYM_CAMELLIA_128_CBC:
	case SYM_CAMELLIA_192_CBC:
	case SYM_CAMELLIA_256_CBC:
		blk = len - (len % blk);
		err = mbedtls_camellia_crypt_cbc((mbedtls_camellia_context *)ctx->cipher_ctx, ctx->operation, blk, ctx->nonce, input, BIN_TAIL(bin));
		if (err) return err;
		SERIES_TAIL(bin) += blk;
		ctx->state = CRYPT_PORT_HAS_DATA;
		input += blk;
		break;
#endif
#endif


#ifdef MBEDTLS_ARIA_C
	case SYM_ARIA_128_ECB:
	case SYM_ARIA_192_ECB:
	case SYM_ARIA_256_ECB:
		for (ofs = 0; ofs <= len - blk; ofs += blk) {
			err = mbedtls_aria_crypt_ecb((mbedtls_aria_context *)ctx->cipher_ctx, input, BIN_TAIL(bin));
			if (err) return err;
			SERIES_TAIL(bin) += blk;
			ctx->state = CRYPT_PORT_HAS_DATA;
			input += blk;
		}
		break;
#ifdef MBEDTLS_CIPHER_MODE_CBC
	case SYM_ARIA_128_CBC:
	case SYM_ARIA_192_CBC:
	case SYM_ARIA_256_CBC:
		blk = len - (len % blk);
		err = mbedtls_aria_crypt_cbc((mbedtls_aria_context *)ctx->cipher_ctx, ctx->operation, blk, ctx->nonce, input, BIN_TAIL(bin));
		if (err) return err;
		SERIES_TAIL(bin) += blk;
		ctx->state = CRYPT_PORT_HAS_DATA;
		input += blk;
		break;
#endif
#endif


#ifdef MBEDTLS_CHACHA20_C
	case SYM_CHACHA20:
		err = mbedtls_chacha20_update((mbedtls_chacha20_context *)ctx->cipher_ctx, len, input, BIN_TAIL(bin));
		if (err) return err;
		SERIES_TAIL(bin) += len;
		ctx->state = CRYPT_PORT_HAS_DATA;
		input += len;
		break;
#endif

#ifdef MBEDTLS_CHACHAPOLY_C
	case SYM_CHACHA20_POLY1305:
		if (ctx->state == CRYPT_PORT_NO_DATA) {
			size_t i;
			size_t dynamic_iv_len = len < 8 ? len : 8;
			unsigned char *dst_iv;
			dst_iv = ctx->nonce;
			memset(dst_iv, 0, 12);
			memcpy(dst_iv, ctx->IV, 12);
			dst_iv += 12 - dynamic_iv_len;
			for (i = 0; i < dynamic_iv_len; i++)
				dst_iv[i] ^= input[i];

			// https://github.com/ARMmbed/mbedtls/issues/5474
			mbedtls_chachapoly_mode_t mode = ctx->operation == MBEDTLS_ENCRYPT ? MBEDTLS_CHACHAPOLY_ENCRYPT : MBEDTLS_CHACHAPOLY_DECRYPT;
			
			err = mbedtls_chachapoly_starts((CHACHAPOLY_CTX *)ctx->cipher_ctx, ctx->nonce, mode);
			if (err) return err;

			err = mbedtls_chachapoly_update_aad((mbedtls_chachapoly_context *)ctx->cipher_ctx, input, len);
			if (err) return err;
			*olen = len;
			ctx->state = CRYPT_PORT_READY;
			// exit, because aad is not part of the output
			return CRYPT_OK;
		}
		else {
			err = mbedtls_chachapoly_update((CHACHAPOLY_CTX *)ctx->cipher_ctx, len, input, BIN_TAIL(bin));
		}
		if (err) return err;
		SERIES_TAIL(bin) += len;
		ctx->state = CRYPT_PORT_HAS_DATA;
		input += len;
		break;
#endif
	}

	*olen = input - start;
	return CRYPT_OK;
}


/***********************************************************************
**
*/	static REBINT Crypt_Init(CRYPT_CTX *ctx)
/*
***********************************************************************/
{
	REBINT  ret = CRYPT_OK;
	REBCNT  counter;

	CLEAR_SERIES(ctx->buffer);
	SERIES_TAIL(ctx->buffer) = 0;
	CLEAR(ctx->unprocessed_data, MBEDTLS_MAX_BLOCK_LENGTH);
	ctx->unprocessed_len = 0;
	ctx->state = CRYPT_PORT_NO_DATA;
	ctx->error = 0;

	switch (ctx->cipher_type) {

	case SYM_AES_128_ECB:
	case SYM_AES_192_ECB:
	case SYM_AES_256_ECB:
	case SYM_AES_128_CBC:
	case SYM_AES_192_CBC:
	case SYM_AES_256_CBC:
		if (ctx->operation == MBEDTLS_ENCRYPT) {
			ret = mbedtls_aes_setkey_enc((mbedtls_aes_context *)ctx->cipher_ctx, ctx->key, ctx->key_bitlen);
		}
		else {
			ret = mbedtls_aes_setkey_dec((mbedtls_aes_context *)ctx->cipher_ctx, ctx->key, ctx->key_bitlen);
		}
		break;

	#ifdef MBEDTLS_CCM_C
	case SYM_AES_128_CCM:
	case SYM_AES_192_CCM:
	case SYM_AES_256_CCM:
	#ifdef MBEDTLS_CAMELLIA_C
	case SYM_CAMELLIA_128_CCM:
	case SYM_CAMELLIA_192_CCM:
	case SYM_CAMELLIA_256_CCM:
	#endif
	#ifdef MBEDTLS_ARIA_C
	case SYM_ARIA_128_CCM:
	case SYM_ARIA_192_CCM:
	case SYM_ARIA_256_CCM:
	#endif
	{
		mbedtls_ccm_context* ccm = (mbedtls_ccm_context*)ctx->cipher_ctx;
		ret = mbedtls_ccm_setkey(ccm, get_cipher_id(ctx->cipher_type), ctx->key, ctx->key_bitlen);
		if (ret) return ret;
		ctx->IV_len = MAX(7, MIN(13, ctx->IV_len));
		ret = mbedtls_ccm_starts(ccm, ctx->operation, ctx->IV, ctx->IV_len);
		if (!ctx->tag_len && !ctx->aad_len) {
			// if there was not defined size of tag and aad, than we must  use STAR variant,
			// which ignores them correctly
			ccm->private_mode = ctx->operation == MBEDTLS_ENCRYPT ? MBEDTLS_CCM_STAR_ENCRYPT : MBEDTLS_CCM_STAR_DECRYPT;
		}
		ctx->unprocessed_len = 0;
		break;
	}
	#endif

	#ifdef MBEDTLS_GCM_C
	case SYM_AES_128_GCM:
	case SYM_AES_192_GCM:
	case SYM_AES_256_GCM:
	#ifdef MBEDTLS_CAMELLIA_C
	case SYM_CAMELLIA_128_GCM:
	case SYM_CAMELLIA_192_GCM:
	case SYM_CAMELLIA_256_GCM:
	#endif
	#ifdef MBEDTLS_ARIA_C
	case SYM_ARIA_128_GCM:
	case SYM_ARIA_192_GCM:
	case SYM_ARIA_256_GCM:
	#endif
	{
		mbedtls_gcm_context *gcm = (mbedtls_gcm_context *)ctx->cipher_ctx;
		ret = mbedtls_gcm_setkey(gcm, get_cipher_id(ctx->cipher_type), ctx->key, ctx->key_bitlen);
		if (ret) return ret;
		ret = mbedtls_gcm_starts(gcm, ctx->operation, ctx->IV, ctx->IV_len);
		ctx->unprocessed_len = 0;
		break;
	}
	#endif

	#ifdef MBEDTLS_CAMELLIA_C
	case SYM_CAMELLIA_128_ECB:
	case SYM_CAMELLIA_192_ECB:
	case SYM_CAMELLIA_256_ECB:
	case SYM_CAMELLIA_128_CBC:
	case SYM_CAMELLIA_192_CBC:
	case SYM_CAMELLIA_256_CBC:
		mbedtls_camellia_init((mbedtls_camellia_context *)ctx->cipher_ctx);
		if (ctx->operation == MBEDTLS_ENCRYPT) {
			ret = mbedtls_camellia_setkey_enc((mbedtls_camellia_context *)ctx->cipher_ctx, ctx->key, ctx->key_bitlen);
		}
		else {
			ret = mbedtls_camellia_setkey_dec((mbedtls_camellia_context *)ctx->cipher_ctx, ctx->key, ctx->key_bitlen);
		}
		COPY_MEM(ctx->nonce, ctx->IV, MBEDTLS_MAX_IV_LENGTH);
		break;
	#endif

	#ifdef MBEDTLS_ARIA_C
	case SYM_ARIA_128_ECB:
	case SYM_ARIA_192_ECB:
	case SYM_ARIA_256_ECB:
	case SYM_ARIA_128_CBC:
	case SYM_ARIA_192_CBC:
	case SYM_ARIA_256_CBC:
		mbedtls_aria_init((mbedtls_aria_context *)ctx->cipher_ctx);
		if (ctx->operation == MBEDTLS_ENCRYPT) {
			ret = mbedtls_aria_setkey_enc((mbedtls_aria_context *)ctx->cipher_ctx, ctx->key, ctx->key_bitlen);
		}
		else {
			ret = mbedtls_aria_setkey_dec((mbedtls_aria_context *)ctx->cipher_ctx, ctx->key, ctx->key_bitlen);
		}
		COPY_MEM(ctx->nonce, ctx->IV, MBEDTLS_MAX_IV_LENGTH);
		break;
#endif

	#ifdef MBEDTLS_CHACHA20_C
	case SYM_CHACHA20:
		ret = mbedtls_chacha20_setkey((mbedtls_chacha20_context *)ctx->cipher_ctx, ctx->key);
		if (ret) return ret;
		counter = MBEDTLS_GET_UINT32_BE(ctx->IV, 12);
		ret = mbedtls_chacha20_starts((mbedtls_chacha20_context *)ctx->cipher_ctx, ctx->IV, counter);
		break;
	#endif


	#ifdef MBEDTLS_CHACHAPOLY_C
	case SYM_CHACHA20_POLY1305:
		ret = mbedtls_chachapoly_setkey((CHACHAPOLY_CTX *)ctx->cipher_ctx, ctx->key);
		break;
	#endif

	}
	return ret;
}

/***********************************************************************
**
*/	static REBINT Crypt_Update(CRYPT_CTX* ctx)
/*
***********************************************************************/
{
	REBSER* bin;
	REBCNT  len, ofs, blk;
	REBINT  ret = CRYPT_OK;
	size_t  olen = 0;

	bin = ctx->buffer;

#ifdef MBEDTLS_CHACHAPOLY_C
	if (ctx->cipher_type == SYM_CHACHA20_POLY1305) {
		Extend_Series(bin, 16);
		ret = mbedtls_chachapoly_finish((mbedtls_chachapoly_context*)ctx->cipher_ctx, BIN_TAIL(bin));
		SERIES_TAIL(bin) += 16;
		ctx->state = CRYPT_PORT_FINISHED;
		return ret;
	}
#endif

#ifdef MBEDTLS_CCM_C
	if (ctx->cipher_mode == MBEDTLS_MODE_CCM) {
		// the tag is computed immediatelly, so no need to finish CCM
		return CRYPT_OK;
	}
#endif

	if (ctx->unprocessed_len > 0) {
		blk = ctx->cipher_block_size;
		Extend_Series(bin, blk);

		if (ctx->unprocessed_len > blk) abort();
		len = blk - ctx->unprocessed_len;
		// pad with zeros...
		CLEAR(ctx->unprocessed_data + ctx->unprocessed_len, len);

		ret = Crypt_Crypt(ctx, ctx->unprocessed_data, blk, &ofs);
		ctx->unprocessed_len = 0;
	}
	if (ctx->tag_len) {
		#ifdef MBEDTLS_GCM_C
		if (ctx->cipher_mode == MBEDTLS_MODE_GCM) {
			// compute tag
			Extend_Series(bin, ctx->tag_len);
			ret = mbedtls_gcm_finish((mbedtls_gcm_context*)ctx->cipher_ctx, NULL, 0, &olen, BIN_TAIL(bin), ctx->tag_len);
			if (ret) return ret;
			SERIES_TAIL(bin) += ctx->tag_len; // don't use olen, because it is not set!!!
			ctx->state = CRYPT_PORT_FINISHED;
		}
		#endif
	}

	return ret;
}

/***********************************************************************
**
*/	static REBINT Crypt_Write(CRYPT_CTX *ctx, REBYTE *input, REBCNT len)
/*
***********************************************************************/
{
	REBINT ret = CRYPT_OK;
	REBCNT blk, olen;
	REBCNT unprocessed_free;

	if (ctx->state == CRYPT_PORT_NEEDS_INIT) {
		ret = Crypt_Init(ctx);
		if (ret) return ret;
	}

	if (len == 0) {
		// it is valid to encrypt empty message
		if (ctx->cipher_mode == MBEDTLS_MODE_GCM) {
			ret = Crypt_Crypt(ctx, input, 0, &olen);
		}
		return ret;
	}

	blk = ctx->cipher_block_size;
	if (blk > MBEDTLS_MAX_BLOCK_LENGTH) return CRYPT_ERROR_BAD_BLOCK_SIZE;


	unprocessed_free = blk - ctx->unprocessed_len;
	if (len < unprocessed_free) {
		// input has not enough bytes to fill the block!
		COPY_MEM(ctx->unprocessed_data + ctx->unprocessed_len, input, len);
		ctx->unprocessed_len += len;
		if (ctx->unprocessed_len < blk)	return CRYPT_OK;
	}

	if (ctx->unprocessed_len > 0) {
		if (ctx->unprocessed_len > blk)
			return CRYPT_ERROR_BAD_UNPROCESSED_SIZE;
		// complete the block using the current input
		COPY_MEM(ctx->unprocessed_data + ctx->unprocessed_len, input, unprocessed_free);
		// complete the block
		ret = Crypt_Crypt(ctx, ctx->unprocessed_data, blk, &olen);
		input += unprocessed_free;
		len -= unprocessed_free;
		// make the processed block empty again
		ctx->unprocessed_len = 0;
	}
	// input data could be already consumed on the unprocessed buffer
	if (len == 0) {
		return ret;
	}
	// test if input have enough data to do the block crypt
	if (len >= blk) {
		// we have enough data to call crypt
		ret = Crypt_Crypt(ctx, input, len, &olen);
		if (ret) return ret;
		if (olen > len) return CRYPT_ERROR_BAD_PROCESSED_SIZE;
		input += olen;
		len -= olen;
	}
	// test if there are some unprocessed data
	if (len > 0) {
		if (len > MBEDTLS_MAX_BLOCK_LENGTH) return CRYPT_ERROR_BAD_UNPROCESSED_SIZE;
		COPY_MEM(ctx->unprocessed_data, input, len);
		ctx->unprocessed_len = len;
	}
	// done
	return ret;
}


/***********************************************************************
**
*/	static int Crypt_Actor(REBVAL *ds, REBVAL *port_value, REBCNT action)
/*
***********************************************************************/
{
	REBSER *port;
	REBVAL *state;
	REBVAL *arg1;
	REBVAL *arg2;
	REBSER *bin;
	REBSER *out;
	REBCNT  len;
	CRYPT_CTX *ctx = NULL;

	port = Validate_Port_Value(port_value);
	
	state = BLK_SKIP(port, STD_PORT_STATE);
	if (IS_HANDLE(state)) {
		if (VAL_HANDLE_TYPE(state) != SYM_CRYPT)
			Trap_Port(RE_INVALID_PORT, port, 0);
		ctx = (CRYPT_CTX *)VAL_HANDLE_CONTEXT_DATA(state);
	}

	if (action == A_OPEN) {
		if (ctx) {
			Trap_Port(RE_ALREADY_OPEN, port, 0);
		}
		if (!Crypt_Open(port)) {
			Trap_Port(RE_CANNOT_OPEN, port, 0);
			return R_FALSE;
		}
		return R_ARG1;
	}
	if (!ctx) {
		Trap_Port(RE_NOT_OPEN, port, 0);
		return R_NONE;
	}

	bin = ctx->buffer;

	switch (action) {
	case A_WRITE:
		arg1 = D_ARG(2);
		if (!IS_BINARY(arg1)) {
			Trap_Port(RE_FEATURE_NA, port, 0);
		}
		ctx->error = Crypt_Write(ctx, VAL_BIN_AT(arg1), VAL_LEN(arg1));
		break;

	case A_TAKE:
		// `take` is same like `read`, but also calls `update` to process also
		//  yet not processed data from the unprocessed buffer
		ctx->error = Crypt_Update(ctx);
	case A_READ:
		if (ctx->state == CRYPT_PORT_FINISHED) {
			// result of chacha20-poly1305 is in two steps...
			// TODO: make it in one as with the CCM mode?
			ctx->state = CRYPT_PORT_HAS_DATA;
		}
		if (ctx->state != CRYPT_PORT_HAS_DATA || ctx->error) {
			return R_NONE;
		}
		ctx->state = CRYPT_PORT_NO_DATA;

		len = BIN_LEN(bin);
		out = Make_Binary(len);
		if (len > 0) {
			COPY_MEM(BIN_DATA(out), BIN_DATA(bin), len);
		}
		SET_BINARY(D_RET, out);
		BIN_LEN(out) = len;
		BIN_LEN(bin) = 0;
		return R_RET;

	case A_UPDATE:
		ctx->error = Crypt_Update(ctx);
		return R_ARG1;

	case A_CLOSE:
		if (ctx) {
			UNPROTECT_SERIES(ctx->buffer);
			Free_Hob(VAL_HANDLE_CTX(state));
			SET_NONE(state);
			ctx = NULL;
		}
		break;

	case A_OPENQ:
		return (ctx) ? R_TRUE : R_FALSE;

	case A_MODIFY:
		arg1 = D_ARG(2); // field
		arg2 = D_ARG(3); // value
		if (!IS_WORD(arg1)) break;
		switch (VAL_WORD_CANON(arg1)) {
		case SYM_ALGORITHM:
			if (!init_crypt_algorithm(ctx, arg2)) return R_FALSE;
			break;
		case SYM_DIRECTION:
			if (!init_crypt_direction(ctx, arg2)) return R_FALSE;
			break;
		case SYM_KEY:
			if (!init_crypt_key(ctx, arg2)) return R_FALSE;
			break;
		case SYM_IV:
		case SYM_INIT_VECTOR:
			if (!init_crypt_iv(ctx, arg2)) return R_FALSE;
			break;
		case SYM_TAG_LENGTH:
			if (!ctx) return R_FALSE;
			ctx->tag_len = VAL_UNT32(arg2);
			break;
		case SYM_AAD_LENGTH:
			if (!ctx) return R_FALSE;
			ctx->aad_len = VAL_UNT32(arg2);
			break;
		default:
			Trap1(RE_INVALID_ARG, arg1);
		}
		break;
	default:
		Trap1(RE_NO_PORT_ACTION, Get_Action_Word(action));
	}
	return R_ARG1;
}


/***********************************************************************
**
*/	void Init_Crypt_Scheme(void)
/*
***********************************************************************/
{
	Register_Scheme(SYM_CRYPT, 0, Crypt_Actor);
}

/* DEFINE_DEV would normally be in os/dev-crypt.c but I'm not using it so it is here */
DEFINE_DEV(Dev_Crypt, "Crypt", 1, NULL, RDC_MAX, 0);

#endif //INCLUDE_MBEDTLS