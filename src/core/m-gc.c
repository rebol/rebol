/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2021 Rebol Open Source Contributors
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
**  Module:  m-gc.c
**  Summary: main memory garbage collection
**  Section: memory
**  Author:  Carl Sassenrath, Ladislav Mecir, HostileFork
**  Notes:
**    WARNING WARNING WARNING
**    This is highly tuned code that should only be modified by experts
**    who fully understand its design. It is very easy to create odd
**    side effects so please be careful and extensively test all changes!
**
**	  The process consists of two stages:
**
**		MARK -	Mark all series and gobs ("collectible values")
*				that can be found in:
**
**				Root Block: special structures and buffers
**				Task Block: special structures and buffers per task
**				Data Stack: current state of evaluation
**				Safe Series: saves the last N allocations
**
**				Mark is recursive until we reach the terminals, or
**				until we hit values already marked.
**
**		SWEEP - Free all collectible values that were not marked.
**
**	  GC protection methods:
**
**		KEEP flag - protects an individual series from GC, but
**			does not protect its contents (if it holds values).
**			Reserved for non-block system series.
**
**		Root_Context - protects all series listed. This list is
**			used by Sweep as the root of the in-use memory tree.
**			Reserved for important system series only.
**
**		Task_Context - protects all series listed. This list is
**			the same as Root, but per the current task context.
**
**		Save_Series - protects temporary series. Used with the
**			SAVE_SERIES and UNSAVE_SERIES macros. Throws and errors
**			must roll back this series to avoid "stuck" memory.
**
**		Safe_Series - protects last MAX_SAFE_SERIES series from GC.
**			Can only be used if no deeply allocating functions are
**			called within the scope of its protection. Not affected
**			by throws and errors.
**
**		Data_Stack - all values in the data stack that are below
**			the TOP (DSP) are automatically protected. This is a
**			common protection method used by native functions.
**
**		DISABLE_GC - macro that turns off GC. A quick way to avoid
**			GC, but must only be used for well-behaved sections
**			or could cause substantial memory growth.
**
**		DONE flag - do not scan the series; it has no links.
**
***********************************************************************/

#include "sys-core.h"
#include "reb-evtypes.h"
//#include "stdio.h"

#ifdef REB_API
extern REBOL_HOST_LIB *Host_Lib;
#endif

//-- For Serious Debugging:
#ifdef WATCH_GC_VALUE
REBSER *Watcher = 0;
REBVAL *WatchVar = 0;
REBVAL *GC_Break_Point(REBVAL *val) {return val;}
REBVAL *N_watch(REBFRM *frame, REBVAL **inter_block)
{
	WatchVar = Get_Word(FRM_ARG1(frame));
	Watcher = VAL_SERIES(WatchVar);
	SET_INTEGER(FRM_ARG1(frame), 0);
	return Nothing;
}
#endif

// This can be put below
#ifdef WATCH_GC_VALUE
			if (Watcher && ser == Watcher)
				GC_Break_Point(val);

		// for (n = 0; n < depth * 2; n++) Prin_Str(" ");
		// Mark_Count++;
		// Print("Mark: %s %x", TYPE_NAME(val), val);
#endif

static void Mark_Series(REBSER *series, REBCNT depth);
//static void Mark_Value(REBVAL *val, REBCNT depth);

/***********************************************************************
**
*/	static void Mark_Gob(REBGOB *gob, REBCNT depth)
/*
***********************************************************************/
{
	REBGOB **pane;
	REBCNT i;

	if (IS_GOB_MARK(gob)) return;

	MARK_GOB(gob);

	if (GOB_PANE(gob)) {
		MARK_SERIES(GOB_PANE(gob));
		pane = GOB_HEAD(gob);
		for (i = 0; i < GOB_TAIL(gob); i++, pane++) {
			Mark_Gob(*pane, depth);
		}
	}

	if (GOB_PARENT(gob)) Mark_Gob(GOB_PARENT(gob), depth);

	if (GOB_CONTENT(gob)) {
		if (GOB_TYPE(gob) >= GOBT_IMAGE && GOB_TYPE(gob) <= GOBT_STRING) {
			MARK_SERIES(GOB_CONTENT(gob));
		} else if (GOB_TYPE(gob) >= GOBT_DRAW && GOB_TYPE(gob) <= GOBT_EFFECT) {
			CHECK_MARK(GOB_CONTENT(gob), depth);
		}
	}

	if (GOB_DATA(gob) && GOB_DTYPE(gob) && GOB_DTYPE(gob) != GOBD_INTEGER) {
		CHECK_MARK(GOB_DATA(gob), depth);
	}
}

/***********************************************************************
**
*/	static void Mark_Struct_Field(REBSTU *stu, struct Struct_Field *field, REBCNT depth)
/*
***********************************************************************/
{
	if (field->type == STRUCT_TYPE_STRUCT) {
		REBCNT len = 0;
		REBSER *series = NULL;

		CHECK_MARK(field->fields, depth);
		CHECK_MARK(field->spec, depth);

		series = field->fields;
		for (len = 0; len < series->tail; len++) {
			Mark_Struct_Field(stu, (struct Struct_Field *)SERIES_SKIP(series, len), depth + 1);
		}
	}
#ifdef unused
	else if (field->type == STRUCT_TYPE_REBVAL) {
		REBCNT i;
		ASSERT2(field->size == sizeof(REBVAL), RP_BAD_SIZE);
		for (i = 0; i < field->dimension; i++) {
			REBVAL *data = (REBVAL *)SERIES_SKIP(STRUCT_DATA_BIN(stu),
				STRUCT_OFFSET(stu) + field->offset + i * field->size);
			if (field->done) {
				Mark_Value(data, depth);
			}
		}
	}
#endif
	/* ignore primitive datatypes */
}

/***********************************************************************
**
*/	static void Mark_Struct(REBSTU *stu, REBCNT depth)
/*
***********************************************************************/
{
	REBCNT len = 0;
	REBSER *series = NULL;
	if (IS_MARK_SERIES(STRUCT_DATA_BIN(stu))) return;

	CHECK_MARK(stu->spec, depth);
	CHECK_MARK(stu->fields, depth);
	CHECK_MARK(STRUCT_DATA_BIN(stu), depth);

	//Debug_Num("mark spec:  ", (int)stu->spec);
	//Debug_Num("mark fields:", (int)stu->fields);
	//Debug_Num("mark bin:   ", (int)STRUCT_DATA_BIN(stu));

	ASSERT2(IS_BARE_SERIES(stu->data), RP_BAD_SERIES);
	ASSERT2(!IS_EXT_SERIES(stu->data), RP_BAD_SERIES);
	ASSERT2(SERIES_TAIL(stu->data) == 1, RP_BAD_SERIES);
	CHECK_MARK(stu->data, depth);

	series = stu->fields;
	for (len = 0; len < series->tail; len++) {
		struct Struct_Field *field = (struct Struct_Field *)SERIES_SKIP(series, len);
		Mark_Struct_Field(stu, field, depth + 1);
	}
}


/***********************************************************************
**
*/	static void Mark_Event(REBVAL *value, REBCNT depth)
/*
***********************************************************************/
{
	REBREQ *req;
	
	if (
		   IS_EVENT_MODEL(value, EVM_PORT)
		|| IS_EVENT_MODEL(value, EVM_OBJECT)
		|| (VAL_EVENT_TYPE(value) == EVT_DROP_FILE && GET_FLAG(VAL_EVENT_FLAGS(value), EVF_COPIED))
	) {
		// The ->ser field of the REBEVT is void*, so we must cast
		// Comment says it is a "port or object"
		CHECK_MARK((REBSER*)VAL_EVENT_SER(value), depth);
	}

	if (IS_EVENT_MODEL(value, EVM_GUI)) {
		Mark_Gob(VAL_EVENT_SER(value), depth);
	}

	if (IS_EVENT_MODEL(value, EVM_DEVICE)) {
		// In the case of being an EVM_DEVICE event type, the port! will
		// not be in VAL_EVENT_SER of the REBEVT structure.  It is held
		// indirectly by the REBREQ ->req field of the event, which
		// in turn possibly holds a singly linked list of other requests.
		req = VAL_EVENT_REQ(value);
		
		while(req) {
			// The ->port field of the REBREQ is void*, so we must cast
			// Comment says it is "link back to REBOL port object"
			if (req->port) CHECK_MARK((REBSER*)req->port, depth);
			req = req->next;
		}
	}
}

/***********************************************************************
**
*/ static void Mark_Devices(REBCNT depth)
/*
**  Mark all devices. Search for pending requests.
**
***********************************************************************/
{
	int d;
	REBDEV *dev;
	REBREQ *req;
	REBDEV **devices = Host_Lib->devices;
	
	for (d = 0; d < RDI_MAX; d++) {
		dev = devices[d];
		if (dev)
			for (req = dev->pending; req; req = req->next)
				if (req->port) CHECK_MARK((REBSER*)req->port, depth);
	}
}

/***********************************************************************
**
*/	static void Mark_Series(REBSER *series, REBCNT depth)
/*
**		Mark all series reachable from the block.
**
***********************************************************************/
{
	REBCNT len;
	REBSER *ser;
	REBVAL *val;
	REBHOB *hob;

	ASSERT(series != 0, RP_NULL_MARK_SERIES);

	if (SERIES_FREED(series)) return; // series data freed already

	MARK_SERIES(series);

	// If not a block, go no further
	if (SERIES_WIDE(series) != sizeof(REBVAL) || IS_BARE_SERIES(series)) return;

	ASSERT2(RP_SERIES_OVERFLOW, SERIES_TAIL(series) < SERIES_REST(series));

	//Moved to end: ASSERT1(IS_END(BLK_TAIL(series)), RP_MISSING_END);

	//if (depth == 1 && series->label) Print("Marking %s", series->label);

	depth++;

	for (len = 0; len < series->tail; len++) {
		val = BLK_SKIP(series, len);

		switch (VAL_TYPE(val)) {

		case REB_END:
			// We should never reach the end before len above.
			// Exception is the stack itself.
			if (series != DS_Series) Crash(RP_UNEXPECTED_END);
			break;

		case REB_UNSET:
		case REB_TYPESET:
			break;
		case REB_HANDLE:
			if (IS_CONTEXT_HANDLE(val)) {
				hob = VAL_HANDLE_CTX(val);
				//printf("marked hob: %p %p\n", hob, val);
				MARK_HANDLE_CONTEXT(val);
				if (hob->series) {
					Mark_Series(hob->series, depth);
				}
			}	
			else if (IS_SERIES_HANDLE(val) && !HANDLE_GET_FLAG(val, HANDLE_RELEASABLE)) {
				//printf("markserhandle %0xh val: %0xh %s \n", (void*)val, VAL_HANDLE(val), VAL_HANDLE_NAME(val));
				Mark_Series(VAL_HANDLE_DATA(val), depth);
			}
			break;

		case REB_DATATYPE:
			if (VAL_TYPE_SPEC(val)) {	// allow it to be zero
				CHECK_MARK(VAL_TYPE_SPEC(val), depth); // check typespec.reb file
			}
			break;

		case REB_ERROR:
			// If it has an actual error object, then mark it. Otherwise,
			// it is a THROW, and GC of a THROW value is invalid because
			// it contains temporary values on the stack that could be
			// above the current DSP (where the THROW was done).
			if (VAL_ERR_NUM(val) > RE_THROW_MAX) {
				if (VAL_ERR_OBJECT(val)) CHECK_MARK(VAL_ERR_OBJECT(val), depth);
			}
			// else Crash(RP_THROW_IN_GC); // !!!! in question - is it true?
			break;

		case REB_TASK: // not yet implemented
			break;

		case REB_FRAME:
			// Mark special word list. Contains no pointers because
			// these are special word bindings (to typesets if used).
			if (VAL_FRM_WORDS(val)) MARK_SERIES(VAL_FRM_WORDS(val));
			if (VAL_FRM_SPEC(val)) {CHECK_MARK(VAL_FRM_SPEC(val), depth);}
			break;

		case REB_PORT:
			// Debug_Fmt("\n\nmark port: %x %d", val, VAL_TAIL(val));
			// Debug_Values(VAL_OBJ_VALUE(val,1), VAL_TAIL(val)-1, 100);
			goto mark_obj;

		case REB_MODULE:
			if (VAL_MOD_BODY(val)) CHECK_MARK(VAL_MOD_BODY(val), depth);
		case REB_OBJECT:
			// Object is just a block with special first value (context):
mark_obj:
			if (!IS_MARK_SERIES(VAL_OBJ_FRAME(val))) {
				Mark_Series(VAL_OBJ_FRAME(val), depth);
				if (SERIES_TAIL(VAL_OBJ_FRAME(val)) >= 1)
					; //Dump_Frame(VAL_OBJ_FRAME(val), 4);
			}
			break;

		case REB_FUNCTION:
		case REB_COMMAND:
		case REB_CLOSURE:
		case REB_REBCODE:
			CHECK_MARK(VAL_FUNC_BODY(val), depth);
			/* no break */
		case REB_NATIVE:
		case REB_ACTION:
		case REB_OP:
			CHECK_MARK(VAL_FUNC_SPEC(val), depth);
			MARK_SERIES(VAL_FUNC_ARGS(val));
			// There is a problem for user define function operators !!!
			// Their bodies are not GC'd!
			break;

		case REB_WORD:	// (and also used for function STACK backtrace frame)
		case REB_SET_WORD:
		case REB_GET_WORD:
		case REB_LIT_WORD:
		case REB_REFINEMENT:
		case REB_ISSUE:
			// Special word used in word frame, stack, or errors:
			if (VAL_GET_OPT(val, OPTS_UNWORD)) break;
			// Mark its context, if it has one:
			if (VAL_WORD_INDEX(val) > 0 && NZ(ser = VAL_WORD_FRAME(val))) {
				//if (SERIES_TAIL(ser) > 100) Dump_Word_Value(val);
				CHECK_MARK(ser, depth);
			}
			// Possible bug above!!! We cannot mark relative words (negative
			// index) because the frame pointer does not point to a context,
			// it may point to a function body, native code, or action number.
			// But, what if a function is GC'd during it's own evaluation, what
			// keeps the function's code block from being GC'd?
			break;

		case REB_NONE:
		case REB_LOGIC:
		case REB_INTEGER:
		case REB_DECIMAL:
		case REB_PERCENT:
		case REB_MONEY:
		case REB_TIME:
		case REB_DATE:
		case REB_CHAR:
		case REB_PAIR:
		case REB_TUPLE:
			break;

		case REB_STRING:
		case REB_BINARY:
		case REB_FILE:
		case REB_EMAIL:
		case REB_URL:
		case REB_TAG:
		case REB_BITSET:
		case REB_REF:
			ser = VAL_SERIES(val);
			if (SERIES_WIDE(ser) > sizeof(REBUNI))
				Crash(RP_BAD_WIDTH, sizeof(REBUNI), SERIES_WIDE(ser), VAL_TYPE(val));
			MARK_SERIES(ser);
			break;

		case REB_IMAGE:
			//MARK_SERIES(VAL_SERIES_SIDE(val)); //????
			MARK_SERIES(VAL_SERIES(val));
			break;

		case REB_VECTOR:
			MARK_SERIES(VAL_SERIES(val));
			break;

		case REB_BLOCK:
		case REB_PAREN:
		case REB_PATH:
		case REB_SET_PATH:
		case REB_GET_PATH:
		case REB_LIT_PATH:
			ser = VAL_SERIES(val);
			ASSERT(ser != 0, RP_NULL_SERIES);
			if (IS_BARE_SERIES(ser)) {
				MARK_SERIES(ser);
				break;
			}
#if (ALEVEL>0)
			if (!IS_END(BLK_SKIP(ser, SERIES_TAIL(ser))) && ser != DS_Series)
				Crash(RP_MISSING_END);
#endif
			if (SERIES_WIDE(ser) != sizeof(REBVAL) && SERIES_WIDE(ser) != 4 && SERIES_WIDE(ser) != 0)
				Crash(RP_BAD_WIDTH, 16, SERIES_WIDE(ser), VAL_TYPE(val));
			CHECK_MARK(ser, depth);
			break;

		case REB_MAP:
			ser = VAL_SERIES(val);
			CHECK_MARK(ser, depth);
			if (ser->series) {
				MARK_SERIES(ser->series);
			}
			break;

#ifdef ndef
		case REB_ROUTINE:
		  // Deal with the co-joined struct value...
			CHECK_MARK(VAL_STRUCT_SPEC(VAL_ROUTINE_SPEC(val)), depth);
			CHECK_MARK(VAL_STRUCT_VALS(VAL_ROUTINE_SPEC(val)), depth);
			MARK_SERIES(VAL_STRUCT_DATA(VAL_ROUTINE_SPEC(val)));
			MARK_SERIES(VAL_ROUTINE_SPEC_SER(val));
//!!!			if (Current_Closing_Library && VAL_ROUTINE_ID(val) == Current_Closing_Library)
				VAL_ROUTINE_ID(val) = 0; // Invalidate the routine
			break;
#endif

		case REB_LIBRARY:
			MARK_SERIES(VAL_LIBRARY_NAME(val));
//!!!			if (Current_Closing_Library && VAL_LIBRARY_ID(val) == Current_Closing_Library)
				VAL_LIBRARY_ID(val) = 0; // Invalidate the library
			break;

		case REB_STRUCT:
			Mark_Struct(&VAL_STRUCT(val), depth);
			break;

		case REB_GOB:
			Mark_Gob(VAL_GOB(val), depth);
			break;

		case REB_EVENT:
			Mark_Event(val, depth);
			break;

		default:
			Crash(RP_DATATYPE+1, VAL_TYPE(val));
		}
	}

#if (ALEVEL>0)
	if (!IS_END(BLK_SKIP(series, len)) && series != DS_Series)
		Crash(RP_MISSING_END);
#endif
}


/***********************************************************************
**
*/	static REBCNT Sweep_Series(void)
/*
**		Free all unmarked series.
**
**		Scans all series in all segments that are part of the
**		SERIES_POOL. Free series that have not been marked.
**
***********************************************************************/
{
	REBSEG	*seg;
	REBSER	*series;
	REBCNT  n;
	REBCNT	count = 0;

	for (seg = Mem_Pools[SERIES_POOL].segs; seg; seg = seg->next) {
		series = (REBSER *) (seg + 1);
		for (n = Mem_Pools[SERIES_POOL].units; n > 0; n--) {
			SKIP_WALL(series);
			MUNG_CHECK(SERIES_POOL, series, sizeof(*series));
			if (!SERIES_FREED(series)) {
				if (IS_FREEABLE(series)) {
					//printf("free: %0xh %s\n", (int)series, series->label);
					Free_Series(series);
					count++;
				} else
					UNMARK_SERIES(series);
			}
			series++;
			SKIP_WALL(series);
		}
	}

	return count;
}


/***********************************************************************
**
*/	static REBCNT Sweep_Gobs(void)
/*
**		Free all unmarked gobs.
**
**		Scans all gobs in all segments that are part of the
**		GOB_POOL. Free gobs that have not been marked.
**
***********************************************************************/
{
	REBSEG	*seg;
	REBGOB	*gob;
	REBCNT  n;
	REBCNT	count = 0;

	for (seg = Mem_Pools[GOB_POOL].segs; seg; seg = seg->next) {
		gob = (REBGOB *) (seg + 1);
		for (n = Mem_Pools[GOB_POOL].units; n > 0; n--) {
			SKIP_WALL_TYPE(gob, REBGOB);
			MUNG_CHECK(GOB_POOL, gob, sizeof(*gob));
			if (IS_GOB_USED(gob)) {
				if (IS_GOB_MARK(gob))
					UNMARK_GOB(gob);
				else {
					Free_Gob(gob);
					count++;
				}
			}
			gob++;
			SKIP_WALL_TYPE(gob, REBGOB);
		}
	}

	return count;
}


/***********************************************************************
**
*/	static REBCNT Sweep_Handles(void)
/*
**		Free all unmarked handles.
**
**		Scans all hobs in all segments that are part of the
**		HOB_POOL. Free hobs that have not been marked.
**
***********************************************************************/
{
	REBSEG	*seg;
	REBHOB	*hob;
	REBCNT  n;
	REBCNT	count = 0;

	for (seg = Mem_Pools[HOB_POOL].segs; seg; seg = seg->next) {
		hob = (REBHOB *) (seg + 1);
		for (n = Mem_Pools[HOB_POOL].units; n > 0; n--) {
			SKIP_WALL_TYPE(hob, REBHOB);
			MUNG_CHECK(HOB_POOL, hob, sizeof(*hob));
			if (IS_USED_HOB(hob)) {
				//printf("sweep hob: %p\n", hob);
				if (IS_MARK_HOB(hob))
					UNMARK_HOB(hob);
				else {
					count += Free_Hob(hob);
				}
			}
			hob++;
			SKIP_WALL_TYPE(hob, REBHOB);
		}
	}

	return count;
}


/***********************************************************************
**
*/	REBCNT Recycle(REBOOL all)
/*
**		Recycle memory no longer needed.
**
**      When all is TRUE, then infant series are not protected!
**
***********************************************************************/
{
	REBINT n;
	REBSER **sp;
	REBCNT count;

	//Debug_Num("GC", GC_Disabled);

	// If disabled, exit now but set the pending flag.
	if (GC_Disabled || !GC_Active) {
		SET_SIGNAL(SIG_RECYCLE);
		//Print("pending");
		return 0;
	}

	if (Reb_Opts->watch_recycle) Debug_Str(cs_cast(BOOT_STR(RS_WATCH, 0)));

	GC_Disabled = 1;

	PG_Reb_Stats->Recycle_Counter++;
	PG_Reb_Stats->Recycle_Series = Mem_Pools[SERIES_POOL].free;

	PG_Reb_Stats->Mark_Count = 0;

	// WARNING: These terminate existing open blocks. This could
	// be a problem if code is building a new value at the tail,
	// but has not yet updated the TAIL marker.
	DS_TERMINATE; // Update data stack tail
//	SET_END(DS_NEXT);
	VAL_BLK_TERM(TASK_BUF_EMIT);
	VAL_BLK_TERM(TASK_BUF_WORDS);
//!!!	SET_END(BLK_TAIL(Save_Value_List));

	// Mark series stack (temp-saved series):
	sp = (REBSER **)GC_Protect->data;
	for (n = SERIES_TAIL(GC_Protect); n > 0; n--) {
		Mark_Series(*sp++, 0);
	}

	// Mark all special series:
	sp = (REBSER **)GC_Series->data;
	for (n = SERIES_TAIL(GC_Series); n > 0; n--) {
		Mark_Series(*sp++, 0);
	}

	// Mark the last MAX_SAFE "infant" series that were created.
	// We must assume that infant blocks are valid - that they contain
	// no partially valid datatypes (that are under construction).
	if (!all) {
		for (n = 0; n < MAX_SAFE_SERIES; n++) {
			REBSER *ser;
			if (NZ(ser = GC_Infants[n])) {
				//Dump_Series(ser, "Safe Series");
				Mark_Series(ser, 0);
			}
			else break;
		}
	}

	// Mark all root series:
	Mark_Series(VAL_SERIES(ROOT_ROOT), 0);
	Mark_Series(Task_Series, 0);

	// Mark all devices:
	Mark_Devices(0);
	
	count = Sweep_Series();
	count += Sweep_Gobs();
	count += Sweep_Handles();

	CHECK_MEMORY(4);

	// Compute new stats:
	PG_Reb_Stats->Recycle_Series = Mem_Pools[SERIES_POOL].free - PG_Reb_Stats->Recycle_Series;
	PG_Reb_Stats->Recycle_Series_Total += PG_Reb_Stats->Recycle_Series;
	PG_Reb_Stats->Recycle_Prior_Eval = Eval_Cycles;

	// Reset stack to prevent invalid MOLD access:
	RESET_TAIL(DS_Series);

	GC_Ballast = VAL_INT32(TASK_BALLAST);
	GC_Disabled = 0;

	if (Reb_Opts->watch_recycle) Debug_Fmt(BOOT_STR(RS_WATCH, 1), count);
	return count;
}


/***********************************************************************
**
*/	void Save_Series(REBSER *series)
/*
***********************************************************************/
{
	if (SERIES_FULL(GC_Protect)) Extend_Series(GC_Protect, 8);
	((REBSER **)GC_Protect->data)[GC_Protect->tail++] = series;
}


/***********************************************************************
**
*/	void Guard_Series(REBSER *series)
/*
**		A list of protected series, managed by specific removal.
**
***********************************************************************/
{
	LABEL_SERIES(series, "guarded");
	if (SERIES_FULL(GC_Series)) Extend_Series(GC_Series, 8);
	((REBSER **)GC_Series->data)[GC_Series->tail++] = series;
}


/***********************************************************************
**
*/	void Loose_Series(REBSER *series)
/*
**		Remove a series from the protected list.
**
***********************************************************************/
{
	REBSER **sp;
	REBCNT n;

	LABEL_SERIES(series, "unguarded");
	sp = (REBSER **)GC_Series->data;
	for (n = 0; n < SERIES_TAIL(GC_Series); n++) {
		if (sp[n] == series) {
			Remove_Series(GC_Series, n, 1);
			break;
		}
	}
}


/***********************************************************************
**
*/	void Init_Memory(REBINT scale)
/*
**		Initialize memory system.
**
***********************************************************************/
{
	GC_Active = 0;			// TRUE when recycle is enabled (set by RECYCLE func)
	GC_Disabled = 0;		// GC disabled counter for critical sections.
	GC_Ballast = MEM_BALLAST;
	GC_Last_Infant = 0;		// Keep the last N series safe from GC.
	GC_Infants = Make_Mem((MAX_SAFE_SERIES + 2) * sizeof(REBSER*)); // extra

	Init_Pools(scale);

	Prior_Expand = Make_Mem(MAX_EXPAND_LIST * sizeof(REBSER*));
	Prior_Expand[0] = (REBSER*)1;

	// Temporary series protected from GC. Holds series pointers.
	GC_Protect = Make_Series(15, sizeof(REBSER *), FALSE);
	KEEP_SERIES(GC_Protect, "gc protected");

	GC_Series = Make_Series(60, sizeof(REBSER *), FALSE);
	KEEP_SERIES(GC_Series, "gc guarded");
}

/***********************************************************************
**
*/	void Dispose_Memory(void)
/*
**		Dispose memory system when application quits.
**
***********************************************************************/
{
	REBCNT n;
	GC_Disabled = 0;
	// Dispose context handles first, because they may depend on other series!
	Dispose_Hobs();
	/* remove everything from GC_Infants (GC protection) */
	for (n = 0; n < MAX_SAFE_SERIES; n++) {
		GC_Infants[n] = NULL;
	}
	Free_Series(GC_Protect);
	Free_Series(GC_Series);
	Sweep_Series();
	Sweep_Gobs();
	Free_Mem(GC_Infants, 0);
	Free_Mem(Prior_Expand, 0);
	Dispose_Pools();
}
