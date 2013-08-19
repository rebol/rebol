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
**  Module:  d-dump.c
**  Summary: various debug output functions
**  Section: debug
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#include "sys-core.h"


/***********************************************************************
**
*/	void Dump_Series(REBSER *series, REBYTE *memo)
/*
***********************************************************************/
{
	if (!series) return;
	Debug_Fmt(
		Str_Dump[0], //"%s Series %x %s: Wide: %2d Size: %6d - Bias: %d Tail: %d Rest: %d Flags: %x"
		memo,
		series,
		(SERIES_LABEL(series) ? SERIES_LABEL(series) : "-"),
		SERIES_WIDE(series),
		SERIES_TOTAL(series),
		SERIES_BIAS(series),
		SERIES_TAIL(series),
		SERIES_REST(series),
		SERIES_FLAGS(series)
	);
	if (SERIES_WIDE(series) == sizeof(REBVAL))
		Dump_Values(BLK_HEAD(series), SERIES_TAIL(series));
	else
		Dump_Bytes(series->data, (SERIES_TAIL(series)+1) * SERIES_WIDE(series));
}

/***********************************************************************
**
*/	void Dump_Bytes(REBYTE *bp, REBCNT limit)
/*
***********************************************************************/
{
	const max_lines = 120;
	REBYTE buf[2048];
	REBYTE str[40];
	REBYTE *cp, *tp;
	REBYTE c;
	REBCNT l, n;
	REBCNT cnt = 0;

	cp = buf;
	for (l = 0; l < max_lines; l++) {
		cp = Form_Hex_Pad(cp, (REBCNT) bp, 8);

		*cp++ = ':';
		*cp++ = ' ';
		tp = str;

		for (n = 0; n < 16; n++) {
			if (cnt++ >= limit) break;
			c = *bp++;
			cp = Form_Hex2(cp, c);
			if ((n & 3) == 3) *cp++ = ' ';
			if ((c < 32) || (c > 126)) c = '.';
			*tp++ = c;
		}

		for (; n < 16; n++) {
			c = ' ';
			*cp++ = c;
			*cp++ = c;
			if ((n & 3) == 3) *cp++ = ' ';
			if ((c < 32) || (c > 126)) c = '.';
			*tp++ = c;
		}
		*tp++ = 0;

		for (tp = str; *tp;) *cp++ = *tp++;

		*cp = 0;
		Debug_Str(buf);
		if (cnt >= limit) break;
		cp = buf;
	}
}

/***********************************************************************
**
*/	void Dump_Values(REBVAL *vp, REBCNT count)
/*
**		Print out values in raw hex; If memory is corrupted
**		this function still needs to work.
**
***********************************************************************/
{
	REBYTE buf[2048];
	REBYTE *cp;
	REBCNT l, n;
	REBCNT *bp = (REBCNT*)vp;
	REBYTE *type;

	cp = buf;
	for (l = 0; l < count; l++) {
		cp = Form_Hex_Pad(cp, (REBCNT) l, 4);
		*cp++ = ':';
		*cp++ = ' ';

		type = Get_Type_Name((REBVAL*)bp);
		for (n = 0; n < 11; n++) {
			if (*type) *cp++ = *type++;
			else *cp++ = ' ';
		}
		*cp++ = ' ';
		for (n = 0; n < 4; n++) {
			cp = Form_Hex_Pad(cp, *bp++, 8);
			*cp++ = ' ';
		}

		*cp = 0;
		Debug_Str(buf);
		cp = buf;
	}
}

#ifdef not_used
/***********************************************************************
**
xx*/	void Dump_Block_Raw(REBSER *series, int depth, int max_depth)
/*
***********************************************************************/
{
	REBVAL *val;
	REBCNT n;
	REBYTE *str;

	if (!IS_BLOCK_SERIES(series) || depth > max_depth) return;

	for (n = 0, val = BLK_HEAD(series); NOT_END(val); val++, n++) {
		Debug_Chars(' ', depth * 4);
		if (IS_BLOCK(val)) {
			Debug_Fmt("%3d: [%s] len: %d", n, Get_Type_Name(val), VAL_TAIL(val));
			Dump_Block_Raw(VAL_SERIES(val), depth + 1, max_depth);
		} else {
			str = "";
			if (ANY_WORD(val)) str = Get_Word_Name(val);
			Debug_Fmt("%3d: [%s] %s", n, Get_Type_Name(val), str);
		}
	}
	//if (depth == 2) Input_Str();
}

/***********************************************************************
**
xx*/  REBSER *Dump_Value(REBVAL *block, REBSER *series)
/*
**		Dump a values's contents for debugging purposes.
**
***********************************************************************/
{
	REB_MOLD mo = {0};
	mo.digits = 17; // max digits

	if (VAL_TYPE(block) >= REB_MAX) Crash(RP_DATATYPE+7, VAL_TYPE(block));

	ASSERT2(series, 9997);
	mo.series = series; 
	Emit(&mo, "T: ", block);

	Mold_Value(&mo, block, TRUE);

	if (ANY_WORD(block)) {
		if (!VAL_WORD_FRAME(block)) Append_Bytes(series, " - unbound");
		else if (VAL_WORD_INDEX(block) < 0) Append_Bytes(series, " - relative");
		else Append_Bytes(series, " - absolute");
	}
	return series;
}


/***********************************************************************
**
xx*/  void Print_Dump_Value(REBVAL *value, REBYTE *label)
/*
**		Dump a value's contents for debugging purposes.
**
***********************************************************************/
{
	REBSER *series;
	series = Copy_Bytes(label, -1);
	SAVE_SERIES(series);
	series = Dump_Value(value, series);
	Debug_Str(STR_HEAD(series));
	UNSAVE_SERIES(series);
}


/***********************************************************************
**
xx*/  void Dump_Block(REBVAL *blk, REBINT len)
/*
**		Dump a block's contents for debugging purposes.
**
***********************************************************************/
{
	REBSER *series;
	//REBVAL *blk = BLK_HEAD(block);

	//Print("BLOCK: %x Tail: %d Size: %d", block, block->tail, block->rest);
	// change to a make string!!!  no need to append to a series, this is a debug function
	series = Make_Binary(100);
	Append_Bytes(series, "[\n");
	while (NOT_END(blk) && len-- > 0) {
		Append_Byte(series, '\t');
		Dump_Value(blk, series);
		Append_Byte(series, '\n');
		blk++;
	}
	Append_Byte(series, ']');
	*STR_TAIL(series) = 0;
	Debug_Str(STR_HEAD(series));
}


/***********************************************************************
**
xx*/  void Dump_Frame(REBSER *frame, REBINT limit)
/*
***********************************************************************/
{
	REBINT n;
	REBVAL *values = FRM_VALUES(frame);
	REBVAL *words  = FRM_WORDS(frame);

	if (limit == -1 || limit > (REBINT)SERIES_TAIL(frame))
		limit = SERIES_TAIL(frame);

	Debug_Fmt("Frame: %x len = %d", frame, SERIES_TAIL(frame));
	for (n = 0; n < limit; n++, values++, words++) {
		Debug_Fmt("  %02d: %s (%s) [%s]",
			n,
			Get_Sym_Name(VAL_BIND_SYM(words)),
			Get_Sym_Name(VAL_BIND_CANON(words)),
			Get_Type_Name(values)
		);
	}

	if (limit >= (REBINT)SERIES_TAIL(frame) && NOT_END(words))
		Debug_Fmt("** Word list not terminated! Type: %d, Tail: %d", VAL_TYPE(words), SERIES_TAIL(frame));
}


/***********************************************************************
**
xx*/	void Dump_Word_Value(REBVAL *word)
/*
***********************************************************************/
{
	Debug_Fmt("Word: %s (Symbol %d Frame %x Index %d)", Get_Word_Name(word),
		VAL_WORD_SYM(word), VAL_WORD_FRAME(word), VAL_WORD_INDEX(word));
}


/***********************************************************************
**
xx*/	void Dump_Word_Table()
/*
***********************************************************************/
{
	REBCNT	n;
	REBVAL *words = BLK_HEAD(PG_Word_Table.series);

	for (n = 1; n < PG_Word_Table.series->tail; n++) {
		Debug_Fmt("%03d: %s = %d (%d)", n, VAL_SYM_NAME(words+n),
			VAL_SYM_CANON(words+n), VAL_SYM_ALIAS(words+n));
//		if ((n % 40) == 0) getchar();
	}
}


/***********************************************************************
**
xx*/	void Dump_Bind_Table()
/*
***********************************************************************/
{
	REBCNT	n;
	REBINT *binds = WORDS_HEAD(Bind_Table);

	Debug_Fmt("Bind Table (Size: %d)", SERIES_TAIL(Bind_Table));
	for (n = 1; n < SERIES_TAIL(Bind_Table); n++) {
		if (binds[n])
			Debug_Fmt("Bind: %3d to %3d (%s)", n, binds[n], Get_Sym_Name(n));
	}
}
#endif


/***********************************************************************
**
*/  void Dump_Info(void)
/*
***********************************************************************/
{
	REBINT n;
	REBINT nums [] = {
		0,
		0,
		(REBINT)Eval_Cycles,
		Eval_Count,
		Eval_Dose,
		Eval_Signals,
		Eval_Sigmask,
		DSP,
		DSF,
		0,
		GC_Ballast,
		GC_Disabled,
		SERIES_TAIL(GC_Protect),
		GC_Last_Infant,
	};

	DISABLE_GC;
	for (n = 0; n < 14; n++) Debug_Fmt(BOOT_STR(RS_DUMP, n), nums[n]);
	ENABLE_GC;
}


/***********************************************************************
**
*/  void Dump_Stack(REBINT dsf, REBINT dsp)
/*
***********************************************************************/
{
	REBINT n;
	REBINT m;
	REBVAL *args;

	if (dsf == 0) {
		dsf = DSF;
		dsp = DSP;
	}

	m = dsp - dsf - DSF_SIZE;
	Debug_Fmt(BOOT_STR(RS_STACK, 1), dsp, Get_Word_Name(DSF_WORD(dsf)), m, Get_Type_Name(DSF_FUNC(dsf)));

	if (dsf > 0) {
		if (ANY_FUNC(DSF_FUNC(dsf))) {
			args = BLK_HEAD(VAL_FUNC_ARGS(DSF_FUNC(dsf)));
			m = SERIES_TAIL(VAL_FUNC_ARGS(DSF_FUNC(dsf)));
			for (n = 1; n < m; n++)
				Debug_Fmt("\t%s: %72r", Get_Word_Name(args+n), DSF_ARGS(dsf, n));
		}
		//Debug_Fmt(Str_Stack[2], PRIOR_DSF(dsf));
		if (PRIOR_DSF(dsf) > 0) Dump_Stack(PRIOR_DSF(dsf), dsf-1);
	}

	//for (n = 1; n <= 2; n++) {
	//	Debug_Fmt("  ARG%d: %s %r", n, Get_Type_Name(DSF_ARGS(dsf, n)), DSF_ARGS(dsf, n));
	//}
}

#ifdef TEST_PRINT
	// Simple low-level tests:
	Print("%%d %d", 1234);
	Print("%%d %d", -1234);
	Print("%%d %d", 12345678);
	Print("%%d %d", 0);
	Print("%%6d %6d", 1234);
	Print("%%10d %10d", 123456789);
	Print("%%x %x", 0x1234ABCD);
	Print("%%x %x", -1);
	Print("%%4x %x", 0x1234);
	Print("%%s %s", "test");
	Print("%%s %s", 0);
	Print("%%c %c", (REBINT)'X');
	Print("%s %d %x", "test", 1234, 1234);
	getchar();
#endif
