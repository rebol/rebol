/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2014 Atronix Engineering, Inc.
**  Copyright 2021-2025 Rebol Open Source Contributors
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

// These depend on the order of the types above!
#define IS_STRUCT_INTEGER_TYPE(t) ((t) < STRUCT_TYPE_INTEGER)
#define IS_STRUCT_DECIMAL_TYPE(t) ((t) > STRUCT_TYPE_INTEGER && (t) < STRUCT_TYPE_DECIMAL)
#define IS_STRUCT_NUMERIC_TYPE(t) (IS_STRUCT_INTEGER_TYPE(t) || IS_STRUCT_DECIMAL_TYPE(t))

#define VAL_STRUCT_LIMIT	MAX_U32
