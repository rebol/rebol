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

const REBYTE rc4_name[] = "RC4-context"; //Used as a context handle name

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