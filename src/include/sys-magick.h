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
**  Summary: GraphicMagick related defines and declarations
**  Module:  sys-magick.h
**  Section: utility
**  Author:  Oldes (ported (stripped) code from GraphicsMagick - see bellow)
**
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

// Options

#define MAX_RESIZE_BLUR 100.0
#define DefaultResizeFilter LanczosFilter
#define DefaultThumbnailFilter BoxFilter
#define MagickEpsilon 1.0e-12
#define MagickPI 3.14159265358979323846264338327950288419716939937510


// Typedef declarations.

typedef struct _ContributionInfo {
	REBDEC weight;
	REBINT pixel;
} ContributionInfo;

typedef struct _FilterInfo {
	REBDEC (*function)(const REBDEC,const REBDEC);
	REBDEC support;
} FilterInfo;

typedef unsigned char Quantum;
typedef Quantum IndexPacket;
typedef struct _PixelPacket
{
#ifdef ENDIAN_LITTLE
  /* BGRA (as used by Microsoft Windows DIB) */
#define MAGICK_PIXELS_BGRA 1
  Quantum
    blue,
    green,
    red,
    opacity;
#else
  /* RGBA */
#define MAGICK_PIXELS_RGBA 1
  Quantum
    red,
    green,
    blue,
    opacity;
#endif
} PixelPacket;

typedef struct _DoublePixelPacket
{
  double
    red,
    green,
    blue,
    opacity;
} DoublePixelPacket;

typedef enum
{
  UndefinedFilter,
  PointFilter,
  BoxFilter,
  TriangleFilter,
  HermiteFilter,
  HanningFilter,
  HammingFilter,
  BlackmanFilter,
  GaussianFilter,
  QuadraticFilter,
  CubicFilter,
  CatromFilter,
  MitchellFilter,
  LanczosFilter,
  BesselFilter,
  SincFilter
} FilterTypes;



REBSER *ResizeImage(const REBSER *image,const REBCNT columns,
					const REBCNT rows,const FilterTypes filter,
					const REBDEC blur, REBOOL has_alpha);


// Defines

#ifndef ABS
#define ABS(x)  ((x) < 0 ? -(x) : (x))
#endif

# define DEBUG_RESIZE_TRACE(...) //printf(__VA_ARGS__)

// Rebol is using only 8 bits (4 bytes per color) so far

//#if (QuantumDepth == 8)
#  define MaxColormapSize 256U
#  define MaxMap          255U
#  define MaxMapDepth       8
#  define MaxMapFloat     255.0f
#  define MaxMapDouble    255.0
#  define MaxRGB          255U
#  define MaxRGBFloat     255.0f
#  define MaxRGBDouble    255.0

#define OpaqueOpacity  MaxRGB
#define TransparentOpacity  0UL

#define RoundDoubleToQuantum(value) ((Quantum) (value < 0.0 ? 0U : \
  (value > MaxRGBDouble) ? MaxRGB : value + 0.5))

