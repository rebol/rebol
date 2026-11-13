/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2025 Rebol Open Source Contributors
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
**  Module:  u-wav.c
**  Summary: conversion to and from WAV audio format
**  Section: utility
**  Author:  Oldes
**  Notes:
**    This is an optional part of R3. This file can be replaced by
**    library function calls into an updated implementation.
**
**    To be honest, I'm not sure how useful it is as one gets just binary,
**    but no aditional info. It should return a sound object instead!
**    So far it can be considered just as a minimal codec example.
**
***********************************************************************
**  Base-code:

	if find system/codecs 'wav [
		system/codecs/wav/suffixes: [%.wav %.wave]
		system/codecs/wav/type: 'binary!
		append append system/catalog/file-types system/codecs/wav/suffixes 'wav
	] 

***********************************************************************/

#include "sys-core.h"
#ifdef INCLUDE_WAV_CODEC

//**********************************************************************

enum WavChunks {
	ChunkID_RIFF = 0x46464952,
	ChunkID_WAVE = 0x45564157,
	ChunkID_FMT  = 0x20746D66,
	ChunkID_DATA = 0x61746164,

//    Format          = 0x666D7420
//    LabeledText     = 0x478747C6,
//    Instrumentation = 0x478747C6,
//    Sample = 0x6C706D73,
//    Fact = 0x47361666,
//    Data = 0x61746164,
//    Junk = 0x4b4e554a,
};

typedef struct {
	u32 chunk_id;
	u32 chunk_size;
	u32 format;
	u32 fmtchunk_id;
	u32 fmtchunk_size;
	u16 audio_format;
	u16 num_channels;
	u32 sample_rate;
	u32 byte_rate;
	u16 block_align;
	u16 bps;
	u32 datachunk_id;
	u32 datachunk_size;
} WavHeader;

/***********************************************************************
**
*/	static void Decode_WAV(REBCDI *codi)
/*
**		Input:  WAV encoded sound (codi->data, len)
**		Output: Samples bits (codi->bits, len)
**		Error:  Code in codi->error
**		Return: Success as TRUE or FALSE
**
***********************************************************************/
{
	REBYTE *cp, *tp;
	WavHeader *hdr;


	int bytes = codi->len;

	cp = codi->data;
//	printf("[Decode_WAV] input bytes: %u\n", bytes);

	hdr = (WavHeader*)cp;

	if(bytes < sizeof(WavHeader) || hdr->chunk_id != ChunkID_RIFF || hdr->format != ChunkID_WAVE) {
		codi->error = CODI_ERR_SIGNATURE;
		return;
	}

//	puts("RIFF");
//	printf("chunk size: %u\n", hdr->chunk_size);
//	printf("num_channels: %i\n", hdr->num_channels);
//	printf("format: %i\n", hdr->audio_format);
//	printf("sample_rate: %i\n", hdr->sample_rate);
//	printf("bps: %i\n", hdr->bps);
//	printf("data id: %0X\n", hdr->datachunk_id);
//	printf("data size: %u\n", hdr->datachunk_size);

	if (hdr->audio_format != 1){
		// Only PCM encoding supported
		codi->error = CODI_ERR_ENCODING;
		return;
	}


	codi->len = bytes = hdr->datachunk_size;

	tp = Make_Mem(bytes);
	memcpy(tp, cp + sizeof(WavHeader), bytes);
	codi->data = tp;
}


/***********************************************************************
**
*/	static void Encode_WAV(REBCDI *codi)
/*
**		Not implemented!
**
***********************************************************************/
{
	codi->error = CODI_ERR_NA;
}


/***********************************************************************
**
*/	REBINT Codec_WAV(REBCDI *codi)
/*
***********************************************************************/
{
	codi->error = 0;

	if (codi->action == CODI_IDENTIFY) {
		Decode_WAV(codi);
		return CODI_CHECK; // error code is inverted result
	}

	if (codi->action == CODI_DECODE) {
		Decode_WAV(codi);
		return CODI_BINARY;
	}

	if (codi->action == CODI_ENCODE) {
		Encode_WAV(codi);
		return CODI_BINARY;
	}

	codi->error = CODI_ERR_NA;
	return CODI_ERROR;
}


/***********************************************************************
**
*/	void Init_WAV_Codec(void)
/*
***********************************************************************/
{
	Register_Codec("wav", Codec_WAV);
}

#endif
