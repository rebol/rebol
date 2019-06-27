/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2017 Rebol Open Source Contributors
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
**  Module:  n-crypt.c
**  Summary: native functions for data sets
**  Section: natives
**  Author:  Oldes, Cyphre
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "sys-rc4.h"
#include "sys-aes.h"
#include "sys-rsa.h"
#include "sys-dh.h"
#include "sys-chacha20.h"

/***********************************************************************
**
*/	REBNATIVE(rc4)
/*
//	rc4: native [
//		"Encrypt/decrypt data (modifies) using RC4 algorithm."
//
//		/key "Provided only for the first time to get stream HANDLE!"
//			crypt-key [binary!]  "Crypt key."
//		/stream
//			ctx  [handle!] "Stream cipher context."
//			data [binary!] "Data to encrypt/decrypt."
//	]
***********************************************************************/
{
    REBOOL  ref_key       = D_REF(1);
    REBVAL *val_crypt_key = D_ARG(2); 
    REBOOL  ref_stream    = D_REF(3);
    REBVAL *val_ctx       = D_ARG(4);
    REBVAL *val_data      = D_ARG(5);

    REBVAL *ret = D_RET;
	REBSER *ctx;

    if(ref_stream) {
    	ctx = VAL_HANDLE_DATA(val_ctx);

    	if (VAL_HANDLE_TYPE(val_ctx) != SYM_RC4) {
    		Trap0(RE_INVALID_HANDLE);
    	}

    	REBYTE *data = VAL_BIN_AT(val_data);
    	RC4_crypt((RC4_CTX*)ctx->data, data, data, VAL_LEN(val_data));
    	DS_RET_VALUE(val_data);

    } else if (ref_key) {
    	//key defined - setup new context
		//making series from POOL so it will be GCed automaticaly
		REBSER* ctx = Make_Series(sizeof(RC4_CTX), 1, FALSE);

		RC4_setup(
			(RC4_CTX*)ctx->data,
            VAL_BIN_AT(val_crypt_key),
            VAL_LEN(val_crypt_key)
        );

        SET_HANDLE(ret, ctx, SYM_RC4, HANDLE_SERIES);
    }
    return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(aes)
/*
//  aes: native [
//		"Encrypt/decrypt data using AES algorithm. Returns stream cipher context handle or encrypted/decrypted data."
//		/key                "Provided only for the first time to get stream HANDLE!"
//			crypt-key [binary!] "Crypt key (16 or 32 bytes)."
//			iv  [none! binary!] "Optional initialization vector (16 bytes)."
//		/decrypt            "Use the crypt-key for decryption (default is to encrypt)"
//		/stream
//			ctx [handle!]   "Stream cipher context."
//			data [binary!]  "Data to encrypt/decrypt."
//  ]
***********************************************************************/
{
	REBOOL  ref_key       = D_REF(1);
    REBVAL *val_crypt_key = D_ARG(2);
    REBVAL *val_iv        = D_ARG(3);
	REBOOL  ref_decrypt   = D_REF(4);
    REBOOL  ref_stream    = D_REF(5);
    REBVAL *val_ctx       = D_ARG(6);
    REBVAL *val_data      = D_ARG(7);
    
    REBVAL *ret = D_RET;
	REBSER *ctx;
	REBINT  len, pad_len;

	//TODO: could be optimized by reusing the handle

	if (ref_key) {
    	//key defined - setup new context

    	uint8_t iv[AES_IV_SIZE];

		if (IS_BINARY(val_iv)) {
			if (VAL_LEN(val_iv) < AES_IV_SIZE) {
				return R_NONE;
			}
			memcpy(iv, VAL_BIN_AT(val_iv), AES_IV_SIZE);
		} else {
			//TODO: Use ECB encryption if IV is not specified
			memset(iv, 0, AES_IV_SIZE);
		}
		
		len = VAL_LEN(val_crypt_key) << 3;

		if (len != 128 && len != 256) {
			return R_NONE;
		}

		//making series from POOL so it will be GCed automaticaly
		ctx = Make_Series(sizeof(AES_CTX), (REBCNT)1, FALSE);
		SERIES_TAIL(ctx) = sizeof(AES_CTX);

		AES_set_key(
			(AES_CTX*)ctx->data,
			VAL_BIN_AT(val_crypt_key),
			(const uint8_t *)iv,
			(len == 128) ? AES_MODE_128 : AES_MODE_256
		);

		if (ref_decrypt) AES_convert_key((AES_CTX*)ctx->data);

		SET_HANDLE(ret, ctx, SYM_AES, HANDLE_SERIES);
		// the ctx in the handle is released by GC once the handle is not referenced

    } else if(ref_stream) {

		ctx = VAL_HANDLE_DATA(val_ctx);

    	if (VAL_HANDLE_TYPE(val_ctx) != SYM_AES || ctx == NULL || SERIES_TAIL(ctx) != sizeof(AES_CTX)){
    		Trap0(RE_INVALID_HANDLE);
    	}

    	len = VAL_LEN(val_data);
    	if (len == 0) return R_NONE;

		pad_len = (((len - 1) >> 4) << 4) + AES_BLOCKSIZE;

		REBYTE *data = VAL_BIN_AT(val_data);
		REBYTE *pad_data;

		if (len < pad_len) {
			//  make new data input with zero-padding
			//TODO: instead of making new data, the original could be extended with padding.
			pad_data = (REBYTE*)MAKE_MEM(pad_len);
			memset(pad_data, 0, pad_len);
			memcpy(pad_data, data, len);
			data = pad_data;
		}
		else {
			pad_data = NULL;
		}

		REBSER  *binaryOut = Make_Binary(pad_len);
		AES_CTX *aes_ctx = (AES_CTX *)ctx->data;
		if (aes_ctx->key_mode == AES_MODE_DECRYPT) {
			AES_cbc_decrypt(aes_ctx, data, BIN_HEAD(binaryOut),	pad_len);
		}
		else {
			AES_cbc_encrypt(aes_ctx, data, BIN_HEAD(binaryOut),	pad_len);
		}
		if (pad_data) FREE_MEM(pad_data);

		SET_BINARY(ret, binaryOut);
		VAL_TAIL(ret) = pad_len;

    }
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(rsa_init)
/*
//  rsa-init: native [
//		"Creates a context which is than used to encrypt or decrypt data using RSA"
//		n  [binary!]  "Modulus"
//		e  [binary!]  "Public exponent"
//		/private "Init also private values"
//			d [binary!] "Private exponent"
//			p [binary!] "Prime number 1"
//			q [binary!] "Prime number 2"
//			dP [binary!]
//			dQ [binary!]
//			qInv [binary!]
//  ]
***********************************************************************/
{
	REBSER *n       = VAL_SERIES(D_ARG(1));
	REBSER *e       = VAL_SERIES(D_ARG(2));
	REBOOL  ref_private =        D_REF(3);
	REBSER *d       = VAL_SERIES(D_ARG(4));
	REBSER *p       = VAL_SERIES(D_ARG(5));
	REBSER *q       = VAL_SERIES(D_ARG(6));
	REBSER *dP      = VAL_SERIES(D_ARG(7));
	REBSER *dQ      = VAL_SERIES(D_ARG(8));
	REBSER *qInv    = VAL_SERIES(D_ARG(9));

	REBVAL *ret = D_RET;
	REBSER *ser = Make_Series(sizeof(RSA_CTX), 1, FALSE);
	SERIES_TAIL(ser) = sizeof(RSA_CTX);
	
	RSA_CTX *rsa_ctx = (RSA_CTX*)SERIES_DATA(ser);
	CLEARS(rsa_ctx);

	SET_HANDLE(ret, ser, SYM_RSA, HANDLE_SERIES);

	if(ref_private) {
		RSA_priv_key_new(
			rsa_ctx,
			BIN_DATA(n), BIN_LEN(n),
			BIN_DATA(e), BIN_LEN(e),
			BIN_DATA(d), BIN_LEN(d),
			BIN_DATA(p), BIN_LEN(p),
			BIN_DATA(q), BIN_LEN(q),
			BIN_DATA(dP), BIN_LEN(dP),
			BIN_DATA(dQ), BIN_LEN(dQ),
			BIN_DATA(qInv), BIN_LEN(qInv)
		);
	} else {
		RSA_pub_key_new(
			rsa_ctx,
			BIN_DATA(n), BIN_LEN(n),
			BIN_DATA(e), BIN_LEN(e)
		);
	}
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(rsa)
/*
//  rsa: native [
//		"Encrypt/decrypt/sign/verify data using RSA cryptosystem. Only one refinement must be used!"
//		rsa-key [handle!] "RSA context created using `rsa-init` function"
//		data    [binary! none!] "Data to work with. Use NONE to release the RSA handle resources!"
//		/encrypt  "Use public key to encrypt data"
//		/decrypt  "Use private key to decrypt data"
//		/sign     "Use private key to sign data"
//		/verify   "Use public key to verify signed data"
//  ]
***********************************************************************/
{
	REBVAL *key         = D_ARG(1);
	REBVAL *val_data    = D_ARG(2);
	REBOOL  refEncrypt  = D_REF(3);
	REBOOL  refDecrypt  = D_REF(4);
	REBOOL  refSign     = D_REF(5);
	REBOOL  refverify   = D_REF(6);

	// make sure that only one refinement is used!
	if(
		(refEncrypt && (refDecrypt || refSign    || refverify)) ||
		(refDecrypt && (refEncrypt || refSign    || refverify)) ||
		(refSign    && (refDecrypt || refEncrypt || refverify)) ||
		(refverify  && (refDecrypt || refSign    || refEncrypt))
	) {
		Trap0(RE_BAD_REFINES);
	}

	if (VAL_HANDLE_TYPE(key) != SYM_RSA || VAL_HANDLE_DATA(key) == NULL) {
		Trap0(RE_INVALID_HANDLE);
	}

	REBSER  *rsa_ser = VAL_HANDLE_DATA(key);
	if(SERIES_TAIL(rsa_ser) != sizeof(RSA_CTX)) return R_NONE; // probably released (and so invalidated) handle
	RSA_CTX *rsa_ctx = (RSA_CTX*)SERIES_DATA(rsa_ser);

	if (IS_NONE(val_data)) {
		// release RSA key resources
		RSA_free(rsa_ctx);
		// and invalidate the handle's data (the series will be GCed once the handle will not be referenced
		CLEARS(rsa_ctx);
		SERIES_TAIL(rsa_ser) = 0;
		return R_TRUE;
	}

	REBSER *data = VAL_SERIES(val_data);
	REBVAL *ret = D_RET;
	REBINT  outBytes;
	bigint *data_bi;

	if(
		(rsa_ctx->m == NULL || rsa_ctx->e == NULL) ||
		((refDecrypt || refSign) && (
			rsa_ctx->d  == NULL ||
			rsa_ctx->p  == NULL ||
			rsa_ctx->q  == NULL ||
			rsa_ctx->dP == NULL ||
			rsa_ctx->dQ == NULL ||
			rsa_ctx->qInv == NULL
		))
	) {
		//puts("[RSA] Missing RSA key data!");
		return R_NONE;
	}

	REBYTE* inBinary = BIN_DATA(data);
	REBINT  inBytes = BIN_LEN(data);

	data_bi = bi_import(rsa_ctx->bi_ctx, inBinary, inBytes);

	//allocate new binary!
	REBSER* output = Make_Binary(rsa_ctx->num_octets);
	REBYTE* outBinary = BIN_DATA(output);

	if(refDecrypt || refverify) {
		outBytes = RSA_decrypt(rsa_ctx, inBinary, outBinary, refDecrypt, FALSE);
	} else {
		outBytes = RSA_encrypt(rsa_ctx, inBinary, inBytes, outBinary, refSign, TRUE);
	}

	bi_free(rsa_ctx->bi_ctx, data_bi);

	if(outBytes < 0) {
		Free_Series(output);
		return R_NONE;
	}

	SET_BINARY(ret, output);
	VAL_TAIL(ret) = outBytes;

	return R_RET;

}


/***********************************************************************
**
*/	REBNATIVE(dh_init)
/*
//  dh-init: native [
//		"Generates a new Diffie-Hellman private/public key pair"
//		g [binary!] "Generator"
//		p [binary!] "Field prime"
//		/into
//			dh-key [handle!] "Existing DH key handle"
//  ]
***********************************************************************/
{
	REBSER *g = VAL_SERIES(D_ARG(1));
	REBSER *p = VAL_SERIES(D_ARG(2));
	REBOOL  ref_into =     D_REF(3);
	REBVAL *val_dh   =     D_ARG(4);
	
	DH_CTX *dh;
	REBYTE *bin;
	REBVAL *ret;

	REBCNT  len_g = BIN_LEN(g);
	REBCNT  len_p = BIN_LEN(p);
	REBYTE *buffer = NULL;
	REBSER *dh_ser;

	// allocating buffers for all keys as a one blob
	REBCNT  buffer_len = BIN_LEN(g) + (5 * BIN_LEN(p));
	
	if(ref_into) {
		if(!IS_HANDLE(val_dh) || VAL_HANDLE_TYPE(val_dh) != SYM_DH) {
			//error!
			return R_NONE;
		}
		ret = val_dh;
		*D_RET = *D_ARG(4);
		dh_ser = VAL_HANDLE_DATA(val_dh);
		if (dh_ser == NULL) goto new_dh_handle;
		HANDLE_CLR_FLAG(val_dh, HANDLE_RELEASABLE);
		if(SERIES_REST(dh_ser) < (sizeof(DH_CTX) + buffer_len)) {
			//needs more space for keys
			Expand_Series(dh_ser, AT_TAIL, (sizeof(DH_CTX) + buffer_len) - SERIES_TAIL(dh_ser));
		}
	} else {
		ret = D_RET;
	new_dh_handle:
		dh_ser = Make_Series(sizeof(DH_CTX) + buffer_len, 1, FALSE);
		SET_HANDLE(ret, dh_ser, SYM_DH, HANDLE_SERIES);
	}
	dh = (DH_CTX*)SERIES_DATA(dh_ser);
	buffer = SERIES_DATA(dh_ser) + sizeof(DH_CTX);
	CLEAR(buffer, buffer_len);
	dh->len_data = buffer_len;

	bin = BIN_DATA(g); //@@ use VAL_BIN_AT instead?
	dh->len_g = len_g;
	dh->g = buffer;
	COPY_MEM(dh->g, bin, len_g);
	
	buffer += len_g;

	bin = BIN_DATA(p);
	dh->len = len_p;
	dh->p = buffer;
	COPY_MEM(dh->p, bin, len_p);
	
	buffer += len_p;
	
	dh->x  = buffer; //private key
	buffer += len_p;
	dh->gx = buffer; //public key (self)
	buffer += len_p;
	dh->gy = buffer; //public key (peer)
	buffer += len_p;
	dh->k  = buffer; //negotiated key
	
	DH_generate_key(dh);

	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(dh)
/*
//  dh: native [
//		"Diffie-Hellman key exchange"
//		dh-key [handle!] "DH key created using `dh-init` function"
//		/release "Releases internal DH key resources"
//		/public  "Returns public key as a binary"
//		/secret  "Computes secret result using peer's public key"
//			public-key [binary!] "Peer's public key"
//  ]
***********************************************************************/
{
	REBVAL *val_dh      = D_ARG(1);
	REBOOL  ref_release = D_REF(2);
	REBOOL  ref_public  = D_REF(3);
	REBOOL  ref_secret  = D_REF(4);
	REBVAL *pub_key     = D_ARG(5);

	REBVAL *ret = D_RET;
	REBSER *bin;
	REBCNT len;

	if (ref_public && ref_secret) {
		// only one can be used
		Trap0(RE_BAD_REFINES);
	}

	if (VAL_HANDLE_TYPE(val_dh) != SYM_DH || VAL_HANDLE_DATA(val_dh) == NULL) {
		Trap0(RE_INVALID_HANDLE);
	}

	REBSER *dh_ser = VAL_HANDLE_DATA(val_dh);
	DH_CTX *dh = (DH_CTX*)SERIES_DATA(dh_ser);
	
	if (dh->g == NULL) return R_NONE; //or error?

	if(ref_public) {
		bin = Make_Binary(dh->len);
		COPY_MEM(BIN_DATA(bin), dh->gx, dh->len);
		SET_BINARY(ret, bin);
		BIN_LEN(bin) = dh->len;
	}

	if(ref_secret) {
		bin = VAL_SERIES(pub_key); //@@ use VAL_BIN_AT instead?
		len = BIN_LEN(bin);
		if(len != dh->len) {
			return R_NONE; // throw an error?
		}
		COPY_MEM(dh->gy, BIN_DATA(bin), len);

		DH_compute_key(dh);

		bin = Make_Binary(len);
		COPY_MEM(BIN_DATA(bin), dh->k, len);
		SET_BINARY(ret, bin);
		BIN_LEN(bin) = len;
	}

	if(ref_release) {
	//	if(dh->data != NULL) FREE_MEM(dh->data);
		CLEARS(dh);
		HANDLE_SET_FLAG(val_dh, HANDLE_RELEASABLE);
		if(!ref_public && !ref_secret) return R_ARG1;
	}
	
	return R_RET;

}


#include "uECC.h"
const struct uECC_Curve_t* ECC_curves[5] = {0,0,0,0,0};
typedef struct {
	REBCNT  curve_type;
	uint8_t public[64];
	uint8_t private[32];
} ECC_CTX;

/***********************************************************************
**
*/	REBNATIVE(ecdh)
/*
//  ecdh: native [
//		"Elliptic-curve Diffie-Hellman key exchange"
//		key [handle! none!] "Keypair to work with, may be NONE for /init refinement"
//		/init   "Initialize ECC keypair."
//			type [word!] "One of supported curves: [secp256k1 secp256r1 secp224r1 secp192r1 secp160r1]"
//		/curve  "Returns handles curve type"
//		/public "Returns public key as a binary"
//		/secret  "Computes secret result using peer's public key"
//			public-key [binary!] "Peer's public key"
//		/release "Releases internal ECDH key resources"
//  ]
***********************************************************************/
{
	REBVAL *val_handle  = D_ARG(1);
	REBOOL  ref_init    = D_REF(2);
	REBVAL *val_curve   = D_ARG(3);
	REBOOL  ref_type    = D_REF(4);
	REBOOL  ref_public  = D_REF(5);
	REBOOL  ref_secret  = D_REF(6);
	REBVAL *val_public  = D_ARG(7);
	REBOOL  ref_release = D_REF(8);

	REBSER *ecc_ser = NULL;
	REBSER *bin = NULL;
	REBVAL *ret;
	REBCNT curve_type = 0;
	uECC_Curve curve = NULL;
	ECC_CTX *ecc = NULL;

	if (IS_HANDLE(val_handle)) {
		if (VAL_HANDLE_TYPE(val_handle) != SYM_ECDH || VAL_HANDLE_DATA(val_handle) == NULL) {
			Trap0(RE_INVALID_HANDLE);
		}
		ecc_ser = VAL_HANDLE_DATA(val_handle);
		ecc = (ECC_CTX*)SERIES_DATA(ecc_ser);
		curve_type = ecc->curve_type;
	}

	if (ref_init) {
		if(ecc_ser == NULL) {
			ecc_ser = Make_Series(sizeof(ECC_CTX), 1, FALSE);
		}
		ecc = (ECC_CTX*)SERIES_DATA(ecc_ser);	
		CLEARS(ecc);
		curve_type = ecc->curve_type = VAL_WORD_CANON(val_curve);
		SET_HANDLE(val_handle, ecc_ser, SYM_ECDH, HANDLE_SERIES);
	}

	switch (curve_type) {
		case SYM_SECP256K1:
			curve = ECC_curves[4];
			if(curve == NULL) {
				curve = uECC_secp256k1();
				ECC_curves[4] = curve;
			}
			break;
		case SYM_SECP256R1:
			curve = ECC_curves[3];
			if(curve == NULL) {
				curve = uECC_secp256r1();
				ECC_curves[3] = curve;
			}
			break;
		case SYM_SECP224R1:
			curve = ECC_curves[2];
			if(curve == NULL) {
				curve = uECC_secp224r1();
				ECC_curves[2] = curve;
			}
			break;
		case SYM_SECP192R1:
			curve = ECC_curves[1];
			if(curve == NULL) {
				curve = uECC_secp192r1();
				ECC_curves[1] = curve;
			}
			break;		
		case SYM_SECP160R1:
			curve = ECC_curves[0];
			if(curve == NULL) {
				curve = uECC_secp160r1();
				ECC_curves[0] = curve;
			}
			break;
		default:
			return R_NONE;
	}

	if (ref_init) {
		if(!uECC_make_key(ecc->public, ecc->private, curve)) {
			puts("failed to init ECDH key");
			Trap0(RE_INVALID_HANDLE); //TODO: change to something better!
		} else return R_ARG1;
	}

	if (ref_secret) {
		if (IS_HANDLE(val_handle)) {
			bin = Make_Binary(32);
			if (!uECC_shared_secret(VAL_DATA(val_public), ecc->private, BIN_DATA(bin), curve)) {
				return R_NONE;
            }
			if(ref_release) {
				CLEARS(ecc);
				HANDLE_SET_FLAG(val_handle, HANDLE_RELEASABLE);
			}
			SET_BINARY(D_RET, bin);
			BIN_LEN(bin) = 32;
			return R_RET;
		}
		else {
			Trap0(RE_INVALID_HANDLE);
			return R_NONE;
		}
	}

	if (ref_public) {
		if (IS_HANDLE(val_handle)) {
			bin = Make_Binary(64);
			COPY_MEM(BIN_DATA(bin), ecc->public, 64);
			SET_BINARY(D_RET, bin);
			BIN_LEN(bin) = 64;
			return R_RET;
		}
		else {
			return R_NONE;
		}
	}

	if(ref_release) {
		CLEARS(ecc);
		HANDLE_SET_FLAG(val_handle, HANDLE_RELEASABLE);
		return R_ARG1;
	}

	if (ref_type) {
		if (IS_HANDLE(val_handle)) {
			Init_Word(val_curve, curve_type);
			return R_ARG3;
		}
		else {
			return R_NONE;
		}
	}
	return R_ARG1;
}


/***********************************************************************
**
*/	REBNATIVE(chacha20)
/*
//  chacha20: native [
//		"Encrypt/decrypt data using ChaCha20 algorithm. Returns stream cipher context handle or encrypted/decrypted data."
//		/key                "Provided only for the first time to get stream HANDLE!"
//			crypt-key [binary!] "Crypt key (16 or 32 bytes)."
//			nonce [binary!] "Initialization nonce (8 bytes)."
//			count [integer!] "A 32-bit block count parameter"
//		/stream
//			ctx [handle!]   "Stream cipher context."
//			data [binary!]  "Data to encrypt/decrypt."
//		/into
//			out [binary!]   "Output buffer (NOT YET IMPLEMENTED)"
//  ]
***********************************************************************/
{
	REBOOL  ref_key       = D_REF(1);
    REBVAL *val_crypt_key = D_ARG(2);
    REBVAL *val_nonce     = D_ARG(3);
	REBVAL *val_count     = D_ARG(4);
    REBOOL  ref_stream    = D_REF(5);
    REBVAL *val_ctx       = D_ARG(6);
    REBVAL *val_data      = D_ARG(7);
	REBOOL  ref_into      = D_REF(8);
    
    REBVAL *ret = D_RET;
	REBSER *ctx_ser;
	REBINT  len;

	if (ref_key) {
    	//key defined - setup new context
		
		len = VAL_LEN(val_crypt_key);

		if (len != 16 && len != 32 && VAL_LEN(val_nonce) != 8) {
			return R_NONE;
		}

		//making series from POOL so it will be GCed automaticaly
		ctx_ser = Make_Series(sizeof(chacha20_ctx), (REBCNT)1, FALSE);
		SERIES_TAIL(ctx_ser) = sizeof(chacha20_ctx);

		chacha20_setup(
			(chacha20_ctx*)ctx_ser->data,
			VAL_BIN_AT(val_crypt_key),
			len,
			VAL_BIN_AT(val_nonce)
		);
		chacha20_counter_set((chacha20_ctx*)ctx_ser->data, VAL_INT64(val_count));

		SET_HANDLE(ret, ctx_ser, SYM_CHACHA20, HANDLE_SERIES);
		// the ctx in the handle is released by GC once the handle is not referenced

    } else if(ref_stream) {

		ctx_ser = VAL_HANDLE_DATA(val_ctx);

    	if (VAL_HANDLE_TYPE(val_ctx) != SYM_CHACHA20 || ctx_ser == NULL || SERIES_TAIL(ctx_ser) != sizeof(chacha20_ctx)){
    		Trap0(RE_INVALID_HANDLE);
    	}

    	len = VAL_LEN(val_data);
    	if (len == 0) return R_NONE;

		REBYTE *data = VAL_BIN_AT(val_data);
		REBSER  *binaryOut = Make_Binary(len);

		chacha20_encrypt(
			(chacha20_ctx *)ctx_ser->data,
			(const uint8_t*)data,
			(      uint8_t*)BIN_DATA(binaryOut),
			len
		);

		SET_BINARY(ret, binaryOut);
		VAL_TAIL(ret) = len;

    }
	return R_RET;
}


#include "sys-poly1305.h"
/***********************************************************************
**
*/	REBNATIVE(poly1305)
/*
//  poly1305: native [
//		"poly1305 message-authentication"
//		ctx [handle! binary!] "poly1305 handle and or binary key for initialization (32 bytes)"
//		/update data [binary!] "data to authenticate"
//		/finish                "finish data stream and return raw result as a binary"
//		/verify                "finish data stream and compare result with expected result (MAC)"
//			mac      [binary!] "16 bytes of verification MAC"
//  ]
***********************************************************************/
{
	REBVAL *val_ctx       = D_ARG(1);
	REBOOL  ref_update    = D_REF(2);
	REBVAL *val_data      = D_ARG(3);
	REBOOL  ref_finish    = D_REF(4);
	REBOOL  ref_verify    = D_REF(5);
	REBVAL *val_mac       = D_ARG(6);
    
    REBVAL *ret = D_RET;
	REBSER *ctx_ser, *bin;
	REBINT  len;
	REBCNT  i;
	REBYTE  mac[16];

	if (IS_BINARY(val_ctx)) {
		len = VAL_LEN(val_ctx);
		if (len < 32) {
			Trap1(RE_INVALID_DATA, val_ctx);
			return R_NONE;
		}
		//making series from POOL so it will be GCed automaticaly
		ctx_ser = Make_Series(sizeof(poly1305_context), (REBCNT)1, FALSE);

		poly1305_init((poly1305_context*)ctx_ser->data, VAL_BIN_AT(val_ctx));

		SERIES_TAIL(ctx_ser) = sizeof(poly1305_context);
		SET_HANDLE(val_ctx, ctx_ser, SYM_POLY1305, HANDLE_SERIES);
		// the ctx_ser in the handle is released by GC once the handle is not referenced
	}
	else {
		ctx_ser = VAL_HANDLE_DATA(val_ctx);
		if (VAL_HANDLE_TYPE(val_ctx) != SYM_POLY1305 || ctx_ser == NULL || SERIES_TAIL(ctx_ser) != sizeof(poly1305_context)){
    		Trap0(RE_INVALID_HANDLE);
		}
	}

	if (ref_update) {
		poly1305_update((poly1305_context*)ctx_ser->data, VAL_BIN_AT(val_data), VAL_LEN(val_data));
	}

	if (ref_finish) {
		SET_BINARY(ret, Make_Series(16, (REBCNT)1, FALSE));
		VAL_TAIL(ret) = 16;
		poly1305_finish((poly1305_context*)ctx_ser->data, VAL_BIN(ret));
		return R_RET;
	}

	if (ref_verify) {
		if (VAL_LEN(val_mac) != 16)
			return R_FALSE; // or error?
		CLEARS(mac);
		poly1305_finish((poly1305_context*)ctx_ser->data, mac);
		return (poly1305_verify(VAL_BIN_AT(val_mac), mac)) ? R_TRUE : R_FALSE;
	}

	return R_ARG1;
}