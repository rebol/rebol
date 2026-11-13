/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2014 Atronix Engineering, Inc.
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
**  Summary: Integer Datatype Functions
**  Module:  sys-int-funcs.h
**  Notes:
*/

#ifndef __SYS_INT_FUNCS_H_
#define __SYS_INT_FUNCS_H_



#if __has_builtin(__builtin_sadd_overflow) || GCC_VERSION_AT_LEAST(5, 1)
#define	REB_I32_ADD_OF(x, y, sum) __builtin_sadd_overflow((x), (y), (sum))
#else
REBOOL reb_i32_add_overflow(i32 x, i32 y, i32 *sum);
#define	REB_I32_ADD_OF(x, y, sum) reb_i32_add_overflow((x), (y), (sum))
#endif

#if __has_builtin(__builtin_uadd_overflow) || GCC_VERSION_AT_LEAST(5, 1)
#define	REB_U32_ADD_OF(x, y, sum) __builtin_uadd_overflow((x), (y), (sum))
#else
REBOOL reb_u32_add_overflow(u32 x, u32 y, u32 *sum);
#define	REB_U32_ADD_OF(x, y, sum) reb_u32_add_overflow((x), (y), (sum))
#endif

#if __has_builtin(__builtin_saddl_overflow) && __has_builtin(__builtin_saddll_overflow) || GCC_VERSION_AT_LEAST(5, 1)
#ifdef __LP64__
#define	REB_I64_ADD_OF(x, y, sum) __builtin_saddl_overflow((x), (y), (long*)(sum))
#else // presumably __LLP64__ or __LP32__
#define	REB_I64_ADD_OF(x, y, sum) __builtin_saddll_overflow((x), (y), (sum))
#endif //__LP64__
#else
REBOOL reb_i64_add_overflow(i64 x, i64 y, i64 *sum);
#define	REB_I64_ADD_OF(x, y, sum) reb_i64_add_overflow((x), (y), (sum))
#endif

#if __has_builtin(__builtin_uaddl_overflow) && __has_builtin(__builtin_uaddll_overflow) || GCC_VERSION_AT_LEAST(5, 1)
#ifdef __LP64__
#define	REB_U64_ADD_OF(x, y, sum) __builtin_uaddl_overflow((x), (y), (long*)(sum))
#else // presumably __LLP64__ or __LP32__
#define	REB_U64_ADD_OF(x, y, sum) __builtin_uaddll_overflow((x), (y), (sum))
#endif //__LP64__
#else
REBOOL reb_u64_add_overflow(u64 x, u64 y, u64 *sum);
#define	REB_U64_ADD_OF(x, y, sum) reb_u64_add_overflow((x), (y), (sum))
#endif

#if __has_builtin(__builtin_ssub_overflow) || GCC_VERSION_AT_LEAST(5, 1)
#define	REB_I32_SUB_OF(x, y, diff) __builtin_ssub_overflow((x), (y), (diff))
#else
REBOOL reb_i32_sub_overflow(i32 x, i32 y, i32 *diff);
#define	REB_I32_SUB_OF(x, y, diff) reb_i32_sub_overflow((x), (y), (diff))
#endif

#if __has_builtin(__builtin_ssubl_overflow) && __has_builtin(__builtin_ssubll_overflow) || GCC_VERSION_AT_LEAST(5, 1)
#ifdef __LP64__
#define	REB_I64_SUB_OF(x, y, diff) __builtin_ssubl_overflow((x), (y), (long*)(diff))
#else // presumably __LLP64__ or __LP32__
#define	REB_I64_SUB_OF(x, y, diff) __builtin_ssubll_overflow((x), (y), (diff))
#endif //__LP64__
#else
REBOOL reb_i64_sub_overflow(i64 x, i64 y, i64 *diff);
#define	REB_I64_SUB_OF(x, y, diff) reb_i64_sub_overflow((x), (y), (diff))
#endif

#if __has_builtin(__builtin_smul_overflow) || GCC_VERSION_AT_LEAST(5, 1)
#define	REB_I32_MUL_OF(x, y, prod) __builtin_smul_overflow((x), (y), (prod))
#else
REBOOL reb_i32_mul_overflow(i32 x, i32 y, i32 *prod);
#define	REB_I32_MUL_OF(x, y, prod) reb_i32_mul_overflow((x), (y), (prod))
#endif

#if __has_builtin(__builtin_umul_overflow) || GCC_VERSION_AT_LEAST(5, 1)
#define	REB_U32_MUL_OF(x, y, prod) __builtin_umul_overflow((x), (y), (prod))
#else
REBOOL reb_u32_mul_overflow(u32 x, u32 y, u32 *prod);
#define	REB_U32_MUL_OF(x, y, prod) reb_u32_mul_overflow((x), (y), (prod))
#endif

#if __has_builtin(__builtin_smull_overflow) && __has_builtin(__builtin_smulll_overflow) || GCC_VERSION_AT_LEAST(5, 1)
#ifdef __LP64__
#define	REB_I64_MUL_OF(x, y, prod) __builtin_smull_overflow((x), (y), (long*)(prod))
#elif !defined(__clang__) //__builtin_smulll_overflow doesn't work on 32-bit systems yet, causing undefined reference to __mulodi4
#define	REB_I64_MUL_OF(x, y, prod) __builtin_smulll_overflow((x), (y), (prod))
#else
REBOOL reb_i64_mul_overflow(i64 x, i64 y, i64 *prod);
#define	REB_I64_MUL_OF(x, y, prod) reb_i64_mul_overflow((x), (y), (prod))
#endif //__LP64__
#else
REBOOL reb_i64_mul_overflow(i64 x, i64 y, i64 *prod);
#define	REB_I64_MUL_OF(x, y, prod) reb_i64_mul_overflow((x), (y), (prod))
#endif

#if __has_builtin(__builtin_umull_overflow) && __has_builtin(__builtin_umulll_overflow) || GCC_VERSION_AT_LEAST(5, 1)
#ifdef __LP64__
#define	REB_U64_MUL_OF(x, y, prod) __builtin_umull_overflow((x), (y), (long*)(prod))
#else // presumably __LLP64__ or __LP32__
#define	REB_U64_MUL_OF(x, y, prod) __builtin_umulll_overflow((x), (y), (prod))
#endif //__LP64__
#else
REBOOL reb_u64_mul_overflow(u64 x, u64 y, u64 *prod);
#define	REB_U64_MUL_OF(x, y, prod) reb_u64_mul_overflow((x), (y), (prod))
#endif

#endif //__SYS_INT_FUNCS_H_
