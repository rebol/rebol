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
**  Module:  f-random.c
**  Summary: random number generation
**  Section: functional
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

/*	This program by D E Knuth is in the public domain and freely copyable.
 *	It is explained in Seminumerical Algorithms, 3rd edition, Section 3.6
 *	(or in the errata to the 2nd edition --- see
 *		http://www-cs-faculty.stanford.edu/~knuth/taocp.html
 *	in the changes to Volume 2 on pages 171 and following).				*/

/*	N.B. The MODIFICATIONS introduced in the 9th printing (2002) are
		included here; there's no backwards compatibility with the original. */

/*	This version also adopts Brendan McKay's suggestion to
		accommodate naive users who forget to call Set_Random (seed).			*/

/*	If you find any bugs, please report them immediately to
 *				 taocp@cs.stanford.edu
 *	(and you will be rewarded if the bug is genuine). Thanks!			*/

/************ see the book for explanations and caveats! *******************/
/************ in particular, you need two's complement arithmetic **********/

/* Modified by Ladislav Mecir for REBOL to generate 62-bit numbers */

#define KK 100								/* the long lag */
#define LL	37								/* the short lag */
#define MM ((REBI64)1<<62)					/* the modulus, 2^62 */
#define mod_diff(x,y) (((x)-(y))&(MM-1))	/* subtraction mod MM */

static REBI64 ran_x[KK];					/* the generator state */

#ifdef __STDC__
void ran_array(REBI64 aa[], int n)
#else
void ran_array(aa,n)	/* put n new random numbers in aa */
	REBI64 *aa;			/* destination */
	int n;				/* array length (must be at least KK) */
#endif
{
	register int i,j;
	for (j=0;j<KK;j++) aa[j]=ran_x[j];
	for (;j<n;j++) aa[j]=mod_diff(aa[j-KK],aa[j-LL]);
	for (i=0;i<LL;i++,j++) ran_x[i]=mod_diff(aa[j-KK],aa[j-LL]);
	for (;i<KK;i++,j++) ran_x[i]=mod_diff(aa[j-KK],ran_x[i-LL]);
}

/* the following routines are from exercise 3.6--15 */
/* after calling Set_Random, get new randoms by, e.g., "x=ran_arr_next()" */

#define QUALITY 1009 /* recommended quality level for high-res use */
static REBI64 ran_arr_buf[QUALITY];
static REBI64 ran_arr_dummy=-1, ran_arr_started=-1;
static REBI64 *ran_arr_ptr=&ran_arr_dummy;	/* the next random number, or -1 */

#define TT	70		/* guaranteed separation between streams */
#define is_odd(x)	((x)&1)			/* units bit of x */

/***********************************************************************
**
*/	void Set_Random(REBI64 seed)
/*
***********************************************************************/
{
	register int t,j;
	REBI64 x[KK+KK-1];					/* the preparation buffer */
	register REBI64 ss=(seed+2)&(MM-2);
	for (j=0;j<KK;j++) {
		x[j]=ss;						/* bootstrap the buffer */
		ss<<=1; if (ss>=MM) ss-=MM-2;	/* cyclic shift 61 bits */
	}
	x[1]++;				/* make x[1] (and only x[1]) odd */
	for (ss=seed&(MM-1),t=TT-1; t;) {
		for (j=KK-1;j>0;j--) x[j+j]=x[j], x[j+j-1]=0; /* "square" */
		for (j=KK+KK-2;j>=KK;j--)
			x[j-(KK-LL)]=mod_diff(x[j-(KK-LL)],x[j]),
			x[j-KK]=mod_diff(x[j-KK],x[j]);
		if (is_odd(ss)) {				/* "multiply by z" */
			for (j=KK;j>0;j--)	x[j]=x[j-1];
			x[0]=x[KK];			/* shift the buffer cyclically */
			x[LL]=mod_diff(x[LL],x[KK]);
		}
		if (ss) ss>>=1; else t--;
	}
	for (j=0;j<LL;j++) ran_x[j+KK-LL]=x[j];
	for (;j<KK;j++) ran_x[j-LL]=x[j];
	for (j=0;j<10;j++) ran_array(x,KK+KK-1); /* warm things up */
	ran_arr_ptr=&ran_arr_started;
}

#define ran_arr_next() (*ran_arr_ptr>=0? *ran_arr_ptr++: ran_arr_cycle())
static REBI64 ran_arr_cycle()
{
	if (ran_arr_ptr==&ran_arr_dummy)
		Set_Random(314159L); /* the user forgot to initialize */
	ran_array(ran_arr_buf,QUALITY);
	ran_arr_buf[KK]=-1;
	ran_arr_ptr=ran_arr_buf+1;
	return ran_arr_buf[0];
}

/***********************************************************************
**
*/	REBI64 Random_Int(REBFLG secure)
/*
**		Return random integer. Secure uses SHA1 for better safety.
**
***********************************************************************/
{
	REBI64 tmp;
	tmp = ran_arr_next();

	if (secure) {
		REBYTE srcbuf[20], dstbuf[20];

		memcpy(srcbuf, (REBYTE*)&tmp, sizeof(tmp));
		memset(srcbuf + sizeof(tmp), *(REBYTE*)&tmp, 20 - sizeof(tmp));

		SHA1(srcbuf, 20, dstbuf);
		memcpy((REBYTE*)&tmp, dstbuf, sizeof(tmp));
	}

	return tmp;
}

#define MAX_U64 ((REBU64)(REBI64)-1)
/***********************************************************************
**
*/	REBI64 Random_Range(REBI64 r, REBFLG secure)
/*
***********************************************************************/
{
	REBU64 s, m, u;
	if (r == 0) return 0;
	s = (r < 0) ? -r : r;
	if (!secure && s > MM) Trap0(RE_OVERFLOW);
	m = secure ? MAX_U64 - (MAX_U64 - s + 1) % s : MM - MM % s - 1; /* rejection limit */
	do u = Random_Int(secure); while (u > m); /* get a random below the limit */
	u = u % s + 1;
	return (r > 0) ? u : - (REBI64)u;
}

/***********************************************************************
**
*/	REBDEC Random_Dec(REBDEC r, REBFLG secure)
/*
***********************************************************************/
{
	REBDEC t, s;
	t = secure ? 5.4210108624275222e-20 /* 2^-64 */ :  2.1684043449710089e-19 /* 2^-62 */;
	/* care is taken to never overflow and yield a correct sign */
	s = (REBDEC)Random_Int(secure);
	if (s < 0.0) s += 1.8446744073709552e19;
	return (s * t) * r;
}
