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
**  Module:  s-trim.c
**  Summary: string trimming
**  Section: strings
**  Author:  Carl Sassenrath, Oldes
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

FORCE_INLINE
static REBFLG find_in_uni(REBU32 *up, REBINT len, REBU32 c)
{
	while (len-- > 0) if (*up++ == c) return TRUE;
	return FALSE;
}


/***********************************************************************
**
*/	static void replace_with(REBSER *ser, REBCNT index, REBCNT tail, REBVAL *with)
/*
**		Replace whitespace chars that match WITH string.
**
**		Resulting string is always smaller than it was to start.
**
***********************************************************************/
{
	#define MAX_WITH 32
	REBCNT wlen = 0, n;
	REBU32  with_chars[MAX_WITH];	// chars to be trimmed
	REBU32  *up = with_chars;
	REBU32  chr;
	const REBYTE *src = NULL;
	REBYTE *dst;

	// Setup WITH array from arg or the default:
	n = 0;
	if (IS_NONE(with)) {
		src = cb_cast("\n \r\t");
		wlen = n = 4;
	}
	else if (IS_CHAR(with)) {
		wlen = 1;
		*up++ = VAL_CHAR(with);
	}
	else if (IS_INTEGER(with)) {
		wlen = 1;
		*up++ = Int32s(with, 0);
	}
	else if (ANY_BINSTR(with)) {
		n = VAL_LEN(with);
		if (n >= MAX_WITH) n = MAX_WITH-1;
		src = VAL_BIN_DATA(with);
		wlen = n;
	}
	if (src) {
		while (n > 0) {
			*up++ = UTF8_Decode_Codepoint(&src, &n);
		}
	}

	src = dst = STR_SKIP(ser, index);
	// Remove all occurances of chars found in WITH string:
	for (n = tail - index; n > 0; ) {
		chr = UTF8_Decode_Codepoint(&src, &n);
		if (!find_in_uni(with_chars, wlen, chr)) {
			dst += Encode_UTF8_Char(dst, chr);
		}
	}
	SERIES_TAIL(ser) = AS_REBLEN(dst - BIN_HEAD(ser));
	TERM_SERIES(ser);
}


/***********************************************************************
**
*/	static void trim_auto(REBSER *ser, REBCNT index, REBCNT tail)
/*
**		Skip any blank lines and then determine indent of
**		first line and make the rest align with it.
**
**		BUG!!! If the indentation uses TABS, then it could
**		fill past the source pointer!
**
***********************************************************************/
{
	REBCNT out = index;
	REBCNT line;
	REBCNT len;
	REBCNT indent;
	REBYTE uc = 0;
	REBYTE *bp = BIN_HEAD(ser);

	// Skip whitespace, remember start of last line:
	for (line = index; index < tail; index++) {
		uc = bp[index];
		if (!IS_WHITE(uc)) break;
		if (uc == LF) line = index+1;
	}

	// Count the indentation used:
	for (indent = 0; line < index; line++) {
		if (bp[line] == ' ') indent++;
		else indent = (indent + TAB_SIZE) & ~3;
	}

	// For each line, pad with necessary indentation:
	while (index < tail) {
		// Skip to next content, track indentation:
		for (len = 0; index < tail; index++) {
			uc = bp[index];
			if (!IS_SPACE(uc) || len >= indent) break;
			if (uc == ' ') len++;
			else len = (len + TAB_SIZE) & ~3;
		}

		// Indent the line:
		for (; len > indent; len--) {
			bp[out] = ' ';
			out++;
		}

		// Copy line contents:
		while (index < tail) {
			bp[out] = uc = bp[index];
			out++;
			index++;
			if (uc == LF) break;
		}
	}

	SERIES_TAIL(ser) = out;
	TERM_SERIES(ser);
}


/***********************************************************************
**
*/	static void trim_lines(REBSER *ser, REBCNT index, REBCNT tail)
/*
**		Remove all newlines and extra space.
**
***********************************************************************/
{
	REBINT pad = 1; // used to allow a single space
	REBYTE uc;
	REBYTE *bp = BIN_HEAD(ser);
	REBCNT out = index;

	for (; index < tail; index++) {
		uc = bp[index];
		if (IS_WHITE(uc)) {
			uc = ' ';
			if (!pad) {
				bp[out++] = uc;
				pad = 2;
			}
		}
		else {
			bp[out++] = uc;
			pad = 0;
		}
	}

	// Remove extra end pad if found:
	if (pad == 2) out--;

	SERIES_TAIL(ser) = out;
	TERM_SERIES(ser);
}


/***********************************************************************
**
*/	static void trim_head_tail(REBSER *ser, REBCNT index, REBCNT tail, REBFLG h, REBFLG t)
/*
**		Trim from head and tail of each line, trim any leading or
**		trailing lines as well, leaving one at the end if present
**
***********************************************************************/
{
	REBCNT out = index;
	REBOOL append_line_feed = FALSE;
	REBU32 chr;
	const REBYTE *src = BIN_SKIP(ser, index);
	REBCNT size;

	// Skip head lines if required:
	if (h || !t) {
		while (index < tail && IS_WHITE(*src)) src++, index++;
	}

	// Skip tail lines if required:
	if (t || !h) {
		for (; index < tail; tail--) {
			REBYTE b = BIN_HEAD(ser)[tail-1];
			if (b == LF) append_line_feed = TRUE;
			if (!IS_WHITE(b)) break;
		}
	}

	// Trim head and tail of innner lines if required:
	if (!h && !t) {
		REBOOL outside = FALSE; // inside an inner line
		REBCNT left = 0; // index of leftmost space (in output)
		for (size = tail - index; size > 0;) {
			chr = UTF8_Decode_Codepoint(&src, &size);
			if (chr < 0x7F && IS_SPACE(chr)) {
				if (outside) continue;
				if (!left) left = out;
			}
			else if (chr == LF) {
				outside = TRUE;
				if (left) out = left, left = 0;
			}
			else if (chr == UNI_ERROR) {
				break;
			}
			else {
				outside = FALSE;
				left = 0;
			}

			out += Encode_UTF8_Char(BIN_SKIP(ser, out), chr);
		}
	}
	else {
		size = tail - index;
		while (size > 0) {
			chr = UTF8_Decode_Codepoint(&src, &size);
			out += Encode_UTF8_Char(BIN_SKIP(ser, out), chr);
		}
	}

	// Append line feed if necessary
	if (append_line_feed && !t) {
		out += Encode_UTF8_Char(BIN_SKIP(ser, out), LF);
	}
	SERIES_TAIL(ser) = out;
	TERM_SERIES(ser);
}


/***********************************************************************
**
*/	void Trim_String(REBSER *ser, REBCNT index, REBCNT len, REBCNT flags, REBVAL *with)
/*
***********************************************************************/
{
	REBCNT tail = index + len;

	// /all or /with
	if (flags & (AM_TRIM_ALL | AM_TRIM_WITH)) {
		replace_with(ser, index, tail, with);
	}
	// /auto option
	else if (flags & AM_TRIM_AUTO) {
		trim_auto(ser, index, tail);
		if (flags & AM_TRIM_TAIL) {
			tail = SERIES_TAIL(ser);
			for (; index < tail; tail--) {
				REBYTE b = BIN_HEAD(ser)[tail - 1];
				if (!IS_WHITE(b)) break;
			}
			SERIES_TAIL(ser) = tail;
			TERM_SERIES(ser);
		}
	}
	// /lines option
	else if (flags & AM_TRIM_LINES) {
		trim_lines(ser, index, tail);
	}
	else {
		trim_head_tail(ser, index, tail, flags & AM_TRIM_HEAD, flags & AM_TRIM_TAIL);
	}
}

/***********************************************************************
**
*/	void Trim_Binary(REBSER *ser, REBCNT index, REBCNT len, REBCNT flags, REBVAL *with)
/*
***********************************************************************/
{
	REBCNT tail = index + len;
	REBCNT n;
	if (flags & AM_TRIM_WITH) Trap0(RE_BAD_REFINES);
	if (flags & AM_TRIM_ALL) {
		// Remove all nulls...
		REBYTE* src = BIN_SKIP(ser, index);
		REBYTE* dst = src;
		for (n = 0; n < len; n++) {
			if (src[n])	*dst++ = src[n];
			else tail--;
		}
		SERIES_TAIL(ser) = tail;
		BIN_HEAD(ser)[tail] = 0;
		return;
	}
	// /head
	if (!flags || flags & AM_TRIM_HEAD) {
		for (n = 0; n < len; n++) {
			if (BIN_HEAD(ser)[index + n]) break;
		}
		if (n > 0) {
			Remove_Series(ser, index, n);
			tail -= n;
		}
	}
	// /tail
	if (!flags || flags & AM_TRIM_TAIL) {
		for (; index < tail; tail--) {
			if (BIN_HEAD(ser)[tail - 1]) break;
		}
		SERIES_TAIL(ser) = tail;
	}
	// these are not supported on binary values
	if (flags & (AM_TRIM_AUTO | AM_TRIM_LINES)) {
		Trap0(RE_BAD_REFINES);
	}
}

/***********************************************************************
**
*/	void Trim_Block(REBSER *ser, REBCNT index, REBCNT flags)
/*
***********************************************************************/
{
	REBVAL *blk = BLK_HEAD(ser);
	REBCNT out = index;
	REBCNT end = ser->tail;

	if (flags & AM_TRIM_ALL) {
		if (flags != (flags & AM_TRIM_ALL)) Trap0(RE_BAD_REFINES);
		for (; index < end; index++) {
			if (VAL_TYPE(blk + index) > REB_NONE) {
				*BLK_SKIP(ser, out) = blk[index];
				out++;
			}
		}
		Remove_Series(ser, out, end - out);
		return;
	}
	
	if (flags & ~(AM_TRIM_HEAD | AM_TRIM_TAIL)) Trap0(RE_BAD_REFINES);

	if (!flags || flags & AM_TRIM_TAIL) {
		for (; end >= (index + 1); end--) {
			if (VAL_TYPE(blk + end - 1) > REB_NONE) break;
		}
		Remove_Series(ser, end, ser->tail - end);
	}

	if (!flags || flags & AM_TRIM_HEAD) {
		for (; index < end; index++) {
			if (VAL_TYPE(blk + index) > REB_NONE) break;
		}
		Remove_Series(ser, out, index - out);
	}
}
