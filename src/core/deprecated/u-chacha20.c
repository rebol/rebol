#ifndef EXCLUDE_CHACHA20POLY1305
/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2019 Rebol Open Source Developers
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
**  Module:  u-chacha20.c
**  Summary: ChaCha20 cipher
**  Section: utility
**  Author:  Daniel Bernstein (http://cr.yp.to/chacha.html)
**	         Insane coder (http://chacha20.insanecoding.org/)
**           Eduard Suica (https://github.com/eduardsui/tlse)
**           Oldes (code derived from above sources)
**
***********************************************************************/

/*
Copyright (C) 2014 insane coder (http://insanecoding.blogspot.com/, http://chacha20.insanecoding.org/)

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

This implementation is intended to be simple, many optimizations can be performed.
*/
    
/********************************************************************************
 Copyright (c) 2016-2019, Eduard Suica
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or other
 materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************************/

#include <string.h>
#include "reb-c.h"
#include "sys-chacha20.h"
#include "sys-poly1305.h"

void chacha20_keysetup(chacha20_ctx *ctx, const u8 *key, size_t length)
{
	const char *constants = (length == 32) ? "expand 32-byte k" : "expand 16-byte k";

	ctx->schedule[0]  = U8TO32_LE(constants + 0);
	ctx->schedule[1]  = U8TO32_LE(constants + 4);
	ctx->schedule[2]  = U8TO32_LE(constants + 8);
	ctx->schedule[3]  = U8TO32_LE(constants + 12);
	ctx->schedule[4]  = U8TO32_LE(key + 0);
	ctx->schedule[5]  = U8TO32_LE(key + 4);
	ctx->schedule[6]  = U8TO32_LE(key + 8);
	ctx->schedule[7]  = U8TO32_LE(key + 12);
	ctx->schedule[8]  = U8TO32_LE(key + 16 % length);
	ctx->schedule[9]  = U8TO32_LE(key + 20 % length);
	ctx->schedule[10] = U8TO32_LE(key + 24 % length);
	ctx->schedule[11] = U8TO32_LE(key + 28 % length);

	ctx->available = 0;
}

void chacha20_ivsetup(chacha20_ctx *ctx, const u8 *nonce, size_t nonce_length, u64 counter, const u8 *sequence)
{
	ctx->nonce_length = nonce_length;
	ctx->available = 0;
	
	if(nonce_length == 12) {
		//IETF version as deffined in the rfc7539 - 96bit nonce
		ctx->schedule[12] = counter & UINT16_C(0xFFFF);
		if(nonce) {
			ctx->schedule[13] = U8TO32_LE(nonce+0);
			ctx->schedule[14] = U8TO32_LE(nonce+4) ^ U8TO32_LE(sequence + 0);
			ctx->schedule[15] = U8TO32_LE(nonce+8) ^ U8TO32_LE(sequence + 4); 
		}
	} else if(nonce_length == 8) {
		ctx->schedule[12] = counter & UINT32_C(0xFFFFFFFF);
		ctx->schedule[13] = counter >> 32;
		if(nonce) {
			ctx->schedule[14] = U8TO32_LE(nonce+0) ^ U8TO32_LE(sequence + 0);
			ctx->schedule[15] = U8TO32_LE(nonce+4) ^ U8TO32_LE(sequence + 4);
		}
	} else {
		// report error?
	}
	//puts("chacha20_ivsetup1:"); Dump_Bytes((u8 *)ctx->schedule, 64);
}

void chacha20_counter_set(chacha20_ctx *ctx, u64 counter)
{
	if (ctx->nonce_length == 12) {
		ctx->schedule[12] = counter & UINT16_C(0xFFFF);
	}
	else {
		ctx->schedule[12] = counter & UINT32_C(0xFFFFFFFF);
		ctx->schedule[13] = counter >> 32;
	}
	ctx->available = 0;
}

#define QUARTERROUND(x, a, b, c, d) \
	x[a] += x[b]; x[d] = ROTL32(x[d] ^ x[a], 16); \
	x[c] += x[d]; x[b] = ROTL32(x[b] ^ x[c], 12); \
	x[a] += x[b]; x[d] = ROTL32(x[d] ^ x[a], 8); \
	x[c] += x[d]; x[b] = ROTL32(x[b] ^ x[c], 7);

void chacha20_block(chacha20_ctx *ctx, u32 output[16])
{
  u32 *const nonce = ctx->schedule+12; //12 is where the 128 bit counter is
  int i = 10;

  memcpy(output, ctx->schedule, sizeof(ctx->schedule));

  while (i--)
  {
	QUARTERROUND(output, 0, 4, 8, 12)
	QUARTERROUND(output, 1, 5, 9, 13)
	QUARTERROUND(output, 2, 6, 10, 14)
	QUARTERROUND(output, 3, 7, 11, 15)
	QUARTERROUND(output, 0, 5, 10, 15)
	QUARTERROUND(output, 1, 6, 11, 12)
	QUARTERROUND(output, 2, 7, 8, 13)
	QUARTERROUND(output, 3, 4, 9, 14)
  }
  for (i = 0; i < 16; ++i)
  {
	u32 result = output[i] + ctx->schedule[i];
	U32TO8_LE((u8 *)(output+i), result);
  }

  /*
  Official specs calls for performing a 64 bit increment here, and limit usage to 2^64 blocks.
  However, recommendations for CTR mode in various papers recommend including the nonce component for a 128 bit increment.
  This implementation will remain compatible with the official up to 2^64 blocks, and past that point, the official is not intended to be used.
  This implementation with this change also allows this algorithm to become compatible for a Fortuna-like construct.
  */
  if (!++nonce[0] && !++nonce[1] && !++nonce[2]) { ++nonce[3]; }
}

static inline void chacha20_xor(u8 *keystream, const u8 **in, u8 **out, size_t length)
{
u8 *end_keystream = keystream + length;
do { *(*out)++ = *(*in)++ ^ *keystream++; } while (keystream < end_keystream);
}

void chacha20_encrypt(chacha20_ctx *ctx, const u8 *in, u8 *out, size_t length)
{
  if (length)
  {
	u8 *const k = (u8 *)ctx->keystream;

	//puts("chacha20_encrypt  schedule:"); Dump_Bytes(ctx->schedule, 64);
	//puts("chacha20_encrypt keystream:"); Dump_Bytes(ctx->keystream, 64);
	//puts("chacha20_encrypt        in:"); Dump_Bytes(in, length);

	//First, use any buffered keystream from previous calls
	if (ctx->available)
	{
	  size_t amount = MIN(length, ctx->available);
	  chacha20_xor(k + (sizeof(ctx->keystream)-ctx->available), &in, &out, amount);
	  ctx->available -= amount;
	  length -= amount;
	}

	//Then, handle new blocks
	while (length)
	{
	  size_t amount = MIN(length, sizeof(ctx->keystream));
	  chacha20_block(ctx, ctx->keystream);
	  chacha20_xor(k, &in, &out, amount);
	  length -= amount;
	  ctx->available = sizeof(ctx->keystream) - amount;
	}
  }
}

void chacha20_decrypt(chacha20_ctx *ctx, const u8 *in, u8 *out, size_t length)
{
  chacha20_encrypt(ctx, in, out, length);
}



// Chacha20POLY1305 related code:

static inline void chacha_key(chacha20_ctx *x, u8 *k) {
	U32TO8_LE(k +  0, x->schedule[4]);
	U32TO8_LE(k +  4, x->schedule[5]);
	U32TO8_LE(k +  8, x->schedule[6]);
	U32TO8_LE(k + 12, x->schedule[7]);

	U32TO8_LE(k + 16, x->schedule[8]);
	U32TO8_LE(k + 20, x->schedule[9]);
	U32TO8_LE(k + 24, x->schedule[10]);
	U32TO8_LE(k + 28, x->schedule[11]);
}

static inline void chacha_nonce(chacha20_ctx *ctx, u8 *nonce) {
	U32TO8_LE(nonce + 0, ctx->schedule[13]);
	U32TO8_LE(nonce + 4, ctx->schedule[14]);
	U32TO8_LE(nonce + 8, ctx->schedule[15]);
}

static inline int poly1305_generate_key(u8 *key256, u8 *nonce, u32 noncelen, u8 *poly_key, u32 counter) {
	struct chacha20_ctx ctx;
	u64 sequence = 0;
	u32 state[16];
	int i = 10;

	chacha20_keysetup(&ctx, key256, 32);
	chacha20_ivsetup(&ctx, nonce, noncelen, counter, (u8 *)&sequence);

	// this is like chacha20_block, but the result is only 32 bytes!
	memcpy(state, ctx.schedule, sizeof(ctx.schedule));

	while (i--)
	{
		QUARTERROUND(state, 0, 4, 8, 12)
		QUARTERROUND(state, 1, 5, 9, 13)
		QUARTERROUND(state, 2, 6, 10, 14)
		QUARTERROUND(state, 3, 7, 11, 15)
		QUARTERROUND(state, 0, 5, 10, 15)
		QUARTERROUND(state, 1, 6, 11, 12)
		QUARTERROUND(state, 2, 7, 8, 13)
		QUARTERROUND(state, 3, 4, 9, 14)
	}
	for (i = 0; i < 8; ++i) //poly_key has only 32 bytes (8 * 4)
	{
		u32 result = state[i] + ctx.schedule[i];
		U32TO8_LE((u8 *)(poly_key+(i*4)), result);
	}
	return 0;
}

void chacha20_poly1305_key(struct chacha20_ctx *ctx, u8 *poly1305_key) {
	u8 key[32];
	u8 nonce[12];
	chacha_key(ctx, key);
	chacha_nonce(ctx, nonce);
	//puts("key:"); Dump_Bytes(key, 32);
	//puts("nonce:"); Dump_Bytes(nonce, 12);
	poly1305_generate_key(key, nonce, sizeof(nonce), poly1305_key, 0);
}

/* encypts `len` bytes of data from `pt` and appends 16 bytes of poly1305 checksum */
int chacha20_poly1305_aead(struct chacha20_ctx *ctx,  u8 *pt, u32 len, u8 *aad, u32 aad_len, u8 *poly_key, u8 *out) {
	static u8 zeropad[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	u8 trail[16];
	int rem;
	if (aad_len > POLY1305_MAX_AAD)
		return -1;

	u32 counter = 1;
	chacha20_ivsetup(ctx, NULL, 12, counter, zeropad); // updates only counter
	chacha20_encrypt(ctx, pt, out, len); // encrypts data
	
	// and follows 16 bytes of checksum computation
	poly1305_context aead_ctx;
	poly1305_init(&aead_ctx, poly_key);
	poly1305_update(&aead_ctx, aad, aad_len);
	rem = aad_len % 16;
	if (rem)
		poly1305_update(&aead_ctx, zeropad, 16 - rem);
	poly1305_update(&aead_ctx, out, len);
	rem = len % 16;
	if (rem)
		poly1305_update(&aead_ctx, zeropad, 16 - rem);

	U32TO8_LE(&trail[0], (aad_len == 5) ? 5 : 13);
	*(int *)&trail[4] = 0;
	U32TO8_LE(&trail[8], len);
	*(int *)&trail[12] = 0;

	//puts("trail:"); Dump_Bytes(trail, 16);

	poly1305_update(&aead_ctx, trail, 16);
	poly1305_finish(&aead_ctx, out + len);
	
	return len + POLY1305_TAGLEN;
}
#endif // EXCLUDE_CHACHA20POLY1305