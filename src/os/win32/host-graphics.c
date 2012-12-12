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
**  Title: Graphics Compositing
**  Author: Cyphre, Carl
**  Purpose: Interface from graphics commands to AGG library.
**  Tools: make-host-ext.r
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

#include <windows.h>

#include "reb-host.h"
#include "host-lib.h"

#include "agg-draw.h"
#include "agg-text.h"

#define INCLUDE_EXT_DATA
#include "host-ext-graphics.h"
#include "host-ext-draw.h"
#include "host-ext-shape.h"
#include "host-ext-text.h"

//***** Externs *****

extern REBINT As_OS_Str(REBSER *series, REBCHR **string);
extern REBINT Show_Gob(REBGOB *gob);
extern HCURSOR Cursor;
static REBOOL Custom_Cursor = FALSE;

static u32* draw_ext_words;
static u32* shape_ext_words;
static u32* text_ext_words;
static u32* graphics_ext_words;

void* Rich_Text;

RL_LIB *RL; // Link back to reb-lib from embedded extensions

/***********************************************************************
**
*/	HCURSOR Image_To_Cursor(REBYTE* image, REBINT width, REBINT height)
/*
**      Converts REBOL image! to Windows CURSOR
**
***********************************************************************/
{
	int xHotspot = 0;
	int yHotspot = 0;

	HICON result = NULL;
	HBITMAP hSourceBitmap;
	BITMAPINFO  BitmapInfo;
	ICONINFO iconinfo;

    //Get the system display DC
    HDC hDC = GetDC(NULL);

	//Create DIB
	unsigned char* ppvBits;
	int bmlen = width * height * 4;
	int i;

	BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BitmapInfo.bmiHeader.biWidth = width;
	BitmapInfo.bmiHeader.biHeight = -(signed)height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	BitmapInfo.bmiHeader.biSizeImage = 0;
	BitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	BitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	BitmapInfo.bmiHeader.biClrUsed = 0;
	BitmapInfo.bmiHeader.biClrImportant = 0;

	hSourceBitmap = CreateDIBSection(hDC, &BitmapInfo, DIB_RGB_COLORS, (void**)&ppvBits, NULL, 0);

	//Release the system display DC
    ReleaseDC(NULL, hDC);

	//Copy the image content to DIB
	COPY_MEM(ppvBits, image, bmlen);

	//Invert alphachannel from the REBOL format
	for (i = 3;i < bmlen;i+=4){
		ppvBits[i] ^= 0xff;
	}

	//Create the cursor using the masks and the hotspot values provided
	iconinfo.fIcon		= FALSE;
	iconinfo.xHotspot	= xHotspot;
	iconinfo.yHotspot	= yHotspot;
	iconinfo.hbmMask	= hSourceBitmap;
	iconinfo.hbmColor	= hSourceBitmap;

	result = CreateIconIndirect(&iconinfo);

	DeleteObject(hSourceBitmap);

	return result;
}

/***********************************************************************
**
*/	REBINT Draw_Gob(void *graphics, REBSER *block, REBSER *args)
/*
**		Temporary trampoline. Move to compositor!
**
***********************************************************************/
{
   	REBCEC ctx;

	ctx.envr = graphics;
	ctx.block = block;
	ctx.index = 0;

	RL_Do_Commands(block, 0, &ctx);
	return 0;
}


/***********************************************************************
**
*/	REBINT Text_Gob(void *richtext, REBSER *block)
/*
**		Temporary trampoline. Move to compositor!
**
***********************************************************************/
{
   	REBCEC ctx;

	ctx.envr = richtext;
	ctx.block = block;
	ctx.index = 0;

	RL_Do_Commands(block, 0, &ctx);
	return 0;
}

/***********************************************************************
**
*/	RXIEXT int RXD_Graphics(int cmd, RXIFRM *frm, REBCEC *data)
/*
**		Graphics command extension dispatcher.
**
***********************************************************************/
{
	switch (cmd) {

	case CMD_GRAPHICS_SHOW:
		Show_Gob((REBGOB*)RXA_SERIES(frm, 1));
		RXA_TYPE(frm, 1) = RXT_GOB;
		return RXR_VALUE;

    case CMD_GRAPHICS_SIZE_TEXT:
        if (Rich_Text) {
            RXA_TYPE(frm, 2) = RXT_PAIR;
            rt_size_text(Rich_Text, (REBGOB*)RXA_SERIES(frm, 1),&RXA_PAIR(frm, 2));
            RXA_PAIR(frm, 1).x = RXA_PAIR(frm, 2).x;
            RXA_PAIR(frm, 1).y = RXA_PAIR(frm, 2).y;
            RXA_TYPE(frm, 1) = RXT_PAIR;
            return RXR_VALUE;
        }

        break;

    case CMD_GRAPHICS_OFFSET_TO_CARET:
        if (Rich_Text) {
            REBINT element = 0, position = 0;
            REBSER* dialect;
            REBSER* block;
            RXIARG val; //, str;
            REBCNT n, type;

            rt_offset_to_caret(Rich_Text, (REBGOB*)RXA_SERIES(frm, 1), RXA_PAIR(frm, 2), &element, &position);
//            RL_Print("OTC: %d, %d\n", element, position);
            dialect = (REBSER *)GOB_CONTENT((REBGOB*)RXA_SERIES(frm, 1));
            block = RL_MAKE_BLOCK(RL_SERIES(dialect, RXI_SER_TAIL));
            for (n = 0; type = RL_GET_VALUE(dialect, n, &val); n++) {
                if (n == element) val.index = position;
                RL_SET_VALUE(block, n, val, type);
            }

            RXA_TYPE(frm, 1) = RXT_BLOCK;
            RXA_SERIES(frm, 1) = block;
            RXA_INDEX(frm, 1) = element;

            return RXR_VALUE;
        }

        break;

    case CMD_GRAPHICS_CARET_TO_OFFSET:
        if (Rich_Text) {
            REBXYF result;
            REBINT elem,pos;
            if (RXA_TYPE(frm, 2) == RXT_INTEGER){
                elem = RXA_INT64(frm, 2)-1;
            } else {
                elem = RXA_INDEX(frm, 2);
            }
            if (RXA_TYPE(frm, 3) == RXT_INTEGER){
                pos = RXA_INT64(frm, 3)-1;
            } else {
                pos = RXA_INDEX(frm, 3);
            }
//            RL_Print("CTO: %d, %d\n", element, position);
            rt_caret_to_offset(Rich_Text, (REBGOB*)RXA_SERIES(frm, 1), &result, elem, pos);

            RXA_PAIR(frm, 1).x = result.x;
            RXA_PAIR(frm, 1).y = result.y;
            RXA_TYPE(frm, 1) = RXT_PAIR;
            return RXR_VALUE;
        }
        break;

    case CMD_GRAPHICS_CURSOR:
        {
            REBINT n = 0;
            REBSER image = 0;

            if (RXA_TYPE(frm, 1) == RXT_IMAGE) {
                image = RXA_IMAGE_BITS(frm,1);
            } else {
                n = RXA_INT64(frm,1);
            }

            if (Custom_Cursor) {
                //Destroy cursor object only if it is a custom image
                DestroyCursor(Cursor);
                Custom_Cursor = FALSE;
            }

            if (n > 0)
                Cursor = LoadCursor(NULL, (LPCTSTR)n);
            else if (image) {
                Cursor = Image_To_Cursor(image, RXA_IMAGE_WIDTH(frm,1), RXA_IMAGE_HEIGHT(frm,1));
                Custom_Cursor = TRUE;
            } else
                Cursor = NULL;

            SetCursor(Cursor);

        }
        break;

    case CMD_GRAPHICS_DRAW:
        {
            REBYTE* img = 0;
            REBINT w,h;
            if (RXA_TYPE(frm, 1) == RXT_IMAGE) {
                img = RXA_IMAGE_BITS(frm, 1);
                w = RXA_IMAGE_WIDTH(frm, 1);
                h = RXA_IMAGE_HEIGHT(frm, 1);
            } else {
                REBSER* i;
                w = RXA_PAIR(frm,1).x;
                h = RXA_PAIR(frm,1).y;
                i = RL_MAKE_IMAGE(w,h);
                img = (REBYTE *)RL_SERIES(i, RXI_SER_DATA);

                RXA_TYPE(frm, 1) = RXT_IMAGE;
                RXA_ARG(frm, 1).width = w;
                RXA_ARG(frm, 1).height = h;
                RXA_ARG(frm, 1).image = i;
            }
            Draw_Image(img, w, h, RXA_SERIES(frm, 2));
            return RXR_VALUE;
        }
        break;

    case CMD_GRAPHICS_GUI_METRIC:
        {
            REBINT x,y;
            u32 w = RL_FIND_WORD(graphics_ext_words,RXA_WORD(frm, 1));

            switch(w)
            {
                case W_GRAPHICS_SCREEN_SIZE:
                    x = GetSystemMetrics(SM_CXSCREEN);
                    y = GetSystemMetrics(SM_CYSCREEN);
                    break;

                case W_GRAPHICS_TITLE_SIZE:
                    x = 0;
                    y = GetSystemMetrics(SM_CYCAPTION);
                    break;

                case W_GRAPHICS_BORDER_SIZE:
                    x = GetSystemMetrics(SM_CXSIZEFRAME);
                    y = GetSystemMetrics(SM_CYSIZEFRAME);
                    break;

                case W_GRAPHICS_BORDER_FIXED:
                    x = GetSystemMetrics(SM_CXFIXEDFRAME);
                    y = GetSystemMetrics(SM_CYFIXEDFRAME);
                    break;

                case W_GRAPHICS_WORK_ORIGIN:
                    {
                        RECT rect;
                        SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
                        x = rect.left;
                        y = rect.top;
                    }
                    break;

                case W_GRAPHICS_WORK_SIZE:
                    {
                        RECT rect;
                        SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
                        x = rect.right;
                        y = rect.bottom;
                    }
                    break;
            }

            if (w){
                RXA_PAIR(frm, 1).x = x;
                RXA_PAIR(frm, 1).y = y;
                RXA_TYPE(frm, 1) = RXT_PAIR;
            } else {
                RXA_TYPE(frm, 1) = RXT_NONE;
            }
            return RXR_VALUE;
        }
        break;

	case CMD_GRAPHICS_INIT:
		Gob_Root = (REBGOB*)RXA_SERIES(frm, 1); // system/view/screen-gob
		Gob_Root->size.x = (REBD32)GetSystemMetrics(SM_CXSCREEN);
		Gob_Root->size.y = (REBD32)GetSystemMetrics(SM_CYSCREEN);

		//Initialize text rendering context
		if (Rich_Text) Destroy_RichText(Rich_Text);
		Rich_Text = Create_RichText();

		break;

    case CMD_GRAPHICS_INIT_WORDS:
        //temp hack - will be removed later
        graphics_ext_words = RL_MAP_WORDS(RXA_SERIES(frm,1));
        break;

	default:
		return RXR_NO_COMMAND;
	}
    return RXR_UNSET;
}

/***********************************************************************
**
*/	RXIEXT int RXD_Text(int cmd, RXIFRM *frm, REBCEC *ctx)
/*
**		DRAW command dispatcher.
**
***********************************************************************/
{
	switch (cmd) {

    case CMD_TEXT_INIT_WORDS:
        //temp hack - will be removed later
        text_ext_words = RL_MAP_WORDS(RXA_SERIES(frm,1));
        break;

    case CMD_TEXT_ANTI_ALIAS:
        rt_anti_alias(ctx->envr, RXA_LOGIC(frm, 1));
        break;

    case CMD_TEXT_BOLD:
        rt_bold(ctx->envr, RXA_LOGIC(frm, 1));
        break;

    case CMD_TEXT_CARET:
        {
            RXIARG val;
            u32 *words, *w;
            REBSER *obj;
            REBCNT type;
            REBXYF caret, highlightStart, highlightEnd;
            REBXYF *pcaret = 0, *phighlightStart = 0;
            obj = RXA_OBJECT(frm, 1);
//Reb_Print("RXI_WORDS_OF_OBJECT() called\n");
            words = RL_WORDS_OF_OBJECT(obj);
//Reb_Print("RXI_WORDS_OF_OBJECT() OK\n");
            w = words;

            while (type = RL_GET_FIELD(obj, w[0], &val))
            {
//                RL_Print("word: %d %d %d\n", w[0],w[1], (REBYTE)w[1]);
                switch(RL_FIND_WORD(text_ext_words,w[0]))
                {
                    case W_TEXT_CARET:
                        if (type == RXT_BLOCK){
                            REBSER* block = val.series;
                            REBINT len = RL_SERIES(block, RXI_SER_TAIL);
                            if (len > 1){
                                RXIARG pos, elem;
                                if (
                                    RL_GET_VALUE(block, 0, &pos) == RXT_BLOCK &&
                                    RL_GET_VALUE(block, 1, &elem) == RXT_STRING
                                ){
                                    caret.x = 1 + pos.index;
                                    caret.y = 1 + elem.index;
                                    pcaret = &caret;
                                }
                            }
                        }
                        break;

                    case W_TEXT_HIGHLIGHT_START:
                        if (type == RXT_BLOCK){
                            REBSER* block = val.series;
                            REBINT len = RL_SERIES(block, RXI_SER_TAIL);
                            if (len > 1){
                                RXIARG pos, elem;
                                if (
                                    RL_GET_VALUE(block, 0, &pos) == RXT_BLOCK &&
                                    RL_GET_VALUE(block, 1, &elem) == RXT_STRING
                                ){
                                    highlightStart.x = 1 + pos.index;
                                    highlightStart.y = 1 + elem.index;
                                    phighlightStart = &highlightStart;
                                }
                            }
                        }
                        break;

                    case W_TEXT_HIGHLIGHT_END:
                        if (type == RXT_BLOCK){
                            REBSER* block = val.series;
                            REBINT len = RL_SERIES(block, RXI_SER_TAIL);
                            if (len > 1){
                                RXIARG pos, elem;
                                if (
                                    RL_GET_VALUE(block, 0, &pos) == RXT_BLOCK &&
                                    RL_GET_VALUE(block, 1, &elem) == RXT_STRING
                                ){
                                    highlightEnd.x = 1 + pos.index;
                                    highlightEnd.y = 1 + elem.index;
                                }
                            }
                        }
                        break;
                }

                w++;
            }
            OS_Free(words);
            rt_caret(ctx->envr, pcaret, phighlightStart, highlightEnd);
        }

        break;

    case CMD_TEXT_CENTER:
        rt_center(ctx->envr);
        break;

    case CMD_TEXT_COLOR:
        rt_color(ctx->envr, RXA_TUPLE(frm,1) + 1);
       break;

    case CMD_TEXT_DROP:
        rt_drop(ctx->envr, RXA_INT32(frm,1));
        break;

    case CMD_TEXT_FONT:
        {
            RXIARG val;
            u32 *words,*w;
            REBSER *obj;
            REBCNT type;
            REBFNT *font = rt_get_font(ctx->envr);

            obj = RXA_OBJECT(frm, 1);
            words = RL_WORDS_OF_OBJECT(obj);
            w = words;

            while (type = RL_GET_FIELD(obj, w[0], &val))
            {
                switch(RL_FIND_WORD(text_ext_words,w[0]))
                {
                    case W_TEXT_NAME:
                        if (type == RXT_STRING){
                            font->name_gc = As_OS_Str(val.series, &(font->name));
                        }
                        break;

                    case W_TEXT_STYLE:
                        switch(type)
                        {
                            case RXT_WORD:
                            {
                                u32 styleWord = RL_FIND_WORD(text_ext_words,val.int32a);
                                if (styleWord) rt_set_font_styles(font, styleWord);
                            }
                            break;

                            case RXT_BLOCK:
                            {
                                RXIARG styleVal;
                                REBCNT styleType;
                                REBCNT n;
                                u32 styleWord;
                                for (n = 0; styleType = RL_GET_VALUE(val.series, n, &styleVal); n++) {
                                    if (styleType == RXT_WORD) {
                                        styleWord = RL_FIND_WORD(text_ext_words,styleVal.int32a);
                                        if (styleWord) rt_set_font_styles(font, styleWord);
                                    }
                                }
                            }
                            break;
                        }
                        break;

                    case W_TEXT_SIZE:
                        if (type == RXT_INTEGER)
                            font->size = val.int64;
                        break;

                    case W_TEXT_COLOR:
                        if (type == RXT_TUPLE)
                            memcpy(font->color,val.bytes + 1 , 4);
                        break;

                    case W_TEXT_OFFSET:
                        if (type == RXT_PAIR) {
                            font->offset_x = val.pair.x;
                            font->offset_y = val.pair.y;
                        }
                        break;

                    case W_TEXT_SPACE:
                        if (type == RXT_PAIR) {
                            font->space_x = val.pair.x;
                            font->space_y = val.pair.y;
                        }
                        break;

                    case W_TEXT_SHADOW:
                        switch(type)
                        {
                            case RXT_PAIR:
                            {
                                font->shadow_x = val.pair.x;
                                font->shadow_y = val.pair.y;
                            }
                            break;

                            case RXT_BLOCK:
                            {
                                RXIARG shadowVal;
                                REBCNT shadowType;
                                REBCNT n;
                                for (n = 0; shadowType = RL_GET_VALUE(val.series, n, &shadowVal); n++) {
                                    switch (shadowType)
                                    {
                                        case RXT_PAIR:
                                            font->shadow_x = shadowVal.pair.x;
                                            font->shadow_y = shadowVal.pair.y;
                                            break;

                                        case RXT_TUPLE:
                                            memcpy(font->shadow_color,shadowVal.bytes + 1 , 4);
                                            break;

                                        case RXT_INTEGER:
                                            font->shadow_blur = shadowVal.int64;
                                            break;
                                    }
                                }
                            }
                            break;
                        }
                        break;
                }

                w++;
            }
            OS_Free(words);
            rt_font(ctx->envr, font);
        }
        break;

    case CMD_TEXT_ITALIC:
        rt_italic(ctx->envr, RXA_LOGIC(frm, 1));
        break;

    case CMD_TEXT_LEFT:
        rt_left(ctx->envr);
        break;

	case CMD_TEXT_NEWLINE:
        rt_newline(ctx->envr, ctx->index + 1);
		break;

    case CMD_TEXT_PARA:
        {
            RXIARG val;
            u32 *words,*w;
            REBSER *obj;
            REBCNT type;
            REBPRA *para = rt_get_para(ctx->envr);

            obj = RXA_OBJECT(frm, 1);
            words = RL_WORDS_OF_OBJECT(obj);
            w = words;

            while (type = RL_GET_FIELD(obj, w[0], &val))
            {
                switch(RL_FIND_WORD(text_ext_words,w[0]))
                {
                    case W_TEXT_ORIGIN:
                       if (type == RXT_PAIR) {
                            para->origin_x = val.pair.x;
                            para->origin_y = val.pair.y;
                        }
                        break;
                    case W_TEXT_MARGIN:
                       if (type == RXT_PAIR) {
                            para->margin_x = val.pair.x;
                            para->margin_y = val.pair.y;
                        }
                        break;
                    case W_TEXT_INDENT:
                       if (type == RXT_PAIR) {
                            para->indent_x = val.pair.x;
                            para->indent_y = val.pair.y;
                        }
                        break;
                    case W_TEXT_TABS:
                       if (type == RXT_INTEGER) {
                            para->tabs = val.int64;
                        }
                        break;
                    case W_TEXT_WRAPQ:
                       if (type == RXT_LOGIC) {
                            para->wrap = val.int32a;
                        }
                        break;
                    case W_TEXT_SCROLL:
                       if (type == RXT_PAIR) {
                            para->scroll_x = val.pair.x;
                            para->scroll_y = val.pair.y;
                        }
                        break;
                    case W_TEXT_ALIGN:
                        if (type == RXT_WORD) {
                            para->align = RL_FIND_WORD(text_ext_words,val.int32a);
                        }
                        break;
                    case W_TEXT_VALIGN:
                        if (type == RXT_WORD) {
                            para->valign = RL_FIND_WORD(text_ext_words,val.int32a);
                        }
                        break;
                }

                w++;
            }
            OS_Free(words);
            rt_para(ctx->envr, para);
        }
        break;

    case CMD_TEXT_RIGHT:
        rt_right(ctx->envr);
        break;

	case CMD_TEXT_SCROLL:
		rt_scroll(ctx->envr, RXA_PAIR(frm, 1));
		break;

    case CMD_TEXT_SHADOW:
        rt_shadow(ctx->envr, RXA_PAIR(frm, 1), RXA_TUPLE(frm,2) + 1, RXA_INT32(frm,3));
        break;

	case CMD_TEXT_SIZE:
		rt_font_size(ctx->envr, RXA_INT32(frm,1));
		break;

    case CMD_TEXT_TEXT:
        {
            REBCHR* str;
            REBOOL gc = As_OS_Str(RXA_SERIES(frm, 1), &str);
            rt_text(ctx->envr, str, ctx->index + 2, gc);
        }
        break;

    case CMD_TEXT_UNDERLINE:
        rt_underline(ctx->envr, RXA_LOGIC(frm, 1));
		break;

	default:
		return RXR_NO_COMMAND;
	}
    return RXR_UNSET;
}


/***********************************************************************
**
*/	RXIEXT int RXD_Shape(int cmd, RXIFRM *frm, REBCEC *ctx)
/*
**		DRAW command dispatcher.
**
***********************************************************************/
{
//    Reb_Print("SHAPE called\n");
    REBCNT rel = 0;

	switch (cmd) {

    case CMD_SHAPE_INIT_WORDS:
        //temp hack - will be removed later
        shape_ext_words = RL_MAP_WORDS(RXA_SERIES(frm,1));
        break;

    case CMD_SHAPE_ARC_LIT:
        rel = 1;
    case CMD_SHAPE_ARC:
        agg_path_arc(
            ctx->envr,
            rel,
            RXA_PAIR(frm, 1),
            RXA_PAIR(frm, 2),
            (RXA_TYPE(frm, 3) == RXT_DECIMAL) ? RXA_DEC64(frm, 3) : RXA_INT64(frm, 3),
            RL_FIND_WORD(shape_ext_words , RXA_WORD(frm, 4)) - W_SHAPE_NEGATIVE,
            RL_FIND_WORD(shape_ext_words , RXA_WORD(frm, 5)) - W_SHAPE_SMALL
        );
        break;

    case CMD_SHAPE_CLOSE:
        agg_path_close(ctx->envr);
        break;

    case CMD_SHAPE_CURV_LIT:
        rel = 1;
    case CMD_SHAPE_CURV:
        {
			RXIARG val[2];
			REBCNT type;
			REBCNT n, m = 0;
			REBSER blk = RXA_SERIES(frm, 1);

			for (n = 0; type = RL_GET_VALUE(blk, n, &val[m]); n++) {
			    if (type == RXT_PAIR && ++m == 2) {
                    agg_path_curv(ctx->envr, rel, val[0].pair, val[1].pair);
                    m = 0;
			    }
			}
        }
        break;

    case CMD_SHAPE_CURVE_LIT:
        rel = 1;
    case CMD_SHAPE_CURVE:
        {
			RXIARG val[3];
			REBCNT type;
			REBCNT n, m = 0;
			REBSER blk = RXA_SERIES(frm, 1);

			for (n = 0; type = RL_GET_VALUE(blk, n, &val[m]); n++) {
                if (type == RXT_PAIR && ++m == 3) {
                    agg_path_curve(ctx->envr, rel, val[0].pair, val[1].pair, val[2].pair);
                    m = 0;
                }
			}
        }
        break;

    case CMD_SHAPE_HLINE_LIT:
        rel = 1;
    case CMD_SHAPE_HLINE:
        agg_path_hline(ctx->envr, rel, (RXA_TYPE(frm, 1) == RXT_DECIMAL) ? RXA_DEC64(frm, 1) : RXA_INT64(frm, 1));
        break;

    case CMD_SHAPE_LINE_LIT:
        rel = 1;
    case CMD_SHAPE_LINE:
        if (RXA_TYPE(frm, 1) == RXT_PAIR)
            agg_path_line(ctx->envr, rel, RXA_PAIR(frm, 1));
        else {
			RXIARG val;
			REBCNT type;
			REBCNT n;
			REBSER blk = RXA_SERIES(frm, 1);

			for (n = 0; type = RL_GET_VALUE(blk, n, &val); n++) {
				if (type == RXT_PAIR)
                    agg_path_line(ctx->envr, rel, val.pair);
			}
        }
        break;

    case CMD_SHAPE_MOVE_LIT:
        rel = 1;
    case CMD_SHAPE_MOVE:
        agg_path_move(ctx->envr, rel, RXA_PAIR(frm, 1));
        break;

    case CMD_SHAPE_QCURV_LIT:
        rel = 1;
    case CMD_SHAPE_QCURV:
        agg_path_qcurv(ctx->envr, rel, RXA_PAIR(frm, 1));
        break;

    case CMD_SHAPE_QCURVE_LIT:
        rel = 1;
    case CMD_SHAPE_QCURVE:
        {
			RXIARG val[2];
			REBCNT type;
			REBCNT n, m = 0;
			REBSER blk = RXA_SERIES(frm, 1);

			for (n = 0; type = RL_GET_VALUE(blk, n, &val[m]); n++) {
			    if (type == RXT_PAIR && ++m == 2) {
                    agg_path_qcurve(ctx->envr, rel, val[0].pair, val[1].pair);
                    m = 0;
			    }
			}
        }
        break;

    case CMD_SHAPE_VLINE_LIT:
        rel = 1;
    case CMD_SHAPE_VLINE:
        agg_path_vline(ctx->envr, rel, (RXA_TYPE(frm, 1) == RXT_DECIMAL) ? RXA_DEC64(frm, 1) : RXA_INT64(frm, 1));
        break;

    default:
		return RXR_NO_COMMAND;
	}

    return RXR_UNSET;
}

/***********************************************************************
**
*/	RXIEXT int RXD_Draw(int cmd, RXIFRM *frm, REBCEC *ctx)
/*
**		DRAW command dispatcher.
**
***********************************************************************/
{
	switch (cmd) {

    case CMD_DRAW_INIT_WORDS:
        //temp hack - will be removed later
        draw_ext_words = RL_MAP_WORDS(RXA_SERIES(frm,1));
        break;
    case CMD_DRAW_ANTI_ALIAS:
        agg_anti_alias(ctx->envr, RXA_LOGIC(frm, 1));
        break;

	case CMD_DRAW_ARC:
		agg_arc(
            ctx->envr,
            RXA_PAIR(frm, 1),
            RXA_PAIR(frm, 2),
            (RXA_TYPE(frm, 3) == RXT_DECIMAL) ? RXA_DEC64(frm, 3) : RXA_INT64(frm, 3),
            (RXA_TYPE(frm, 4) == RXT_DECIMAL) ? RXA_DEC64(frm, 4) : RXA_INT64(frm, 4),
            RL_FIND_WORD(draw_ext_words , RXA_WORD(frm, 5)) - W_DRAW_OPENED
        );
		break;

	case CMD_DRAW_ARROW:
		agg_arrow(ctx->envr, RXA_PAIR(frm, 1), (RXA_TYPE(frm, 2) == RXT_NONE) ? NULL : RXA_TUPLE(frm, 2)+1);
		break;

	case CMD_DRAW_BOX:
		agg_box(ctx->envr, RXA_PAIR(frm, 1), RXA_PAIR(frm, 2), (RXA_TYPE(frm, 3) == RXT_DECIMAL) ? RXA_DEC64(frm, 3) : RXA_INT64(frm, 3));
		break;

	case CMD_DRAW_CIRCLE:
		agg_circle(ctx->envr, RXA_PAIR(frm, 1), RXA_PAIR(frm, 2));
		break;

	case CMD_DRAW_CLIP:
		agg_clip(ctx->envr, RXA_PAIR(frm, 1), RXA_PAIR(frm, 2));
		break;

	case CMD_DRAW_CURVE:
        if (RXA_TYPE(frm, 4) == RXT_NONE)
			agg_curve3(ctx->envr, RXA_PAIR(frm, 1), RXA_PAIR(frm, 2), RXA_PAIR(frm, 3));
        else
			agg_curve4(ctx->envr, RXA_PAIR(frm, 1), RXA_PAIR(frm, 2), RXA_PAIR(frm, 3), RXA_PAIR(frm, 4));

		break;

	case CMD_DRAW_ELLIPSE:
		agg_ellipse(ctx->envr, RXA_PAIR(frm, 1), RXA_PAIR(frm, 2));
		break;

	case CMD_DRAW_FILL_PEN:
		{
   			//REBYTE* val;
			//REBCNT type;
			//REBSER* img;

        if (RXA_TYPE(frm, 1) == RXT_TUPLE)
            agg_fill_pen(ctx->envr, RXA_TUPLE(frm, 1)+1);
        else if (RXA_TYPE(frm, 1) == RXT_LOGIC && !RXA_LOGIC(frm,1))
            agg_fill_pen(ctx->envr, NULL);
        else {
            agg_fill_pen_image(ctx->envr, RXA_IMAGE_BITS(frm,1), RXA_IMAGE_WIDTH(frm,1), RXA_IMAGE_HEIGHT(frm,1));
            }
        }
		break;

    case CMD_DRAW_FILL_RULE:
        agg_fill_rule(ctx->envr, RL_FIND_WORD(draw_ext_words , RXA_WORD(frm, 1)));
        break;

    case CMD_DRAW_GAMMA:
        agg_gamma(ctx->envr, (RXA_TYPE(frm, 1) == RXT_DECIMAL) ? RXA_DEC64(frm, 1) : RXA_INT64(frm, 1));
        break;

	case CMD_DRAW_GRAD_PEN:
        if (RXA_TYPE(frm, 7) == RXT_NONE)
            agg_reset_gradient_pen(ctx->envr);
        else
            agg_gradient_pen(
                ctx->envr,
                RL_FIND_WORD(draw_ext_words , RXA_WORD(frm, 1)), //type
                RL_FIND_WORD(draw_ext_words , RXA_WORD(frm, 2)), //mode
                RXA_PAIR(frm, 3), //offset
                RXA_PAIR(frm, 4), //range - (begin, end)
                (RXA_TYPE(frm, 5) == RXT_DECIMAL) ? RXA_DEC64(frm, 5) : RXA_INT64(frm, 5), // angle
                RXA_PAIR(frm, 6), // scale
                RXA_SERIES(frm, 7) // unsigned char *colors
            );
        break;

    case CMD_DRAW_IMAGE:
        if (RXA_TYPE(frm, 2) == RXT_PAIR)
            agg_image(ctx->envr, RXA_IMAGE_BITS(frm,1), RXA_IMAGE_WIDTH(frm,1), RXA_IMAGE_HEIGHT(frm,1), RXA_PAIR(frm, 2));
        else {
            agg_image_scale(ctx->envr, RXA_IMAGE_BITS(frm,1), RXA_IMAGE_WIDTH(frm,1), RXA_IMAGE_HEIGHT(frm,1), RXA_SERIES(frm, 2));
        }
        break;

    case CMD_DRAW_IMAGE_FILTER:
        agg_image_filter(
            ctx->envr,
            RL_FIND_WORD(draw_ext_words , RXA_WORD(frm, 1)) - W_DRAW_NEAREST,
            RL_FIND_WORD(draw_ext_words , RXA_WORD(frm, 2)) - W_DRAW_RESIZE,
            (RXA_TYPE(frm, 3) == RXT_NONE) ? 1.0 : (RXA_TYPE(frm, 3) == RXT_DECIMAL) ? RXA_DEC64(frm, 3) : RXA_INT64(frm, 3)
        );
        break;

    case CMD_DRAW_IMAGE_OPTIONS:
        agg_image_options(ctx->envr, (RXA_TYPE(frm, 1) == RXT_NONE) ? NULL : RXA_TUPLE(frm, 1)+1, RL_FIND_WORD(draw_ext_words , RXA_WORD(frm, 2)) - W_DRAW_NO_BORDER);
        break;

    case CMD_DRAW_IMAGE_PATTERN:
        agg_image_pattern(ctx->envr, RL_FIND_WORD(draw_ext_words , RXA_WORD(frm, 1)) - W_DRAW_NORMAL, RXA_PAIR(frm, 2), RXA_PAIR(frm, 3));
        break;


	case CMD_DRAW_LINE:
		{
			RXIARG val[2];
			REBCNT type;
			REBCNT n, m = 0;
			REBSER blk = RXA_SERIES(frm, 1);

			for (n = 0; type = RL_GET_VALUE(blk, n, &val[m]); n++) {
				if (type == RXT_PAIR) {
				    switch (++m){
                        case 1:
                            agg_path_open(ctx->envr);
                            break;
				        case 2:
                            agg_line(ctx->envr, val[0].pair,val[1].pair);
                            val[0] = val[1];
                            m--;
                            break;
				    }
				}
			}
		}
		break;

	case CMD_DRAW_LINE_CAP:
		agg_line_cap(ctx->envr, RL_FIND_WORD(draw_ext_words , RXA_WORD(frm, 1)) - W_DRAW_BUTT);
		break;

	case CMD_DRAW_LINE_JOIN:
		agg_line_join(ctx->envr, RL_FIND_WORD(draw_ext_words , RXA_WORD(frm, 1)) - W_DRAW_MITER);
		break;

	case CMD_DRAW_LINE_WIDTH:
		agg_line_width(ctx->envr, (RXA_TYPE(frm, 1) == RXT_DECIMAL) ? RXA_DEC64(frm, 1) : RXA_INT64(frm, 1), RL_FIND_WORD(draw_ext_words , RXA_WORD(frm, 2)) - W_DRAW_VARIABLE);
		break;

	case CMD_DRAW_LINE_PATTERN:
        if (RXA_TYPE(frm, 2) == RXT_NONE)
            agg_line_pattern(ctx->envr, NULL, NULL);
        else {
            REBSER patterns = RXA_SERIES(frm, 2);
            REBINT len = RL_SERIES(patterns, RXI_SER_TAIL);

            if (len > 1){

                RXIARG val;
                REBCNT type;
                REBCNT n;
                REBDEC* pattern = (REBDEC*) malloc ((len + 1) * sizeof(REBDEC)) ;

                pattern[0] = len;

                for (n = 0; type = RL_GET_VALUE(patterns, n, &val); n++) {
                    if (type == RXT_DECIMAL)
                        pattern[n+1] = val.dec64;
                    else if (type == RXT_INTEGER)
                        pattern[n+1] = val.int64;
                    else
                        break;
                }
                agg_line_pattern(ctx->envr, RXA_TUPLE(frm, 1)+1, pattern);
            }

        }
		break;

	case CMD_DRAW_INVERT_MATRIX:
		agg_invert_matrix(ctx->envr);
		break;

	case CMD_DRAW_MATRIX:
        agg_matrix(ctx->envr, RXA_SERIES(frm, 1));
		break;

	case CMD_DRAW_PEN:
        if (RXA_TYPE(frm, 1) == RXT_TUPLE)
            agg_pen(ctx->envr, RXA_TUPLE(frm, 1)+1);
        else if (RXA_TYPE(frm, 1) == RXT_LOGIC && !RXA_LOGIC(frm,1))
            agg_pen(ctx->envr, NULL);
        else
            agg_pen_image(ctx->envr, RXA_IMAGE_BITS(frm,1), RXA_IMAGE_WIDTH(frm,1), RXA_IMAGE_HEIGHT(frm,1));
		break;

	case CMD_DRAW_POLYGON:
		{
			RXIARG val;
			REBCNT type;
			REBCNT n;
			REBSER blk = RXA_SERIES(frm, 1);

			for (n = 0; type = RL_GET_VALUE(blk, n, &val); n++) {
				if (type == RXT_PAIR) {
					if (n > 0)
						agg_add_vertex(ctx->envr, val.pair);
					else
						agg_begin_poly(ctx->envr, val.pair);
				}
			}
			agg_end_poly(ctx->envr);
		}
		break;

    case CMD_DRAW_PUSH:
        {
            REBCEC innerCtx;

            innerCtx.envr = ctx->envr;
            innerCtx.block = RXA_SERIES(frm, 1);
            innerCtx.index = 0;

            agg_push_matrix(ctx->envr);
            RL_Do_Commands(RXA_SERIES(frm, 1), 0, &innerCtx);
            agg_pop_matrix(ctx->envr);
        }
        break;

	case CMD_DRAW_RESET_MATRIX:
		agg_reset_matrix(ctx->envr);
		break;

    case CMD_DRAW_ROTATE:
        agg_rotate(ctx->envr, (RXA_TYPE(frm, 1) == RXT_DECIMAL) ? RXA_DEC64(frm, 1) : RXA_INT64(frm, 1));
        break;

    case CMD_DRAW_SCALE:
        agg_scale(ctx->envr, RXA_PAIR(frm, 1));
        break;

    case CMD_DRAW_SHAPE:
        {
            REBCEC innerCtx;

            innerCtx.envr = ctx->envr;
            innerCtx.block = RXA_SERIES(frm, 1);
            innerCtx.index = 0;

            agg_path_open(ctx->envr);
            RL_Do_Commands(RXA_SERIES(frm, 1), 0, &innerCtx);
            agg_path_close(ctx->envr);
        }
        break;

    case CMD_DRAW_SKEW:
        agg_skew(ctx->envr, RXA_PAIR(frm, 1));
        break;

	case CMD_DRAW_SPLINE:
        {
            REBSER points = RXA_SERIES(frm, 1);
            REBINT len = RL_SERIES(points, RXI_SER_TAIL);

            if (len > 3){
                RXIARG val;
                REBCNT type;
                REBCNT n;

                for (n = 0; type = RL_GET_VALUE(points, n, &val); n++) {
                    if (type == RXT_PAIR) {
                        if (n > 0)
                            agg_add_vertex(ctx->envr, val.pair);
                        else
                            agg_begin_poly(ctx->envr, val.pair);
                    }
                }
                agg_end_spline(ctx->envr, RXA_INT32(frm, 2), RL_FIND_WORD(draw_ext_words , RXA_WORD(frm, 3)) - W_DRAW_OPENED);
            }

		}
		break;

    case CMD_DRAW_TEXT:
        agg_text(
            ctx->envr,
            (RL_FIND_WORD(draw_ext_words , RXA_WORD(frm, 3)) == W_DRAW_VECTORIAL) ? 1 : 0,
             RXA_PAIR(frm, 1),
             RXA_PAIR(frm, 2),
             RXA_SERIES(frm, 4)
        );
        break;

	case CMD_DRAW_TRANSFORM:
		agg_transform(
            ctx->envr,
            (RXA_TYPE(frm, 1) == RXT_DECIMAL) ? RXA_DEC64(frm, 1) : RXA_INT64(frm, 1), // angle
            RXA_PAIR(frm, 2), // center
            RXA_PAIR(frm, 3), // scale
            RXA_PAIR(frm, 4) // offset
        );
        break;

    case CMD_DRAW_TRANSLATE:
        agg_translate(ctx->envr, RXA_PAIR(frm, 1));
		break;

	case CMD_DRAW_TRIANGLE:
        {
            REBYTE b[4] = {0,0,0,0};
            agg_triangle(
                ctx->envr,
                RXA_PAIR(frm, 1), // vertex-1
                RXA_PAIR(frm, 2), // vertex-2
                RXA_PAIR(frm, 3), // vertex-3
                (RXA_TYPE(frm, 4) == RXT_NONE) ? NULL : RXA_TUPLE(frm, 4)+1, // color-1
                (RXA_TYPE(frm, 5) == RXT_NONE) ? b : RXA_TUPLE(frm, 5)+1, // color-2
                (RXA_TYPE(frm, 6) == RXT_NONE) ? b : RXA_TUPLE(frm, 6)+1, // color-3
                (RXA_TYPE(frm, 7) == RXT_DECIMAL) ? RXA_DEC64(frm, 7) : RXA_INT64(frm, 7) // dilation
            );
        }
		break;

	default:
		return RXR_NO_COMMAND;
	}

    return RXR_UNSET;
}


/***********************************************************************
**
*/	void Init_Graphics(void)
/*
**	Initialize special variables of the graphics subsystem.
**
***********************************************************************/
{
	RL = RL_Extend((REBYTE *)(&RX_graphics[0]), &RXD_Graphics);
	RL_Extend((REBYTE *)(&RX_draw[0]), &RXD_Draw);
	RL_Extend((REBYTE *)(&RX_shape[0]), &RXD_Shape);
	RL_Extend((REBYTE *)(&RX_text[0]), &RXD_Text);
}

#ifdef OLD__FUNCS_NEED_CONVERSION

/***********************************************************************
**
*/  REBINT OS_Effect_Image(REBSER *image, REBSER *block)
/*
**      Render EFFECT dialect into an image.
**		Clip to keep render inside the image provided.
**
***********************************************************************/
{
//	return Effect_Image(image, block);
	return 0;
}

#endif

