/*
 * Copyright (c) 2007, Cameron Rich
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *	 this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *	 this list of conditions and the following disclaimer in the documentation
 *	 and/or other materials provided with the distribution.
 * * Neither the name of the axTLS project nor the names of its contributors
 *	 may be used to endorse or promote products derived from this software
 *	 without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>  // uint{8,16,32}_t

/**************************************************************************
 * AES declarations
 **************************************************************************/

#define AES_MAXROUNDS   14
#define AES_BLOCKSIZE   16
#define AES_IV_SIZE     16

typedef enum
{
	AES_MODE_128,
	AES_MODE_256,
	AES_MODE_ENCRYPT,
	AES_MODE_DECRYPT
} AES_MODE;

typedef struct aes_key_st
{
	uint16_t rounds;
	uint16_t key_size;
	uint32_t ks[(AES_MAXROUNDS+1)*8];
	uint8_t iv[AES_IV_SIZE];
	AES_MODE key_mode;
} AES_CTX;

void AES_set_key(AES_CTX *ctx, const uint8_t *key,
		const uint8_t *iv, AES_MODE mode);
void AES_cbc_encrypt(AES_CTX *ctx, const uint8_t *msg,
		uint8_t *out, int length);
void AES_cbc_decrypt(AES_CTX *ks, const uint8_t *in, uint8_t *out, int length);
void AES_convert_key(AES_CTX *ctx);
