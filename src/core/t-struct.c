/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2014 Atronix Engineering, Inc.
**  Copyright 2021-2026 Rebol Open Source Contributors
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
#include "sys-hash.h"

REBFLG MT_Struct(REBVAL *out, REBVAL *data, REBCNT type);
static void init_fields(REBVAL *ret, REBVAL *spec);


static const REBCNT type_to_sym [STRUCT_TYPE_MAX] = {
	SYM_UINT8X,
	SYM_INT8X,
	SYM_UINT16X,
	SYM_INT16X,
	SYM_UINT32X,
	SYM_INT32X,
	SYM_UINT64X,
	SYM_INT64X,
	NOT_FOUND, //SYM_INTEGER,

	SYM_FLOAT32X,
	SYM_FLOAT64X,
	NOT_FOUND, //SYM_DECIMAL,

	SYM_POINTER,
	NOT_FOUND, //SYM_STRUCT
	SYM_WORD_TYPE,
	SYM_REBVALX
};

// Vector encoding for field types which have a vector equivalent (-1 = none)
static const REBINT type_to_vect [STRUCT_TYPE_MAX] = {
	VTUI08,
	VTSI08,
	VTUI16,
	VTSI16,
	VTUI32,
	VTSI32,
	VTUI64,
	VTSI64,
	-1, //STRUCT_TYPE_INTEGER

	VTSF32,
	VTSF64,
	-1, //STRUCT_TYPE_DECIMAL

	-1, //STRUCT_TYPE_POINTER
	-1, //STRUCT_TYPE_STRUCT
	-1, //STRUCT_TYPE_WORD
	-1  //STRUCT_TYPE_REBVAL
};

// Array fields of these types are exposed as a block of values, not as a vector!
#define IS_BLOCK_BACKED_FIELD(f) \
	((f)->type > STRUCT_TYPE_DOUBLE || type_to_sym[(f)->type] == NOT_FOUND)

// The struct's data are packed, so its fields may be misaligned! Accessing
// them using a typed pointer is an undefined behavior, so the value must be
// copied in and out instead.
#define GET_FIELD_VALUE(type, set) { type n; COPY_MEM(&n, data, sizeof(n)); set(val, n); break; }
#define SET_FIELD_VALUE(type, num) { type n = (type)(num); COPY_MEM(data, &n, sizeof(n)); break; }

static REBFLG get_scalar(REBSTU *stu,
				  REBSTF *field,
				  REBCNT n, /* element index, starting from 0 */
				  REBVAL *val)
{
	REBYTE *data = STRUCT_DATA_BIN(stu) + field->offset + n * field->size;
	switch (field->type) {
		case STRUCT_TYPE_UINT8:   GET_FIELD_VALUE(u8,  SET_INTEGER)
		case STRUCT_TYPE_INT8:    GET_FIELD_VALUE(i8,  SET_INTEGER)
		case STRUCT_TYPE_UINT16:  GET_FIELD_VALUE(u16, SET_INTEGER)
		case STRUCT_TYPE_INT16:   GET_FIELD_VALUE(i16, SET_INTEGER)
		case STRUCT_TYPE_UINT32:  GET_FIELD_VALUE(u32, SET_INTEGER)
		case STRUCT_TYPE_INT32:   GET_FIELD_VALUE(i32, SET_INTEGER)
		case STRUCT_TYPE_UINT64:  GET_FIELD_VALUE(u64, SET_INTEGER)
		case STRUCT_TYPE_INT64:   GET_FIELD_VALUE(i64, SET_INTEGER)
		case STRUCT_TYPE_FLOAT:   GET_FIELD_VALUE(float,  SET_DECIMAL)
		case STRUCT_TYPE_DOUBLE:  GET_FIELD_VALUE(double, SET_DECIMAL)
		case STRUCT_TYPE_POINTER: GET_FIELD_VALUE(REBUPT, SET_INTEGER)
		case STRUCT_TYPE_STRUCT:
			{
				SET_TYPE(val, REB_STRUCT);
				VAL_STRUCT_SPEC(val) = field->spec;
				VAL_STRUCT_DATA(val) = stu->data;
				VAL_STRUCT_OFFSET(val) = stu->offset + field->offset + n * field->size;
				VAL_STRUCT_SIZE(val) = field->size;
			}
			break;

		case STRUCT_TYPE_WORD: {
				REBCNT sym;
				COPY_MEM(&sym, data, sizeof(sym));
				if (sym == 0) SET_NONE(val);
				else Set_Word(val, sym, NULL, 0);
			}
			break;
		case STRUCT_TYPE_REBVAL:
			COPY_MEM(val, data, sizeof(REBVAL));
			if (IS_END(val)) SET_NONE(val); // the field was never set
			break;
		default:
			/* should never be here */
			SET_NONE(val);
			return FALSE;
	}
	return TRUE;
}

// Retrieves the value of a struct field and stores it in `val`.
// Handles both array fields (returning a block or vector) and scalar fields.
static
void Get_Struct_Field_Value(REBSTU* stu, REBSTF* field, REBVAL* val)
{
	if (field->array) {
		REBSER* ser;
		if (IS_BLOCK_BACKED_FIELD(field)) {
			// Type has no vector equivalent - fall back to a block of scalars
			REBCNT n;
			ser = Make_Block(field->dimension);
			// The value must reference the series BEFORE anything else can be
			// allocated! `val` is usually pvs->store, which lives on the data
			// stack, so the GC would else mark a block with a stale series.
			Set_Block(val, ser);

			// Iterate over each element and extract it as a scalar
			for (n = 0; n < field->dimension; n++) {
				get_scalar(stu, field, n, Append_Value(ser));
			}
		}
		else {
			// Type maps to a known vector word - use a vector for efficiency
			ser = Make_Vector_From_Word((REBCNT)type_to_sym[field->type], field->dimension);
			SET_VECTOR(val, ser);

			// Bulk-copy the raw field bytes directly into the vector's data buffer
			COPY_MEM(
				SERIES_DATA(ser),
				STRUCT_DATA_BIN(stu) + field->offset,
				field->dimension * field->size
			);
		}
	}
	else {
		// Non-array field - retrieve a single scalar value directly
		get_scalar(stu, field, 0, val);
	}
}

// Searches a struct's field list for a field whose symbol matches `word`.
// Returns a pointer to the matching REBSTF, or NULL if not found.
static
REBSTF* Get_Struct_Field(REBSTU* stu, REBVAL* word)
{
	REBSTF* field = STRUCT_FIELDS(stu);

	for (REBCNT i = 0; i < STRUCT_FIELDS_NUM(stu); i++, field++) {
		// Compare canonical (case-insensitive) symbol of word against field's symbol
		if (VAL_WORD_CANON(word) == VAL_SYM_CANON(BLK_SKIP(PG_Word_Table.series, field->sym))) {
			return field;
		}
	}

	return NULL; // No matching field found
}

// Looks up a field by word in a struct and writes its value into `val`.
// Returns TRUE if the field was found and val was populated, FALSE otherwise.
static
REBFLG Get_Struct_Var(REBSTU *stu, REBVAL *word, REBVAL *val)
{
	REBSTF *field = NULL;
	REBCNT i = 0;

	field = STRUCT_FIELDS(stu);

	for (i = 0; i < STRUCT_FIELDS_NUM(stu); i++, field++) {
		// Match the word against each field's canonical symbol
		if (VAL_WORD_CANON(word) == VAL_SYM_CANON(BLK_SKIP(PG_Word_Table.series, field->sym))) {
			Get_Struct_Field_Value(stu, field, val);
			return TRUE;
		}
	}
	return FALSE; // Word did not match any field in this struct
}

/***********************************************************************
**
*/	void Get_Struct_Reflect(REBVAL *ret, REBSTU *stu, REBCNT type)
/*
***********************************************************************/
{
	REBVAL* val = NULL;
	REBSER* out, * dim;
	REBSTF* field = STRUCT_FIELDS(stu);
	REBCNT i, n, cnt;

	cnt = STRUCT_FIELDS_NUM(stu);
	out = Make_Block(cnt * (type==SYM_BODY?2:1));
	Set_Block(ret, out);
	LABEL_SERIES(out, "struct_body");

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
			if (field->array) {
				dim = Make_Block(field->dimension);
				Set_Block(val, dim);
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
	if (SERIES_TAIL(tgt) != SERIES_TAIL(src)) {
		return FALSE;
	}
	REBSTI *tgt_info = (REBSTI*) SERIES_DATA(tgt);
	REBSTI *src_info = (REBSTI*) SERIES_DATA(src);
	return (src_info->hash == tgt_info->hash);
}

static REBOOL assign_scalar(REBSTU *stu,
							REBSTF *field,
							REBCNT n, /* element index, starting from 0 */
							REBVAL *val)
{
	u64 i = 0;
	double d = 0;
	void *data = STRUCT_DATA_BIN(stu) + field->offset + n * field->size;

	if (field->type == STRUCT_TYPE_REBVAL) {
		COPY_MEM(data, val, sizeof(REBVAL));
		return TRUE;
	}

	switch (VAL_TYPE(val)) {
		case REB_DECIMAL:
			if (!IS_STRUCT_NUMERIC_TYPE(field->type)) {
				Trap_Type(val);
			}
			d = VAL_DECIMAL(val);
			if (IS_STRUCT_INTEGER_TYPE(field->type)) {
				// Casting a value which does not fit into a 64bit integer
				// (or a NaN) is an undefined behavior in C!
				if (!(d >= -9223372036854775808.0 && d < 9223372036854775808.0))
					Trap0(RE_OVERFLOW);
				// Signed conversion! Truncates towards zero also for negatives.
				i = (u64)(i64)d;
			}
			break;
		case REB_INTEGER:
			if (!IS_STRUCT_NUMERIC_TYPE(field->type)
				&& field->type != STRUCT_TYPE_POINTER) {
				Trap_Type(val);
			}
			i = (u64) VAL_INT64(val);
			// Must not be converted from the unsigned value, else the sign is lost!
			d = (double)VAL_INT64(val);
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
		case REB_BLOCK:
			if (STRUCT_TYPE_STRUCT == field->type) {
				DS_PUSH_NONE;
				SET_TYPE(DS_TOP, REB_STRUCT);
				VAL_STRUCT_SPEC(DS_TOP) = field->spec;
				VAL_STRUCT_DATA(DS_TOP) = stu->data;
				// The offset must be relative to the data series, so it has to
				// include the offset of the (possibly nested) parent struct!
				VAL_STRUCT_OFFSET(DS_TOP) = stu->offset + field->offset + n * field->size;
				VAL_STRUCT_SIZE(DS_TOP) = field->size;
				init_fields(DS_TOP, val);
				DS_POP;
			}
			else Trap_Type(val);
			return TRUE;
		default:
			Trap_Type(val);
	}

	switch (field->type) {
		case STRUCT_TYPE_INT8:    SET_FIELD_VALUE(i8,  i)
		case STRUCT_TYPE_UINT8:   SET_FIELD_VALUE(u8,  i)
		case STRUCT_TYPE_INT16:   SET_FIELD_VALUE(i16, i)
		case STRUCT_TYPE_UINT16:  SET_FIELD_VALUE(u16, i)
		case STRUCT_TYPE_INT32:   SET_FIELD_VALUE(i32, i)
		case STRUCT_TYPE_UINT32:
		case STRUCT_TYPE_WORD:    SET_FIELD_VALUE(u32, i)
		case STRUCT_TYPE_INT64:   SET_FIELD_VALUE(i64, i)
		case STRUCT_TYPE_UINT64:  SET_FIELD_VALUE(u64, i)
		case STRUCT_TYPE_POINTER: SET_FIELD_VALUE(REBUPT, i)
		case STRUCT_TYPE_FLOAT:   SET_FIELD_VALUE(float,  d)
		case STRUCT_TYPE_DOUBLE:  SET_FIELD_VALUE(double, d)
		case STRUCT_TYPE_STRUCT:
			if (field->size != VAL_STRUCT_SIZE(val)) {
				Trap_Arg(val);
			}
			if (same_fields(field->spec->series, VAL_STRUCT_FIELDS(val))) {
				COPY_MEM(data, VAL_STRUCT_DATA_BIN(val), field->size);
			}
			else {
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
	REBSTF *field = STRUCT_FIELDS(stu);
	REBCNT i = 0;
	for (i = 0; i < STRUCT_FIELDS_NUM(stu); i ++, field ++) {
		if (VAL_WORD_CANON(word) == VAL_SYM_CANON(BLK_SKIP(PG_Word_Table.series, field->sym))) {
			if (field->array) {
				if (elem == NULL) { //set the whole array
					// Must be a block or vector with the same number of values as the array.
					if (!(IS_VECTOR(val) || IS_BLOCK(val)) || field->dimension != VAL_LEN(val)) {
						return FALSE;
					}
					if (IS_VECTOR(val)) {
						// The vector's element type must match the field's type!
						// Else raw bytes of a foreign type would be stored, which
						// is fatal for a word! field (an invalid symbol id).
						REBINT vect = type_to_vect[field->type];
						if (vect < 0 || VECT_TYPE(VAL_SERIES(val)) != (REBCNT)vect) {
							return FALSE;
						}
						// NOTE: VAL_DATA (not VAL_BIN_DATA) - the vector's index
						// counts elements, not bytes!
						COPY_MEM(STRUCT_DATA_BIN(stu) + field->offset, VAL_DATA(val), field->dimension * field->size);
					}
					else {
						// data in a block
						for (REBCNT n = 0; n < field->dimension; n++) {
							if (!assign_scalar(stu, field, n, VAL_BLK_SKIP(val, n))) {
								return FALSE;
							}
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

#ifdef unused_struct
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

	if (raw_size >= 0 && raw_size != VAL_STRUCT_SIZE(out)) {
		Trap0(RE_INVALID_DATA);
	}

	ser = (REBSER *)Make_Node(SERIES_POOL);
	Prop_Series(ser, VAL_STRUCT_DATA_BIN(out));
	ser->data = (REBYTE*)raw_addr;
	EXT_SERIES(ser);

	VAL_STRUCT_DATA_BIN(out) = ser;
}
#endif

static REBOOL parse_field_type(REBSTU *stu, REBSTF *field, REBVAL *spec)
{
	REBVAL *val = VAL_BLK_DATA(spec);

	if (IS_WORD(val)){
		REBCNT sym = VAL_WORD_SYM(val) = Normalize_Vector_Type_Symbol(VAL_WORD_CANON(val));
		switch (sym) {
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
			case SYM_FLOAT32X:
				field->type = STRUCT_TYPE_FLOAT;
				field->size = 4;
				break;
			case SYM_FLOAT64X:
				field->type = STRUCT_TYPE_DOUBLE;
				field->size = 8;
				break;
			case SYM_STRUCT_TYPE:
				field->type = STRUCT_TYPE_STRUCT;
				field->size = 0;
				++ val;
				if (IS_BLOCK(val) || IS_WORD(val) || IS_INTEGER(val)) {
					REBFLG res;
					DS_PUSH_END;
					REBVAL *inner = DS_TOP;
					res = Prepare_Struct(inner, val);
					if (res) {
						field->size = VAL_STRUCT_SIZE(inner);
						field->spec = VAL_STRUCT_SPEC(inner);
						STRUCT_FLAGS(stu) |= VAL_STRUCT_FLAGS(inner);
					}
					DS_POP;
					if (!res) return FALSE; //RL_Print("Failed to make nested struct!\n");
				}
				else {
					return FALSE;
				}
				break;
#ifdef TODO
			case SYM_POINTER:
				field->type = STRUCT_TYPE_POINTER;
				field->size = sizeof(void *);
				break;
#endif
			case SYM_WORD_TYPE:
				field->type = STRUCT_TYPE_WORD;
				field->size = 4;
				break;
			case SYM_REBVALX:
				field->type = STRUCT_TYPE_REBVAL;
				field->size = sizeof(REBVAL);
				break;
			default:
				return FALSE;
		}
	}
	else {
		return FALSE;
	}
	++ val;

	if (IS_BLOCK(val)) {// make struct [a [int32! [2]]]
		// Array field - the block must hold exactly one positive integer
		// which fits into the field's dimension (32 bits)!
		REBVAL *dim = VAL_BLK_DATA(val);
		if (!IS_INTEGER(dim) || NOT_END(dim + 1)) {
			return FALSE;
		}
		if (VAL_INT64(dim) < 1 || VAL_INT64(dim) > (REBI64)VAL_STRUCT_LIMIT) {
			return FALSE;
		}
		field->dimension = (REBCNT)VAL_INT64(dim);
		field->array = TRUE;
		++ val;
	} else {
		field->dimension = 1; /* scalar */
		field->array = FALSE;
	}

	if (NOT_END(val)) {
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************
**
*/	REBFLG MT_Struct(REBVAL *out, REBVAL *data, REBCNT type)
/*
***********************************************************************/
{
	REBVAL *values = data + 1;

	if (!Prepare_Struct(out, data))
		return FALSE;

	// At this point, the struct specification should be ready.
	REBSTU *stu = &VAL_STRUCT(out);

	// The struct always owns its data!
	STRUCT_DATA(stu) = Make_Binary(STRUCT_SIZE(stu));
	// Rebol values which may be stored in the data are marked by Mark_Struct!
	BARE_SERIES(STRUCT_DATA(stu));
	LABEL_SERIES(STRUCT_DATA(stu), "struct_data");
	SERIES_TAIL(STRUCT_DATA(stu)) = STRUCT_SIZE(stu);
	// Remember the root field list in the (otherwise unused) series link, so
	// that the GC can always mark ALL values in the data, even when the data
	// are reached only from a value which is a view into a nested part!
	STRUCT_DATA_ROOT(stu) = STRUCT_FIELDS_SER(stu);

	if (IS_BINARY(values)) {
		// Raw data must not be used to initialize a struct holding Rebol
		// values, because the GC would try to mark random bytes as values!
		if (STRUCT_PROTECTED(stu)) Trap0(RE_PROTECTED);
		if (VAL_BIN_LEN(values) < STRUCT_SIZE(stu)) Trap_Arg(values);
		COPY_MEM(STRUCT_DATA_BIN(stu), VAL_BIN_DATA(values), STRUCT_SIZE(stu));
	}
	else if (IS_BLOCK(values)) {
		init_fields(out, values);
	}
	return TRUE;
}

/***********************************************************************
**
*/	REBFLG Prepare_Struct(REBVAL *out, REBVAL *data)
/*
 * Format:
 * make struct! [
 *     field1  [type]    "Single value"
 * 	   field2  [type[3]] "Array with 3 values"
 * 	   ...
 * ]
***********************************************************************/
{
	REBVAL key;
	REBVAL spec;
	REBVAL *struct_specs = Get_System(SYS_CATALOG, CAT_STRUCTS);
	REBCNT hash = 0, n = NOT_FOUND;
	REBOOL new_spec = FALSE;

	if (IS_INTEGER(data)) {
		// Struct spec id used like:
		// #(struct! 111111 [a: 1 b: 2])
		hash = VAL_UNT32(data);
	}
	else if (IS_WORD(data)) {
		// Struct spec as a registered struct name
		// #(struct! some-struct [a: 1 b: 2])
		n = Find_Entry(VAL_SERIES(struct_specs), data, 0, TRUE);
	}
	else if (!IS_BLOCK(data)) return FALSE; // validate early!
	else {
		hash = Hash_Block_Value(data);
		// The zero hash means "no key to look for", so a spec must never use it!
		if (hash == 0) hash = 1;
	}

	SET_INTEGER(&key, hash);
	if (hash) {
		n = Find_Entry(VAL_SERIES(struct_specs), &key, 0, TRUE);
	}
	if (n == NOT_FOUND) {
		if (!IS_BLOCK(data)) {
			Trap_Arg(data);
		}
		Set_Block(&spec, Clone_Block(VAL_SERIES(data)));
		LABEL_SERIES(VAL_SERIES(&spec), "struct_spec");
		// make sure that user cannot modify it
		Protect_Series(&spec, FLAGIT(PROT_SET) | FLAGIT(PROT_LOCK) | FLAGIT(PROT_DEEP));
		// NOTE: the spec is not disposed on exit because of this protection!
		new_spec = TRUE;
	}
	else {
		spec = *VAL_BLK_SKIP(struct_specs, n);
	}

	REBSTU *stu = &VAL_STRUCT(out);
	VAL_STRUCT_SPEC(out) = VAL_SERIES(&spec);
	VAL_STRUCT_DATA(out) = NULL;
	//VAL_STRUCT_FIELDS(out) = NULL;
	VAL_STRUCT_OFFSET(out) = 0;
	/* set type early such that GC will handle it correctly, i.e, not collect series in the struct */
	SET_TYPE(out, REB_STRUCT);

	if (VAL_STRUCT_FIELDS(out) == NULL) {
		REBVAL *blk = VAL_BLK_DATA(&spec);
		REBINT field_num = 0; /* for field index */
		REBU64 offset = 0;    /* offset in data */
		REBCNT error = 0;
		REBCNT k1, h1 = 0;    /* hash computation */

		// Count fields and validate spec to optimize series preallocation
		while (IS_STRING(blk)) ++blk;
		if (IS_BLOCK(blk)) ++blk;
		while (NOT_END(blk)) {
			if (IS_WORD(blk) && IS_BLOCK(blk + 1)) {
				++field_num;
				blk += 2;
				while (IS_STRING(blk)) ++blk;
			}
			else Trap1(RE_MALCONSTRUCT, blk);
		}
		// Don't allow empty struct!
		if (field_num == 0) Trap1(RE_MALCONSTRUCT, data);
		// Reset specification
		blk = VAL_BLK_DATA(&spec);
		// Initialize series to store field metadata
		VAL_STRUCT_FIELDS(out) = Make_Series(field_num+1, sizeof(REBSTF), FALSE); // keeps info at its head
		VAL_STRUCT_COUNT(out) = field_num;
		BARE_SERIES(VAL_STRUCT_FIELDS(out));                  // does not hold Rebol values
		KEEP_SERIES(VAL_STRUCT_FIELDS(out), "struct_fields"); // protect from GC
		SERIES_TAIL(VAL_STRUCT_FIELDS(out)) = field_num = 1;  // info at the head
		VAL_STRUCT_ID(out) = hash;

		// skip optional doc strings
		while (IS_STRING(blk)) ++blk;

		// optional attributes
		if (IS_BLOCK(blk)) {
			//TODO: ignored now!
		//	parse_attr(blk, &raw_size, &raw_addr);
			++blk;
		}

		//!!! IMPORTANT NOTE !!!
		//!!! Don't throw an error from this loop!
		//!!! VAL_STRUCT_FIELDS(out) must be manually freed if an error occurs!
		#define FIELD_ERROR_SIZE_LIMIT   1
		#define FIELD_ERROR_INVALID_SPEC 2
		while (NOT_END(blk)) {
			REBSTF *field = NULL;
			REBU64 step = 0;

			// The specification was partially verified.
			// Content should be: [WORD BLOCK opt STR...]

			field = (REBSTF *)SERIES_SKIP(VAL_STRUCT_FIELDS(out), field_num);
			field->offset = (REBCNT)offset;
			field->sym = VAL_WORD_CANON(blk);
			VAL_SET_LINE(blk);
			++blk;

			if (!parse_field_type(stu, field, blk)) {
				error = FIELD_ERROR_INVALID_SPEC;
				break;
			}
			// Fields holding Rebol values must be aligned for the GC!
			if (field->type == STRUCT_TYPE_REBVAL
				|| (field->type == STRUCT_TYPE_STRUCT && FIELD_SPEC_HAS_VALUES(field))) {
				offset = ALIGN(offset, STRUCT_VALUE_ALIGN);
				field->offset = (REBCNT)offset;
			}
			// Compute a hash of the field's type and dimensions (used for fast struct comparison).
			k1 = field->type;
			bmix(&h1, &k1);
			k1 = field->dimension;
			bmix(&h1, &k1);
			if (field->spec) {
				// Inner struct...
				if (field->spec->series) {
					REBSTI *inf = (REBSTI *)SERIES_DATA(field->spec->series);
					k1 = inf->hash;
					bmix(&h1, &k1);
				}
			}
			VAL_CLR_LINE(blk);
			++blk;

			// skip optional doc strings
			while (IS_STRING(blk)) {
				VAL_CLR_LINE(blk);
				++blk;
			}

			STATIC_ASSERT(sizeof(field->size) <= 4);
			STATIC_ASSERT(sizeof(field->dimension) <= 4);

			step = (REBU64)field->size * (REBU64)field->dimension;
			if (step > VAL_STRUCT_LIMIT) {
				error = FIELD_ERROR_SIZE_LIMIT;
				break;
			}

			offset += step;
			/*
			if (alignment != 0) {
				//offset = ((offset + alignment - 1) / alignment) * alignment;
				// if alignement is power of 2:
				offset = (offset + alignment - 1) & ~(alignment - 1);
			}
			*/
			if (offset > VAL_STRUCT_LIMIT) {
				error = FIELD_ERROR_SIZE_LIMIT;
				break;
			}
			
			if (field->type == STRUCT_TYPE_REBVAL)
				VAL_STRUCT_FLAGS(out) |= STRUCT_FLAG_MARK | STRUCT_FLAG_PROTECTED;
		
			field->done = TRUE;
			++SERIES_TAIL(VAL_STRUCT_FIELDS(out));
			++field_num;
		}

		if (error) {
			Free_Series(VAL_STRUCT_FIELDS(out));
			VAL_STRUCT_FIELDS(out) = NULL; // don't keep a freed series in the spec!
			SET_UNSET(out);
			switch (error) {
			case FIELD_ERROR_SIZE_LIMIT:   Trap1(RE_SIZE_LIMIT, data);
			case FIELD_ERROR_INVALID_SPEC: Trap_Arg(blk);
			}
		}

		//Dump_Series(VAL_STRUCT_FIELDS(out), "struct_fields");

		// Store complete length of the struct
		// (rounded up, so that an array of these structs keeps its values aligned)
		if (VAL_STRUCT_FLAGS(out) & 1) offset = ALIGN(offset, STRUCT_VALUE_ALIGN);
		STRUCT_SIZE(stu) = (REBCNT)offset;

		// Finalize and store the fields hash
		h1 ^= STRUCT_SIZE(stu);
		STRUCT_HASH(stu) = fmix32(h1);

		// Append value to system/catalog/structs
		n = Find_Entry(VAL_SERIES(struct_specs), &key, &spec, TRUE);
		ASSERT1(n != NOT_FOUND, RP_NO_STRUCT_REGISTER);
	}


	return TRUE;
}


/***********************************************************************
**
*/	REBINT PD_Struct(REBPVS *pvs)
/*
***********************************************************************/
{
	REBSTU* stu = &VAL_STRUCT(pvs->value);
	REBSTF* field = NULL;
	REBFLG res = 1;

	// Struct allows only named field access (so far).
	if (!IS_WORD(pvs->select))
		return PE_BAD_SELECT;

	//Debug_Fmt("?? setval: %r pvs->path+1: %r", pvs->setval, pvs->path+1);
	if (!pvs->setval || NOT_END(pvs->path + 1)) {
		// Get-path or deep set-path (e.g. struct/field/1: 0)

		// Find the requested field by name; bail if not found.
		if (!(field = Get_Struct_Field(stu, pvs->select)))
			return PE_BAD_SELECT;

		// Retrieve the field's current value into pvs->store.
		Get_Struct_Field_Value(stu, field, pvs->store);

		//Debug_Fmt("?? store: %r value: %r", pvs->store, pvs->value);

		// Simple get-path - just return the stored value.
		if (!pvs->setval) return PE_USE;

		// Deep set-path: save the field selector, then advance pvs->value
		// to pvs->store so Next_Path operates on the field's current value.
		REBVAL* sel = pvs->select;
		REBVAL* prev_path = pvs->path;
		pvs->value = pvs->store;

		// Walk the remainder of the path, resolving the final value to set.
		Next_Path(pvs);

		//Debug_Fmt("<- value: %r select: %r prev: %r", pvs->value, pvs->select, sel);
		//Debug_Fmt("== store: %r value : %r", pvs->store, pvs->value);

		// NOTE: don't test pvs->store here! It may be already modified
		// by Next_Path above (it is used as a scratch value).
		if (field->array && IS_INTEGER(pvs->select)) {
			// Setting one element of an array field by index,
			// e.g.: st/arr/2: st/arr/1
			// NOTE: the index is validated when the temporary series with the
			// array's values is accessed, but don't rely on it here!
			REBI64 idx = VAL_INT64(pvs->select);
			if (idx < 1 || idx > (REBI64)field->dimension)
				return PE_BAD_SET; // index out of range
			if (IS_BLOCK_BACKED_FIELD(field)) {
				// Next_Path modified only the temporary block, so the new
				// value must still be stored into the struct's data.
				// assign_scalar validates that the value may be assigned!
				res = assign_scalar(stu, field, (REBCNT)(idx - 1), pvs->value);
			} else {
				// Numeric arrays are exposed as a vector which Next_Path
				// modified in place - store the whole vector back.
				res = Set_Struct_Var(stu, sel, NULL, pvs->value);
			}
		}
		else switch (field->type) {
		case STRUCT_TYPE_STRUCT:
			// A nested struct shares the data series, so the inner set-path
			// has already written through it - nothing to do here.
			break;
		case STRUCT_TYPE_REBVAL:
			// An immediate value (like date!) is modified only in the scratch
			// value, so it must be stored back into the field. That is valid
			// just for a single path step - a deeper path uses pvs->store for
			// its own results and modifies the real value in place anyway!
			// A numeric sub-select means Next_Path already resolved the target.
			if (!IS_INTEGER(pvs->select) && pvs->path == prev_path + 1)
				res = Set_Struct_Var(stu, sel, NULL, pvs->store);
			break;
		default:
			// For all other types, set the field to the path-resolved value.
			res = Set_Struct_Var(stu, sel, NULL, pvs->value);
		}
	}
	else {
		// Simple set-path (e.g. struct/field: 123) - set the field directly.
		res = Set_Struct_Var(stu, pvs->select, NULL, pvs->setval);
	}
	return res ? PE_OK : PE_BAD_SET;
}

/***********************************************************************
**
*/	REBINT Cmp_Struct(REBVAL *s, REBVAL *t)
/*
**		Compares identity only: the same fields (spec), the same data
**		series and the same position in it. Never compares content!
**
***********************************************************************/
{
    REBUPT a, b;

    a = (REBUPT)VAL_STRUCT_FIELDS(s);
    b = (REBUPT)VAL_STRUCT_FIELDS(t);
    if (a < b) return -1;
    if (a > b) return 1;

    a = (REBUPT)VAL_STRUCT_DATA(s);
    b = (REBUPT)VAL_STRUCT_DATA(t);
    if (a < b) return -1;
    if (a > b) return 1;

    a = VAL_STRUCT_OFFSET(s);
    b = VAL_STRUCT_OFFSET(t);
    if (a < b) return -1;
    if (a > b) return 1;

    return 0;
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
				 && VAL_STRUCT_SIZE(a) == VAL_STRUCT_SIZE(b)
				 && same_fields(VAL_STRUCT_FIELDS(a), VAL_STRUCT_FIELDS(b))
				 && !memcmp(VAL_STRUCT_DATA_BIN(a), VAL_STRUCT_DATA_BIN(b), VAL_STRUCT_SIZE(a));
		default:
			return -1;
	}
	return -1;
}

static void Copy_Struct(REBSTU *src, REBSTU *dst)
{
	/* Read only field */
	dst->spec = src->spec;

	/* Writable field */
	STRUCT_DATA(dst) = Make_Binary(STRUCT_SIZE(src));
	BARE_SERIES(STRUCT_DATA(dst));
	SERIES_TAIL(STRUCT_DATA(dst)) = STRUCT_SIZE(src);
	STRUCT_DATA_ROOT(dst) = STRUCT_FIELDS_SER(dst); // the root field list
	COPY_MEM(STRUCT_DATA_BIN(dst), STRUCT_DATA_BIN(src), STRUCT_SIZE(src));
}

static void Copy_Struct_Val(REBVAL *src, REBVAL *dst)
{
	SET_STRUCT(dst);
	Copy_Struct(&VAL_STRUCT(src), &VAL_STRUCT(dst));
}

static void init_field(REBVAL *ret, REBSTF *fld, REBVAL *value) {
	if (fld->array) {
		if (IS_BLOCK(value)) {
			if (VAL_LEN(value) != fld->dimension) {
				Trap_Arg(value);
			}
			for (REBCNT n = 0; n < fld->dimension; n++) {
				if (!assign_scalar(&VAL_STRUCT(ret), fld, n, VAL_BLK_SKIP(value, n))) {
					Trap_Arg(value);
				}
			}
		}
		else {
			Trap_Arg(value);
		}
	}
	else {
		if (!assign_scalar(&VAL_STRUCT(ret), fld, 0, value)) {
			Trap_Arg(value);
		}
	}
}

/* a: make struct! [i: [uint8!] 1]
 * b: make a [i: 10]
 */
static void init_fields(REBVAL *ret, REBVAL *spec)
{
	REBVAL *blk = VAL_BLK_DATA(spec);
	REBVAL *word;
	REBVAL *fld_val;
	REBSTF *fld = NULL;
	REBSER *fields = VAL_STRUCT_FIELDS(ret);

	if (IS_SET_WORD(blk)) {
		// Specification in format like: [a: 1 b: 2]
		for (; NOT_END(blk); blk += 2) {
			REBCNT i = 0;
			word = blk;
			fld_val = blk + 1;
			if (!IS_SET_WORD(word))
				Trap_Arg(word);
			if (IS_END(fld_val))
				Trap1(RE_NEED_VALUE, word);
			// Iterate all fields (first value is used for info)
			for (i = 1; i < SERIES_TAIL(fields); i++) {
				fld = (REBSTF *)SERIES_SKIP(fields, i);
				if (fld->sym == VAL_WORD_CANON(word)) {
					init_field(ret, fld, fld_val);
					break;
				}
			}

			if (i == SERIES_TAIL(fields)) {
				Trap_Arg(word); /* field not found in the parent struct */
			}
		}
	}
	else {
		// Only values...
		for (REBCNT i = 1; i < SERIES_TAIL(fields); i++) {
			if (IS_END(blk)) return;
			fld = (REBSTF *)SERIES_SKIP(fields, i);
			init_field(ret, fld, blk);
			++blk;
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
		case A_TO:
			// Clone an existing STRUCT:
			if (IS_STRUCT(val)) {
				// Raw data must never be used with a struct holding Rebol values,
				// no matter how long the data are!
				if (IS_BINARY(arg) && VAL_STRUCT_PROTECTED(val)) Trap0(RE_PROTECTED);
				Copy_Struct_Val(val, ret);
				/* only accept value initialization */
				if (IS_BLOCK(arg)) {
					// The values are used as they are, like in the construction
					// syntax. Use `compose` when evaluation is needed, e.g.:
					//     make proto compose [a: (1 + 2)]
					init_fields(ret, arg);
				}
				else if (IS_BINARY(arg) && VAL_BIN_LEN(arg) >= VAL_STRUCT_SIZE(val)) {
					//TODO: special error when data are not large enough?
					COPY_MEM(VAL_STRUCT_DATA_BIN(ret), VAL_BIN_DATA(arg), VAL_STRUCT_SIZE(val));
				}
				else {
					Trap_Arg(arg);
				}
			} else if (!IS_DATATYPE(val)) {
				goto is_arg_error;
			} else {
				// Initialize STRUCT from block:
				// make struct! [a [uint16!]]
				if (IS_BLOCK(arg)) {
					// MT_Struct expects the spec to be followed by an optional
					// initial value (block or binary), like in the construction
					// syntax: #(struct! [a [uint16!]] [1]). There is none here,
					// so pass a terminated pair instead of relying on the value
					// which happens to follow the argument on the stack!
					REBVAL data[2];
					data[0] = *arg;
					SET_END(&data[1]);
					if (!MT_Struct(ret, data, REB_STRUCT)) {
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
				if (IS_BLOCK(arg)) {
					init_fields(val, arg);
					return R_ARG1;
				}
				if (VAL_STRUCT_PROTECTED(val)) Trap0(RE_PROTECTED);
				if (!IS_BINARY(arg)) {
					Trap_Types(RE_EXPECT_VAL, REB_BINARY, VAL_TYPE(arg));
				}
				COPY_MEM(VAL_STRUCT_DATA_BIN(val), VAL_BIN_DATA(arg), MIN(VAL_BIN_LEN(arg),VAL_STRUCT_SIZE(val)));
				return R_ARG1;
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
						// no need to copy as it is protected value
						Set_Block(ret, VAL_STRUCT_SPEC(val));
						break;
					default:
						Trap_Reflect(REB_STRUCT, arg);
				}
			}
			break;
		//TODO: A_QUERY to access struct's name and id?

		case A_LENGTHQ:
			SET_INTEGER(ret, VAL_STRUCT_SIZE(val));
			break;

		case A_CLEAR:
			CLEAR(VAL_STRUCT_DATA_BIN(val), VAL_STRUCT_SIZE(val))
			return R_ARG1;

		case A_COPY:
			// Allow only a simple copy without any refinements.
			if (D_REF(ARG_COPY_PART) || D_REF(ARG_COPY_DEEP) || D_REF(ARG_COPY_TYPES))
				Trap0(RE_BAD_REFINES);
			Copy_Struct_Val(val, ret);
			break;

		default:
			Trap_Action(REB_STRUCT, action);
	}
	return R_RET;

is_arg_error:
	Trap_Types(RE_EXPECT_VAL, REB_STRUCT, VAL_TYPE(arg));
}