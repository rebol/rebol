/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2021 Rebol Open Source Developers
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
**  Module:  u-image-blur.c
**  Summary: image blur
**  Section: utility
**  Author:  Oldes
** 
**  Note:
**  Based on Ivan Kuckir's Fastest Gaussian blur implementation algorithm
**  from article: http://blog.ivank.net/fastest-gaussian-blur.html and
**  SilverWin's C port of it: https://github.com/SilverWin/blur_lib
**
***********************************************************************/

#include "sys-core.h"

#ifdef INCLUDE_IMAGE_NATIVES
#include "sys-blur.h"

static void boxes_for_gauss(REBDEC sigma, REBINT sizes[3])
{
	REBDEC wIdeal;
	REBINT wl;
	REBINT wu;
	REBDEC mIdeal;
	REBINT m;
	REBINT n = 3;
	REBINT i;

	wIdeal = sqrt((REBDEC)(12 * sigma * sigma / n) + 1); /* Ideal averaging filter width */
	wl = (REBINT)floor(wIdeal);
	if (wl % 2 == 0)
		wl--;
	wu = wl + 2;

	mIdeal = (12 * sigma * sigma - (REBDEC)wl * wl * n - 4.0 * n * wl - 3.0 * n) / (-4.0 * wl - 4);
	m = (REBINT)round(mIdeal);

	for (i = 0; i < n; i++)
	{
		if (i < m)
		{
			sizes[i] = wl;
		}
		else
		{
			sizes[i] = wu;
		}
	}
}

static void box_blur_H(REBYTE *scl, REBYTE*tcl, REBINT w, REBINT h, REBINT r, REBINT bpp)
{
	REBINT i, j, k, ti, li, ri, fv, lv, val;
	for (i = 0; i < h; i++)
	{
		for (k = 0; k < bpp; k++)
		{
			ti  = i * w * bpp + k;
			li  = ti;
			ri  = ti + r * bpp;
			fv  = scl[li];
			lv  = scl[ti + (w - 1) * bpp];
			val = (r + 1) * fv;
			for (j = 0; j < r; j++)
			{
				val += scl[ti + (j * bpp)];
			}
			for (j = 0; j <= r; j++)
			{
				val += scl[ri] - fv;
				tcl[ti] = (REBYTE)round(val / (r + r + 1));
				ri += bpp;
				ti += bpp;
			}
			for (j = r + 1; j < (w - r); j++)
			{
				val += scl[ri] - scl[li];
				tcl[ti] = (REBYTE)round(val / (r + r + 1));
				li += bpp;
				ri += bpp;
				ti += bpp;
			}
			for (j = w - r; j < w; j++)
			{
				val += lv - scl[li];
				tcl[ti] = (REBYTE)round(val / (r + r + 1));
				li += bpp;
				ti += bpp;
			}
		}
	}
}

static void box_blur_T(REBYTE*scl, REBYTE*tcl, REBINT w, REBINT h, REBINT r, REBINT bpp)
{
	REBINT i, j, k, ti, li, ri, fv, lv, val;
	for (i = 0; i < w; i++)
	{
		for (k = 0; k < bpp; k++)
		{
			ti = i * bpp + k;
			li = ti;
			ri = ti + r * w * bpp;
			fv = scl[ti];
			lv = scl[ti + w * (h - 1) * bpp];
			val = (r + 1) * fv;
			for (j = 0; j < r; j++)
			{
				val += scl[ti + j * w * bpp];
			}
			for (j = 0; j <= r; j++)
			{
				val += scl[ri] - fv;
				tcl[ti] = (REBYTE)round(val / (r + r + 1));
				ri += w * bpp;
				ti += w * bpp;
			}
			for (j = r + 1; j < (h - r); j++)
			{
				val += scl[ri] - scl[li];
				tcl[ti] = (REBYTE)round(val / (r + r + 1));
				li += w * bpp;
				ri += w * bpp;
				ti += w * bpp;
			}
			for (j = h - r; j < h; j++)
			{
				val += lv - scl[li];
				tcl[ti] = (REBYTE)round(val / (r + r + 1));
				li += w * bpp;
				ti += w * bpp;
			}
		}
	}
}

static void box_blur(REBYTE*scl, REBYTE*tcl, REBINT w, REBINT h, REBINT r, REBINT bpp)
{
	REBINT i;
	for (i = 0; i < (h * w * bpp); i++)
	{
		tcl[i] = scl[i];
	}
	box_blur_H(tcl, scl, w, h, r, bpp);
	box_blur_T(scl, tcl, w, h, r, bpp);
}

void fast_gauss_blur(REBYTE*scl, REBYTE*tcl, REBINT w, REBINT h, REBINT r, REBINT bpp)
{
	REBINT i;
	REBINT bxs[3];
	boxes_for_gauss(r, bxs);
	box_blur(scl, tcl, w, h, (bxs[0] - 1) / 2, bpp);
	box_blur(tcl, scl, w, h, (bxs[1] - 1) / 2, bpp);
	box_blur(scl, tcl, w, h, (bxs[2] - 1) / 2, bpp);
	// result would be in tcl, so copy it back to source, as it is modified anyway
	for (i = 0; i < (h * w * bpp); i++)
	{
		scl[i] = tcl[i];
	}
}

void BlurImage(REBSER *image, REBCNT radius)
{
	REBSER *temp_image;

	// Validate input.
	if ( image == NULL
		|| (IMG_WIDE(image) == 0UL)
		|| (IMG_HIGH(image) == 0UL)
		|| radius == 0
	) return;

	radius = MIN(radius, IMG_WIDE(image) / 2);
	radius = MIN(radius, IMG_HIGH(image) / 2);

	// Prepare temporary image.

	temp_image = Make_Image(IMG_WIDE(image), IMG_HIGH(image), TRUE);

	// Resize image.
	
	fast_gauss_blur(IMG_DATA(image), IMG_DATA(temp_image), IMG_WIDE(image), IMG_HIGH(image), radius, 4);
}

#endif // INCLUDE_IMAGE_NATIVES