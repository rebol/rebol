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
**  Module:  c-task.c
**  Summary: sub-task support
**  Section: core
**  Author:  Carl Sassenrath
**  Notes:   INCOMPLETE IMPLEMENTATION (partially operational)
**
***********************************************************************/

/*
	Making a Task:

	1. Local copies of:
		Main globals
			For data stack
			Interpreter flags
			Memory management
		Root series (all or part?)
		Data stack
		System object (for module)
		C stack (thread provided)

	2. Share copies of:
		Boot strings and values
		System functions (natives and mezzanine)
		Word table
		Various sub-objects of system object

	Task Spec is a module definition. Needs new context.

	Questions:
		System object is already copied for local user context
		System blocks might hold references to local series (how to GC)
		Can system values (objects and functions) be modified by other
		tasks? How are they protected? Is it good enough that our local
		references to functions refer to the older ones? How can we
		"update" our references?
*/

#include "sys-core.h"

/***********************************************************************
**
*/	static void Launch_Task(REBVAL *task)
/*
***********************************************************************/
{
	REBSER *body;

	Debug_Str("Begin Task");

	Init_Task();
	body = Clone_Block(VAL_MOD_BODY(task));
	OS_TASK_READY(0);
	Do_Blk(body, 0);

	Debug_Str("End Task");
}


/***********************************************************************
**
*/	void Do_Task(REBVAL *task)
/*
***********************************************************************/
{
	OS_CREATE_THREAD((void*)Launch_Task, task, 50000);
}
