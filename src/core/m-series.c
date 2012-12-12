/***********************************************************************
**
**  REBOL Language Interpreter and Run-time Environment
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
**  Module:  m-series.c
**  Summary: implements REBOL's series concept
**  Section: memory
**  Author:  Carl Sassenrath
**
***********************************************************************/

#include "sys-core.h"


/***********************************************************************
**
*/	void Expand_Series(REBSER *series, REBCNT index, REBCNT delta)
/*
**		Expand a series at a particular index point by the number
**		number of units specified by delta.
**
**			index - where space is expanded (but not cleared)
**			delta - number of UNITS to expand (keeping terminator)
**			tail  - will be updated
**
**			        |<---rest--->|
**			<-bias->|<-tail->|   |
**			+--------------------+
**			|       abcdefghi    |
**			+--------------------+
**			        |    |
**			        data index
**
**		If the series has enough space within it, then it will be used,
**		otherwise the series data will be reallocated.
**
**		When expanded at the head, if bias space is available, it will
**		be used (if it provides enough space).
**
**		WARNING: never use direct pointers into the series data, as the
**		series data can be relocated in memory.
**
***********************************************************************/
{
	REBCNT start;
	REBCNT size;
	REBCNT extra;
	REBCNT wide;
	REBSER *newser, swap;
	REBCNT n;
	REBCNT x;

	if (delta == 0) return;

	// Optimized case of head insertion:
	if (index == 0 && SERIES_BIAS(series) >= delta) {
		series->data -= SERIES_WIDE(series) * delta;
		SERIES_TAIL(series) += delta;
		SERIES_REST(series) += delta;
		SERIES_SUB_BIAS(series, delta);
		return;
	}

	// Range checks:
	if (delta & 0x80000000) Trap0(RE_PAST_END); // 2GB max
	if (index > series->tail) index = series->tail; // clip

	// Width adjusted variables:
	wide  = SERIES_WIDE(series);
	start = index * wide;
	extra = delta * wide;
	size  = (series->tail + 1) * wide;

	// Do we need to expand the current series allocation?
	// WARNING: Do not use ">=" below or newser size may be the same!
	if ((size + extra) > SERIES_SPACE(series)) {
		if (IS_LOCK_SERIES(series)) Crash(RP_LOCKED_SERIES);
		//DISABLE_GC; // Don't let GC occur just for an expansion.

		if (Reb_Opts->watch_expand) {
			Debug_Fmt("Expand %x wide: %d tail: %d delta: %d", series, wide, series->tail, delta);
		}

		// Create a new series that is bigger.
		// Have we recently expanded the same series?
		x = 1;
		n = (REBCNT)(Prior_Expand[0]);
		do {
			if (Prior_Expand[n] == series) {
				x = series->tail + delta + 1; // Double the size
				break;
			}
			if (++n >= MAX_EXPAND_LIST) n = 1;
		} while (n != (REBCNT)(Prior_Expand[0]));
#ifdef DEBUGGING
		Print_Num("Expand:", series->tail + delta + 1);
#endif
		newser = Make_Series(series->tail + delta + x, wide, TRUE);
		// If necessary, add series to the recently expanded list:
		if (Prior_Expand[n] != series) {
			n = (REBCNT)(Prior_Expand[0]) + 1;
			if (n >= MAX_EXPAND_LIST) n = 1;
			Prior_Expand[n] = series;
		}
		Prior_Expand[0] = (REBSER*)n; // start next search here
		Prop_Series(newser, series);
		//ENABLE_GC;

		// Copy the series up to the expansion point:
		memcpy(newser->data, series->data, start);

		// Copy the series after the expansion point:
		// In AT_TAIL cases, this just moves the terminator to the new tail.
		memcpy(newser->data + start + extra, series->data + start, size - start);

		newser->tail = series->tail + delta;

		// Swap new and old series, then free the old one.
		// This seems silly, but this method isolates us from
		// needing to know the internals series headers.
		swap = *series;
		*series = *newser;
		*newser = swap;
		Free_Series(newser);
		SERIES_SET_BIAS(series, 0); // be sure it is reset

		PG_Reb_Stats->Series_Expanded++;	// Metric
		CHECK_MEMORY(3);
		return;
	}

	// No expansion was need. Slide data down if necessary.
	// Note that the tail is always moved here. This is probably faster
	// than doing the computation to determine if it is needs to be done.
	memmove(series->data + start + extra, series->data + start, size - start);
	series->tail += delta;

	if ((SERIES_TAIL(series) + SERIES_BIAS(series)) * wide >= SERIES_TOTAL(series)) {
		Dump_Series(series, "Overflow");
		ASSERT(0, RP_OVER_SERIES);
	}

	CHECK_MEMORY(3);
}


/***********************************************************************
**
*/	void Extend_Series(REBSER *series, REBCNT delta)
/*
**		Extend a series at its end without affecting its tail index.
**
***********************************************************************/
{
	REBCNT tail = series->tail;	// maintain tail position
	EXPAND_SERIES_TAIL(series, delta);
	series->tail = tail;
}


/***********************************************************************
**
*/	REBCNT Insert_Series(REBSER *series, REBCNT index, REBYTE *data, REBCNT len)
/*
**		Insert a series of values (bytes, longs, reb-vals) into the
**		series at the given index.  Expand it if necessary.  Does
**		not add a terminator to tail.
**
***********************************************************************/
{
	if (index > series->tail) index = series->tail;
	Expand_Series(series, index, len); // tail += len
	//Print("i: %d t: %d l: %d x: %d s: %d", index, series->tail, len, (series->tail + 1) * SERIES_WIDE(series), series->size);
	memcpy(series->data + (SERIES_WIDE(series) * index), data, SERIES_WIDE(series) * len);
	//*(int *)(series->data + (series->tail-1) * SERIES_WIDE(series)) = 5; // for debug purposes
	return index + len;
}


/***********************************************************************
**
*/	void Append_Series(REBSER *series, REBYTE *data, REBCNT len)
/*
**		Append value(s) onto the tail of a series.  The len is
**		the number of units (bytes, REBVALS, etc.) of the data,
**		and does not include the terminator (which will be added).
**		The new tail position will be returned as the result.
**		A terminator will be added to the end of the appended data.
**
***********************************************************************/
{
	REBCNT tail = series->tail;
	REBCNT wide = SERIES_WIDE(series);

	EXPAND_SERIES_TAIL(series, len);
	memcpy(series->data + (wide * tail), data, wide * len);
	CLEAR(series->data + (wide * series->tail), wide); // terminator
}


/***********************************************************************
**
*/	void Append_Mem_Extra(REBSER *series, REBYTE *data, REBCNT len, REBCNT extra)
/*
**		An optimized function for appending raw memory bytes to
**		a byte-sized series. The series will be expanded if room
**		is needed. A zero terminator will be added at the tail.
**		The extra size will be assured in the series, but is not
**		part of the appended length. (Allows adding additional bytes.)
**
***********************************************************************/
{
	REBCNT tail = series->tail;

	if ((tail + len + extra + 1) >= SERIES_REST(series)) {
		Expand_Series(series, tail, len+extra); // series->tail changed
		series->tail -= extra;
	}
	else {
		series->tail += len;
	}

	memcpy(series->data + tail, data, len);
	STR_TERM(series);
}


/***********************************************************************
**
*/	REBSER *Copy_Series(REBSER *source)
/*
**		Copy any series, including terminator for it.
**
***********************************************************************/
{
	REBCNT len = source->tail + 1;
	REBSER *series = Make_Series(len, SERIES_WIDE(source), FALSE);

	memcpy(series->data, source->data, len * SERIES_WIDE(source));
	series->tail = source->tail;
	return series;
}


/***********************************************************************
**
*/	REBSER *Copy_Series_Part(REBSER *source, REBCNT index, REBCNT length)
/*
**		Copy any subseries, including terminator for it.
**
***********************************************************************/
{
	REBSER *series = Make_Series(length+1, SERIES_WIDE(source), FALSE);

	memcpy(series->data, source->data + index * SERIES_WIDE(source), (length+1) * SERIES_WIDE(source));
	series->tail = length;
	return series;
}


/***********************************************************************
**
*/	REBSER *Copy_Series_Value(REBVAL *value)
/*
**		Copy a series from its value structure.
**		Index does not need to be at head location.
**
***********************************************************************/
{
	return Copy_Series_Part(VAL_SERIES(value), VAL_INDEX(value), VAL_LEN(value));
}


#ifdef NOT_USED
/***********************************************************************
**
*/	REBINT Clone_Series(REBVAL *dst, REBVAL *src)
/*
**		Properly deep copy all types of series.
**		Return TRUE if BLOCK type.
**
***********************************************************************/
{
	Check_Stack();
	if (VAL_TYPE(src) < REB_BLOCK) {
		if (VAL_SERIES_WIDTH(src) == 4)
			VAL_SERIES(dst) = Make_Quad(VAL_BIN(src), VAL_TAIL(src));
		else
			VAL_SERIES(dst) = Copy_String(VAL_SERIES(src));
		return FALSE;
	} else {

		VAL_SERIES(dst) = Clone_Block(VAL_SERIES(src));
		if (IS_HASH(dst) || IS_LIST(dst))
			VAL_SERIES_SIDE(dst) = Copy_Side_Series(VAL_SERIES_SIDE(dst));
		return TRUE;
	}
}
#endif


/***********************************************************************
**
*/	void Remove_Series(REBSER *series, REBCNT index, REBINT len)
/*
**		Remove a series of values (bytes, longs, reb-vals) from the
**		series at the given index.
**
***********************************************************************/
{
	REBCNT	start;
	REBCNT	length;
	REBYTE	*data;

	if (len <= 0) return;

	// Optimized case of head removal:
	if (index == 0) {
		if ((REBCNT)len > series->tail) len = series->tail;
		SERIES_TAIL(series) -= len;
		if (SERIES_TAIL(series) == 0) {
			// Reset bias to zero:
			len = SERIES_BIAS(series);
			SERIES_SET_BIAS(series, 0);
			SERIES_REST(series) += len;
			series->data -= SERIES_WIDE(series) * len;
			CLEAR(series->data, SERIES_WIDE(series)); // terminate
		} else {
			// Add bias to head:
			SERIES_ADD_BIAS(series, len);
			SERIES_REST(series) -= len;
			series->data += SERIES_WIDE(series) * len;
			if (NZ(start = SERIES_BIAS(series))) {
				// If more than half biased:
				if (start >= MAX_SERIES_BIAS || start > SERIES_REST(series))
					Reset_Bias(series);
			}
		}
		return;
	}

	if (index >= series->tail) return;

	start = index * SERIES_WIDE(series);

	// Clip if past end and optimize the remove operation:
	if (len + index >= series->tail) {
		series->tail = index;
		CLEAR(series->data + start, SERIES_WIDE(series));
		return;
	}

	length = SERIES_LEN(series) * SERIES_WIDE(series);
	series->tail -= (REBCNT)len;
	len *= SERIES_WIDE(series);
	data = series->data + start;
	memmove(data, data + len, length - (start + len));

	CHECK_MEMORY(5);
}


/***********************************************************************
**
*/	void Remove_Last(REBSER *series)
/*
**		Remove last value from a series.
**
***********************************************************************/
{
	if (series->tail == 0) return;
	series->tail--;
	CLEAR(series->data + SERIES_WIDE(series) * series->tail, SERIES_WIDE(series));
}


/***********************************************************************
**
*/	void Reset_Bias(REBSER *series)
/*
**		Reset series bias.
**
***********************************************************************/
{
	REBCNT len;
	REBYTE *data = series->data;

	len = SERIES_BIAS(series);
	SERIES_SET_BIAS(series, 0);
	SERIES_REST(series) += len;
	series->data -= SERIES_WIDE(series) * len;

	memmove(series->data, data, SERIES_USED(series));
}


/***********************************************************************
**
*/	void Reset_Series(REBSER *series)
/*
**		Reset series to empty. Reset bias, tail, and termination.
**		The tail is reset to zero.
**
***********************************************************************/
{
	series->tail = 0;
	if (SERIES_BIAS(series)) Reset_Bias(series);
	CLEAR(series->data, SERIES_WIDE(series)); // re-terminate
}


/***********************************************************************
**
*/	void Clear_Series(REBSER *series)
/*
**		Clear an entire series to zero. Resets bias and tail.
**		The tail is reset to zero.
**
***********************************************************************/
{
	series->tail = 0;
	if (SERIES_BIAS(series)) Reset_Bias(series);
	CLEAR(series->data, SERIES_SPACE(series));
}


/***********************************************************************
**
*/	void Resize_Series(REBSER *series, REBCNT size)
/*
**		Reset series and expand it to required size.
**		The tail is reset to zero.
**
***********************************************************************/
{
	series->tail = 0;
	if (SERIES_BIAS(series)) Reset_Bias(series);
	EXPAND_SERIES_TAIL(series, size);
	series->tail = 0;
	CLEAR(series->data, SERIES_WIDE(series)); // re-terminate
}


/***********************************************************************
**
*/	void Terminate_Series(REBSER *series)
/*
**		Put terminator at tail of the series.
**
***********************************************************************/
{
	CLEAR(series->data + SERIES_WIDE(series) * series->tail, SERIES_WIDE(series));
}


/***********************************************************************
**
*/	void Shrink_Series(REBSER *series, REBCNT units)
/*
**		Shrink a series back to a given maximum size. All
**		content is deleted and tail is reset.
**
**		WARNING: This should only be used for strings or other
**		series that cannot contain internally referenced values.
**
***********************************************************************/
{
	if (SERIES_REST(series) <= units) return;
	//DISABLE_GC;
	Free_Series_Data(series, FALSE);
	Make_Series_Data(series, units);
	//ENABLE_GC;
}


/***********************************************************************
**
*/  REBYTE *Reset_Buffer(REBSER *buf, REBCNT len)
/*
**		Setup to reuse a shared buffer. Expand it if needed.
**
**		NOTE:The tail is set to the length position.
**
***********************************************************************/
{
	if (!buf) Crash(RP_NO_BUFFER);

	RESET_TAIL(buf);
	if (SERIES_BIAS(buf)) Reset_Bias(buf);
	Expand_Series(buf, 0, len); // sets new tail

	return BIN_DATA(buf);
}


/***********************************************************************
**
*/  REBSER *Copy_Buffer(REBSER *buf, void *end)
/*
**		Copy a shared buffer. Set tail and termination.
**
***********************************************************************/
{
	REBSER *ser;
	REBCNT len;

	len = BYTE_SIZE(buf) ? ((REBYTE *)end) - BIN_HEAD(buf)
		: ((REBUNI *)end) - UNI_HEAD(buf);

	ser = Make_Series(len+1, SERIES_WIDE(buf), FALSE);

	memcpy(ser->data, buf->data, SERIES_WIDE(buf) * len);
	ser->tail = len;
	TERM_SERIES(ser);

	return ser;
}
