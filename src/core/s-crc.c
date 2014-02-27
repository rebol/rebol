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
#define MASK_CRC(crc) ((crc) & 0x00ffffffL)	  /* if CRCBITS is 24 */
#define CRCHIBIT ((REBCNT) (1L<<(CRCBITS-1))) /* 0x8000 if CRCBITS is 16 */
#define CRCSHIFTS (CRCBITS-8)
#define CCITTCRC 0x1021 	/* CCITT's 16-bit CRC generator polynomial */
#define PRZCRC   0x864cfb	/* PRZ's 24-bit CRC generator polynomial */
#define CRCINIT  0xB704CE	/* Init value for CRC accumulator */

static REBCNT *CRC_Table;

/***********************************************************************
**
*/	static REBCNT Generate_CRC(REBYTE ch, REBCNT poly, REBCNT accum)
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
*/	static void Make_CRC_Table(REBCNT poly)
/*
**		Derives a CRC lookup table from the CRC polynomial.
**		The table is used later by crcupdate function given below.
**		Only needs to be called once at the dawn of time.
**
***********************************************************************/
{
	REBINT i;

	FOREACH (i, 256) CRC_Table[i] = Generate_CRC((REBYTE) i, poly, 0);
}


/***********************************************************************
**
*/	REBINT Compute_CRC(REBYTE *str, REBCNT len)
/*
***********************************************************************/
{
	REBYTE	n;
	REBINT crc = (REBINT)len + (REBINT)((REBYTE)(*str));

	for (; len > 0; len--) {
		n = (REBYTE)((crc >> CRCSHIFTS) ^ (REBYTE)(*str++));
		crc = MASK_CRC(crc << 8) ^ (REBINT)CRC_Table[n];
	}

	return crc;
}


/***********************************************************************
**
*/	REBINT Hash_String(REBYTE *str, REBCNT len)
/*
**		Return a case insensitive hash value for the string.  The
**		string does not have to be zero terminated and UTF8 is ok.
**
***********************************************************************/
{
	REBYTE	n;
	REBINT hash = (REBINT)len + (REBINT)((REBYTE)LO_CASE(*str));

	for (; len > 0; len--) {
		n = (REBYTE)((hash >> CRCSHIFTS) ^ (REBYTE)LO_CASE(*str++));
		hash = MASK_CRC(hash << 8) ^ (REBINT)CRC_Table[n];
	}

	return hash;
}


/***********************************************************************
**
*/	REBINT Hash_Word(REBYTE *str, REBINT len)
/*
**		Return a case insensitive hash value for the string.
**
***********************************************************************/
{
	REBINT m, n;
	REBINT hash;
	REBCNT ulen;

	if (len < 0) len = LEN_BYTES(str);

	hash = (REBINT)len + (REBINT)((REBYTE)LO_CASE(*str));

	ulen = (REBCNT)len; // so the & operation later isn't for the wrong type

	for (; ulen > 0; str++, ulen--) {
		n = *str;
		if (n > 127 && NZ(m = Decode_UTF8_Char(&str, &ulen))) n = m; // mods str, ulen
		if (n < UNICODE_CASES) n = LO_CASE(n);
		n = (REBYTE)((hash >> CRCSHIFTS) ^ (REBYTE)n); // drop upper 8 bits
		hash = MASK_CRC(hash << 8) ^ (REBINT)CRC_Table[n];
	}

	return hash;
}


/***********************************************************************
**
*/	REBINT Hash_Value(REBVAL *val, REBCNT hash_size)
/*
**		Return a case insensitive hash value for any value.
**
**		Result will be > 0 and < hash_size, except if
**		datatype cannot be hashed, a 0 is returned.
**
***********************************************************************/
{
	REBCNT	ret;

	switch(VAL_TYPE(val)) {

	case REB_WORD:
	case REB_SET_WORD:
	case REB_GET_WORD:
	case REB_LIT_WORD:
	case REB_REFINEMENT:
	case REB_ISSUE:
		ret = VAL_WORD_CANON(val);
		break;

	case REB_BINARY:
	case REB_STRING:
	case REB_FILE:
	case REB_EMAIL:
	case REB_URL:
	case REB_TAG:
		ret = Hash_String(VAL_BIN_DATA(val), Val_Byte_Len(val));
		break;

	case REB_LOGIC:
		ret = VAL_LOGIC(val) ? (hash_size/5) : (2*hash_size/5);
		break;

	case REB_INTEGER:
	case REB_DECIMAL: // depends on INT64 sharing the DEC64 bits
		ret = (REBCNT)(VAL_INT64(val) >> 32) ^ ((REBCNT)VAL_INT64(val));
		break;

	case REB_CHAR:
		ret = VAL_CHAR(val) << 15; // avoid running into WORD hashes
		break;

	case REB_MONEY:
		ret = VAL_ALL_BITS(val)[0] ^ VAL_ALL_BITS(val)[1] ^ VAL_ALL_BITS(val)[2];
		break;

	case REB_TIME:
	case REB_DATE:
		ret = (REBCNT)(VAL_TIME(val) ^ (VAL_TIME(val) / SEC_SEC));
		if (IS_DATE(val)) ret ^= VAL_DATE(val).bits;
		break;

	case REB_TUPLE:
		ret = Hash_String(VAL_TUPLE(val), VAL_TUPLE_LEN(val));
		break;

	case REB_PAIR:
		ret = VAL_ALL_BITS(val)[0] ^ VAL_ALL_BITS(val)[1];
		break;

	case REB_OBJECT:
		ret = ((REBCNT)VAL_OBJ_FRAME(val)) >> 4;
		break;

	case REB_DATATYPE:
		ret = Hash_Word(Get_Sym_Name(VAL_DATATYPE(val)+1), -1);
		break;

	case REB_NONE:
		ret = 1;
		break;

	case REB_UNSET:
		ret = 0;
		break;

	default:
		return 0;  //ret = 3 * (hash_size/5);
	}

	return 1 + ((hash_size-1) & ret);
}


/***********************************************************************
**
*/	REBSER *Make_Hash_Array(REBCNT len)
/*
***********************************************************************/
{
	REBCNT n;
	REBSER *ser;

	n = Get_Hash_Prime(len * 2); // best when 2X # of keys
	if (!n) Trap_Num(RE_SIZE_LIMIT, len);

	ser = Make_Series(n + 1, sizeof(REBCNT), FALSE);
	LABEL_SERIES(ser, "make hash array");
	Clear_Series(ser);
	ser->tail = n;

	return ser;
}


/***********************************************************************
**
*/	REBSER *Hash_Block(REBVAL *block, REBCNT cased)
/*
**		Hash ALL values of a block. Return hash array series.
**		Used for SET logic (unique, union, etc.)
**
**		Note: hash array contents (indexes) are 1-based!
**
***********************************************************************/
{
	REBCNT n;
	REBCNT key;
	REBSER *hser;
	REBCNT *hashes;
	REBSER *series = VAL_SERIES(block);

	// Create the hash array (integer indexes):
	hser = Make_Hash_Array(VAL_LEN(block));
	hashes = (REBCNT*)hser->data;

	for (n = VAL_INDEX(block); n < series->tail; n++) {
		key = Find_Key(series, hser, BLK_SKIP(series, n), 1, cased, 0);
		hashes[key] = n + 1;
	}

	return hser;
}


/***********************************************************************
**
*/	void Init_CRC(void)
/*
***********************************************************************/
{
	CRC_Table = Make_Mem(sizeof(REBCNT) * 256);
	Make_CRC_Table(PRZCRC);
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




static u32 *crc32_table = 0;

static void Make_CRC32_Table(void) {
	unsigned long c;
	int n,k;

	crc32_table = Make_Mem(256 * sizeof(u32));

	for(n=0;n<256;n++) {
		c=(unsigned long)n;
		for(k=0;k<8;k++) {
			if(c&1)
				c=0xedb88320L^(c>>1);
			else
				c=c>>1;
		}
		crc32_table[n]=c;
	}
}

REBCNT Update_CRC32(u32 crc, REBYTE *buf, int len) {
	u32 c = ~crc;
	int n;

	if(!crc32_table) Make_CRC32_Table();

	for(n = 0; n < len; n++)
		c = crc32_table[(c^buf[n])&0xff]^(c>>8);

	return ~c;
}

/***********************************************************************
**
*/	REBCNT CRC32(REBYTE *buf, REBCNT len)
/*
***********************************************************************/
{
	return Update_CRC32(0x00000000L, buf, len);
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
