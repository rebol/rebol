/*
	poly1305 implementation using 64 bit * 64 bit = 128 bit multiplication and 128 bit addition
*/

#if defined(_MSC_VER)
	#include <intrin.h>

	typedef struct uint128_t {
		u64 lo;
		u64 hi;
	} uint128_t;

	#define MUL(out, x, y) out.lo = _umul128((x), (y), &out.hi)
	#define ADD(out, in) { u64 t = out.lo; out.lo += in.lo; out.hi += (out.lo < t) + in.hi; }
	#define ADDLO(out, in) { u64 t = out.lo; out.lo += in; out.hi += (out.lo < t); }
	#define SHR(in, shift) (__shiftright128(in.lo, in.hi, (shift)))
	#define LO(in) (in.lo)

	#define POLY1305_NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
	#if defined(__SIZEOF_INT128__)
		typedef unsigned __int128 uint128_t;
	#else
		typedef unsigned uint128_t __attribute__((mode(TI)));
	#endif

	#define MUL(out, x, y) out = ((uint128_t)x * y)
	#define ADD(out, in) out += in
	#define ADDLO(out, in) out += in
	#define SHR(in, shift) (u64)(in >> (shift))
	#define LO(in) (u64)(in)

	#define POLY1305_NOINLINE __attribute__((noinline))
#endif

#define poly1305_block_size 16

/* 17 + sizeof(size_t) + 8*sizeof(u64) */
typedef struct poly1305_state_internal_t {
	u64 r[3];
	u64 h[3];
	u64 pad[2];
	size_t leftover;
	u8 buffer[poly1305_block_size];
	u8 final;
} poly1305_state_internal_t;

void
poly1305_init(poly1305_context *ctx, const u8 key[32]) {
	poly1305_state_internal_t *st = (poly1305_state_internal_t *)ctx;
	u64 t0,t1;

	/* r &= 0xffffffc0ffffffc0ffffffc0fffffff */
	t0 = U8TO64_LE(&key[0]);
	t1 = U8TO64_LE(&key[8]);

	st->r[0] = ( t0                    ) & 0xffc0fffffff;
	st->r[1] = ((t0 >> 44) | (t1 << 20)) & 0xfffffc0ffff;
	st->r[2] = ((t1 >> 24)             ) & 0x00ffffffc0f;

	/* h = 0 */
	st->h[0] = 0;
	st->h[1] = 0;
	st->h[2] = 0;

	/* save pad for later */
	st->pad[0] = U8TO64_LE(&key[16]);
	st->pad[1] = U8TO64_LE(&key[24]);

	st->leftover = 0;
	st->final = 0;
}

static void
poly1305_blocks(poly1305_state_internal_t *st, const u8 *m, size_t bytes) {
	const u64 hibit = (st->final) ? 0 : ((u64)1 << 40); /* 1 << 128 */
	u64 r0,r1,r2;
	u64 s1,s2;
	u64 h0,h1,h2;
	u64 c;
	uint128_t d0,d1,d2,d;

	r0 = st->r[0];
	r1 = st->r[1];
	r2 = st->r[2];

	h0 = st->h[0];
	h1 = st->h[1];
	h2 = st->h[2];

	s1 = r1 * (5 << 2);
	s2 = r2 * (5 << 2);

	while (bytes >= poly1305_block_size) {
		u64 t0,t1;

		/* h += m[i] */
		t0 = U8TO64_LE(&m[0]);
		t1 = U8TO64_LE(&m[8]);

		h0 += (( t0                    ) & 0xfffffffffff);
		h1 += (((t0 >> 44) | (t1 << 20)) & 0xfffffffffff);
		h2 += (((t1 >> 24)             ) & 0x3ffffffffff) | hibit;

		/* h *= r */
		MUL(d0, h0, r0); MUL(d, h1, s2); ADD(d0, d); MUL(d, h2, s1); ADD(d0, d);
		MUL(d1, h0, r1); MUL(d, h1, r0); ADD(d1, d); MUL(d, h2, s2); ADD(d1, d);
		MUL(d2, h0, r2); MUL(d, h1, r1); ADD(d2, d); MUL(d, h2, r0); ADD(d2, d);

		/* (partial) h %= p */
		              c = SHR(d0, 44); h0 = LO(d0) & 0xfffffffffff;
		ADDLO(d1, c); c = SHR(d1, 44); h1 = LO(d1) & 0xfffffffffff;
		ADDLO(d2, c); c = SHR(d2, 42); h2 = LO(d2) & 0x3ffffffffff;
		h0  += c * 5; c = (h0 >> 44);  h0 =    h0  & 0xfffffffffff;
		h1  += c;

		m += poly1305_block_size;
		bytes -= poly1305_block_size;
	}

	st->h[0] = h0;
	st->h[1] = h1;
	st->h[2] = h2;
}


POLY1305_NOINLINE void
poly1305_finish(poly1305_context *ctx, u8 mac[16]) {
	poly1305_state_internal_t *st = (poly1305_state_internal_t *)ctx;
	u64 h0,h1,h2,c;
	u64 g0,g1,g2;
	u64 t0,t1;

	/* process the remaining block */
	if (st->leftover) {
		size_t i = st->leftover;
		st->buffer[i] = 1;
		for (i = i + 1; i < poly1305_block_size; i++)
			st->buffer[i] = 0;
		st->final = 1;
		poly1305_blocks(st, st->buffer, poly1305_block_size);
	}

	/* fully carry h */
	h0 = st->h[0];
	h1 = st->h[1];
	h2 = st->h[2];

	             c = (h1 >> 44); h1 &= 0xfffffffffff;
	h2 += c;     c = (h2 >> 42); h2 &= 0x3ffffffffff;
	h0 += c * 5; c = (h0 >> 44); h0 &= 0xfffffffffff;
	h1 += c;     c = (h1 >> 44); h1 &= 0xfffffffffff;
	h2 += c;     c = (h2 >> 42); h2 &= 0x3ffffffffff;
	h0 += c * 5; c = (h0 >> 44); h0 &= 0xfffffffffff;
	h1 += c;

	/* compute h + -p */
	g0 = h0 + 5; c = (g0 >> 44); g0 &= 0xfffffffffff;
	g1 = h1 + c; c = (g1 >> 44); g1 &= 0xfffffffffff;
	g2 = h2 + c - ((u64)1 << 42);

	/* select h if h < p, or h + -p if h >= p */
	c = (g2 >> ((sizeof(u64) * 8) - 1)) - 1;
	g0 &= c;
	g1 &= c;
	g2 &= c;
	c = ~c;
	h0 = (h0 & c) | g0;
	h1 = (h1 & c) | g1;
	h2 = (h2 & c) | g2;

	/* h = (h + pad) */
	t0 = st->pad[0];
	t1 = st->pad[1];

	h0 += (( t0                    ) & 0xfffffffffff)    ; c = (h0 >> 44); h0 &= 0xfffffffffff;
	h1 += (((t0 >> 44) | (t1 << 20)) & 0xfffffffffff) + c; c = (h1 >> 44); h1 &= 0xfffffffffff;
	h2 += (((t1 >> 24)             ) & 0x3ffffffffff) + c;                 h2 &= 0x3ffffffffff;

	/* mac = h % (2^128) */
	h0 = ((h0      ) | (h1 << 44));
	h1 = ((h1 >> 20) | (h2 << 24));

	U64TO8_LE(&mac[0], h0);
	U64TO8_LE(&mac[8], h1);

	/* zero out the state */
	st->h[0] = 0;
	st->h[1] = 0;
	st->h[2] = 0;
	st->r[0] = 0;
	st->r[1] = 0;
	st->r[2] = 0;
	st->pad[0] = 0;
	st->pad[1] = 0;
}

