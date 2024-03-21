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
*/

#ifndef CHACHA20_SIMPLE_H
#define CHACHA20_SIMPLE_H
#include <stdint.h>
#include "reb-c.h"


#define ROTL32(v, n) ((v) << (n)) | ((v) >> (32 - (n)))


#define U32V(v) ((u32)(v) & U32_C(0xFFFFFFFF))
#define ROTATE(v,c) (ROTL32(v,c))
#define XOR(v,w) ((v) ^ (w))
#define PLUS(v,w) (U32V((v) + (w)))
#define PLUSONE(v) (PLUS((v),1))


#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct chacha20_ctx
{
	u32 schedule[16];
	u32 keystream[16];
	size_t available;
	size_t nonce_length; // may be 8 or 12 
};

typedef struct chacha20_ctx chacha20_ctx;

typedef struct chacha20poly1305_ctx {
	chacha20_ctx local_chacha;
	u8      local_iv[12];
	chacha20_ctx remote_chacha;
	u8      remote_iv[12];
} chacha20poly1305_ctx;



//Call this to initilize a chacha20_ctx, must be called before all other functions
void chacha20_keysetup(chacha20_ctx *ctx, const u8 *key, size_t length);
void chacha20_ivsetup(chacha20_ctx *ctx, const u8 *nonce, size_t nonce_length, u64 counter, const u8 *sequence);


//Call this if you need to process a particular block number
void chacha20_counter_set(chacha20_ctx *ctx, u64 counter);

//Raw keystream for the current block, convert output to u8[] for individual bytes. Counter is incremented upon use
void chacha20_block(chacha20_ctx *ctx, u32 output[16]);

//Encrypt an arbitrary amount of plaintext, call continuously as needed
void chacha20_encrypt(chacha20_ctx *ctx, const u8 *in, u8 *out, size_t length);
void chacha_encrypt_bytes(chacha20_ctx *x, const u8 *m, u8 *c, u32 bytes) ;

//Decrypt an arbitrary amount of ciphertext. Actually, for chacha20, decryption is the same function as encryption
void chacha20_decrypt(chacha20_ctx *ctx, const u8 *in, u8 *out, size_t length);

//Generate POLY1305 key
void chacha20_poly1305_key(struct chacha20_ctx *ctx, unsigned char *poly1305_key);

/* encypts `len` bytes of data from `pt` and appends 16 bytes of poly1305 checksum */
int chacha20_poly1305_aead(struct chacha20_ctx *ctx,  unsigned char *pt, unsigned int len, unsigned char *aad, unsigned int aad_len, unsigned char *poly_key, unsigned char *out);

#ifdef __cplusplus
}
#endif

#endif
