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

#define U8C(v) (v##U)
#define U32C(v) (v##U)
#define U8V(v) ((uint8_t)(v) & U8C(0xFF))
#define U32TO8_LE(p, v) \
  do { \
    (p)[0] = U8V((v)); \
    (p)[1] = U8V((v) >>  8); \
    (p)[2] = U8V((v) >> 16); \
    (p)[3] = U8V((v) >> 24); \
  } while (0)

#define U8TO32_LE(p) \
  (((uint32_t)((p)[0])) | \
   ((uint32_t)((p)[1]) <<  8) | \
   ((uint32_t)((p)[2]) << 16) | \
   ((uint32_t)((p)[3]) << 24))

#define ROTL32(v, n) ((v) << (n)) | ((v) >> (32 - (n)))

#define LE(p) (((uint32_t)((p)[0])) | ((uint32_t)((p)[1]) << 8) | ((uint32_t)((p)[2]) << 16) | ((uint32_t)((p)[3]) << 24))
#define FROMLE(b, i) (b)[0] = i & 0xFF; (b)[1] = (i >> 8) & 0xFF; (b)[2] = (i >> 16) & 0xFF; (b)[3] = (i >> 24) & 0xFF;

#define U32V(v) ((uint32_t)(v) & U32C(0xFFFFFFFF))
#define ROTATE(v,c) (ROTL32(v,c))
#define XOR(v,w) ((v) ^ (w))
#define PLUS(v,w) (U32V((v) + (w)))
#define PLUSONE(v) (PLUS((v),1))
#define QUARTERROUND2(a,b,c,d) \
  a = PLUS(a,b); d = ROTATE(XOR(d,a),16); \
  c = PLUS(c,d); b = ROTATE(XOR(b,c),12); \
  a = PLUS(a,b); d = ROTATE(XOR(d,a), 8); \
  c = PLUS(c,d); b = ROTATE(XOR(b,c), 7);


#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct chacha20_ctx
{
  uint32_t schedule[16];
  uint32_t keystream[16];
  size_t available;
  size_t nonce_length; // may be 8 or 12 
};

typedef struct chacha20_ctx chacha20_ctx;

typedef struct chacha20poly1305_ctx {
	chacha20_ctx local_chacha;
	uint64_t     local_sequence;
	uint8_t      local_iv[12];
	chacha20_ctx remote_chacha;
	uint64_t     remote_sequence;
	uint8_t      remote_iv[12];
} chacha20poly1305_ctx;



//Call this to initilize a chacha20_ctx, must be called before all other functions
void chacha20_keysetup(chacha20_ctx *ctx, const uint8_t *key, size_t length);
void chacha20_ivsetup(chacha20_ctx *ctx, const uint8_t *nonce, size_t nonce_length, uint64_t counter, const uint8_t *sequence);


//Call this if you need to process a particular block number
void chacha20_counter_set(chacha20_ctx *ctx, uint64_t counter);

//Raw keystream for the current block, convert output to uint8_t[] for individual bytes. Counter is incremented upon use
void chacha20_block(chacha20_ctx *ctx, uint32_t output[16]);

//Encrypt an arbitrary amount of plaintext, call continuously as needed
void chacha20_encrypt(chacha20_ctx *ctx, const uint8_t *in, uint8_t *out, size_t length);
void chacha_encrypt_bytes(chacha20_ctx *x, const uint8_t *m, uint8_t *c, uint32_t bytes) ;

//Decrypt an arbitrary amount of ciphertext. Actually, for chacha20, decryption is the same function as encryption
void chacha20_decrypt(chacha20_ctx *ctx, const uint8_t *in, uint8_t *out, size_t length);

//Generate POLY1305 key
void chacha20_poly1305_key(struct chacha20_ctx *ctx, unsigned char *poly1305_key);

/* encypts `len` bytes of data from `pt` and appends 16 bytes of poly1305 checksum */
int chacha20_poly1305_aead(struct chacha20_ctx *ctx,  unsigned char *pt, unsigned int len, unsigned char *aad, unsigned int aad_len, unsigned char *poly_key, unsigned char *out);

#ifdef __cplusplus
}
#endif

#endif
