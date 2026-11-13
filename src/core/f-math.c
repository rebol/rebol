/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2025 Rebol Open Source Contributors
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
**  Author:  Carl Sassenrath, Ladislav Mecir
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
*/	const REBYTE *Grab_Int(const REBYTE *cp, REBINT *val)
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
*/	const REBYTE *Grab_Int_Scale(const REBYTE *cp, REBINT *val, REBCNT scale)
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
	REBINT len = 0;
	REBU64 uval;

	// defaults for problem cases
	buf[0] = '?';
	buf[1] = 0;

	if (maxl == 0) return 0;

	if (val == 0) {
		*buf++ = '0';
		*buf = 0;
		return 1;
	}

	// Handle negative numbers
	if (val < 0) {
		*buf++ = '-';
		// Handle INT64_MIN safely: cast to unsigned
		// (REBU64)-(INT64_MIN) == 9223372036854775808ULL
		// This is well-defined in C99+
		uval = (REBU64)(-(val + 1)) + 1;
		maxl--;
		len = 1;
	}
	else {
		uval = (REBU64)val;
	}

	// Generate string in reverse:
	*tp++ = 0;
	while (uval != 0) {
		*tp++ = '0' + (uval % 10);
		uval /= 10;
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
		strcpy(s_cast(buf), "??");
		return buf;  // too long
	}

	if (len >= 0) {
		strcpy(s_cast(buf), s_cast(tmp));
		buf += n;
		for (; n < len; n++) *buf++ = pad;
	}
	else { // len < 0
		for (; n < -len; len++) *buf++ = pad;
		strcpy(s_cast(buf), s_cast(tmp));
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
	return (REBINT)LEN_BYTES(buf);
}


/***********************************************************************
**
*/	REBLEN Emit_Decimal(REBYTE *cp, REBDEC d, REBFLG trim, REBYTE point, REBINT decimal_digits)
/*
***********************************************************************/
{
#define MIN_DIGITS 1
/* this is appropriate for 64-bit IEEE754 binary floating point format */
#define MAX_DIGITS 17
	
	REBYTE *start = cp, *sig, *rve;
	int e, sgn;
	REBINT digits_obtained;

	/* sanity checks */
	if (decimal_digits < MIN_DIGITS) decimal_digits = MIN_DIGITS;
	else if (decimal_digits > MAX_DIGITS) decimal_digits = MAX_DIGITS;

	sig = (REBYTE *) dtoa (d, 2, decimal_digits, &e, &sgn, (char **) &rve);

	digits_obtained = AS_INT(rve - sig);

	/* handle sign */
	if (sgn) *cp++ = '-';

	if (trim == DEC_MOLD_PERCENT && (digits_obtained > 1 || sig[0] != '0')) e += 2;

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
			/* handle 1.INF and 1.#NaN cases (issue #2544) */
			if (sig[1] == '#') {
				*cp++ = *sig++;
				*cp++ = point;
				memcpy(cp, sig, 4);
				cp += 4;
				goto end;
			}
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
		cp = b_cast(strchr(s_cast(cp), 0));
	}

 	if (trim == DEC_MOLD_PERCENT) *cp++ = '%';
end:
	*cp = 0;
	return AS_REBLEN(cp - start);
}
