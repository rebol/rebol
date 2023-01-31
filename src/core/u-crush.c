/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2023 Rebol Open Source Contributors
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
**  Module:  u-crush.c
**  Summary: custom Crush compression algorithm
**  Section: natives
**  Author:  Oldes, Ilya Muravyov
**  Notes:
**		This code was ported from the original Ilya Muravyov's code,
**		which he placed in the public domain.
**		https://compressme.net/
**		https://sourceforge.net/projects/crush
**
***********************************************************************/
// Original credits:
// crush.cpp
// Written and placed in the public domain by Ilya Muravyov
//

#include "sys-core.h"
#ifdef INCLUDE_CRUSH

#ifdef _MSC_VER
#define _CRT_DISABLE_PERFCRIT_LOCKS
#endif

typedef struct crush_ctx {
	REBINT  bit_buf;
	REBINT  bit_count;
	REBYTE *data;  // used as output in compression and as input in decompression
	REBCNT  index;
} CRUSH;

// Bit I/O
//

static inline void put_bits(CRUSH *c, int n, int x) {
	c->bit_buf |= x << c->bit_count;
	c->bit_count += n;
	while (c->bit_count >= 8) {
		c->data[c->index++] = c->bit_buf;
		c->bit_buf >>= 8;
		c->bit_count -= 8;
	}
}

static inline int get_bits(CRUSH *c, int n) {
	while (c->bit_count < n) {
		c->bit_buf |= c->data[c->index++] << c->bit_count;
		c->bit_count += 8;
	}
	const int x = c->bit_buf & ((1 << n) - 1);
	c->bit_buf >>= n;
	c->bit_count -= n;
	return x;
}

// LZ77
//

#define CRUSH_SLOT_BITS 	4
#define CRUSH_NUM_SLOTS 	16			// 1 << CRUSH_SLOT_BITS
#define CRUSH_A_BITS		2			// 1 xx
#define CRUSH_B_BITS		2			// 01 xx
#define CRUSH_C_BITS		2			// 001 xx
#define CRUSH_D_BITS		3			// 0001 xxx
#define CRUSH_E_BITS		5			// 00001 xxxxx
#define CRUSH_F_BITS		9			// 00000 xxxxxxxxx
#define CRUSH_A				4			//  1 << A_BITS;
#define CRUSH_B				8			// (1 << B_BITS) + A
#define CRUSH_C				12			// (1 << C_BITS) + B
#define CRUSH_D				20			// (1 << D_BITS) + C
#define CRUSH_E				52			// (1 << E_BITS) + D
#define CRUSH_F				564			// (1 << F_BITS) + E
#define CRUSH_MIN_MATCH 	3
#define CRUSH_MAX_MATCH 	566			// (CRUSH_F - 1) + CRUSH_MIN_MATCH
#define CRUSH_TOO_FAR		65536		// 1 << 16
#define CRUSH_HASH1_LEN		3			// CRUSH_MIN_MATCH
#define CRUSH_HASH2_LEN		4			// CRUSH_MIN_MATCH + 1

//#define ORIGINAL_CRUSH_CONSTANTS
#ifdef ORIGINAL_CRUSH_CONSTANTS
#define CRUSH_W_BITS 		21			// window size (17..23)
#define CRUSH_W_SIZE 		2097152		// 1 << CRUSH_W_BITS
#define CRUSH_W_MASK 		2097151		// CRUSH_W_SIZE - 1
#define CRUSH_HASH1_BITS	21
#define CRUSH_HASH2_BITS	24
#define CRUSH_HASH1_SIZE	2097152		// 1 << CRUSH_HASH1_BITS
#define CRUSH_HASH2_SIZE	16777216	// 1 << CRUSH_HASH2_BITS
#define CRUSH_HASH1_MASK	2097151		// CRUSH_HASH1_SIZE - 1
#define CRUSH_HASH2_MASK	16777215	// CRUSH_HASH2_SIZE - 1
#define CRUSH_HASH1_SHIFT	7           // (CRUSH_HASH1_BITS + (CRUSH_HASH1_LEN - 1)) / CRUSH_HASH1_LEN
#define CRUSH_HASH2_SHIFT	6           // (CRUSH_HASH2_BITS + (CRUSH_HASH2_LEN - 1)) / CRUSH_HASH2_LEN
#else
// constants used in Red language code (less memory, better results when compressing Redbol like data)
#define CRUSH_W_BITS 		18			// window size (17..23)
#define CRUSH_W_SIZE 		262144		// 1 << CRUSH_W_BITS
#define CRUSH_W_MASK 		262143		// CRUSH_W_SIZE - 1
#define CRUSH_HASH1_BITS	19
#define CRUSH_HASH2_BITS	20
#define CRUSH_HASH1_SIZE	524288		// 1 << CRUSH_HASH1_BITS
#define CRUSH_HASH2_SIZE	1048576		// 1 << CRUSH_HASH2_BITS
#define CRUSH_HASH1_MASK	524287		// CRUSH_HASH1_SIZE - 1
#define CRUSH_HASH2_MASK	1048575		// CRUSH_HASH2_SIZE - 1
#define CRUSH_HASH1_SHIFT	7           // (CRUSH_HASH1_BITS + (CRUSH_HASH1_LEN - 1)) / CRUSH_HASH1_LEN
#define CRUSH_HASH2_SHIFT	5           // (CRUSH_HASH2_BITS + (CRUSH_HASH2_LEN - 1)) / CRUSH_HASH2_LEN
#endif

static inline int update_hash1(int h, int c) {
	return ((h << CRUSH_HASH1_SHIFT) + c) & CRUSH_HASH1_MASK;
}

static inline int update_hash2(int h, int c) {
	return ((h << CRUSH_HASH2_SHIFT) + c) & CRUSH_HASH2_MASK;
}

static inline int get_penalty(int a, int b) {
	int p=0;
	while (a > b) {
		a >>= 3;
		++p;
	}
	return p;
}

/***********************************************************************
**
*/	REBSER *CompressCrush(REBSER *input, REBINT index, REBCNT size, int level)
/*
***********************************************************************/
{
	CRUSH  ctx;
	REBSER *ser;
	REBYTE *buf = BIN_SKIP(input, index);
	REBCNT tail = 0;
	REBCNT p;
	REBINT len, offset, chain_len, s, i;
	REBINT h1 = 0, h2 = 0;

	static int head[CRUSH_HASH1_SIZE + CRUSH_HASH2_SIZE];
	static int prev[CRUSH_W_SIZE];
	const  int max_chain[]={4, 256, 1<<12};

	level = MAX(0, MIN(2, level));

	ser = Make_Series(MAX(16, size+4), sizeof(REBYTE), FALSE);
	ctx.data = BIN_HEAD(ser);

	((REBCNT *)ctx.data)[0] = size;
	ctx.index = sizeof(REBCNT);
	ctx.bit_count = 0;
	ctx.bit_buf = 0;

	for (i = 0; i < CRUSH_HASH1_SIZE + CRUSH_HASH2_SIZE; ++i)
		head[i] = -1;


	for (i = 0; i < CRUSH_HASH1_LEN; ++i)
		h1 = update_hash1(h1, buf[i]);
	for (i = 0; i < CRUSH_HASH2_LEN; ++i)
		h2 = update_hash2(h2, buf[i]);

	p = 0;
	while (p < size)
	{
		len = CRUSH_MIN_MATCH - 1;
		offset = CRUSH_W_SIZE;

		const int max_match = MIN(CRUSH_MAX_MATCH, size - p);
		const int limit = (p <= CRUSH_W_SIZE) ? 0 : p - CRUSH_W_SIZE;

		if (head[h1] >= limit)
		{
			int s = head[h1];
			if (buf[s] == buf[p])
			{
				int l = 0;
				while (++l < max_match)
					if (buf[s + l] != buf[p + l])
						break;
				if (l > len)
				{
					len = l;
					offset = p - s;
				}
			}
		}

		if (len < CRUSH_MAX_MATCH)
		{
			chain_len = max_chain[level];
			s = head[h2 + CRUSH_HASH1_SIZE];

			while ((chain_len-- != 0) && (s >= limit))
			{
				if ((buf[s + len] == buf[p + len]) && (buf[s] == buf[p]))
				{
					i = 0;
					while (++i < max_match)
						if (buf[s + i] != buf[p + i])
							break;
					if (i > len + get_penalty((p - s) >> 4, offset))
					{
						len = i;
						offset = p - s;
					}
					if (i == max_match)
						break;
				}
				s = prev[s & CRUSH_W_MASK];
			}
		}

		if ((len == CRUSH_MIN_MATCH) && (offset > CRUSH_TOO_FAR))
			len = 0;

		if ((level >= 2) && (len >= CRUSH_MIN_MATCH) && (len < max_match))
		{
			const int next_p = p + 1;
			const int max_lazy = MIN(len + 4, max_match);

			chain_len = max_chain[level];
			s = head[update_hash2(h2, buf[next_p + (CRUSH_HASH2_LEN - 1)]) + CRUSH_HASH1_SIZE];

			while ((chain_len-- != 0) && (s >= limit))
			{
				if ((buf[s + len] == buf[next_p + len]) && (buf[s] == buf[next_p]))
				{
					i = 0;
					while (++i < max_lazy)
						if (buf[s + i] != buf[next_p + i])
							break;
					if (i > len + get_penalty(next_p - s, offset))
					{
						len = 0;
						break;
					}
					if (i == max_lazy)
						break;
				}
				s = prev[s & CRUSH_W_MASK];
			}
		}

		// If input is already well compressed, output from the Crush may be larger!
		// Instead of adding this check to each `put_bits` call, count with some hopefuly safe range (16bytes)
		if (ctx.index+16 >= SERIES_REST(ser)) {
			SERIES_TAIL(ser) = ctx.index;
			Expand_Series(ser, ctx.index, MAX(16, SERIES_REST(ser) >> 3)); // using 1/4 of the current size for the delta
			ctx.data = BIN_DATA(ser);
		}

		if (len >= CRUSH_MIN_MATCH) // Match
		{
			put_bits(&ctx, 1, 1);

			const int l = len - CRUSH_MIN_MATCH;
			if (l < CRUSH_A)
			{
				put_bits(&ctx, 1, 1); // 1
				put_bits(&ctx, CRUSH_A_BITS, l);
			}
			else if (l < CRUSH_B)
			{
				put_bits(&ctx, 2, 1 << 1); // 01
				put_bits(&ctx, CRUSH_B_BITS, l - CRUSH_A);
			}
			else if (l < CRUSH_C)
			{
				put_bits(&ctx, 3, 1 << 2); // 001
				put_bits(&ctx, CRUSH_C_BITS, l - CRUSH_B);
			}
			else if (l < CRUSH_D)
			{
				put_bits(&ctx, 4, 1 << 3); // 0001
				put_bits(&ctx, CRUSH_D_BITS, l - CRUSH_C);
			}
			else if (l < CRUSH_E)
			{
				put_bits(&ctx, 5, 1 << 4); // 00001
				put_bits(&ctx, CRUSH_E_BITS, l - CRUSH_D);
			}
			else
			{
				put_bits(&ctx, 5, 0); // 00000
				put_bits(&ctx, CRUSH_F_BITS, l - CRUSH_E);
			}

			--offset;
			int log = CRUSH_W_BITS - CRUSH_NUM_SLOTS;
			while (offset >= (2 << log))
				++log;
			put_bits(&ctx, CRUSH_SLOT_BITS, log - (CRUSH_W_BITS - CRUSH_NUM_SLOTS));
			if (log > (CRUSH_W_BITS - CRUSH_NUM_SLOTS)) {
				put_bits(&ctx, log, offset - (1 << log));
			}
			else {
				put_bits(&ctx, CRUSH_W_BITS - (CRUSH_NUM_SLOTS - 1), offset);
			}
		}
		else // Literal
		{
			len = 1;
			put_bits(&ctx, 9, buf[p] << 1); // 0 xxxxxxxx
		}

		while (len-- != 0) // Insert new strings
		{
			head[h1] = p;
			prev[p & CRUSH_W_MASK] = head[h2 + CRUSH_HASH1_SIZE];
			head[h2 + CRUSH_HASH1_SIZE] = p;
			++p;
			h1 = update_hash1(h1, buf[p + (CRUSH_HASH1_LEN - 1)]);
			h2 = update_hash2(h2, buf[p + (CRUSH_HASH2_LEN - 1)]);
		}
	}
	//flush_bits...
	put_bits(&ctx, 7, 0);
	SERIES_TAIL(ser) = ctx.index;
	return ser;
}

/***********************************************************************
**
*/	REBSER *DecompressCrush(REBSER *data, REBINT index, REBCNT length, REBCNT limit)
/*
***********************************************************************/
{
	REBYTE *buf;
	REBSER *ser;
	REBCNT size, p;
	REBINT len, s;
	CRUSH  ctx;

	if(length < 4) Trap1(RE_BAD_PRESS, DS_ARG(2));

	ctx.data = BIN_SKIP(data, index);
	size = ((REBCNT *)ctx.data)[0]; // reads expected decompressed length
	ctx.index = sizeof(REBCNT);
	ctx.bit_count = 0;
	ctx.bit_buf = 0;

	// allow partial output if specified...
	if (limit && size > limit) size = limit;
	
	ser = Make_Binary(size); // output series
	buf = BIN_HEAD(ser);     // output binary
	
	p = 0;
	while (p < size)
	{
		if (get_bits(&ctx, 1))
		{
			if (get_bits(&ctx, 1))
				len = get_bits(&ctx, CRUSH_A_BITS);
			else if (get_bits(&ctx, 1))
				len = get_bits(&ctx, CRUSH_B_BITS) + CRUSH_A;
			else if (get_bits(&ctx, 1))
				len = get_bits(&ctx, CRUSH_C_BITS) + CRUSH_B;
			else if (get_bits(&ctx, 1))
				len = get_bits(&ctx, CRUSH_D_BITS) + CRUSH_C;
			else if (get_bits(&ctx, 1))
				len = get_bits(&ctx, CRUSH_E_BITS) + CRUSH_D;
			else
				len = get_bits(&ctx, CRUSH_F_BITS) + CRUSH_E;

			const int log = get_bits(&ctx, CRUSH_SLOT_BITS) + (CRUSH_W_BITS - CRUSH_NUM_SLOTS);
			s = ~(log > (CRUSH_W_BITS - CRUSH_NUM_SLOTS)
				? get_bits(&ctx, log) + (1 << log)
				: get_bits(&ctx, CRUSH_W_BITS - (CRUSH_NUM_SLOTS - 1))) + p;
			if (s < 0)
			{
				Trap1(RE_BAD_PRESS, DS_ARG(2));
				return NULL;
			}

			buf[p++] = buf[s++];
			buf[p++] = buf[s++];
			buf[p++] = buf[s++];
			while (len-- != 0)
				buf[p++] = buf[s++];
		}
		else
			buf[p++] = get_bits(&ctx, 8);
	}

	SERIES_TAIL(ser) = size;
	return ser;
}
#endif //INCLUDE_CRUSH
