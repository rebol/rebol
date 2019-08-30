/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  REBOL is a trademark of REBOL Technologies
**
**  Additional code modifications and improvements:
**	Copyright 2012-2018 Saphirion AG & Atronix
**	Copyright 2019 Oldes
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
**  Title: Compositor context definition
**  Author: Oldes
**
************************************************************************
**
**  NOTE to PROGRAMMERS:
**
**    1. Keep code clear and simple.
**    2. Document unusual code, reasoning, or gotchas.
**    3. Use same style for code, vars, indent(4), comments, etc.
**    4. Keep in mind Linux, OS X, BSD, big/little endian CPUs.
**    5. Test everything, then test it again.
**
***********************************************************************/

typedef struct rebol_compositor_ctx {
	REBGOB    *root_gob;
	REBGOB    *wind_gob;
	REBXYI     wind_size;
	REBYTE    *wind_buffer; // can be used for conversion to image; was also used with AGG
	#ifdef TO_WINDOWS
		HDC        wind_DC;
		HBITMAP    back_buffer;
		HDC        back_DC;
		HRGN       win_clip;
		HRGN       new_clip;
		HRGN       old_clip;
		REBXYF     abs_offset;
		HBRUSH     brush_DC;
		BITMAPINFO bmpInfo;
		RECT       win_rect;
		REBCNT     flags; // base flags:
	#endif
} REBCMP;

