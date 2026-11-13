/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2025 Rebol Open Source Contributors
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

extern REBDEV *Devices[];

static void Mark_Series(REBSER *series, REBCNT depth);
static void Mark_Value(REBVAL *val, REBCNT depth);

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
*/	static void Mark_Struct_Fields(REBSTU *stu, REBSER *fields, REBCNT offset, REBCNT depth)
/*
**		Mark all Rebol values stored in a struct's data, including values
**		held in nested structs. `fields` is the field record series of the
**		(nested) struct and `offset` is its position in the data series.
**
***********************************************************************/
{
	REBSTF *field = (REBSTF *)BLK_HEAD(fields) + 1; // the info is at the head
	REBCNT count = SERIES_TAIL(fields) - 1;
	REBCNT i, n;
	REBVAL *val;

	for (i = 0; i < count; i++, field++) {
		if (!field->done) continue;
		switch (field->type) {
		case STRUCT_TYPE_REBVAL:
			ASSERT2(field->size == sizeof(REBVAL), RP_BAD_SIZE);
			for (n = 0; n < field->dimension; n++) {
				val = (REBVAL *)BIN_SKIP(STRUCT_DATA(stu), offset + field->offset + n * field->size);
				if (!IS_END(val)) Mark_Value(val, depth + 1);
			}
			break;
		case STRUCT_TYPE_STRUCT:
			// A nested struct may hold Rebol values too!
			// Skip it when there is nothing to mark in it.
			if (!FIELD_SPEC_HAS_VALUES(field)) break;
			for (n = 0; n < field->dimension; n++) {
				Mark_Struct_Fields(stu, field->spec->series, offset + field->offset + n * field->size, depth + 1);
			}
			break;
		}
	}
}

/***********************************************************************
**
*/	static void Mark_Struct(REBSTU *stu, REBCNT depth)
/*
***********************************************************************/
{
	// STRUCT_SPEC is shared and persistently stored in system/catalog/structs
	// so it is marked from there.
	//CHECK_MARK(STRUCT_SPEC(stu), depth);
	if (!STRUCT_DATA(stu) || !STRUCT_FIELDS_SER(stu))
		return;
	if (IS_MARK_SERIES(STRUCT_DATA(stu)))
		return;
	MARK_SERIES(STRUCT_DATA(stu));
	MARK_SERIES(STRUCT_FIELDS_SER(stu));
	ASSERT2(IS_BARE_SERIES(stu->data), RP_BAD_SERIES);
	ASSERT2(!IS_EXT_SERIES(stu->data), RP_BAD_SERIES);

	// The data series is shared by all views into it, and it is marked just
	// once, so the values must always be marked from the root of the data -
	// this struct may be only a view into a nested part of it!
	REBSER *root = STRUCT_DATA(stu)->series;
	if (root) {
		MARK_SERIES(root);
		if (FIELDS_NEED_MARK(root))
			Mark_Struct_Fields(stu, root, 0, depth);
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
	REBDEV **devices = Devices;// Host_Lib->devices;
	
	for (d = 0; d < RDI_MAX; d++) {
		dev = devices[d];
		if (dev)
			for (req = dev->pending; req; req = req->next)
				if (req->port) CHECK_MARK((REBSER*)req->port, depth);
	}
}

/***********************************************************************
**
*/	static void Queue_Mark_Series(REBSER* series)
/*
**		Instead of directly marking all series, queue them for later
**		to avoid a stack overflow in case of deep recursion.
**
***********************************************************************/
{
	if (SERIES_FULL(GC_Mark_Queue)) Extend_Series(GC_Mark_Queue, 8);
	((REBSER**)GC_Mark_Queue->data)[GC_Mark_Queue->tail++] = series;
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
	REBVAL *val;

	ASSERT(series != 0, RP_NULL_MARK_SERIES);

	if (SERIES_FREED(series)) return; // series data freed already

	MARK_SERIES(series);

	// If not a block, go no further
	if (SERIES_WIDE(series) != sizeof(REBVAL) || IS_BARE_SERIES(series)) return;

	ASSERT2(RP_SERIES_OVERFLOW, SERIES_TAIL(series) < SERIES_REST(series));

	//Moved to end: ASSERT1(IS_END(BLK_TAIL(series)), RP_MISSING_END);

	//if (depth == 1 && series->label) RL_Print("Marking %s\n", series->label);

	depth++;

	for (len = 0; len < series->tail; len++) {
		val = BLK_SKIP(series, len);

		if (ANY_SCALAR(val)) {
			continue;
		}

		if (IS_END(val)) {
			// We should never reach the end before len above.
			// Exception is the stack itself.
			if (series != DS_Series) Crash(RP_UNEXPECTED_END);
			break;
		}
		Mark_Value(val, depth);
	}

#if (ALEVEL>0)
	if (!IS_END(BLK_SKIP(series, len)) && series != DS_Series)
		Crash(RP_MISSING_END);
#endif
}


/***********************************************************************
**
*/	static void Mark_Value(REBVAL *val, REBCNT depth)
/*
**
***********************************************************************/
{
	REBSER *ser;
	REBHOB *hob;

	if (ANY_SCALAR(val)) {
		return;
	}
	if (ANY_WORD(val)) {
		// Special word used in word frame, stack, or errors:
		if (VAL_GET_OPT(val, OPTS_UNWORD)) return;
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
		return;
	}
	if (ANY_BLOCK(val)) {
		ser = VAL_SERIES(val);
		ASSERT(ser != 0, RP_NULL_SERIES);
		if (IS_BARE_SERIES(ser)) {
			MARK_SERIES(ser);
			return;
		}
#if (ALEVEL>0)
		if (!IS_END(BLK_SKIP(ser, SERIES_TAIL(ser))) && ser != DS_Series)
			Crash(RP_MISSING_END);
#endif
		if (SERIES_WIDE(ser) != sizeof(REBVAL) && SERIES_WIDE(ser) != 4 && SERIES_WIDE(ser) != 0)
			Crash(RP_BAD_WIDTH, 16, SERIES_WIDE(ser), VAL_TYPE(val));
		QUEUE_CHECK_MARK(ser, depth);
		return;
	}
	if (VAL_TYPE(val) >= REB_BINARY && VAL_TYPE(val) <= REB_BITSET) {
		ser = VAL_SERIES(val);
		if (SERIES_WIDE(ser) > sizeof(REBUNI))
			Crash(RP_BAD_WIDTH, sizeof(REBUNI), SERIES_WIDE(ser), VAL_TYPE(val));
		MARK_SERIES(ser);
		return;
	}

	switch (VAL_TYPE(val)) {
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
	case REB_TYPESET:
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
		if (VAL_FRM_SPEC(val)) { CHECK_MARK(VAL_FRM_SPEC(val), depth); }
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
			if (depth < 64) Mark_Series(VAL_OBJ_FRAME(val), depth);
			else Queue_Mark_Series(VAL_OBJ_FRAME(val));
			//if (SERIES_TAIL(VAL_OBJ_FRAME(val)) >= 1)
			//	Dump_Frame(VAL_OBJ_FRAME(val), 4);
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
		CHECK_MARK(VAL_FUNC_SPEC(val), depth);
		MARK_SERIES(VAL_FUNC_ARGS(val));
		break;
	case REB_OP:
		if (VAL_GET_EXT(val) == REB_FUNCTION) {
			CHECK_MARK(VAL_FUNC_BODY(val), depth);
		}
		CHECK_MARK(VAL_FUNC_SPEC(val), depth);
		MARK_SERIES(VAL_FUNC_ARGS(val));
		break;

	case REB_IMAGE:
	case REB_VECTOR:
		MARK_SERIES(VAL_SERIES(val));
		break;

	case REB_MAP:
		ser = VAL_SERIES(val);
		QUEUE_CHECK_MARK(ser, depth);
		if (ser->series) {
			MARK_SERIES(ser->series);
		}
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

	default:
		Crash(RP_DATATYPE + 1, VAL_TYPE(val));
	}
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
*/	REBI64 Recycle(REBFLG all, REBFLG pools)
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
#ifdef DEBUG
	if (Reb_Opts->watch_recycle) Debug_Str(cs_cast(BOOT_STR(RS_WATCH, 0)));
#endif
	GC_Disabled = 1;

	PG_Reb_Stats->Recycle_Counter++;
	PG_Reb_Stats->Recycle_Series = Mem_Pools[SERIES_POOL].free;

	//printf("PG_Mem_Usage: %llu\n", PG_Mem_Usage);
	REBI64 mem_used = PG_Mem_Usage;                // to count number of managed memory bytes
//	REBI64 ser_used = PG_Reb_Stats->Series_Memory; // to count number of series bytes released (can be to pools)

	//PG_Reb_Stats->Mark_Count = 0;

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

	MARK_SERIES(GC_Mark_Queue);

	// Mark all root series:
	Mark_Series(VAL_SERIES(ROOT_ROOT), 0);
	Mark_Series(Task_Series, 0);

	// Mark all devices:
	Mark_Devices(0);

	// Mark series queued to avoid a stack overflow in case of deep recursion
	while (GC_Mark_Queue->tail > 0) {
		Mark_Series(((REBSER**)GC_Mark_Queue->data)[--GC_Mark_Queue->tail], 0);
	}
	
	count = Sweep_Series();
	count += Sweep_Gobs();
	count += Sweep_Handles();

	// Check memory pool segments.
	// If used recycle/pools refinement, check all segments where usage is less than 90%.
	// Otherwise, check only pools where usage is less than 20%.
	Free_Empty_Pool_Segments(pools ? 90 : 20);

	CHECK_MEMORY(4);

	// Compute new stats:
	PG_Reb_Stats->Recycle_Series = Mem_Pools[SERIES_POOL].free - PG_Reb_Stats->Recycle_Series;
	PG_Reb_Stats->Recycle_Series_Total += PG_Reb_Stats->Recycle_Series;
	PG_Reb_Stats->Recycle_Prior_Eval = Eval_Cycles;

	// Reset stack to prevent invalid MOLD access:
	RESET_TAIL(DS_Series);

	GC_Ballast = VAL_INT32(TASK_BALLAST);
	GC_Disabled = 0;
#ifdef DEBUG
	if (Reb_Opts->watch_recycle) Debug_Fmt(BOOT_STR(RS_WATCH, 1), count);
	//printf("PG_Mem_Usage- %llu\n", PG_Mem_Usage);
	//printf("ser_used: %llu - %llu\n", ser_used, PG_Reb_Stats->Series_Memory);
#endif
	return (mem_used - PG_Mem_Usage);
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
	GC_Infants = Make_Clear_Mem(sizeof(REBSER*), (MAX_SAFE_SERIES + 2)); // extra

	Init_Pools(scale);

	Prior_Expand = Make_Clear_Mem(sizeof(REBSER*), MAX_EXPAND_LIST);
	Prior_Expand[0] = (REBSER*)1;

	// Temporary series protected from GC. Holds series pointers.
	GC_Protect = Make_Series(15, sizeof(REBSER *), FALSE);
	KEEP_SERIES(GC_Protect, "gc protected");

	GC_Series = Make_Series(60, sizeof(REBSER *), FALSE);
	KEEP_SERIES(GC_Series, "gc guarded");

	// Series queued to be marked in case of deep recursion.
	GC_Mark_Queue = Make_Series(15, sizeof(REBSER*), FALSE);
	BARE_SERIES(GC_Mark_Queue);
	LABEL_SERIES(GC_Mark_Queue, "gc mark queue");
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
	Free_Mem(GC_Infants, sizeof(REBSER*) * (MAX_SAFE_SERIES + 2));
	Free_Mem(Prior_Expand, sizeof(REBSER*) * MAX_EXPAND_LIST);
}