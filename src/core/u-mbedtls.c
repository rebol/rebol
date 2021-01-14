/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2021 Rebol Open Source Developers
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
**  Module:  u-mbedtls.c
**  Summary: Rebol wrappers around the Mbed TLS library
**  Section: utility
**  Author:  Oldes
**  Version: 0.1.0
**  History: 13-01-2021 Initial version with checksums
**
***********************************************************************/

#include "sys-core.h"
#ifdef INCLUDE_MBEDTLS
#ifdef MBEDTLS_MD4_C
#include "mbedtls/md4.h"
#endif
#include "mbedtls/md5.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"
#ifdef MBEDTLS_RIPEMD160_C
#include "mbedtls/ripemd160.h"
#endif


#ifdef MBEDTLS_MD4_C
/***********************************************************************
**
*/	REBYTE *MD4(REBYTE *d, REBCNT n, REBYTE *md)
/*
***********************************************************************/
{
	// d is data, n is length
	mbedtls_md4_context c;
	static unsigned char m[16];

	if (md == NULL) md=m;
	mbedtls_md4_starts_ret(&c);
	mbedtls_md4_update_ret(&c,d,n);
	mbedtls_md4_finish_ret(&c, md);
	CLEARS(&c);
	return(md);
}
int  MD4_CtxSize(void) {return sizeof(mbedtls_md4_context);}
void MD4_Starts( mbedtls_md4_context *ctx )
{
    mbedtls_md4_starts_ret( ctx );
}
void MD4_Update( mbedtls_md4_context *ctx,
                         const unsigned char *input,
                         size_t ilen )
{
    mbedtls_md4_update_ret( ctx, input, ilen );
}
void MD4_Finish( mbedtls_md4_context *ctx,
                         unsigned char output[16] )
{
    mbedtls_md4_finish_ret( ctx, output );
}
#endif


/***********************************************************************
**
*/	REBYTE *MD5(REBYTE *d, REBCNT n, REBYTE *md)
/*
***********************************************************************/
{
	// d is data, n is length
	mbedtls_md5_context c;
	static unsigned char m[16];

	if (md == NULL) md=m;
	mbedtls_md5_starts_ret(&c);
	mbedtls_md5_update_ret(&c,d,n);
	mbedtls_md5_finish_ret(&c, md);
	CLEARS(&c);
	return(md);
}
int  MD5_CtxSize(void) { return sizeof(mbedtls_md5_context); }
void MD5_Starts( mbedtls_md5_context *ctx )
{
    mbedtls_md5_starts_ret( ctx );
}
void MD5_Update( mbedtls_md5_context *ctx,
                         const unsigned char *input,
                         size_t ilen )
{
    mbedtls_md5_update_ret( ctx, input, ilen );
}
void MD5_Finish( mbedtls_md5_context *ctx,
                         unsigned char output[16] )
{
    mbedtls_md5_finish_ret( ctx, output );
}


/***********************************************************************
**
*/	REBYTE *SHA1(REBYTE *d, REBCNT n, REBYTE *md)
/*
***********************************************************************/
{
	// d is data, n is length
	mbedtls_sha1_context c;
	static unsigned char m[20];

	if (md == NULL) md=m;
	mbedtls_sha1_starts_ret(&c);
	mbedtls_sha1_update_ret(&c,d,n);
	mbedtls_sha1_finish_ret(&c, md);
	CLEARS(&c);
	return(md);
}
int  SHA1_CtxSize(void) { return sizeof(mbedtls_sha1_context); }
void SHA1_Starts( mbedtls_sha1_context *ctx )
{
    mbedtls_sha1_starts_ret( ctx );
}
void SHA1_Update( mbedtls_sha1_context *ctx,
                         const unsigned char *input,
                         size_t ilen )
{
    mbedtls_sha1_update_ret( ctx, input, ilen );
}
void SHA1_Finish( mbedtls_sha1_context *ctx,
                         unsigned char output[16] )
{
    mbedtls_sha1_finish_ret( ctx, output );
}


/***********************************************************************
**
*/	REBYTE *SHA224(REBYTE *d, REBCNT n, REBYTE *md)
/*
***********************************************************************/
{
	// d is data, n is length
	mbedtls_sha256_context c;
	static unsigned char m[28];

	if (md == NULL) md=m;
	mbedtls_sha256_starts_ret(&c, 1);
	mbedtls_sha256_update_ret(&c,d,n);
	mbedtls_sha256_finish_ret(&c, md);
	CLEARS(&c);
	return(md);
}
void SHA224_Starts( mbedtls_sha256_context *ctx)
{
    mbedtls_sha256_starts_ret( ctx, 1 );
}


/***********************************************************************
**
*/	REBYTE *SHA256(REBYTE *d, REBCNT n, REBYTE *md)
/*
***********************************************************************/
{
	// d is data, n is length
	mbedtls_sha256_context c;
	static unsigned char m[32];

	if (md == NULL) md=m;
	mbedtls_sha256_starts_ret(&c, 0);
	mbedtls_sha256_update_ret(&c,d,n);
	mbedtls_sha256_finish_ret(&c, md);
	CLEARS(&c);
	return(md);
}
int  SHA256_CtxSize(void) { return sizeof(mbedtls_sha256_context); }
void SHA256_Starts( mbedtls_sha256_context *ctx )
{
    mbedtls_sha256_starts_ret( ctx, 0 );
}
void SHA256_Update( mbedtls_sha256_context *ctx,
                         const unsigned char *input,
                         size_t ilen )
{
    mbedtls_sha256_update_ret( ctx, input, ilen );
}
void SHA256_Finish( mbedtls_sha256_context *ctx,
                         unsigned char output[16] )
{
    mbedtls_sha256_finish_ret( ctx, output );
}


/***********************************************************************
**
*/	REBYTE *SHA384(REBYTE *d, REBCNT n, REBYTE *md)
/*
***********************************************************************/
{
	// d is data, n is length
	mbedtls_sha512_context c;
	static unsigned char m[64];

	if (md == NULL) md=m;
	mbedtls_sha512_starts_ret(&c, 1);
	mbedtls_sha512_update_ret(&c,d,n);
	mbedtls_sha512_finish_ret(&c, md);
	CLEARS(&c);
	return(md);
}
void SHA384_Starts( mbedtls_sha512_context *ctx )
{
    mbedtls_sha512_starts_ret( ctx, 1 );
}


/***********************************************************************
**
*/	REBYTE *SHA512(REBYTE *d, REBCNT n, REBYTE *md)
/*
***********************************************************************/
{
	// d is data, n is length
	mbedtls_sha512_context c;
	static unsigned char m[64];

	if (md == NULL) md=m;
	mbedtls_sha512_starts_ret(&c, 0);
	mbedtls_sha512_update_ret(&c,d,n);
	mbedtls_sha512_finish_ret(&c, md);
	CLEARS(&c);
	return(md);
}
int  SHA512_CtxSize(void) { return sizeof(mbedtls_sha512_context); }
void SHA512_Starts( mbedtls_sha512_context *ctx )
{
    mbedtls_sha512_starts_ret( ctx, 0 );
}
void SHA512_Update( mbedtls_sha512_context *ctx,
                         const unsigned char *input,
                         size_t ilen )
{
    mbedtls_sha512_update_ret( ctx, input, ilen );
}
void SHA512_Finish( mbedtls_sha512_context *ctx,
                         unsigned char output[16] )
{
    mbedtls_sha512_finish_ret( ctx, output );
}


#ifdef MBEDTLS_RIPEMD160_C
/***********************************************************************
**
*/	REBYTE *RIPEMD160(REBYTE *d, REBCNT n, REBYTE *md)
/*
***********************************************************************/
{
	// d is data, n is length
	mbedtls_ripemd160_context c;
	static unsigned char m[20];

	if (md == NULL) md=m;
	mbedtls_ripemd160_starts_ret(&c);
	mbedtls_ripemd160_update_ret(&c,d,n);
	mbedtls_ripemd160_finish_ret(&c, md);
	CLEARS(&c);
	return(md);
}
int  RIPEMD160_CtxSize(void) {return sizeof(mbedtls_ripemd160_context);}
void RIPEMD160_Starts( mbedtls_ripemd160_context *ctx )
{
    mbedtls_ripemd160_starts_ret( ctx);
}
void RIPEMD160_Update( mbedtls_ripemd160_context *ctx,
                         const unsigned char *input,
                         size_t ilen )
{
    mbedtls_ripemd160_update_ret( ctx, input, ilen );
}
void RIPEMD160_Finish( mbedtls_ripemd160_context *ctx,
                         unsigned char output[16] )
{
    mbedtls_ripemd160_finish_ret( ctx, output );
}
#endif
#endif //INCLUDE_MBEDTLS

