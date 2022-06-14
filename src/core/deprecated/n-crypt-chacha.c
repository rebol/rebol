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
**  Module:  n-crypt-aes.c
**  Summary: native functions for old AES code
**  Section: natives
**  Author:  Oldes, Cyphre
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

#ifdef INCLUDE_CHACHA20POLY1305_DEPRECATED
#include "sys-chacha20.h"
#include "sys-poly1305.h"
#endif


/***********************************************************************
**
*/	REBNATIVE(chacha20)
/*
//  chacha20: native [
//		"Encrypt/decrypt data using ChaCha20 algorithm. Returns stream cipher context handle or encrypted/decrypted data."
//		ctx [handle! binary!] "ChaCha20 handle and or binary key for initialization (16 or 32 bytes)"
//		/init
//			nonce [binary!] "Initialization nonce (IV) - 8 or 12 bytes."
//			count [integer!] "A 32-bit block count parameter"
//		/aad sequence [integer!] "Sequence number used with /init to modify nonce"
//		/stream
//			data [binary!]  "Data to encrypt/decrypt."
//		/into
//			out [binary!]   "Output buffer (NOT YET IMPLEMENTED)"
//  ]
***********************************************************************/
{
#ifndef INCLUDE_CHACHA20POLY1305_DEPRECATED
	Trap0(RE_FEATURE_NA);
#else
	REBVAL *val_ctx       = D_ARG(1);
	REBOOL  ref_init      = D_REF(2);
    REBVAL *val_nonce     = D_ARG(3);
	REBVAL *val_counter   = D_ARG(4);
	REBOOL  ref_aad       = D_REF(5);
	REBVAL *val_sequence  = D_ARG(6);
    REBOOL  ref_stream    = D_REF(7);
    REBVAL *val_data      = D_ARG(8);
	REBOOL  ref_into      = D_REF(9);
	
	if(ref_into)
		Trap0(RE_FEATURE_NA);

	REBINT  len;
	REBU64  sequence;

	if (IS_BINARY(val_ctx)) {
		len = VAL_LEN(val_ctx);
		if (!(len == 32 || len == 16)) {
			//printf("ChaCha20 key must be of size 32 or 16 bytes! Is: %i\n", len);
			Trap1(RE_INVALID_DATA, val_ctx);
			return R_NONE;
		}
		REBYTE *bin_key = VAL_BIN_AT(val_ctx);
		
		MAKE_HANDLE(val_ctx, SYM_CHACHA20);
		chacha20_keysetup((chacha20_ctx*)VAL_HANDLE_CONTEXT_DATA(val_ctx), bin_key, len);
	}
	else {
		if (NOT_VALID_CONTEXT_HANDLE(val_ctx, SYM_CHACHA20)) {
    		Trap0(RE_INVALID_HANDLE);
			return R_NONE; // avoid wornings later
		}
	}

	if (ref_init) {
		// initialize nonce with counter
		
		len = VAL_LEN(val_nonce);

		if (!(len == 12 || len == 8)) {
			Trap1(RE_INVALID_DATA, val_nonce);
			return R_NONE;
		}

		sequence = (ref_aad) ? VAL_INT64(val_sequence) : 0;
		chacha20_ivsetup((chacha20_ctx*)VAL_HANDLE_CONTEXT_DATA(val_ctx), VAL_BIN_AT(val_nonce), len, VAL_INT64(val_counter), (u8 *)&sequence);
    }
	
	if (ref_stream) {

    	len = VAL_LEN(val_data);
    	if (len == 0) return R_NONE;

		REBYTE *data = VAL_BIN_AT(val_data);
		REBSER  *binaryOut = Make_Binary(len);

		chacha20_encrypt(
			(chacha20_ctx *)VAL_HANDLE_CONTEXT_DATA(val_ctx),
			(const uint8_t*)data,
			(      uint8_t*)BIN_DATA(binaryOut),
			len
		);

		SET_BINARY(val_ctx, binaryOut);
		VAL_TAIL(val_ctx) = len;

    }
#endif // EXCLUDE_CHACHA20POLY1305
	return R_ARG1;
}



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
#ifndef INCLUDE_CHACHA20POLY1305_DEPRECATED
	Trap0(RE_FEATURE_NA);
#else
	REBVAL *val_ctx       = D_ARG(1);
	REBOOL  ref_update    = D_REF(2);
	REBVAL *val_data      = D_ARG(3);
	REBOOL  ref_finish    = D_REF(4);
	REBOOL  ref_verify    = D_REF(5);
	REBVAL *val_mac       = D_ARG(6);
    
    REBVAL *ret = D_RET;
//	REBSER *ctx_ser;
	REBINT  len;
//	REBCNT  i;
	REBYTE  mac[16];

	if (IS_BINARY(val_ctx)) {
		len = VAL_LEN(val_ctx);
		if (len < POLY1305_KEYLEN) {
			Trap1(RE_INVALID_DATA, val_ctx);
			return R_NONE;
		}
		
		REBYTE *bin_key = VAL_BIN_AT(val_ctx);
		
		MAKE_HANDLE(val_ctx, SYM_POLY1305);
		poly1305_init((poly1305_context*)VAL_HANDLE_CONTEXT_DATA(val_ctx), bin_key);
	}
	else {
		if (NOT_VALID_CONTEXT_HANDLE(val_ctx, SYM_POLY1305)) {
    		Trap0(RE_INVALID_HANDLE);
		}
	}

	if (ref_update) {
		poly1305_update((poly1305_context*)VAL_HANDLE_CONTEXT_DATA(val_ctx), VAL_BIN_AT(val_data), VAL_LEN(val_data));
	}

	if (ref_finish) {
		SET_BINARY(ret, Make_Series(16, 1, FALSE));
		VAL_TAIL(ret) = 16;
		poly1305_finish((poly1305_context*)VAL_HANDLE_CONTEXT_DATA(val_ctx), VAL_BIN(ret));
		return R_RET;
	}

	if (ref_verify) {
		if (VAL_LEN(val_mac) != POLY1305_TAGLEN)
			return R_FALSE; // or error?
		CLEARS(mac);
		poly1305_finish((poly1305_context*)VAL_HANDLE_CONTEXT_DATA(val_ctx), mac);
		return (poly1305_verify(VAL_BIN_AT(val_mac), mac)) ? R_TRUE : R_FALSE;
	}

#endif // EXCLUDE_CHACHA20POLY1305
	return R_ARG1;
}

/***********************************************************************
**
*/	REBNATIVE(chacha20poly1305)
/*
//  chacha20poly1305: native [
//		"..."
//		ctx [none! handle!]
//		/init 
//			local-key     [binary!]
//			local-iv      [binary!]
//			remote-key    [binary!]
//			remote-iv     [binary!]
//		/encrypt
//			data-out      [binary!]
//			aad-out       [binary!]
//		/decrypt
//			data-in       [binary!]
//			aad-in        [binary!]
//  ]
***********************************************************************/
{
#ifndef INCLUDE_CHACHA20POLY1305_DEPRECATED
	Trap0(RE_FEATURE_NA);
#else
	REBVAL *val_ctx        = D_ARG(1);
	REBOOL  ref_init       = D_REF(2);
	REBVAL *val_local_key  = D_ARG(3);
	REBVAL *val_local_iv   = D_ARG(4);
	REBVAL *val_remote_key = D_ARG(5);
	REBVAL *val_remote_iv  = D_ARG(6);
	REBOOL  ref_encrypt    = D_REF(7);
	REBVAL *val_plain      = D_ARG(8);
	REBVAL *val_local_aad  = D_ARG(9);
	REBOOL  ref_decrypt    = D_REF(10);
	REBVAL *val_cipher     = D_ARG(11);
	REBVAL *val_remote_aad = D_ARG(12);

	REBSER *ctx_ser;
	REBINT  len;
	chacha20poly1305_ctx *chacha;
	unsigned char poly1305_key[POLY1305_KEYLEN];
	size_t aad_size;
	REBU64 sequence = 0;

	if (ref_init) {
		MAKE_HANDLE(val_ctx, SYM_CHACHA20POLY1305);

		chacha = (chacha20poly1305_ctx*)VAL_HANDLE_CONTEXT_DATA(val_ctx);
		len = VAL_LEN(val_local_key);
		if (!(len == 32 || len == 16))
			Trap1(RE_INVALID_DATA, val_local_key);
		chacha20_keysetup(&chacha->local_chacha, VAL_BIN_AT(val_local_key), len);
		len = VAL_LEN(val_remote_key);
		if (!(len == 32 || len == 16))
			Trap1(RE_INVALID_DATA, val_remote_key);
		chacha20_keysetup(&chacha->remote_chacha, VAL_BIN_AT(val_remote_key), len);

		len = VAL_LEN(val_local_iv);
		if (!(len == 12 || len == 8))
			Trap1(RE_INVALID_DATA, val_local_iv);
		chacha20_ivsetup(&chacha->local_chacha, VAL_BIN_AT(val_local_iv), len, 1, (u8 *)&sequence);
		memcpy(chacha->local_iv, VAL_BIN_AT(val_local_iv), len);

		len = VAL_LEN(val_remote_iv);
		if (!(len == 12 || len == 8))
			Trap1(RE_INVALID_DATA, val_remote_iv);
		chacha20_ivsetup(&chacha->remote_chacha, VAL_BIN_AT(val_remote_iv), len, 1, (u8 *)&sequence);
		memcpy(chacha->remote_iv, VAL_BIN_AT(val_remote_iv), len);
		return R_ARG1;
	}

	if (NOT_VALID_CONTEXT_HANDLE(val_ctx, SYM_CHACHA20POLY1305)) {
    	Trap0(RE_INVALID_HANDLE);
		return R_NONE;
	}
	chacha = (chacha20poly1305_ctx*)VAL_HANDLE_CONTEXT_DATA(val_ctx);

	if (ref_encrypt) {
		chacha20_ivsetup(&chacha->local_chacha, chacha->local_iv, 12, 1,  VAL_BIN_AT(val_local_aad));
		chacha20_poly1305_key(&chacha->local_chacha, poly1305_key);
		//puts("poly1305_key:"); Dump_Bytes(poly1305_key, POLY1305_KEYLEN);
		
		len = VAL_LEN(val_plain) + POLY1305_TAGLEN;
		ctx_ser = Make_Series(len, (REBCNT)1, FALSE);

		// AEAD
		// sequence number (8 bytes)
		// content type (1 byte)
		// version (2 bytes)
		// length (2 bytes)
		unsigned char aad[13];
		aad_size = sizeof(aad);
//		unsigned char *sequence = aad;

		chacha20_poly1305_aead(&chacha->local_chacha, VAL_BIN_AT(val_plain), (REBCNT)len-POLY1305_TAGLEN, VAL_BIN_AT(val_local_aad), VAL_LEN(val_local_aad), poly1305_key, ctx_ser->data);

		//chacha->local_sequence++;

		SERIES_TAIL(ctx_ser) = len;
		SET_BINARY(val_ctx, ctx_ser);
		return R_ARG1;
	}

	if (ref_decrypt) {
		static unsigned char zeropad[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		unsigned char trail[16];
		unsigned char mac_tag[POLY1305_TAGLEN];

		chacha20_ivsetup(&chacha->remote_chacha, chacha->remote_iv, 12, 1, VAL_BIN_AT(val_remote_aad));

		len = VAL_LEN(val_cipher) - POLY1305_TAGLEN;
		if (len <= 0)
			return R_NONE;

		ctx_ser = Make_Series(len, (REBCNT)1, FALSE);

		//puts("\nDECRYPT:");

		chacha20_encrypt(&chacha->remote_chacha, VAL_BIN_AT(val_cipher), ctx_ser->data, len);
		//chacha_encrypt_bytes(&chacha->remote_chacha, VAL_BIN_AT(val_cipher), ctx_ser->data, len);
		chacha20_poly1305_key(&chacha->remote_chacha, poly1305_key);
		//puts("poly1305_key:"); Dump_Bytes(poly1305_key, POLY1305_KEYLEN);

		poly1305_context aead_ctx;
		poly1305_init(&aead_ctx, poly1305_key);

		aad_size = VAL_LEN(val_remote_aad);
		poly1305_update(&aead_ctx, VAL_BIN_AT(val_remote_aad), aad_size);
		int rem = aad_size % 16;
        if (rem)
            poly1305_update(&aead_ctx, zeropad, 16 - rem);
		//puts("update:"); Dump_Bytes(VAL_BIN_AT(val_cipher), len);
        poly1305_update(&aead_ctx, VAL_BIN_AT(val_cipher), len);
        rem = len % 16;
        if (rem)
            poly1305_update(&aead_ctx, zeropad, 16 - rem);
            
        U32TO8_LE(&trail[0], aad_size == 5 ? 5 : 13);
        *(int *)&trail[4] = 0;
        U32TO8_LE(&trail[8], len);
        *(int *)&trail[12] = 0;

		//puts("trail:"); Dump_Bytes(trail, 16);

        poly1305_update(&aead_ctx, trail, 16);
        poly1305_finish(&aead_ctx, mac_tag);

		if (!poly1305_verify(mac_tag, VAL_BIN_TAIL(val_cipher) - POLY1305_TAGLEN)) {
			//puts("MAC verification failed!");
			return R_NONE;
		}

		//puts("mac result:"); Dump_Bytes(mac_tag, POLY1305_TAGLEN);

		SERIES_TAIL(ctx_ser) = len;
		SET_BINARY(val_ctx, ctx_ser);
	}
#endif // EXCLUDE_CHACHA20POLY1305
	return R_ARG1;
}