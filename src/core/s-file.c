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
**  Module:  s-file.c
**  Summary: file and path string handling
**  Section: strings
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include <wchar.h>

#define FN_PAD 2	// pad file name len for adding /, /*, and /?


/***********************************************************************
**
*/	REBSER *To_REBOL_Path(REBYTE *bp, REBCNT len, REBINT uni, REBFLG dir)
/*
**		Convert local filename to a REBOL filename.
**
**		Allocate and return a new series with the converted path.
**		Return 0 on error.
**
**		Reduces width when possible.
**		Adds extra space at end for appending a dir /?
**
**		REBDIFF: No longer appends current dir to volume when no
**		root slash is provided (that odd MSDOS c:file case).
**
***********************************************************************/
{
	REBOOL colon = 0;    // have we hit a ':' yet?
	REBOOL slash = 0;    // was the previous char a '/'? (for collapsing)
	REBOOL any_slash = 0;// have we hit a '/' yet?
	REBYTE c = 0;
	REBSER *dst;
	REBCNT n;
	REBCNT i;
	REBYTE *out;
	REBYTE *src;

	if (uni) {
		len = OS_Wide_To_Multibyte((const REBUNI*)bp, &src, len);
	}
	else {
		src = bp;
		if (len == UNKNOWN) len = LEN_BYTES(src);
	}
	
	n = 0;
	dst = Make_Binary(len+FN_PAD);
	out = BIN_HEAD(dst);

	for (i = 0; i < len;) {
		c = src[i];
		i++;
#ifdef TO_WINDOWS
		if (c == ':') {
			// Handle the vol:dir/file format:
			if (colon || any_slash) { // no prior : or / allowed
				if (uni) FREE_MEM(src);
				return 0;
			}
			colon = 1;
			if (i < len) {
				c = src[i];
				if (c == '\\' || c == '/') { i++; slash = 1; } // skip / in foo:/file
			}
			any_slash = 1;
			c = '/'; // replace : with a /
		}
		else
#endif
		if (c == '\\' || c== '/') {
			if (slash > 0) continue;
			c = '/';
			slash = any_slash = 1;
		}
		else slash = 0;
		out[n++] = c;
	}
	if (dir && c != '/') {  // watch for %/c/ case
		out[n++] = '/';
	}
	SERIES_TAIL(dst) = n;
	STR_TERM(dst);

	if (!Is_ASCII(src, len)) UTF8_SERIES(dst);
	if (uni) free(src);

	// Change C:/ to /C/ (and C:X to /C/X):
	if (colon)
		Insert_Char(dst, 0, (REBCNT)'/');
	return dst;
}


/***********************************************************************
**
*/	REBSER *Value_To_REBOL_Path(REBVAL *val, REBOOL dir)
/*
**		Helper to above function.
**
***********************************************************************/
{
	ASSERT1(ANY_BINSTR(val), RP_MISC);
	//if (!VAL_BYTE_SIZE(val))
	//	puts("Value_To_REBOL_Path expects UTF8 encode input!");

	return To_REBOL_Path(VAL_DATA(val), VAL_LEN(val), (REBOOL)!VAL_BYTE_SIZE(val), dir);
}


/***********************************************************************
**
*/	REBSER *To_Local_Path(REBYTE *bp, REBCNT len, REBOOL wide, REBFLG full)
/*
**		Convert REBOL filename to a local filename.
**
**		Allocate and return a new series with the converted path.
**		Return 0 on error.
**
**		Adds extra space at end for appending a dir /?
**		Expands width for OS's that require it.
**
***********************************************************************/
{
	REBYTE c;
	REBSER *dst;
	REBYTE *src;
	REBCNT i = 0;
	REBCNT n = 0;
	REBYTE *out;
	REBYTE *lpath = NULL;
	REBCNT l = 0;

	if (!bp) return NULL;

	if (len == 0) len = (REBCNT)LEN_BYTES(bp);
	src = bp;

	// Prescan for: /c/dir = c:/dir, /vol/dir = //vol/dir, //dir = ??
	c = src[i];
	if (c == '/') {			// %/
		dst = Make_Binary(len+FN_PAD);
		out = STR_HEAD(dst);
#ifdef TO_WINDOWS
		if (len == 1) {
			// it was really just: %/
			// so return empty string in such a case
			goto term_out;
		}
		i++;
		if (i < len) {
			c = src[i];
			i++;
		}
		if (c != '/') {		// %/c or %/c/ but not %/ %// %//c
			// peek ahead for a '/':
			REBUNI d = '/';
			if (i < len) d = src[i];
			if (d == '/') {	// %/c/ => "c:/"
				i++;
				out[n++] = c;
				out[n++] = ':';
			}
			else {
				out[n++] = OS_DIR_SEP;  // %/cc %//cc => "//cc"
				i--;
			}
		}
#endif
		out[n++] = OS_DIR_SEP;
	}
	else {
		if (full) l = OS_Get_Current_Dir(&lpath); // lpath is UTF-8 encoded!
		dst = Make_Binary(l + len + FN_PAD); // may be longer (if lpath is encoded)
		if (full) {
			Append_Bytes_Len(dst, lpath, l);
			if (OS_DIR_SEP != STR_LAST(dst)[0]) {
				EXPAND_SERIES_TAIL(dst, 1);
				*STR_LAST(dst) = OS_DIR_SEP;
				*STR_TAIL(dst) = 0;
			}
			OS_Free(lpath);
		}
		out = STR_HEAD(dst);
		n = SERIES_TAIL(dst);
	}

	// Prescan each file segment for: . .. directory names:
	// (Note the top of this loop always follows / or start)
	while (i < len) {
		if (full) {
			// Peek for: . ..
			c = src[i];
			if (c == '.') {		// .
				i++;
				c = src[i];
				if (c == '.') {	// ..
					c = src[i+1];
					if (c == 0 || c == '/') { // ../ or ..
						i++;
						// backup a dir
						n -= (n > 2) ? 2 : n;
						for (; n > 0 && out[n] != OS_DIR_SEP; n--);
						c = c ? 0 : OS_DIR_SEP; // add / if necessary
					}
					// fall through on invalid ..x combination:
				}
				else {	// .a or . or ./
					if (c == '/') {
						i++;
						c = 0; // ignore it
					}
					else if (c) c = '.'; // for store below
				}
				if (c) out[n++] = c;
			}
		}
		for (; i < len; i++) {
			c = src[i];
			if (c == '/') {
				if (n == 0 || out[n-1] != OS_DIR_SEP) out[n++] = OS_DIR_SEP;
				i++;
				break;
			}
			out[n++] = c;
		}
	}
term_out:
	SERIES_TAIL(dst) = n;
	STR_TERM(dst);

	if (wide && n > 0) {
		REBYTE *uni = NULL;
		REBLEN len = OS_Multibyte_To_Wide(STR_HEAD(dst), &uni);
		if (uni == NULL) return NULL;
		//wprintf(L"--wide: [ %s ]-- %u %u\n", (REBCHR*)uni, len, wcslen(uni));
		Free_Series(dst);
		dst = Make_Unicode(len + 1);
		memcpy(BIN_HEAD(dst), (const void*)uni, len * sizeof(REBUNI));
		OS_Free((void*)uni);

		SERIES_TAIL(dst) = len;
		UNI_TERM(dst);
		//Debug_Uni(dst);
	}

	return dst;
}


/***********************************************************************
**
*/	REBSER *Value_To_Local_Path(REBVAL *val, REBFLG full)
/*
**		Helper to above function.
**
***********************************************************************/
{
	ASSERT1(ANY_BINSTR(val), RP_MISC);
	return To_Local_Path(VAL_DATA(val), VAL_LEN(val), OS_WIDE, full);
}


FORCE_INLINE
/***********************************************************************
**
*/	REBSER *Value_To_OS_Path(REBVAL *val, REBFLG full)
/*
**		Helper to above function. Result will be wide size on Windows.
**
***********************************************************************/
{
	ASSERT1(ANY_BINSTR(val), RP_MISC);
	return To_Local_Path(VAL_DATA(val), VAL_LEN(val), OS_WIDE, full);
}
