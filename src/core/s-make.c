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
**		Memory is cleared.
**
***********************************************************************/
{
	REBSER *series = Make_Series(length + 1, sizeof(REBYTE), FALSE);
	LABEL_SERIES(series, "make binary");
	return series;
}


/***********************************************************************
**
*/	REBSER *Make_Unicode(REBCNT length)
/*
**		Make a unicode string series. Used for internal strings.
**		Add 1 extra for terminator.
**		Memory is cleared.
**
***********************************************************************/
{
	REBSER *series = Make_Series(length + 1, sizeof(REBUNI), FALSE);
	LABEL_SERIES(series, "make unicode");
	return series;
}


/***********************************************************************
**
*/	REBSER *Copy_Bytes(const REBYTE *src, REBLEN len)
/*
**		Create a string series from the given bytes.
**		Source is always latin-1 valid. Result is always 8bit.
**
***********************************************************************/
{
	REBSER *dst;

	if (len == UNKNOWN) len = LEN_BYTES(src);

	dst = Make_Binary(len);
	COPY_MEM(STR_DATA(dst), src, len);
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

#ifdef unused
/***********************************************************************
**
*/	REBSER *Copy_Wide_Str(void *src, REBINT len)
/*
**		Create a REBOL string series from a wide char string.
**		Minimize to bytes if possible
*/
{
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
}
#endif

/***********************************************************************
**
*/	REBSER *Copy_OS_Str(void *src, REBINT len)
/*
**		Create a REBOL string series from an OS native string.
**
**		Converts wide char string using UTF8 encoding.
**		Else just copy the string.
**		Marks series if not ASCII.
**
***********************************************************************/
{
#ifdef OS_WIDE_CHAR
	REBSER *ser = Encode_UTF8_String(src, len, TRUE, 0);
#else
	REBSER *ser = Copy_Bytes((REBYTE*)src, len);
#endif
	if (!Is_ASCII(BIN_HEAD(ser), BIN_LEN(ser))) UTF8_SERIES(ser);
	return ser;
}

/***********************************************************************
**
*/	REBSER *Copy_Str(const REBYTE *src, REBLEN len)
/*
**		Create a string series from the given UTF-8 encoded input.
**		Marks series if not ASCII.
**
***********************************************************************/
{
	REBSER *dst = Copy_Bytes(src, len);
	if (!Is_ASCII(src, len)) UTF8_SERIES(dst);
	return dst;
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
	ASSERT1(BYTE_SIZE(dst), RP_BAD_SIZE);

	REBCNT len = UTF8_Codepoint_Size(chr);

	if (index > dst->tail) index = dst->tail;
	Expand_Series(dst, index, len);
	Encode_UTF8_Char(STR_SKIP(dst, index), chr);
}


/***********************************************************************
**
*/	void Insert_String(REBSER *dst, REBCNT idx, REBSER *src, REBCNT pos, REBCNT len, REBFLG no_expand)
/*
**		Insert a UTF8 encoded string into a series at given index.
**		Source and destination are UTF8 encoded (BYTE_SIZE).
**
***********************************************************************/
{
	if (idx > dst->tail) idx = dst->tail;
	if (!no_expand) Expand_Series(dst, idx, len); // tail changed too

	ASSERT1(BYTE_SIZE(src), RP_BAD_SIZE);
	ASSERT1(BYTE_SIZE(dst), RP_BAD_SIZE);

	COPY_MEM(BIN_SKIP(dst, idx), BIN_SKIP(src, pos), len);
	if (IS_UTF8_SERIES(src) && !IS_UTF8_SERIES(dst) && !Is_ASCII(BIN_SKIP(src, pos), len)) {
		UTF8_SERIES(dst);
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
*/	REBSER *Copy_String(REBSER *src, REBCNT index, REBLEN length)
/*
**		Copies a portion of any string (byte or unicode).
**		Will slim the string, if needed.
**
**		The index + length must be in range unsigned int 32.
**
***********************************************************************/
{
	REBSER *dst;

	if (length == NO_LIMIT) length = src->tail;

	ASSERT1(BYTE_SIZE(src), RP_BAD_SIZE);

	dst = Make_Series(length + 1, 1, FALSE);
	Insert_String(dst, 0, src, index, length, TRUE);
	SERIES_TAIL(dst) = length;
	//No need to terminate the series, because Make_Series guarantees completely cleared memory.
	//TERM_SERIES(dst);

	if ((IS_UTF8_SERIES(src) || src == BUF_SCAN) && !Is_ASCII(BIN_DATA(dst), length)) {
		// Validate that the string ends with a valid UTF-8 byte.
		dst->tail = UTF8_Validate_Index(BIN_DATA(dst), dst->tail+1);
		STR_TERM(dst);
		UTF8_SERIES(dst);
	}

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
		REBSER *up;
		// On windows, we need to convert byte to wide:
		if (IS_UTF8_STRING(val)) {
			REBYTE *wide = NULL;
			REBLEN len = OS_Multibyte_To_Wide(VAL_BIN_DATA(val), &wide);
			if (!wide) return NULL;
			up = Make_Unicode(len);  // will be GC'd ok
			COPY_MEM(BIN_HEAD(up), wide, len * sizeof(REBUNI));
			OS_Free(wide);
			SERIES_TAIL(up) = len;
			UNI_TERM(up);
		}
		else {
			up = Copy_Bytes_To_Unicode(VAL_BIN_DATA(val), VAL_LEN(val));
		}
		return UNI_HEAD(up);
	}
	else {
		// Already wide, we can use it as-is:
		// !Assumes the OS uses same wide format!
		return VAL_UNI_DATA(val);
	}
#else
	// On Linux/Unix we can use UTF-8 directly:
	TERM_SERIES(VAL_SERIES(val)); // Make sure that it's null terminated
	return VAL_BIN_DATA(val);
#endif
}


/***********************************************************************
**
*/	REBSER *Append_Bytes_Len(REBSER *dst, const REBYTE *src, REBCNT len)
/*
**		Optimized function to append a non-encoded byte string.
**
**		If dst is null, it will be created and returned.
**		Such src strings normally come from C code or tables.
**		Destination is always 1 byte wide.
**
***********************************************************************/
{
	REBCNT tail;

	ASSERT1(dst, RP_NULL_SERIES);
	//if (!dst) dst = Make_Binary(len);
	tail = SERIES_TAIL(dst);
	EXPAND_SERIES_TAIL(dst, len); // sets the final tail!
	COPY_MEM(BIN_SKIP(dst, tail), src, len);
	return dst;
}


/***********************************************************************
**
*/	REBSER *Append_Bytes(REBSER *dst, const char *src)
/*
**		Optimized function to append a non-encoded byte string.
**		If dst is null, it will be created and returned.
**		Such src strings normally come from C code or tables.
**		Destination can be 1 or 2 bytes wide.
**
***********************************************************************/
{
	return Append_Bytes_Len(dst, cb_cast(src), (REBCNT)LEN_BYTES(cb_cast(src)));
}


/***********************************************************************
**
*/	REBSER *Append_Byte(REBSER *dst, REBCNT chr)
/*
**		Optimized function to append a character.
**		If dst is null, it will be created and returned and the
**		chr will be used to determine the width.
**
**		Destination must be 1 byte wide!
**
***********************************************************************/
{
	REBCNT tail;
	REBCNT len = UTF8_Codepoint_Size(chr);

	if (!dst) dst = Make_Binary(len);
	ASSERT1(BYTE_SIZE(dst), RP_BAD_SIZE);
	tail = SERIES_TAIL(dst);
	EXPAND_SERIES_TAIL(dst, len);
	Encode_UTF8_Char(STR_SKIP(dst, tail), chr);
	return dst;
}


/***********************************************************************
**
*/	void Append_Uni_Bytes(REBSER *dst, REBUNI *src, REBCNT len)
/*
**		Append a unicode (wide) string to a byte string. OPTIMZED.
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
	Append_Bytes(dst, cs_cast(PG_Boot_Strs[num]));
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
	Append_Bytes(dst, cs_cast(buf));
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

	Append_Bytes(dst, cs_cast(buf));
}



/***********************************************************************
**
*/	REBSER *Append_UTF8(REBSER *dst, const REBYTE *src, REBLEN len)
/*
**		Appends bytes to a string.
**
**		Result is always 8 bits wide (UTF8 encoded).
**
**		dst = null means make a new string.
**
***********************************************************************/
{
	if (len == NO_LIMIT) {
		len = LEN_BYTES(src);
	}
	if (!dst) dst = Make_Binary(len);
	REBCNT tail = SERIES_TAIL(dst);
	EXPAND_SERIES_TAIL(dst, len); // sets the final tail!
	COPY_MEM(BIN_SKIP(dst, tail), src, len);
	if (!IS_UTF8_SERIES(dst) && !Is_ASCII(src, len))
		UTF8_SERIES(dst);
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
	REBSER *series = BUF_SCAN;
	REBVAL *val;
	REBCNT tail = 0;
	REBCNT len;

	RESET_TAIL(series);

	for (val = VAL_BLK_DATA(blk); NOT_END(val); val++) {
		switch (VAL_TYPE(val)) {

		case REB_INTEGER:
			if (VAL_INT64(val) > (i64)255 || VAL_INT64(val) < 0) Trap_Range(val);
			EXPAND_SERIES_TAIL(series, 1);
			*BIN_SKIP(series, tail) = (REBYTE)VAL_INT32(val);
			break;

		case REB_BINARY:
		case REB_STRING:
		case REB_FILE:
		case REB_EMAIL:
		case REB_URL:
		case REB_TAG:
			len = VAL_LEN(val);
			EXPAND_SERIES_TAIL(series, len);
			memcpy(BIN_SKIP(series, tail), VAL_BIN_DATA(val), len);
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
