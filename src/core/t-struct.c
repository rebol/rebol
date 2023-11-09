/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2014 Atronix Engineering, Inc.
**  Copyright 2021-2023 Rebol Open Source Developers
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
**  Module:  t-strut.c
**  Summary: C struct object datatype
**  Section: datatypes
**  Author:  Shixin Zeng, Oldes
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

#define IS_INTEGER_TYPE(t) ((t) < STRUCT_TYPE_INTEGER)
#define IS_DECIMAL_TYPE(t) ((t) > STRUCT_TYPE_INTEGER && (t) < STRUCT_TYPE_DECIMAL)
#define IS_NUMERIC_TYPE(t) (IS_INTEGER_TYPE(t) || IS_DECIMAL_TYPE(t))

REBFLG MT_Struct(REBVAL *out, REBVAL *data, REBCNT type);

static const REBINT type_to_sym [STRUCT_TYPE_MAX] = {
	SYM_UINT8X,
	SYM_INT8X,
	SYM_UINT16X,
	SYM_INT16X,
	SYM_UINT32X,
	SYM_INT32X,
	SYM_UINT64X,
	SYM_INT64X,
	-1, //SYM_INTEGER,

	SYM_FLOATX,
	SYM_DOUBLEX,
	-1, //SYM_DECIMAL,

	SYM_POINTER,
	-1, //SYM_STRUCT
	SYM_WORD_TYPE,
	//SYM_REBVAL // unused
	//STRUCT_TYPE_MAX
};

static REBFLG get_scalar(REBSTU *stu,
				  struct Struct_Field *field,
				  REBCNT n, /* element index, starting from 0 */
				  REBVAL *val)
{
	REBYTE *data = SERIES_SKIP(STRUCT_DATA_BIN(stu),
							 STRUCT_OFFSET(stu) + field->offset + n * field->size);
	switch (field->type) {
		case STRUCT_TYPE_UINT8:
			SET_INTEGER(val, *(u8*)data);
			break;
		case STRUCT_TYPE_INT8:
			SET_INTEGER(val, *(i8*)data);
			break;
		case STRUCT_TYPE_UINT16:
			SET_INTEGER(val, *(u16*)data);
			break;
		case STRUCT_TYPE_INT16:
			SET_INTEGER(val, *(i8*)data);
			break;
		case STRUCT_TYPE_UINT32:
			SET_INTEGER(val, *(u32*)data);
			break;
		case STRUCT_TYPE_INT32:
			SET_INTEGER(val, *(i32*)data);
			break;
		case STRUCT_TYPE_UINT64:
			SET_INTEGER(val, *(u64*)data);
			break;
		case STRUCT_TYPE_INT64:
			SET_INTEGER(val, *(i64*)data);
			break;
		case STRUCT_TYPE_FLOAT:
			SET_DECIMAL(val, *(float*)data);
			break;
		case STRUCT_TYPE_DOUBLE:
			SET_DECIMAL(val, *(double*)data);
			break;
		case STRUCT_TYPE_POINTER:
			SET_INTEGER(val, (u64)*(void**)data);
			break;
		case STRUCT_TYPE_STRUCT:
			{
				SET_TYPE(val, REB_STRUCT);
				VAL_STRUCT_FIELDS(val) = field->fields;
				VAL_STRUCT_SPEC(val) = field->spec;
				VAL_STRUCT_DATA(val) = Make_Series(1, sizeof(struct Struct_Data), FALSE);
				VAL_STRUCT_DATA_BIN(val) = STRUCT_DATA_BIN(stu);
				VAL_STRUCT_OFFSET(val) = data - SERIES_DATA(VAL_STRUCT_DATA_BIN(val));
				VAL_STRUCT_LEN(val) = field->size;
			}
			break;
		case STRUCT_TYPE_WORD:
			Set_Word(val, *(REBINT *)data, NULL, 0);
			break;
#ifdef unused
		case STRUCT_TYPE_REBVAL:
			memcpy(val, data, sizeof(REBVAL));
			break;
#endif
		default:
			/* should never be here */
			return FALSE;
	}
	return TRUE;
}

/***********************************************************************
**
*/	static REBFLG Get_Struct_Var(REBSTU *stu, REBVAL *word, REBVAL *val)
/*
***********************************************************************/
{
	struct Struct_Field *field = NULL;
	REBCNT i = 0;
	field = (struct Struct_Field *)SERIES_DATA(stu->fields);
	for (i = 0; i < SERIES_TAIL(stu->fields); i ++, field ++) {
		if (VAL_WORD_CANON(word) == VAL_SYM_CANON(BLK_SKIP(PG_Word_Table.series, field->sym))) {
			if (field->array) {
				REBSER *ser = Make_Block(field->dimension);
				REBCNT n = 0;
				SET_TYPE(val, REB_BLOCK);
				for (n = 0; n < field->dimension; n ++) {
					REBVAL elem;
					get_scalar(stu, field, n, &elem);
					Append_Val(ser, &elem);
				}
				VAL_SERIES(val) = ser;
				VAL_INDEX(val) = 0;
			} else {
				get_scalar(stu, field, 0, val);
			}
			return TRUE;
		}
	}
	return FALSE;
}

#ifdef unused
/***********************************************************************
**
*/	static void Set_Struct_Vars(REBSTU *strut, REBVAL *blk)
/*
***********************************************************************/
{
}
#endif

/***********************************************************************
**
*/	REBSER *Struct_To_Block(REBSTU *stu)
/*
**		Used by MOLD to create a block.
**
***********************************************************************/
{
	REBSER *ser = Make_Block(10);
	struct Struct_Field *field = (struct Struct_Field*) SERIES_DATA(stu->fields);
	REBCNT i;

	for(i = 0; i < SERIES_TAIL(stu->fields); i ++, field ++) {
		REBVAL *val = NULL;
		REBVAL *type_blk = NULL;

		/* required field name */
		val = Append_Value(ser);
		Init_Word(val, field->sym);
		SET_TYPE(val, REB_SET_WORD);
		VAL_SET_LINE(val);

		/* required type */
		type_blk = Append_Value(ser);
		VAL_SERIES(type_blk) = Make_Block(1);
		SET_TYPE(type_blk, REB_BLOCK);
		val = Append_Value(VAL_SERIES(type_blk));
		if (field->type == STRUCT_TYPE_STRUCT) {
			REBVAL *nested = NULL;
			DS_PUSH_NONE;
			nested = DS_TOP;

			Init_Word(val, SYM_STRUCT_TYPE);
			get_scalar(stu, field, 0, nested);
			val = Append_Value(VAL_SERIES(type_blk));
			SET_TYPE(val, REB_BLOCK);
			VAL_SERIES(val) = Struct_To_Block(&VAL_STRUCT(nested));

			DS_POP;
		} else {
			REBINT sym = type_to_sym[field->type];
			Init_Word(val, sym);
		}

		/* optional dimension */
		if (field->dimension > 1) {
			REBSER *dim = Make_Block(1);
			REBVAL *dv = NULL;
			val = Append_Value(VAL_SERIES(type_blk));
			SET_TYPE(val, REB_BLOCK);
			VAL_SERIES(val) = dim;

			dv = Append_Value(dim);
			SET_INTEGER(dv, field->dimension);
		}

		/* optional initialization */
		val = Append_Value(ser);
		if (field->dimension > 1) {
			REBSER *dim = Make_Block(1);
			REBCNT n = 0;
			SET_TYPE(val, REB_BLOCK);
			VAL_SERIES(val) = dim;
			for (n = 0; n < field->dimension; n ++) {
				REBVAL *dv = Append_Value(dim);
				get_scalar(stu, field, n, dv);
			}
		} else {
			get_scalar(stu, field, 0, val);
#ifdef ALLOW_CODE_EVALUATION_INSIDE_STRUCT_CONSTRUCTION_SPEC 
			if (IS_WORD(val)) SET_TYPE(val, REB_LIT_WORD);
#endif
		}
	}
	return ser;
}

static void Get_Struct_Reflect(REBVAL* ret, REBSTU* stu, REBCNT type) {
	REBVAL* val = NULL;
	REBVAL* type_blk = NULL;
	REBSER* out, * dim;
	struct Struct_Field* field = (struct Struct_Field*)SERIES_DATA(stu->fields);
	REBCNT i, n, cnt;

	cnt = SERIES_TAIL(stu->fields);
	out = Make_Block(cnt * (type==SYM_BODY?2:1));
	Set_Block(ret, out);

	for (i = 0; i < cnt; i++, field++) {
		if (type != SYM_VALUES) {
			val = Append_Value(out);
			Init_Word(val, field->sym);
			if (type == SYM_WORDS) {
				SET_TYPE(val, REB_WORD);
			} else {
				SET_TYPE(val, REB_SET_WORD);
				VAL_SET_LINE(val);
			}
		}
		if (type != SYM_WORDS) {
			val = Append_Value(out);
			if (field->dimension > 1) {
				dim = Make_Block(field->dimension);
				SET_TYPE(val, REB_BLOCK);
				VAL_SERIES(val) = dim;
				for (n = 0; n < field->dimension; n++) {
					REBVAL* dv = Append_Value(dim);
					get_scalar(stu, field, n, dv);
				}
			}
			else {
				get_scalar(stu, field, 0, val);
			}
		}
	}
}

static REBOOL same_fields(REBSER *tgt, REBSER *src)
{
	struct Struct_Field *tgt_fields = (struct Struct_Field *) SERIES_DATA(tgt);
	struct Struct_Field *src_fields = (struct Struct_Field *) SERIES_DATA(src);
	REBCNT n;

	if (SERIES_TAIL(tgt) != SERIES_TAIL(src)) {
		return FALSE;
	}

	for(n = 0; n < SERIES_TAIL(src); n ++) {
		if (tgt_fields[n].type != src_fields[n].type) {
			return FALSE;
		}
		if (VAL_SYM_CANON(BLK_SKIP(PG_Word_Table.series, tgt_fields[n].sym))
			!= VAL_SYM_CANON(BLK_SKIP(PG_Word_Table.series, src_fields[n].sym))
			|| tgt_fields[n].offset != src_fields[n].offset
			|| tgt_fields[n].dimension != src_fields[n].dimension
			|| tgt_fields[n].size != src_fields[n].size) {
			return FALSE;
		}
		if (tgt_fields[n].type == STRUCT_TYPE_STRUCT
			&& ! same_fields(tgt_fields[n].fields, src_fields[n].fields)) {
			return FALSE;
		}
	}

	return TRUE;
}

static REBOOL assign_scalar(REBSTU *stu,
							struct Struct_Field *field,
							REBCNT n, /* element index, starting from 0 */
							REBVAL *val)
{
	u64 i = 0;
	double d = 0;
	void *data = SERIES_SKIP(STRUCT_DATA_BIN(stu),
							 STRUCT_OFFSET(stu) + field->offset + n * field->size);

	if (field->type == STRUCT_TYPE_REBVAL) {
		memcpy(data, val, sizeof(REBVAL));
		return TRUE;
	}

	switch (VAL_TYPE(val)) {
		case REB_DECIMAL:
			if (!IS_NUMERIC_TYPE(field->type)) {
				Trap_Type(val);
			}
			d = VAL_DECIMAL(val);
			i = (u64) d;
			break;
		case REB_INTEGER:
			if (!IS_NUMERIC_TYPE(field->type)
				&& field->type != STRUCT_TYPE_POINTER) {
				Trap_Type(val);
			}
			i = (u64) VAL_INT64(val);
			d = (double)i;
			break;
		case REB_STRUCT:
			if (STRUCT_TYPE_STRUCT != field->type) {
				Trap_Type(val);
			}
			break;
		case REB_WORD:
			if (STRUCT_TYPE_WORD != field->type) {
				Trap_Type(val);
			}
			i = (u64)VAL_WORD_SYM(val);
			break;
		default:
			Trap_Type(val);
	}

	switch (field->type) {
		case STRUCT_TYPE_INT8:
			*(i8*)data = (i8)i;
			break;
		case STRUCT_TYPE_UINT8:
			*(u8*)data = (u8)i;
			break;
		case STRUCT_TYPE_INT16:
			*(i16*)data = (i16)i;
			break;
		case STRUCT_TYPE_UINT16:
			*(u16*)data = (u16)i;
			break;
		case STRUCT_TYPE_INT32:
			*(i32*)data = (i32)i;
			break;
		case STRUCT_TYPE_UINT32:
		case STRUCT_TYPE_WORD:
			*(u32*)data = (u32)i;
			break;
		case STRUCT_TYPE_INT64:
			*(i64*)data = (i64)i;
			break;
		case STRUCT_TYPE_UINT64:
			*(u64*)data = (u64)i;
			break;
		case STRUCT_TYPE_POINTER:
			*(void**)data = (void*)i;
			break;
		case STRUCT_TYPE_FLOAT:
			*(float*)data = (float)d;
			break;
		case STRUCT_TYPE_DOUBLE:
			*(double*)data = (double)d;
			break;
		case STRUCT_TYPE_STRUCT:
			if (field->size != VAL_STRUCT_LEN(val)) {
				Trap_Arg(val);
			}
			if (same_fields(field->fields, VAL_STRUCT_FIELDS(val))) {
				memcpy(data, SERIES_SKIP(VAL_STRUCT_DATA_BIN(val), VAL_STRUCT_OFFSET(val)), field->size);
			} else {
				Trap_Arg(val);
			}
			break;
		default:
			/* should never be here */
			return FALSE;
	}
	return TRUE;
}

/***********************************************************************
**
*/	static REBFLG Set_Struct_Var(REBSTU *stu, REBVAL *word, REBVAL *elem, REBVAL *val)
/*
***********************************************************************/
{
	struct Struct_Field *field = NULL;
	REBCNT i = 0;
	field = (struct Struct_Field *)SERIES_DATA(stu->fields);
	for (i = 0; i < SERIES_TAIL(stu->fields); i ++, field ++) {
		if (VAL_WORD_CANON(word) == VAL_SYM_CANON(BLK_SKIP(PG_Word_Table.series, field->sym))) {
			if (field->array) {
				if (elem == NULL) { //set the whole array
					REBCNT n = 0;
					if ((!IS_BLOCK(val) || field->dimension != VAL_LEN(val))) {
						return FALSE;
					}

					for(n = 0; n < field->dimension; n ++) {
						if (!assign_scalar(stu, field, n, VAL_BLK_SKIP(val, n))) {
							return FALSE;
						}
					}

				} else {// set only one element
					if (!IS_INTEGER(elem)
						|| VAL_INT32(elem) <= 0
						|| VAL_UNT32(elem) > field->dimension) {
						return FALSE;
					}
					return assign_scalar(stu, field, VAL_INT32(elem) - 1, val);
				}
				return TRUE;
			} else {
				return assign_scalar(stu, field, 0, val);
			}
			return TRUE;
		}
	}
	return FALSE;
}

/* parse struct attribute */
static void parse_attr (REBVAL *blk, REBINT *raw_size, REBUPT *raw_addr)
{
	*raw_size = -1;
	*raw_addr = 0;

	REBVAL *attr = VAL_BLK_DATA(blk);
	while (NOT_END(attr)) {
		if (IS_SET_WORD(attr)) {
			switch (VAL_WORD_CANON(attr)) {
				case SYM_RAW_SIZE:
					++ attr;
					if (IS_INTEGER(attr)) {
						if (*raw_size > 0) { /* duplicate raw-size */
							Trap_Arg(attr);
						}
						*raw_size = VAL_INT64(attr);
						if (*raw_size <= 0) {
							Trap_Arg(attr);
						}
					} else {
						Trap_Arg(attr);
					}
					break;
				case SYM_RAW_MEMORY:
					++ attr;
					if (IS_INTEGER(attr)) {
						if (*raw_addr != 0) { /* duplicate raw-memory */
							Trap_Arg(attr);
						}
						*raw_addr = VAL_UNT64(attr);
						if (*raw_addr == 0) {
							Trap_Arg(attr);
						}
					} else {
						Trap_Arg(attr);
					}
					break;
				case SYM_EXTERN:
#ifdef unused
					++ attr;

					if (*raw_addr != 0) /* raw-memory is exclusive with extern */
						Trap_Arg(attr);

					if (!IS_BLOCK(attr)
						|| VAL_LEN(attr) != 2) {
						Trap_Arg(attr);
					} else {
						REBVAL *lib;
						REBVAL *sym;
						void *addr;

						lib = VAL_BLK_SKIP(attr, 0);
						sym = VAL_BLK_SKIP(attr, 1);

						if (!IS_LIBRARY(lib))
							Trap_Arg(attr);
						if (IS_CLOSED_LIB(VAL_LIB_HANDLE(lib)))
							Trap0(RE_BAD_LIBRARY);
						if (!ANY_BINSTR(sym))
							Trap_Arg(sym);

						addr = OS_FIND_FUNCTION(LIB_FD(VAL_LIB_HANDLE(lib)), VAL_DATA(sym));
						if (!addr)
							Trap1(RE_SYMBOL_NOT_FOUND, sym);

						*raw_addr = (REBUPT)addr;
					}
#else
					Trap0(RE_FEATURE_NA);
#endif
					break;
					/*
					   case SYM_ALIGNMENT:
					   ++ attr;
					   if (IS_INTEGER(attr)) {
					   alignment = VAL_INT64(attr);
					   } else {
					   Trap_Arg(attr);
					   }
					   break;
					   */
				default:
					Trap_Arg(attr);
			}
		} else {
			Trap_Arg(attr);
		}
		++ attr;
	}
}

/* set storage memory to external addr: raw_addr */
static void set_ext_storage (REBVAL *out, REBINT raw_size, REBUPT raw_addr)
{
	REBSER *ser = NULL;

	if (raw_size >= 0 && raw_size != VAL_STRUCT_LEN(out)) {
		Trap0(RE_INVALID_DATA);
	}

	ser = (REBSER *)Make_Node(SERIES_POOL);
	Prop_Series(ser, VAL_STRUCT_DATA_BIN(out));
	ser->data = (REBYTE*)raw_addr;
	EXT_SERIES(ser);

	VAL_STRUCT_DATA_BIN(out) = ser;
}

static REBOOL parse_field_type(struct Struct_Field *field, REBVAL *spec, REBVAL *inner, REBVAL **init)
{
	REBVAL *val = VAL_BLK_DATA(spec);

	if (IS_WORD(val)){
		switch (VAL_WORD_CANON(val)) {
			case SYM_UINT8X:
				field->type = STRUCT_TYPE_UINT8;
				field->size = 1;
				break;
			case SYM_INT8X:
				field->type = STRUCT_TYPE_INT8;
				field->size = 1;
				break;
			case SYM_UINT16X:
				field->type = STRUCT_TYPE_UINT16;
				field->size = 2;
				break;
			case SYM_INT16X:
				field->type = STRUCT_TYPE_INT16;
				field->size = 2;
				break;
			case SYM_UINT32X:
				field->type = STRUCT_TYPE_UINT32;
				field->size = 4;
				break;
			case SYM_INT32X:
				field->type = STRUCT_TYPE_INT32;
				field->size = 4;
				break;
			case SYM_UINT64X:
				field->type = STRUCT_TYPE_UINT64;
				field->size = 8;
				break;
			case SYM_INT64X:
				field->type = STRUCT_TYPE_INT64;
				field->size = 8;
				break;
			case SYM_FLOATX:
				field->type = STRUCT_TYPE_FLOAT;
				field->size = 4;
				break;
			case SYM_DOUBLEX:
				field->type = STRUCT_TYPE_DOUBLE;
				field->size = 8;
				break;
			case SYM_POINTER:
				field->type = STRUCT_TYPE_POINTER;
				field->size = sizeof(void*);
				break;
			case SYM_STRUCT_TYPE:
				++ val;
				if (IS_BLOCK(val)) {
					REBFLG res;

					res = MT_Struct(inner, val, REB_STRUCT);

					if (!res) {
						//RL_Print("Failed to make nested struct!\n");
						return FALSE;
					}

					field->size = SERIES_TAIL(VAL_STRUCT_DATA_BIN(inner));
					field->type = STRUCT_TYPE_STRUCT;
					field->fields = VAL_STRUCT_FIELDS(inner);
					field->spec = VAL_STRUCT_SPEC(inner);
					*init = inner; /* a shortcut for struct intialization */
				} else {
					Trap_Types(RE_EXPECT_VAL, REB_BLOCK, VAL_TYPE(val));
				}
				break;
			case SYM_WORD_TYPE:
				field->type = STRUCT_TYPE_WORD;
				field->size = 4;
				break;
#ifdef unused
			case SYM_REBVAL:
				field->type = STRUCT_TYPE_REBVAL;
				field->size = sizeof(REBVAL);
				break;
#endif
			default:
				Trap_Type(val);
		}
	} else if (IS_STRUCT(val)) { //[b: [struct-a] val-a]
		field->size = SERIES_TAIL(VAL_STRUCT_DATA_BIN(val));
		field->type = STRUCT_TYPE_STRUCT;
		field->fields = VAL_STRUCT_FIELDS(val);
		field->spec = VAL_STRUCT_SPEC(val);
		*init = val;
	} else {
		Trap_Type(val);
	}
	++ val;

	if (IS_BLOCK(val)) {// make struct! [a: [int32! [2]] [0 0]]

		REBVAL *ret = Do_Blk(VAL_SERIES(val), 0);

		if (!IS_INTEGER(ret)) {
			Trap_Types(RE_EXPECT_VAL, REB_INTEGER, VAL_TYPE(val));
		}
		field->dimension = (REBCNT)VAL_INT64(ret);
		field->array = TRUE;
		++ val;
	} else {
		field->dimension = 1; /* scalar */
		field->array = FALSE;
	}

	if (NOT_END(val)) {
		Trap_Type(val);
	}

	return TRUE;
}

/***********************************************************************
**
*/	REBFLG MT_Struct(REBVAL *out, REBVAL *data, REBCNT type)
/*
 * Format:
 * make struct! [
 *     field1  [type1]
 *     field2: [type2] field2-init-value
 * 	   field3: [struct [field1 [type1]]] field3-init-struct-value
 * 	   field4: [type1[3]] field4-init-block-value
 * 	   ...
 * ]
***********************************************************************/
{
	//RL_Print("%s\n", __func__);
	
	if (!IS_BLOCK(data)) return FALSE; // validate early!
	
	/* Using spec block length as a prediction of fields number
	   (each field requires at least 2 spec values)	*/
	REBCNT num_spec_values = VAL_TAIL(data) - VAL_INDEX(data);
	REBCNT min_fileds = num_spec_values >> 1;
	/* Don't allow empty struct! */
	if (min_fileds == 0) Trap_Arg(data);
	
	VAL_STRUCT_FIELDS(out) = Make_Series(min_fileds, sizeof(struct Struct_Field), FALSE);
	BARE_SERIES(VAL_STRUCT_FIELDS(out));

	//Reduce_Block_No_Set(VAL_SERIES(data), 0, NULL);
	//data = DS_POP;
	REBVAL *blk = VAL_BLK_DATA(data);
	REBINT field_idx = 0; /* for field index */
	u64 offset = 0; /* offset in data */
//	REBCNT eval_idx = 0; /* for spec block evaluation */
	REBVAL *init = NULL; /* for result to save in data */
	REBOOL expect_init = FALSE;
	REBINT raw_size = -1;
	REBUPT raw_addr = 0;
//	REBCNT alignment = 0;

	VAL_STRUCT_SPEC(out) = Copy_Series(VAL_SERIES(data));
	VAL_STRUCT_DATA(out) = Make_Series(1, sizeof(struct Struct_Data), FALSE);
	EXPAND_SERIES_TAIL(VAL_STRUCT_DATA(out), 1);
	BARE_SERIES(VAL_STRUCT_DATA(out));

	/* one byte per field as a minimum (expands when needed) */
	VAL_STRUCT_DATA_BIN(out) = Make_Series(min_fileds, 1, FALSE);
	BARE_SERIES(VAL_STRUCT_DATA_BIN(out));
	VAL_STRUCT_OFFSET(out) = 0;

	/* set type early such that GC will handle it correctly, i.e, not collect series in the struct */
	SET_TYPE(out, REB_STRUCT);

	if (IS_BLOCK(blk)) {
		parse_attr(blk, &raw_size, &raw_addr);
		++ blk;
	}

	while (NOT_END(blk)) {
		REBVAL *inner;
		struct Struct_Field *field = NULL;
		u64 step = 0;

		EXPAND_SERIES_TAIL(VAL_STRUCT_FIELDS(out), 1);
			
		DS_PUSH_NONE;
		inner = DS_TOP; /* save in stack so that it won't be GC'ed when MT_Struct is recursively called */

		field = (struct Struct_Field *)SERIES_SKIP(VAL_STRUCT_FIELDS(out), field_idx);
		field->offset = (REBCNT)offset;
		if (IS_SET_WORD(blk)) {
			field->sym = VAL_WORD_SYM(blk); 
			expect_init = TRUE;
			if (raw_addr) {
				/* initialization is not allowed for raw memory struct */
				Trap_Arg(blk);
			}
		} else if (IS_WORD(blk)) {
			field->sym = VAL_WORD_SYM(blk); 
			expect_init = FALSE;
		} else {
			Trap_Type(blk);
		}
		++ blk;

		if (!IS_BLOCK(blk)) {
			Trap_Arg(blk);
		}

		if (!parse_field_type(field, blk, inner, &init)) { return FALSE; }
		++ blk;

		STATIC_ASSERT(sizeof(field->size) <= 4);
		STATIC_ASSERT(sizeof(field->dimension) <= 4);

		step = (u64)field->size * (u64)field->dimension;
		if (step > VAL_STRUCT_LIMIT) {
			Trap1(RE_SIZE_LIMIT, out);
		}

		EXPAND_SERIES_TAIL(VAL_STRUCT_DATA_BIN(out), step);

		if (expect_init) {
			if (IS_END(blk)) {
				Trap_Types(RE_EXPECT_VAL, REB_STRUCT, REB_END);
			}
#ifdef ALLOW_CODE_EVALUATION_INSIDE_STRUCT_CONSTRUCTION_SPEC 
			else if (IS_BLOCK(blk)) {
				Reduce_Block(VAL_SERIES(blk), 0, NULL); //result is on stack
				init = DS_POP;
				++ blk;
			} else {
				eval_idx = blk - VAL_BLK_DATA(data);

				eval_idx = Do_Next(VAL_SERIES(data), eval_idx, 0);

				blk = VAL_BLK_SKIP(data, eval_idx);
				init = DS_POP; //Do_Next saves result on stack
			}
#else
			init = blk++;
#endif
			if (field->array) {
// O: It is probably not a good idea to let access to C pointers!
//    https://github.com/Oldes/Rebol-issues/issues/2567
//				if (IS_INTEGER(init)) { /* interpreted as a C pointer */
//					void *ptr = (void *)VAL_INT64(init);
//
//					/* assuming it's an valid pointer and holding enough space */
//					memcpy(SERIES_SKIP(VAL_STRUCT_DATA_BIN(out), (REBCNT)offset), ptr, field->size * field->dimension);
//				} else 
				if (IS_BLOCK(init)) {
					REBCNT n = 0;

					if (VAL_LEN(init) != field->dimension) {
						Trap_Arg(init);
					}
					/* assign */
					for (n = 0; n < field->dimension; n ++) {
						if (!assign_scalar(&VAL_STRUCT(out), field, n, VAL_BLK_SKIP(init, n))) {
							//RL_Print("Failed to assign element value\n");
							goto failed;
						}
					}
				} else {
					Trap_Types(RE_EXPECT_VAL, REB_BLOCK, VAL_TYPE(blk));
				}
			} else {
				/* scalar */
				if (!assign_scalar(&VAL_STRUCT(out), field, 0, init)) {
					//RL_Print("Failed to assign scalar value\n");
					goto failed;
				}
			}
		} else if (raw_addr == 0) {
			if (field->type == STRUCT_TYPE_STRUCT) {
				REBCNT n = 0;
				for (n = 0; n < field->dimension; n ++) {
					memcpy(SERIES_SKIP(VAL_STRUCT_DATA_BIN(out), ((REBCNT)offset) + n * field->size), SERIES_DATA(VAL_STRUCT_DATA_BIN(init)), field->size);
				}
			}
			else if (field->type == STRUCT_TYPE_WORD) {
				// use word `none` as a default value
				REBCNT n = 0;
				REBCNT sym = SYM_NONE;
				for (n = 0; n < field->dimension; n++) {
					memcpy(SERIES_SKIP(VAL_STRUCT_DATA_BIN(out), ((REBCNT)offset) + n * field->size), &sym, field->size);
				}
			}
			else if (field->type == STRUCT_TYPE_REBVAL) {
#ifdef unused
				REBVAL unset;
				REBCNT n = 0;
				SET_UNSET(&unset);
				for (n = 0; n < field->dimension; n ++) {
					if (!assign_scalar(&VAL_STRUCT(out), field, n, &unset)) {
						//RL_Print("Failed to assign element value\n");
						goto failed;
					}
				}
#endif
			} else {
				memset(SERIES_SKIP(VAL_STRUCT_DATA_BIN(out), (REBCNT)offset), 0, field->size * field->dimension);
			}
		}

		offset +=  step;
		/* 
		if (alignment != 0) {
			offset = ((offset + alignment - 1) / alignment) * alignment;
		}
		*/
		if (offset > VAL_STRUCT_LIMIT) {
			Trap1(RE_SIZE_LIMIT, out);
		}

		field->done = TRUE;

		++ field_idx;

		DS_POP; /* pop up the inner struct*/
	}

	VAL_STRUCT_LEN(out) = (REBCNT)offset;

	if (raw_addr) {
		set_ext_storage(out, raw_size, raw_addr);
	}

	return TRUE;


failed:
	Free_Series(VAL_STRUCT_FIELDS(out));
	Free_Series(VAL_STRUCT_SPEC(out));
	Free_Series(VAL_STRUCT_DATA_BIN(out));
	Free_Series(VAL_STRUCT_DATA(out));

	return FALSE;
}


/***********************************************************************
**
*/	REBINT PD_Struct(REBPVS *pvs)
/*
***********************************************************************/
{
//	struct Struct_Field *field = NULL;
//	REBCNT i = 0;
	REBSTU *stu = &VAL_STRUCT(pvs->value);
	if (!IS_WORD(pvs->select)) {
		return PE_BAD_SELECT;
	}
	if (! pvs->setval || NOT_END(pvs->path + 1)) {
		if (!Get_Struct_Var(stu, pvs->select, pvs->store)) {
			return PE_BAD_SELECT;
		}

		/* Setting element to an array in the struct:
		 * struct/field/1: 0
		 * */
		if (pvs->setval
			&& IS_BLOCK(pvs->store)
			&& IS_END(pvs->path + 2)) {
			REBVAL *sel = pvs->select;
			pvs->value = pvs->store;
			Next_Path(pvs); // sets value in pvs->value
			if (!Set_Struct_Var(stu, sel, pvs->select, pvs->value)) {
				return PE_BAD_SET;
			}
			return PE_OK;
		}
		return PE_USE;
	} else {// setval && END
		if (!Set_Struct_Var(stu, pvs->select, NULL, pvs->setval)) {
			return PE_BAD_SET;
		}
		return PE_OK;
	}
	return PE_BAD_SELECT;
}

/***********************************************************************
**
*/	REBINT Cmp_Struct(REBVAL *s, REBVAL *t)
/*
***********************************************************************/
{
	REBINT n = VAL_STRUCT_FIELDS(s) - VAL_STRUCT_FIELDS(t);
	if (n != 0) {
		return n;
	}
	n = VAL_STRUCT_DATA(s) - VAL_STRUCT_DATA(t);
	return n;
}

/***********************************************************************
**
*/	REBINT CT_Struct(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	//printf("comparing struct a (%p) with b (%p), mode: %d\n", a, b, mode);
	switch (mode) {
		case 3: /* same? */
		case 2: /* strict equality */
			return 0 == Cmp_Struct(a, b);
		case 1: /* equvilance */
		case 0: /* coersed equality*/
			if (Cmp_Struct(a, b) == 0) {
				return 1;
			}
			return IS_STRUCT(a) && IS_STRUCT(b)
				 && same_fields(VAL_STRUCT_FIELDS(a), VAL_STRUCT_FIELDS(b))
				 && VAL_STRUCT_LEN(a) == VAL_STRUCT_LEN(b)
				 && !memcmp(SERIES_DATA(VAL_STRUCT_DATA_BIN(a)), SERIES_DATA(VAL_STRUCT_DATA_BIN(b)), VAL_STRUCT_LEN(a));
		default:
			return -1;
	}
	return -1;
}

void Copy_Struct(REBSTU *src, REBSTU *dst)
{
	/* Read only fields */
	dst->spec = src->spec;
	dst->fields = src->fields;

	/* writable field */
	dst->data = Copy_Series(src->data);
	BARE_SERIES(dst->data);
	STRUCT_DATA_BIN(dst) = Copy_Series(STRUCT_DATA_BIN(src));
}

void Copy_Struct_Val(REBVAL *src, REBVAL *dst)
{
	SET_TYPE(dst, REB_STRUCT);
	Copy_Struct(&VAL_STRUCT(src), &VAL_STRUCT(dst));
}

/* a: make struct! [i: [uint8!] 1]
 * b: make a [i: 10]
 */
static void init_fields(REBVAL *ret, REBVAL *spec)
{
	REBVAL *blk = NULL;

	for (blk = VAL_BLK_DATA(spec); NOT_END(blk); blk += 2) {
		struct Struct_Field *fld = NULL;
		REBSER *fields = VAL_STRUCT_FIELDS(ret);
		REBCNT i = 0;
		REBVAL *word = blk;
		REBVAL *fld_val = blk + 1;

		if (IS_BLOCK(word)) { /* options: raw-memory, etc */
			REBINT raw_size = -1;
			REBUPT raw_addr = 0;

			if (VAL_TAIL(spec) != 1) { /* make sure no other field initialization */
				Trap_Arg(spec);
			}

			parse_attr(word, &raw_size, &raw_addr);
			set_ext_storage(ret, raw_size, raw_addr);

			break;
		} else if (! IS_SET_WORD(word)) {
			Trap_Arg(word);
		}

		if (IS_END(fld_val)) {
			Trap1(RE_NEED_VALUE, fld_val);
		}

		for (i = 0; i < SERIES_TAIL(fields); i ++) {
			fld = (struct Struct_Field*)SERIES_SKIP(fields, i);
			if (fld->sym == VAL_WORD_CANON(word)) {
				if (fld->dimension > 1) {
					REBCNT n = 0;
					if (IS_BLOCK(fld_val)) {
						if (VAL_LEN(fld_val) != fld->dimension) {
							Trap_Arg(fld_val);
						}
						for(n = 0; n < fld->dimension; n ++) {
							if (!assign_scalar(&VAL_STRUCT(ret), fld, n, VAL_BLK_SKIP(fld_val, n))) {
								Trap_Arg(fld_val);
							}
						}
					} else if (IS_INTEGER(fld_val)) {
						void *ptr = (void *)VAL_INT64(fld_val);

						/* assuming it's an valid pointer and holding enough space */
						memcpy(SERIES_SKIP(VAL_STRUCT_DATA_BIN(ret), fld->offset), ptr, fld->size * fld->dimension);
					} else {
						Trap_Arg(fld_val);
					}
				} else {
					if (!assign_scalar(&VAL_STRUCT(ret), fld, 0, fld_val)) {
						Trap_Arg(fld_val);
					}
				}
				break;
			}
		}

		if (i == SERIES_TAIL(fields)) {
			Trap_Arg(word); /* field not found in the parent struct */
		}
	}
}

/***********************************************************************
**
*/	REBTYPE(Struct)
/*
***********************************************************************/
{
	REBVAL *val;
	REBVAL *arg;
	REBSTU *strut;
	REBVAL *ret;
	
	arg = D_ARG(2);
	val = D_ARG(1);
	strut = 0;

	ret = DS_RETURN;
	// unary actions
	switch(action) {
		case A_MAKE:
			//RL_Print("%s, %d, Make struct action\n", __func__, __LINE__);
		case A_TO:
			//RL_Print("%s, %d, To struct action\n", __func__, __LINE__);

			// Clone an existing STRUCT:
			if (IS_STRUCT(val)) {
				Copy_Struct_Val(val, ret);

				/* only accept value initialization */
				init_fields(ret, arg);
			} else if (!IS_DATATYPE(val)) {
				goto is_arg_error;
			} else {
				// Initialize STRUCT from block:
				// make struct! [float a: 0]
				// make struct! [double a: 0]
				if (IS_BLOCK(arg)) {
					if (!MT_Struct(ret, arg, REB_STRUCT)) {
						goto is_arg_error;
					}
				} else {
					Trap_Make(REB_STRUCT, arg);
				}
			}
			SET_TYPE(ret, REB_STRUCT);
			break;

		case A_CHANGE:
			{
				if (!IS_BINARY(arg)) {
					Trap_Types(RE_EXPECT_VAL, REB_BINARY, VAL_TYPE(arg));
				}

				if (VAL_LEN(arg) != SERIES_TAIL(VAL_STRUCT_DATA_BIN(val))) {
					Trap_Arg(arg);
				}
				memcpy(SERIES_DATA(VAL_STRUCT_DATA_BIN(val)),
					   SERIES_DATA(VAL_SERIES(arg)),
					   SERIES_TAIL(VAL_STRUCT_DATA_BIN(val)));
			}
			break;
		case A_REFLECT:
			{
				REBINT n = VAL_WORD_CANON(arg); // zero on error
				switch (n) {
					case SYM_WORDS:
					case SYM_VALUES:
					case SYM_BODY:
						Get_Struct_Reflect(ret, &VAL_STRUCT(val), n);
						break;
					case SYM_SPEC:
						Set_Block(ret, Clone_Block(VAL_STRUCT_SPEC(val)));
						break;
					case SYM_ADDR:
						SET_INTEGER(ret, (REBUPT)SERIES_SKIP(VAL_STRUCT_DATA_BIN(val), VAL_STRUCT_OFFSET(val)));
						break;
					default:
						Trap_Reflect(REB_STRUCT, arg);
				}
			}
			break;

		case A_LENGTHQ:
			SET_INTEGER(ret, SERIES_TAIL(VAL_STRUCT_DATA_BIN(val)));
			break;
		default:
			Trap_Action(REB_STRUCT, action);
	}
	return R_RET;

is_arg_error:
	Trap_Types(RE_EXPECT_VAL, REB_STRUCT, VAL_TYPE(arg));

    return R_NONE; // Not needed, but Xcode complains 'Control may reach end of non-void function'
}
