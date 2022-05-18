/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2022 Rebol Open Source Contributors
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


#ifdef MBEDTLS_CAMELLIA_C
#include "mbedtls/camellia.h"
#endif

#ifdef MBEDTLS_ARIA_C
#include "mbedtls/aria.h"
#endif

#ifdef MBEDTLS_CHACHA20_C
#include "mbedtls/chacha20.h"
#endif

#ifdef MBEDTLS_CHACHAPOLY_C
#include "mbedtls/chachapoly.h"
typedef mbedtls_chachapoly_context CHACHAPOLY_CTX;
#define CHACHAPOLY_STATE_AAD (1)
#define CHACHAPOLY_STATE_CIPHERTEXT (2)
#endif

#ifdef MBEDTLS_CCM_C
#include "mbedtls/ccm.h"
#endif

#ifdef MBEDTLS_GCM_C
#include "mbedtls/gcm.h"
#endif


typedef mbedtls_rsa_context   RSA_CTX;
typedef mbedtls_dhm_context   DHM_CTX;
typedef mbedtls_ecdh_context ECDH_CTX;
//typedef mbedtls_aes_context   AES_CTX;

typedef enum {
	CRYPT_PORT_CLOSED = 0,
	CRYPT_PORT_NEEDS_INIT,
	CRYPT_PORT_NO_DATA,
	CRYPT_PORT_HAS_DATA,
	CRYPT_PORT_READY,
	CRYPT_PORT_FINISHED,
} crypt_port_state_t;

typedef enum {
	CRYPT_OK = 0,
	CRYPT_ERROR_BAD_BLOCK_SIZE,
	CRYPT_ERROR_BAD_PROCESSED_SIZE,
	CRYPT_ERROR_BAD_UNPROCESSED_SIZE,
} crypt_port_error_t;

typedef struct crypt_ctx {
	crypt_port_state_t  state;
	mbedtls_operation_t operation;
	REBCNT              cipher_type;
	mbedtls_cipher_mode_t cipher_mode;
	REBCNT              cipher_block_size;
	void                *cipher_ctx;
	REBSER              *buffer;
	REBINT              error;
	unsigned int        key_bitlen;
	unsigned int        IV_len;
	unsigned int        tag_len;
	unsigned int        aad_len;
	unsigned int        unprocessed_len;
	unsigned char       nonce[MBEDTLS_MAX_IV_LENGTH]; // nonce may be changed, like in Camellia cipher!
	unsigned char       IV[MBEDTLS_MAX_IV_LENGTH];
	unsigned char       key[MBEDTLS_MAX_KEY_LENGTH];
	// Block-based ciphers needs full block to process, if the input is not long enough,
	// it is stored in this temp buffer...
	unsigned char       unprocessed_data[MBEDTLS_MAX_BLOCK_LENGTH];
} CRYPT_CTX;

void crypt_context_free(void *ctx);



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

