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
**  Summary: CPU State
**  Module:  sys-state.h
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

// Create this on your local stack frame or globally:
typedef struct {		// State variables to save
	jmp_buf *last_jmp_buf;
	REBINT	dsp;
	REBINT	dsf;
	REBINT	hold_tail;	// Tail for GC_Protect
	REBSER	*error;
	jmp_buf cpu_state;
} REBOL_STATE;

// Save current state info into a structure:
// g is Saved_State or Halt_State.
#define PUSH_STATE(s, g) do {\
		(s).last_jmp_buf = g;\
		(s).dsp = DSP;\
		(s).dsf = DSF;\
		(s).hold_tail = GC_Protect->tail;\
		(s).error = 0;\
	} while(0)

#define POP_STATE(s, g) do {\
		g = (s).last_jmp_buf;\
		DSP = (s).dsp;\
		DSF = (s).dsf;\
		GC_Protect->tail = (s).hold_tail;\
	} while (0)

// Do not restore prior state:
#define DROP_STATE(s, g) g = (s).last_state

// Set the pointer for the prior state:
#define	SET_STATE(s, g) g = &(s).cpu_state

// Store all CPU registers into the structure:
#define	SET_JUMP(s) setjmp((s).cpu_state)
