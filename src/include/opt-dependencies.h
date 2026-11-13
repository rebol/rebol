/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2021 Rebol Open Source Contributors
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
**  Summary: Optional build configuration dependency check
**  Module:  opt-dependencies.h
**  Author:  Oldes
**  Notes:
**      This file is used to validate user defined options
**
***********************************************************************/

#ifndef REBOL_OPTIONS_H
#define REBOL_OPTIONS_H

//**************************************************************//
//** dependency fixes! ****************************************//

#if defined(INCLUDE_MBEDTLS)
	//#include "mbedtls/config.h"

	#define MBEDTLS_HAVE_ASM
	#define MBEDTLS_HAVE_SSE2
	#define MBEDTLS_HAVE_TIME
	#define MBEDTLS_HAVE_TIME_DATE
	#define MBEDTLS_PLATFORM_C

	#define MBEDTLS_PLATFORM_MEMORY

	#ifdef INCLUDE_RSA
	#define MBEDTLS_RSA_C
	#endif

	#define MBEDTLS_CIPHER_C

	#define MBEDTLS_MD_C
	#define MBEDTLS_MD5_C
	#define MBEDTLS_SHA1_C
	#define MBEDTLS_SHA224_C
	#define MBEDTLS_SHA256_C
	#define MBEDTLS_SHA512_C

	#if defined(INCLUDE_MD4)
	#define MBEDTLS_MD4_C
	#endif

	#if defined(INCLUDE_RIPEMD160)
	#define MBEDTLS_RIPEMD160_C
	#endif

	// INCLUDE_SHA224 is now required for SHA256, which is always included!
	//#if defined(INCLUDE_SHA224)
	//#define MBEDTLS_SHA224_C
	//#endif

	#if defined(INCLUDE_SHA384)
	#define MBEDTLS_SHA384_C
	#endif

	#if defined(INCLUDE_SHA3)
	#define MBEDTLS_SHA3_C
	#endif
#else
	// no mbedTLS
	#if defined(INCLUDE_RIPEMD160)
	#undef INCLUDE_RIPEMD160
	#endif

	#if defined(INCLUDE_SHA224)
	#undef INCLUDE_SHA224
	#endif

	#if defined(INCLUDE_MD4)
	#undef INCLUDE_MD4
	#endif
#endif

#endif //REBOL_OPTIONS_H