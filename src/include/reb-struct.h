/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2014 Atronix Engineering, Inc.
**  Copyright 2021 Rebol Open Source Contributors
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
**  Summary: Struct to C function
**  Module:  reb-struct.h
**  Author:  Shixin Zeng, Oldes
**
***********************************************************************/

enum {
	STRUCT_TYPE_UINT8 = 0,
	STRUCT_TYPE_INT8,
	STRUCT_TYPE_UINT16,
	STRUCT_TYPE_INT16,
	STRUCT_TYPE_UINT32,
	STRUCT_TYPE_INT32,
	STRUCT_TYPE_UINT64,
	STRUCT_TYPE_INT64,
	STRUCT_TYPE_INTEGER,

	STRUCT_TYPE_FLOAT,
	STRUCT_TYPE_DOUBLE,
	STRUCT_TYPE_DECIMAL,

	STRUCT_TYPE_POINTER,
	STRUCT_TYPE_STRUCT,
	STRUCT_TYPE_WORD,
	STRUCT_TYPE_REBVAL,
	STRUCT_TYPE_MAX
};

struct Struct_Field {
	REBSER* spec; /* for nested struct */
	REBSER* fields; /* for nested struct */
	REBCNT sym;

	REBINT type; /* rebol type */

	/* size is limited by struct->offset, so only 16-bit */
	REBCNT offset;
	REBCNT dimension; /* for arrays */
	REBCNT size; /* size of element, in bytes */
	REBOOL array:1;
	REBOOL done:1; /* field is initialized?, used by GC to decide if the value needs to be marked */
};

/* this is hackish to work around the size limit of REBSTU
 *	VAL_STRUCT_DATA(val) is not the actual data, but a series with 
 *	one Struct_Data element, and this element has various infomation
 *	about the struct data
 * */
struct Struct_Data {
	REBSER *data;
	REBCNT offset;
	REBCNT len;
	REBFLG flags;
};

#define STRUCT_DATA_BIN(v) (((struct Struct_Data*)SERIES_DATA((v)->data))->data)
#define STRUCT_OFFSET(v) (((struct Struct_Data*)SERIES_DATA((v)->data))->offset)
#define STRUCT_LEN(v) (((struct Struct_Data*)SERIES_DATA((v)->data))->len)
#define STRUCT_FLAGS(v) (((struct Struct_Data*)SERIES_DATA((v)->data))->flags)

#define VAL_STRUCT_DATA_BIN(v) (((struct Struct_Data*)SERIES_DATA(VAL_STRUCT_DATA(v)))->data)
#define VAL_STRUCT_OFFSET(v) (((struct Struct_Data*)SERIES_DATA(VAL_STRUCT_DATA(v)))->offset)
#define VAL_STRUCT_LEN(v) (((struct Struct_Data*)SERIES_DATA(VAL_STRUCT_DATA(v)))->len)
#define VAL_STRUCT_FLAGS(v) (((struct Struct_Data*)SERIES_DATA(VAL_STRUCT_DATA(v)))->flags)
#define VAL_STRUCT_LIMIT	MAX_U32
