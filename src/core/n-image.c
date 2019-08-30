/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2019 Rebol Open Source Contributors
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
**  Module:  n-image.c
**  Summary: image related native functions
**  Section: natives
**  Author:  Oldes
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "reb-codec.h"


typedef struct REBCLR {
	union {
		unsigned char r;
		unsigned char h;
	};
	union {
		unsigned char g;
		unsigned char s;
	};
	union {
		unsigned char b;
		unsigned char v;
	};
	unsigned char a;
} REBCLR;

// Conversion using raw tuple bytes is (a little bit) faster but less precise
// using decimals in conversions is producing results closer to Rebol2
#define HSV_CONVERSION_USING_DECIMAL
// Currently HSW is using tuple datatype, which is limited to range 0-255
// That is also adding some precision lost for round-trip conversions.

// Code based on: https://stackoverflow.com/q/3018313/494472

/***********************************************************************
**
*/	REBNATIVE(hsv_to_rgb)
/*
//	hsv-to-rgb: native [
//		"Converts HSV (hue, saturation, value) to RGB"
//		hsv [tuple!]
//	]
***********************************************************************/
{
	REBCLR *val = (REBCLR*)VAL_TUPLE(D_ARG(1));
	if (val->s == 0) {
		// achromatic (grey)
		val->r = val->v;
		val->g = val->v;
		val->b = val->v;
		return R_ARG1;
	}
#ifdef HSV_CONVERSION_USING_DECIMAL
	
	REBDEC h = (float)val->h;
	REBDEC s = (float)val->s / 255.0;
	REBDEC v = (float)val->v / 255.0;

	int i;
	REBDEC f, p, q, t;

	h /= 42.5;    // sector 0 to 5; we are using 255 instead of 360 because the value is in tuple's byte ! (255 / 6)
	i = floor(h);
	f = h - i;    // factorial part of h
	p = v * (1 - s);
	q = v * (1 - s * f);
	t = v * (1 - s * (1 - f));
	switch (i) {
		case 0:  val->r = (REBYTE)(255.0 * v); val->g = (REBYTE)(255.0 * t); val->b = (REBYTE)(255.0 * p); break;
		case 1:  val->r = (REBYTE)(255.0 * q); val->g = (REBYTE)(255.0 * v); val->b = (REBYTE)(255.0 * p); break;
		case 2:  val->r = (REBYTE)(255.0 * p); val->g = (REBYTE)(255.0 * v); val->b = (REBYTE)(255.0 * t); break;
		case 3:  val->r = (REBYTE)(255.0 * p); val->g = (REBYTE)(255.0 * q); val->b = (REBYTE)(255.0 * v); break;
		case 4:  val->r = (REBYTE)(255.0 * t); val->g = (REBYTE)(255.0 * p); val->b = (REBYTE)(255.0 * v); break;
		default: val->r = (REBYTE)(255.0 * v); val->g = (REBYTE)(255.0 * p); val->b = (REBYTE)(255.0 * q); break; // case 5
	}
#else
	REBYTE region, remainder, p, q, t;
	
	region = val->h / 43; // six regions: 255 / 6
	remainder = (val->h - (region * 43)) * 6;

	p = (val->v * (255 - val->s)) >> 8;
	q = (val->v * (255 - ((val->s * remainder) >> 8))) >> 8;
	t = (val->v * (255 - ((val->s * (255 - remainder)) >> 8))) >> 8;

	switch (region)	{
		case 0:	val->r = val->v; val->g = t; val->b = p; break;
		case 1:	val->r = q; val->g = val->v; val->b = p; break;
		case 2:	val->r = p; val->g = val->v; val->b = t; break;
		case 3:	val->r = p; val->g = q; val->b = val->v; break;
		case 4:	val->r = t; val->g = p; val->b = val->v; break;
		default: // case 5
			val->r = val->v; val->g = p; val->b = q;
			break;
	}
#endif
    return R_ARG1;
}

/***********************************************************************
**
*/	REBNATIVE(rgb_to_hsv)
/*
//	rgb-to-hsv: native [
//		"Converts RGB value to HSV (hue, saturation, value)"
//		rgb [tuple!]
//	]
***********************************************************************/
{
	REBCLR *val = (REBCLR*)VAL_TUPLE(D_ARG(1));
	REBYTE rgbMin, rgbMax, r, g, b;

	r = val->r;
	g = val->g;
	b = val->b;

	rgbMax = r > g ? (r > b ? r : b) : (g > b ? g : b);
	rgbMin = r < g ? (r < b ? r : b) : (g < b ? g : b);

	val->v = rgbMax;
	if (val->v == 0 || rgbMax == rgbMin) {
		val->h = val->s = 0;
		return R_ARG1;
	}

#ifdef HSV_CONVERSION_USING_DECIMAL
	REBDEC delta = rgbMax - rgbMin;

	val->s = (REBYTE)(255.0 * delta / rgbMax);
	if (rgbMax == r)
		val->h = (REBYTE)(42.5 * (g - b) / delta);
	else if (rgbMax == g)
		val->h = (REBYTE)(85.0 + 42.5 * (b - r) / delta);
	else
		val->h = (REBYTE)(170.0 + 42.5 * (r - g) / delta);

#else
	REBINT delta = rgbMax - rgbMin;
	val->s = (REBYTE)(255 * delta / rgbMax);

	if (rgbMax == r)
		val->h = 0 + 43 * (g - b) / delta;
	else if (rgbMax == g)
		val->h = 85 + 43 * (b - r) / delta;
	else
		val->h = 171 + 43 * (r - g) / delta;
#endif

	return R_ARG1;
}

/***********************************************************************
**
*/	REBNATIVE(image)
/*
//  image: native [
//		"Interface to basic image encoding/decoding (only on Windows so far!)"
//		/load      "Image file to load or binary to decode"
//		 src-file  [file! binary!]
//		/save      "Encodes image to file or binary"
//		 dst-file  [none! file! binary!] "If NONE is used, binary is made internally"
//		 dst-image [none! image!]  "If NONE, loaded image may be used if there is any"
//		/frame     "Some image formats may contain multiple images"
//		 num       [integer!]  "1-based index of the image to receive"
//		/as        "Used to define which codec should be used"
//		 type      [word!] "One of: [PNG JPEG JPEGXR BMP DDS GIF TIFF] read only: [DNG ICO HEIF]"
//		/scale
//		 sc        [pair! percent!]
//  ]
***********************************************************************/
{
	REBOOL  ref_load     = D_REF(1);
	REBVAL *val_src_file = D_ARG(2);
	REBOOL  ref_save     = D_REF(3);
	REBVAL *val_dest     = D_ARG(4);
	REBVAL *val_dst_img  = D_ARG(5);
	REBOOL  ref_frame    = D_REF(6);
	REBVAL *val_frame    = D_ARG(7);
	REBOOL  ref_as       = D_REF(8);
	REBVAL *val_type     = D_ARG(9);
	REBOOL  ref_scale    = D_REF(10);
	REBVAL *val_scale    = D_ARG(11);

	REBCDI codi;
	REBSER *ser = NULL;
	REBCNT  frm = ref_frame ? VAL_INT32(val_frame) - 1 : 0;
	REBVAL *ret = D_RET;
	REBCNT length;


#if defined(TO_WINDOWS) && defined(USE_NATIVE_IMAGE_CODECS)
	CLEARS(&codi);
	if (ref_as) {
		switch (VAL_WORD_CANON(val_type)) {
			case SYM_PNG:   codi.type = CODI_IMG_PNG;  break;
			case SYM_JPEG:
			case SYM_JPG:   codi.type = CODI_IMG_JPEG; break;
			case SYM_JPEGXR:
			case SYM_HDP:
			case SYM_JXR:   codi.type = CODI_IMG_JXR;  break;
			case SYM_BMP:   codi.type = CODI_IMG_BMP;  break;
			case SYM_GIF:   codi.type = CODI_IMG_GIF;  break;
			case SYM_DDS:   codi.type = CODI_IMG_DDS;  break;
			case SYM_DNG:   codi.type = CODI_IMG_DNG;  break;
			case SYM_TIFF:  codi.type = CODI_IMG_TIFF; break;
			case SYM_HEIF:  codi.type = CODI_IMG_HEIF; break;
			case SYM_WEBP:  codi.type = CODI_IMG_WEBP; break;
			case SYM_ICO:   codi.type = CODI_IMG_ICO;  break;
			default:
				Trap1(RE_BAD_FUNC_ARG, val_type);
		}
	}
	if (ref_load) {
		
		if (IS_FILE(val_src_file)) {
			ser = Value_To_Local_Path(val_src_file, TRUE);
		} else {
			// raw binary data
			codi.data = VAL_BIN(val_src_file);
			codi.len  = VAL_LEN(val_src_file);
		}

		OS_Load_Image(ser ? SERIES_DATA(ser) : NULL, frm, &codi);

		if(codi.error) {
			switch (codi.error) {
			case WINCODEC_ERR_COMPONENTNOTFOUND:
				Trap1(RE_NO_CODEC, val_type);
				break;
			default:
				SET_INTEGER(D_RET, codi.error);
				if (IS_BINARY(val_src_file)) {
					Trap1(RE_NO_CODEC, D_RET);
				} else {
					Trap2(RE_CANNOT_OPEN, val_src_file, D_RET); // need better!!!
				}
			}
		} else {
			ser = Make_Image(codi.w, codi.h, TRUE);
			memcpy(IMG_DATA(ser), codi.data, codi.w * codi.h * 4);
			SET_IMAGE(D_RET, ser);
			free(codi.data);
			codi.data = NULL;
		}
	}
	if (ref_save) {
		codi.bits = VAL_IMAGE_BITS(val_dst_img);
		codi.w    = VAL_IMAGE_WIDE(val_dst_img);
		codi.h    = VAL_IMAGE_HIGH(val_dst_img);

		if (IS_FILE(val_dest)) {
			ser = Value_To_Local_Path(val_dest, TRUE);
		} else {
			// raw binary data...
			// ... predict number of bytes large enough to hold the result
			length = (codi.w * codi.h * 4) + 1024;
			if (IS_NONE(val_dest)) {
				ser = Make_Binary(length);
				SET_BINARY(val_dest, ser);
				codi.len = length;
			} else {
				ser = VAL_SERIES(val_dest);
				REBCNT avail = SERIES_REST(ser) - VAL_INDEX(val_dest);
				if (length > avail) {
					Extend_Series(ser, length - avail);
					avail = SERIES_REST(ser) - VAL_INDEX(val_dest);
				}
				codi.len = avail;
			}
			codi.data = VAL_BIN_AT(val_dest);
		}

		//if (ref_scale) {
		//	if (IS_PERCENT(val_scale)) {
		//		codi.w *= VAL_DECIMAL(val_scale);
		//		codi.h *= VAL_DECIMAL(val_scale);
		//	} else {
		//		codi.w = VAL_PAIR_X_INT(val_scale);
		//		codi.h = VAL_PAIR_Y_INT(val_scale);
		//	}
		//}
		
		OS_Save_Image(IS_FILE(val_dest) ? SERIES_DATA(ser) : NULL, &codi);

		if(codi.error) {
			switch (codi.error) {
			case WINCODEC_ERR_COMPONENTNOTFOUND:
				Trap1(RE_NO_CODEC, val_type);
				break;
			default:
				SET_INTEGER(D_RET, codi.error);
				if (IS_BINARY(val_dest)) {
					Trap1(RE_NO_CODEC, D_RET);
				} else {
					Trap2(RE_CANNOT_OPEN, val_dest, D_RET);  // need better!!!
				}
			}
		}

		if (IS_BINARY(val_dest)) {
			REBCNT tail = codi.len + VAL_INDEX(val_dest);
			if (tail > SERIES_TAIL(ser))
				SERIES_TAIL(ser) = tail;
		}
		*D_RET = *val_dest;
	}
#else
	Trap0(RE_FEATURE_NA);
#endif
	return R_RET;
}
