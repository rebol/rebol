/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2020 Rebol Open Source Developers
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
**  Module:  u-image-resize.c
**  Summary: image resizing
**  Section: utility
**  Author:  Oldes (ported code from GraphicsMagick - see bellow)
**
***********************************************************************
**  Base-code:

	append system/catalog [
		filters [
		  Point
		  Box
		  Triangle
		  Hermite
		  Hanning
		  Hamming
		  Blackman
		  Gaussian
		  Quadratic
		  Cubic
		  Catrom
		  Mitchell
		  Lanczos
		  Bessel
		  Sinc
		]
	]

***********************************************************************/
/*
% Copyright (C) 2003 - 2019 GraphicsMagick Group
% Copyright (C) 2002 ImageMagick Studio
%
% This program is covered by multiple licenses, which are described in
% Copyright.txt. You should have received a copy of Copyright.txt with this
% package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                 RRRR   EEEEE  SSSSS  IIIII  ZZZZZ  EEEEE                    %
%                 R   R  E      SS       I       ZZ  E                        %
%                 RRRR   EEE     SSS     I     ZZZ   EEE                      %
%                 R R    E         SS    I    ZZ     E                        %
%                 R  R   EEEEE  SSSSS  IIIII  ZZZZZ  EEEEE                    %
%                                                                             %
%                    GraphicsMagick Image Resize Methods                      %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
	Include declarations.
*/
#include "sys-core.h"

#ifdef INCLUDE_IMAGE_NATIVES

#include "sys-magick.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   B e s s e l O r d e r O n e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  BesselOrderOne() computes the Bessel function of x of the first kind of
%  order 0:
%
%    Reduce x to |x| since j1(x)= -j1(-x), and for x in (0,8]
%
%       j1(x) = x*j1(x);
%
%    For x in (8,inf)
%
%       j1(x) = sqrt(2/(pi*x))*(p1(x)*cos(x1)-q1(x)*sin(x1))
%
%    where x1 = x-3*pi/4. Compute sin(x1) and cos(x1) as follow:
%
%       cos(x1) =  cos(x)cos(3pi/4)+sin(x)sin(3pi/4)
%               =  1/sqrt(2) * (sin(x) - cos(x))
%       sin(x1) =  sin(x)cos(3pi/4)-cos(x)sin(3pi/4)
%               = -1/sqrt(2) * (sin(x) + cos(x))
%
%  The format of the BesselOrderOne method is:
%
%      REBDEC BesselOrderOne(REBDEC x)
%
%  A description of each parameter follows:
%
%    o value: Method BesselOrderOne returns the Bessel function of x of the
%      first kind of orders 1.
%
%    o x: REBDEC value.
%
%
*/

static REBDEC J1(REBDEC x)
{
	REBDEC
		p,
		q;

	register long
		i;

	static const REBDEC
		Pone[] =
		{
			 0.581199354001606143928050809e+21,
			-0.6672106568924916298020941484e+20,
			 0.2316433580634002297931815435e+19,
			-0.3588817569910106050743641413e+17,
			 0.2908795263834775409737601689e+15,
			-0.1322983480332126453125473247e+13,
			 0.3413234182301700539091292655e+10,
			-0.4695753530642995859767162166e+7,
			 0.270112271089232341485679099e+4
		},
		Qone[] =
		{
			0.11623987080032122878585294e+22,
			0.1185770712190320999837113348e+20,
			0.6092061398917521746105196863e+17,
			0.2081661221307607351240184229e+15,
			0.5243710262167649715406728642e+12,
			0.1013863514358673989967045588e+10,
			0.1501793594998585505921097578e+7,
			0.1606931573481487801970916749e+4,
			0.1e+1
		};

	p=Pone[8];
	q=Qone[8];
	for (i=7; i >= 0; i--)
	{
		p=p*x*x+Pone[i];
		q=q*x*x+Qone[i];
	}
	return(p/q);
}

static REBDEC P1(REBDEC x)
{
	REBDEC
		p,
		q;

	register long
		i;

	static const REBDEC
		Pone[] =
		{
			0.352246649133679798341724373e+5,
			0.62758845247161281269005675e+5,
			0.313539631109159574238669888e+5,
			0.49854832060594338434500455e+4,
			0.2111529182853962382105718e+3,
			0.12571716929145341558495e+1
		},
		Qone[] =
		{
			0.352246649133679798068390431e+5,
			0.626943469593560511888833731e+5,
			0.312404063819041039923015703e+5,
			0.4930396490181088979386097e+4,
			0.2030775189134759322293574e+3,
			0.1e+1
		};

	p=Pone[5];
	q=Qone[5];
	for (i=4; i >= 0; i--)
	{
		p=p*(8.0/x)*(8.0/x)+Pone[i];
		q=q*(8.0/x)*(8.0/x)+Qone[i];
	}
	return(p/q);
}

static REBDEC Q1(REBDEC x)
{
	REBDEC
		p,
		q;

	register long
		i;

	static const REBDEC
		Pone[] =
		{
			0.3511751914303552822533318e+3,
			0.7210391804904475039280863e+3,
			0.4259873011654442389886993e+3,
			0.831898957673850827325226e+2,
			0.45681716295512267064405e+1,
			0.3532840052740123642735e-1
		},
		Qone[] =
		{
			0.74917374171809127714519505e+4,
			0.154141773392650970499848051e+5,
			0.91522317015169922705904727e+4,
			0.18111867005523513506724158e+4,
			0.1038187585462133728776636e+3,
			0.1e+1
		};

	p=Pone[5];
	q=Qone[5];
	for (i=4; i >= 0; i--)
	{
		p=p*(8.0/x)*(8.0/x)+Pone[i];
		q=q*(8.0/x)*(8.0/x)+Qone[i];
	}
	return(p/q);
}

static REBDEC BesselOrderOne(REBDEC x)
{
	REBDEC
		p,
		q;

	if (x == 0.0)
		return(0.0);
	p=x;
	if (x < 0.0)
		x=(-x);
	if (x < 8.0)
		return(p*J1(x));
	q=sqrt(2.0/(MagickPI*x))*(P1(x)*(1.0/sqrt(2.0)*(sin(x)-cos(x)))-8.0/x*Q1(x)*
		(-1.0/sqrt(2.0)*(sin(x)+cos(x))));
	if (p < 0.0)
		q=(-q);
	return(q);
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s i z e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResizeImage() scales an image to the desired dimensions with one of these
%  filters:
%
%    Bessel   Blackman   Box
%    Catrom   Cubic      Gaussian
%    Hanning  Hermite    Lanczos
%    Mitchell Point      Quandratic
%    Sinc     Triangle
%
%  Most of the filters are FIR (finite impulse response), however, Bessel,
%  Gaussian, and Sinc are IIR (infinite impulse response).  Bessel and Sinc
%  are windowed (brought down to zero) with the Blackman filter.
%
%  ResizeImage() was inspired by Paul Heckbert's zoom program.
%
*/

static REBDEC Bessel(const REBDEC x,const REBDEC support)
{
	UNUSED(support);
	if (x == 0.0)
		return(MagickPI/4.0);
	return(BesselOrderOne(MagickPI*x)/(2.0*x));
}

static REBDEC Sinc(const REBDEC x,const REBDEC support)
{
	UNUSED(support);
	if (x == 0.0)
		return(1.0);
	return(sin(MagickPI*x)/(MagickPI*x));
}

static REBDEC Blackman(const REBDEC x,const REBDEC support)
{
	UNUSED(support);
	return(0.42+0.5*cos(MagickPI*x)+0.08*cos(2*MagickPI*x));
}

static REBDEC BlackmanBessel(const REBDEC x,const REBDEC support)
{
	return(Blackman(x/support,support)*Bessel(x,support));
}

static REBDEC BlackmanSinc(const REBDEC x,const REBDEC support)
{
	return(Blackman(x/support,support)*Sinc(x,support));
}

static REBDEC Box(const REBDEC x,const REBDEC support)
{
	UNUSED(support);
	if (x < -0.5)
		return(0.0);
	if (x < 0.5)
		return(1.0);
	return(0.0);
}

static REBDEC Catrom(const REBDEC x,const REBDEC support)
{
	UNUSED(support);
	if (x < -2.0)
		return(0.0);
	if (x < -1.0)
		return(0.5*(4.0+x*(8.0+x*(5.0+x))));
	if (x < 0.0)
		return(0.5*(2.0+x*x*(-5.0-3.0*x)));
	if (x < 1.0)
		return(0.5*(2.0+x*x*(-5.0+3.0*x)));
	if (x < 2.0)
		return(0.5*(4.0+x*(-8.0+x*(5.0-x))));
	return(0.0);
}

static REBDEC Cubic(const REBDEC x,const REBDEC support)
{
	UNUSED(support);
	if (x < -2.0)
		return(0.0);
	if (x < -1.0)
		return((2.0+x)*(2.0+x)*(2.0+x)/6.0);
	if (x < 0.0)
		return((4.0+x*x*(-6.0-3.0*x))/6.0);
	if (x < 1.0)
		return((4.0+x*x*(-6.0+3.0*x))/6.0);
	if (x < 2.0)
		return((2.0-x)*(2.0-x)*(2.0-x)/6.0);
	return(0.0);
}

static REBDEC Gaussian(const REBDEC x,const REBDEC support)
{
	UNUSED(support);
	return(exp(-2.0*x*x)*sqrt(2.0/MagickPI));
}

static REBDEC Hanning(const REBDEC x,const REBDEC support)
{
	UNUSED(support);
	return(0.5+0.5*cos(MagickPI*x));
}

static REBDEC Hamming(const REBDEC x,const REBDEC support)
{
	UNUSED(support);
	return(0.54+0.46*cos(MagickPI*x));
}

static REBDEC Hermite(const REBDEC x,const REBDEC support)
{
	UNUSED(support);
	if (x < -1.0)
		return(0.0);
	if (x < 0.0)
		return((2.0*(-x)-3.0)*(-x)*(-x)+1.0);
	if (x < 1.0)
		return((2.0*x-3.0)*x*x+1.0);
	return(0.0);
}

static REBDEC Lanczos(const REBDEC x,const REBDEC support)
{
	if (x < -3.0)
		return(0.0);
	if (x < 0.0)
		return(Sinc(-x,support)*Sinc(-x/3.0,support));
	if (x < 3.0)
		return(Sinc(x,support)*Sinc(x/3.0,support));
	return(0.0);
}

static REBDEC Mitchell(const REBDEC x,const REBDEC support)
{
#define B   (1.0/3.0)
#define C   (1.0/3.0)
#define P0  ((  6.0- 2.0*B       )/6.0)
#define P2  ((-18.0+12.0*B+ 6.0*C)/6.0)
#define P3  (( 12.0- 9.0*B- 6.0*C)/6.0)
#define Q0  ((       8.0*B+24.0*C)/6.0)
#define Q1  ((     -12.0*B-48.0*C)/6.0)
#define Q2  ((       6.0*B+30.0*C)/6.0)
#define Q3  ((     - 1.0*B- 6.0*C)/6.0)

	UNUSED(support);
	if (x < -2.0)
		return(0.0);
	if (x < -1.0)
		return(Q0-x*(Q1-x*(Q2-x*Q3)));
	if (x < 0.0)
		return(P0+x*x*(P2-x*P3));
	if (x < 1.0)
		return(P0+x*x*(P2+x*P3));
	if (x < 2.0)
		return(Q0+x*(Q1+x*(Q2+x*Q3)));
	return(0.0);
}

static REBDEC Quadratic(const REBDEC x,const REBDEC support)
{
	UNUSED(support);
	if (x < -1.5)
		return(0.0);
	if (x < -0.5)
		return(0.5*(x+1.5)*(x+1.5));
	if (x < 0.5)
		return(0.75-x*x);
	if (x < 1.5)
		return(0.5*(x-1.5)*(x-1.5));
	return(0.0);
}

static REBDEC Triangle(const REBDEC x,const REBDEC support)
{
	UNUSED(support);
	if (x < -1.0)
		return(0.0);
	if (x < 0.0)
		return(1.0+x);
	if (x < 1.0)
		return(1.0-x);
	return(0.0);
}

static void
HorizontalFilter(const REBSER *source, REBSER *destination, REBSER *data_set,
				 const REBDEC x_factor,const FilterInfo * filter_info,
				 const REBDEC blur, REBOOL has_alpha)
{
	REBDEC scale;
	REBDEC support;
	REBINT x, y;
	DoublePixelPacket zero;
	const PixelPacket *p = (PixelPacket*)IMG_DATA(source);
	      PixelPacket *q = (PixelPacket*)IMG_DATA(destination);
	ContributionInfo  *contribution = (ContributionInfo*)SERIES_DATA(data_set);

	scale   = blur  * MAX(1.0 / x_factor, 1.0);
	support = scale * filter_info->support;

	if (support <= 0.5)	{
		// Reduce to point sampling.
		support = 0.5 + MagickEpsilon;
		scale   = 1.0;
	}
	scale = 1.0 / scale;
	CLEAR(&zero, sizeof(DoublePixelPacket));

	for (x=0; x < (long) IMG_WIDE(destination); x++) {
		REBDEC center;
		REBDEC density = 0.0;
		REBINT n, start, stop, i, j;

		center = (REBDEC) (x+0.5)/x_factor;
		start  = (REBINT) MAX(center-support+0.5,0);
		stop   = (REBINT) MIN(center+support+0.5,IMG_WIDE(source));
		
		for (n=0; n < (stop-start); n++) {
			contribution[n].pixel = start+n;
			contribution[n].weight = filter_info->function(scale*((REBDEC) start+n-center+0.5), filter_info->support);
			density += contribution[n].weight;
		}

		if ((density != 0.0) && (density != 1.0)) {
			// Normalize.
			density = 1.0 / density;
			for (i=0; i < n; i++)
				contribution[i].weight*=density;
		}

		for (y=0; y < (long) IMG_HIGH(destination); y++) {
			REBDEC weight;
			DoublePixelPacket pixel = zero;

			if (has_alpha) {
				REBDEC transparency_coeff;
				REBDEC normalize = 0.0;
				
				for (i=0; i < n; i++) {
					j = (int)((y * IMG_WIDE(source)) + contribution[i].pixel);
					weight=contribution[i].weight;
					transparency_coeff = weight * ((REBDEC) p[j].opacity/OpaqueOpacity);
					pixel.red     += transparency_coeff * p[j].red;
					pixel.green   += transparency_coeff * p[j].green;
					pixel.blue    += transparency_coeff * p[j].blue;
					pixel.opacity += weight * p[j].opacity;
					normalize     += transparency_coeff;
				}
				normalize    = 1.0 / (ABS(normalize) <= MagickEpsilon ? 1.0 : normalize);
				pixel.red   *= normalize;
				pixel.green *= normalize;
				pixel.blue  *= normalize;
			}
			else {
				for (i=0; i < n; i++) {
					j = (int)((y * IMG_WIDE(source)) + contribution[i].pixel);
					weight=contribution[i].weight;
					pixel.red     += weight * p[j].red;
					pixel.green   += weight * p[j].green;
					pixel.blue    += weight * p[j].blue;
				}
				pixel.opacity = OpaqueOpacity;
			}
			REBINT pix = (y * IMG_WIDE(destination)) + x;
			q[pix].red     = RoundDoubleToQuantum(pixel.red);
			q[pix].green   = RoundDoubleToQuantum(pixel.green);
			q[pix].blue    = RoundDoubleToQuantum(pixel.blue);
			q[pix].opacity = RoundDoubleToQuantum(pixel.opacity);
		}
	}
}

static void
VerticalFilter(const REBSER *source, REBSER *destination, REBSER *data_set,
			   const REBDEC y_factor,const FilterInfo * filter_info,
			   const REBDEC blur, REBOOL has_alpha)
{
	REBDEC scale;
	REBDEC support;
	REBINT x, y;
	DoublePixelPacket zero;
	const PixelPacket *p = (PixelPacket*)IMG_DATA(source);
	      PixelPacket *q = (PixelPacket*)IMG_DATA(destination);

	scale   = blur  * MAX(1.0 / y_factor, 1.0);
	support = scale * filter_info->support;

	if (support <= 0.5)	{
		// Reduce to point sampling.
		support = 0.5 + MagickEpsilon;
		scale   = 1.0;
	}
	scale = 1.0 / scale;
	CLEAR(&zero, sizeof(DoublePixelPacket));

	for (y=0; y < (long) IMG_HIGH(destination); y++) {
		REBDEC center;
		REBDEC density = 0.0;
		REBINT n, start, stop, i, j;
		ContributionInfo *contribution = (ContributionInfo*)SERIES_DATA(data_set);

		center = (REBDEC)(y+0.5) / y_factor;
		start  = (REBINT)MAX(center - support + 0.5, 0);
		stop   = (REBINT)MIN(center + support + 0.5, IMG_HIGH(source));

		for (n=0; n < (stop-start); n++) {
			contribution[n].pixel = start+n;
			contribution[n].weight = filter_info->function(scale*((REBDEC) start+n-center+0.5), filter_info->support);
			density += contribution[n].weight;
		}
		if ((density != 0.0) && (density != 1.0)) {
			// Normalize.
			density = 1.0 / density;
			for (i=0; i < n; i++)
				contribution[i].weight *= density;
		}
		for (x=0; x < (long) IMG_WIDE(destination); x++) {
			REBDEC weight;
			DoublePixelPacket pixel = zero;

			if (has_alpha) {
				REBDEC transparency_coeff;
				REBDEC normalize = 0.0;
				for (i=0; i < n; i++) {
					j = (REBINT)((contribution[i].pixel * IMG_WIDE(source)) + x);
					weight=contribution[i].weight;
					transparency_coeff = weight * (((REBDEC) p[j].opacity / OpaqueOpacity));
					pixel.red    += transparency_coeff * p[j].red;
					pixel.green  += transparency_coeff * p[j].green;
					pixel.blue   += transparency_coeff * p[j].blue;
					pixel.opacity+= weight * p[j].opacity;
					normalize    += transparency_coeff;
				}
				normalize    = 1.0 / (ABS(normalize) <= MagickEpsilon ? 1.0 : normalize);
				pixel.red   *= normalize;
				pixel.green *= normalize;
				pixel.blue  *= normalize;
			}
			else {
				for (i=0; i < n; i++) {
					j = (REBINT)((contribution[i].pixel * IMG_WIDE(source)) + x);
					weight       = contribution[i].weight;
					pixel.red   += weight * p[j].red;
					pixel.green += weight * p[j].green;
					pixel.blue  += weight * p[j].blue;
				}
				pixel.opacity = OpaqueOpacity;
			}
			REBINT pix = (y * IMG_WIDE(destination)) + x;
			q[pix].red     = RoundDoubleToQuantum(pixel.red);
			q[pix].green   = RoundDoubleToQuantum(pixel.green);
			q[pix].blue    = RoundDoubleToQuantum(pixel.blue);
			q[pix].opacity = RoundDoubleToQuantum(pixel.opacity);
		}
	}
}


REBSER *ResizeImage(const REBSER *image,const REBCNT columns,
					const REBCNT rows,const FilterTypes filter,
					const REBDEC blur, REBOOL has_alpha)
{
	REBDEC support;
	REBDEC x_factor;
	REBDEC x_support;
	REBDEC y_factor;
	REBDEC y_support;
	REBOOL order;
	REBSER *resized_image;
	REBSER *temp_image;
	REBSER *data_set;
	register REBINT i;
	static const FilterInfo
		filters[SincFilter+1] =
		{
			{ Box,            0.0   }, // UndefinedFilter
			{ Box,            0.0   }, // PointFilter
			{ Box,            0.5   }, // BoxFilter
			{ Triangle,       1.0   },
			{ Hermite,        1.0   },
			{ Hanning,        1.0   },
			{ Hamming,        1.0   },
			{ Blackman,       1.0   },
			{ Gaussian,       1.25  },
			{ Quadratic,      1.5   },
			{ Cubic,          2.0   },
			{ Catrom,         2.0   },
			{ Mitchell,       2.0   },
			{ Lanczos,        3.0   },
			{ BlackmanBessel, 3.2383},
			{ BlackmanSinc,   4.0   } 
		};

	// Validate input.
	if ( image == NULL
		|| (((int) filter < 0) && ((int) filter > SincFilter))
		|| (IMG_WIDE(image) == 0UL)
		|| (IMG_HIGH(image) == 0UL)
		|| (columns == 0UL)
		|| (rows == 0UL)
	) return NULL;

	// Prepare output and temporary image.

	resized_image = Make_Image(columns, rows, TRUE);

	if ((columns == IMG_WIDE(image)) && (rows == IMG_HIGH(image)) && (blur == 1.0)) {
		return resized_image;
	}

	order = columns*(IMG_HIGH(image)+rows) > rows*(IMG_WIDE(image)+columns);
	if (order)
		temp_image=Make_Image(columns, IMG_HIGH(image), TRUE);
	else
		temp_image=Make_Image(IMG_WIDE(image), rows, TRUE);

	// Allocate filter contribution info.

	x_factor=(REBDEC) columns / IMG_WIDE(image);
	y_factor=(REBDEC)    rows / IMG_HIGH(image);

	if (filter)
		i = (REBINT)filter;
	else {
		i = (REBINT)(
		  (has_alpha || ((x_factor*y_factor) > 1.0)) ? MitchellFilter : DefaultResizeFilter);
	}

	DEBUG_RESIZE_TRACE("Resizing image of size %lux%lu to %lux%lu using filter %lu\n",
				IMG_WIDE(image), IMG_HIGH(image), columns,rows, i);

	x_support=blur*MAX(1.0/x_factor,1.0)*filters[i].support;
	y_support=blur*MAX(1.0/y_factor,1.0)*filters[i].support;
	support=MAX(x_support,y_support);
	if (support < filters[i].support)
		support=filters[i].support;

	// Allocate contribution data set.
	
	data_set = Make_Series((REBCNT)(2.0*MAX(support,0.5)+3), sizeof(ContributionInfo), FALSE);
	//LABEL_SERIES(series, "ContributionInfo");

	// Resize image.
	
	if (order) {
		HorizontalFilter(image, temp_image, data_set, x_factor, &filters[i], blur, has_alpha);
		VerticalFilter(temp_image, resized_image, data_set, y_factor, &filters[i], blur, has_alpha);
	} else {
		VerticalFilter(image, temp_image, data_set, y_factor, &filters[i], blur, has_alpha);
		HorizontalFilter(temp_image, resized_image, data_set, x_factor, &filters[i], blur, has_alpha);
	}

	Free_Series(data_set);
	Free_Series(temp_image);
	return(resized_image);
}

#endif // INCLUDE_IMAGE_NATIVES