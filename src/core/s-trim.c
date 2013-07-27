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
**  Module:  s-trim.c
**  Summary: string trimming
**  Section: strings
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

static REBFLG find_in_uni(REBUNI *up, REBINT len, REBUNI c)
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
	REBCNT wlen;
	REBUNI with_chars[MAX_WITH];	// chars to be trimmed
	REBUNI *up = with_chars;
	REBYTE *bp;
	REBCNT n;
	REBUNI uc;

	// Setup WITH array from arg or the default:
	n = 0;
	if (IS_NONE(with)) {
		bp = "\n \r\t";
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
		wlen = n;
		if (VAL_BYTE_SIZE(with)) {
			bp = VAL_BIN_DATA(with);
		} else {
			memcpy(up, VAL_UNI_DATA(with), n * sizeof(REBUNI));
			n = 0;
		}
	}
	for (; n > 0; n--) *up++ = (REBUNI)*bp++;

	// Remove all occurances of chars found in WITH string:
	for (n = index; index < tail; index++) {
		uc = GET_ANY_CHAR(ser, index);
		if (!find_in_uni(with_chars, wlen, uc)) {
			SET_ANY_CHAR(ser, n, uc);
			n++;
		}
	}

	SET_ANY_CHAR(ser, n, 0);	
	SERIES_TAIL(ser) = n;
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
	REBUNI uc = 0;

	// Skip whitespace, remember start of last line:
	for (line = index; index < tail; index++) {
		uc = GET_ANY_CHAR(ser, index);
		if (!IS_WHITE(uc)) break;
		if (uc == LF) line = index+1;
	}

	// Count the indentation used:
	for (indent = 0; line < index; line++) {
		if (GET_ANY_CHAR(ser, line) == ' ') indent++;
		else indent = (indent + TAB_SIZE) & ~3;
	}

	// For each line, pad with necessary indentation:
	while (index < tail) {
		// Skip to next content, track indentation:
		for (len = 0; index < tail; index++) {
			uc = GET_ANY_CHAR(ser, index);
			if (!IS_SPACE(uc) || len >= indent) break;
			if (uc == ' ') len++;
			else len = (len + TAB_SIZE) & ~3;
		}

		// Indent the line:
		for (; len > indent; len--) {
			SET_ANY_CHAR(ser, out, ' ');
			out++;
		}

		// Copy line contents:
		while (index < tail) {
			uc = GET_ANY_CHAR(ser, index);
			SET_ANY_CHAR(ser, out, uc);
			out++;
			index++;
			if (uc == LF) break;
		}
	}

	SET_ANY_CHAR(ser, out, 0);
	SERIES_TAIL(ser) = out;
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
	REBUNI uc;
	REBCNT out = index;

	for (; index < tail; index++) {
		uc = GET_ANY_CHAR(ser, index);
		if (IS_WHITE(uc)) {
			uc = ' ';
			if (!pad) {
				SET_ANY_CHAR(ser, out, uc);
				out++;
				pad = 2;
			}
		}
		else {
			SET_ANY_CHAR(ser, out, uc);
			out++;
			pad = 0;
		}
	}

	// Remove extra end pad if found:
	if (pad == 2) out--;

	SET_ANY_CHAR(ser, out, 0);	
	SERIES_TAIL(ser) = out;
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
	REBUNI uc;

	// Skip head lines if required:
	if (h || !t) {
		for (; index < tail; index++) {
			uc = GET_ANY_CHAR(ser, index);
			if (!IS_WHITE(uc)) break;
		}
	}

	// Skip tail lines if required:
	if (t || !h) {
		for (; index < tail; tail--) {
			uc = GET_ANY_CHAR(ser, tail -1);
			if (uc == LF) append_line_feed = TRUE;
			if (!IS_WHITE(uc)) break;
		}
	}

	// Trim head and tail of innner lines if required:
	if (!h && !t) {
		REBOOL outside = FALSE; // inside an inner line
		REBCNT left = 0; // index of leftmost space (in output)

		for (; index < tail; index++) {

			uc = GET_ANY_CHAR(ser, index);

			if (IS_SPACE(uc)) {
				if (outside) continue;
				if (!left) left = out;
			}
			else if (uc == LF) {
				outside = TRUE;
				if (left) out = left, left = 0;
			}
			else {
				outside = FALSE;
				left = 0;
			}

			SET_ANY_CHAR(ser, out, uc);
			out++;
		}
	}
	else {
		for (; index < tail; index++) {
			uc = GET_ANY_CHAR(ser, index);
			SET_ANY_CHAR(ser, out, uc);
			out++;
		}
	}

	// Append line feed if necessary
	if (append_line_feed && !t) {
		SET_ANY_CHAR(ser, out, LF);
		out++;
	}

	SET_ANY_CHAR(ser, out, 0);
	SERIES_TAIL(ser) = out;
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
	}
	// /lines option
	else if (flags & AM_TRIM_LINES) {
		trim_lines(ser, index, tail);
	}
	else {
		trim_head_tail(ser, index, tail, flags & AM_TRIM_HEAD, flags & AM_TRIM_TAIL);
	}
}
