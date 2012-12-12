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
**  Summary: Extensions Include File
**  Module:  reb-ext.h
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "reb-defs.h"
#include "ext-types.h"

/* Prefix naming conventions:

  RL:  REBOL library API function (or function access macro)
  RXI: REBOL eXtensions Interface (general constructs)
  RXA: REBOL eXtensions function Argument (value)
  RXR: REBOL eXtensions function Return types
  RXE: REBOL eXtensions Error codes
  RXC: REBOL eXtensions Callback flag

*/


// Value structure (for passing args to and from):
#pragma pack(4)
typedef union rxi_arg_val {
	void *addr;
	i64    int64;
	double dec64;
	REBXYF pair;
	REBYTE bytes[8];
	struct {
		i32 int32a;
		i32 int32b;
	};
	struct {
		REBD32 dec32a;
		REBD32 dec32b;
	};
	struct {
		void *series;
		u32 index;
	};
	struct {
		void *image;
		int width:16;
		int height:16;
	};
} RXIARG;

// For direct access to arg array:
#define RXI_COUNT(a)	(a[0].bytes[0])
#define RXI_TYPE(a,n)	(a[0].bytes[n])

// Command function call frame:
typedef struct rxi_cmd_frame {
	RXIARG args[8];	// arg values (64 bits each)
} RXIFRM;

typedef struct rxi_cmd_context {
	void *envr;		// for holding a reference to your environment
	REBSER *block;	// block being evaluated
	REBCNT index;	// 0-based index of current command in block
} REBCEC;

typedef int (*RXICAL)(int cmd, RXIFRM *args, REBCEC *ctx);

#pragma pack()

// Access macros (indirect access via RXIFRM pointer):
#define RXA_ARG(f,n)	((f)->args[n])
#define RXA_COUNT(f)	(RXA_ARG(f,0).bytes[0]) // number of args
#define RXA_TYPE(f,n)	(RXA_ARG(f,0).bytes[n]) // types (of first 7 args)
#define RXA_REF(f,n)	(RXA_ARG(f,n).int32a)

#define RXA_INT64(f,n)	(RXA_ARG(f,n).int64)
#define RXA_INT32(f,n)	(i32)(RXA_ARG(f,n).int64)
#define RXA_DEC64(f,n)	(RXA_ARG(f,n).dec64)
#define RXA_LOGIC(f,n)	(RXA_ARG(f,n).int32a)
#define RXA_CHAR(f,n)	(RXA_ARG(f,n).int32a)
#define RXA_TIME(f,n)	(RXA_ARG(f,n).int64)
#define RXA_DATE(f,n)	(RXA_ARG(f,n).int32a)
#define RXA_WORD(f,n)	(RXA_ARG(f,n).int32a)
#define RXA_PAIR(f,n)	(RXA_ARG(f,n).pair)
#define RXA_TUPLE(f,n)	(RXA_ARG(f,n).bytes)
#define RXA_SERIES(f,n)	(RXA_ARG(f,n).series)
#define RXA_INDEX(f,n)	(RXA_ARG(f,n).index)
#define RXA_OBJECT(f,n)	(RXA_ARG(f,n).addr)
#define RXA_MODULE(f,n)	(RXA_ARG(f,n).addr)
#define RXA_HANDLE(f,n)	(RXA_ARG(f,n).addr)
#define RXA_IMAGE(f,n)	(RXA_ARG(f,n).image)
#define RXA_IMAGE_BITS(f,n)	  ((REBYTE *)RL_SERIES((RXA_ARG(f,n).image), RXI_SER_DATA))
#define RXA_IMAGE_WIDTH(f,n)  (RXA_ARG(f,n).width)
#define RXA_IMAGE_HEIGHT(f,n) (RXA_ARG(f,n).height)

// Command function return values:
enum rxi_return {
	RXR_UNSET,
	RXR_NONE,
	RXR_TRUE,
	RXR_FALSE,

	RXR_VALUE,
	RXR_BLOCK,
	RXR_ERROR,
	RXR_BAD_ARGS,
	RXR_NO_COMMAND,
};

// Used with RXI_SERIES_INFO:
enum {
	RXI_SER_DATA,	// pointer to data
	RXI_SER_TAIL,	// series tail index (length of data)
	RXI_SER_SIZE,	// size of series (in units)
	RXI_SER_WIDE,	// width of series (in bytes)
	RXI_SER_LEFT,	// units free in series (past tail)
};

// Error Codes (returned in result value from some API functions):
enum {
	RXE_NO_ERROR,
	RXE_NO_WORD,	// the word cannot be found (e.g. in an object)
	RXE_NOT_FUNC,	// the value is not a function (for callback)
	RXE_BAD_ARGS,	// function arguments to not match
};

#define SET_EXT_ERROR(v,n) ((v)->int32a = (n))
#define GET_EXT_ERROR(v)   ((v)->int32a)

typedef struct rxi_callback_info {
	u32 flags;
	REBSER *obj;	// object that holds the function
	u32 word;		// word id for function (name)
	RXIARG *args;	// argument list for function
	RXIARG result;	// result from function
} RXICBI;

enum {
	RXC_NONE,
	RXC_ASYNC,		// async callback
	RXC_QUEUED,		// pending in event queue
	RXC_DONE,		// call completed, structs can be freed
};
