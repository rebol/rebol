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
**  Summary: REBOL series structure
**  Module:  reb-series.h
**  Author:  Carl Sassenrath
**  Notes:
**      WARNING: struct size may change -- do not malloc()
**
***********************************************************************/

typedef struct rebol_series {
	REBYTE *data;
	REBCNT tail;
	REBCNT rest;
	REBINT info;
	REBCNT size; // Temp - size of image w/h
	// OPTIONAL Extensions
} REBSER;

#define SERIES_TAIL(s) ((s)->tail)
#define SERIES_DATA(s) ((s)->data)

#define BLK_HEAD(s) ((REBVAL *)((s)->data))
#define STR_HEAD(s) ((REBYTE *)((s)->data))

#define IMG_SIZE(s) ((s)->size)
#define IMG_WIDE(s) ((s)->size & 0xffff)
#define IMG_HIGH(s) ((s)->size >> 16)
#define IMG_DATA(s) ((REBYTE *)((s)->data))
