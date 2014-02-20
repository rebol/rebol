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
**  Module:  f-deci.c
**  Summary: extended precision arithmetic functions
**  Section: functional
**  Author:  Ladislav Mecir for REBOL Technologies
**  Notes:
**    Deci significands are 87-bit long, unsigned, unnormalized, stored in
**    little endian order. (Maximal deci significand is 1e26 - 1, i.e. 26
**    nines)
**
**    Sign is one-bit, 1 means nonpositive, 0 means nonnegative.
**
**    Exponent is 8-bit, unbiased.
**
**    Functions may be inlined (especially the ones marked by INLINE).
**    64-bit and/or double arithmetic used where they bring advantage.
**
***********************************************************************/

#include "sys-core.h"
#include "sys-deci-funcs.h"
#include "sys-dec-to-char.h"

#ifndef TEST_MODE
#define OVERFLOW_ERROR Trap0(RE_OVERFLOW)
#define DIVIDE_BY_ZERO_ERROR Trap0(RE_ZERO_DIVIDE)
#endif

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')

#define MASK32(i) (REBCNT)(i)

#define two_to_32 4294967296.0
#define two_to_32l 4294967296.0l

/* useful deci constants */
static const deci deci_zero = {0u, 0u, 0u, 0u, 0};
static const deci deci_one = {1u, 0u, 0u, 0u, 0};
static const deci deci_minus_one = {1u, 0u, 0u, 1u, 0};
/* end of deci constants */

static const REBCNT min_int64_t_as_deci[] = {0u, 0x80000000u, 0u};

/*
	Compare significand a and significand b;
	-1 means a < b;
	0 means a = b;
	1 means a > b;
*/
INLINE REBINT m_cmp (REBINT n, const REBCNT a[], const REBCNT b[]) {
	REBINT i;
	for (i = n - 1; i >= 0; i--)
		if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
	return 0;
}

INLINE REBFLG m_is_zero (REBINT n, const REBCNT a[]) {
	REBINT i;
	for (i = 0; (i < n) && (a[i] == 0); i++);
	return i == n;
}

/* unnormalized powers of ten */
static const REBCNT P[][3] = {
	{1u, 0u, 0u},							/* 1e0 */
	{10u, 0u, 0u},							/* 1e1 */
	{100u, 0u, 0u},							/* 1e2 */
	{1000u, 0u, 0u},						/* 1e3 */
	{10000u, 0u, 0u},						/* 1e4 */
	{100000u, 0u, 0u},						/* 1e5 */
	{1000000u, 0u, 0u},						/* 1e6 */
	{10000000u, 0u, 0u},					/* 1e7 */
	{100000000u, 0u, 0u},					/* 1e8 */
	{1000000000u, 0u, 0u},					/* 1e9 */
	{1410065408u, 2u, 0u},					/* 1e10 */
	{1215752192u, 23u, 0u},					/* 1e11 */
	{3567587328u, 232u, 0u},				/* 1e12 */
	{1316134912u, 2328u, 0u},				/* 1e13 */
	{276447232u, 23283u, 0u},				/* 1e14 */
	{2764472320u, 232830u, 0u},				/* 1e15 */
	{1874919424u, 2328306u, 0u},			/* 1e16 */
	{1569325056u, 23283064u, 0u},			/* 1e17 */
	{2808348672u, 232830643u, 0u},			/* 1e18 */
	{2313682944u, 2328306436u, 0u},			/* 1e19 */
	{1661992960u, 1808227885u, 5u},			/* 1e20 */
	{3735027712u, 902409669u, 54u},			/* 1e21 */
	{2990538752u, 434162106u, 542u},		/* 1e22 */
	{4135583744u, 46653770u, 5421u},		/* 1e23 */
	{2701131776u, 466537709u, 54210u},		/* 1e24 */
	{1241513984u, 370409800u, 542101u},		/* 1e25 */
	{3825205248u, 3704098002u, 5421010u}	/* 1e26 */
};

/* 1e26 as double significand */
static const REBCNT P26[] = {3825205248u, 3704098002u, 5421010u, 0u, 0u, 0u};
/* 1e26 - 1 */
static const REBCNT P26_1[] = {3825205247u, 3704098002u, 5421010u};

/*
	Computes max decimal shift left for nonzero significand a with length 3;
	using double arithmetic;
*/
INLINE REBINT max_shift_left (const REBCNT a[]) {
    REBINT i;
	i = (REBINT)(log10((a[2] * two_to_32 + a[1]) * two_to_32 + a[0]) + 0.5);
    return m_cmp (3, P[i], a) <= 0 ? 25 - i : 26 - i;
}

/* limits for "double significand" right shift */
static const REBCNT Q[][6] = {
    {3892314107u, 2681241660u, 54210108u, 0u, 0u, 0u},						/* 1e27-5e0 */
    {268435406u, 1042612833u, 542101086u, 0u, 0u, 0u},						/* 1e28-5e1 */
    {2684354060u, 1836193738u, 1126043566u, 1u, 0u, 0u},					/* 1e29-5e2 */
    {1073736824u, 1182068202u, 2670501072u, 12u, 0u, 0u},					/* 1e30-5e3 */
    {2147433648u, 3230747430u, 935206946u, 126u, 0u, 0u},					/* 1e31-5e4 */
    {4294467296u, 2242703232u, 762134875u, 1262u, 0u, 0u},					/* 1e32-5e5 */
    {4289967296u, 952195849u, 3326381459u, 12621u, 0u, 0u},					/* 1e33-5e6 */
    {4244967296u, 932023907u, 3199043520u, 126217u, 0u, 0u},				/* 1e34-5e7 */
    {3794967296u, 730304487u, 1925664130u, 1262177u, 0u, 0u},				/* 1e35-5e8 */
    {3589934592u, 3008077582u, 2076772117u, 12621774u, 0u, 0u},				/* 1e36-5e9 */
    {1539607552u, 16004756u, 3587851993u, 126217744u, 0u, 0u},				/* 1e37-5e10 */
    {2511173632u, 160047563u, 1518781562u, 1262177448u, 0u, 0u},			/* 1e38-5e11 */
    {3636899840u, 1600475635u, 2302913732u, 4031839891u, 2u, 0u},			/* 1e39-5e12 */
    {2009260032u, 3119854470u, 1554300843u, 1663693251u, 29u, 0u},			/* 1e40-5e13*/
    {2912731136u, 1133773632u, 2658106549u, 3752030625u, 293u, 0u},			/* 1e41-5e14 */
    {3357507584u, 2747801734u, 811261716u, 3160567888u, 2938u, 0u},			/* 1e42-5e15 */
    {3510304768u, 1708213571u, 3817649870u, 1540907809u, 29387u, 0u},		/* 1e43-5e16 */
    {743309312u, 4197233830u, 3816760335u, 2524176210u, 293873u, 0u},		/* 1e44-5e17 */
    {3138125824u, 3317632637u, 3807864991u, 3766925628u, 2938735u, 0u},		/* 1e45-5e18 */
    {1316487168u, 3111555305u, 3718911549u, 3309517920u, 29387358u, 0u},	/* 1e46-5e19 */
    {279969792u, 1050781981u, 2829377129u, 3030408136u, 293873587u, 0u},	/* 1e47-5e20 */
    {2799697920u, 1917885218u, 2523967516u, 239310294u, 2938735877u, 0u},	/* 1e48-5e21 */
    {2227175424u, 1998983002u, 3764838684u, 2393102945u, 3617554994u, 6u},	/* 1e49-5e22 */
    {796917760u, 2809960841u, 3288648476u, 2456192978u, 1815811577u, 68u},	/* 1e50-5e23 */
    {3674210304u, 2329804635u, 2821713694u, 3087093307u, 978246591u, 684u},	/* 1e51-5e24 */
    {2382364672u, 1823209878u, 2447333169u, 806162004u, 1192531325u, 6842u}	/* 1e52-5e25 */
};

/*
    Computes minimal decimal shift right for "double significand" a with length 6
    to fit length 3;
	using double arithmetic;
*/
INLINE REBINT min_shift_right (const REBCNT a[6]) {
    REBINT i;
    if (m_cmp (6, a, P26) < 0) return 0;
    i = (REBINT) (log10 (
		((((a[5] * two_to_32 + a[4]) * two_to_32 + a[3]) * two_to_32 + a[2]) * two_to_32 + a[1]) * two_to_32 + a[0]
	) + 0.5);
	if (i == 26) return 1;
    return (m_cmp (6, Q[i - 27], a) <= 0) ? i - 25 : i - 26;
}

/* Finds out if deci a is zero */
REBFLG deci_is_zero (const deci a) {
	return (a.m0 == 0) && (a.m1 == 0) && (a.m2 == 0);
}

/* Changes the sign of a deci value */
deci deci_negate (deci a) {
	a.s = !a.s;
	return a;
}

/* Returns the absolute value of deci a */
deci deci_abs (deci a) {
	a.s = 0;
	return a;
}

/*
	Adds unsigned 32-bit value b to significand a;
	a must be "large enough" to contain the sum;
	using 64-bit arithmetic;
*/
INLINE void m_add_1 (REBCNT *a, const REBCNT b) {
	REBU64 c = (REBU64) b;
	while (c) {
		c += (REBU64) *a;
		*(a++) = (REBCNT)c;
		c >>= 32;
	}
}

/*
	Subtracts unsigned 32-bit value b from significand a;
	using 64-bit arithmetic;
*/
INLINE void m_subtract_1 (REBCNT *a, const REBCNT b) {
	REBI64 c = - (REBI64) b;
	while (c) {
		c += 0xffffffffu + (REBI64)*a + 1;
		*(a++) = MASK32(c);
		c = (c >> 32) - 1;
	}
}

/*
	Adds significand b to significand a yielding sum s;
	using 64-bit arithmetic;
*/
INLINE void m_add (REBINT n, REBCNT s[], const REBCNT a[], const REBCNT b[]) {
	REBU64 c = (REBU64) 0;
	REBINT i;
	for (i = 0; i < n; i++) {
		c += (REBU64) a[i] + (REBU64) b[i];
		s[i] = MASK32(c);
		c >>= 32; 
	}
	s[n] = (REBCNT)c;
}

/*
	Subtracts significand b from significand a yielding difference d;
	returns carry flag to signal whether the result is negative;
	using 64-bit arithmetic;
*/
INLINE REBINT m_subtract (REBINT n, REBCNT d[], const REBCNT a[], const REBCNT b[]) {
	REBU64 c = (REBU64) 1;
	REBINT i;
	for (i = 0; i < n; i++) {
		c += (REBU64) 0xffffffffu + (REBU64) a[i] - (REBU64) b[i];
		d[i] = MASK32(c);
		c >>= 32;
	}
	return (REBINT) c - 1;
}

/*
	Negates significand a;
	using 64-bit arithmetic;
*/
INLINE void m_negate (REBINT n, REBCNT a[]) {
	REBU64 c = (REBU64) 1;
	REBINT i;
	for (i = 0; i < n; i++) {
		c += (REBU64) 0xffffffffu - (REBU64) a[i];
		a[i] = MASK32(c);
		c >>= 32;
	}
}

/* 
	Multiplies significand a by b storing the product to p;
	p and a may be the same;
	using 64-bit arithmetic;
*/
INLINE void m_multiply_1 (REBINT n, REBCNT p[], const REBCNT a[], REBCNT b) {
	REBINT j;
	REBU64 f = b, g = (REBU64) 0;
	for (j = 0; j < n; j++) {
		g += f * (REBU64) a[j];
		p[j] = MASK32(g);
		g >>= 32; 
	}
	p[n] = (REBCNT) g;
}

/*
	Decimally shifts significand a to the "left";
	a must be longer than the complete result;
	n is the initial length of a; 
*/
INLINE void dsl (REBINT n, REBCNT a[], REBINT shift) {
	REBINT shift1;
	for (; shift > 0; shift -= shift1) {
		shift1 = 9 <= shift ? 9 : shift;
		m_multiply_1 (n, a, a, P[shift1][0]);
		if (a[n] != 0) n++;
	}
}

/*
	Multiplies significand a by significand b yielding the product p;
	using 64-bit arithmetic;
*/
INLINE void m_multiply (REBCNT p[/* n + m */], REBINT n, const REBCNT a[], REBINT m, const REBCNT b[]) {
	REBINT i, j;
	REBU64 f, g;
	memset (p, 0, (n + m) * sizeof (REBCNT));
	for (i = 0; i < m; i++) {
		f = (REBU64) b[i];
		g = (REBU64) 0;
		for (j = 0; j < n; j++) {
			g += f * (REBU64) a[j] + p[i + j];
			p[i + j] = MASK32(g);
			g >>= 32;
		}
		m_add_1 (p + i + j, (REBCNT) g);
	}	
}

/* 
	Divides significand a by b yielding quotient q;
	returns the remainder;
	b must be nonzero!
	using 64-bit arithmetic;
*/
INLINE REBCNT m_divide_1 (REBINT n, REBCNT q[], const REBCNT a[], REBCNT b) {
	REBINT i;
	REBU64 f = 0, g = b;
	for (i = n - 1; i >= 0; i--) {
		f = (f << 32) + (REBU64) a[i];
		q[i] = (REBCNT)(f / g);
		f %= g;
	}
	return (REBCNT) f;
}

/*
	Decimally shifts significand a to the "right";
	truncate flag t_flag is an I/O value with the following meaning:
	0 - result is exact
	1 - less than half of the least significant unit truncated
	2 - exactly half of the least significant unit truncated
	3 - more than half of the least significant unit truncated
*/
INLINE void dsr (REBINT n, REBCNT a[], REBINT shift, REBINT *t_flag) {
	REBCNT remainder, divisor;
	REBINT shift1;
	for (; shift > 0; shift -= shift1) {
		shift1 = 9 <= shift ? 9 : shift;
		remainder = m_divide_1 (n, a, a, divisor = P[shift1][0]);
		if (remainder < divisor / 2) {
			if (remainder || *t_flag) *t_flag = 1;
		} else if ((remainder > divisor / 2) || *t_flag) *t_flag = 3;
		else *t_flag = 2;
	}
} 

/*
	Decimally shifts significands a and b to make them comparable;
	ea and eb are exponents;
	ta and tb are truncate flags like above;
*/
INLINE void make_comparable (REBCNT a[4], REBINT *ea, REBINT *ta, REBCNT b[4], REBINT *eb, REBINT *tb) {
	REBCNT *c;
	REBINT *p;
	REBINT shift, shift1;
	
	/* set truncate flags to zero */
	*ta = 0;
	*tb = 0;	

	if (*ea == *eb) return; /* no work needed */
	
	if (*ea < *eb) {
		/* swap a and b to fulfill the condition below */
		c = a;
		a = b;
		b = c;
		
		p = ea;
		ea = eb;
		eb = p;
		
		p = ta;
		ta = tb;
		tb = p;
	}
	/* (*ea > *eb) */

	/* decimally shift a to the left */
	if (m_is_zero (3, a)) {
		*ea = *eb;
		return;
	}
	shift1 = max_shift_left (a) + 1;
	shift = *ea - *eb;
    dsl (3, a, shift1 = shift1 < shift ? shift1 : shift);
    *ea -= shift1;
	
	/* decimally shift b to the right if necessary */
	shift = *ea - *eb;
	if (!shift) return;
	if (shift > 26) {
		/* significand underflow */
		if (!m_is_zero (3, b)) *tb = 1; 		 
		memset (b, 0, 3 * sizeof (REBCNT));
		*eb = *ea;
		return;
	}
	dsr (3, b, shift, tb);
	*eb = *ea;
}

REBFLG deci_is_equal (deci a, deci b) {
	REBINT ea = a.e, eb = b.e, ta, tb;
	REBCNT sa[] = {a.m0, a.m1, a.m2, 0}, sb[] = {b.m0, b.m1, b.m2, 0};
	
	make_comparable (sa, &ea, &ta, sb, &eb, &tb);
	
	/* round */
	if ((ta == 3) || ((ta == 2) && (sa[0] % 2 == 1))) m_add_1 (sa, 1);
	else if ((tb == 3) || ((tb == 2) && (sb[0] % 2 == 1))) m_add_1 (sb, 1);

	return (m_cmp (3, sa, sb) == 0) && ((a.s == b.s) || m_is_zero (3, sa));
}

REBFLG deci_is_lesser_or_equal (deci a, deci b) {
	REBINT ea = a.e, eb = b.e, ta, tb;
	REBCNT sa[] = {a.m0, a.m1, a.m2, 0}, sb[] = {b.m0, b.m1, b.m2, 0};

	if (a.s && !b.s) return 1;
	if (!a.s && b.s) return m_is_zero (3, sa) && m_is_zero (3, sb);
	make_comparable (sa, &ea, &ta, sb, &eb, &tb);

	/* round */
	if ((ta == 3) || ((ta == 2) && (sa[0] % 2 == 1))) m_add_1 (sa, 1);
	else if ((tb == 3) || ((tb == 2) && (sb[0] % 2 == 1))) m_add_1 (sb, 1);

	return a.s ? (m_cmp (3, sa, sb) >= 0) : (m_cmp (3, sa, sb) <= 0); 
}

deci deci_add (deci a, deci b) {
	deci c;
	REBCNT sc[4];
	REBINT ea = a.e, eb = b.e, ta, tb, tc, test;
	REBCNT sa[] = {a.m0, a.m1, a.m2, 0}, sb[] = {b.m0, b.m1, b.m2, 0};
	
	make_comparable (sa, &ea, &ta, sb, &eb, &tb);
	
	c.s = a.s;
	if (a.s == b.s) {
		/* addition */
		m_add (3, sc, sa, sb);
		tc = ta + tb;
		
		/* significand normalization */
		test = m_cmp (3, sc, P26_1);
		if ((test > 0) || ((test == 0) && ((tc == 3) || ((tc == 2) && (sc[0] % 2 == 1))))) {
			if (ea == 127) OVERFLOW_ERROR;
			ea++;
			dsr (3, sc, 1, &tc);
			/* the shift may be needed once again */
			test = m_cmp (3, sc, P26_1);
			if ((test > 0) || ((test == 0) && ((tc == 3) || ((tc == 2) && (sc[0] % 2 == 1))))) {
				if (ea == 127) OVERFLOW_ERROR;
				ea++;
				dsr (3, sc, 1, &tc);
			}
		}

		/* round */
		if ((tc == 3) || ((tc == 2) && (sc[0] % 2 == 1))) m_add_1 (sc, 1);

	} else {
		/* subtraction */
		tc = ta - tb;
		if (m_subtract (3, sc, sa, sb)) {
			m_negate (3, sc);
			c.s = b.s;
			tc = -tc;
		}
		/* round */
		if ((tc == 3) || ((tc == 2) && (sc[0] % 2 == 1))) m_add_1 (sc, 1);
		else if ((tc == -3) || ((tc == -2) && (sc[0] % 2 == 1))) m_subtract_1 (sc, 1);
	}
	c.m0 = sc[0];
	c.m1 = sc[1];
	c.m2 = sc[2];
	c.e = ea;
	return c;
}

deci deci_subtract (deci a, deci b) {return deci_add (a, deci_negate (b));}

/* using 64-bit arithmetic */
deci int_to_deci (REBI64 a) {
	deci c;
	c.e = 0;
	if (0 <= a) c.s = 0; else {c.s = 1; a = -a;}
	c.m0 = (REBCNT)a;
	c.m1 = (REBCNT)(a >> 32);
	c.m2 = 0;
	return c;
}

/* using 64-bit arithmetic */
REBI64 deci_to_int (const deci a) {
	REBCNT sa[] = {a.m0, a.m1, a.m2, 0};
	REBINT ta;
	REBI64 result;
	
	/* handle zero and small numbers */
	if (m_is_zero (3, sa) || (a.e < -26)) return (REBI64) 0;
	
	/* handle exponent */
	if (a.e >= 20) OVERFLOW_ERROR;
	if (a.e > 0)
		if (m_cmp (3, P[20 - a.e], sa) <= 0) OVERFLOW_ERROR;
		else dsl (3, sa, a.e);
	else if (a.e < 0) dsr (3, sa, -a.e, &ta);
	
	/* convert significand to integer */
	if (m_cmp (3, sa, min_int64_t_as_deci) > 0) OVERFLOW_ERROR;
	result = ((REBI64) sa[1] << 32) | (REBI64) sa[0];
	
	/* handle sign */
	if (a.s) result = -result;
	if (!a.s && (result < 0)) OVERFLOW_ERROR;
	 
	return result;
}

REBDEC deci_to_decimal (const deci a) {
	/* use STRTOD */
	char *se;
    REBYTE b [34];
	deci_to_string(b, a, 0, '.');
	return STRTOD((char *)b, &se);
}

#define DOUBLE_DIGITS 17
/* using the dtoa function */
deci decimal_to_deci (REBDEC a) {
	deci result;
	REBI64 d; /* decimal significand */
	int e; /* decimal exponent */
	int s; /* sign */
	REBYTE *c;
	REBYTE *rve;

    /* convert a to string */
	c = (REBYTE *) dtoa (a, 0, DOUBLE_DIGITS, &e, &s, (char **) &rve);

	e -= (rve - c);

	d = CHR_TO_INT(c);

	result.s = s;
	result.m2 = 0;
	result.m1 = (REBCNT)(d >> 32);
	result.m0 = (REBCNT)d;
	result.e = 0;

	return deci_ldexp(result, e);
}

/*
	Calculates a * (10 ** (*f + e));
    returns zero when underflow occurs;
	ta is a truncate flag as described above;
	*f is supposed to be in range [-128; 127];
*/
INLINE void m_ldexp (REBCNT a[4], REBINT *f, REBINT e, REBINT ta) {
	/* take care of zero significand */
	if (m_is_zero (3, a)) {
		*f = 0;
		return;
	}

	/* take care of exponent overflow */
	if (e >= 281) OVERFLOW_ERROR;
	if (e < -281) e = -282;
	
	*f += e;
	
	/* decimally shift the significand to the right if needed */
	if (*f < -128) {
		if (*f < -154) {
			/* underflow */
			memset (a, 0, 3 * sizeof (REBCNT));
			*f = 0;
			return;						
		}
		/* shift and round */
		dsr (3, a, -128 - *f, &ta);
		*f = -128;
		if ((ta == 3) || ((ta == 2) && (a[0] % 2 == 1))) m_add_1 (a, 1);
		return;
	}
	
	/* decimally shift the significand to the left if needed */
	if (*f > 127) {
		if ((*f >= 153) || (m_cmp (3, P[153 - *f], a) <= 0)) OVERFLOW_ERROR;
		dsl (3, a, *f - 127);
		*f = 127;
	}
}

/* Calculates a * (10 ** e); returns zero when underflow occurs */
deci deci_ldexp (deci a, REBINT e) {
	REBCNT sa[] = {a.m0, a.m1, a.m2, 0};
	REBINT f = a.e;
	
	m_ldexp (sa, &f, e, 0);
	a.m0 = sa[0];
	a.m1 = sa[1];
	a.m2 = sa[2];
	a.e = f;
	return a;
}

#define denormalize \
	if (a.e >= b.e) return a; \
	sa[0] = a.m0; \
	sa[1] = a.m1; \
	sa[2] = a.m2; \
	dsr (3, sa, b.e - a.e, &ta); \
	a.m0 = sa[0]; \
	a.m1 = sa[1]; \
	a.m2 = sa[2]; \
	a.e = b.e; \
	return a;	

/* truncate a to obtain a multiple of b */
deci deci_truncate (deci a, deci b) {
	deci c;
	REBCNT sa[3];
	REBINT ta = 0;

	c = deci_mod (a, b);
	/* negate c */
	c.s = !c.s;
	a = deci_add (a, c);
	/* a is now a multiple of b */
	
	denormalize
}

/* round a away from zero to obtain a multiple of b */
deci deci_away (deci a, deci b) {
	deci c;
	REBCNT sa[3];
	REBINT ta = 0;

	c = deci_mod (a, b);
	if (!deci_is_zero (c)) {
		/* negate c and add b with the sign of c */
		b.s = c.s;
		c.s = !c.s;
		c = deci_add (c, b);
	}
	a = deci_add (a, c);
	/* a is now a multiple of b */
	
	denormalize
}

/* round a down to obtain a multiple of b */
deci deci_floor (deci a, deci b) {
	deci c;
	REBCNT sa[3];
	REBINT ta = 0;
	
	c = deci_mod (a, b);
	/* negate c */
	c.s = !c.s;
	if (!c.s && !deci_is_zero (c)) {
		/* c is positive, add negative b to obtain a negative value */
		b.s = 1;
		c = deci_add (b, c);
	}
	a = deci_add (a, c);
	/* a is now a multiple of b */
	
	denormalize
}

/* round a up to obtain a multiple of b */
deci deci_ceil (deci a, deci b) {
	deci c;
	REBCNT sa[3];
	REBINT ta = 0;
	
	c = deci_mod (a, b);
	/* negate c */
	c.s = !c.s;
	if (c.s && !deci_is_zero (c)) {
		/* c is negative, add positive b to obtain a positive value */ 
		b.s = 0;
		c = deci_add (c, b);
	}
	a = deci_add (a, c);
	/* a is now a multiple of b */

	denormalize
}

/* round a half even to obtain a multiple of b */
deci deci_half_even (deci a, deci b) {
	deci c, d, e, f;
	REBCNT sa[3];
	REBINT ta = 0;
	REBFLG g;
	
	c = deci_mod (a, b);
	
	/* compare c with b/2 not causing overflow */
	b.s = 0;
	c.s = 1;
	d = deci_add (b, c);
	c.s = 0;
	if (deci_is_equal (c, d)) {
		/* rounding half */
		e = deci_add(b, b); /* this may cause overflow for large b */
		f = deci_mod(a, e);
		f.s = 0;
		g = deci_is_lesser_or_equal(f, b);
	} else g = deci_is_lesser_or_equal(c, d);
	if (g) {
		/* rounding towards zero */
		c.s = !a.s;
	} else {
		/* rounding away from zero */
		c = d;
		c.s = a.s;
	}
	a = deci_add (a, c);
	/* a is now a multiple of b */
	
	denormalize
}

/* round a half away from zero to obtain a multiple of b */
deci deci_half_away (deci a, deci b) {
	deci c, d;
	REBCNT sa[3];
	REBINT ta = 0;
	
	c = deci_mod (a, b);
	
	/* compare c with b/2 not causing overflow */
	b.s = 0;
	c.s = 1;
	d = deci_add (b, c);
	c.s = 0;
	if (deci_is_lesser_or_equal (d, c)) {
		/* rounding away */
		c = d;
		c.s = a.s;
	} else {
		/* truncating */
		c.s = !a.s;
	}
	a = deci_add (a, c);
	/* a is now a multiple of b */
	
	denormalize
}

/* round a half truncate to obtain a multiple of b */
deci deci_half_truncate (deci a, deci b) {
	deci c, d;
	REBCNT sa[3];
	REBINT ta = 0;
	
	c = deci_mod (a, b);
	
	/* compare c with b/2 not causing overflow */
	b.s = 0;
	c.s = 1;
	d = deci_add (b, c);
	c.s = 0;
	if (deci_is_lesser_or_equal (c, d)) {
		/* truncating */
		c.s = !a.s;
	} else {
		/* rounding away */
		c = d;
		c.s = a.s;
	}
	a = deci_add (a, c);
	/* a is now a multiple of b */
	
	denormalize
}

/* round a half up to obtain a multiple of b */
deci deci_half_ceil (deci a, deci b) {
	deci c, d;
	REBCNT sa[3];
	REBINT ta = 0;
	
	c = deci_mod (a, b);
	
	/* compare c with b/2 not causing overflow */
	b.s = 0;
	c.s = 1;
	d = deci_add (b, c);
	c.s = 0;

	if (a.s ? deci_is_lesser_or_equal(c, d) : !deci_is_lesser_or_equal(d, c)) {
		/* truncating */
		c.s = !a.s;
	} else {
		/* rounding away */
		c = d;
		c.s = a.s;
	}

#ifdef RM_FIX_B1471
	if (deci_is_lesser_or_equal (d, c)) {
		/* rounding up */
		c.s = !a.s;
		if (c.s && !deci_is_zero (c)) {
			/* c is negative, use d */
			c = d;
			c.s = a.s;
		}
	} else {
		/* rounding down */
		c.s = !a.s;
		if (!c.s && !deci_is_zero (c)) {
			/* c is positive, use d */
			c = d;
			c.s = a.s;
		}
	}
#endif
	a = deci_add(a, c);
	/* a is now a multiple of b */
	
	denormalize
}

/* round a half down to obtain a multiple of b */
deci deci_half_floor (deci a, deci b) {
	deci c, d;
	REBCNT sa[3];
	REBINT ta = 0;
	
	c = deci_mod (a, b);
	
	/* compare c with b/2 not causing overflow */
	b.s = 0;
	c.s = 1;
	d = deci_add (b, c);
	c.s = 0;

	if (a.s ? !deci_is_lesser_or_equal(d, c) : deci_is_lesser_or_equal(c, d)) {
		/* truncating */
		c.s = !a.s;
	} else {
		/* rounding away */
		c = d;
		c.s = a.s;
	}

#ifdef RM_FIX_B1471
	if (deci_is_lesser_or_equal (c, d)) {
		/* rounding down */
		c.s = !a.s;
		if (!c.s && !deci_is_zero (c)) {
			/* c is positive, use d */
			c = d;
			c.s = a.s;
		}
	} else {
		/* rounding up */
		c.s = !a.s;
		if (c.s && !deci_is_zero (c)) {
			/* c is negative, use d */
			c = d;
			c.s = a.s;
		}
	}
#endif
	a = deci_add(a, c);
	/* a is now a multiple of b */
	
	denormalize
}

deci deci_multiply (const deci a, const deci b) {
	deci c;
	REBCNT sa[] = {a.m0, a.m1, a.m2}, sb[] = {b.m0, b.m1, b.m2}, sc[7];
	REBINT shift, tc = 0, e, f = 0;
	
	/* compute the sign */
	c.s = (!a.s && b.s) || (a.s && !b.s);
	
	/* multiply sa by sb yielding "double significand" sc */
	m_multiply (sc, 3, sa, 3, sb);

	/* normalize "double significand" sc and round if needed */
	shift = min_shift_right (sc);
	e = a.e + b.e + shift;
	if (shift > 0) {
		dsr (6, sc, shift, &tc);
		if (((tc == 3) || ((tc == 2) && (sc[0] % 2 == 1))) && (e >= -128)) m_add_1 (sc, 1);
	}

	m_ldexp (sc, &f, e, tc);
	c.m0 = sc[0];
	c.m1 = sc[1];
	c.m2 = sc[2];
	c.e = f;
	return c;
}

/*
	b[m - 1] is supposed to be nonzero;
	m <= n required;
	a, b are copied on entry;
	uses 64-bit arithmetic;
*/

#define MAX_N 7
#define MAX_M 3

INLINE void m_divide (
	REBCNT q[/* n - m + 1 */],
	REBCNT r[/* m */],
	const REBINT n,
	const REBCNT a[/* n */],
	const REBINT m,
	const REBCNT b[/* m */]
) {
	REBCNT c[MAX_N + 1], d[MAX_M + 1], e[MAX_M + 1];
	REBCNT bm = b[m - 1];
	REBU64 cm, dm;
	REBINT i, j, k;
	
	if (m == 1) {
		r[0] = m_divide_1 (n, q, a, bm);
		return;
	}

    /*
        we shift both the divisor and the dividend to the left
        to obtain quotients that are off by one at most
    */
	/* the most significant bit of b[m - 1] */
	i = 0;
	j = 31;
	while (i < j) {
		k = (i + j + 1) / 2;
		if ((REBCNT)(1 << k) <= bm) i = k; else j = k - 1;
	}
	
	/* shift the dividend to the left */
	for (j = 0; j < n; j++) c[j] = a[j] << (31 - i);
	c[n] = 0;
	for (j = 0; j < n; j++) c[j + 1] |= a[j] >> (i + 1);
	
	/* shift the divisor to the left */
	for (j = 0; j < m; j++) d[j] = b[j] << (31 - i);
	d[m] = 0;
	for (j = 0; j < m; j++) d[j + 1] |= b[j] >> (i + 1);

	dm = (REBU64) d[m - 1];
	
	for (j = n - m; j >= 0; j--) {
		cm = ((REBU64) c[j + m] << 32) + (REBU64) c[j + m - 1];
		cm /= dm;
		if (cm > 0xffffffffu) cm = 0xffffffffu;
		m_multiply_1 (m, e, d, (REBCNT) cm);
		if (m_subtract (m + 1, c + j, c + j, e)) {
			/* the quotient is off by one */
			cm--;
			m_add (m, c + j, c + j, d);
		}
		q[j] = (REBCNT) cm;
	}

    /* shift the remainder back to the right */
    c[m] = 0;
    for (j = 0; j < m; j++) r[j] = c[j] >> (31 - i);
    for (j = 0; j < m; j++) r[j] |= c[j + 1] << (i + 1);
}

/* uses double arithmetic */
deci deci_divide (deci a, deci b) {
	REBINT e = a.e - b.e, f = 0;
	deci c;
	REBCNT q[] = {0, 0, 0, 0, 0, 0}, r[4];
	REBCNT sa[] = {a.m0, a.m1, a.m2, 0, 0, 0}, sb[] = {b.m0, b.m1, b.m2, 0};
	double a_dbl, b_dbl, l10;
	REBINT shift, na, nb, tc;

	if (deci_is_zero (b)) DIVIDE_BY_ZERO_ERROR;

	/* compute sign */
	c.s = (!a.s && b.s) || (a.s && !b.s);

	if (deci_is_zero (a)) {
		c.m0 = 0;
		c.m1 = 0;
		c.m2 = 0;
		c.e = 0;
		return c;
	}

	/* compute decimal shift needed to obtain the highest accuracy */	
	a_dbl = (a.m2 * two_to_32 + a.m1) * two_to_32 + a.m0;
	b_dbl = (b.m2 * two_to_32 + b.m1) * two_to_32 + b.m0;
	l10 = log10 (a_dbl);
	shift = (REBINT)ceil (25.5 + log10(b_dbl) - l10);
	dsl (3, sa, shift);
	e -= shift;

	/* count radix 2 ** 32 digits of the shifted significand sa */
	na = (REBINT)ceil ((l10 + shift) * 0.10381025296523 + 0.5);
	if (sa[na - 1] == 0) na--;
	
	nb = b.m2 ? 3 : (b.m1 ? 2 : 1);
	m_divide (q, r, na, sa, nb, sb);
	
	/* compute the truncate flag */
	m_multiply_1 (nb, r, r, 2);
	tc = m_cmp (nb + 1, r, sb);
	if (tc >= 0) tc = tc == 0 ? 2 : 3;
	else tc = m_is_zero (nb + 1, r) ? 0 : 1;

	/* normalize the significand q */
	shift = min_shift_right (q);
	if (shift > 0) {
		dsr (3, q, shift, &tc);
		e += shift;
	}

	/* round q if needed */
	if (((tc == 3) || ((tc == 2) && (q[0] % 2 == 1))) && (e >= -128)) m_add_1 (q, 1);
	
	m_ldexp (q, &f, e, tc);
	c.m0 = q[0];
	c.m1 = q[1];
	c.m2 = q[2];
	c.e = f; 
	return c;
}

#define MAX_NB 7

INLINE REBINT m_to_string (REBYTE *s, REBINT n, const REBCNT a[]) {
    REBCNT r, b[MAX_NB];
	REBYTE v[10 * MAX_NB + 1], *vmax, *k;
    
    /* finds the first nonzero radix 2 ** 32 "digit" */
    for (; (n > 0) && (a[n - 1] == 0); n--);
    
    if (n == 0) {
    	s[0] = '0';
    	s[1] = '\0';
    	return 1;
	}
	
    /* copy a to preserve it */
	memcpy (b, a, n * sizeof (REBCNT));
	
	k = vmax = v + 10 * MAX_NB;
	*k = '\0';
    while (n > 0) {
    	r = m_divide_1 (n, b, b, 10u);
		if (b[n - 1] == 0) n--;
		*--k = '0' + r;
	}

	strcpy(s, k);
    return vmax - k;
}

REBINT deci_to_string(REBYTE *string, const deci a, const REBYTE symbol, const REBYTE point) {
	REBYTE *s = string;
	REBCNT sa[] = {a.m0, a.m1, a.m2};
	REBINT j, e;
	
	/* sign */
	if (a.s) *s++ = '-';

	if (symbol) *s++ = symbol;

	if (deci_is_zero (a)) {
		*s++ = '0';
		*s = '\0';
		return s-string;
	}

	j = m_to_string(s, 3, sa);
	e = j + a.e;
	
	if (e < j) {
		if (e <= 0) {
			if (e < -6) {
				s++;
				if (j > 1) {
					memmove(s + 1, s, j);
					*s = point;
					s += j;
				}
				*s++ = 'e';
				INT_TO_STR(e - 1, s);
				s = strchr(s, '\0');
			} else { /* -6 <= e <= 0 */
				memmove(s + 2 - e, s, j + 1);
				*s++ = '0';
				*s++ = point;
				memset(s, '0', -e);
				s += j - e;
			}
		} else { /* 0 < e < j */
			s += e;
			memmove(s + 1, s, j - e + 1);
			*s++ = point;
			s += j - e;
		}
	} else if (e == j) {
		s += j;
	} else { /* j < e */
			s += j;
			*s++ = 'e';
			INT_TO_STR(e - j, s);
			s = strchr(s, '\0');
	}
	
	return s - string;
}

deci deci_mod (deci a, deci b) {
	REBCNT sa[] = {a.m0, a.m1, a.m2};
	REBCNT sb[] = {b.m0, b.m1, b.m2,0}; /* the additional place is for dsl */
	REBCNT sc[] = {10u, 0, 0};
	REBCNT p[6]; /* for multiplication results */
	REBINT e, nb;
	
	if (deci_is_zero (b)) DIVIDE_BY_ZERO_ERROR;
	if (deci_is_zero (a)) return deci_zero;

	e = a.e - b.e;
	if (e < 0) {
		if (max_shift_left (sb) < -e) return a; /* a < b */
		dsl (3, sb, -e);
		b.e = a.e;
		e = 0;
	}
	/* e >= 0 */

	/* count radix 2 ** 32 digits of sb */
	nb = sb[2] ? 3 : (sb[1] ? 2 : 1);
	
	/* sa = remainder(sa, sb) */
	m_divide (p, sa, 3, sa, nb, sb);

    while (e > 0) {
        /* invariants:
            computing remainder (sa * pow (sc, e), sb)
            sa has nb radix pow (2, 32) digits
        */
		if (e % 2) {
			/* sa = remainder (sa * sc, sb) */
			m_multiply (p, nb, sa, nb, sc);
			m_divide (p, sa, nb + nb, p, nb, sb);
			e--;
		} else {
			/* sc = remainder (sc * sc, sb) */
			m_multiply (p, nb, sc, nb, sc);
			m_divide (p, sc, nb + nb, p, nb, sb);
			e /= 2;
		}
	}
	/* e = 0 */
	
	a.m0 = sa[0];
	a.m1 = nb >= 2 ? sa[1] : 0;
	a.m2 = nb == 3 ? sa[2] : 0;
	a.e = b.e;
	return a;
}

/* in case of error the function returns deci_zero and *endptr = s */
deci string_to_deci (REBYTE *s, REBYTE **endptr) {
	REBYTE *a = s;
	deci b = {0, 0, 0, 0, 0};
	REBCNT sb[] = {0, 0, 0, 0}; /* significand */
	REBINT f = 0, e = 0; /* exponents */
	REBINT fp = 0; /* full precision flag */
	REBINT dp = 0; /* decimal point encountered */
	REBINT tb = 0; /* truncate flag */
	REBINT d; /* digit */
	REBINT es = 1; /* exponent sign */
	
	/* sign */
	if ('+' == *a) a++; else if ('-' == *a) {
		b.s = 1;
		a++;
	}
	
	// optional $
	if ('$' == *a) a++;

	/* significand */
	for (; ; a++)
		if (IS_DIGIT(*a)) {
			d = *a - '0';
			if (m_cmp (3, sb, P[25]) < 0) {
				m_multiply_1 (3, sb, sb, 10u);
				m_add_1 (sb, d);
				if (dp) f--;
			} else {
				if (fp) {
					if ((tb == 0) && (d != 0)) tb = 1;
					else if ((tb == 2) && (d != 0)) tb = 3;
				} else {
					fp = 1;
					if (d > 0) tb = d < 5 ? 1 : (d == 5 ? 2 : 3);
				}
				if (!dp) f++;
			}
		} else if (('.' == *a) || (',' == *a)) {
			/* decimal point */
			if (dp) {
				*endptr = s;
				return deci_zero;				
			}
			else dp = 1;
		} else if ('\'' != *a) break;
	
	/* exponent */
	if (('e' == *a) || ('E' == *a)) {
		a++;
		/* exponent sign */
		if ('+' == *a) a++; else if ('-' == *a) {
			a++;
			es = -1;
		}
		for (; ; a++) {
			if (IS_DIGIT(*a)) {
				d = *a - '0';
				e = e * 10 + d;
				if (e > 200000000) {
					if (es == 1) OVERFLOW_ERROR;
					else e = 200000000;
				}
			} else break;
		}
		e *= es;
	}
	/* that is supposed to be all */
	*endptr = a;
	e += f;
	f = 0;
	
	/* round */
	if (((tb == 3) || ((tb == 2) && (sb[0] % 2 == 1))) && (e >= -128)) {
		if (m_cmp (3, sb, P26_1) < 0) m_add_1 (sb, 1);
		else {
			dsr (3, sb, 1, &tb);
			e++;
			if ((tb == 3) || ((tb == 2) && (sb[0] % 2 == 1))) m_add_1 (sb, 1);
		}
	}
	
	m_ldexp (sb, &f, e, tb);
	
	b.m0 = sb[0];
	b.m1 = sb[1];
	b.m2 = sb[2];
	b.e = f;
	return b;  
}

deci deci_sign (deci a) {
	if (deci_is_zero (a)) return a;
	if (a.s) return deci_minus_one; else return deci_one;
}

REBFLG deci_is_same (deci a, deci b) {
	if (deci_is_zero (a)) return deci_is_zero (b);
	return (a.m0 == b.m0) && (a.m1 == b.m1) && (a.m2 == b.m2) && (a.s == b.s) && (a.e == b.e);
}

deci binary_to_deci(REBYTE s[12]) {
	deci d;
	/* this looks like the only way, since the order of bits in bitsets is compiler-dependent */
	d.s = s[0] >> 7;
	d.e = s[0] << 1 | s[1] >> 7;
	d.m2 = (REBCNT)(s[1] << 1) << 15 | (REBCNT)s[2] << 8 | s[3];
	d.m1 = (REBCNT)s[4] << 24 | (REBCNT)s[5] << 16 | (REBCNT)s[6] << 8 | s[7];
	d.m0 = (REBCNT)s[8] << 24 | (REBCNT)s[9] << 16 | (REBCNT)s[10] << 8 | s[11];
	/* validity checks */
	if (d.m2 >= 5421010u) {
		if (d.m1 >= 3704098002u) {
			if (d.m0 > 3825205247u || d.m1 > 3704098002u) OVERFLOW_ERROR;
		} else if (d.m2 > 5421010u) OVERFLOW_ERROR;
	}
	return d;
}

REBYTE *deci_to_binary(REBYTE s[12], const deci d) {
	/* this looks like the only way, since the order of bits in bitsets is compiler-dependent */
	s[0] = d.s << 7 | (REBYTE)d.e >> 1;
	s[1] = (REBYTE)d.e << 7 | d.m2 >> 16;
	s[2] = d.m2 >> 8;
	s[3] = d.m2;
	s[4] = d.m1 >> 24;
	s[5] = d.m1 >> 16;
	s[6] = d.m1 >> 8;
	s[7] = d.m1;
	s[8] = d.m0 >> 24;
	s[9] = d.m0 >> 16;
	s[10] = d.m0 >> 8;
	s[11] = d.m0;
	return s;
}
