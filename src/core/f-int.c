/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2014 Atronix Engineering, Inc
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
**  Module:  f-int.c
**  Summary: integer arithmetic functions
**  Section: functional
**  Author:  Shixin Zeng
**  Notes: Based on original code in t-integer.c
**
***********************************************************************/

#include "reb-c.h"
#include "sys-int-funcs.h"

REBOOL reb_i32_add_overflow(i32 x, i32 y, i32 *sum)
{
	i64 sum64 = (i64)x + (i64)y;
	if (sum64 > MAX_I32 || sum64 < MIN_I32) return TRUE;
	*sum = (i32)sum64;
	return FALSE;
}

REBOOL reb_u32_add_overflow(u32 x, u32 y, u32 *sum)
{
	u64 s = (u64)x + (u64)y;
	if (s > MAX_I32) return TRUE;
	*sum = (u32)s;
	return FALSE;
}

REBOOL reb_i64_add_overflow(i64 x, i64 y, i64 *sum)
{
	*sum = (REBU64)x + (REBU64)y; /* never overflow with unsigned integers*/
	if (((x < 0) == (y < 0))
		&& ((x < 0) != (*sum < 0))) return TRUE;
	return FALSE;
}

REBOOL reb_u64_add_overflow(u64 x, u64 y, u64 *sum)
{
	*sum = x + y;
	if (*sum < x || *sum < y) return TRUE;
	return FALSE;
}

REBOOL reb_i32_sub_overflow(i32 x, i32 y, i32 *diff)
{
	*diff = (i64)x - (i64)y;
	if (((x < 0) != (y < 0)) && ((x < 0) != (*diff < 0))) return TRUE;

	return FALSE;
}

REBOOL reb_i64_sub_overflow(i64 x, i64 y, i64 *diff)
{
	*diff = (REBU64)x - (REBU64)y;
	if (((x < 0) != (y < 0)) && ((x < 0) != (*diff < 0))) return TRUE;

	return FALSE;
}

REBOOL reb_i32_mul_overflow(i32 x, i32 y, i32 *prod)
{
	i64 p = (i64)x * (i64)y;
	if (p > MAX_I32 || p < MIN_I32) return TRUE;
	*prod = (i32)p;
	return FALSE;
}

REBOOL reb_u32_mul_overflow(u32 x, u32 y, u32 *prod)
{
	u64 p = (u64)x * (u64)y;
	if (p > MAX_U32) return TRUE;
	*prod = (u32)p;
	return FALSE;
}

REBOOL reb_i64_mul_overflow(i64 x, i64 y, i64 *prod)
{
	REBFLG sgn;
	u64 p = 0;

	if (y == 0 || x == 0) {
		*prod = 0;
		return FALSE;
	}

	sgn = (x < 0);
	if (sgn) {
		if (x == MIN_I64) {
			switch (y) {
				case 1:
					*prod = x;
					return 0;
				default:
					return 1;
			}
		}
		x = -x; /* undefined when x == MIN_I64 */
	}
	if (y < 0) {
		sgn = !sgn;
		if (y == MIN_I64) {
			switch (x) {
				case 1:
					if (!sgn) {
						return 1;
					} else {
						*prod = y;
						return 0;
					}
				default:
					return 1;
			}
		}
		y = -y; /* undefined when y == MIN_I64 */
	}

	if (REB_U64_MUL_OF(x, y, (u64 *)&p)
		|| (!sgn && p > MAX_I64)
		|| (sgn && p - 1 > MAX_I64)) return TRUE; /* assumes 2's complements */

	if (sgn && p == (u64)MIN_I64) {
		*prod = MIN_I64;
		return FALSE;
	}

	*prod = sgn? -(i64)p : p;
	return FALSE;
}

REBOOL reb_u64_mul_overflow(u64 x, u64 y, u64 *prod)
{
	u64 x0, y0, x1, y1;
	u64 b = U64_C(1) << 32;
	u64 tmp = 0;
	x1 = x >> 32;
	x0 = (u32)x;
	y1 = y >> 32;
	y0 = (u32)y;
	
	/* p = (x1 * y1) * b^2 + (x0 * y1 + x1 * y0) * b + x0 * y0 */

	if (x1 && y1) return TRUE; /* (x1 * y1) * b^2 overflows */

	tmp = (x0 * y1 + x1 * y0); /* never overflow, because x1 * y1 == 0 */
	if (tmp >= b) return TRUE;  /*(x0 * y1 + x1 * y0) * b overflows */

	return REB_U64_ADD_OF(tmp << 32, x0 * y0, prod);
}
