/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2023 Rebol Open Source Contributors
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
**  Module:  c-handle.c
**  Summary: handle's related functions
**  Section: core
**  Author:  Oldes
**  Notes:
**    Initially handles were just user transparent pointer holders.
**    But it seems to be more useful to have it as a more advanced type,
**    with possibility to register a function for releasing resources
**    from GC when handle value is no more needed.
**
***********************************************************************/

#include "sys-core.h"

#define MAX_HANDLE_TYPES 64


/***********************************************************************
**
*/	REBCNT Register_Handle(REBCNT sym, REBCNT size, REB_HANDLE_FREE_FUNC free_func)
/*
**      Stores handle's specification (required data size and optional free callback.
**		Returns table index for the word (whether found or new).
**      Returns NOT_FOUND if handle with give ID is already registered.
**
***********************************************************************/
{
	REBCNT idx;
	REBVAL *handles = Get_System(SYS_CATALOG, CAT_HANDLES);

	if (!sym) return NOT_FOUND;

	//printf("Register_Handle: %s with size %u\n", SYMBOL_TO_NAME(sym),  size);

	idx = Find_Handle_Index(sym);
	if (idx != NOT_FOUND)
		return idx;            //TODO: make sure, that the already registered handle is compatible!
	idx = VAL_TAIL(handles);
	if (idx >= MAX_HANDLE_TYPES)
		Crash(RP_MAX_HANDLES); //TODO: realloc PG_Handles instead!

	REBVAL *val = Append_Value(VAL_SERIES(handles));
	Set_Word(val, sym, 0, 0);

	PG_Handles[idx].size = size;
	PG_Handles[idx].free = free_func;
	
	return idx;
}

/***********************************************************************
**
*/	REBCNT Register_Handle_Spec(REBCNT sym, REBHSP *spec)
/*
**      Stores handle's specification (required data size and optional free callback.
**		Returns table index for the word (whether found or new).
**      Returns NOT_FOUND if handle with give ID is already registered.
**
***********************************************************************/
{
	REBCNT idx;
	REBVAL *handles = Get_System(SYS_CATALOG, CAT_HANDLES);

	if (!sym) return NOT_FOUND;

	//printf("Register_Handle: %s with size %u\n", SYMBOL_TO_NAME(sym),  size);

	idx = Find_Handle_Index(sym);
	if (idx != NOT_FOUND) {
		// Handle with this name already exists... what to do?
		// So far at least check, if the size is same!
		if (PG_Handles[idx].size != spec->size) {
			Trap1(RE_HANDLE_EXISTS, VAL_BLK_SKIP(handles, idx));
		}
		return idx;
	}
	idx = VAL_TAIL(handles);
	if (idx >= MAX_HANDLE_TYPES)
		Crash(RP_MAX_HANDLES); //TODO: realloc PG_Handles instead!

	REBVAL *val = Append_Value(VAL_SERIES(handles));
	Set_Word(val, sym, 0, 0);

	PG_Handles[idx].size     = spec->size;
	PG_Handles[idx].flags    = spec->flags;  // reserved for future
	PG_Handles[idx].free     = spec->free;
	PG_Handles[idx].mold     = spec->mold;
	PG_Handles[idx].get_path = spec->get_path;
	PG_Handles[idx].set_path = spec->set_path;
	
	return idx;
}

/***********************************************************************
**
*/	REBHOB* Make_Handle_Context(REBCNT sym)
/*
**		Allocates memory large enough to hold given handle's id
**      storing reference to it using HOB's memory pool.
**
***********************************************************************/
{
	REBHSP spec;
	REBCNT size;
	REBHOB *hob;
	REBCNT idx = Find_Handle_Index(sym);
	if (idx == NOT_FOUND) return NULL;
	
	spec = PG_Handles[idx];
	size = spec.size;

	//printf("Requested HOB for %s (%u) of size %u\n", SYMBOL_TO_NAME(sym), sym, size);
	hob = (REBHOB*)Make_Node(HOB_POOL);
	hob->data  = MAKE_MEM(size);
	hob->index = idx;
	hob->flags = HANDLE_CONTEXT;
	hob->sym   = sym;
	CLEAR(hob->data, size);
	USE_HOB(hob);
	//printf("HOB %p made mem: %p\n", hob, hob->data);
	return hob;
}

/***********************************************************************
**
*/	REBCNT Find_Handle_Index(REBCNT sym)
/*
**		Finds handle's word in system/catalog/handles and returns it's index
**      Returns NOT_FOUND if handle is not found.
**
***********************************************************************/
{
	REBCNT idx = 0;
	REBVAL *handle = VAL_BLK(Get_System(SYS_CATALOG, CAT_HANDLES));

	while(IS_WORD(handle)){
		if(VAL_WORD_SYM(handle) == sym) return idx;
		idx++; handle++;
	}
	return NOT_FOUND;
}

/***********************************************************************
**
*/	void Init_Handles(void)
/*
**		Creates handles table
**
***********************************************************************/
{
	REBVAL *handles;

	REBSER *handle_names = Make_Block(MAX_HANDLE_TYPES);
	handles = Get_System(SYS_CATALOG, CAT_HANDLES);
	Set_Block(handles, handle_names);
	PG_Handles = (REBHSP*)MAKE_MEM(MAX_HANDLE_TYPES * sizeof(REBHSP));
	CLEAR(PG_Handles, MAX_HANDLE_TYPES * sizeof(REBHSP));

#ifdef INCLUDE_MBEDTLS
	//Init_MbedTLS(); // not yet public!
#endif
#ifdef INCLUDE_CRYPTOGRAPHY
	Init_Crypt(); // old crypt code handles
#endif
}
