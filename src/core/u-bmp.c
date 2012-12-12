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
**  Module:  u-bmp.c
**  Summary: conversion to and from BMP graphics format
**  Section: utility
**  Notes:
**    This is an optional part of R3. This file can be replaced by
**    library function calls into an updated implementation.
**
***********************************************************************/

#include "sys-core.h"

//**********************************************************************

#define	WADJUST(x) (((x * 3L + 3) / 4) * 4)

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned int	DWORD;
typedef int				LONG;

typedef struct tagBITMAP
{
    int     bmType;
    int     bmWidth;
    int     bmHeight;
    int     bmWidthBytes;
    BYTE    bmPlanes;
    BYTE    bmBitsPixel;
    void 	*bmBits;
} BITMAP;
typedef BITMAP *PBITMAP;
typedef BITMAP *NPBITMAP;
typedef BITMAP *LPBITMAP;

/* Bitmap Header structures */
typedef struct tagRGBTRIPLE
{
    BYTE    rgbtBlue;
    BYTE    rgbtGreen;
    BYTE    rgbtRed;
} RGBTRIPLE;
typedef RGBTRIPLE *LPRGBTRIPLE;

typedef struct tagRGBQUAD
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;
typedef RGBQUAD *LPRGBQUAD;

/* structures for defining DIBs */
typedef struct tagBITMAPCOREHEADER
{
    DWORD   bcSize;
    short   bcWidth;
    short   bcHeight;
    WORD    bcPlanes;
    WORD    bcBitCount;
} BITMAPCOREHEADER;
typedef BITMAPCOREHEADER*      PBITMAPCOREHEADER;
typedef BITMAPCOREHEADER *LPBITMAPCOREHEADER;

char *mapBITMAPCOREHEADER = "lssss";

typedef struct tagBITMAPINFOHEADER
{
    DWORD   biSize;
    LONG    biWidth;
    LONG    biHeight;
    WORD    biPlanes;
    WORD    biBitCount;
    DWORD   biCompression;
    DWORD   biSizeImage;
    LONG    biXPelsPerMeter;
    LONG    biYPelsPerMeter;
    DWORD   biClrUsed;
    DWORD   biClrImportant;
} BITMAPINFOHEADER;

char *mapBITMAPINFOHEADER = "lllssllllll";

typedef BITMAPINFOHEADER*      PBITMAPINFOHEADER;
typedef BITMAPINFOHEADER *LPBITMAPINFOHEADER;

/* constants for the biCompression field */
#define BI_RGB      0L
#define BI_RLE8     1L
#define BI_RLE4     2L

typedef struct tagBITMAPINFO
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[1];
} BITMAPINFO;
typedef BITMAPINFO*     PBITMAPINFO;
typedef BITMAPINFO *LPBITMAPINFO;

typedef struct tagBITMAPCOREINFO
{
    BITMAPCOREHEADER bmciHeader;
    RGBTRIPLE        bmciColors[1];
} BITMAPCOREINFO;
typedef BITMAPCOREINFO*      PBITMAPCOREINFO;
typedef BITMAPCOREINFO *LPBITMAPCOREINFO;

typedef struct tagBITMAPFILEHEADER
{
    char    bfType[2];
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} BITMAPFILEHEADER;
typedef BITMAPFILEHEADER*      PBITMAPFILEHEADER;
typedef BITMAPFILEHEADER *LPBITMAPFILEHEADER;

char *mapBITMAPFILEHEADER = "bblssl";

typedef RGBQUAD *RGBQUADPTR;

//**********************************************************************

static int longaligned(void) {
	static char filldata[] = {0,0,1,1,1,1};
	struct {
		unsigned short a;
		unsigned int b;
	} a={0};
	memcpy(&a, filldata, 6);
	if (a.b != 0x01010101) return TRUE;
	return FALSE;
}

void Map_Bytes(void *dstp, REBYTE **srcp, char *map) {
	REBYTE *src = *srcp;
	REBYTE *dst = dstp;
	char c;
#ifdef ENDIAN_LITTLE
	while ((c = *map++) != 0) {
		switch(c) {
		case 'b':
			*dst++ = *src++;
			break;

		case 's':
			*((short *)dst) = *((short *)src);
			dst += sizeof(short);
			src += 2;
			break;

		case 'l':
			if (longaligned()) {
				while(((unsigned long)dst)&3)
					dst++;
			}
			*((REBCNT *)dst) = *((REBCNT *)src);
			dst += sizeof(REBCNT);
			src += 4;
			break;
		}
	}
#else
	while ((c = *map++) != 0) {
		switch(c) {
		case 'b':
			*dst++ = *src++;
			break;

		case 's':
			*((short *)dst) = src[0]|(src[1]<<8);
			dst += sizeof(short);
			src += 2;
			break;

		case 'l':
			if (longaligned()) {
				while (((unsigned long)dst)&3)
					dst++;
			}
			*((REBCNT *)dst) = src[0]|(src[1]<<8)|
					(src[2]<<16)|(src[3]<<24);
			dst += sizeof(REBCNT);
			src += 4;
			break;
		}
	}
#endif
	*srcp = src;
}

void Unmap_Bytes(void *srcp, REBYTE **dstp, char *map) {
	REBYTE *src = srcp;
	REBYTE *dst = *dstp;
	char c;
#ifdef ENDIAN_LITTLE
	while ((c = *map++) != 0) {
		switch(c) {
		case 'b':
			*dst++ = *src++;
			break;

		case 's':
			*((short *)dst) = *((short *)src);
			src += sizeof(short);
			dst += 2;
			break;

		case 'l':
			if (longaligned()) {
				while(((unsigned long)src)&3)
					src++;
			}
			*((REBCNT *)dst) = *((REBCNT *)src);
			src += sizeof(REBCNT);
			dst += 4;
			break;
		}
	}
#else
	while ((c = *map++) != 0) {
		switch(c) {
		case 'b':
			*dst++ = *src++;
			break;

		case 's':
			*((short *)dst) = src[0]|(src[1]<<8);
			src += sizeof(short);
			dst += 2;
			break;

		case 'l':
			if (longaligned()) {
				while (((unsigned long)src)&3)
					src++;
			}
			*((REBCNT *)dst) = src[0]|(src[1]<<8)|
					(src[2]<<16)|(src[3]<<24);
			src += sizeof(REBCNT);
			dst += 4;
			break;
		}
	}
#endif
	*dstp = dst;
}


/***********************************************************************
**
*/	static void Decode_BMP_Image(REBCDI *codi)
/*
**		Input:  BMP encoded image (codi->data, len)
**		Output: Image bits (codi->bits, w, h)
**		Error:  Code in codi->error
**		Return: Success as TRUE or FALSE
**
***********************************************************************/
{
	REBINT				i, j, x, y, c;
	REBINT				colors, compression, bitcount;
	REBINT				w, h;
	BITMAPFILEHEADER	bmfh;
	BITMAPINFOHEADER	bmih;
	BITMAPCOREHEADER	bmch;
	REBYTE				*cp, *tp;
	REBCNT				*dp;
	RGBQUADPTR			color;
	RGBQUADPTR			ctab = 0;

	cp = codi->data;
	Map_Bytes(&bmfh, &cp, mapBITMAPFILEHEADER);
	if (bmfh.bfType[0] != 'B' || bmfh.bfType[1] != 'M') {
		codi->error = CODI_ERR_SIGNATURE;
		return;
	}
	if (codi->action == CODI_IDENTIFY) return; // no error means success

	tp = cp;
	Map_Bytes(&bmih, &cp, mapBITMAPINFOHEADER);
	if (bmih.biSize < sizeof(BITMAPINFOHEADER)) {
		cp = tp;
		Map_Bytes(&bmch, &cp, mapBITMAPCOREHEADER);

		w = bmch.bcWidth;
		h = bmch.bcHeight;
		compression = 0;
		bitcount = bmch.bcBitCount;

		if (bmch.bcBitCount < 24)
			colors = 1 << bmch.bcBitCount;
		else
			colors = 0;

		if (colors) {
			ctab = (RGBQUADPTR)Make_Mem(colors * sizeof(RGBQUAD));
			for (i = 0; i<colors; i++) {
				ctab[i].rgbBlue = *cp++;
				ctab[i].rgbGreen = *cp++;
				ctab[i].rgbRed = *cp++;
				ctab[i].rgbReserved = 0;
			}
		}
	}
	else {
		w = bmih.biWidth;
		h = bmih.biHeight;
		compression = bmih.biCompression;
		bitcount = bmih.biBitCount;

		if (bmih.biClrUsed == 0 && bmih.biBitCount < 24)
			colors = 1 << bmih.biBitCount;
		else
			colors = bmih.biClrUsed;

		if (colors) {
			ctab = (RGBQUADPTR)Make_Mem(colors * sizeof(RGBQUAD));
			memcpy(ctab, cp, colors * sizeof(RGBQUAD));
			cp += colors * sizeof(RGBQUAD);
		}
	}

	if (bmfh.bfOffBits != (DWORD)(cp - codi->data))
		cp = codi->data + bmfh.bfOffBits;

	codi->w = w;
	codi->h = h;
	codi->bits = Make_Mem(w * h * 4);

	dp = (REBCNT *) codi->bits;
	dp += w * h - w;

	for (y = 0; y<h; y++) {
		switch(compression) {
		case BI_RGB:
			switch(bitcount) {
			case 1:
				x = 0;
				for (i = 0; i<w; i++) {
					if (x == 0) {
						x = 0x80;
						c = *cp++ & 0xff;
					}
					color = &ctab[(c&x) != 0];
					*dp++ = ((int)color->rgbRed << 16) |
							((int)color->rgbGreen << 8) | color->rgbBlue;
					x >>= 1;
				}
				i = (w+7) / 8;
				break;

			case 4:
				for (i = 0; i<w; i++) {
					if ((i&1) == 0) {
						c = *cp++ & 0xff;
						x = c >> 4;
					}
					else
						x = c & 0xf;
					if (x > colors) {
						codi->error = CODI_ERR_BAD_TABLE;
						goto error;
					}
					color = &ctab[x];
					*dp++ = ((int)color->rgbRed << 16) |
							((int)color->rgbGreen << 8) | color->rgbBlue;
				}
				i = (w+1) / 2;
				break;

			case 8:
				for (i = 0; i<w; i++) {
					c = *cp++ & 0xff;
					if (c > colors) {
						codi->error = CODI_ERR_BAD_TABLE;
						goto error;
					}
					color = &ctab[c];
					*dp++ = ((int)color->rgbRed << 16) |
							((int)color->rgbGreen << 8) | color->rgbBlue;
				}
				break;

			case 24:
				for (i = 0; i<w; i++) {
#ifdef ENDIAN_BIG
					*dp++=cp[0]|(cp[1]<<8)|(cp[2]<<16);
#else
					*dp++ = (*(int *)cp) & 0xffffffL;
#endif
					cp += 3;
				}
				i = w * 3;
				break;

			default:
				codi->error = CODI_ERR_BIT_LEN;
				goto error;
			}
			while (i++ % 4)
				cp++;
			break;

		case BI_RLE4:
			i = 0;
			for (;;) {
				c = *cp++ & 0xff;

				if (c == 0) {
					c = *cp++ & 0xff;
					if (c == 0 || c == 1)
						break;
					if (c == 2) {
						codi->error = CODI_ERR_BAD_TABLE;
						goto error;
					}
					for (j = 0; j<c; j++) {
						if (i == w)
							goto error;
						if ((j&1) == 0) {
							x = *cp++ & 0xff;
							color = &ctab[x>>4];
						}
						else
							color = &ctab[x&0x0f];
						*dp++ = ((int)color->rgbRed << 16) |
							((int)color->rgbGreen << 8) | color->rgbBlue;
					}
					j = (c+1) / 2;
					while (j++%2)
						cp++;
				}
				else {
					x = *cp++ & 0xff;
					for (j = 0; j<c; j++) {
						if (i == w) {
							codi->error = CODI_ERR_BAD_TABLE;
							goto error;
						}
						if (j&1)
							color = &ctab[x&0x0f];
						else
							color = &ctab[x>>4];
						*dp++ = ((int)color->rgbRed << 16) |
							((int)color->rgbGreen << 8) | color->rgbBlue;
					}
				}
			}
			break;

		case BI_RLE8:
			i = 0;
			for (;;) {
				c = *cp++ & 0xff;

				if (c == 0) {
					c = *cp++ & 0xff;
					if (c == 0 || c == 1)
						break;
					if (c == 2) {
						codi->error = CODI_ERR_BAD_TABLE;
						goto error;
					}
					for (j = 0; j<c; j++) {
						x = *cp++ & 0xff;
						color = &ctab[x];
						*dp++ = ((int)color->rgbRed << 16) |
							((int)color->rgbGreen << 8) | color->rgbBlue;
					}
					while (j++ % 2)
						cp++;
				}
				else {
					x = *cp++ & 0xff;
					for (j = 0; j<c; j++) {
						color = &ctab[x];
						*dp++ = ((int)color->rgbRed << 16) |
							((int)color->rgbGreen << 8) | color->rgbBlue;
					}
				}
			}
			break;

		default:
			codi->error = CODI_ERR_ENCODING;
			goto error;
		}
		dp -= 2 * w;
	}
error:
	if (ctab) free(ctab);
}


/***********************************************************************
**
*/	static void Encode_BMP_Image(REBCDI *codi)
/*
**		Input:  Image bits (codi->bits, w, h)
**		Output: BMP encoded image (codi->data, len)
**		Error:  Code in codi->error
**		Return: Success as TRUE or FALSE
**
***********************************************************************/
{
	REBINT i, y;
	REBINT w, h;
	REBYTE *cp;
	REBCNT *dp, v;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;

	w = codi->w;
	h = codi->h;

	memset(&bmfh, 0, sizeof(bmfh));
	bmfh.bfType[0] = 'B';
	bmfh.bfType[1] = 'M';
	bmfh.bfSize = 14 + 40 + h * WADJUST(w);
	bmfh.bfOffBits = 14 + 40;

	// Create binary string:
	cp = codi->data = Make_Mem(bmfh.bfSize);
	codi->len = bmfh.bfSize;
	Unmap_Bytes(&bmfh, &cp, mapBITMAPFILEHEADER);

	memset(&bmih, 0, sizeof(bmih));
	bmih.biSize = 40;
	bmih.biWidth = w;
	bmih.biHeight = h;
	bmih.biPlanes = 1;
	bmih.biBitCount = 24;
	bmih.biCompression = 0;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;
	Unmap_Bytes(&bmih, &cp, mapBITMAPINFOHEADER);

	dp = (REBCNT *) codi->bits;
	dp += w * h - w;

	for (y = 0; y<h; y++) {
		for (i = 0; i<w; i++) {
			v = *dp++;
			cp[0] = v & 0xff;
			cp[1] = (v >> 8) & 0xff;
			cp[2] = (v >> 16) & 0xff;
			cp += 3;
		}
		i = w * 3;
		while (i++ % 4)
			*cp++ = 0;
		dp -= 2 * w;
	}
}


/***********************************************************************
**
*/	REBINT Codec_BMP_Image(REBCDI *codi)
/*
***********************************************************************/
{
	codi->error = 0;

	if (codi->action == CODI_IDENTIFY) {
		Decode_BMP_Image(codi);
		return CODI_CHECK; // error code is inverted result
	}

	if (codi->action == CODI_DECODE) {
		Decode_BMP_Image(codi);
		return CODI_IMAGE;
	}

	if (codi->action == CODI_ENCODE) {
		Encode_BMP_Image(codi);
		return CODI_BINARY;
	}

	codi->error = CODI_ERR_NA;
	return CODI_ERROR;
}


/***********************************************************************
**
*/	void Init_BMP_Codec(void)
/*
***********************************************************************/
{
	Register_Codec("bmp", Codec_BMP_Image);
}
