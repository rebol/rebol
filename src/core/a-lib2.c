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
**  Module:  a-lib2.c
**  Summary: skip
**  Section: environment
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "reb-dialect.h"
#include "reb-ext.h"
#include "sys-state.h"


// Load this to verify function prototypes:
#include "rebol-lib.h"

/***********************************************************************
**
x*/	REBOL_API REBINT Reb_Dialect(REBINT dialect, REBSER *block, REBCNT *index, REBSER **arglist)
/*
**		Process a standard dialect.
**
**		The index points to the next value to interpret and is updated
**		on return (for next loop or error). The system/dialect
**		object is used for the dialect specification.
**
**		A block is returned with the arguments, ordered according
**		to the dialect specification for the command. Note that the
**		returned block is reset and reused with each command. (To
**		minimize GC trash.). The cmd arg returns the command number
**		or error number (when result is zero).
**
**		A zero is returned for errors and end-of-block. For the former
**		an error is returned in cmd. For the latter, cmd is zero.
**
***********************************************************************/
{
	REBVAL *val = Get_System(SYS_DIALECTS, 0);

	if (!IS_OBJECT(val)
		||  dialect <= 0
		||	dialect >= (REBINT)SERIES_TAIL(VAL_OBJ_FRAME(val))
	) {
		return -REB_DIALECT_MISSING;
	}

	val = Get_System(SYS_DIALECTS, dialect);
	if (!IS_OBJECT(val)) return -REB_DIALECT_MISSING;;
	return Do_Dialect(VAL_OBJ_FRAME(val), block, index, arglist);
}


/***********************************************************************
**
x*/	REBOL_API void Reb_Set_Var(void *var, void *value)
/*
***********************************************************************/
{
	Set_Var(var, value); // Check context, index, range
}


/***********************************************************************
**
x*/	REBOL_API REBINT Reb_Map_Words(REBYTE **names, REBCNT *symbols)
/*
**		Given null terminated list of word names, supply the
**		symbol values for those words. Return length.
**		The names must be UTF8 valid.
**
***********************************************************************/
{
	REBINT count = 0;

	for (; *names; names++, count++) {
		*symbols++ = Make_Word(*names, 0);
	}
	*symbols++ = 0;

	return count;
}


/***********************************************************************
**
x*/	REBOL_API REBINT Reb_Find_Word(REBCNT sym, REBCNT *symbols, REBINT limit)
/*
**		Search a symbol list for a word, and return the index for it.
**		Return -1 if not found.  Limit can be used to control how many
**		words in the symbol list will be compared.
**
***********************************************************************/
{
	REBINT index;

	if (sym >= SERIES_TAIL(PG_Word_Table.series)) return -1;
	if (limit == 0) limit = 100000;

	for (index = 0; limit > 0 && symbols[index]; limit--, index++) {
		if (sym == symbols[index] || SYMBOL_TO_CANON(sym) == SYMBOL_TO_CANON(symbols[index]))
			return index;
	}

	return -1;
}
