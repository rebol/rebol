/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2023 Rebol Open Source Developers
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

// FLOAT16 credits: Steven Pigeon
// https://hbfs.wordpress.com/2013/02/12/float16/
typedef union {
	struct {
		// type determines alignment!
		u16 m : 10;
		u16 e : 5;
		u16 s : 1;
	} bits;
	struct {
		u8 low;
		u8 high;
	} bytes;
} float16_s;

typedef union {
	float v;
	struct {
		u32 m : 23;
		u32 e : 8;
		u32 s : 1;
	} bits;
} float32_s;

float float16to32(float16_s f16) {
	// back to 32
	float32_s f32;
#ifndef USE_NO_INFINITY 
	if (f16.bits.e == 31) { // inifinity or nan
		return (f16.bits.m == 0) ? (f16.bits.s ? -1.0 : 1.0) * INFINITY : NAN;
	}
#endif
	f32.bits.s = f16.bits.s;
	f32.bits.e = (f16.bits.e - 15) + 127; // safe in this direction
	f32.bits.m = ((u32)f16.bits.m) << 13;
	return f32.v;
}

#pragma pack(4) 
typedef union {
	u16 num;
	struct {
		u32 day   : 5;
		u32 month : 4;
		u32 year  : 7;
	} date;
} ms_date;

typedef union {
	u16 num;
	struct {
		u32 second : 5;
		u32 minute : 6;
		u32 hour   : 5;
	} time;
} ms_time;

typedef union {
	u32 num;
	struct {
		u32 second : 5;
		u32 minute : 6;
		u32 hour   : 5;
		u32 day    : 5;
		u32 month  : 4;
		u32 year   : 7;
	} val;
} ms_datetime;
#pragma pack() 

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

#ifdef ENDIAN_LITTLE
#define WRITE_BE_2(cp, bp)  cp[0] = bp[1]; cp[1] = bp[0];
#define WRITE_BE_3(cp, bp)  cp[0] = bp[2]; cp[1] = bp[1]; cp[2] = bp[0];
#define WRITE_BE_4(cp, bp)  cp[0] = bp[3]; cp[1] = bp[2]; cp[2] = bp[1]; cp[3] = bp[0];
#define WRITE_BE_8(cp, bp)  cp[0] = bp[7]; cp[1] = bp[6]; cp[2] = bp[5]; cp[3] = bp[4]; \
                            cp[4] = bp[3]; cp[5] = bp[2]; cp[6] = bp[1]; cp[7] = bp[0];
#define WRITE_LE_2(cp, bp)  memcpy(cp, bp, 2);
#define WRITE_LE_3(cp, bp)  memcpy(cp, bp, 3);
#define WRITE_LE_4(cp, bp)  memcpy(cp, bp, 4);
#define WRITE_LE_8(cp, bp)  memcpy(cp, bp, 8);
#else
#define WRITE_BE_2(cp, bp)  memcpy(cp, bp, 2);
#define WRITE_BE_3(cp, bp)  memcpy(cp, bp, 3);
#define WRITE_BE_4(cp, bp)  memcpy(cp, bp, 4);
#define WRITE_BE_8(cp, bp)  memcpy(cp, bp, 8);
#define WRITE_LE_2(cp, bp)  cp[0] = bp[1]; cp[1] = bp[0];
#define WRITE_LE_3(cp, bp)  cp[0] = bp[2]; cp[1] = bp[1]; cp[2] = bp[0];
#define WRITE_LE_4(cp, bp)  cp[0] = bp[3]; cp[1] = bp[2]; cp[2] = bp[1]; cp[3] = bp[0];
#define WRITE_LE_8(cp, bp)  cp[0] = bp[7]; cp[1] = bp[6]; cp[2] = bp[5]; cp[3] = bp[4]; \
                            cp[4] = bp[3]; cp[5] = bp[2]; cp[6] = bp[1]; cp[7] = bp[0];

#endif

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
//			code [word! block! integer! binary!]   "Input encoding"
//		/into    "Put READ results in out block, instead of creating a new block"
//			out  [block!] "Target block for results, when /into is used"
//		/with    "Additional input argument"
//			num  [integer!] "Bits/bytes number used with WORD! code type to resolve just single value"
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
//	REBOOL  ref_with  = D_REF(10);
	REBVAL *val_num   = D_ARG(11);

    REBVAL *ret = D_RET;
	//REBVAL *buf;
	REBSER *bin, *bin_new;
	REBSER *obj;
	REBVAL *buffer_write;
	REBVAL *buffer_read;
	REBCNT inBit, nbits;
	REBYTE *cp, *bp, *ep;
	REBCNT n, count, index, tail, tail_new;
	REBDEC dbl;
	REBD32 d32;
	REBCNT cmd;
	REBLEN i, len;
	REBINT j;
	u64 u;
	i64 si;
	u32 ulong;
	u16 ushort;
	float16_s f16;
	float32_s f32;
	REBYTE buf[8];


	ms_datetime* msdt = NULL;
	ms_date*     msd  = NULL;
	ms_time*     mst  = NULL;

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
					if (VAL_INT64(val_spec) < 0 || VAL_UNT64(val_spec) >= MAX_U32)
						Trap1(RE_INVALID_ARG, val_spec);
					if (VAL_UNT32(val_spec) > SERIES_REST(bin)){
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
				//case REB_GET_PATH:
				//	data = Do_Path(&value, NULL);
				//	data = DS_POP; // volatile stack reference
				//	break;
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
					} else if (IS_GET_PATH(next)) {	
						Do_Path(&next, NULL);
						next = DS_POP; // volatile stack reference
					}
					if (IS_STRING(next) || IS_FILE(next) || IS_URL(next)) {
						DS_PUSH_NONE;
						Set_Binary(DS_TOP, Encode_UTF8_Value(next, VAL_LEN(next), 0));
						next = DS_POP;
					}
					cmd = VAL_WORD_CANON(data);
					switch (cmd) {
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
					case SYM_ATZ:
						if (IS_INTEGER(next)) {
							if (count > tail) tail = count;
							j = (cmd == SYM_AT ? 1 : 0);
							// AT is using ABSOLUTE positioning, so it cannot be < 1 (one-indexed) or < 0 (zero-based)
							if(VAL_INT32(next) < j) Trap1(RE_OUT_OF_RANGE, next);
							count = VAL_INT32(next) - j;
							continue;
						}
						goto error;
					case SYM_PAD:
						if (IS_INTEGER(next)) {
							if (VAL_INT32(next) < 0) Trap1(RE_INVALID_ARG, next);
							i = count % VAL_INT32(next);
							count += (i > 0) ? VAL_UNT32(next) - i : 0;
							continue;
						}
						goto error;
					case SYM_BYTES:
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
					case SYM_UI16LEBYTES:
					case SYM_UI16BEBYTES:
						if (IS_BINARY(next)) {
							ASSERT_UIBYTES_RANGE(next, 0xFFFF);
							count += (2 + VAL_LEN(next));
							continue;
						}
						goto error;
					case SYM_UI24BYTES:
					case SYM_UI24LEBYTES:
					case SYM_UI24BEBYTES:
						if (IS_BINARY(next)) {
							ASSERT_UIBYTES_RANGE(next, 0xFFFFFF);
							count += (3 + VAL_LEN(next));
							continue;
						}
						goto error;
					case SYM_UI32BYTES:
					case SYM_UI32LEBYTES:
					case SYM_UI32BEBYTES:
						if (IS_BINARY(next)) {
							ASSERT_UIBYTES_RANGE(next, 0xFFFFFFFF);
							count += (4 + VAL_LEN(next));
							continue;
						}
						goto error;

					case SYM_ENCODEDU32:
						if (IS_INTEGER(next)) {
							count += EncodedU32_Size(VAL_UNT32(next));
							continue;
						}
						goto error;

					case SYM_FLOAT:
					case SYM_F32:
					case SYM_F32LE:
					case SYM_F32BE:
						if (IS_INTEGER(next) || IS_DECIMAL(next)) {
							count += 4;
							continue;
						}
						goto error;
					case SYM_DOUBLE:
					case SYM_F64:
					case SYM_F64LE:
					case SYM_F64BE:
						if (IS_INTEGER(next) || IS_DECIMAL(next)) {
							count += 8;
							continue;
						}
						goto error;
					case SYM_FLOAT16:
					case SYM_F16:
					case SYM_F16LE:
					case SYM_F16BE:
						if (IS_INTEGER(next) || IS_DECIMAL(next)) {
							count += 2;
							continue;
						}
						goto error;

					case SYM_UNIXTIME_NOW:
					case SYM_UNIXTIME_NOW_LE:
						value--; //there is no argument so no next
						count += 4;
						break;

					case SYM_MSDOS_DATE:
						if (!IS_DATE(next)) goto error; // only date allowed
						//continue..
					case SYM_MSDOS_TIME:
						if (IS_DATE(next) || IS_TIME(next)) {
							count += 2;
							continue;
						}
						goto error;

					case SYM_MSDOS_DATETIME:
						if (IS_DATE(next) || IS_TIME(next)) {
							count += 4;
							continue;
						}
						goto error;

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
						(VAL_WORD_CANON(value) != SYM_INDEX && VAL_WORD_CANON(value) != SYM_INDEXZ)
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
				//	Do_Path(&value, NULL);
				//	value = DS_POP; // volatile stack reference
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
					else if (IS_GET_PATH(next)) {
						Do_Path(&next, NULL);
						next = DS_POP; // volatile stack reference
					}
					if (IS_STRING(next) || IS_FILE(next) || IS_URL(next)) {
						DS_PUSH_NONE;
						Set_Binary(DS_TOP, Encode_UTF8_Value(next, VAL_LEN(next), 0));
						next = DS_POP;
					}
					cmd = VAL_WORD_CANON(data);
					switch (cmd) {
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
						WRITE_BE_2(cp, bp);
						break;
					case SYM_UI24:
					case SYM_UI24BE:
						ASSERT_UI_RANGE(next, 0xFFFFFF);
					write_ui24:
						n = 3;
						i = VAL_INT32(next);
						bp = (REBYTE*)&i;
						WRITE_BE_3(cp, bp);
						break;
					case SYM_UI32:
					case SYM_UI32BE:
						ASSERT_U32_RANGE(next);
					write_ui32:
						n = 4;
						bp = (REBYTE*)&VAL_INT64(next);
						WRITE_BE_4(cp, bp);
						break;
					case SYM_UI64:
					case SYM_UI64BE:
					write_ui64:
						n = 8;
						bp = (REBYTE*)&VAL_INT64(next);
						WRITE_BE_8(cp, bp);
						break;
					case SYM_UI16LE:
						ASSERT_UI_RANGE(next, 0xFFFF);
						n = 2;
						bp = (REBYTE*)&VAL_INT64(next);
						WRITE_LE_2(cp, bp);
						break;
					case SYM_UI24LE:
						ASSERT_UI_RANGE(next, 0xFFFFFF);
						n = 3;
						bp = (REBYTE*)&VAL_INT64(next);
						WRITE_LE_3(cp, bp);
						break;
					case SYM_UI32LE:
						ASSERT_U32_RANGE(next);
						n = 4;
						bp = (REBYTE*)&VAL_INT64(next);
						WRITE_LE_4(cp, bp);
						break;
					case SYM_UI64LE:
						n = 8;
						bp = (REBYTE*)&VAL_INT64(next);
						WRITE_LE_8(cp, bp);
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
					case SYM_UI16BEBYTES:
						n = VAL_LEN(next);
						bp = (REBYTE*)&n;
						WRITE_BE_2(cp, bp);
						cp+=2;
						memcpy(cp, VAL_BIN_AT(next), n);
						VAL_INDEX(buffer_write)+=2; //for the length byte;
						break;
					case SYM_UI16LEBYTES:
						n = VAL_LEN(next);
						bp = (REBYTE*)&n;
						WRITE_LE_2(cp, bp);
						cp+=2;
						memcpy(cp, VAL_BIN_AT(next), n);
						VAL_INDEX(buffer_write)+=2; //for the length byte;
						break;

					case SYM_UI24BYTES:
					case SYM_UI24BEBYTES:
						n = VAL_LEN(next);
						bp = (REBYTE*)&n;
						WRITE_BE_3(cp, bp);
						cp += 3;
						memcpy(cp, VAL_BIN_AT(next), n);
						VAL_INDEX(buffer_write) += 3; //for the length byte;
						break;
					case SYM_UI24LEBYTES:
						n = VAL_LEN(next);
						bp = (REBYTE*)&n;
						WRITE_LE_3(cp, bp);
						cp += 3;
						memcpy(cp, VAL_BIN_AT(next), n);
						VAL_INDEX(buffer_write) += 3; //for the length byte;
						break;
					case SYM_UI32BYTES:
					case SYM_UI32BEBYTES:
						n = VAL_LEN(next);
						bp = (REBYTE*)&n;
						WRITE_BE_4(cp, bp);
						cp += 4;
						memcpy(cp, VAL_BIN_AT(next), n);
						VAL_INDEX(buffer_write) += 4; //for the length byte;
						break;
					case SYM_UI32LEBYTES:
						n = VAL_LEN(next);
						bp = (REBYTE*)&n;
						WRITE_LE_4(cp, bp);
						cp += 4;
						memcpy(cp, VAL_BIN_AT(next), n);
						VAL_INDEX(buffer_write) += 4; //for the length byte;
						break;

					case SYM_FLOAT:
					case SYM_F32:
					case SYM_F32LE:
						f32.v = (float)(IS_INTEGER(next) ? VAL_INT64(next) : VAL_DECIMAL(next));
						bp = (REBYTE *)&f32;
						WRITE_LE_4(cp, bp);
						n = 4;
						break;
					case SYM_F32BE:
						f32.v = (float)(IS_INTEGER(next) ? VAL_INT64(next) : VAL_DECIMAL(next));
						bp = (REBYTE *)&f32;
						WRITE_BE_4(cp, bp);
						n = 4;
						break;
					case SYM_DOUBLE:
					case SYM_F64:
					case SYM_F64LE:
						dbl = (REBDEC)(IS_INTEGER(next) ? VAL_INT64(next) : VAL_DECIMAL(next));
						n = 8;
						bp = (REBYTE *)&dbl;
						WRITE_LE_8(cp, bp);
						break;
					case SYM_F64BE:
						dbl = (REBDEC)(IS_INTEGER(next) ? VAL_INT64(next) : VAL_DECIMAL(next));
						n = 8;
						bp = (REBYTE *)&dbl;
						WRITE_BE_8(cp, bp);
						break;
					case SYM_FLOAT16:
					case SYM_F16:
					case SYM_F16BE:
					case SYM_F16LE:
						d32 = (REBDEC)(IS_INTEGER(next) ? VAL_INT64(next) : VAL_DECIMAL(next));
						if (isnan(d32)) { // 1.#NaN
							ushort = 0x7e00;
						} else {
							// based on: https://stackoverflow.com/a/15118210/494472
							ulong = *((u32*)&d32);
							u32 t1 = (ulong & 0x7fffffff) >> 13;   // Non-sign bits; Align mantissa on MSB
							u32 t2 = (ulong & 0x80000000) >> 16;   // Sign bit; Shift sign bit into position
							u32 t3 =  ulong & 0x7f800000;          // Exponent

							t1 -= 0x1c000;                         // Adjust bias
							t1 = (t3 < 0x38800000) ? 0 : t1;       // Flush-to-zero
							t1 = (t3 > 0x47000000) ? 0x7bff : t1;  // Clamp-to-max
							// NOTE: now infinity value is also clamped to max, is it ok?

							t1 |= t2;                              // Re-insert sign bit

							ushort = (u16)t1;
						}
						bp = (REBYTE *)&ushort;
						if (cmd == SYM_F16BE) {
							WRITE_BE_2(cp, bp);
						}
						else {
							WRITE_LE_2(cp, bp);
						}
						n = 2;
						break;

					case SYM_AT:
					case SYM_ATZ:
						VAL_INDEX(buffer_write) = VAL_INT32(next) - (cmd == SYM_AT ? 1 : 0);
						cp = BIN_DATA(bin) + VAL_INDEX(buffer_write);
						n = 0;
						break;

					case SYM_PAD:
						n = VAL_INDEX(buffer_write) % VAL_INT32(next);
						if (n > 0) {
							n = VAL_INT32(next) - n;
							memset(cp, 0, n);
						}
						break;

					case SYM_ENCODEDU32:
						ASSERT_U32_RANGE(next);
						ulong = VAL_UNT32(next);
						if (ulong == 0) {
							n = 1;
							cp[0] = 0;
						} else {
							n = EncodedU32_Size(VAL_UNT32(next));
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
						WRITE_BE_4(cp, bp);
						break;
					case SYM_UNIXTIME_NOW_LE:
						value--; // no args
						n = 4;
						i = (i32)time(NULL);
						bp = (REBYTE*)&i;
						WRITE_LE_4(cp, bp);
						break;

					case SYM_MSDOS_DATE:
						msd = (ms_date*)cp;
						msd->date.year = VAL_YEAR(next) - 1980;
						msd->date.month = VAL_MONTH(next);
						msd->date.day = VAL_DAY(next);
						n = 2;
						break;

					case SYM_MSDOS_TIME:
						if (VAL_TIME(next) == NO_TIME) {
							cp[0] = 0;
							cp[1] = 0;
						} else {
							mst = (ms_time*)cp;
							si = VAL_TIME(next) / SEC_SEC;
							mst->time.hour = (u32)(si / 3600);
							si -= 3600 * (i64)mst->time.hour;
							mst->time.minute = (u32)(si / 60);
							si -= 60 * (i64)mst->time.minute;
							mst->time.second = (u32)si / 2; // this format has only 2 sec resolution!
						}
						n = 2;
						break;

					case SYM_MSDOS_DATETIME:
						msdt = (ms_datetime*)cp;
						msdt->val.year = VAL_YEAR(next) - 1980;
						msdt->val.month = VAL_MONTH(next);
						msdt->val.day = VAL_DAY(next);
						if (VAL_TIME(next) == NO_TIME) {
							msdt->val.hour = 0;
							msdt->val.minute = 0;
							msdt->val.second = 0;
						}
						else {
							si = VAL_TIME(next) / SEC_SEC;
							msdt->val.hour = (u32)(si / 3600);
							si -= 3600 * (i64)msdt->val.hour;
							msdt->val.minute = (u32)(si / 60);
							si -= 60 * (i64)msdt->val.minute;
							msdt->val.second = (u32)si / 2; // this format has only 2 sec resolution!
						}
						n = 4;
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

		if (IS_INTEGER(val_read)) {
			n = (REBCNT)VAL_INT64(val_read);
			ASSERT_READ_SIZE(val_read, cp, ep, n);
			if(ref_into) {
				Trap0(RE_FEATURE_NA);
			}
			Set_Binary(D_RET, Copy_Series_Part(bin, VAL_INDEX(buffer_read), n));
			VAL_TAIL(D_RET) = n;
			VAL_INDEX(buffer_read) += n;
			return R_RET;
		}
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
			if (IS_INTEGER(val_num)) DS_PUSH(val_num);
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
					cmd = VAL_WORD_CANON(value);
					switch (cmd) {
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
						case SYM_SI16LE:
							n = 2;
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, (i16)((i16)cp[0] + ((i16)cp[1] << 8)));
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
						case SYM_BITSET8:
							n = 1;
							goto readBitsetN;
						case SYM_BITSET16:
							n = 2;
							goto readBitsetN;
						case SYM_BITSET32:
							n = 4;
						readBitsetN:
							ASSERT_READ_SIZE(value, cp, ep, n);
							VAL_SET(temp, REB_BITSET);
							bin_new = Copy_Series_Part(bin, VAL_INDEX(buffer_read), n);
							VAL_SERIES(temp) = bin_new;
							VAL_INDEX(temp) = 0;
							break;
						case SYM_ENCODEDU32:
							ASSERT_READ_SIZE(value, cp, ep, 1);
							u = (u64)cp[0];
							if (!(u & 0x00000080)) {
								n = 1;
								goto setEnU32Result;
							}
							ASSERT_READ_SIZE(value, cp, ep, 2);
							u = (u & 0x0000007f) | (u64)cp[1] << 7;
							if (!(u & 0x00004000)) {
								n = 2;
								goto setEnU32Result;
							}
							ASSERT_READ_SIZE(value, cp, ep, 3);
							u = (u & 0x00003fff) | (u64)cp[2] << 14;
							if (!(u & 0x00200000)) {
								n = 3;
								goto setEnU32Result;
							}
							ASSERT_READ_SIZE(value, cp, ep, 4);
							u = (u & 0x001fffff) | (u64)cp[3] << 21;
							if (!(u & 0x10000000)) {
								n = 4;
								goto setEnU32Result;
							}
							ASSERT_READ_SIZE(value, cp, ep, 5);
							u = (u & 0x0fffffff) | (u64)cp[4] << 28;
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
							// could be optimized?
							nbits = VAL_INT32(next);
							//printf("bits: %i %i\n", nbits, 1 << nbits);
							if (nbits > 0) {
								if (inBit == 0) inBit = 0x80;
								nbits = 1 << nbits;
								ASSERT_READ_SIZE(value, cp, ep, 1);
								while(nbits > 1) {
									nbits = nbits >> 1;
									if(IS_BIT_SET(cp[0], inBit)) i = i | nbits;
									//printf("?? %i %i\n", inBit, i);
									NEXT_IN_BIT(inBit);
									//printf("inBit: %i\n", inBit);
								}
								STORE_IN_BIT(val_ctx, inBit);
							}
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, i);
							break;
						case SYM_SB:
						case SYM_FB:
							next = ++value;
							if (IS_GET_WORD(next)) next = Get_Var(next);
							if (!IS_INTEGER(next)) Trap1(RE_INVALID_SPEC, value);
							u = 0;
							// could be optimized?
							nbits = VAL_INT32(next);
							if (nbits > 0) {
								if (inBit == 0) inBit = 0x80;
								// http://graphics.stanford.edu/~seander/bithacks.html#VariableSignExtend
								u64 m = (u64)1 << (nbits - 1); // sign bit mask
								nbits = 1 << nbits;
								//if (nbits > 0) {
									//printf("SB nbits: %i\n", nbits);
									while (nbits > 1) {
										nbits >>= 1;
										if (IS_BIT_SET(cp[0], inBit)) u = u | nbits;
										//printf("?? %i %i %u\n", nbits, inBit, u);
										NEXT_IN_BIT(inBit);
									}
									u = (u ^ m) - m;
								//}
								STORE_IN_BIT(val_ctx, inBit);
							}
							if (cmd == SYM_SB) {
								SET_INTEGER(temp, u);
							} else {
								SET_DECIMAL(temp, ((double)((i64)u) / 65536.0));
							}
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
							//if (inBit == 128) inBit = 0;
							if (inBit > 0 && inBit < 128) {
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
						case SYM_OCTAL_BYTES:
						case SYM_STRING_BYTES:
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
							// WARNING: this piece of code is also used for SYM_UI*BYTES commands! 
							if (cmd == SYM_OCTAL_BYTES) {
								// octal number
								u = 0;
								i = 0;
								while ((i < n) && cp[i]) {
									u = (u << 3) | (u64)(cp[i++] - '0');
								}
								SET_INTEGER(temp, u);
							} else {
								VAL_SET(temp, (cmd == SYM_STRING_BYTES) ? REB_STRING: REB_BINARY);
								bin_new = Copy_Series_Part(bin, VAL_INDEX(buffer_read), n);
								VAL_SERIES(temp) = bin_new;
								VAL_INDEX(temp) = 0;
								if (cmd == SYM_STRING_BYTES) {
									VAL_TAIL(temp) = (REBCNT)strnlen(cs_cast(VAL_BIN(temp)), n);
								}
							}
							break;
						case SYM_STRING:
							bp = cp;
							for(;;) {
								if(bp[0] == 0) break;
								if(bp == ep) Trap1(RE_OUT_OF_RANGE, value);
								bp++;
							}
							n = (REBCNT)(bp - cp);
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
						case SYM_UI32BYTES:
							ASSERT_READ_SIZE(value, cp, ep, 4);
							n = (REBCNT)(
                                ((u64)cp[3]      ) |
							    ((u64)cp[2] << 8 ) |
							    ((u64)cp[1] << 16) |
							    ((u64)cp[0] << 24));
							ASSERT_READ_SIZE(value, cp, ep, n);
							cp += 4;
							VAL_INDEX(buffer_read) += 4;
							goto readNBytes;
						case SYM_UI32LEBYTES:
							ASSERT_READ_SIZE(value, cp, ep, 4);
                            n = (REBCNT)(
                                ((u64)cp[0]      ) |
							    ((u64)cp[1] << 8 ) |
							    ((u64)cp[2] << 16) |
							    ((u64)cp[3] << 24));
							ASSERT_READ_SIZE(value, cp, ep, n);
							cp += 4;
							VAL_INDEX(buffer_read) += 4;
							goto readNBytes;
						case SYM_AT:
						case SYM_ATZ:
							// uses absolute positioning from series HEAD!
							next = ++value;
							if (IS_GET_WORD(next)) next = Get_Var(next);
							if (!IS_INTEGER(next)) Trap1(RE_INVALID_SPEC, value);
							i = VAL_INT32(next) - (cmd == SYM_AT ? 1 : 0);
							ASSERT_INDEX_RANGE(buffer_read, i, value);
							VAL_INDEX(buffer_read) = i;
							cp = BIN_DATA(bin) + VAL_INDEX(buffer_read);
							continue;
						case SYM_INDEX:
						case SYM_INDEXZ:
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, VAL_INDEX(buffer_read)) + (cmd == SYM_INDEX ? 1 : 0);
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
						case SYM_PAD:
							next = ++value;
							if (IS_GET_WORD(next)) next = Get_Var(next);
							if (!IS_INTEGER(next)) Trap1(RE_INVALID_SPEC, value);
							i = VAL_INDEX(buffer_read) % VAL_INT32(next);
							if (i > 0) {
								i = VAL_INT32(next) - i;
								ASSERT_INDEX_RANGE(buffer_read, i, value);
								VAL_INDEX(buffer_read) += i;
								cp = BIN_DATA(bin) + VAL_INDEX(buffer_read);
							}
							continue;
						case SYM_SKIPBITS:
							next = ++value;
							if (IS_GET_WORD(next)) next = Get_Var(next);
							if (!IS_INTEGER(next)) Trap1(RE_INVALID_SPEC, value);
							i = VAL_INT32(next);
							if (i >= 8) {
								i /= 8;
								//printf("byte skip: %d\n", i);
								ASSERT_READ_SIZE(value, cp, ep, i);
								cp += i;
								VAL_INDEX(buffer_read) += i;
								i = VAL_INT32(next) - (i * 8);
							}
							if (inBit == 0) inBit = 0x80;
							while (i > 0) {
								i--;
								//printf("inbit %d: %d %d\n",i, inBit, VAL_INDEX(buffer_read));
								NEXT_IN_BIT(inBit);
							}
							continue;
						case SYM_LENGTHQ:
							VAL_SET(temp, REB_INTEGER);
							SET_INT32(temp, VAL_TAIL(buffer_read) - VAL_INDEX(buffer_read));
							break;
						case SYM_TUPLE3:
							n = 3;
							goto readNTuple;
						case SYM_TUPLE4:
							n = 4;
						readNTuple:
							ASSERT_READ_SIZE(value, cp, ep, n);
							Set_Tuple(temp, BIN_DATA(bin) + VAL_INDEX(buffer_read), n);
							break;
						case SYM_FLOAT16:
						case SYM_F16:
						case SYM_F16LE:
							n = 2;
							ASSERT_READ_SIZE(value, cp, ep, n);
							f16.bytes.low  = cp[0];
							f16.bytes.high = cp[1];
							SET_DECIMAL(temp, float16to32(f16) );
							break;
						case SYM_F16BE:
							n = 2;
							ASSERT_READ_SIZE(value, cp, ep, n);
							f16.bytes.low = cp[1];
							f16.bytes.high = cp[0];
							SET_DECIMAL(temp, float16to32(f16));
							break;
						case SYM_FLOAT:
						case SYM_F32:
						case SYM_F32LE:
							n = 4;
							ASSERT_READ_SIZE(value, cp, ep, n);
							SET_DECIMAL(temp, ((float*)cp)[0]);
							break;
						case SYM_F32BE:
							n = 4;
							ASSERT_READ_SIZE(value, cp, ep, n);
							buf[0] = cp[3];
							buf[1] = cp[2];
							buf[2] = cp[1];
							buf[3] = cp[0];
							SET_DECIMAL(temp, ((float*)buf)[0]);
							break;
						case SYM_DOUBLE:
						case SYM_F64:
						case SYM_F64LE:
							n = 8;
							ASSERT_READ_SIZE(value, cp, ep, n);
							SET_DECIMAL(temp, ((double*)cp)[0]);
							break;
						case SYM_F64BE:
							n = 8;
							ASSERT_READ_SIZE(value, cp, ep, n);
							buf[0] = cp[7];
							buf[1] = cp[6];
							buf[2] = cp[5];
							buf[3] = cp[4];
							buf[4] = cp[3];
							buf[5] = cp[2];
							buf[6] = cp[1];
							buf[7] = cp[0];
							SET_DECIMAL(temp, ((double *)buf)[0]);
							break;
						case SYM_FIXED8:
							n = 2;
							ASSERT_READ_SIZE(value, cp, ep, n);
							i = ((i32)cp[0] << 0)  |
								((i32)cp[1] << 8)  ;
							SET_DECIMAL(temp, (float)i / 256.0f);
							break;
						case SYM_FIXED16:
							n = 4;
							ASSERT_READ_SIZE(value, cp, ep, n);
							i = ((i32)cp[0] << 0)  |
								((i32)cp[1] << 8)  |
								((i32)cp[2] << 16) |
								((i32)cp[3] << 24) ;
							VAL_SET(temp, REB_DECIMAL);
							VAL_DECIMAL(temp) = ((float)i / 65536.0f);
							break;
						case SYM_MSDOS_DATETIME:
							n = 4;
							ASSERT_READ_SIZE(value, cp, ep, n);
							msdt = (ms_datetime*)cp;
							VAL_SET  (temp, REB_DATE);
							VAL_YEAR (temp) = msdt->val.year + 1980;
							VAL_MONTH(temp) = msdt->val.month;
							VAL_DAY  (temp) = msdt->val.day;
							VAL_ZONE (temp) = 0;
							VAL_TIME (temp) = TIME_SEC(msdt->val.second * 2
							                         + msdt->val.minute * 60
							                         + msdt->val.hour * 3600);
							break;
						case SYM_MSDOS_DATE:
							n = 2;
							ASSERT_READ_SIZE(value, cp, ep, n);
							msd = (ms_date*)cp;
							CLEARS(temp);
							VAL_SET  (temp, REB_DATE);
							VAL_YEAR (temp) = msd->date.year + 1980;
							VAL_MONTH(temp) = msd->date.month;
							VAL_DAY  (temp) = msd->date.day;
							VAL_TIME (temp) = NO_TIME;
							break;
						case SYM_MSDOS_TIME:
							n = 2;
							ASSERT_READ_SIZE(value, cp, ep, n);
							mst = (ms_time*)cp;
							CLEARS(temp);
							VAL_SET  (temp, REB_TIME);
							VAL_TIME (temp) = TIME_SEC(mst->time.second * 2 // this format has only 2 sec resolution!
							                         + mst->time.minute * 60
							                         + mst->time.hour * 3600);
							break;
						case SYM_CROP:
							n = 0;
							if (IS_PROTECT_SERIES(bin)) Trap0(RE_PROTECTED);
							Remove_Series(VAL_SERIES(buffer_read), 0, VAL_INDEX(buffer_read));
							cp = VAL_BIN_HEAD(buffer_read);
							VAL_INDEX(buffer_write) = MAX(0, (REBI64)VAL_INDEX(buffer_write) - VAL_INDEX(buffer_read));
							VAL_INDEX(buffer_read) = 0;
							continue;
						default:
							Trap1(RE_INVALID_SPEC, value);
					}
					break;
				case REB_SET_WORD:
					//puts("-- found set-word?");
					DS_PUSH(value);
					continue;
				case REB_BINARY:
					// do binary comparison, return TRUE and skip if data matches
					// or return FALSE and keep position where it is
					VAL_SET(temp, REB_LOGIC);
					n = Val_Series_Len(value);
					if ((cp + n) > ep || NZ(memcmp(cp, VAL_BIN_DATA(value), n))) {
						VAL_LOGIC(temp) = FALSE;
						n = 0; // no advance
					}
					else {
						VAL_LOGIC(temp) = TRUE;
					}
					break;
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
			if (IS_INTEGER(val_num)) DS_DROP;
			//@@ could above be done better?
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
