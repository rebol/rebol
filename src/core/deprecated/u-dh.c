/*
Simple implementation of Diffie-Hellman algorithm (c) 2013 Richard Smolak
The code uses Bigint implementation Copyright (c) 2007, Cameron Rich
*/

#include "sys-core.h"
#include "sys-dh.h"

void DH_generate_key(DH_CTX *dh_ctx)
{
    BI_CTX *bi_ctx = bi_initialize();
	int len = dh_ctx->len;
	bigint *p = bi_import(bi_ctx, dh_ctx->p, len); //p modulus
	bigint *g = bi_import(bi_ctx, dh_ctx->g, dh_ctx->len_g); //generator
	bigint *x, *gx;

	bi_permanent(g);

    //generate private key  X
	Random_Bytes(dh_ctx->x, len, 1);
	x = bi_import(bi_ctx, dh_ctx->x, len);
	bi_permanent(x);

	//calculate public key gx = g^x mod p
	bi_set_mod(bi_ctx, p,  BIGINT_M_OFFSET);
	bi_ctx->mod_offset = BIGINT_M_OFFSET;
	gx = bi_mod_power(bi_ctx, g, x);
	bi_permanent(gx);

	bi_export(bi_ctx, x, dh_ctx->x, len);
	bi_export(bi_ctx, gx, dh_ctx->gx, len);

	bi_depermanent(g);
	bi_depermanent(x);
	bi_depermanent(gx);
    bi_free(bi_ctx, g);
    bi_free(bi_ctx, x);
    bi_free(bi_ctx, gx);

	bi_free_mod(bi_ctx, BIGINT_M_OFFSET);
	bi_terminate(bi_ctx);
}

void DH_compute_key(DH_CTX *dh_ctx)
{
    BI_CTX *bi_ctx = bi_initialize();
	int len = dh_ctx->len;
	bigint *p = bi_import(bi_ctx, dh_ctx->p, len); //p modulus
	bigint *x = bi_import(bi_ctx, dh_ctx->x, len); //private key
	bigint *gy = bi_import(bi_ctx, dh_ctx->gy, len); //public key(peer)
	bigint *k;										//negotiated(session) key

	bi_permanent(x);
	bi_permanent(gy);

	//calculate session key k = gy^x mod p
	bi_set_mod(bi_ctx, p,  BIGINT_M_OFFSET);
	bi_ctx->mod_offset = BIGINT_M_OFFSET;
	k = bi_mod_power(bi_ctx, gy, x);
	bi_permanent(k);

	bi_export(bi_ctx, k, dh_ctx->k, len);

	bi_depermanent(x);
	bi_depermanent(gy);
	bi_depermanent(k);
    bi_free(bi_ctx, x);
    bi_free(bi_ctx, gy);
    bi_free(bi_ctx, k);

	bi_free_mod(bi_ctx, BIGINT_M_OFFSET);
	bi_terminate(bi_ctx);
}
