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
**  Summary: REBOL Stack Definitions
**  Module:  sys-stack.h
**  Author:  Carl Sassenrath
**  Notes:
**
**	DSP: index to the top of stack (active value)
**	DSF: index to the base of stack frame (return value)
**
**	Stack frame format:
**
**		   +---------------+
**	DSF->0:| Return Value  | normally becomes TOS after func return
**		   +---------------+
**		 1:|  Prior Frame  | old DSF, block, and block index
**		   +---------------+
**		 2:|   Func Word   | for backtrace info
**		   +---------------+
**		 3:|   Func Value  | in case value is moved or modified
**		   +---------------+
**		 4:|     Arg 1     | args begin here
**		   +---------------+
**		   |     Arg 2     |
**		   +---------------+
**
***********************************************************************/

// Special stack controls (used by init and GC):
#define DS_RESET		(DSP=DSF=0)
#define DS_TERMINATE	(SERIES_TAIL(DS_Series) = DSP+1);

// Access value at given stack location:
#define DS_VALUE(d)		(&DS_Base[d])

// Stack pointer based actions:
#define DS_POP			(&DS_Base[DSP--])
#define DS_TOP			(&DS_Base[DSP])
#define DS_NEXT			(&DS_Base[DSP+1])
#define DS_SKIP			(DSP++)
#define DS_DROP			(DSP--)
#define DS_GET(d)		(&DS_Base[d])
#define DS_PUSH(v)		(DS_Base[++DSP]=*(v))		// atomic
#define DS_PUSH_UNSET	SET_UNSET(&DS_Base[++DSP])	// atomic
#define DS_PUSH_NONE	SET_NONE(&DS_Base[++DSP])	// atomic
#define DS_PUSH_TRUE	VAL_SET(&DS_Base[++DSP], REB_LOGIC), \
						VAL_LOGIC(&DS_Base[DSP]) = TRUE // not atomic
#define DS_PUSH_INTEGER(n)	VAL_SET(&DS_Base[++DSP], REB_INTEGER), \
						VAL_INT64(&DS_Base[DSP]) = n // not atomic
#define DS_PUSH_DECIMAL(n)	VAL_SET(&DS_Base[++DSP], REB_DECIMAL), \
						VAL_DECIMAL(&DS_Base[DSP]) = n // not atomic

// References from DSF (stack frame base, RETURN value index):
#define DSF_SIZE		3					// from DSF to ARGS-1
#define DSF_BIAS		(DSF_SIZE+1)		// from RETURN to DSP
#define DSF_RETURN(d)	(&DS_Base[d])		// return value
#define	DSF_BACK(d)		(&DS_Base[(d)+1])	// block, index, prior DSF (VAL_BACK)
#define DSF_WORD(d)		(&DS_Base[(d)+2])	// func word backtrace
#define DSF_FUNC(d)		(&DS_Base[(d)+3])	// function value saved
#define DSF_ARGS(d,n)	(&DS_Base[(d)+DSF_SIZE+(n)])
#define PRIOR_DSF(d)	VAL_BACK(DSF_BACK(d))

// Reference from ds that points to current return value:
#define	D_RET			(ds)
#define D_ARG(n)		(ds+(DSF_SIZE+n))
#define D_REF(n)		(!IS_NONE(D_ARG(n)))

// Reference from current DSF index:
#define DS_ARG_BASE		(DSF+DSF_SIZE)
#define DS_ARG(n)		DSF_ARGS(DSF, n)
#define DS_REF(n)		(!IS_NONE(DS_ARG(n)))
#define DS_ARGC			(DSP-DS_ARG_BASE)

// RETURN operations:
#define DS_RETURN		(&DS_Base[DSF])
#define DS_RET_VALUE(v)	(*DS_RETURN=*(v))
#define DS_RET_INT(n)	VAL_SET(DS_RETURN, REB_INTEGER), \
						VAL_INT64(DS_RETURN) = n // not atomic

// Helpers:
#define DS_RELOAD(d)	(d = DS_RETURN)
#define	SET_BACK(v,b,i,f) VAL_SET((v), REB_BLOCK), VAL_SERIES(v)=(b), \
						VAL_INDEX(v)=i, VAL_BACK(v)=f

enum {
	R_RET = 0,
	R_TOS,
	R_TOS1,
	R_NONE,
	R_UNSET,
	R_TRUE,
	R_FALSE,
	R_ARG1,
	R_ARG2,
	R_ARG3
};
