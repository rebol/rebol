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
