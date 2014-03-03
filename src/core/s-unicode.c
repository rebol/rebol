/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
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
**  Module:  s-unicode.c
**  Summary: unicode support functions
**  Section: strings
**  Author:  Carl Sassenrath
**  Notes:
**    The top part of this code is from Unicode Inc. The second
**    part was added by REBOL Technologies.
**
***********************************************************************/


/*
 * Copyright 2001-2004 Unicode, Inc.
 * 
 * Disclaimer
 * 
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 * 
 * Limitations on Rights to Redistribute This Code
 * 
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */

/* ---------------------------------------------------------------------

	Conversions between UTF32, UTF-16, and UTF-8.  Header file.

	Several funtions are included here, forming a complete set of
	conversions between the three formats.  UTF-7 is not included
	here, but is handled in a separate source file.

	Each of these routines takes pointers to input buffers and output
	buffers.  The input buffers are const.

	Each routine converts the text between *sourceStart and sourceEnd,
	putting the result into the buffer between *targetStart and
	targetEnd. Note: the end pointers are *after* the last item: e.g. 
	*(sourceEnd - 1) is the last item.

	The return result indicates whether the conversion was successful,
	and if not, whether the problem was in the source or target buffers.
	(Only the first encountered problem is indicated.)

	After the conversion, *sourceStart and *targetStart are both
	updated to point to the end of last text successfully converted in
	the respective buffers.

	Input parameters:
	sourceStart - pointer to a pointer to the source buffer.
		The contents of this are modified on return so that
		it points at the next thing to be converted.
	targetStart - similarly, pointer to pointer to the target buffer.
	sourceEnd, targetEnd - respectively pointers to the ends of the
		two buffers, for overflow checking only.

	These conversion functions take a ConversionFlags argument. When this
	flag is set to strict, both irregular sequences and isolated surrogates
	will cause an error.  When the flag is set to lenient, both irregular
	sequences and isolated surrogates are converted.

	Whether the flag is strict or lenient, all illegal sequences will cause
	an error return. This includes sequences such as: <F4 90 80 80>, <C0 80>,
	or <A0> in UTF-8, and values above 0x10FFFF in UTF-32. Conformant code
	must check for illegal sequences.

	When the flag is set to lenient, characters over 0x10FFFF are converted
	to the replacement character; otherwise (when the flag is set to strict)
	they constitute an error.

	Output parameters:
	The value "sourceIllegal" is returned from some routines if the input
	sequence is malformed.  When "sourceIllegal" is returned, the source
	value will point to the illegal value that caused the problem. E.g.,
	in UTF-8 when a sequence is malformed, it points to the start of the
	malformed sequence.  

	Author: Mark E. Davis, 1994.
	Rev History: Rick McGowan, fixes & updates May 2001.
		 Fixes & updates, Sept 2001.

------------------------------------------------------------------------ */

#include "sys-core.h"


/* ---------------------------------------------------------------------
	The following 4 definitions are compiler-specific.
	The C standard does not guarantee that wchar_t has at least
	16 bits, so wchar_t is no less portable than unsigned short!
	All should be unsigned values to avoid sign extension during
	bit mask & shift operations.
------------------------------------------------------------------------ */

typedef unsigned long	UTF32;	/* at least 32 bits */
typedef unsigned short	UTF16;	/* at least 16 bits */
typedef unsigned char	UTF8;	/* typically 8 bits */
typedef unsigned char	Boolean; /* 0 or 1 */

/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_MAX_BMP (UTF32)0x0000FFFF
#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (UTF32)0x0010FFFF

typedef enum {
	conversionOK, 		/* conversion successful */
	sourceExhausted,	/* partial character in source, but hit end */
	targetExhausted,	/* insuff. room in target for conversion */
	sourceIllegal		/* source sequence is illegal/malformed */
} ConversionResult;

typedef enum {
	strictConversion = 0,
	lenientConversion
} ConversionFlags;


ConversionResult ConvertUTF8toUTF16 (
		const UTF8** sourceStart, const UTF8* sourceEnd, 
		UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags);

ConversionResult ConvertUTF16toUTF8 (
		const UTF16** sourceStart, const UTF16* sourceEnd, 
		UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags);
		
ConversionResult ConvertUTF8toUTF32 (
		const UTF8** sourceStart, const UTF8* sourceEnd, 
		UTF32** targetStart, UTF32* targetEnd, ConversionFlags flags);

ConversionResult ConvertUTF32toUTF8 (
		const UTF32** sourceStart, const UTF32* sourceEnd, 
		UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags);
		
ConversionResult ConvertUTF16toUTF32 (
		const UTF16** sourceStart, const UTF16* sourceEnd, 
		UTF32** targetStart, UTF32* targetEnd, ConversionFlags flags);

ConversionResult ConvertUTF32toUTF16 (
		const UTF32** sourceStart, const UTF32* sourceEnd, 
		UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags);

Boolean isLegalUTF8Sequence(const UTF8 *source, const UTF8 *sourceEnd);

/* ---------------------------------------------------------------------

	Conversions between UTF32, UTF-16, and UTF-8. Source code file.
	Author: Mark E. Davis, 1994.
	Rev History: Rick McGowan, fixes & updates May 2001.
	Sept 2001: fixed const & error conditions per
	mods suggested by S. Parent & A. Lillich.
	June 2002: Tim Dodd added detection and handling of incomplete
	source sequences, enhanced error detection, added casts
	to eliminate compiler warnings.
	July 2003: slight mods to back out aggressive FFFE detection.
	Jan 2004: updated switches in from-UTF8 conversions.
	Oct 2004: updated to use UNI_MAX_LEGAL_UTF32 in UTF-32 conversions.

	See the header file "ConvertUTF.h" for complete documentation.

------------------------------------------------------------------------ */

#ifdef CVTUTF_DEBUG
#include <stdio.h>
#endif

static const int halfShift  = 10; /* used for shifting by 10 bits */

static const UTF32 halfBase = 0x0010000UL;
static const UTF32 halfMask = 0x3FFUL;

#define UNI_SUR_HIGH_START  (UTF32)0xD800
#define UNI_SUR_HIGH_END    (UTF32)0xDBFF
#define UNI_SUR_LOW_START   (UTF32)0xDC00
#define UNI_SUR_LOW_END     (UTF32)0xDFFF
#define false	   0
#define true	    1

/* --------------------------------------------------------------------- */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
static const char trailingBytesForUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/*
 * Magic values subtracted from a buffer value during UTF8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence.
 */
static const UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 
			 0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
static const UTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

#ifdef unused

ConversionResult ConvertUTF32toUTF16 (
	const UTF32** sourceStart, const UTF32* sourceEnd, 
	UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags) {
	ConversionResult result = conversionOK;
	const UTF32* source = *sourceStart;
	UTF16* target = *targetStart;
	while (source < sourceEnd) {
	UTF32 ch;
	if (target >= targetEnd) {
		result = targetExhausted; break;
	}
	ch = *source++;
	if (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
		/* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
		if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
		if (flags == strictConversion) {
			--source; /* return to the illegal value itself */
			result = sourceIllegal;
			break;
		} else {
			*target++ = UNI_REPLACEMENT_CHAR;
		}
		} else {
		*target++ = (UTF16)ch; /* normal case */
		}
	} else if (ch > UNI_MAX_LEGAL_UTF32) {
		if (flags == strictConversion) {
		result = sourceIllegal;
		} else {
		*target++ = UNI_REPLACEMENT_CHAR;
		}
	} else {
		/* target is a character in range 0xFFFF - 0x10FFFF. */
		if (target + 1 >= targetEnd) {
		--source; /* Back up source pointer! */
		result = targetExhausted; break;
		}
		ch -= halfBase;
		*target++ = (UTF16)((ch >> halfShift) + UNI_SUR_HIGH_START);
		*target++ = (UTF16)((ch & halfMask) + UNI_SUR_LOW_START);
	}
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF16toUTF32 (
	const UTF16** sourceStart, const UTF16* sourceEnd, 
	UTF32** targetStart, UTF32* targetEnd, ConversionFlags flags) {
	ConversionResult result = conversionOK;
	const UTF16* source = *sourceStart;
	UTF32* target = *targetStart;
	UTF32 ch, ch2;
	while (source < sourceEnd) {
	const UTF16* oldSource = source; /*  In case we have to back up because of target overflow. */
	ch = *source++;
	/* If we have a surrogate pair, convert to UTF32 first. */
	if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
		/* If the 16 bits following the high surrogate are in the source buffer... */
		if (source < sourceEnd) {
		ch2 = *source;
		/* If it's a low surrogate, convert to UTF32. */
		if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
			ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
			+ (ch2 - UNI_SUR_LOW_START) + halfBase;
			++source;
		} else if (flags == strictConversion) { /* it's an unpaired high surrogate */
			--source; /* return to the illegal value itself */
			result = sourceIllegal;
			break;
		}
		} else { /* We don't have the 16 bits following the high surrogate. */
		--source; /* return to the high surrogate */
		result = sourceExhausted;
		break;
		}
	} else if (flags == strictConversion) {
		/* UTF-16 surrogate values are illegal in UTF-32 */
		if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
		--source; /* return to the illegal value itself */
		result = sourceIllegal;
		break;
		}
	}
	if (target >= targetEnd) {
		source = oldSource; /* Back up source pointer! */
		result = targetExhausted; break;
	}
	*target++ = ch;
	}
	*sourceStart = source;
	*targetStart = target;
#ifdef CVTUTF_DEBUG
if (result == sourceIllegal) {
	fprintf(stderr, "ConvertUTF16toUTF32 illegal seq 0x%04x,%04x\n", ch, ch2);
	fflush(stderr);
}
#endif
	return result;
}

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF16toUTF8 (
	const UTF16** sourceStart, const UTF16* sourceEnd, 
	UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags) {
	ConversionResult result = conversionOK;
	const UTF16* source = *sourceStart;
	UTF8* target = *targetStart;
	while (source < sourceEnd) {
	UTF32 ch;
	unsigned short bytesToWrite = 0;
	const UTF32 byteMask = 0xBF;
	const UTF32 byteMark = 0x80; 
	const UTF16* oldSource = source; /* In case we have to back up because of target overflow. */
	ch = *source++;
	/* If we have a surrogate pair, convert to UTF32 first. */
	if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
		/* If the 16 bits following the high surrogate are in the source buffer... */
		if (source < sourceEnd) {
		UTF32 ch2 = *source;
		/* If it's a low surrogate, convert to UTF32. */
		if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
			ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
			+ (ch2 - UNI_SUR_LOW_START) + halfBase;
			++source;
		} else if (flags == strictConversion) { /* it's an unpaired high surrogate */
			--source; /* return to the illegal value itself */
			result = sourceIllegal;
			break;
		}
		} else { /* We don't have the 16 bits following the high surrogate. */
		--source; /* return to the high surrogate */
		result = sourceExhausted;
		break;
		}
	} else if (flags == strictConversion) {
		/* UTF-16 surrogate values are illegal in UTF-32 */
		if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
		--source; /* return to the illegal value itself */
		result = sourceIllegal;
		break;
		}
	}
	/* Figure out how many bytes the result will require */
	if (ch < (UTF32)0x80) {	     bytesToWrite = 1;
	} else if (ch < (UTF32)0x800) {     bytesToWrite = 2;
	} else if (ch < (UTF32)0x10000) {   bytesToWrite = 3;
	} else if (ch < (UTF32)0x110000) {  bytesToWrite = 4;
	} else {			    bytesToWrite = 3;
						ch = UNI_REPLACEMENT_CHAR;
	}

	target += bytesToWrite;
	if (target > targetEnd) {
		source = oldSource; /* Back up source pointer! */
		target -= bytesToWrite; result = targetExhausted; break;
	}
	switch (bytesToWrite) { /* note: everything falls through. */
		case 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
		case 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
		case 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
		case 1: *--target =  (UTF8)(ch | firstByteMark[bytesToWrite]);
	}
	target += bytesToWrite;
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}
#endif //unused

/* --------------------------------------------------------------------- */

/*
 * Utility routine to tell whether a sequence of bytes is legal UTF-8.
 * This must be called with the length pre-determined by the first byte.
 * If not calling this from ConvertUTF8to*, then the length can be set by:
 *  length = trailingBytesForUTF8[*source]+1;
 * and the sequence is illegal right away if there aren't that many bytes
 * available.
 * If presented with a length > 4, this returns false.  The Unicode
 * definition of UTF-8 goes up to 4-byte sequences.
 */

static Boolean isLegalUTF8(const UTF8 *source, int length) {
	UTF8 a;
	const UTF8 *srcptr = source+length;

	switch (length) {
	default: return false;
	/* Everything else falls through when "true"... */
	case 4: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
	case 3: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
	case 2: if ((a = (*--srcptr)) > 0xBF) return false;

		switch (*source) {
			/* no fall-through in this inner switch */
			case 0xE0: if (a < 0xA0) return false; break;
			case 0xED: if (a > 0x9F) return false; break;
			case 0xF0: if (a < 0x90) return false; break;
			case 0xF4: if (a > 0x8F) return false; break;
			default:   if (a < 0x80) return false;
		}

	case 1: if (*source >= 0x80 && *source < 0xC2) return false;
	}

	if (*source > 0xF4) return false;

	return true;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 sequence is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8Sequence(const UTF8 *source, const UTF8 *sourceEnd) {
	int length = trailingBytesForUTF8[*source]+1;
	if (source+length > sourceEnd) return false;
	return isLegalUTF8(source, length);
}

/* --------------------------------------------------------------------- */
#ifdef unused
ConversionResult ConvertUTF8toUTF16 (
	const UTF8** sourceStart, const UTF8* sourceEnd, 
	UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags) {
	ConversionResult result = conversionOK;
	const UTF8* source = *sourceStart;
	UTF16* target = *targetStart;
	while (source < sourceEnd) {
	UTF32 ch = 0;
	unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
	if (source + extraBytesToRead >= sourceEnd) {
		result = sourceExhausted; break;
	}
	/* Do this check whether lenient or strict */
	if (! isLegalUTF8(source, extraBytesToRead+1)) {
		result = sourceIllegal;
		break;
	}
	/*
	 * The cases all fall through. See "Note A" below.
	 */
	switch (extraBytesToRead) {
		case 5: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
		case 4: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
		case 3: ch += *source++; ch <<= 6;
		case 2: ch += *source++; ch <<= 6;
		case 1: ch += *source++; ch <<= 6;
		case 0: ch += *source++;
	}
	ch -= offsetsFromUTF8[extraBytesToRead];

	if (target >= targetEnd) {
		source -= (extraBytesToRead+1); /* Back up source pointer! */
		result = targetExhausted; break;
	}
	if (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
		/* UTF-16 surrogate values are illegal in UTF-32 */
		if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
		if (flags == strictConversion) {
			source -= (extraBytesToRead+1); /* return to the illegal value itself */
			result = sourceIllegal;
			break;
		} else {
			*target++ = UNI_REPLACEMENT_CHAR;
		}
		} else {
		*target++ = (UTF16)ch; /* normal case */
		}
	} else if (ch > UNI_MAX_UTF16) {
		if (flags == strictConversion) {
		result = sourceIllegal;
		source -= (extraBytesToRead+1); /* return to the start */
		break; /* Bail out; shouldn't continue */
		} else {
		*target++ = UNI_REPLACEMENT_CHAR;
		}
	} else {
		/* target is a character in range 0xFFFF - 0x10FFFF. */
		if (target + 1 >= targetEnd) {
		source -= (extraBytesToRead+1); /* Back up source pointer! */
		result = targetExhausted; break;
		}
		ch -= halfBase;
		*target++ = (UTF16)((ch >> halfShift) + UNI_SUR_HIGH_START);
		*target++ = (UTF16)((ch & halfMask) + UNI_SUR_LOW_START);
	}
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF32toUTF8 (
	const UTF32** sourceStart, const UTF32* sourceEnd, 
	UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags) {
	ConversionResult result = conversionOK;
	const UTF32* source = *sourceStart;
	UTF8* target = *targetStart;
	while (source < sourceEnd) {
	UTF32 ch;
	unsigned short bytesToWrite = 0;
	const UTF32 byteMask = 0xBF;
	const UTF32 byteMark = 0x80; 
	ch = *source++;
	if (flags == strictConversion ) {
		/* UTF-16 surrogate values are illegal in UTF-32 */
		if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
		--source; /* return to the illegal value itself */
		result = sourceIllegal;
		break;
		}
	}
	/*
	 * Figure out how many bytes the result will require. Turn any
	 * illegally large UTF32 things (> Plane 17) into replacement chars.
	 */
	if (ch < (UTF32)0x80) {	     bytesToWrite = 1;
	} else if (ch < (UTF32)0x800) {     bytesToWrite = 2;
	} else if (ch < (UTF32)0x10000) {   bytesToWrite = 3;
	} else if (ch <= UNI_MAX_LEGAL_UTF32) {  bytesToWrite = 4;
	} else {			    bytesToWrite = 3;
						ch = UNI_REPLACEMENT_CHAR;
						result = sourceIllegal;
	}
	
	target += bytesToWrite;
	if (target > targetEnd) {
		--source; /* Back up source pointer! */
		target -= bytesToWrite; result = targetExhausted; break;
	}
	switch (bytesToWrite) { /* note: everything falls through. */
		case 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
		case 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
		case 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
		case 1: *--target = (UTF8) (ch | firstByteMark[bytesToWrite]);
	}
	target += bytesToWrite;
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF8toUTF32 (
	const UTF8** sourceStart, const UTF8* sourceEnd, 
	UTF32** targetStart, UTF32* targetEnd, ConversionFlags flags) {
	ConversionResult result = conversionOK;
	const UTF8* source = *sourceStart;
	UTF32* target = *targetStart;
	while (source < sourceEnd) {
	UTF32 ch = 0;
	unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
	if (source + extraBytesToRead >= sourceEnd) {
		result = sourceExhausted; break;
	}
	/* Do this check whether lenient or strict */
	if (! isLegalUTF8(source, extraBytesToRead+1)) {
		result = sourceIllegal;
		break;
	}
	/*
	 * The cases all fall through. See "Note A" below.
	 */
	switch (extraBytesToRead) {
		case 5: ch += *source++; ch <<= 6;
		case 4: ch += *source++; ch <<= 6;
		case 3: ch += *source++; ch <<= 6;
		case 2: ch += *source++; ch <<= 6;
		case 1: ch += *source++; ch <<= 6;
		case 0: ch += *source++;
	}
	ch -= offsetsFromUTF8[extraBytesToRead];

	if (target >= targetEnd) {
		source -= (extraBytesToRead+1); /* Back up the source pointer! */
		result = targetExhausted; break;
	}
	if (ch <= UNI_MAX_LEGAL_UTF32) {
		/*
		 * UTF-16 surrogate values are illegal in UTF-32, and anything
		 * over Plane 17 (> 0x10FFFF) is illegal.
		 */
		if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
		if (flags == strictConversion) {
			source -= (extraBytesToRead+1); /* return to the illegal value itself */
			result = sourceIllegal;
			break;
		} else {
			*target++ = UNI_REPLACEMENT_CHAR;
		}
		} else {
		*target++ = ch;
		}
	} else { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
		result = sourceIllegal;
		*target++ = UNI_REPLACEMENT_CHAR;
	}
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

/* ---------------------------------------------------------------------

	Note A.
	The fall-through switches in UTF-8 reading code save a
	temp variable, some decrements & conditionals.  The switches
	are equivalent to the following loop:
	{
		int tmpBytesToRead = extraBytesToRead+1;
		do {
		ch += *source++;
		--tmpBytesToRead;
		if (tmpBytesToRead) ch <<= 6;
		} while (tmpBytesToRead > 0);
	}
	In UTF-8 writing code, the switches on "bytesToWrite" are
	similarly unrolled loops.

   --------------------------------------------------------------------- */

#endif //unused


/***********************************************************************
************************************************************************
**
**	Code below added by REBOL Technologies 2008
**
************************************************************************
***********************************************************************/

/***********************************************************************
**
*/	REBINT What_UTF(REBYTE *bp, REBCNT len)
/*
**		Tell us what UTF encoding the string has. Negative for LE.
**
***********************************************************************/
{
	// UTF8:
	if (len >= 3 && bp[0] == 0xef && bp[1] == 0xbb && bp[2] == 0xbf) return 8;

	if (len >= 2) {

		// UTF16:
		if (bp[0] == 0xfe && bp[1] == 0xff) return 16;

		// Either UTF16 or 32:
		if (bp[0] == 0xff && bp[1] == 0xfe) {
			if (len >= 4 && bp[2] == 0 && bp[3] == 0) return -32;
			return -16;
		}

		// UTF32
		if (len >= 4 && bp[0] == 0 && bp[1] == 0 && bp[2] == 0xfe && bp[3] == 0xff)
			return 32;
	}

	// Unknown:
	return 0;
}

/***********************************************************************
**
*/	REBFLG Legal_UTF8_Char(REBYTE *str, REBCNT len)
/*
**		Returns TRUE if char is legal.
**
***********************************************************************/
{
	return isLegalUTF8Sequence(str, str + len);
}


/***********************************************************************
**
*/	REBYTE *Check_UTF8(REBYTE *str, REBCNT len)
/*
**		Returns 0 for success, else str where error occurred.
**
***********************************************************************/
{
	REBINT n;
	REBYTE *end = str + len;

	for (;str < end; str += n) {
		n = trailingBytesForUTF8[*str] + 1;
		if (str + n > end || !isLegalUTF8(str, n)) return str;
	}

	return 0;
}


/***********************************************************************
**
*/	REBCNT Decode_UTF8_Char(REBYTE **str, REBCNT *len)
/*
**		Converts a single UTF8 code-point (to 32 bit).
**		Errors are returned as zero. (So prescan source for null.)
**		Increments str by extra chars needed.
**		Decrements len by extra chars needed.
**
***********************************************************************/
{
	UTF8 *source = *str;
	UTF32 ch = 0;
	int slen = trailingBytesForUTF8[*source];

	// Check that we have enough valid source bytes:
	if (len) {
		if (slen+1 > *len) return 0;
	}
	else {
		for (; slen >= 0; slen--)
			if (source[slen] < 0x80) return 0;
		slen = trailingBytesForUTF8[*source];
	}

	// Do this check whether lenient or strict:
	// if (!isLegalUTF8(source, slen+1)) return 0;

	switch (slen) {
		case 5: ch += *source++; ch <<= 6;
		case 4: ch += *source++; ch <<= 6;
		case 3: ch += *source++; ch <<= 6;
		case 2: ch += *source++; ch <<= 6;
		case 1: ch += *source++; ch <<= 6;
		case 0: ch += *source++;
	}
	ch -= offsetsFromUTF8[slen];

	// UTF-16 surrogate values are illegal in UTF-32, and anything
	// over Plane 17 (> 0x10FFFF) is illegal.
	if (ch > UNI_MAX_LEGAL_UTF32) return 0;
	if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) return 0;

	if (len) *len -= slen;
	*str += slen;
	return ch;
}


/***********************************************************************
**
*/	int Decode_UTF8(REBUNI *dst, REBYTE *src, REBCNT len, REBFLG ccr)
/*
**		Decode UTF8 byte string into a 16 bit preallocated array.
**
**		dst: the desination array, must always be large enough!
**		src: source binary data
**		len: byte-length of source (not number of chars)
**		ccr: convert CRLF/CR to LF
**
**		Returns length in chars (negative if all chars are latin-1).
**		No terminator is added.
**
***********************************************************************/
{
	int flag = -1;
	UTF32 ch;
	REBUNI *start = dst;

	for (; len > 0; len--, src++) {
		if ((ch = *src) >= 0x80) {
			ch = Decode_UTF8_Char(&src, &len);
			if (ch == 0) ch = UNI_REPLACEMENT_CHAR; // temporary!
			if (ch > 0xff) flag = 1;
		} if (ch == CR && ccr) {
			if (src[1] == LF) continue;
			ch = LF;
		}
		*dst++ = (REBUNI)ch;
	}

	return (dst - start) * flag;
}


/***********************************************************************
**
*/	int Decode_UTF16(REBUNI *dst, REBYTE *src, REBCNT len, REBFLG lee, REBFLG ccr)
/*
**		dst: the desination array, must always be large enough!
**		src: source binary data
**		len: byte-length of source (not number of chars)
**		lee: little endian encoded
**		ccr: convert CRLF/CR to LF
**
**		Returns length in chars (negative if all chars are latin-1).
**		No terminator is added.
**
***********************************************************************/
{
	int flag = -1;
	UTF32 ch;
	REBUNI *start = dst;

	if (ccr) ccr = 1;

	for (; len > 0; len--, src++) {

		// Combine bytes in big or little endian format:
		ch = *src;
		if (!lee) ch <<= 8;
		if (--len <= 0) break;
		src++;
		ch |= lee ? (UTF32)(*src) << 8 : *src;

		// Skip CR, but add LF (even if missing)
		if (ccr) {
			if (ccr < 0 && ch != LF) {
				ccr = 1;
				*dst++ = LF;
			}
			if (ch == CR) {
				ccr = -1;
				continue;
			}
		}

		// check for surrogate pair ??

		if (ch > 0xff) flag = 1;

		*dst++ = (REBUNI)ch;
	}

	return (dst - start) * flag;
}


/***********************************************************************
**
*/	int Decode_UTF32(REBUNI *dst, REBYTE *src, REBINT len, REBFLG lee, REBFLG ccr)
/*
***********************************************************************/
{
	return 0;
}


/***********************************************************************
**
*/	REBSER *Decode_UTF_String(REBYTE *bp, REBCNT len, REBINT utf)
/*
**		Do all the details to decode a string.
**		Input is a byte series. Len is len of input.
**		The utf is 0, 8, +/-16, +/-32.
**		A special -1 means use the BOM.
**
***********************************************************************/
{
	REBSER *ser = BUF_UTF8; // buffer is Unicode width
	REBSER *dst;
	REBINT size;

	if (utf == -1) {
		utf = What_UTF(bp, len);
		if (utf) {
			if (utf == 8) bp += 3, len -= 3;
			else if (utf == -16 || utf == 16) bp += 2, len -= 2;
			else if (utf == -32 || utf == 32) bp += 4, len -= 4;
		}
	}

	if (utf == 0 || utf == 8) {
		size = Decode_UTF8((REBUNI*)Reset_Buffer(ser, len), bp, len, TRUE);
	} 
	else if (utf == -16 || utf == 16) {
		size = Decode_UTF16((REBUNI*)Reset_Buffer(ser, len/2 + 1), bp, len, utf < 0, TRUE);
	}
//	else if (utf == -32 || utf == 32) {
//		size = Decode_UTF32((REBUNI*)Reset_Buffer(ser, len/4 + 1), bp, len, utf < 0, TRUE);
//	}

	if (size < 0) {
		size = -size;
		dst = Make_Binary(size);
		Append_Uni_Bytes(dst, UNI_HEAD(ser), size);
	}
	else {
		dst = Make_Unicode(size);
		Append_Uni_Uni(dst, UNI_HEAD(ser), size);
	}

	return dst;
}


/***********************************************************************
**
*/	REBCNT Length_As_UTF8(REBUNI *src, REBCNT len, REBOOL uni, REBOOL ccr)
/*
**		Returns how long the UTF8 encoded string would be.
**
***********************************************************************/
{
	REBCNT size = 0;
	REBCNT c;
	REBYTE *bp = (REBYTE*)src;

	for (; len > 0; len--) {
		c = uni ? *src++ : *bp++;
		if (c < (UTF32)0x80) {
#ifdef TO_WIN32
			if (ccr && c == LF) size++; // because we will add a CR to it
#endif
			size++;
		}
		else if (c < (UTF32)0x800)         size += 2;
		else if (c < (UTF32)0x10000)       size += 3;
		else if (c <= UNI_MAX_LEGAL_UTF32) size += 4;
		else size += 3;
	}

	return size;
}


/***********************************************************************
**
*/	REBCNT Encode_UTF8_Char(REBYTE *dst, REBCNT src)
/*
**		Converts a single char to UTF8 code-point.
**		Returns length of char stored in dst.
**		Be sure dst has at least 4 bytes available.
**
***********************************************************************/
{
	int len = 0;
	const UTF32 mask = 0xBF;
	const UTF32 mark = 0x80; 

	if (src < (UTF32)0x80) len = 1;
	else if (src < (UTF32)0x800) len = 2;
	else if (src < (UTF32)0x10000) len = 3;
	else if (src <= UNI_MAX_LEGAL_UTF32) len = 4;
	else {
		len = 3;
		src = UNI_REPLACEMENT_CHAR;
	}

	dst += len;

	switch (len) {
		case 4: *--dst = (UTF8)((src | mark) & mask); src >>= 6;
		case 3: *--dst = (UTF8)((src | mark) & mask); src >>= 6;
		case 2: *--dst = (UTF8)((src | mark) & mask); src >>= 6;
		case 1: *--dst = (UTF8) (src | firstByteMark[len]);
	}

	return len;
}


/***********************************************************************
**
*/	REBCNT Encode_UTF8(REBYTE *dst, REBINT max, void *src, REBCNT *len, REBFLG uni, REBFLG ccr)
/*
**		Encode the unicode into UTF8 byte string.
**
**		Source string can be byte or unichar sized (uni = TRUE);
**		Max is the maximum size of the result (UTF8).
**		Returns number of source chars used.
**		Updates len for dst bytes used.
**		Does not add a terminator.
**
***********************************************************************/
{
	REBUNI c;
	REBINT n;
	REBYTE buf[8];
	REBYTE *bs = dst; // save start
	REBYTE *bp = (REBYTE*)src;
	REBUNI *up = (REBUNI*)src;
	REBCNT cnt;

	if (len) cnt = *len;
	else {
		cnt = uni ? wcslen((REBUNI*)bp) : LEN_BYTES((REBYTE*)bp);
	}

	for (; max > 0 && cnt > 0; cnt--) {
		c = uni ? *up++ : *bp++;
		if (c < 0x80) {
#if defined(TO_WIN32)
			if (ccr && c == LF) {
				// If there's not room, don't try to output CRLF
				if (2 > max) {up--; break;}
				*dst++ = CR;
				max--;
				c = LF;
			}
#endif
			*dst++ = (REBYTE)c;
			max--;
		}
		else {
			n = Encode_UTF8_Char(buf, c);
			if (n > max) {up--; break;}
			memcpy(dst, buf, n);
			dst += n;
			max -= n;
		}
	}

	if (len) *len = dst - bs;

	return uni ? up - (REBUNI*)src : bp - (REBYTE*)src;
}


/***********************************************************************
**
*/  int Encode_UTF8_Line(REBSER *dst, REBSER *src, REBCNT idx)
/*
**		Encode a unicode source buffer into a binary line of UTF8.
**		Include the LF terminator in the result.
**		Return the length of the line buffer.
**
***********************************************************************/
{
	REBUNI *up = UNI_HEAD(src);
	REBCNT len  = SERIES_TAIL(src);
	REBCNT tail;
	REBUNI c;
	REBINT n;
	REBYTE buf[8];

	tail = RESET_TAIL(dst);

	while (idx < len) {
		if ((c = up[idx]) < 0x80) {
			EXPAND_SERIES_TAIL(dst, 1);
			BIN_HEAD(dst)[tail++] = (REBYTE)c;
		}
		else {
			n = Encode_UTF8_Char(buf, c);
			EXPAND_SERIES_TAIL(dst, n);
			memcpy(BIN_SKIP(dst, tail), buf, n);
			tail += n;
		}
		idx++;
		if (c == LF) break;
	}

	BIN_HEAD(dst)[tail] = 0;
	SERIES_TAIL(dst) = tail;
	return idx;
}


/***********************************************************************
**
*/	REBSER *Encode_UTF8_Value(REBVAL *arg, REBCNT len, REBFLG opts)
/*
**		Do all the details to encode a string as UTF8.
**		No_copy means do not make a copy.
**		Result can be a shared buffer!
**
***********************************************************************/
{
	REBSER *ser = BUF_FORM; // a shared buffer
	REBCNT size;
	REBYTE *cp;
	REBFLG ccr = GET_FLAG(opts, ENC_OPT_CRLF);

	if (VAL_BYTE_SIZE(arg)) {
		REBYTE *bp = VAL_BIN_DATA(arg);

		if (Is_Not_ASCII(bp, len)) {
			size = Length_As_UTF8((REBUNI*)bp, len, FALSE, (REBOOL)ccr);
			cp = Reset_Buffer(ser, size + (GET_FLAG(opts, ENC_OPT_BOM) ? 3 : 0));
			Encode_UTF8(cp, size, bp, &len, FALSE, ccr);
		}
		else if (GET_FLAG(opts, ENC_OPT_NO_COPY)) return 0;
		else return Copy_Bytes(bp, len);

	} else {
		REBUNI *up = VAL_UNI_DATA(arg);

		size = Length_As_UTF8(up, len, TRUE, (REBOOL)ccr);
		cp = Reset_Buffer(ser, size + (GET_FLAG(opts, ENC_OPT_BOM) ? 3 : 0));
		Encode_UTF8(Reset_Buffer(ser, size), size, up, &len, TRUE, ccr);
	}

	SERIES_TAIL(ser) = len;
	STR_TERM(ser);

	return Copy_Bytes(BIN_HEAD(ser), len);
}


/***********************************************************************
**
*/	REBSER *Encode_String(void *str, REBCNT len, REBCNT opts)
/*
**		str: byte or unicode string
**		len: length in chars
**		opt: special options (UTF, LE/BE, CR/LF, BOM)
**
***********************************************************************/
{
	REBSER *ser = 0;

	if (GET_FLAG(opts, ENC_OPT_UTF8)) {
		//ser = Encode_UTF8_Value(arg, len, opts);
	}

	if (GET_FLAG(opts, ENC_OPT_UTF16)) {
		// ser = Encode_UTF16_Value(arg, len, FALSE, ccr);
	}

//	if (utf == 0 || ser == 0) {
		// Enline_Bytes();
//	}

	return ser;
}
