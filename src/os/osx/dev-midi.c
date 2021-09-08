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
 **  Title: Device: MIDI access for macOS
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

#if defined(TO_OSX) || defined(TO_OSX_X64) || defined(TO_OSXI) //!!! could be made better! 

// NOTE: this will be useful for higher level launchpad schemes:
// https://github.com/FMMT666/launchpad.py/blob/master/launchpad_py/launchpad.py
// (code includes led/key mapping values)
//*********************************************************************/

#include <CoreMIDI/MIDIServices.h>
#include <CoreFoundation/CFRunLoop.h>
#include <stdio.h>
#include <mach/mach_time.h>

#undef BYTE_SIZE // vm_param.h from above includes define BYTE_SIZE as number of bits
				 // sys-value.h use it to detect if series is byte-sized

#include "reb-host.h"
#include "host-lib.h"
#include "sys-net.h"

#define MIDI_BUF_SIZE (1024 * sizeof(u32))
//#define DEBUG_MIDI

//temp global variables
MIDIClientRef   gClient  = 0;
MIDIPortRef     gInPort  = 0;
MIDIPortRef     gOutPort = 0;
//===========================

// Midi handle structure:
typedef struct midi_port_handle {
    REBSER*  port;
    REBCNT   inp_id;
    REBCNT   out_id;
    MIDIEndpointRef inp_device;
    MIDIEndpointRef out_device;
    REBSER*  inp_buffer;
    REBU64   started;
} REBMID;

#define MIDI_PORTS_ALLOC 16 // number of port handles allocated in the pool

typedef struct midi_ports {
    REBCNT  count;
    REBMID* ports;
} midi_ports;


static midi_ports Midi_Ports_Pool;

static REBINT Get_New_Midi_Port(REBMID **port)
{
    REBINT n;
    for (n = 0; n < Midi_Ports_Pool.count; n++) {
        if (Midi_Ports_Pool.ports[n].port == NULL) {
            *port = &Midi_Ports_Pool.ports[n];
            return n;
        }
    }
    return -1;
}

static REBSER *CFString_To_REBSER(CFStringRef source)
{
    char buffer[64];
    REBSER *ser;
    
    CFStringGetCString(source, buffer, 64, kCFStringEncodingUTF8);
    REBU64 length = strlen(buffer);
    ser = RL_Make_String((REBCNT)length, FALSE);
    COPY_STR(BIN_DATA(ser), buffer, length+1);
    ser->tail = (REBCNT)length;
    return ser;
}

/***********************************************************************
 **
 */ static int Midi_Push_Buffer(REBSER* buffer, u32 data)
/*
 **    Stores data of DWORD size into port's input buffer
 **    Using REBSER as circular buffer.. reusing the existing fields
 **        tail => writers head
 **        rest => readers head
 **        size => maxlen
 ***********************************************************************/
{
    int next = buffer->tail + sizeof(data);
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
    //    printf("buff-size: %u rest: %u  tail: %u\n",
    //        buffer->size,
    //        buffer->rest,
    //        buffer->tail);
    
    return 0;
}

/***********************************************************************
 **
 **    MidiIn callback procedure.
 **    dwInstance contains id of the MIDI port structure in pool
 **
 ***********************************************************************/
static void MidiInProc(const MIDIPacketList *pktlist, void *refCon, void *connRefCon)
{
    REBMID midi_port;
    REBU64 id = (REBU64)refCon;
    
    if (id >= Midi_Ports_Pool.count) return;
    
    midi_port = Midi_Ports_Pool.ports[id];
    
    MIDIPacket *packet = (MIDIPacket *)pktlist->packet; // remove const (!)
    for (unsigned int j = 0; j < pktlist->numPackets; ++j) {
        //printf("packet length: %u time: %llu\n", packet->length, packet->timeStamp);
        if (packet->data[0] >= 0x80 && packet->data[0] < 0xF0) {
            Midi_Push_Buffer(midi_port.inp_buffer, ((u32*)packet->data)[0]);
            // report timestamp in miliseconds like on Windows
            Midi_Push_Buffer(midi_port.inp_buffer, (u32)((packet->timeStamp - midi_port.started) / 1000000UL));
        }
        packet = MIDIPacketNext(packet);
    }
}


#ifdef DEBUG_MIDI
static void PrintMidiDevices()
{
    unsigned long i, n;
    CFStringRef pname, pmanuf, pmodel;
    char name[64], manuf[64], model[64];
    
    n = MIDIGetNumberOfDevices();
    for (i = 0; i < n; ++i) {
        MIDIDeviceRef dev = MIDIGetDevice(i);
        
        MIDIObjectGetStringProperty(dev, kMIDIPropertyName, &pname);
        MIDIObjectGetStringProperty(dev, kMIDIPropertyManufacturer, &pmanuf);
        MIDIObjectGetStringProperty(dev, kMIDIPropertyModel, &pmodel);
        
        CFStringGetCString(pname, name, sizeof(name), 0);
        CFStringGetCString(pmanuf, manuf, sizeof(manuf), 0);
        CFStringGetCString(pmodel, model, sizeof(model), 0);
        CFRelease(pname);
        CFRelease(pmanuf);
        CFRelease(pmodel);
        
        printf("name=%s, manuf=%s, model=%s\n", name, manuf, model);
    }
}
#endif

/***********************************************************************
 **
 */    DEVICE_CMD Init_MIDI(REBREQ *req)
/*
 ***********************************************************************/
{
#ifdef DEBUG_MIDI
    printf("Init_MIDI: sizeof(Midi_Ports_Pool): %lu sizeof(REBMID): %lu\n", sizeof(Midi_Ports_Pool), sizeof(REBMID));
#endif
    if (gClient) return DR_DONE;
    
    MIDIClientCreate(CFSTR("Rebol3 MIDI"), NULL, NULL, &gClient);
    MIDIInputPortCreate(gClient, CFSTR("Input port"), MidiInProc, NULL, &gInPort);
    MIDIOutputPortCreate(gClient, CFSTR("Output port"), &gOutPort);
#ifdef DEBUG_MIDI   
    printf("in %u out %u\n", gInPort, gOutPort);
#endif   
    Midi_Ports_Pool.count = MIDI_PORTS_ALLOC;
    Midi_Ports_Pool.ports = MAKE_MEM(MIDI_PORTS_ALLOC * sizeof(REBMID));
    CLEAR(Midi_Ports_Pool.ports, MIDI_PORTS_ALLOC * sizeof(REBMID));
    return DR_DONE;
}


/***********************************************************************
 **
 */    DEVICE_CMD Open_MIDI(REBREQ *req)
/*
 ***********************************************************************/
{
    REBSER *port = req->port;
    REBCNT device_in = req->midi.device_in;
    REBCNT device_out = req->midi.device_out;
    REBINT port_num;
    REBMID *midi_port = NULL;
    
    port_num = Get_New_Midi_Port(&midi_port);
    if (port_num < 0) {
        puts("Failed to get new empty MIDI port!");
        return DR_ERROR;
    }
    
#ifdef DEBUG_MIDI
    printf("Open_MIDI... port: %0llX in: %i out: %i\n", (REBU64)port, device_in, device_out);
    PrintMidiDevices();
    printf("new MIDI port NUM: %i\n", port_num);
#endif
    
    if (
        (device_in  && device_in  > MIDIGetNumberOfSources())
        || (device_out && device_out > MIDIGetNumberOfDestinations())
        ) {
        puts("Some of the requested MIDI device IDs are out of range!");
        return DR_ERROR;
    }
    
    midi_port->port = port;
    if (device_in) {
        midi_port->inp_id = device_in;
        midi_port->inp_device = MIDIGetSource(device_in-1);
        if (midi_port->inp_device == 0) {
            printf("MIDI failed to open input device %i\n", device_in);
            return DR_ERROR;
        }
        MIDIPortConnectSource(gInPort, midi_port->inp_device, (void*)(REBU64)port_num);
        midi_port->started = mach_absolute_time();
    }
    if (device_out) {
        midi_port->out_id = device_out;
        //printf("opening %u => %0X\n", device_out, midi_port->out_device);
        midi_port->out_device = MIDIGetDestination(device_out-1);
        if (midi_port->out_device == 0) {
            printf("MIDI failed to open output device %i\n", device_out);
            if (midi_port->inp_device) {
                // closing already opened input device if any
                //midiInStop(midi_port->inp_device);
                midi_port->started = 0;
                MIDIPortDisconnectSource(gInPort, midi_port->inp_device);
                midi_port->inp_device = 0;
            }
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
 */    DEVICE_CMD Close_MIDI(REBREQ *req)
/*
 ***********************************************************************/
{
    REBMID *midi_port= (REBMID *)req->handle;
    
#ifdef DEBUG_MIDI
    printf("Closing MIDI port: %0llX %0llX\n", (REBU64)req->port, (REBU64)midi_port);
#endif

    if (midi_port->inp_device) {
        //midiInStop(midi_port->inp_device);
        MIDIPortDisconnectSource(gInPort, midi_port->inp_device);
        midi_port->inp_device = 0;
    }
    if (midi_port->inp_buffer) RL_Protect_GC(midi_port->inp_buffer, FALSE);
    if (midi_port->out_device) {
        midi_port->started = 0;
        midi_port->out_device = 0;
    }
    
    midi_port->port = NULL;
    req->handle = NULL;
    SET_CLOSED(req);
    return DR_DONE;
}


/***********************************************************************
 **
 */    DEVICE_CMD Read_MIDI(REBREQ *req)
/*
 ***********************************************************************/
{
    REBMID *midi_port = (REBMID *)req->handle;
    REBSER *buffer = midi_port->inp_buffer;
    REBINT len;
    
    //printf("Read_MIDI... port: %0llX  buffer %u\n", (REBU64)req->port, midi_port->inp_buffer);
    
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
 */    DEVICE_CMD Write_MIDI(REBREQ *req)
/*
 **        Works for Unicode and ASCII strings.
 **        Length is number of bytes passed (not number of chars).
 **
 ***********************************************************************/
{
    REBMID  *midi_port = (REBMID *)req->handle;
    REBYTE  *data_out = req->data;
    REBYTE  *tail = data_out + req->length;
    MIDIPacketList pktlist;
    MIDIPacket packet;
    
    //printf("Write_MIDI... port: %0llX  device: %u\n", (REBU64)req->port, midi_port->out_device);
    
    if (midi_port->out_device == 0) return DR_ERROR;
    
    pktlist.numPackets = 1;
    
    while (data_out < tail) {
        u8 type = data_out[0] & 0xF0;
        //printf("writing... %u\n", type);
        if (type == 0xF0) {
            puts("sysex not yet supported!");
            return DR_ERROR;
        } else if (type >= 0x80) {
            packet.timeStamp = mach_absolute_time();
            packet.length = 4;
            packet.data[0] = data_out[0];
            packet.data[1] = data_out[1];
            packet.data[2] = data_out[2];
            packet.data[3] = data_out[3];
            pktlist.packet[0] = packet;
            MIDISend(gOutPort, midi_port->out_device, &pktlist);
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
 */    DEVICE_CMD Poll_MIDI(REBREQ *req)
/*
 ***********************************************************************/
{
    puts("Poll_MIDI");
    return DR_DONE;
}

/***********************************************************************
 **
 */    DEVICE_CMD Query_MIDI(REBREQ *req)
/*
 ***********************************************************************/
{
    REBU64 nMidiDeviceNum;
    REBVAL *val;
    RXIARG arg;
    REBCNT i;
    CFStringRef pname;
    
    if (!gClient && (DR_DONE != Init_MIDI(req))) return DR_ERROR;
    
#ifdef DEBUG_MIDI
    printf("Query_MIDI sources: %i dests: %i\n", MIDIGetNumberOfSources(), MIDIGetNumberOfDestinations());
    PrintMidiDevices();
#endif

    //input devices block
    val = FRM_VALUES((REBSER*)req->data) + 1;
    nMidiDeviceNum = MIDIGetNumberOfSources();
    for (i = 0; i < nMidiDeviceNum; ++i) {
        MIDIEndpointRef src = MIDIGetSource(i);
        MIDIObjectGetStringProperty(src, kMIDIPropertyName, &pname);
        arg.series = CFString_To_REBSER(pname);
        arg.index = 0;
        RL_Set_Value(VAL_SERIES(val), i, arg, RXT_STRING);
    }
    //output devices block
    val = FRM_VALUES((REBSER*)req->data) + 2;
    nMidiDeviceNum = MIDIGetNumberOfDestinations();
    for (i = 0; i < nMidiDeviceNum; ++i) {
        MIDIEndpointRef src = MIDIGetSource(i);
        MIDIObjectGetStringProperty(src, kMIDIPropertyName, &pname);
        arg.series = CFString_To_REBSER(pname);
        arg.index = 0;
        RL_Set_Value(VAL_SERIES(val), i, arg, RXT_STRING);
    }
    return DR_DONE;
}


/***********************************************************************
 **
 **    Command Dispatch Table (RDC_ enum order)
 **
 ***********************************************************************/

static DEVICE_CMD_FUNC Dev_Cmds[RDC_MAX] =
{
    Init_MIDI,
    0,
    Open_MIDI,
    Close_MIDI,
    Read_MIDI,
    Write_MIDI,
    Poll_MIDI,
    0,    // connect
    Query_MIDI,
    0,    // modify
    0,    // create
    0,    // delete
    0,    // rename
    0    // lookup
};

DEFINE_DEV(Dev_MIDI, "MIDI", 1, Dev_Cmds, RDC_MAX, 0);

#endif //defined(TO_OSX) || defined(TO_OSX_X64)
#endif //INCLUDE_MIDI_DEVICE
