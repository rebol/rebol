/***********************************************************************
**
**  Hash Function Externs
**
***********************************************************************/

#ifdef INCLUDE_MBEDTLS

#include "mbedtls/ripemd160.h"
#include "mbedtls/md4.h"
#include "mbedtls/md5.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"

#ifdef INCLUDE_SHA224
#define SHA224_Update  SHA256_Update
#define SHA224_Finish  SHA256_Finish
#define SHA224_CtxSize SHA256_CtxSize
#endif

#ifdef INCLUDE_SHA384
#define SHA384_Update  SHA512_Update
#define SHA384_Finish  SHA512_Finish
#define SHA384_CtxSize SHA512_CtxSize
#endif

#endif

// following are compatible between old implementations and mbedtls

#ifdef INCLUDE_MD4
REBYTE *MD4(REBYTE *, REBCNT, REBYTE *);
void MD4_Starts(void *c);
void MD4_Update(void *c, REBYTE *data, REBCNT len);
void MD4_Finish(void *c, REBYTE *md);
int  MD4_CtxSize(void);
#endif

REBYTE *MD5(REBYTE *, REBCNT, REBYTE *);
void MD5_Starts(void *c);
void MD5_Update(void *c, REBYTE *data, REBCNT len);
void MD5_Finish(void *c, REBYTE *md);
int  MD5_CtxSize(void);

REBYTE *SHA1(REBYTE *, REBCNT, REBYTE *);
void SHA1_Starts(void *c);
void SHA1_Update(void *c, REBYTE *data, REBCNT len);
void SHA1_Finish(void *c, REBYTE *md);
int  SHA1_CtxSize(void);

#ifdef INCLUDE_SHA224
REBYTE *SHA224(REBYTE *, REBCNT, REBYTE *);
void SHA224_Starts(void *c);
void SHA224_Update(void *c, REBYTE *data, REBCNT len);
void SHA224_Finish(void *c, REBYTE *md);
int  SHA224_CtxSize(void);
#endif

REBYTE *SHA256(REBYTE *, REBCNT, REBYTE *);
void SHA256_Starts(void *c);
void SHA256_Update(void *c, REBYTE *data, REBCNT len);
void SHA256_Finish(void *c, REBYTE *md);
int  SHA256_CtxSize(void);

#ifdef INCLUDE_SHA384
REBYTE *SHA384(REBYTE *, REBCNT, REBYTE *);
void SHA384_Starts(void *c);
void SHA384_Update(void *c, REBYTE *data, REBCNT len);
void SHA384_Finish(void *c, REBYTE *md);
int  SHA384_CtxSize(void);
#endif

REBYTE *SHA512(REBYTE *, REBCNT, REBYTE *);
void SHA512_Starts(void *c);
void SHA512_Update(void *c, REBYTE *data, REBCNT len);
void SHA512_Finish(void *c, REBYTE *md);
int  SHA512_CtxSize(void);

#ifdef INCLUDE_RIPEMD160
REBYTE *RIPEMD160(REBYTE *, REBCNT, REBYTE *);
void RIPEMD160_Starts(void *c);
void RIPEMD160_Update(void *c, REBYTE *data, REBCNT len);
void RIPEMD160_Finish(void *c, REBYTE *md);
int  RIPEMD160_CtxSize(void);
#endif

#ifdef INCLUDE_SHA3
void SHA3_Update(void* c, REBYTE* data, REBCNT len);
int  SHA3_CtxSize(void);
REBYTE* SHA3_224(REBYTE*, REBCNT, REBYTE*);
void SHA3_224_Starts(void* c);
void SHA3_256_Starts(void* c);
void SHA3_384_Starts(void* c);
void SHA3_512_Starts(void* c);
void SHA3_224_Finish(void* c, REBYTE* md);
void SHA3_256_Finish(void* c, REBYTE* md);
void SHA3_384_Finish(void* c, REBYTE* md);
void SHA3_512_Finish(void* c, REBYTE* md);
#endif

#ifdef INCLUDE_XXHASH
#define XXH_STATIC_LINKING_ONLY 
#include "sys-xxhash.h"

#define XXH3_CTX     XXH3_state_t
#define XXH32_CTX    XXH32_state_t
#define XXH64_CTX    XXH64_state_t
#define XXH128_CTX   XXH3_state_t
REBYTE* HashXXH3(REBYTE*, REBCNT, REBYTE*);
REBYTE* HashXXH32(REBYTE*, REBCNT, REBYTE*);
REBYTE* HashXXH64(REBYTE*, REBCNT, REBYTE*);
REBYTE* HashXXH128(REBYTE*, REBCNT, REBYTE*);

void XXH3_Starts(void* c);
void XXH3_Update(void* c, REBYTE* data, REBCNT len);
void XXH3_Finish(void* c, REBYTE* md);
int  XXH3_CtxSize(void);

void XXH32_Starts(void* c);
void XXH32_Update(void* c, REBYTE* data, REBCNT len);
void XXH32_Finish(void* c, REBYTE* md);
int  XXH32_CtxSize(void);

void XXH64_Starts(void* c);
void XXH64_Update(void* c, REBYTE* data, REBCNT len);
void XXH64_Finish(void* c, REBYTE* md);
int  XXH64_CtxSize(void);

void XXH128_Starts(void* c);
void XXH128_Update(void* c, REBYTE* data, REBCNT len);
void XXH128_Finish(void* c, REBYTE* md);
int  XXH128_CtxSize(void);

#endif