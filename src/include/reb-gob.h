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
**  Summary: Graphical compositing objects
**  Module:  reb-gob.h
**  Author:  Carl Sassenrath
**  Description:
**		GOBs are lower-level graphics object used by the compositing
**		and rendering system. Because a GUI can contain thousands of
**		GOBs, they are designed and structured to be simple and small.
**		Note that GOBs are also used for windowing.
**	Warning:
**		GOBs are allocated from a special pool and
**		are accounted for by the standard garbage collector.
**
***********************************************************************/

enum GOB_FLAGS {		// GOB attribute and option flags
	GOBF_TOP = 0,		// Top level (window or output image)
	GOBF_WINDOW,		// Window (parent is OS window reference)
	GOBF_OPAQUE,		// Has no alpha
	GOBF_STATIC,		// Does not change
	GOBF_HIDDEN,		// Is hidden (e.g. hidden window)
	GOBF_RESIZE,		// Can be resized
	GOBF_NO_TITLE,		// Has window title
	GOBF_NO_BORDER,		// Has window border
	GOBF_DROPABLE,		// Let window receive drag and drop
	GOBF_TRANSPARENT,	// Window is in transparent mode
	GOBF_POPUP,			// Window is a popup (with owner window)
	GOBF_MODAL,			// Modal event filtering
	GOBF_ON_TOP,		// The window is always on top
};

enum GOB_STATE {		// GOB state flags
	GOBS_OPEN = 0,		// Window is open
	GOBS_ACTIVE,		// Window is active
	GOBS_NEW,			// Gob is new to pane (old-offset, old-size wrong)
};

enum GOB_TYPES {		// Types of content
	GOBT_NONE = 0,
	GOBT_COLOR,
	GOBT_IMAGE,
	GOBT_STRING,
	GOBT_DRAW,
	GOBT_TEXT,
	GOBT_EFFECT,
	GOBT_MAX
};

enum GOB_DTYPES {		// Userdata types
	GOBD_NONE = 0,
	GOBD_OBJECT,
	GOBD_BLOCK,
	GOBD_STRING,
	GOBD_BINARY,
	GOBD_RESV,			// unicode
	GOBD_INTEGER
};


#pragma pack(4)

typedef struct rebol_gob REBGOB;

struct rebol_gob {		// size: 64 bytes!
	REBCNT flags;		// option flags
	REBCNT state;		// state flags

	REBSER *pane;		// List of child GOBs
	REBGOB *parent;		// Parent GOB (or window ptr)

	REBYTE alpha;		// transparency
	REBYTE ctype;		// content data type
	REBYTE dtype;		// pointer data type
	REBYTE resv;		// reserved

	union {
		REBGOB *owner;	// temp field - reused for different things
	};

	REBSER *content;	// content value (block, string, color)
	REBSER *data;		// user defined data

	REBXYF offset;		// location
	REBXYF size;
	REBXYF old_offset;	// prior location
	REBXYF old_size;	// prior size
};
#pragma pack()

#define GOB_X(g)		((g)->offset.x)
#define GOB_Y(g)		((g)->offset.y)
#define GOB_W(g)		((g)->size.x)
#define GOB_H(g)		((g)->size.y)

#define GOB_X_INT(g)	ROUND_TO_INT((g)->offset.x)
#define GOB_Y_INT(g)	ROUND_TO_INT((g)->offset.y)
#define GOB_W_INT(g)	ROUND_TO_INT((g)->size.x)
#define GOB_H_INT(g)	ROUND_TO_INT((g)->size.y)

#define GOB_XO(g)		((g)->old_offset.x)
#define GOB_YO(g)		((g)->old_offset.y)
#define GOB_WO(g)		((g)->old_size.x)
#define GOB_HO(g)		((g)->old_size.y)
#define GOB_XO_INT(g)	ROUND_TO_INT((g)->old_offset.x)
#define GOB_YO_INT(g)	ROUND_TO_INT((g)->old_offset.y)
#define GOB_WO_INT(g)	ROUND_TO_INT((g)->old_size.x)
#define GOB_HO_INT(g)	ROUND_TO_INT((g)->old_size.y)

#define CLEAR_GOB_STATE(g) ((g)->state = 0)

#define SET_GOB_FLAG(g,f)  SET_FLAG((g)->flags, f)
#define GET_GOB_FLAG(g,f)  GET_FLAG((g)->flags, f)
#define CLR_GOB_FLAG(g,f)  CLR_FLAG((g)->flags, f)
#define SET_GOB_STATE(g,f) SET_FLAG((g)->state, f)
#define GET_GOB_STATE(g,f) GET_FLAG((g)->state, f)
#define CLR_GOB_STATE(g,f) CLR_FLAG((g)->state, f)

#define GOB_ALPHA(g)		((g)->alpha)
#define GOB_TYPE(g)			((g)->ctype)
#define SET_GOB_TYPE(g,t)	((g)->ctype = (t))
#define GOB_DTYPE(g)		((g)->dtype)
#define SET_GOB_DTYPE(g,t)	((g)->dtype = (t))
#define GOB_DATA(g)			((g)->data)
#define SET_GOB_DATA(g,v)	((g)->data = (v))
#define GOB_TMP_OWNER(g)	((g)->owner)

#define IS_GOB_OPAQUE(g)  GET_GOB_FLAG(g, GOBF_OPAQUE)
#define SET_GOB_OPAQUE(g) SET_GOB_FLAG(g, GOBF_OPAQUE)
#define CLR_GOB_OPAQUE(g) CLR_GOB_FLAG(g, GOBF_OPAQUE)

#define GOB_PANE(g)		((g)->pane)
#define GOB_PARENT(g)	((g)->parent)
#define GOB_CONTENT(g)	((g)->content)

// Control dependencies on series structures:
#ifdef REB_DEF
#define GOB_STRING(g)	SERIES_DATA(GOB_CONTENT(g))
#define GOB_TAIL(g)		SERIES_TAIL((g)->pane)
#define GOB_HEAD(g)		((REBGOB **)(SERIES_DATA(GOB_PANE(g))))
#else
#define GOB_STRING(g)	((REBYTE *)RL_Series(GOB_CONTENT(g), (REBCNT)RXI_SER_DATA))
#define GOB_TAIL(g)		((REBCNT)RL_Series(GOB_PANE(g), (REBCNT)RXI_SER_TAIL))
#define GOB_HEAD(g)		((REBGOB **)RL_Series(GOB_PANE(g), (REBCNT)RXI_SER_DATA))
#endif
#define GOB_BITMAP(g)	GOB_STRING(g)
#define GOB_SKIP(g,n)	(GOB_HEAD(g)+n)

#define IS_WINDOW(g)	(GOB_PARENT(g) == Gob_Root && GET_GOB_FLAG(g, GOBF_WINDOW))

#define IS_GOB_COLOR(g)	 (GOB_TYPE(g) == GOBT_COLOR)
#define IS_GOB_DRAW(g)	 (GOB_CONTENT(g) && GOB_TYPE(g) == GOBT_DRAW)
#define IS_GOB_IMAGE(g)	 (GOB_CONTENT(g) && GOB_TYPE(g) == GOBT_IMAGE)
#define IS_GOB_EFFECT(g) (GOB_CONTENT(g) && GOB_TYPE(g) == GOBT_EFFECT)
#define IS_GOB_STRING(g) (GOB_CONTENT(g) && GOB_TYPE(g) == GOBT_STRING)
#define IS_GOB_TEXT(g)   (GOB_CONTENT(g) && GOB_TYPE(g) == GOBT_TEXT)

// GC Flags:
enum {
	GOB_MARK = 1,		// Gob was found during GC mark scan.
	GOB_USED = 1<<1		// Gob is used (not free).
};

#define IS_GOB_MARK(g)	((g)->resv & GOB_MARK)
#define MARK_GOB(g)		((g)->resv |= GOB_MARK)
#define UNMARK_GOB(g)	((g)->resv &= ~GOB_MARK)
#define IS_GOB_USED(g)	((g)->resv & GOB_USED)
#define USE_GOB(g)		((g)->resv |= GOB_USED)
#define FREE_GOB(g)		((g)->resv &= ~GOB_USED)

extern REBGOB *Gob_Root; // Top level GOB (the screen)
