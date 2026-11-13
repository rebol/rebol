/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  REBOL is a trademark of REBOL Technologies
**
**  Additional code modifications and improvements Copyright 2012 Saphirion AG
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
**  Title: Windowing Event Handler
**  Author: Oldes
**  Purpose: This code handles windowing related events.
**	Related: host-window.c, dev-event.c
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


#include "reb-host.h"
#include "host-lib.h"
#include <gtk/gtk.h>

//***** Globals *****//

REBINT exit_loop = 0;

/***********************************************************************
**
**	Local Functions
**
***********************************************************************/

static void Add_Event_XY(REBGOB *gob, REBINT id, REBINT xy, REBINT flags)
{
	REBEVT evt;

	evt.type  = id;
	evt.flags = (u8) (flags | (1<<EVF_HAS_XY));
	evt.model = EVM_GUI;
	evt.data  = xy;
	evt.ser = (void*)gob;

	RL_Event(&evt);	// returns 0 if queue is full
}

/***********************************************************************
**
**	Events
**
***********************************************************************/

void on_window_removed(GApplication *app, gpointer gob)
{
	exit_loop--;
	printf("GTK window removed; exit_loop: %i; gob: %p\n", exit_loop, gob);
	if( gob ) {
		Add_Event_XY((REBGOB*)gob, EVT_CLOSE, 0, 0);
		OS_Close_Window((REBGOB*)gob);
	}
	if(exit_loop == 0) {
		//OS_Abort_Device();
	}
}

void on_clicked(GtkWidget *widget, gpointer gob)
{
	printf("on_clicked; gob: %p\n", gob);
	Add_Event_XY((REBGOB*)gob, EVT_CLICK, 0, 0);
}