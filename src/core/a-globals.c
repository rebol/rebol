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
**  Module:  a-globals.c
**  Summary: global variables
**  Section: environment
**  Author:  Carl Sassenrath
**  Notes:
**	  There are two types of global variables:
**		process vars - single instance for main process
**		thread vars - duplicated within each R3 task
**
***********************************************************************/

/* To do: there are still a few globals in various modules that need to be
** incorporated back into sys-globals.h.
*/

#include "sys-core.h"

#undef PVAR
#undef TVAR

#define PVAR
#define TVAR THREAD

#include "sys-globals.h"
