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
**  Module:  u-dialect.c
**  Summary: support for dialecting
**  Section: utility
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"
#include "reb-dialect.h"

typedef struct Reb_Dialect_Parse {
	REBSER *dialect;	// dialect object
	REBSER *fargs;		// formal arg block
	REBCNT fargi;		// start index in fargs
	REBSER *args;		// argument block
	REBCNT argi;		// current arg index
	REBINT cmd;			// command id
	REBINT len;			// limit of current command
	REBSER *out;		// result block
	REBINT outi;		// result block index
	REBINT flags;
	REBINT missed;		// counter of arg misses
	REBVAL *contexts;	// contexts to search for variables
} REBDIA;

enum {
	RDIA_NO_CMD,		// do not store command in block
	RDIA_LIT_CMD,		// 'command
	RDIA_ALL,			// all commands, do not reset output
};

static REBINT Delect_Debug = 0;
static REBINT Total_Missed = 0;
static char *Dia_Fmt = "DELECT - cmd: %s length: %d missed: %d total: %d";


/***********************************************************************
**
*/	static int Find_Command(REBSER *dialect, REBVAL *word)
/*
**		Given a word, check to see if it is in the dialect object.
**		If so, return its index. If not, return 0.
**
***********************************************************************/
{
	REBINT n;

	if (dialect == VAL_WORD_FRAME(word)) n = VAL_WORD_INDEX(word);
	else {
		if (NZ(n = Find_Word_Index(dialect, VAL_WORD_SYM(word), FALSE))) {
			VAL_WORD_FRAME(word) = dialect;
			VAL_WORD_INDEX(word) = n;
		}
		else return 0;
	}

	// If keyword (not command) return negated index:
	if (IS_NONE(FRM_VALUES(dialect) + n)) return -n;
	return n;
}


/***********************************************************************
**
*/	static int Count_Dia_Args(REBVAL *args)
/*
**		Return number of formal args provided to the function.
**		This is just a guess, because * repeats count as zero.
**
***********************************************************************/
{
	REBINT n = 0;

	for (; NOT_END(args); args++) {
		if (IS_WORD(args)) {
			if (VAL_WORD_SYM(args) == SYM__P) { // skip: * type
				if (NOT_END(args+1)) args++;
			} else n++;
		}
		else if (IS_DATATYPE(args) || IS_TYPESET(args)) n++;
	}
	return n;
}


/***********************************************************************
**
*/	static REBVAL *Eval_Arg(REBDIA *dia)
/*
**		Handle all values passed in a dialect.
**
**		Contexts can be used for finding a word in a block of
**		contexts without using a path.
**
**		Returns zero on error.
**		Note: stack used to hold temp values
**
***********************************************************************/
{
	REBVAL *value = BLK_SKIP(dia->args, dia->argi);

	switch (VAL_TYPE(value)) {

	case REB_WORD:
		// Only look it up if not part of dialect:
		if (Find_Command(dia->dialect, value) == 0) {
			REBVAL *val = value; // save
			if (dia->contexts) {
				value = Find_In_Contexts(VAL_WORD_CANON(value), dia->contexts);
				if (value) break;
			}
			value = Get_Var_No_Trap(val); // may return zero
		}
		break;

	case REB_PATH:
		if (Do_Path(&value, 0)) return 0;
		value = DS_TOP;
		break;

	case REB_LIT_WORD:
		DS_PUSH(value);
		value = DS_TOP;
		VAL_SET(value, REB_WORD);
		break;

	case REB_PAREN:
		value = DO_BLK(value);
		DS_SKIP; // do not overwrite TOS
		break;
	}
	
	return value;
}


/***********************************************************************
**
*/	static REBINT Add_Arg(REBDIA *dia, REBVAL *value)
/*
**		Add an actual argument to the output block.
**
**		Note that the argument may be out sequence with the formal
**		arguments so we must scan for a slot that matches.
**
**		Returns:
**		  1: arg matches a formal arg and has been stored
**		  0: no arg of that type was found
**		 -N: error (type block contains a bad value)
**
***********************************************************************/
{
	REBINT type = 0;
	REBINT accept = 0;
	REBVAL *fargs;
	REBINT fargi;
	REBVAL *outp;
	REBINT rept = 0;

	outp = BLK_SKIP(dia->out, dia->outi);

	// Scan all formal args, looking for one that matches given value:
	for (fargi = dia->fargi;; fargi++) {

		//Debug_Fmt("Add_Arg fargi: %d outi: %d", fargi, outi);
		
		if (IS_END(fargs = BLK_SKIP(dia->fargs, fargi))) return 0;

again:
		// Formal arg can be a word (type or refinement), datatype, or * (repeater):
		if (IS_WORD(fargs)) {

			// If word is a datatype name:
			type = VAL_WORD_CANON(fargs);
			if (type < REB_MAX) {
				type--;	// the type id
			}
			else if (type == SYM__P) {
				// repeat: * integer!
				rept = 1;
				fargs++;
				goto again;
			}
			else {
				// typeset or refinement
				REBVAL *temp;

				type = -1;

				// Is it a refinement word?
				if (IS_WORD(value) && VAL_WORD_CANON(fargs) == VAL_WORD_CANON(value)) {
					accept = 4;
				}
				// Is it a typeset?
				else if (NZ(temp = Get_Var_No_Trap(fargs)) && IS_TYPESET(temp)) {
					if (TYPE_CHECK(temp, VAL_TYPE(value))) accept = 1;
				}
				else if (!IS_WORD(value)) return 0; // do not search past a refinement
				//else return -REB_DIALECT_BAD_SPEC;
			}
		}
		// It's been reduced and is an actual datatype or typeset:
		else if (IS_DATATYPE(fargs)) {
			type = VAL_DATATYPE(fargs);
		}
		else if (IS_TYPESET(fargs)) {
			if (TYPE_CHECK(fargs, VAL_TYPE(value))) accept = 1;
		} else
			return -REB_DIALECT_BAD_SPEC;

		// Make room for it in the output block:
		if (IS_END(outp))
			outp = Append_Value(dia->out);
		else if (!IS_NONE(outp)) {
			// There's already an arg in this slot, so skip it...
			if (dia->cmd > 1) outp++;	
			if (!rept) continue; // see if there's another farg that will work for it
			// Look for first empty slot:
			while (NOT_END(outp) && !IS_NONE(outp)) outp++;
			if (IS_END(outp)) outp = Append_Value(dia->out);
		}

		// The datatype was correct from above!
		if (accept) break;

		//Debug_Fmt("want: %d got: %d rept: %d", type, VAL_TYPE(value), rept);

		// Direct match to datatype or to integer/decimal coersions:
		if (type == (REBINT)VAL_TYPE(value)) {
			accept = 1;
			break;
		}
		else if (type == REB_INTEGER && IS_DECIMAL(value)) {
			accept = 2;
			break;
		}
		else if (type == REB_DECIMAL && IS_INTEGER(value)) {
			accept = 3;
			break;
		}

		dia->missed++;				// for debugging

		// Repeat did not match, so stop repeating and remove unused output slot:
		if (rept) {
			Remove_Last(dia->out);
			outp--;
			rept = 0;
			continue;
		}

		if (dia->cmd > 1) outp++;	// skip output slot (for non-default values)
	}

	// Process the result:
	switch (accept) {

	case 1:
		*outp = *value;
		break;

	case 2:
		SET_INTEGER(outp, (REBI64)VAL_DECIMAL(value));
		break;

	case 3:
		SET_DECIMAL(outp, (REBDEC)VAL_INT64(value));
		break;

	case 4:	// refinement:
		dia->fargi = fargs - BLK_HEAD(dia->fargs) + 1;
		dia->outi = outp - BLK_HEAD(dia->out) + 1;
		*outp = *value;
		return 1;

	case 0:
		return 0;
	}

	// Optimization: arg was in correct order:
	if (!rept && fargi == (signed)(dia->fargi)) {
		dia->fargi++;
		dia->outi++;
	}

	return 1;
}


/***********************************************************************
**
*/	static REBINT Do_Cmd(REBDIA *dia)
/*
**		Returns the length of command processed or error. See below.
**
***********************************************************************/
{
	REBVAL *fargs;
	REBINT size;
	REBVAL *val;
	REBINT err;
	REBINT n;

	// Get formal arguments block for this command:
	fargs = FRM_VALUES(dia->dialect) + dia->cmd;
	if (!IS_BLOCK(fargs)) return -REB_DIALECT_BAD_SPEC;
	dia->fargs = VAL_SERIES(fargs);
	fargs = VAL_BLK_DATA(fargs);
	size = Count_Dia_Args(fargs); // approximate

	// Preallocate output block (optimize for large blocks):
	if (dia->len > size) size = dia->len;
	if (GET_FLAG(dia->flags, RDIA_ALL)) {
		Extend_Series(dia->out, size+1);
	}
	else {
		Resize_Series(dia->out, size+1); // tail = 0
		CLEAR_SERIES(dia->out); // Be sure it is entirely cleared
	}

	// Insert command word:
	if (!GET_FLAG(dia->flags, RDIA_NO_CMD)) {
		val = Append_Value(dia->out);
		Set_Word(val, FRM_WORD_SYM(dia->dialect, dia->cmd), dia->dialect, dia->cmd);
		if (GET_FLAG(dia->flags, RDIA_LIT_CMD)) VAL_SET(val, REB_LIT_WORD);
		dia->outi++;
		size++;
	}
	if (dia->cmd > 1) dia->argi++; // default cmd has no word arg

	// Foreach argument provided:
	for (n = dia->len; n > 0; n--, dia->argi++) {
		val = Eval_Arg(dia);
		if (!val)
			return -REB_DIALECT_BAD_ARG;
		if (IS_END(val)) break;
		if (!IS_NONE(val)) {
			//Print("n %d len %d argi %d", n, dia->len, dia->argi);
			err = Add_Arg(dia, val); // 1: good, 0: no-type, -N: error
			if (err == 0) return n; // remainder
			if (err < 0) return err;
		}
	}

	// If not enough args, pad with NONE values:
	if (dia->cmd > 1) {
		for (n = SERIES_TAIL(dia->out); n < size; n++) {
			Append_Value(dia->out);
		}
	}

	dia->outi = SERIES_TAIL(dia->out);

	return 0;
}


/***********************************************************************
**
*/	static REBINT Do_Dia(REBDIA *dia)
/*
**		Process the next command in the dialect.
**		Returns the length of command processed.
**		Zero indicates end of block.
**		Negative indicate error.
**		The args holds resulting args.
**
***********************************************************************/
{
	REBVAL *next = BLK_SKIP(dia->args, dia->argi);
	REBVAL *head;
	REBINT err;

	if (IS_END(next)) return 0;

	// Find the command if a word is provided:
	if (IS_WORD(next) || IS_LIT_WORD(next)) {
		if (IS_LIT_WORD(next)) SET_FLAG(dia->flags, RDIA_LIT_CMD);
		dia->cmd = Find_Command(dia->dialect, next);
	}

	// Handle defaults - process values before a command is reached:
	if (dia->cmd <= 1) {
		dia->cmd = 1;
		dia->len = 1;
		err = Do_Cmd(dia); // DEFAULT cmd
		// It must be processed, else it is not in the dialect.
		// Check for noop result:
		if (err > 0) err = -REB_DIALECT_BAD_ARG;
		return err;
	}

	// Delimit the command - search for next command or end:
	for (head = ++next; NOT_END(next); next++) {
		if ((IS_WORD(next) || IS_LIT_WORD(next)) && Find_Command(dia->dialect, next) > 1) break;
	}

	// Note: command may be shorter than length provided here (defaults):
	dia->len = next - head; // length of args, not including command
	err = Do_Cmd(dia);
	if (GET_FLAG(dia->flags, RDIA_LIT_CMD)) dia->cmd += DIALECT_LIT_CMD;
	return err;
}


/***********************************************************************
**
*/	REBINT Do_Dialect(REBSER *dialect, REBSER *block, REBCNT *index, REBSER **out)
/*
**		Format for dialect is:
**			CMD arg1 arg2 arg3 CMD arg1 arg2 ...
**
**		Returns:
**			cmd value or error as result (or zero for end)
**			index is updated
**			if *out is zero, then we create a new output block
**
**		The arg sequence is terminated by:
**			1. Maximum # of args for command
**			2. An arg that is not of a specified datatype for CMD
**			3. Encountering a new CMD
**			4. End of the dialect block
**
***********************************************************************/
{
	REBDIA dia;
	REBINT n;
	REBINT dsp = DSP; // Save stack position

	CLEARS(&dia);

	if (*index >= SERIES_TAIL(block)) return 0; // end of block

	DISABLE_GC; // Avoid GC during Dialect (prevents unknown crash problem)

	if (!*out) *out = Make_Block(25);

	dia.dialect = dialect;
	dia.args = block;
	dia.argi = *index;
	dia.out  = *out;	
	SET_FLAG(dia.flags, RDIA_NO_CMD);

	//Print("DSP: %d Dinp: %r - %m", DSP, BLK_SKIP(block, *index), block);
	n = Do_Dia(&dia);
	//Print("DSP: %d Dout: CMD: %d %m", DSP, dia.cmd, *out);
	DSP = dsp; // pop any temp values used above

	if (Delect_Debug > 0) {
		Total_Missed += dia.missed;
		// !!!! debug
		if (dia.missed) Debug_Fmt(Dia_Fmt, Get_Field_Name(dia.dialect, dia.cmd), dia.out->tail, dia.missed, Total_Missed);
	}

	if (n < 0) return n; //error
	*index = dia.argi;

	ENABLE_GC;

	return dia.cmd;
}


/***********************************************************************
**
*/	REBNATIVE(delect)
/*
***********************************************************************/
{
	REBDIA dia;
	REBINT err;
	REBFLG all;

	CLEARS(&dia);

	dia.dialect = VAL_OBJ_FRAME(D_ARG(1));
	dia.args = VAL_SERIES(D_ARG(2));
	dia.argi = VAL_INDEX(D_ARG(2));
	dia.out = VAL_SERIES(D_ARG(3));	
	dia.outi = VAL_INDEX(D_ARG(3));

	if (dia.argi >= SERIES_TAIL(dia.args)) return R_NONE; // end of block

	if (D_REF(4)) { // in
		if (!IS_BLOCK(dia.contexts = D_ARG(5))) Trap_Arg(dia.contexts);
		dia.contexts = VAL_BLK_DATA(dia.contexts);
	}

	if (NZ(all = D_REF(6))) {
		SET_FLAG(dia.flags, RDIA_ALL);
		Resize_Series(dia.out, VAL_LEN(D_ARG(2)));
		while (TRUE) {
			dia.cmd = 0;
			dia.len = 0;
			dia.fargi = 0;
			err = Do_Dia(&dia);
			//Debug_Fmt("Ret: %d argi: %d outi: %d len: %d", err, dia.argi, dia.outi, dia.len);
			if (err < 0 || IS_END(BLK_SKIP(dia.args, dia.argi))) break;
		}
	}
	else
		err = Do_Dia(&dia);

	DS_RELOAD(ds);

	VAL_INDEX(D_ARG(2)) = MIN(dia.argi, SERIES_TAIL(dia.args));

	if (Delect_Debug > 0) {
		Total_Missed += dia.missed;
		if (dia.missed) Debug_Fmt(Dia_Fmt, Get_Field_Name(dia.dialect, dia.cmd), dia.out->tail, dia.missed, Total_Missed);
	}

	if (err < 0) Trap_Arg(D_ARG(2)); // !!! needs better error

	return R_ARG2;
}


/***********************************************************************
**
*/	void Trace_Delect(REBINT level)
/*
***********************************************************************/
{
	Delect_Debug = level;
}
