/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2018 Rebol Open Source Developers
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
**  Module:  u-bincode.c
**  Summary: binary data encoder/decoder
**  Section: utility
**  Author:  Oldes
**  Version: 0.1.0
**  History: 15-10-2018 Initial release with limited set of functionality
**
**  NOTE:    WORK in PROGRESS! Use with care!
**
***********************************************************************/
/*
	General idea of usage (does not have to be exact yet or may change):

	bin: binary 256 ;creates ctx of size 256 bytes
	;or
	bin: binary #{7B1948656C6C6F00} ;creates ctx and inits buffers from given binary
	;which is same like:
	bin: binary none
	bin: binary/write bin #{7B1948656C6C6F00}

	binary/read bin 'length?  ; <- number of available bytes in read-buffer
	;== 8 

	binary/read bin 'bytes ; <- read all remaining bytes
	;== #{7B1948656C6C6F00} 

	binary/init bin 1024  ; resets existing ctx and makes sure it has space for at least 1024 bytes
	binary/init bin #{FF} ; resets existing ctx and fills it with the given binary

	binary/read b 'ui8    ; to read just single byte as integer

	binary/clear bin      ; clear buffers, preallocated space is still available
	binary/free bin       ; to release internal resources

	;- using port?
	bin: open binary://
	write bin [UI16 6523 STRING "Hello"]
	;which would create internal binary: #{7B1948656C6C6F00}

	read bin ;would return #{7B1948656C6C6F00}
	;or
	read/as bin [UI16 STRING] ;would return [6523 "Hello"]
	;or
	read/as bin [size: UI16 name: STRING] ;could return [size: 6523 name: "Hello"]
	close bin
*/

#ifndef IGNORE_BINCODE

#include "sys-core.h"
#include <stdio.h>
#include <time.h>

#define ASSERT_SI_RANGE(v, n) if (VAL_INT64(v) < (- (i64)n) || VAL_INT64(v) > (i64)n) Trap1(RE_OUT_OF_RANGE, v);
#define ASSERT_UI_RANGE(v, n) if (VAL_INT32(v) > n) Trap1(RE_OUT_OF_RANGE, v);
#define ASSERT_UIBYTES_RANGE(v, n) if (VAL_LEN(v) > n) Trap1(RE_OUT_OF_RANGE, v);
#define ASSERT_U32_RANGE(v) if((i64)VAL_UNT64(v) > (i64)0xFFFFFFFFLL || (i64)VAL_UNT64(v) < (i64)0xFFFFFFFF00000001LL)  Trap1(RE_OUT_OF_RANGE, v);
#define ASSERT_READ_SIZE(v, p, e, n) if((p + n) > e) Trap1(RE_OUT_OF_RANGE, v);
#define ASSERT_INDEX_RANGE(v, p, e) if(i < 0 || i > VAL_TAIL(v)) Trap1(RE_OUT_OF_RANGE, e);

#define IS_BIT_SET(v, b) ((v & b) == b)
#define NEXT_IN_BIT(inBit) \
	do { \
		inBit = inBit >> 1; \
		if (inBit == 0) { \
			inBit = 0x80; \
			ASSERT_READ_SIZE(value, cp, ep, 1); \
			cp++; \
			VAL_INDEX(buffer_read)++; \
		} \
	} while (0)
#define STORE_IN_BIT(val, inBit)  SET_INT32(VAL_OBJ_VALUE(val, BINCODE_READ_BITMASK), inBit);

//**********************************************************************
//MUST be in order like the values in system/standard/bincode object bellow!!!
enum BincodeContextValues {
	BINCODE_TYPE = 1,
	BINCODE_READ,
	BINCODE_WRITE,
	BINCODE_READ_BITMASK,
	BINCODE_WRITE_BITMASK
};
/***********************************************************************
**  Base-code:

system/standard/bincode: make object! [
	type: 'bincode
	buffer:
	buffer-write: none
	r-mask:
	w-mask: 0
]

***********************************************************************/

static REBCNT EncodedU32_Size(u32 value) {
	REBCNT count = 0;
	if (value == 0) return 1;
	else while (value > 0) {
		value = value >> 7;
		count++;
	}
	return count;
}

/***********************************************************************
**
*/	REBNATIVE(binary)
/*
//	binary: native [
//		"Entry point of the binary DSL (Bincode)"
//
//		ctx [object! binary! integer! none!] "Bincode context. If none, it will create a new one."
//		/init    "Initializes buffers in the context"
//			spec [binary! integer! none!]
//		/write   "Write data into output buffer"
//			data [binary! block!] "Data dialect"
//		/read    "Read data from the input buffer"
//			code [word! block!]   "Input encoding"
//		/into    "Put READ results in out block, instead of creating a new block"
//			out  [block!] "Target block for results, when /into is used"
//	]
***********************************************************************/
{
    REBVAL *val_ctx   = D_ARG(1); 
	REBOOL  ref_init  = D_REF(2);
	REBVAL *val_spec  = D_ARG(3);
	REBOOL  ref_write = D_REF(4);
    REBVAL *val_write = D_ARG(5);
	REBOOL  ref_read  = D_REF(6);
	REBVAL *val_read  = D_ARG(7);
	REBOOL  ref_into  = D_REF(8);
	REBVAL *val_into  = D_ARG(9);

    REBVAL *ret = D_RET;
	//REBVAL *buf;
	REBSER *bin, *bin_new;
	REBSER *obj;
	REBVAL *buffer_write;
	REBVAL *buffer_read;
	REBCNT inBit, nbits;
	REBYTE *cp, *bp, *ep;
	REBCNT n, count, index, tail, tail_new;
	i32 i, len;
	u64 u;
	u32 ulong;

	REBVAL *value, *next;
	REBVAL *data;
	REBSER *blk = NULL; //used to store results of the read action
	REBVAL *temp;

	//printf("\nDS_TOP start: %x\n", DS_TOP);

	value = DS_TOP;

	//Oldes: note: change val_ctx from OBJECT to HANDLE or keep it as it is?
	//       the object has advantage of direct access to buffers (at least for reading)
	if(ref_init) {
		if(IS_OBJECT(val_ctx)) {
			//puts("existing context... so just reset it");
			
			buffer_write = VAL_OBJ_VALUE(val_ctx, BINCODE_WRITE);
			buffer_read  = VAL_OBJ_VALUE(val_ctx, BINCODE_READ);
			bin = VAL_SERIES(buffer_write);

			VAL_INDEX(buffer_write) = VAL_INDEX(buffer_read) = 0;

			switch(VAL_TYPE(val_spec)) {
				case REB_BINARY:
					n = VAL_TAIL(val_spec) - VAL_INDEX(val_spec);
					//Dump_Series(bin, "BIN1:");
					if (n > SERIES_REST(bin)) {

						Expand_Series(bin, AT_TAIL, n - SERIES_TAIL(bin) );
					}
					memcpy(BIN_DATA(bin), VAL_DATA(val_spec), n);
					VAL_TAIL(buffer_write) = VAL_TAIL(buffer_read) = n;
					//Dump_Series(bin, "BIN2:");
					break;
				case REB_INTEGER:
					//printf("resize from %i, needs %i\n", SERIES_REST(bin), VAL_INT32(val_spec));
					if(VAL_INT32(val_spec) > SERIES_REST(bin)){
						Expand_Series(bin, AT_TAIL, VAL_INT32(val_spec) - SERIES_TAIL(bin) - 1 );
					}
					//printf("resiz? new: %i\n", SERIES_REST(bin));
					VAL_TAIL(buffer_write) = VAL_TAIL(buffer_read) = 0;
					//Dump_Series(bin, "BIN2:");
					break;
				case REB_NONE:
					// just reset the index and tail pointers
					VAL_INDEX(buffer_write) = VAL_INDEX(buffer_read) = 0;
					VAL_TAIL(buffer_write) = VAL_TAIL(buffer_read) = 0;
					break;
			}
		}
	}

    if(IS_BINARY(val_ctx) || IS_NONE(val_ctx) || IS_INTEGER(val_ctx)) {
		REBVAL *ctx = Get_System(SYS_STANDARD, STD_BINCODE);
		if (!IS_OBJECT(ctx)) Trap_Arg(ctx);
		obj = CLONE_OBJECT(VAL_OBJ_FRAME(ctx));

		//IN and OUT buffers share the series, but may have different positions
		buffer_write = Get_Field(obj, BINCODE_WRITE);
		buffer_read  = Get_Field(obj, BINCODE_READ);

		if(IS_BINARY(val_ctx)) {
			bin = VAL_SERIES(val_ctx);
			if (ref_write && IS_PROTECT_SERIES(bin)) Trap0(RE_PROTECTED);
			SET_BINARY(buffer_write, bin);
			SET_BINARY(buffer_read, bin);
			VAL_INDEX(buffer_write) = VAL_INDEX(val_ctx);
			VAL_INDEX(buffer_read) = VAL_INDEX(val_ctx);
			//should be copied? locked? protected?
		} else {

			n = (IS_INTEGER(val_ctx)) ? VAL_INT32(val_ctx) : 1024;
			if(n <= 1) n = 16;
			bin = Make_Binary(n-1);			
			SET_BINARY(buffer_write, bin);
			SET_BINARY(buffer_read, bin);
			SET_OBJECT(val_ctx, obj);
			PROTECT_SERIES(bin);
			//LOCK_SERIES(bin);
			//printf("Making new bincode ctx of size: %i\n", n);
		}

		//printf("Making new bincode ctx of size: %i\n", bin->tail);
		
		SET_OBJECT(val_ctx, obj);

	} else {
		if (!IS_OBJECT(val_ctx)) Trap_Arg(val_ctx);

		//obj = VAL_OBJ_FRAME(val_ctx);
		//bin = VAL_SERIES(VAL_OBJ_VALUE(val_ctx, BINCODE_WRITE));

		//obj = VAL_OBJ_FRAME(val_ctx);
		buffer_write = VAL_OBJ_VALUE(val_ctx, BINCODE_WRITE);
		buffer_read  = VAL_OBJ_VALUE(val_ctx, BINCODE_READ);
		bin = VAL_SERIES(buffer_write);
	}

	//printf("bincode buffer rest: %i tail: %i index: %i\n", SERIES_REST(bin), bin->tail, VAL_INDEX(buffer_write));

	REBOOL as_block;

	if(ref_write) {
		//printf("In at: %i\n", VAL_INDEX(in));

		//puts("-----------write");
		//Dump_Series(bin, "BIN");

		DS_PUSH_NONE;
		temp = DS_TOP;

		index = VAL_INDEX(buffer_write);
		tail  = VAL_TAIL(buffer_write);

		switch(VAL_TYPE(val_write)) {
		case REB_BINARY:
			// optimized version for RAW binary input
			len = VAL_LEN(val_write);

			tail_new = index + len; //bytes needed to write
			//printf("index: %i tail: %i len: %i tail_new: %i rest: %i\n", index, tail, len, tail_new, SERIES_REST(bin));
			if(tail_new > SERIES_REST(bin)) {
				//printf("expanding by delta: %i\n", tail_new - SERIES_TAIL(bin));
				Expand_Series(bin, AT_TAIL, tail_new - SERIES_TAIL(bin));
				//Dump_Series(bin, "BIN");
			}
			if(tail_new > tail) {
				SERIES_TAIL(bin) = tail_new;
				VAL_TAIL(buffer_read) = tail_new;
			}
			memcpy(VAL_BIN_AT(buffer_write), VAL_BIN_AT(val_write), len);
			VAL_INDEX(buffer_write) += len;
			//Dump_Series(bin, "NewBin=>");
			DS_POP;
			return R_ARG1;
			break;

		case REB_BLOCK:
			// writing data specified in a WRITE dialect
			value = VAL_BLK(val_write);
			if (IS_END(value)) return R_ARG1; //empty block

			// try to predict number of bytes needed...
			count = VAL_INDEX(buffer_write);

			for (; NOT_END(value); value++) {
				switch (VAL_TYPE(value)) {
				case REB_GET_WORD:
					data = Get_Var(value);
					break;
				case REB_GET_PATH:
					data = Do_Path(&value, NULL);
					break;
				default:
					data = value;
				}
				switch (VAL_TYPE(data)) {
				case REB_BINARY:
					count += VAL_LEN(data);
					break;
				case REB_WORD:
					next = ++value;
					if (IS_GET_WORD(next)) {
						next = Get_Var(next);
					}

					switch (VAL_WORD_CANON(data)) {
					case SYM_SI8:
					case SYM_UI8:
						if (IS_INTEGER(next) || IS_CHAR(next)) {
							count += 1;
						}
						else goto error;
						break;
					case SYM_SI16:
					case SYM_UI16:
					case SYM_UI16LE:
					case SYM_UI16BE:
						if (IS_INTEGER(next) || IS_CHAR(next)) {
							count += 2;
						}
						else goto error;
						break;
					case SYM_SI24:
					case SYM_UI24:
					case SYM_UI24LE:
					case SYM_UI24BE:
						if (IS_INTEGER(next) || IS_CHAR(next)) {
							count += 3;
						}
						else goto error;
						break;
					case SYM_SI32:
					case SYM_UI32:
					case SYM_UI32LE:
					case SYM_UI32BE:
						if (IS_INTEGER(next) || IS_CHAR(next)) {
							count += 4;
						}
						else goto error;
						break;
					case SYM_SI64:
					case SYM_UI64:
					case SYM_UI64LE:
					case SYM_UI64BE:
						if (IS_INTEGER(next) || IS_CHAR(next)) {
							count += 8;
							continue;
						}
						goto error;
					case SYM_AT:
						if (IS_INTEGER(next)) {
							if (count > tail) tail = count;
							// AT is using ABSOLUTE positioning, so it cannot be < 1 (one-indexed)
							if(VAL_INT32(next) < 1) Trap1(RE_OUT_OF_RANGE, next);
							count = VAL_INT32(next) - 1;
							continue;
						}
						goto error;
					case SYM_BYTES:
						if (IS_GET_WORD(next)) next = Get_Var(next);
						if (IS_BINARY(next)) {
							count += VAL_LEN(next);
							continue;
						}
						goto error;
					case SYM_UI8BYTES:
						if (IS_BINARY(next)) {
							ASSERT_UIBYTES_RANGE(next, 0xFF);
							count += (1 + VAL_LEN(next));
						}
						else goto error;
						break;
					case SYM_UI16BYTES:
						if (IS_BINARY(next)) {
							ASSERT_UIBYTES_RANGE(next, 0xFFFF);
							count += (2 + VAL_LEN(next));
							continue;
						}
						goto error;
					case SYM_UI24BYTES:
						if (IS_BINARY(next)) {
							ASSERT_UIBYTES_RANGE(next, 0xFFFFFF);
							count += (3 + VAL_LEN(next));
							continue;
						}
						goto error;
					case SYM_UI32BYTES:
						if (IS_BINARY(next)) {
							ASSERT_UIBYTES_RANGE(next, 0xFFFFFFFF);
							count += (4 + VAL_LEN(next));
							continue;
						}
						goto error;

					case SYM_ENCODEDU32:
						if (IS_INTEGER(next)) {
							count += EncodedU32_Size(VAL_INT64(next));
							continue;
						}
						goto error;
						
					case SYM_UNIXTIME_NOW:
					case SYM_UNIXTIME_NOW_LE:
						value--; //there is no argument so no next
						count += 4;
						break;
					case SYM_RANDOM_BYTES:
						if (IS_INTEGER(next)) {
							count += VAL_INT32(next);
							continue;
						}
						goto error;
					default:
						value--;
						goto error;
					}
					break;
				case REB_SET_WORD:
					value++; //skip optional INDEX word...
					if (
						(VAL_TYPE(value) != REB_WORD) ||
						(VAL_WORD_CANON(value) != SYM_INDEX)
					) {
						value--; //... or revert it
					}

					VAL_SET(temp, REB_INTEGER);
					SET_INT32(temp, count + 1);
					Set_Var(value, temp);

					break;
				default:
					
					goto error;
				}
			} // end of for loop

			tail_new = count; //bytes needed to write
			//printf("index: %i tail: %i count: %i tail_new: %i rest: %i\n", index, tail, count, tail_new, SERIES_REST(bin));
			if (tail_new > SERIES_REST(bin)) {
				//puts("expanding");
				Expand_Series(bin, AT_TAIL, tail_new - SERIES_TAIL(bin));
			}

			if (tail_new > tail) tail = tail_new;
			SERIES_TAIL(bin) = tail;
			VAL_TAIL(buffer_read) = tail;
			

			value = VAL_BLK(val_write); //resets value at head of the input block

			//#########################################################
			//## Do real WRITE evaluation #############################
			//#########################################################
			
			cp = BIN_DATA(bin) + VAL_INDEX(buffer_write);

			for (; NOT_END(value); value++) {
				n = 0;
				switch(VAL_TYPE(value)) {
				case REB_GET_WORD:
					data = Get_Var(value);
					break;
				//case REB_GET_PATH:
				//	data = Do_Path(&value, NULL);
				//	break;
				default:
					data = value;
				}

				//printf("type: %i\n", VAL_TYPE(value), VAL_TYPE(data));

				switch (VAL_TYPE(data)) {
				case REB_BINARY:
					n = VAL_LEN(data);
					//printf("write bytes: %i to #%0X\n", n, cp);
					memcpy(cp, VAL_BIN_AT(data), n);
					break;

				case REB_WORD:
					next = ++value;
					if (IS_GET_WORD(next)) next = Get_Var(next);

					switch (VAL_WORD_CANON(data)) {
					case SYM_UI8:
						ASSERT_UI_RANGE(next, 0xFF);
					write_ui8:
						cp[0] = VAL_INT32(next) & 0xFF;
						n = 1;
						break;
					
					case SYM_UI16:
					case SYM_UI16BE:
						ASSERT_UI_RANGE(next, 0xFFFF);
					write_ui16:
						n = 2;
						i = VAL_INT32(next);
						bp = (REBYTE*)&i;
#ifdef ENDIAN_LITTLE
						cp[0] = bp[1]; cp[1] = bp[0];
#else
						memcpy(cp, bp, 2);
#endif
						break;
					case SYM_UI24:
					case SYM_UI24BE:
						ASSERT_UI_RANGE(next, 0xFFFFFF);
					write_ui24:
						n = 3;
						i = VAL_INT32(next);
						bp = (REBYTE*)&i;
#ifdef ENDIAN_LITTLE
						cp[0] = bp[2]; cp[1] = bp[1]; cp[2] = bp[0];
#else
						memcpy(cp, bp, 3);
#endif
						break;
					case SYM_UI32:
					case SYM_UI32BE:
						ASSERT_U32_RANGE(next);
					write_ui32:
						n = 4;
						bp = (REBYTE*)&VAL_INT64(next);
#ifdef ENDIAN_LITTLE
						cp[0] = bp[3]; cp[1] = bp[2]; cp[2] = bp[1]; cp[3] = bp[0];
#else
						memcpy(cp, bp, 4);
#endif
						break;
					case SYM_UI64:
					case SYM_UI64BE:
					write_ui64:
						n = 8;
						bp = (REBYTE*)&VAL_INT64(next);
#ifdef ENDIAN_LITTLE
						cp[0] = bp[7]; cp[1] = bp[6]; cp[2] = bp[5]; cp[3] = bp[4];
						cp[4] = bp[3]; cp[5] = bp[2]; cp[6] = bp[1]; cp[7] = bp[0];
#else
						memcpy(cp, bp, 8);
#endif
						break;
					case SYM_UI16LE:
						ASSERT_UI_RANGE(next, 0xFFFF);
						n = 2;
						bp = (REBYTE*)&VAL_INT64(next);
#ifdef ENDIAN_BIG
						cp[0] = bp[1]; cp[1] = bp[0];
#else
						memcpy(cp, bp, 2);
#endif
						break;
					case SYM_UI24LE:
						ASSERT_UI_RANGE(next, 0xFFFFFF);
						n = 3;
						bp = (REBYTE*)&VAL_INT64(next);
#ifdef ENDIAN_BIG
						cp[0] = bp[2]; cp[1] = bp[1]; cp[2] = bp[0];
#else
						memcpy(cp, bp, 3);
#endif
						break;
					case SYM_UI32LE:
						ASSERT_U32_RANGE(next);
						n = 4;
						bp = (REBYTE*)&VAL_INT64(next);
#ifdef ENDIAN_BIG
						cp[0] = bp[3]; cp[1] = bp[2]; cp[2] = bp[1]; cp[3] = bp[0];
#else
						memcpy(cp, bp, 4);
#endif
						break;
					case SYM_UI64LE:
						n = 8;
						bp = (REBYTE*)&VAL_INT64(next);
#ifdef ENDIAN_LITTLE
						memcpy(cp, bp, 8);
#else
						cp[0] = bp[7]; cp[1] = bp[6]; cp[2] = bp[5]; cp[3] = bp[4];
						cp[4] = bp[3]; cp[5] = bp[2]; cp[6] = bp[1]; cp[7] = bp[0];
#endif
						break;

					case SYM_SI8:
						ASSERT_SI_RANGE(next, 0x7F);
						goto write_ui8;
					case SYM_SI16:
						ASSERT_SI_RANGE(next, 0x7FFF);
						goto write_ui16;
					case SYM_SI24:
						ASSERT_SI_RANGE(next, 0x7FFFFF);
						goto write_ui24;
					case SYM_SI32:
						ASSERT_SI_RANGE(next, 0x7FFFFFFF);
						goto write_ui32;
					case SYM_SI64:
						goto write_ui64;

					case SYM_BYTES:
						n = VAL_LEN(next);
						memcpy(cp, VAL_BIN_AT(next), n);
						break;
					case SYM_UI8BYTES:
						n = cp[0] = VAL_LEN(next);
						memcpy(++cp, VAL_BIN_AT(next), n);
						VAL_INDEX(buffer_write)++; //for the length byte;
						break;
					case SYM_UI16BYTES:
						n = VAL_LEN(next);
						bp = (REBYTE*)&n;
#ifdef ENDIAN_LITTLE
						cp[0] = bp[1]; cp[1] = bp[0];
#else
						memcpy(cp, bp, 2);
#endif
						cp+=2;
						memcpy(cp, VAL_BIN_AT(next), n);
						VAL_INDEX(buffer_write)+=2; //for the length byte;
						break;
					case SYM_UI24BYTES:
						n = VAL_LEN(next);
						bp = (REBYTE*)&n;
#ifdef ENDIAN_LITTLE
						cp[0] = bp[2]; cp[1] = bp[1]; cp[2] = bp[0];
#else
						memcpy(cp, bp, 3);
#endif
						cp += 3;
						memcpy(cp, VAL_BIN_AT(next), n);
						VAL_INDEX(buffer_write) += 3; //for the length byte;
						break;
					case SYM_UI32BYTES:
						n = VAL_LEN(next);
						bp = (REBYTE*)&n;
#ifdef ENDIAN_LITTLE
						cp[0] = bp[3]; cp[1] = bp[2]; cp[2] = bp[1]; cp[3] = bp[0];
#else
						memcpy(cp, bp, 4);
#endif
						cp += 4;
						memcpy(cp, VAL_BIN_AT(next), n);
						VAL_INDEX(buffer_write) += 4; //for the length byte;
						break;
					case SYM_AT:
						VAL_INDEX(buffer_write) = VAL_INT32(next) - 1;
						cp = BIN_DATA(bin) + VAL_INDEX(buffer_write);
						n = 0;
						break;

					case SYM_ENCODEDU32:
						ASSERT_U32_RANGE(next);
						ulong = (u32)VAL_INT64(next);
						if (ulong == 0) {
							n = 1;
							cp[0] = 0;
						} else {
							n = EncodedU32_Size(VAL_INT64(next));
							for (u = 0; u < n-1; u++) {
								cp[u] = (char)(128 + ((ulong >> (u * 7)) & 127));
							}
							cp[n-1] = (char)((ulong >> ((n-1) * 7)) & 255);
						}
						break;

					case SYM_UNIXTIME_NOW:
						value--; // no args
						n = 4;
						i = (i32)time(NULL);
						bp = (REBYTE*)&i;
#ifdef ENDIAN_LITTLE
						cp[0] = bp[3]; cp[1] = bp[2]; cp[2] = bp[1]; cp[3] = bp[0];
#else
						memcpy(cp, bp, 4);
#endif
						break;
					case SYM_UNIXTIME_NOW_LE:
						value--; // no args
						n = 4;
						i = (i32)time(NULL);
						bp = (REBYTE*)&i;
#ifdef ENDIAN_LITTLE
						memcpy(cp, bp, 4);
#else
						cp[0] = bp[3]; cp[1] = bp[2]; cp[2] = bp[1]; cp[3] = bp[0];
#endif
						break;
					case SYM_RANDOM_BYTES:
						if (IS_INTEGER(next)) {
							n = (REBCNT)VAL_INT32(next);
							Random_Bytes(cp, n, TRUE);
						}
						break;
					default:
						Trap_Arg(value);
					}
					break;
				case REB_SET_WORD:
					VAL_SET(temp, REB_INTEGER);
					SET_INT32(temp, VAL_INDEX(buffer_write) + 1);
					Set_Var(data, temp);
					continue;
				}
				cp += n;
				VAL_INDEX(buffer_write) += n;
			}

			break; // end of REB_BLOCK write

		} //end of VAL_TYPE switch

		DS_DROP; // remove temp
	}

	if (ref_read) {	
		//printf("\nREADING... in-index: %i\n\n", VAL_INDEX(buffer_read));

		cp = BIN_DATA(bin) + VAL_INDEX(buffer_read);
		ep = BIN_DATA(bin) + VAL_TAIL(buffer_read);

		inBit = IS_OBJECT(val_ctx) ? VAL_INT32(VAL_OBJ_VALUE(val_ctx, BINCODE_READ_BITMASK)): 0;

		as_block = IS_BLOCK(val_read);

		if(as_block) {
			// encoding is block, so we can use its values normally
			value = VAL_BLK(val_read);
			if (IS_END(value)) return R_NONE; //empty block
											  
			// create a block for results if needed
			if(ref_into) {
				blk = VAL_SERIES(val_into);
				if (IS_PROTECT_SERIES(blk)) Trap0(RE_PROTECTED);
			} else {
				blk = Make_Block(VAL_LEN(val_read));
			}
			Set_Block(ret, blk);
		} else {
			// if encoding is just a word, simulate block with single value on stack
			DS_PUSH(val_read);
			value = DS_TOP;
			DS_PUSH_END; // marks end of the block
			if(ref_into) {
				blk = VAL_SERIES(val_into);
				Set_Block(ret, blk);
			} else {
				SET_NONE(ret);
			}
		}

		DS_PUSH_NONE;
		temp = DS_TOP;
		REBINT ssp = DSP;  // starting stack pointer
		
		for (; NOT_END(value); value++) {
			n = 0;
			switch (VAL_TYPE(value)) {
				case REB_WORD:
					switch (VAL_WORD_CANON(value)) {
						case SYM_UI8:
							n = 1;
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, cp[0]);
							break;
						case SYM_SI8:
							n = 1;
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, (i8)cp[0]);
							break;
						case SYM_UI16:
						case SYM_UI16BE:
							n = 2;
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, (u32)cp[1] + ((u32)cp[0] << 8));
							break;
						case SYM_SI16:
						case SYM_SI16BE:
							n = 2;
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, (i16)((i16)cp[1] + ((i16)cp[0] << 8)));
							break;
						case SYM_UI24:
						case SYM_UI24BE:
							n = 3;
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, ((u32)cp[2] <<  0) |
							                ((u32)cp[1] <<  8) |
							                ((u32)cp[0] << 16));
							break;
						case SYM_SI24:
						case SYM_SI24BE:
							n = 3;
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_INTEGER);
							i = ((i32)cp[2] <<  0) |
								((i32)cp[1] <<  8) |
								((i32)cp[0] << 16) ;
							if((cp[0] & 0x80) == 0x80) i |= 0xFF000000;
							SET_INT32(temp, i);
							break;
						case SYM_UI32:
						case SYM_UI32BE:
							n = 4;
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_INTEGER);
							VAL_UNT64(temp) = ((u64)cp[3]      ) |
							                  ((u64)cp[2] << 8 ) |
							                  ((u64)cp[1] << 16) |
							                  ((u64)cp[0] << 24) ;
							break;
						case SYM_UI32LE:
							n = 4;
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_INTEGER);
							VAL_UNT64(temp) = ((u64)cp[0] << 0 ) |
							                  ((u64)cp[1] << 8 ) |
							                  ((u64)cp[2] << 16) |
							                  ((u64)cp[3] << 24) ;
							break;
						case SYM_SI32:
						case SYM_SI32BE:
							n = 4;
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, ((i32)cp[3] << 0 ) |
							                ((i32)cp[2] << 8 ) |
							                ((i32)cp[1] << 16) |
							                ((i32)cp[0] << 24));
							break;
						case SYM_SI32LE:
							n = 4;
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, ((i32)cp[0] << 0 ) |
							                ((i32)cp[1] << 8 ) |
							                ((i32)cp[2] << 16) |
							                ((i32)cp[3] << 24));
							break;
						case SYM_UI64:
						case SYM_UI64BE:
							n = 8;
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_INTEGER);
							VAL_UNT64(temp) =
								((u64)cp[7] << 0 ) |
								((u64)cp[6] << 8 ) |
								((u64)cp[5] << 16) |
								((u64)cp[4] << 24) |
								((u64)cp[3] << 32) |
								((u64)cp[2] << 40) |
								((u64)cp[1] << 48) |
								((u64)cp[0] << 56) ;
							break;
						case SYM_UI64LE:
							n = 8;
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_INTEGER);
							VAL_UNT64(temp) =
								((u64)cp[0] << 0 ) |
								((u64)cp[1] << 8 ) |
								((u64)cp[2] << 16) |
								((u64)cp[3] << 24) |
								((u64)cp[4] << 32) |
								((u64)cp[5] << 40) |
								((u64)cp[6] << 48) |
								((u64)cp[7] << 56) ;
							break;
						case SYM_LENGTH:
							ASSERT_READ_SIZE(value, cp, ep, 1);
							VAL_SET(temp, REB_INTEGER);
							if (cp[0] <= 128) {
								SET_INT32(temp, cp[0]);
							}
							else {
								n = cp[0] & 127;
								ASSERT_READ_SIZE(value, cp, ep, n);
								len = 0;
								//printf("///%i\n", n);
								for (i = 1; i <= n; i++) {
									len = (len << 8) + cp[i];
								}
								SET_INT32(temp, len);
							}
							n++;
							break;
						case SYM_ENCODEDU32:
							ASSERT_READ_SIZE(value, cp, ep, 1);
							u = (u64)cp[0];
							if (!(u & 0x00000080)) {
								n = 1;
								goto setEnU32Result;
							}
							ASSERT_READ_SIZE(value, cp, ep, 2);
							u = (u & 0x0000007f) | cp[1] << 7;
							if (!(u & 0x00004000)) {
								n = 2;
								goto setEnU32Result;
							}
							ASSERT_READ_SIZE(value, cp, ep, 3);
							u = (u & 0x00003fff) | cp[2] << 14;
							if (!(u & 0x00200000)) {
								n = 3;
								goto setEnU32Result;
							}
							ASSERT_READ_SIZE(value, cp, ep, 4);
							u = (u & 0x001fffff) | cp[3] << 21;
							if (!(u & 0x10000000)) {
								n = 4;
								goto setEnU32Result;
							}
							ASSERT_READ_SIZE(value, cp, ep, 5);
							u = (u & 0x0fffffff) | cp[4] << 28;
							n = 5;
						setEnU32Result:
							VAL_SET(temp, REB_INTEGER);
							VAL_UNT64(temp) = u & 0xffffffff; // limits result to 32 bit unsigned integer!
							break;
						case SYM_UB:
							next = ++value;
							if (IS_GET_WORD(next)) next = Get_Var(next);
							if (!IS_INTEGER(next)) Trap1(RE_INVALID_SPEC, value);
							i = 0;
							if (inBit == 0) inBit = 0x80;
							// could be optimized?
							nbits = VAL_INT32(next);
							//printf("bits: %i %i\n", nbits, 1 << nbits);
							nbits = 1 << nbits;
							ASSERT_READ_SIZE(value, cp, ep, 1);
							while(nbits > 1) {
								nbits = nbits >> 1;
								if(IS_BIT_SET(cp[0], inBit)) i = i | nbits;
								//printf("?? %i %i\n", inBit, i);
								NEXT_IN_BIT(inBit);
								//printf("inBit: %i\n", inBit);
							}
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, i);
							STORE_IN_BIT(val_ctx, inBit);
							break;
						case SYM_SB:
							next = ++value;
							if (IS_GET_WORD(next)) next = Get_Var(next);
							if (!IS_INTEGER(next)) Trap1(RE_INVALID_SPEC, value);
							i = 0;
							if (inBit == 0) inBit = 0x80;
							// could be optimized?
							nbits = VAL_INT32(next);
							nbits = 1 << nbits;
							if (nbits > 0) {
								//printf("nbits: %i\n", nbits);
								ASSERT_READ_SIZE(value, cp, ep, 1);
								BOOL negative = IS_BIT_SET(cp[0], inBit);
								nbits = nbits >> 1;
								NEXT_IN_BIT(inBit);
								while (nbits > 1) {
									nbits = nbits >> 1;
									if (IS_BIT_SET(cp[0], inBit)) i = i | nbits;
									//printf("?? %i %i\n", inBit, i);
									NEXT_IN_BIT(inBit);
								}
								if(negative) i = -i;
							}
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, i);
							break;
						case SYM_BIT:
						case SYM_NOT_BIT:
							ASSERT_READ_SIZE(value, cp, ep, 1);
							//printf("inbit: %i cp: %i\n", inBit, cp[0]);
							if(inBit == 0) inBit = 0x80;
							VAL_SET(temp, REB_LOGIC);
							if(VAL_WORD_CANON(value) == SYM_BIT) {
								VAL_LOGIC(temp) = (cp[0] & inBit) == inBit ? TRUE : FALSE;
							} else {
								VAL_LOGIC(temp) = (cp[0] & inBit) == inBit ? FALSE : TRUE;
							}
							inBit = inBit >> 1;
							if(inBit == 0) n++;
							STORE_IN_BIT(val_ctx, inBit);
							break;
						case SYM_ALIGN:
							// aligns bit buffer to byte boundary
							if (inBit > 0) {
								inBit = 0;
								cp++;
								VAL_INDEX(buffer_read)++;
								STORE_IN_BIT(val_ctx, inBit);
							}
							continue;
						case SYM_UI16LE:
							n = 2;
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, cp[0] + (cp[1] << 8));
							break;
						case SYM_UI24LE:
							n = 3;
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, cp[0] + (cp[1] << 8) + (cp[2] << 16));
							break;
						case SYM_BYTES:
							//puts("bytes");
							next = ++value;
							if (IS_GET_WORD(next)) next = Get_Var(next);
							if (IS_END(next)) {
								// reads all bytes to tail of the buffer
								// used in case like: bincode/read b 'bytes
								n = VAL_TAIL(buffer_read) - VAL_INDEX(buffer_read);
								value--;
							} else {
								if (!IS_INTEGER(next)) Trap1(RE_INVALID_SPEC, value);
								n = VAL_INT32(next);
							}
							ASSERT_READ_SIZE(value, cp, ep, n);
						readNBytes:
							//printf("num: %i\n", n);
							VAL_SET(temp, REB_BINARY);
							bin_new = Copy_Series_Part(bin, VAL_INDEX(buffer_read), n);
							VAL_SERIES(temp) = bin_new;
							VAL_INDEX(temp) = 0;
							break;
						case SYM_STRING:
							bp = cp;
							for(;;) {
								if(bp[0] == 0) break;
								if(bp == ep) Trap1(RE_OUT_OF_RANGE, value);
								bp++;
							}
							n = bp - cp;
							VAL_SET(temp, REB_STRING);
							bin_new = Copy_Series_Part(bin, VAL_INDEX(buffer_read), n);
							SET_STR_END(bin_new, n);
							VAL_SERIES(temp) = bin_new;
							VAL_INDEX(temp) = 0;
							n++;
							break;
						case SYM_UI8BYTES:
							//puts("ui8bytes");
							ASSERT_READ_SIZE(value, cp, ep, 1);
							n = cp[0];
							ASSERT_READ_SIZE(value, cp, ep, n);
							cp++;
							VAL_INDEX(buffer_read)++;
							goto readNBytes;
						case SYM_UI16BYTES:
							//puts("ui16bytes");
							ASSERT_READ_SIZE(value, cp, ep, 2);
							n = (cp[0] << 8) + cp[1];
							ASSERT_READ_SIZE(value, cp, ep, n);
							cp += 2;
							VAL_INDEX(buffer_read) += 2;
							goto readNBytes;
						case SYM_UI16LEBYTES:
							ASSERT_READ_SIZE(value, cp, ep, 2);
							n = (cp[1] << 8) + cp[0];
							ASSERT_READ_SIZE(value, cp, ep, n);
							cp += 2;
							VAL_INDEX(buffer_read) += 2;
							goto readNBytes;
						case SYM_UI24BYTES:
							//puts("ui16bytes");
							ASSERT_READ_SIZE(value, cp, ep, 3);
							n = cp[2] + (cp[1] << 8) + (cp[0] << 16);
							ASSERT_READ_SIZE(value, cp, ep, n);
							cp += 3;
							VAL_INDEX(buffer_read) += 3;
							goto readNBytes;
						case SYM_UI24LEBYTES:
							//puts("ui16bytes");
							ASSERT_READ_SIZE(value, cp, ep, 3);
							n = cp[0] + (cp[1] << 8) + (cp[2] << 16);
							ASSERT_READ_SIZE(value, cp, ep, n);
							cp += 3;
							VAL_INDEX(buffer_read) += 3;
							goto readNBytes;
						case SYM_AT:
							// uses absolute positioning from series HEAD!
							next = ++value;
							if (IS_GET_WORD(next)) next = Get_Var(next);
							if (!IS_INTEGER(next)) Trap1(RE_INVALID_SPEC, value);
							i = VAL_INT32(next) - 1;
							ASSERT_INDEX_RANGE(buffer_read, i, value);
							VAL_INDEX(buffer_read) = i;
							cp = BIN_DATA(bin) + VAL_INDEX(buffer_read);
							continue;
						case SYM_INDEX:
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, VAL_INDEX(buffer_read) + 1);
							n = 0;
							break;
						case SYM_SKIP:
							next = ++value;
							if (IS_GET_WORD(next)) next = Get_Var(next);
							if (!IS_INTEGER(next)) Trap1(RE_INVALID_SPEC, value);
							i = VAL_INDEX(buffer_read) + VAL_INT32(next);
							ASSERT_INDEX_RANGE(buffer_read, i, value);
							VAL_INDEX(buffer_read) = i; //TODO: range test
							cp = BIN_DATA(bin) + VAL_INDEX(buffer_read);
							continue;
						case SYM_LENGTHQ:
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, VAL_TAIL(buffer_read) - VAL_INDEX(buffer_read));
							break;
						default:
							Trap1(RE_INVALID_SPEC, value);
					}
					break;
				case REB_SET_WORD:
					//puts("-- found set-word?");
					DS_PUSH(value);
					continue;
			}
			// Set prior set-words:
			while (DSP > ssp) {
				Set_Var(DS_TOP, temp);
				DS_DROP;
			}
			
			if (ref_into) {
				Insert_Series(blk, VAL_INDEX(val_into), (REBYTE*)temp, 1);
				VAL_INDEX(val_into)++;
			} else {
				if (as_block) {
					Append_Val(blk, temp);
				}
			}
			
			cp += n;
			VAL_INDEX(buffer_read) += n;
		}
		
		if (as_block) {
			DS_DROP; // temp
		} else {
			*ret = *temp;
			DS_DROP; // temp
			DS_DROP; // END of the virtual block
			DS_DROP; // value
		}

		if(ref_into) *ret = *val_into;

		return R_RET;
	}

	//printf("DS_TOP end: %x\n", DS_TOP);
    return R_ARG1;

error:
	Trap_Word(RE_DIALECT, SYM_BINCODE, value);
	return R_ARG1; //just to make Clang happy
}
#endif //IGNORE_BINCODE
