/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 Saphirion AG
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
**  Title: Core extension. Contains commands not yet migrated to core codebase.
**  Author: Richard Smolak
**
************************************************************************
**
**  NOTE to PROGRAMMERS:
**
**    1. Keep code clear and simple.
**    2. Document unusual code, reasoning, or gotchas.
**    3. Use same style for code, vars, indent(4), comments, etc.
**    4. Keep in mind Linux, OS X, BSD, big/little endian CPUs.
**    5. Test everything, then test it again.
**
***********************************************************************/

#ifdef TO_WIN32
#include <windows.h>
#endif

#include <stdlib.h> //for free()

#include "reb-host.h"
#include "host-lib.h"

#include "rc4/rc4.h"
#include "rsa/rsa.h"
#include "dh/dh.h"
#include "aes/aes.h"

#define INCLUDE_EXT_DATA
#include "host-ext-core.h"

//***** Externs *****
#ifdef TO_WIN32
extern void Console_Window(BOOL show);
extern void Console_Output(BOOL state);
extern REBINT As_OS_Str(REBSER *series, REBCHR **string);
extern REBOOL OS_Request_Dir(REBCHR *title, REBCHR **folder, REBCHR *path);
#endif

REBYTE *encapBuffer = NULL;
REBINT encapBufferLen;
RL_LIB *RL; // Link back to reb-lib from embedded extensions
static u32 *core_ext_words;

/***********************************************************************
**
*/	RXIEXT int RXD_Core(int cmd, RXIFRM *frm, REBCEC *data)
/*
**		Core command extension dispatcher.
**
***********************************************************************/
{
		switch (cmd) {

		case CMD_CORE_RC4:
		{
			RC4_CTX *ctx;
			REBSER *data, key;
			REBYTE *dataBuffer;

			if (RXA_TYPE(frm, 4) == RXT_HANDLE) {
				//set current context
				ctx = (RC4_CTX*)RXA_HANDLE(frm, 4);

				if (RXA_TYPE(frm, 5) == RXT_NONE) {
					//destroy context
					OS_Free(ctx);
					RXA_LOGIC(frm, 1) = TRUE;
					RXA_TYPE(frm,1) = RXT_LOGIC;
					return RXR_VALUE;
				}
				
				//get data
				data = RXA_SERIES(frm,5);
				dataBuffer = (REBYTE *)RL_SERIES(data, RXI_SER_DATA) + RXA_INDEX(frm,5);

				RC4_crypt(ctx, dataBuffer, dataBuffer, RL_SERIES(data, RXI_SER_TAIL) - RXA_INDEX(frm,5));

			} else if (RXA_TYPE(frm, 2) == RXT_BINARY) {
				//key defined - setup new context
				ctx = (RC4_CTX*)OS_Make(sizeof(*ctx));
				memset(ctx, 0, sizeof(*ctx));
				
				key = RXA_SERIES(frm, 2);

				RC4_setup(ctx, (REBYTE *)RL_SERIES(key, RXI_SER_DATA) + RXA_INDEX(frm, 2), RL_SERIES(key, RXI_SER_TAIL) - RXA_INDEX(frm, 2));

				RXA_TYPE(frm, 1) = RXT_HANDLE;
				RXA_HANDLE(frm,1) = ctx;
			} 

			return RXR_VALUE;
		}

		case CMD_CORE_AES:
		{
			AES_CTX *ctx;
			REBSER *data, key;
			REBYTE *dataBuffer, *pad_data = NULL;
			REBINT len, pad_len;

			if (RXA_TYPE(frm, 5) == RXT_HANDLE) {
				REBSER *binaryOut;
				REBYTE *binaryOutBuffer;

				//set current context
				ctx = (AES_CTX*)RXA_HANDLE(frm,5);

				if (RXA_TYPE(frm, 6) == RXT_NONE) {
					//destroy context
					OS_Free(ctx);
					RXA_LOGIC(frm, 1) = TRUE;
					RXA_TYPE(frm,1) = RXT_LOGIC;
					return RXR_VALUE;
				}

				//get data
				data = RXA_SERIES(frm, 6);
				dataBuffer = (REBYTE *)RL_SERIES(data, RXI_SER_DATA) + RXA_INDEX(frm, 6);
				len = RL_SERIES(data, RXI_SER_TAIL) - RXA_INDEX(frm, 6);
				
				if (len == 0) return RXT_NONE;

				//calculate padded length
				pad_len = (((len - 1) >> 4) << 4) + AES_BLOCKSIZE;

				if (len < pad_len)
				{
					//make new data input with zero-padding
					pad_data = (REBYTE *)OS_Make(pad_len);
					memset(pad_data, 0, pad_len);
					memcpy(pad_data, dataBuffer, len);
					dataBuffer = pad_data;
				}

				//allocate new binary! for output
				binaryOut = (REBSER*)RL_Make_String(pad_len, FALSE);
				binaryOutBuffer =(REBYTE *)RL_SERIES(binaryOut, RXI_SER_DATA);
				memset(binaryOutBuffer, 0, pad_len);

				if (ctx->key_mode == AES_MODE_DECRYPT) // check the key mode
				{
					AES_cbc_decrypt(
						ctx,
						(const uint8_t *)dataBuffer, 
						binaryOutBuffer, pad_len
					);
				} else {
					AES_cbc_encrypt(
						ctx,
						(const uint8_t *)dataBuffer, 
						binaryOutBuffer, pad_len
					);
				}

				if (pad_data) OS_Free(pad_data);
		
				//hack! - will set the tail to buffersize
				*((REBCNT*)(binaryOut+1)) = pad_len;
		
				//setup returned binary! value
				RXA_TYPE(frm, 1) = RXT_BINARY;		
				RXA_SERIES(frm, 1) = binaryOut;
				RXA_INDEX(frm, 1) = 0;

			}
			else if (RXA_TYPE(frm, 2) == RXT_BINARY)
			{
				uint8_t iv[AES_IV_SIZE];

				if (RXA_TYPE(frm, 3) == RXT_BINARY)
				{
					data = RXA_SERIES(frm, 3);
					dataBuffer = (REBYTE *)RL_SERIES(data, RXI_SER_DATA) + RXA_INDEX(frm, 3);

					if ((RL_SERIES(data, RXI_SER_TAIL) - RXA_INDEX(frm, 3)) < AES_IV_SIZE) return RXR_NONE;

					memcpy(iv, dataBuffer, AES_IV_SIZE);
				}
				else
				{
					memset(iv, 0, AES_IV_SIZE);
				}

				//key defined - setup new context
				ctx = (AES_CTX*)OS_Make(sizeof(*ctx));
				memset(ctx, 0, sizeof(*ctx));
				
				key = RXA_SERIES(frm,2);
				len = (RL_SERIES(key, RXI_SER_TAIL) - RXA_INDEX(frm,2)) << 3;

				if (len != 128 && len != 256) return RXR_NONE;

				AES_set_key(
					ctx,
					(const uint8_t *)RL_SERIES(key, RXI_SER_DATA) + RXA_INDEX(frm,2),
					(const uint8_t *)iv,
					(len == 128) ? AES_MODE_128 : AES_MODE_256
				);

				if (RXA_WORD(frm, 7)) // decrypt refinement
					AES_convert_key(ctx);

				RXA_TYPE(frm, 1) = RXT_HANDLE;
				RXA_HANDLE(frm,1) = ctx;
			}

			return RXR_VALUE;
		}

		case CMD_CORE_RSA:
		{
			RXIARG val;
			u32 *words,*w;
			REBCNT type;
			REBSER *data = RXA_SERIES(frm, 1);
			REBYTE *dataBuffer = (REBYTE *)RL_SERIES(data, RXI_SER_DATA) + RXA_INDEX(frm,1);
			REBSER *obj = RXA_OBJECT(frm, 2);
			REBYTE *objData = NULL, *n = NULL, *e = NULL, *d = NULL, *p = NULL, *q = NULL, *dp = NULL, *dq = NULL, *qinv = NULL;
			REBINT data_len = RL_SERIES(data, RXI_SER_TAIL) - RXA_INDEX(frm,1), objData_len = 0, n_len = 0, e_len = 0, d_len = 0, p_len = 0, q_len = 0, dp_len = 0, dq_len = 0, qinv_len = 0;
			REBSER *binary;
			REBINT binary_len;
			REBYTE *binaryBuffer;
			REBOOL padding = TRUE; //PKCS1 is on by default

			BI_CTX *bi_ctx;
			bigint *data_bi;
			RSA_CTX *rsa_ctx = NULL;

			if (RXA_WORD(frm, 5)) { //padding refinement
				padding = (RXA_TYPE(frm, 6) != RXT_NONE);
			}
			
			words = RL_WORDS_OF_OBJECT(obj);
			w = words;
			
			while ((type = RL_GET_FIELD(obj, w[0], &val)))
			{
				if (type == RXT_BINARY){
					objData = (REBYTE *)RL_SERIES(val.series, RXI_SER_DATA) + val.index;
					objData_len = RL_SERIES(val.series, RXI_SER_TAIL) - val.index;
					
					switch(RL_FIND_WORD(core_ext_words,w[0]))
					{
						case W_CORE_N:
							n = objData;
							n_len = objData_len;
							break;
						case W_CORE_E:
							e = objData;
							e_len = objData_len;
							break;
						case W_CORE_D:
							d = objData;
							d_len = objData_len;
							break;
						case W_CORE_P:
							p = objData;
							p_len = objData_len;
							break;
						case W_CORE_Q:
							q = objData;
							q_len = objData_len;
							break;
						case W_CORE_DP:
							dp = objData;
							dp_len = objData_len;
							break;
						case W_CORE_DQ:
							dq = objData;
							dq_len = objData_len;
							break;
						case W_CORE_QINV:
							qinv = objData;
							qinv_len = objData_len;
							break;
					}
				}
				w++;
			}

			if (!n || !e) return RXR_NONE;

			if (RXA_WORD(frm, 4)) // private refinement
			{
				if (!d) return RXR_NONE;
				RSA_priv_key_new(
					&rsa_ctx, n, n_len, e, e_len, d, d_len,
					p, p_len, q, q_len, dp, dp_len, dq, dq_len, qinv, qinv_len
				);
				binary_len = d_len;
			} else {
				RSA_pub_key_new(&rsa_ctx, n, n_len, e, e_len);
				binary_len = n_len;
			}

			bi_ctx = rsa_ctx->bi_ctx;
			data_bi = bi_import(bi_ctx, dataBuffer, data_len);

			//allocate new binary!
			binary = (REBSER*)RL_Make_String(binary_len, FALSE);
			binaryBuffer = (REBYTE *)RL_SERIES(binary, RXI_SER_DATA);

			if (RXA_WORD(frm, 3)) // decrypt refinement
			{

				binary_len = RSA_decrypt(rsa_ctx, dataBuffer, binaryBuffer, RXA_WORD(frm, 4), padding);

				if (binary_len == -1) return RXR_NONE;
			} else {
				if (-1 == RSA_encrypt(rsa_ctx, dataBuffer, data_len, binaryBuffer, RXA_WORD(frm, 4), padding))
					return RXR_NONE;
			}

			//hack! - will set the tail to buffersize
			*((REBCNT*)(binary+1)) = binary_len;
			
			//setup returned binary! value
			RXA_TYPE(frm,1) = RXT_BINARY;			
			RXA_SERIES(frm,1) = binary;
			RXA_INDEX(frm,1) = 0;			
			return RXR_VALUE;
		}
		
		case CMD_CORE_DH_GENERATE_KEY:
		{
			DH_CTX dh_ctx;
			RXIARG val, priv_key, pub_key;
			REBCNT type;
			REBSER *obj = RXA_OBJECT(frm, 1);
			u32 *words = RL_WORDS_OF_OBJECT(obj);
			REBYTE *objData;

			memset(&dh_ctx, 0, sizeof(dh_ctx));

			while ((type = RL_GET_FIELD(obj, words[0], &val)))
			{
				if (type == RXT_BINARY)
				{
					objData = (REBYTE *)RL_SERIES(val.series, RXI_SER_DATA) + val.index;
					
					switch(RL_FIND_WORD(core_ext_words,words[0]))
					{
						case W_CORE_P:
							dh_ctx.p = objData;
							dh_ctx.len = RL_SERIES(val.series, RXI_SER_TAIL) - val.index;
							break;
						case W_CORE_G:
							dh_ctx.g = objData;
							dh_ctx.glen = RL_SERIES(val.series, RXI_SER_TAIL) - val.index;
							break;
					}
				}
				words++;
			}
			
			if (!dh_ctx.p || !dh_ctx.g) break;

			//allocate new binary! blocks for priv/pub keys
			priv_key.series = (REBSER*)RL_Make_String(dh_ctx.len, FALSE);
			priv_key.index = 0;
			dh_ctx.x = (REBYTE *)RL_SERIES(priv_key.series, RXI_SER_DATA);
			memset(dh_ctx.x, 0, dh_ctx.len);
			//hack! - will set the tail to key size
			*((REBCNT*)(((void**)priv_key.series)+1)) = dh_ctx.len;
			
			pub_key.series = (REBSER*)RL_Make_String(dh_ctx.len, FALSE);
			pub_key.index = 0;
			dh_ctx.gx = (REBYTE *)RL_SERIES(pub_key.series, RXI_SER_DATA);
			memset(dh_ctx.gx, 0, dh_ctx.len);
			//hack! - will set the tail to key size
			*((REBCNT*)(((void**)pub_key.series)+1)) = dh_ctx.len;

			//generate keys
			DH_generate_key(&dh_ctx);

			//set the object fields
			RL_Set_Field(obj, core_ext_words[W_CORE_PRIV_KEY], priv_key, RXT_BINARY);	
			RL_Set_Field(obj, core_ext_words[W_CORE_PUB_KEY], pub_key, RXT_BINARY);	
			
			break;
		}

		case CMD_CORE_DH_COMPUTE_KEY:
		{
			DH_CTX dh_ctx;
			RXIARG val;
			REBCNT type;
			REBSER *obj = RXA_OBJECT(frm, 1);
			REBSER *pub_key = RXA_SERIES(frm, 2);
			u32 *words = RL_WORDS_OF_OBJECT(obj);
			REBYTE *objData;
			REBSER *binary;
			REBYTE *binaryBuffer;

			memset(&dh_ctx, 0, sizeof(dh_ctx));

			while ((type = RL_GET_FIELD(obj, words[0], &val)))
			{
				if (type == RXT_BINARY)
				{
					objData = (REBYTE *)RL_SERIES(val.series, RXI_SER_DATA) + val.index;
					
					switch(RL_FIND_WORD(core_ext_words,words[0]))
					{
						case W_CORE_P:
							dh_ctx.p = objData;
							dh_ctx.len = RL_SERIES(val.series, RXI_SER_TAIL) - val.index;
							break;
						case W_CORE_PRIV_KEY:
							dh_ctx.x = objData;
							break;
					}
				}
				words++;
			}
			
			dh_ctx.gy = (REBYTE *)RL_SERIES(pub_key, RXI_SER_DATA) + RXA_INDEX(frm, 2);

			if (!dh_ctx.p || !dh_ctx.x || !dh_ctx.gy) return RXR_NONE;

			//allocate new binary!
			binary = (REBSER*)RL_Make_String(dh_ctx.len, FALSE);
			binaryBuffer = (REBYTE *)RL_SERIES(binary, RXI_SER_DATA);
			memset(binaryBuffer, 0, dh_ctx.len);
			//hack! - will set the tail to buffersize
			*((REBCNT*)(binary+1)) = dh_ctx.len;

			dh_ctx.k = binaryBuffer;

			DH_compute_key(&dh_ctx);

			
			//setup returned binary! value
			RXA_TYPE(frm,1) = RXT_BINARY;			
			RXA_SERIES(frm,1) = binary;
			RXA_INDEX(frm,1) = 0;			
			return RXR_VALUE;
		}

		case CMD_CORE_INIT_WORDS:
		core_ext_words = RL_MAP_WORDS(RXA_SERIES(frm,1));
		break;

		default:
		return RXR_NO_COMMAND;
		}

		return RXR_UNSET;
}

/***********************************************************************
**
*/	void Init_Core_Ext(void)
/*
**	Initialize special variables of the core extension.
**
***********************************************************************/
{
	RL = RL_Extend((REBYTE *)(&RX_core[0]), &RXD_Core);
}
