#ifndef POLY1305_DONNA_H
#define POLY1305_DONNA_H

#include <stddef.h>

#define POLY1305_MAX_AAD 32
#define POLY1305_KEYLEN  32
#define POLY1305_TAGLEN  16

typedef struct poly1305_context {
	size_t aligner;
	unsigned char opaque[136];
} poly1305_context;

void poly1305_init(poly1305_context *ctx, const unsigned char key[POLY1305_KEYLEN]);
void poly1305_update(poly1305_context *ctx, const unsigned char *m, size_t bytes);
void poly1305_finish(poly1305_context *ctx, unsigned char mac[POLY1305_TAGLEN]);
void poly1305_auth(unsigned char mac[POLY1305_TAGLEN], const unsigned char *m, size_t bytes, const unsigned char key[POLY1305_KEYLEN]);

int poly1305_verify(const unsigned char mac1[POLY1305_TAGLEN], const unsigned char mac2[POLY1305_TAGLEN]);
//int poly1305_power_on_self_test(void);

#endif /* POLY1305_DONNA_H */

