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
**  Summary: REBOL Codec Definitions
**  Module:  reb-codec.h
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/
#ifndef CODI_DEFINED
#define CODI_DEFINED

// Codec image interface:
//
// If your codec routine returns CODI_IMAGE, it is expected that the
// ->bits field contains a block of memory allocated with Make_Mem
// of size (->w * ->h * 4).  This will be freed by the
// REBNATIVE(do_codec) in n-system.c
//
// If your codec routine returns CODI_BINARY, it is
// expected that the ->data field contains a block of memory
// allocated with Make_Mem of size ->len.  This will be freed by
// the REBNATIVE(do_codec) in n-system.c
//
// If your codec routine returns CODI_TEXT, it is
// expected that the ->data field is 3rd input binary! argument in
// the REBNATIVE(do_codec) in n-system.c
// so the deallocation is left to GC
//
struct reb_codec_image {
	int action;
	int w;
	int h;
	int len;
	union {
		int alpha;
		int type;  // used to provide info about prefered image type (codec)
	};
	unsigned char *data;
	union {
		unsigned int *bits;
		void *other;
	};
	int error;
};

typedef struct reb_codec_image REBCDI;

typedef int (*codo)(REBCDI *cdi);

// Media types:
enum {
	CODI_ERROR,
	CODI_CHECK,				// error code is inverted result (IDENTIFY)
	CODI_BINARY,
	CODI_TEXT,
	CODI_IMAGE,
	CODI_SOUND,
	CODI_BLOCK,
	CODI_STRING,			// result is in codi->other as a series (no need to copy).
};

// Codec commands:
enum {
	CODI_IDENTIFY,
	CODI_DECODE,
	CODI_ENCODE,
};

// Codec errors:
enum {
	CODI_ERR_NA = 1,		// Feature not available
	CODI_ERR_NO_ACTION,		// Requested action unknown
	CODI_ERR_ENCODING,		// Encoding method not supported
	CODI_ERR_SIGNATURE,		// Header signature is not correct
	CODI_ERR_BIT_LEN,		// Bit length is not supported
	CODI_ERR_BAD_TABLE,		// Image tables are wrong
	CODI_ERR_BAD_DATA,		// Generic
};

enum {
	CODI_IMG_PNG,           // Portable Network Graphics
	CODI_IMG_JPEG,          // Joint Photographic Experts Group
	CODI_IMG_GIF,           // Graphics Interchange Format
	CODI_IMG_DDS,           // DirectDraw Surface
	CODI_IMG_DNG,           // Digital Negative
	CODI_IMG_BMP,           // Device independent bitmap
	CODI_IMG_ICO,
	CODI_IMG_TIFF,          // Tagged Image File Format
	CODI_IMG_JXR,           // Windows Digital Photo (JpegXR)
	CODI_IMG_HEIF,          // High Efficiency Image Format
	CODI_IMG_WEBP,          //
	CODI_IMG_JP2,           // JPEG 2000 (JP2)
};

//#ifndef WINCODEC_ERR_COMPONENTNOTFOUND
//#define WINCODEC_ERR_COMPONENTNOTFOUND  0x88982F50
//#endif

#endif
