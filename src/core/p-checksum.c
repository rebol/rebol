/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
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

//#include "sys-core.h"
#include "sys-sha2.h"
#include "reb-net.h"
#include "sys-md5.h"
#include "sys-sha1.h"



/***********************************************************************
**
*/	static void Init_sizes(REBVAL *method, int *blk, int *ctx)
/*
***********************************************************************/
{
    if(!method) return;
	switch (VAL_WORD_CANON(method)) {
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
	REBVAL *ctx  = BLK_SKIP(port, STD_PORT_LOCALS);

	if(!IS_BINARY(ctx)) {
		SET_BINARY(ctx, Make_Binary(ctx_size));
		PROTECT_SERIES(VAL_SERIES(ctx));
	} else {
		CLEAR(VAL_BIN(ctx), VAL_TAIL(ctx));
	}
	VAL_TAIL(ctx) = ctx_size;
	switch (VAL_WORD_CANON(method)) {
		case SYM_MD5:
			MD5_Init((MD5_CTX*)VAL_BIN(ctx));
			return TRUE;
		case SYM_SHA1:
			SHA1_Init((SHA_CTX*)VAL_BIN(ctx));
			return TRUE;
		case SYM_SHA256:
			SHA256_Init((SHA256_CTX*)VAL_BIN(ctx));
			return TRUE;
		case SYM_SHA384:
			SHA384_Init((SHA384_CTX*)VAL_BIN(ctx));
			return TRUE;
		case SYM_SHA512:
			SHA512_Init((SHA512_CTX*)VAL_BIN(ctx));
			return TRUE;
	}
	return FALSE;
}

/***********************************************************************
**
*/	static int Checksum_Actor(REBVAL *ds, REBSER *port, REBCNT action)
/*
***********************************************************************/
{
	REBVAL *spec;
	REBVAL *method;
	REBREQ *req;
	REBVAL *arg;
	REBCNT  args = 0;
	REBVAL *data;
	REBVAL *ctx;

	Validate_Port(port, action);

	spec = BLK_SKIP(port, STD_PORT_SPEC);
	if (!IS_OBJECT(spec)) Trap1(RE_INVALID_SPEC, spec);
	method = Obj_Value(spec, STD_PORT_SPEC_CHECKSUM_METHOD);
    if (!method || !IS_WORD(method)) {
        Trap1(RE_INVALID_SPEC, spec);
        return 0; //just to make xcode analyze happy
    }

	*D_RET = *D_ARG(1);

	req = Use_Port_State(port, RDI_CHECKSUM, sizeof(REBREQ));

	data = BLK_SKIP(port, STD_PORT_DATA); //will hold result
	ctx  = BLK_SKIP(port, STD_PORT_LOCALS);
	int ctx_size = 0, blk_size = 0;

	Init_sizes(method, &blk_size, &ctx_size);

	if(ctx_size == 0) Trap1(RE_INVALID_SPEC, method);

	switch (action) {
	case A_WRITE:
		if (!IS_OPEN(req)) {
			Checksum_Open(port, method, ctx_size);
			SET_OPEN(req);
		}
		args = Find_Refines(ds, ALL_WRITE_REFS);
		arg = D_ARG(2);
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

		//using copy so READ will not destroy intermediate context state by calling *_Final
		memcpy(DS_TOP, VAL_BIN(ctx), ctx_size);

		switch (VAL_WORD_CANON(method)) {
		case SYM_MD5:
			MD5_Final(VAL_BIN_DATA(data), (MD5_CTX*)DS_TOP);
			break;
		case SYM_SHA1:
			SHA1_Final(VAL_BIN_DATA(data), (SHA_CTX*)DS_TOP);
			break;
		case SYM_SHA256:
			SHA256_Final(VAL_BIN_DATA(data), (SHA256_CTX*)DS_TOP);
			break;
		case SYM_SHA384:
			SHA384_Final(VAL_BIN_DATA(data), (SHA384_CTX*)DS_TOP);
			break;
		case SYM_SHA512:
			SHA512_Final(VAL_BIN_DATA(data), (SHA512_CTX*)DS_TOP);
			break;
		}
		if(action == A_READ) *D_RET = *data;
		return R_RET;

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
		Trap_Action(REB_PORT, action);
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
