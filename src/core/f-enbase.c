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
**  Module:  f-enbase.c
**  Summary: base representation conversions
**  Section: functional
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "sys-scan.h"


/***********************************************************************
**
*/	static const REBYTE Debase64[128] =
/*
**		Base-64 binary decoder table.
**
***********************************************************************/
{
	#define	BIN_ERROR	(REBYTE)0x80
	#define BIN_SPACE	(REBYTE)0x55
	#define BIN_VALUE	(REBYTE)0x3f
	#define	IS_BIN_SPACE(c) (Debase64[c] & BIN_SPACE)

	/* Control Chars */
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,    /* 80 */
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,
	BIN_SPACE,BIN_SPACE,BIN_SPACE,BIN_ERROR,
	BIN_SPACE,BIN_SPACE,BIN_ERROR,BIN_ERROR,
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,

	/* 20     */    BIN_SPACE,
	/* 21 !   */    BIN_ERROR,
	/* 22 "   */    BIN_ERROR,
	/* 23 #   */    BIN_ERROR,
	/* 24 $   */    BIN_ERROR,
	/* 25 %   */    BIN_ERROR,
	/* 26 &   */    BIN_ERROR,
	/* 27 '   */    BIN_SPACE,
	/* 28 (   */    BIN_ERROR,
	/* 29 )   */    BIN_ERROR,
	/* 2A *   */    BIN_ERROR,
	/* 2B +   */    62,
	/* 2C ,   */    BIN_ERROR,
	/* 2D -   */    BIN_ERROR,
	/* 2E .   */    BIN_ERROR,
	/* 2F /   */    63,

	/* 30 0   */    52,
	/* 31 1   */    53,
	/* 32 2   */    54,
	/* 33 3   */    55,
	/* 34 4   */    56,
	/* 35 5   */    57,
	/* 36 6   */    58,
	/* 37 7   */    59,
	/* 38 8   */    60,
	/* 39 9   */    61,
	/* 3A :   */    BIN_ERROR,
	/* 3B ;   */    BIN_ERROR,
	/* 3C <   */    BIN_ERROR,
	/* 3D =   */    0,		// pad char
	/* 3E >   */    BIN_ERROR,
	/* 3F ?   */    BIN_ERROR,

	/* 40 @   */    BIN_ERROR,
	/* 41 A   */    0,
	/* 42 B   */    1,
	/* 43 C   */    2,
	/* 44 D   */    3,
	/* 45 E   */    4,
	/* 46 F   */    5,
	/* 47 G   */    6,
	/* 48 H   */    7,
	/* 49 I   */    8,
	/* 4A J   */    9,
	/* 4B K   */    10,
	/* 4C L   */    11,
	/* 4D M   */    12,
	/* 4E N   */    13,
	/* 4F O   */    14,

	/* 50 P   */    15,
	/* 51 Q   */    16,
	/* 52 R   */    17,
	/* 53 S   */    18,
	/* 54 T   */    19,
	/* 55 U   */    20,
	/* 56 V   */    21,
	/* 57 W   */    22,
	/* 58 X   */    23,
	/* 59 Y   */    24,
	/* 5A Z   */    25,
	/* 5B [   */    BIN_ERROR,
	/* 5C \   */    BIN_ERROR,
	/* 5D ]   */    BIN_ERROR,
	/* 5E ^   */    BIN_ERROR,
	/* 5F _   */    BIN_ERROR,

	/* 60 `   */    BIN_ERROR,
	/* 61 a   */    26,
	/* 62 b   */    27,
	/* 63 c   */    28,
	/* 64 d   */    29,
	/* 65 e   */    30,
	/* 66 f   */    31,
	/* 67 g   */    32,
	/* 68 h   */    33,
	/* 69 i   */    34,
	/* 6A j   */    35,
	/* 6B k   */    36,
	/* 6C l   */    37,
	/* 6D m   */    38,
	/* 6E n   */    39,
	/* 6F o   */    40,

	/* 70 p   */    41,
	/* 71 q   */    42,
	/* 72 r   */    43,
	/* 73 s   */    44,
	/* 74 t   */    45,
	/* 75 u   */    46,
	/* 76 v   */    47,
	/* 77 w   */    48,
	/* 78 x   */    49,
	/* 79 y   */    50,
	/* 7A z   */    51,
	/* 7B {   */    BIN_ERROR,
	/* 7C |   */    BIN_ERROR,
	/* 7D }   */    BIN_ERROR,
	/* 7E ~   */    BIN_ERROR,
	/* 7F DEL */    BIN_ERROR,
};

/***********************************************************************
**
*/	static const REBYTE Debase64URL[128] =
/*
**		Base-64-URL binary decoder table.
**      See: https://tools.ietf.org/html/rfc4648#section-5
**
***********************************************************************/
{
	/* Control Chars */
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,    /* 80 */
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,
	BIN_SPACE,BIN_SPACE,BIN_SPACE,BIN_ERROR,
	BIN_SPACE,BIN_SPACE,BIN_ERROR,BIN_ERROR,
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,

	/* 20     */    BIN_SPACE,
	/* 21 !   */    BIN_ERROR,
	/* 22 "   */    BIN_ERROR,
	/* 23 #   */    BIN_ERROR,
	/* 24 $   */    BIN_ERROR,
	/* 25 %   */    BIN_ERROR,
	/* 26 &   */    BIN_ERROR,
	/* 27 '   */    BIN_SPACE,
	/* 28 (   */    BIN_ERROR,
	/* 29 )   */    BIN_ERROR,
	/* 2A *   */    BIN_ERROR,
	/* 2B +   */    BIN_ERROR,
	/* 2C ,   */    BIN_ERROR,
	/* 2D -   */    62,
	/* 2E .   */    BIN_ERROR,
	/* 2F /   */    BIN_ERROR,

	/* 30 0   */    52,
	/* 31 1   */    53,
	/* 32 2   */    54,
	/* 33 3   */    55,
	/* 34 4   */    56,
	/* 35 5   */    57,
	/* 36 6   */    58,
	/* 37 7   */    59,
	/* 38 8   */    60,
	/* 39 9   */    61,
	/* 3A :   */    BIN_ERROR,
	/* 3B ;   */    BIN_ERROR,
	/* 3C <   */    BIN_ERROR,
	/* 3D =   */    0,		// pad char
	/* 3E >   */    BIN_ERROR,
	/* 3F ?   */    BIN_ERROR,

	/* 40 @   */    BIN_ERROR,
	/* 41 A   */    0,
	/* 42 B   */    1,
	/* 43 C   */    2,
	/* 44 D   */    3,
	/* 45 E   */    4,
	/* 46 F   */    5,
	/* 47 G   */    6,
	/* 48 H   */    7,
	/* 49 I   */    8,
	/* 4A J   */    9,
	/* 4B K   */    10,
	/* 4C L   */    11,
	/* 4D M   */    12,
	/* 4E N   */    13,
	/* 4F O   */    14,

	/* 50 P   */    15,
	/* 51 Q   */    16,
	/* 52 R   */    17,
	/* 53 S   */    18,
	/* 54 T   */    19,
	/* 55 U   */    20,
	/* 56 V   */    21,
	/* 57 W   */    22,
	/* 58 X   */    23,
	/* 59 Y   */    24,
	/* 5A Z   */    25,
	/* 5B [   */    BIN_ERROR,
	/* 5C \   */    BIN_ERROR,
	/* 5D ]   */    BIN_ERROR,
	/* 5E ^   */    BIN_ERROR,
	/* 5F _   */    63,

	/* 60 `   */    BIN_ERROR,
	/* 61 a   */    26,
	/* 62 b   */    27,
	/* 63 c   */    28,
	/* 64 d   */    29,
	/* 65 e   */    30,
	/* 66 f   */    31,
	/* 67 g   */    32,
	/* 68 h   */    33,
	/* 69 i   */    34,
	/* 6A j   */    35,
	/* 6B k   */    36,
	/* 6C l   */    37,
	/* 6D m   */    38,
	/* 6E n   */    39,
	/* 6F o   */    40,

	/* 70 p   */    41,
	/* 71 q   */    42,
	/* 72 r   */    43,
	/* 73 s   */    44,
	/* 74 t   */    45,
	/* 75 u   */    46,
	/* 76 v   */    47,
	/* 77 w   */    48,
	/* 78 x   */    49,
	/* 79 y   */    50,
	/* 7A z   */    51,
	/* 7B {   */    BIN_ERROR,
	/* 7C |   */    BIN_ERROR,
	/* 7D }   */    BIN_ERROR,
	/* 7E ~   */    BIN_ERROR,
	/* 7F DEL */    BIN_ERROR,
};

#ifdef INCLUDE_BASE85
#define BASE85_DIGITS	5	 /* log85 (2^32) is 4.9926740807112 */
/***********************************************************************
**
*/	static const REBYTE Debase85[128] =
/*
**		Base-85 (ASCII85) binary decoder table.
**
***********************************************************************/
{
	/* Control Chars */
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,    /* 80 */
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,
	BIN_SPACE,BIN_SPACE,BIN_SPACE,BIN_ERROR,
	BIN_SPACE,BIN_SPACE,BIN_ERROR,BIN_ERROR,
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,
	BIN_ERROR,BIN_ERROR,BIN_ERROR,BIN_ERROR,

        /* 20     */    BIN_SPACE,
        /* 21 !   */    0,
        /* 22 "   */    1,
        /* 23 #   */    2,
        /* 24 $   */    3,
        /* 25 %   */    4,
        /* 26 &   */    5,
        /* 27 '   */    6,
        /* 28 (   */    7,
        /* 29 )   */    8,
        /* 2A *   */    9,
        /* 2B +   */    10,
        /* 2C ,   */    11,
        /* 2D -   */    12,
        /* 2E .   */    13,
        /* 2F /   */    14,
        /* 30 0   */    15,
        /* 31 1   */    16,
        /* 32 2   */    17,
        /* 33 3   */    18,
        /* 34 4   */    19,
        /* 35 5   */    20,
        /* 36 6   */    21,
        /* 37 7   */    22,
        /* 38 8   */    23,
        /* 39 9   */    24,
        /* 3A :   */    25,
        /* 3B ;   */    26,
        /* 3C <   */    27,
        /* 3D =   */    28,
        /* 3E >   */    29,
        /* 3F ?   */    30,
        /* 40 @   */    31,
        /* 41 A   */    32,
        /* 42 B   */    33,
        /* 43 C   */    34,
        /* 44 D   */    35,
        /* 45 E   */    36,
        /* 46 F   */    37,
        /* 47 G   */    38,
        /* 48 H   */    39,
        /* 49 I   */    40,
        /* 4A J   */    41,
        /* 4B K   */    42,
        /* 4C L   */    43,
        /* 4D M   */    44,
        /* 4E N   */    45,
        /* 4F O   */    46,
        /* 50 P   */    47,
        /* 51 Q   */    48,
        /* 52 R   */    49,
        /* 53 S   */    50,
        /* 54 T   */    51,
        /* 55 U   */    52,
        /* 56 V   */    53,
        /* 57 W   */    54,
        /* 58 X   */    55,
        /* 59 Y   */    56,
        /* 5A Z   */    57,
        /* 5B [   */    58,
        /* 5C \   */    59,
        /* 5D ]   */    60,
        /* 5E ^   */    61,
        /* 5F _   */    62,
        /* 60 `   */    63,
        /* 61 a   */    64,
        /* 62 b   */    65,
        /* 63 c   */    66,
        /* 64 d   */    67,
        /* 65 e   */    68,
        /* 66 f   */    69,
        /* 67 g   */    70,
        /* 68 h   */    71,
        /* 69 i   */    72,
        /* 6A j   */    73,
        /* 6B k   */    74,
        /* 6C l   */    75,
        /* 6D m   */    76,
        /* 6E n   */    77,
        /* 6F o   */    78,
        /* 70 p   */    79,
        /* 71 q   */    80,
        /* 72 r   */    81,
        /* 73 s   */    82,
        /* 74 t   */    83,
        /* 75 u   */    84,
        /* 76 v   */    BIN_ERROR,
        /* 77 w   */    BIN_ERROR,
        /* 78 x   */    BIN_ERROR,
        /* 79 y   */    BIN_ERROR,
        /* 7A z   */    BIN_ERROR,
        /* 7B {   */    BIN_ERROR,
        /* 7C |   */    BIN_ERROR,
        /* 7D }   */    BIN_ERROR,
        /* 7E ~   */    BIN_ERROR,
        /* 7F DEL */    BIN_ERROR
};
#endif

/***********************************************************************
**
*/	static const REBYTE Enbase64[64] =
/*
**		Base-64 binary encoder table.
**
***********************************************************************/
{
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/"
};

/***********************************************************************
**
*/	static const REBYTE Enbase64URL[64] =
/*
**		Base-64-URL binary encoder table.
**
***********************************************************************/
{
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789-_"
};


#ifdef INCLUDE_BASE85
/***********************************************************************
**
*/	static const REBYTE Enbase85[85] =
/*
**		Base-85 binary encoder table.
**
***********************************************************************/
{
	"!\"#$%&'()*+,-./0123456789:;<=>?@"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
	"abcdefghijklmnopqrstu"
};
#endif


#ifdef INCLUDE_BASE36
#define BASE36_LENGTH 13
/***********************************************************************
**
*/	static const REBYTE Enbase36[36] =
/*
**		Base-36 binary encoder table.
**
***********************************************************************/
{
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
};

static REBU64 base36_powers[BASE36_LENGTH] = {
	1ULL,
	36ULL,
	1296ULL,
	46656ULL,
	1679616ULL,
	60466176ULL,
	2176782336ULL,
	78364164096ULL,
	2821109907456ULL,
	101559956668416ULL,
	3656158440062976ULL,
	131621703842267136ULL,
	4738381338321616896ULL
};
#endif


/***********************************************************************
**
*/	static REBSER *Decode_Base2(const REBYTE **src, REBCNT len, REBYTE delim)
/*
***********************************************************************/
{
	REBYTE *bp;
	const REBYTE *cp;
	REBCNT count = 0;
	REBINT accum = 0;
	REBYTE lex;
	REBSER *ser;

	ser = Make_Binary(len >> 3);
	bp = BIN_HEAD(ser);
	cp = *src;

	for (; len > 0; cp++, len--) {

		if (delim && *cp == delim) break;

		lex = Lex_Map[*cp];

		if (lex >= LEX_NUMBER) {

			if (*cp == '0') accum *= 2;
			else if (*cp == '1') accum = (accum * 2) + 1;
			else goto err;

			if (count++ >= 7) {
				*bp++ = (REBYTE)accum;
				count = 0;
				accum = 0;
			}
		}
		else if (!*cp || lex > LEX_DELIMIT_RETURN) goto err;
	}
	if (count) goto err; // improper modulus

	*bp = 0;
	ser->tail = bp - STR_HEAD(ser);
	return ser;

err:
	Free_Series(ser);
	*src = cp;
	return 0;
}


/***********************************************************************
**
*/	static REBSER *Decode_Base16(const REBYTE **src, REBCNT len, REBYTE delim)
/*
***********************************************************************/
{
	REBYTE *bp;
	const REBYTE *cp;
	REBCNT count = 0;
	REBINT accum = 0;
	REBYTE lex;
	REBINT val;
	REBSER *ser;

	ser = Make_Binary(len / 2);
	bp = STR_HEAD(ser);
	cp = *src;

	for (; len > 0; cp++, len--) {

		if (delim && *cp == delim) break;

		lex = Lex_Map[*cp];

		if (lex > LEX_WORD) {
			val = lex & LEX_VALUE; // char num encoded into lex
			if (!val && lex < LEX_NUMBER) goto err;  // invalid char (word but no val)
			accum = (accum << 4) + val;
			if (count++ & 1) *bp++ = (REBYTE)accum;
		}
		else if (!*cp || lex > LEX_DELIMIT_RETURN) goto err;
	}
	if (count & 1) goto err; // improper modulus

	*bp = 0;
	ser->tail = bp - STR_HEAD(ser);
	return ser;

err:
	Free_Series(ser);
	*src = cp;
	return 0;
}

								
/***********************************************************************
**
*/	static REBSER *Decode_Base64(const REBYTE **src, REBCNT len, REBYTE delim, REBOOL urlSafe)
/*
***********************************************************************/
{
	REBYTE *bp;
	const REBYTE *cp;
	REBCNT flip, pos;
	REBINT accum;
	REBYTE lex;
	REBSER *ser;

	// Allocate buffer large enough to hold result:
	// Accounts for e bytes decoding into 3 bytes.
	ser = Make_Binary(((len + 3) * 3) / 4);

start:
	bp = STR_HEAD(ser);
	cp = *src;
	accum = 0;
	flip = 0;

	const REBYTE *table;
	if (urlSafe) {
		table = Debase64URL;
	} else {
		table = Debase64;
	}

	for (pos = len; pos > 0; cp++, pos--) {

		// Check for terminating delimiter (optional):
		if (delim && *cp == delim) break;

		// Check for char out of range:
		if (*cp > 127) {
			if (*cp == 0xA0) continue;  // hard space
			goto err;
		}

		lex = table[*cp];

		if (lex < BIN_SPACE) {

			if (*cp != '=')	{
				accum = (accum << 6) + lex;
				if (flip++ == 3) {
					*bp++ = (REBYTE)(accum >> 16);
					*bp++ = (REBYTE)(accum >> 8);
					*bp++ = (REBYTE)(accum);
					accum = 0;
					flip = 0;
				}
			} else {
				// Special padding: "="
				cp++;
				pos--;
				if (flip == 3) {
					*bp++ = (REBYTE)(accum >> 10);
					*bp++ = (REBYTE)(accum >> 2);
					flip = 0;
				}
				else if (flip == 2) {
					if (!Skip_To_Char(cp, cp + pos, '=')) goto err;
					cp++;
					*bp++ = (REBYTE)(accum >> 4);
					flip = 0;
				}
				else goto err;
				break;
			}
		}
		else if (lex == BIN_ERROR) {
			if(!urlSafe && (*cp == '-' || *cp == '_')) {
				// there was found char, which is used in URL safe encoding, but we are in simple Base64 mode,
				// so try to decode it using safe table, instead of throwing an error immediately.
				urlSafe = TRUE;
				goto start;
			}
			goto err;
		}
	}

	if (flip) {
		if(urlSafe) {
			// in url safe mode may be ommited the final inut padding
			if (flip == 3) {
				*bp++ = (REBYTE)(accum >> 10);
				*bp++ = (REBYTE)(accum >> 2);
			}
			else if (flip == 2) {
				*bp++ = (REBYTE)(accum >> 4);
			}
			else goto err;
		} else goto err;
	}

	*bp = 0;
	ser->tail = bp - STR_HEAD(ser);
	return ser;

err:
	Free_Series(ser);
	*src = cp;
	return 0;
}


#ifdef INCLUDE_BASE85
/***********************************************************************
**
*/	static REBSER *Decode_Base85(const REBYTE **src, REBCNT len, REBYTE delim)
/*
***********************************************************************/
{
	REBYTE *bp;
	const REBYTE *cp;
	REBSER *ser;
	REBCNT ser_size;
	REBCNT chunk;
	REBCNT pos, c;
	REBINT pad=0;

	// Allocate buffer large enough to hold result:
	ser = Make_Binary(((len + 4) / 5) * 4);
	ser_size = SERIES_AVAIL(ser);

	bp = STR_HEAD(ser);
	cp = *src;
	
	pos = 0;
	while(len > 0) {
		if(pos >= ser_size) {
			// in extreme cases (a lot of 'z' chars) initialy computed size may not be enough
			SERIES_TAIL(ser) = ser_size;     // sets current series' tail (used by expand function bellow)
			Expand_Series(ser, ser_size, 8); // may expand more than 8 bytes
			bp = STR_HEAD(ser);              // because above could reallocate
			ser_size = SERIES_AVAIL(ser);
		}
		/* 'z' is a special way to encode 4 bytes of 0s */
		if(*cp=='z') {
			cp++;
			len--;
			bp[pos++] = 0u;
			bp[pos++] = 0u;
			bp[pos++] = 0u;
			bp[pos++] = 0u;
			continue;
		}
		chunk = 0;
		for(c=0; c<BASE85_DIGITS; c++) {
			REBYTE d = 0;
			if(len > 0) {
				len--;
				d=Debase85[(REBYTE)*cp++];
				if (d == BIN_SPACE) {
					// ignore spaces
					c--; continue;
				}
				if (d > 127) goto err; /* failure - invalid character */
			}
			else {
				if(!pad) chunk++;
				pad++;
			}
			if(c == 4) {
				// math overflow checking.. for example input: {s8W-"}
				if (chunk > (MAX_U32 / 85u)) goto err;
				chunk *= 85;
				if (chunk > (MAX_U32 - d)) goto err;
				chunk += d;
			} else chunk = chunk * 85 + d;
		}
		bp[pos  ] = (REBYTE)(chunk >> 24);
		bp[pos+1] = (REBYTE)(chunk >> 16);
		bp[pos+2] = (REBYTE)(chunk >>  8);
		bp[pos+3] = (REBYTE)(chunk      );
		pos += 4;
	}
	bp[pos] = 0;
	ser->tail = pos - pad;
	return ser;

err:
	Free_Series(ser);
	*src = cp;
	return 0;
}
#endif

#ifdef INCLUDE_BASE36
/***********************************************************************
**
*/	static REBSER* Decode_Base36(const REBYTE** src, REBCNT len, REBYTE delim)
/*
***********************************************************************/
{
	REBYTE* bp;
	const REBYTE* cp;
	REBSER* ser;
	REBCNT ser_size;
	REBINT pad = 0;
	REBU64 c = 0;
	REBINT d = 0;
	REBCNT i;

	cp = *src;

	if (len > BASE36_LENGTH) goto err;
	else if (len == 0) {
		ser = Make_Binary(1);
		ser->tail = 0;
		return ser;
	}

	for (i = 0; i < len; i++) {
		if (cp[i] >= '0' && cp[i] <= '9')
			d = cp[i] - '0';
		else if (cp[i] >= 'A' && cp[i] <= 'Z')
			d = 10 + cp[i] - 'A';
		else if (cp[i] >= 'a' && cp[i] <= 'z')
			d = 10 + cp[i] - 'a';
		else goto err;

		c += d * base36_powers[len - i - 1];
		if (c < 0) goto err;
	}
	// Allocate buffer large enough to hold result:
	ser = Make_Binary(8);
	ser_size = SERIES_AVAIL(ser);
	bp = STR_HEAD(ser);

	i = 7;
	do {
		bp[i] = (REBYTE)(c & 0xFF);
		c >>= 8;
	} while (i-- > 0);

	ser->tail = 8;
	return ser;

err:
	*src = cp;
	return 0;
}
#endif


/***********************************************************************
**
*/	const REBYTE *Decode_Binary(REBVAL *value, const REBYTE *src, REBCNT len, REBINT base, REBYTE delim, REBOOL urlSafe)
/*
**		Scan and convert a binary string.
**
***********************************************************************/
{
	REBSER *ser = 0;

	switch (base) {
	case 64:
		ser = Decode_Base64(&src, len, delim, urlSafe);
		break;
	case 16:
		ser = Decode_Base16(&src, len, delim);
		break;
	case 2:
		ser = Decode_Base2 (&src, len, delim);
		break;
	case 85:
#ifdef INCLUDE_BASE85
		ser = Decode_Base85 (&src, len, delim);
#else
		Trap0(RE_FEATURE_NA);
#endif
		break;
	case 36:
#ifdef INCLUDE_BASE36
		ser = Decode_Base36(&src, len, delim);
#else
		Trap0(RE_FEATURE_NA);
#endif
		break;
	}

	if (!ser) return 0;

	Set_Binary(value, ser);

	return src;
}


/***********************************************************************
**
*/  REBSER *Encode_Base2(REBVAL *value, REBSER *series, REBCNT len, REBFLG brk)
/*
**		Base2 encode a given series. Must be BYTES, not UNICODE.
**
***********************************************************************/
{
	REBYTE *p;	// ?? should it be REBYTE? Same with below functions?
	REBYTE *src;
	REBCNT i;
	REBCNT n;
	REBYTE b;

	if(len > VAL_LEN(value)) len = VAL_LEN(value);
	src = VAL_BIN_DATA(value);

	// Add slop-factor
	series = Prep_String (series, &p, 8 * len + 2 * (len / 8) + 4);
	if (len > 8 && brk) *p++ = LF;

	for (i = 0; i < len; i++) {

		b = src[i];

		for (n = 0x80; n > 0; n = n>>1) {
			*p++ = (b & n) ? '1' : '0';
		}
	
		if ((i+1) % 8 == 0 && brk)
			*p++ = LF;
	}
	*p = 0;

	//if (*(p-1) != LF && len > 9 && brk) *p++ = LF; // adds LF before closing bracket

	SERIES_TAIL(series) = DIFF_PTRS(p, series->data);
	return series;
}


/***********************************************************************
**
*/  REBSER *Encode_Base16(REBVAL *value, REBSER *series, REBCNT len, REBFLG brk)
/*
**		Base16 encode a given series. Must be BYTES, not UNICODE.
**
***********************************************************************/
{
	REBCNT count;
	REBYTE *bp;
	REBYTE *src;

	if(len > VAL_LEN(value)) len = VAL_LEN(value);
	src = VAL_BIN_DATA(value);

	// Account for hex, lines, and extra syntax:
	series = Prep_String(series, &bp, len*2 + len/32 + 32);
	// (Note: tail not properly set yet)

	if (len >= 32 && brk) *bp++ = LF;
	for (count = 1; count <= len; count++) {
		bp = Form_Hex2(bp, *src++);
		if (brk && ((count % 32) == 0)) *bp++ = LF;
	}

	//if ((len >= 32) && brk && *(bp-1) != LF) *bp++ = LF; // adds LF before closing bracket
	*bp = 0;
	
	SERIES_TAIL(series) = DIFF_PTRS(bp, series->data);

	return series;
}


/***********************************************************************
**
*/  REBSER *Encode_Base64(REBVAL *value, REBSER *series, REBCNT len, REBFLG brk, REBOOL urlSafe)
/*
**		Base64 encode a given series. Must be BYTES, not UNICODE.
**
***********************************************************************/
{
	REBYTE *p;
	REBYTE *src;
	REBINT x, loop, pad;

	if(len > VAL_LEN(value)) len = VAL_LEN(value);
	src = VAL_BIN_DATA(value);

	const REBYTE *table;
	if(urlSafe) {
		table = Enbase64URL;
	} else {
		table = Enbase64;
	}

	// slop-factor
  	series = Prep_String (series, &p, 4 * len / 3 + 2 * (len / 32) + 5);
	loop = (int) (len / 3) - 1;
	if (4 * loop > 64 && brk) *p++ = LF;

 	for (x = 0; x <= 3 * loop; x += 3) {
		*p++ = table[src[x] >> 2];
		*p++ = table[((src[x] & 0x3) << 4) + (src[x + 1] >> 4)];
		*p++ = table[((src[x + 1] & 0xF) << 2) + (src[x + 2] >> 6)];
		*p++ = table[(src[x + 2] % 0x40)];
		if ((x+3) % 48 == 0 && brk)
			*p++ = LF;
	}
	pad = len % 3;
	if (pad != 0) {
		*p++ = table[src[x] >> 2];
		if (pad >= 1)
			*p++ = table[((src[x] & 0x3) << 4) + (pad == 1 ? 0 : src[x + 1] >> 4)];
		if (pad == 2)
			*p++ = table[(src[x + 1] & 0xF) << 2];
		if (!urlSafe) {
			// add padding
			while (pad++ < 3) { *p = '='; p++; }
		}
	}

	//if (*(p-1) != LF && x > 49 && brk) *p++ = LF; // adds LF before closing bracket
	*p = 0;

	SERIES_TAIL(series) = DIFF_PTRS(p, series->data); /* 4 * (int) (len % 3 ? (len / 3) + 1 : len / 3); */

	return series;
}


#ifdef INCLUDE_BASE85
/***********************************************************************
**
*/  REBSER *Encode_Base85(REBVAL *value, REBSER *series, REBCNT len, REBFLG brk)
/*
**		Base85 encode a given series. Must be BYTES, not UNICODE.
**
***********************************************************************/
{
	REBYTE *bp;
	REBYTE *src;
	REBCNT x=0;
	REBCNT loop;
	REBCNT i, chunk;

	if(len > VAL_LEN(value)) len = VAL_LEN(value);
	src = VAL_BIN_DATA(value);

	// Account for hex, lines, and extra syntax:
	series = Prep_String(series, &bp, ((len + 3) / 4) * 5);
	// (Note: tail not properly set yet)

	//if (len >= 32 && brk) *bp++ = LF;
	if(len >= 4) {
		loop = (len / 4) - 1;
		for (x = 0; x <= 4 * loop;) {
			chunk  = ((REBCNT)src[x++]) << 24u;
			chunk |= ((REBCNT)src[x++]) << 16u;
			chunk |= ((REBCNT)src[x++]) <<  8u;
			chunk |= ((REBCNT)src[x++])       ;
			if(chunk==0) {
				*bp++='z'; /* this is a special zero character */
			} else {
				for(i = BASE85_DIGITS;i--;) {
					bp[i] = Enbase85[chunk%85];
					chunk /= 85;
				}
				bp += 5;
			}
		}
	}
	if ((len % 4) != 0) {
        chunk  =                      (((REBCNT)src[x++]) << 24u);
        chunk |= ((x < (REBCNT)len) ? (((REBCNT)src[x++]) << 16u): 0u);
        chunk |= ((x < (REBCNT)len) ? (((REBCNT)src[x++]) <<  8u): 0u);
        chunk |= ((x < (REBCNT)len) ? (((REBCNT)src[x++])       ): 0u);
		for(i = BASE85_DIGITS;i--;) {
			bp[i] = Enbase85[chunk%85];
			chunk /= 85;
		}
		bp += (len % 4) + 1;
	}
	*bp = 0;
	SERIES_TAIL(series) = DIFF_PTRS(bp, series->data);

	return series;
}
#endif


#ifdef INCLUDE_BASE36
/***********************************************************************
**
*/  REBSER* Encode_Base36(REBVAL* value, REBSER* series, REBCNT len, REBFLG brk)
/*
**		Base36 encode a given series. Must be BYTES, not UNICODE.
**
***********************************************************************/
{
	REBYTE* bp;
	REBYTE* src;
	REBOOL discard = TRUE;
	REBU64 d, m = 0;
	REBCNT n, p = 0;
	REBINT i;

	if (len > VAL_LEN(value)) len = VAL_LEN(value);
	if (len > sizeof(REBI64)) {
		Trap1(RE_OUT_OF_RANGE, value);
	}
	else if (len == 0) {
		series = Prep_String(series, &bp, 1);
		SERIES_TAIL(series) = 0;
		return series;
	}
	src = VAL_BIN_DATA(value);

	for (bp = src, n = len; n; n--, bp++)
		m = (m << 8) | *bp;

	series = Prep_String(series, &bp, BASE36_LENGTH + 1);

	for (i = BASE36_LENGTH - 1; i >= 0; i--) {
		d = m / base36_powers[i];
		m = m - base36_powers[i] * d;
		discard = discard && (d == 0 && i > 0);
		if (!discard)
			bp[p++] = Enbase36[d];
	}
	SERIES_TAIL(series) = p;
	return series;
}
#endif