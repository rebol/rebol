//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#define MURMUR_HASH_3_X86_32_C1     0xcc9e2d51
#define MURMUR_HASH_3_X86_32_C2     0x1b873593

//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here
FORCE_INLINE REBCNT getblock32(const REBCNT* p, int i) {
    return p[i];
}
/*-----------------------------------------------------------------------------
// Block mix - mix the key block, combine with hash block, mix the hash block,
// repeat. */

FORCE_INLINE void bmix(REBCNT* h1, REBCNT* k1)
{
    *k1 *= MURMUR_HASH_3_X86_32_C1;
    *k1 = ROTL32(*k1, 15);
    *k1 *= MURMUR_HASH_3_X86_32_C2;
    *h1 ^= *k1;
    *h1 = ROTL32(*h1, 13); *h1 = *h1 * 5 + 0xe6546b64;
}
//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche
FORCE_INLINE REBCNT fmix32(REBCNT h) {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}