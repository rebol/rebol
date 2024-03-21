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
**  Module:  m-pools.c
**  Summary: memory allocation pool management
**  Section: memory
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/
/*
	Ideas...

	Each task needs its own series-save list that is simply a pointer
	array of un-rooted (NEW) series that should not be GCed. When
	a TRAP or THROW occurs, the list is trimmed back to its prior
	marker, allowing series that were orphaned by the TRAP to be GCed.

	When GC occurs, each series on the save list is mark-scanned to
	keep it alive. The save list can be expanded, but care should be
	used to avoid creating a huge list when recursion happens.

	What if interpreter kept track of save list marker when calling
	each native, and reset it on return?
*/

//-- Special Debugging Options:
//#define CHAFF					// Fill series data to crash old references
//#define HIT_END				// Crash if block tail is past block terminator.
//#define WATCH_FREED			// Show # series freed each GC
//#define MEM_STRESS			// Special torture mode enabled
//#define INSPECT_SERIES

#include "sys-core.h"

#define POOL_MAP

#ifdef __LLP64__
#define	BAD_MEM_PTR ((REBYTE *)0xBAD1BAD1BAD1BAD1)
#else
#define	BAD_MEM_PTR ((REBYTE *)0xBAD1BAD1)
#endif

//#define GC_TRIGGER (GC_Active && (GC_Ballast <= 0 || (GC_Pending && !GC_Disabled)))

#ifdef POOL_MAP
#define FIND_POOL(n) ((n <= 4 * MEM_BIG_SIZE) ? (REBCNT)(PG_Pool_Map[n]) : SYSTEM_POOL)
#else
#define FIND_POOL(n) Find_Pool(n);
#endif


/***********************************************************************
**
**	MEMORY POOLS
**
**		Memory management operates off an array of pools, the first
**		group of which are fixed size (so require no compaction).
**
***********************************************************************/
const REBPOOLSPEC Mem_Pool_Spec[MAX_POOLS] =
{
	{8, 512},			// 0-8 Small string pool

	MOD_POOL( 1, 1024),	// 9-16 (when REBVAL is 16)
	MOD_POOL( 2, 512),	// 17-32 - Small series (x 16)
	MOD_POOL( 3, 1024),	// 33-64
	MOD_POOL( 4, 512),
	MOD_POOL( 5, 256),
	MOD_POOL( 6, 128),
	MOD_POOL( 7, 128),
	MOD_POOL( 8,  64),
	MOD_POOL( 9,  64),
	MOD_POOL(10,  64),
	MOD_POOL(11,  32),
	MOD_POOL(12,  32),
	MOD_POOL(13,  32),
	MOD_POOL(14,  32),
	MOD_POOL(15,  32),
	MOD_POOL(16,  64),	// 257
	MOD_POOL(20,  32),	// 321 - Mid-size series (x 64)
	MOD_POOL(24,  16),	// 385
	MOD_POOL(28,  16),	// 449
	MOD_POOL(LAST_SMALL_SIZE,  16),	// 513

	DEF_POOL(MEM_BIG_SIZE,   16),	// 1K - Large series (x 1024)
	DEF_POOL(MEM_BIG_SIZE*2, 16),	// 2K
	DEF_POOL(MEM_BIG_SIZE*3,  4),	// 3K
	DEF_POOL(MEM_BIG_SIZE*4,  8),	// 4K

	DEF_POOL(sizeof(REBSER), 4096),	// Series headers
	DEF_POOL(sizeof(REBGOB), 128),	// Gobs
	DEF_POOL(sizeof(REBHOB), 16),	// Handle objects
	DEF_POOL(1, 1),	// Just used for tracking main memory
};


/***********************************************************************
**
*/	void *Make_Mem(size_t size)
/*
**		Main memory allocation wrapper function.
**
***********************************************************************/
{
	void *ptr;

	if (!(ptr = malloc(size))) return 0;
	PG_Mem_Usage += size;
	if (PG_Mem_Limit != 0 && (PG_Mem_Usage > PG_Mem_Limit)) {
		Check_Security(SYM_MEMORY, POL_EXEC, 0);
	}
	CLEAR(ptr, size);

	return ptr;
}


/***********************************************************************
**
*/	void Free_Mem(void *mem, size_t size)
/*
***********************************************************************/
{
	PG_Mem_Usage -= size;
	free(mem);
}


/***********************************************************************
**
*/	void Init_Pools(REBINT scale)
/*
**		Initialize memory pool array.
**
***********************************************************************/
{
	REBINT n;
	REBINT unscale = 1;

	if (scale == 0) scale = 1;
	else if (scale < 0) unscale = -scale, scale = 1;

	// Copy pool sizes to new pool structure:
	Mem_Pools = Make_Mem(sizeof(REBPOL) * MAX_POOLS);
	for (n = 0; n < MAX_POOLS; n++) {
		Mem_Pools[n].wide = Mem_Pool_Spec[n].wide;
		Mem_Pools[n].units = (Mem_Pool_Spec[n].units * scale) / unscale;
		if (Mem_Pools[n].units < 2) Mem_Pools[n].units = 2;
	}

	// For pool lookup. Maps size to pool index. (See Find_Pool below)
	PG_Pool_Map = Make_Mem((4 * MEM_BIG_SIZE) + 4); // extra
	n = 9;  // sizes 0 - 8 are pool 0
	for (; n <= 16 * MEM_MIN_SIZE; n++) PG_Pool_Map[n] = MEM_TINY_POOL     + ((n-1) / MEM_MIN_SIZE);
	for (; n <= LAST_SMALL_SIZE * MEM_MIN_SIZE; n++) PG_Pool_Map[n] = MEM_SMALL_POOLS-4 + ((n-1) / (MEM_MIN_SIZE * 4));
	for (; n <=  4 * MEM_BIG_SIZE; n++) PG_Pool_Map[n] = MEM_MID_POOLS     + ((n-1) / MEM_BIG_SIZE);
}


#ifndef POOL_MAP
/***********************************************************************
**
*/	static INLINE REBCNT Find_Pool(REBCNT size)
/*
**		Given a size, tell us what pool it belongs to.
**
***********************************************************************/
{
	if (size <= 8) return 0;  // Note: 0 - 8 (and size change for proper modulus)
	size--;
	if (size < 16 * MEM_MIN_SIZE) return MEM_TINY_POOL   + (size / MEM_MIN_SIZE);
	if (size < 32 * MEM_MIN_SIZE) return MEM_SMALL_POOLS-4 + (size / (MEM_MIN_SIZE * 4));
	if (size <  4 * MEM_BIG_SIZE) return MEM_MID_POOLS   + (size / MEM_BIG_SIZE);
	return SYSTEM_POOL;
}


/***********************************************************************
**
**	void Check_Pool_Map(void)
/*
************************************************************************
{
	int n;

	for (n = 0; n <= 4 * MEM_BIG_SIZE + 1; n++)
		if (FIND_POOL(n) != Find_Pool(n))
			Debug_Fmt("%d: %d %d", n, FIND_POOL(n), Find_Pool(n));
}
*/
#endif


/***********************************************************************
**
*/	static void Fill_Pool(REBPOL *pool)
/*
**		Allocate memory for a pool.  The amount allocated will be
**		determined from the size and units specified when the
**		pool header was created.  The nodes of the pool are linked
**		to the free list.
**
***********************************************************************/
{
	REBSEG	*seg;
	REBNOD	*node;
	REBYTE	*next;
	REBCNT	units = pool->units;
#ifdef MUNGWALL
	REBCNT	mem_size = (pool->wide + 2 * MUNG_SIZE) * units + sizeof(REBSEG);
#else
	REBCNT	mem_size = pool->wide * units + sizeof(REBSEG);
#endif

	seg = (REBSEG *) Make_Mem(mem_size);
	if (!seg) Crash(RP_NO_MEMORY, mem_size);

	CLEAR(seg, mem_size);  // needed to clear series nodes
	seg->size = mem_size;
	seg->next = pool->segs;
   	pool->segs = seg;
	pool->free += units;
	pool->has += units;

	// Add new nodes to the end of free list:
	for (node = (REBNOD *)&pool->first; *node; node = *node);	// goto end

#ifdef MUNGWALL
	for (next = (REBYTE *)(seg + 1); units > 0; units--) {
		memcpy(next,MUNG_PATTERN1,MUNG_SIZE);
		memcpy(next+MUNG_SIZE+pool->wide,MUNG_PATTERN2,MUNG_SIZE);
		*node = (REBNOD) (next+MUNG_SIZE);
		node  = *node;
		next+=pool->wide+2*MUNG_SIZE;
	}
#else
	for (next = (REBYTE *)(seg + 1); units > 0; units--, next += pool->wide) {
		*node = (REBNOD) next;
		node  = *node;
	}
#endif
	*node = 0;
}


/***********************************************************************
**
*/	void *Make_Node(REBCNT pool_id)
/*
**		Allocate a node from a pool.  The node will NOT be cleared.
**		If the pool has run out of nodes, it will be refilled.
**
***********************************************************************/
{
	REBNOD *node;
	REBPOL *pool;

	pool = &Mem_Pools[pool_id];
	if (!pool->first) Fill_Pool(pool);
	node = pool->first;
	pool->first = *node;
#ifdef WATCH_SERIES_POOL
	printf(cs_cast("*** SERIES_POOL Make_Node=> has: %u free: %u\n"), Mem_Pools[SERIES_POOL].has, Mem_Pools[SERIES_POOL].free);
#endif
	pool->free--;
	return (void *)node;
}


/***********************************************************************
**
*/	void Free_Node(REBCNT pool_id, REBNOD *node)
/*
**		Free a node, returning it to its pool.
**
***********************************************************************/
{
	MUNG_CHECK(pool_id, node, Mem_Pools[pool_id].wide);
	*node = Mem_Pools[pool_id].first;
	Mem_Pools[pool_id].first = node;
	Mem_Pools[pool_id].free++;
#ifdef WATCH_SERIES_POOL
	if(pool_id == SERIES_POOL) {
		//if(Mem_Pools[SERIES_POOL].has == Mem_Pools[SERIES_POOL].free) {
		//	puts("last?");
		//}
		printf(cs_cast("*** SERIES_POOL Free_Node=> has: %u free: %u\n"), Mem_Pools[SERIES_POOL].has, Mem_Pools[SERIES_POOL].free);
	}
#endif
}


/***********************************************************************
**
*/	REBSER *Make_Series_Data(REBSER *series, REBCNT length)
/*
**		Allocates memory for series data of the given width
**		and length (number of units).
**
**		Can be used by Make_Series below once we measure to
**		determine performance impact.  !!!
**
***********************************************************************/
{
	REBNOD *node;
	REBPOL *pool;
	REBCNT pool_num;

//	if (GC_TRIGGER) Recycle(FALSE);

	length *= SERIES_WIDE(series);
	pool_num = FIND_POOL(length);
	if (pool_num < SYSTEM_POOL) {
		pool = &Mem_Pools[pool_num];
		if (!pool->first) Fill_Pool(pool);
		node = pool->first;
		pool->first = *node;
		pool->free--;
		length = pool->wide;
#ifdef WATCH_SERIES_POOL
		if(pool_num == SERIES_POOL) printf(cs_cast("*** SERIES_POOL Make_Series_Data=> has: %u free: %u (size: %u)\n"), Mem_Pools[SERIES_POOL].has, Mem_Pools[SERIES_POOL].free, length);
#endif
	} else {
		length = ALIGN(length, 2048);
#ifdef DEBUGGING
		Debug_Fmt_Num("Alloc1:", length);
#endif
#ifdef MUNGWALL
		node = (REBNOD *) Make_Mem(length+2*MUNG_SIZE);
#else
		node = (REBNOD *) Make_Mem(length);
#endif
		if (!node) Trap0(RE_NO_MEMORY);
#ifdef MUNGWALL
		memcpy((REBYTE *)node,MUNG_PATTERN1,MUNG_SIZE);
		memcpy(((REBYTE *)node)+length+MUNG_SIZE,MUNG_PATTERN2,MUNG_SIZE);
		node=(REBNOD *)(((REBYTE *)node)+MUNG_SIZE);
#endif
		Mem_Pools[SYSTEM_POOL].has += length;
		Mem_Pools[SYSTEM_POOL].free++;
#ifdef WATCH_SYSTEM_POOL
		printf(cs_cast("*** SYSTEM_POOL Make_Series_Data=> has: %u free: %u (size: %u)\n"), Mem_Pools[SYSTEM_POOL].has, Mem_Pools[SYSTEM_POOL].free, length);
#endif
	}
#ifdef CHAFF
	memset((REBYTE *)node, 0xff, length);
#endif
	series->tail = 0;
	SERIES_REST(series) = length / SERIES_WIDE(series);
	series->data = (REBYTE *)node;
	if ((GC_Ballast -= length) <= 0) SET_SIGNAL(SIG_RECYCLE);
	return series;
}


/***********************************************************************
**
*/	REBSER *Make_Series(REBCNT length, REBCNT wide, REBOOL powerof2)
/*
**		Make a series of a given length and width (unit size).
**		Small series will be allocated from a REBOL pool.
**		Large series will be allocated from system memory.
**		A width of zero is not allowed.
**
***********************************************************************/
{
	REBSER *series;
	REBNOD *node;
	REBPOL *pool;
	REBCNT pool_num;

	CHECK_STACK(&series);

	if (((REBU64)length * wide) > MAX_I32) Trap0(RE_NO_MEMORY);

	ASSERT(wide != 0, RP_BAD_SERIES);

//	if (GC_TRIGGER) Recycle(FALSE);

	series = (REBSER *)Make_Node(SERIES_POOL);
	length *= wide;
	ASSERT(length != 0, RP_BAD_SERIES);

	pool_num = FIND_POOL(length);
	if (pool_num < SYSTEM_POOL) {
		pool = &Mem_Pools[pool_num];
		if (!pool->first) Fill_Pool(pool);
		node = pool->first;
		pool->first = *node;
		pool->free--;
		length = pool->wide;
		memset(node, 0, length);
#ifdef WATCH_SERIES_POOL
		if(pool_num == SERIES_POOL) printf(cs_cast("*** SERIES_POOL Make_Series=> has: %u free: %u (size: %u)\n"), Mem_Pools[SERIES_POOL].has, Mem_Pools[SERIES_POOL].free, length);
#endif
	} else {
		if (powerof2) {
			U32_ROUND_UP_POWER_OF_2(length);
		} else
			length = ALIGN(length, 2048);
#ifdef DEBUGGING
			Debug_Num("Alloc2:", length);
#endif
#ifdef MUNGWALL
		node = (REBNOD *) Make_Mem(length+2*MUNG_SIZE);
#else
		node = (REBNOD *) Make_Mem(length);
#endif
		if (!node) {
			Free_Node(SERIES_POOL, (REBNOD *)series);
			Trap0(RE_NO_MEMORY);
		}
#ifdef MUNGWALL
		memcpy((REBYTE *)node,MUNG_PATTERN1,MUNG_SIZE);
		memcpy(((REBYTE *)node)+length+MUNG_SIZE,MUNG_PATTERN2,MUNG_SIZE);
		node=(REBNOD *)(((REBYTE *)node)+MUNG_SIZE);
#endif
		Mem_Pools[SYSTEM_POOL].has += length;
		Mem_Pools[SYSTEM_POOL].free++;
#ifdef WATCH_SYSTEM_POOL
		printf(cs_cast("*** SYSTEM_POOL Make_Series => has: %u free: %u (size: %u)\n"), Mem_Pools[SYSTEM_POOL].has, Mem_Pools[SYSTEM_POOL].free, length);
#endif
	}
#ifdef CHAFF
	memset((REBYTE *)node, 0xff, length);
#endif
	series->tail = series->size = 0;
	SERIES_REST(series) = length / wide; //FIXME: This is based on the assumption that length is multiple of wide
	series->data = (REBYTE *)node;
	series->sizes = wide; // also clears bias
	SERIES_FLAGS(series) = 0;
	LABEL_SERIES(series, "make");

	if ((GC_Ballast -= length) <= 0) SET_SIGNAL(SIG_RECYCLE);

	// Keep the last few series in the nursery, safe from GC:
	if (GC_Last_Infant >= MAX_SAFE_SERIES) GC_Last_Infant = 0;
	GC_Infants[GC_Last_Infant++] = series;

	CHECK_MEMORY(2);

	PG_Reb_Stats->Series_Made++;
	PG_Reb_Stats->Series_Memory += length;

	return series;
}


/***********************************************************************
**
*/	void Free_Series_Data(REBSER *series, REBOOL protect)
/*
**		Free series data, but leave series header. Protect flag
**		can be used to prevent GC away from the data field.
**
***********************************************************************/
{
	REBNOD *node;
	REBPOL *pool;
	REBCNT pool_num;
	REBCNT size;

	// !!!! Dump_Series(series, "Free-Data");

	if (SERIES_FREED(series) || series->data == BAD_MEM_PTR) return; // No free twice.
	if (IS_EXT_SERIES(series)) goto clear_header;  // Must be library related

	size = SERIES_TOTAL(series);
	if ((GC_Ballast += size) > VAL_INT32(TASK_BALLAST))
		GC_Ballast = VAL_INT32(TASK_BALLAST);

	// GC may no longer be necessary:
	if (GC_Ballast > 0) CLR_SIGNAL(SIG_RECYCLE);

	series->data -= SERIES_WIDE(series) * SERIES_BIAS(series);
	node = (REBNOD *)series->data;
	pool_num = FIND_POOL(size);

	if (GC_Stay_Dirty) {
		memset(series->data, 0xbb, size);
		return;
	}

	// Verify that size matches pool size:
	if (pool_num < SERIES_POOL) {
		/* size < wide when "wide" is not a multiple of element size */
		ASSERT(Mem_Pools[pool_num].wide >= size, RP_FREE_NODE_SIZE);
	}
	MUNG_CHECK(pool_num,node, size);

	if (pool_num < SYSTEM_POOL) {
		pool = &Mem_Pools[pool_num];
		*node = pool->first;
		pool->first = node;
		pool->free++;
#ifdef WATCH_SERIES_POOL
		if(pool_num == SERIES_POOL) printf(cs_cast("*** SERIES_POOL Free_Series_Data=> has: %u free: %u (size: %u)\n"), Mem_Pools[SERIES_POOL].has, Mem_Pools[SERIES_POOL].free, size);
#endif
	} else {
#ifdef MUNGWALL
		Free_Mem(((REBYTE *)node)-MUNG_SIZE, size + MUNG_SIZE*2);
#else
		Free_Mem(node, size);
#endif
		Mem_Pools[SYSTEM_POOL].has -= size;
		Mem_Pools[SYSTEM_POOL].free--;
#ifdef WATCH_SYSTEM_POOL
		printf(cs_cast("*** SYSTEM_POOL Free_Series_Data=> has: %u free: %u (size: %u)\n"), Mem_Pools[SYSTEM_POOL].has, Mem_Pools[SYSTEM_POOL].free, size);
#endif
	}

	CHECK_MEMORY(2);

clear_header:
	if (protect) {
		series->data = BAD_MEM_PTR; // force bad references to trap
		series->sizes = 0;  // indicates series deallocated (wide = 0)
		series->flags = 0;
	}
}


/***********************************************************************
**
*/	void Free_Series(REBSER *series)
/*
**		Free a series, returning its memory for reuse.
**
***********************************************************************/
{
	REBCNT n;
#ifdef WATCH_SERIES_POOL
	if (SERIES_FREED(series)) {
		puts("series already free!");
	}
#endif
	PG_Reb_Stats->Series_Freed++;
	PG_Reb_Stats->Series_Memory -= SERIES_TOTAL(series);

	// Remove series from expansion list, if found:
	for (n = 1; n < MAX_EXPAND_LIST; n++) {
		if (Prior_Expand[n] == series) Prior_Expand[n] = 0;
	}

	if (!IS_EXT_SERIES(series)) {
		Free_Series_Data(series, TRUE);
	}
	series->sizes = 0; // includes bias and width
	series->flags = 0;
	//series->data = BAD_MEM_PTR;
	//series->tail = 0xBAD2BAD2;
	//series->size = 0xBAD3BAD3;

	Free_Node(SERIES_POOL, (REBNOD *)series);

	/* remove from GC_Infants */
	for (n = 0; n < MAX_SAFE_SERIES; n++) {
		if (GC_Infants[n] == series)
			GC_Infants[n] = NULL;
	}

/* Old torture mode:
	if (!SERIES_FREED(series)) { // Don't try to free twice.
		MUNG_CHECK(SERIES_POOL, (REBNOD *)series, Mem_Pools[SERIES_POOL].wide);
		FREE_SERIES(series); // special GC mark as freed
	}
*/
}


/***********************************************************************
**
*/	void Free_Gob(REBGOB *gob)
/*
**		Free a gob, returning its memory for reuse.
**
***********************************************************************/
{
	FREE_GOB(gob);

	Free_Node(GOB_POOL, (REBNOD *)gob);
}


/***********************************************************************
**
*/	int Free_Hob(REBHOB *hob)
/*
**		Free a hob, returning its memory for reuse.
**
***********************************************************************/
{
	REBHSP spec;
	REBCNT idx = hob->index;

	if( !IS_USED_HOB(hob) || hob->data == NULL ) return 0;

	spec = PG_Handles[idx];
	//printf("HOB %p free mem: %p %i\n", hob, hob->data, spec.flags);

	if (spec.free) {
		if (spec.flags & HANDLE_REQUIRES_HOB_ON_FREE) {
			spec.free((void*)hob);
			// Although there are no references, the extension may still need the handle.
			// If extension marks the hob, do not free it now.
			if (IS_MARK_HOB(hob)) return 0;
		}
		else {
			spec.free(hob->data);
		}
	}
	
	CLEAR(hob->data, spec.size); 
	FREE_MEM(hob->data);
	UNUSE_HOB(hob);
	Free_Node(HOB_POOL, (REBNOD *)hob);
	return 1;
}


/***********************************************************************
**
*/	void Prop_Series(REBSER *newser, REBSER *oldser)
/*
**		Propagate a series from another.
**
***********************************************************************/
{
	newser->sizes = oldser->sizes;
	newser->flags = oldser->flags;
	newser->all = oldser->all;
#ifdef SERIES_LABELS
	newser->label = oldser->label;
#endif
}


/***********************************************************************
**
*/	REBFLG Series_In_Pool(REBSER *series)
/*
**		Confirm that the series value is in the series pool.
**
***********************************************************************/
{
	REBSEG	*seg;
	REBSER *start;

	// Scan all series headers to check that series->size is correct:
	for (seg = Mem_Pools[SERIES_POOL].segs; seg; seg = seg->next) {
		start = (REBSER *) (seg + 1);
		if (series >= start && series <= (REBSER*)((REBYTE*)start + seg->size - sizeof(REBSER)))
			return TRUE;
	}

	return FALSE;
}


/***********************************************************************
**
*/	REBCNT Check_Memory(void)
/*
**		FOR DEBUGGING ONLY:
**		Traverse the free lists of all pools -- just to prove we can.
**		This is useful for finding corruption from bad memory writes,
**		because a write past the end of a node will destory the pointer
**		for the next free area.
**
***********************************************************************/
{
	REBCNT pool_num;
	REBNOD *node;
	//REBNOD *pnode;
	REBCNT count = 0;
	REBSEG *seg;
	REBSER *series;

	//Debug_Str("<ChkMem>");
	PG_Reb_Stats->Free_List_Checked++;

	// Scan all series headers to check that series->size is correct:
	for (seg = Mem_Pools[SERIES_POOL].segs; seg; seg = seg->next) {
		series = (REBSER *) (seg + 1);
		for (count = Mem_Pools[SERIES_POOL].units; count > 0; count--) {
			SKIP_WALL(series);
			MUNG_CHECK(SERIES_POOL, series, sizeof(*series));
			if (!SERIES_FREED(series)) {
				if (!SERIES_REST(series) || !series->data)
					goto crash;
				// Does the size match a known pool?
				pool_num = FIND_POOL(SERIES_TOTAL(series));
				// Just to be sure the pool size is enough to hold total series length
				// Originaly it was expecting exact size match, but there may be cases
				// where series' total size may be lower than pool wide.
				if (pool_num < SERIES_POOL && Mem_Pools[pool_num].wide < SERIES_TOTAL(series))
					goto crash;
			}
			series++;
			SKIP_WALL(series);
		}
	}

	// Scan each memory pool:
	for (pool_num = 0; pool_num < SYSTEM_POOL; pool_num++) {
		count = 0;
		// Check each free node in the memory pool:
		for (node = Mem_Pools[pool_num].first; node; node = *node) {
			count++;
			// The node better belong to one of the pool's segments:
			for (seg = Mem_Pools[pool_num].segs; seg; seg = seg->next) {
				if ((REBUPT)node > (REBUPT)seg && (REBUPT)node < (REBUPT)seg + (REBUPT)seg->size) break;
			}
			if (!seg)
				goto crash;
			//pnode = node; // for debugger
		}
		// The number of free nodes must agree with header:
		if (
			(Mem_Pools[pool_num].free != count) ||
			(Mem_Pools[pool_num].free == 0 && Mem_Pools[pool_num].first != 0)
		)
			goto crash;
	}

	return count;
crash:
	Crash(RP_CORRUPT_MEMORY);
	return 0; // for compiler only
}


/***********************************************************************
**
*/	void Dump_All(REBINT size)
/*
**		Dump all series of a given size.
**
***********************************************************************/
{
	REBSEG	*seg;
	REBSER *series;
	REBCNT count;
	REBCNT n = 0;

	for (seg = Mem_Pools[SERIES_POOL].segs; seg; seg = seg->next) {
		series = (REBSER *) (seg + 1);
		for (count = Mem_Pools[SERIES_POOL].units; count > 0; count--) {
			SKIP_WALL(series);
			if (!SERIES_FREED(series)) {
				if (SERIES_WIDE(series) == size && SERIES_GET_FLAG(series, SER_MON)) {
					//Debug_Fmt("%3d %4d %4d = \"%s\"", n++, series->tail, SERIES_TOTAL(series), series->data);
					Debug_Fmt(cb_cast("%3d %4d %4d = \"%s\""), n++, series->tail, SERIES_REST(series), (SERIES_LABEL(series) ? SERIES_LABEL(series) : "-"));
				}
			}
			series++;
			SKIP_WALL(series);
		}
	}
}

/***********************************************************************
**
*/	void Dump_Series_In_Pool(int pool_id)
/*
**		Dump all series in the pool @pool_id, -1 for all pools
**
***********************************************************************/
{
	REBSEG	*seg;
	REBSER *series;
	REBCNT count;
	//REBCNT n = 0;

	for (seg = Mem_Pools[SERIES_POOL].segs; seg; seg = seg->next) {
		series = (REBSER *) (seg + 1);
		for (count = Mem_Pools[SERIES_POOL].units; count > 0; count--) {
			SKIP_WALL(series);
			if (!SERIES_FREED(series)) {
				if (pool_id < 0 || FIND_POOL(SERIES_TOTAL(series)) == pool_id) {
					Debug_Fmt(
							  Str_Dump[0], //"%s Series %x: Wide: %2d Size: %6d - Bias: %d Tail: %d Rest: %d Flags: %x %s"
							  "Dump",
							  series,
							  SERIES_WIDE(series),
							  SERIES_TOTAL(series),
							  SERIES_BIAS(series),
							  SERIES_TAIL(series),
							  SERIES_REST(series),
							  SERIES_FLAGS(series),
							  (SERIES_LABEL(series) ? SERIES_LABEL(series) : "-")
							 );
					//Dump_Series(series, "Dump");
					if (SERIES_WIDE(series) == sizeof(REBVAL)) {
						Debug_Values(BLK_HEAD(series), SERIES_TAIL(series), 1024); /* FIXME limit */
					} else{
						Dump_Bytes(series->data, (SERIES_TAIL(series)+1) * SERIES_WIDE(series));
					}
				}
			}
			series++;
			SKIP_WALL(series);
		}
	}
}

#ifdef DEBUG_HANDLES
/***********************************************************************
**
*/	void Dump_Handles(void)
/*
**		Dump all series in the pool @pool_id, -1 for all pools
**
***********************************************************************/
{
	REBSEG *seg;
	REBHOB *hob;
	REBCNT count;
	REBCNT n = 0;
	puts("\nUsed handles:\n");
	for (seg = Mem_Pools[HOB_POOL].segs; seg; seg = seg->next, n++) {
		hob = (REBHOB *) (seg + 1);
		printf("seg %u units: %u free: %u\n", n, Mem_Pools[HOB_POOL].units, Mem_Pools[HOB_POOL].free);
		for (count = Mem_Pools[HOB_POOL].units; count > 0; count--) {
			SKIP_WALL_TYPE(hob, REBHOB);
			if (IS_USED_HOB(hob)) {
				printf("hob %s used\n", SYMBOL_TO_NAME(hob->sym));
			}
			hob++;
			SKIP_WALL_TYPE(hob, REBHOB);
		}
	}
}
#endif

/***********************************************************************
**
*/	static void Dump_Pools(void)
/*
**		Print statistics about all memory pools.
**
***********************************************************************/
{
	REBSEG	*seg;
	REBCNT	segs;
	REBCNT	size;
	REBCNT  used;
	REBCNT	total = 0;
	REBCNT  tused = 0;
	REBCNT  n;

	FOREACH(n, SYSTEM_POOL) {
		size = segs = 0;

		for (seg = Mem_Pools[n].segs; seg; seg = seg->next, segs++)
			size += seg->size;

		used = Mem_Pools[n].has - Mem_Pools[n].free;
		Debug_Fmt(cb_cast("Pool[%-2d] %-4dB %-5d/%-5d:%-4d (%-2d%%) %-2d segs, %-07d total"),
			n,
			Mem_Pools[n].wide,
			used,
			Mem_Pools[n].has,
			Mem_Pools[n].units,
			Mem_Pools[n].has ? ((used * 100) / Mem_Pools[n].has) : 0,
			segs,
			size
		);

		tused += used * Mem_Pools[n].wide;
		total += size;
	}
	Debug_Fmt(cb_cast("Pools used %d of %d (%2d%%)"), tused, total, (tused*100) / total);
	Debug_Fmt(cb_cast("System pool used %d"), Mem_Pools[SYSTEM_POOL].has);
	//Debug_Fmt("Raw allocator reports %d", PG_Mem_Usage);
#ifdef DEBUG_HANDLES
	Dump_Handles();
#endif
}


/***********************************************************************
**
*/	REBU64 Inspect_Series(REBCNT flags)
/*
***********************************************************************/
{
	REBSEG	*seg;
	REBSER	*series;
	REBCNT  segs, n, tot, blks, strs, unis, nons, odds, fre;
	REBCNT  str_size, uni_size, blk_size, odd_size, seg_size, fre_size;
	REBFLG  f = 0;
	REBINT  pool_num;
#ifdef SERIES_LABELS
	REBYTE  *kind;
#endif
	REBU64  tot_size;

	segs = tot = blks = strs = unis = nons = odds = fre = 0;
	seg_size = str_size = uni_size = blk_size = odd_size = fre_size = 0;
	tot_size = 0;
	DS_TERMINATE;

	for (seg = Mem_Pools[SERIES_POOL].segs; seg; seg = seg->next) {

		seg_size += seg->size;
		segs++;

		series = (REBSER *) (seg + 1);

		for (n = Mem_Pools[SERIES_POOL].units; n > 0; n--) {
			SKIP_WALL(series);
			MUNG_CHECK(SERIES_POOL, series, sizeof(*series));

			if (SERIES_WIDE(series)) {
				tot++;
				tot_size += SERIES_TOTAL(series);
				f = 0;
			} else {
				fre++;
			}

#ifdef SERIES_LABELS
			kind = "----";
			if (SERIES_GET_FLAG(series, SER_KEEP)) kind = "KEEP";
			//if (Find_Root(series)) kind = "ROOT";
			if (!SERIES_FREED(series) && series->label) {
				Debug_Fmt_("%08x: %16s %s ", series, series->label, kind);
				f = 1;
			} else if (!SERIES_FREED(series) && (flags & 0x100)) {
				Debug_Fmt_("%08x: %s ", series, kind);
				f = 1;
			}
#endif
			if (SERIES_WIDE(series) == sizeof(REBVAL)) {
				blks++;
				blk_size += SERIES_TOTAL(series);
				if (f) Debug_Fmt_(cb_cast("BLOCK "));
			}
			else if (SERIES_WIDE(series) == 1) {
				strs++;
				str_size += SERIES_TOTAL(series);
				if (f) Debug_Fmt_(cb_cast("STRING"));
			}
			else if (SERIES_WIDE(series) == sizeof(REBUNI)) {
				unis++;
				uni_size += SERIES_TOTAL(series);
				if (f) Debug_Fmt_(cb_cast("UNICOD"));
			}
			else if (SERIES_WIDE(series)) {
				odds++;
				odd_size += SERIES_TOTAL(series);
				if (f) Debug_Fmt_(cb_cast("ODD[%d]"), SERIES_WIDE(series));
			}
			if (f && SERIES_WIDE(series)) {
				Debug_Fmt(cb_cast(" units: %-5d tail: %-5d bytes: %-7d"), SERIES_REST(series), SERIES_TAIL(series), SERIES_TOTAL(series));
			}

			series++;
			SKIP_WALL(series);
		}
	}

	// Size up unused memory:
	for (pool_num = 0; pool_num < SYSTEM_POOL; pool_num++) {
		fre_size += Mem_Pools[pool_num].free * Mem_Pools[pool_num].wide;
	}

	if (flags & 1) {
		Debug_Fmt(cb_cast(
			  "Series Memory Info:\n"
			  "  node   size = %d\n"
			  "  series size = %d\n"
			  "  %-6d segs = %-7d bytes - headers\n"
			  "  %-6d blks = %-7d bytes - blocks\n"
			  "  %-6d strs = %-7d bytes - byte strings\n"
			  "  %-6d unis = %-7d bytes - unicode strings\n"
			  "  %-6d odds = %-7d bytes - odd series\n"
			  "  %-6d used = %-7d bytes - total used\n"
			  "  %-6d free / %-7d bytes - free headers / node-space\n"
			  ),
			  sizeof(REBVAL),
			  sizeof(REBSER),
			  segs, seg_size,
			  blks, blk_size,
			  strs, str_size,
			  unis, uni_size,
			  odds, odd_size,
			  tot,  tot_size,
			  fre,  fre_size   // the 2 are not related
		);
	}

	if (flags & 2) Dump_Pools();

	return tot_size;
}

/***********************************************************************
**
*/	void Dispose_Hobs(void)
/*
**		Free all HOB pool segments
**
***********************************************************************/
{
	REBSEG	*seg;
	REBHOB *hob;
	REBCNT  n;

	//puts("===== Dispose_Hobs ======");
	// HOB at this moment does not use system series, so handle it separately
	for (seg = Mem_Pools[HOB_POOL].segs; seg; seg = seg->next) {
		hob = (REBHOB *) (seg + 1);
		for (n = Mem_Pools[HOB_POOL].units; n > 0; n--) {
			SKIP_WALL_TYPE(hob, REBHOB);
			if (IS_USED_HOB(hob)) Free_Hob(hob);
			hob++;
			SKIP_WALL_TYPE(hob, REBHOB);
		}
	}
}

/***********************************************************************
**
*/	void Dispose_Pools(void)
/*
**		Free memory pool array when application quits.
**
***********************************************************************/
{
	REBSEG	*seg, *next;
	REBCNT  n;

	//Dump_Pools();
	//Dump_Series_In_Pool(-1);
	//puts("===== Dispose_Pools ======");

	// Release all series from all system pools
	FOREACH(n, SYSTEM_POOL) {
		//printf(cs_cast("*** Dispose_Pools[%u] Has: %u free: %u\n"), n, Mem_Pools[n].has, Mem_Pools[n].free);
		if (Mem_Pools[n].has == Mem_Pools[n].free) {
			seg = Mem_Pools[n].segs;
			while (seg) {
				next = seg->next;
				Free_Mem(seg, seg->size);
				seg = next;
			}
		}
		//else {
		//	printf(cs_cast("!!! Mem_Pools[%u] not empty! Has: %u free: %u\n"), n, Mem_Pools[n].has, Mem_Pools[n].free);
		//}
	}
	// SYSTEM_POOL contains not system series sizes (big series), at this state it should be empty!
	//if (Mem_Pools[SYSTEM_POOL].has > 0)
	//	printf(cs_cast("!!! Mem_Pools[SYSTEM_POOL].has: %u\n"), Mem_Pools[SYSTEM_POOL].has);
	Free_Mem(Mem_Pools, 0);
	Free_Mem(PG_Pool_Map, 0);
}
