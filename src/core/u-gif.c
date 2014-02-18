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
**  Module:  u-gif.c
**  Summary: GIF image format conversion
**  Section: utility
**  Notes:
**    This is an optional part of R3. This file can be replaced by
**    library function calls into an updated implementation.
**
***********************************************************************/

#include "sys-core.h"


#define	MAX_STACK_SIZE	4096
#define	NULL_CODE		(-1)
#define BitSet(byte,bit)  (((byte) & (bit)) == (bit))
#define LSBFirstOrder(x,y)  (((y) << 8) | (x))

static REBINT	interlace_rate[4] = { 8, 8, 4, 2 },
				interlace_start[4] = { 0, 4, 2, 1 };


#ifdef COMP_IMAGES
// Because graphics.c is not included, we must have a copy here.
void Chrom_Key_Alpha(REBVAL *v,REBCNT col,REBINT blitmode) {
	REBOOL found=FALSE;
	int i;
	REBCNT *p;

	p=(REBCNT *)VAL_IMAGE_HEAD(v);
	i=VAL_IMAGE_WIDTH(v)*VAL_IMAGE_HEIGHT(v);
	switch(blitmode) {
		case BLIT_MODE_COLOR:
			for(;i>0;i--,p++) {
				if(*p==col) {
					found=TRUE;
					*p=col|0xff000000;
				}
			}
		case BLIT_MODE_LUMA:
			for(;i>0;i--,p++) {
				if(BRIGHT(((REBRGB *)p))<=col) {
					found=TRUE;
					*p|=0xff000000;
				}
			}
			break;
	}
	if(found)
		VAL_IMAGE_TRANSP(v)=VITT_ALPHA;
}
#endif

/***********************************************************************
**
*/	void Decode_LZW(REBCNT *data, REBYTE **cpp, REBYTE *colortab, REBINT w, REBINT h, REBOOL interlaced)
/*
**	Perform LZW decompression.
**
***********************************************************************/
{
	REBYTE	*cp = *cpp;
	REBYTE	*rp;
	REBINT	available, clear, code_mask, code_size, end_of_info, in_code;
	REBINT	old_code, bits, code, count, x, y, data_size, row, i;
	REBCNT	*dp, datum;
	short	*prefix;
	REBYTE	first, *pixel_stack, *suffix, *top_stack;

	suffix = Make_Mem(MAX_STACK_SIZE * (sizeof(REBYTE) + sizeof(REBYTE) + sizeof(short)));
	pixel_stack = suffix + MAX_STACK_SIZE;
	prefix = (short *)(pixel_stack + MAX_STACK_SIZE);

	data_size = *cp++;
	clear = 1 << data_size;
	end_of_info = clear + 1;
	available = clear + 2;
	old_code = NULL_CODE;
	code_size = data_size + 1;
	code_mask = (1 << code_size) - 1;

	for (code=0; code<clear; code++) {
		prefix[code] = 0;
		suffix[code] = code;
	}

	datum = 0;
	bits = 0;
	count = 0;
	first = 0;
	row = i = 0;
	top_stack = pixel_stack;
	dp = data;
	for (y=0; y<h; y++) {
		for (x=0; x<w;) {
			// if the stack is empty
			if (top_stack == pixel_stack) {
				// if we don't have enough bits
				if (bits < code_size) {
					// if we run out of bytes in the packet
					if (count == 0) {
						// get size of next packet
						count = *cp++;
						// if 0, end of image
						if (count == 0)
							break;
					}
					// add bits from next byte and adjust counters
					datum += *cp++ << bits;
					bits += 8;
					count--;
					continue;
				}
				// isolate the code bits and adjust the temps
				code = datum & code_mask;
				datum >>= code_size;
				bits -= code_size;

				// sanity check
				if (code > available || code == end_of_info)
					break;
				// time to reset the tables
				if (code == clear) {
					code_size = data_size + 1;
					code_mask = (1 << code_size) - 1;
					available = clear + 2;
					old_code = NULL_CODE;
					continue;
				}
				// if we are the first code, just stack it
				if (old_code == NULL_CODE) {
					*top_stack++ = suffix[code];
					old_code = code;
					first = code;
					continue;
				}
				in_code = code;
				if (code == available) {
					*top_stack++ = first;
					code = old_code;
				}
				while (code > clear) {
					*top_stack++ = suffix[code];
					code = prefix[code];
				}
				first = suffix[code];

				// add a new string to the table
				if (available >= MAX_STACK_SIZE)
					break;
				*top_stack++ = first;
				prefix[available] = old_code;
				suffix[available++] = first;
				if ((available & code_mask) == 0 && available < MAX_STACK_SIZE) {
					code_size++;
					code_mask += available;
				}
				old_code = in_code;
			}
			top_stack--;
			rp = colortab + 3 * *top_stack;
			*dp++ = rp[2] | (rp[1] << 8) | (rp[0] << 16);
			x++;
		}
		if (interlaced) {
			row += interlace_rate[i];
			if (row >= h) {
				row = interlace_start[++i];
			}
			dp = data + row * w;
		}
	}
	*cpp = cp + count + 1;
	Free_Mem(suffix, MAX_STACK_SIZE * (sizeof(REBYTE) + sizeof(REBYTE) + sizeof(short)));
}


/***********************************************************************
**
*/	void Decode_GIF_Image(REBCDI *codi)
/*
**		Input:  GIF encoded image (codi->data, len)
**		Output: Image bits (codi->bits, w, h)
**		Error:  Code in codi->error
**		Return: Success as TRUE or FALSE
**
***********************************************************************/
{
	REBINT	w, h;
	REBINT	transparency_index;
	REBYTE	c, *global_colormap, *colormap;
	REBCNT	global_colors, image_count, local_colormap;
	REBCNT	colors;
	REBYTE	*cp;
	REBCNT	*dp;
	REBOOL	interlaced;
	REBYTE	*end;

	cp  = codi->data;
	end = codi->data + codi->len;

	if (strncmp((char *)cp, "GIF87", 5) != 0 && strncmp((char *)cp, "GIF89", 5) != 0) {
		codi->error = CODI_ERR_SIGNATURE;
		return;
	}
	if (codi->action == CODI_IDENTIFY) return; // no error means success

	global_colors = 0;
	global_colormap = (unsigned char *) NULL;
	if (cp[10] & 0x80) {
		// Read global colormap.
		global_colors = 1 << ((cp[10] & 0x07) + 1);
		global_colormap = cp + 13;
		cp += global_colors * 3;
	}
	cp += 13;
	transparency_index = -1;
	image_count = 0;
	for (;;) {
		if (cp >= end) break;
		c = *cp++;

		if (c == ';')
			break;  // terminator

		if (c == '!') {
			// GIF Extension block.
			c = *cp++;
			switch (c) {
			case 0xf9:
				// Transparency extension block.
				while (cp[0] != 0 && cp[5] != 0)
					cp += 5;
				if ((cp[1] & 0x01) == 1)
					transparency_index = cp[4];
				cp += cp[0] + 1 + 1;
				break;

			default:
				while (cp[0] != 0)
					cp += cp[0] + 1;
				cp++;
				break;
			}
		}

		if (c != ',') continue;

		image_count++;
		interlaced = (cp[8] & 0x40) != 0;
		local_colormap = cp[8] & 0x80;

		w = LSBFirstOrder(cp[4],cp[5]);
		h = LSBFirstOrder(cp[6],cp[7]);
		// if(w * h * 4 > VAL_STR_LEN(img)) 
		//			h = 4 * VAL_STR_LEN(img) / w;

		// Inititialize colormap.
		colors = !local_colormap ? global_colors : 1 << ((cp[8] & 0x07)+1);
		if (!local_colormap) {
			colormap = global_colormap;
		}
		else {
			colormap = cp + 9;
			cp += 3 * colors;
		}
		cp += 9;

/*
		if (image_count == 2) {
			VAL_SERIES(Temp2_Value) = Make_Block(0, 0);
			VAL_INIT(Temp2_Value, REB_BLOCK);
			VAL_INDEX(Temp2_Value) = 0;
			Append_Series(VAL_SERIES(Temp2_Value), (REBMEM *)Temp_Value, 1);
		}
*/
		dp = codi->bits = (u32 *)Make_Mem(w * h * 4);
		codi->w = w;
		codi->h = h;

		Decode_LZW(dp, &cp, colormap, w, h, interlaced);

		if(transparency_index >= 0) {
			int ADD_alpha_key_detection;
			REBYTE *p=colormap+3*transparency_index;
			///Chroma_Key_Alpha(Temp_Value, (REBCNT)(p[2]|(p[1]<<8)|(p[0]<<16)), BLIT_MODE_COLOR);
		}

//		if (image_count == 1)
//			*Temp2_Value = *Temp_Value;
//		else
//			Append_Series(VAL_SERIES(Temp2_Value), (REBMEM *)Temp_Value, 1);
	}
}


/***********************************************************************
**
*/	REBINT Codec_GIF_Image(REBCDI *codi)
/*
***********************************************************************/
{
	codi->error = 0;

	if (codi->action == CODI_IDENTIFY) {
		Decode_GIF_Image(codi);
		return CODI_CHECK; // error code is inverted result
	}

	if (codi->action == CODI_DECODE) {
		Decode_GIF_Image(codi);
		return CODI_IMAGE;
	}

	codi->error = CODI_ERR_NA;
	return CODI_ERROR;
}


/***********************************************************************
**
*/	void Init_GIF_Codec(void)
/*
***********************************************************************/
{
	Register_Codec("gif", Codec_GIF_Image);
}
