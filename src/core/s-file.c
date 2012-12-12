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
**  Module:  s-file.c
**  Summary: file and path string handling
**  Section: strings
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

#define FN_PAD 2	// pad file name len for adding /, /*, and /?


/***********************************************************************
**
*/	REBSER *To_REBOL_Path(void *bp, REBCNT len, REBINT uni, REBFLG dir)
/*
**		Convert local filename to a REBOL filename.
**
**		Allocate and return a new series with the converted path.
**		Return 0 on error.
**
**		Reduces width when possible.
**		Adds extra space at end for appending a dir /*
**
**		REBDIFF: No longer appends current dir to volume when no
**		root slash is provided (that odd MSDOS c:file case).
**
***********************************************************************/
{
	REBOOL colon = 0;  // have we hit a ':' yet?
	REBOOL slash = 0; // have we hit a '/' yet?
	REBUNI c;
	REBSER *dst;
	REBCNT n;
	REBCNT i;

	if (len == 0)
		len = uni ? wcslen((REBUNI*)bp) : LEN_BYTES((REBYTE*)bp);
	
	n = 0;
	dst = ((uni == -1) || (uni && Is_Wide((REBUNI*)bp, len))) 
		? Make_Unicode(len+FN_PAD) : Make_Binary(len+FN_PAD);

	for (i = 0; i < len;) {
		c = uni ? ((REBUNI*)bp)[i] : ((REBYTE*)bp)[i];
		i++;
		if (c == ':') {
			// Handle the vol:dir/file format:
			if (colon || slash) return 0; // no prior : or / allowed
			colon = 1;
			if (i < len) {
				c = uni ? ((REBUNI*)bp)[i] : ((REBYTE*)bp)[i];
				if (c == '\\' || c == '/') i++; // skip / in foo:/file
			}
			c = '/'; // replace : with a /
		}
		else if (c == '\\' || c== '/') {
			if (slash > 0) continue;
			c = '/';
			slash = 1;
		}
		else slash = 0;
		SET_ANY_CHAR(dst, n++, c);
	}
	if (dir && c != '/') {  // watch for %/c/ case
		SET_ANY_CHAR(dst, n++, '/');
	}
	SERIES_TAIL(dst) = n;
	TERM_SERIES(dst);

	// Change C:/ to /C/ (and C:X to /C/X):
	if (colon) Insert_Char(dst, 0, (REBCNT)'/');

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
	return To_REBOL_Path(VAL_DATA(val), VAL_LEN(val), (REBOOL)!VAL_BYTE_SIZE(val), dir);
}


/***********************************************************************
**
*/	REBSER *To_Local_Path(void *bp, REBCNT len, REBOOL uni, REBFLG full)
/*
**		Convert REBOL filename to a local filename.
**
**		Allocate and return a new series with the converted path.
**		Return 0 on error.
**
**		Adds extra space at end for appending a dir /*
**		Expands width for OS's that require it.
**
***********************************************************************/
{
	REBUNI c, d;
	REBSER *dst;
	REBCNT i = 0;
	REBCNT n = 0;
	REBUNI *out;
	REBCHR *lpath;
	REBCNT l = 0;

	if (len == 0)
		len = uni ? wcslen((REBUNI*)bp) : LEN_BYTES((REBYTE*)bp);

	// Prescan for: /c/dir = c:/dir, /vol/dir = //vol/dir, //dir = ??
	c = GET_CHAR_UNI(uni, bp, i);
	if (c == '/') {			// %/
		dst = Make_Unicode(len+FN_PAD);
		out = UNI_HEAD(dst);
#ifdef TO_WIN32
		i++;
		if (i < len) {
			c = GET_CHAR_UNI(uni, bp, i);
			i++;
		}
		if (c != '/') {		// %/c or %/c/ but not %/ %// %//c
			// peek ahead for a '/':
			d = '/';
			if (i < len) d = GET_CHAR_UNI(uni, bp, i);
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
		if (full) l = OS_GET_CURRENT_DIR(&lpath);
		dst = Make_Unicode(l + len + FN_PAD); // may be longer (if lpath is encoded)
		if (full) {
#ifdef TO_WIN32
			Append_Uni_Uni(dst, lpath, l);
#else
			REBINT clen = Decode_UTF8(UNI_HEAD(dst), lpath, l, FALSE);
			dst->tail = abs(clen);
			//Append_Bytes(dst, lpath);
#endif
			Append_Byte(dst, OS_DIR_SEP);
			OS_FREE(lpath);
		}
		out = UNI_HEAD(dst);
		n = SERIES_TAIL(dst);
	}

	// Prescan each file segment for: . .. directory names:
	// (Note the top of this loop always follows / or start)
	while (i < len) {
		if (full) {
			// Peek for: . ..
			c = GET_CHAR_UNI(uni, bp, i);
			if (c == '.') {		// .
				i++;
				c = GET_CHAR_UNI(uni, bp, i);
				if (c == '.') {	// ..
					c = GET_CHAR_UNI(uni, bp, i+1);
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
			c = GET_CHAR_UNI(uni, bp, i);
			if (c == '/') {
				if (n == 0 || out[n-1] != OS_DIR_SEP) out[n++] = OS_DIR_SEP;
				i++;
				break;
			}
			out[n++] = c;
		}
	}
	out[n] = 0;
	SERIES_TAIL(dst) = n;
//	TERM_SERIES(dst);
//	Debug_Uni(dst);

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
	return To_Local_Path(VAL_DATA(val), VAL_LEN(val), (REBOOL)!VAL_BYTE_SIZE(val), full);
}


/***********************************************************************
**
*/	REBSER *Value_To_OS_Path(REBVAL *val)
/*
**		Helper to above function.
**
***********************************************************************/
{
	REBSER *ser; // will be unicode size
#ifndef TO_WIN32
	REBSER *bin;
	REBCNT n;
#endif

	ASSERT1(ANY_BINSTR(val), RP_MISC);

	ser = To_Local_Path(VAL_DATA(val), VAL_LEN(val), (REBOOL)!VAL_BYTE_SIZE(val), TRUE);

#ifndef TO_WIN32
	// Posix needs UTF8 conversion:
	n = Length_As_UTF8(UNI_HEAD(ser), SERIES_TAIL(ser), TRUE, OS_CRLF);
	bin = Make_Binary(n + FN_PAD);
	Encode_UTF8(BIN_HEAD(bin), n+FN_PAD, UNI_HEAD(ser), &n, TRUE, OS_CRLF);
	SERIES_TAIL(bin) = n;
	TERM_SERIES(bin);
	ser = bin;
#endif

	return ser;
}
