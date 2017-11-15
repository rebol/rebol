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

const REBYTE rc4_name[] = "RC4-context"; //Used as a context handle name
const REBYTE aes_name[] = "AES-context";

/**********************************************************************/
//
//  rc4: native [
//
//  "Encrypt/decrypt data (modifies) using RC4 algorithm."
//
//      /key "Provided only for the first time to get stream HANDLE!"
//     	crypt-key [binary!] "Crypt key."
//      /stream
//		ctx       [handle!] "Stream cipher context."
//      data      [binary! none!] "Data to encrypt/decrypt."
//  ]
REBNATIVE(rc4)
{
    REBOOL  ref_key       = D_REF(1);
    REBVAL *val_crypt_key = D_ARG(2); 
    REBOOL  ref_stream    = D_REF(3);
    REBVAL *val_ctx       = D_ARG(4);
    REBVAL *val_data      = D_ARG(5);

    REBVAL *ret = D_RET;
	REBSER *ctx;

    if(ref_stream) {
    	ctx = (REBSER*)VAL_HANDLE(val_ctx);

    	if (VAL_HANDLE_NAME(val_ctx) != rc4_name) {
    		Trap0(RE_INVALID_HANDLE);
    	}

    	REBYTE *data = VAL_BIN_AT(val_data);
    	RC4_crypt((RC4_CTX*)ctx->data, data, data, VAL_LEN(val_data));
    	DS_RET_VALUE(val_data);

    } else if (ref_key) {
    	//key defined - setup new context
		//making series from POOL so it will be GCed automaticaly
		REBSER* ctx = Make_Series(sizeof(RC4_CTX), (REBCNT)1, FALSE);

		RC4_setup(
			(RC4_CTX*)ctx->data,
            VAL_BIN_AT(val_crypt_key),
            VAL_LEN(val_crypt_key)
        );

        SET_HANDLE(ret, ctx);
		VAL_HANDLE_NAME(ret) = rc4_name;
    }
    return R_RET;
}


/**********************************************************************/
//
//  aes: native [
//
//	"Encrypt/decrypt data using AES algorithm. Returns stream cipher context handle or encrypted/decrypted data."
//
//		/key                "Provided only for the first time to get stream HANDLE!."
//		crypt-key [binary!] "Crypt key (16 or 32 bytes)."
//		iv  [none! binary!] "Optional initialization vector (16 bytes)."
//		/decrypt            "Use the crypt-key for decryption (default is to encrypt)"
//		/stream
//		ctx       [handle!] "Stream cipher context."
//		data      [binary!] "Data to encrypt/decrypt. Or NONE to close the cipher stream."

//  ]
REBNATIVE(aes)
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
	REBYTE *data;

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

		AES_set_key(
			(AES_CTX*)ctx->data,
			VAL_BIN_AT(val_crypt_key),
			(const uint8_t *)iv,
			(len == 128) ? AES_MODE_128 : AES_MODE_256
		);

		if (ref_decrypt) AES_convert_key((AES_CTX*)ctx->data);

		SET_HANDLE(ret, ctx);
		VAL_HANDLE_NAME(ret) = aes_name;
    
    } else if(ref_stream) {

    	if (VAL_HANDLE_NAME(val_ctx) != aes_name) {
    		Trap0(RE_INVALID_HANDLE);
    	}

    	ctx = (REBSER*)VAL_HANDLE(val_ctx);

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
			AES_cbc_decrypt(
				aes_ctx,
				(const uint8_t*)data,
				BIN_HEAD(binaryOut),
				pad_len
			);
		}
		else {
			AES_cbc_encrypt(
				aes_ctx,
				(const uint8_t*)data,
				BIN_HEAD(binaryOut),
				pad_len
			);
		}
		if (pad_data) FREE_MEM(pad_data);

		SET_BINARY(ret, binaryOut);
		VAL_TAIL(ret) = pad_len;

    }
	return R_RET;
}