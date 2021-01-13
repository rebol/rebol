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
**  Summary: Optional build configuration dependency check
**  Module:  opt-dependencies.h
**  Author:  Oldes
**  Notes:
**      This file is used to check user defined options
**
***********************************************************************/

#ifndef REBOL_OPTIONS_H
#define REBOL_OPTIONS_H

//**************************************************************//
//** dependencies... do not modify! ****************************//
#if defined(INCLUDE_MBEDTLS)
#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif
#endif

#if defined(INCLUDE_MD4)
#define MBEDTLS_MD4_C
#else
#undef MBEDTLS_MD4_C
#endif

#if defined(INCLUDE_RIPEMD160)
#define MBEDTLS_RIPEMD160_C
#else
#undef MBEDTLS_RIPEMD160_C
#endif

#endif //REBOL_OPTIONS_H