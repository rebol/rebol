/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
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
**  Summary: Cryptography
**  Module:  sys-crypt.h
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "mbedtls/rsa.h"
#include "mbedtls/dhm.h"
#include "mbedtls/bignum.h"
#include "mbedtls/sha256.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/asn1.h"

#include "mbedtls/cipher.h"
#include "mbedtls/md.h"
#include "mbedtls/platform_util.h"
//#include "mbedtls/asn1write.h"


typedef mbedtls_rsa_context   RSA_CTX;
typedef mbedtls_dhm_context   DHM_CTX;
typedef mbedtls_ecdh_context ECDH_CTX;

typedef struct crypt_ctx {
	mbedtls_cipher_context_t cipher;
	mbedtls_operation_t operation;
	int key_bitlen;
	unsigned char IV[MBEDTLS_MAX_IV_LENGTH];
	unsigned char key[512];
	unsigned char unprocessed_data[MBEDTLS_MAX_BLOCK_LENGTH];
	REBCNT unprocessed_len;
	REBSER *buffer;
} CRYPT_CTX;

void crypt_context_free(CRYPT_CTX *ctx);


// these 3 functions were defined as static in dhm.c file, so are not in the header!
extern int dhm_check_range(const mbedtls_mpi *param, const mbedtls_mpi *P);
extern int dhm_random_below(mbedtls_mpi *R, const mbedtls_mpi *M,
	int (*f_rng)(void *, unsigned char *, size_t), void *p_rng);
// originally static in ecdsa.c
extern int ecdsa_signature_to_asn1(const mbedtls_mpi *r, const mbedtls_mpi *s,
	unsigned char *sig, size_t sig_size,
	size_t *slen);
extern int ecdsa_verify_restartable(mbedtls_ecp_group *grp,
	const unsigned char *buf, size_t blen,
	const mbedtls_ecp_point *Q,
	const mbedtls_mpi *r, const mbedtls_mpi *s,
	mbedtls_ecdsa_restart_ctx *rs_ctx);

