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
**  Title: Test for Embedded Extension Modules
**  Author: Carl Sassenrath
**  Purpose:
**      Provides test code for extensions that can be easily
**		built and run in the host-kit. Not part of release,
**		but can be used as an example.
**  See: http://www.rebol.com/r3/docs/concepts/extensions-embedded.html
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

#ifdef TEST_EXTENSIONS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reb-host.h"
#include "host-lib.h"
#include "sys-value.h"

RL_LIB *RL; // Link back to reb-lib from embedded extensions

static REBCNT Handle_XTest;
typedef struct XTest_Context {
	REBCNT id;
	REBINT num;
} XTEST;

char *RX_Spec =
	"REBOL [\n"
		"Title: {Test of Embedded Extension}\n"
		"Name: ext-test\n"
		"Type: module\n"
		"Options: [boot extension]\n"
		"Exports: [xtest]\n"
	"]\n"

	"xarg0:  command [{return zero}]\n"
	"xarg1:  command [{return first arg} arg]\n"
	"xarg2:  command [{return second arg} arg1 arg2]\n"
	"xword0: command [{return system word from internal string}]\n"
	"xword1: command [{return word from string} str [string!]]\n"
	"xobj1:  command [{return obj field value} obj [object!] field [word! lit-word!]]\n"
	"calls:  command [{test sync callback} context [object!] word [word!]]\n"
	"calla:  command [{test async callback} context [object!] word [word!]]\n"
	"img0:   command [{return 10x20 image}]\n"
	"cec0:   command [{test command context struct} blk [block!]]\n"
	"cec1:   command [{returns cec.index value or -1 if no cec}]\n"
	"hndl1:  command [{creates a handle}]\n"
	"hndl2:  command [{return handle's internal value as integer} hnd [handle!]]\n"
	"vec0:   command [{return vector size in bytes} v [vector!]]\n"
	"vec1:   command [{return vector size in bytes (from object)} o [object!]]\n"
	"blk1:   command [{print type ids of all values in a block} b [block!]]\n"
	"hob1:   command [{creates XTEST handle}]"
	"hob2:   command [{prints XTEST handle's data} hndl [handle!]]"
	"str0:   command [{return a constructed string}]"

	"a: b: c: h: x: none\n"
	"xtest: does [\n"
		"foreach blk [\n"
			"[x: hob1]"
			"[hob2 x]"
			"[h: hndl1]\n"
			"[hndl2 h]\n"
			"[xarg0]\n"
			"[xarg1 111]\n"
			"[xarg1 1.1]\n"
			"[xarg1 {test}]\n"
			"[xarg1 [1 2 3]]\n"
			"[xarg1 10-Sep-2010]\n"
			"[xarg2 111 222]\n"
			"[xword0]\n"
			"[xword1 {system}]\n"
			"[xobj1 system 'version]\n"


			// We just use this context as example. Normally, it would be
			// your own object that has your special functions within it.
			"[calls lib 'negate]\n"
			"[calls lib 'sine]\n"
			"[calla lib 'print]\n"
			"[img0]\n"
			"[c: do-commands [a: xarg0 b: xarg1 333 xobj1 system 'version] reduce [a b c]]\n"
			"[cec0 [a: cec1 b: cec1 c: cec1] reduce [a b c]]\n"
			"[vec0 make vector! [integer! 16 [1 2 3]]]\n"
			"[vec1 object [v: make vector! [integer! 16 [1 2 3]]]]\n"
			"[blk1 [read %img /at 1]]\n"
			"[str0]"
		"][\n"
			"print [{^/^[[7mtest:^[[0m^[[1;32m} mold blk {^[[0m}]\n"
			//"replace {x} {x} {y}\n"
			"print join {^[[1;33m} [do blk {^[[m}]\n"
		"]\n"
	"prin {^/^[[7mAsync call result (should be printed 1234):^[[0m }"
		"wait 0.1 ; let async events happen\n"
		"exit\n"
	"]\n"
	//"print {^/Loaded resident extension: Test - type xtest^/^/}\n"
;


REBCNT Test_Sync_Callback(REBSER *obj, REBCNT word, RXIARG *result)
{
	RXICBI cbi;
	RXIARG args[4];
	REBCNT n;

	printf("Test_Sync_Callback: ");

	// These can be on the stack, because it's synchronous.
	CLEAR(&cbi, sizeof(cbi));
	CLEAR(&args[0], sizeof(args));
	cbi.obj = obj;
	cbi.word = word;
	cbi.args = args;

	// Pass a single integer arg to the callback function:
	RXI_COUNT(args) = 1;
	RXI_TYPE(args, 1) = RXT_INTEGER;

	args[1].int64 = 123;

	n = RL_CALLBACK(&cbi);

	*result = cbi.result;
	return n;
}


REBCNT Test_Async_Callback(REBSER *obj, REBCNT word)
{
	RXICBI *cbi;
	RXIARG *args;
	REBCNT n;

	printf("Test_Async_Callback: ");

	// These cannot be on the stack, because they are used
	// when the callback happens later.
	cbi = MAKE_NEW(*cbi);
	CLEAR(cbi, sizeof(cbi));
	args = MAKE_MEM(sizeof(RXIARG) * 4);
	CLEAR(args, sizeof(RXIARG) * 4);
	cbi->obj = obj;
	cbi->word = word;
	cbi->args = args;
	SET_FLAG(cbi->flags, RXC_ASYNC);

	// Pass a single integer arg to the callback function:
	RXI_COUNT(args) = 1;
	RXI_TYPE(args, 1) = RXT_INTEGER;

	args[1].int64 = 1234;

	n = RL_CALLBACK(cbi); // result is in cbi struct, if wanted

	return n;
}

RXIEXT int RX_Call(int cmd, RXIFRM *frm, void *ctx) {
	REBYTE *str;

	//printf("Context ptr: %p\n", ctx);

	switch (cmd) {

	case 0: //command [{return zero}]
		RXA_INT64(frm, 1) = 0;
		RXA_TYPE(frm, 1) = RXT_INTEGER;
		break;

	case 1: //command [{return first arg} arg]
		break; // same as arg

	case 2: //command [{return second arg} arg1 arg2]
		RXA_INT64(frm, 1) = RXA_INT64(frm, 2);
		RXA_TYPE(frm, 1)  = RXA_TYPE(frm, 2);
		break;

	case 3: //command [{return system word from internal string}]
		RXA_WORD(frm, 1) = AS_WORD("system"); //?? is frame always long enough??
		RXA_TYPE(frm, 1) = RXT_WORD;
		break;

	case 4: //command [{return word from string} str [string!]]
		RL_GET_STRING(RXA_SERIES(frm, 1), 0, (void*)(&str), FALSE); // latin-1 only for test
		RXA_WORD(frm, 1) = RL_MAP_WORD(str);
		RXA_TYPE(frm, 1) = RXT_WORD;
		break;

	case 5: //command [{return obj field value} obj [object!] field [word! lit-word!]]
		RXA_TYPE(frm, 1) = RL_GET_FIELD(RXA_OBJECT(frm, 1), RXA_WORD(frm, 2), &RXA_ARG(frm, 1));
		break;

	case 6: //command [{test sync callback} context [object!] word [word!]]
		RXA_TYPE(frm, 1) = Test_Sync_Callback(RXA_OBJECT(frm, 1), RXA_WORD(frm, 2), &RXA_ARG(frm, 1));
		break;

	case 7: //command [{test async callback} context [object!] word [word!]]
		RXA_LOGIC(frm, 1) = Test_Async_Callback(RXA_OBJECT(frm, 1), RXA_WORD(frm, 2));
		RXA_TYPE(frm, 1) = RXT_LOGIC;
		break;

	case 8: //command [{return 2x3 image}]
		RXA_TYPE(frm, 1) = RXT_IMAGE;
		RXA_IMAGE(frm, 1) = RL_MAKE_IMAGE(2, 3);
		RXA_IMAGE_WIDTH(frm, 1) = 2;
		RXA_IMAGE_HEIGHT(frm, 1) = 3;
		break;

	case 9: //command [{test command context struct} blk [block!]]
		{
			REBCEC cec;
			cec.envr = 0;
			cec.block = RXA_SERIES(frm, 1);
			cec.index = 0;
			RL_DO_COMMANDS(RXA_SERIES(frm, 1), 0, &cec);
		}
		return RXR_UNSET;

	case 10: //command [{returns cec.index value or -1 if no cec}]
		{
			REBCEC* cec = (REBCEC*)ctx;
			RXA_INT64(frm, 1) = (i64)(cec ? cec->index : -1);
			RXA_TYPE(frm, 1) = RXT_INTEGER;
		}
		break;

	case 11: //command [{creates a handle}]"
		{
			RXA_HANDLE(frm, 1) = (void*)42;
			RXA_HANDLE_TYPE(frm, 1) = AS_WORD("xtest");
			RXA_TYPE(frm, 1) = RXT_HANDLE;
		}
		break;

	case 12: //command [{return handle's internal value as integer} hnd [handle!]]
		{
			i64 i = (i64)RXA_HANDLE(frm, 1);
			RXA_INT64(frm, 1) = i;
			RXA_TYPE(frm, 1) = RXT_INTEGER;
		}
		break;

	case 13: //command [{return vector size in bytes} v [vector!]]
		{
			REBSER *vec = RXA_SERIES(frm, 1);
			RXA_TYPE(frm, 1) = RXT_INTEGER;
			RXA_INT64(frm, 1) = vec->info * vec->tail;
		}
		break;
	case 14: //command [{return vector size in values (from object)} o [object!]]
		{
			RXIARG vec;
			REBCNT type = RL_GET_FIELD(RXA_OBJECT(frm, 1), AS_WORD("v"), &vec);
			if(type == RXT_VECTOR) {
				REBSER *vecs = (REBSER*)vec.series;
				u16* data = (u16*)vecs->data;
				printf("data[0-2]: %i, %i, %i\n", data[0], data[1], data[2]);
				//RXA_TYPE(frm, 1) = RXT_INTEGER;
				//RXA_INT64(frm, 1) = vecs->tail;
			} else {
				return RXR_FALSE;
			}
		}
		break;
	case 15: //command [{print type ids of all values in a block} b [block!]]
		{
			REBSER *blk = RXA_SERIES(frm, 1);
			REBCNT n, type;
			RXIARG val;
			printf("\nBlock with %llu values:\n", RL_SERIES(blk, RXI_SER_TAIL));
			for(n = 0; (type = RL_GET_VALUE(blk, n, &val)); n++) {
				if(type == RXT_END) break;
				printf("\t%i -> %i\n", n, type);
			}
			RL_MAP_WORDS(RXA_SERIES(frm, 1));
			return RXR_UNSET;
		}
		break;
	case 16: //command [{creates XTEST handle}]"
		{
			REBHOB *hob = RL_MAKE_HANDLE_CONTEXT(Handle_XTest);
			XTEST* data = (XTEST*)hob->data;
			printf("data=> id: %u num: %i\n", data->id, data->num);
			data->id = 1;
			data->num = -42;
			printf("data=> id: %u num: %i\n", data->id, data->num);

			RXA_HANDLE(frm, 1) = hob;
			RXA_HANDLE_TYPE(frm, 1) = hob->sym;
			RXA_HANDLE_FLAGS(frm, 1) = hob->flags;
			RXA_TYPE(frm, 1) = RXT_HANDLE;
		}
		break;
	case 17: //command [{prints XTEST handle's data} hndl [handle!]]"
		{
			REBHOB* hob = RXA_HANDLE(frm, 1);
			if (hob->sym == Handle_XTest) {
				XTEST* data = (XTEST*)hob->data;
				printf("data=> id: %u num: %i\n", data->id, data->num);
				RXA_INT64(frm, 1) = data->num;
				RXA_TYPE(frm, 1) = RXT_INTEGER;
			}
			else {
				puts("Wrong handle used!");
				return RXR_UNSET;
			}
		}
		break;
	case 18: //command [{return a constructed string}]"
		{
			REBSER* str = RL_MAKE_STRING(32, FALSE); // 32 bytes, latin1 (must be large enough!)
			REBYTE ver[8];
			RL_VERSION(ver);
			sprintf_s(SERIES_DATA(str), SERIES_REST(str), "Version: %i.%i.%i", ver[1], ver[2], ver[3]);
			SERIES_TAIL(str) = strlen(SERIES_DATA(str));
			RXA_SERIES(frm, 1) = str;
			RXA_TYPE  (frm, 1) = RXT_STRING;
			RXA_INDEX (frm, 1) = 0;
		}
		break;

	default:
		return RXR_NO_COMMAND;
	}
    return RXR_VALUE;
}


void* releaseXTestContext(void* ctx) {
	XTEST* data = (REBHOB*)ctx;
	printf("Relasing XTest context handle: %p\n", data);
	// do some final cleaning off the context's content
	printf("data=> id: %u num: %i\n", data->id, data->num);
	CLEARS(data);
	printf("data=> id: %u num: %i\n", data->id, data->num);
}

void Init_Ext_Test(void)
{
	RL = RL_Extend(b_cast(&RX_Spec[0]), (RXICAL)&RX_Call);
	Handle_XTest = RL_REGISTER_HANDLE("XTEST", sizeof(XTEST), releaseXTestContext);
}
#endif //TEST_EXTENSIONS