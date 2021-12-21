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
**  Module:  u-qoi.c
**  Summary: QOI image format conversion
**  Section: utility
**
***********************************************************************
**  Base-code:

	if find system/codecs 'qoi [
		system/codecs/qoi/suffixes: [%.qoi]
		append append system/options/file-types system/codecs/qoi/suffixes 'qoi
	]

***********************************************************************/
#include "sys-core.h"

#ifdef INCLUDE_QOI_CODEC

#define QOI_MALLOC(sz) Make_Mem(sz)
#define QOI_FREE(p)    free(p)

#define QOI_IMPLEMENTATION
#define QOI_NO_STDIO
#include "sys-qoi.h"

/***********************************************************************
**
*/	static void Encode_QOI_Image(REBCDI *codi)
/*
**		Input:  Image bits (codi->bits, w, h)
**		Output: QOI encoded image (codi->data, len)
**		Error:  Code in codi->error
**		Return: Success as TRUE or FALSE
**
***********************************************************************/
{
	qoi_desc desc;
	desc.width    = codi->w;
	desc.height   = codi->h;
	desc.channels = 4;
	desc.colorspace = QOI_SRGB;

	codi->data = qoi_encode(codi->bits, &desc, &codi->len);
	codi->error = codi->data == NULL ? -1 : 0;
}


/***********************************************************************
**
*/	void Decode_QOI_Image(REBCDI *codi)
/*
**		Input:  QOI encoded image (codi->data, len)
**		Output: Image bits (codi->bits, w, h)
**		Error:  Code in codi->error
**
***********************************************************************/
{
	qoi_desc desc;

	codi->bits = qoi_decode(codi->data, codi->len, &desc, 4);
	codi->w = desc.width;
	codi->h = desc.height;
	codi->error = codi->bits == NULL ? -1 : 0;
}

/***********************************************************************
**
*/	void Identify_QOI_Image(REBCDI *codi)
/*
**		Input:  QOI encoded image (codi->data)
**		Output: No error means success
**
***********************************************************************/
{
	int p = 0;
	unsigned int header_magic = qoi_read_32(codi->data, &p);
	codi->error = (header_magic != QOI_MAGIC);
}


/***********************************************************************
**
*/	REBINT Codec_QOI_Image(REBCDI *codi)
/*
***********************************************************************/
{
	codi->error = 0;

	if (codi->action == CODI_IDENTIFY) {
		Identify_QOI_Image(codi);
		return CODI_CHECK; // error code is inverted result
	}

	if (codi->action == CODI_DECODE) {
		Decode_QOI_Image(codi);
		return CODI_IMAGE;
	}

	if (codi->action == CODI_ENCODE) {
		Encode_QOI_Image(codi);
		return CODI_BINARY;
	}

	codi->error = CODI_ERR_NA;
	return CODI_ERROR;
}


/***********************************************************************
**
*/	void Init_QOI_Codec(void)
/*
***********************************************************************/
{
	Register_Codec("qoi", Codec_QOI_Image);
}

#endif //INCLUDE_QOI_CODEC
