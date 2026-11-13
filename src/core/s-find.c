/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2026 Rebol Open Source Contributors
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
**  Author:  Carl Sassenrath, Oldes
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
*/	REBINT Compare_Bytes(const REBYTE *b1, const REBYTE *b2, REBCNT len, REBOOL uncase)
/*
**		Compare two byte-wide strings. Return lexical difference.
**
**		Uncase: compare is case-insensitive.
**
***********************************************************************/
{
	REBINT d;

	if (uncase) {
		for (; len > 0; len--, b1++, b2++) {
			d = LO_CASE(*b1) - LO_CASE(*b2);
			if (d != 0) return d;
		}
	}
	else {
		for (; len > 0; len--, b1++, b2++) {
			d = *b1 - *b2;
			if (d != 0) return d;
		}
	}
	return 0;
}


/***********************************************************************
**
*/	const REBYTE *Match_Bytes(const REBYTE *src, const REBYTE *pat)
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
*/	REBFLG Match_Sub_Path(REBSER *s1, REBSER *s2, REBFLG uncase)
/*
**		Compare two file path series, regardless of char size.
**		Return TRUE if s1 is a subpath of s2.
**
***********************************************************************/
{
	REBLEN l1 = s1->tail;
	REBLEN l2 = s2->tail;
	REBU32 c1, c2;

//	Debug_Series(s1);
//	Debug_Series(s2);

	// s1 len must be <= s2 len
	if (l1 > l2) return FALSE;
	const REBYTE* b1 = BIN_DATA(s1);
	const REBYTE* b2 = BIN_DATA(s2);

	for (; l1 > 0 && l2 > 0;) {
		c1 = UTF8_Decode_Codepoint(&b1, &l1);
		c2 = UTF8_Decode_Codepoint(&b2, &l2);
		if (uncase) {
			if (c1 < UNICODE_CASES) c1 = LO_CASE(c1);
			if (c2 < UNICODE_CASES) c2 = LO_CASE(c2);
		}
		if (c1 != c2) return FALSE;
	}
	return (l1 == 0);
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
**		Compare two string values.
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
	if (uncase && (IS_UTF8_SERIES(VAL_SERIES(v1)) || IS_UTF8_SERIES(VAL_SERIES(v2)))) {
		n = Compare_UTF8(VAL_BIN_DATA(v1), VAL_BIN_DATA(v2), len);
		if (n == 0) return 0;
		return (n < 0) ? n + 2 : n - 2;
	}
	else {
		n = Compare_Bytes(VAL_BIN_DATA(v1), VAL_BIN_DATA(v2), len, uncase);
		return (n != 0) ? n : l1 - l2;
	}
}


/***********************************************************************
**
*/	REBINT Compare_UTF8(const REBYTE *s1, const REBYTE *s2, REBCNT l2)
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
	REBCNT l1 = (REBCNT)LEN_BYTES(s1);
	REBINT result = 0;

	for (; l1 > 0 && l2 > 0;) {
		c1 = UTF8_Decode_Codepoint(&s1, &l1);
		c2 = UTF8_Decode_Codepoint(&s2, &l2);
		if (c1 != c2) {
			if (c1 >= UNICODE_CASES || c2 >= UNICODE_CASES ||
				LO_CASE(c1) != LO_CASE(c2)) {
				return (LO_CASE(c1) > LO_CASE(c2)) ? -1 : -3;
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
				if (n == l2) return AS_REBLEN(b1 - BIN_HEAD(series));
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
				if (n == l2) return AS_REBLEN(b1 - BIN_HEAD(series));
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
	REBU32 c1;
	REBU32 c2;
	REBU32 c3;
	REBYTE *str1, *str2;
	REBCNT n = 0;
	const REBOOL uncase = !(flags & AM_FIND_CASE); // uncase = case insenstive

	c2 = GET_UTF8_CHAR(ser2, index2); // starting char
	if (uncase && c2 < UNICODE_CASES) c2 = LO_CASE(c2);
	str1 = BIN_HEAD(ser1);
	str2 = BIN_HEAD(ser2);

	if (IS_UTF8_SERIES(ser1)) {
		while (index >= head && index < tail) {
			str1 = BIN_SKIP(ser1, index);
			str2 = BIN_SKIP(ser2, index2);
			c1 = UTF8_Get_Codepoint(str1);
			if (uncase && c1 < UNICODE_CASES) c1 = LO_CASE(c1);
			if (c1 == c2) {
				REBYTE *end = str1 + len;
				str1 += UTF8_Skip_Forward(str1, 1);
				str2 += UTF8_Skip_Forward(str2, 1);
				while (str1 < end) {
					c1 = UTF8_Get_Codepoint(str1);
					c3 = UTF8_Get_Codepoint(str2);
					if (uncase && c1 < UNICODE_CASES && c3 < UNICODE_CASES) {
						if (LO_CASE(c1) != LO_CASE(c3)) break;
					}
					else {
						if (c1 != c3) break;
					}
					str1 += UTF8_Skip_Forward(str1, 1);
					str2 += UTF8_Skip_Forward(str2, 1);
				}
				if ((str2 - BIN_SKIP(ser2, index2)) == len) {
					if (flags & AM_FIND_TAIL) return index + len;
					return index;
				}
			}
			if (flags & AM_FIND_MATCH) break;
			index = UTF8_Skip(ser1, index, skip);
		}
	}
	else {
		// ser1 is ASCII, so ser2 must also be ASCII to be found
		if (IS_UTF8_SERIES(ser2)) return NOT_FOUND;
		if (uncase) {
			for (; index >= head && index < tail; index += skip) {
				c1 = str1[index];
				if (c1 < UNICODE_CASES) c1 = LO_CASE(c1);
				if (c1 == c2) {
					for (n = 1; n < len; n++) {
						c1 = str1[index + n];
						c3 = str2[index2 + n];
						if (c1 < UNICODE_CASES && c3 < UNICODE_CASES) {
							if (LO_CASE(c1) != LO_CASE(c3)) break;
						}
						else {
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
		}
		else {
			for (; index >= head && index < tail; index += skip) {
				c1 = str1[index];
				if (c1 == c2) {
					for (n = 1; n < len; n++) {
						c1 = str1[index + n];
						c3 = str2[index2 + n];
						if (c1 != c3) break;
					}
					if (n == len) {
						if (flags & AM_FIND_TAIL) return index + len;
						return index;
					}
				}
				if (flags & AM_FIND_MATCH) break;
			}
		}
	}
	return NOT_FOUND;
}

/***********************************************************************
**
*/	REBCNT Find_Str_Tag(REBSER *ser1, REBCNT head, REBCNT index, REBCNT tail, REBINT skip, REBSER *ser2, REBCNT index2, REBCNT len, REBCNT flags)
/*
**		General purpose find a tag in a string.
**
**		Supports: forward/reverse with skip, cased/uncase, Unicode/byte.
**
**		Skip can be set positive or negative (for reverse).
**
**		Flags are set according to ALL_FIND_REFS
**
***********************************************************************/
{
	REBU32 c1;
	REBU32 c2;
	REBYTE *str1, *str2, *end;
	REBCNT n = 0;
	const REBOOL uncase = !(flags & AM_FIND_CASE); // uncase = case insenstive

	str1 = BIN_HEAD(ser1);
	str2 = BIN_HEAD(ser2);

	if (IS_UTF8_SERIES(ser1)) {
		while (index >= head && index < tail) {
			str1 = BIN_SKIP(ser1, index);
			if (str1[0] == '<') {
				str1++;
				str2 = BIN_SKIP(ser2, index2);
				end = str1 + len;
				while (str1 < end) {
					c1 = UTF8_Get_Codepoint(str1);
					c2 = UTF8_Get_Codepoint(str2);
					if (uncase && c1 < UNICODE_CASES && c2 < UNICODE_CASES) {
						if (LO_CASE(c1) != LO_CASE(c2)) break;
					}
					else {
						if (c1 != c2) break;
					}
					str1 += UTF8_Skip_Forward(str1, 1);
					str2 += UTF8_Skip_Forward(str2, 1);
				}
				if (str1 == end) {
					c1 = UTF8_Get_Codepoint(str1);
					if (c1 == '>') {
						if (flags & AM_FIND_TAIL) return index + len + 2;
						return index;
					}
				}
			}
			if (flags & AM_FIND_MATCH) break;
			index = UTF8_Skip(ser1, index, skip);
		}
	}
	else { // ASCII version
		if (IS_UTF8_SERIES(ser2)) return NOT_FOUND;
		while (index >= head && index < tail) {
			str1 = BIN_SKIP(ser1, index);
			if (str1[0] == '<') {
				str1++;
				str2 = BIN_SKIP(ser2, index2);
				end = str1 + len;
				while (str1 < end) {
					c1 = str1[0];
					c2 = str2[0];
					if (uncase && c1 < UNICODE_CASES && c2 < UNICODE_CASES) {
						if (LO_CASE(c1) != LO_CASE(c2)) break;
					}
					else {
						if (c1 != c2) break;
					}
					++str1; ++str2;
				}
				if (str1 == end) {
					if (str1[0] == '>') {
						if (flags & AM_FIND_TAIL) return index + len + 2;
						return index;
					}
				}
			}
			if (flags & AM_FIND_MATCH) break;
			index += skip;
		}
	}
	return NOT_FOUND;
}


/***********************************************************************
**
*/	REBCNT Find_Str_Str_Any(REBSER *ser1, REBCNT head, REBCNT index, REBCNT tail, REBINT skip, REBSER *ser2, REBCNT index2, REBCNT len, REBCNT flags, REBVAL *wild)
/*
**		General purpose find a substring with wildcards.
**
**		Supports: forward/reverse with skip, cased/uncase, Unicode/byte.
**
**		Skip can be set positive or negative (for reverse).
**
**		Flags are set according to ALL_FIND_REFS
**
***********************************************************************/
{
	REBU32 c1;
	REBU32 c2;
	REBU32 c3 = 0;
	REBCNT n = 0, start = 0, pos = 0;
	REBCNT sn = 0;
	REBOOL uncase = !(flags & AM_FIND_CASE); // uncase = case insenstive
	REBU32 c_some = '*';
	REBU32 c_one  = '?';

	if (IS_STRING(wild)) {
		if (VAL_INDEX(wild)   < VAL_TAIL(wild)) c_some = GET_UTF8_CHAR(VAL_SERIES(wild), VAL_INDEX(wild));
		if (VAL_INDEX(wild)+1 < VAL_TAIL(wild)) c_one  = GET_UTF8_CHAR(VAL_SERIES(wild), VAL_INDEX(wild)+1);
	}

	c2 = GET_UTF8_CHAR(ser2, index2); // starting char
	if (uncase && c2 < UNICODE_CASES) c2 = LO_CASE(c2);

	for (; index >= head && index < tail; index = UTF8_Skip(ser1, index, skip)) {
		n = UTF8_Codepoint_Size(c2);
		pos = index;
		if (c2 == c_some) {
			n = 0;
			goto some_loop;
		}
		start = pos;
		if (c2 == c_one) {
			c1 = c2;
		} else {
			c1 = GET_UTF8_CHAR(ser1, index);
			if (uncase && c1 < UNICODE_CASES) c1 = LO_CASE(c1);
		}
		if (c1 == c2) { // found first needle's char
			pos = UTF8_Skip(ser1, pos, 1);
			while (n < len && pos < tail) {
				c1 = GET_UTF8_CHAR(ser1, pos);
				c3 = GET_UTF8_CHAR(ser2, index2 + n);
				// printf("  %c == %c\n", c1, c3);
				if (c3 == c_some) {
				some_loop:
					while (n < len) {
						// skip all * and ? chars in needle
						c3 = GET_UTF8_CHAR(ser2, index2 + n);
						if (c3 != c_some && c3 != c_one) break;
						n += UTF8_Codepoint_Size(c3);
					}
					if (n == len) {
						// * was at tail, so we can resolve it as found
						pos = (skip > 0) ? tail: start;
						goto found;
					}
					sn = n; // store a new needle's start (thru the last found *)
				some_next:
					// skip in 'hay' all chars until found next needle's char
					while (1) {
						if (pos < head || pos >= tail) return NOT_FOUND;
						c1 = GET_UTF8_CHAR(ser1, pos);
						// printf("* %c == %c\n", c1, c3);
						if (c1 == c3) goto next_char;
						if (uncase && c1 < UNICODE_CASES && c3 < UNICODE_CASES) {
							if (LO_CASE(c1) == LO_CASE(c3)) goto next_char;
						}
						index = UTF8_Skip(ser1, index, 1);
						pos += UTF8_Codepoint_Size(c1);
					}
				} else if (c3 == c_one) {
					goto next_char;
				}
				if (uncase && c1 < UNICODE_CASES && c3 < UNICODE_CASES) {
					if (LO_CASE(c1) != LO_CASE(c3)) {
						if (sn) {
							n = sn; // reset needles position to the last know * char
							c3 = GET_UTF8_CHAR(ser2, index2 + n);
							goto some_next;
						}
						else break;
					}
				}
				else {
					if (c1 != c3) {
						if (sn) { n = sn; goto next_char; }
						else break;
					}
				}
			next_char:
				pos = UTF8_Skip(ser1, pos, 1);
				n += UTF8_Codepoint_Size(c3);
			}
			if (n == len) {
			found:
				return (flags & AM_FIND_TAIL) ? pos : start;
			}
		}
		if (flags & AM_FIND_MATCH) break;
	}

	return NOT_FOUND;
}

/***********************************************************************
**
*/	REBCNT Find_Str_Char(REBSER *ser, REBCNT head, REBCNT index, REBCNT tail, REBINT skip, REBU32 c2, REBCNT flags)
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
	REBU32 c1;
	REBYTE *bp;
	const REBOOL uncase = !(flags & AM_FIND_CASE); // uncase = case insenstive

	if (uncase && c2 < UNICODE_CASES) c2 = LO_CASE(c2);

	if (IS_UTF8_SERIES(ser)) {
		while (index >= head && index < tail) {
			bp = BIN_SKIP(ser, index);
			c1 = UTF8_Get_Codepoint(bp);
			if (uncase && c1 < UNICODE_CASES) c1 = LO_CASE(c1);
			if (c1 == c2) {
				if (flags & AM_FIND_TAIL) {
					index += UTF8_Next_Char_Size(bp, 0);
				}
				return index;
			}
			if (flags & AM_FIND_MATCH) break;
			index = UTF8_Skip(ser, index, skip);
		}
	}
	else {
		if (c2 > 0x7F) return NOT_FOUND;
		bp = BIN_HEAD(ser);
		for (; index >= head && index < tail; index += skip) {
			c1 = bp[index];
			if (uncase && c1 < UNICODE_CASES) c1 = LO_CASE(c1);
			if (c1 == c2) {
				if (flags & AM_FIND_TAIL) index++;
				return index;
			}
			if (flags & AM_FIND_MATCH) break;
		}
	}
	return NOT_FOUND;
}


/***********************************************************************
**
*/	REBCNT Find_Str_Bitset(const REBSER *ser, REBCNT head, REBCNT index, REBCNT tail, REBINT skip, const REBSER *bset, REBCNT flags)
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
	REBU32 chr;
	const REBYTE *str = BIN_HEAD(ser);

	const REBOOL uncase = !(flags & AM_FIND_CASE); // uncase = case insenstive

	if (IS_UTF8_SERIES(ser)) {
		while (index >= head && index < tail) {
			str = BIN_SKIP(ser, index);
			chr = UTF8_Get_Codepoint(str);
			if (Check_Bit(bset, chr, uncase)) {
				if (flags & AM_FIND_TAIL) {
					index += UTF8_Next_Char_Size(str, 0);
				}
				return index;
			}
			if (flags & AM_FIND_MATCH) break;
			index = UTF8_Skip(ser, index, skip);
		}
	}
	else {
		for (; index >= head && index < tail; index += skip) {
			if (Check_Bit(bset, str[index], uncase)) {
				if (flags & AM_FIND_TAIL) index++;
				return index;
			}
			if (flags & AM_FIND_MATCH) break;
		}
	}

	return NOT_FOUND;
}


/***********************************************************************
**
*/	REBCNT Find_Str_Wild(REBSER *ser, REBCNT index, REBCNT tail)
/*
**		Returns index of first * or ? chars in series 
**
***********************************************************************/
{
	REBYTE ch;
	REBYTE *str = BIN_HEAD(ser);

	for (; index < tail; index++) {
		ch = str[index];
		if (ch == '*' || ch == '?') return index;
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
