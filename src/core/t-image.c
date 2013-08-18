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
**  Module:  t-image.c
**  Summary: image datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/
/*
**  It's a bit of a shame that alpha channels are represented with
**  an inverted level compared to many standards. Alpha zero must
**  be opaque in order for RGB tuples to be equal RGBA tuples.
**  That is: 10.20.30 = 10.20.30.0
*/

#include "sys-core.h"

#define CLEAR_IMAGE(p, x, y) memset(p, 0, x * y * sizeof(long))


/***********************************************************************
**
*/	REBINT CT_Image(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	REBINT e;

	if (mode < 0) return -1;
	if (mode == 3) return VAL_SERIES(a) == VAL_SERIES(b) && VAL_INDEX(a) == VAL_INDEX(b);
	e = VAL_IMAGE_WIDE(a) == VAL_IMAGE_WIDE(a) && VAL_IMAGE_HIGH(b) == VAL_IMAGE_HIGH(b);
	if (e) e = (0 == Cmp_Value(a, b, mode > 1));
	return e;
}


/***********************************************************************
**
*/	REBFLG MT_Image(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	if (!Create_Image(data, out, 1)) return FALSE;
	VAL_SET(out, REB_IMAGE);
	return TRUE;
}


/***********************************************************************
**
*/	void Reset_Height(REBVAL *value)
/*
**		Set height based on tail and width.
**
***********************************************************************/
{
	REBCNT w = VAL_IMAGE_WIDE(value);
	VAL_IMAGE_HIGH(value) = w ? (VAL_TAIL(value) / w) : 0;
}


/***********************************************************************
**
*/	void Set_Pixel_Tuple(REBYTE *dp, REBVAL *tuple)
/*
***********************************************************************/
{
	// Tuple to pixel.
	REBYTE *tup = VAL_TUPLE(tuple);

	dp[C_R] = tup[0];
	dp[C_G] = tup[1];
	dp[C_B] = tup[2];
	if (VAL_TUPLE_LEN(tuple) > 3) dp[C_A] = tup[3];
}


/***********************************************************************
**
*/	void Set_Tuple_Pixel(REBYTE *dp, REBVAL *tuple)
/*
***********************************************************************/
{
	// Pixel to tuple.
	REBYTE *tup = VAL_TUPLE(tuple);

	VAL_SET(tuple, REB_TUPLE);
	VAL_TUPLE_LEN(tuple) = 4;
	tup[0] = dp[C_R];
	tup[1] = dp[C_G];
	tup[2] = dp[C_B];
	tup[3] = dp[C_A];
}


/***********************************************************************
**
*/	void Fill_Line(REBCNT *ip, REBCNT color, REBCNT len, REBOOL only)
/*
***********************************************************************/
{
	if (only) // only RGB, do not touch Alpha
		for (; len > 0; len--, ip++) *ip = (*ip & 0xff000000) | color;
	else
		for (; len > 0; len--) *ip++ = color;
}


/***********************************************************************
**
*/	void Fill_Rect(REBCNT *ip, REBCNT color, REBCNT w, REBINT dupx, REBINT dupy, REBOOL only)
/*
***********************************************************************/
{
	for (; dupy > 0; dupy--, ip += w)
		Fill_Line(ip, color, dupx, only);
}


/***********************************************************************
**
*/	void Fill_Alpha_Line(REBYTE *rgba, REBYTE alpha, REBINT len)
/*
***********************************************************************/
{
	for (; len > 0; len--, rgba += 4)
		rgba[C_A] = alpha;
}


/***********************************************************************
**
*/	void Fill_Alpha_Rect(REBCNT *ip, REBYTE alpha, REBINT w, REBINT dupx, REBINT dupy)
/*
***********************************************************************/
{
	for (; dupy > 0; dupy--, ip += w)
		Fill_Alpha_Line((REBYTE *)ip, alpha, dupx);
}


/***********************************************************************
**
*/	REBCNT *Find_Color(REBCNT *ip, REBCNT color, REBCNT len, REBOOL only)
/*
***********************************************************************/
{
	if (only) { // only RGB, do not touch Alpha
		for (; len > 0; len--, ip++)
			if (color == (*ip & 0x00ffffff)) return ip;
	} else {
		for (; len > 0; len--, ip++)
			if (color == *ip) return ip;
	}
	return 0;
}


/***********************************************************************
**
*/	REBCNT *Find_Alpha(REBCNT *ip, REBCNT alpha, REBCNT len)
/*
***********************************************************************/
{
	for (; len > 0; len--, ip++) {
		if (alpha == (*ip >> 24)) return ip;
	}
	return 0;
}


/***********************************************************************
**
*/	void RGB_To_Bin(REBYTE *bin, REBYTE *rgba, REBINT len, REBOOL alpha)
/*
***********************************************************************/
{
	// Convert internal image (integer) to RGB/A order binary string:
	if (alpha) {
		for (; len > 0; len--, rgba += 4, bin += 4) {
			bin[0] = rgba[C_R];
			bin[1] = rgba[C_G];
			bin[2] = rgba[C_B];
			bin[3] = rgba[C_A];
		}
	} else {
		// Only the RGB part:
		for (; len > 0; len--, rgba += 4, bin += 3) {
			bin[0] = rgba[C_R];
			bin[1] = rgba[C_G];
			bin[2] = rgba[C_B];
		}
	}
}


/***********************************************************************
**
*/	void Bin_To_RGB(REBYTE *rgba, REBCNT size, REBYTE *bin, REBCNT len)
/*
***********************************************************************/
{
	if (len > size) len = size; // avoid over-run

	// Convert RGB binary string to internal image (integer), no alpha:
	for (; len > 0; len--, rgba += 4, bin += 3) {
		rgba[C_R] = bin[0];
		rgba[C_G] = bin[1];
		rgba[C_B] = bin[2];
	}
}


/***********************************************************************
**
*/	void Bin_To_RGBA(REBYTE *rgba, REBCNT size, REBYTE *bin, REBINT len, REBOOL only)
/*
***********************************************************************/
{
	if (len > (REBINT)size) len = size; // avoid over-run

	// Convert from BGRA format to internal image (integer):
	for (; len > 0; len--, rgba += 4, bin += 4) {
		rgba[C_B] = bin[0];
		rgba[C_G] = bin[1];
		rgba[C_R] = bin[2];
		if (!only) rgba[C_A] = bin[3];
	}
}


/***********************************************************************
**
*/	void Alpha_To_Bin(REBYTE *bin, REBYTE *rgba, REBINT len)
/*
***********************************************************************/
{
	for (; len > 0; len--, rgba += 4)
		*bin++ = rgba[C_A];
}


/***********************************************************************
**
*/	void Bin_To_Alpha(REBYTE *rgba, REBCNT size, REBYTE *bin, REBINT len)
/*
***********************************************************************/
{
	if (len > (REBINT)size) len = size; // avoid over-run

	for (; len > 0; len--, rgba += 4)
		rgba[C_A] = *bin++;
}


/***********************************************************************
**
*/	REBFLG Valid_Tuples(REBVAL *blk)
/*
***********************************************************************/
{
	REBCNT n = VAL_INDEX(blk);
	REBCNT len = VAL_LEN(blk);

	blk = VAL_BLK_DATA(blk);

	for (; n < len; n++)
		if (!IS_TUPLE(blk+n)) return n+1;

	return 0;
}


/***********************************************************************
**
*/	void Tuples_To_RGBA(REBYTE *rgba, REBCNT size, REBVAL *blk, REBCNT len)
/*
***********************************************************************/
{
	REBYTE *bin;

	if (len > size) len = size; // avoid over-run

	for (; len > 0; len--, rgba += 4, blk++) {
		bin = VAL_TUPLE(blk);
		rgba[C_R] = bin[0];
		rgba[C_G] = bin[1];
		rgba[C_B] = bin[2];
		rgba[C_A] = bin[3];
	}
}


/***********************************************************************
**
*/	void Image_To_BGRA(REBYTE *rgba, REBYTE *bin, REBINT len)
/*
***********************************************************************/
{
	// Convert from BGRA format to internal image (integer):
	for (; len > 0; len--, rgba += 4, bin += 4) {
		bin[0] = rgba[C_B];
		bin[1] = rgba[C_G];
		bin[2] = rgba[C_R];
		bin[3] = rgba[C_A];
	}
}

#ifdef ndef
INLINE REBCNT ARGB_To_BGR(REBCNT i)
{
	return
		((i & 0x00ff0000) >> 16) | // red
		((i & 0x0000ff00)) |       // green
		((i & 0x000000ff) << 16);  // blue
}
#endif

/***********************************************************************
**
*/	void Mold_Image_Data(REBVAL *value, REB_MOLD *mold)
/*
***********************************************************************/
{
	REBUNI *up;
	REBCNT len;
	REBCNT size;
	REBCNT *data;

	Emit(mold, "IxI #{", VAL_IMAGE_WIDE(value), VAL_IMAGE_HIGH(value));

	// Output RGB image:
	size = VAL_IMAGE_LEN(value); // # pixels (from index to tail)
	data = (REBCNT *)VAL_IMAGE_DATA(value);
	up = Prep_Uni_Series(mold, (size * 6) + (size / 10) + 1);

	for (len = 0; len < size; len++) {
		if ((len % 10) == 0) *up++ = LF;
		up = Form_RGB_Uni(up, *data++);
	}

	// Output Alpha channel, if it has one:
	if (Image_Has_Alpha(value, FALSE)) {

		Append_Bytes(mold->series, "\n} #{");

		up = Prep_Uni_Series(mold, (size * 2) + (size / 10) + 1);

		data = (REBCNT *)VAL_IMAGE_DATA(value);
		for (len = 0; len < size; len++) {
			if ((len % 10) == 0) *up++ = LF;
			up = Form_Hex2_Uni(up, *data++ >> 24);
		}
	}
	*up = 0; // tail already set from Prep.

	Append_Bytes(mold->series, "\n}");
}


/***********************************************************************
**
*/	REBSER *Make_Image_Binary(REBVAL *image)
/*
***********************************************************************/
{
	REBSER *ser;

#ifdef XENDIAN_BIG
	ser = Make_Quad(0, VAL_IMAGE_LEN(image));
	ser->tail = VAL_IMAGE_LEN(image) * 4;
	Image_To_BGRA(VAL_IMAGE_DATA(image), QUAD_HEAD(ser), VAL_IMAGE_LEN(image));
#else
	ser = Copy_Bytes(VAL_IMAGE_DATA(image), VAL_IMAGE_LEN(image)*4);
#endif
	return ser;
}


/***********************************************************************
**
*/	REBSER *Make_Image(REBCNT w, REBCNT h, REBFLG error)
/*
**		Allocate and initialize an image.
**		If error is TRUE, throw error on bad size.
**		Return zero on oversized image.
**
***********************************************************************/
{
	REBSER *img;

	if (w > 0xFFFF || h > 0xFFFF) {
		if (error) Trap1(RE_SIZE_LIMIT, Get_Type(REB_IMAGE));
		else return 0;
	}
	
	img = Make_Series(w * h + 1, sizeof(REBINT), FALSE);
	LABEL_SERIES(img, "make image");
	img->tail = w * h;
	CLEAR(img->data, (img->tail + 1) * sizeof(REBINT));
	IMG_WIDE(img) = w;
	IMG_HIGH(img) = h;
	return img;
}


/***********************************************************************
**
*/	void Clear_Image(REBVAL *img)
/*
**	Clear image data.
**
***********************************************************************/
{
	REBCNT w = VAL_IMAGE_WIDE(img);
	REBCNT h = VAL_IMAGE_HIGH(img);
	REBYTE *p = VAL_IMAGE_HEAD(img);
	CLEAR_IMAGE(p, w, h);
}


/***********************************************************************
**
*/	REBVAL *Create_Image(REBVAL *block, REBVAL *val, REBCNT modes)
/*
**	Create an image value from components block [pair rgb alpha].
**
***********************************************************************/
{
	REBINT w, h;
	REBYTE *ip; // image pointer
	REBCNT size;
	REBSER *img;

	// Check that PAIR is valid:
	if (!IS_PAIR(block)) return 0;
	w = VAL_PAIR_X_INT(block);
	h = VAL_PAIR_Y_INT(block);
	if (w < 0 || h < 0) return 0;

	img = Make_Image(w, h, FALSE);
	if (img == 0) return 0;
	SET_IMAGE(val, img);

	ip = IMG_DATA(img);
	size = w * h;

	//len = VAL_BLK_LEN(arg);
	block++;
	if (IS_BINARY(block)) {

		// Load image data:
		Bin_To_RGB(ip, size, VAL_BIN_DATA(block), VAL_LEN(block) / 3);
		block++;

		// Load alpha channel data:
		if (IS_BINARY(block)) {
			Bin_To_Alpha(ip, size, VAL_BIN_DATA(block), VAL_LEN(block));
//			VAL_IMAGE_TRANSP(value)=VITT_ALPHA;
			block++;
		}

		if (IS_INTEGER(block)) {
			VAL_INDEX(val) = (Int32s(block, 1) - 1);
			block++;
		}
	}
	else if (IS_TUPLE(block)) {
		Fill_Rect((REBCNT *)ip, TO_COLOR_TUPLE(block), w, w, h, TRUE);
		block++;
		if (IS_INTEGER(block)) {
			Fill_Alpha_Rect((REBCNT *)ip, (REBYTE)VAL_INT32(block), w, w, h);
//			VAL_IMAGE_TRANSP(value)=VITT_ALPHA;
			block++;
		}
	}
	else if (IS_BLOCK(block)) {
		if (w = Valid_Tuples(block)) Trap_Arg(block+w-1);
		Tuples_To_RGBA(ip, size, VAL_BLK_DATA(block), VAL_LEN(block));
	}
	else if (!IS_END(block)) return 0;

	//if (!IS_END(block)) Trap_Arg(block);

	return val;
}


/***********************************************************************
**
*/  REBVAL *Modify_Image(REBVAL *ds, REBCNT action)
/*
**		Insert or change image
**		ACTION value arg /part len /only /dup count
**
***********************************************************************/
{
	REBVAL	*value = D_ARG(1);
	REBVAL	*arg   = D_ARG(2);
	REBVAL	*len   = D_ARG(4);
	REBVAL	*count = D_ARG(7);
	REBINT  part = 1; // /part len
	REBINT	partx, party;
	REBINT	dup = 1;  // /dup count
	REBINT  dupx, dupy;
	REBOOL  only = 0; // /only
	REBCNT  index = VAL_INDEX(value);
	REBCNT  tail = VAL_TAIL(value);
	REBINT  n;
	REBINT  x;
	REBINT  w;
	REBINT  y;
	REBYTE  *ip;

	if (!(w = VAL_IMAGE_WIDE(value))) return value;

	if (action == A_APPEND) {
		index = tail;
		action = A_INSERT;
	}

	x = index % w; // offset on the line
	y = index / w; // offset line

	if (D_REF(5)) only = 1;

	// Validate that block arg is all tuple values:
	if (IS_BLOCK(arg) && NZ(n = Valid_Tuples(arg))) {
		Trap_Arg(VAL_BLK_SKIP(arg, n-1));
	}

	// Get the /dup refinement. It specifies fill size.
	if (D_REF(6)) {
		if (IS_INTEGER(count)) {
			dup = VAL_INT32(count);
			dup = MAX(dup, 0);
			if (dup == 0) return value;
		} else if (IS_PAIR(count)) { // rectangular dup
			dupx = VAL_PAIR_X_INT(count);
			dupy = VAL_PAIR_Y_INT(count);
			dupx = MAX(dupx, 0);
			dupx = MIN(dupx, (REBINT)w - x); // clip dup width
			dupy = MAX(dupy, 0);
			if (action != A_INSERT)
				dupy = MIN(dupy, (REBINT)VAL_IMAGE_HIGH(value) - y);
			else
				dup = dupy * w;
			if (dupx == 0 || dupy == 0) return value;
		} else
			Trap_Type(count);
	}

	// Get the /part refinement. Only allowed when arg is a series.
	if (D_REF(3)) {
		if (IS_BINARY(arg)) {
			if (IS_INTEGER(len)) {
				part = VAL_INT32(len);
			} else if (IS_BINARY(len)) {
				part = (VAL_INDEX(len) - VAL_INDEX(arg)) / 4;
			} else
				Trap_Arg(len);
			part = MAX(part, 0);
		} else if (IS_IMAGE(arg)) {
			if (IS_INTEGER(len)) {
				part = VAL_INT32(len);
				part = MAX(part, 0);
			} else if (IS_IMAGE(len)) {
				if (!VAL_IMAGE_WIDE(len)) Trap_Arg(len);
				partx = VAL_INDEX(len) - VAL_INDEX(arg);
				party = partx / VAL_IMAGE_WIDE(len);
				party = MAX(party, 1);
				partx = MIN(partx, (REBINT)VAL_IMAGE_WIDE(arg));
				goto len_compute;
			} else if (IS_PAIR(len)) {
				partx = VAL_PAIR_X_INT(len);
				party = VAL_PAIR_Y_INT(len);
			len_compute:
				partx = MAX(partx, 0);
				partx = MIN(partx, (REBINT)w - x); // clip part width
				party = MAX(party, 0);
				if (action != A_INSERT)
					party = MIN(party, (REBINT)VAL_IMAGE_HIGH(value) - y);
				else
					part = party * w;
				if (partx == 0 || party == 0) return value;
			} else
				Trap_Type(len);
		} else
			 Trap_Arg(arg); // /part not allowed
	} else {
		if (IS_IMAGE(arg)) { // Use image for /part sizes
			partx = VAL_IMAGE_WIDE(arg);
			party = VAL_IMAGE_HIGH(arg);
			partx = MIN(partx, (REBINT)w - x); // clip part width
			if (action != A_INSERT)
				party = MIN(party, (REBINT)VAL_IMAGE_HIGH(value) - y);
			else
				part = party * w;
		} else if (IS_BINARY(arg)) {
			part = VAL_LEN(arg) / 4;
		} else if (IS_BLOCK(arg)) {
			part = VAL_LEN(arg);
		} else if (! (IS_INTEGER(arg) || IS_TUPLE(arg)))
			Trap_Type(arg);
	}

	// Expand image data if necessary:
	if (action == A_INSERT) {
		if (index > tail) index = tail;
		Expand_Series(VAL_SERIES(value), index, dup * part);
		CLEAR(VAL_BIN(value) + (index * 4), dup * part * 4);
		Reset_Height(value);
		tail = VAL_TAIL(value);
		only = 0;
	}
	ip = VAL_IMAGE_HEAD(value);

	// Handle the datatype of the argument.
	if (IS_INTEGER(arg) || IS_TUPLE(arg)) { // scalars
		if (index + dup > tail) dup = tail - index; // clip it
		ip += index * 4;
		if (IS_INTEGER(arg)) { // Alpha channel
			n = VAL_INT32(arg);
			if ((n < 0) || (n > 255)) Trap_Range(arg);
			if (IS_PAIR(count)) // rectangular fill
				Fill_Alpha_Rect((REBCNT *)ip, (REBYTE)n, w, dupx, dupy);
			else
				Fill_Alpha_Line(ip, (REBYTE)n, dup);
		} else if (IS_TUPLE(arg)) { // RGB
			if (IS_PAIR(count)) // rectangular fill
				Fill_Rect((REBCNT *)ip, TO_COLOR_TUPLE(arg), w, dupx, dupy, only);
			else
				Fill_Line((REBCNT *)ip, TO_COLOR_TUPLE(arg), dup, only);
		}
	} else if (IS_IMAGE(arg)) {
		Copy_Rect_Data(value, x, y, partx, party, arg, 0, 0); // dst dx dy w h src sx sy
	} else if (IS_BINARY(arg)) {
		if (index + part > tail) part = tail - index; // clip it
		ip += index * 4;
		for (; dup > 0; dup--, ip += part * 4)
			Bin_To_RGBA(ip, part, VAL_BIN_DATA(arg), part, only);
	} else if (IS_BLOCK(arg)) {
		if (index + part > tail) part = tail - index; // clip it
		ip += index * 4;
		for (; dup > 0; dup--, ip += part * 4)
			Tuples_To_RGBA(ip, part, VAL_BLK_DATA(arg), part);
	} else Trap_Type(arg);

	Reset_Height(value);

	if (action == A_APPEND) VAL_INDEX(value) = 0;
	return value;
}


/***********************************************************************
**
*/  REBVAL *Find_Image(REBVAL *ds)
/*
**	Finds a value in a series and returns the series at the start of it.
**
**		 1 image
**		 2 value [any-type!]
**		 3 /part {Limits the search to a given length or position.}
**		 4 range [number! series! port!]
**		 5 /only {ignore alpha value.}
**		 6 /case - ignored
**		 7 /any  - ignored
**		 8 /with - ignored
**		 9 wild  - ignored
**		10 /skip - ignored
**		11 size  - ignored
**		12 /match {Performs comparison and returns the tail of the match.}
**		13 /tail  {Returns the end of the string.}
**		14 /last  {Backwards from end of string.}
**		15 /reverse {Backwards from the current position.}
**
***********************************************************************/
{
	REBVAL	*value = D_ARG(1);
	REBVAL	*arg   = D_ARG(2);
	REBCNT  index = VAL_INDEX(value);
	REBCNT  tail = VAL_TAIL(value);
	REBCNT	len;
	REBCNT	*ip = (REBCNT *)VAL_IMAGE_DATA(value); // NOTE ints not bytes
	REBCNT  *p;
	REBINT  n;
	REBOOL  only = FALSE;
	REBYTE  no_refs[10] = {5, 6, 7, 8, 9, 10, 13, 14}; // ref - 1 (invalid refinements)

	len = tail - index;
	if (!len) goto find_none;

	for (n = 0; n < 8; n++) // (zero based)
		if (D_REF((REBINT)no_refs[n]))
			Trap0(RE_BAD_REFINE);
//			Trap2(RE_CANNOT_USE, FRM_WORDS(me, (REBINT)no_refs[n]), Get_Global(REB_IMAGE));

	if (IS_TUPLE(arg)) {
		only = (REBOOL)(VAL_TUPLE_LEN(arg) < 4);
		if (D_REF(5)) only = TRUE; // /only flag
		p = Find_Color(ip, TO_COLOR_TUPLE(arg), len, only);
	} else if (IS_INTEGER(arg)) {
		n = VAL_INT32(arg);
		if (n < 0 || n > 255) Trap_Range(arg);
		p = Find_Alpha(ip, n, len);
	} else if (IS_IMAGE(arg)) {
		p = 0;
	} else if (IS_BINARY(arg)) {
		p = 0;
	} else
		Trap_Type(arg);

	// Post process the search (failure or apply /match and /tail):
	if (p) {
		n = (REBCNT)(p - (REBCNT *)VAL_IMAGE_HEAD(value));
		if (D_REF(11)) { // match
			if (n != (REBINT)index) goto find_none;
			n++;
		} else if (D_REF(12)) n++; // /tail
		index = n;
		VAL_INDEX(value) = index;
		return value;
	}
find_none:
	return NONE_VALUE;
}


/***********************************************************************
**
*/	REBFLG Image_Has_Alpha(REBVAL *v, REBFLG save)
/*
***********************************************************************/
{
	int i;
	REBCNT *p;

//	if (VAL_IMAGE_TRANSP_TYPE(v)==VITT_NONE) return FALSE;
//	if (VAL_IMAGE_TRANSP_TYPE(v)==VITT_ALPHA) return TRUE;

	p = (REBCNT *)VAL_IMAGE_HEAD(v);
	i = VAL_IMAGE_WIDE(v)*VAL_IMAGE_HIGH(v);
	for(; i > 0; i--) {
		if (*p++ & 0xff000000) {
//			if (save) VAL_IMAGE_TRANSP(v) = VITT_ALPHA;
			return TRUE;
		}
	}
//	if (save) VAL_IMAGE_TRANSP(v) = VITT_NONE;

	return FALSE;
}


/***********************************************************************
**
*/	void Copy_Rect_Data(REBVAL *dst, REBINT dx, REBINT dy, REBINT w, REBINT h, REBVAL *src, REBINT sx, REBINT sy)
/*
***********************************************************************/
{
	REBCNT	*sbits, *dbits;

	if (w <= 0 || h <= 0) return;

	// Clip at edges:
	if ((REBCNT)(dx + w) > VAL_IMAGE_WIDE(dst)) w = VAL_IMAGE_WIDE(dst) - dx;
	if ((REBCNT)(dy + h) > VAL_IMAGE_HIGH(dst)) h = VAL_IMAGE_HIGH(dst) - dy;

	sbits = VAL_IMAGE_BITS(src) + sy * VAL_IMAGE_WIDE(src) + sx;
	dbits = VAL_IMAGE_BITS(dst) + dy * VAL_IMAGE_WIDE(dst) + dx;
	while (h--) {
		memcpy(dbits, sbits, w*4);
		sbits += VAL_IMAGE_WIDE(src);
		dbits += VAL_IMAGE_WIDE(dst);
	}
}

#ifdef removed_feature
/***********************************************************************
**
*/  static REBVAL* Xandor_Image(REBCNT action, REBVAL *value, REBVAL *arg)
/*
***********************************************************************/
{
	REBCNT i;
	REBCNT *p3;
	REBCNT *p2 = (REBCNT*) VAL_IMAGE_BITS(value);
	REBCNT *p1 = (REBCNT*) VAL_IMAGE_BITS(arg);
	REBCNT tw, ow, aw;
	REBCNT th, oh, ah;
	REBCNT j;

	ow = VAL_IMAGE_WIDE(value);
	oh = VAL_IMAGE_HIGH(value);
	aw = VAL_IMAGE_WIDE(arg);
	ah = VAL_IMAGE_HIGH(arg);
	tw = MAX(ow, aw);
	th = MAX(oh, ah);

	*DS_RETURN = *Make_Image(tw, th);
	p3 = (REBCNT*) VAL_IMAGE_HEAD(DS_RETURN);
	CLEAR_IMAGE(p3, tw, th);

	for (i = 0; i < th; i++) {
		for (j = 0; j < tw; j++) {
			if (j < ow && i < oh && j < aw && i < ah)
				*(p3 + (i*tw) +j) = (REBCNT) (action == A_AND) ?
					*(p2 + (i*ow) + j) & *(p1 + (i*aw) + j) :
						(action == A_OR) ?
							*(p2 + (i*ow) + j) | *(p1 + (i*aw) + j) :
							*(p2 + (i*ow) + j) ^ *(p1 + (i*aw) + j) ;
			else {
				if (j < ow && i < oh) *(p3 + (i*tw) + j) = *(p2 + (i*ow) + j);
				if (j < aw && i < ah) *(p3 + (i*tw) + j) = *(p1 + (i*aw) + j);
			}
		}
	}

	return DS_RETURN;
}
#endif

/***********************************************************************
**
*/  static REBSER *Complement_Image(REBVAL *value)
/*
***********************************************************************/
{
	REBCNT *img = (REBCNT*) VAL_IMAGE_DATA(value);
	REBCNT *out;
	REBINT len = VAL_IMAGE_LEN(value);
	REBSER *ser;

	ser = Make_Image(VAL_IMAGE_WIDE(value), VAL_IMAGE_HIGH(value), TRUE);
	out = (REBCNT*) IMG_DATA(ser);

	for (; len > 0; len --) *out++ = ~ *img++;

	return ser;
}


/***********************************************************************
**
*/	REBTYPE(Image)
/*
***********************************************************************/
{
	REBVAL	*value = D_ARG(1);
	REBVAL	*arg = D_ARG(2);
	REBSER	*series = VAL_SERIES(value);
	REBINT	index = (REBINT)VAL_INDEX(value);
	REBINT	tail;
	REBINT	diff, len, w, h;
	REBVAL	*val;

	// Clip index if past tail:
	if (action != A_MAKE && action != A_TO) {
		tail = (REBINT)SERIES_TAIL(series);
		if (index > tail) index = tail;
	}

	// Check must be in this order (to avoid checking a non-series value);
	if (action >= A_TAKE && action <= A_SORT && IS_PROTECT_SERIES(series))
		Trap0(RE_PROTECTED);

	// Dispatch action:
	switch (action) {

	case A_HEAD: VAL_INDEX(value) = 0; break;
	case A_TAIL: VAL_INDEX(value) = (REBCNT)tail; break;
	case A_HEADQ: DECIDE(index == 0);
	case A_TAILQ: DECIDE(index >= tail);
	case A_NEXT: if (index < tail) VAL_INDEX(value)++; break;
	case A_BACK: if (index > 0) VAL_INDEX(value)--; break;

#ifdef removed_feature
	case A_AND:
	case A_OR:
	case A_XOR:
		if (IS_IMAGE(value) && IS_IMAGE(arg)) {
			Xandor_Image(action, value, arg); // sets DS_RETURN
			return R_RET;
		}
		else Trap_Action(VAL_TYPE(value), action);
#endif

	case A_COMPLEMENT:
		series = Complement_Image(value);
		SET_IMAGE(value, series); // use series var not func
		break;

	case A_INDEXQ:
		if (D_REF(2)) {
			VAL_SET(D_RET, REB_PAIR);
			VAL_PAIR_X(D_RET) = (REBD32)(index % VAL_IMAGE_WIDE(value));
			VAL_PAIR_Y(D_RET) = (REBD32)(index / VAL_IMAGE_WIDE(value));
			return R_RET;
		} else {
			DS_RET_INT(index + 1);
			return R_RET;
		}
	case A_LENGTHQ:
		DS_RET_INT(tail > index ? tail - index : 0);
		return R_RET;

	case A_PICK:
		Pick_Path(value, arg, 0);
		return R_TOS;

	case A_POKE:
		Pick_Path(value, arg, D_ARG(3));
		return R_ARG3;

	case A_SKIP:
	case A_AT:
		// This logic is somewhat complicated by the fact that INTEGER args use
		// base-1 indexing, but PAIR args use base-0.
		if (IS_PAIR(arg)) {
			if (action == A_AT) action = A_SKIP;
			diff = (VAL_PAIR_Y_INT(arg) * VAL_IMAGE_WIDE(value) + VAL_PAIR_X_INT(arg)) +
				((action == A_SKIP) ? 0 : 1);
		} else
			diff = Get_Num_Arg(arg);

		index += diff;
		if (action == A_SKIP) {
			if (IS_LOGIC(arg)) index--;
		} else {
			if (diff > 0) index--; // For at, pick, poke.
		}

		if (index > tail) index = tail;
		else if (index < 0) index = 0;
		VAL_INDEX(value) = (REBCNT)index;
		break;

#ifdef obsolete
		if (action == A_SKIP || action == A_AT) {
		}

		if (diff == 0 || index < 0 || index >= tail) {
			if (action == A_POKE)
				Trap_Range(arg);
			goto is_none;
		}

		if (action == A_POKE) {
			REBINT *dp = QUAD_SKIP(series, index));
			REBINT n;

			arg = D_ARG(3);
			if (IS_TUPLE(arg) && (IS_IMAGE(value))) {
				Set_Pixel_Tuple(QUAD_SKIP(series, index), arg);
				//*dp = (long) (VAL_TUPLE_LEN(arg) < 4) ?
				//	((*dp & 0xff000000) | (VAL_TUPLE(arg)[0] << 16) | (VAL_TUPLE(arg)[1] << 8) | (VAL_TUPLE(arg)[2])) :
				//	((VAL_TUPLE(arg)[3] << 24) | (VAL_TUPLE(arg)[0] << 16) | (VAL_TUPLE(arg)[1] << 8) | (VAL_TUPLE(arg)[2]));
				DS_RET_VALUE(arg);
				return R_RET;
			}
			if (IS_INTEGER(arg) && VAL_INT64(arg) > 0 && VAL_INT64(arg) < 255)
				n = VAL_INT32(arg);
			else if (IS_CHAR(arg))
				n = VAL_CHAR(arg);
			else
				Trap_Arg(arg);

			*dp = (*dp & 0xffffff) | (n << 24);
			DS_RET_VALUE(arg);
			return R_RET; //was value;

		} else {
			Set_Tuple_Pixel(QUAD_SKIP(series, index), D_RET);
			return R_RET;
		}
		break;
#endif

	case A_CLEAR:	// clear series
		if (index < tail) {
			VAL_TAIL(value) = (REBCNT)index;
			Reset_Height(value);
		}
		break;

	case A_REMOVE:	// remove series /part count
		if (D_REF(2)) {
			val = D_ARG(3);
			if (IS_INTEGER(val)) {
				len = VAL_INT32(val);
			} else if (IS_IMAGE(val)) {
				if (!VAL_IMAGE_WIDE(val)) Trap_Arg(val);
				len = VAL_INDEX(val) - VAL_INDEX(value); // may not be same, is ok
			} else
				Trap_Type(val);
		} else len = 1;
		index = (REBINT)VAL_INDEX(value);
		if (index < tail && len != 0) {
			Remove_Series(series, VAL_INDEX(value), len);
		}
		Reset_Height(value);
		break;

	case A_APPEND:
	case A_INSERT:	// insert ser val /part len /only /dup count
	case A_CHANGE:	// change ser val /part len /only /dup count
		value = Modify_Image(ds, action); // sets DS_RETURN
		break;

	case A_FIND:	// find   ser val /part len /only /case /any /with wild /match /tail
		Find_Image(ds); // sets DS_RETURN
		break;

	case A_TO:
		if (IS_IMAGE(arg)) goto makeCopy;
		else if (IS_GOB(arg)) {
			//value = Make_Image(ROUND_TO_INT(GOB_W(VAL_GOB(arg))), ROUND_TO_INT(GOB_H(VAL_GOB(arg))));
			//*D_RET = *value;
			series = OS_GOB_TO_IMAGE(VAL_GOB(arg));
			if (!series) Trap_Make(REB_IMAGE, arg);
			SET_IMAGE(value, series);
			break;
		}
		else if (IS_BINARY(arg)) {
			diff = VAL_LEN(arg) / 4;
			if (diff == 0) Trap_Make(REB_IMAGE, arg);
			if (diff < 100) w = diff;
			else if (diff < 10000) w = 100;
			else w = 500;
			h = diff / w;
			if (w * h < diff) h++; // partial line
			series = Make_Image(w, h, TRUE);
			SET_IMAGE(value, series);
			Bin_To_RGBA(IMG_DATA(series), w*h, VAL_BIN_DATA(arg), VAL_LEN(arg)/4, 0);
			break;
		}
		Trap_Type(arg);
		break;

	case A_MAKE:
		// make image! img
		if (IS_IMAGE(arg)) goto makeCopy;

		// make image! [] (or none)
		if (IS_IMAGE(value) && (IS_NONE(arg) || (IS_BLOCK(arg) && (VAL_BLK_LEN(arg) == 0)))) {
			arg = value;
			goto makeCopy;
		}

		// make image! size
		if (IS_PAIR(arg)) {
			w = VAL_PAIR_X_INT(arg);
			h = VAL_PAIR_Y_INT(arg);
			w = MAX(w, 0);
			h = MAX(h, 0);
			series = Make_Image(w, h, TRUE);
			SET_IMAGE(value, series);
			break;
		}
//		else if (IS_NONE(arg)) {
//			*value = *Make_Image(0, 0);
//			CLEAR_IMAGE(VAL_IMAGE_HEAD(value), 0, 0);
//			break;
//		}
		// make image! [size rgb alpha index]
		else if (IS_BLOCK(arg)) {
			if (Create_Image(VAL_BLK_DATA(arg), value, 0)) break;
		}
		Trap_Type(arg);
		break;

	case A_COPY:  // copy series /part len
		if (!D_REF(2)) {
			arg = value;
			goto makeCopy;
		}
		arg = D_ARG(3); // can be image, integer, pair.
		if (IS_IMAGE(arg)) {
			if (VAL_SERIES(arg) != VAL_SERIES(value)) Trap_Arg(arg);
			len = VAL_INDEX(arg) - VAL_INDEX(value);
			arg = value;
			goto makeCopy2;
		}
		if (IS_INTEGER(arg)) {
			len = VAL_INT32(arg);
			arg = value;
			goto makeCopy2;
		}
		if (IS_PAIR(arg)) {
			w = VAL_PAIR_X_INT(arg);
			h = VAL_PAIR_Y_INT(arg);
			w = MAX(w, 0);
			h = MAX(h, 0);
			diff = MIN(VAL_TAIL(value), VAL_INDEX(value)); // index offset
			diff = MAX(0, diff);
			index = VAL_IMAGE_WIDE(value); // width
			if (index) {
				len = diff / index; // compute y offset
				diff %= index; // compute x offset
			} else len = diff = 0; // avoid div zero
			w = MIN(w, index - diff); // img-width - x-pos
			h = MIN(h, (int)(VAL_IMAGE_HIGH(value) - len)); // img-high - y-pos
			series = Make_Image(w, h, TRUE);
			SET_IMAGE(D_RET, series);
			Copy_Rect_Data(D_RET, 0, 0, w, h, value, diff, len);
//			VAL_IMAGE_TRANSP(D_RET) = VAL_IMAGE_TRANSP(value);
			return R_RET;
		}
		Trap_Type(arg);

makeCopy:
		// Src image is arg.
		len = VAL_IMAGE_LEN(arg);
makeCopy2:
		len = MAX(len, 0); // no negatives
		len = MIN(len, (REBINT)VAL_IMAGE_LEN(arg));
		w = VAL_IMAGE_WIDE(arg);
		w = MAX(w, 1);
		if (len <= w) h = 1, w = len;
		else h = len / w;
		if (w == 0) h = 0;
		series = Make_Image(w, h, TRUE);
		SET_IMAGE(D_RET, series);
		memcpy(VAL_IMAGE_HEAD(D_RET), VAL_IMAGE_DATA(arg), w * h * 4);
//		VAL_IMAGE_TRANSP(D_RET) = VAL_IMAGE_TRANSP(arg);
		return R_RET;
		break;

	default:
		Trap_Action(VAL_TYPE(value), action);
	}

	*DS_RETURN = *value;
	return R_RET;

is_false:
	return R_FALSE;

is_true:
	return R_TRUE;
}


/***********************************************************************
**
*/	REBINT PD_Image(REBPVS *pvs)
/*
***********************************************************************/
{
	REBVAL *data = pvs->value;
	REBVAL *sel = pvs->select;
	REBVAL *val = pvs->setval;
	REBINT n;
	REBINT len;
	REBYTE *src;
	REBINT index = (REBINT)VAL_INDEX(data);
	REBSER *nser;
	REBSER *series = VAL_SERIES(data);
	REBCNT *dp;

	len = VAL_TAIL(data) - index;
	len = MAX(len, 0);
	src = VAL_IMAGE_DATA(data);

	if (IS_PAIR(sel)) n = (VAL_PAIR_Y_INT(sel) * VAL_IMAGE_WIDE(data) + VAL_PAIR_X_INT(sel)) + 1;
	else if (IS_INTEGER(sel)) n = VAL_INT32(sel);
	else if (IS_DECIMAL(sel)) n = (REBINT)VAL_DECIMAL(sel);
	else if (IS_LOGIC(sel))   n = (VAL_LOGIC(sel) ? 1 : 2);
	else if (IS_WORD(sel)) {
		if (val == 0) {
			val = pvs->value = pvs->store;
			switch (VAL_WORD_CANON(sel)) {

			case SYM_SIZE:
				VAL_SET(val, REB_PAIR);
				VAL_PAIR_X(val) = (REBD32)VAL_IMAGE_WIDE(data);
				VAL_PAIR_Y(val) = (REBD32)VAL_IMAGE_HIGH(data);
				break;

			case SYM_RGB:
				nser = Make_Binary(len * 3);
				SERIES_TAIL(nser) = len * 3;
				RGB_To_Bin(QUAD_HEAD(nser), src, len, FALSE);
				Set_Binary(val, nser);
				break;

			case SYM_ALPHA:
				nser = Make_Binary(len);
				SERIES_TAIL(nser) = len;
				Alpha_To_Bin(QUAD_HEAD(nser), src, len);
				Set_Binary(val, nser);
				break;

			default:
				return PE_BAD_SELECT;
			}
			return PE_OK;

		} else {

			switch (VAL_WORD_CANON(sel)) {

			case SYM_SIZE:
				if (!IS_PAIR(val) || !VAL_PAIR_X(val)) return PE_BAD_SET;
				VAL_IMAGE_WIDE(data) = VAL_PAIR_X_INT(val);
				VAL_IMAGE_HIGH(data) = MIN(VAL_PAIR_Y_INT(val), (REBINT)VAL_TAIL(data) / VAL_PAIR_X_INT(val));
				break;

			case SYM_RGB:
				if (IS_TUPLE(val)) {
					Fill_Line((REBCNT *)src, TO_COLOR_TUPLE(val), len, 1);
				} else if (IS_INTEGER(val)) {
					n = VAL_INT32(val);
					if (n < 0 || n > 255) return PE_BAD_RANGE;
					Fill_Line((REBCNT *)src, TO_COLOR(n,n,n,0), len, 1);
				} else if (IS_BINARY(val)) {
					Bin_To_RGB(src, len, VAL_BIN_DATA(val), VAL_LEN(val) / 3);
				} else return PE_BAD_SET;
				break;

			case SYM_ALPHA:
				if (IS_INTEGER(val)) {
					n = VAL_INT32(val);
					if (n < 0 || n > 255) return PE_BAD_RANGE;
					Fill_Alpha_Line(src, (REBYTE)n, len);
				} else if (IS_BINARY(val)) {
					Bin_To_Alpha(src, len, VAL_BIN_DATA(val), VAL_LEN(val));
				} else return PE_BAD_SET;
				break;

			default:
				return PE_BAD_SELECT;
			}
			return PE_OK;
		}
	}
	else return PE_BAD_SELECT;

	// Handle index path:
	index += n;
	if (n > 0) index--;

	TRAP_PROTECT(series);

	// Out of range:
	if (n == 0 || index < 0 || index >= (REBINT)series->tail) {
		if (val) return PE_BAD_SET;
		return PE_NONE;
	}

	// Get the pixel:
	if (val == 0) {
		Set_Tuple_Pixel(QUAD_SKIP(series, index), pvs->store);
		return PE_USE;
	}

	// Set the pixel:
	if (IS_TUPLE(val) && (IS_IMAGE(data))) {
		Set_Pixel_Tuple(QUAD_SKIP(series, index), val);
		//*dp = (long) (VAL_TUPLE_LEN(val) < 4) ?
		//	((*dp & 0xff000000) | (VAL_TUPLE(val)[0] << 16) | (VAL_TUPLE(val)[1] << 8) | (VAL_TUPLE(val)[2])) :
		//	((VAL_TUPLE(val)[3] << 24) | (VAL_TUPLE(val)[0] << 16) | (VAL_TUPLE(val)[1] << 8) | (VAL_TUPLE(val)[2]));
		return PE_OK;
	}

	// Set the alpha only:
	if (IS_INTEGER(val) && VAL_INT64(val) > 0 && VAL_INT64(val) < 255) n = VAL_INT32(val);
	else if (IS_CHAR(val)) n = VAL_CHAR(val);
	else return PE_BAD_RANGE;

	dp = (REBCNT*)QUAD_SKIP(series, index);
	*dp = (*dp & 0xffffff) | (n << 24);
	return PE_OK;
}
