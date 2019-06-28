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

#include <string.h>
#include "sys-chacha20.h"
#include "sys-poly1305.h"

void chacha20_keysetup(chacha20_ctx *ctx, const uint8_t *key, size_t length)
{
	const char *constants = (length == 32) ? "expand 32-byte k" : "expand 16-byte k";

	ctx->schedule[0] = LE(constants + 0);
	ctx->schedule[1] = LE(constants + 4);
	ctx->schedule[2] = LE(constants + 8);
	ctx->schedule[3] = LE(constants + 12);
	ctx->schedule[4] = LE(key + 0);
	ctx->schedule[5] = LE(key + 4);
	ctx->schedule[6] = LE(key + 8);
	ctx->schedule[7] = LE(key + 12);
	ctx->schedule[8] = LE(key + 16 % length);
	ctx->schedule[9] = LE(key + 20 % length);
	ctx->schedule[10] = LE(key + 24 % length);
	ctx->schedule[11] = LE(key + 28 % length);

	ctx->available = 0;
}

void chacha20_ivsetup(chacha20_ctx *ctx, const uint8_t *nonce, size_t nonce_length, uint64_t counter, const uint8_t *sequence)
{
	ctx->nonce_length = nonce_length;
	ctx->available = 0;
	
	if(nonce_length == 12) {
		//IETF version as deffined in the rfc7539 - 96bit nonce
		ctx->schedule[12] = counter & UINT16_C(0xFFFF);
		if(nonce) {
			ctx->schedule[13] = LE(nonce+0);
			ctx->schedule[14] = LE(nonce+4) ^ LE(sequence + 0);
			ctx->schedule[15] = LE(nonce+8) ^ LE(sequence + 4); 
		}
	} else if(nonce_length == 8) {
		ctx->schedule[12] = counter & UINT32_C(0xFFFFFFFF);
		ctx->schedule[13] = counter >> 32;
		if(nonce) {
			ctx->schedule[14] = LE(nonce+0) ^ LE(sequence + 0);
			ctx->schedule[15] = LE(nonce+4) ^ LE(sequence + 4);
		}
	} else {
		// report error?
	}
	//puts("chacha20_ivsetup1:"); Dump_Bytes((uint8_t *)ctx->schedule, 64);
}

void chacha20_counter_set(chacha20_ctx *ctx, uint64_t counter)
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

void chacha20_block(chacha20_ctx *ctx, uint32_t output[16])
{
  uint32_t *const nonce = ctx->schedule+12; //12 is where the 128 bit counter is
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
    uint32_t result = output[i] + ctx->schedule[i];
    FROMLE((uint8_t *)(output+i), result);
  }

  /*
  Official specs calls for performing a 64 bit increment here, and limit usage to 2^64 blocks.
  However, recommendations for CTR mode in various papers recommend including the nonce component for a 128 bit increment.
  This implementation will remain compatible with the official up to 2^64 blocks, and past that point, the official is not intended to be used.
  This implementation with this change also allows this algorithm to become compatible for a Fortuna-like construct.
  */
  if (!++nonce[0] && !++nonce[1] && !++nonce[2]) { ++nonce[3]; }
}

static inline void chacha20_xor(uint8_t *keystream, const uint8_t **in, uint8_t **out, size_t length)
{
  uint8_t *end_keystream = keystream + length;
  do { *(*out)++ = *(*in)++ ^ *keystream++; } while (keystream < end_keystream);
}

#ifdef no_use
void chacha_encrypt_bytes(chacha20_ctx *x, const uint8_t *m, uint8_t *c, uint32_t bytes) {
    uint32_t x0, x1, x2, x3, x4, x5, x6, x7;
    uint32_t x8, x9, x10, x11, x12, x13, x14, x15;
    uint32_t j0, j1, j2, j3, j4, j5, j6, j7;
    uint32_t j8, j9, j10, j11, j12, j13, j14, j15;
    uint8_t *ctarget = NULL;
    uint8_t tmp[64];
    uint32_t i;
	
	//puts("chacha20_ctx:");Dump_Bytes((uint8_t*)x, sizeof(chacha20_ctx));

    if (!bytes)
        return;

    j0 = x->schedule[0];
    j1 = x->schedule[1];
    j2 = x->schedule[2];
    j3 = x->schedule[3];
    j4 = x->schedule[4];
    j5 = x->schedule[5];
    j6 = x->schedule[6];
    j7 = x->schedule[7];
    j8 = x->schedule[8];
    j9 = x->schedule[9];
    j10 = x->schedule[10];
    j11 = x->schedule[11];
    j12 = x->schedule[12];
    j13 = x->schedule[13];
    j14 = x->schedule[14];
    j15 = x->schedule[15];

    for (;;) {
        if (bytes < 64) {
            for (i = 0; i < bytes; ++i)
                tmp[i] = m[i];
            m = tmp;
            ctarget = c;
            c = tmp;
        }
        x0 = j0;
        x1 = j1;
        x2 = j2;
        x3 = j3;
        x4 = j4;
        x5 = j5;
        x6 = j6;
        x7 = j7;
        x8 = j8;
        x9 = j9;
        x10 = j10;
        x11 = j11;
        x12 = j12;
        x13 = j13;
        x14 = j14;
        x15 = j15;
        for (i = 20; i > 0; i -= 2) {
            QUARTERROUND2(x0, x4, x8, x12)
            QUARTERROUND2(x1, x5, x9, x13)
            QUARTERROUND2(x2, x6, x10, x14)
            QUARTERROUND2(x3, x7, x11, x15)
            QUARTERROUND2(x0, x5, x10, x15)
            QUARTERROUND2(x1, x6, x11, x12)
            QUARTERROUND2(x2, x7, x8, x13)
            QUARTERROUND2(x3, x4, x9, x14)
        }
        x0 = PLUS(x0, j0);
        x1 = PLUS(x1, j1);
        x2 = PLUS(x2, j2);
        x3 = PLUS(x3, j3);
        x4 = PLUS(x4, j4);
        x5 = PLUS(x5, j5);
        x6 = PLUS(x6, j6);
        x7 = PLUS(x7, j7);
        x8 = PLUS(x8, j8);
        x9 = PLUS(x9, j9);
        x10 = PLUS(x10, j10);
        x11 = PLUS(x11, j11);
        x12 = PLUS(x12, j12);
        x13 = PLUS(x13, j13);
        x14 = PLUS(x14, j14);
        x15 = PLUS(x15, j15);

        if (bytes < 64) {
            U32TO8_LE(x->keystream + 0, x0);
            U32TO8_LE(x->keystream + 4, x1);
            U32TO8_LE(x->keystream + 8, x2);
            U32TO8_LE(x->keystream + 12, x3);
            U32TO8_LE(x->keystream + 16, x4);
            U32TO8_LE(x->keystream + 20, x5);
            U32TO8_LE(x->keystream + 24, x6);
            U32TO8_LE(x->keystream + 28, x7);
            U32TO8_LE(x->keystream + 32, x8);
            U32TO8_LE(x->keystream + 36, x9);
            U32TO8_LE(x->keystream + 40, x10);
            U32TO8_LE(x->keystream + 44, x11);
            U32TO8_LE(x->keystream + 48, x12);
            U32TO8_LE(x->keystream + 52, x13);
            U32TO8_LE(x->keystream + 56, x14);
            U32TO8_LE(x->keystream + 60, x15);
        }

        x0 = XOR(x0, U8TO32_LE(m + 0));
        x1 = XOR(x1, U8TO32_LE(m + 4));
        x2 = XOR(x2, U8TO32_LE(m + 8));
        x3 = XOR(x3, U8TO32_LE(m + 12));
        x4 = XOR(x4, U8TO32_LE(m + 16));
        x5 = XOR(x5, U8TO32_LE(m + 20));
        x6 = XOR(x6, U8TO32_LE(m + 24));
        x7 = XOR(x7, U8TO32_LE(m + 28));
        x8 = XOR(x8, U8TO32_LE(m + 32));
        x9 = XOR(x9, U8TO32_LE(m + 36));
        x10 = XOR(x10, U8TO32_LE(m + 40));
        x11 = XOR(x11, U8TO32_LE(m + 44));
        x12 = XOR(x12, U8TO32_LE(m + 48));
        x13 = XOR(x13, U8TO32_LE(m + 52));
        x14 = XOR(x14, U8TO32_LE(m + 56));
        x15 = XOR(x15, U8TO32_LE(m + 60));

        j12 = PLUSONE(j12);
        if (!j12) {
            j13 = PLUSONE(j13);
            /*
             * Stopping at 2^70 bytes per nonce is the user's
             * responsibility.
             */
        }

        U32TO8_LE(c + 0, x0);
        U32TO8_LE(c + 4, x1);
        U32TO8_LE(c + 8, x2);
        U32TO8_LE(c + 12, x3);
        U32TO8_LE(c + 16, x4);
        U32TO8_LE(c + 20, x5);
        U32TO8_LE(c + 24, x6);
        U32TO8_LE(c + 28, x7);
        U32TO8_LE(c + 32, x8);
        U32TO8_LE(c + 36, x9);
        U32TO8_LE(c + 40, x10);
        U32TO8_LE(c + 44, x11);
        U32TO8_LE(c + 48, x12);
        U32TO8_LE(c + 52, x13);
        U32TO8_LE(c + 56, x14);
        U32TO8_LE(c + 60, x15);

        if (bytes <= 64) {
            if (bytes < 64) {
                for (i = 0; i < bytes; ++i)
                    ctarget[i] = c[i];
            }
            x->schedule[12] = j12;
            x->schedule[13] = j13;
            x->available = 64 - bytes;
            return;
        }
        bytes -= 64;
        c += 64;
        m += 64;
    }
}
#endif

void chacha20_encrypt(chacha20_ctx *ctx, const uint8_t *in, uint8_t *out, size_t length)
{
  if (length)
  {
    uint8_t *const k = (uint8_t *)ctx->keystream;

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

void chacha20_decrypt(chacha20_ctx *ctx, const uint8_t *in, uint8_t *out, size_t length)
{
  chacha20_encrypt(ctx, in, out, length);
}



// POLY1305 related code:

static inline void chacha_key(chacha20_ctx *x, uint8_t *k) {
    U32TO8_LE(k, x->schedule[4]);
    U32TO8_LE(k + 4, x->schedule[5]);
    U32TO8_LE(k + 8, x->schedule[6]);
    U32TO8_LE(k + 12, x->schedule[7]);

    U32TO8_LE(k + 16, x->schedule[8]);
    U32TO8_LE(k + 20, x->schedule[9]);
    U32TO8_LE(k + 24, x->schedule[10]);
    U32TO8_LE(k + 28, x->schedule[11]);
}

static inline void chacha_nonce(chacha20_ctx *ctx, uint8_t *nonce) {
    U32TO8_LE(nonce + 0, ctx->schedule[13]);
    U32TO8_LE(nonce + 4, ctx->schedule[14]);
    U32TO8_LE(nonce + 8, ctx->schedule[15]);
}

static inline int poly1305_generate_key(unsigned char *key256, unsigned char *nonce, unsigned int noncelen, unsigned char *poly_key, unsigned int counter) {
	struct chacha20_ctx ctx;
	uint64_t ctr;
	uint64_t sequence = 0;
	uint32_t state[16];
	int i = 10;

	chacha20_keysetup(&ctx, key256, 32);
	chacha20_ivsetup(&ctx, nonce, noncelen, counter, (uint8_t *)&sequence);

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
		uint32_t result = state[i] + ctx.schedule[i];
		FROMLE((uint8_t *)(poly_key+(i*4)), result);
	}
    return 0;
}

void chacha20_poly1305_key(struct chacha20_ctx *ctx, unsigned char *poly1305_key) {
    unsigned char key[32];
    unsigned char nonce[12];
    chacha_key(ctx, key);
    chacha_nonce(ctx, nonce);
	//puts("key:"); Dump_Bytes(key, 32);
	//puts("nonce:"); Dump_Bytes(nonce, 12);
    poly1305_generate_key(key, nonce, sizeof(nonce), poly1305_key, 0);
}

/* encypts `len` bytes of data from `pt` and appends 16 bytes of poly1305 checksum */
int chacha20_poly1305_aead(struct chacha20_ctx *ctx,  unsigned char *pt, unsigned int len, unsigned char *aad, unsigned int aad_len, unsigned char *poly_key, unsigned char *out) {
    static unsigned char zeropad[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char trail[16];
	int rem;
    if (aad_len > POLY1305_MAX_AAD)
        return -1;

	unsigned int counter = 1;
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

	 U32TO8_LE(&trail[0], aad_len);
    *(int *)&trail[4] = 0;
	 U32TO8_LE(&trail[8], len);
    *(int *)&trail[12] = 0;

	poly1305_update(&aead_ctx, trail, 16);
	poly1305_finish(&aead_ctx, out + len);
    
    return len + POLY1305_TAGLEN;
}
