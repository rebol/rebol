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
**  Module:  l-scan.c
**  Summary: lexical analyzer for source to binary translation
**  Section: lexical
**  Author:  Carl Sassenrath
**  Notes:
**    WARNING WARNING WARNING
**    This is highly tuned code that should only be modified by experts
**    who fully understand its design. It is very easy to create odd
**    side effects so please be careful and extensively test all changes!
**
***********************************************************************/

#include "sys-core.h"
#include "sys-scan.h"

// In UTF8 C0, C1, F5, and FF are invalid.
#ifdef USE_UNICODE
#define LEX_UTFE LEX_DEFAULT
#else
#define LEX_UTFE LEX_WORD
#endif

/***********************************************************************
**
*/	const REBYTE Lex_Map[256] =
/*
**		Maps each character to its lexical attributes, using
**		a frequency optimized encoding.
**
**		UTF8: The values C0, C1, F5 to FF never appear.
**
***********************************************************************/
{
	/* 00 EOF */    LEX_DELIMIT|LEX_DELIMIT_END_FILE,
	/* 01     */    LEX_DEFAULT,
	/* 02     */    LEX_DEFAULT,
	/* 03     */    LEX_DEFAULT,
	/* 04     */    LEX_DEFAULT,
	/* 05     */    LEX_DEFAULT,
	/* 06     */    LEX_DEFAULT,
	/* 07     */    LEX_DEFAULT,
	/* 08 BS  */    LEX_DEFAULT,
	/* 09 TAB */    LEX_DEFAULT,
	/* 0A LF  */    LEX_DELIMIT|LEX_DELIMIT_LINEFEED,
	/* 0B     */    LEX_DEFAULT,
	/* 0C PG  */    LEX_DEFAULT,
	/* 0D CR  */    LEX_DELIMIT|LEX_DELIMIT_RETURN,
	/* 0E     */    LEX_DEFAULT,
	/* 0F     */    LEX_DEFAULT,

	/* 10     */    LEX_DEFAULT,
	/* 11     */    LEX_DEFAULT,
	/* 12     */    LEX_DEFAULT,
	/* 13     */    LEX_DEFAULT,
	/* 14     */    LEX_DEFAULT,
	/* 15     */    LEX_DEFAULT,
	/* 16     */    LEX_DEFAULT,
	/* 17     */    LEX_DEFAULT,
	/* 18     */    LEX_DEFAULT,
	/* 19     */    LEX_DEFAULT,
	/* 1A     */    LEX_DEFAULT,
	/* 1B     */    LEX_DEFAULT,
	/* 1C     */    LEX_DEFAULT,
	/* 1D     */    LEX_DEFAULT,
	/* 1E     */    LEX_DEFAULT,
	/* 1F     */    LEX_DEFAULT,

	/* 20     */    LEX_DELIMIT|LEX_DELIMIT_SPACE,
	/* 21 !   */    LEX_WORD,
	/* 22 "   */    LEX_DELIMIT|LEX_DELIMIT_QUOTE,
	/* 23 #   */    LEX_SPECIAL|LEX_SPECIAL_POUND,
	/* 24 $   */    LEX_SPECIAL|LEX_SPECIAL_DOLLAR,
	/* 25 %   */    LEX_SPECIAL|LEX_SPECIAL_PERCENT,
	/* 26 &   */    LEX_WORD,
	/* 27 '   */    LEX_SPECIAL|LEX_SPECIAL_TICK,
	/* 28 (   */    LEX_DELIMIT|LEX_DELIMIT_LEFT_PAREN,
	/* 29 )   */    LEX_DELIMIT|LEX_DELIMIT_RIGHT_PAREN,
	/* 2A *   */    LEX_WORD,
	/* 2B +   */    LEX_SPECIAL|LEX_SPECIAL_PLUS,
	/* 2C ,   */    LEX_SPECIAL|LEX_SPECIAL_COMMA,
	/* 2D -   */    LEX_SPECIAL|LEX_SPECIAL_MINUS,
	/* 2E .   */    LEX_SPECIAL|LEX_SPECIAL_PERIOD,
	/* 2F /   */    LEX_DELIMIT|LEX_DELIMIT_SLASH,

	/* 30 0   */    LEX_NUMBER|0,
	/* 31 1   */    LEX_NUMBER|1,
	/* 32 2   */    LEX_NUMBER|2,
	/* 33 3   */    LEX_NUMBER|3,
	/* 34 4   */    LEX_NUMBER|4,
	/* 35 5   */    LEX_NUMBER|5,
	/* 36 6   */    LEX_NUMBER|6,
	/* 37 7   */    LEX_NUMBER|7,
	/* 38 8   */    LEX_NUMBER|8,
	/* 39 9   */    LEX_NUMBER|9,
	/* 3A :   */    LEX_SPECIAL|LEX_SPECIAL_COLON,
	/* 3B ;   */    LEX_DELIMIT|LEX_DELIMIT_SEMICOLON,
	/* 3C <   */    LEX_SPECIAL|LEX_SPECIAL_LESSER,
	/* 3D =   */    LEX_WORD,
	/* 3E >   */    LEX_SPECIAL|LEX_SPECIAL_GREATER,
	/* 3F ?   */    LEX_WORD,

	/* 40 @   */    LEX_SPECIAL|LEX_SPECIAL_AT,
	/* 41 A   */    LEX_WORD|10,
	/* 42 B   */    LEX_WORD|11,
	/* 43 C   */    LEX_WORD|12,
	/* 44 D   */    LEX_WORD|13,
	/* 45 E   */    LEX_WORD|14,
	/* 46 F   */    LEX_WORD|15,
	/* 47 G   */    LEX_WORD,
	/* 48 H   */    LEX_WORD,
	/* 49 I   */    LEX_WORD,
	/* 4A J   */    LEX_WORD,
	/* 4B K   */    LEX_WORD,
	/* 4C L   */    LEX_WORD,
	/* 4D M   */    LEX_WORD,
	/* 4E N   */    LEX_WORD,
	/* 4F O   */    LEX_WORD,

	/* 50 P   */    LEX_WORD,
	/* 51 Q   */    LEX_WORD,
	/* 52 R   */    LEX_WORD,
	/* 53 S   */    LEX_WORD,
	/* 54 T   */    LEX_WORD,
	/* 55 U   */    LEX_WORD,
	/* 56 V   */    LEX_WORD,
	/* 57 W   */    LEX_WORD,
	/* 58 X   */    LEX_WORD,
	/* 59 Y   */    LEX_WORD,
	/* 5A Z   */    LEX_WORD,
	/* 5B [   */    LEX_DELIMIT|LEX_DELIMIT_LEFT_BRACKET,
	/* 5C \   */    LEX_SPECIAL|LEX_SPECIAL_BACKSLASH,
	/* 5D ]   */    LEX_DELIMIT|LEX_DELIMIT_RIGHT_BRACKET,
	/* 5E ^   */    LEX_WORD,
	/* 5F _   */    LEX_WORD,

	/* 60 `   */    LEX_WORD,
	/* 61 a   */    LEX_WORD|10,
	/* 62 b   */    LEX_WORD|11,
	/* 63 c   */    LEX_WORD|12,
	/* 64 d   */    LEX_WORD|13,
	/* 65 e   */    LEX_WORD|14,
	/* 66 f   */    LEX_WORD|15,
	/* 67 g   */    LEX_WORD,
	/* 68 h   */    LEX_WORD,
	/* 69 i   */    LEX_WORD,
	/* 6A j   */    LEX_WORD,
	/* 6B k   */    LEX_WORD,
	/* 6C l   */    LEX_WORD,
	/* 6D m   */    LEX_WORD,
	/* 6E n   */    LEX_WORD,
	/* 6F o   */    LEX_WORD,

	/* 70 p   */    LEX_WORD,
	/* 71 q   */    LEX_WORD,
	/* 72 r   */    LEX_WORD,
	/* 73 s   */    LEX_WORD,
	/* 74 t   */    LEX_WORD,
	/* 75 u   */    LEX_WORD,
	/* 76 v   */    LEX_WORD,
	/* 77 w   */    LEX_WORD,
	/* 78 x   */    LEX_WORD,
	/* 79 y   */    LEX_WORD,
	/* 7A z   */    LEX_WORD,
	/* 7B {   */    LEX_DELIMIT|LEX_DELIMIT_LEFT_BRACE,
	/* 7C |   */    LEX_WORD,
	/* 7D }   */    LEX_DELIMIT|LEX_DELIMIT_RIGHT_BRACE,
	/* 7E ~   */    LEX_WORD,  //LEX_SPECIAL|LEX_SPECIAL_TILDE,
	/* 7F DEL */    LEX_DEFAULT,

	/* Odd Control Chars */
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,    /* 80 */
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,

	/* Alternate Chars */
#ifdef USE_UNICODE
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
#else
	LEX_DEFAULT,LEX_WORD,LEX_WORD,LEX_WORD, /* A0 (a space) */
#endif
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,

	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,

	// C0, C1
	LEX_UTFE,LEX_UTFE,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,

	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,

	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,

	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_UTFE,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_WORD,
	LEX_WORD,LEX_WORD,LEX_WORD,LEX_UTFE
};

#ifdef LOWER_CASE_BYTE
/***********************************************************************
**
*/	const REBYTE Upper_Case[256] =
/*
**		Maps each character to its upper case value.  Done this
**		way for speed.  Note the odd cases in last block.
**
***********************************************************************/
{
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,

	 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	 96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,123,124,125,126,127,

	128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
	144,145,146,147,148,149,150,151,152,153,138,155,156,141,142,159, /* some up/low cases mod 16 (not mod 32) */
	160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
	176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,

	192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
	208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
	192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
	208,209,210,211,212,213,214,247,216,217,218,219,220,221,222,159
};


/***********************************************************************
**
*/	const REBYTE Lower_Case[256] =
/*
**		Maps each character to its lower case value.  Done this
**		way for speed.  Note the odd cases in last block.
**
***********************************************************************/
{
	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,

	 64, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
	112,113,114,115,116,117,118,119,120,121,122, 91, 92, 93, 94, 95,
	 96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
	112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,

	128,129,130,131,132,133,134,135,136,137,154,139,140,157,158,143,
	144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,255, /* some up/low cases mod 16 (not mod 32) */
	160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
	176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,

	224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
	240,241,242,243,244,245,246,215,248,249,250,251,252,253,254,223,
	224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
	240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
};
#endif


/***********************************************************************
**
*/  static REBINT Scan_Char(REBYTE **bp)
/*
**      Scan a char, handling ^A, ^/, ^(null), ^(1234)
**
**		Returns the numeric value for char, or -1 for errors.
**
**		Advances the cp to just past the last position.
**
**		test: to-integer load to-binary mold to-char 1234
**
***********************************************************************/
{
	REBINT n;
	REBYTE *cp;
	REBYTE c;
	REBYTE lex;

	c = **bp;

	// Handle unicoded char:
	if (c >= 0x80) {
		n = Decode_UTF8_Char(bp, 0); // zero on error
		(*bp)++; // skip char
		return n;
	}

	(*bp)++;

	if (c != '^') return c;

	// Must be ^ escaped char:
	c = **bp;
	(*bp)++;

    switch (c) {

	case 0:
		n = 0;
		break;

	case '/':
		n = LF;
		break;

	case '^':
		n = c;
		break;

    case '-':
		n = TAB;
		break;

	case '!':
		n = '\036'; // record separator
		break;

	case '(':	// ^(tab) ^(1234)
		// Check for hex integers ^(1234):
		cp = *bp; // restart location
		n = 0;
		while ((lex = Lex_Map[*cp]) > LEX_WORD) {
			c = lex & LEX_VALUE;
			if (!c && lex < LEX_NUMBER) break;
			n = (n << 4) + c;
			cp++;
		}
		if ((cp - *bp) > 4) return -1;
		if (*cp == ')') {
			cp++;
			*bp = cp;
			return n;
		}

		// Check for identifiers:
		for (n = 0; n < ESC_MAX; n++) {
			if (NZ(cp = Match_Bytes(*bp, (REBYTE*)(Esc_Names[n])))) {
				if (cp && *cp == ')') {
					*bp = cp + 1;
					return Esc_Codes[n];
				}
			}
		}
		return -1;

    default:
		n = UP_CASE(c);
		if (n >= '@' && n <= '_') n -= '@';
		else if (n == '~') n = 0x7f; // special for DEL
		else n = c;  // includes: ^{ ^} ^"
    }

	return n;
}


/***********************************************************************
**
*/  REBYTE *Scan_Quote(REBYTE *src, SCAN_STATE *scan_state)
/*
**      Scan a quoted string, handling all the escape characters.
**
**		The result will be put into the temporary MOLD_BUF unistring.
**
***********************************************************************/
{
    REBINT nest = 0;
	REBUNI term;
	REBINT chr;
	REBCNT lines = 0;
	REBSER *buf = BUF_MOLD;

	RESET_TAIL(buf);

	term = (*src++ == '{') ? '}' : '"';	// pick termination

	while (*src != term || nest > 0) {

		chr = *src;

        switch (chr) {

		case 0:
			return 0; // Scan_state shows error location.
        
		case '^':
			chr = Scan_Char(&src);
			if (chr == -1) return 0;
			src--;
            break;

		case '{':
			if (term != '"') nest++;
			break;

		case '}':
			if (term != '"' && nest > 0) nest--;
			break;

		case CR:
			if (src[1] == LF) src++;
			// fall thru
        case LF:
			if (term == '"') return 0;
			lines++;
			chr = LF;
			break;

		default:
			if (chr >= 0x80) {
				chr = Decode_UTF8_Char(&src, 0); // zero on error
				if (chr == 0) return 0;
			}
		}

		src++;

		*UNI_SKIP(buf, buf->tail) = chr;

		if (++(buf->tail) >= SERIES_REST(buf)) Extend_Series(buf, 1);
    }

	src++; // Skip ending quote or brace.

	if (scan_state) scan_state->line_count += lines;

	UNI_TERM(buf);

	return src;
}


/***********************************************************************
**
*/  REBYTE *Scan_Item(REBYTE *src, REBYTE *end, REBUNI term, REBYTE *invalid)
/*
**      Scan as UTF8 an item like a file or URL.
**
**		Returns continuation point or zero for error.
**
**		Put result into the MOLD_BUF as uni-chars.
**
***********************************************************************/
{
	REBUNI c;
	REBSER *buf;

	buf = BUF_MOLD;
	RESET_TAIL(buf);

	while (src < end && *src != term) {

		c = *src;

		// End of stream?
		if (c == 0) break;

		// If no term, then any white will terminate:
		if (!term && IS_WHITE(c)) break;

		// Ctrl chars are invalid:
		if (c < ' ') return 0;	// invalid char

		if (c == '\\') c = '/';

		// Accept %xx encoded char:
		else if (c == '%') {
			if (!Scan_Hex2(src+1, &c, FALSE)) return 0;
			src += 2;
		}

		// Accept ^X encoded char:
		else if (c == '^') {
			if (src+1 == end) return 0; // nothing follows ^
			c = Scan_Char(&src);
			if (!term && IS_WHITE(c)) break;
			src--;
		}

		// Accept UTF8 encoded char:
		else if (c >= 0x80) {
			c = Decode_UTF8_Char(&src, 0); // zero on error
			if (c == 0) return 0;
		}

		// Is char as literal valid? (e.g. () [] etc.)
		else if (invalid && strchr(invalid, c)) return 0;

		src++;

		*UNI_SKIP(buf, buf->tail) = c; // not affected by Extend_Series

		if (++(buf->tail) >= SERIES_REST(buf)) Extend_Series(buf, 1);
    }

	if (*src && *src == term) src++;

	UNI_TERM(buf);

	return src;
}


/***********************************************************************
**
*/  static REBYTE *Skip_Tag(REBYTE *cp)
/*
**		Skip the entire contents of a tag, including quoted strings.
**		The argument points to the opening '<'.  Zero is returned on
**		errors.
**
***********************************************************************/
{
	if (*cp == '<') cp++;
	while (*cp && *cp != '>') {
		if (*cp == '"') {
			cp++;
			while (*cp && *cp != '"') cp++;
			if (!*cp) return 0;
		}
		cp++;
	}
	if (*cp) return cp+1;
    return 0;
}


/***********************************************************************
**
*/	static void Scan_Error(REBCNT errnum, SCAN_STATE *ss, REBCNT tkn, REBYTE *arg, REBCNT size, REBVAL *relax)
/*
**		Scanner error handler
**
***********************************************************************/
{
	ERROR_OBJ *error;
	REBSER *errs;
	REBYTE *name;
	REBYTE *cp;
	REBYTE *bp;
	REBSER *ser;
	REBCNT len = 0;

	ss->errors++;

	if (PG_Boot_Strs)
		name = BOOT_STR(RS_SCAN,tkn);
	else
		name = (REBYTE*)"boot";

	cp = ss->head_line;
    while (IS_LEX_SPACE(*cp)) cp++;	// skip indentation
	bp = cp;
	while (NOT_NEWLINE(*cp)) cp++, len++;

	//DISABLE_GC;
	errs = Make_Error(errnum, 0, 0, 0);
	error = (ERROR_OBJ *)FRM_VALUES(errs);
	ser = Make_Binary(len + 16);
	Append_Bytes(ser, "(line ");
	Append_Int(ser, ss->line_count);
	Append_Bytes(ser, ") ");
	Append_Series(ser, (REBYTE*)bp, len);
	Set_String(&error->nearest, ser);
	Set_String(&error->arg1, Copy_Bytes(name, -1));
	Set_String(&error->arg2, Copy_Bytes(arg, size));

	if (relax) {
		SET_ERROR(relax, errnum, errs);
		//ENABLE_GC;
		return;
	}

	Throw_Error(errs);	// ENABLE_GC implied
}


/***********************************************************************
**
*/  static REBCNT Prescan(SCAN_STATE *scan_state)
/*
**		The general idea of this function is to break up a string
**      into tokens, with sensitivity to common token frequencies.
**      That is, find DELIMITERS, simple WORDS, and simple NUMBERS
**      rapidly.  For everything else, find the substring and note
**      the special characters that it contains.  All scans start
**      by skipping whitespace and are concluded by a delimiter.
**      A delimiter is returned only when nothing was found before
**      it (i.e. not part of other lexical tokens).
**
**      Returns a word with bit flags indicating special chars
**      that were found during the scan (other than the first
**      char, which is not part of the flags).
**      Both the beginning and ending positions are updated.
**
***********************************************************************/
{
    REBYTE *cp = scan_state->begin; /* char scan pointer */
    REBCNT flags = 0;               /* lexical flags */

    while (IS_LEX_SPACE(*cp)) cp++; /* skip white space */
    scan_state->begin = cp;         /* start of lexical symbol */

    while (1) {
        switch (GET_LEX_CLASS(*cp)) {

        case LEX_CLASS_DELIMIT:
            if (cp == scan_state->begin) cp++;  /* returning delimiter */
            scan_state->end = cp;
            return flags;

        case LEX_CLASS_SPECIAL:     /* Flag all but first special char: */
            if (cp != scan_state->begin) SET_LEX_FLAG(flags, GET_LEX_VALUE(*cp));
            cp++;
            break;

        case LEX_CLASS_WORD:
            SET_LEX_FLAG(flags, LEX_SPECIAL_WORD);  /* flags word char (for nums) */
            while (IS_LEX_AT_LEAST_WORD(*cp)) cp++; /* word or number */
            break;

        case LEX_CLASS_NUMBER:
            while (IS_LEX_AT_LEAST_NUMBER(*cp)) cp++;
            break;
        }
    }
}


/***********************************************************************
**
*/  static REBINT Scan_Token(SCAN_STATE *scan_state)
/*
**		Scan the next lexical object and determine its datatype.
**		Skip all leading whitespace and conclude on a delimiter.
**
**      Returns the value type (VT) identifying the token.
**		Negative value types indicate an error in that type.
**      Both the beginning and ending positions are updated.
**
**      Note: this function does not need to find errors in types
**      that are to be scanned and converted.  It only needs to
**      recognize that the value should be of that type. For words
**      however, since no further scanning is done, they must be
**      checked for errors here.  Same is true for delimiters.
**
***********************************************************************/
{
    REBCNT flags;
    REBYTE *cp;
    REBINT type;

    flags = Prescan(scan_state);
    cp = scan_state->begin;

    switch (GET_LEX_CLASS(*cp)) {

    case LEX_CLASS_DELIMIT:
        switch (GET_LEX_VALUE(*cp)) {
        case LEX_DELIMIT_SPACE:         /* white space (pre-processed above) */
        case LEX_DELIMIT_SEMICOLON:     /* ; begin comment */
            while (NOT_NEWLINE(*cp)) cp++;
            if (!*cp) cp--;             /* avoid passing EOF  */
			if (*cp == LF) goto line_feed;
            /* fall thru  */
        case LEX_DELIMIT_RETURN:        /* CR */
            if (cp[1] == LF) cp++;
            /* fall thru */
        case LEX_DELIMIT_LINEFEED:      /* LF */
		line_feed:
            scan_state->line_count++;
            scan_state->end = cp+1;
            return TOKEN_LINE;

        case LEX_DELIMIT_LEFT_BRACKET:  /* [ begin block */
            return TOKEN_BLOCK;

        case LEX_DELIMIT_RIGHT_BRACKET: /* ] end block */
            return TOKEN_BLOCK_END;

        case LEX_DELIMIT_LEFT_PAREN:    /* ( begin paren */
            return TOKEN_PAREN;

        case LEX_DELIMIT_RIGHT_PAREN:   /* ) end paren */
            return TOKEN_PAREN_END;

        case LEX_DELIMIT_QUOTE:         /* " quote */
            cp = Scan_Quote(cp, scan_state); // stores result string in BUF_MOLD
            goto check_str;

        case LEX_DELIMIT_LEFT_BRACE:    /* { begin quote */
            cp = Scan_Quote(cp, scan_state);  // stores result string in BUF_MOLD
        check_str:
            if (cp) {
                scan_state->end = cp;
                return TOKEN_STRING;
            } else {        /* try to recover at next new line... */
                for (cp = (scan_state->begin)+1; NOT_NEWLINE(*cp); cp++);
                scan_state->end = cp;
                return -TOKEN_STRING;
            }

        case LEX_DELIMIT_RIGHT_BRACE:   /* } end quote  !!! handle better (missing) */
            return -TOKEN_STRING;

        case LEX_DELIMIT_SLASH:         /* probably / or / *   */
            while (*cp && *cp == '/') cp++;
            if (IS_LEX_AT_LEAST_WORD(*cp) || *cp=='+' || *cp=='-' || *cp=='.') {
				// ///refine not allowed
            	if (scan_state->begin + 1 != cp) {
            		scan_state->end = cp;
            		return -TOKEN_REFINE;
            	}
				scan_state->begin = cp;
				flags = Prescan(scan_state);
				scan_state->begin--;
				type = TOKEN_REFINE;
				// Fast easy case:
		        if (ONLY_LEX_FLAG(flags, LEX_SPECIAL_WORD)) return type;
				goto scanword;
            }
			if (cp[0] == '<' || cp[0] == '>') {
	            scan_state->end = cp+1;
				return -TOKEN_REFINE;
			}
            scan_state->end = cp;
            return TOKEN_WORD;

        case LEX_DELIMIT_END_FILE:      /* end of file */
            scan_state->end--;
            return TOKEN_EOF;

        case LEX_DELIMIT_UTF8_ERROR:
        default:
            return -TOKEN_WORD;			/* just in case */
        }

    case LEX_CLASS_SPECIAL:
        if (HAS_LEX_FLAG(flags, LEX_SPECIAL_AT) && *cp != '<') return TOKEN_EMAIL;
    next_ls:
        switch (GET_LEX_VALUE(*cp)) {

        case LEX_SPECIAL_AT:
            return -TOKEN_EMAIL;

        case LEX_SPECIAL_PERCENT:       /* %filename */
            cp = scan_state->end;
            if (*cp == '"') {
				cp = Scan_Quote(cp, scan_state);  // stores result string in BUF_MOLD
				if (!cp) return -TOKEN_FILE;
				scan_state->end = cp;
				return TOKEN_FILE;
            }
            while (*cp == '/') {        /* deal with path delimiter */
                cp++;
                while (IS_LEX_AT_LEAST_SPECIAL(*cp)) cp++;
            }
            scan_state->end = cp;
            return TOKEN_FILE;

        case LEX_SPECIAL_COLON:         /* :word :12 (time) */
            if (IS_LEX_NUMBER(cp[1])) return TOKEN_TIME;
            if (ONLY_LEX_FLAG(flags, LEX_SPECIAL_WORD)) return TOKEN_GET;   /* common case */
			if (cp[1] == '\'') return -TOKEN_WORD;
			// Various special cases of < << <> >> > >= <=
			if (cp[1] == '<' || cp[1] == '>') {
				cp++;
				if (cp[1] == '<' || cp[1] == '>' || cp[1] == '=') cp++;
				if (!IS_LEX_DELIMIT(cp[1])) return -TOKEN_GET;
				scan_state->end = cp+1;
				return TOKEN_GET;
			}
            type = TOKEN_GET;
            cp++;                       /* skip ':' */
            goto scanword;

        case LEX_SPECIAL_TICK:
			if (IS_LEX_NUMBER(cp[1])) return -TOKEN_LIT;		// no '2nd
			if (cp[1] == ':') return -TOKEN_LIT;				// no ':X
            if (ONLY_LEX_FLAG(flags, LEX_SPECIAL_WORD)) return TOKEN_LIT;   /* common case */
			if (!IS_LEX_WORD(cp[1])) {
				// Various special cases of < << <> >> > >= <=
				if ((cp[1] == '-' || cp[1] == '+') && IS_LEX_NUMBER(cp[2])) return -TOKEN_WORD;
				if (cp[1] == '<' || cp[1] == '>') {
					cp++;
					if (cp[1] == '<' || cp[1] == '>' || cp[1] == '=') cp++;
					if (!IS_LEX_DELIMIT(cp[1])) return -TOKEN_LIT;
					scan_state->end = cp+1;
					return TOKEN_LIT;
				}
			}
			if (cp[1] == '\'') return -TOKEN_WORD;
            type = TOKEN_LIT;
            goto scanword;

        case LEX_SPECIAL_COMMA:         /* ,123 */
        case LEX_SPECIAL_PERIOD:        /* .123 .123.456.789 */
            SET_LEX_FLAG(flags, (GET_LEX_VALUE(*cp)));
            if (IS_LEX_NUMBER(cp[1])) goto num;
			if (GET_LEX_VALUE(*cp) != LEX_SPECIAL_PERIOD) return -TOKEN_WORD;
			type = TOKEN_WORD;
			goto scanword;

		case LEX_SPECIAL_GREATER:
			if (IS_LEX_DELIMIT(cp[1])) return TOKEN_WORD;	// RAMBO 3903 
			if (cp[1] == '>') {
				if (IS_LEX_DELIMIT(cp[2])) return TOKEN_WORD;
				return -TOKEN_WORD;
			}
		case LEX_SPECIAL_LESSER:
			if (IS_LEX_ANY_SPACE(cp[1]) || cp[1] == ']' || cp[1] == 0) return TOKEN_WORD;	// CES.9121 Was LEX_DELIMIT - changed for </tag>
			if ((cp[0] == '<' && cp[1] == '<') || cp[1] == '=' || cp[1] == '>') {
				if (IS_LEX_DELIMIT(cp[2])) return TOKEN_WORD;
				return -TOKEN_WORD;
			}
			if (GET_LEX_VALUE(*cp) == LEX_SPECIAL_GREATER) return -TOKEN_WORD;
			cp = Skip_Tag(cp);
			if (!cp) return -TOKEN_TAG;
            scan_state->end = cp;
			return TOKEN_TAG;

        case LEX_SPECIAL_PLUS:          /* +123 +123.45 +$123 */
        case LEX_SPECIAL_MINUS:         /* -123 -123.45 -$123 */
            if (HAS_LEX_FLAG(flags, LEX_SPECIAL_AT)) return TOKEN_EMAIL;
            if (HAS_LEX_FLAG(flags, LEX_SPECIAL_DOLLAR)) return TOKEN_MONEY;
            if (HAS_LEX_FLAG(flags, LEX_SPECIAL_COLON)) {
	            cp = Skip_To_Char(cp, scan_state->end, ':');
				if (cp && (cp+1) != scan_state->end) return TOKEN_TIME;  /* 12:34 */
			    cp = scan_state->begin;
				if (cp[1] == ':') {		// +: -:
                    type = TOKEN_WORD;
                    goto scanword;
				}
			}
            cp++;
            if (IS_LEX_AT_LEAST_NUMBER(*cp)) goto num;
            if (IS_LEX_SPECIAL(*cp)) {
                if ((GET_LEX_VALUE(*cp)) >= LEX_SPECIAL_PERIOD) goto next_ls;
/*              if (*cp == '#') goto hex; */
                if (*cp == '+' || *cp == '-') {
                    type = TOKEN_WORD;
                    goto scanword;
                }
                return -TOKEN_WORD;
            }
            type = TOKEN_WORD;
            goto scanword;

        case LEX_SPECIAL_POUND:
        pound:
            cp++;
/*        hex:
          if (HAS_LEX_FLAGS(flags, ~(LEX_FLAG(LEX_SPECIAL_POUND) | LEX_FLAG(LEX_SPECIAL_PERIOD)
                        | LEX_FLAG(LEX_SPECIAL_TICK) | LEX_FLAG(LEX_SPECIAL_WORD)))) return -TOKEN_INTEGER;
*/
/*        if (HAS_LEX_FLAG(flags, LEX_SPECIAL_PERIOD)) return TOKEN_BYTES; */
			if (*cp == '[') {
				scan_state->end = ++cp;
				return TOKEN_CONSTRUCT;
			}
			if (*cp == '"') { /* CHAR #"C" */
				cp++;
				type = Scan_Char(&cp); 
				if (type >= 0 && *cp == '"') {
					scan_state->end = cp+1;
					return TOKEN_CHAR;
				} else {        /* try to recover at next new line... */
					for (cp = (scan_state->begin)+1; NOT_NEWLINE(*cp); cp++);
					scan_state->end = cp;
					return -TOKEN_CHAR;
				}
			}
			if (*cp == '{') { /* BINARY #{12343132023902902302938290382} */
				scan_state->end = scan_state->begin;  /* save start */
				scan_state->begin = cp;
	            cp = Scan_Quote(cp, scan_state);  // stores result string in BUF_MOLD !!??
				scan_state->begin = scan_state->end;  /* restore start */
				if (cp) {
					scan_state->end = cp;
					return TOKEN_BINARY;
				} else {        /* try to recover at next new line... */
					for (cp = (scan_state->begin)+1; NOT_NEWLINE(*cp); cp++);
					scan_state->end = cp;
					return -TOKEN_BINARY;
				}
			}
			if (cp-1 == scan_state->begin) return TOKEN_ISSUE;
			else return -TOKEN_INTEGER;

        case LEX_SPECIAL_DOLLAR:
            if (HAS_LEX_FLAG(flags, LEX_SPECIAL_AT)) return TOKEN_EMAIL;
            return TOKEN_MONEY;

        default:
            return -TOKEN_WORD;
        }

    case LEX_CLASS_WORD:
        if (ONLY_LEX_FLAG(flags, LEX_SPECIAL_WORD)) return TOKEN_WORD;
        type = TOKEN_WORD;
        goto scanword;

    case LEX_CLASS_NUMBER:      /* order of tests is important */
    num:
		if (!flags) return TOKEN_INTEGER;       /* simple integer */
		if (HAS_LEX_FLAG(flags, LEX_SPECIAL_AT)) return TOKEN_EMAIL;
		if (HAS_LEX_FLAG(flags, LEX_SPECIAL_POUND)) {
			if (cp == scan_state->begin) { // no +2 +16 +64 allowed
				if (
					   (cp[0] == '6' && cp[1] == '4' && cp[2] == '#' && cp[3] == '{')
					|| (cp[0] == '1' && cp[1] == '6' && cp[2] == '#' && cp[3] == '{') // rare
				) {cp += 2; goto pound;}
				if (cp[0] == '2' && cp[1] == '#' && cp[2] == '{')
					{cp++; goto pound;} // very rare
			}
			return -TOKEN_INTEGER;
		}
		if (HAS_LEX_FLAG(flags, LEX_SPECIAL_COLON)) return TOKEN_TIME;  /* 12:34 */
		if (HAS_LEX_FLAG(flags, LEX_SPECIAL_PERIOD)) {  /* 1.2 1.2.3 1,200.3 1.200,3 1.E-2 */
			if (Skip_To_Char(cp, scan_state->end, 'x')) return TOKEN_PAIR;
			cp = Skip_To_Char(cp, scan_state->end, '.');
			if (!(HAS_LEX_FLAG(flags, LEX_SPECIAL_COMMA)) &&        /* no comma in bytes */
				Skip_To_Char(cp+1, scan_state->end, '.')) return TOKEN_TUPLE;
			return TOKEN_DECIMAL;
		}
		if (HAS_LEX_FLAG(flags, LEX_SPECIAL_COMMA)) {
			if (Skip_To_Char(cp, scan_state->end, 'x')) return TOKEN_PAIR;
			return TOKEN_DECIMAL;  /* 1,23 */
		}
		if (HAS_LEX_FLAG(flags, LEX_SPECIAL_POUND)) {   /* -#123 2#1010        */
			if (HAS_LEX_FLAGS(flags, ~(LEX_FLAG(LEX_SPECIAL_POUND) | LEX_FLAG(LEX_SPECIAL_PERIOD) | LEX_FLAG(LEX_SPECIAL_TICK)))) return -TOKEN_INTEGER;
			if (HAS_LEX_FLAG(flags, LEX_SPECIAL_PERIOD)) return TOKEN_TUPLE;
			return TOKEN_INTEGER;
		}
		/* Note: cannot detect dates of the form 1/2/1998 because they
		** may appear within a path, where they are not actually dates!
		** Special parsing is required at the next level up. */
		for (;cp != scan_state->end; cp++) {    /* what do we hit first? 1-AUG-97 or 123E-4 */
			if (*cp == '-') return TOKEN_DATE;      /* 1-2-97 1-jan-97 */
			if (*cp == 'x' || *cp == 'X') return TOKEN_PAIR; // 320x200
			if (*cp == 'E' || *cp == 'e') {
				if (Skip_To_Char(cp, scan_state->end, 'x')) return TOKEN_PAIR;
				return TOKEN_DECIMAL; /* 123E4 */
			}
			if (*cp == '%') return TOKEN_PERCENT;
		}
		/*cp = scan_state->begin;*/
		if (HAS_LEX_FLAG(flags, LEX_SPECIAL_TICK)) return TOKEN_INTEGER; /* 1'200 */
		return -TOKEN_INTEGER;

    default:
        return -TOKEN_WORD;
    }

#if ndef	// unreachable code
    /* avoid '123 :123 from scanning as a word.... */
    if (IS_LEX_WORD(cp[1]) && !HAS_LEX_FLAGS(flags, LEX_WORD_FLAGS))
        return TOKEN_LIT;
    return -TOKEN_WORD;
#endif

scanword:
    if (HAS_LEX_FLAG(flags, LEX_SPECIAL_COLON)) {    /* word:  url:words */
        if (type != TOKEN_WORD) return type; //-TOKEN_WORD;  /* only valid with WORD (not set or lit) */
        cp = Skip_To_Char(cp, scan_state->end, ':'); /* always returns a pointer (always a ':') */
        if (cp[1] != '/' && Lex_Map[(REBYTE)cp[1]] < LEX_SPECIAL) { /* a valid delimited word SET? */
            if (HAS_LEX_FLAGS(flags, ~LEX_FLAG(LEX_SPECIAL_COLON) & LEX_WORD_FLAGS)) return -TOKEN_WORD;
            return TOKEN_SET;
        }
        cp = scan_state->end;   /* then, must be a URL */
        while (*cp == '/') {    /* deal with path delimiter */
            cp++;
            while (IS_LEX_AT_LEAST_SPECIAL(*cp) || *cp == '/') cp++;
        }
        scan_state->end = cp;
        return TOKEN_URL;
    }
    if (HAS_LEX_FLAG(flags, LEX_SPECIAL_AT)) return TOKEN_EMAIL;
    if (HAS_LEX_FLAG(flags, LEX_SPECIAL_DOLLAR)) return TOKEN_MONEY;
    if (HAS_LEX_FLAGS(flags, LEX_WORD_FLAGS)) return -type;   /* has chars not allowed in word (eg % \ ) */
	if (HAS_LEX_FLAG(flags, LEX_SPECIAL_LESSER)) {
		// Allow word<tag> and word</tag> but not word< word<= word<> etc.
        cp = Skip_To_Char(cp, scan_state->end, '<');
		if (cp[1] == '<' || cp[1] == '>' || cp[1] == '=' ||
			IS_LEX_SPACE(cp[1]) || (cp[1] != '/' && IS_LEX_DELIMIT(cp[1])))
			return -type;
		/*bogus: if (HAS_LEX_FLAG(flags, LEX_SPECIAL_GREATER) &&
			Skip_To_Char(scan_state->begin, cp, '>')) return -TOKEN_WORD; */
		scan_state->end = cp;
	} else if (HAS_LEX_FLAG(flags, LEX_SPECIAL_GREATER)) return -type;
    return type;
}


/***********************************************************************
**
*/  static void Init_Scan_State(SCAN_STATE *scan_state, REBYTE *cp, REBCNT limit)
/*
**		Initialize a scanner state structure.  Set the standard
**		scan pointers and the limit pointer.
**
***********************************************************************/
{
    scan_state->head_line = scan_state->begin = scan_state->end = cp;
    scan_state->limit = cp + limit;
    scan_state->line_count = 1;
	scan_state->opts = 0;
	scan_state->errors = 0;
//    scan_state->error_id = (REBYTE *)"";
}


/***********************************************************************
**
*/	static REBINT Scan_Head(SCAN_STATE *scan_state)
/*
**		Search text for a REBOL header.  It is distinguished as
**		the word REBOL followed by a '[' (they can be separated
**		only by lines and comments).  There can be nothing on the
**		line before the header.  Also, if a '[' preceedes the
**		header, then note its position (for embedded code).
**		The scan_state begin pointer is updated to point to the header block.
**		Keep track of line-count.
**
**		Returns:
**			0 if no header,
**			1 if header,
**		   -1 if embedded header (inside []).
**
**		The scan_state structure is updated to point to the
**		beginning of the source text.
**
***********************************************************************/
{
	REBYTE *rp = 0;   /* pts to the REBOL word */
	REBYTE *bp = 0;   /* pts to optional [ just before REBOL */
    REBYTE *cp = scan_state->begin;
	REBCNT count = scan_state->line_count;

	while (TRUE) {
        while (IS_LEX_SPACE(*cp)) cp++; /* skip white space */
		switch (*cp) {
		case '[':
            if (rp) {
                scan_state->begin = ++cp; //(bp ? bp : cp);
				scan_state->line_count = count;
                return (bp ? -1 : 1);
            }
			bp = cp++;
			break;
        case 'R':
		case 'r':
			if (Match_Bytes(cp, (REBYTE *)&Str_REBOL[0])) {
				rp = cp;
                cp += 5;
				break;
            }
            cp++;
			bp = 0;	/* prior '[' was a red herring */
			/* fall thru... */
        case ';':
            goto skipline;
        case 0:
			return 0;
        default:	/* everything else... */
			if NOT_NEWLINE(*cp) rp = bp = 0;
		skipline:
			while NOT_NEWLINE(*cp) cp++;
			if (*cp == CR && cp[1] == LF) cp++;
			if (*cp) cp++;
			count++;
            break;
		}
	}
}

#ifdef not_used
//!!!
/***********************************************************************
**
	REBOOL Construct_Simple(REBVAL *value, REBSER *spec)
/*
**		Handle special #[type] constructs. These are used to
**		boot REBOL, so must not require binding.
**
***********************************************************************/
{
	REBVAL *blk = BLK_HEAD(spec);
	if (!IS_WORD(blk)) return FALSE;
	switch (VAL_WORD_SYM(blk)-1) {
	case SYM_NONE:
		SET_NONE(value);
		break;
	case SYM_FALSE:
		SET_LOGIC(value, FALSE);
		break;
	case SYM_TRUE:
		SET_LOGIC(value, TRUE);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
#endif

extern REBSER *Scan_Full_Block(SCAN_STATE *scan_state, REBYTE mode_char);

/***********************************************************************
**
*/  static REBSER *Scan_Block(SCAN_STATE *scan_state, REBYTE mode_char)
/*
**		Scan a block (or paren) and return it.
**		Sub scanners may return bad by setting value type to zero.
**
***********************************************************************/
{
    REBINT token;
    REBCNT len;
    REBYTE *bp;
	REBYTE *ep;
	REBVAL *value = 0;
	REBSER *emitbuf = BUF_EMIT;
	REBSER *block;
	REBCNT begin = emitbuf->tail;	// starting point in block buffer
	REBOOL line = FALSE;
#ifdef COMP_LINES
	REBINT linenum;
#endif
	REBCNT start = scan_state->line_count;
	REBYTE *start_line = scan_state->head_line;
	// just_once for load/next see Load_Script for more info.
	REBOOL just_once = GET_FLAG(scan_state->opts, SCAN_NEXT);

	CHECK_STACK(&token);
	
	if (just_once)
		CLR_FLAG(scan_state->opts, SCAN_NEXT); // no deeper

	//scan_state->error_id = (REBYTE *) "";

    while (
#ifdef COMP_LINES
		linenum=scan_state->line_count,
#endif
		((token = Scan_Token(scan_state)) != TOKEN_EOF)
	) {

		bp = scan_state->begin;
		ep = scan_state->end;
		len = (REBCNT)(ep - bp);

		if (token < 0) {	// Check for error tokens
			token = -token;
			ACCEPT_TOKEN(scan_state);
			goto syntax_error;
		}

		// Is output block buffer large enough?
		if (token >= TOKEN_WORD && SERIES_FULL(emitbuf))
			Extend_Series(emitbuf, 1024);

		value = BLK_TAIL(emitbuf);
		SET_END(value);
		// Line opt was set here. Moved to end in 3.0.

        // If in a path, handle start of path /word or word//word cases:
        if (mode_char == '/' && *bp == '/') {
			SET_NONE(value);
			emitbuf->tail++;
			scan_state->begin = bp + 1;
			continue;
        }

        // Check for new path: /word or word/word:
        if ((token == TOKEN_PATH || ((token == TOKEN_WORD || token == TOKEN_LIT ||
				token == TOKEN_GET) && *ep == '/'))
					&& mode_char != '/') {
			//line = VAL_GET_LINE(value);
			block = Scan_Block(scan_state, '/');  // (could realloc emitbuf)
			value = BLK_TAIL(emitbuf);
			VAL_SERIES(value) = block;
			if (token == TOKEN_LIT) {
				token = REB_LIT_PATH;
				VAL_SET(BLK_HEAD(block), REB_WORD); // NO_FRAME
			}
			else if (IS_GET_WORD(BLK_HEAD(block))) {
				if (*scan_state->end == ':') goto syntax_error;
				token = REB_GET_PATH;
				VAL_SET(BLK_HEAD(block), REB_WORD); // NO_FRAME
			}
			else {
				if (*scan_state->end == ':') {
					token = REB_SET_PATH;
					scan_state->begin = ++(scan_state->end);
				} else token = REB_PATH;
			}
//			if (IS_SET_WORD(BLK_SKIP(block, block->tail - 1)
			VAL_SET(value, token);
			VAL_INDEX(value) = 0;
			//if (line) line = FALSE, VAL_SET_LINE(value);
			token = TOKEN_PATH;
        } else {
            ACCEPT_TOKEN(scan_state);
        }

		// Process each lexical token appropriately:
		switch (token) {  // (idea is that compiler selects computed branch)

		case TOKEN_LINE:
			#ifdef TEST_SCAN
			Wait_User("next...");
			#endif
			line = TRUE;
			scan_state->head_line = ep;
			continue;

		case TOKEN_LIT:
		case TOKEN_GET:
			if (ep[-1] == ':') {
				if (len == 1 || mode_char != '/') goto syntax_error;
				len--, scan_state->end--;
			}
			bp++;
		case TOKEN_SET:
			len--;
			if (mode_char == '/' && token == TOKEN_SET) {
				token = TOKEN_WORD; // will be a PATH_SET
				scan_state->end--;	// put ':' back on end but not beginning
			}
		case TOKEN_WORD:
			if (len == 0) {bp--; goto syntax_error;}
			VAL_SET(value, (REBYTE)(REB_WORD + (token - TOKEN_WORD))); // NO_FRAME
			if (!(VAL_WORD_SYM(value) = Make_Word(bp, len))) goto syntax_error;
			VAL_WORD_FRAME(value) = 0;
			break;

		case TOKEN_REFINE:
			VAL_SET(value, REB_REFINEMENT); // NO_FRAME
			if (!(VAL_WORD_SYM(value) = Make_Word(bp+1, len-1))) goto syntax_error;
			break;

		case TOKEN_ISSUE:
			if (len == 1) {
				if (bp[1] == '(') {token = TOKEN_CONSTRUCT; goto syntax_error;}
				SET_NONE(value);  // A single # means NONE
			}
			else {
				VAL_SET(value, REB_ISSUE); // NO_FRAME
				if (!(VAL_WORD_SYM(value) = Scan_Issue(bp+1, len-1))) goto syntax_error;
			}
			break;

		case TOKEN_BLOCK:
		case TOKEN_PAREN:
			//line = VAL_GET_LINE(value);
			block = Scan_Block(scan_state, (REBYTE)((token == TOKEN_BLOCK) ? ']' : ')'));
			// (above line could have realloced emitbuf)
			ep = scan_state->end;
			value = BLK_TAIL(emitbuf);
			if (scan_state->errors) {
				*value = *BLK_LAST(block); // Copy the error
				emitbuf->tail++;
				goto exit_block;
			}
			VAL_SERIES(value) = block;
			VAL_SET(value, (REBYTE)((token == TOKEN_BLOCK) ? REB_BLOCK : REB_PAREN));
			VAL_INDEX(value) = 0;
			//if (line) line = FALSE, VAL_SET_LINE(value);
			break;

		case TOKEN_PATH:
			break;

		case TOKEN_BLOCK_END:
			if (!mode_char) { mode_char = '['; goto extra_error; }
			else if (mode_char != ']') goto missing_error;
			else goto exit_block;

		case TOKEN_PAREN_END:
			if (!mode_char) { mode_char = '('; goto extra_error; }
			else if (mode_char != ')') goto missing_error;
			else goto exit_block;

		case TOKEN_INTEGER:		// or start of DATE
            if (*ep != '/' || mode_char == '/') {
				if (0 == Scan_Integer(bp, len, value))
					goto syntax_error;
			}
			else {				// A / and not in block
				token = TOKEN_DATE;
				while (*ep == '/' || IS_LEX_AT_LEAST_SPECIAL(*ep)) ep++;
				scan_state->begin = ep;
				len = (REBCNT)(ep - bp);
				if (ep != Scan_Date(bp, len, value)) goto syntax_error;
            }
			break;

		case TOKEN_DECIMAL:
		case TOKEN_PERCENT:
			// Do not allow 1.2/abc:
			if (*ep == '/' || !Scan_Decimal(bp, len, value, 0)) goto syntax_error;
			if (bp[len-1] == '%') {
				VAL_SET(value, REB_PERCENT);
				VAL_DECIMAL(value) /= 100.0;
			}
			break;

		case TOKEN_MONEY:
			// Do not allow $1/$2:
			if (*ep == '/') {ep++; goto syntax_error;}
			if (!Scan_Money(bp, len, value)) goto syntax_error;
			break;

		case TOKEN_TIME:
			if (bp[len-1] == ':' && mode_char == '/') {	// could be path/10: set
				if (!Scan_Integer(bp, len-1, value)) goto syntax_error;
				scan_state->end--;	// put ':' back on end but not beginning
				break;
			}
			if (ep != Scan_Time(bp, len, value)) goto syntax_error;
			break;

		case TOKEN_DATE:
			while (*ep == '/' && mode_char != '/') {  // Is it a date/time?
				ep++;
				while (IS_LEX_AT_LEAST_SPECIAL(*ep)) ep++;
				len = (REBCNT)(ep - bp);
				if (len > 50) break; // prevent inf-loop - should never be longer than this
				scan_state->begin = ep;  // End point extended to cover time
            }
			if (ep != Scan_Date(bp, len, value)) goto syntax_error;
			break;

		case TOKEN_CHAR:
			bp += 2; // skip #"
			VAL_CHAR(value) = Scan_Char(&bp);
			bp++; // skip end "
			VAL_SET(value, REB_CHAR);
			break;

		case TOKEN_STRING:
			// During scan above, string was stored in BUF_MOLD (with Uni width)
			Set_String(value, Copy_String(BUF_MOLD, 0, -1));
			LABEL_SERIES(VAL_SERIES(value), "scan string");
			break;

		case TOKEN_BINARY:
			Scan_Binary(bp, len, value);
			LABEL_SERIES(VAL_SERIES(value), "scan binary");
			break;

		case TOKEN_PAIR:
			Scan_Pair(bp, len, value);
			break;

		case TOKEN_TUPLE:
			if (!Scan_Tuple(bp, len, value)) goto syntax_error;
			break;

		case TOKEN_FILE:
			Scan_File(bp, len, value);
			LABEL_SERIES(VAL_SERIES(value), "scan file");
			break;

		case TOKEN_EMAIL:
			Scan_Email(bp, len, value);
			LABEL_SERIES(VAL_SERIES(value), "scan email");
			break;

		case TOKEN_URL:
			Scan_URL(bp, len, value);
			LABEL_SERIES(VAL_SERIES(value), "scan url");
			break;

		case TOKEN_TAG:
			Scan_Any(bp+1, len-2, value, REB_TAG);
			LABEL_SERIES(VAL_SERIES(value), "scan tag");
			break;

		case TOKEN_CONSTRUCT:
			block = Scan_Full_Block(scan_state, ']');
			value = BLK_TAIL(emitbuf);
			emitbuf->tail++; // Protect the block from GC
//			if (!Construct_Simple(value, block)) {
			Bind_Block(Lib_Context, BLK_HEAD(block), BIND_ALL|BIND_DEEP);
			//Bind_Global_Block(BLK_HEAD(block));
			if (!Construct_Value(value, block)) {
				if (IS_END(value)) Set_Block(value, block);
				Trap1(RE_MALCONSTRUCT, value);
			}
			emitbuf->tail--; // Unprotect
			break;

		case TOKEN_EOF: continue;

		default: ;
			SET_NONE(value);
		}

		if (line) {
			line = FALSE;
			VAL_SET_LINE(value);
		}

		#ifdef TEST_SCAN
		Print((REBYTE*)"%s - %s", Token_Names[token], Use_Buf(bp,ep));
		if (VAL_TYPE(value) >= REB_STRING && VAL_TYPE(value) <= REB_URL)
			Print_Str(VAL_BIN(value));
		//Wait_User(0);
		#endif

#ifdef COMP_LINES
		VAL_LINE(value)=linenum;
		VAL_FLAGS(value)|=FLAGS_LINE;
#endif
		if (VAL_TYPE(value)) emitbuf->tail++;
		else {
		syntax_error:
			value = BLK_TAIL(emitbuf);
			Scan_Error(RE_INVALID, scan_state, (REBCNT)token, bp, (REBCNT)(ep-bp), GET_FLAG(scan_state->opts, SCAN_RELAX) ? value : 0);
			emitbuf->tail++;
			goto exit_block;
		missing_error:
			scan_state->line_count = start;	// where block started
			scan_state->head_line = start_line;
		extra_error: {
				REBYTE tmp_buf[4];	// Temporary error string
				tmp_buf[0] = mode_char;
				tmp_buf[1] = 0;
				value = BLK_TAIL(emitbuf);
				Scan_Error(RE_MISSING, scan_state, (REBCNT)token, tmp_buf, 1, GET_FLAG(scan_state->opts, SCAN_RELAX) ? value : 0);
				emitbuf->tail++;
				goto exit_block;
			}
		}

		// Check for end of path:
		if (mode_char == '/') {
			if (*ep == '/') {
				ep++;
				scan_state->begin = ep;  // skip next /
				if (*ep != '(' && IS_LEX_DELIMIT(*ep)) {
					token = TOKEN_PATH;
					goto syntax_error;
				}
			}
			else goto exit_block;
		}

		// Added for load/next
	    if (GET_FLAG(scan_state->opts, SCAN_ONLY) || just_once) goto exit_block;
	}

    if (mode_char == ']' || mode_char == ')') goto missing_error;

exit_block:
	if (line && value) VAL_SET_LINE(value);
	#ifdef TEST_SCAN
	Print((REBYTE*)"block of %d values ", emitbuf->tail - begin); //Wait_User("...");
	#endif

	len = emitbuf->tail;
	block = Copy_Values(BLK_SKIP(emitbuf, begin), len - begin);
	LABEL_SERIES(block, "scan block");
	SERIES_SET_FLAG(block, SER_MON);
	emitbuf->tail = begin;
//!!!!	if (value) VAL_OPTS(BLK_TAIL(block)) = VAL_OPTS(value); // save NEWLINE marker

	return block;
}


/***********************************************************************
**
*/  REBSER *Scan_Full_Block(SCAN_STATE *scan_state, REBYTE mode_char)
/*
**		Simple variation of scan_block to avoid problem with
**		construct of aggregate values.
**
***********************************************************************/
{
	REBFLG only = GET_FLAG(scan_state->opts, SCAN_ONLY);
	REBSER *ser;
	CLR_FLAG(scan_state->opts, SCAN_ONLY);
	ser = Scan_Block(scan_state, mode_char);
	if (only) SET_FLAG(scan_state->opts, SCAN_ONLY);
	return ser;
}


/***********************************************************************
**
*/	REBSER *Scan_Code(SCAN_STATE *scan_state, REBYTE mode_char)
/*
**		Scan source code, given a scan state. Allows scan of source
**		code a section at a time (used for LOAD/next).
**
**		Note: Renamed this from Scan_Trap (a bad name, no trap used)
**
***********************************************************************/
{
//	REBSER *ser;

	BLK_RESET(BUF_EMIT); // Prevents growth (when errors are thrown)
	return Scan_Block(scan_state, mode_char);
//	Set_Block(Temp_Scan_Value, ser);
//	return Temp_Scan_Value;
}


/***********************************************************************
**
*/	REBSER *Scan_Source(REBYTE *src, REBCNT len)
/*
**		Scan source code. Scan state initialized. No header required.
**		If len = 0, then use the C string terminated length.
**
***********************************************************************/
{
    SCAN_STATE scan_state;

	Check_Stack();
	if (!len) len = LEN_BYTES(src);
    Init_Scan_State(&scan_state, src, len);
	return Scan_Code(&scan_state, 0);
}


/***********************************************************************
**
*/	REBINT Scan_Header(REBYTE *src, REBCNT len)
/*
**		Scan for header, return its offset if found or -1 if not.
**
***********************************************************************/
{
    SCAN_STATE scan_state;
	REBYTE *cp;
	REBINT result;

	// Must be UTF8 byte-stream:
    Init_Scan_State(&scan_state, src, len);
	result = Scan_Head(&scan_state);
	if (!result) return -1;

	cp = scan_state.begin-2;
	// Backup to start of it:
	if (result > 0) { // normal header found
		while (cp != src && *cp != 'r' && *cp != 'R') cp--;
	} else {
		while (cp != src && *cp != '[') cp--;
	}
	return (REBINT)(cp - src);
}


/***********************************************************************
**
*/	void Init_Scanner(void)
/*
***********************************************************************/
{
	Set_Root_Series(TASK_BUF_EMIT, Make_Block(511), "emit block");
	Set_Root_Series(TASK_BUF_UTF8, Make_Unicode(1020), "utf8 buffer");
}


/***********************************************************************
**
*/	REBNATIVE(transcode)
/*
**		Allows BINARY! input only!
**
***********************************************************************/
{
	REBSER *blk;
    SCAN_STATE scan_state;

    Init_Scan_State(&scan_state, VAL_BIN_DATA(D_ARG(1)), VAL_LEN(D_ARG(1)));

	if (D_REF(2)) SET_FLAG(scan_state.opts, SCAN_NEXT);
	if (D_REF(3)) SET_FLAG(scan_state.opts, SCAN_ONLY);
	if (D_REF(4)) SET_FLAG(scan_state.opts, SCAN_RELAX);

	blk = Scan_Code(&scan_state, 0);
	DS_RELOAD(ds); // in case stack moved
	Set_Block(D_RET, blk);

	VAL_INDEX(D_ARG(1)) = scan_state.end - VAL_BIN(D_ARG(1));
	Append_Val(blk, D_ARG(1));

	return R_RET;
}


/***********************************************************************
**
*/  REBCNT Scan_Word(REBYTE *cp, REBCNT len)
/*
**		Scan word chars and make word symbol for it.
**		This method gets exactly the same results as scanner.
**		Returns symbol number, or zero for errors.
**
***********************************************************************/
{
    SCAN_STATE scan_state;

	Init_Scan_State(&scan_state, cp, len);

	if (TOKEN_WORD == Scan_Token(&scan_state)) return Make_Word(cp, len);

	return 0;
}


/***********************************************************************
**
*/  REBCNT Scan_Issue(REBYTE *cp, REBCNT len)
/*
**		Scan an issue word, allowing special characters.
**
***********************************************************************/
{
	REBYTE *bp;
	REBCNT l = len;
	REBCNT c;

	if (len == 0) return 0;
    while (IS_LEX_SPACE(*cp)) cp++; /* skip white space */

	bp = cp;

    while (l > 0) {
        switch (GET_LEX_CLASS(*bp)) {

        case LEX_CLASS_DELIMIT:
            return 0;

        case LEX_CLASS_SPECIAL:     /* Flag all but first special char: */
			c = GET_LEX_VALUE(*bp);
			if (!(LEX_SPECIAL_TICK    == c
				|| LEX_SPECIAL_COMMA  == c
				|| LEX_SPECIAL_PERIOD == c
				|| LEX_SPECIAL_PLUS   == c
				|| LEX_SPECIAL_MINUS  == c
				|| LEX_SPECIAL_TILDE  == c
			))
			return 0;

        case LEX_CLASS_WORD:
        case LEX_CLASS_NUMBER:
            bp++;
			l--;
            break;
        }
    }

	return Make_Word(cp, len);
}
