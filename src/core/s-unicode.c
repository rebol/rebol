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
**  Module:  s-unicode.c
**  Summary: unicode support functions
**  Section: strings
**  Author:  Carl Sassenrath, Oldes
**  Notes:
**    UTF-8 decoding uses code by Bjoern Hoehrmann.
**    The remaining code was added or adapted by REBOL Technologies
**    and Rebol Open Source Contributors.
**
***********************************************************************/


#include "sys-core.h"
#include <wchar.h>

// Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

static const REBYTE utf8d[] = {
  // The first part of the table maps bytes to character classes that
  // to reduce the size of the transition table and create bitmasks.
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0x00 - 0x1F
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0x20 - 0x2F
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0x40 - 0x4F
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0x60 - 0x6F
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 0x80 - 0x8F
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // 0xA0 - 0xAF
   8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // 0xC0 - 0xCF
  10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8, // 0xE0 - 0xEF

  // The second part is a transition table that maps a combination
  // of a state of the automaton and a character class to a state.
   0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
  12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
  12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
  12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
  12,36,12,12,12,12,12,12,12,12,12,12, 
};

//static REBYTE const u8_length[] = {
//	// 0 1 2 3 4 5 6 7 8 9 A B C D E F
//	   1,1,1,1,1,1,1,1,0,0,0,0,2,2,3,4
//};


// Helper to write a UTF-32 code point with specified endianness
FORCE_INLINE
static void write_u32(REBYTE *dst, REBU32 codepoint, int is_little_endian) {
	if (is_little_endian) {
		dst[0] = (REBYTE)(codepoint & 0xFF);
		dst[1] = (REBYTE)((codepoint >> 8) & 0xFF);
		dst[2] = (REBYTE)((codepoint >> 16) & 0xFF);
		dst[3] = (REBYTE)((codepoint >> 24) & 0xFF);
	}
	else {
		dst[0] = (REBYTE)((codepoint >> 24) & 0xFF);
		dst[1] = (REBYTE)((codepoint >> 16) & 0xFF);
		dst[2] = (REBYTE)((codepoint >> 8) & 0xFF);
		dst[3] = (REBYTE)(codepoint & 0xFF);
	}
}
FORCE_INLINE
static void write_u16(REBYTE *dst, REBU32 codepoint, int is_little_endian) {
	if (is_little_endian) {
		dst[0] = (REBYTE)(codepoint & 0xFF);
		dst[1] = (REBYTE)((codepoint >> 8) & 0xFF);
	}
	else {
		dst[0] = (REBYTE)((codepoint >> 8) & 0xFF);
		dst[1] = (REBYTE)(codepoint & 0xFF);
	}
}
// Helper to read a 16-bit code unit with endianness
FORCE_INLINE
static REBU16 read_u16(const REBYTE *src, int is_little_endian) {
	if (is_little_endian)
		return (REBU16)src[0] | ((REBU16)src[1] << 8);
	else
		return ((REBU16)src[0] << 8) | (REBU16)src[1];
}

// Helper to read a 32-bit code unit with endianness
FORCE_INLINE
static REBU32 read_u32(const REBYTE *src, int is_little_endian) {
	if (is_little_endian)
		return (REBU32)src[0] | ((REBU32)src[1] << 8) |
		((REBU32)src[2] << 16) | ((REBU32)src[3] << 24);
	else
		return ((REBU32)src[0] << 24) | ((REBU32)src[1] << 16) |
		((REBU32)src[2] << 8) | (REBU32)src[3];
}


FORCE_INLINE
/***********************************************************************
**
*/	REBCNT UTF8_Codepoint_Size(REBU32 codepoint)
/*
**		Returns the size of the given codepoint in bytes.
**
***********************************************************************/
{
	if (codepoint <= 0x7F) return 1;
	if (codepoint <= 0x7FF) return 2;
	if (codepoint <= 0xFFFF) return 3;
	return 4;
}

FORCE_INLINE
/***********************************************************************
**
*/	REBCNT UTF8_Next_Char_Size(const REBYTE *str, REBLEN index)
/*
**		Returns the size of the current UTF8 char in bytes
**
***********************************************************************/
{
	REBYTE c = str[index];

	if ((c & 0x80) == 0) return 1;     // ASCII (0xxxxxxx)
	if ((c & 0xE0) == 0xC0) return 2;   // 2-byte sequence (110xxxxx)
	if ((c & 0xF0) == 0xE0) return 3;   // 3-byte sequence (1110xxxx)
	if ((c & 0xF8) == 0xF0) return 4;   // 4-byte sequence (11110xxx)
	return 1; // Fallback for invalid/continuation bytes
}

FORCE_INLINE
/***********************************************************************
**
*/	REBCNT UTF8_Decode_Step(REBCNT* state, REBCNT* codep, REBCNT byte)
/*
***********************************************************************/
{
  REBCNT type = utf8d[byte];

  *codep = (*state != UTF8_ACCEPT) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

  *state = utf8d[256 + *state + type];
  return *state;
}

FORCE_INLINE
/***********************************************************************
**
*/	REBCNT UTF8_Prev_Char_Position(const REBYTE *str, REBLEN index)
/*
**		Returns the index of the previous UTF-8 character
**
***********************************************************************/
{
	while (index > 0 && (str[--index] & 0xC0) == 0x80);
	return index;
}

FORCE_INLINE
/***********************************************************************
**
*/	REBLEN UTF8_Prev_Char_Size(const REBYTE *str, REBLEN index)
/*
**		Returns the number of bytes in the previous UTF-8 character.
**
***********************************************************************/
{
	REBLEN start = index;
	while (index > 0 && (str[--index] & 0xC0) == 0x80);
	return start - index;
}

/***********************************************************************
**
*/	REBCNT UTF8_Index_To_Position(const REBYTE *str, REBLEN index)
/*
**		Returns number of codepoints which UTF8 string have at the given position
**
***********************************************************************/
{
	REBCNT pos = 0;
	const REBYTE *end = str + index;
	while (*str && str < end) {
		pos += (*str++ & 0xC0) != 0x80;
	}
	return pos;
}

FORCE_INLINE
/***********************************************************************
**
*/	REBCNT UTF8_Skip_Forward(const REBYTE *str, REBLEN chars)
/*
**		Return number of bytes needed for given number of chars forward.
**
***********************************************************************/
{
	REBLEN index = 0;
	while (chars-- > 0 && str[index]) {
		index += UTF8_Next_Char_Size(str, index);
	}
	return index;
}

FORCE_INLINE
/***********************************************************************
**
*/	REBCNT UTF8_Skip(const REBSER *ser, REBCNT index, REBINT chars)
/*
**		Return position in series after skipping number of chars forward or reverse.
**
***********************************************************************/
{
	REBYTE *head = BIN_HEAD(ser);
	if (chars > 0) {
		while (chars-- > 0 && head[index]) {
			index += UTF8_Next_Char_Size(head, index);
		}
	}
	else {
		while (index > 0 && chars < 0) {
			chars++;
			index -= UTF8_Prev_Char_Size(head, index);
		}
		if (chars != 0) index = UNKNOWN;
	}
	return index;
}


FORCE_INLINE
/***********************************************************************
**
*/	REBCNT UTF8_Validate_Index(const REBYTE *str, REBLEN index)
/*
**		Returns the index of the previous UTF-8 character
**
***********************************************************************/
{
	do { index--; } while (index > 0 && (str[index] & 0xC0) == 0x80);
	return index;
}


/***********************************************************************
**
*/	REBLEN UTF8_Bytes_For_Char_Count(const REBYTE *str, REBLEN tail, REBLEN len)
/*
**		Returns number of bytes consumed by given number of UTF8 chars in a string
**
***********************************************************************/
{
	REBLEN pos = 0;
	while (pos < tail && len-- > 0) {
		pos += UTF8_Next_Char_Size(str, pos);
	}
	return pos;
}

/***********************************************************************
**
*/	REBLEN UTF8_Bytes_For_Char_Count_Back(const REBYTE *str, REBLEN index, REBLEN num)
/*
**		Returns number of bytes consumed by given number of UTF8 chars in a string
**
***********************************************************************/
{
	REBLEN size;
	while (index > 0 && num-- > 0) {
		size = UTF8_Prev_Char_Size(str, index);
		ASSERT1(size <= index, RP_BAD_SIZE);
		index -= size;
	}
	return index;
}

FORCE_INLINE
/***********************************************************************
**
*/	REBCNT UTF8_Get_Codepoint(const REBYTE *src)
/*
**		Gets a single UTF8 code-point (to 32 bit).
**
***********************************************************************/
{
	REBCNT codepoint = 0;
	REBCNT state = 0;

	for (; *src; ++src) {
		// read bytes until codepoint is not complete or invalid...
		if (UTF8_Decode_Step(&state, &codepoint, *src)) {
			if (state != UTF8_REJECT) continue; // not yet complete
			break;
		}
		break; // only one codepoint
	}
	if (state != UTF8_ACCEPT) codepoint = UNI_REPLACEMENT_CHAR;
	return codepoint;
}

FORCE_INLINE
/***********************************************************************
**
*/	void UTF8_Replace_Codepoint(REBSER *ser, REBLEN index, REBU32 codepoint)
/*
**		Replace codepoint at given index with a new one.
**
***********************************************************************/
{
	REBLEN s1 = UTF8_Next_Char_Size(STR_HEAD(ser), index);
	REBLEN s2 = UTF8_Codepoint_Size(codepoint);
	if (s2 > s1)
		Expand_Series(ser, index, s2 - s1);
	else if (s2 < s1)
		Remove_Series(ser, index, s1 - s2);
	Encode_UTF8_Char(STR_SKIP(ser, index), codepoint);
}

/***********************************************************************
**
*/	REBU32 Decode_Surrogate_Pair(const REBYTE *src)
/*
**		Returns codepoint decoded from surrogate pair or UNI_ERROR.
**
***********************************************************************/
{
	REBU32 c1 = ((src[0] & 0x0F) << 12) | ((src[1] & 0x3F) << 6) | (src[2] & 0x3F);
	REBU32 c2 = ((src[3] & 0x0F) << 12) | ((src[4] & 0x3F) << 6) | (src[5] & 0x3F);
	if (c1 >= 0xD800 && c1 <= 0xDBFF && c2 >= 0xDC00 && c2 <= 0xDFFF) {
		return 0x10000 + ((c1 - 0xD800) << 10) + (c2 - 0xDC00);
	}
	return UNI_ERROR;
}

/***********************************************************************
**
*/	const REBYTE *UTF8_Check(const REBYTE *str, REBCNT len, REBFLG *surrogates)
/*
**		Returns 0 for success, else str where error occurred.
**
***********************************************************************/
{
	if (len == 0) return 0;
	const REBYTE *end = str + len;
	const REBYTE *acc = str - 1;
	REBCNT codepoint = 0;
	REBCNT state = UTF8_ACCEPT;
	*surrogates = FALSE;

	for (; str < end; ++str) {
		switch (UTF8_Decode_Step(&state, &codepoint, *str)) {
		case UTF8_ACCEPT: acc = str; break; // remember last accepted char position
		case UTF8_REJECT:
			if (Decode_Surrogate_Pair(str - 1) == UNI_ERROR) {
				return acc + 1;
			}
			*surrogates = TRUE;
			str += 4;
			acc = str;
			state = UTF8_ACCEPT;

			break;
		}
	}
	if (state == UTF8_ACCEPT) return 0;
	// if state is not accepted, we must have incomplete utf-8 sequence
	// not using str-1, because the sequence may have more than 2 bytes!
	return acc + 1;
}


/***********************************************************************
**
*/	REBSER *UTF8_Copy_Surrogates(const REBYTE *str, REBCNT len, REBCNT *err)
/*
**		Copy UTF-8 string while collapsing all surrogate pairs.
**
***********************************************************************/
{
	if (len == 0) return 0;
	const REBYTE *start = str;
	const REBYTE *end = str + len;
	const REBYTE *acc = start;
	REBU32 codepoint = 0;
	REBCNT state = UTF8_ACCEPT;
	REBLEN nlen = 0; // Used to shorten the original length in case of surrogate pairs

	REBSER *dst = Make_Series(len, 1, FALSE);

	for (; str < end; str++) {
		switch (UTF8_Decode_Step(&state, &codepoint, *str)) {
		case UTF8_ACCEPT: acc = str + 1; break; // remember last accepted char position
		case UTF8_REJECT:
			codepoint = Decode_Surrogate_Pair(str - 1);
			if (codepoint != UNI_ERROR) {
				REBLEN bytes = AS_REBLEN(str - 1 - start);
				if (bytes > 0) {
					// Copy all already validated chars to the outpout
					Append_Bytes_Len(dst, start, bytes);
				}
				Append_Byte(dst, codepoint);
				str += 5;
				nlen += 6 - UTF8_Codepoint_Size(codepoint);
				start = str;
				state = UTF8_ACCEPT;
			}
			break;
		}
	}
	if (state == UTF8_ACCEPT) {
		if (start < end) Append_Bytes_Len(dst, start, AS_REBLEN(end - start));
		return dst;
	}
	else {
		// if state is not accepted, we must have incomplete utf-8 sequence or error!
		if (err) *err = AS_REBLEN(acc - start);
		return NULL;
	}
	SERIES_TAIL(dst) -= nlen;
	return dst;
}

/***********************************************************************
**
*/	REBU32 UTF8_Decode_Codepoint(const REBYTE **RESTRICT str, REBCNT *RESTRICT len)
/*
**		Converts a single UTF8 code-point (to 32 bit).
**		Errors are returned as zero. (So prescan source for null.)
**		Increments str by extra chars needed.
**		Decrements len by extra chars needed.
**
***********************************************************************/
{
	REBYTE *src = (REBYTE *)*str;
	REBU32 codepoint = 0;
	REBCNT state = 0;
	REBCNT bytes = *len;

	for (; bytes > 0; ++src, --bytes) {
		// read bytes until codepoint is not complete or invalid...
		if (UTF8_Decode_Step(&state, &codepoint, *src)) {
			if (state != UTF8_REJECT) continue; // not yet complete
			// on reject, try to decode surrogate pair...
			if (bytes >= 5) {
				codepoint = Decode_Surrogate_Pair(src - 1);
				if (codepoint != UNI_ERROR) {
					src += 5;
					bytes -= 5;
					goto done;
				}
			}
		}
		++src; --bytes;
		break; // only one codepoint
	}
	if (state != UTF8_ACCEPT) codepoint = UNI_ERROR;
done:
	*len = bytes;
	*str = src;
	return codepoint;
}

/***********************************************************************
**
*/	REBSER *UTF8_To_UTF16(REBSER *dst_ser, const REBYTE *str, REBCNT len, REBFLG little_endian)
/*
**		Converts UTF-8 encoded byte stream to UTF-16 (UCS2) array.
**		If dst_ser is NULL, a new series is created.
**		If len == -1, the input size is determined using a null char.
**
***********************************************************************/
{
	REBLEN  dst_len = 0; // expected destination length in bytes
	REBLEN  src_len = 0;
	REBYTE *dst_bin;
	REBU32 codepoint;

	const REBYTE *bp = str;

	if (len == UNKNOWN) len = LEN_BYTES(str);

	src_len = len;
	// Count number of bytes needed...
	while (src_len > 0) {
		codepoint = UTF8_Decode_Codepoint(&bp, &src_len);
		if (codepoint <= 0xFFFF) {
			dst_len += 2; // BMP character
		}
		else if (codepoint <= 0x10FFFF) {
			dst_len += 4; // Surrogate pair needed
		}
	}
	dst_len += 2; // For NULL

	if (!dst_ser)
		dst_ser = Make_Series(dst_len, 1, FALSE);
	else
		Expand_Series(dst_ser, 0, dst_len);

	dst_bin = BIN_HEAD(dst_ser);
	src_len = len;
	bp = str;

	while (src_len > 0) {
		codepoint = UTF8_Decode_Codepoint(&bp, &src_len);
		if (codepoint <= 0xFFFF) {
			// Skip codepoints in surrogate range?
			// if (codepoint < 0xD800 || codepoint > 0xDFFF) ...
			write_u16(dst_bin, codepoint, little_endian);
			dst_bin += 2;
		}
		else if (codepoint <= 0x10FFFF) {
			REBU32 temp = codepoint - 0x10000;  // Remove the offset for supplementary planes
			write_u16(dst_bin, 0xD800 | (temp >> 10), little_endian);
			dst_bin += 2;
			write_u16(dst_bin, 0xDC00 | (temp & 0x3FF), little_endian);
			dst_bin += 2;
		}
	}
	SERIES_TAIL(dst_ser) = AS_REBLEN(dst_bin - BIN_HEAD(dst_ser));
	// Terminate... (don't use UNI_TERM as the series is not really UNI)
	write_u16(dst_bin, 0, little_endian);
	dst_bin += 2;
	return dst_ser;
}


/***********************************************************************
**
*/	REBSER* UTF8_To_UTF32(REBSER *dst_ser, const REBYTE *str, REBCNT len, REBFLG little_endian)
/*
***********************************************************************/
{
	REBLEN  dst_len = 0;
	REBYTE *dst_bin;
	REBU32 codepoint;

	const REBYTE *bp = str;

	while (*bp) dst_len += (*bp++ & 0xC0) != 0x80;

	if (!dst_ser)
		dst_ser = Make_Series((dst_len + 1) * 4, 1, FALSE);
	else
		Expand_Series(dst_ser, 0, dst_len);

	dst_bin = BIN_HEAD(dst_ser);

	bp = str;
	while (*bp && len > 0) {
		codepoint = UTF8_Decode_Codepoint(&bp, &len);
		write_u32(dst_bin, codepoint, little_endian);
		dst_bin+=4;
	}
	SERIES_TAIL(dst_ser) = AS_REBLEN(dst_bin - BIN_HEAD(dst_ser));
	return dst_ser;
}


/***********************************************************************
**
*/	REBSER *UTF32_To_UTF8(REBSER *dst_ser, const REBYTE *str, REBCNT len, REBFLG little_endian)
/*
***********************************************************************/
{
	REBLEN  dst_len = 0, i;
	REBYTE *dst_bin;

	const REBU32 *uni = (REBU32*)str;
	const REBLEN uni_len = len / 4;

	for (i = 0; i < uni_len; i++) {
		dst_len += UTF8_Codepoint_Size(uni[i]);
	}

	if (!dst_ser)
		dst_ser = Make_Series(dst_len+1, 1, FALSE);
	else
		Expand_Series(dst_ser, 0, dst_len);

	dst_bin = BIN_HEAD(dst_ser);


	for (i = 0; i < uni_len; i++) {
		dst_bin += Encode_UTF8_Char(dst_bin, uni[i]);
	}
	SERIES_TAIL(dst_ser) = AS_REBLEN(dst_bin - BIN_HEAD(dst_ser));
	return dst_ser;
}

// -------------------------------------------------------------------------
// Cross-platform wcwidth implementation
// Ranges collected using:
// https://gist.github.com/Oldes/2f31b16f333151744991b22a3e15e792

#define DEFINE_IN_RANGE(TYPE)                                \
struct utf8range_##TYPE {                                    \
TYPE lower;       /* lower inclusive */                      \
TYPE upper;       /* upper inclusive */                      \
};                                                           \
static int utf8_in_range_##TYPE(                             \
    const struct utf8range_##TYPE *ranges, int num, TYPE ch) \
{                                                            \
    int lo = 0, hi = num - 1;                                \
    while (lo <= hi) {                                       \
        int mid = (lo + hi) >> 1;                            \
        if      (ch < ranges[mid].lower) hi = mid - 1;       \
        else if (ch > ranges[mid].upper) lo = mid + 1;       \
        else return 1;                                       \
    }                                                        \
    return 0;                                                \
}
DEFINE_IN_RANGE(u16)
DEFINE_IN_RANGE(u32)
#undef DEFINE_IN_RANGE

/* From https://unicode.org/Public/UNIDATA/UnicodeData.txt */
static const struct utf8range_u16 unicode_zero_u16[] = {
  {0x0000,0x001F},
  {0x007F,0x009F},
//{0x00AD,0x00AD}, // ??? https://github.com/jquast/wcwidth/issues/8
													 {0x0300,0x036F}, {0x0483,0x0489}, {0x0591,0x05BD},
  {0x05BF,0x05BF}, {0x05C1,0x05C2}, {0x05C4,0x05C5}, {0x05C7,0x05C7}, {0x0600,0x0605}, {0x0610,0x061A},
  {0x061C,0x061C}, {0x064B,0x065F}, {0x0670,0x0670}, {0x06D6,0x06DD}, {0x06DF,0x06E4}, {0x06E7,0x06E8},
  {0x06EA,0x06ED}, {0x070F,0x070F}, {0x0711,0x0711}, {0x0730,0x074A}, {0x07A6,0x07B0}, {0x07EB,0x07F3},
  {0x07FD,0x07FD}, {0x0816,0x0819}, {0x081B,0x0823}, {0x0825,0x0827}, {0x0829,0x082D}, {0x0859,0x085B},
  {0x0890,0x089F}, {0x08CA,0x0902}, {0x093A,0x093A}, {0x093C,0x093C}, {0x0941,0x0948}, {0x094D,0x094D},
  {0x0951,0x0957}, {0x0962,0x0963}, {0x0981,0x0981}, {0x09BC,0x09BC}, {0x09C1,0x09C4}, {0x09CD,0x09CD},
  {0x09E2,0x09E3}, {0x09FE,0x0A02}, {0x0A3C,0x0A3C}, {0x0A41,0x0A51}, {0x0A70,0x0A71}, {0x0A75,0x0A75},
  {0x0A81,0x0A82}, {0x0ABC,0x0ABC}, {0x0AC1,0x0AC8}, {0x0ACD,0x0ACD}, {0x0AE2,0x0AE3}, {0x0AFA,0x0B01},
  {0x0B3C,0x0B3C}, {0x0B3F,0x0B3F}, {0x0B41,0x0B44}, {0x0B4D,0x0B56}, {0x0B62,0x0B63}, {0x0B82,0x0B82},
  {0x0BC0,0x0BC0}, {0x0BCD,0x0BCD}, {0x0C00,0x0C00}, {0x0C04,0x0C04}, {0x0C3C,0x0C3C}, {0x0C3E,0x0C40},
  {0x0C46,0x0C56}, {0x0C62,0x0C63}, {0x0C81,0x0C81}, {0x0CBC,0x0CBC}, {0x0CBF,0x0CBF}, {0x0CC6,0x0CC6},
  {0x0CCC,0x0CCD}, {0x0CE2,0x0CE3}, {0x0D00,0x0D01}, {0x0D3B,0x0D3C}, {0x0D41,0x0D44}, {0x0D4D,0x0D4D},
  {0x0D62,0x0D63}, {0x0D81,0x0D81}, {0x0DCA,0x0DCA}, {0x0DD2,0x0DD6}, {0x0E31,0x0E31}, {0x0E34,0x0E3A},
  {0x0E47,0x0E4E}, {0x0EB1,0x0EB1}, {0x0EB4,0x0EBC}, {0x0EC8,0x0ECE}, {0x0F18,0x0F19}, {0x0F35,0x0F35},
  {0x0F37,0x0F37}, {0x0F39,0x0F39}, {0x0F71,0x0F7E}, {0x0F80,0x0F84}, {0x0F86,0x0F87}, {0x0F8D,0x0FBC},
  {0x0FC6,0x0FC6}, {0x102D,0x1030}, {0x1032,0x1037}, {0x1039,0x103A}, {0x103D,0x103E}, {0x1058,0x1059},
  {0x105E,0x1060}, {0x1071,0x1074}, {0x1082,0x1082}, {0x1085,0x1086}, {0x108D,0x108D}, {0x109D,0x109D},
  {0x135D,0x135F}, {0x1712,0x1714}, {0x1732,0x1733}, {0x1752,0x1753}, {0x1772,0x1773}, {0x17B4,0x17B5},
  {0x17B7,0x17BD}, {0x17C6,0x17C6}, {0x17C9,0x17D3}, {0x17DD,0x17DD}, {0x180B,0x180F}, {0x1885,0x1886},
  {0x18A9,0x18A9}, {0x1920,0x1922}, {0x1927,0x1928}, {0x1932,0x1932}, {0x1939,0x193B}, {0x1A17,0x1A18},
  {0x1A1B,0x1A1B}, {0x1A56,0x1A56}, {0x1A58,0x1A60}, {0x1A62,0x1A62}, {0x1A65,0x1A6C}, {0x1A73,0x1A7F},
  {0x1AB0,0x1B03}, {0x1B34,0x1B34}, {0x1B36,0x1B3A}, {0x1B3C,0x1B3C}, {0x1B42,0x1B42}, {0x1B6B,0x1B73},
  {0x1B80,0x1B81}, {0x1BA2,0x1BA5}, {0x1BA8,0x1BA9}, {0x1BAB,0x1BAD}, {0x1BE6,0x1BE6}, {0x1BE8,0x1BE9},
  {0x1BED,0x1BED}, {0x1BEF,0x1BF1}, {0x1C2C,0x1C33}, {0x1C36,0x1C37}, {0x1CD0,0x1CD2}, {0x1CD4,0x1CE0},
  {0x1CE2,0x1CE8}, {0x1CED,0x1CED}, {0x1CF4,0x1CF4}, {0x1CF8,0x1CF9}, {0x1DC0,0x1DFF}, {0x200B,0x200F},
  {0x202A,0x202E}, {0x2060,0x206F}, {0x20D0,0x20F0}, {0x2CEF,0x2CF1}, {0x2D7F,0x2D7F}, {0x2DE0,0x2DFF},
  {0x302A,0x302D}, {0x3099,0x309A}, {0xA66F,0xA672}, {0xA674,0xA67D}, {0xA69E,0xA69F}, {0xA6F0,0xA6F1},
  {0xA802,0xA802}, {0xA806,0xA806}, {0xA80B,0xA80B}, {0xA825,0xA826}, {0xA82C,0xA82C}, {0xA8C4,0xA8C5},
  {0xA8E0,0xA8F1}, {0xA8FF,0xA8FF}, {0xA926,0xA92D}, {0xA947,0xA951}, {0xA980,0xA982}, {0xA9B3,0xA9B3},
  {0xA9B6,0xA9B9}, {0xA9BC,0xA9BD}, {0xA9E5,0xA9E5}, {0xAA29,0xAA2E}, {0xAA31,0xAA32}, {0xAA35,0xAA36},
  {0xAA43,0xAA43}, {0xAA4C,0xAA4C}, {0xAA7C,0xAA7C}, {0xAAB0,0xAAB0}, {0xAAB2,0xAAB4}, {0xAAB7,0xAAB8},
  {0xAABE,0xAABF}, {0xAAC1,0xAAC1}, {0xAAEC,0xAAED}, {0xAAF6,0xAAF6}, {0xABE5,0xABE5}, {0xABE8,0xABE8},
  {0xABED,0xABED}, {0xFB1E,0xFB1E}, {0xFE00,0xFE0F}, {0xFE20,0xFE2F}, {0xFEFF,0xFEFF}, {0xFFF9,0xFFFB},
};
static const struct utf8range_u32 unicode_zero_u32[] = {
 {0x101FD,0x101FD}, {0x102E0,0x102E0}, {0x10376,0x1037A}, {0x10A01,0x10A0F}, {0x10A38,0x10A3F}, {0x10AE5,0x10AE6},
 {0x10D24,0x10D27}, {0x10D69,0x10D6D}, {0x10EAB,0x10EAC}, {0x10EFA,0x10EFF}, {0x10F46,0x10F50}, {0x10F82,0x10F85},
 {0x11001,0x11001}, {0x11038,0x11046}, {0x11070,0x11070}, {0x11073,0x11074}, {0x1107F,0x11081}, {0x110B3,0x110B6},
 {0x110B9,0x110BA}, {0x110BD,0x110BD}, {0x110C2,0x110CD}, {0x11100,0x11102}, {0x11127,0x1112B}, {0x1112D,0x11134},
 {0x11173,0x11173}, {0x11180,0x11181}, {0x111B6,0x111BE}, {0x111C9,0x111CC}, {0x111CF,0x111CF}, {0x1122F,0x11231},
 {0x11234,0x11234}, {0x11236,0x11237}, {0x1123E,0x1123E}, {0x11241,0x11241}, {0x112DF,0x112DF}, {0x112E3,0x112EA},
 {0x11300,0x11301}, {0x1133B,0x1133C}, {0x11340,0x11340}, {0x11366,0x11374}, {0x113BB,0x113C0}, {0x113CE,0x113CE},
 {0x113D0,0x113D0}, {0x113D2,0x113D2}, {0x113E1,0x113E2}, {0x11438,0x1143F}, {0x11442,0x11444}, {0x11446,0x11446},
 {0x1145E,0x1145E}, {0x114B3,0x114B8}, {0x114BA,0x114BA}, {0x114BF,0x114C0}, {0x114C2,0x114C3}, {0x115B2,0x115B5},
 {0x115BC,0x115BD}, {0x115BF,0x115C0}, {0x115DC,0x115DD}, {0x11633,0x1163A}, {0x1163D,0x1163D}, {0x1163F,0x11640},
 {0x116AB,0x116AB}, {0x116AD,0x116AD}, {0x116B0,0x116B5}, {0x116B7,0x116B7}, {0x1171D,0x1171D}, {0x1171F,0x1171F},
 {0x11722,0x11725}, {0x11727,0x1172B}, {0x1182F,0x11837}, {0x11839,0x1183A}, {0x1193B,0x1193C}, {0x1193E,0x1193E},
 {0x11943,0x11943}, {0x119D4,0x119DB}, {0x119E0,0x119E0}, {0x11A01,0x11A0A}, {0x11A33,0x11A38}, {0x11A3B,0x11A3E},
 {0x11A47,0x11A47}, {0x11A51,0x11A56}, {0x11A59,0x11A5B}, {0x11A8A,0x11A96}, {0x11A98,0x11A99}, {0x11B60,0x11B60},
 {0x11B62,0x11B64}, {0x11B66,0x11B66}, {0x11C30,0x11C3D}, {0x11C3F,0x11C3F}, {0x11C92,0x11CA7}, {0x11CAA,0x11CB0},
 {0x11CB2,0x11CB3}, {0x11CB5,0x11CB6}, {0x11D31,0x11D45}, {0x11D47,0x11D47}, {0x11D90,0x11D91}, {0x11D95,0x11D95},
 {0x11D97,0x11D97}, {0x11EF3,0x11EF4}, {0x11F00,0x11F01}, {0x11F36,0x11F3A}, {0x11F40,0x11F40}, {0x11F42,0x11F42},
 {0x11F5A,0x11F5A}, {0x13430,0x13440}, {0x13447,0x13455}, {0x1611E,0x16129}, {0x1612D,0x1612F}, {0x16AF0,0x16AF4},
 {0x16B30,0x16B36}, {0x16F4F,0x16F4F}, {0x16F8F,0x16F92}, {0x16FE4,0x16FE4}, {0x1BC9D,0x1BC9E}, {0x1BCA0,0x1BCA3},
 {0x1CF00,0x1CF46}, {0x1D167,0x1D169}, {0x1D173,0x1D182}, {0x1D185,0x1D18B}, {0x1D1AA,0x1D1AD}, {0x1D242,0x1D244},
 {0x1DA00,0x1DA36}, {0x1DA3B,0x1DA6C}, {0x1DA75,0x1DA75}, {0x1DA84,0x1DA84}, {0x1DA9B,0x1DAAF}, {0x1E000,0x1E02A},
 {0x1E08F,0x1E08F}, {0x1E130,0x1E136}, {0x1E2AE,0x1E2AE}, {0x1E2EC,0x1E2EF}, {0x1E4EC,0x1E4EF}, {0x1E5EE,0x1E5EF},
 {0x1E6E3,0x1E6E3}, {0x1E6E6,0x1E6E6}, {0x1E6EE,0x1E6EF}, {0x1E6F5,0x1E6F5}, {0x1E8D0,0x1E8D6}, {0x1E944,0x1E94A},
 {0xE0001,0xE01EF},
};

/* From https://unicode.org/Public/UNIDATA/EastAsianWidth.txt */
static const struct utf8range_u16 unicode_wide_u16[] = {
 {0x1100,0x115F}, {0x231A,0x231B}, {0x2329,0x232A}, {0x23E9,0x23EC}, {0x23F0,0x23F0}, {0x23F3,0x23F3},
 {0x25FD,0x25FE}, {0x2614,0x2615}, {0x2630,0x2637}, {0x2648,0x2653}, {0x267F,0x267F}, {0x268A,0x268F},
 {0x2693,0x2693}, {0x26A1,0x26A1}, {0x26AA,0x26AB}, {0x26BD,0x26BE}, {0x26C4,0x26C5}, {0x26CE,0x26CE},
 {0x26D4,0x26D4}, {0x26EA,0x26EA}, {0x26F2,0x26F3}, {0x26F5,0x26F5}, {0x26FA,0x26FA}, {0x26FD,0x26FD},
 {0x2705,0x2705}, {0x270A,0x270B}, {0x2728,0x2728}, {0x274C,0x274C}, {0x274E,0x274E}, {0x2753,0x2755},
 {0x2757,0x2757}, {0x2795,0x2797}, {0x27B0,0x27B0}, {0x27BF,0x27BF}, {0x2B1B,0x2B1C}, {0x2B50,0x2B50},
 {0x2B55,0x2B55}, {0x2E80,0x2FFF}, {0x3001,0x303E}, {0x3041,0x3247}, {0x3250,0xA4C6}, {0xA960,0xA97C},
 {0xAC00,0xD7A3}, {0xF900,0xFAFF}, {0xFE10,0xFE19}, {0xFE30,0xFE6B},
};
static const struct utf8range_u32 unicode_wide_u32[] = {
 {0x16FE0,0x1B2FB}, {0x1D300,0x1D376}, {0x1F004,0x1F004}, {0x1F0CF,0x1F0CF}, {0x1F18E,0x1F18E}, {0x1F191,0x1F19A},
 {0x1F200,0x1F320}, {0x1F32D,0x1F335}, {0x1F337,0x1F37C}, {0x1F37E,0x1F393}, {0x1F3A0,0x1F3CA}, {0x1F3CF,0x1F3D3},
 {0x1F3E0,0x1F3F0}, {0x1F3F4,0x1F3F4}, {0x1F3F8,0x1F43E}, {0x1F440,0x1F440}, {0x1F442,0x1F4FC}, {0x1F4FF,0x1F53D},
 {0x1F54B,0x1F54E}, {0x1F550,0x1F567}, {0x1F57A,0x1F57A}, {0x1F595,0x1F596}, {0x1F5A4,0x1F5A4}, {0x1F5FB,0x1F64F},
 {0x1F680,0x1F6C5}, {0x1F6CC,0x1F6CC}, {0x1F6D0,0x1F6D2}, {0x1F6D5,0x1F6DF}, {0x1F6EB,0x1F6EC}, {0x1F6F4,0x1F6FC},
 {0x1F7E0,0x1F7F0}, {0x1F90C,0x1F93A}, {0x1F93C,0x1F945}, {0x1F947,0x1F9FF}, {0x1FA70,0x1FAF8}, {0x20000,0x3FFFD},
};

#define ARRAYSIZE(A) (sizeof(A) / sizeof(*(A)))
/***********************************************************************
**
*/	REBINT UTF8_Width(REBU32 ch)
/*
***********************************************************************/
{
	if (ch > 0x1F && ch < 0x7F) return 1; // common case
	if (ch <= 0xFFFF) {
		// zero-widths
		if (utf8_in_range_u16(unicode_zero_u16, ARRAYSIZE(unicode_zero_u16), (u16)ch)) {
			return 0;
		}
		// EastAsian wide
		if (utf8_in_range_u16(unicode_wide_u16, ARRAYSIZE(unicode_wide_u16), (u16)ch)) {
			return 2;
		}
	}
	else {
		// zero-widths
		if (utf8_in_range_u32(unicode_zero_u32, ARRAYSIZE(unicode_zero_u32), ch)) {
			return 0;
		}
		// EastAsian wide
		if (utf8_in_range_u32(unicode_wide_u32, ARRAYSIZE(unicode_wide_u32), ch)) {
			return 2;
		}
	}
	return 1;
}
#undef ARRAYSIZE





/***********************************************************************
************************************************************************
**
**	Code below added by REBOL Technologies 2008
**
************************************************************************
***********************************************************************/

/***********************************************************************
**
*/	REBINT What_UTF(const REBYTE *bp, REBCNT len)
/*
**		Tell us what UTF encoding the string has. Negative for LE.
**
***********************************************************************/
{
	// UTF8:
	if (len >= 3 && bp[0] == 0xef && bp[1] == 0xbb && bp[2] == 0xbf) return 8;

	if (len >= 2) {

		// UTF16:
		if (bp[0] == 0xfe && bp[1] == 0xff) return 16;

		// Either UTF16 or 32:
		if (bp[0] == 0xff && bp[1] == 0xfe) {
			if (len >= 4 && bp[2] == 0 && bp[3] == 0) return -32;
			return -16;
		}

		// UTF32
		if (len >= 4 && bp[0] == 0 && bp[1] == 0 && bp[2] == 0xfe && bp[3] == 0xff)
			return 32;
	}

	// Unknown:
	return 0;
}








/***********************************************************************
**
*/	int Decode_UTF8(REBUNI *dst, const REBYTE *src, REBCNT len, REBFLG ccr)
/*
**		Decode UTF8 byte string into a 16 bit preallocated array.
**
**		dst: the desination array, must always be large enough!
**		src: source binary data
**		len: byte-length of source (not number of chars)
**		ccr: convert CRLF/CR to LF
**
**		Returns length in chars (negative if all chars are latin-1).
**		No terminator is added.
**
***********************************************************************/
{
	int flag = -1;
	REBU32 ch;
	REBUNI *start = dst;

	while (len > 0) {
		if ((ch = *src) >= 0x80) {
			flag = 1;
			ch = UTF8_Decode_Codepoint(&src, &len);
			if (ch == 0) {
				ch = UNI_REPLACEMENT_CHAR; // temporary!
			}
			*dst++ = (REBUNI)ch;
			continue;	
		}
		len--;
		src++;
		if (ch == CR && ccr) {
			if (src[1] == LF) continue;
			ch = LF;
		}
		*dst++ = (REBUNI)ch;
	}

	return AS_INT(dst - start) * flag;
}


/***********************************************************************
**
*/	REBSER *Decode_UTF_String(const REBYTE *bp, REBCNT len, REBINT utf, REBFLG ccr, REBCNT *err)
/*
**		Do all the details to decode a string.
**		Input is a byte series. Len is len of input.
**		The utf is 0, 8, +/-16, +/-32.
**		A special -1 means use the BOM.
**		Use `uni = TRUE` not to shorten ASCII result
**		ccr: convert CRLF 
* 
**
***********************************************************************/
{
	REBU32 codepoint=0;
	REBYTE *dst;
	REBCNT unit_size;
	REBFLG is_little_endian;

	if (len == 0) {
		return Make_Series(1, 1, FALSE);
	}

	if (utf == -1) {
		utf = What_UTF(bp, len);
		if (utf) {
			if (utf == 8) bp += 3, len -= 3;
			else if (utf == -16 || utf == 16) bp += 2, len -= 2;
			else if (utf == -32 || utf == 32) bp += 4, len -= 4;
		}
	}

	if (utf == 0 || utf == 8) {
		REBSER *ser = UTF8_Copy_Surrogates(bp, len, err);
		if (!ser) return NULL;
		if (ccr) {
			ser->tail = Replace_CRLF_to_LF_Bytes(BIN_HEAD(ser), BIN_LEN(ser));
		}
		if (!Is_ASCII(BIN_HEAD(ser), BIN_LEN(ser))) UTF8_SERIES(ser);
		return ser;
	} 
	else if (utf == -16 || utf == 16) {
		unit_size = 2;
	}
	else if (utf == -32 || utf == 32) {
		unit_size = 4;
	}
    else {
        return NULL; // Unknown UTF
    }

	is_little_endian = (utf < 0);
	dst = Reset_Buffer(BUF_SCAN, len); // should be large enough for the worst scenario
	const REBYTE *start = bp;
	const REBYTE *end = bp + len;
	while (bp < end) {
		// Read next code unit(s)
		if (unit_size == 2) {
			// UTF-16: handle surrogate pairs
			REBUNI w1 = read_u16(bp, is_little_endian);
			if (w1 >= 0xD800 && w1 <= 0xDBFF) {
				bp += 2;
				if (bp >= end) {
					// Truncated surrogate pair
					goto u16_error;
				}
				REBUNI w2 = read_u16(bp, is_little_endian);
				if (w2 < 0xDC00 || w2 > 0xDFFF) {
					// Invalid surrogate pair
					goto u16_error;
				}
				codepoint = 0x10000 + (((w1 - 0xD800) << 10) | (w2 - 0xDC00));
				bp += 2;
			}
			else if (w1 >= 0xDC00 && w1 <= 0xDFFF) {
				// Unpaired low surrogate
				goto u16_error;
			}
			else {
				codepoint = w1;
				bp += 2;
			}
		}
		else if (unit_size == 4) {
			// UTF-32: each unit is a codepoint
			codepoint = read_u32(bp, is_little_endian);
			// Validate data input
			// Reject code points outside Unicode range or within the surrogate range.
			if (codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
				if (err) *err = AS_REBLEN(bp - start);
				return NULL;
			}
			bp += 4;
		}
		dst += Encode_UTF8_Char(dst, codepoint);

		if (codepoint > 0x7F) UTF8_SERIES(BUF_SCAN);
	}
	len = AS_REBLEN(dst - BIN_HEAD(BUF_SCAN));
	if (ccr) {
		len = Replace_CRLF_to_LF_Bytes(BIN_HEAD(BUF_SCAN), len);
	}
	return Copy_String(BUF_SCAN, 0, len);

u16_error:
	if (err) *err = 2 * AS_REBLEN(bp - 2 - start);
	return NULL;
}

/***********************************************************************
**
*/	REBCNT Length_As_UTF8_Code_Points(REBYTE *src)
/*
**		Returns number of code points encoded in UTF-8.
**
***********************************************************************/
{
	REBCNT size = 0;
	while (*src) {
        size += (*src++ & 0xC0) != 0x80;
    }
	return size;
}


/***********************************************************************
**
*/	REBLEN Length_As_Terminal_Width(const REBYTE* str, const REBYTE* end)
/*
**		Returns the column width required for the given UTF-8 string.
**
***********************************************************************/
{
	REBLEN width = 0;
	REBLEN len = end - str;
	while (str < end) {
		REBU32 ch = UTF8_Decode_Codepoint(&str, &len);
		width += UTF8_Width(ch);
	}
	return width;
}

/***********************************************************************
**
*/	REBCNT Length_As_UTF8(REBUNI *src, REBCNT len, REBOOL uni, REBOOL ccr)
/*
**		Returns how long the UTF8 encoded string would be.
**
***********************************************************************/
{


	REBCNT size = 0;
	REBCNT c;
	REBYTE *bp = (REBYTE*)src;

	for (; len > 0; len--) {
		c = uni ? *src++ : *bp++;
		if (c <= 0x7F) {
#ifdef TO_WINDOWS
			if (ccr && c == LF) size++; // because we will add a CR to it
#endif
			size++;
		}
		else if (c <= 0x7FF)        size += 2;
		else if (c <= 0xFFFF)       size += 3;
		else if (c <= UNI_MAX_LEGAL_UTF32) size += 4;
		else size += 3; // because of the replacement char size
	}

	return size;
}

FORCE_INLINE
/***********************************************************************
**
*/	REBCNT Encode_UTF8_Char(REBYTE *dst, REBU32 chr)
/*
**		Converts a single char to UTF8 code-point.
**		Returns length of char stored in dst.
**		Be sure dst has at least 4 bytes available.
**
***********************************************************************/
{
	if (chr <= 0x7F) {
		// 1-byte/7-bit ascii
		// (0b0xxxxxxx)
		dst[0] = (REBYTE)chr;
		return 1;
	}
	if (chr <= 0x7FF) {
		// 2-byte/11-bit utf8 code point
		// (0b110xxxxx 0b10xxxxxx)
		dst[0] = (REBYTE)(0xc0 | (REBYTE)((chr >> 6) & 0x1f));
		dst[1] = (REBYTE)(0x80 | (REBYTE)(chr & 0x3f));
		return 2;
	}
	if (chr <= 0xFFFF) {
		// 3-byte/16-bit utf8 code point
		// (0b1110xxxx 0b10xxxxxx 0b10xxxxxx)
		dst[0] = (REBYTE)(0xe0 | (REBYTE)((chr >> 12) & 0x0f));
		dst[1] = (REBYTE)(0x80 | (REBYTE)((chr >> 6) & 0x3f));
		dst[2] = (REBYTE)(0x80 | (REBYTE)(chr & 0x3f));
		return 3;
	}
	// 4-byte/21-bit utf8 code point
	// (0b11110xxx 0b10xxxxxx 0b10xxxxxx 0b10xxxxxx)
	dst[0] = (REBYTE)(0xf0 | (REBYTE)((chr >> 18) & 0x07));
	dst[1] = (REBYTE)(0x80 | (REBYTE)((chr >> 12) & 0x3f));
	dst[2] = (REBYTE)(0x80 | (REBYTE)((chr >> 6) & 0x3f));
	dst[3] = (REBYTE)(0x80 | (REBYTE)(chr & 0x3f));
	return 4;
}


/***********************************************************************
**
*/	REBCNT Encode_UTF8(REBYTE *dst, REBINT max, void *src, REBLEN *len, REBFLG uni, REBFLG ccr)
/*
**		Encode the unicode into UTF8 byte string.
**
**		Source string can be byte or unichar sized (uni = TRUE);
**		Max is the maximum size of the result (UTF8).
**		Returns number of source chars used.
**		Updates len for dst bytes used.
**		Does not add a terminator.
**
***********************************************************************/
{
	REBU32 c, c2;
	REBINT n;
	REBYTE buf[8];
	REBYTE *bs = dst; // save start
	REBYTE *bp;
	REBUNI *up;
	REBLEN cnt=0;

	if (len) cnt = *len;
	if (uni) {
		up = (REBUNI*)src;
		if (!cnt) {
			// not using wcslen, because on some systems wchar_t has 4 bytes!
			cnt = 0;
			while (*up++ != 0 && cnt < (REBLEN)max) cnt++;
			up = (REBUNI*)src;
		}
		for (; max > 0 && cnt > 0; cnt--) {
			c = *up++;
			if (c < 0x80) {
#if defined(TO_WINDOWS)
				if (ccr) {
					if (c == CR && up[0] == LF) {
						*dst++ = CR;
						*dst++ = LF;
						up++;
						cnt--;
						max -= 2;
						continue;
					}
					if (c == LF) {
						// If there's not room, don't try to output CRLF
						if (2 > max) { up--; break; }
						*dst++ = CR;
						max--;
						c = LF;
					}
				}
#endif
				*dst++ = (REBYTE)c;
				max--;
			}
			else {
				if (c >= 0xD800 && c <= 0xDBFF) {
					c2 = *up++; cnt--;
					c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				}
				n = Encode_UTF8_Char(buf, c);
				if (n > max) { up--; break; }
				memcpy(dst, buf, n);
				dst += n;
				max -= n;
			}
		}
		if (len) *len = AS_REBLEN(dst - bs);
		return AS_REBLEN(up - (REBUNI*)src);
	}
	else {
		bp = (REBYTE*)src;
		if (!len) cnt = LEN_BYTES(bp);
		for (; max > 0 && cnt > 0; cnt--) {
			c = *bp++;
			if (c < 0x80) {
#if defined(TO_WINDOWS)
				if (ccr) {
					if (c == CR && bp[0] == LF) {
						*dst++ = CR;
						*dst++ = LF;
						bp++;
						cnt--;
						max -= 2;
						continue;
					}
					if (c == LF) {
						// If there's not room, don't try to output CRLF
						if (2 > max) { bp--; break; }
						*dst++ = CR;
						max--;
						c = LF;
					}
				}
#endif
				*dst++ = (REBYTE)c;
				max--;
			}
			else {
				n = Encode_UTF8_Char(buf, c);
				if (n > max) { bp--; break; }
				memcpy(dst, buf, n);
				dst += n;
				max -= n;
			}
		}
		if (len) *len = AS_REBLEN(dst - bs);
		return AS_REBLEN(bp - (REBYTE*)src);
	}


}

#ifdef unused
/***********************************************************************
**
X*/  int Encode_UTF8_Line(REBSER *dst, REBSER *src, REBCNT idx)
/*
**		Encode a unicode source buffer into a binary line of UTF8.
**		Include the LF terminator in the result.
**		Return the length of the line buffer.
**
***********************************************************************/
{
	REBUNI *up = UNI_HEAD(src);
	REBCNT len  = SERIES_TAIL(src);
	REBCNT tail;
	REBUNI c;
	REBINT n;
	REBYTE buf[8];

	tail = RESET_TAIL(dst);

	while (idx < len) {
		if ((c = up[idx]) < 0x80) {
			EXPAND_SERIES_TAIL(dst, 1);
			BIN_HEAD(dst)[tail++] = (REBYTE)c;
		}
		else {
			n = Encode_UTF8_Char(buf, c);
			EXPAND_SERIES_TAIL(dst, n);
			memcpy(BIN_SKIP(dst, tail), buf, n);
			tail += n;
		}
		idx++;
		if (c == LF) break;
	}

	BIN_HEAD(dst)[tail] = 0;
	SERIES_TAIL(dst) = tail;
	return idx;
}

/***********************************************************************
**
X*/	REBSER *Encode_UTF8_Value(REBVAL *arg, REBCNT len, REBFLG opts)
/*
**		Do all the details to encode a string as UTF8.
**		No_copy means do not make a copy.
**		Result can be a shared buffer!
**
***********************************************************************/
{
	REBSER * ser;
	if (VAL_BYTE_SIZE(arg)) {
		ser = Encode_UTF8_String(VAL_BIN_DATA(arg), len, FALSE, opts);
	} else {
		ser = Encode_UTF8_String(VAL_UNI_DATA(arg), len, TRUE, opts);
	}
	return ser;
}
#endif

/***********************************************************************
**
*/	REBSER *Encode_UTF8_String(void *src, REBLEN len, REBFLG uni, REBFLG opts)
/*
**		Do all the details to encode a string as UTF8.
**		No_copy means do not make a copy.
**		Result can be a shared buffer!
**
***********************************************************************/
{
	REBSER *ser; // a shared buffer
	REBLEN size;
	REBYTE *cp;
//	REBFLG ccr = GET_FLAG(opts, ENC_OPT_CRLF);
	REBFLG no_copy = GET_FLAG(opts, ENC_OPT_NO_COPY); // using share buffer

	if (uni) {
		REBYTE *utf8 = NULL;
		// Uasing OS conversion, because the old Rebol UTF-8 encoder does not support surrogates yet!
		size = OS_Wide_To_Multibyte((const REBUNI *)src, &utf8, len);
		if (no_copy) {
			ser = BUF_SCAN;
			cp = Reset_Buffer(ser, size); // +(GET_FLAG(opts, ENC_OPT_BOM) ? 3 : 0));
			COPY_MEM(cp, utf8, size);
			SERIES_TAIL(ser) = size;
			STR_TERM(ser);
		}
		else {
			ser = Copy_Bytes(utf8, size);
		}
		OS_Free(utf8);
	}
	else {
		size = len;
		ser = Copy_Bytes((REBYTE *)src, size);
#ifdef unused
		if (ccr || !Is_ASCII(bp, len)) {
			size = Length_As_UTF8((REBUNI*)bp, len, FALSE, (REBOOL)ccr);
			cp = Reset_Buffer(ser, size + (GET_FLAG(opts, ENC_OPT_BOM) ? 3 : 0));
			Encode_UTF8(cp, size, bp, &len, FALSE, ccr);
		}
		else if (GET_FLAG(opts, ENC_OPT_NO_COPY)) return 0;
		else return Copy_Bytes(bp, len);
#endif
	}
	if (!Is_ASCII(BIN_HEAD(ser), size)) UTF8_SERIES(ser);
	return ser;
}
