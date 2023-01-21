/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2023 Rebol Open Source Developers
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
**  Module:  d-print.c
**  Summary: low-level console print interface
**  Section: debug
**  Author:  Carl Sassenrath
**  Notes:
**    R3 is intended to run on fairly minimal devices, so this code may
**    duplicate functions found in a typical C lib. That's why output
**    never uses standard clib printf functions.
**
***********************************************************************/

/*
		Print_OS... - low level OS output functions
		Out_...     - general console output functions
		Debug_...   - debug mode (trace) output functions
*/

#include "sys-core.h"
#include <wchar.h>

static REBREQ *Req_SIO;


/***********************************************************************
**
**	Lower Level Print Interface
**
***********************************************************************/

/***********************************************************************
**
*/	void Init_StdIO(void)
/*
***********************************************************************/
{
	//OS_CALL_DEVICE(RDI_STDIO, RDC_INIT);
	Req_SIO = OS_MAKE_DEVREQ(RDI_STDIO);
	if (!Req_SIO) Crash(RP_IO_ERROR);

	// The device is already open, so this call will just setup
	// the request fields properly.
	OS_DO_DEVICE(Req_SIO, RDC_OPEN);
}

/***********************************************************************
**
*/	void Dispose_StdIO(void)
/*
***********************************************************************/
{
	OS_FREE(Req_SIO);
	Req_SIO = NULL;
}


/***********************************************************************
**
*/	static void Print_OS_Line(REBOOL err)
/*
**		Print a new line.
**
***********************************************************************/
{
	// !!! Don't put const literal directly into mutable Req_SIO->data
	static REBYTE newline[] = "\n";

	Req_SIO->data = newline;
	Req_SIO->length = 1;
	Req_SIO->actual = 0;

	if (err)
		SET_FLAG(Req_SIO->flags, RRF_ERROR);

	OS_DO_DEVICE(Req_SIO, RDC_WRITE);

	if (Req_SIO->error) Crash(RP_IO_ERROR);
}


/***********************************************************************
**
*/	static void Prin_OS_String(const REBYTE *bp, REBLEN len, REBOOL uni, REBOOL err)
/*
**		Print a string, but no line terminator or space.
**
**		The width of the input is specified by UNI.
**
***********************************************************************/
{
	#define BUF_SIZE 1024
	REBYTE buffer[BUF_SIZE]; // on stack
	REBYTE *buf = &buffer[0];
	REBINT n;
	REBCNT len2;
	REBUNI *up = (REBUNI*)bp;

	if (!bp) Crash(RP_NO_PRINT_PTR);

	// Determine length if not provided:
	if (len == UNKNOWN) len = (uni ? (REBLEN)wcslen((const wchar_t*)up) : LEN_BYTES(bp));

	SET_FLAG(Req_SIO->flags, RRF_FLUSH);
	if (err)
		SET_FLAG(Req_SIO->flags, RRF_ERROR);

	Req_SIO->actual = 0;
	Req_SIO->data = buf;
	buf[0] = 0; // for debug tracing

	while ((len2 = len) > 0) {

		Do_Signals();

		// returns # of chars, size returns buf bytes output
		n = Encode_UTF8(buf, BUF_SIZE-4, uni ? (void*)up : (void*)bp, &len2, uni, OS_CRLF);
		if (n == 0) break;

		Req_SIO->length = len2; // byte size of buffer

		if (uni) up += n; else bp += n;
		len -= n;

		OS_DO_DEVICE(Req_SIO, RDC_WRITE);
		if (Req_SIO->error) Crash(RP_IO_ERROR);
	}
}


/***********************************************************************
**
*/  void Out_Value(REBVAL *value, REBCNT limit, REBOOL mold, REBINT lines, REBOOL err)
/*
***********************************************************************/
{
	Print_Value(value, limit, mold, err); // higher level!
	for (; lines > 0; lines--) Print_OS_Line(err);
}


/***********************************************************************
**
*/	void Out_Str(const REBYTE *bp, REBINT lines, REBOOL err)
/*
***********************************************************************/
{
	Prin_OS_String(bp, UNKNOWN, 0, err);
	for (; lines > 0; lines--) Print_OS_Line(err);
}


/***********************************************************************
**
**	Debug Print Interface
**
**		If the Trace_Buffer exists, then output goes there,
**		otherwise output goes to OS output.
**
***********************************************************************/


/***********************************************************************
**
*/	void Enable_Backtrace(REBFLG on)
/*
***********************************************************************/
{
	if (on) {
		if (Trace_Limit == 0) {
			Trace_Limit = 100000;
			Trace_Buffer = Make_Binary(Trace_Limit);
			KEEP_SERIES(Trace_Buffer, "trace-buffer"); // !!! use better way
		}
	}
	else {
		if (Trace_Limit) Free_Series(Trace_Buffer);
		Trace_Limit = 0;
		Trace_Buffer = 0;
	}
}


/***********************************************************************
**
*/	void Display_Backtrace(REBCNT lines)
/*
***********************************************************************/
{
	REBCNT tail;
	REBCNT i;

	if (Trace_Limit > 0) {
		tail = Trace_Buffer->tail;
		i = tail - 1;
		for (lines++ ;lines > 0; lines--, i--) {
			i = Find_Str_Char(Trace_Buffer, 0, i, tail, -1, LF, 0);
			if (i == NOT_FOUND || i == 0) {
				i = 0;
				break;
			}
		}

		if (lines == 0) i += 2; // start of next line
		Prin_OS_String(BIN_SKIP(Trace_Buffer, i), tail-i, 0, TRUE);
		//RESET_SERIES(Trace_Buffer);
	}
	else {
		Out_Str(cb_cast("backtrace not enabled"), 1, TRUE);
	}
}


/***********************************************************************
**
*/	void Debug_String(const REBYTE *bp, REBINT len, REBOOL uni, REBINT lines)
/*
***********************************************************************/
{
	const REBUNI *up = cast(const REBUNI*, bp);
	REBUNI uc;

	if (Trace_Limit > 0) {
		if (Trace_Buffer->tail >= Trace_Limit)
			Remove_Series(Trace_Buffer, 0, 2000);
		if (len == UNKNOWN) len = (REBINT)(uni ? wcslen((const wchar_t*)up) : LEN_BYTES(bp));
		// !!! account for unicode!
		for (; len > 0; len--) {
			uc = uni ? *up++ : *bp++;
			Append_Byte(Trace_Buffer, uc);
		}
		//Append_Bytes_Len(Trace_Buffer, bp, len);
		for (; lines > 0; lines--) Append_Byte(Trace_Buffer, LF);
	}
	else {
		Prin_OS_String(bp, len, uni, TRUE);
		for (; lines > 0; lines--) Print_OS_Line(TRUE);
	}
}


/***********************************************************************
**
*/	void Debug_Line(void)
/*
***********************************************************************/
{
	Debug_String(cb_cast(""), UNKNOWN, 0, 1);
}


/***********************************************************************
**
*/	void Debug_Str(const char *str)
/*
**		Print a string followed by a newline.
**
***********************************************************************/
{
	Debug_String(cb_cast(str), UNKNOWN, 0, 1);
}


/***********************************************************************
**
*/	void Debug_Uni(REBSER *ser)
/*
**		Print debug unicode string followed by a newline.
**
***********************************************************************/
{
	REBCNT ul;
	REBCNT bl;
	REBYTE buf[1024];
	REBUNI *up = UNI_HEAD(ser);
	REBINT size = Length_As_UTF8(up, SERIES_TAIL(ser), TRUE, OS_CRLF);

	while (size > 0) {
		ul = Encode_UTF8(buf, MIN(size, 1020), up, &bl, TRUE, OS_CRLF);
		Debug_String(buf, bl, 0, 0);
		size -= ul;
		up += ul;
	}

	Debug_Line();
}


/***********************************************************************
**
*/	void Debug_Series(REBSER *ser)
/*
***********************************************************************/
{
	if (BYTE_SIZE(ser)) Debug_Str(s_cast(BIN_HEAD(ser)));
	else Debug_Uni(ser);
}


/***********************************************************************
**
*/	void Debug_Num(REBYTE *str, REBINT num)
/*
**		Print a string followed by a number.
**
***********************************************************************/
{
	REBYTE buf[40];

	Debug_String(str, UNKNOWN, 0, 0);
	Debug_String(cb_cast(" "), 1, 0, 0);
	Form_Hex_Pad(buf, num, 8);
	Debug_Str(s_cast(buf));
}


/***********************************************************************
**
*/	void Debug_Chars(REBYTE chr, REBCNT num)
/*
**		Print a number of spaces.
**
***********************************************************************/
{
	REBYTE spaces[100];

	memset(spaces, chr, MIN(num, 99));
	spaces[num] = 0;
	Debug_String(spaces, num, 0, 0);
}


/***********************************************************************
**
*/	void Debug_Space(REBCNT num)
/*
**		Print a number of spaces.
**
***********************************************************************/
{
	if (num > 0) Debug_Chars(' ', num);
}


/***********************************************************************
**
*/	void Debug_Word(REBVAL *word)
/*
**		Print a REBOL word.
**
***********************************************************************/
{
	Debug_Str(cs_cast(Get_Word_Name(word)));
}


/***********************************************************************
**
*/	void Debug_Type(REBVAL *value)
/*
**		Print a REBOL datatype name.
**
***********************************************************************/
{
	if (VAL_TYPE(value) < REB_MAX) Debug_Str(cs_cast(Get_Type_Name(value)));
	else Debug_Str("TYPE?!");
}


/***********************************************************************
**
*/  void Debug_Value(REBVAL *value, REBCNT limit, REBOOL mold)
/*
***********************************************************************/
{
	Print_Value(value, limit, mold, TRUE); // higher level!
}


/***********************************************************************
**
*/  void Debug_Values(REBVAL *value, REBCNT count, REBCNT limit)
/*
***********************************************************************/
{
	REBSER *out;
	REBCNT i1;
	REBCNT i2;
	REBUNI uc, pc = ' ';
	REBCNT n;

	for (n = 0; n < count; n++, value++) {
		Debug_Space(1);
		if (n > 0 && VAL_TYPE(value) <= REB_NONE) Debug_Chars('.', 1);
		else {
			out = Mold_Print_Value(value, limit, TRUE, TRUE); // shared mold buffer
			for (i1 = i2 = 0; i1 < out->tail; i1++) {
				uc = GET_ANY_CHAR(out, i1);
				if (uc < ' ') uc = ' ';
				if (uc > ' ' || pc > ' ') { SET_ANY_CHAR(out, i2++, uc); }
				pc = uc;
			}
			SET_ANY_CHAR(out, i2, 0);
			Debug_String(out->data, i2, TRUE, 0);
		}
	}
	Debug_Line();
}


/***********************************************************************
**
*/	void Debug_Buf(REBCNT limit, const REBYTE *fmt, va_list args)
/*
**		Lower level formatted print for debugging purposes.
**
**		1. Does not support UNICODE.
**		2. Does not auto-expand the output buffer.
**		3. No termination buffering (limited length).
**
**		Print using a format string and variable number
**		of arguments.  All args must be long word aligned
**		(no short or char sized values unless recast to long).
**
**		Output will be held in series print buffer and
**		will not exceed its max size.  No line termination
**		is supplied after the print.
**
***********************************************************************/
{
	REBSER *buf = BUF_PRINT;
	REBCNT len;
	REBCNT n;
	REBYTE *bp;
	REBCNT tail;

	if (!buf) Crash(RP_NO_BUFFER);

	RESET_SERIES(buf);

	if (limit > SERIES_REST(buf) - 1) {
		limit = SERIES_REST(buf) - 1;
	}

	// Limits output to size of buffer, will not expand it:
	bp = Form_Var_Args(STR_HEAD(buf), limit, fmt, args);
	tail = bp - STR_HEAD(buf);

	for (n = 0; n < tail; n += len) {
		len = (REBCNT)LEN_BYTES(STR_SKIP(buf, n));
		if (len > 1024) len = 1024;
		Debug_String(STR_SKIP(buf, n), len, 0, 0);
	}
}


/***********************************************************************
**
*/	void Debug_Fmt_(const REBYTE *fmt, ...)
/*
**		Print using a formatted string and variable number
**		of arguments.  All args must be long word aligned
**		(no short or char sized values unless recast to long).
**		Output will be held in a series print buffer and
**		will not exceed its max size.  No line termination
**		is supplied after the print.
**
***********************************************************************/
{
	va_list args;
	va_start(args, fmt);
	Debug_Buf(NO_LIMIT, fmt, args);
	va_end(args);
}


/***********************************************************************
**
*/	void Debug_Fmt(const REBYTE *fmt, ...)
/*
**		Print using a formatted string and variable number
**		of arguments.  All args must be long word aligned
**		(no short or char sized values unless recast to long).
**		Output will be held in a series print buffer and
**		will not exceed its max size.  A line termination
**		is supplied after the print.
**
***********************************************************************/
{
	va_list args;
	va_start(args, fmt);
	Debug_Buf(NO_LIMIT, fmt, args);
	Debug_Line();
	va_end(args);
}
/***********************************************************************
**
*/	void Debug_Fmt_Limited(REBCNT limit, const REBYTE *fmt, ...)
/*
**		Same like Debug_Fmt, but limits length of the output.
**
***********************************************************************/
{
	va_list args;
	va_start(args, fmt);
	Debug_Buf(limit, fmt, args);
	Debug_Line();
	va_end(args);
}


/***********************************************************************
**
*/	REBINT Echo_File(REBCHR *file)
/*
***********************************************************************/
{
	Req_SIO->file.path = file;
	return OS_DO_DEVICE(Req_SIO, RDC_CREATE);
}


#ifdef unused
/***********************************************************************
**
*/	REBYTE *Input_Str(void)
/*
**		Very simple string input, limited to 255 chars.
**
***********************************************************************/
{
	static REBYTE buffer[256];
	REBINT res;

	Req_SIO->data = buffer;
	Req_SIO->length = 255;
	Req_SIO->actual = 0;
	res = OS_DO_DEVICE(Req_SIO, RDC_READ);
	if (Req_SIO->error) Crash(RP_IO_ERROR);
	//if (res > 0) Wait_Device(Req_SIO, 1000); // pending
	//if (res < 0) return 0; // error

	return buffer;
}
#endif


/***********************************************************************
**
*/	REBYTE *Form_Hex_Pad(REBYTE *buf, REBU64 val, REBINT len)
/*
**		Form an integer hex string in the given buffer with a
**		width padded out with zeros.
**		If len = 0 and val = 0, a null string is formed.
**		Does not insert a #.
**		Make sure you have room in your buffer before calling this!
**
***********************************************************************/
{
	REBYTE buffer[MAX_HEX_LEN+4];
	REBYTE *bp = (REBYTE*)(buffer + MAX_HEX_LEN + 1);
	REBU64 sgn = 0; // was: sgn = (val < 0) ? -1 : 0;

	len = MIN(len, MAX_HEX_LEN);
	*bp-- = 0;
	while (val != sgn && len > 0) {
		*bp-- = Hex_Digits[val & 0xf];
		val >>= 4;
		len--;
	}
	for (; len > 0; len--) *bp-- = (REBYTE)(sgn ? 'F' : '0');
	bp++;
	while (NZ(*buf++ = *bp++));
	return buf-1;
}


/***********************************************************************
**
*/	REBYTE *Form_Hex2(REBYTE *bp, REBCNT val)
/*
**		Convert byte-sized int to xx format. Very fast.
**
***********************************************************************/
{
	bp[0] = Hex_Digits[(val & 0xf0) >> 4];
	bp[1] = Hex_Digits[val & 0xf];
	bp[2] = 0;
	return bp+2;
}


/***********************************************************************
**
*/	REBUNI *Form_Hex2_Uni(REBUNI *up, REBCNT val)
/*
**		Convert byte-sized int to unicode xx format. Very fast.
**
***********************************************************************/
{
	up[0] = Hex_Digits[(val & 0xf0) >> 4];
	up[1] = Hex_Digits[val & 0xf];
	up[2] = 0;
	return up+2;
}


/***********************************************************************
**
*/	REBUNI *Form_Hex_Esc_Uni(REBUNI *up, REBUNI c)
/*
**		Convert byte int to %xx format (in unicode destination)
**
***********************************************************************/
{
	up[0] = '%';
	up[1] = Hex_Digits[(c & 0xf0) >> 4];
	up[2] = Hex_Digits[c & 0xf];
	up[3] = 0;
	return up+3;
}


/***********************************************************************
**
*/	REBUNI *Form_RGB_Uni(REBUNI *up, REBCNT val)
/*
**		Convert 24 bit RGB to xxxxxx format.
**
***********************************************************************/
{
#ifdef ENDIAN_LITTLE
	up[0] = Hex_Digits[(val >>  4) & 0xf];
	up[1] = Hex_Digits[val & 0xf];
	up[2] = Hex_Digits[(val >> 12) & 0xf];
	up[3] = Hex_Digits[(val >>  8) & 0xf];
	up[4] = Hex_Digits[(val >> 20) & 0xf];
	up[5] = Hex_Digits[(val >> 16) & 0xf];
#else
	up[0] = Hex_Digits[(val >>  28) & 0xf];
	up[1] = Hex_Digits[(val >> 24) & 0xf];
	up[2] = Hex_Digits[(val >> 20) & 0xf];
	up[3] = Hex_Digits[(val >> 16) & 0xf];
	up[4] = Hex_Digits[(val >> 12) & 0xf];
	up[5] = Hex_Digits[(val >>  8) & 0xf];
#endif
	up[6] = 0;

	return up+6;
}


/***********************************************************************
**
*/	REBUNI *Form_Uni_Hex(REBUNI *out, REBCNT n)
/*
**		Fast var-length hex output for uni-chars.
**		Returns next position (just past the insert).
**
***********************************************************************/
{
	REBUNI buffer[10];
	REBUNI *up = &buffer[10];

	while (n != 0) {
		*(--up) = Hex_Digits[n & 0xf];
		n >>= 4;
	}

	while (up < &buffer[10]) *out++ = *up++;

	return out;
}


/***********************************************************************
**
*/	REBYTE *Form_Var_Args(REBYTE *bp, REBCNT max, const REBYTE *fmt, va_list args)
/*
**		Lower level (debugging) value formatter.
**		Can restrict to max char size.
**
***********************************************************************/
{
	REBYTE *cp;
	REBCNT len = 0;
	REBINT pad;
	REBVAL *vp;
	REBYTE desc;
	REBSER *ser;
	REBVAL value;
	REBYTE padding;
	REBINT l;

	max--; // adjust for the fact that it adds a NULL at the end.

	//*bp++ = '!'; len++;

	for (; *fmt && len < max; fmt++) {

		// Copy string until next % escape:
		for (; *fmt && *fmt != '%' && len < max; len++) *bp++ = *fmt++;
		if (*fmt != '%') break;

		pad = 1;
		padding = ' ';
		fmt++; // skip %

pick:
		switch (desc = *fmt) {

		case '0':
			padding = '0';
		case '-':
		case '1':	case '2':	case '3':	case '4':
		case '5':	case '6':	case '7':	case '8':	case '9':
			fmt = Grab_Int((REBYTE*)fmt, &pad);
			goto pick;

		case 'd':
			l = va_arg(args, REBINT);
			cp = Form_Int_Pad(bp, (REBI64)l, max-len, pad, padding);
			len += (REBCNT)(cp - bp);
			bp = cp;
			break;

		case 'D':
			cp = Form_Int_Pad(bp, va_arg(args, REBI64), max-len, pad, padding);
			len += (REBCNT)(cp - bp);
			bp = cp;
			break;

		case 's':
			cp = va_arg(args, REBYTE *);
			if ((REBUPT)cp < 100) cp = (REBYTE*)Bad_Ptr;
			if (pad == 1) pad = (REBINT)LEN_BYTES(cp);
			if (pad < 0) {
				pad = -pad;
				pad -= (REBINT)LEN_BYTES(cp);
				for (; pad > 0 && len < max; len++, pad--) *bp++ = ' ';
			}
			for (; *cp && len < max && pad > 0; pad--, len++) *bp++ = *cp++;
			for (; pad > 0 && len < max; len++, pad--) *bp++ = ' ';
			break;

		case 'r':	// use Mold
		case 'v':	// use Form
			vp = va_arg(args, REBVAL *);
mold_value:
			// Form the REBOL value into a reused buffer:
			ser = Mold_Print_Value(vp, max, desc != 'v', TRUE);

			l = Length_As_UTF8(UNI_HEAD(ser), SERIES_TAIL(ser), TRUE, OS_CRLF);
			if (pad != 1 && l > pad) l = pad;
			if (l+len >= max) l = max-len-1;

			Encode_UTF8(bp, l, UNI_HEAD(ser), 0, TRUE, OS_CRLF);

			// Filter out CTRL chars:
			for (; l > 0; l--, bp++) if (*bp < ' ') *bp = ' ';
			break;

		case 'm':  // Mold a series
			ser = va_arg(args, REBSER *);
			Set_Block(&value, ser);
			vp = &value;
			goto mold_value;

		case 'c':
			if (len < max) {
				*bp++ = (REBYTE)va_arg(args, REBINT);
				len++;
			}
			break;

		case 'x':
			if (len + MAX_HEX_LEN + 1 < max) { // A cheat, but it is safe.
				*bp++ = '#';
				if (pad == 1) pad = 8;
				cp = Form_Hex_Pad(bp, (REBU64)(REBUPT)(va_arg(args, REBYTE*)), pad);
				len += 1 + (REBCNT)(cp - bp);
				bp = cp;
			}
			break;

		default:
			*bp++ = *fmt;
			len++;
		}
	}
	*bp = 0;
	return bp;
}


/***********************************************************************
**
**	User Output Print Interface
**
***********************************************************************/

/***********************************************************************
**
*/  void Prin_Value(REBVAL *value, REBCNT limit, REBOOL mold, REBOOL err)
/*
**		Print a value or block's contents for user viewing.
**		Can limit output to a given size. Set limit to 0 for full size.
**
***********************************************************************/
{
	REBSER *out = Mold_Print_Value(value, limit, mold, FALSE);
	Prin_OS_String(out->data, out->tail, TRUE, err);
}


/***********************************************************************
**
*/  void Print_Value(REBVAL *value, REBCNT limit, REBOOL mold, REBOOL err)
/*
**		Print a value or block's contents for user viewing.
**		Can limit output to a given size. Set limit to 0 for full size.
**
***********************************************************************/
{
	Prin_Value(value, limit, mold, err);
	Print_OS_Line(err);
}


#ifdef unused
/***********************************************************************
**
*/	static void Prin_Mold_Block(REBVAL *block, REBCNT limit)
/*
**		Can limit output to a given size. Set limit to 0 for full size.
**
***********************************************************************/
{
	REBCNT  n;

//	Reset_Mold_Buffer();
	old_Block_Series(block, BUF_MOLD, 0, 0);

	// Note: do not need to protect BUF_MOLD
	if (limit != 0 && STR_LEN(BUF_MOLD) > limit) {
		SERIES_TAIL(BUF_MOLD) = limit;
		Append_Bytes(BUF_MOLD, "...");
	}

	for (n = 0; n < SERIES_TAIL(BUF_MOLD);) {
		n = Encode_Uni_UTF8(BUF_MOLD, n, BUF_PRINT);
		Prin_OS_String(STR_HEAD(BUF_PRINT), SERIES_TAIL(BUF_PRINT));
	}
}


/***********************************************************************
**
*/  void Print_Mold_Block(REBVAL *block, REBCNT limit)
/*
**		Print a block contents for user viewing.
**		Can limit output to a given size. Set limit to 0 for full size.
**
***********************************************************************/
{
	Prin_Mold_Block(block, limit);
	Out_Line();
}

/***********************************************************************
**
*/	REBYTE *Form_Args(REBYTE *cp, REBCNT limit, REBYTE *fmt, ...)
/*
**		Format a string into a string buffer up to a maximum length.
**		Used mostly for debugging output.
**
***********************************************************************/
{
	va_list args;

	va_start(args, fmt);
	cp = Form_Var_Args(cp, limit, fmt, args);
	va_end(args);
	return cp;
}
#endif


/***********************************************************************
**
*/	void Init_Raw_Print(void)
/*
**		Initialize print module.
**
***********************************************************************/
{
	Set_Root_Series(TASK_BUF_PRINT, Make_Binary(1000), cb_cast("print buffer"));
	Set_Root_Series(TASK_BUF_FORM,  Make_Binary(64), cb_cast("form buffer"));
}
