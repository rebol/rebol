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
**  Title: Rich-Text Dialect Backend
**  Author: Cyphre, Carl
**  Purpose: Evaluates rich-text commands; calls graphics functions.
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

#include <stdlib.h>
#include <string.h>

#include "reb-host.h"
///#include "reb-series.h"
///#include "reb-gob.h"
#include "host-lib.h"
#include "reb-types.h"
#include "reb-value.h"
#include "reb-dialect.h"
#include "words-text.h"
#include "words-graphics.h"
#include "rebol-lib.h"

#define ENABLE_DRAW
#include "agg-text.h"

//#define AGGC ((agg_graphics*)context)

#define ARG_LOGIC(n)	VAL_LOGIC(arg+n)
#define ARG_INTEGER(n)	VAL_INT32(arg+n)
#define ARG_STRING(n)	VAL_STRING(arg+n)
#define ARG_PAIR(n)		VAL_PAIR(arg+n)
#define ARG_DECIMAL(n)	VAL_DECIMAL(arg+n)
#define ARG_TUPLE(n)	VAL_TUPLE(arg+n)
#define ARG_WORD(n)		VAL_WORD(arg+n)

#define ARG_OBJECT(n)	VAL_SERIES(arg+n)  // temp
#define ARG_IMAGE(n)	VAL_SERIES(arg+n)  // temp
#define ARG_BLOCK(n)	VAL_BLOCK(arg+n)

#define ARG_WORDS(n,s,e) ((ARG_WORD(n)>=s && ARG_WORD(n)<=e) ? ARG_WORD(n)-s : 0)
#define ARG_OPT_LOGIC(n) (IS_LOGIC(arg+n)?VAL_LOGIC(arg+n):TRUE)
#define ARG_OPT_INTEGER(n) (IS_INTEGER(arg+n)?VAL_INT32(arg+n): 1)

void set_font_styles(REBFNT* font, REBVAL* val){
	REBINT result = Reb_Find_Word(VAL_WORD_SYM(val), Symbol_Ids, 0);
	switch (result){
		case SW_BOLD:
			font->bold = TRUE;
			break;
		case SW_ITALIC:
			font->italic = TRUE;
			break;
		case SW_UNDERLINE:
			font->underline = TRUE;
			break;
	}
}

/***********************************************************************
**
*/	REBINT Text_Gob(void *richtext, REBSER *block)
/*
**		Handles all commands for the TEXT dialect as specified
**		in the system/dialects/text object.
**
**		This function calls the REBOL_Dialect interpreter to
**		parse the dialect and build and return the command number
**		(the index offset in the text object above) and a block
**		of arguments. (For now, just a REBOL block, but this could
**		be changed to isolate it from changes in REBOL's internals).
**
**		Each arg will be of the specified datatype (given in the
**		dialect) or NONE when no argument of that type was given
**		and this code must determine the proper default value.
**
**		If the cmd result is zero, then it is either the end of
**		the block, or an error has occurred. If the error value
**		is non-zero, then it was an error.
**
***********************************************************************/
{
	REBCNT index = 0;
	REBINT cmd;
	REBSER *args = 0;
	REBVAL *arg;
	REBCNT nargs;

	//font object conversion related values
	REBFNT* font;
	REBVAL* val;
	REBPAR  offset;
	REBPAR  space;

	//para object conversion related values
	REBPRA* para;
	REBPAR  origin;
	REBPAR  margin;
	REBPAR  indent;
	REBPAR  scroll;

	do {
		cmd = Reb_Dialect(DIALECTS_TEXT, block, &index, &args);

		if (cmd == 0) return 0;
		if (cmd < 0) {
//			Reb_Print("ERROR: %d, Index %d", -cmd, index);
			return -((REBINT)index+1);
		}
//		else
//			Reb_Print("TEXT: Cmd %d, Index %d, Args %m", cmd, index, args);

		arg = BLK_HEAD(args);
		nargs = SERIES_TAIL(args);
//		Reb_Print("Number of args: %d", nargs);

		switch (cmd) {

		case TW_TYPE_SPEC:

			if (IS_STRING(arg)) {
				rt_text(richtext, ARG_STRING(0), index);
			} else if (IS_TUPLE(arg)) {
				rt_color(richtext, ARG_TUPLE(0));
			}
			break;
		case TW_ANTI_ALIAS:
			rt_anti_alias(richtext, ARG_OPT_LOGIC(0));
			break;

		case TW_SCROLL:
			rt_scroll(richtext, ARG_PAIR(0));
			break;

		case TW_BOLD:
		case TW_B:
			rt_bold(richtext, ARG_OPT_LOGIC(0));
			break;

		case TW_ITALIC:
		case TW_I:
			rt_italic(richtext, ARG_OPT_LOGIC(0));
			break;

		case TW_UNDERLINE:
		case TW_U:
			rt_underline(richtext, ARG_OPT_LOGIC(0));
			break;
		case TW_CENTER:
			rt_center(richtext);
			break;
		case TW_LEFT:
			rt_left(richtext);
			break;
		case TW_RIGHT:
			rt_right(richtext);
			break;
		case TW_FONT:

		if (!IS_OBJECT(arg)) break;

		font = (REBFNT*)rt_get_font(richtext);

		val = BLK_HEAD(ARG_OBJECT(0))+1;

		if (IS_STRING(val)) {
			font->name = VAL_STRING(val);
		}

//		Reb_Print("font/name: %s", font->name);

		val++;

		if (IS_BLOCK(val)) {
			REBSER* styles = VAL_SERIES(val);
			REBVAL* slot = BLK_HEAD(styles);
			REBCNT len = SERIES_TAIL(styles) ,i;

			for (i = 0;i<len;i++){
				if (IS_WORD(slot+i)){
					set_font_styles(font, slot+i);
				}
			}

		} else if (IS_WORD(val)) {
			set_font_styles(font, val);
		}

		val++;
		if (IS_INTEGER(val)) {
			font->size = VAL_INT32(val);
		}

//		Reb_Print("font/size: %d", font->size);

		val++;
		if ((IS_TUPLE(val)) || (IS_NONE(val))) {
			COPY_MEM(font->color,VAL_TUPLE(val), 4);
		}

//		Reb_Print("font/color: %d.%d.%d.%d", font->color[0],font->color[1],font->color[2],font->color[3]);

		val++;
		if ((IS_PAIR(val)) || (IS_NONE(val))) {
			offset = VAL_PAIR(val);
			font->offset_x = offset.x;
			font->offset_y = offset.y;
		}

//		Reb_Print("font/offset: %dx%d", offset.x,offset.y);

		val++;
		if ((IS_PAIR(val)) || (IS_NONE(val))) {
			space = VAL_PAIR(val);
			font->space_x = space.x;
			font->space_y = space.y;
		}

//		Reb_Print("font/space: %dx%d", space.x, space.y);


		val++;

		font->shadow_x = 0;
		font->shadow_y = 0;

		if (IS_BLOCK(val)) {
			REBSER* ser = VAL_SERIES(val);
			REBVAL* slot = BLK_HEAD(ser);
			REBCNT len = SERIES_TAIL(ser) ,i;

			for (i = 0;i<len;i++){
				if (IS_PAIR(slot)) {
					REBPAR shadow = VAL_PAIR(slot);
					font->shadow_x = shadow.x;
					font->shadow_y = shadow.y;
				} else if (IS_TUPLE(slot)) {
					COPY_MEM(font->shadow_color,VAL_TUPLE(slot), 4);
				} else if (IS_INTEGER(slot)) {
					font->shadow_blur = VAL_INT32(slot);
				}
				slot++;
			}
		} else if (IS_PAIR(val)) {
			REBPAR shadow = VAL_PAIR(val);
			font->shadow_x = shadow.x;
			font->shadow_y = shadow.y;
		}

			rt_font(richtext, font);
			break;

		case TW_PARA:
			if (!IS_OBJECT(arg)) break;

			para = (REBPRA*)rt_get_para(richtext);

			val = BLK_HEAD(ARG_OBJECT(0))+1;


			if (IS_PAIR(val)) {
				origin = VAL_PAIR(val);
				para->origin_x = origin.x;
				para->origin_y = origin.y;
			}

//			Reb_Print("para/origin: %dx%d", origin.x, origin.y);

			val++;
			if (IS_PAIR(val)) {
				margin = VAL_PAIR(val);
				para->margin_x = margin.x;
				para->margin_y = margin.y;
			}

//			Reb_Print("para/margin: %dx%d", margin.x, margin.y);

			val++;
			if (IS_PAIR(val)) {
				indent = VAL_PAIR(val);
				para->indent_x = indent.x;
				para->indent_y = indent.y;
			}

//			Reb_Print("para/indent: %dx%d", indent.x, indent.y);

			val++;
			if (IS_INTEGER(val)) {
				para->tabs = VAL_INT32(val);
			}

//			Reb_Print("para/tabs: %d", para->tabs);

			val++;
			if (IS_LOGIC(val)) {
				para->wrap = VAL_LOGIC(val);
			}

//			Reb_Print("para/wrap?: %d", para->wrap);

			val++;
			if (IS_PAIR(val)) {
				scroll = VAL_PAIR(val);
				para->scroll_x = scroll.x;
				para->scroll_y = scroll.y;
			}
//			Reb_Print("para/scroll: %dx%d", scroll.x, scroll.y);

			val++;

			if (IS_WORD(val)) {
				REBINT result = Reb_Find_Word(VAL_WORD_SYM(val), Symbol_Ids, 0);
				switch (result){
					case SW_RIGHT:
					case SW_LEFT:
					case SW_CENTER:
						para->align = result;
						break;
					default:
						para->align = SW_LEFT;
						break;
				}

			}

			val++;

			if (IS_WORD(val)) {
				REBINT result = Reb_Find_Word(VAL_WORD_SYM(val), Symbol_Ids, 0);
				switch (result){
					case SW_TOP:
					case SW_BOTTOM:
					case SW_MIDDLE:
						para->valign = result;
						break;
					default:
						para->valign = SW_TOP;
						break;
				}
			}

			rt_para(richtext, para);
			break;

		case TW_SIZE:
			rt_font_size(richtext, ARG_INTEGER(0));
			break;

		case TW_SHADOW:
			rt_shadow(richtext, &ARG_PAIR(0), ARG_TUPLE(1), ARG_INTEGER(2));
			break;

		case TW_DROP:
			rt_drop(richtext, ARG_OPT_INTEGER(0));
			break;

		case TW_NEWLINE:
		case TW_NL:
			rt_newline(richtext, index);
			break;
		case TW_CARET:
			{
				REBPAR caret = {0,0};
				REBPAR highlightStart = {0,0};
				REBPAR highlightEnd = {0,0};
				REBVAL *slot;
				if (!IS_OBJECT(arg)) break;

				val = BLK_HEAD(ARG_OBJECT(0))+1;
				if (IS_BLOCK(val)) {
					slot = BLK_HEAD(VAL_SERIES(val));
					if (SERIES_TAIL(VAL_SERIES(val)) == 2 && IS_BLOCK(slot) && IS_STRING(slot+1)){
						caret.x = 1 + slot->data.series.index;
						caret.y = 1 + (slot+1)->data.series.index;;
						//Reb_Print("caret %d, %d", caret.x, caret.y);
					}
				}
				val++;
				if (IS_BLOCK(val)) {
					slot = BLK_HEAD(VAL_SERIES(val));
					if (SERIES_TAIL(VAL_SERIES(val)) == 2 && IS_BLOCK(slot) && IS_STRING(slot+1)){
						highlightStart.x = 1 + slot->data.series.index;
						highlightStart.y = 1 + (slot+1)->data.series.index;;
						//Reb_Print("highlight-start %d, %d", highlightStart.x, highlightStart.y);
					}
				}
				val++;
				if (IS_BLOCK(val)) {
					slot = BLK_HEAD(VAL_SERIES(val));
					if (SERIES_TAIL(VAL_SERIES(val)) == 2 && IS_BLOCK(slot) && IS_STRING(slot+1)){
						highlightEnd.x = 1 + slot->data.series.index;
						highlightEnd.y = 1 + (slot+1)->data.series.index;;
						//Reb_Print("highlight-End %d, %d", highlightEnd.x, highlightEnd.y);
					}
				}

				rt_caret(richtext, &caret, &highlightStart,&highlightEnd);
			}
			break;
		}
	} while (TRUE);
}

