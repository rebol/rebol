/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2022 Rebol Open Source Contributors
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


void crypt_context_free(CRYPT_CTX *ctx) {
	if (ctx == NULL) return;
	if (ctx->buffer) {
		CLEAR(ctx->buffer->data, ctx->buffer->rest);
		Free_Series(ctx->buffer);
	}
	mbedtls_cipher_free(&ctx->cipher);
	CLEARS(ctx);
}


static REBOOL init_crypt_key(CRYPT_CTX *ctx, REBVAL *val) {
	REBSER *ser;
	REBCNT  len = 0;
	REBYTE *bin = NULL;
	if (val == NULL) return FALSE;
	if (IS_NONE(val)) {
		memset(&ctx->key, 0, 512);
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
		if (len > 512) len = 512;
		memset(&ctx->key, 0, 512);
		memcpy(&ctx->key, bin, len);
		return TRUE;
	}
	return FALSE;
}

static REBOOL init_crypt_iv(CRYPT_CTX *ctx, REBVAL *val) {
	REBCNT len;
	if (val == NULL) return FALSE;
	if (IS_NONE(val)) {
		memset(&ctx->IV, 0, 16);
		return TRUE;
	}
	if (IS_BINARY(val)) {
		len = VAL_LEN(val);
		if (len > 0) {
			if (len > 16) len = 16;
			memset(&ctx->IV, 0, 16);
			memcpy(&ctx->IV, VAL_BIN_AT(val), len);
		}
		return TRUE;
	}
	return FALSE;
}

static REBOOL init_crypt_direction(CRYPT_CTX *ctx, REBVAL *val) {
	if (!val || !IS_WORD(val)) return FALSE;
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

static REBOOL init_crypt_algorithm(CRYPT_CTX *ctx, REBVAL *val) {
	const mbedtls_cipher_info_t *cipher_info;

	if (val == NULL || !IS_WORD(val)) return FALSE;
	cipher_info = mbedtls_cipher_info_from_string(VAL_WORD_NAME(val));
	if (!cipher_info) return FALSE;
	ctx->key_bitlen = cipher_info->private_key_bitlen;
	return (0 == mbedtls_cipher_setup(&ctx->cipher, cipher_info));
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
	REBVAL *val_key;
	REBYTE *name;
	REBINT  err = 0;
	REBCNT  len;
	REBINT  i;
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

	mbedtls_cipher_init(&ctx->cipher);

	val = Obj_Value(spec, STD_PORT_SPEC_CRYPT_ALGORITHM);
	if (!init_crypt_algorithm(ctx, val)) goto failed;

	val = Obj_Value(spec, STD_PORT_SPEC_CRYPT_IV);
	if (!init_crypt_iv(ctx, val)) { err = 1; goto failed; }
	// as we have a copy, make it invisible from the spec
	SET_NONE(val);

	val = Obj_Value(spec, STD_PORT_SPEC_CRYPT_KEY);
	if(!init_crypt_key(ctx, val)) { err = 1; goto failed; }
	// as we have a copy, make it invisible from the spec
	SET_NONE(val);

	val = Obj_Value(spec, STD_PORT_SPEC_CRYPT_DIRECTION);
	if (!init_crypt_direction(ctx, val)) { err = 1; goto failed; }

	err = mbedtls_cipher_setkey(&ctx->cipher, ctx->key, ctx->key_bitlen, ctx->operation);
	if (err) goto failed;

	err = mbedtls_cipher_set_iv(&ctx->cipher, ctx->IV, 16);
	if (err) goto failed;

	err = mbedtls_cipher_reset(&ctx->cipher);
	if (err) goto failed;

	ctx->buffer = Make_Binary(256);
	// buffer is extended when needed.
	// protected using KEEP, because it is not accesible from any real Rebol value!
	KEEP_SERIES(ctx->buffer, "crypt");

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
*/	static int Crypt_Actor(REBVAL *ds, REBSER *port, REBCNT action)
/*
***********************************************************************/
{
	REBVAL *spec;
	REBVAL *state;
	REBVAL *data;
	REBVAL *arg1;
	REBVAL *arg2;
	REBSER *bin;
	REBSER *out;
	REBCNT  len;
	REBCNT  ofs, blk;
	REBINT  err;
	size_t olen = 0;
	CRYPT_CTX *ctx = NULL;

	Validate_Port(port, action);

	//printf("Crypt device action: %i\n", action);
	
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
		len = VAL_LEN(arg1);
		if (len > 0) {
			// make space at tail if needed...
			olen = SERIES_TAIL(bin);
			Expand_Series(bin, AT_TAIL, len);
			// reset the tail (above expand modifies it!)
			SERIES_TAIL(bin) = olen;
			ofs = 0;
			blk = mbedtls_cipher_get_block_size(&ctx->cipher);
			if (blk == 1) blk = len; // MBEDTLS_MODE_STREAM, so we can process all data at once
			REBYTE *p = VAL_BIN_AT(arg1);
			if (ctx->unprocessed_len > 0) {
				if (ctx->unprocessed_len > blk) abort();
				REBCNT n = blk - ctx->unprocessed_len;
				memcpy(ctx->unprocessed_data + ctx->unprocessed_len, p, n);
				err = mbedtls_cipher_update(&ctx->cipher, ctx->unprocessed_data, blk, BIN_TAIL(bin), &olen);
				SERIES_TAIL(bin) += olen;
				p += n;
				len -= n;
				ctx->unprocessed_len = 0;
				
			}
			if (len >= blk) {
				for (ofs = 0; ofs <= len - blk; ofs += blk) {
					err = mbedtls_cipher_update(&ctx->cipher, p, blk, BIN_TAIL(bin), &olen);
					if (err) {
						Trap1(RE_INVALID_ARG, arg1);
					}
					SERIES_TAIL(bin) += olen;
					p += blk;
				}
			}
			if (ofs < len) {
				// unprocessed data...
				len -= ofs;
				memcpy(ctx->unprocessed_data, p, len);
				ctx->unprocessed_len = len;
			}
		}
		break;
	case A_READ:
	case A_TAKE:
		len = BIN_LEN(bin);
		if (len > 0) {
			out = Make_Binary(len);
			COPY_MEM(BIN_DATA(out), BIN_DATA(bin), len);
			SET_BINARY(D_RET, out);
			BIN_LEN(out) = len;
			BIN_LEN(bin) = 0;
			return R_RET;
		}
		else return R_NONE;
		break;
	case A_UPDATE:
		if (ctx->unprocessed_len > 0) {
			ofs = 0;
			blk = mbedtls_cipher_get_block_size(&ctx->cipher);
			olen = SERIES_TAIL(bin);
			Expand_Series(bin, AT_TAIL, blk);
			// reset the tail (above expand modifies it!)
			SERIES_TAIL(bin) = olen;

			if (ctx->unprocessed_len > blk) abort();
			len = blk - ctx->unprocessed_len;
			// pad with zeros...
			memset(ctx->unprocessed_data + ctx->unprocessed_len, 0, len);
			err = mbedtls_cipher_update(&ctx->cipher, ctx->unprocessed_data, blk, BIN_TAIL(bin), &olen);
			SERIES_TAIL(bin) += olen;
			ctx->unprocessed_len = 0;
		}
		//TODO... do we really want to finish on update?!
		mbedtls_cipher_finish(&ctx->cipher, BIN_TAIL(bin), &olen);
		SERIES_TAIL(bin) += olen;
		break;

	case A_OPEN:
		if (ctx) {
			Trap_Port(RE_ALREADY_OPEN, port, 0);
		}
		if (!Crypt_Open(port)) {
			Trap_Port(RE_CANNOT_OPEN, port, 0);
			return R_FALSE;
		}
		break;

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
			if (!init_crypt_algorithm(ctx, arg2)) return FALSE;
			break;
		case SYM_DIRECTION:
			if (!init_crypt_direction(ctx, arg2)) return R_FALSE;
			break;
		case SYM_KEY:
			if (!init_crypt_key(ctx, arg2)) return R_FALSE;
			break;
		case SYM_IV:
			if (!init_crypt_iv(ctx, arg2)) return R_FALSE;
			break;
		default:
			Trap1(RE_INVALID_ARG, arg1);
		}
		err = mbedtls_cipher_setkey(&ctx->cipher, ctx->key, ctx->key_bitlen, ctx->operation);
		if (err) return R_FALSE;
		err = mbedtls_cipher_set_iv(&ctx->cipher, ctx->IV, 16);
		if (err) return R_FALSE;
		err = mbedtls_cipher_reset(&ctx->cipher);
		break;
	default:
		puts("not supported command");
		Trap_Action(REB_PORT, action);
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