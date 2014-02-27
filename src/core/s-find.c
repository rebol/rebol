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
**  Module:  s-find.c
**  Summary: string search and comparison
**  Section: strings
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"


/***********************************************************************
**
*/	REBINT Compare_Binary_Vals(REBVAL *v1, REBVAL *v2)
/*
**		Compare two binary values.
**
**		Compares bytes, not chars. Return the difference.
**
**		Used for: Binary comparision function
**
***********************************************************************/
{
	REBCNT l1 = VAL_LEN(v1);
	REBCNT l2 = VAL_LEN(v2);
	REBCNT len = MIN(l1, l2);
	REBINT n;

	if (IS_IMAGE(v1)) len *= 4;

	n = memcmp(VAL_BIN_DATA(v1), VAL_BIN_DATA(v2), len);

	if (n != 0) return n;

	return l1 - l2;
}


/***********************************************************************
**
*/	REBINT Compare_Bytes(REBYTE *b1, REBYTE *b2, REBCNT len, REBOOL uncase)
/*
**		Compare two byte-wide strings. Return lexical difference.
**
**		Uncase: compare is case-insensitive.
**
***********************************************************************/
{
	REBINT d;

	for (; len > 0; len--, b1++, b2++) {

		if (uncase)
			d = LO_CASE(*b1) - LO_CASE(*b2);
		else
			d = *b1 - *b2;

		if (d != 0) return d;
	}

	return 0;
}


/***********************************************************************
**
*/	REBYTE *Match_Bytes(REBYTE *src, REBYTE *pat)
/*
**		Compare two binary strings. Return where the first differed.
**		Case insensitive.
**
***********************************************************************/
{
	while (*src && *pat) {
		if (LO_CASE(*src++) != LO_CASE(*pat++)) return 0;
	}

	if (*pat) return 0;	// if not at end of pat, then error

	return src;
}


/***********************************************************************
**
*/	REBFLG Match_Sub_Path(REBSER *s1, REBSER *s2)
/*
**		Compare two file path series, regardless of char size.
**		Return TRUE if s1 is a subpath of s2.
**		Case insensitive.
**
***********************************************************************/
{
	REBCNT len = s1->tail;
	REBCNT n;
	REBUNI c1 = 0;
	REBUNI c2;

//	Debug_Series(s1);
//	Debug_Series(s2);

	// s1 len must be <= s2 len
	if (len > s2->tail) return FALSE;

	for (n = 0; n < len; n++) { // includes terminator

		c1 = GET_ANY_CHAR(s1, n);
		c2 = GET_ANY_CHAR(s2, n);

		if (c1 < UNICODE_CASES) c1 = LO_CASE(c1);
		if (c2 < UNICODE_CASES) c2 = LO_CASE(c2);

		if (c1 != c2) break;
	}

	// a/b matches: a/b, a/b/, a/b/c
	c2 = GET_ANY_CHAR(s2, n);
	return (
			n >= len  // all chars matched
			&&  // Must be at end or at dir sep:
			(c1 == '/' || c1 == '\\'
			|| c2 == 0 || c2 == '/' || c2 == '\\')
	);
}


/***********************************************************************
**
*/	REBINT Compare_Uni_Byte(REBUNI *u1, REBYTE *b2, REBCNT len, REBOOL uncase)
/*
**		Compare unicode and byte-wide strings. Return lexical difference.
**
**		Uncase: compare is case-insensitive.
**
***********************************************************************/
{
	REBINT d;
	REBUNI c1;
	REBUNI c2;

	for (; len > 0; len--) {

		c1 = *u1++;
		c2 = *b2++;

		if (uncase && c1 < UNICODE_CASES)
			d = LO_CASE(c1) - LO_CASE(c2);
		else
			d = c1 - c2;

		if (d != 0) return d;
	}

	return 0;
}


/***********************************************************************
**
*/	REBINT Compare_Uni_Str(REBUNI *u1, REBUNI *u2, REBCNT len, REBOOL uncase)
/*
**		Compare two unicode-wide strings. Return lexical difference.
**
**		Uncase: compare is case-insensitive.
**
***********************************************************************/
{
	REBINT d;
	REBUNI c1;
	REBUNI c2;

	for (; len > 0; len--) {

		c1 = *u1++;
		c2 = *u2++;

		if (uncase && c1 < UNICODE_CASES && c2 < UNICODE_CASES)
			d = LO_CASE(c1) - LO_CASE(c2);
		else
			d = c1 - c2;

		if (d != 0) return d;
	}

	return 0;
}


/***********************************************************************
**
*/	REBINT Compare_String_Vals(REBVAL *v1, REBVAL *v2, REBOOL uncase)
/*
**		Compare two string values. Either can be byte or unicode wide.
**
**		Uncase: compare is case-insensitive.
**
**		Used for: general string comparions (various places)
**
***********************************************************************/
{
	REBCNT l1  = VAL_LEN(v1);
	REBCNT l2  = VAL_LEN(v2);
	REBCNT len = MIN(l1, l2);
	REBINT n;

	if (IS_BINARY(v1) || IS_BINARY(v2)) uncase = FALSE;

	if (VAL_BYTE_SIZE(v1)) { // v1 is 8
		if (VAL_BYTE_SIZE(v2))
			n = Compare_Bytes(VAL_BIN_DATA(v1), VAL_BIN_DATA(v2), len, uncase);
		else
			n = -Compare_Uni_Byte(VAL_UNI_DATA(v2), VAL_BIN_DATA(v1), len, uncase);
	}
	else { // v1 is 16
		if (VAL_BYTE_SIZE(v2))
			n = Compare_Uni_Byte(VAL_UNI_DATA(v1), VAL_BIN_DATA(v2), len, uncase);
		else
			n = Compare_Uni_Str(VAL_UNI_DATA(v1), VAL_UNI_DATA(v2), len, uncase);
	}

	if (n != 0) return n;
	return l1 - l2;
}


/***********************************************************************
**
*/	REBINT Compare_UTF8(REBYTE *s1, REBYTE *s2, REBCNT l2)
/*
**		Compare two UTF8 strings.
**
**		It is necessary to decode the strings to check if the match
**		case-insensitively.
**
**		Returns:
**			-3: no match, s2 > s1
**			-1: no match, s1 > s2
**			 0: exact match
**			 1: non-case match, s2 > s1
**			 3: non-case match, s1 > s2
**
**		So, result + 2 for no-match gives proper sort order.
**		And, result - 2 for non-case match gives sort order.
**
**		Used for: WORD comparison.
**
***********************************************************************/
{
	REBINT c1, c2;
	REBCNT l1 = LEN_BYTES(s1);
	REBINT result = 0;

	for (; l1 > 0 && l2 > 0; s1++, s2++, l1--, l2--) {
		c1 = (REBYTE)*s1;
		c2 = (REBYTE)*s2;
		if (c1 > 127) c1 = Decode_UTF8_Char(&s1, &l1); //!!! can return 0 on error!
		if (c2 > 127) c2 = Decode_UTF8_Char(&s2, &l2);
		if (c1 != c2) {
			if (c1 >= UNICODE_CASES || c2 >= UNICODE_CASES ||
				LO_CASE(c1) != LO_CASE(c2)) {
				return (c1 > c2) ? -1 : -3;
			}
			if (!result) result = (c1 > c2) ? 3 : 1;
		}
	}
	if (l1 != l2) result = (l1 > l2) ? -1 : -3;

	return result;
}


/***********************************************************************
**
*/	REBCNT Find_Byte_Str(REBSER *series, REBCNT index, REBYTE *b2, REBCNT l2, REBFLG uncase, REBFLG match)
/*
**		Find a byte string within a byte string. Optimized for speed.
**
**		Returns starting position or NOT_FOUND.
**
**		Uncase: compare is case-insensitive.
**		Match: compare to first position only.
**
**		NOTE: Series tail must be > index.
**
***********************************************************************/
{
	REBYTE *b1;
	REBYTE *e1;
	REBCNT l1;
	REBYTE c;
	REBCNT n;

	// The pattern empty or is longer than the target:
	if (l2 == 0 || (l2 + index) > SERIES_TAIL(series)) return NOT_FOUND;

	b1 = BIN_SKIP(series, index);
	l1 = SERIES_TAIL(series) - index;

	e1 = b1 + (match ? 1 : l1 - (l2 - 1));

	c = *b2; // first char

	if (!uncase) {

		while (b1 != e1) {
			if (*b1 == c) { // matched first char
				for (n = 1; n < l2; n++) {
					if (b1[n] != b2[n]) break;
				}
				if (n == l2) return (b1 - BIN_HEAD(series));
			}
			b1++;
		}

	} else {

		c = (REBYTE)LO_CASE(c); // OK! (never > 255)

		while (b1 != e1) {
			if (LO_CASE(*b1) == c) { // matched first char
				for (n = 1; n < l2; n++) {
					if (LO_CASE(b1[n]) != LO_CASE(b2[n])) break;
				}
				if (n == l2) return (b1 - BIN_HEAD(series));
			}
			b1++;
		}

	}

	return NOT_FOUND;
}


/***********************************************************************
**
*/	REBCNT Find_Str_Str(REBSER *ser1, REBCNT head, REBCNT index, REBCNT tail, REBINT skip, REBSER *ser2, REBCNT index2, REBCNT len, REBCNT flags)
/*
**		General purpose find a substring.
**
**		Supports: forward/reverse with skip, cased/uncase, Unicode/byte.
**
**		Skip can be set positive or negative (for reverse).
**
**		Flags are set according to ALL_FIND_REFS
**
***********************************************************************/
{
	REBUNI c1;
	REBUNI c2;
	REBUNI c3;
	REBCNT n = 0;
	REBOOL uncase = !(flags & AM_FIND_CASE); // uncase = case insenstive

	c2 = GET_ANY_CHAR(ser2, index2); // starting char
	if (uncase && c2 < UNICODE_CASES) c2 = LO_CASE(c2);

	for (; index >= head && index < tail; index += skip) {

		c1 = GET_ANY_CHAR(ser1, index);
		if (uncase && c1 < UNICODE_CASES) c1 = LO_CASE(c1);

		if (c1 == c2) {
			for (n = 1; n < len; n++) {
				c1 = GET_ANY_CHAR(ser1, index+n);
				c3 = GET_ANY_CHAR(ser2, index2+n);
				if (uncase && c1 < UNICODE_CASES && c3 < UNICODE_CASES) {
					if (LO_CASE(c1) != LO_CASE(c3)) break;
				} else {
					if (c1 != c3) break;
				}
			}
			if (n == len) {
				if (flags & AM_FIND_TAIL) return index + len;
				return index;
			}
		}
		if (flags & AM_FIND_MATCH) break;
	}

	return NOT_FOUND;
}


/***********************************************************************
**
*/	REBCNT Find_Str_Char(REBSER *ser, REBCNT head, REBCNT index, REBCNT tail, REBINT skip, REBUNI c2, REBCNT flags)
/*
**		General purpose find a char in a string.
**
**		Supports: forward/reverse with skip, cased/uncase, Unicode/byte.
**
**		Skip can be set positive or negative (for reverse).
**
**		Flags are set according to ALL_FIND_REFS
**
***********************************************************************/
{
	REBUNI c1;
	REBOOL uncase = !GET_FLAG(flags, ARG_FIND_CASE-1); // uncase = case insenstive

	if (uncase && c2 < UNICODE_CASES) c2 = LO_CASE(c2);

	for (; index >= head && index < tail; index += skip) {

		c1 = GET_ANY_CHAR(ser, index);
		if (uncase && c1 < UNICODE_CASES) c1 = LO_CASE(c1);

		if (c1 == c2) return index;

		if GET_FLAG(flags, ARG_FIND_MATCH-1) break;
	}

	return NOT_FOUND;
}


/***********************************************************************
**
*/	REBCNT Find_Str_Bitset(REBSER *ser, REBCNT head, REBCNT index, REBCNT tail, REBINT skip, REBSER *bset, REBCNT flags)
/*
**		General purpose find a bitset char in a string.
**
**		Supports: forward/reverse with skip, cased/uncase, Unicode/byte.
**
**		Skip can be set positive or negative (for reverse).
**
**		Flags are set according to ALL_FIND_REFS
**
***********************************************************************/
{
	REBUNI c1;
	REBOOL uncase = !GET_FLAG(flags, ARG_FIND_CASE-1); // uncase = case insenstive

	for (; index >= head && index < tail; index += skip) {

		c1 = GET_ANY_CHAR(ser, index);

		//if (uncase && c1 < UNICODE_CASES) {
		//	if (Check_Bit(bset, LO_CASE(c1)) || Check_Bit(bset, UP_CASE(c1)))
		//		return index;
		//}
		//else
		if (Check_Bit(bset, c1, uncase)) return index;

		if (flags & AM_FIND_MATCH) break;
	}

	return NOT_FOUND;
}


#ifdef old
/***********************************************************************
**
x*/	REBCNT Match_2_String(REBSER *series, REBCNT index, REBYTE *str, REBCNT len, REBINT uncase)
/*
**		(Evaluate if there is another function to use. ???!!!)
**
**		Used for: PARSE function
**
***********************************************************************/
{
	REBYTE *ser = STR_SKIP(series, index);
	REBCNT tail = series->tail;

	if (uncase) {
		for (;len > 0 && index < tail; index++, len--) {
			if (*ser++ != *str++) return 0;
		}
	} else {
		for (;len > 0 && index < tail; index++, len--) {
			if (LO_CASE(*ser++) != LO_CASE(*str++)) return 0;
		}
	}
	if (len == 0) return index;
	return 0;
}

/***********************************************************************
**
x*/	REBYTE *Match_Str_Part(REBYTE *str, REBYTE *pat, REBCNT len)
/*
**		If the string matches the pattern for the given length
**		return the char string just past the match (in str).
**		Else, return 0.  A case insensitive compare is made.
**
***********************************************************************/
{
	REBYTE *pp = pat;
	REBYTE *cp = str;

	for (;len > 0 && *pp && *cp; pp++, cp++, len--) {
		if (UP_CASE(*pp) != UP_CASE(*cp)) return 0;
	}

	if (len == 0) return cp;
	return 0;
}
#endif


/***********************************************************************
**
*/	REBCNT Count_Lines(REBYTE *bp, REBCNT len)
/*
**		Count lines in a UTF-8 file.
**
***********************************************************************/
{
	REBCNT count = 0;

	for (; len > 0; bp++, len--) {
		if (*bp == CR) {
			count++;
			if (len == 1) break;
			if (bp[1] == LF) bp++, len--;
		}
		else if (*bp == LF) count++;
	}

	return count;
}


/***********************************************************************
**
*/	REBCNT Next_Line(REBYTE **bin)
/*
**		Find next line termination. Advance the bp; return bin length.
**
***********************************************************************/
{
	REBCNT count = 0;
	REBYTE *bp = *bin;

	for (; *bp; bp++) {
		if (*bp == CR) {
			bp++;
			if (*bp == LF) bp++;
			break;
		}
		else if (*bp == LF) {
			bp++;
			break;
		}
		else count++;
	}

	*bin = bp;
	return count;
}
