/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2023 Rebol Open Source Developers
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
**  Title: Device: Audio for Win32
**  Author: Oldes
**  Purpose:
**      Provides a very simple interface to the XAudio2
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

//* Options ************************************************************

#if !defined(REBOL_OPTIONS_FILE)
#include "opt-config.h"
#else
#include REBOL_OPTIONS_FILE
#endif

#ifdef INCLUDE_AUDIO_DEVICE

#include <windows.h>
#include <xaudio2.h>

#include "reb-host.h"
#include "sys-utils.h"

// Globals:
static IXAudio2 *xaudio = NULL;
static IXAudio2MasteringVoice *xaudioMasterVoice = NULL;
//===========================================================

static void Send_Audio_Event(void *pBufferContext, u8 eventType, u32 data) {
	REBEVT evt;
	CLEARS(&evt);
	evt.model = EVM_PORT;
	evt.port = pBufferContext;
	evt.type = eventType;
	evt.data = data;
	RL_Event(&evt);
}
static void __stdcall OnBufferEnd(IXAudio2VoiceCallback *This, void *ctx) { Send_Audio_Event(ctx, EVT_WROTE, 0); }
static void __stdcall OnStreamEnd(IXAudio2VoiceCallback *This) {  }
static void __stdcall OnVoiceProcessingPassEnd(IXAudio2VoiceCallback *This) { }//puts("OnVoiceProcessingPassEnd"); }
static void __stdcall OnVoiceProcessingPassStart(IXAudio2VoiceCallback *This, UINT32 SamplesRequired) { }//puts("OnVoiceProcessingPassStart"); }
static void __stdcall OnBufferStart(IXAudio2VoiceCallback *This, void *ctx) { }//puts("OnBufferStart"); }
static void __stdcall OnLoopEnd(IXAudio2VoiceCallback *This, void *ctx) { Send_Audio_Event(ctx, EVT_TIME, 0); }
static void __stdcall OnVoiceError(IXAudio2VoiceCallback *This, void *ctx, HRESULT err) { Send_Audio_Event(ctx, EVT_ERROR, err); }

IXAudio2VoiceCallback xAudioCallbacks = {
	.lpVtbl = &(IXAudio2VoiceCallbackVtbl) {
		.OnStreamEnd = OnStreamEnd,
		.OnVoiceProcessingPassEnd = OnVoiceProcessingPassEnd,
		.OnVoiceProcessingPassStart = OnVoiceProcessingPassStart,
		.OnBufferEnd = OnBufferEnd,
		.OnBufferStart = OnBufferStart,
		.OnLoopEnd = OnLoopEnd,
		.OnVoiceError = OnVoiceError
	}
};


/***********************************************************************
**
*/	DEVICE_CMD Init_XAudio2(REBREQ *req)
/*
***********************************************************************/
{
	HRESULT hr = S_OK;
	do {
		
		hr = XAudio2Create(&xaudio, XAUDIO2_STOP_ENGINE_WHEN_IDLE, XAUDIO2_DEFAULT_PROCESSOR);
		ASSERT_HR("Failed to initialize XAudio!");
		hr = IXAudio2_CreateMasteringVoice(
			xaudio,
			&xaudioMasterVoice,
			XAUDIO2_DEFAULT_CHANNELS,
			XAUDIO2_DEFAULT_SAMPLERATE,
			0,
			NULL,
			NULL,
			AudioCategory_Other
		);
		ASSERT_HR("Failed to initialize XAudio mastering voice!");
	} while (FALSE);
	if (FAILED(hr))
		return DR_ERROR;
	return DR_DONE;
}

/***********************************************************************
**
*/	DEVICE_CMD Open_XAudio2(REBREQ *req)
/*
***********************************************************************/
{
	HRESULT hr;
	IXAudio2SourceVoice *voice;
	WAVEFORMATEXTENSIBLE wfx = { 0 };

	if(!xaudio) return DR_ERROR;

	wfx.Format.nChannels = req->audio.channels;
	wfx.Format.wFormatTag = req->audio.type;
	wfx.Format.nSamplesPerSec = req->audio.rate;
	wfx.Format.wBitsPerSample = req->audio.bits;
	wfx.Format.nBlockAlign = (wfx.Format.nChannels * wfx.Format.wBitsPerSample) / 8;
	wfx.Format.nAvgBytesPerSec = wfx.Format.nSamplesPerSec * wfx.Format.nChannels * wfx.Format.wBitsPerSample / 8;
	wfx.Format.cbSize = 0;

	hr = IXAudio2_CreateSourceVoice(
		xaudio,
		&voice,
		(WAVEFORMATEX *)&wfx,
		0,
		XAUDIO2_DEFAULT_FREQ_RATIO,
		&xAudioCallbacks,
		NULL,
		NULL
	);
	if (FAILED(hr))
		return DR_ERROR;

	req->handle = voice;
	SET_OPEN(req);
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Close_XAudio2(REBREQ *req)
/*
***********************************************************************/
{
	IXAudio2SourceVoice *voice;

	if (!xaudio) return DR_ERROR;

	if (req->handle) {
		voice = req->handle;
		IXAudio2SourceVoice_Stop(voice, 0, XAUDIO2_COMMIT_NOW);
		IXAudio2SourceVoice_DestroyVoice(voice);
	}
	req->handle = NULL;
	SET_CLOSED(req);
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Read_XAudio2(REBREQ *req)
/*
***********************************************************************/
{
	// XAudio2 does not provide microphone access, but maybe
	// we could use `read` to get some other info?
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Write_XAudio2(REBREQ *req)
/*
**	NOTE: XAudio2 does not copy the source data and so these are still live!
**	      User should take care not to modify these before buffer is done.
**
***********************************************************************/
{
	IXAudio2SourceVoice *voice;
	XAUDIO2_BUFFER xaudioBuffer = { 0 };
	voice = req->handle;

	if (!xaudio) return DR_ERROR;

	if (voice && req->data && req->length > 0) {
		
		xaudioBuffer.AudioBytes = req->length;
		xaudioBuffer.pAudioData = req->data;
		xaudioBuffer.pContext   = req->port;
		xaudioBuffer.Flags      = XAUDIO2_END_OF_STREAM;
		xaudioBuffer.LoopCount  = min(255, req->audio.loop_count);
		
		IXAudio2SourceVoice_SubmitSourceBuffer(voice, &xaudioBuffer, NULL);
		IXAudio2SourceVoice_Start(voice, 0, XAUDIO2_COMMIT_NOW);
	}
	req->actual = 0;
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Poll_XAudio2(REBREQ *req)
/*
***********************************************************************/
{
	//puts("Poll_XAudio2");
	return DR_DONE;
}

/***********************************************************************
**
*/	DEVICE_CMD Query_XAudio2(REBREQ *req)
/*
***********************************************************************/
{
	//TODO!!!
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Modify_XAudio2(REBREQ *req)
/*
***********************************************************************/
{
	IXAudio2SourceVoice *voice;
	XAUDIO2_BUFFER xaudioBuffer = { 0 };
	voice = req->handle;

	if (!xaudio || !voice) return DR_ERROR;

	switch (req->modify.mode) {
	case MODE_AUDIO_PLAY:
		IXAudio2SourceVoice_Stop(voice, 0, XAUDIO2_COMMIT_NOW);
		break;
	}
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Quit_XAudio2(REBREQ *req)
/*
***********************************************************************/
{
	if (xaudio) {
		IXAudio2_Release(xaudio);
		xaudio = NULL;
		xaudioMasterVoice = NULL;
	}
	return DR_DONE;
}


/***********************************************************************
**
**	Command Dispatch Table (RDC_ enum order)
**
***********************************************************************/

static DEVICE_CMD_FUNC Dev_Cmds[RDC_MAX] =
{
	Init_XAudio2,
	Quit_XAudio2,
	Open_XAudio2,
	Close_XAudio2,
	Read_XAudio2,
	Write_XAudio2,
	Poll_XAudio2,
	0,	// connect
	Query_XAudio2,
	Modify_XAudio2,
	0,	// create
	0,	// delete
	0,	// rename
	0	// lookup
};

DEFINE_DEV(Dev_Audio, "AUDIO", 1, Dev_Cmds, RDC_MAX, 0);

#endif //INCLUDE_AUDIO_DEVICE
