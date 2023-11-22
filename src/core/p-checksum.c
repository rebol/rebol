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
**  Module:  p-checksum.c
**  Summary: Checksum port interface
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

#ifdef INCLUDE_MBEDTLS
#include "sys-core.h"
#include "sys-checksum.h"
#include "reb-net.h"
#include "mbedtls/md4.h"
#include "mbedtls/md5.h"
#include "mbedtls/ripemd160.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"
#include "mbedtls/sha3.h"
#else
#include "deprecated/sys-sha2.h"
#include "reb-net.h"
#include "deprecated/sys-sha1.h"
#include "deprecated/sys-md5.h"
#endif


/***********************************************************************
**
*/	static void Init_sizes(REBVAL *method, REBLEN *blk, REBLEN *ctx)
/*
***********************************************************************/
{
    if(!method) return;
	switch (VAL_WORD_CANON(method)) {
#ifdef INCLUDE_MBEDTLS
		case SYM_MD5:
			*ctx = sizeof(mbedtls_md5_context);
			*blk = 16;
			break;
		case SYM_SHA1:
			*ctx = sizeof(mbedtls_sha1_context);
			*blk = 20;
			return;
		case SYM_SHA256:
			*ctx = sizeof(mbedtls_sha256_context);
			*blk = 32;
			return;
		case SYM_SHA224:
			*ctx = sizeof(mbedtls_sha256_context);
			*blk = 28;
			return;
#ifdef INCLUDE_SHA384
		case SYM_SHA384:
			*ctx = sizeof(mbedtls_sha512_context);
			*blk = 48;
			return;
#endif
		case SYM_SHA512:
			*ctx = sizeof(mbedtls_sha512_context);
			*blk = 64;
			return;
#ifdef INCLUDE_SHA3
		case SYM_SHA3_224:
			*ctx = sizeof(mbedtls_sha3_context);
			*blk = 28;
			return;
		case SYM_SHA3_256:
			*ctx = sizeof(mbedtls_sha3_context);
			*blk = 32;
			return;
		case SYM_SHA3_384:
			*ctx = sizeof(mbedtls_sha3_context);
			*blk = 48;
			return;
		case SYM_SHA3_512:
			*ctx = sizeof(mbedtls_sha3_context);
			*blk = 64;
			return;
#endif
#ifdef INCLUDE_RIPEMD160
		case SYM_RIPEMD160:
			*ctx = sizeof(mbedtls_ripemd160_context);
			*blk = 20;
			return;
#endif
#ifdef INCLUDE_MD4
		case SYM_MD4:
			*ctx = sizeof(mbedtls_md4_context);
			*blk = 16;
			break;
#endif
#else
		// NO MBEDTLS
		case SYM_MD5:
			*ctx = sizeof(MD5_CTX);
			*blk = MD5_DIGEST_LENGTH;
			break;
		case SYM_SHA1:
			*ctx = sizeof(SHA_CTX);
			*blk = SHA_DIGEST_LENGTH;
			break;
		case SYM_SHA256:
			*ctx = sizeof(SHA256_CTX);
			*blk = SHA256_DIGEST_LENGTH;
			return;
		case SYM_SHA384:
			*ctx = sizeof(SHA384_CTX);
			*blk = SHA384_DIGEST_LENGTH;
			return;
		case SYM_SHA512:
			*ctx = sizeof(SHA512_CTX);
			*blk = SHA512_DIGEST_LENGTH;
			return;
#endif
		default:
			*ctx = *blk = 0;
			break;
	}
}

/***********************************************************************
**
*/	static REBOOL Checksum_Open(REBSER *port, REBVAL *method, int ctx_size)
/*
***********************************************************************/
{
	//REBVAL *data = BLK_SKIP(port, STD_PORT_DATA);
	REBVAL *ctx  = BLK_SKIP(port, STD_PORT_EXTRA);

	if(!IS_BINARY(ctx)) {
		SET_BINARY(ctx, Make_Binary(ctx_size));
		PROTECT_SERIES(VAL_SERIES(ctx));
	} else {
		CLEAR(VAL_BIN(ctx), VAL_TAIL(ctx));
	}
	VAL_TAIL(ctx) = ctx_size;
	switch (VAL_WORD_CANON(method)) {
#ifdef INCLUDE_MBEDTLS
		case SYM_MD5:
			mbedtls_md5_starts((mbedtls_md5_context*)VAL_BIN(ctx));
			return TRUE;
		case SYM_SHA1:
			mbedtls_sha1_starts((mbedtls_sha1_context*)VAL_BIN(ctx));
			return TRUE;
		case SYM_SHA256:
			mbedtls_sha256_starts((mbedtls_sha256_context*)VAL_BIN(ctx), 0);
			return TRUE;
		case SYM_SHA224:
			mbedtls_sha256_starts((mbedtls_sha256_context*)VAL_BIN(ctx), 1);
			return TRUE;
		case SYM_SHA512:
			mbedtls_sha512_starts((mbedtls_sha512_context*)VAL_BIN(ctx), 0);
			return TRUE;
	#ifdef INCLUDE_SHA384
		case SYM_SHA384:
			mbedtls_sha512_starts((mbedtls_sha512_context*)VAL_BIN(ctx), 1);
			return TRUE;
	#endif
	#ifdef INCLUDE_SHA3
		case SYM_SHA3_224:
			SHA3_224_Starts((mbedtls_sha3_context*)VAL_BIN(ctx));
			return TRUE;
		case SYM_SHA3_256:
			SHA3_256_Starts((mbedtls_sha3_context*)VAL_BIN(ctx));
			return TRUE;
		case SYM_SHA3_384:
			SHA3_384_Starts((mbedtls_sha3_context*)VAL_BIN(ctx));
			return TRUE;
		case SYM_SHA3_512:
			SHA3_512_Starts((mbedtls_sha3_context*)VAL_BIN(ctx));
			return TRUE;
	#endif
	#ifdef INCLUDE_RIPEMD160
		case SYM_RIPEMD160:
			mbedtls_ripemd160_starts((mbedtls_ripemd160_context*)VAL_BIN(ctx));
			return TRUE;
	#endif
	#ifdef INCLUDE_MD4
		case SYM_MD4:
			mbedtls_md4_starts((mbedtls_md4_context*)VAL_BIN(ctx));
			return TRUE;
	#endif
#else
		case SYM_MD5:
			MD5_Starts((MD5_CTX*)VAL_BIN(ctx));
			return TRUE;
		case SYM_SHA1:
			SHA1_Starts((SHA_CTX*)VAL_BIN(ctx));
			return TRUE;
		case SYM_SHA256:
			SHA256_Starts((SHA256_CTX*)VAL_BIN(ctx));
			return TRUE;
		case SYM_SHA384:
			SHA384_Starts((SHA384_CTX*)VAL_BIN(ctx));
			return TRUE;
		case SYM_SHA512:
			SHA512_Starts((SHA512_CTX*)VAL_BIN(ctx));
			return TRUE;
#endif
	}
	return FALSE;
}

/***********************************************************************
**
*/	static int Checksum_Actor(REBVAL *ds, REBVAL *port_value, REBCNT action)
/*
***********************************************************************/
{
	REBSER *port;
	REBVAL *spec;
	REBVAL *method;
	REBREQ *req;
	REBVAL *arg;
	REBCNT  args = 0;
	REBVAL *data;
	REBVAL *ctx;

	port = Validate_Port_Value(port_value);

	spec = BLK_SKIP(port, STD_PORT_SPEC);
	if (!IS_OBJECT(spec)) Trap1(RE_INVALID_SPEC, spec);
	method = Obj_Value(spec, STD_PORT_SPEC_CHECKSUM_METHOD);
    if (!method || !IS_WORD(method)) {
        Trap1(RE_INVALID_SPEC, spec);
        return 0; //just to make xcode analyze happy
    }

	req = Use_Port_State(port, RDI_CHECKSUM, sizeof(REBREQ));

	data = BLK_SKIP(port, STD_PORT_DATA); //will hold result
	ctx  = BLK_SKIP(port, STD_PORT_EXTRA);
	REBLEN ctx_size = 0, blk_size = 0;

	Init_sizes(method, &blk_size, &ctx_size);

	if(ctx_size == 0) Trap1(RE_INVALID_SPEC, method);

	*D_RET = *port_value;

	switch (action) {
	case A_WRITE:
		if (!IS_OPEN(req)) {
			Checksum_Open(port, method, ctx_size);
			SET_OPEN(req);
		}
		args = Find_Refines(ds, ALL_WRITE_REFS);
		arg = D_ARG(2);
		if (!ANY_BINSTR(arg)) Trap_Arg(arg);
		REBI64  pos = (REBI64)VAL_INDEX(arg);
		if (args & AM_WRITE_SEEK) {
			pos += Int64(D_ARG(ARG_WRITE_INDEX));
			if(pos < 0) pos = 0;
			else if (pos > VAL_TAIL(arg)) pos = VAL_TAIL(arg);
		}
		REBI64 part = (REBI64)VAL_TAIL(arg) - pos;
		if (args & AM_WRITE_PART) {	
			REBI64 cnt = Int64(D_ARG(ARG_WRITE_LENGTH));
			if(cnt < 0) {
				cnt = -cnt;
				pos -= cnt;
				if (pos < 0) {
					cnt += pos;
					pos = 0;
				}
				part = cnt;
			} else if (cnt < part) part = cnt;
		}
		if(part <= 0) return R_RET;
		switch (VAL_WORD_CANON(method)) {
#ifdef INCLUDE_MBEDTLS
			case SYM_MD5:
				mbedtls_md5_update((mbedtls_md5_context*)VAL_BIN(ctx), VAL_BIN_SKIP(arg, pos), part);
				break;
			case SYM_SHA1:
				mbedtls_sha1_update((mbedtls_sha1_context*)VAL_BIN(ctx), VAL_BIN_SKIP(arg, pos), part);
				break;
			case SYM_SHA256:
			case SYM_SHA224:
				mbedtls_sha256_update((mbedtls_sha256_context*)VAL_BIN(ctx), VAL_BIN_SKIP(arg, pos), part);
				break;
			case SYM_SHA512:
#ifdef INCLUDE_SHA384
			case SYM_SHA384:
#endif
				mbedtls_sha512_update((mbedtls_sha512_context*)VAL_BIN(ctx), VAL_BIN_SKIP(arg, pos), part);
				break;
#ifdef INCLUDE_SHA3
			case SYM_SHA3_224:
			case SYM_SHA3_256:
			case SYM_SHA3_384:
			case SYM_SHA3_512:
				SHA3_Update((mbedtls_sha3_context*)VAL_BIN(ctx), VAL_BIN_SKIP(arg, pos), part);
				break;
#endif
#ifdef INCLUDE_RIPEMD160
			case SYM_RIPEMD160:
				mbedtls_ripemd160_update((mbedtls_ripemd160_context*)VAL_BIN(ctx), VAL_BIN_SKIP(arg, pos), part);
				break;
#endif
#ifdef INCLUDE_MD4
			case SYM_MD4:
				mbedtls_md4_update((mbedtls_md4_context*)VAL_BIN(ctx), VAL_BIN_SKIP(arg, pos), part);
				break;
#endif
#else
			case SYM_MD5:
				MD5_Update((MD5_CTX*)VAL_BIN(ctx), VAL_BIN_SKIP(arg, pos), part);
				break;
			case SYM_SHA1:
				SHA1_Update((SHA_CTX*)VAL_BIN(ctx), VAL_BIN_SKIP(arg, pos), part);
				break;
			case SYM_SHA256:
				SHA256_Update((SHA256_CTX*)VAL_BIN(ctx), VAL_BIN_SKIP(arg, pos), part);
				break;
			case SYM_SHA384:
				SHA384_Update((SHA384_CTX*)VAL_BIN(ctx), VAL_BIN_SKIP(arg, pos), part);
				break;
			case SYM_SHA512:
				SHA512_Update((SHA512_CTX*)VAL_BIN(ctx), VAL_BIN_SKIP(arg, pos), part);
				break;
#endif
			}
		break;
	case A_READ:
	case A_UPDATE:
		if (!IS_OPEN(req)) return R_NONE;

		if (!IS_BINARY(data)) {
			SET_BINARY(data, Make_Binary(blk_size));
			VAL_TAIL(data) = blk_size;
		}
		else {
			if (blk_size > SERIES_REST(VAL_SERIES(data)) - VAL_INDEX(data)) {
				Expand_Series(VAL_SERIES(data), AT_TAIL, blk_size - VAL_TAIL(data));
			}
			if (VAL_TAIL(data) < VAL_INDEX(data) + blk_size) {
				VAL_TAIL(data) = VAL_INDEX(data) + blk_size;
			}
		}

		//using copy so READ will not destroy intermediate context state by calling *_Finish
		memcpy(DS_TOP, VAL_BIN(ctx), ctx_size);

		switch (VAL_WORD_CANON(method)) {

#ifdef INCLUDE_MBEDTLS
		case SYM_MD5:
			mbedtls_md5_finish((mbedtls_md5_context*)DS_TOP, VAL_BIN_DATA(data));
			break;
		case SYM_SHA1:
			mbedtls_sha1_finish((mbedtls_sha1_context*)DS_TOP, VAL_BIN_DATA(data));
			break;
		case SYM_SHA256:
		case SYM_SHA224:
			mbedtls_sha256_finish((mbedtls_sha256_context*)DS_TOP, VAL_BIN_DATA(data));
			break;
		case SYM_SHA512:
#ifdef INCLUDE_SHA384
		case SYM_SHA384:
#endif
			mbedtls_sha512_finish((mbedtls_sha512_context*)DS_TOP, VAL_BIN_DATA(data));
			break;
#ifdef INCLUDE_SHA3
		case SYM_SHA3_224:
			SHA3_224_Finish((mbedtls_sha3_context*)DS_TOP, VAL_BIN_DATA(data));
			break;
		case SYM_SHA3_256:
			SHA3_256_Finish((mbedtls_sha3_context*)DS_TOP, VAL_BIN_DATA(data));
			break;
		case SYM_SHA3_384:
			SHA3_384_Finish((mbedtls_sha3_context*)DS_TOP, VAL_BIN_DATA(data));
			break;
		case SYM_SHA3_512:
			SHA3_512_Finish((mbedtls_sha3_context*)DS_TOP, VAL_BIN_DATA(data));
			break;
#endif
#ifdef INCLUDE_RIPEMD160
		case SYM_RIPEMD160:
			mbedtls_ripemd160_finish((mbedtls_ripemd160_context*)DS_TOP, VAL_BIN_DATA(data));
			break;
#endif
#ifdef INCLUDE_MD4
		case SYM_MD4:
			mbedtls_md4_finish((mbedtls_md4_context*)DS_TOP, VAL_BIN_DATA(data));
			break;
#endif
#else
		case SYM_MD5:
			MD5_Finish((MD5_CTX*)DS_TOP, VAL_BIN_DATA(data));
			break;
		case SYM_SHA1:
			SHA1_Finish((SHA_CTX*)DS_TOP, VAL_BIN_DATA(data));
			break;
		case SYM_SHA256:
			SHA256_Finish((SHA256_CTX*)DS_TOP, VAL_BIN_DATA(data));
			break;
		case SYM_SHA384:
			SHA384_Finish((SHA384_CTX*)DS_TOP, VAL_BIN_DATA(data));
			break;
		case SYM_SHA512:
			SHA512_Finish((SHA512_CTX*)DS_TOP, VAL_BIN_DATA(data));
			break;
#endif
		}
		if(action == A_READ) *D_RET = *data;
		break;

	case A_OPEN:
		if (Checksum_Open(port, method, ctx_size)) {
			SET_OPEN(req);
		} else {
			//@@ throw an error?
			//Trap_Port(RE_CANNOT_OPEN, port, 0);
			return R_FALSE;
		}
		break;

	case A_CLOSE:
		if (IS_OPEN(req)) {
			if (IS_BINARY(ctx)) {
				CLEAR(VAL_BIN(ctx), ctx_size);
				VAL_TAIL(ctx) = 0;
			}
			SET_NONE(data);
			SET_CLOSED(req);
		}
		break;

	case A_OPENQ:
		if (IS_OPEN(req)) return R_TRUE;
		return R_FALSE;

	default:
		Trap1(RE_NO_PORT_ACTION, Get_Action_Word(action));
	}
	return R_RET;
}


/***********************************************************************
**
*/	void Init_Checksum_Scheme(void)
/*
***********************************************************************/
{
	Register_Scheme(SYM_CHECKSUM, 0, Checksum_Actor);
}

/* DEFINE_DEV would normally be in os/dev-checksum.c but I'm not using it so it is here */
DEFINE_DEV(Dev_Checksum, "Checksum", 1, NULL, RDC_MAX, 0);
