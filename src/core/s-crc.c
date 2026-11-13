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
**  Module:  s-crc.c
**  Summary: CRC computation
**  Section: strings
**  Author:  Carl Sassenrath (REBOL interface sections)
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

#define CRC_DEFINED

#define CRCBITS 24			/* may be 16, 24, or 32 */
#define MASK_CRC(crc) ((crc) & I32_C(0x00ffffff))	  /* if CRCBITS is 24 */
#define CRCHIBIT ((REBCNT) (I32_C(1)<<(CRCBITS-1))) /* 0x8000 if CRCBITS is 16 */
#define CRCSHIFTS (CRCBITS-8)
#define CCITTCRC 0x1021 	/* CCITT's 16-bit CRC generator polynomial */
#define PRZCRC   0x864cfb	/* PRZ's 24-bit CRC generator polynomial */
#define CRCINIT  0xB704CE	/* Init value for CRC accumulator */

static REBCNT *CRC24_Table;
static REBCNT *CRC32_Table = 0;

/***********************************************************************
**
*/	static REBCNT Generate_CRC24(REBYTE ch, REBCNT poly, REBCNT accum)
/*
**		Simulates CRC hardware circuit.  Generates true CRC
**		directly, without requiring extra NULL bytes to be appended
**		to the message. Returns new updated CRC accumulator.
**
**		These CRC functions are derived from code in chapter 19 of the book
**		"C Programmer's Guide to Serial Communications", by Joe Campbell.
**		Generalized to any CRC width by Philip Zimmermann.
**
**			CRC-16		X^16 + X^15 + X^2 + 1
**			CRC-CCITT	X^16 + X^12 + X^2 + 1
**
**		Notes on making a good 24-bit CRC:
**		The primitive irreducible polynomial of degree 23 over GF(2),
**		040435651 (octal), comes from Appendix C of "Error Correcting Codes,
**		2nd edition" by Peterson and Weldon, page 490.  This polynomial was
**		chosen for its uniform density of ones and zeros, which has better
**		error detection properties than polynomials with a minimal number of
**		nonzero terms.	Multiplying this primitive degree-23 polynomial by
**		the polynomial x+1 yields the additional property of detecting any
**		odd number of bits in error, which means it adds parity.  This
**		approach was recommended by Neal Glover.
**
**		To multiply the polynomial 040435651 by x+1, shift it left 1 bit and
**		bitwise add (xor) the unshifted version back in.  Dropping the unused
**		upper bit (bit 24) produces a CRC-24 generator bitmask of 041446373
**		octal, or 0x864cfb hex.
**
**		You can detect spurious leading zeros or framing errors in the
**		message by initializing the CRC accumulator to some agreed-upon
**		nonzero "random-like" value, but this is a bit nonstandard.
**
***********************************************************************/
{
	REBINT i;
	REBCNT data;

	data = ch;
	data <<= CRCSHIFTS; 	/* shift data to line up with MSB of accum */
	i = 8;					/* counts 8 bits of data */
	do {	/* if MSB of (data XOR accum) is TRUE, shift and subtract poly */
		if ((data ^ accum) & CRCHIBIT) accum = (accum<<1) ^ poly;
		else accum <<= 1;
		data <<= 1;
	} while (--i);	/* counts 8 bits of data */
	return (MASK_CRC(accum));
}


/***********************************************************************
**
*/	static void Make_CRC24_Table(REBCNT poly)
/*
**		Derives a CRC lookup table from the CRC polynomial.
**		The table is used later by crcupdate function given below.
**		Only needs to be called once at the dawn of time.
**
***********************************************************************/
{
	REBINT i;

	FOREACH (i, 256) CRC24_Table[i] = Generate_CRC24((REBYTE) i, poly, 0);
}


/***********************************************************************
**
*/	REBCNT Compute_CRC24(const REBYTE *str, REBCNT len)
/*
***********************************************************************/
{
	REBCNT crc = CRCINIT; // originally there was not standard seed: len + *str

	while (len--) {
		REBYTE n = (REBYTE)(crc >> CRCSHIFTS) ^ *str++;
		crc = MASK_CRC(crc << 8) ^ CRC24_Table[n];
	}

	return crc;
}


/***********************************************************************
**
*/	void Init_CRC(void)
/*
***********************************************************************/
{
	CRC24_Table = Make_Mem(sizeof(REBCNT) * 256);
	Make_CRC24_Table(PRZCRC);
}


/***********************************************************************
**
*/	void Dispose_CRC(void)
/*
***********************************************************************/
{
	if (CRC24_Table) Free_Mem(CRC24_Table, sizeof(REBCNT) * 256);
	if (CRC32_Table) Free_Mem(CRC32_Table, sizeof(REBCNT) * 256);
}

#ifdef unused
/***********************************************************************
**
X*/	REBINT CRC_String(REBVAL *val)
/*
**		Return a case insensitive hash value for the string.  The
**		string does not have to be zero terminated and UTF8 is ok.
**
***********************************************************************/
{
	REBYTE	n;
	REBINT hash;
	REBYTE* bin;
	REBUNI* uni;
	REBLEN  len;

	if (BYTE_SIZE(VAL_SERIES(val))) {
		bin = VAL_BIN_DATA(val);
		len = Val_Byte_Len(val);
		hash = (REBINT)len + (REBINT)((REBYTE)LO_CASE(*bin));
		for (; len > 0; len--) {
			n = (REBYTE)((hash >> CRCSHIFTS) ^ (REBYTE)LO_CASE(*bin++));
			hash = MASK_CRC(hash << 8) ^ (REBINT)CRC24_Table[n];
		}
	}
	else {
		uni = VAL_UNI_DATA(val);
		len = Val_Series_Len(val);
		hash = (REBINT)len + (REBINT)((REBYTE)LO_CASE(*uni));
		for (; len > 0; len--) {
			n = (REBYTE)((hash >> CRCSHIFTS) ^ (REBYTE)LO_CASE(*uni++));
			hash = MASK_CRC(hash << 8) ^ (REBINT)CRC24_Table[n];
		}
	}

	return hash;
}
#endif

/***********************************************************************
**
*/	REBCNT CRC_Word(const REBYTE* str, REBCNT len)
/*
**		Return a case insensitive hash value for the string.
**
***********************************************************************/
{
	REBCNT m, n;
	REBCNT hash;

	if (len == UNKNOWN) len = LEN_BYTES(str);

	hash = len + (REBYTE)LO_CASE(*str);

	for (; len > 0;) {
		n = *str;
		if (n > 127) {
			m = UTF8_Decode_Codepoint(&str, &len); // mods str, ulen
			if (m == UNI_ERROR)
				Trap0(RE_INVALID_CHARS);
			n = m;
		}
		else len--, str++;
		if (n < UNICODE_CASES) n = LO_CASE(n);
		n = (REBYTE)((hash >> CRCSHIFTS) ^ (REBYTE)n); // drop upper 8 bits
		hash = MASK_CRC(hash << 8) ^ CRC24_Table[n];
	}

	return hash;
}


/***********************************************************************
**
*/	REBCNT Compute_IPC(const REBYTE *data, REBCNT length)
/*
**		Compute an IP checksum given some data and a length.
**		Used only on BINARY values.
**
***********************************************************************/
{
	REBCNT hash = 0;
	const REBYTE* up = data;

	while (length > 1) {
		hash += ((REBCNT)up[0] << 8) | up[1];
		up += 2;
		length -= 2;
	}

	// Handle the odd byte if necessary
	if (length) hash += *up;

	// Fold 32-bit sum to 16 bits
	hash  = (hash >> 16) + (hash & 0xFFFF);
	hash += (hash >> 16);

	return (~hash) & 0xFFFF;
}


#ifdef INCLUDE_DEFLATE
u32 libdeflate_crc32(u32 crc, const void* p, size_t len);
#else
static void Make_CRC32_Table(void) {
	u32 c;
	int n,k;

	CRC32_Table = Make_Mem(256 * sizeof(u32));
	ASSERT(CRC32_Table != NULL, RP_NO_MEMORY);

	for(n=0;n<256;n++) {
		c=(u32)n;
		for(k=0;k<8;k++) {
			if(c&1)
				c=U32_C(0xedb88320)^(c>>1);
			else
				c=c>>1;
		}
		CRC32_Table[n]=c;
	}
}

REBCNT Update_CRC32(u32 crc, REBYTE *buf, int len) {
	u32 c = ~crc;
	int n;

	if(!CRC32_Table) Make_CRC32_Table();

	for(n = 0; n < len; n++)
		c = CRC32_Table[(c^buf[n])&0xff]^(c>>8);

	return ~c;
}
#endif

/***********************************************************************
**
*/	REBCNT CRC32(REBYTE *buf, REBCNT len)
/*
***********************************************************************/
{
#ifdef INCLUDE_DEFLATE
	return libdeflate_crc32(0, buf, len);
#else
	return Update_CRC32(U32_C(0x00000000), buf, len);
#endif
}
