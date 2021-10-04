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
**  Module:  u-png-filter.c
**  Summary: prepare data for optimum compression
**  Section: utility
**  Author:  Oldes
**  Notes:
**      It's used as a part of PNG compression, but also in PDF files.
**      https://www.rfc-editor.org/rfc/rfc2083.html#page-31
**      https://en.wikipedia.org/wiki/Portable_Network_Graphics#Filtering
***********************************************************************/

#include "sys-core.h"

#ifdef INCLUDE_PNG_CODEC
int paeth_predictor(int a, int b, int c);
#else
#define int_abs(a) (((a)<0)?(-(a)):(a))
static int paeth_predictor(int a, int b, int c) {
	int p, pa, pb, pc;

	p = a + b - c;
	pa = int_abs(p - a);
	pb = int_abs(p - b);
	pc = int_abs(p - c);
	if ((pa <= pb) && (pa <= pc))
		return a;
	else if (pb <= pc)
		return b;
	return c;
}
#endif

enum PNG_Filter_Types {
	PNG_FILTER_SUB = 1,
	PNG_FILTER_UP,
	PNG_FILTER_AVERAGE,
	PNG_FILTER_PAETH
};

static REBYTE get_png_filter_type(REBVAL* val) {
	if (IS_WORD(val)) {
		switch (VAL_WORD_SYM(val)) {
		case SYM_SUB:     return PNG_FILTER_SUB;
		case SYM_UP:      return PNG_FILTER_UP;
		case SYM_AVERAGE: return PNG_FILTER_AVERAGE;
		case SYM_PAETH:   return PNG_FILTER_PAETH;
		}
	}
	else if (IS_INTEGER(val)) {
		return MIN(PNG_FILTER_PAETH, MAX(0, VAL_INT32(val)));
	}
	Trap1(RE_INVALID_ARG, val);
	return 0; // to make xcode happy
}

/***********************************************************************
**
*/	REBNATIVE(filter)
/*
//	filter: native [
//		"PNG delta filter"
//		data   [binary!] "Input"
//		width  [number!] "Scanline width"
//		type   [integer! word!] "1..4 or one of: [sub up average paeth]"
//		/skip bpp [integer!] "Bytes per pixel"
//	]
***********************************************************************/
{
	REBVAL *val_data  = D_ARG(1);
	REBVAL *val_width = D_ARG(2);
	REBVAL *val_type  = D_ARG(3);
	REBOOL  ref_skip  = D_REF(4);
	REBVAL *val_bpp   = D_ARG(5);

	REBSER *ser;
	REBYTE *bin = VAL_BIN_DATA(val_data);
	REBYTE *scan, *prev, *temp, *out;
	REBCNT r, c, rows, bytes;
	REBINT width = AS_INT32(val_width);
	REBYTE filter = get_png_filter_type(val_type);
	REBCNT bpp = ref_skip ? VAL_INT32(val_bpp) : 1;

	bytes = VAL_LEN(val_data);

	if (width <= 1 || width > bytes)
		Trap1(RE_INVALID_ARG, val_width);
	if (bpp < 1 || bpp > width)
		Trap1(RE_INVALID_ARG, val_bpp);

	rows = bytes / width;
	ser  = Make_Binary(bytes);
	out  = BIN_DATA(ser);

	temp = malloc(width);
	if (!temp) {
		Trap0(RE_NO_MEMORY);
		return R_NONE;
	}
	memset(temp, 0, width);

	prev = temp;
	for (r = 0; r < rows; r++) {
		scan = bin + (r * width);
		out  = BIN_SKIP(ser, r * width);

		switch (filter) {
		case PNG_FILTER_SUB:
			for (c = 0; c < bpp; c++)
				out[c] = scan[c];
			for (c = bpp; c < width; c++)
				out[c] = scan[c] - scan[c - bpp];
			break;
		case PNG_FILTER_UP:
			for (c = 0; c < width; c++)
				out[c] = scan[c] - prev[c];
			break;
		case PNG_FILTER_AVERAGE:
			for (c = 0; c < bpp; c++)
				out[c] = scan[c] - (prev[c] >> 1);
			for (c = bpp; c < width; c++)
				out[c] = scan[c] - ((scan[c - bpp] + prev[c]) >> 1) & 0xFF;
			break;
		case PNG_FILTER_PAETH:
			for (c = 0; c < bpp; c++)
				out[c] = scan[c] - prev[c];
			for (c = bpp; c < width; c++)
				out[c] = scan[c] - paeth_predictor(scan[c - bpp], prev[c], prev[c - bpp]);
			break;
		}
		prev = scan;
	}
	free(temp);
	SET_BINARY(D_RET, ser);
	VAL_TAIL(D_RET) = bytes;
	return R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(unfilter)
/*
//	unfilter: native [
//		"Reversed PNG delta filter"
//		data  [binary!]  "Input"
//		width [number!]  "Scanline width (not counting the type byte)"
//		/as   "Filter type. If not used, type is decoded from first byte on each line."
//		 type [integer! word!] "1..4 or one of: [sub up average paeth]"
//		/skip
//		 bpp [integer!]  "Bytes per pixel"
//	]
***********************************************************************/
{
	REBVAL *val_data  = D_ARG(1);
	REBVAL *val_width = D_ARG(2);
	REBOOL  ref_as    = D_REF(3);
	REBVAL *val_type  = D_ARG(4);
	REBOOL  ref_skip  = D_REF(5);
	REBVAL *val_bpp   = D_ARG(6);

	REBSER *ser;
	REBYTE *bin = VAL_BIN_DATA(val_data);
	REBINT width = AS_INT32(val_width);
	REBCNT r, c, rows;
	REBYTE *scan, *prev, *temp, *out;
	REBYTE filter = 0;
	REBCNT bytes = VAL_LEN(val_data);
	REBCNT bpp = ref_skip ? VAL_INT32(val_bpp) : 1;

	if (!ref_as) width++;
	if (width <= 1 || width > bytes)
		Trap1(RE_INVALID_ARG, val_width);
	if (bpp < 1 || bpp > width)
		Trap1(RE_INVALID_ARG, val_bpp);

	rows = ceil(bytes / width);
	ser  = Make_Binary(bytes);
	out  = BIN_DATA(ser);

	if (ref_as)
		filter = get_png_filter_type(val_type);

	temp = malloc(width);
	if (!temp) {
		Trap0(RE_NO_MEMORY);
		return R_NONE;
	}
	memset(temp, 0, width);

	if (!ref_as) width--;

	prev = temp;
	for (r = 0; r < rows; r++) {
		if (ref_as) {
			scan = bin + r * width;
			out = BIN_SKIP(ser, r * width);
		} else {
			scan = bin + r * (width + 1);
			out = BIN_SKIP(ser, r * (width));
			filter = scan[0];
			scan++;
		}
		
		switch (filter) {
		case PNG_FILTER_SUB:
			for (c = 0; c < bpp; c++)
				out[c] = scan[c];
			for (c = bpp; c < width; c++)
				out[c] = scan[c] + out[c - bpp];
			break;
		case PNG_FILTER_UP:
			for (c = 0; c < width; c++)
				out[c] = scan[c] + prev[c];
			break;
		case PNG_FILTER_AVERAGE:
			for (c = 0; c < bpp; c++)
				out[c] = scan[c] + (prev[c] >> 1) & 0xFF;
			for (c = bpp; c < width; c++)
				out[c] = scan[c] + ((out[c - bpp] + prev[c]) >> 1) & 0xFF;
			break;
		case PNG_FILTER_PAETH:
			for (c = 0; c < bpp; c++)
				out[c] = scan[c] + prev[c];
			for (c = bpp; c < width; c++)
				out[c] = scan[c] + paeth_predictor(out[c - bpp], prev[c], prev[c - bpp]);
			break;
		}
		prev = out;
	}
	free(temp);
	SET_BINARY(D_RET, ser);
	VAL_TAIL(D_RET) = ref_as ? bytes : bytes - rows;
	return R_RET;
}
