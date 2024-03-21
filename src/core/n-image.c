/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2021 Rebol Open Source Contributors
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

#ifdef INCLUDE_IMAGE_NATIVES

#include "reb-codec.h"
#include "sys-magick.h" // used for `resize` native
#include "sys-blur.h" // used for `blur` native
#if defined(TO_WINDOWS)
# ifndef WINCODEC_ERR_COMPONENTNOTFOUND
# define WINCODEC_ERR_COMPONENTNOTFOUND   0x88982F50L
# endif
#endif


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

double rgb_color_distance(long r1, long g1, long b1, long r2, long g2, long b2)
{
	long r = r1 - r2;
	long g = g1 - g2;
	long b = b1 - b2;

	return sqrt((r*r) + (g*g) + (b*b));
}
double weighted_rgb_color_distance(long r1, long g1, long b1, long r2, long g2, long b2)
{
	long r = r1 - r2;
	long g = g1 - g2;
	long b = b1 - b2;

	long rmean = (r1  + r2 ) / 2;
	return sqrt((((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8));
}
/***********************************************************************
**
*/	REBNATIVE(color_distance)
/*
**	It's by design providing only the weighted version! The simple distance
**  could be provided in more generic `distance` function.
**
**	https://www.compuphase.com/cmetric.htm
**  https://observablehq.com/@luciyer/euclidian-distance-in-rgb-color-space
**
//	color-distance: native [
//		"Human perception weighted Euclidean distance between two RGB colors"
//		a [tuple!]
//		b [tuple!]
//	]
***********************************************************************/
{
	REBCLR *val1 = (REBCLR*)VAL_TUPLE(D_ARG(1));
	REBCLR *val2 = (REBCLR*)VAL_TUPLE(D_ARG(2));

	SET_DECIMAL(D_RET, weighted_rgb_color_distance(
		(long)val1->r, (long)val1->g, (long)val1->b,
		(long)val2->r, (long)val2->g, (long)val2->b
	));
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(image_diff)
/*
**	Note that input image indexes are ignored!
**
//	image-diff: native [
//		"Count difference (using weighted RGB distance) of two images of the same size. Returns 0% if images are same and 100% if completely different."
//		a [image!]      "If sizes of the input images are not same..."
//		b [image!]      "... then only the smaller part is compared!"
//		/part           "Limit computation only to a part of the image"
//		 offset [pair!] "Zero based top-left corner"
//		 size   [pair!] "Size of the sub-image to use"
//	]
***********************************************************************/
{
	REBVAL *img1 = D_ARG(1);
	REBVAL *img2 = D_ARG(2);
	REBOOL  part = D_REF(3);

	REBYTE *rgba1 = VAL_IMAGE_HEAD(img1);
	REBYTE *rgba2 = VAL_IMAGE_HEAD(img2);
	REBINT ofsx, ofsy, wide, high, maxx, maxy;
	REBCNT pixels;
	REBDEC dist = 0;

	if ( part
	  || VAL_IMAGE_WIDE(img1) != VAL_IMAGE_WIDE(img2)
	  || VAL_IMAGE_HIGH(img1) != VAL_IMAGE_HIGH(img2)
	) {
		maxx = MAX(VAL_IMAGE_WIDE(img1), VAL_IMAGE_WIDE(img2));
		maxy = MAX(VAL_IMAGE_HIGH(img1), VAL_IMAGE_HIGH(img2));

		if (part) {
			ofsx = VAL_PAIR_X_INT(D_ARG(4));
			ofsy = VAL_PAIR_Y_INT(D_ARG(4));
			wide = VAL_PAIR_X_INT(D_ARG(5));
			high = VAL_PAIR_Y_INT(D_ARG(5));

			if (wide < 0) {ofsx += wide; wide = -wide;}
			if (high < 0) {ofsy += high; high = -high;}
			if (ofsx < 0) {wide += ofsx; ofsx = 0;}
			if (ofsy < 0) {high += ofsy; ofsy = 0;}

			if ((ofsx + wide) > maxx) wide = maxx - ofsx - wide;
			if ((ofsy + high) > maxy) high = maxy - ofsy - high;
			
			if (wide == 0 || high == 0)
				Trap1(RE_INVALID_DATA, D_ARG(5));
			if (ofsx >= maxx || ofsy >= maxy)
				Trap1(RE_INVALID_DATA, D_ARG(4));
		} else {
			// input images have different sizes, so compare just the smaller part
			ofsx = 0;
			ofsy = 0;
			wide = MIN(VAL_IMAGE_WIDE(img1), VAL_IMAGE_WIDE(img2));
			high = MIN(VAL_IMAGE_HIGH(img1), VAL_IMAGE_HIGH(img2));
		}

		pixels = wide * high;

		if (ofsy > 0) {
			rgba1 += 4*VAL_IMAGE_WIDE(img1)*ofsy;
			rgba2 += 4*VAL_IMAGE_WIDE(img2)*ofsy;
		}

		for(int row=0; row<high; row++) {
			rgba1 += 4*ofsx;
			rgba2 += 4*ofsx;
			for(int col=0; col<wide; col++) {
				dist += weighted_rgb_color_distance(
					rgba1[C_R], rgba1[C_G], rgba1[C_B],
					rgba2[C_R], rgba2[C_G], rgba2[C_B]
				);
				rgba1 += 4;
				rgba2 += 4;
			}
			// skip also ignored pixels on the right side of the region
			rgba1 += 4*(VAL_IMAGE_WIDE(img1)-ofsx-wide);
			rgba2 += 4*(VAL_IMAGE_WIDE(img2)-ofsx-wide);
		}

	} else {
		pixels = VAL_IMAGE_WIDE(img1) * VAL_IMAGE_HIGH(img1);

		for (REBCNT i = 0; i < pixels; i++, rgba1 += 4, rgba2 += 4) {
			dist += weighted_rgb_color_distance(
				rgba1[C_R], rgba1[C_G], rgba1[C_B],
				rgba2[C_R], rgba2[C_G], rgba2[C_B]
			);
		}
	}
	// used rounding to have nice 100% when completely different
	// 441.672955930064 max not weighted distance
	// 764.833315173967 max weighted distance
	dist = round(dist / pixels * 1000000000000);
	SET_PERCENT(D_RET, dist /  764833315173967);
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(tint)
/*
//	tint: native [
//		"Mixing colors (tint and or brightness)"
//		target [tuple! image!] "Target RGB color or image (modifed)"
//		rgb    [tuple!]        "Color to use for mixture"
//		amount [number!]       "Effect amount"
//	]
***********************************************************************/
{
	REBVAL *val_trg = D_ARG(1);
	REBCLR *rgb = (REBCLR*)VAL_TUPLE(D_ARG(2));
	REBVAL *val_amount = D_ARG(3);
	REBCLR *clr_trg;
	REBDEC r, g, b, r1, g1, b1, r2, g2, b2, amount0, amount1;

	r2 = rgb->r;
	g2 = rgb->g;
	b2 = rgb->b;

	amount0 = Clip_Dec(AS_DECIMAL(val_amount), 0.0, 1.0);
	amount1 = 1.0 - amount0;

	if (IS_TUPLE(val_trg)) {
		clr_trg = (REBCLR*)VAL_TUPLE(val_trg);
		r1 = clr_trg->r;
		g1 = clr_trg->g;
		b1 = clr_trg->b;
		r = (r1 >= r2) ? r2 + ((r1 - r2) * amount1) : r1 + ((r2 - r1) * amount0);
		g = (g1 >= g2) ? g2 + ((g1 - g2) * amount1) : g1 + ((g2 - g1) * amount0);
		b = (b1 >= b2) ? b2 + ((b1 - b2) * amount1) : b1 + ((b2 - b1) * amount0);
		clr_trg->r = (REBYTE)Clip_Int((int)(0.5 + r), 0, 255);
		clr_trg->g = (REBYTE)Clip_Int((int)(0.5 + g), 0, 255);
		clr_trg->b = (REBYTE)Clip_Int((int)(0.5 + b), 0, 255);
	} else {
		REBINT len = VAL_IMAGE_LEN(val_trg);
		REBYTE *rgba = VAL_IMAGE_DATA(val_trg);
		for (; len > 0; len--, rgba += 4) {
			r1 = rgba[C_R];
			g1 = rgba[C_G];
			b1 = rgba[C_B];
			r = (r1 >= r2) ? r2 + ((r1 - r2) * amount1) : r1 + ((r2 - r1) * amount0);
			g = (g1 >= g2) ? g2 + ((g1 - g2) * amount1) : g1 + ((g2 - g1) * amount0);
			b = (b1 >= b2) ? b2 + ((b1 - b2) * amount1) : b1 + ((b2 - b1) * amount0);
			rgba[C_R] = (REBYTE)Clip_Int((int)(0.5 + r), 0, 255);
			rgba[C_G] = (REBYTE)Clip_Int((int)(0.5 + g), 0, 255);
			rgba[C_B] = (REBYTE)Clip_Int((int)(0.5 + b), 0, 255);
		}
	}
	return R_ARG1;
}

/***********************************************************************
**
*/	REBNATIVE(resize)
/*
//	resize: native [
//		"Resizes an image to the given size."
//		 image    [image!]         "Image to resize"
//		 size     [pair! percent! integer!]
//		                           "Size of the new image (integer value is used as width)"
//		/filter                    "Using given filter type (default is Lanczos)"
//		 name     [word! integer!] "One of: system/catalog/filters"
//		/blur
//		 factor  [number!]   "The blur factor where > 1 is blurry, < 1 is sharp"
//	]
***********************************************************************/
{
	REBVAL *val_img    = D_ARG(1);
	REBVAL *val_size   = D_ARG(2);
//	REBOOL  ref_filter = D_REF(3);
	REBVAL *val_filter = D_ARG(4);
	REBOOL  ref_blur   = D_REF(5);
	REBVAL *val_blur   = D_ARG(6);
	REBSER *result;
	REBCNT  filter = 0;
	REBDEC  blur = 1.0;
	REBOOL  has_alpha = Image_Has_Alpha(val_img, FALSE);
	REBINT  wide = 0, high = 0;
	
	if (IS_INTEGER(val_filter))
		filter = VAL_INT32(val_filter);
	else if (IS_WORD(val_filter)) {
		filter = VAL_WORD_CANON(val_filter) - SYM_POINT + 1;
	}

	if (filter < 0 || filter > 15)
		Trap1(RE_INVALID_ARG, val_filter);

	if (ref_blur) {
		blur = AS_DECIMAL(val_blur);
		blur = MIN(blur, MAX_RESIZE_BLUR);
	}

	if (IS_PAIR(val_size)) {
		wide = VAL_PAIR_X_INT(val_size);
		high = VAL_PAIR_Y_INT(val_size);
	}
	else if (IS_INTEGER(val_size)) {
		wide = VAL_INT32(val_size);
		high = 0; // will be computed later
	}
	else if (IS_PERCENT(val_size)) {
		wide = ROUND_TO_INT(VAL_DECIMAL(val_size) * VAL_IMAGE_WIDE(val_img));
		high = ROUND_TO_INT(VAL_DECIMAL(val_size) * VAL_IMAGE_HIGH(val_img));
	}
	if (wide < 0 || high < 0 || (wide == 0 && high == 0)) 
		Trap1(RE_INVALID_ARG, val_size);

	if (wide == 0) {
		wide = ROUND_TO_INT( VAL_IMAGE_WIDE(val_img) * high / VAL_IMAGE_HIGH(val_img) );
	}
	if (high == 0) {
		high = ROUND_TO_INT( VAL_IMAGE_HIGH(val_img) * wide / VAL_IMAGE_WIDE(val_img) );
	}
		
	result = ResizeImage(VAL_SERIES(val_img), wide, high, filter, blur, has_alpha);

	if (result == NULL) {
		Trap1(RE_NO_CREATE, Get_Type_Word(REB_IMAGE));
	}

	SET_IMAGE(D_RET, result);
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(premultiply)
/*
//	premultiply: native [
//		"Premultiplies RGB channel with its alpha channel"
//		 image    [image!]         "Image to premultiply (modified)"
//	]
***********************************************************************/
{
	// All pixels are modified even when the input image is not at its head!
	REBVAL *val_img = D_ARG(1);
	REBINT len      = VAL_IMAGE_WIDE(val_img) * VAL_IMAGE_HIGH(val_img);
	REBYTE *rgba    = VAL_IMAGE_HEAD(val_img);
	REBINT a;
	// Note: could be optimized!
	for (; len > 0; len--, rgba += 4) {
		a = (REBINT)rgba[C_A];
		if (a == 0xFF) continue;
		rgba[C_R] = (REBYTE)(((REBINT)rgba[C_R] * a) / 255);
		rgba[C_G] = (REBYTE)(((REBINT)rgba[C_G] * a) / 255);
		rgba[C_B] = (REBYTE)(((REBINT)rgba[C_B] * a) / 255);
	}
	return R_ARG1;
}

/***********************************************************************
**
*/	REBNATIVE(blur)
/*
//	blur: native [
//		"Blur (Gaussian) given image"
//		 image    [image!]  "Image to blur (modified)"
//		 radius   [number!] "Blur amount"
//	]
***********************************************************************/
{
	REBVAL* val_img = D_ARG(1);
	REBVAL* val_rad = D_ARG(2);
	REBINT radius = IS_INTEGER(val_rad) ? VAL_INT32(val_rad) : (REBINT)round(VAL_DECIMAL(val_rad));
	if (radius > 0) BlurImage(VAL_SERIES(val_img), (REBCNT)radius);
	return R_ARG1;
}

/***********************************************************************
**
*/	REBNATIVE(image)
/*
//  image: native [
//		"Interface to basic image encoding/decoding (only on Windows and macOS so far!)"
//		/load      "Image file to load or binary to decode"
//		 src-file  [file! binary!]
//		/save      "Encodes image to file or binary"
//		 dst-file  [none! file! binary!] "If NONE is used, binary is made internally"
//		 dst-image [none! image!]  "If NONE, loaded image may be used if there is any"
//		/frame     "Some image formats may contain multiple images"
//		 num       [integer!]  "1-based index of the image to receive"
//		/as        "Used to define which codec should be used"
//		 type      [word!] "One of: [PNG JPEG JPEGXR BMP DDS GIF TIFF] read only: [DNG ICO HEIF]"
//;		/scale
//;		 sc        [pair! percent!]
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
	//REBOOL  ref_scale    = D_REF(10);
	//REBVAL *val_scale    = D_ARG(11);

	REBCDI codi;
	REBSER *ser = NULL;
	REBCNT  frm = ref_frame ? VAL_INT32(val_frame) - 1 : 0;
	REBCNT length;


#ifdef INCLUDE_IMAGE_OS_CODEC
	CLEARS(&codi);
	if (ref_as) {
		switch (VAL_WORD_CANON(val_type)) {
			case SYM_PNG:   codi.type = CODI_IMG_PNG;  break;
			case SYM_JPEG:
			case SYM_JPG:   codi.type = CODI_IMG_JPEG; break;
			case SYM_JPEGXR:
			case SYM_HDP:
			case SYM_JXR:   codi.type = CODI_IMG_JXR;  break;
			case SYM_JP2:   codi.type = CODI_IMG_JP2;  break;
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
#ifdef TO_WINDOWS
			ser = Value_To_Local_Path(val_src_file, TRUE);
#else
			ser = Value_To_OS_Path(val_src_file, TRUE);
#endif
		} else {
			// raw binary data
			codi.data = VAL_BIN(val_src_file);
			codi.len  = VAL_LEN(val_src_file);
		}
		
		OS_LOAD_IMAGE(ser ? (REBCHR*)SERIES_DATA(ser) : NULL, frm, &codi);

		if(codi.error) {
			switch (codi.error) {
#ifdef TO_WINDOWS
			case WINCODEC_ERR_COMPONENTNOTFOUND:
				Trap1(RE_NO_CODEC, val_type);
				break;
#endif
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
#ifdef TO_WINDOWS
			ser = Value_To_Local_Path(val_dest, TRUE);
#else
			ser = Value_To_OS_Path(val_dest, TRUE);
#endif
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
		
		OS_SAVE_IMAGE(IS_FILE(val_dest) ? (REBCHR *)SERIES_DATA(ser) : NULL, &codi);

		if(codi.error) {
			switch (codi.error) {
#ifdef TO_WINDOWS
			case WINCODEC_ERR_COMPONENTNOTFOUND:
				Trap1(RE_NO_CODEC, val_type);
				break;
#endif
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

#endif // INCLUDE_IMAGE_NATIVES
