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
**  Module:  n-audio.c
**  Summary: native audio related functions
**  Section: natives
**  Author:  Oldes
**  Notes:
**
***********************************************************************/

#include "sys-core.h"


const REBYTE sound_name[] = "sound!"; //Used as a context handle name
//const REBYTE aes_name[] = "AES-context";

/***********************************************************************
**
*/	REBNATIVE(audio)
/*
//	audio: native [
//		"Audio DSL entry"
//		commands [block!] "Audio commands to evaluate."
//	]
***********************************************************************/
{
	REBVAL *temp;
	REBINT ssp;  // starting stack pointer

	DS_PUSH_NONE;
	temp = DS_TOP;
	ssp = DSP;

    REBVAL *value = VAL_BLK(D_ARG(1));
	REBSER *ctx;
	REBVAL *next;

	REBSER *frame;
	REBVAL *spec;
	REBSER *obj;

	for (; NOT_END(value); value++) {
		if (IS_SET_WORD(value)) {
			puts("-- found set-word?");
			DS_PUSH(value);
		} else {
			// Get value:
			if (IS_WORD(value)) {
				switch (VAL_WORD_CANON(value)) {
				case SYM_LOAD:
					printf("nextvaltype: %i\n", VAL_TYPE(value+1));
					//ctx = Make_Series(1024, (REBCNT)1, FALSE);
					spec = Get_System(SYS_STANDARD, STD_SOUND);
					if (!IS_OBJECT(spec)) Trap_Arg(spec);
					obj = CLONE_OBJECT(VAL_OBJ_FRAME(spec));

					SET_HANDLE(temp, obj);
					VAL_HANDLE_NAME(temp) = sound_name;
					puts("KEY");
					break;
				case SYM_CREATE:
					// Build standard sound object:
					spec = Get_System(SYS_STANDARD, STD_SOUND);
					if (!IS_OBJECT(spec)) Trap_Arg(spec);
					obj = CLONE_OBJECT(VAL_OBJ_FRAME(spec));
					//SET_OBJECT(temp, VAL_OBJ_FRAME(obj));
					//puts("CREATE");
					break;
				case SYM_PLAY:
					next = value + 1;
					if (IS_HANDLE(next) || (IS_WORD(next) && IS_HANDLE(Get_Var(next)))) {
						puts("haauds handle");
						obj = (REBVAL*)VAL_HANDLE(next);
						SET_OBJECT(temp, obj);
						value++;
					}
					else {
						puts("command missing handle value");
					}
					
					//SET_
					break;
				case SYM_NONE:
					SET_NONE(temp);
					break;
				case SYM_TRUE:
				case SYM_ON:
				case SYM_YES:
					SET_TRUE(temp);
					break;
				case SYM_FALSE:
				case SYM_OFF:
				case SYM_NO:
					SET_FALSE(temp);
					break;
				default:
					*temp = *value;
					VAL_SET(temp, REB_WORD);
				}
			}
			else if (IS_LIT_WORD(value)) {
				*temp = *value;
				VAL_SET(temp, REB_WORD);
			}
			else if (IS_LIT_PATH(value)) {
				*temp = *value;
				VAL_SET(temp, REB_PATH);
			}
			else if (VAL_TYPE(value) >= REB_NONE) { // all valid values
				*temp = *value;
			}
			else
				SET_NONE(temp);

			// Set prior set-words:
			while (DSP > ssp) {
				Set_Var(DS_TOP, temp);
				DS_DROP;
			}
		}
	}
	DS_DROP; // temp
#if 0
	REBSER *ser;
	REBINT index;
//	REBINT tail;

//  index = (REBINT)VAL_INDEX(value);
//	tail  = (REBINT)VAL_TAIL(value);
//	ser   = VAL_SERIES(value);

	REBVAL *cmds = VAL_BLK(val_commands);
	REBVAL *tail = VAL_BLK_TAIL(val_commands);
	REBINT len = VAL_BLK_LEN(val_commands);

	printf("Audio commands: %i %i\n", len, tail - cmds);

	while(cmds < tail) {
		printf("Value has type ID: %i\n", VAL_TYPE(cmds));
		if (IS_SET_WORD(cmds)) {
			//Set_Var((REBVAL*)cmds, (REBVAL*)cmds);
			//SET_LOGIC(cmds, TRUE);
			
			//SET_TRUE(Get_Var(cmds));
			SET_INTEGER(Get_Var(cmds), 42);
		}
		if (IS_WORD(cmds) && VAL_WORD_CANON(cmds) == SYM_ON) {
			puts("ON");
		}
		cmds++;
	}
#endif
	/*

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
    return R_RET;*/
    return R_ARG1;
}
