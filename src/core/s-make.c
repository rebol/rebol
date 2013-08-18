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
**  Module:  s-make.c
**  Summary: binary and unicode string support
**  Section: strings
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "sys-scan.h"


/***********************************************************************
**
*/	REBSER *Make_Binary(REBCNT length)
/*
**		Make a binary string series. For byte, C, and UTF8 strings.
**		Add 1 extra for terminator.
**
***********************************************************************/
{
	REBSER *series = Make_Series(length + 1, sizeof(REBYTE), FALSE);
	LABEL_SERIES(series, "make binary");
	BIN_DATA(series)[length] = 0;
	return series;
}


/***********************************************************************
**
*/	REBSER *Make_Unicode(REBCNT length)
/*
**		Make a unicode string series. Used for internal strings.
**		Add 1 extra for terminator.
**
***********************************************************************/
{
	REBSER *series = Make_Series(length + 1, sizeof(REBUNI), FALSE);
	LABEL_SERIES(series, "make unicode");
	UNI_HEAD(series)[length] = 0;
	return series;
}


/***********************************************************************
**
*/	REBSER *Copy_Bytes(REBYTE *src, REBINT len)
/*
**		Create a string series from the given bytes.
**		Source is always latin-1 valid. Result is always 8bit.
**
***********************************************************************/
{
	REBSER *dst;

	if (len < 0) len = LEN_BYTES(src);

	dst = Make_Binary(len);
	memcpy(STR_DATA(dst), src, len);
	SERIES_TAIL(dst) = len;
	STR_TERM(dst);

	return dst;
}


/***********************************************************************
**
*/	REBSER *Copy_Bytes_To_Unicode(REBYTE *src, REBINT len)
/*
**		Convert a byte string to a unicode string. This can
**		be used for ASCII or LATIN-8 strings.
**
***********************************************************************/
{
	REBSER *series;
	REBUNI *dst;

	series = Make_Unicode(len);
	dst = UNI_HEAD(series);
	SERIES_TAIL(series) = len;

	for (; len > 0; len--) {
		*dst++ = (REBUNI)(*src++);
	}

	UNI_TERM(series);

	return series;
}


/***********************************************************************
**
*/	REBSER *Copy_OS_Str(void *src, REBINT len)
/*
**		Create a REBOL string series from an OS native string.
**
**		For example, in Win32 with the wide char interface, we must
**		convert wide char strings, minimizing to bytes if possible.
**
**		For Linux the char string could be UTF-8, so that must be
**		converted to REBOL Unicode or Latin byte strings.
**
***********************************************************************/
{
#ifdef OS_WIDE_CHAR
	REBSER *dst;
	REBUNI *str = (REBUNI*)src;	
	if (Is_Wide(str, len)) {
		REBUNI *up;
		dst = Make_Unicode(len);
		SERIES_TAIL(dst) = len;
		up = UNI_HEAD(dst);
		while (len-- > 0) *up++ = *str++;
		*up = 0;
	}
	else {
		REBYTE *bp;
		dst = Make_Binary(len);
		SERIES_TAIL(dst) = len;
		bp = BIN_HEAD(dst);
		while (len-- > 0) *bp++ = (REBYTE)*str++;
		*bp = 0;
	}
	return dst;
#else
	return Decode_UTF_String((REBYTE*)src, len, 8);
#endif
}


/***********************************************************************
**
*/	void Widen_String(REBSER *series)
/*
**		Widen string from 1 byte to 2 bytes.
**
**		NOTE: allocates new memory. Cached pointers are invalid.
**
***********************************************************************/
{
	REBSER *uni = Make_Unicode(STR_LEN(series));
	REBUNI *up;
	REBYTE *bp;
	REBCNT n;
	REBSER tmp;

	// !!! optimize the empty case by just modifying series header??

	bp = BIN_HEAD(series);
	up = UNI_HEAD(uni);
	for (n = 0; n < STR_LEN(series); n++) up[n] = bp[n];
	SERIES_TAIL(uni) = SERIES_TAIL(series);

	// Swap series headers: // !!?? is it valid for all?
	tmp = *series;
	*series = *uni;
	*uni = tmp;
}


/***********************************************************************
**
*/	void Insert_Char(REBSER *dst, REBCNT index, REBCNT chr)
/*
**		Insert a Char (byte or unicode) into a string.
**
***********************************************************************/
{
	if (index > dst->tail) index = dst->tail;
	if (chr > 0xFF && BYTE_SIZE(dst)) Widen_String(dst);
	Expand_Series(dst, index, 1);
	SET_ANY_CHAR(dst, index, chr);
}


/***********************************************************************
**
*/	void Insert_String(REBSER *dst, REBCNT idx, REBSER *src, REBCNT pos, REBCNT len, REBFLG no_expand)
/*
**		Insert a non-encoded string into a series at given index.
**		Source and/or destination can be 1 or 2 bytes wide.
**		If destination is not wide enough, it will be widened.
**
***********************************************************************/
{
	REBUNI *up;
	REBYTE *bp;
	REBCNT n;

	if (idx > dst->tail) idx = dst->tail;
	if (!no_expand) Expand_Series(dst, idx, len); // tail changed too

	// Src and dst have same width (8 or 16):
	if (SERIES_WIDE(dst) == SERIES_WIDE(src)) {
cp_same:
		if (BYTE_SIZE(dst))
			memcpy(BIN_SKIP(dst, idx), BIN_SKIP(src, pos), len);
		else
			memcpy(UNI_SKIP(dst, idx), UNI_SKIP(src, pos), sizeof(REBUNI) * len);
		return;
	}

	// Src is 8 and dst is 16:
	if (!BYTE_SIZE(dst)) {
		bp = BIN_SKIP(src, pos);
		up = UNI_SKIP(dst, idx);
		for (n = 0; n < len; n++) up[n] = (REBUNI)bp[n];
		return;
	}

	// Src is 16 and dst is 8:
	bp = BIN_SKIP(dst, idx);
	up = UNI_SKIP(src, pos);
	for (n = 0; n < len; n++) {
		if (up[n] > 0xFF) {
			//Debug_Num("##Widen-series because char value is:", up[n]);
			// Expand dst and restart:
			idx += n;
			pos += n;
			len -= n;
			Widen_String(dst);
			goto cp_same;
		}
		bp[n] = (REBYTE)up[n];
	}
}

#ifdef not_used
/***********************************************************************
**
x*/	REBCNT Insert_Value(REBSER *series, REBCNT index, REBVAL *item, REBCNT type, REBFLG only)
/*
**		A general method to insert a value into a block, string,
**		or binary.
**
**		Returns: index past the insert.
**
***********************************************************************/
{
	REBCNT len = 1;

	if (type >= REB_BLOCK) {
		if (only || !ANY_BLOCK(item))
			Insert_Series(series, index, (void*)item, len);
		else {
			len = VAL_LEN(item);
			Insert_Series(series, index, (void*)VAL_BLK_DATA(item), len);
		}
	}
	else if (type == REB_BINARY) {
		if (IS_BINARY(item)) {
			len = VAL_LEN(item);
			Insert_String(series, index, VAL_SERIES(item), VAL_INDEX(item), len, 0);
		}
		else if (IS_INTEGER(item)) {
			Insert_Char(series, index, (0xff & VAL_INT32(item)));
		}
		else if (IS_CHAR(item)) {
			Insert_Char(series, index, (0xff & VAL_CHAR(item)));
		}
	}
	else { // other strings
		if (ANY_STR(item)) {
			len = VAL_LEN(item);
			Insert_String(series, index, VAL_SERIES(item), VAL_INDEX(item), len, 0);
		}
		else if (IS_CHAR(item)) {
			Insert_Char(series, index, VAL_CHAR(item));
		}
	}

	return index + len;
}
#endif


/***********************************************************************
**
*/	REBSER *Copy_String(REBSER *src, REBCNT index, REBINT length)
/*
**		Copies a portion of any string (byte or unicode).
**		Will slim the string, if needed.
**
**		The index + length must be in range unsigned int 32.
**
***********************************************************************/
{
	REBUNI *up;
	REBINT wide = 1;
	REBSER *dst;
	REBINT n;

	if (length < 0) length = src->tail;

	// Can it be slimmed down?
	if (!BYTE_SIZE(src)) {
		up = UNI_SKIP(src, index);
		for (n = 0; n < length; n++)
			if (up[n] > 0xff) break;
		if (n < length) wide = sizeof(REBUNI);
	}

	dst = Make_Series(length + 1, wide, FALSE);
	Insert_String(dst, 0, src, index, length, TRUE);
	SERIES_TAIL(dst) = length;
	TERM_SERIES(dst);

	return dst;
}


/***********************************************************************
**
*/	REBCHR *Val_Str_To_OS(REBVAL *val)
/*
**		This is used to pass a REBOL value string to an OS API.
**
**		The REBOL (input) string can be byte or wide sized.
**		The OS (output) string is in the native OS format.
**		On Windows, its a wide-char, but on Linux, its UTF-8.
**
**		If we know that the string can be used directly as-is,
**		(because it's in the OS size format), we can used it
**		like that.
**
***********************************************************************/
{
#ifdef OS_WIDE_CHAR
	if (VAL_BYTE_SIZE(val)) {
		// On windows, we need to convert byte to wide:
		REBINT n = VAL_LEN(val);
		REBSER *up = Make_Unicode(n);  // will be GC'd ok
		n = Decode_UTF8(UNI_HEAD(up), VAL_BIN_DATA(val), n, FALSE);
		SERIES_TAIL(up) = abs(n);
		UNI_TERM(up);
		return UNI_HEAD(up);
	}
	else {
		// Already wide, we can use it as-is:
		// !Assumes the OS uses same wide format!
		return VAL_UNI_DATA(val);
	}
#else
	if (VAL_STR_IS_ASCII(val)) {
		// On Linux/Unix we can use ASCII directly (it is valid UTF-8):
		return VAL_BIN_DATA(val);
	}
	else {
		REBINT n = VAL_LEN(val);
		REBSER *ser = Prep_Bin_Str(val, 0, &n);
		// NOTE: may return a shared buffer!
		return BIN_HEAD(ser); // (actually, it's a byte pointer)
	}
#endif
}


/***********************************************************************
**
*/	REBSER *Append_Bytes_Len(REBSER *dst, REBYTE *src, REBCNT len)
/*
**		Optimized function to append a non-encoded byte string.
**
**		If dst is null, it will be created and returned.
**		Such src strings normally come from C code or tables.
**		Destination can be 1 or 2 bytes wide.
**
***********************************************************************/
{
	REBUNI *up;
	REBCNT tail;

	if (!dst) {
		dst = Make_Binary(len);
		tail = 0;
	} else {
		tail = SERIES_TAIL(dst);
		EXPAND_SERIES_TAIL(dst, len);
	}

	if (BYTE_SIZE(dst)) {
		memcpy(STR_SKIP(dst, tail), src, len);
		STR_TERM(dst);
	}
	else {
		up = UNI_SKIP(dst, tail);
		for (; len > 0; len--) *up++ = (REBUNI)*src++;
		*up = 0;
	}

	return dst;
}


/***********************************************************************
**
*/	REBSER *Append_Bytes(REBSER *dst, REBYTE *src)
/*
**		Optimized function to append a non-encoded byte string.
**		If dst is null, it will be created and returned.
**		Such src strings normally come from C code or tables.
**		Destination can be 1 or 2 bytes wide.
**
***********************************************************************/
{
	return Append_Bytes_Len(dst, src, LEN_BYTES(src));
}


/***********************************************************************
**
*/	REBSER *Append_Byte(REBSER *dst, REBCNT chr)
/*
**		Optimized function to append a non-encoded character.
**		If dst is null, it will be created and returned and the
**		chr will be used to determine the width.
**
**		Destination can be 1 or 2 bytes wide, but DOES NOT WIDEN.
**
***********************************************************************/
{
	REBCNT tail;

	if (!dst) {
		dst = (chr > 255) ? Make_Unicode(3) : Make_Binary(3);
		tail = 0;
		SERIES_TAIL(dst) = 1;
	} else {
		tail = SERIES_TAIL(dst);
		EXPAND_SERIES_TAIL(dst, 1);
	}

	if (BYTE_SIZE(dst)) {
		*STR_SKIP(dst, tail) = (REBYTE)chr;
		STR_TERM(dst);
	}
	else {
		*UNI_SKIP(dst, tail) = (REBUNI)chr;
		UNI_TERM(dst);
	}

	return dst;
}


/***********************************************************************
**
*/	void Append_Uni_Bytes(REBSER *dst, REBUNI *src, REBCNT len)
/*
**		Append a unicode string to a byte string. OPTIMZED.
**
***********************************************************************/
{
	REBYTE *bp;
	REBCNT tail = SERIES_TAIL(dst);

	EXPAND_SERIES_TAIL(dst, len);

	bp = BIN_SKIP(dst, tail);

	for (; len > 0; len--)
		*bp++ = (REBYTE)*src++;

	*bp = 0;
}


/***********************************************************************
**
*/	void Append_Uni_Uni(REBSER *dst, REBUNI *src, REBCNT len)
/*
**		Append a unicode string to a unicode string. OPTIMZED.
**
***********************************************************************/
{
	REBUNI *up;
	REBCNT tail = SERIES_TAIL(dst);

	EXPAND_SERIES_TAIL(dst, len);

	up = UNI_SKIP(dst, tail);

	for (; len > 0; len--)
		*up++ = *src++;

	*up = 0;
}


/***********************************************************************
**
*/	void Append_String(REBSER *dst, REBSER *src, REBCNT i, REBCNT len)
/*
**		Append a byte or unicode string to a unicode string.
**
***********************************************************************/
{
	Insert_String(dst, SERIES_TAIL(dst), src, i, len, 0);
}


/***********************************************************************
**
*/	void Append_Boot_Str(REBSER *dst, REBINT num)
/*
***********************************************************************/
{
	Append_Bytes(dst, PG_Boot_Strs[num]);
}


/***********************************************************************
**
*/  void Append_Int(REBSER *dst, REBINT num)
/*
**		Append an integer string.
**
***********************************************************************/
{
	REBYTE buf[32];
	
	Form_Int(buf, num);
	Append_Bytes(dst, buf);
}


/***********************************************************************
**
*/  void Append_Int_Pad(REBSER *dst, REBINT num, REBINT digs)
/*
**		Append an integer string.
**
***********************************************************************/
{
	REBYTE buf[32];
	if (digs > 0)
		Form_Int_Pad(buf, num, digs, -digs, '0');
	else
		Form_Int_Pad(buf, num, -digs, digs, '0');

	Append_Bytes(dst, buf);
}



/***********************************************************************
**
*/	REBSER *Append_UTF8(REBSER *dst, REBYTE *src, REBINT len)
/*
**		Append (or create) decoded UTF8 to a string. OPTIMIZED.
**
**		Result can be 8 bits (latin-1 optimized) or 16 bits wide.
**
**		dst = null means make a new string.
**
***********************************************************************/
{
	REBSER *ser = BUF_UTF8;	// buffer is Unicode width

	if (len < 0) len = LEN_BYTES(src);

	Resize_Series(ser, len+1); // needs at most this much

	len = Decode_UTF8(UNI_HEAD(ser), src, len, FALSE);

	if (len < 0) {
		len = -len;
		if (!dst) dst = Make_Binary(len);
		if (BYTE_SIZE(dst)) {
			Append_Uni_Bytes(dst, UNI_HEAD(ser), len);
			return dst;
		}
	} else {
		if (!dst) dst = Make_Unicode(len);
	}

	Append_Uni_Uni(dst, UNI_HEAD(ser), len);

	return dst;
}


/***********************************************************************
**
*/  REBSER *Join_Binary(REBVAL *blk)
/*
**		Join a binary from component values for use in standard
**		actions like make, insert, or append.
**
**		WARNING: returns BUF_FORM, not a copy!
**
***********************************************************************/
{
	REBSER *series = BUF_FORM;
	REBVAL *val;
	REBCNT tail = 0;
	REBCNT len;
	void *bp;

	RESET_TAIL(series);

	for (val = VAL_BLK_DATA(blk); NOT_END(val); val++) {
		switch (VAL_TYPE(val)) {

		case REB_INTEGER:
			if (VAL_INT64(val) > (i64)255 || VAL_INT64(val) < 0) Trap_Range(val);
			EXPAND_SERIES_TAIL(series, 1);
			*BIN_SKIP(series, tail) = (REBYTE)VAL_INT32(val);
			break;

		case REB_BINARY:
			len = VAL_LEN(val);
			EXPAND_SERIES_TAIL(series, len);
			memcpy(BIN_SKIP(series, tail), VAL_BIN_DATA(val), len);
			break;

		case REB_STRING:
		case REB_FILE:
		case REB_EMAIL:
		case REB_URL:
		case REB_TAG:
			len = VAL_LEN(val);
			bp = VAL_BYTE_SIZE(val) ? VAL_BIN_DATA(val) : (REBYTE*)VAL_UNI_DATA(val);
			len = Length_As_UTF8(bp, len, (REBOOL)!VAL_BYTE_SIZE(val), 0);
			EXPAND_SERIES_TAIL(series, len);
			Encode_UTF8(BIN_SKIP(series, tail), len, bp, &len, !VAL_BYTE_SIZE(val), 0);
			series->tail = tail + len;
			break;

		case REB_CHAR:
			EXPAND_SERIES_TAIL(series, 6);
			len = Encode_UTF8_Char(BIN_SKIP(series, tail), VAL_CHAR(val));
			series->tail = tail + len;
			break;

		default:
			Trap_Arg(val);
		}

		tail = series->tail;
	}

	SET_STR_END(series, tail);

	return series;  // SHARED FORM SERIES!
}
