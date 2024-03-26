/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2024 Rebol Open Source Contributors
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
*/	REBINT Compute_CRC24(REBYTE *str, REBCNT len)
/*
***********************************************************************/
{
	REBYTE	n;
	REBINT crc = (REBINT)len + (REBINT)((REBYTE)(*str));

	for (; len > 0; len--) {
		n = (REBYTE)((crc >> CRCSHIFTS) ^ (REBYTE)(*str++));
		crc = MASK_CRC(crc << 8) ^ (REBINT)CRC24_Table[n];
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
	Free_Mem(CRC24_Table, 0);
	Free_Mem(CRC32_Table, 0);
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
	REBINT m, n;
	REBINT hash;

	if (len == UNKNOWN) len = (REBINT)LEN_BYTES(str);

	hash = len + (REBYTE)LO_CASE(*str);

	for (; len > 0; str++, len--) {
		n = *str;
		if (n > 127) {
			m = Decode_UTF8_Char(&str, &len); // mods str, ulen
			if (!m) Trap0(RE_INVALID_CHARS);
			n = m;
		}
		if (n < UNICODE_CASES) n = LO_CASE(n);
		n = (REBYTE)((hash >> CRCSHIFTS) ^ (REBYTE)n); // drop upper 8 bits
		hash = MASK_CRC(hash << 8) ^ CRC24_Table[n];
	}

	return hash;
}


/***********************************************************************
**
*/	REBINT Compute_IPC(REBYTE *data, REBCNT length)
/*
**		Compute an IP checksum given some data and a length.
**		Used only on BINARY values.
**
***********************************************************************/
{
	REBCNT	lSum = 0;	// stores the summation
	REBYTE	*up = data;

	while (length > 1) {
		lSum += (up[0] << 8) | up[1];
		up += 2;
		length -= 2;
	}

	// Handle the odd byte if necessary
	if (length) lSum += *up;

	// Add back the carry outs from the 16 bits to the low 16 bits
	lSum = (lSum >> 16) + (lSum & 0xffff);	// Add high-16 to low-16
	lSum += (lSum >> 16);					// Add carry
	return (REBINT)( (~lSum) & 0xffff);		// 1's complement, then truncate
}



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

/***********************************************************************
**
*/	REBCNT CRC32(REBYTE *buf, REBCNT len)
/*
***********************************************************************/
{
	return Update_CRC32(U32_C(0x00000000), buf, len);
}



#ifdef ndef
Header File
// CRCdemo.h

protected:
      ULONG crc32_table[256];  // Lookup table array
      void Init_CRC32_Table();  // Builds lookup table array
      ULONG Reflect(ULONG ref, char ch);  // Reflects CRC bits in the lookup table
      int Get_CRC(CString& text);  // Creates a CRC from a text string
 

Source File
// CRCdemo.cpp

void CRCdemo::Init_CRC32_Table()
{// Call this function only once to initialize the CRC table.

      // This is the official polynomial used by CRC-32
      // in PKZip, WinZip and Ethernet.
      ULONG ulPolynomial = 0x04c11db7;

      // 256 values representing ASCII character codes.
      for(int i = 0; i <= 0xFF; i++)
      {
            crc32_table[i]=Reflect(i, 8) << 24;
            for (int j = 0; j < 8; j++)
                  crc32_table[i] = (crc32_table[i] << 1) ^ (crc32_table[i] & (1 << 31) ? ulPolynomial : 0);
            crc32_table[i] = Reflect(crc32_table[i], 32);
      }
}

ULONG CRCdemo::Reflect(ULONG ref, char ch)
{// Used only by Init_CRC32_Table().

      ULONG value(0);

      // Swap bit 0 for bit 7
      // bit 1 for bit 6, etc.
      for(int i = 1; i < (ch + 1); i++)
      {
            if(ref & 1)
                  value |= 1 << (ch - i);
            ref >>= 1;
      }
      return value;
}

int CRCdemo::Get_CRC(CString& text)
{ // Pass a text string to this function and it will return the CRC.

      // Once the lookup table has been filled in by the two functions above,
      // this function creates all CRCs using only the lookup table.
      // Note that CString is an MFC class.
      // If you don't have MFC, use the function below instead.

      // Be sure to use unsigned variables,
      // because negative values introduce high bits
      // where zero bits are required.

      // Start out with all bits set high.
      ULONG  ulCRC(0xffffffff);
      int len;
      unsigned char* buffer;

      // Get the length.
      len = text.GetLength();
      // Save the text in the buffer.
      buffer = (unsigned char*)(LPCTSTR)text;
      // Perform the algorithm on each character
      // in the string, using the lookup table values.
      while(len--)
            ulCRC = (ulCRC >> 8) ^ crc32_table[(ulCRC & 0xFF) ^ *buffer++];
      // Exclusive OR the result with the beginning value.
      return ulCRC ^ 0xffffffff;
}


If you don't have an MFC compiler, you can substitute this function, which doesn't use a CString.  Just change the declaration in the header file to: int Get_CRC(char* text);  // Creates a CRC from a text string

int CRCdemo::Get_CRC(char* text)
{// Pass a text string to this function and it will return the CRC.

      // Once the lookup table has been filled in by the two functions above,
      // this function creates all CRCs using only the lookup table.

      // Be sure to use unsigned variables,
      // because negative values introduce high bits
      // where zero bits are required.

      // Start out with all bits set high.
      ULONG  ulCRC(0xffffffff);
      int len;
      unsigned char* buffer;

      // Get the length.
      len = LEN_BYTES(text);
      // Save the text in the buffer.
      buffer = (unsigned char*)text;
      // Perform the algorithm on each character
      // in the string, using the lookup table values.
      while(len--)
            ulCRC = (ulCRC >> 8) ^ crc32_table[(ulCRC & 0xFF) ^ *buffer++];
      // Exclusive OR the result with the beginning value.
      return ulCRC ^ 0xffffffff;
} 

//----------------

/* 
 * crc32.c
 * This code is in the public domain; copyright abandoned.
 * Liability for non-performance of this code is limited to the amount
 * you paid for it.  Since it is distributed for free, your refund will
 * be very very small.  If it breaks, you get to keep both pieces.
 */

#include "crc32.h"

#if __GNUC__ >= 3	/* 2.x has "attribute", but only 3.0 has "pure */
#define attribute(x) __attribute__(x)
#else
#define attribute(x)
#endif

/*
 * There are multiple 16-bit CRC polynomials in common use, but this is
 * *the* standard CRC-32 polynomial, first popularized by Ethernet.
 * x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+x^0
 */
#define CRCPOLY_LE 0xedb88320
#define CRCPOLY_BE 0x04c11db7

/* How many bits at a time to use.  Requires a table of 4<<CRC_xx_BITS bytes. */
/* For less performance-sensitive, use 4 */
#define CRC_LE_BITS 8
#define CRC_BE_BITS 8

/*
 * Little-endian CRC computation.  Used with serial bit streams sent
 * lsbit-first.  Be sure to use cpu_to_le32() to append the computed CRC.
 */
#if CRC_LE_BITS > 8 || CRC_LE_BITS < 1 || CRC_LE_BITS & CRC_LE_BITS-1
# error CRC_LE_BITS must be a power of 2 between 1 and 8
#endif

#if CRC_LE_BITS == 1
/*
 * In fact, the table-based code will work in this case, but it can be
 * simplified by inlining the table in ?: form.
 */
#define crc32init_le()
#define crc32cleanup_le()
/**
 * crc32_le() - Calculate bitwise little-endian Ethernet AUTODIN II CRC32
 * @crc - seed value for computation.  ~0 for Ethernet, sometimes 0 for
 *        other uses, or the previous crc32 value if computing incrementally.
 * @p   - pointer to buffer over which CRC is run
 * @len - length of buffer @p
 * 
 */
uint32_t attribute((pure)) crc32_le(uint32_t crc, unsigned char const *p, size_t len)
{
	int i;
	while (len--) {
		crc ^= *p++;
		for (i = 0; i < 8; i++)
			crc = (crc >> 1) ^ ((crc & 1) ? CRCPOLY_LE : 0);
	}
	return crc;
}
#else				/* Table-based approach */

static uint32_t *crc32table_le;
/**
 * crc32init_le() - allocate and initialize LE table data
 *
 * crc is the crc of the byte i; other entries are filled in based on the
 * fact that crctable[i^j] = crctable[i] ^ crctable[j].
 *
 */
static int
crc32init_le(void)
{
	unsigned i, j;
	uint32_t crc = 1;

	crc32table_le =
		malloc((1 << CRC_LE_BITS) * sizeof(uint32_t));
	if (!crc32table_le)
		return 1;
	crc32table_le[0] = 0;

	for (i = 1 << (CRC_LE_BITS - 1); i; i >>= 1) {
		crc = (crc >> 1) ^ ((crc & 1) ? CRCPOLY_LE : 0);
		for (j = 0; j < 1 << CRC_LE_BITS; j += 2 * i)
			crc32table_le[i + j] = crc ^ crc32table_le[j];
	}
	return 0;
}

/**
 * crc32cleanup_le(): free LE table data
 */
static void
crc32cleanup_le(void)
{
	if (crc32table_le) free(crc32table_le);
	crc32table_le = NULL;
}

/**
 * crc32_le() - Calculate bitwise little-endian Ethernet AUTODIN II CRC32
 * @crc - seed value for computation.  ~0 for Ethernet, sometimes 0 for
 *        other uses, or the previous crc32 value if computing incrementally.
 * @p   - pointer to buffer over which CRC is run
 * @len - length of buffer @p
 * 
 */
uint32_t attribute((pure)) crc32_le(uint32_t crc, unsigned char const *p, size_t len)
{
	while (len--) {
# if CRC_LE_BITS == 8
		crc = (crc >> 8) ^ crc32table_le[(crc ^ *p++) & 255];
# elif CRC_LE_BITS == 4
		crc ^= *p++;
		crc = (crc >> 4) ^ crc32table_le[crc & 15];
		crc = (crc >> 4) ^ crc32table_le[crc & 15];
# elif CRC_LE_BITS == 2
		crc ^= *p++;
		crc = (crc >> 2) ^ crc32table_le[crc & 3];
		crc = (crc >> 2) ^ crc32table_le[crc & 3];
		crc = (crc >> 2) ^ crc32table_le[crc & 3];
		crc = (crc >> 2) ^ crc32table_le[crc & 3];
# endif
	}
	return crc;
}
#endif

/*
 * Big-endian CRC computation.  Used with serial bit streams sent
 * msbit-first.  Be sure to use cpu_to_be32() to append the computed CRC.
 */
#if CRC_BE_BITS > 8 || CRC_BE_BITS < 1 || CRC_BE_BITS & CRC_BE_BITS-1
# error CRC_BE_BITS must be a power of 2 between 1 and 8
#endif

#if CRC_BE_BITS == 1
/*
 * In fact, the table-based code will work in this case, but it can be
 * simplified by inlining the table in ?: form.
 */
#define crc32init_be()
#define crc32cleanup_be()

/**
 * crc32_be() - Calculate bitwise big-endian Ethernet AUTODIN II CRC32
 * @crc - seed value for computation.  ~0 for Ethernet, sometimes 0 for
 *        other uses, or the previous crc32 value if computing incrementally.
 * @p   - pointer to buffer over which CRC is run
 * @len - length of buffer @p
 * 
 */
uint32_t attribute((pure)) crc32_be(uint32_t crc, unsigned char const *p, size_t len)
{
	int i;
	while (len--) {
		crc ^= *p++ << 24;
		for (i = 0; i < 8; i++)
			crc =
			    (crc << 1) ^ ((crc & 0x80000000) ? CRCPOLY_BE :
					  0);
	}
	return crc;
}

#else				/* Table-based approach */
static uint32_t *crc32table_be;

/**
 * crc32init_be() - allocate and initialize BE table data
 */
static int
crc32init_be(void)
{
	unsigned i, j;
	uint32_t crc = 0x80000000;

	crc32table_be =
		malloc((1 << CRC_BE_BITS) * sizeof(uint32_t));
	if (!crc32table_be)
		return 1;
	crc32table_be[0] = 0;

	for (i = 1; i < 1 << CRC_BE_BITS; i <<= 1) {
		crc = (crc << 1) ^ ((crc & 0x80000000) ? CRCPOLY_BE : 0);
		for (j = 0; j < i; j++)
			crc32table_be[i + j] = crc ^ crc32table_be[j];
	}
	return 0;
}

/**
 * crc32cleanup_be(): free BE table data
 */
static void
crc32cleanup_be(void)
{
	if (crc32table_be) free(crc32table_be);
	crc32table_be = NULL;
}


/**
 * crc32_be() - Calculate bitwise big-endian Ethernet AUTODIN II CRC32
 * @crc - seed value for computation.  ~0 for Ethernet, sometimes 0 for
 *        other uses, or the previous crc32 value if computing incrementally.
 * @p   - pointer to buffer over which CRC is run
 * @len - length of buffer @p
 * 
 */
uint32_t attribute((pure)) crc32_be(uint32_t crc, unsigned char const *p, size_t len)
{
	while (len--) {
# if CRC_BE_BITS == 8
		crc = (crc << 8) ^ crc32table_be[(crc >> 24) ^ *p++];
# elif CRC_BE_BITS == 4
		crc ^= *p++ << 24;
		crc = (crc << 4) ^ crc32table_be[crc >> 28];
		crc = (crc << 4) ^ crc32table_be[crc >> 28];
# elif CRC_BE_BITS == 2
		crc ^= *p++ << 24;
		crc = (crc << 2) ^ crc32table_be[crc >> 30];
		crc = (crc << 2) ^ crc32table_be[crc >> 30];
		crc = (crc << 2) ^ crc32table_be[crc >> 30];
		crc = (crc << 2) ^ crc32table_be[crc >> 30];
# endif
	}
	return crc;
}
#endif

/*
 * A brief CRC tutorial.
 *
 * A CRC is a long-division remainder.  You add the CRC to the message,
 * and the whole thing (message+CRC) is a multiple of the given
 * CRC polynomial.  To check the CRC, you can either check that the
 * CRC matches the recomputed value, *or* you can check that the
 * remainder computed on the message+CRC is 0.  This latter approach
 * is used by a lot of hardware implementations, and is why so many
 * protocols put the end-of-frame flag after the CRC.
 *
 * It's actually the same long division you learned in school, except that
 * - We're working in binary, so the digits are only 0 and 1, and
 * - When dividing polynomials, there are no carries.  Rather than add and
 *   subtract, we just xor.  Thus, we tend to get a bit sloppy about
 *   the difference between adding and subtracting.
 *
 * A 32-bit CRC polynomial is actually 33 bits long.  But since it's
 * 33 bits long, bit 32 is always going to be set, so usually the CRC
 * is written in hex with the most significant bit omitted.  (If you're
 * familiar with the IEEE 754 floating-point format, it's the same idea.)
 *
 * Note that a CRC is computed over a string of *bits*, so you have
 * to decide on the endianness of the bits within each byte.  To get
 * the best error-detecting properties, this should correspond to the
 * order they're actually sent.  For example, standard RS-232 serial is
 * little-endian; the most significant bit (sometimes used for parity)
 * is sent last.  And when appending a CRC word to a message, you should
 * do it in the right order, matching the endianness.
 *
 * Just like with ordinary division, the remainder is always smaller than
 * the divisor (the CRC polynomial) you're dividing by.  Each step of the
 * division, you take one more digit (bit) of the dividend and append it
 * to the current remainder.  Then you figure out the appropriate multiple
 * of the divisor to subtract to being the remainder back into range.
 * In binary, it's easy - it has to be either 0 or 1, and to make the
 * XOR cancel, it's just a copy of bit 32 of the remainder.
 *
 * When computing a CRC, we don't care about the quotient, so we can
 * throw the quotient bit away, but subtract the appropriate multiple of
 * the polynomial from the remainder and we're back to where we started,
 * ready to process the next bit.
 *
 * A big-endian CRC written this way would be coded like:
 * for (i = 0; i < input_bits; i++) {
 * 	multiple = remainder & 0x80000000 ? CRCPOLY : 0;
 * 	remainder = (remainder << 1 | next_input_bit()) ^ multiple;
 * }
 * Notice how, to get at bit 32 of the shifted remainder, we look
 * at bit 31 of the remainder *before* shifting it.
 *
 * But also notice how the next_input_bit() bits we're shifting into
 * the remainder don't actually affect any decision-making until
 * 32 bits later.  Thus, the first 32 cycles of this are pretty boring.
 * Also, to add the CRC to a message, we need a 32-bit-long hole for it at
 * the end, so we have to add 32 extra cycles shifting in zeros at the
 * end of every message,
 *
 * So the standard trick is to rearrage merging in the next_input_bit()
 * until the moment it's needed.  Then the first 32 cycles can be precomputed,
 * and merging in the final 32 zero bits to make room for the CRC can be
 * skipped entirely.
 * This changes the code to:
 * for (i = 0; i < input_bits; i++) {
 *      remainder ^= next_input_bit() << 31;
 * 	multiple = (remainder & 0x80000000) ? CRCPOLY : 0;
 * 	remainder = (remainder << 1) ^ multiple;
 * }
 * With this optimization, the little-endian code is simpler:
 * for (i = 0; i < input_bits; i++) {
 *      remainder ^= next_input_bit();
 * 	multiple = (remainder & 1) ? CRCPOLY : 0;
 * 	remainder = (remainder >> 1) ^ multiple;
 * }
 *
 * Note that the other details of endianness have been hidden in CRCPOLY
 * (which must be bit-reversed) and next_input_bit().
 *
 * However, as long as next_input_bit is returning the bits in a sensible
 * order, we can actually do the merging 8 or more bits at a time rather
 * than one bit at a time:
 * for (i = 0; i < input_bytes; i++) {
 * 	remainder ^= next_input_byte() << 24;
 * 	for (j = 0; j < 8; j++) {
 * 		multiple = (remainder & 0x80000000) ? CRCPOLY : 0;
 * 		remainder = (remainder << 1) ^ multiple;
 * 	}
 * }
 * Or in little-endian:
 * for (i = 0; i < input_bytes; i++) {
 * 	remainder ^= next_input_byte();
 * 	for (j = 0; j < 8; j++) {
 * 		multiple = (remainder & 1) ? CRCPOLY : 0;
 * 		remainder = (remainder << 1) ^ multiple;
 * 	}
 * }
 * If the input is a multiple of 32 bits, you can even XOR in a 32-bit
 * word at a time and increase the inner loop count to 32.
 *
 * You can also mix and match the two loop styles, for example doing the
 * bulk of a message byte-at-a-time and adding bit-at-a-time processing
 * for any fractional bytes at the end.
 *
 * The only remaining optimization is to the byte-at-a-time table method.
 * Here, rather than just shifting one bit of the remainder to decide
 * in the correct multiple to subtract, we can shift a byte at a time.
 * This produces a 40-bit (rather than a 33-bit) intermediate remainder,
 * but again the multiple of the polynomial to subtract depends only on
 * the high bits, the high 8 bits in this case.  
 *
 * The multile we need in that case is the low 32 bits of a 40-bit
 * value whose high 8 bits are given, and which is a multiple of the
 * generator polynomial.  This is simply the CRC-32 of the given
 * one-byte message.
 *
 * Two more details: normally, appending zero bits to a message which
 * is already a multiple of a polynomial produces a larger multiple of that
 * polynomial.  To enable a CRC to detect this condition, it's common to
 * invert the CRC before appending it.  This makes the remainder of the
 * message+crc come out not as zero, but some fixed non-zero value.
 *
 * The same problem applies to zero bits prepended to the message, and
 * a similar solution is used.  Instead of starting with a remainder of
 * 0, an initial remainder of all ones is used.  As long as you start
 * the same way on decoding, it doesn't make a difference.
 */


/**
 * init_crc32(): generates CRC32 tables
 * 
 * On successful initialization, use count is increased.
 * This guarantees that the library functions will stay resident
 * in memory, and prevents someone from 'rmmod crc32' while
 * a driver that needs it is still loaded.
 * This also greatly simplifies drivers, as there's no need
 * to call an initialization/cleanup function from each driver.
 * Since crc32.o is a library module, there's no requirement
 * that the user can unload it.
 */
int
init_crc32(void)
{
	int rc1, rc2, rc;
	rc1 = crc32init_le();
	rc2 = crc32init_be();
	rc = rc1 || rc2;
	return rc;
}

/**
 * cleanup_crc32(): frees crc32 data when no longer needed
 */
void
cleanup_crc32(void)
{
	crc32cleanup_le();
	crc32cleanup_be();
}

#endif
