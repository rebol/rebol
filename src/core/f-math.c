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
**  Module:  f-math.c
**  Summary: basic math conversions
**  Section: functional
**  Author:  Carl Sassenrath
**  Notes:
**    Do not underestimate what it takes to make some parts of this
**    portable over all systems. Modifications to this code should be
**    tested on multiple operating system runtime libraries, including
**    older/obsolete systems.
**
***********************************************************************/

#include "sys-core.h"
#include "sys-dec-to-char.h"


/***********************************************************************
**
*/	REBYTE *Grab_Int(REBYTE *cp, REBINT *val)
/*
**		Grab an integer value from the string.
**
**		Return the character position just after the integer and return
**		the integer via a pointer to it.
**
**		Notes:
**			1. Stops at the first non-digit.
**			2. If no integer found, pointer doesn't change position.
**			3. Integers may contain REBOL tick (') marks.
**
***********************************************************************/
{
	REBINT value = 0;
	REBINT neg = FALSE;

	if (*cp == '-') cp++, neg = TRUE;
	else if (*cp == '+') cp++;

	while (*cp >= '0' && *cp <= '9') {
		value = (value * 10) + (*cp - '0');
		cp++;
	}

	*val = neg ? -value : value;

	return cp;
}


/***********************************************************************
**
*/	REBYTE *Grab_Int_Scale(REBYTE *cp, REBINT *val, REBCNT scale)
/*
**		Return integer scaled to the number of digits specified.
**		Used for the decimal part of numbers (e.g. times).
**
***********************************************************************/
{
	REBI64 value = 0;

	for (;scale > 0 && *cp >= '0' && *cp <= '9'; scale--) {
		value = (value * 10) + (*cp - '0');
		cp++;
	}

	// Round up if necessary:
	if (*cp >= '5' && *cp <= '9') value++;

	// Ignore excess digits:
	while (*cp >= '0' && *cp <= '9') cp++;

	// Make sure its full scale:
	for (;scale > 0; scale--) value *= 10;

	*val = (REBINT)value;
	return cp;
}


/***********************************************************************
**
*/	REBINT Form_Int_Len(REBYTE *buf, REBI64 val, REBINT maxl)
/*
**		Form an integer string into the given buffer. Result will
**		not exceed maxl length, including terminator.
**
**		Returns the length of the string.
**
**		Notes:
**			1. If result is longer than maxl, returns 0 length.
**			2. Make sure you have room in your buffer!
**
***********************************************************************/
{
	REBYTE tmp[MAX_NUM_LEN];
	REBYTE *tp = tmp;
	REBI64 n;
	REBI64 r;
	REBINT len = 0;

	// defaults for problem cases
	buf[0] = '?';
	buf[1] = 0;

	if (maxl == 0) return 0;

	if (val == 0) {
		*buf++ = '0';
		*buf = 0;
		return 1;
	}

	if (val < 0) {
		val = -val;
		*buf++ = '-';
		maxl--;
		len = 1;
	}

	// Generate string in reverse:
	*tp++ = 0;
	while (val != 0) {
		n = val / 10;	// not using ldiv for easier compatibility
		r = val % 10;
		if (r < 0) {	// check for overflow case when val = 0x80000000...
			r = -r;
			n = -n;
		}
		*tp++ = (REBYTE)('0' + (REBYTE)(r));
		val = n;
	}
	tp--;

	if (tp - tmp > maxl) return 0;

	while (NZ(*buf++ = *tp--)) len++;
	return len;
}


/***********************************************************************
**
*/	REBYTE *Form_Int_Pad(REBYTE *buf, REBI64 val, REBINT max, REBINT len, REBYTE pad)
/*
**		Form an integer string in the given buffer with a min
**		width padded out with the given character. Len > 0 left
**		aligned. Len < 0 is right aligned.
**
**		If len = 0 and val = 0, a null string is formed.
**		Make sure you have room in your buffer before calling this!
**
***********************************************************************/
{
	REBYTE tmp[MAX_NUM_LEN];
	REBINT n;

	n = Form_Int_Len(tmp, val, max);
	if (n == 0) {
		strcpy(buf, "??");
		return buf;  // too long
	}

	if (len >= 0) {
		strcpy(buf, tmp);
		buf += n;
		for (; n < len; n++) *buf++ = pad;
	}
	else { // len < 0
		for (; n < -len; len++) *buf++ = pad;
		strcpy(buf, tmp);
		buf += n;
	}

	*buf = 0;
	return buf;
}


/***********************************************************************
**
*/	REBYTE *Form_Int(REBYTE *buf, REBINT val)
/*
**		Form 32 bit integer string in the given buffer.
**		Make sure you have room in your buffer before calling this!
**
***********************************************************************/
{
	REBINT len = Form_Int_Len(buf, val, MAX_NUM_LEN);
	return buf + len;
}


/***********************************************************************
**
*/	REBYTE *Form_Integer(REBYTE *buf, REBI64 val)
/*
**		Form standard REBOL integer value (32 or 64).
**		Make sure you have room in your buffer before calling this!
**
***********************************************************************/
{
	INT_TO_STR(val, buf);
	return buf+LEN_BYTES(buf);
}


/***********************************************************************
**
*/	REBINT Emit_Integer(REBYTE *buf, REBI64 val)
/*
***********************************************************************/
{
	INT_TO_STR(val, buf);
	return LEN_BYTES(buf);
}


#ifdef OLDER
/***********************************************************************
**
xx*/	REBCNT Set_Random(REBCNT seed)
/*
***********************************************************************/
{
	REBCNT save = next;
	next = seed;
	return save;
}


/***********************************************************************
**
xx*/	REBCNT Random_Int(REBFLG secure)
/*
**		Return random integer. Secure uses SHA1 for better quality.
**		Be careful of endian-ness.
**
***********************************************************************/
{
	REBCNT	tmp;

	next = next * 1103515245L + 12345L;
	tmp = next & 0xffff0000;
	next = next * 1103515245L + 12345L;
	tmp |= (next >> 16);

	if (secure) {
		REBYTE srcbuf[20], dstbuf[20];
		REBCNT i;

		Long_To_Bytes(srcbuf, tmp);
		for(i = sizeof(tmp); i < 20; i += sizeof(tmp))
			memcpy(srcbuf + i, srcbuf, sizeof(tmp));
		SHA1(srcbuf, i, dstbuf);
		tmp = Bytes_To_Long(dstbuf);
	}

	return tmp;
}
#endif

#ifdef OLD_DEC_TO_STR
static int Convert_Decimal(REBDEC d, REBI64 *sig, REBINT *point)
{
	REBDEC e;
	REBDEC n;

	// Check if num needs exp format:
	e = floor(log10(d));
	if (e > 15 || e < -6) return 0; // use gcvt

	modf(d * pow(10, (15-e)), &n);
	*sig = (REBI64)n;
	*point = 1 + (REBINT)e;
	return 1;
}

/***********************************************************************
**
*/  REBINT Emit_Decimal(REBYTE *cp, REBDEC d, REBFLG percent, REBYTE point, REBINT digits)
/*
***********************************************************************/
{
	REBYTE out[MAX_NUMCHR];
	REBINT len;
	REBINT n;
	REBINT i;
	REBI64 sig;
	REBINT pt;
	REBFLG neg;
	REBYTE *start = cp;

	*cp = out[0] = 0;

	// Deal with 0 as special case:
	if (d == 0.0 || d == -0.0) {
		*cp++ = '0';
		if (!percent) {
			*cp++ = '.';
			*cp++ = '0';
		}
	}
	else {

		if (percent) d *= 100.0;

		if (NZ(neg = (d < 0))) d = -d;

		if (Convert_Decimal(d, &sig, &pt)) {
			// Not exp format.
			len = Form_Integer(out, sig) - out;
			if (neg) *cp++ = '-';

			// Trim un-needed trailing zeros:
			for (len--; len > 0 && len >= pt; len--) {
				if (out[len] == '0') out[len] = 0;
				else break;
			}

			// Leading zero, as in 0.1
			if (pt <= 0) *cp++ = '0';

			// Other leading digits:
			for (n = 0; out[n] && n < pt; n++) *cp++ = out[n];

			if (!percent || n <= len) {
				// Decimal point:
				*cp++ = point;

				// Zeros before first significant digit:
				for (i = 0; i > pt; i--) *cp++ = '0';

				// All remaining digits:
				for (; n <= len; n++) *cp++ = out[n];

				// Force extra zero in 1.0 cases:
				if (cp[-1] == point) *cp++ = '0';
			}
		}
		else {
			REBYTE *pp;

			// Requires exp format:
			if (percent) Trap0(RE_OVERFLOW);
			len = Get_System_Int(SYS_OPTIONS, OPTIONS_DECIMAL_DIGITS, MAX_DIGITS);
			if (len > MAX_DIGITS) len = MAX_DIGITS;
			gcvt(d, len, cp); // returns 1.2e123 (also 1e123)
			pp = strchr(cp, '.');
			if (pp && (pp[1] == 'e' || pp[1] == 'E')) {
				memcpy(pp, pp+1, strlen(pp));
			}
			if (point != '.' && pp) {
				cp = strchr(cp, '.');
				if (cp) *cp = point;
			}
			cp = start + LEN_BYTES(start);
		}
	}

	if (percent) *cp++ = '%';
	*cp = 0;

	return cp - start;
}

#else // NEW_DEC_TO_STR

#define MIN_DIGITS 1
/* this is appropriate for 64-bit IEEE754 binary floating point format */
#define MAX_DIGITS 17

REBINT Emit_Decimal(REBYTE *cp, REBDEC d, REBFLG trim, REBYTE point, REBINT decimal_digits) {
	REBYTE *start = cp, *sig, *rve;
	int e, sgn;
	REBINT digits_obtained;

	/* sanity checks */
	if (decimal_digits < MIN_DIGITS) decimal_digits = MIN_DIGITS;
	else if (decimal_digits > MAX_DIGITS) decimal_digits = MAX_DIGITS;

	sig = (REBYTE *) dtoa (d, 0, decimal_digits, &e, &sgn, (char **) &rve);

	digits_obtained = rve - sig;

	/* handle sign */
	if (sgn) *cp++ = '-';

	if (trim == DEC_MOLD_PERCENT) e += 2;

	if ((e > decimal_digits) || (e <= -6)) {
		/* e-format */
		*cp++ = *sig++;

		/* insert the radix point */
		*cp++ = point;

		/* insert the rest */
		memcpy(cp, sig, digits_obtained - 1);
		cp += digits_obtained - 1;
	} else if (e > 0) {
		if (e <= digits_obtained) {
			/* insert digits preceding point */
			memcpy (cp, sig, e);
			cp += e;
			sig += e;

			*cp++ = point;

			/* insert digits following point */
			memcpy(cp, sig, digits_obtained -  e);
			cp += digits_obtained - e;
		} else {
			/* insert all digits obtained */
			memcpy (cp, sig, digits_obtained);
			cp += digits_obtained;

			/* insert zeros preceding point */
			memset (cp, '0', e - digits_obtained);
			cp += e - digits_obtained;

			*cp++ = point;
		}
		e = 0;
	} else {
		*cp++ = '0';

		*cp++ = point;

		memset(cp, '0', -e);
		cp -= e;

		memcpy(cp, sig, digits_obtained);
		cp += digits_obtained;

		e = 0;
	}

	// Add at least one zero after point (unless percent or pair):
	if (*(cp - 1) == point) {if (trim) cp--; else *cp++ = '0';}

	// Add E part if needed:
	if (e) {
		*cp++ = 'e';
		INT_TO_STR(e - 1, cp);
		cp = strchr(cp, 0);
	}

 	if (trim == DEC_MOLD_PERCENT) *cp++ = '%';
	*cp = 0;
	return cp - start;
}
#endif // NEW_DEC_TO_STR
