/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2022 Rebol Open Source Developers
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
**  Module:  u-parse.c
**  Summary: parse dialect interpreter
**  Section: utility
**  Author:  Carl Sassenrath, Oldes
**  Notes:
**
***********************************************************************/

#include "sys-core.h"

// Parser flags:
enum Parse_Flags {
	PF_ALL = 1,
	PF_CASE = 2,
	PF_CASED = 4, // was set as initial option
};

#define BLOCK_COLLECT 1
typedef struct reb_parse_collect {
	REBVAL *value;
	REBSER *block;
	REBCNT  mode;  // BLOCK_COLLECT, SYM_SET, SYM_INTO and SYM_AFTER
	REBINT  depth; // needed to detect error in: parse [1] [collect integer! keep (1)]
} REB_PARSE_COLLECT;

typedef struct reb_parse {
	REBSER *series;
	REBCNT type;
	REBCNT flags;
	REBINT result;
//	REBVAL *retval;
	REB_PARSE_COLLECT *collect;
} REBPARSE;

enum parse_flags {
	PF_SET_OR_COPY, // test PF_COPY first; if false, this means PF_SET 
	PF_COPY,
	PF_NOT,
	PF_NOT2,
	PF_THEN,
	PF_AND,
	PF_REMOVE,
	PF_INSERT,
	PF_CHANGE,
	PF_RETURN,
	PF_WHILE,
	PF_ADVANCE, // used to report that although index was not changed, rule is suppose to advance
	PF_COLLECT,
	PF_KEEP,
	PF_PICK,
};

#define MAX_PARSE_DEPTH 512

// Returns SYMBOL or 0 if not a command:
#define GET_CMD(n) (((n) >= SYM_OR_BAR && (n) <= SYM_END) ? (n) : 0)
#define VAL_CMD(v) GET_CMD(VAL_WORD_CANON(v))
#define HAS_CASE(p) (p->flags & AM_FIND_CASE)
#define IS_OR_BAR(v) (IS_WORD(v) && VAL_WORD_CANON(v) == SYM_OR_BAR)
#define SKIP_TO_BAR(r) while (NOT_END(r) && !IS_SAME_WORD(r, SYM_OR_BAR)) r++;
#define IS_BLOCK_INPUT(p) (p->type >= REB_BLOCK)

static REBCNT Parse_Rules_Loop(REBPARSE *parse, REBCNT index, REBVAL *rules, REBCNT depth);

void Print_Parse_Index(REBCNT type, REBVAL *rules, REBSER *series, REBCNT index)
{
	REBVAL val;
	Set_Series(type, &val, series);
	VAL_INDEX(&val) = index;
	//TODO: use console line width?
	Debug_Fmt_Limited(76, cb_cast("%r: %r"), rules, &val);
}


/***********************************************************************
**
*/	static REBCNT Parse_Series(REBVAL *val, REBVAL *rules, REBCNT flags, REBCNT depth, REB_PARSE_COLLECT *collect)
/*
***********************************************************************/
{
	REBPARSE parse;

	parse.series = VAL_SERIES(val);
	parse.type = VAL_TYPE(val);
	parse.flags = flags;
	parse.result = 0;
	//parse.retval = NULL;
	parse.collect = collect;

	return Parse_Rules_Loop(&parse, VAL_INDEX(val), rules, depth);
}


/***********************************************************************
**
*/	static REBCNT Set_Parse_Series(REBPARSE *parse, REBVAL *item)
/*
**		Change the series and return the new index.
**
***********************************************************************/
{
	parse->series = VAL_SERIES(item);
	parse->type = VAL_TYPE(item);
	if (IS_BINARY(item) || (parse->flags & PF_CASED)) parse->flags |= PF_CASE;
	else parse->flags &= ~PF_CASE;
	return (VAL_INDEX(item) > VAL_TAIL(item)) ? VAL_TAIL(item) : VAL_INDEX(item);
}


/***********************************************************************
**
*/	static REBVAL *Get_Parse_Value(REBVAL *item)
/*
**		Get the value of a word (when not a command) or path.
**		Returns all other values as-is.
**
***********************************************************************/
{
	if (IS_WORD(item)) {
		if (!VAL_CMD(item)) item = Get_Var(item);
	}
	else if (IS_PATH(item)) {
		REBVAL *path = item;
		if (Do_Path(&path, 0)) return item; // found a function
		item = DS_TOP;
	}
	return item;
}


/***********************************************************************
**
*/	static REBVAL *Do_Parse_Path(REBVAL *item, REBPARSE *parse, REBCNT *index)
/*
**		Handle a PATH, including get and set, that's found in a rule.
**
***********************************************************************/
{
	REBVAL *path = item;
	REBVAL tmp;

	if (IS_PATH(item)) {
		if (Do_Path(&path, 0)) return item; // found a function
		item = DS_TOP;
	}
	else if (IS_SET_PATH(item)) {
		Set_Series(parse->type, &tmp, parse->series);
		VAL_INDEX(&tmp) = *index;
		if (Do_Path(&path, &tmp)) return item; // found a function
		return 0;
	}
	else if (IS_GET_PATH(item)) {
		if (Do_Path(&path, 0)) return item; // found a function
		item = DS_TOP;
		// CureCode #1263 change
		//		if (parse->type != VAL_TYPE(item) || VAL_SERIES(item) != parse->series) 
		if (!ANY_SERIES(item)) Trap1(RE_PARSE_SERIES, path);
		*index = Set_Parse_Series(parse, item);
		return 0;
	}

	return item;
}


/***********************************************************************
**
*/	static REBCNT Parse_Next_String(REBPARSE *parse, REBCNT index, REBVAL *item, REBCNT depth)
/*
**		Match the next item in the string ruleset.
**
**		If it matches, return the index just past it.
**		Otherwise return NOT_FOUND.
**
***********************************************************************/
{
	// !!! THIS CODE NEEDS CLEANUP AND REWRITE BASED ON OTHER CHANGES
	REBSER *series = parse->series;
	REBCNT flags = parse->flags | AM_FIND_MATCH | AM_FIND_TAIL;
	REBUNI ch1, ch2;
//	int rewrite_needed;

	if (Trace_Level) {
		Trace_Value(7, item);
		Trace_String(8, STR_SKIP(series, index), series->tail - index);
	}

	if (IS_NONE(item)) return index;

	if (index >= series->tail) return NOT_FOUND;

	switch (VAL_TYPE(item)) {

	// Do we match a single character?
	case REB_CHAR:
		if (HAS_CASE(parse))
			index = (VAL_CHAR(item) == GET_ANY_CHAR(series, index)) ? index+1 : NOT_FOUND;
		else {
			ch1 = VAL_CHAR(item);
			ch2 = GET_ANY_CHAR(series, index);
			if (ch1 < UNICODE_CASES) ch1 = UP_CASE(ch1);
			if (ch2 < UNICODE_CASES) ch2 = UP_CASE(ch2);
			index = (ch1 == ch2) ? index + 1 : NOT_FOUND;
		}
		break;

	case REB_STRING:
	case REB_BINARY: 
	case REB_FILE:
	case REB_URL:
	case REB_EMAIL:
	case REB_REF:
		index = Find_Str_Str(series, 0, index, SERIES_TAIL(series), 1, VAL_SERIES(item), VAL_INDEX(item), VAL_LEN(item), flags);
		break;

	// Do we match to a char set?
	case REB_BITSET:
		flags = Check_Bit(VAL_SERIES(item), GET_ANY_CHAR(series, index), !HAS_CASE(parse));
		index = flags ? index + 1 : NOT_FOUND;
		break;
/*
	case REB_DATATYPE:	// Currently: integer!
		if (VAL_DATATYPE(item) == REB_INTEGER) {
			REBCNT begin = index;
			while (IS_LEX_NUMBER(*str)) str++, index++;
			if (begin == index) index = NOT_FOUND;
		}
		break;
*/
	case REB_TAG:
		index = Find_Str_Tag(series, 0, index, SERIES_TAIL(series), 1, VAL_SERIES(item), VAL_INDEX(item), VAL_LEN(item), flags);
		break;

	case REB_NONE:
		break;

	// Parse a sub-rule block:
	case REB_BLOCK:
		index = Parse_Rules_Loop(parse, index, VAL_BLK_DATA(item), depth);
		break;

	// Do an expression:
	case REB_PAREN:
		item = Do_Block_Value_Throw(item); // might GC
        UNUSED(item);
		// old: if (IS_ERROR(item)) Throw_Error(VAL_ERR_OBJECT(item));
        index = MIN(index, series->tail); // may affect tail
		break;

	default:
		Trap1(RE_PARSE_RULE, item);
	}

	return index;
}


/***********************************************************************
**
*/	static REBCNT Parse_Next_Block(REBPARSE *parse, REBCNT index, REBVAL *item, REBCNT depth)
/*
**		Used for parsing blocks to match the next item in the ruleset.
**		If it matches, return the index just past it. Otherwise, return zero.
**
***********************************************************************/
{
	// !!! THIS CODE NEEDS CLEANUP AND REWRITE BASED ON OTHER CHANGES
	REBSER *series = parse->series;
	REBVAL *blk = BLK_SKIP(series, index);

	if (Trace_Level) {
		Trace_Value(7, item);
		Trace_Value(8, blk);
	}

	switch (VAL_TYPE(item)) {

	// Look for specific datattype:
	case REB_DATATYPE:
		index++;
		if (VAL_TYPE(blk) == (REBYTE)VAL_DATATYPE(item)) break;
		goto no_result;

	// Look for a set of datatypes:
	case REB_TYPESET:
		index++;
		if (TYPE_CHECK(item, VAL_TYPE(blk))) break;
		goto no_result;

	// 'word
	case REB_LIT_WORD:
		index++;
		if (IS_WORD(blk) && !Compare_Word(blk, item, HAS_CASE(parse))) break;
		goto no_result;

	case REB_LIT_PATH:
		index++;
		if (IS_PATH(blk) && !Cmp_Block(blk, item, HAS_CASE(parse))) break;
		goto no_result;

	case REB_NONE:
		break;

	// Parse a sub-rule block:
	case REB_BLOCK:
		index = Parse_Rules_Loop(parse, index, VAL_BLK_DATA(item), depth);
		break;

	// Do an expression:
	case REB_PAREN:
		item = Do_Block_Value_Throw(item); // might GC
        UNUSED(item);
        // old: if (IS_ERROR(item)) Throw_Error(VAL_ERR_OBJECT(item));
        index = MIN(index, series->tail); // may affect tail
		break;

	// Match with some other value:
	default:
		index++;
		if (Cmp_Value(blk, item, (REBOOL)HAS_CASE(parse))) goto no_result;
	}

	return index;

no_result:
	return NOT_FOUND;
}


/***********************************************************************
**
*/	static REBCNT To_Thru(REBPARSE *parse, REBCNT index, REBVAL *block, REBFLG is_thru)
/*
***********************************************************************/
{
	REBSER *series = parse->series;
	REBCNT type = parse->type;
	REBVAL *blk;
	REBVAL *item;
	REBCNT cmd;
	REBCNT i;
	REBCNT len;

	for (; index <= series->tail; index++) {

		for (blk = VAL_BLK(block); NOT_END(blk); blk++) {

			item = blk;

			// Deal with words and commands
			if (IS_WORD(item)) {
				if ((cmd = VAL_CMD(item))) {
					if (cmd == SYM_END) {
						if (index >= series->tail) {
							index = series->tail;
							goto found;
						}
						goto next;
					}
					else if (cmd == SYM_QUOTE) {
						item = ++blk; // next item is the quoted value
						if (IS_END(item)) goto bad_target;
						if (IS_PAREN(item)) {
							item = Do_Block_Value_Throw(item); // might GC
						}

					}
					else goto bad_target;
				}
				else {
					item = Get_Var(item);
				}
			}
			else if (IS_PATH(item)) {
				item = Get_Parse_Value(item);
			}

			// Try to match it:
			if (type >= REB_BLOCK) {
				if (ANY_BLOCK(item)) goto bad_target;
				i = Parse_Next_Block(parse, index, item, 0);
				if (i != NOT_FOUND) {
					if (!is_thru) i--;
					index = i;
					goto found;
				}
			}
			else if (type == REB_BINARY) {
				REBYTE ch1 = *BIN_SKIP(series, index);

				// Handle special string types:
				if (IS_CHAR(item)) {
					if (VAL_CHAR(item) > 0xff) goto bad_target;
					if (ch1 == VAL_CHAR(item)) goto found1;
				}
				else if (IS_BINARY(item)) {
					if (ch1 == *VAL_BIN_DATA(item)) {
						len = VAL_LEN(item);
						if (len == 1) goto found1;
						if (0 == Compare_Bytes(BIN_SKIP(series, index), VAL_BIN_DATA(item), len, 0)) {
							if (is_thru) index += len;
							goto found;
						}
					}
				}
				else if (IS_INTEGER(item)) {
					if (VAL_INT64(item) > 0xff) goto bad_target;
					if (ch1 == VAL_INT32(item)) goto found1;
				}
				else goto bad_target;
			}
			else { // String
				REBUNI ch1 = GET_ANY_CHAR(series, index);
				REBUNI ch2;

				if (!HAS_CASE(parse) && ch1 < UNICODE_CASES) ch1 = UP_CASE(ch1);

				// Handle special string types:
				if (IS_CHAR(item)) {
					ch2 = VAL_CHAR(item);
					if (!HAS_CASE(parse) && ch2 < UNICODE_CASES) ch2 = UP_CASE(ch2);
					if (ch1 == ch2) goto found1;
				}
				else if (IS_TAG(item)) {
					if (ch1 == '<') {
						i = Find_Str_Tag(series, 0, index, series->tail, 1, VAL_SERIES(item), VAL_INDEX(item), VAL_LEN(item),  AM_FIND_MATCH | parse->flags);
						if (i != NOT_FOUND) {
							if (is_thru) i += VAL_LEN(item) + 2;
							index = i;
							goto found;
						}
					}
				}
				// bitset
				else if (IS_BITSET(item)) {
					if (Check_Bit(VAL_SERIES(item), ch1, !HAS_CASE(parse)))
						goto found1;
				}
				else if (ANY_STR(item)) {
					ch2 = VAL_ANY_CHAR(item);
					if (!HAS_CASE(parse) && ch2 < UNICODE_CASES) ch2 = UP_CASE(ch2);
					if (ch1 == ch2) {
						len = VAL_LEN(item);
						if (len == 1) goto found1;
						i = Find_Str_Str(series, 0, index, SERIES_TAIL(series), 1, VAL_SERIES(item), VAL_INDEX(item), len, AM_FIND_MATCH | parse->flags);
						if (i != NOT_FOUND) {
							if (is_thru) i += len;
							index = i;
							goto found;
						}
					}
				}
				else if (IS_INTEGER(item)) {
					ch1 = GET_ANY_CHAR(series, index);  // No casing!
					if (ch1 == (REBCNT)VAL_INT32(item)) goto found1;
				}
				else goto bad_target;
			}

next:		// Check for | (required if not end)
			blk++;
			if (IS_PAREN(blk)) blk++;
			if (IS_END(blk)) break;
			if (!IS_OR_BAR(blk)) {
				item = blk;
				goto bad_target;
			}
		}
	}
	return NOT_FOUND;

found:
	if (IS_PAREN(blk+1)) Do_Block_Value_Throw(blk+1);
	return index;

found1:
	if (IS_PAREN(blk+1)) Do_Block_Value_Throw(blk+1);
	return index + (is_thru ? 1 : 0);

bad_target:
	Trap1(RE_PARSE_RULE, item);
	return 0;
}


/***********************************************************************
**
*/	static REBCNT Parse_To(REBPARSE *parse, REBCNT index, REBVAL *item, REBFLG is_thru)
/*
**		Parse TO a specific:
**			1. integer - index position
**			2. END - end of input
**			3. value - according to datatype
**			4. block of values - the first one we hit
**
***********************************************************************/
{
	REBSER *series = parse->series;
	REBCNT i = 0;

	// TO a specific index position.
	if (IS_INTEGER(item)) {
		i = (REBCNT)Int32(item) - (is_thru ? 0 : 1);
		if (i > series->tail) i = series->tail;
	}
	// END
	else if (IS_WORD(item) && VAL_WORD_CANON(item) == SYM_END) {
		i = series->tail;
	}
	else if (IS_BLOCK(item)) {
		i = To_Thru(parse, index, item, is_thru);
	}
	else {
		if (IS_BLOCK_INPUT(parse)) {
			REBVAL word; /// !!!Temp, but where can we put it?
			if (IS_LIT_WORD(item)) {  // patch to search for word, not lit.
				word = *item;
				VAL_SET(&word, REB_WORD);
				item = &word;
			}
			///i = Find_Value(series, index, tail-index, item, 1, (REBOOL)(PF_CASE & flags), FALSE, 1);
			i = Find_Block(series, index, series->tail, item, 1, HAS_CASE(parse)?AM_FIND_CASE:0, 1);
			if (i != NOT_FOUND && is_thru) i++;
		}
		else {
			// "str"
			//O: not using ANY_BINSTR as TAG is now handled separately
			if (VAL_TYPE(item) >= REB_BINARY && VAL_TYPE(item) < REB_TAG) {
				i = Find_Str_Str(series, 0, index, series->tail, 1, VAL_SERIES(item), VAL_INDEX(item), VAL_LEN(item), HAS_CASE(parse));
				if (i != NOT_FOUND && is_thru) i += VAL_LEN(item);
			}
			// #"A"
			else if (IS_CHAR(item)) {
				i = Find_Str_Char(series, 0, index, series->tail, 1, VAL_CHAR(item), HAS_CASE(parse));
				if (i != NOT_FOUND && is_thru) i++;
			}
			// bitset
			else if (IS_BITSET(item)) {
				i = Find_Str_Bitset(series, 0, index, series->tail, 1, VAL_BITSET(item), HAS_CASE(parse));
				if (i != NOT_FOUND && is_thru) i++;
			}
			else if (IS_TAG(item)) {
				i = Find_Str_Tag(series, 0, index, series->tail, 1, VAL_SERIES(item), VAL_INDEX(item), VAL_LEN(item), HAS_CASE(parse));
				if (i != NOT_FOUND && is_thru) i += VAL_LEN(item) + 2;
			}
			else
				Trap1(RE_PARSE_RULE, item - 1);
		}
	}

	return i;
}

#ifdef USE_DO_PARSE_RULE
//@@ https://github.com/Oldes/Rebol-issues/issues/2083
/***********************************************************************
**
*/	static REBCNT Do_Eval_Rule(REBPARSE *parse, REBCNT index, REBVAL **rule)
/*
**		Evaluate the input as a code block. Advance input if
**		rule succeeds. Return new index or failure.
**
**		Examples:
**			do skip
**			do end
**			do "abc"
**			do 'abc
**			do [...]
**			do variable
**			do datatype!
**			do quote 123
**			do into [...]
**
**		Problem: cannot write:  set var do datatype!
**
***********************************************************************/
{
	REBVAL value;
	REBVAL *item = *rule;
	REBCNT n;
	REBPARSE newparse;

	// First, check for end of input:
	if (index >= parse->series->tail) {
		if (IS_WORD(item) && VAL_CMD(item) == SYM_END) return index;
		else return NOT_FOUND;
	}

	// Evaluate next N input values:
	index = Do_Next(parse->series, index, FALSE);

	// Value is on top of stack (volatile!):
	value = *DS_POP;
	if (THROWN(&value)) Throw_Break(&value);

	// Get variable or command:
	if (IS_WORD(item)) {

		n = VAL_CMD(item);

		if (n == SYM_SKIP)
			return (IS_SET(&value)) ? index : NOT_FOUND;

		if (n == SYM_QUOTE) {
			item = item + 1;
			(*rule)++;
			if (IS_END(item)) Trap1(RE_PARSE_END, item-2);
			if (IS_PAREN(item)) {
				item = Do_Block_Value_Throw(item); // might GC
			}
		}
		else if (n == SYM_INTO) {
			item = item + 1;
			(*rule)++;
			if (IS_END(item)) Trap1(RE_PARSE_END, item-2);
			item = Get_Parse_Value(item); // sub-rules
			if (!IS_BLOCK(item)) Trap1(RE_PARSE_RULE, item-2);
			if (!ANY_BINSTR(&value) && !ANY_BLOCK(&value)) return NOT_FOUND;
			return (Parse_Series(&value, VAL_BLK_DATA(item), parse->flags, 0) == VAL_TAIL(&value))
				? index : NOT_FOUND;
		}
		else if (n > 0)
			Trap1(RE_PARSE_RULE, item);
		else	
			item = Get_Parse_Value(item); // variable
	}
	else if (IS_PATH(item)) {
		item = Get_Parse_Value(item); // variable
	}
	else if (IS_SET_WORD(item) || IS_GET_WORD(item) || IS_SET_PATH(item) || IS_GET_PATH(item))
		Trap1(RE_PARSE_RULE, item);

	if (IS_NONE(item)) {
		return (VAL_TYPE(&value) > REB_NONE) ? NOT_FOUND : index;
	}

	// Copy the value into its own block:
	newparse.series = Make_Block(1);
	SAVE_SERIES(newparse.series);
	Append_Val(newparse.series, &value);
	newparse.type = REB_BLOCK;
	newparse.flags = parse->flags;
	newparse.result = 0;

	n = (Parse_Next_Block(&newparse, 0, item, 0) != NOT_FOUND) ? index : NOT_FOUND;
	UNSAVE_SERIES(newparse.series);
	return n;
}
#endif // USE_DO_PARSE_RULE

/***********************************************************************
**
*/	static void Parse_Collect_End(REB_PARSE_COLLECT *collect)
/*
***********************************************************************/
{
	// COLLECT ends
	// get the previous target block from the stack and use it
	//printf("collect ends %u dsp: %i blk: %x\n", collect->depth, DSP, collect->block);
	collect->mode = VAL_INT32(DS_POP);
	if (collect->mode == SYM_INTO) {
		VAL_INDEX(collect->value) = VAL_INT32(DS_POP);
	}
	collect->value = DS_POP;
	collect->block = VAL_SERIES(collect->value);
	collect->depth--;
}

/***********************************************************************
**
*/	static void Parse_Keep(REBPARSE *parse, REBSER *series, REBCNT begin, REBCNT count, REBOOL pick)
/*
***********************************************************************/
{
	REBVAL *val;
	REBINT i, e;
	REBSER *ser;
	REBSER *block = parse->collect->block; // Parse_Collect_Block(parse);
	REBCNT index;

	if (parse->collect->depth == 0)
		Trap0(RE_PARSE_NO_COLLECT);

	ASSERT2(block, RP_MISC); // should never happen

	//printf("Keep from %i count: %i to: %x\n", begin, count, block);

	if (parse->collect->mode == BLOCK_COLLECT || parse->collect->mode == SYM_SET) {
		// Collects into a new block (noname or named (set)).
		// Always appends to tail, so may use faster code...
		if (count > 1) {
			if (IS_BLOCK_INPUT(parse)) {
				if (pick) {
					Insert_Series(block, AT_TAIL, SERIES_SKIP(series, begin), count);
				}
				else {
					val = Append_Value(block);
					Set_Block(val, Copy_Block_Len(series, begin, count));
				}
			}
			else {
				if (pick) {
					e = begin + count;
					if (parse->type == REB_BINARY) {
						for (i = begin; i < e; i++) {
							val = Append_Value(block);
							SET_INTEGER(val, BIN_HEAD(series)[i]);
						}
					}
					else {
						for (i = begin; i < e; i++) {
							val = Append_Value(block);
							SET_CHAR(val, GET_ANY_CHAR(series, i));
						}
					}
				}
				else {
					val = Append_Value(block);
					VAL_SERIES(val) = Copy_String(series, begin, count);
					VAL_INDEX(val) = 0;
					VAL_SET(val, parse->type);
				}
			}
		}
		else if (count == 1) {
			val = Append_Value(block);
			if (IS_BLOCK_INPUT(parse)) {
				*val = *BLK_SKIP(series, begin);
			}
			else if (parse->type == REB_BINARY) {
				SET_INTEGER(val, BIN_HEAD(series)[begin]);
			}
			else {
				SET_CHAR(val, GET_ANY_CHAR(series, begin));
			}
		}
	}
	else {
		// `collect into` and `collect after` keeps at any index, 
		// so we must take care of it!
		index = VAL_INDEX(parse->collect->value);
		//printf("series %x index: %u\n", parse->collect->value, index);
		if (count > 1) {
			if (IS_BLOCK_INPUT(parse)) {
				if (pick) {
					Insert_Series(block, index, SERIES_SKIP(series, begin), count);
				}
				else {
					ser = Copy_Block_Len(series, begin, count);
					Expand_Series(block, index, 1);
					val = BLK_SKIP(block, index);
					VAL_SERIES(val) = ser;
					VAL_INDEX(val) = 0;
					VAL_SET(val, parse->type);
				}
			}
			else {
				Expand_Series(block, index, count);
				if (ANY_BLOCK(parse->collect->value)) {
					if (pick) {
						e = begin + count;
						if (parse->type == REB_BINARY) {
							for (i = begin; i < e; i++) {
								val = Append_Value(block);
								SET_INTEGER(val, BIN_HEAD(series)[i]);
							}
						}
						else {
							for (i = begin; i < e; i++) {
								val = Append_Value(block);
								SET_CHAR(val, GET_ANY_CHAR(series, i));
							}
						}
					}
					else {
						val = BLK_SKIP(block, index);
						VAL_SERIES(val) = Copy_String(series, begin, count);
						VAL_INDEX(val) = 0;
						VAL_SET(val, parse->type);
					}
				}
				else {
					// string like parse input into string value
					Insert_String(block, index, series, begin, count, TRUE);
					VAL_INDEX(parse->collect->value) += count;
				}
			}
		}
		else if (count == 1) {
			Expand_Series(block, index, 1);
			if (ANY_BLOCK(parse->collect->value)) {
				// string like parse intput into block value
				
				val = BLK_SKIP(block, index);

				if (IS_BLOCK_INPUT(parse)) {
					*val = *BLK_SKIP(series, begin);
				}
				else if (parse->type == REB_BINARY) {
					SET_INTEGER(val, BIN_HEAD(series)[begin]);
				}
				else {
					SET_CHAR(val, GET_ANY_CHAR(series, begin));
				}
			}
			else {
				// string like parse input into string value
				Insert_String(block, index, series, begin, 1, TRUE);
			}
			VAL_INDEX(parse->collect->value)++;
		}
	}
}

/***********************************************************************
**
*/	static void Parse_Keep_Expression(REBPARSE *parse, REBVAL *expr)
/*
***********************************************************************/
{
	REBVAL *value;
	REBSER *block;
	REBVAL *item;
	REBINT index;

	if (parse->collect->depth == 0)
		Trap0(RE_PARSE_NO_COLLECT);

	block = parse->collect->block;
	ASSERT2(block, RP_MISC); // should never happen

	item = Do_Block_Value_Throw(expr); // might GC
	value = parse->collect->value;
	if (ANY_BLOCK(value)) {
		Append_Val(block, item);
	}
	else {
		index = VAL_INDEX(value);
		index = Modify_String(A_INSERT, VAL_SERIES(value), index, item, 0, 1, 1);
		VAL_INDEX(value) = index;
	}
}


/***********************************************************************
**
*/	static REBCNT Parse_Rules_Loop(REBPARSE *parse, REBCNT index, REBVAL *rules, REBCNT depth)
/*
***********************************************************************/
{
	REBSER *series = parse->series;
	REBVAL *item;		// current rule item
	REBVAL *word;		// active word to be set
	REBCNT start;		// recovery restart point
	REBCNT i;			// temp index point
	REBCNT begin;		// point at beginning of match
	REBINT count;		// iterated pattern counter
	REBINT mincount;	// min pattern count
	REBINT maxcount;	// max pattern count
	REBVAL *item_hold;
	REBVAL *val;		// spare
	REBCNT rulen;
	REBSER *ser;
	REBFLG flags;
	REBCNT cmd, wrd;
	REB_PARSE_COLLECT *collect = parse->collect;
	//REBVAL *rule_head = rules;

	CHECK_STACK(&flags);
	//if (depth > MAX_PARSE_DEPTH) Trap_Word(RE_LIMIT_HIT, SYM_PARSE, 0);
	flags = 0;
	word = 0;
	mincount = maxcount = 1;
	start = begin = index;

	// For each rule in the rule block:
	while (NOT_END(rules)) {

		//Print_Parse_Index(parse->type, rules, series, index);

		if (--Eval_Count <= 0 || Eval_Signals) Do_Signals();

		//--------------------------------------------------------------------
		// Pre-Rule Processing Section
		//
		// For non-iterated rules, including setup for iterated rules.
		// The input index is not advanced here, but may be changed by
		// a GET-WORD variable.
		//--------------------------------------------------------------------

		item = rules++;

		// If word, set-word, or get-word, process it:
		if (VAL_TYPE(item) >= REB_WORD && VAL_TYPE(item) <= REB_GET_WORD) {

			// Is it a command word?
			if ((cmd = VAL_CMD(item))) {

				if (!IS_WORD(item)) Trap1(RE_PARSE_COMMAND, item); // SET or GET not allowed

				if (cmd <= SYM_BREAK) { // optimization

					switch (cmd) {

					case SYM_OR_BAR:
						return index;	// reached it successfully

					// Note: mincount = maxcount = 1 on entry
					case SYM_WHILE:
						SET_FLAG(flags, PF_WHILE);
					case SYM_ANY:
						mincount = 0;
					case SYM_SOME:
						maxcount = MAX_I32;
						continue;

					case SYM_OPT:
						mincount = 0;
						continue;

					case SYM_COPY:
						SET_FLAG(flags, PF_COPY);
					case SYM_SET:
						SET_FLAG(flags, PF_SET_OR_COPY);
						item = rules++;
						if (!(IS_WORD(item) || IS_SET_WORD(item))) Trap1(RE_PARSE_VARIABLE, item);
						if (VAL_CMD(item)) Trap1(RE_PARSE_COMMAND, item);
						word = item;
						continue;

					case SYM_NOT:
						SET_FLAG(flags, PF_NOT);
						flags ^= (1<<PF_NOT2);
						continue;
	
					case SYM_AND:
					case SYM_AHEAD:
						SET_FLAG(flags, PF_AND);
						continue;

					case SYM_THEN:
						SET_FLAG(flags, PF_THEN);
						continue;

					case SYM_REMOVE:
						SET_FLAG(flags, PF_REMOVE);
						if (IS_WORD(rules)) {
							// optionally using stored index as a counter
							// https://github.com/Oldes/Rebol-issues/issues/2541
							item = Get_Var(rules);
							if (VAL_SERIES(item) == parse->series) {
								rules++;
								if (VAL_INDEX(item) < index) {
									begin = VAL_INDEX(item);
									count = index - VAL_INDEX(item);
								}
								else {
									begin = index;
									count = VAL_INDEX(item) - index;
								}
								goto do_remove;
							}
						}
						continue;
					
					case SYM_INSERT:
						SET_FLAG(flags, PF_INSERT);
						goto post;
					
					case SYM_CHANGE:
						SET_FLAG(flags, PF_CHANGE);
						continue;

					case SYM_COLLECT:
						if (IS_END(rules))
							Trap1(RE_PARSE_END, rules - 1);
						//printf("COLLECT start %i\n", collect->depth);
						SET_FLAG(flags, PF_COLLECT);
						wrd = IS_WORD(rules) ? VAL_SYM_CANON(rules) : 0;
						if (wrd == SYM_SET) {
							// COLLECT INTO A NEW VAR
							rules++;
							
							if (!(IS_WORD(rules) || IS_SET_WORD(rules)))
								Trap1(RE_PARSE_VARIABLE, rules);

							collect->block = Make_Block(2);
							Set_Var_Series(rules, REB_BLOCK, collect->block, 0);
						
							val = Get_Var(rules);
							collect->value = val;
							collect->mode = SYM_SET;
							DS_PUSH(val);
							DS_PUSH_INTEGER(SYM_SET); // store mode
							rules++;
						}
						else if (wrd == SYM_INTO || wrd == SYM_AFTER) {
							// COLLECT INTO EXISTING VAR
							rules++;

							if (!(IS_WORD(rules) || IS_GET_WORD(rules)))
								Trap1(RE_PARSE_VARIABLE, rules);

							val = Get_Var(rules);
							
							if (!(
								ANY_BLOCK(val) ||
								(ANY_STR(val) && ANY_STR_TYPE(parse->type)) ||
								(IS_BINARY(val) && parse->type == REB_BINARY)
								)
							) {
								Trap0(RE_PARSE_INTO_TYPE);
							}

							collect->value = val;
							collect->block = VAL_SERIES(val);
							collect->mode = wrd;
							DS_PUSH(val);
							if (wrd == SYM_INTO)
								DS_PUSH_INTEGER(VAL_INDEX(val)); // store current index (will be restored)
							DS_PUSH_INTEGER(wrd); // store mode (into or after)
							rules++;
						}
						else {
							// NON-WORD COLLECT
							// reserve a new value on stack
							DS_PUSH_NONE;
							collect->value = DS_TOP;
							if (collect->block == NULL) { //don't use collect->result! (first collect may fail)
								// --- FIRST collect -------------------------
								// allocate the resulting block on the stack, so it is GC safe
								Set_Series(REB_BLOCK, DS_TOP, Make_Block(2));
								//collect->result = DS_TOP;
								collect->block = VAL_SERIES(DS_TOP);
							}
							else {
								// --- SUBSEQUENT collect ---------------------
								// store current block on stack
								Set_Series(REB_BLOCK, DS_TOP, collect->block);
								// make a new block on its tail for the new collections
								val = Append_Value(collect->block);
								collect->block = Make_Block(2);
								Set_Series(REB_BLOCK, val, collect->block);
							}
							collect->mode = BLOCK_COLLECT;
							DS_PUSH_INTEGER(BLOCK_COLLECT); // no special mode (block collect)
						}
						collect->depth++;
						//printf("collect started %u dsp: %i blk: %x\n", collect->depth, DSP, collect->block);
						continue;

					case SYM_KEEP:
						if (IS_END(rules)) {
							Trap1(RE_PARSE_END, rules - 1);
						}
						if (IS_WORD(rules) && VAL_SYM_CANON(rules) == SYM_PICK) {
							SET_FLAG(flags, PF_PICK);
							rules++;
							if (IS_END(rules))
								Trap1(RE_PARSE_END, rules - 2);
						}
						if (IS_PAREN(rules)) {
							Parse_Keep_Expression(parse, rules);
							rules++;
							continue;
						}
						SET_FLAG(flags, PF_KEEP);

						continue;

					case SYM_RETURN:
						if (IS_PAREN(rules)) {
							item = Do_Block_Value_Throw(rules); // might GC
							Throw_Return_Value(item);
						}
						SET_FLAG(flags, PF_RETURN);
						continue;

					case SYM_ACCEPT:
					case SYM_BREAK:
						parse->result = 1;
						return index;

					case SYM_REJECT:
						parse->result = 0;
						return NOT_FOUND;

					case SYM_FAIL:
						index = NOT_FOUND;
						goto post;

					case SYM_IF:
						item = rules++;
						if (IS_END(item)) goto bad_end;
						if (!IS_PAREN(item)) Trap1(RE_PARSE_RULE, item);
						item = Do_Block_Value_Throw(item); // might GC
						if (IS_TRUE(item)) continue;
						else {
							index = NOT_FOUND;
							goto post;
						}

					case SYM_LIMIT:
						Trap0(RE_NOT_DONE);
						//val = Get_Parse_Value(rules++);
					//	if (IS_INTEGER(val)) limit = index + Int32(val);
					//	else if (ANY_SERIES(val)) limit = VAL_INDEX(val);
					//	else goto
						//goto bad_rule;
					//	goto post;

					case SYM_QQ:
						Print_Parse_Index(parse->type, rules, series, index);
						continue;

					case SYM_CASE:
						parse->flags |= AM_FIND_CASE;
						continue;
					case SYM_NO_CASE:
						parse->flags &= ~AM_FIND_CASE;
						continue;
					}
				}
				// Any other cmd must be a match command, so proceed...

			} else { // It's not a PARSE command, get or set it:
			
				// word: - set a variable to the series at current index
				if (IS_SET_WORD(item)) {
					Set_Var_Series(item, parse->type, series, index);
					continue;
				}

				// :word - change the index for the series to a new position
				if (IS_GET_WORD(item)) {
					item = Get_Var(item);
					// CureCode #1263 change
					//if (parse->type != VAL_TYPE(item) || VAL_SERIES(item) != series)
					//	Trap1(RE_PARSE_SERIES, rules-1);
					if (!ANY_SERIES(item)) Trap1(RE_PARSE_SERIES, rules-1);
					index = Set_Parse_Series(parse, item);
					series = parse->series;

					// #2269 - reset the position if we are not in the middle of any rule
					// don't allow code like: [copy x :pos integer!]
					if (flags != 0) Trap1(RE_PARSE_RULE, rules-1); 
					begin = index;
					SET_FLAG(parse->flags, PF_ADVANCE);
					continue;
				}

				// word - some other variable
				if (IS_WORD(item)) {
					item = Get_Var(item);
				}

				// item can still be 'word or /word
			}
		}
		else if (ANY_PATH(item)) {
			item = Do_Parse_Path(item, parse, &index); // index can be modified
			if (index > series->tail) index = series->tail;
			if (item == 0) continue; // for SET and GET cases
		}

		if (IS_PAREN(item)) {
			Do_Block_Value_Throw(item); // might GC
			if (index > series->tail) index = series->tail;
			continue;
		}

		// Counter? 123
		if (IS_INTEGER(item)) {	// Specify count or range count
			SET_FLAG(flags, PF_WHILE);
			mincount = maxcount = Int32s(item, 0);
			item = Get_Parse_Value(rules++);
			if (IS_END(item)) Trap1(RE_PARSE_END, rules-2);
			if (IS_INTEGER(item)) {
				maxcount = Int32s(item, 0);
				item = Get_Parse_Value(rules++);
				if (IS_END(item)) Trap1(RE_PARSE_END, rules-2);
			}
		}
		// else fall through on other values and words

		//--------------------------------------------------------------------
		// Iterated Rule Matching Section:
		//
		// Repeats the same rule N times or until the rule fails.
		// The index is advanced and stored in a temp variable i until
		// the entire rule has been satisfied.
		//--------------------------------------------------------------------

		item_hold = item;	// a command or literal match value
		if (VAL_TYPE(item) <= REB_UNSET || VAL_TYPE(item) >= REB_NATIVE) goto bad_rule;
		begin = index;		// input at beginning of match section
		rulen = 0;			// rules consumed (do not use rule++ below)
		i = index;

		//note: rules var already advanced

		for (count = 0; count < maxcount;) {

			item = item_hold;

			if (IS_WORD(item)) {

				switch (cmd = VAL_WORD_CANON(item)) {

				case SYM_SKIP:
					i = (index < series->tail) ? index+1 : NOT_FOUND;
					break;

				case SYM_END:
					i = (index < series->tail) ? NOT_FOUND : series->tail;
					break;

				case SYM_TO:
				case SYM_THRU:
					if (IS_END(rules)) goto bad_end;
					item = Get_Parse_Value(rules);
					rulen = 1;
					i = Parse_To(parse, index, item, cmd == SYM_THRU);
					break;
					
				case SYM_QUOTE:
					if (IS_END(rules)) goto bad_end;
					rulen = 1;
					if (IS_PAREN(rules)) {
						item = Do_Block_Value_Throw(rules); // might GC
					}
					else item = rules;
					i = (0 == Cmp_Value(BLK_SKIP(series, index), item, parse->flags & AM_FIND_CASE)) ? index+1 : NOT_FOUND;
					break;

				case SYM_INTO:
					if (IS_END(rules)) goto bad_end;
					rulen = 1;
					item = Get_Parse_Value(rules); // sub-rules
					if (!IS_BLOCK(item)) goto bad_rule;
					val = BLK_SKIP(series, index);
					i = (
						(ANY_BINSTR(val) || ANY_BLOCK(val))
						&& (Parse_Series(val, VAL_BLK_DATA(item), parse->flags, depth+1, collect) == VAL_TAIL(val))
					) ? index+1 : NOT_FOUND;
					break;
#ifdef USE_DO_PARSE_RULE
				case SYM_DO:
					if (!IS_BLOCK_INPUT(parse)) goto bad_rule;
					i = Do_Eval_Rule(parse, index, &rules);
					rulen = 1;
					break;
#endif
				default:
					goto bad_rule;
				}
			}
			else if (IS_BLOCK(item)) {
				item = VAL_BLK_DATA(item);
				//if (IS_END(rules) && item == rule_head) {
				//	rules = item;
				//	goto top;
				//}
				i = Parse_Rules_Loop(parse, index, item, depth+1);
				if (parse->result) {
					index = (parse->result > 0) ? i : NOT_FOUND;
					parse->result = 0;
					break;
				}
			}
			// Parse according to datatype:
			else {
				if (IS_BLOCK_INPUT(parse))
					i = Parse_Next_Block(parse, index, item, depth+1);
				else
					i = Parse_Next_String(parse, index, item, depth+1);
			}

			// Necessary for special cases like: some [to end]
			// i: indicates new index or failure of the match, but
			// that does not mean failure of the rule, because optional
			// matches can still succeed, if if the last match failed.
			if (i != NOT_FOUND) {
				count++; // may overflow to negative
				if (count < 0) count = MAX_I32; // the forever case

				// If input did not advance:
				if (i == index) {
					// check if there was processed some _modifying_ rule, which should advance
					// even if index was not changed (https://github.com/Oldes/Rebol-issues/issues/2452)
					if (GET_FLAG(parse->flags, PF_ADVANCE)) {
						// clear the state in case, that there are other rules to be processed
						// keep it in case that we were at the last one
						if(count < maxcount) CLR_FLAG(parse->flags, PF_ADVANCE);
					}
					else if (!GET_FLAG(flags, PF_WHILE)) {
						if (count < mincount) index = NOT_FOUND; // was not enough
						break;
					}
				}
			}
			//if (i >= series->tail) {     // OLD check: no more input
			else {
				if (count < mincount) {
					index = NOT_FOUND; // was not enough
				// Uncomment bellow code, to have result:
				// [? []] = parse ["a"][collect some [keep ('?) collect keep integer!]]
				//	if (GET_FLAG(flags, PF_KEEP))
				//		Parse_Collect_Block(parse);
				} 
				else if (i != NOT_FOUND) index = i;
				// else keep index as is.
				break;
			}
			index = i;

			// A BREAK word stopped us:
			//if (parse->result) {parse->result = 0; break;}
		}

		rules += rulen;

		//if (index > series->tail && index != NOT_FOUND) index = series->tail;
		if (index > series->tail) index = NOT_FOUND;

		//--------------------------------------------------------------------
		// Post Match Processing:
		//--------------------------------------------------------------------
post:
		// Process special flags:
		if (flags) {
			// NOT before all others:
			if (GET_FLAG(flags, PF_NOT)) {
				if (GET_FLAG(flags, PF_NOT2) && index != NOT_FOUND) index = NOT_FOUND;
				else index = begin;
			}
			if (index == NOT_FOUND) { // Failure actions:
				// not decided: if (word) Set_Var_Basic(word, REB_NONE);
				if (GET_FLAG(flags, PF_THEN)) {
					SKIP_TO_BAR(rules);
					if (!IS_END(rules)) rules++;
				}
				if (GET_FLAG(flags, PF_COLLECT)) {
					Parse_Collect_End(collect);
				}
			}
			else {  // Success actions:
				count = (begin > index) ? 0 : index - begin; // how much we advanced the input
				ser = NULL;
				if (GET_FLAG(flags, PF_COPY)) {
					ser = (IS_BLOCK_INPUT(parse))
						? Copy_Block_Len(series, begin, count)
						: Copy_String(series, begin, count); // condenses
					Set_Var_Series(word, parse->type, ser, 0);
				}
				else if (GET_FLAG(flags, PF_SET_OR_COPY)) {
					if (IS_BLOCK_INPUT(parse)) {
						item = Get_Var_Safe(word);
						if (count == 0) SET_NONE(item);
						else *item = *BLK_SKIP(series, begin);
					}
					else {
						item = Get_Var_Safe(word);
						if (count == 0) SET_NONE(item);
						else {
							i = GET_ANY_CHAR(series, begin);
							if (parse->type == REB_BINARY) {
								SET_INTEGER(item, i);
							} else {
								SET_CHAR(item, i);
							}
						}
					}
				}
				if (GET_FLAG(flags, PF_KEEP)) {
					if (ser && GET_FLAG(flags, PF_COPY)) {
						val = Append_Value(collect->block);
						if (IS_BLOCK_INPUT(parse)) {
							Set_Block(val, ser);
						}
						else if (parse->type == REB_BINARY) {
							Set_Binary(val, ser);
						}
						else {
							VAL_SET(val, parse->type);
							VAL_SERIES(val) = ser;
							VAL_INDEX(val) = 0;
							VAL_SERIES_SIDE(val) = 0;
						}
					}
					else {
						Parse_Keep(parse, series, begin, count, GET_FLAG(flags, PF_PICK));
					}
				}
				if (GET_FLAG(flags, PF_COLLECT)) {
					Parse_Collect_End(collect);
				}
				if (GET_FLAG(flags, PF_RETURN)) {
					ser = (IS_BLOCK_INPUT(parse))
						? Copy_Block_Len(series, begin, count)
						: Copy_String(series, begin, count); // condenses
					Throw_Return_Series(parse->type, ser);
				}
				if (GET_FLAG(flags, PF_REMOVE)) {
do_remove:
					if (count) {
						if (IS_PROTECT_SERIES(series)) Trap0(RE_PROTECTED);
						Remove_Series(series, begin, count);
					}
					SET_FLAG(parse->flags, PF_ADVANCE);
					index = begin;
				}
				if (flags & (1<<PF_INSERT | 1<<PF_CHANGE)) {
					count = GET_FLAG(flags, PF_INSERT) ? 0 : count;
					cmd = GET_FLAG(flags, PF_INSERT) ? 0 : (1<<AN_PART);
					item = rules++;
					if (IS_END(item)) goto bad_end;
					// Check for ONLY flag:
					if (IS_WORD(item) && NZ(VAL_CMD(item))) {
						if (VAL_CMD(item) != SYM_ONLY) goto bad_rule;
						cmd |= (1<<AN_ONLY);
						item = rules++;
					}
					// CHECK FOR QUOTE!!
					if (IS_PAREN(item)) {
						item = Do_Block_Value_Throw(item); // might GC
					} else {
						item = Get_Parse_Value(item); // new value
					}
					if (IS_UNSET(item)) Trap1(RE_NO_VALUE, rules-1);
					if (IS_END(item)) goto bad_end;
					if (IS_PROTECT_SERIES(series)) Trap0(RE_PROTECTED);
					if (IS_BLOCK_INPUT(parse)) {
						index = Modify_Block(GET_FLAG(flags, PF_CHANGE) ? A_CHANGE : A_INSERT,
								series, begin, item, cmd, count, 1);
						if (IS_LIT_WORD(item)) SET_TYPE(BLK_SKIP(series, index-1), REB_WORD);
					}
					else {
						if (parse->type == REB_BINARY) cmd |= (1<<AN_SERIES); // special flag
						index = Modify_String(GET_FLAG(flags, PF_CHANGE) ? A_CHANGE : A_INSERT,
								series, begin, item, cmd, count, 1);
					}
					SET_FLAG(parse->flags, PF_ADVANCE);
				}
				if (GET_FLAG(flags, PF_AND)) index = begin;
			}

			flags = 0;
			word = 0;
		}

		// Goto alternate rule and reset input:
		if (index == NOT_FOUND) {
			SKIP_TO_BAR(rules);
			if (IS_END(rules)) break;
			rules++;
			index = begin = start;
		}

		begin = index;
		mincount = maxcount = 1;

	}
	
	if (--Eval_Count <= 0 || Eval_Signals) Do_Signals();

	return index;

bad_rule:
	Trap1(RE_PARSE_RULE, rules-1);
bad_end:
	Trap1(RE_PARSE_END, rules-1);
	return 0;
}

#ifdef INCLUDE_PARSE_SERIES_SPLITTING
/***********************************************************************
**
*/	REBSER *Parse_String(REBSER *series, REBCNT index, REBVAL *rules, REBCNT flags)
/************************************************************************/
{
	REBCNT tail = series->tail;
	REBSER *blk;
	REBSER *set;
	REBCNT begin;
	REBCNT end;
	REBOOL skip_spaces = !(flags & PF_ALL);
	REBUNI uc;

	blk = BUF_EMIT;	// shared series
	RESET_SERIES(blk);

	// String of delimiters or single character:
	if (IS_STRING(rules) || IS_CHAR(rules)) {
		begin = Find_Max_Bit(rules);
		if (begin <= ' ') begin = ' ' + 1;
		set = Make_Bitset(begin);
		Set_Bits(set, rules, TRUE);
	}
	// None, so use defaults ",;":
	else {
		set = Make_Bitset(1+MAX(',',';'));
		Set_Bit(set, ',', TRUE);
		Set_Bit(set, ';', TRUE);
	}
	SAVE_SERIES(set);

	// If required, make space delimiters too:
	if (skip_spaces) {
		for (uc = 1; uc <= ' '; uc++) Set_Bit(set, uc, TRUE);
	}

	while (index < tail) {

		if (--Eval_Count <= 0 || Eval_Signals) Do_Signals();

		// Skip whitespace if not /all refinement: 
		if (skip_spaces) {
			uc = 0;
			for (; index < tail; index++) {
				uc = GET_ANY_CHAR(series, index);
				if (!IS_WHITE(uc)) break;
			}
		}
		else
			uc = GET_ANY_CHAR(series, index); // prefetch

		if (index < tail) {

			// Handle quoted strings (in a simple way):
			if (uc == '"') {
				begin = ++index; // eat quote
				for (; index < tail; index++) {
					uc = GET_ANY_CHAR(series, index);
					if (uc == '"') break;
				}
				end = index;
				if (index < tail) index++;
			}
			// All other tokens:
			else {
				begin = index;
				for (; index < tail; index++) {
					if (Check_Bit(set, GET_ANY_CHAR(series, index), !(flags & PF_CASE))) break;
				}
				end = index;
			}

			// Skip trailing spaces:
			if (skip_spaces)
				for (; index < tail; index++) {
					uc = GET_ANY_CHAR(series, index);
					if (!IS_WHITE(uc)) break;
				}

			// Check for and remove separator:
			if (Check_Bit(set, GET_ANY_CHAR(series, index), !(flags & PF_CASE))) index++;

			// Append new string:
			Set_String(Append_Value(blk), Copy_String(series, begin, end - begin)); 
		}
	}
	UNSAVE_SERIES(set);

	return Copy_Block(blk, 0);
}
#endif

/***********************************************************************
**
*/	REBSER *Parse_Lines(REBSER *src)
/*
**		Convert a string buffer to a block of strings.
**		Note that the string must already be converted
**		to REBOL LF format (no CRs).
**
***********************************************************************/
{
	REBSER	*blk;
	REBUNI c;
	REBCNT i;
	REBCNT s;
	REBVAL *val;
	REBOOL uni = !BYTE_SIZE(src);
	REBYTE *bp = BIN_HEAD(src);
	REBUNI *up = UNI_HEAD(src);

	blk = BUF_EMIT;
	RESET_SERIES(blk);

	// Scan string, looking for LF and CR terminators:
	for (i = s = 0; i < SERIES_TAIL(src); i++) {
		c = uni ? up[i] : bp[i];
		if (c == LF || c == CR) {
			val = Append_Value(blk);
			Set_String(val, Copy_String(src, s, i - s));
			VAL_SET_LINE(val);
			// Skip CRLF if found:
			if (c == CR && LF == uni ? up[i] : bp[i]) i++; 
			s = i;
		}
	}

	// Partial line (no linefeed):
	if (s + 1 != i) {
		val = Append_Value(blk);
		Set_String(val, Copy_String(src, s, i - s));
		VAL_SET_LINE(val);
	}

	return Copy_Block(blk, 0);
}


/***********************************************************************
**
*/	REBNATIVE(parse)
/*
***********************************************************************/
{
	REBVAL *val = D_ARG(1);
	REBVAL *arg = D_ARG(2);
	REBCNT opts = 0;

	if (D_REF(3)) opts |= PF_CASE;
	if (IS_BINARY(val)) opts |= PF_CASE;

// There was originally also /all
//	if (D_REF(3)) opts |= PF_ALL;
//	if (D_REF(4)) opts |= PF_CASE;
//	if (IS_BINARY(val)) opts |= PF_ALL | PF_CASE;

#ifdef INCLUDE_PARSE_SERIES_SPLITTING
	// Is it a simple string?
	if (IS_NONE(arg) || IS_STRING(arg) || IS_CHAR(arg)) {
		REBSER *ser;
		if (!ANY_BINSTR(val)) Trap_Types(RE_EXPECT_VAL, REB_STRING, VAL_TYPE(val));
		ser = Parse_String(VAL_SERIES(val), VAL_INDEX(val), arg, opts);
		Set_Block(DS_RETURN, ser);
	}
	else if (IS_SAME_WORD(arg, SYM_TEXT)) {
		Set_Block(DS_RETURN, Parse_Lines(VAL_SERIES(val)));
	}
	else {
#endif
		REBCNT n;
		REBOL_STATE state;
		REB_PARSE_COLLECT collect;
		// Let user RETURN and THROW out of the PARSE. All other errors should relay.
		PUSH_STATE(state, Saved_State);
		if (SET_JUMP(state)) {
			POP_STATE(state, Saved_State);
			Catch_Error(arg = DS_RETURN); // Stores error value here
			if (VAL_ERR_NUM(arg) == RE_BREAK) {
				if (!VAL_ERR_VALUE(arg)) return R_NONE;
				*DS_RETURN = *VAL_ERR_VALUE(arg);
				return R_RET;
			}
			if (VAL_ERR_NUM(arg) == RE_RETURN && VAL_ERR_SYM(arg) == SYM_RETURN) {
				*DS_RETURN = *VAL_ERR_VALUE(arg);
				return R_RET;
			}
			// How to handle RETURN, BREAK, etc. ???? does not work !!!!
			if (THROWN(DS_RETURN)) return R_RET; //Throw_Break(DS_RETURN);
			Throw_Error(VAL_ERR_OBJECT(DS_RETURN));
		}
		SET_STATE(state, Saved_State);
		CLEARS(&collect);

		n = Parse_Series(val, VAL_BLK_DATA(arg), (opts & PF_CASE) ? AM_FIND_CASE : 0, 0, &collect);
		if (collect.mode == BLOCK_COLLECT) {
			*D_RET = *collect.value;
		}
		else {
			SET_LOGIC(DS_RETURN, n >= VAL_TAIL(val) && n != NOT_FOUND);
		}
		POP_STATE(state, Saved_State);
#ifdef INCLUDE_PARSE_SERIES_SPLITTING
	}
#endif

	return R_RET;
}



#ifdef save_unused
/***********************************************************************
**
*/	static REBFLG Get_Index_Var(REBVAL *item, REBSER *series, REBINT *index)
/*
**		Get the series index from a word or path or integer.
**
**		Returns: TRUE if value was a series. FALSE if integer.
**
***********************************************************************/
{
	REBVAL *hold = item;

	if (IS_END(item)) Trap1(RE_PARSE_END, item);

	if (IS_WORD(item)) {
		if (!VAL_CMD(item)) item = Get_Var(item);
	}
	else if (IS_PATH(item)) {
		REBVAL *path = item;
		Do_Path(&path, 0); //!!! function!
		item = DS_TOP;
	}
	else if (!IS_INTEGER(item))
		Trap1(RE_PARSE_VARIABLE, hold); 

	if (IS_INTEGER(item)) {
		*index = Int32(item);
		return FALSE;
	}

	if (!ANY_SERIES(item) || VAL_SERIES(item) != series)
		Trap1(RE_PARSE_SERIES, hold);

	*index = VAL_INDEX(item);
	return TRUE;
}
#endif

