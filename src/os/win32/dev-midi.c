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
**  Title: Device: MIDI access for Win32
**  Author: Oldes
**  Purpose:
**      Provides a very simple interface to the MIDI devices
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

#ifdef INCLUDE_MIDI_DEVICE
//#define DEBUG_MIDI

#ifdef DEBUG_MIDI
#define LOG(...) do { printf(__VA_ARGS__); } while (0)
#else
#define LOG(...)
#endif

// NOTE: this will be useful for higher level launchpad schemes:
// https://github.com/FMMT666/launchpad.py/blob/master/launchpad_py/launchpad.py
// (code includes led/key mapping values)
//*********************************************************************/

#include <sdkddkver.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include "reb-host.h"
#include "host-lib.h"
#include "sys-net.h"


#include <mmsystem.h>
//#include <MMREG.H> // contains manufacturer and product id enumerations if needed
#pragma comment(lib, "winmm.lib")

#define MIDI_BUF_SIZE (1024 * sizeof(u32))	

// Midi handle structure:
typedef struct midi_port_handle {
	REBSER*  port;
	REBCNT   inp_id;
	REBCNT   out_id;
	HMIDIIN  inp_device;
	HMIDIOUT out_device;
	REBSER*  inp_buffer;
} REBMID;

#define MIDI_PORTS_ALLOC 16 // number of port handles allocated in the pool

typedef struct midi_ports {
	REBCNT  count;
	REBMID* ports;
} midi_ports;


static midi_ports Midi_Ports_Pool;

static REBINT Get_New_Midi_Port(REBMID **port)
{
	REBCNT n;
	for (n = 0; n < Midi_Ports_Pool.count; n++) {
		if (Midi_Ports_Pool.ports[n].port == NULL) {
			*port = &Midi_Ports_Pool.ports[n];
			return n;
		}
	}
	return -1;
}

/***********************************************************************
**
*/ static int Midi_Push_Buffer(REBSER* buffer, u32 data)
/*
**	Stores data of DWORD size into port's input buffer
**	Using REBSER as circular buffer.. reusing the existing fields
**		tail => writers head
**		rest => readers head
**		size => maxlen
***********************************************************************/
{
	REBCNT next = buffer->tail + sizeof(data);
	if (next >= buffer->size)
		next = 0;
	
	if (next == buffer->rest) {
		// circular buffer is full!
		// input data lost! Should not happen with large enough buffer
		return -1; // Should it be reported to user?
	}

	u32* buf = (u32*)(buffer->data + buffer->tail);
	buf[0] = data;
	buffer->tail = next;

	//if (buffer)
	//	printf("buff-size: %u rest: %u  tail: %u\n",
	//		buffer->size,
	//		buffer->rest,
	//		buffer->tail);

	return 0;
}

/***********************************************************************
**
**	MidiIn callback procedure.
**	dwInstance contains id of the MIDI port structure in pool
**
***********************************************************************/
static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	int status, channel, param1, param2;
	REBEVT evt;
	REBMID midi_port;

	if (dwInstance >= Midi_Ports_Pool.count) return;

	midi_port = Midi_Ports_Pool.ports[dwInstance];

	CLEARS(&evt);
	evt.model = EVM_MIDI;
	evt.port = midi_port.port;

	LOG("MidiInProc... port: %0X\n", midi_port.port);

	switch (wMsg) {
	case MIM_DATA:
		status = (0xF0 & dwParam1);
		channel = (0x0F & dwParam1);
		param1 = 0xFF & (dwParam1 >> 8);
		param2 = 0xFF & (dwParam1 >> 16);
#ifdef DEBUG_MIDI
		printf("wMsg=MIM_DATA, dwInstance=%u, dwParam1=%08x, dwParam2=%08x = status=%08x, channel=%u\n", dwInstance, dwParam1, dwParam2, status, channel);

		if (midi_port.inp_buffer)
			printf("buff-size: %u rest: %u  tail: %u\n",
				midi_port.inp_buffer->size,
				midi_port.inp_buffer->rest,
				midi_port.inp_buffer->tail);

		switch (status) {
		case 0x80: //Note Off event.
			printf("Release key: %u velocity: %u\n", param1, param2);
			break;
		case 0x90: //Note On event.
			printf("Pressed key: %u velocity: %u\n", param1, param2);
			break;
		case 0xb0: //control change.
			printf("Control num: %u value: %u\n", param1, param2);

			break;
		}
#endif
		Midi_Push_Buffer(midi_port.inp_buffer, dwParam1);
		Midi_Push_Buffer(midi_port.inp_buffer, dwParam2);
		evt.type = EVT_READ;
		evt.data = dwParam1;
		RL_Update_Event(&evt);
		break;
#ifdef DEBUG_MIDI
	case MIM_LONGDATA:
		printf("wMsg=MIM_LONGDATA\n");
		break;
	case MIM_ERROR:
		printf("wMsg=MIM_ERROR\n");
		break;
	case MIM_LONGERROR:
		printf("wMsg=MIM_LONGERROR\n");
		break;
	case MIM_MOREDATA:
		printf("wMsg=MIM_MOREDATA\n");
		break;
#endif
	case MIM_OPEN:
		//printf("wMsg=MIM_OPEN\n");
		evt.type = EVT_OPEN;
		RL_Event(&evt);
		break;
	case MIM_CLOSE:
		//printf("wMsg=MIM_CLOSE\n");
		evt.type = EVT_CLOSE;
		RL_Event(&evt);	// returns 0 if queue is full
		break;
	default:
		//printf("wMsg = unknown\n");
		break;
	}
	return;
}

/***********************************************************************
**
**	MidiOut callback procedure.
**	dwInstance contains id of the MIDI port structure in pool
**
***********************************************************************/
static void CALLBACK MidiOutProc(HMIDIOUT hMidiOut, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	REBMID midi_port;
	
	if (dwInstance >= Midi_Ports_Pool.count) return;
	
	midi_port = Midi_Ports_Pool.ports[dwInstance];
	
	switch (wMsg) {
	case MM_MOM_OPEN:
	case MM_MOM_CLOSE:
		return; // don't report these
	default:
		LOG("MidiOutProc... port: %0p\n", midi_port.port);
		LOG("dwInstance=%u dwParam1=%08x, dwParam2=%08x wMsg=%08x\n", dwInstance, dwParam1, dwParam2, wMsg);
	}
	return;
}

#ifdef DEBUG_MIDI
static void PrintMidiDevices()
{
	UINT nMidiDeviceNum;
	MIDIINCAPS capsInp;
	MIDIOUTCAPS capsOut;

	nMidiDeviceNum = midiInGetNumDevs();
	if (nMidiDeviceNum == 0) {
		fprintf(stderr, "midiInGetNumDevs() return 0...");
	}
	else {
		printf("== Input MidiDevices == \n");
		for (unsigned int i = 0; i < nMidiDeviceNum; ++i) {
			midiInGetDevCaps(i, &capsInp, sizeof(MIDIINCAPS));
			printf("\t%d : name = %S\n", i, capsInp.szPname);
			printf("\t\tManufacturer ID: %d\n", capsInp.wMid);
			printf("\t\tProduct ID:      %d\n", capsInp.wPid);
			printf("\t\tDriver version:  %d\n", capsInp.vDriverVersion);
		}
		printf("=====\n");
	}

	nMidiDeviceNum = midiOutGetNumDevs();
	if (nMidiDeviceNum == 0) {
		fprintf(stderr, "midiInGetNumDevs() return 0...");
	}
	else {
		printf("== Output MidiDevices == \n");
		for (unsigned int i = 0; i < nMidiDeviceNum; ++i) {
			midiOutGetDevCaps(i, &capsOut, sizeof(MIDIOUTCAPS));
			printf("\t%d : name = %S\n", i, capsOut.szPname);
			printf("\t\tManufacturer ID: %d\n", capsOut.wMid);
			printf("\t\tProduct ID:      %d\n", capsOut.wPid);
			printf("\t\tDriver version:  %d\n", capsOut.vDriverVersion);
#if (WINVER >= 0x0400)
			printf("\t\tSupport:         %d\n", capsOut.dwSupport);
#endif
		}
		printf("=====\n");
	}
}
#endif

/***********************************************************************
**
*/	DEVICE_CMD Init_MIDI(REBREQ *req)
/*
***********************************************************************/
{
	//printf("Init_MIDI: sizeof(Midi_Ports_Pool): %d sizeof(REBMID): %d\n", sizeof(Midi_Ports_Pool), sizeof(REBMID));
	Midi_Ports_Pool.count = MIDI_PORTS_ALLOC;
	Midi_Ports_Pool.ports = MAKE_CLEAR_MEM(sizeof(REBMID) * MIDI_PORTS_ALLOC);
	if (!Midi_Ports_Pool.ports) return DR_ERROR;
	return DR_DONE;
}

/***********************************************************************
**
*/	DEVICE_CMD Open_MIDI(REBREQ *req)
/*
***********************************************************************/
{
	HMIDIIN hMidiDevice = NULL;
	HMIDIOUT hMidiDeviceOut = NULL;
	//DWORD nMidiPort = 0;
	MMRESULT rv;
	REBSER *port = req->port;
	REBCNT device_in = req->midi.device_in;
	REBCNT device_out = req->midi.device_out;
	REBCNT port_num;
	REBMID *midi_port = NULL;
#ifdef DEBUG_MIDI
	UINT nMidiDeviceNum;
#endif

	if(!device_in && !device_out) {
		LOG("No devices!\n");
        req->error = MMSYSERR_BADDEVICEID;
        return DR_ERROR;
    }

	port_num = Get_New_Midi_Port(&midi_port);
	if (port_num < 0) {
		LOG("Failed to get new empty MIDI port!\n");
		req->error = MMSYSERR_ERROR;
		return DR_ERROR;
	}

#ifdef DEBUG_MIDI
	printf("Open_MIDI... port: %0p in: %i out: %i\n", port, device_in, device_out);
	PrintMidiDevices();
	printf("new MIDI port NUM: %u\n", port_num);
#endif

	if (
		   (device_in  && device_in  > midiInGetNumDevs())
		|| (device_out && device_out > midiOutGetNumDevs())
	) {
		LOG("Some of the requested MIDI device IDs are out of range!\n");
		req->error = MMSYSERR_BADDEVICEID;
		return DR_ERROR;
	}

	midi_port->port = port;

	if (device_in) {
		midi_port->inp_id = device_in;
		rv = midiInOpen(&midi_port->inp_device, device_in - 1, (DWORD_PTR)MidiInProc, (DWORD_PTR)port_num, CALLBACK_FUNCTION);
		if (rv != MMSYSERR_NOERROR) {
			LOG("midiInOpen() failed...rv=%d\n", rv);
			req->error = rv;
			return DR_ERROR;
		}
		midiInStart(midi_port->inp_device);
	}
	if (device_out) {
		midi_port->out_id = device_out;
		//printf("opening %u => %0X\n", device_out, midi_port->out_device);
		rv = midiOutOpen(
			&midi_port->out_device,
			device_out - 1,
			(DWORD_PTR)MidiOutProc,
			(DWORD_PTR)port_num,
			CALLBACK_FUNCTION);

		if (rv != MMSYSERR_NOERROR) {
			LOG("midiOutOpen() failed...rv=%d\n", rv);
			if (midi_port->inp_device) {
				// closing already opened input device if any 
				midiInStop(midi_port->inp_device);
				midiInClose(midi_port->inp_device);
				midi_port->inp_device = NULL;
			}
			req->error = rv;
			return DR_ERROR;
		}
	}
	if (device_in) {
		midi_port->inp_buffer = RL_Make_String(MIDI_BUF_SIZE-1, FALSE); // allocate input binary ...
		RL_Protect_GC(midi_port->inp_buffer, TRUE);     // ... and prevent it from GC
		midi_port->inp_buffer->size = MIDI_BUF_SIZE;    // ... and init it for use as circular buffer
		midi_port->inp_buffer->rest = 0;                // rest is used as readers TAIL
	}
	req->handle = (void*)midi_port;
	SET_OPEN(req);
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Close_MIDI(REBREQ *req)
/*
***********************************************************************/
{
	REBMID *midi_port= (REBMID *)req->handle;
	//printf("Closing MIDI port: %0X %0X\n", req->port, midi_port);
	if (midi_port->inp_device) {
		midiInStop(midi_port->inp_device);
		midiInClose(midi_port->inp_device);
		midi_port->inp_device = NULL;
	}
	if (midi_port->inp_buffer) RL_Protect_GC(midi_port->inp_buffer, FALSE);
	if (midi_port->out_device) {
		midiOutClose(midi_port->out_device);
		midi_port->out_device = NULL;
	}
	midi_port->port = NULL;
	req->handle = NULL;
	SET_CLOSED(req);
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Read_MIDI(REBREQ *req)
/*
***********************************************************************/
{
	REBMID *midi_port = (REBMID *)req->handle;
	REBSER *buffer = midi_port->inp_buffer;
	REBINT len;

	//printf("Read_MIDI... port: %0X\n", req->port);

	if (buffer == NULL) {
		req->actual = 0;
	} else {
		if (buffer->rest > buffer->tail) {
			len = buffer->size - buffer->rest;
		} else {
			len = buffer->tail - buffer->rest;
		}
		req->data = (buffer->data + buffer->rest);
		req->actual = len;
		buffer->rest = buffer->tail;
	}
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Write_MIDI(REBREQ *req)
/*
**		Works for Unicode and ASCII strings.
**		Length is number of bytes passed (not number of chars).
**
***********************************************************************/
{
	MMRESULT rv;
	REBMID  *midi_port = (REBMID *)req->handle;
	REBYTE  *data_out = req->data;
	REBYTE  *tail = data_out + req->length;

	while (data_out < tail) {
		u8 type = data_out[0] & 0xF0;
		if (type == 0xF0) {
			LOG("sysex not yet supported!\n");
			req->error = 10;
			return DR_ERROR;
		} else  if (type >= 0x80) {
			rv = midiOutShortMsg(midi_port->out_device, ((DWORD*)data_out)[0]);
			//printf("midiOutShortMsg rv=%d\n", rv);
			data_out += 4;
		} else {
			// invalid data
			return DR_ERROR;
		} 
	}
	req->actual = 0;
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Poll_MIDI(REBREQ *req)
/*
***********************************************************************/
{
	LOG("Poll_MIDI\n");
	return DR_DONE;
}

/***********************************************************************
**
*/	DEVICE_CMD Query_MIDI(REBREQ *req)
/*
***********************************************************************/
{
	UINT nMidiDeviceNum, i;
	MIDIINCAPS capsInp;
	MIDIOUTCAPS capsOut;
	REBVAL *val;
	RXIARG arg;

	//puts("Query_MIDI");

	//input devices block
	val = FRM_VALUES((REBSER*)req->data) + 1;
	nMidiDeviceNum = midiInGetNumDevs();
	for (i = 0; i < nMidiDeviceNum; ++i) {
		midiInGetDevCaps(i, &capsInp, sizeof(MIDIINCAPS));
		arg.series = RL_Encode_UTF8_String(capsInp.szPname, (REBCNT)wcslen(capsInp.szPname), TRUE, FALSE);
		arg.index = 0;
		RL_Set_Value(VAL_SERIES(val), i, arg, RXT_STRING);
	}
	//output devices block
	val = FRM_VALUES((REBSER*)req->data) + 2; 
	nMidiDeviceNum = midiOutGetNumDevs();
	for (i = 0; i < nMidiDeviceNum; ++i) {
		midiOutGetDevCaps(i, &capsOut, sizeof(MIDIOUTCAPS));
		arg.series = RL_Encode_UTF8_String(capsOut.szPname, (REBCNT)wcslen(capsOut.szPname), TRUE, FALSE);
		arg.index = 0;
		RL_Set_Value(VAL_SERIES(val), i, arg, RXT_STRING);
	}
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Quit_MIDI(REBREQ *req)
/*
***********************************************************************/
{
	//LOG("Quit_MIDI\n");
	for (REBCNT n = 0; n < Midi_Ports_Pool.count; n++) {
		if (Midi_Ports_Pool.ports[n].port != NULL) {
			req->handle = &Midi_Ports_Pool.ports[n];
			Close_MIDI(req);
		}
	}
	Midi_Ports_Pool.count = 0;
	FREE_MEM(Midi_Ports_Pool.ports);
	return DR_DONE;
}


/***********************************************************************
**
**	Command Dispatch Table (RDC_ enum order)
**
***********************************************************************/

static DEVICE_CMD_FUNC Dev_Cmds[RDC_MAX] =
{
	Init_MIDI,
	Quit_MIDI,
	Open_MIDI,
	Close_MIDI,
	Read_MIDI,
	Write_MIDI,
	Poll_MIDI,
	0,	// connect
	Query_MIDI,
	0,	// modify
	0,	// create
	0,	// delete
	0,	// rename
	0	// lookup
};

DEFINE_DEV(Dev_MIDI, "MIDI", 1, Dev_Cmds, RDC_MAX, 0);

#endif //INCLUDE_MIDI_DEVICE
