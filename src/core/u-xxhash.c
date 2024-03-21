/*
 * xxHash - Extremely Fast Hash algorithm
 * Copyright (C) 2012-2023 Yann Collet
 *
 * BSD 2-Clause License (https://www.opensource.org/licenses/bsd-license.php)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You can contact the author at:
 *   - xxHash homepage: https://www.xxhash.com
 *   - xxHash source repository: https://github.com/Cyan4973/xxHash
 */

/*
 * xxhash.c instantiates functions defined in xxhash.h
 */

#include "sys-core.h"

#ifdef INCLUDE_XXHASH
#define XXH_STATIC_LINKING_ONLY /* access advanced declarations */
#define XXH_IMPLEMENTATION      /* access definitions */

#include "sys-xxhash.h"

/***********************************************************************
**
*/	REBYTE *HashXXH3(REBYTE *d, REBCNT n, REBYTE *md)
/*
***********************************************************************/
{
	// d is data, n is length
    XXH64_hash_t const hash = XXH3_64bits(d, n);
	XXH64_canonicalFromHash((XXH64_canonical_t *)md, hash);
    return md;
}
/***********************************************************************
**
*/	REBYTE *HashXXH32(REBYTE *d, REBCNT n, REBYTE *md)
/*
***********************************************************************/
{
    XXH32_hash_t const hash = XXH32(d, n, 0);
	XXH32_canonicalFromHash((XXH32_canonical_t *)md, hash);
    return md;
}
/***********************************************************************
**
*/	REBYTE *HashXXH64(REBYTE *d, REBCNT n, REBYTE *md)
/*
***********************************************************************/
{
	// d is data, n is length
    XXH64_hash_t const hash = XXH64(d, n, 0);
	XXH64_canonicalFromHash((XXH64_canonical_t *)md, hash);
    return md;
}
/***********************************************************************
**
*/	REBYTE* HashXXH128(REBYTE* d, REBCNT n, REBYTE* md)
/*
***********************************************************************/
{
	// d is data, n is length
	XXH128_hash_t const hash = XXH128(d, n, 0);
	XXH128_canonicalFromHash((XXH128_canonical_t *)md, hash);
	return md;
}
int  XXH3_CtxSize(void) { return sizeof(XXH3_state_t); }
int  XXH32_CtxSize(void) { return sizeof(XXH32_state_t); }
int  XXH64_CtxSize(void) { return sizeof(XXH64_state_t); }
int  XXH128_CtxSize(void) { return sizeof(XXH3_state_t); }

void XXH3_Starts( XXH3_state_t *ctx )
{
	XXH3_64bits_reset(ctx);
}
void XXH3_Update( XXH3_state_t *ctx,
                         const unsigned char *input,
                         size_t ilen )
{
    XXH3_64bits_update( ctx, input, ilen );
}
void XXH3_Finish(XXH3_state_t*ctx,
                         unsigned char output[8] )
{
	XXH64_hash_t const hash = XXH3_64bits_digest(ctx);
	XXH64_canonicalFromHash((XXH64_canonical_t *)output, hash);
}

void XXH32_Starts( XXH32_state_t *ctx )
{
    ctx = XXH32_createState();
    XXH32_reset(ctx, 0);
}
void XXH32_Update( XXH32_state_t *ctx,
                         const unsigned char *input,
                         size_t ilen )
{
    XXH32_update( ctx, input, ilen );
}
void XXH32_Finish(XXH32_state_t*ctx,
                         unsigned char output[4] )
{
	XXH32_hash_t const hash = XXH32_digest(ctx);
    XXH32_canonicalFromHash((XXH32_canonical_t *)output, hash);
}

void XXH64_Starts( XXH64_state_t *ctx )
{
    ctx = XXH64_createState();
    XXH64_reset(ctx, 0);
}
void XXH64_Update( XXH64_state_t *ctx,
                         const unsigned char *input,
                         size_t ilen )
{
    XXH64_update( ctx, input, ilen );
}
void XXH64_Finish(XXH64_state_t*ctx,
                         unsigned char output[8] )
{
	XXH64_hash_t const hash = XXH64_digest(ctx);
    XXH64_canonicalFromHash((XXH64_canonical_t *)output, hash);
}

void XXH128_Starts( XXH3_state_t *ctx )
{
    XXH3_128bits_reset(ctx);
}
void XXH128_Update( XXH3_state_t *ctx,
                         const unsigned char *input,
                         size_t ilen )
{
    XXH3_128bits_update( ctx, input, ilen );
}
void XXH128_Finish(XXH3_state_t*ctx,
                         unsigned char output[16] )
{
	XXH128_hash_t const hash = XXH3_128bits_digest(ctx);
    XXH128_canonicalFromHash((XXH128_canonical_t*)output, hash);
}

#endif
