////////////////////////////////////////////////////////////////////////
// File: u-zlib.c
// Home: https://github.com/Siskin-framework/Rebol-Zlib
// Date: 4-Jan-2024
// Note: This file is amalgamated from these sources:
//
//       crc32.c
//       crc32.h
//       adler32.c
//       deflate.c
//       zutil.c
//       compress.c
//       uncompr.c
//       trees.c
//       trees.h
//       inftrees.h
//       inftrees.c
//       inffast.h
//       inflate.h
//       inffast.c
//       inflate.c
//       inffixed.h
//
////////////////////////////////////////////////////////////////////////

#include "sys-zlib.h"

/* crc32.c -- compute the CRC-32 of a data stream
 * Copyright (C) 1995-2022 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 *
 * This interleaved implementation of a CRC makes use of pipelined multiple
 * arithmetic-logic units, commonly found in modern CPU cores. It is due to
 * Kadatch and Jenkins (2010). See doc/crc-doc.1.0.pdf in this distribution.
 */

/* @(#) $Id$ */

/*
  Note on the use of DYNAMIC_CRC_TABLE: there is no mutex or semaphore
  protection on the static variables used to control the first-use generation
  of the crc tables. Therefore, if you #define DYNAMIC_CRC_TABLE, you should
  first call get_crc_table() to initialize the tables before allowing more than
  one thread to use crc32().

  MAKECRCH can be #defined to write out crc32.h. A main() routine is also
  produced, so that this one source file can be compiled to an executable.
 */

#ifdef MAKECRCH
#  include <stdio.h>
#  ifndef DYNAMIC_CRC_TABLE
#    define DYNAMIC_CRC_TABLE
#  endif /* !DYNAMIC_CRC_TABLE */
#endif /* MAKECRCH */

//REBOL: #include "zutil.h"      /* for Z_U4, Z_U8, z_crc_t, and FAR definitions */

 /*
  A CRC of a message is computed on N braids of words in the message, where
  each word consists of W bytes (4 or 8). If N is 3, for example, then three
  running sparse CRCs are calculated respectively on each braid, at these
  indices in the array of words: 0, 3, 6, ..., 1, 4, 7, ..., and 2, 5, 8, ...
  This is done starting at a word boundary, and continues until as many blocks
  of N * W bytes as are available have been processed. The results are combined
  into a single CRC at the end. For this code, N must be in the range 1..6 and
  W must be 4 or 8. The upper limit on N can be increased if desired by adding
  more #if blocks, extending the patterns apparent in the code. In addition,
  crc32.h would need to be regenerated, if the maximum N value is increased.

  N and W are chosen empirically by benchmarking the execution time on a given
  processor. The choices for N and W below were based on testing on Intel Kaby
  Lake i7, AMD Ryzen 7, ARM Cortex-A57, Sparc64-VII, PowerPC POWER9, and MIPS64
  Octeon II processors. The Intel, AMD, and ARM processors were all fastest
  with N=5, W=8. The Sparc, PowerPC, and MIPS64 were all fastest at N=5, W=4.
  They were all tested with either gcc or clang, all using the -O3 optimization
  level. Your mileage may vary.
 */

/* Define N */
#ifdef Z_TESTN
#  define N Z_TESTN
#else
#  define N 5
#endif
#if N < 1 || N > 6
#  error N must be in 1..6
#endif

/*
  z_crc_t must be at least 32 bits. z_word_t must be at least as long as
  z_crc_t. It is assumed here that z_word_t is either 32 bits or 64 bits, and
  that bytes are eight bits.
 */

/*
  Define W and the associated z_word_t type. If W is not defined, then a
  braided calculation is not used, and the associated tables and code are not
  compiled.
 */
#ifdef Z_TESTW
#  if Z_TESTW-1 != -1
#    define W Z_TESTW
#  endif
#else
#  ifdef MAKECRCH
#    define W 8         /* required for MAKECRCH */
#  else
#    if defined(__x86_64__) || defined(__aarch64__)
#      define W 8
#    else
#      define W 4
#    endif
#  endif
#endif
#ifdef W
#  if W == 8 && defined(Z_U8)
     typedef Z_U8 z_word_t;
#  elif defined(Z_U4)
#    undef W
#    define W 4
     typedef Z_U4 z_word_t;
#  else
#    undef W
#  endif
#endif

/* If available, use the ARM processor CRC32 instruction. */
#if defined(__aarch64__) && defined(__ARM_FEATURE_CRC32) && W == 8
#  define ARMCRC32
#endif

#if defined(W) && (!defined(ARMCRC32) || defined(DYNAMIC_CRC_TABLE))
/*
  Swap the bytes in a z_word_t to convert between little and big endian. Any
  self-respecting compiler will optimize this to a single machine byte-swap
  instruction, if one is available. This assumes that word_t is either 32 bits
  or 64 bits.
 */
local z_word_t byte_swap(z_word_t word) {
#  if W == 8
    return
        (word & 0xff00000000000000) >> 56 |
        (word & 0xff000000000000) >> 40 |
        (word & 0xff0000000000) >> 24 |
        (word & 0xff00000000) >> 8 |
        (word & 0xff000000) << 8 |
        (word & 0xff0000) << 24 |
        (word & 0xff00) << 40 |
        (word & 0xff) << 56;
#  else   /* W == 4 */
    return
        (word & 0xff000000) >> 24 |
        (word & 0xff0000) >> 8 |
        (word & 0xff00) << 8 |
        (word & 0xff) << 24;
#  endif
}
#endif

#ifdef DYNAMIC_CRC_TABLE
/* =========================================================================
 * Table of powers of x for combining CRC-32s, filled in by make_crc_table()
 * below.
 */
   local z_crc_t FAR x2n_table[32];
#else
/* =========================================================================
 * Tables for byte-wise and braided CRC-32 calculations, and a table of powers
 * of x for combining CRC-32s, all made by make_crc_table().
 */
//REBOL: #  include "crc32.h"
/* crc32.h -- tables for rapid CRC calculation
 * Generated automatically by crc32.c
 */

local const z_crc_t FAR crc_table[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
    0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
    0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
    0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
    0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
    0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
    0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
    0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
    0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
    0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
    0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
    0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
    0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
    0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
    0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
    0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
    0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
    0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
    0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
    0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
    0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
    0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
    0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
    0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
    0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
    0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
    0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
    0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
    0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
    0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
    0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
    0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
    0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
    0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
    0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
    0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
    0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
    0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
    0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
    0x2d02ef8d};

#ifdef W

#if W == 8

local const z_word_t FAR crc_big_table[] = {
    0x0000000000000000, 0x9630077700000000, 0x2c610eee00000000,
    0xba51099900000000, 0x19c46d0700000000, 0x8ff46a7000000000,
    0x35a563e900000000, 0xa395649e00000000, 0x3288db0e00000000,
    0xa4b8dc7900000000, 0x1ee9d5e000000000, 0x88d9d29700000000,
    0x2b4cb60900000000, 0xbd7cb17e00000000, 0x072db8e700000000,
    0x911dbf9000000000, 0x6410b71d00000000, 0xf220b06a00000000,
    0x4871b9f300000000, 0xde41be8400000000, 0x7dd4da1a00000000,
    0xebe4dd6d00000000, 0x51b5d4f400000000, 0xc785d38300000000,
    0x56986c1300000000, 0xc0a86b6400000000, 0x7af962fd00000000,
    0xecc9658a00000000, 0x4f5c011400000000, 0xd96c066300000000,
    0x633d0ffa00000000, 0xf50d088d00000000, 0xc8206e3b00000000,
    0x5e10694c00000000, 0xe44160d500000000, 0x727167a200000000,
    0xd1e4033c00000000, 0x47d4044b00000000, 0xfd850dd200000000,
    0x6bb50aa500000000, 0xfaa8b53500000000, 0x6c98b24200000000,
    0xd6c9bbdb00000000, 0x40f9bcac00000000, 0xe36cd83200000000,
    0x755cdf4500000000, 0xcf0dd6dc00000000, 0x593dd1ab00000000,
    0xac30d92600000000, 0x3a00de5100000000, 0x8051d7c800000000,
    0x1661d0bf00000000, 0xb5f4b42100000000, 0x23c4b35600000000,
    0x9995bacf00000000, 0x0fa5bdb800000000, 0x9eb8022800000000,
    0x0888055f00000000, 0xb2d90cc600000000, 0x24e90bb100000000,
    0x877c6f2f00000000, 0x114c685800000000, 0xab1d61c100000000,
    0x3d2d66b600000000, 0x9041dc7600000000, 0x0671db0100000000,
    0xbc20d29800000000, 0x2a10d5ef00000000, 0x8985b17100000000,
    0x1fb5b60600000000, 0xa5e4bf9f00000000, 0x33d4b8e800000000,
    0xa2c9077800000000, 0x34f9000f00000000, 0x8ea8099600000000,
    0x18980ee100000000, 0xbb0d6a7f00000000, 0x2d3d6d0800000000,
    0x976c649100000000, 0x015c63e600000000, 0xf4516b6b00000000,
    0x62616c1c00000000, 0xd830658500000000, 0x4e0062f200000000,
    0xed95066c00000000, 0x7ba5011b00000000, 0xc1f4088200000000,
    0x57c40ff500000000, 0xc6d9b06500000000, 0x50e9b71200000000,
    0xeab8be8b00000000, 0x7c88b9fc00000000, 0xdf1ddd6200000000,
    0x492dda1500000000, 0xf37cd38c00000000, 0x654cd4fb00000000,
    0x5861b24d00000000, 0xce51b53a00000000, 0x7400bca300000000,
    0xe230bbd400000000, 0x41a5df4a00000000, 0xd795d83d00000000,
    0x6dc4d1a400000000, 0xfbf4d6d300000000, 0x6ae9694300000000,
    0xfcd96e3400000000, 0x468867ad00000000, 0xd0b860da00000000,
    0x732d044400000000, 0xe51d033300000000, 0x5f4c0aaa00000000,
    0xc97c0ddd00000000, 0x3c71055000000000, 0xaa41022700000000,
    0x10100bbe00000000, 0x86200cc900000000, 0x25b5685700000000,
    0xb3856f2000000000, 0x09d466b900000000, 0x9fe461ce00000000,
    0x0ef9de5e00000000, 0x98c9d92900000000, 0x2298d0b000000000,
    0xb4a8d7c700000000, 0x173db35900000000, 0x810db42e00000000,
    0x3b5cbdb700000000, 0xad6cbac000000000, 0x2083b8ed00000000,
    0xb6b3bf9a00000000, 0x0ce2b60300000000, 0x9ad2b17400000000,
    0x3947d5ea00000000, 0xaf77d29d00000000, 0x1526db0400000000,
    0x8316dc7300000000, 0x120b63e300000000, 0x843b649400000000,
    0x3e6a6d0d00000000, 0xa85a6a7a00000000, 0x0bcf0ee400000000,
    0x9dff099300000000, 0x27ae000a00000000, 0xb19e077d00000000,
    0x44930ff000000000, 0xd2a3088700000000, 0x68f2011e00000000,
    0xfec2066900000000, 0x5d5762f700000000, 0xcb67658000000000,
    0x71366c1900000000, 0xe7066b6e00000000, 0x761bd4fe00000000,
    0xe02bd38900000000, 0x5a7ada1000000000, 0xcc4add6700000000,
    0x6fdfb9f900000000, 0xf9efbe8e00000000, 0x43beb71700000000,
    0xd58eb06000000000, 0xe8a3d6d600000000, 0x7e93d1a100000000,
    0xc4c2d83800000000, 0x52f2df4f00000000, 0xf167bbd100000000,
    0x6757bca600000000, 0xdd06b53f00000000, 0x4b36b24800000000,
    0xda2b0dd800000000, 0x4c1b0aaf00000000, 0xf64a033600000000,
    0x607a044100000000, 0xc3ef60df00000000, 0x55df67a800000000,
    0xef8e6e3100000000, 0x79be694600000000, 0x8cb361cb00000000,
    0x1a8366bc00000000, 0xa0d26f2500000000, 0x36e2685200000000,
    0x95770ccc00000000, 0x03470bbb00000000, 0xb916022200000000,
    0x2f26055500000000, 0xbe3bbac500000000, 0x280bbdb200000000,
    0x925ab42b00000000, 0x046ab35c00000000, 0xa7ffd7c200000000,
    0x31cfd0b500000000, 0x8b9ed92c00000000, 0x1daede5b00000000,
    0xb0c2649b00000000, 0x26f263ec00000000, 0x9ca36a7500000000,
    0x0a936d0200000000, 0xa906099c00000000, 0x3f360eeb00000000,
    0x8567077200000000, 0x1357000500000000, 0x824abf9500000000,
    0x147ab8e200000000, 0xae2bb17b00000000, 0x381bb60c00000000,
    0x9b8ed29200000000, 0x0dbed5e500000000, 0xb7efdc7c00000000,
    0x21dfdb0b00000000, 0xd4d2d38600000000, 0x42e2d4f100000000,
    0xf8b3dd6800000000, 0x6e83da1f00000000, 0xcd16be8100000000,
    0x5b26b9f600000000, 0xe177b06f00000000, 0x7747b71800000000,
    0xe65a088800000000, 0x706a0fff00000000, 0xca3b066600000000,
    0x5c0b011100000000, 0xff9e658f00000000, 0x69ae62f800000000,
    0xd3ff6b6100000000, 0x45cf6c1600000000, 0x78e20aa000000000,
    0xeed20dd700000000, 0x5483044e00000000, 0xc2b3033900000000,
    0x612667a700000000, 0xf71660d000000000, 0x4d47694900000000,
    0xdb776e3e00000000, 0x4a6ad1ae00000000, 0xdc5ad6d900000000,
    0x660bdf4000000000, 0xf03bd83700000000, 0x53aebca900000000,
    0xc59ebbde00000000, 0x7fcfb24700000000, 0xe9ffb53000000000,
    0x1cf2bdbd00000000, 0x8ac2baca00000000, 0x3093b35300000000,
    0xa6a3b42400000000, 0x0536d0ba00000000, 0x9306d7cd00000000,
    0x2957de5400000000, 0xbf67d92300000000, 0x2e7a66b300000000,
    0xb84a61c400000000, 0x021b685d00000000, 0x942b6f2a00000000,
    0x37be0bb400000000, 0xa18e0cc300000000, 0x1bdf055a00000000,
    0x8def022d00000000};

#else /* W == 4 */

local const z_word_t FAR crc_big_table[] = {
    0x00000000, 0x96300777, 0x2c610eee, 0xba510999, 0x19c46d07,
    0x8ff46a70, 0x35a563e9, 0xa395649e, 0x3288db0e, 0xa4b8dc79,
    0x1ee9d5e0, 0x88d9d297, 0x2b4cb609, 0xbd7cb17e, 0x072db8e7,
    0x911dbf90, 0x6410b71d, 0xf220b06a, 0x4871b9f3, 0xde41be84,
    0x7dd4da1a, 0xebe4dd6d, 0x51b5d4f4, 0xc785d383, 0x56986c13,
    0xc0a86b64, 0x7af962fd, 0xecc9658a, 0x4f5c0114, 0xd96c0663,
    0x633d0ffa, 0xf50d088d, 0xc8206e3b, 0x5e10694c, 0xe44160d5,
    0x727167a2, 0xd1e4033c, 0x47d4044b, 0xfd850dd2, 0x6bb50aa5,
    0xfaa8b535, 0x6c98b242, 0xd6c9bbdb, 0x40f9bcac, 0xe36cd832,
    0x755cdf45, 0xcf0dd6dc, 0x593dd1ab, 0xac30d926, 0x3a00de51,
    0x8051d7c8, 0x1661d0bf, 0xb5f4b421, 0x23c4b356, 0x9995bacf,
    0x0fa5bdb8, 0x9eb80228, 0x0888055f, 0xb2d90cc6, 0x24e90bb1,
    0x877c6f2f, 0x114c6858, 0xab1d61c1, 0x3d2d66b6, 0x9041dc76,
    0x0671db01, 0xbc20d298, 0x2a10d5ef, 0x8985b171, 0x1fb5b606,
    0xa5e4bf9f, 0x33d4b8e8, 0xa2c90778, 0x34f9000f, 0x8ea80996,
    0x18980ee1, 0xbb0d6a7f, 0x2d3d6d08, 0x976c6491, 0x015c63e6,
    0xf4516b6b, 0x62616c1c, 0xd8306585, 0x4e0062f2, 0xed95066c,
    0x7ba5011b, 0xc1f40882, 0x57c40ff5, 0xc6d9b065, 0x50e9b712,
    0xeab8be8b, 0x7c88b9fc, 0xdf1ddd62, 0x492dda15, 0xf37cd38c,
    0x654cd4fb, 0x5861b24d, 0xce51b53a, 0x7400bca3, 0xe230bbd4,
    0x41a5df4a, 0xd795d83d, 0x6dc4d1a4, 0xfbf4d6d3, 0x6ae96943,
    0xfcd96e34, 0x468867ad, 0xd0b860da, 0x732d0444, 0xe51d0333,
    0x5f4c0aaa, 0xc97c0ddd, 0x3c710550, 0xaa410227, 0x10100bbe,
    0x86200cc9, 0x25b56857, 0xb3856f20, 0x09d466b9, 0x9fe461ce,
    0x0ef9de5e, 0x98c9d929, 0x2298d0b0, 0xb4a8d7c7, 0x173db359,
    0x810db42e, 0x3b5cbdb7, 0xad6cbac0, 0x2083b8ed, 0xb6b3bf9a,
    0x0ce2b603, 0x9ad2b174, 0x3947d5ea, 0xaf77d29d, 0x1526db04,
    0x8316dc73, 0x120b63e3, 0x843b6494, 0x3e6a6d0d, 0xa85a6a7a,
    0x0bcf0ee4, 0x9dff0993, 0x27ae000a, 0xb19e077d, 0x44930ff0,
    0xd2a30887, 0x68f2011e, 0xfec20669, 0x5d5762f7, 0xcb676580,
    0x71366c19, 0xe7066b6e, 0x761bd4fe, 0xe02bd389, 0x5a7ada10,
    0xcc4add67, 0x6fdfb9f9, 0xf9efbe8e, 0x43beb717, 0xd58eb060,
    0xe8a3d6d6, 0x7e93d1a1, 0xc4c2d838, 0x52f2df4f, 0xf167bbd1,
    0x6757bca6, 0xdd06b53f, 0x4b36b248, 0xda2b0dd8, 0x4c1b0aaf,
    0xf64a0336, 0x607a0441, 0xc3ef60df, 0x55df67a8, 0xef8e6e31,
    0x79be6946, 0x8cb361cb, 0x1a8366bc, 0xa0d26f25, 0x36e26852,
    0x95770ccc, 0x03470bbb, 0xb9160222, 0x2f260555, 0xbe3bbac5,
    0x280bbdb2, 0x925ab42b, 0x046ab35c, 0xa7ffd7c2, 0x31cfd0b5,
    0x8b9ed92c, 0x1daede5b, 0xb0c2649b, 0x26f263ec, 0x9ca36a75,
    0x0a936d02, 0xa906099c, 0x3f360eeb, 0x85670772, 0x13570005,
    0x824abf95, 0x147ab8e2, 0xae2bb17b, 0x381bb60c, 0x9b8ed292,
    0x0dbed5e5, 0xb7efdc7c, 0x21dfdb0b, 0xd4d2d386, 0x42e2d4f1,
    0xf8b3dd68, 0x6e83da1f, 0xcd16be81, 0x5b26b9f6, 0xe177b06f,
    0x7747b718, 0xe65a0888, 0x706a0fff, 0xca3b0666, 0x5c0b0111,
    0xff9e658f, 0x69ae62f8, 0xd3ff6b61, 0x45cf6c16, 0x78e20aa0,
    0xeed20dd7, 0x5483044e, 0xc2b30339, 0x612667a7, 0xf71660d0,
    0x4d476949, 0xdb776e3e, 0x4a6ad1ae, 0xdc5ad6d9, 0x660bdf40,
    0xf03bd837, 0x53aebca9, 0xc59ebbde, 0x7fcfb247, 0xe9ffb530,
    0x1cf2bdbd, 0x8ac2baca, 0x3093b353, 0xa6a3b424, 0x0536d0ba,
    0x9306d7cd, 0x2957de54, 0xbf67d923, 0x2e7a66b3, 0xb84a61c4,
    0x021b685d, 0x942b6f2a, 0x37be0bb4, 0xa18e0cc3, 0x1bdf055a,
    0x8def022d};

#endif

#if N == 1

#if W == 8

local const z_crc_t FAR crc_braid_table[][256] = {
   {0x00000000, 0xccaa009e, 0x4225077d, 0x8e8f07e3, 0x844a0efa,
    0x48e00e64, 0xc66f0987, 0x0ac50919, 0xd3e51bb5, 0x1f4f1b2b,
    0x91c01cc8, 0x5d6a1c56, 0x57af154f, 0x9b0515d1, 0x158a1232,
    0xd92012ac, 0x7cbb312b, 0xb01131b5, 0x3e9e3656, 0xf23436c8,
    0xf8f13fd1, 0x345b3f4f, 0xbad438ac, 0x767e3832, 0xaf5e2a9e,
    0x63f42a00, 0xed7b2de3, 0x21d12d7d, 0x2b142464, 0xe7be24fa,
    0x69312319, 0xa59b2387, 0xf9766256, 0x35dc62c8, 0xbb53652b,
    0x77f965b5, 0x7d3c6cac, 0xb1966c32, 0x3f196bd1, 0xf3b36b4f,
    0x2a9379e3, 0xe639797d, 0x68b67e9e, 0xa41c7e00, 0xaed97719,
    0x62737787, 0xecfc7064, 0x205670fa, 0x85cd537d, 0x496753e3,
    0xc7e85400, 0x0b42549e, 0x01875d87, 0xcd2d5d19, 0x43a25afa,
    0x8f085a64, 0x562848c8, 0x9a824856, 0x140d4fb5, 0xd8a74f2b,
    0xd2624632, 0x1ec846ac, 0x9047414f, 0x5ced41d1, 0x299dc2ed,
    0xe537c273, 0x6bb8c590, 0xa712c50e, 0xadd7cc17, 0x617dcc89,
    0xeff2cb6a, 0x2358cbf4, 0xfa78d958, 0x36d2d9c6, 0xb85dde25,
    0x74f7debb, 0x7e32d7a2, 0xb298d73c, 0x3c17d0df, 0xf0bdd041,
    0x5526f3c6, 0x998cf358, 0x1703f4bb, 0xdba9f425, 0xd16cfd3c,
    0x1dc6fda2, 0x9349fa41, 0x5fe3fadf, 0x86c3e873, 0x4a69e8ed,
    0xc4e6ef0e, 0x084cef90, 0x0289e689, 0xce23e617, 0x40ace1f4,
    0x8c06e16a, 0xd0eba0bb, 0x1c41a025, 0x92cea7c6, 0x5e64a758,
    0x54a1ae41, 0x980baedf, 0x1684a93c, 0xda2ea9a2, 0x030ebb0e,
    0xcfa4bb90, 0x412bbc73, 0x8d81bced, 0x8744b5f4, 0x4beeb56a,
    0xc561b289, 0x09cbb217, 0xac509190, 0x60fa910e, 0xee7596ed,
    0x22df9673, 0x281a9f6a, 0xe4b09ff4, 0x6a3f9817, 0xa6959889,
    0x7fb58a25, 0xb31f8abb, 0x3d908d58, 0xf13a8dc6, 0xfbff84df,
    0x37558441, 0xb9da83a2, 0x7570833c, 0x533b85da, 0x9f918544,
    0x111e82a7, 0xddb48239, 0xd7718b20, 0x1bdb8bbe, 0x95548c5d,
    0x59fe8cc3, 0x80de9e6f, 0x4c749ef1, 0xc2fb9912, 0x0e51998c,
    0x04949095, 0xc83e900b, 0x46b197e8, 0x8a1b9776, 0x2f80b4f1,
    0xe32ab46f, 0x6da5b38c, 0xa10fb312, 0xabcaba0b, 0x6760ba95,
    0xe9efbd76, 0x2545bde8, 0xfc65af44, 0x30cfafda, 0xbe40a839,
    0x72eaa8a7, 0x782fa1be, 0xb485a120, 0x3a0aa6c3, 0xf6a0a65d,
    0xaa4de78c, 0x66e7e712, 0xe868e0f1, 0x24c2e06f, 0x2e07e976,
    0xe2ade9e8, 0x6c22ee0b, 0xa088ee95, 0x79a8fc39, 0xb502fca7,
    0x3b8dfb44, 0xf727fbda, 0xfde2f2c3, 0x3148f25d, 0xbfc7f5be,
    0x736df520, 0xd6f6d6a7, 0x1a5cd639, 0x94d3d1da, 0x5879d144,
    0x52bcd85d, 0x9e16d8c3, 0x1099df20, 0xdc33dfbe, 0x0513cd12,
    0xc9b9cd8c, 0x4736ca6f, 0x8b9ccaf1, 0x8159c3e8, 0x4df3c376,
    0xc37cc495, 0x0fd6c40b, 0x7aa64737, 0xb60c47a9, 0x3883404a,
    0xf42940d4, 0xfeec49cd, 0x32464953, 0xbcc94eb0, 0x70634e2e,
    0xa9435c82, 0x65e95c1c, 0xeb665bff, 0x27cc5b61, 0x2d095278,
    0xe1a352e6, 0x6f2c5505, 0xa386559b, 0x061d761c, 0xcab77682,
    0x44387161, 0x889271ff, 0x825778e6, 0x4efd7878, 0xc0727f9b,
    0x0cd87f05, 0xd5f86da9, 0x19526d37, 0x97dd6ad4, 0x5b776a4a,
    0x51b26353, 0x9d1863cd, 0x1397642e, 0xdf3d64b0, 0x83d02561,
    0x4f7a25ff, 0xc1f5221c, 0x0d5f2282, 0x079a2b9b, 0xcb302b05,
    0x45bf2ce6, 0x89152c78, 0x50353ed4, 0x9c9f3e4a, 0x121039a9,
    0xdeba3937, 0xd47f302e, 0x18d530b0, 0x965a3753, 0x5af037cd,
    0xff6b144a, 0x33c114d4, 0xbd4e1337, 0x71e413a9, 0x7b211ab0,
    0xb78b1a2e, 0x39041dcd, 0xf5ae1d53, 0x2c8e0fff, 0xe0240f61,
    0x6eab0882, 0xa201081c, 0xa8c40105, 0x646e019b, 0xeae10678,
    0x264b06e6},
   {0x00000000, 0xa6770bb4, 0x979f1129, 0x31e81a9d, 0xf44f2413,
    0x52382fa7, 0x63d0353a, 0xc5a73e8e, 0x33ef4e67, 0x959845d3,
    0xa4705f4e, 0x020754fa, 0xc7a06a74, 0x61d761c0, 0x503f7b5d,
    0xf64870e9, 0x67de9cce, 0xc1a9977a, 0xf0418de7, 0x56368653,
    0x9391b8dd, 0x35e6b369, 0x040ea9f4, 0xa279a240, 0x5431d2a9,
    0xf246d91d, 0xc3aec380, 0x65d9c834, 0xa07ef6ba, 0x0609fd0e,
    0x37e1e793, 0x9196ec27, 0xcfbd399c, 0x69ca3228, 0x582228b5,
    0xfe552301, 0x3bf21d8f, 0x9d85163b, 0xac6d0ca6, 0x0a1a0712,
    0xfc5277fb, 0x5a257c4f, 0x6bcd66d2, 0xcdba6d66, 0x081d53e8,
    0xae6a585c, 0x9f8242c1, 0x39f54975, 0xa863a552, 0x0e14aee6,
    0x3ffcb47b, 0x998bbfcf, 0x5c2c8141, 0xfa5b8af5, 0xcbb39068,
    0x6dc49bdc, 0x9b8ceb35, 0x3dfbe081, 0x0c13fa1c, 0xaa64f1a8,
    0x6fc3cf26, 0xc9b4c492, 0xf85cde0f, 0x5e2bd5bb, 0x440b7579,
    0xe27c7ecd, 0xd3946450, 0x75e36fe4, 0xb044516a, 0x16335ade,
    0x27db4043, 0x81ac4bf7, 0x77e43b1e, 0xd19330aa, 0xe07b2a37,
    0x460c2183, 0x83ab1f0d, 0x25dc14b9, 0x14340e24, 0xb2430590,
    0x23d5e9b7, 0x85a2e203, 0xb44af89e, 0x123df32a, 0xd79acda4,
    0x71edc610, 0x4005dc8d, 0xe672d739, 0x103aa7d0, 0xb64dac64,
    0x87a5b6f9, 0x21d2bd4d, 0xe47583c3, 0x42028877, 0x73ea92ea,
    0xd59d995e, 0x8bb64ce5, 0x2dc14751, 0x1c295dcc, 0xba5e5678,
    0x7ff968f6, 0xd98e6342, 0xe86679df, 0x4e11726b, 0xb8590282,
    0x1e2e0936, 0x2fc613ab, 0x89b1181f, 0x4c162691, 0xea612d25,
    0xdb8937b8, 0x7dfe3c0c, 0xec68d02b, 0x4a1fdb9f, 0x7bf7c102,
    0xdd80cab6, 0x1827f438, 0xbe50ff8c, 0x8fb8e511, 0x29cfeea5,
    0xdf879e4c, 0x79f095f8, 0x48188f65, 0xee6f84d1, 0x2bc8ba5f,
    0x8dbfb1eb, 0xbc57ab76, 0x1a20a0c2, 0x8816eaf2, 0x2e61e146,
    0x1f89fbdb, 0xb9fef06f, 0x7c59cee1, 0xda2ec555, 0xebc6dfc8,
    0x4db1d47c, 0xbbf9a495, 0x1d8eaf21, 0x2c66b5bc, 0x8a11be08,
    0x4fb68086, 0xe9c18b32, 0xd82991af, 0x7e5e9a1b, 0xefc8763c,
    0x49bf7d88, 0x78576715, 0xde206ca1, 0x1b87522f, 0xbdf0599b,
    0x8c184306, 0x2a6f48b2, 0xdc27385b, 0x7a5033ef, 0x4bb82972,
    0xedcf22c6, 0x28681c48, 0x8e1f17fc, 0xbff70d61, 0x198006d5,
    0x47abd36e, 0xe1dcd8da, 0xd034c247, 0x7643c9f3, 0xb3e4f77d,
    0x1593fcc9, 0x247be654, 0x820cede0, 0x74449d09, 0xd23396bd,
    0xe3db8c20, 0x45ac8794, 0x800bb91a, 0x267cb2ae, 0x1794a833,
    0xb1e3a387, 0x20754fa0, 0x86024414, 0xb7ea5e89, 0x119d553d,
    0xd43a6bb3, 0x724d6007, 0x43a57a9a, 0xe5d2712e, 0x139a01c7,
    0xb5ed0a73, 0x840510ee, 0x22721b5a, 0xe7d525d4, 0x41a22e60,
    0x704a34fd, 0xd63d3f49, 0xcc1d9f8b, 0x6a6a943f, 0x5b828ea2,
    0xfdf58516, 0x3852bb98, 0x9e25b02c, 0xafcdaab1, 0x09baa105,
    0xfff2d1ec, 0x5985da58, 0x686dc0c5, 0xce1acb71, 0x0bbdf5ff,
    0xadcafe4b, 0x9c22e4d6, 0x3a55ef62, 0xabc30345, 0x0db408f1,
    0x3c5c126c, 0x9a2b19d8, 0x5f8c2756, 0xf9fb2ce2, 0xc813367f,
    0x6e643dcb, 0x982c4d22, 0x3e5b4696, 0x0fb35c0b, 0xa9c457bf,
    0x6c636931, 0xca146285, 0xfbfc7818, 0x5d8b73ac, 0x03a0a617,
    0xa5d7ada3, 0x943fb73e, 0x3248bc8a, 0xf7ef8204, 0x519889b0,
    0x6070932d, 0xc6079899, 0x304fe870, 0x9638e3c4, 0xa7d0f959,
    0x01a7f2ed, 0xc400cc63, 0x6277c7d7, 0x539fdd4a, 0xf5e8d6fe,
    0x647e3ad9, 0xc209316d, 0xf3e12bf0, 0x55962044, 0x90311eca,
    0x3646157e, 0x07ae0fe3, 0xa1d90457, 0x579174be, 0xf1e67f0a,
    0xc00e6597, 0x66796e23, 0xa3de50ad, 0x05a95b19, 0x34414184,
    0x92364a30},
   {0x00000000, 0xcb5cd3a5, 0x4dc8a10b, 0x869472ae, 0x9b914216,
    0x50cd91b3, 0xd659e31d, 0x1d0530b8, 0xec53826d, 0x270f51c8,
    0xa19b2366, 0x6ac7f0c3, 0x77c2c07b, 0xbc9e13de, 0x3a0a6170,
    0xf156b2d5, 0x03d6029b, 0xc88ad13e, 0x4e1ea390, 0x85427035,
    0x9847408d, 0x531b9328, 0xd58fe186, 0x1ed33223, 0xef8580f6,
    0x24d95353, 0xa24d21fd, 0x6911f258, 0x7414c2e0, 0xbf481145,
    0x39dc63eb, 0xf280b04e, 0x07ac0536, 0xccf0d693, 0x4a64a43d,
    0x81387798, 0x9c3d4720, 0x57619485, 0xd1f5e62b, 0x1aa9358e,
    0xebff875b, 0x20a354fe, 0xa6372650, 0x6d6bf5f5, 0x706ec54d,
    0xbb3216e8, 0x3da66446, 0xf6fab7e3, 0x047a07ad, 0xcf26d408,
    0x49b2a6a6, 0x82ee7503, 0x9feb45bb, 0x54b7961e, 0xd223e4b0,
    0x197f3715, 0xe82985c0, 0x23755665, 0xa5e124cb, 0x6ebdf76e,
    0x73b8c7d6, 0xb8e41473, 0x3e7066dd, 0xf52cb578, 0x0f580a6c,
    0xc404d9c9, 0x4290ab67, 0x89cc78c2, 0x94c9487a, 0x5f959bdf,
    0xd901e971, 0x125d3ad4, 0xe30b8801, 0x28575ba4, 0xaec3290a,
    0x659ffaaf, 0x789aca17, 0xb3c619b2, 0x35526b1c, 0xfe0eb8b9,
    0x0c8e08f7, 0xc7d2db52, 0x4146a9fc, 0x8a1a7a59, 0x971f4ae1,
    0x5c439944, 0xdad7ebea, 0x118b384f, 0xe0dd8a9a, 0x2b81593f,
    0xad152b91, 0x6649f834, 0x7b4cc88c, 0xb0101b29, 0x36846987,
    0xfdd8ba22, 0x08f40f5a, 0xc3a8dcff, 0x453cae51, 0x8e607df4,
    0x93654d4c, 0x58399ee9, 0xdeadec47, 0x15f13fe2, 0xe4a78d37,
    0x2ffb5e92, 0xa96f2c3c, 0x6233ff99, 0x7f36cf21, 0xb46a1c84,
    0x32fe6e2a, 0xf9a2bd8f, 0x0b220dc1, 0xc07ede64, 0x46eaacca,
    0x8db67f6f, 0x90b34fd7, 0x5bef9c72, 0xdd7beedc, 0x16273d79,
    0xe7718fac, 0x2c2d5c09, 0xaab92ea7, 0x61e5fd02, 0x7ce0cdba,
    0xb7bc1e1f, 0x31286cb1, 0xfa74bf14, 0x1eb014d8, 0xd5ecc77d,
    0x5378b5d3, 0x98246676, 0x852156ce, 0x4e7d856b, 0xc8e9f7c5,
    0x03b52460, 0xf2e396b5, 0x39bf4510, 0xbf2b37be, 0x7477e41b,
    0x6972d4a3, 0xa22e0706, 0x24ba75a8, 0xefe6a60d, 0x1d661643,
    0xd63ac5e6, 0x50aeb748, 0x9bf264ed, 0x86f75455, 0x4dab87f0,
    0xcb3ff55e, 0x006326fb, 0xf135942e, 0x3a69478b, 0xbcfd3525,
    0x77a1e680, 0x6aa4d638, 0xa1f8059d, 0x276c7733, 0xec30a496,
    0x191c11ee, 0xd240c24b, 0x54d4b0e5, 0x9f886340, 0x828d53f8,
    0x49d1805d, 0xcf45f2f3, 0x04192156, 0xf54f9383, 0x3e134026,
    0xb8873288, 0x73dbe12d, 0x6eded195, 0xa5820230, 0x2316709e,
    0xe84aa33b, 0x1aca1375, 0xd196c0d0, 0x5702b27e, 0x9c5e61db,
    0x815b5163, 0x4a0782c6, 0xcc93f068, 0x07cf23cd, 0xf6999118,
    0x3dc542bd, 0xbb513013, 0x700de3b6, 0x6d08d30e, 0xa65400ab,
    0x20c07205, 0xeb9ca1a0, 0x11e81eb4, 0xdab4cd11, 0x5c20bfbf,
    0x977c6c1a, 0x8a795ca2, 0x41258f07, 0xc7b1fda9, 0x0ced2e0c,
    0xfdbb9cd9, 0x36e74f7c, 0xb0733dd2, 0x7b2fee77, 0x662adecf,
    0xad760d6a, 0x2be27fc4, 0xe0beac61, 0x123e1c2f, 0xd962cf8a,
    0x5ff6bd24, 0x94aa6e81, 0x89af5e39, 0x42f38d9c, 0xc467ff32,
    0x0f3b2c97, 0xfe6d9e42, 0x35314de7, 0xb3a53f49, 0x78f9ecec,
    0x65fcdc54, 0xaea00ff1, 0x28347d5f, 0xe368aefa, 0x16441b82,
    0xdd18c827, 0x5b8cba89, 0x90d0692c, 0x8dd55994, 0x46898a31,
    0xc01df89f, 0x0b412b3a, 0xfa1799ef, 0x314b4a4a, 0xb7df38e4,
    0x7c83eb41, 0x6186dbf9, 0xaada085c, 0x2c4e7af2, 0xe712a957,
    0x15921919, 0xdececabc, 0x585ab812, 0x93066bb7, 0x8e035b0f,
    0x455f88aa, 0xc3cbfa04, 0x089729a1, 0xf9c19b74, 0x329d48d1,
    0xb4093a7f, 0x7f55e9da, 0x6250d962, 0xa90c0ac7, 0x2f987869,
    0xe4c4abcc},
   {0x00000000, 0x3d6029b0, 0x7ac05360, 0x47a07ad0, 0xf580a6c0,
    0xc8e08f70, 0x8f40f5a0, 0xb220dc10, 0x30704bc1, 0x0d106271,
    0x4ab018a1, 0x77d03111, 0xc5f0ed01, 0xf890c4b1, 0xbf30be61,
    0x825097d1, 0x60e09782, 0x5d80be32, 0x1a20c4e2, 0x2740ed52,
    0x95603142, 0xa80018f2, 0xefa06222, 0xd2c04b92, 0x5090dc43,
    0x6df0f5f3, 0x2a508f23, 0x1730a693, 0xa5107a83, 0x98705333,
    0xdfd029e3, 0xe2b00053, 0xc1c12f04, 0xfca106b4, 0xbb017c64,
    0x866155d4, 0x344189c4, 0x0921a074, 0x4e81daa4, 0x73e1f314,
    0xf1b164c5, 0xccd14d75, 0x8b7137a5, 0xb6111e15, 0x0431c205,
    0x3951ebb5, 0x7ef19165, 0x4391b8d5, 0xa121b886, 0x9c419136,
    0xdbe1ebe6, 0xe681c256, 0x54a11e46, 0x69c137f6, 0x2e614d26,
    0x13016496, 0x9151f347, 0xac31daf7, 0xeb91a027, 0xd6f18997,
    0x64d15587, 0x59b17c37, 0x1e1106e7, 0x23712f57, 0x58f35849,
    0x659371f9, 0x22330b29, 0x1f532299, 0xad73fe89, 0x9013d739,
    0xd7b3ade9, 0xead38459, 0x68831388, 0x55e33a38, 0x124340e8,
    0x2f236958, 0x9d03b548, 0xa0639cf8, 0xe7c3e628, 0xdaa3cf98,
    0x3813cfcb, 0x0573e67b, 0x42d39cab, 0x7fb3b51b, 0xcd93690b,
    0xf0f340bb, 0xb7533a6b, 0x8a3313db, 0x0863840a, 0x3503adba,
    0x72a3d76a, 0x4fc3feda, 0xfde322ca, 0xc0830b7a, 0x872371aa,
    0xba43581a, 0x9932774d, 0xa4525efd, 0xe3f2242d, 0xde920d9d,
    0x6cb2d18d, 0x51d2f83d, 0x167282ed, 0x2b12ab5d, 0xa9423c8c,
    0x9422153c, 0xd3826fec, 0xeee2465c, 0x5cc29a4c, 0x61a2b3fc,
    0x2602c92c, 0x1b62e09c, 0xf9d2e0cf, 0xc4b2c97f, 0x8312b3af,
    0xbe729a1f, 0x0c52460f, 0x31326fbf, 0x7692156f, 0x4bf23cdf,
    0xc9a2ab0e, 0xf4c282be, 0xb362f86e, 0x8e02d1de, 0x3c220dce,
    0x0142247e, 0x46e25eae, 0x7b82771e, 0xb1e6b092, 0x8c869922,
    0xcb26e3f2, 0xf646ca42, 0x44661652, 0x79063fe2, 0x3ea64532,
    0x03c66c82, 0x8196fb53, 0xbcf6d2e3, 0xfb56a833, 0xc6368183,
    0x74165d93, 0x49767423, 0x0ed60ef3, 0x33b62743, 0xd1062710,
    0xec660ea0, 0xabc67470, 0x96a65dc0, 0x248681d0, 0x19e6a860,
    0x5e46d2b0, 0x6326fb00, 0xe1766cd1, 0xdc164561, 0x9bb63fb1,
    0xa6d61601, 0x14f6ca11, 0x2996e3a1, 0x6e369971, 0x5356b0c1,
    0x70279f96, 0x4d47b626, 0x0ae7ccf6, 0x3787e546, 0x85a73956,
    0xb8c710e6, 0xff676a36, 0xc2074386, 0x4057d457, 0x7d37fde7,
    0x3a978737, 0x07f7ae87, 0xb5d77297, 0x88b75b27, 0xcf1721f7,
    0xf2770847, 0x10c70814, 0x2da721a4, 0x6a075b74, 0x576772c4,
    0xe547aed4, 0xd8278764, 0x9f87fdb4, 0xa2e7d404, 0x20b743d5,
    0x1dd76a65, 0x5a7710b5, 0x67173905, 0xd537e515, 0xe857cca5,
    0xaff7b675, 0x92979fc5, 0xe915e8db, 0xd475c16b, 0x93d5bbbb,
    0xaeb5920b, 0x1c954e1b, 0x21f567ab, 0x66551d7b, 0x5b3534cb,
    0xd965a31a, 0xe4058aaa, 0xa3a5f07a, 0x9ec5d9ca, 0x2ce505da,
    0x11852c6a, 0x562556ba, 0x6b457f0a, 0x89f57f59, 0xb49556e9,
    0xf3352c39, 0xce550589, 0x7c75d999, 0x4115f029, 0x06b58af9,
    0x3bd5a349, 0xb9853498, 0x84e51d28, 0xc34567f8, 0xfe254e48,
    0x4c059258, 0x7165bbe8, 0x36c5c138, 0x0ba5e888, 0x28d4c7df,
    0x15b4ee6f, 0x521494bf, 0x6f74bd0f, 0xdd54611f, 0xe03448af,
    0xa794327f, 0x9af41bcf, 0x18a48c1e, 0x25c4a5ae, 0x6264df7e,
    0x5f04f6ce, 0xed242ade, 0xd044036e, 0x97e479be, 0xaa84500e,
    0x4834505d, 0x755479ed, 0x32f4033d, 0x0f942a8d, 0xbdb4f69d,
    0x80d4df2d, 0xc774a5fd, 0xfa148c4d, 0x78441b9c, 0x4524322c,
    0x028448fc, 0x3fe4614c, 0x8dc4bd5c, 0xb0a494ec, 0xf704ee3c,
    0xca64c78c},
   {0x00000000, 0xb8bc6765, 0xaa09c88b, 0x12b5afee, 0x8f629757,
    0x37def032, 0x256b5fdc, 0x9dd738b9, 0xc5b428ef, 0x7d084f8a,
    0x6fbde064, 0xd7018701, 0x4ad6bfb8, 0xf26ad8dd, 0xe0df7733,
    0x58631056, 0x5019579f, 0xe8a530fa, 0xfa109f14, 0x42acf871,
    0xdf7bc0c8, 0x67c7a7ad, 0x75720843, 0xcdce6f26, 0x95ad7f70,
    0x2d111815, 0x3fa4b7fb, 0x8718d09e, 0x1acfe827, 0xa2738f42,
    0xb0c620ac, 0x087a47c9, 0xa032af3e, 0x188ec85b, 0x0a3b67b5,
    0xb28700d0, 0x2f503869, 0x97ec5f0c, 0x8559f0e2, 0x3de59787,
    0x658687d1, 0xdd3ae0b4, 0xcf8f4f5a, 0x7733283f, 0xeae41086,
    0x525877e3, 0x40edd80d, 0xf851bf68, 0xf02bf8a1, 0x48979fc4,
    0x5a22302a, 0xe29e574f, 0x7f496ff6, 0xc7f50893, 0xd540a77d,
    0x6dfcc018, 0x359fd04e, 0x8d23b72b, 0x9f9618c5, 0x272a7fa0,
    0xbafd4719, 0x0241207c, 0x10f48f92, 0xa848e8f7, 0x9b14583d,
    0x23a83f58, 0x311d90b6, 0x89a1f7d3, 0x1476cf6a, 0xaccaa80f,
    0xbe7f07e1, 0x06c36084, 0x5ea070d2, 0xe61c17b7, 0xf4a9b859,
    0x4c15df3c, 0xd1c2e785, 0x697e80e0, 0x7bcb2f0e, 0xc377486b,
    0xcb0d0fa2, 0x73b168c7, 0x6104c729, 0xd9b8a04c, 0x446f98f5,
    0xfcd3ff90, 0xee66507e, 0x56da371b, 0x0eb9274d, 0xb6054028,
    0xa4b0efc6, 0x1c0c88a3, 0x81dbb01a, 0x3967d77f, 0x2bd27891,
    0x936e1ff4, 0x3b26f703, 0x839a9066, 0x912f3f88, 0x299358ed,
    0xb4446054, 0x0cf80731, 0x1e4da8df, 0xa6f1cfba, 0xfe92dfec,
    0x462eb889, 0x549b1767, 0xec277002, 0x71f048bb, 0xc94c2fde,
    0xdbf98030, 0x6345e755, 0x6b3fa09c, 0xd383c7f9, 0xc1366817,
    0x798a0f72, 0xe45d37cb, 0x5ce150ae, 0x4e54ff40, 0xf6e89825,
    0xae8b8873, 0x1637ef16, 0x048240f8, 0xbc3e279d, 0x21e91f24,
    0x99557841, 0x8be0d7af, 0x335cb0ca, 0xed59b63b, 0x55e5d15e,
    0x47507eb0, 0xffec19d5, 0x623b216c, 0xda874609, 0xc832e9e7,
    0x708e8e82, 0x28ed9ed4, 0x9051f9b1, 0x82e4565f, 0x3a58313a,
    0xa78f0983, 0x1f336ee6, 0x0d86c108, 0xb53aa66d, 0xbd40e1a4,
    0x05fc86c1, 0x1749292f, 0xaff54e4a, 0x322276f3, 0x8a9e1196,
    0x982bbe78, 0x2097d91d, 0x78f4c94b, 0xc048ae2e, 0xd2fd01c0,
    0x6a4166a5, 0xf7965e1c, 0x4f2a3979, 0x5d9f9697, 0xe523f1f2,
    0x4d6b1905, 0xf5d77e60, 0xe762d18e, 0x5fdeb6eb, 0xc2098e52,
    0x7ab5e937, 0x680046d9, 0xd0bc21bc, 0x88df31ea, 0x3063568f,
    0x22d6f961, 0x9a6a9e04, 0x07bda6bd, 0xbf01c1d8, 0xadb46e36,
    0x15080953, 0x1d724e9a, 0xa5ce29ff, 0xb77b8611, 0x0fc7e174,
    0x9210d9cd, 0x2aacbea8, 0x38191146, 0x80a57623, 0xd8c66675,
    0x607a0110, 0x72cfaefe, 0xca73c99b, 0x57a4f122, 0xef189647,
    0xfdad39a9, 0x45115ecc, 0x764dee06, 0xcef18963, 0xdc44268d,
    0x64f841e8, 0xf92f7951, 0x41931e34, 0x5326b1da, 0xeb9ad6bf,
    0xb3f9c6e9, 0x0b45a18c, 0x19f00e62, 0xa14c6907, 0x3c9b51be,
    0x842736db, 0x96929935, 0x2e2efe50, 0x2654b999, 0x9ee8defc,
    0x8c5d7112, 0x34e11677, 0xa9362ece, 0x118a49ab, 0x033fe645,
    0xbb838120, 0xe3e09176, 0x5b5cf613, 0x49e959fd, 0xf1553e98,
    0x6c820621, 0xd43e6144, 0xc68bceaa, 0x7e37a9cf, 0xd67f4138,
    0x6ec3265d, 0x7c7689b3, 0xc4caeed6, 0x591dd66f, 0xe1a1b10a,
    0xf3141ee4, 0x4ba87981, 0x13cb69d7, 0xab770eb2, 0xb9c2a15c,
    0x017ec639, 0x9ca9fe80, 0x241599e5, 0x36a0360b, 0x8e1c516e,
    0x866616a7, 0x3eda71c2, 0x2c6fde2c, 0x94d3b949, 0x090481f0,
    0xb1b8e695, 0xa30d497b, 0x1bb12e1e, 0x43d23e48, 0xfb6e592d,
    0xe9dbf6c3, 0x516791a6, 0xccb0a91f, 0x740cce7a, 0x66b96194,
    0xde0506f1},
   {0x00000000, 0x01c26a37, 0x0384d46e, 0x0246be59, 0x0709a8dc,
    0x06cbc2eb, 0x048d7cb2, 0x054f1685, 0x0e1351b8, 0x0fd13b8f,
    0x0d9785d6, 0x0c55efe1, 0x091af964, 0x08d89353, 0x0a9e2d0a,
    0x0b5c473d, 0x1c26a370, 0x1de4c947, 0x1fa2771e, 0x1e601d29,
    0x1b2f0bac, 0x1aed619b, 0x18abdfc2, 0x1969b5f5, 0x1235f2c8,
    0x13f798ff, 0x11b126a6, 0x10734c91, 0x153c5a14, 0x14fe3023,
    0x16b88e7a, 0x177ae44d, 0x384d46e0, 0x398f2cd7, 0x3bc9928e,
    0x3a0bf8b9, 0x3f44ee3c, 0x3e86840b, 0x3cc03a52, 0x3d025065,
    0x365e1758, 0x379c7d6f, 0x35dac336, 0x3418a901, 0x3157bf84,
    0x3095d5b3, 0x32d36bea, 0x331101dd, 0x246be590, 0x25a98fa7,
    0x27ef31fe, 0x262d5bc9, 0x23624d4c, 0x22a0277b, 0x20e69922,
    0x2124f315, 0x2a78b428, 0x2bbade1f, 0x29fc6046, 0x283e0a71,
    0x2d711cf4, 0x2cb376c3, 0x2ef5c89a, 0x2f37a2ad, 0x709a8dc0,
    0x7158e7f7, 0x731e59ae, 0x72dc3399, 0x7793251c, 0x76514f2b,
    0x7417f172, 0x75d59b45, 0x7e89dc78, 0x7f4bb64f, 0x7d0d0816,
    0x7ccf6221, 0x798074a4, 0x78421e93, 0x7a04a0ca, 0x7bc6cafd,
    0x6cbc2eb0, 0x6d7e4487, 0x6f38fade, 0x6efa90e9, 0x6bb5866c,
    0x6a77ec5b, 0x68315202, 0x69f33835, 0x62af7f08, 0x636d153f,
    0x612bab66, 0x60e9c151, 0x65a6d7d4, 0x6464bde3, 0x662203ba,
    0x67e0698d, 0x48d7cb20, 0x4915a117, 0x4b531f4e, 0x4a917579,
    0x4fde63fc, 0x4e1c09cb, 0x4c5ab792, 0x4d98dda5, 0x46c49a98,
    0x4706f0af, 0x45404ef6, 0x448224c1, 0x41cd3244, 0x400f5873,
    0x4249e62a, 0x438b8c1d, 0x54f16850, 0x55330267, 0x5775bc3e,
    0x56b7d609, 0x53f8c08c, 0x523aaabb, 0x507c14e2, 0x51be7ed5,
    0x5ae239e8, 0x5b2053df, 0x5966ed86, 0x58a487b1, 0x5deb9134,
    0x5c29fb03, 0x5e6f455a, 0x5fad2f6d, 0xe1351b80, 0xe0f771b7,
    0xe2b1cfee, 0xe373a5d9, 0xe63cb35c, 0xe7fed96b, 0xe5b86732,
    0xe47a0d05, 0xef264a38, 0xeee4200f, 0xeca29e56, 0xed60f461,
    0xe82fe2e4, 0xe9ed88d3, 0xebab368a, 0xea695cbd, 0xfd13b8f0,
    0xfcd1d2c7, 0xfe976c9e, 0xff5506a9, 0xfa1a102c, 0xfbd87a1b,
    0xf99ec442, 0xf85cae75, 0xf300e948, 0xf2c2837f, 0xf0843d26,
    0xf1465711, 0xf4094194, 0xf5cb2ba3, 0xf78d95fa, 0xf64fffcd,
    0xd9785d60, 0xd8ba3757, 0xdafc890e, 0xdb3ee339, 0xde71f5bc,
    0xdfb39f8b, 0xddf521d2, 0xdc374be5, 0xd76b0cd8, 0xd6a966ef,
    0xd4efd8b6, 0xd52db281, 0xd062a404, 0xd1a0ce33, 0xd3e6706a,
    0xd2241a5d, 0xc55efe10, 0xc49c9427, 0xc6da2a7e, 0xc7184049,
    0xc25756cc, 0xc3953cfb, 0xc1d382a2, 0xc011e895, 0xcb4dafa8,
    0xca8fc59f, 0xc8c97bc6, 0xc90b11f1, 0xcc440774, 0xcd866d43,
    0xcfc0d31a, 0xce02b92d, 0x91af9640, 0x906dfc77, 0x922b422e,
    0x93e92819, 0x96a63e9c, 0x976454ab, 0x9522eaf2, 0x94e080c5,
    0x9fbcc7f8, 0x9e7eadcf, 0x9c381396, 0x9dfa79a1, 0x98b56f24,
    0x99770513, 0x9b31bb4a, 0x9af3d17d, 0x8d893530, 0x8c4b5f07,
    0x8e0de15e, 0x8fcf8b69, 0x8a809dec, 0x8b42f7db, 0x89044982,
    0x88c623b5, 0x839a6488, 0x82580ebf, 0x801eb0e6, 0x81dcdad1,
    0x8493cc54, 0x8551a663, 0x8717183a, 0x86d5720d, 0xa9e2d0a0,
    0xa820ba97, 0xaa6604ce, 0xaba46ef9, 0xaeeb787c, 0xaf29124b,
    0xad6fac12, 0xacadc625, 0xa7f18118, 0xa633eb2f, 0xa4755576,
    0xa5b73f41, 0xa0f829c4, 0xa13a43f3, 0xa37cfdaa, 0xa2be979d,
    0xb5c473d0, 0xb40619e7, 0xb640a7be, 0xb782cd89, 0xb2cddb0c,
    0xb30fb13b, 0xb1490f62, 0xb08b6555, 0xbbd72268, 0xba15485f,
    0xb853f606, 0xb9919c31, 0xbcde8ab4, 0xbd1ce083, 0xbf5a5eda,
    0xbe9834ed},
   {0x00000000, 0x191b3141, 0x32366282, 0x2b2d53c3, 0x646cc504,
    0x7d77f445, 0x565aa786, 0x4f4196c7, 0xc8d98a08, 0xd1c2bb49,
    0xfaefe88a, 0xe3f4d9cb, 0xacb54f0c, 0xb5ae7e4d, 0x9e832d8e,
    0x87981ccf, 0x4ac21251, 0x53d92310, 0x78f470d3, 0x61ef4192,
    0x2eaed755, 0x37b5e614, 0x1c98b5d7, 0x05838496, 0x821b9859,
    0x9b00a918, 0xb02dfadb, 0xa936cb9a, 0xe6775d5d, 0xff6c6c1c,
    0xd4413fdf, 0xcd5a0e9e, 0x958424a2, 0x8c9f15e3, 0xa7b24620,
    0xbea97761, 0xf1e8e1a6, 0xe8f3d0e7, 0xc3de8324, 0xdac5b265,
    0x5d5daeaa, 0x44469feb, 0x6f6bcc28, 0x7670fd69, 0x39316bae,
    0x202a5aef, 0x0b07092c, 0x121c386d, 0xdf4636f3, 0xc65d07b2,
    0xed705471, 0xf46b6530, 0xbb2af3f7, 0xa231c2b6, 0x891c9175,
    0x9007a034, 0x179fbcfb, 0x0e848dba, 0x25a9de79, 0x3cb2ef38,
    0x73f379ff, 0x6ae848be, 0x41c51b7d, 0x58de2a3c, 0xf0794f05,
    0xe9627e44, 0xc24f2d87, 0xdb541cc6, 0x94158a01, 0x8d0ebb40,
    0xa623e883, 0xbf38d9c2, 0x38a0c50d, 0x21bbf44c, 0x0a96a78f,
    0x138d96ce, 0x5ccc0009, 0x45d73148, 0x6efa628b, 0x77e153ca,
    0xbabb5d54, 0xa3a06c15, 0x888d3fd6, 0x91960e97, 0xded79850,
    0xc7cca911, 0xece1fad2, 0xf5facb93, 0x7262d75c, 0x6b79e61d,
    0x4054b5de, 0x594f849f, 0x160e1258, 0x0f152319, 0x243870da,
    0x3d23419b, 0x65fd6ba7, 0x7ce65ae6, 0x57cb0925, 0x4ed03864,
    0x0191aea3, 0x188a9fe2, 0x33a7cc21, 0x2abcfd60, 0xad24e1af,
    0xb43fd0ee, 0x9f12832d, 0x8609b26c, 0xc94824ab, 0xd05315ea,
    0xfb7e4629, 0xe2657768, 0x2f3f79f6, 0x362448b7, 0x1d091b74,
    0x04122a35, 0x4b53bcf2, 0x52488db3, 0x7965de70, 0x607eef31,
    0xe7e6f3fe, 0xfefdc2bf, 0xd5d0917c, 0xcccba03d, 0x838a36fa,
    0x9a9107bb, 0xb1bc5478, 0xa8a76539, 0x3b83984b, 0x2298a90a,
    0x09b5fac9, 0x10aecb88, 0x5fef5d4f, 0x46f46c0e, 0x6dd93fcd,
    0x74c20e8c, 0xf35a1243, 0xea412302, 0xc16c70c1, 0xd8774180,
    0x9736d747, 0x8e2de606, 0xa500b5c5, 0xbc1b8484, 0x71418a1a,
    0x685abb5b, 0x4377e898, 0x5a6cd9d9, 0x152d4f1e, 0x0c367e5f,
    0x271b2d9c, 0x3e001cdd, 0xb9980012, 0xa0833153, 0x8bae6290,
    0x92b553d1, 0xddf4c516, 0xc4eff457, 0xefc2a794, 0xf6d996d5,
    0xae07bce9, 0xb71c8da8, 0x9c31de6b, 0x852aef2a, 0xca6b79ed,
    0xd37048ac, 0xf85d1b6f, 0xe1462a2e, 0x66de36e1, 0x7fc507a0,
    0x54e85463, 0x4df36522, 0x02b2f3e5, 0x1ba9c2a4, 0x30849167,
    0x299fa026, 0xe4c5aeb8, 0xfdde9ff9, 0xd6f3cc3a, 0xcfe8fd7b,
    0x80a96bbc, 0x99b25afd, 0xb29f093e, 0xab84387f, 0x2c1c24b0,
    0x350715f1, 0x1e2a4632, 0x07317773, 0x4870e1b4, 0x516bd0f5,
    0x7a468336, 0x635db277, 0xcbfad74e, 0xd2e1e60f, 0xf9ccb5cc,
    0xe0d7848d, 0xaf96124a, 0xb68d230b, 0x9da070c8, 0x84bb4189,
    0x03235d46, 0x1a386c07, 0x31153fc4, 0x280e0e85, 0x674f9842,
    0x7e54a903, 0x5579fac0, 0x4c62cb81, 0x8138c51f, 0x9823f45e,
    0xb30ea79d, 0xaa1596dc, 0xe554001b, 0xfc4f315a, 0xd7626299,
    0xce7953d8, 0x49e14f17, 0x50fa7e56, 0x7bd72d95, 0x62cc1cd4,
    0x2d8d8a13, 0x3496bb52, 0x1fbbe891, 0x06a0d9d0, 0x5e7ef3ec,
    0x4765c2ad, 0x6c48916e, 0x7553a02f, 0x3a1236e8, 0x230907a9,
    0x0824546a, 0x113f652b, 0x96a779e4, 0x8fbc48a5, 0xa4911b66,
    0xbd8a2a27, 0xf2cbbce0, 0xebd08da1, 0xc0fdde62, 0xd9e6ef23,
    0x14bce1bd, 0x0da7d0fc, 0x268a833f, 0x3f91b27e, 0x70d024b9,
    0x69cb15f8, 0x42e6463b, 0x5bfd777a, 0xdc656bb5, 0xc57e5af4,
    0xee530937, 0xf7483876, 0xb809aeb1, 0xa1129ff0, 0x8a3fcc33,
    0x9324fd72},
   {0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
    0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
    0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
    0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
    0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
    0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
    0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
    0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
    0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
    0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
    0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
    0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
    0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
    0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
    0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
    0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
    0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
    0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
    0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
    0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
    0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
    0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
    0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
    0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
    0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
    0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
    0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
    0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
    0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
    0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
    0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
    0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
    0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
    0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
    0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
    0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
    0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
    0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
    0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
    0x2d02ef8d}};

local const z_word_t FAR crc_braid_big_table[][256] = {
   {0x0000000000000000, 0x9630077700000000, 0x2c610eee00000000,
    0xba51099900000000, 0x19c46d0700000000, 0x8ff46a7000000000,
    0x35a563e900000000, 0xa395649e00000000, 0x3288db0e00000000,
    0xa4b8dc7900000000, 0x1ee9d5e000000000, 0x88d9d29700000000,
    0x2b4cb60900000000, 0xbd7cb17e00000000, 0x072db8e700000000,
    0x911dbf9000000000, 0x6410b71d00000000, 0xf220b06a00000000,
    0x4871b9f300000000, 0xde41be8400000000, 0x7dd4da1a00000000,
    0xebe4dd6d00000000, 0x51b5d4f400000000, 0xc785d38300000000,
    0x56986c1300000000, 0xc0a86b6400000000, 0x7af962fd00000000,
    0xecc9658a00000000, 0x4f5c011400000000, 0xd96c066300000000,
    0x633d0ffa00000000, 0xf50d088d00000000, 0xc8206e3b00000000,
    0x5e10694c00000000, 0xe44160d500000000, 0x727167a200000000,
    0xd1e4033c00000000, 0x47d4044b00000000, 0xfd850dd200000000,
    0x6bb50aa500000000, 0xfaa8b53500000000, 0x6c98b24200000000,
    0xd6c9bbdb00000000, 0x40f9bcac00000000, 0xe36cd83200000000,
    0x755cdf4500000000, 0xcf0dd6dc00000000, 0x593dd1ab00000000,
    0xac30d92600000000, 0x3a00de5100000000, 0x8051d7c800000000,
    0x1661d0bf00000000, 0xb5f4b42100000000, 0x23c4b35600000000,
    0x9995bacf00000000, 0x0fa5bdb800000000, 0x9eb8022800000000,
    0x0888055f00000000, 0xb2d90cc600000000, 0x24e90bb100000000,
    0x877c6f2f00000000, 0x114c685800000000, 0xab1d61c100000000,
    0x3d2d66b600000000, 0x9041dc7600000000, 0x0671db0100000000,
    0xbc20d29800000000, 0x2a10d5ef00000000, 0x8985b17100000000,
    0x1fb5b60600000000, 0xa5e4bf9f00000000, 0x33d4b8e800000000,
    0xa2c9077800000000, 0x34f9000f00000000, 0x8ea8099600000000,
    0x18980ee100000000, 0xbb0d6a7f00000000, 0x2d3d6d0800000000,
    0x976c649100000000, 0x015c63e600000000, 0xf4516b6b00000000,
    0x62616c1c00000000, 0xd830658500000000, 0x4e0062f200000000,
    0xed95066c00000000, 0x7ba5011b00000000, 0xc1f4088200000000,
    0x57c40ff500000000, 0xc6d9b06500000000, 0x50e9b71200000000,
    0xeab8be8b00000000, 0x7c88b9fc00000000, 0xdf1ddd6200000000,
    0x492dda1500000000, 0xf37cd38c00000000, 0x654cd4fb00000000,
    0x5861b24d00000000, 0xce51b53a00000000, 0x7400bca300000000,
    0xe230bbd400000000, 0x41a5df4a00000000, 0xd795d83d00000000,
    0x6dc4d1a400000000, 0xfbf4d6d300000000, 0x6ae9694300000000,
    0xfcd96e3400000000, 0x468867ad00000000, 0xd0b860da00000000,
    0x732d044400000000, 0xe51d033300000000, 0x5f4c0aaa00000000,
    0xc97c0ddd00000000, 0x3c71055000000000, 0xaa41022700000000,
    0x10100bbe00000000, 0x86200cc900000000, 0x25b5685700000000,
    0xb3856f2000000000, 0x09d466b900000000, 0x9fe461ce00000000,
    0x0ef9de5e00000000, 0x98c9d92900000000, 0x2298d0b000000000,
    0xb4a8d7c700000000, 0x173db35900000000, 0x810db42e00000000,
    0x3b5cbdb700000000, 0xad6cbac000000000, 0x2083b8ed00000000,
    0xb6b3bf9a00000000, 0x0ce2b60300000000, 0x9ad2b17400000000,
    0x3947d5ea00000000, 0xaf77d29d00000000, 0x1526db0400000000,
    0x8316dc7300000000, 0x120b63e300000000, 0x843b649400000000,
    0x3e6a6d0d00000000, 0xa85a6a7a00000000, 0x0bcf0ee400000000,
    0x9dff099300000000, 0x27ae000a00000000, 0xb19e077d00000000,
    0x44930ff000000000, 0xd2a3088700000000, 0x68f2011e00000000,
    0xfec2066900000000, 0x5d5762f700000000, 0xcb67658000000000,
    0x71366c1900000000, 0xe7066b6e00000000, 0x761bd4fe00000000,
    0xe02bd38900000000, 0x5a7ada1000000000, 0xcc4add6700000000,
    0x6fdfb9f900000000, 0xf9efbe8e00000000, 0x43beb71700000000,
    0xd58eb06000000000, 0xe8a3d6d600000000, 0x7e93d1a100000000,
    0xc4c2d83800000000, 0x52f2df4f00000000, 0xf167bbd100000000,
    0x6757bca600000000, 0xdd06b53f00000000, 0x4b36b24800000000,
    0xda2b0dd800000000, 0x4c1b0aaf00000000, 0xf64a033600000000,
    0x607a044100000000, 0xc3ef60df00000000, 0x55df67a800000000,
    0xef8e6e3100000000, 0x79be694600000000, 0x8cb361cb00000000,
    0x1a8366bc00000000, 0xa0d26f2500000000, 0x36e2685200000000,
    0x95770ccc00000000, 0x03470bbb00000000, 0xb916022200000000,
    0x2f26055500000000, 0xbe3bbac500000000, 0x280bbdb200000000,
    0x925ab42b00000000, 0x046ab35c00000000, 0xa7ffd7c200000000,
    0x31cfd0b500000000, 0x8b9ed92c00000000, 0x1daede5b00000000,
    0xb0c2649b00000000, 0x26f263ec00000000, 0x9ca36a7500000000,
    0x0a936d0200000000, 0xa906099c00000000, 0x3f360eeb00000000,
    0x8567077200000000, 0x1357000500000000, 0x824abf9500000000,
    0x147ab8e200000000, 0xae2bb17b00000000, 0x381bb60c00000000,
    0x9b8ed29200000000, 0x0dbed5e500000000, 0xb7efdc7c00000000,
    0x21dfdb0b00000000, 0xd4d2d38600000000, 0x42e2d4f100000000,
    0xf8b3dd6800000000, 0x6e83da1f00000000, 0xcd16be8100000000,
    0x5b26b9f600000000, 0xe177b06f00000000, 0x7747b71800000000,
    0xe65a088800000000, 0x706a0fff00000000, 0xca3b066600000000,
    0x5c0b011100000000, 0xff9e658f00000000, 0x69ae62f800000000,
    0xd3ff6b6100000000, 0x45cf6c1600000000, 0x78e20aa000000000,
    0xeed20dd700000000, 0x5483044e00000000, 0xc2b3033900000000,
    0x612667a700000000, 0xf71660d000000000, 0x4d47694900000000,
    0xdb776e3e00000000, 0x4a6ad1ae00000000, 0xdc5ad6d900000000,
    0x660bdf4000000000, 0xf03bd83700000000, 0x53aebca900000000,
    0xc59ebbde00000000, 0x7fcfb24700000000, 0xe9ffb53000000000,
    0x1cf2bdbd00000000, 0x8ac2baca00000000, 0x3093b35300000000,
    0xa6a3b42400000000, 0x0536d0ba00000000, 0x9306d7cd00000000,
    0x2957de5400000000, 0xbf67d92300000000, 0x2e7a66b300000000,
    0xb84a61c400000000, 0x021b685d00000000, 0x942b6f2a00000000,
    0x37be0bb400000000, 0xa18e0cc300000000, 0x1bdf055a00000000,
    0x8def022d00000000},
   {0x0000000000000000, 0x41311b1900000000, 0x8262363200000000,
    0xc3532d2b00000000, 0x04c56c6400000000, 0x45f4777d00000000,
    0x86a75a5600000000, 0xc796414f00000000, 0x088ad9c800000000,
    0x49bbc2d100000000, 0x8ae8effa00000000, 0xcbd9f4e300000000,
    0x0c4fb5ac00000000, 0x4d7eaeb500000000, 0x8e2d839e00000000,
    0xcf1c988700000000, 0x5112c24a00000000, 0x1023d95300000000,
    0xd370f47800000000, 0x9241ef6100000000, 0x55d7ae2e00000000,
    0x14e6b53700000000, 0xd7b5981c00000000, 0x9684830500000000,
    0x59981b8200000000, 0x18a9009b00000000, 0xdbfa2db000000000,
    0x9acb36a900000000, 0x5d5d77e600000000, 0x1c6c6cff00000000,
    0xdf3f41d400000000, 0x9e0e5acd00000000, 0xa224849500000000,
    0xe3159f8c00000000, 0x2046b2a700000000, 0x6177a9be00000000,
    0xa6e1e8f100000000, 0xe7d0f3e800000000, 0x2483dec300000000,
    0x65b2c5da00000000, 0xaaae5d5d00000000, 0xeb9f464400000000,
    0x28cc6b6f00000000, 0x69fd707600000000, 0xae6b313900000000,
    0xef5a2a2000000000, 0x2c09070b00000000, 0x6d381c1200000000,
    0xf33646df00000000, 0xb2075dc600000000, 0x715470ed00000000,
    0x30656bf400000000, 0xf7f32abb00000000, 0xb6c231a200000000,
    0x75911c8900000000, 0x34a0079000000000, 0xfbbc9f1700000000,
    0xba8d840e00000000, 0x79dea92500000000, 0x38efb23c00000000,
    0xff79f37300000000, 0xbe48e86a00000000, 0x7d1bc54100000000,
    0x3c2ade5800000000, 0x054f79f000000000, 0x447e62e900000000,
    0x872d4fc200000000, 0xc61c54db00000000, 0x018a159400000000,
    0x40bb0e8d00000000, 0x83e823a600000000, 0xc2d938bf00000000,
    0x0dc5a03800000000, 0x4cf4bb2100000000, 0x8fa7960a00000000,
    0xce968d1300000000, 0x0900cc5c00000000, 0x4831d74500000000,
    0x8b62fa6e00000000, 0xca53e17700000000, 0x545dbbba00000000,
    0x156ca0a300000000, 0xd63f8d8800000000, 0x970e969100000000,
    0x5098d7de00000000, 0x11a9ccc700000000, 0xd2fae1ec00000000,
    0x93cbfaf500000000, 0x5cd7627200000000, 0x1de6796b00000000,
    0xdeb5544000000000, 0x9f844f5900000000, 0x58120e1600000000,
    0x1923150f00000000, 0xda70382400000000, 0x9b41233d00000000,
    0xa76bfd6500000000, 0xe65ae67c00000000, 0x2509cb5700000000,
    0x6438d04e00000000, 0xa3ae910100000000, 0xe29f8a1800000000,
    0x21cca73300000000, 0x60fdbc2a00000000, 0xafe124ad00000000,
    0xeed03fb400000000, 0x2d83129f00000000, 0x6cb2098600000000,
    0xab2448c900000000, 0xea1553d000000000, 0x29467efb00000000,
    0x687765e200000000, 0xf6793f2f00000000, 0xb748243600000000,
    0x741b091d00000000, 0x352a120400000000, 0xf2bc534b00000000,
    0xb38d485200000000, 0x70de657900000000, 0x31ef7e6000000000,
    0xfef3e6e700000000, 0xbfc2fdfe00000000, 0x7c91d0d500000000,
    0x3da0cbcc00000000, 0xfa368a8300000000, 0xbb07919a00000000,
    0x7854bcb100000000, 0x3965a7a800000000, 0x4b98833b00000000,
    0x0aa9982200000000, 0xc9fab50900000000, 0x88cbae1000000000,
    0x4f5def5f00000000, 0x0e6cf44600000000, 0xcd3fd96d00000000,
    0x8c0ec27400000000, 0x43125af300000000, 0x022341ea00000000,
    0xc1706cc100000000, 0x804177d800000000, 0x47d7369700000000,
    0x06e62d8e00000000, 0xc5b500a500000000, 0x84841bbc00000000,
    0x1a8a417100000000, 0x5bbb5a6800000000, 0x98e8774300000000,
    0xd9d96c5a00000000, 0x1e4f2d1500000000, 0x5f7e360c00000000,
    0x9c2d1b2700000000, 0xdd1c003e00000000, 0x120098b900000000,
    0x533183a000000000, 0x9062ae8b00000000, 0xd153b59200000000,
    0x16c5f4dd00000000, 0x57f4efc400000000, 0x94a7c2ef00000000,
    0xd596d9f600000000, 0xe9bc07ae00000000, 0xa88d1cb700000000,
    0x6bde319c00000000, 0x2aef2a8500000000, 0xed796bca00000000,
    0xac4870d300000000, 0x6f1b5df800000000, 0x2e2a46e100000000,
    0xe136de6600000000, 0xa007c57f00000000, 0x6354e85400000000,
    0x2265f34d00000000, 0xe5f3b20200000000, 0xa4c2a91b00000000,
    0x6791843000000000, 0x26a09f2900000000, 0xb8aec5e400000000,
    0xf99fdefd00000000, 0x3accf3d600000000, 0x7bfde8cf00000000,
    0xbc6ba98000000000, 0xfd5ab29900000000, 0x3e099fb200000000,
    0x7f3884ab00000000, 0xb0241c2c00000000, 0xf115073500000000,
    0x32462a1e00000000, 0x7377310700000000, 0xb4e1704800000000,
    0xf5d06b5100000000, 0x3683467a00000000, 0x77b25d6300000000,
    0x4ed7facb00000000, 0x0fe6e1d200000000, 0xccb5ccf900000000,
    0x8d84d7e000000000, 0x4a1296af00000000, 0x0b238db600000000,
    0xc870a09d00000000, 0x8941bb8400000000, 0x465d230300000000,
    0x076c381a00000000, 0xc43f153100000000, 0x850e0e2800000000,
    0x42984f6700000000, 0x03a9547e00000000, 0xc0fa795500000000,
    0x81cb624c00000000, 0x1fc5388100000000, 0x5ef4239800000000,
    0x9da70eb300000000, 0xdc9615aa00000000, 0x1b0054e500000000,
    0x5a314ffc00000000, 0x996262d700000000, 0xd85379ce00000000,
    0x174fe14900000000, 0x567efa5000000000, 0x952dd77b00000000,
    0xd41ccc6200000000, 0x138a8d2d00000000, 0x52bb963400000000,
    0x91e8bb1f00000000, 0xd0d9a00600000000, 0xecf37e5e00000000,
    0xadc2654700000000, 0x6e91486c00000000, 0x2fa0537500000000,
    0xe836123a00000000, 0xa907092300000000, 0x6a54240800000000,
    0x2b653f1100000000, 0xe479a79600000000, 0xa548bc8f00000000,
    0x661b91a400000000, 0x272a8abd00000000, 0xe0bccbf200000000,
    0xa18dd0eb00000000, 0x62defdc000000000, 0x23efe6d900000000,
    0xbde1bc1400000000, 0xfcd0a70d00000000, 0x3f838a2600000000,
    0x7eb2913f00000000, 0xb924d07000000000, 0xf815cb6900000000,
    0x3b46e64200000000, 0x7a77fd5b00000000, 0xb56b65dc00000000,
    0xf45a7ec500000000, 0x370953ee00000000, 0x763848f700000000,
    0xb1ae09b800000000, 0xf09f12a100000000, 0x33cc3f8a00000000,
    0x72fd249300000000},
   {0x0000000000000000, 0x376ac20100000000, 0x6ed4840300000000,
    0x59be460200000000, 0xdca8090700000000, 0xebc2cb0600000000,
    0xb27c8d0400000000, 0x85164f0500000000, 0xb851130e00000000,
    0x8f3bd10f00000000, 0xd685970d00000000, 0xe1ef550c00000000,
    0x64f91a0900000000, 0x5393d80800000000, 0x0a2d9e0a00000000,
    0x3d475c0b00000000, 0x70a3261c00000000, 0x47c9e41d00000000,
    0x1e77a21f00000000, 0x291d601e00000000, 0xac0b2f1b00000000,
    0x9b61ed1a00000000, 0xc2dfab1800000000, 0xf5b5691900000000,
    0xc8f2351200000000, 0xff98f71300000000, 0xa626b11100000000,
    0x914c731000000000, 0x145a3c1500000000, 0x2330fe1400000000,
    0x7a8eb81600000000, 0x4de47a1700000000, 0xe0464d3800000000,
    0xd72c8f3900000000, 0x8e92c93b00000000, 0xb9f80b3a00000000,
    0x3cee443f00000000, 0x0b84863e00000000, 0x523ac03c00000000,
    0x6550023d00000000, 0x58175e3600000000, 0x6f7d9c3700000000,
    0x36c3da3500000000, 0x01a9183400000000, 0x84bf573100000000,
    0xb3d5953000000000, 0xea6bd33200000000, 0xdd01113300000000,
    0x90e56b2400000000, 0xa78fa92500000000, 0xfe31ef2700000000,
    0xc95b2d2600000000, 0x4c4d622300000000, 0x7b27a02200000000,
    0x2299e62000000000, 0x15f3242100000000, 0x28b4782a00000000,
    0x1fdeba2b00000000, 0x4660fc2900000000, 0x710a3e2800000000,
    0xf41c712d00000000, 0xc376b32c00000000, 0x9ac8f52e00000000,
    0xada2372f00000000, 0xc08d9a7000000000, 0xf7e7587100000000,
    0xae591e7300000000, 0x9933dc7200000000, 0x1c25937700000000,
    0x2b4f517600000000, 0x72f1177400000000, 0x459bd57500000000,
    0x78dc897e00000000, 0x4fb64b7f00000000, 0x16080d7d00000000,
    0x2162cf7c00000000, 0xa474807900000000, 0x931e427800000000,
    0xcaa0047a00000000, 0xfdcac67b00000000, 0xb02ebc6c00000000,
    0x87447e6d00000000, 0xdefa386f00000000, 0xe990fa6e00000000,
    0x6c86b56b00000000, 0x5bec776a00000000, 0x0252316800000000,
    0x3538f36900000000, 0x087faf6200000000, 0x3f156d6300000000,
    0x66ab2b6100000000, 0x51c1e96000000000, 0xd4d7a66500000000,
    0xe3bd646400000000, 0xba03226600000000, 0x8d69e06700000000,
    0x20cbd74800000000, 0x17a1154900000000, 0x4e1f534b00000000,
    0x7975914a00000000, 0xfc63de4f00000000, 0xcb091c4e00000000,
    0x92b75a4c00000000, 0xa5dd984d00000000, 0x989ac44600000000,
    0xaff0064700000000, 0xf64e404500000000, 0xc124824400000000,
    0x4432cd4100000000, 0x73580f4000000000, 0x2ae6494200000000,
    0x1d8c8b4300000000, 0x5068f15400000000, 0x6702335500000000,
    0x3ebc755700000000, 0x09d6b75600000000, 0x8cc0f85300000000,
    0xbbaa3a5200000000, 0xe2147c5000000000, 0xd57ebe5100000000,
    0xe839e25a00000000, 0xdf53205b00000000, 0x86ed665900000000,
    0xb187a45800000000, 0x3491eb5d00000000, 0x03fb295c00000000,
    0x5a456f5e00000000, 0x6d2fad5f00000000, 0x801b35e100000000,
    0xb771f7e000000000, 0xeecfb1e200000000, 0xd9a573e300000000,
    0x5cb33ce600000000, 0x6bd9fee700000000, 0x3267b8e500000000,
    0x050d7ae400000000, 0x384a26ef00000000, 0x0f20e4ee00000000,
    0x569ea2ec00000000, 0x61f460ed00000000, 0xe4e22fe800000000,
    0xd388ede900000000, 0x8a36abeb00000000, 0xbd5c69ea00000000,
    0xf0b813fd00000000, 0xc7d2d1fc00000000, 0x9e6c97fe00000000,
    0xa90655ff00000000, 0x2c101afa00000000, 0x1b7ad8fb00000000,
    0x42c49ef900000000, 0x75ae5cf800000000, 0x48e900f300000000,
    0x7f83c2f200000000, 0x263d84f000000000, 0x115746f100000000,
    0x944109f400000000, 0xa32bcbf500000000, 0xfa958df700000000,
    0xcdff4ff600000000, 0x605d78d900000000, 0x5737bad800000000,
    0x0e89fcda00000000, 0x39e33edb00000000, 0xbcf571de00000000,
    0x8b9fb3df00000000, 0xd221f5dd00000000, 0xe54b37dc00000000,
    0xd80c6bd700000000, 0xef66a9d600000000, 0xb6d8efd400000000,
    0x81b22dd500000000, 0x04a462d000000000, 0x33cea0d100000000,
    0x6a70e6d300000000, 0x5d1a24d200000000, 0x10fe5ec500000000,
    0x27949cc400000000, 0x7e2adac600000000, 0x494018c700000000,
    0xcc5657c200000000, 0xfb3c95c300000000, 0xa282d3c100000000,
    0x95e811c000000000, 0xa8af4dcb00000000, 0x9fc58fca00000000,
    0xc67bc9c800000000, 0xf1110bc900000000, 0x740744cc00000000,
    0x436d86cd00000000, 0x1ad3c0cf00000000, 0x2db902ce00000000,
    0x4096af9100000000, 0x77fc6d9000000000, 0x2e422b9200000000,
    0x1928e99300000000, 0x9c3ea69600000000, 0xab54649700000000,
    0xf2ea229500000000, 0xc580e09400000000, 0xf8c7bc9f00000000,
    0xcfad7e9e00000000, 0x9613389c00000000, 0xa179fa9d00000000,
    0x246fb59800000000, 0x1305779900000000, 0x4abb319b00000000,
    0x7dd1f39a00000000, 0x3035898d00000000, 0x075f4b8c00000000,
    0x5ee10d8e00000000, 0x698bcf8f00000000, 0xec9d808a00000000,
    0xdbf7428b00000000, 0x8249048900000000, 0xb523c68800000000,
    0x88649a8300000000, 0xbf0e588200000000, 0xe6b01e8000000000,
    0xd1dadc8100000000, 0x54cc938400000000, 0x63a6518500000000,
    0x3a18178700000000, 0x0d72d58600000000, 0xa0d0e2a900000000,
    0x97ba20a800000000, 0xce0466aa00000000, 0xf96ea4ab00000000,
    0x7c78ebae00000000, 0x4b1229af00000000, 0x12ac6fad00000000,
    0x25c6adac00000000, 0x1881f1a700000000, 0x2feb33a600000000,
    0x765575a400000000, 0x413fb7a500000000, 0xc429f8a000000000,
    0xf3433aa100000000, 0xaafd7ca300000000, 0x9d97bea200000000,
    0xd073c4b500000000, 0xe71906b400000000, 0xbea740b600000000,
    0x89cd82b700000000, 0x0cdbcdb200000000, 0x3bb10fb300000000,
    0x620f49b100000000, 0x55658bb000000000, 0x6822d7bb00000000,
    0x5f4815ba00000000, 0x06f653b800000000, 0x319c91b900000000,
    0xb48adebc00000000, 0x83e01cbd00000000, 0xda5e5abf00000000,
    0xed3498be00000000},
   {0x0000000000000000, 0x6567bcb800000000, 0x8bc809aa00000000,
    0xeeafb51200000000, 0x5797628f00000000, 0x32f0de3700000000,
    0xdc5f6b2500000000, 0xb938d79d00000000, 0xef28b4c500000000,
    0x8a4f087d00000000, 0x64e0bd6f00000000, 0x018701d700000000,
    0xb8bfd64a00000000, 0xddd86af200000000, 0x3377dfe000000000,
    0x5610635800000000, 0x9f57195000000000, 0xfa30a5e800000000,
    0x149f10fa00000000, 0x71f8ac4200000000, 0xc8c07bdf00000000,
    0xada7c76700000000, 0x4308727500000000, 0x266fcecd00000000,
    0x707fad9500000000, 0x1518112d00000000, 0xfbb7a43f00000000,
    0x9ed0188700000000, 0x27e8cf1a00000000, 0x428f73a200000000,
    0xac20c6b000000000, 0xc9477a0800000000, 0x3eaf32a000000000,
    0x5bc88e1800000000, 0xb5673b0a00000000, 0xd00087b200000000,
    0x6938502f00000000, 0x0c5fec9700000000, 0xe2f0598500000000,
    0x8797e53d00000000, 0xd187866500000000, 0xb4e03add00000000,
    0x5a4f8fcf00000000, 0x3f28337700000000, 0x8610e4ea00000000,
    0xe377585200000000, 0x0dd8ed4000000000, 0x68bf51f800000000,
    0xa1f82bf000000000, 0xc49f974800000000, 0x2a30225a00000000,
    0x4f579ee200000000, 0xf66f497f00000000, 0x9308f5c700000000,
    0x7da740d500000000, 0x18c0fc6d00000000, 0x4ed09f3500000000,
    0x2bb7238d00000000, 0xc518969f00000000, 0xa07f2a2700000000,
    0x1947fdba00000000, 0x7c20410200000000, 0x928ff41000000000,
    0xf7e848a800000000, 0x3d58149b00000000, 0x583fa82300000000,
    0xb6901d3100000000, 0xd3f7a18900000000, 0x6acf761400000000,
    0x0fa8caac00000000, 0xe1077fbe00000000, 0x8460c30600000000,
    0xd270a05e00000000, 0xb7171ce600000000, 0x59b8a9f400000000,
    0x3cdf154c00000000, 0x85e7c2d100000000, 0xe0807e6900000000,
    0x0e2fcb7b00000000, 0x6b4877c300000000, 0xa20f0dcb00000000,
    0xc768b17300000000, 0x29c7046100000000, 0x4ca0b8d900000000,
    0xf5986f4400000000, 0x90ffd3fc00000000, 0x7e5066ee00000000,
    0x1b37da5600000000, 0x4d27b90e00000000, 0x284005b600000000,
    0xc6efb0a400000000, 0xa3880c1c00000000, 0x1ab0db8100000000,
    0x7fd7673900000000, 0x9178d22b00000000, 0xf41f6e9300000000,
    0x03f7263b00000000, 0x66909a8300000000, 0x883f2f9100000000,
    0xed58932900000000, 0x546044b400000000, 0x3107f80c00000000,
    0xdfa84d1e00000000, 0xbacff1a600000000, 0xecdf92fe00000000,
    0x89b82e4600000000, 0x67179b5400000000, 0x027027ec00000000,
    0xbb48f07100000000, 0xde2f4cc900000000, 0x3080f9db00000000,
    0x55e7456300000000, 0x9ca03f6b00000000, 0xf9c783d300000000,
    0x176836c100000000, 0x720f8a7900000000, 0xcb375de400000000,
    0xae50e15c00000000, 0x40ff544e00000000, 0x2598e8f600000000,
    0x73888bae00000000, 0x16ef371600000000, 0xf840820400000000,
    0x9d273ebc00000000, 0x241fe92100000000, 0x4178559900000000,
    0xafd7e08b00000000, 0xcab05c3300000000, 0x3bb659ed00000000,
    0x5ed1e55500000000, 0xb07e504700000000, 0xd519ecff00000000,
    0x6c213b6200000000, 0x094687da00000000, 0xe7e932c800000000,
    0x828e8e7000000000, 0xd49eed2800000000, 0xb1f9519000000000,
    0x5f56e48200000000, 0x3a31583a00000000, 0x83098fa700000000,
    0xe66e331f00000000, 0x08c1860d00000000, 0x6da63ab500000000,
    0xa4e140bd00000000, 0xc186fc0500000000, 0x2f29491700000000,
    0x4a4ef5af00000000, 0xf376223200000000, 0x96119e8a00000000,
    0x78be2b9800000000, 0x1dd9972000000000, 0x4bc9f47800000000,
    0x2eae48c000000000, 0xc001fdd200000000, 0xa566416a00000000,
    0x1c5e96f700000000, 0x79392a4f00000000, 0x97969f5d00000000,
    0xf2f123e500000000, 0x05196b4d00000000, 0x607ed7f500000000,
    0x8ed162e700000000, 0xebb6de5f00000000, 0x528e09c200000000,
    0x37e9b57a00000000, 0xd946006800000000, 0xbc21bcd000000000,
    0xea31df8800000000, 0x8f56633000000000, 0x61f9d62200000000,
    0x049e6a9a00000000, 0xbda6bd0700000000, 0xd8c101bf00000000,
    0x366eb4ad00000000, 0x5309081500000000, 0x9a4e721d00000000,
    0xff29cea500000000, 0x11867bb700000000, 0x74e1c70f00000000,
    0xcdd9109200000000, 0xa8beac2a00000000, 0x4611193800000000,
    0x2376a58000000000, 0x7566c6d800000000, 0x10017a6000000000,
    0xfeaecf7200000000, 0x9bc973ca00000000, 0x22f1a45700000000,
    0x479618ef00000000, 0xa939adfd00000000, 0xcc5e114500000000,
    0x06ee4d7600000000, 0x6389f1ce00000000, 0x8d2644dc00000000,
    0xe841f86400000000, 0x51792ff900000000, 0x341e934100000000,
    0xdab1265300000000, 0xbfd69aeb00000000, 0xe9c6f9b300000000,
    0x8ca1450b00000000, 0x620ef01900000000, 0x07694ca100000000,
    0xbe519b3c00000000, 0xdb36278400000000, 0x3599929600000000,
    0x50fe2e2e00000000, 0x99b9542600000000, 0xfcdee89e00000000,
    0x12715d8c00000000, 0x7716e13400000000, 0xce2e36a900000000,
    0xab498a1100000000, 0x45e63f0300000000, 0x208183bb00000000,
    0x7691e0e300000000, 0x13f65c5b00000000, 0xfd59e94900000000,
    0x983e55f100000000, 0x2106826c00000000, 0x44613ed400000000,
    0xaace8bc600000000, 0xcfa9377e00000000, 0x38417fd600000000,
    0x5d26c36e00000000, 0xb389767c00000000, 0xd6eecac400000000,
    0x6fd61d5900000000, 0x0ab1a1e100000000, 0xe41e14f300000000,
    0x8179a84b00000000, 0xd769cb1300000000, 0xb20e77ab00000000,
    0x5ca1c2b900000000, 0x39c67e0100000000, 0x80fea99c00000000,
    0xe599152400000000, 0x0b36a03600000000, 0x6e511c8e00000000,
    0xa716668600000000, 0xc271da3e00000000, 0x2cde6f2c00000000,
    0x49b9d39400000000, 0xf081040900000000, 0x95e6b8b100000000,
    0x7b490da300000000, 0x1e2eb11b00000000, 0x483ed24300000000,
    0x2d596efb00000000, 0xc3f6dbe900000000, 0xa691675100000000,
    0x1fa9b0cc00000000, 0x7ace0c7400000000, 0x9461b96600000000,
    0xf10605de00000000},
   {0x0000000000000000, 0xb029603d00000000, 0x6053c07a00000000,
    0xd07aa04700000000, 0xc0a680f500000000, 0x708fe0c800000000,
    0xa0f5408f00000000, 0x10dc20b200000000, 0xc14b703000000000,
    0x7162100d00000000, 0xa118b04a00000000, 0x1131d07700000000,
    0x01edf0c500000000, 0xb1c490f800000000, 0x61be30bf00000000,
    0xd197508200000000, 0x8297e06000000000, 0x32be805d00000000,
    0xe2c4201a00000000, 0x52ed402700000000, 0x4231609500000000,
    0xf21800a800000000, 0x2262a0ef00000000, 0x924bc0d200000000,
    0x43dc905000000000, 0xf3f5f06d00000000, 0x238f502a00000000,
    0x93a6301700000000, 0x837a10a500000000, 0x3353709800000000,
    0xe329d0df00000000, 0x5300b0e200000000, 0x042fc1c100000000,
    0xb406a1fc00000000, 0x647c01bb00000000, 0xd455618600000000,
    0xc489413400000000, 0x74a0210900000000, 0xa4da814e00000000,
    0x14f3e17300000000, 0xc564b1f100000000, 0x754dd1cc00000000,
    0xa537718b00000000, 0x151e11b600000000, 0x05c2310400000000,
    0xb5eb513900000000, 0x6591f17e00000000, 0xd5b8914300000000,
    0x86b821a100000000, 0x3691419c00000000, 0xe6ebe1db00000000,
    0x56c281e600000000, 0x461ea15400000000, 0xf637c16900000000,
    0x264d612e00000000, 0x9664011300000000, 0x47f3519100000000,
    0xf7da31ac00000000, 0x27a091eb00000000, 0x9789f1d600000000,
    0x8755d16400000000, 0x377cb15900000000, 0xe706111e00000000,
    0x572f712300000000, 0x4958f35800000000, 0xf971936500000000,
    0x290b332200000000, 0x9922531f00000000, 0x89fe73ad00000000,
    0x39d7139000000000, 0xe9adb3d700000000, 0x5984d3ea00000000,
    0x8813836800000000, 0x383ae35500000000, 0xe840431200000000,
    0x5869232f00000000, 0x48b5039d00000000, 0xf89c63a000000000,
    0x28e6c3e700000000, 0x98cfa3da00000000, 0xcbcf133800000000,
    0x7be6730500000000, 0xab9cd34200000000, 0x1bb5b37f00000000,
    0x0b6993cd00000000, 0xbb40f3f000000000, 0x6b3a53b700000000,
    0xdb13338a00000000, 0x0a84630800000000, 0xbaad033500000000,
    0x6ad7a37200000000, 0xdafec34f00000000, 0xca22e3fd00000000,
    0x7a0b83c000000000, 0xaa71238700000000, 0x1a5843ba00000000,
    0x4d77329900000000, 0xfd5e52a400000000, 0x2d24f2e300000000,
    0x9d0d92de00000000, 0x8dd1b26c00000000, 0x3df8d25100000000,
    0xed82721600000000, 0x5dab122b00000000, 0x8c3c42a900000000,
    0x3c15229400000000, 0xec6f82d300000000, 0x5c46e2ee00000000,
    0x4c9ac25c00000000, 0xfcb3a26100000000, 0x2cc9022600000000,
    0x9ce0621b00000000, 0xcfe0d2f900000000, 0x7fc9b2c400000000,
    0xafb3128300000000, 0x1f9a72be00000000, 0x0f46520c00000000,
    0xbf6f323100000000, 0x6f15927600000000, 0xdf3cf24b00000000,
    0x0eaba2c900000000, 0xbe82c2f400000000, 0x6ef862b300000000,
    0xded1028e00000000, 0xce0d223c00000000, 0x7e24420100000000,
    0xae5ee24600000000, 0x1e77827b00000000, 0x92b0e6b100000000,
    0x2299868c00000000, 0xf2e326cb00000000, 0x42ca46f600000000,
    0x5216664400000000, 0xe23f067900000000, 0x3245a63e00000000,
    0x826cc60300000000, 0x53fb968100000000, 0xe3d2f6bc00000000,
    0x33a856fb00000000, 0x838136c600000000, 0x935d167400000000,
    0x2374764900000000, 0xf30ed60e00000000, 0x4327b63300000000,
    0x102706d100000000, 0xa00e66ec00000000, 0x7074c6ab00000000,
    0xc05da69600000000, 0xd081862400000000, 0x60a8e61900000000,
    0xb0d2465e00000000, 0x00fb266300000000, 0xd16c76e100000000,
    0x614516dc00000000, 0xb13fb69b00000000, 0x0116d6a600000000,
    0x11caf61400000000, 0xa1e3962900000000, 0x7199366e00000000,
    0xc1b0565300000000, 0x969f277000000000, 0x26b6474d00000000,
    0xf6cce70a00000000, 0x46e5873700000000, 0x5639a78500000000,
    0xe610c7b800000000, 0x366a67ff00000000, 0x864307c200000000,
    0x57d4574000000000, 0xe7fd377d00000000, 0x3787973a00000000,
    0x87aef70700000000, 0x9772d7b500000000, 0x275bb78800000000,
    0xf72117cf00000000, 0x470877f200000000, 0x1408c71000000000,
    0xa421a72d00000000, 0x745b076a00000000, 0xc472675700000000,
    0xd4ae47e500000000, 0x648727d800000000, 0xb4fd879f00000000,
    0x04d4e7a200000000, 0xd543b72000000000, 0x656ad71d00000000,
    0xb510775a00000000, 0x0539176700000000, 0x15e537d500000000,
    0xa5cc57e800000000, 0x75b6f7af00000000, 0xc59f979200000000,
    0xdbe815e900000000, 0x6bc175d400000000, 0xbbbbd59300000000,
    0x0b92b5ae00000000, 0x1b4e951c00000000, 0xab67f52100000000,
    0x7b1d556600000000, 0xcb34355b00000000, 0x1aa365d900000000,
    0xaa8a05e400000000, 0x7af0a5a300000000, 0xcad9c59e00000000,
    0xda05e52c00000000, 0x6a2c851100000000, 0xba56255600000000,
    0x0a7f456b00000000, 0x597ff58900000000, 0xe95695b400000000,
    0x392c35f300000000, 0x890555ce00000000, 0x99d9757c00000000,
    0x29f0154100000000, 0xf98ab50600000000, 0x49a3d53b00000000,
    0x983485b900000000, 0x281de58400000000, 0xf86745c300000000,
    0x484e25fe00000000, 0x5892054c00000000, 0xe8bb657100000000,
    0x38c1c53600000000, 0x88e8a50b00000000, 0xdfc7d42800000000,
    0x6feeb41500000000, 0xbf94145200000000, 0x0fbd746f00000000,
    0x1f6154dd00000000, 0xaf4834e000000000, 0x7f3294a700000000,
    0xcf1bf49a00000000, 0x1e8ca41800000000, 0xaea5c42500000000,
    0x7edf646200000000, 0xcef6045f00000000, 0xde2a24ed00000000,
    0x6e0344d000000000, 0xbe79e49700000000, 0x0e5084aa00000000,
    0x5d50344800000000, 0xed79547500000000, 0x3d03f43200000000,
    0x8d2a940f00000000, 0x9df6b4bd00000000, 0x2ddfd48000000000,
    0xfda574c700000000, 0x4d8c14fa00000000, 0x9c1b447800000000,
    0x2c32244500000000, 0xfc48840200000000, 0x4c61e43f00000000,
    0x5cbdc48d00000000, 0xec94a4b000000000, 0x3cee04f700000000,
    0x8cc764ca00000000},
   {0x0000000000000000, 0xa5d35ccb00000000, 0x0ba1c84d00000000,
    0xae72948600000000, 0x1642919b00000000, 0xb391cd5000000000,
    0x1de359d600000000, 0xb830051d00000000, 0x6d8253ec00000000,
    0xc8510f2700000000, 0x66239ba100000000, 0xc3f0c76a00000000,
    0x7bc0c27700000000, 0xde139ebc00000000, 0x70610a3a00000000,
    0xd5b256f100000000, 0x9b02d60300000000, 0x3ed18ac800000000,
    0x90a31e4e00000000, 0x3570428500000000, 0x8d40479800000000,
    0x28931b5300000000, 0x86e18fd500000000, 0x2332d31e00000000,
    0xf68085ef00000000, 0x5353d92400000000, 0xfd214da200000000,
    0x58f2116900000000, 0xe0c2147400000000, 0x451148bf00000000,
    0xeb63dc3900000000, 0x4eb080f200000000, 0x3605ac0700000000,
    0x93d6f0cc00000000, 0x3da4644a00000000, 0x9877388100000000,
    0x20473d9c00000000, 0x8594615700000000, 0x2be6f5d100000000,
    0x8e35a91a00000000, 0x5b87ffeb00000000, 0xfe54a32000000000,
    0x502637a600000000, 0xf5f56b6d00000000, 0x4dc56e7000000000,
    0xe81632bb00000000, 0x4664a63d00000000, 0xe3b7faf600000000,
    0xad077a0400000000, 0x08d426cf00000000, 0xa6a6b24900000000,
    0x0375ee8200000000, 0xbb45eb9f00000000, 0x1e96b75400000000,
    0xb0e423d200000000, 0x15377f1900000000, 0xc08529e800000000,
    0x6556752300000000, 0xcb24e1a500000000, 0x6ef7bd6e00000000,
    0xd6c7b87300000000, 0x7314e4b800000000, 0xdd66703e00000000,
    0x78b52cf500000000, 0x6c0a580f00000000, 0xc9d904c400000000,
    0x67ab904200000000, 0xc278cc8900000000, 0x7a48c99400000000,
    0xdf9b955f00000000, 0x71e901d900000000, 0xd43a5d1200000000,
    0x01880be300000000, 0xa45b572800000000, 0x0a29c3ae00000000,
    0xaffa9f6500000000, 0x17ca9a7800000000, 0xb219c6b300000000,
    0x1c6b523500000000, 0xb9b80efe00000000, 0xf7088e0c00000000,
    0x52dbd2c700000000, 0xfca9464100000000, 0x597a1a8a00000000,
    0xe14a1f9700000000, 0x4499435c00000000, 0xeaebd7da00000000,
    0x4f388b1100000000, 0x9a8adde000000000, 0x3f59812b00000000,
    0x912b15ad00000000, 0x34f8496600000000, 0x8cc84c7b00000000,
    0x291b10b000000000, 0x8769843600000000, 0x22bad8fd00000000,
    0x5a0ff40800000000, 0xffdca8c300000000, 0x51ae3c4500000000,
    0xf47d608e00000000, 0x4c4d659300000000, 0xe99e395800000000,
    0x47ecadde00000000, 0xe23ff11500000000, 0x378da7e400000000,
    0x925efb2f00000000, 0x3c2c6fa900000000, 0x99ff336200000000,
    0x21cf367f00000000, 0x841c6ab400000000, 0x2a6efe3200000000,
    0x8fbda2f900000000, 0xc10d220b00000000, 0x64de7ec000000000,
    0xcaacea4600000000, 0x6f7fb68d00000000, 0xd74fb39000000000,
    0x729cef5b00000000, 0xdcee7bdd00000000, 0x793d271600000000,
    0xac8f71e700000000, 0x095c2d2c00000000, 0xa72eb9aa00000000,
    0x02fde56100000000, 0xbacde07c00000000, 0x1f1ebcb700000000,
    0xb16c283100000000, 0x14bf74fa00000000, 0xd814b01e00000000,
    0x7dc7ecd500000000, 0xd3b5785300000000, 0x7666249800000000,
    0xce56218500000000, 0x6b857d4e00000000, 0xc5f7e9c800000000,
    0x6024b50300000000, 0xb596e3f200000000, 0x1045bf3900000000,
    0xbe372bbf00000000, 0x1be4777400000000, 0xa3d4726900000000,
    0x06072ea200000000, 0xa875ba2400000000, 0x0da6e6ef00000000,
    0x4316661d00000000, 0xe6c53ad600000000, 0x48b7ae5000000000,
    0xed64f29b00000000, 0x5554f78600000000, 0xf087ab4d00000000,
    0x5ef53fcb00000000, 0xfb26630000000000, 0x2e9435f100000000,
    0x8b47693a00000000, 0x2535fdbc00000000, 0x80e6a17700000000,
    0x38d6a46a00000000, 0x9d05f8a100000000, 0x33776c2700000000,
    0x96a430ec00000000, 0xee111c1900000000, 0x4bc240d200000000,
    0xe5b0d45400000000, 0x4063889f00000000, 0xf8538d8200000000,
    0x5d80d14900000000, 0xf3f245cf00000000, 0x5621190400000000,
    0x83934ff500000000, 0x2640133e00000000, 0x883287b800000000,
    0x2de1db7300000000, 0x95d1de6e00000000, 0x300282a500000000,
    0x9e70162300000000, 0x3ba34ae800000000, 0x7513ca1a00000000,
    0xd0c096d100000000, 0x7eb2025700000000, 0xdb615e9c00000000,
    0x63515b8100000000, 0xc682074a00000000, 0x68f093cc00000000,
    0xcd23cf0700000000, 0x189199f600000000, 0xbd42c53d00000000,
    0x133051bb00000000, 0xb6e30d7000000000, 0x0ed3086d00000000,
    0xab0054a600000000, 0x0572c02000000000, 0xa0a19ceb00000000,
    0xb41ee81100000000, 0x11cdb4da00000000, 0xbfbf205c00000000,
    0x1a6c7c9700000000, 0xa25c798a00000000, 0x078f254100000000,
    0xa9fdb1c700000000, 0x0c2eed0c00000000, 0xd99cbbfd00000000,
    0x7c4fe73600000000, 0xd23d73b000000000, 0x77ee2f7b00000000,
    0xcfde2a6600000000, 0x6a0d76ad00000000, 0xc47fe22b00000000,
    0x61acbee000000000, 0x2f1c3e1200000000, 0x8acf62d900000000,
    0x24bdf65f00000000, 0x816eaa9400000000, 0x395eaf8900000000,
    0x9c8df34200000000, 0x32ff67c400000000, 0x972c3b0f00000000,
    0x429e6dfe00000000, 0xe74d313500000000, 0x493fa5b300000000,
    0xececf97800000000, 0x54dcfc6500000000, 0xf10fa0ae00000000,
    0x5f7d342800000000, 0xfaae68e300000000, 0x821b441600000000,
    0x27c818dd00000000, 0x89ba8c5b00000000, 0x2c69d09000000000,
    0x9459d58d00000000, 0x318a894600000000, 0x9ff81dc000000000,
    0x3a2b410b00000000, 0xef9917fa00000000, 0x4a4a4b3100000000,
    0xe438dfb700000000, 0x41eb837c00000000, 0xf9db866100000000,
    0x5c08daaa00000000, 0xf27a4e2c00000000, 0x57a912e700000000,
    0x1919921500000000, 0xbccacede00000000, 0x12b85a5800000000,
    0xb76b069300000000, 0x0f5b038e00000000, 0xaa885f4500000000,
    0x04facbc300000000, 0xa129970800000000, 0x749bc1f900000000,
    0xd1489d3200000000, 0x7f3a09b400000000, 0xdae9557f00000000,
    0x62d9506200000000, 0xc70a0ca900000000, 0x6978982f00000000,
    0xccabc4e400000000},
   {0x0000000000000000, 0xb40b77a600000000, 0x29119f9700000000,
    0x9d1ae83100000000, 0x13244ff400000000, 0xa72f385200000000,
    0x3a35d06300000000, 0x8e3ea7c500000000, 0x674eef3300000000,
    0xd345989500000000, 0x4e5f70a400000000, 0xfa54070200000000,
    0x746aa0c700000000, 0xc061d76100000000, 0x5d7b3f5000000000,
    0xe97048f600000000, 0xce9cde6700000000, 0x7a97a9c100000000,
    0xe78d41f000000000, 0x5386365600000000, 0xddb8919300000000,
    0x69b3e63500000000, 0xf4a90e0400000000, 0x40a279a200000000,
    0xa9d2315400000000, 0x1dd946f200000000, 0x80c3aec300000000,
    0x34c8d96500000000, 0xbaf67ea000000000, 0x0efd090600000000,
    0x93e7e13700000000, 0x27ec969100000000, 0x9c39bdcf00000000,
    0x2832ca6900000000, 0xb528225800000000, 0x012355fe00000000,
    0x8f1df23b00000000, 0x3b16859d00000000, 0xa60c6dac00000000,
    0x12071a0a00000000, 0xfb7752fc00000000, 0x4f7c255a00000000,
    0xd266cd6b00000000, 0x666dbacd00000000, 0xe8531d0800000000,
    0x5c586aae00000000, 0xc142829f00000000, 0x7549f53900000000,
    0x52a563a800000000, 0xe6ae140e00000000, 0x7bb4fc3f00000000,
    0xcfbf8b9900000000, 0x41812c5c00000000, 0xf58a5bfa00000000,
    0x6890b3cb00000000, 0xdc9bc46d00000000, 0x35eb8c9b00000000,
    0x81e0fb3d00000000, 0x1cfa130c00000000, 0xa8f164aa00000000,
    0x26cfc36f00000000, 0x92c4b4c900000000, 0x0fde5cf800000000,
    0xbbd52b5e00000000, 0x79750b4400000000, 0xcd7e7ce200000000,
    0x506494d300000000, 0xe46fe37500000000, 0x6a5144b000000000,
    0xde5a331600000000, 0x4340db2700000000, 0xf74bac8100000000,
    0x1e3be47700000000, 0xaa3093d100000000, 0x372a7be000000000,
    0x83210c4600000000, 0x0d1fab8300000000, 0xb914dc2500000000,
    0x240e341400000000, 0x900543b200000000, 0xb7e9d52300000000,
    0x03e2a28500000000, 0x9ef84ab400000000, 0x2af33d1200000000,
    0xa4cd9ad700000000, 0x10c6ed7100000000, 0x8ddc054000000000,
    0x39d772e600000000, 0xd0a73a1000000000, 0x64ac4db600000000,
    0xf9b6a58700000000, 0x4dbdd22100000000, 0xc38375e400000000,
    0x7788024200000000, 0xea92ea7300000000, 0x5e999dd500000000,
    0xe54cb68b00000000, 0x5147c12d00000000, 0xcc5d291c00000000,
    0x78565eba00000000, 0xf668f97f00000000, 0x42638ed900000000,
    0xdf7966e800000000, 0x6b72114e00000000, 0x820259b800000000,
    0x36092e1e00000000, 0xab13c62f00000000, 0x1f18b18900000000,
    0x9126164c00000000, 0x252d61ea00000000, 0xb83789db00000000,
    0x0c3cfe7d00000000, 0x2bd068ec00000000, 0x9fdb1f4a00000000,
    0x02c1f77b00000000, 0xb6ca80dd00000000, 0x38f4271800000000,
    0x8cff50be00000000, 0x11e5b88f00000000, 0xa5eecf2900000000,
    0x4c9e87df00000000, 0xf895f07900000000, 0x658f184800000000,
    0xd1846fee00000000, 0x5fbac82b00000000, 0xebb1bf8d00000000,
    0x76ab57bc00000000, 0xc2a0201a00000000, 0xf2ea168800000000,
    0x46e1612e00000000, 0xdbfb891f00000000, 0x6ff0feb900000000,
    0xe1ce597c00000000, 0x55c52eda00000000, 0xc8dfc6eb00000000,
    0x7cd4b14d00000000, 0x95a4f9bb00000000, 0x21af8e1d00000000,
    0xbcb5662c00000000, 0x08be118a00000000, 0x8680b64f00000000,
    0x328bc1e900000000, 0xaf9129d800000000, 0x1b9a5e7e00000000,
    0x3c76c8ef00000000, 0x887dbf4900000000, 0x1567577800000000,
    0xa16c20de00000000, 0x2f52871b00000000, 0x9b59f0bd00000000,
    0x0643188c00000000, 0xb2486f2a00000000, 0x5b3827dc00000000,
    0xef33507a00000000, 0x7229b84b00000000, 0xc622cfed00000000,
    0x481c682800000000, 0xfc171f8e00000000, 0x610df7bf00000000,
    0xd506801900000000, 0x6ed3ab4700000000, 0xdad8dce100000000,
    0x47c234d000000000, 0xf3c9437600000000, 0x7df7e4b300000000,
    0xc9fc931500000000, 0x54e67b2400000000, 0xe0ed0c8200000000,
    0x099d447400000000, 0xbd9633d200000000, 0x208cdbe300000000,
    0x9487ac4500000000, 0x1ab90b8000000000, 0xaeb27c2600000000,
    0x33a8941700000000, 0x87a3e3b100000000, 0xa04f752000000000,
    0x1444028600000000, 0x895eeab700000000, 0x3d559d1100000000,
    0xb36b3ad400000000, 0x07604d7200000000, 0x9a7aa54300000000,
    0x2e71d2e500000000, 0xc7019a1300000000, 0x730aedb500000000,
    0xee10058400000000, 0x5a1b722200000000, 0xd425d5e700000000,
    0x602ea24100000000, 0xfd344a7000000000, 0x493f3dd600000000,
    0x8b9f1dcc00000000, 0x3f946a6a00000000, 0xa28e825b00000000,
    0x1685f5fd00000000, 0x98bb523800000000, 0x2cb0259e00000000,
    0xb1aacdaf00000000, 0x05a1ba0900000000, 0xecd1f2ff00000000,
    0x58da855900000000, 0xc5c06d6800000000, 0x71cb1ace00000000,
    0xfff5bd0b00000000, 0x4bfecaad00000000, 0xd6e4229c00000000,
    0x62ef553a00000000, 0x4503c3ab00000000, 0xf108b40d00000000,
    0x6c125c3c00000000, 0xd8192b9a00000000, 0x56278c5f00000000,
    0xe22cfbf900000000, 0x7f3613c800000000, 0xcb3d646e00000000,
    0x224d2c9800000000, 0x96465b3e00000000, 0x0b5cb30f00000000,
    0xbf57c4a900000000, 0x3169636c00000000, 0x856214ca00000000,
    0x1878fcfb00000000, 0xac738b5d00000000, 0x17a6a00300000000,
    0xa3add7a500000000, 0x3eb73f9400000000, 0x8abc483200000000,
    0x0482eff700000000, 0xb089985100000000, 0x2d93706000000000,
    0x999807c600000000, 0x70e84f3000000000, 0xc4e3389600000000,
    0x59f9d0a700000000, 0xedf2a70100000000, 0x63cc00c400000000,
    0xd7c7776200000000, 0x4add9f5300000000, 0xfed6e8f500000000,
    0xd93a7e6400000000, 0x6d3109c200000000, 0xf02be1f300000000,
    0x4420965500000000, 0xca1e319000000000, 0x7e15463600000000,
    0xe30fae0700000000, 0x5704d9a100000000, 0xbe74915700000000,
    0x0a7fe6f100000000, 0x97650ec000000000, 0x236e796600000000,
    0xad50dea300000000, 0x195ba90500000000, 0x8441413400000000,
    0x304a369200000000},
   {0x0000000000000000, 0x9e00aacc00000000, 0x7d07254200000000,
    0xe3078f8e00000000, 0xfa0e4a8400000000, 0x640ee04800000000,
    0x87096fc600000000, 0x1909c50a00000000, 0xb51be5d300000000,
    0x2b1b4f1f00000000, 0xc81cc09100000000, 0x561c6a5d00000000,
    0x4f15af5700000000, 0xd115059b00000000, 0x32128a1500000000,
    0xac1220d900000000, 0x2b31bb7c00000000, 0xb53111b000000000,
    0x56369e3e00000000, 0xc83634f200000000, 0xd13ff1f800000000,
    0x4f3f5b3400000000, 0xac38d4ba00000000, 0x32387e7600000000,
    0x9e2a5eaf00000000, 0x002af46300000000, 0xe32d7bed00000000,
    0x7d2dd12100000000, 0x6424142b00000000, 0xfa24bee700000000,
    0x1923316900000000, 0x87239ba500000000, 0x566276f900000000,
    0xc862dc3500000000, 0x2b6553bb00000000, 0xb565f97700000000,
    0xac6c3c7d00000000, 0x326c96b100000000, 0xd16b193f00000000,
    0x4f6bb3f300000000, 0xe379932a00000000, 0x7d7939e600000000,
    0x9e7eb66800000000, 0x007e1ca400000000, 0x1977d9ae00000000,
    0x8777736200000000, 0x6470fcec00000000, 0xfa70562000000000,
    0x7d53cd8500000000, 0xe353674900000000, 0x0054e8c700000000,
    0x9e54420b00000000, 0x875d870100000000, 0x195d2dcd00000000,
    0xfa5aa24300000000, 0x645a088f00000000, 0xc848285600000000,
    0x5648829a00000000, 0xb54f0d1400000000, 0x2b4fa7d800000000,
    0x324662d200000000, 0xac46c81e00000000, 0x4f41479000000000,
    0xd141ed5c00000000, 0xedc29d2900000000, 0x73c237e500000000,
    0x90c5b86b00000000, 0x0ec512a700000000, 0x17ccd7ad00000000,
    0x89cc7d6100000000, 0x6acbf2ef00000000, 0xf4cb582300000000,
    0x58d978fa00000000, 0xc6d9d23600000000, 0x25de5db800000000,
    0xbbdef77400000000, 0xa2d7327e00000000, 0x3cd798b200000000,
    0xdfd0173c00000000, 0x41d0bdf000000000, 0xc6f3265500000000,
    0x58f38c9900000000, 0xbbf4031700000000, 0x25f4a9db00000000,
    0x3cfd6cd100000000, 0xa2fdc61d00000000, 0x41fa499300000000,
    0xdffae35f00000000, 0x73e8c38600000000, 0xede8694a00000000,
    0x0eefe6c400000000, 0x90ef4c0800000000, 0x89e6890200000000,
    0x17e623ce00000000, 0xf4e1ac4000000000, 0x6ae1068c00000000,
    0xbba0ebd000000000, 0x25a0411c00000000, 0xc6a7ce9200000000,
    0x58a7645e00000000, 0x41aea15400000000, 0xdfae0b9800000000,
    0x3ca9841600000000, 0xa2a92eda00000000, 0x0ebb0e0300000000,
    0x90bba4cf00000000, 0x73bc2b4100000000, 0xedbc818d00000000,
    0xf4b5448700000000, 0x6ab5ee4b00000000, 0x89b261c500000000,
    0x17b2cb0900000000, 0x909150ac00000000, 0x0e91fa6000000000,
    0xed9675ee00000000, 0x7396df2200000000, 0x6a9f1a2800000000,
    0xf49fb0e400000000, 0x17983f6a00000000, 0x899895a600000000,
    0x258ab57f00000000, 0xbb8a1fb300000000, 0x588d903d00000000,
    0xc68d3af100000000, 0xdf84fffb00000000, 0x4184553700000000,
    0xa283dab900000000, 0x3c83707500000000, 0xda853b5300000000,
    0x4485919f00000000, 0xa7821e1100000000, 0x3982b4dd00000000,
    0x208b71d700000000, 0xbe8bdb1b00000000, 0x5d8c549500000000,
    0xc38cfe5900000000, 0x6f9ede8000000000, 0xf19e744c00000000,
    0x1299fbc200000000, 0x8c99510e00000000, 0x9590940400000000,
    0x0b903ec800000000, 0xe897b14600000000, 0x76971b8a00000000,
    0xf1b4802f00000000, 0x6fb42ae300000000, 0x8cb3a56d00000000,
    0x12b30fa100000000, 0x0bbacaab00000000, 0x95ba606700000000,
    0x76bdefe900000000, 0xe8bd452500000000, 0x44af65fc00000000,
    0xdaafcf3000000000, 0x39a840be00000000, 0xa7a8ea7200000000,
    0xbea12f7800000000, 0x20a185b400000000, 0xc3a60a3a00000000,
    0x5da6a0f600000000, 0x8ce74daa00000000, 0x12e7e76600000000,
    0xf1e068e800000000, 0x6fe0c22400000000, 0x76e9072e00000000,
    0xe8e9ade200000000, 0x0bee226c00000000, 0x95ee88a000000000,
    0x39fca87900000000, 0xa7fc02b500000000, 0x44fb8d3b00000000,
    0xdafb27f700000000, 0xc3f2e2fd00000000, 0x5df2483100000000,
    0xbef5c7bf00000000, 0x20f56d7300000000, 0xa7d6f6d600000000,
    0x39d65c1a00000000, 0xdad1d39400000000, 0x44d1795800000000,
    0x5dd8bc5200000000, 0xc3d8169e00000000, 0x20df991000000000,
    0xbedf33dc00000000, 0x12cd130500000000, 0x8ccdb9c900000000,
    0x6fca364700000000, 0xf1ca9c8b00000000, 0xe8c3598100000000,
    0x76c3f34d00000000, 0x95c47cc300000000, 0x0bc4d60f00000000,
    0x3747a67a00000000, 0xa9470cb600000000, 0x4a40833800000000,
    0xd44029f400000000, 0xcd49ecfe00000000, 0x5349463200000000,
    0xb04ec9bc00000000, 0x2e4e637000000000, 0x825c43a900000000,
    0x1c5ce96500000000, 0xff5b66eb00000000, 0x615bcc2700000000,
    0x7852092d00000000, 0xe652a3e100000000, 0x05552c6f00000000,
    0x9b5586a300000000, 0x1c761d0600000000, 0x8276b7ca00000000,
    0x6171384400000000, 0xff71928800000000, 0xe678578200000000,
    0x7878fd4e00000000, 0x9b7f72c000000000, 0x057fd80c00000000,
    0xa96df8d500000000, 0x376d521900000000, 0xd46add9700000000,
    0x4a6a775b00000000, 0x5363b25100000000, 0xcd63189d00000000,
    0x2e64971300000000, 0xb0643ddf00000000, 0x6125d08300000000,
    0xff257a4f00000000, 0x1c22f5c100000000, 0x82225f0d00000000,
    0x9b2b9a0700000000, 0x052b30cb00000000, 0xe62cbf4500000000,
    0x782c158900000000, 0xd43e355000000000, 0x4a3e9f9c00000000,
    0xa939101200000000, 0x3739bade00000000, 0x2e307fd400000000,
    0xb030d51800000000, 0x53375a9600000000, 0xcd37f05a00000000,
    0x4a146bff00000000, 0xd414c13300000000, 0x37134ebd00000000,
    0xa913e47100000000, 0xb01a217b00000000, 0x2e1a8bb700000000,
    0xcd1d043900000000, 0x531daef500000000, 0xff0f8e2c00000000,
    0x610f24e000000000, 0x8208ab6e00000000, 0x1c0801a200000000,
    0x0501c4a800000000, 0x9b016e6400000000, 0x7806e1ea00000000,
    0xe6064b2600000000}};

#else /* W == 4 */

local const z_crc_t FAR crc_braid_table[][256] = {
   {0x00000000, 0xb8bc6765, 0xaa09c88b, 0x12b5afee, 0x8f629757,
    0x37def032, 0x256b5fdc, 0x9dd738b9, 0xc5b428ef, 0x7d084f8a,
    0x6fbde064, 0xd7018701, 0x4ad6bfb8, 0xf26ad8dd, 0xe0df7733,
    0x58631056, 0x5019579f, 0xe8a530fa, 0xfa109f14, 0x42acf871,
    0xdf7bc0c8, 0x67c7a7ad, 0x75720843, 0xcdce6f26, 0x95ad7f70,
    0x2d111815, 0x3fa4b7fb, 0x8718d09e, 0x1acfe827, 0xa2738f42,
    0xb0c620ac, 0x087a47c9, 0xa032af3e, 0x188ec85b, 0x0a3b67b5,
    0xb28700d0, 0x2f503869, 0x97ec5f0c, 0x8559f0e2, 0x3de59787,
    0x658687d1, 0xdd3ae0b4, 0xcf8f4f5a, 0x7733283f, 0xeae41086,
    0x525877e3, 0x40edd80d, 0xf851bf68, 0xf02bf8a1, 0x48979fc4,
    0x5a22302a, 0xe29e574f, 0x7f496ff6, 0xc7f50893, 0xd540a77d,
    0x6dfcc018, 0x359fd04e, 0x8d23b72b, 0x9f9618c5, 0x272a7fa0,
    0xbafd4719, 0x0241207c, 0x10f48f92, 0xa848e8f7, 0x9b14583d,
    0x23a83f58, 0x311d90b6, 0x89a1f7d3, 0x1476cf6a, 0xaccaa80f,
    0xbe7f07e1, 0x06c36084, 0x5ea070d2, 0xe61c17b7, 0xf4a9b859,
    0x4c15df3c, 0xd1c2e785, 0x697e80e0, 0x7bcb2f0e, 0xc377486b,
    0xcb0d0fa2, 0x73b168c7, 0x6104c729, 0xd9b8a04c, 0x446f98f5,
    0xfcd3ff90, 0xee66507e, 0x56da371b, 0x0eb9274d, 0xb6054028,
    0xa4b0efc6, 0x1c0c88a3, 0x81dbb01a, 0x3967d77f, 0x2bd27891,
    0x936e1ff4, 0x3b26f703, 0x839a9066, 0x912f3f88, 0x299358ed,
    0xb4446054, 0x0cf80731, 0x1e4da8df, 0xa6f1cfba, 0xfe92dfec,
    0x462eb889, 0x549b1767, 0xec277002, 0x71f048bb, 0xc94c2fde,
    0xdbf98030, 0x6345e755, 0x6b3fa09c, 0xd383c7f9, 0xc1366817,
    0x798a0f72, 0xe45d37cb, 0x5ce150ae, 0x4e54ff40, 0xf6e89825,
    0xae8b8873, 0x1637ef16, 0x048240f8, 0xbc3e279d, 0x21e91f24,
    0x99557841, 0x8be0d7af, 0x335cb0ca, 0xed59b63b, 0x55e5d15e,
    0x47507eb0, 0xffec19d5, 0x623b216c, 0xda874609, 0xc832e9e7,
    0x708e8e82, 0x28ed9ed4, 0x9051f9b1, 0x82e4565f, 0x3a58313a,
    0xa78f0983, 0x1f336ee6, 0x0d86c108, 0xb53aa66d, 0xbd40e1a4,
    0x05fc86c1, 0x1749292f, 0xaff54e4a, 0x322276f3, 0x8a9e1196,
    0x982bbe78, 0x2097d91d, 0x78f4c94b, 0xc048ae2e, 0xd2fd01c0,
    0x6a4166a5, 0xf7965e1c, 0x4f2a3979, 0x5d9f9697, 0xe523f1f2,
    0x4d6b1905, 0xf5d77e60, 0xe762d18e, 0x5fdeb6eb, 0xc2098e52,
    0x7ab5e937, 0x680046d9, 0xd0bc21bc, 0x88df31ea, 0x3063568f,
    0x22d6f961, 0x9a6a9e04, 0x07bda6bd, 0xbf01c1d8, 0xadb46e36,
    0x15080953, 0x1d724e9a, 0xa5ce29ff, 0xb77b8611, 0x0fc7e174,
    0x9210d9cd, 0x2aacbea8, 0x38191146, 0x80a57623, 0xd8c66675,
    0x607a0110, 0x72cfaefe, 0xca73c99b, 0x57a4f122, 0xef189647,
    0xfdad39a9, 0x45115ecc, 0x764dee06, 0xcef18963, 0xdc44268d,
    0x64f841e8, 0xf92f7951, 0x41931e34, 0x5326b1da, 0xeb9ad6bf,
    0xb3f9c6e9, 0x0b45a18c, 0x19f00e62, 0xa14c6907, 0x3c9b51be,
    0x842736db, 0x96929935, 0x2e2efe50, 0x2654b999, 0x9ee8defc,
    0x8c5d7112, 0x34e11677, 0xa9362ece, 0x118a49ab, 0x033fe645,
    0xbb838120, 0xe3e09176, 0x5b5cf613, 0x49e959fd, 0xf1553e98,
    0x6c820621, 0xd43e6144, 0xc68bceaa, 0x7e37a9cf, 0xd67f4138,
    0x6ec3265d, 0x7c7689b3, 0xc4caeed6, 0x591dd66f, 0xe1a1b10a,
    0xf3141ee4, 0x4ba87981, 0x13cb69d7, 0xab770eb2, 0xb9c2a15c,
    0x017ec639, 0x9ca9fe80, 0x241599e5, 0x36a0360b, 0x8e1c516e,
    0x866616a7, 0x3eda71c2, 0x2c6fde2c, 0x94d3b949, 0x090481f0,
    0xb1b8e695, 0xa30d497b, 0x1bb12e1e, 0x43d23e48, 0xfb6e592d,
    0xe9dbf6c3, 0x516791a6, 0xccb0a91f, 0x740cce7a, 0x66b96194,
    0xde0506f1},
   {0x00000000, 0x01c26a37, 0x0384d46e, 0x0246be59, 0x0709a8dc,
    0x06cbc2eb, 0x048d7cb2, 0x054f1685, 0x0e1351b8, 0x0fd13b8f,
    0x0d9785d6, 0x0c55efe1, 0x091af964, 0x08d89353, 0x0a9e2d0a,
    0x0b5c473d, 0x1c26a370, 0x1de4c947, 0x1fa2771e, 0x1e601d29,
    0x1b2f0bac, 0x1aed619b, 0x18abdfc2, 0x1969b5f5, 0x1235f2c8,
    0x13f798ff, 0x11b126a6, 0x10734c91, 0x153c5a14, 0x14fe3023,
    0x16b88e7a, 0x177ae44d, 0x384d46e0, 0x398f2cd7, 0x3bc9928e,
    0x3a0bf8b9, 0x3f44ee3c, 0x3e86840b, 0x3cc03a52, 0x3d025065,
    0x365e1758, 0x379c7d6f, 0x35dac336, 0x3418a901, 0x3157bf84,
    0x3095d5b3, 0x32d36bea, 0x331101dd, 0x246be590, 0x25a98fa7,
    0x27ef31fe, 0x262d5bc9, 0x23624d4c, 0x22a0277b, 0x20e69922,
    0x2124f315, 0x2a78b428, 0x2bbade1f, 0x29fc6046, 0x283e0a71,
    0x2d711cf4, 0x2cb376c3, 0x2ef5c89a, 0x2f37a2ad, 0x709a8dc0,
    0x7158e7f7, 0x731e59ae, 0x72dc3399, 0x7793251c, 0x76514f2b,
    0x7417f172, 0x75d59b45, 0x7e89dc78, 0x7f4bb64f, 0x7d0d0816,
    0x7ccf6221, 0x798074a4, 0x78421e93, 0x7a04a0ca, 0x7bc6cafd,
    0x6cbc2eb0, 0x6d7e4487, 0x6f38fade, 0x6efa90e9, 0x6bb5866c,
    0x6a77ec5b, 0x68315202, 0x69f33835, 0x62af7f08, 0x636d153f,
    0x612bab66, 0x60e9c151, 0x65a6d7d4, 0x6464bde3, 0x662203ba,
    0x67e0698d, 0x48d7cb20, 0x4915a117, 0x4b531f4e, 0x4a917579,
    0x4fde63fc, 0x4e1c09cb, 0x4c5ab792, 0x4d98dda5, 0x46c49a98,
    0x4706f0af, 0x45404ef6, 0x448224c1, 0x41cd3244, 0x400f5873,
    0x4249e62a, 0x438b8c1d, 0x54f16850, 0x55330267, 0x5775bc3e,
    0x56b7d609, 0x53f8c08c, 0x523aaabb, 0x507c14e2, 0x51be7ed5,
    0x5ae239e8, 0x5b2053df, 0x5966ed86, 0x58a487b1, 0x5deb9134,
    0x5c29fb03, 0x5e6f455a, 0x5fad2f6d, 0xe1351b80, 0xe0f771b7,
    0xe2b1cfee, 0xe373a5d9, 0xe63cb35c, 0xe7fed96b, 0xe5b86732,
    0xe47a0d05, 0xef264a38, 0xeee4200f, 0xeca29e56, 0xed60f461,
    0xe82fe2e4, 0xe9ed88d3, 0xebab368a, 0xea695cbd, 0xfd13b8f0,
    0xfcd1d2c7, 0xfe976c9e, 0xff5506a9, 0xfa1a102c, 0xfbd87a1b,
    0xf99ec442, 0xf85cae75, 0xf300e948, 0xf2c2837f, 0xf0843d26,
    0xf1465711, 0xf4094194, 0xf5cb2ba3, 0xf78d95fa, 0xf64fffcd,
    0xd9785d60, 0xd8ba3757, 0xdafc890e, 0xdb3ee339, 0xde71f5bc,
    0xdfb39f8b, 0xddf521d2, 0xdc374be5, 0xd76b0cd8, 0xd6a966ef,
    0xd4efd8b6, 0xd52db281, 0xd062a404, 0xd1a0ce33, 0xd3e6706a,
    0xd2241a5d, 0xc55efe10, 0xc49c9427, 0xc6da2a7e, 0xc7184049,
    0xc25756cc, 0xc3953cfb, 0xc1d382a2, 0xc011e895, 0xcb4dafa8,
    0xca8fc59f, 0xc8c97bc6, 0xc90b11f1, 0xcc440774, 0xcd866d43,
    0xcfc0d31a, 0xce02b92d, 0x91af9640, 0x906dfc77, 0x922b422e,
    0x93e92819, 0x96a63e9c, 0x976454ab, 0x9522eaf2, 0x94e080c5,
    0x9fbcc7f8, 0x9e7eadcf, 0x9c381396, 0x9dfa79a1, 0x98b56f24,
    0x99770513, 0x9b31bb4a, 0x9af3d17d, 0x8d893530, 0x8c4b5f07,
    0x8e0de15e, 0x8fcf8b69, 0x8a809dec, 0x8b42f7db, 0x89044982,
    0x88c623b5, 0x839a6488, 0x82580ebf, 0x801eb0e6, 0x81dcdad1,
    0x8493cc54, 0x8551a663, 0x8717183a, 0x86d5720d, 0xa9e2d0a0,
    0xa820ba97, 0xaa6604ce, 0xaba46ef9, 0xaeeb787c, 0xaf29124b,
    0xad6fac12, 0xacadc625, 0xa7f18118, 0xa633eb2f, 0xa4755576,
    0xa5b73f41, 0xa0f829c4, 0xa13a43f3, 0xa37cfdaa, 0xa2be979d,
    0xb5c473d0, 0xb40619e7, 0xb640a7be, 0xb782cd89, 0xb2cddb0c,
    0xb30fb13b, 0xb1490f62, 0xb08b6555, 0xbbd72268, 0xba15485f,
    0xb853f606, 0xb9919c31, 0xbcde8ab4, 0xbd1ce083, 0xbf5a5eda,
    0xbe9834ed},
   {0x00000000, 0x191b3141, 0x32366282, 0x2b2d53c3, 0x646cc504,
    0x7d77f445, 0x565aa786, 0x4f4196c7, 0xc8d98a08, 0xd1c2bb49,
    0xfaefe88a, 0xe3f4d9cb, 0xacb54f0c, 0xb5ae7e4d, 0x9e832d8e,
    0x87981ccf, 0x4ac21251, 0x53d92310, 0x78f470d3, 0x61ef4192,
    0x2eaed755, 0x37b5e614, 0x1c98b5d7, 0x05838496, 0x821b9859,
    0x9b00a918, 0xb02dfadb, 0xa936cb9a, 0xe6775d5d, 0xff6c6c1c,
    0xd4413fdf, 0xcd5a0e9e, 0x958424a2, 0x8c9f15e3, 0xa7b24620,
    0xbea97761, 0xf1e8e1a6, 0xe8f3d0e7, 0xc3de8324, 0xdac5b265,
    0x5d5daeaa, 0x44469feb, 0x6f6bcc28, 0x7670fd69, 0x39316bae,
    0x202a5aef, 0x0b07092c, 0x121c386d, 0xdf4636f3, 0xc65d07b2,
    0xed705471, 0xf46b6530, 0xbb2af3f7, 0xa231c2b6, 0x891c9175,
    0x9007a034, 0x179fbcfb, 0x0e848dba, 0x25a9de79, 0x3cb2ef38,
    0x73f379ff, 0x6ae848be, 0x41c51b7d, 0x58de2a3c, 0xf0794f05,
    0xe9627e44, 0xc24f2d87, 0xdb541cc6, 0x94158a01, 0x8d0ebb40,
    0xa623e883, 0xbf38d9c2, 0x38a0c50d, 0x21bbf44c, 0x0a96a78f,
    0x138d96ce, 0x5ccc0009, 0x45d73148, 0x6efa628b, 0x77e153ca,
    0xbabb5d54, 0xa3a06c15, 0x888d3fd6, 0x91960e97, 0xded79850,
    0xc7cca911, 0xece1fad2, 0xf5facb93, 0x7262d75c, 0x6b79e61d,
    0x4054b5de, 0x594f849f, 0x160e1258, 0x0f152319, 0x243870da,
    0x3d23419b, 0x65fd6ba7, 0x7ce65ae6, 0x57cb0925, 0x4ed03864,
    0x0191aea3, 0x188a9fe2, 0x33a7cc21, 0x2abcfd60, 0xad24e1af,
    0xb43fd0ee, 0x9f12832d, 0x8609b26c, 0xc94824ab, 0xd05315ea,
    0xfb7e4629, 0xe2657768, 0x2f3f79f6, 0x362448b7, 0x1d091b74,
    0x04122a35, 0x4b53bcf2, 0x52488db3, 0x7965de70, 0x607eef31,
    0xe7e6f3fe, 0xfefdc2bf, 0xd5d0917c, 0xcccba03d, 0x838a36fa,
    0x9a9107bb, 0xb1bc5478, 0xa8a76539, 0x3b83984b, 0x2298a90a,
    0x09b5fac9, 0x10aecb88, 0x5fef5d4f, 0x46f46c0e, 0x6dd93fcd,
    0x74c20e8c, 0xf35a1243, 0xea412302, 0xc16c70c1, 0xd8774180,
    0x9736d747, 0x8e2de606, 0xa500b5c5, 0xbc1b8484, 0x71418a1a,
    0x685abb5b, 0x4377e898, 0x5a6cd9d9, 0x152d4f1e, 0x0c367e5f,
    0x271b2d9c, 0x3e001cdd, 0xb9980012, 0xa0833153, 0x8bae6290,
    0x92b553d1, 0xddf4c516, 0xc4eff457, 0xefc2a794, 0xf6d996d5,
    0xae07bce9, 0xb71c8da8, 0x9c31de6b, 0x852aef2a, 0xca6b79ed,
    0xd37048ac, 0xf85d1b6f, 0xe1462a2e, 0x66de36e1, 0x7fc507a0,
    0x54e85463, 0x4df36522, 0x02b2f3e5, 0x1ba9c2a4, 0x30849167,
    0x299fa026, 0xe4c5aeb8, 0xfdde9ff9, 0xd6f3cc3a, 0xcfe8fd7b,
    0x80a96bbc, 0x99b25afd, 0xb29f093e, 0xab84387f, 0x2c1c24b0,
    0x350715f1, 0x1e2a4632, 0x07317773, 0x4870e1b4, 0x516bd0f5,
    0x7a468336, 0x635db277, 0xcbfad74e, 0xd2e1e60f, 0xf9ccb5cc,
    0xe0d7848d, 0xaf96124a, 0xb68d230b, 0x9da070c8, 0x84bb4189,
    0x03235d46, 0x1a386c07, 0x31153fc4, 0x280e0e85, 0x674f9842,
    0x7e54a903, 0x5579fac0, 0x4c62cb81, 0x8138c51f, 0x9823f45e,
    0xb30ea79d, 0xaa1596dc, 0xe554001b, 0xfc4f315a, 0xd7626299,
    0xce7953d8, 0x49e14f17, 0x50fa7e56, 0x7bd72d95, 0x62cc1cd4,
    0x2d8d8a13, 0x3496bb52, 0x1fbbe891, 0x06a0d9d0, 0x5e7ef3ec,
    0x4765c2ad, 0x6c48916e, 0x7553a02f, 0x3a1236e8, 0x230907a9,
    0x0824546a, 0x113f652b, 0x96a779e4, 0x8fbc48a5, 0xa4911b66,
    0xbd8a2a27, 0xf2cbbce0, 0xebd08da1, 0xc0fdde62, 0xd9e6ef23,
    0x14bce1bd, 0x0da7d0fc, 0x268a833f, 0x3f91b27e, 0x70d024b9,
    0x69cb15f8, 0x42e6463b, 0x5bfd777a, 0xdc656bb5, 0xc57e5af4,
    0xee530937, 0xf7483876, 0xb809aeb1, 0xa1129ff0, 0x8a3fcc33,
    0x9324fd72},
   {0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
    0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
    0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
    0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
    0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
    0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
    0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
    0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
    0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
    0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
    0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
    0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
    0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
    0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
    0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
    0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
    0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
    0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
    0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
    0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
    0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
    0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
    0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
    0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
    0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
    0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
    0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
    0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
    0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
    0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
    0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
    0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
    0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
    0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
    0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
    0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
    0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
    0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
    0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
    0x2d02ef8d}};

local const z_word_t FAR crc_braid_big_table[][256] = {
   {0x00000000, 0x96300777, 0x2c610eee, 0xba510999, 0x19c46d07,
    0x8ff46a70, 0x35a563e9, 0xa395649e, 0x3288db0e, 0xa4b8dc79,
    0x1ee9d5e0, 0x88d9d297, 0x2b4cb609, 0xbd7cb17e, 0x072db8e7,
    0x911dbf90, 0x6410b71d, 0xf220b06a, 0x4871b9f3, 0xde41be84,
    0x7dd4da1a, 0xebe4dd6d, 0x51b5d4f4, 0xc785d383, 0x56986c13,
    0xc0a86b64, 0x7af962fd, 0xecc9658a, 0x4f5c0114, 0xd96c0663,
    0x633d0ffa, 0xf50d088d, 0xc8206e3b, 0x5e10694c, 0xe44160d5,
    0x727167a2, 0xd1e4033c, 0x47d4044b, 0xfd850dd2, 0x6bb50aa5,
    0xfaa8b535, 0x6c98b242, 0xd6c9bbdb, 0x40f9bcac, 0xe36cd832,
    0x755cdf45, 0xcf0dd6dc, 0x593dd1ab, 0xac30d926, 0x3a00de51,
    0x8051d7c8, 0x1661d0bf, 0xb5f4b421, 0x23c4b356, 0x9995bacf,
    0x0fa5bdb8, 0x9eb80228, 0x0888055f, 0xb2d90cc6, 0x24e90bb1,
    0x877c6f2f, 0x114c6858, 0xab1d61c1, 0x3d2d66b6, 0x9041dc76,
    0x0671db01, 0xbc20d298, 0x2a10d5ef, 0x8985b171, 0x1fb5b606,
    0xa5e4bf9f, 0x33d4b8e8, 0xa2c90778, 0x34f9000f, 0x8ea80996,
    0x18980ee1, 0xbb0d6a7f, 0x2d3d6d08, 0x976c6491, 0x015c63e6,
    0xf4516b6b, 0x62616c1c, 0xd8306585, 0x4e0062f2, 0xed95066c,
    0x7ba5011b, 0xc1f40882, 0x57c40ff5, 0xc6d9b065, 0x50e9b712,
    0xeab8be8b, 0x7c88b9fc, 0xdf1ddd62, 0x492dda15, 0xf37cd38c,
    0x654cd4fb, 0x5861b24d, 0xce51b53a, 0x7400bca3, 0xe230bbd4,
    0x41a5df4a, 0xd795d83d, 0x6dc4d1a4, 0xfbf4d6d3, 0x6ae96943,
    0xfcd96e34, 0x468867ad, 0xd0b860da, 0x732d0444, 0xe51d0333,
    0x5f4c0aaa, 0xc97c0ddd, 0x3c710550, 0xaa410227, 0x10100bbe,
    0x86200cc9, 0x25b56857, 0xb3856f20, 0x09d466b9, 0x9fe461ce,
    0x0ef9de5e, 0x98c9d929, 0x2298d0b0, 0xb4a8d7c7, 0x173db359,
    0x810db42e, 0x3b5cbdb7, 0xad6cbac0, 0x2083b8ed, 0xb6b3bf9a,
    0x0ce2b603, 0x9ad2b174, 0x3947d5ea, 0xaf77d29d, 0x1526db04,
    0x8316dc73, 0x120b63e3, 0x843b6494, 0x3e6a6d0d, 0xa85a6a7a,
    0x0bcf0ee4, 0x9dff0993, 0x27ae000a, 0xb19e077d, 0x44930ff0,
    0xd2a30887, 0x68f2011e, 0xfec20669, 0x5d5762f7, 0xcb676580,
    0x71366c19, 0xe7066b6e, 0x761bd4fe, 0xe02bd389, 0x5a7ada10,
    0xcc4add67, 0x6fdfb9f9, 0xf9efbe8e, 0x43beb717, 0xd58eb060,
    0xe8a3d6d6, 0x7e93d1a1, 0xc4c2d838, 0x52f2df4f, 0xf167bbd1,
    0x6757bca6, 0xdd06b53f, 0x4b36b248, 0xda2b0dd8, 0x4c1b0aaf,
    0xf64a0336, 0x607a0441, 0xc3ef60df, 0x55df67a8, 0xef8e6e31,
    0x79be6946, 0x8cb361cb, 0x1a8366bc, 0xa0d26f25, 0x36e26852,
    0x95770ccc, 0x03470bbb, 0xb9160222, 0x2f260555, 0xbe3bbac5,
    0x280bbdb2, 0x925ab42b, 0x046ab35c, 0xa7ffd7c2, 0x31cfd0b5,
    0x8b9ed92c, 0x1daede5b, 0xb0c2649b, 0x26f263ec, 0x9ca36a75,
    0x0a936d02, 0xa906099c, 0x3f360eeb, 0x85670772, 0x13570005,
    0x824abf95, 0x147ab8e2, 0xae2bb17b, 0x381bb60c, 0x9b8ed292,
    0x0dbed5e5, 0xb7efdc7c, 0x21dfdb0b, 0xd4d2d386, 0x42e2d4f1,
    0xf8b3dd68, 0x6e83da1f, 0xcd16be81, 0x5b26b9f6, 0xe177b06f,
    0x7747b718, 0xe65a0888, 0x706a0fff, 0xca3b0666, 0x5c0b0111,
    0xff9e658f, 0x69ae62f8, 0xd3ff6b61, 0x45cf6c16, 0x78e20aa0,
    0xeed20dd7, 0x5483044e, 0xc2b30339, 0x612667a7, 0xf71660d0,
    0x4d476949, 0xdb776e3e, 0x4a6ad1ae, 0xdc5ad6d9, 0x660bdf40,
    0xf03bd837, 0x53aebca9, 0xc59ebbde, 0x7fcfb247, 0xe9ffb530,
    0x1cf2bdbd, 0x8ac2baca, 0x3093b353, 0xa6a3b424, 0x0536d0ba,
    0x9306d7cd, 0x2957de54, 0xbf67d923, 0x2e7a66b3, 0xb84a61c4,
    0x021b685d, 0x942b6f2a, 0x37be0bb4, 0xa18e0cc3, 0x1bdf055a,
    0x8def022d},
   {0x00000000, 0x41311b19, 0x82623632, 0xc3532d2b, 0x04c56c64,
    0x45f4777d, 0x86a75a56, 0xc796414f, 0x088ad9c8, 0x49bbc2d1,
    0x8ae8effa, 0xcbd9f4e3, 0x0c4fb5ac, 0x4d7eaeb5, 0x8e2d839e,
    0xcf1c9887, 0x5112c24a, 0x1023d953, 0xd370f478, 0x9241ef61,
    0x55d7ae2e, 0x14e6b537, 0xd7b5981c, 0x96848305, 0x59981b82,
    0x18a9009b, 0xdbfa2db0, 0x9acb36a9, 0x5d5d77e6, 0x1c6c6cff,
    0xdf3f41d4, 0x9e0e5acd, 0xa2248495, 0xe3159f8c, 0x2046b2a7,
    0x6177a9be, 0xa6e1e8f1, 0xe7d0f3e8, 0x2483dec3, 0x65b2c5da,
    0xaaae5d5d, 0xeb9f4644, 0x28cc6b6f, 0x69fd7076, 0xae6b3139,
    0xef5a2a20, 0x2c09070b, 0x6d381c12, 0xf33646df, 0xb2075dc6,
    0x715470ed, 0x30656bf4, 0xf7f32abb, 0xb6c231a2, 0x75911c89,
    0x34a00790, 0xfbbc9f17, 0xba8d840e, 0x79dea925, 0x38efb23c,
    0xff79f373, 0xbe48e86a, 0x7d1bc541, 0x3c2ade58, 0x054f79f0,
    0x447e62e9, 0x872d4fc2, 0xc61c54db, 0x018a1594, 0x40bb0e8d,
    0x83e823a6, 0xc2d938bf, 0x0dc5a038, 0x4cf4bb21, 0x8fa7960a,
    0xce968d13, 0x0900cc5c, 0x4831d745, 0x8b62fa6e, 0xca53e177,
    0x545dbbba, 0x156ca0a3, 0xd63f8d88, 0x970e9691, 0x5098d7de,
    0x11a9ccc7, 0xd2fae1ec, 0x93cbfaf5, 0x5cd76272, 0x1de6796b,
    0xdeb55440, 0x9f844f59, 0x58120e16, 0x1923150f, 0xda703824,
    0x9b41233d, 0xa76bfd65, 0xe65ae67c, 0x2509cb57, 0x6438d04e,
    0xa3ae9101, 0xe29f8a18, 0x21cca733, 0x60fdbc2a, 0xafe124ad,
    0xeed03fb4, 0x2d83129f, 0x6cb20986, 0xab2448c9, 0xea1553d0,
    0x29467efb, 0x687765e2, 0xf6793f2f, 0xb7482436, 0x741b091d,
    0x352a1204, 0xf2bc534b, 0xb38d4852, 0x70de6579, 0x31ef7e60,
    0xfef3e6e7, 0xbfc2fdfe, 0x7c91d0d5, 0x3da0cbcc, 0xfa368a83,
    0xbb07919a, 0x7854bcb1, 0x3965a7a8, 0x4b98833b, 0x0aa99822,
    0xc9fab509, 0x88cbae10, 0x4f5def5f, 0x0e6cf446, 0xcd3fd96d,
    0x8c0ec274, 0x43125af3, 0x022341ea, 0xc1706cc1, 0x804177d8,
    0x47d73697, 0x06e62d8e, 0xc5b500a5, 0x84841bbc, 0x1a8a4171,
    0x5bbb5a68, 0x98e87743, 0xd9d96c5a, 0x1e4f2d15, 0x5f7e360c,
    0x9c2d1b27, 0xdd1c003e, 0x120098b9, 0x533183a0, 0x9062ae8b,
    0xd153b592, 0x16c5f4dd, 0x57f4efc4, 0x94a7c2ef, 0xd596d9f6,
    0xe9bc07ae, 0xa88d1cb7, 0x6bde319c, 0x2aef2a85, 0xed796bca,
    0xac4870d3, 0x6f1b5df8, 0x2e2a46e1, 0xe136de66, 0xa007c57f,
    0x6354e854, 0x2265f34d, 0xe5f3b202, 0xa4c2a91b, 0x67918430,
    0x26a09f29, 0xb8aec5e4, 0xf99fdefd, 0x3accf3d6, 0x7bfde8cf,
    0xbc6ba980, 0xfd5ab299, 0x3e099fb2, 0x7f3884ab, 0xb0241c2c,
    0xf1150735, 0x32462a1e, 0x73773107, 0xb4e17048, 0xf5d06b51,
    0x3683467a, 0x77b25d63, 0x4ed7facb, 0x0fe6e1d2, 0xccb5ccf9,
    0x8d84d7e0, 0x4a1296af, 0x0b238db6, 0xc870a09d, 0x8941bb84,
    0x465d2303, 0x076c381a, 0xc43f1531, 0x850e0e28, 0x42984f67,
    0x03a9547e, 0xc0fa7955, 0x81cb624c, 0x1fc53881, 0x5ef42398,
    0x9da70eb3, 0xdc9615aa, 0x1b0054e5, 0x5a314ffc, 0x996262d7,
    0xd85379ce, 0x174fe149, 0x567efa50, 0x952dd77b, 0xd41ccc62,
    0x138a8d2d, 0x52bb9634, 0x91e8bb1f, 0xd0d9a006, 0xecf37e5e,
    0xadc26547, 0x6e91486c, 0x2fa05375, 0xe836123a, 0xa9070923,
    0x6a542408, 0x2b653f11, 0xe479a796, 0xa548bc8f, 0x661b91a4,
    0x272a8abd, 0xe0bccbf2, 0xa18dd0eb, 0x62defdc0, 0x23efe6d9,
    0xbde1bc14, 0xfcd0a70d, 0x3f838a26, 0x7eb2913f, 0xb924d070,
    0xf815cb69, 0x3b46e642, 0x7a77fd5b, 0xb56b65dc, 0xf45a7ec5,
    0x370953ee, 0x763848f7, 0xb1ae09b8, 0xf09f12a1, 0x33cc3f8a,
    0x72fd2493},
   {0x00000000, 0x376ac201, 0x6ed48403, 0x59be4602, 0xdca80907,
    0xebc2cb06, 0xb27c8d04, 0x85164f05, 0xb851130e, 0x8f3bd10f,
    0xd685970d, 0xe1ef550c, 0x64f91a09, 0x5393d808, 0x0a2d9e0a,
    0x3d475c0b, 0x70a3261c, 0x47c9e41d, 0x1e77a21f, 0x291d601e,
    0xac0b2f1b, 0x9b61ed1a, 0xc2dfab18, 0xf5b56919, 0xc8f23512,
    0xff98f713, 0xa626b111, 0x914c7310, 0x145a3c15, 0x2330fe14,
    0x7a8eb816, 0x4de47a17, 0xe0464d38, 0xd72c8f39, 0x8e92c93b,
    0xb9f80b3a, 0x3cee443f, 0x0b84863e, 0x523ac03c, 0x6550023d,
    0x58175e36, 0x6f7d9c37, 0x36c3da35, 0x01a91834, 0x84bf5731,
    0xb3d59530, 0xea6bd332, 0xdd011133, 0x90e56b24, 0xa78fa925,
    0xfe31ef27, 0xc95b2d26, 0x4c4d6223, 0x7b27a022, 0x2299e620,
    0x15f32421, 0x28b4782a, 0x1fdeba2b, 0x4660fc29, 0x710a3e28,
    0xf41c712d, 0xc376b32c, 0x9ac8f52e, 0xada2372f, 0xc08d9a70,
    0xf7e75871, 0xae591e73, 0x9933dc72, 0x1c259377, 0x2b4f5176,
    0x72f11774, 0x459bd575, 0x78dc897e, 0x4fb64b7f, 0x16080d7d,
    0x2162cf7c, 0xa4748079, 0x931e4278, 0xcaa0047a, 0xfdcac67b,
    0xb02ebc6c, 0x87447e6d, 0xdefa386f, 0xe990fa6e, 0x6c86b56b,
    0x5bec776a, 0x02523168, 0x3538f369, 0x087faf62, 0x3f156d63,
    0x66ab2b61, 0x51c1e960, 0xd4d7a665, 0xe3bd6464, 0xba032266,
    0x8d69e067, 0x20cbd748, 0x17a11549, 0x4e1f534b, 0x7975914a,
    0xfc63de4f, 0xcb091c4e, 0x92b75a4c, 0xa5dd984d, 0x989ac446,
    0xaff00647, 0xf64e4045, 0xc1248244, 0x4432cd41, 0x73580f40,
    0x2ae64942, 0x1d8c8b43, 0x5068f154, 0x67023355, 0x3ebc7557,
    0x09d6b756, 0x8cc0f853, 0xbbaa3a52, 0xe2147c50, 0xd57ebe51,
    0xe839e25a, 0xdf53205b, 0x86ed6659, 0xb187a458, 0x3491eb5d,
    0x03fb295c, 0x5a456f5e, 0x6d2fad5f, 0x801b35e1, 0xb771f7e0,
    0xeecfb1e2, 0xd9a573e3, 0x5cb33ce6, 0x6bd9fee7, 0x3267b8e5,
    0x050d7ae4, 0x384a26ef, 0x0f20e4ee, 0x569ea2ec, 0x61f460ed,
    0xe4e22fe8, 0xd388ede9, 0x8a36abeb, 0xbd5c69ea, 0xf0b813fd,
    0xc7d2d1fc, 0x9e6c97fe, 0xa90655ff, 0x2c101afa, 0x1b7ad8fb,
    0x42c49ef9, 0x75ae5cf8, 0x48e900f3, 0x7f83c2f2, 0x263d84f0,
    0x115746f1, 0x944109f4, 0xa32bcbf5, 0xfa958df7, 0xcdff4ff6,
    0x605d78d9, 0x5737bad8, 0x0e89fcda, 0x39e33edb, 0xbcf571de,
    0x8b9fb3df, 0xd221f5dd, 0xe54b37dc, 0xd80c6bd7, 0xef66a9d6,
    0xb6d8efd4, 0x81b22dd5, 0x04a462d0, 0x33cea0d1, 0x6a70e6d3,
    0x5d1a24d2, 0x10fe5ec5, 0x27949cc4, 0x7e2adac6, 0x494018c7,
    0xcc5657c2, 0xfb3c95c3, 0xa282d3c1, 0x95e811c0, 0xa8af4dcb,
    0x9fc58fca, 0xc67bc9c8, 0xf1110bc9, 0x740744cc, 0x436d86cd,
    0x1ad3c0cf, 0x2db902ce, 0x4096af91, 0x77fc6d90, 0x2e422b92,
    0x1928e993, 0x9c3ea696, 0xab546497, 0xf2ea2295, 0xc580e094,
    0xf8c7bc9f, 0xcfad7e9e, 0x9613389c, 0xa179fa9d, 0x246fb598,
    0x13057799, 0x4abb319b, 0x7dd1f39a, 0x3035898d, 0x075f4b8c,
    0x5ee10d8e, 0x698bcf8f, 0xec9d808a, 0xdbf7428b, 0x82490489,
    0xb523c688, 0x88649a83, 0xbf0e5882, 0xe6b01e80, 0xd1dadc81,
    0x54cc9384, 0x63a65185, 0x3a181787, 0x0d72d586, 0xa0d0e2a9,
    0x97ba20a8, 0xce0466aa, 0xf96ea4ab, 0x7c78ebae, 0x4b1229af,
    0x12ac6fad, 0x25c6adac, 0x1881f1a7, 0x2feb33a6, 0x765575a4,
    0x413fb7a5, 0xc429f8a0, 0xf3433aa1, 0xaafd7ca3, 0x9d97bea2,
    0xd073c4b5, 0xe71906b4, 0xbea740b6, 0x89cd82b7, 0x0cdbcdb2,
    0x3bb10fb3, 0x620f49b1, 0x55658bb0, 0x6822d7bb, 0x5f4815ba,
    0x06f653b8, 0x319c91b9, 0xb48adebc, 0x83e01cbd, 0xda5e5abf,
    0xed3498be},
   {0x00000000, 0x6567bcb8, 0x8bc809aa, 0xeeafb512, 0x5797628f,
    0x32f0de37, 0xdc5f6b25, 0xb938d79d, 0xef28b4c5, 0x8a4f087d,
    0x64e0bd6f, 0x018701d7, 0xb8bfd64a, 0xddd86af2, 0x3377dfe0,
    0x56106358, 0x9f571950, 0xfa30a5e8, 0x149f10fa, 0x71f8ac42,
    0xc8c07bdf, 0xada7c767, 0x43087275, 0x266fcecd, 0x707fad95,
    0x1518112d, 0xfbb7a43f, 0x9ed01887, 0x27e8cf1a, 0x428f73a2,
    0xac20c6b0, 0xc9477a08, 0x3eaf32a0, 0x5bc88e18, 0xb5673b0a,
    0xd00087b2, 0x6938502f, 0x0c5fec97, 0xe2f05985, 0x8797e53d,
    0xd1878665, 0xb4e03add, 0x5a4f8fcf, 0x3f283377, 0x8610e4ea,
    0xe3775852, 0x0dd8ed40, 0x68bf51f8, 0xa1f82bf0, 0xc49f9748,
    0x2a30225a, 0x4f579ee2, 0xf66f497f, 0x9308f5c7, 0x7da740d5,
    0x18c0fc6d, 0x4ed09f35, 0x2bb7238d, 0xc518969f, 0xa07f2a27,
    0x1947fdba, 0x7c204102, 0x928ff410, 0xf7e848a8, 0x3d58149b,
    0x583fa823, 0xb6901d31, 0xd3f7a189, 0x6acf7614, 0x0fa8caac,
    0xe1077fbe, 0x8460c306, 0xd270a05e, 0xb7171ce6, 0x59b8a9f4,
    0x3cdf154c, 0x85e7c2d1, 0xe0807e69, 0x0e2fcb7b, 0x6b4877c3,
    0xa20f0dcb, 0xc768b173, 0x29c70461, 0x4ca0b8d9, 0xf5986f44,
    0x90ffd3fc, 0x7e5066ee, 0x1b37da56, 0x4d27b90e, 0x284005b6,
    0xc6efb0a4, 0xa3880c1c, 0x1ab0db81, 0x7fd76739, 0x9178d22b,
    0xf41f6e93, 0x03f7263b, 0x66909a83, 0x883f2f91, 0xed589329,
    0x546044b4, 0x3107f80c, 0xdfa84d1e, 0xbacff1a6, 0xecdf92fe,
    0x89b82e46, 0x67179b54, 0x027027ec, 0xbb48f071, 0xde2f4cc9,
    0x3080f9db, 0x55e74563, 0x9ca03f6b, 0xf9c783d3, 0x176836c1,
    0x720f8a79, 0xcb375de4, 0xae50e15c, 0x40ff544e, 0x2598e8f6,
    0x73888bae, 0x16ef3716, 0xf8408204, 0x9d273ebc, 0x241fe921,
    0x41785599, 0xafd7e08b, 0xcab05c33, 0x3bb659ed, 0x5ed1e555,
    0xb07e5047, 0xd519ecff, 0x6c213b62, 0x094687da, 0xe7e932c8,
    0x828e8e70, 0xd49eed28, 0xb1f95190, 0x5f56e482, 0x3a31583a,
    0x83098fa7, 0xe66e331f, 0x08c1860d, 0x6da63ab5, 0xa4e140bd,
    0xc186fc05, 0x2f294917, 0x4a4ef5af, 0xf3762232, 0x96119e8a,
    0x78be2b98, 0x1dd99720, 0x4bc9f478, 0x2eae48c0, 0xc001fdd2,
    0xa566416a, 0x1c5e96f7, 0x79392a4f, 0x97969f5d, 0xf2f123e5,
    0x05196b4d, 0x607ed7f5, 0x8ed162e7, 0xebb6de5f, 0x528e09c2,
    0x37e9b57a, 0xd9460068, 0xbc21bcd0, 0xea31df88, 0x8f566330,
    0x61f9d622, 0x049e6a9a, 0xbda6bd07, 0xd8c101bf, 0x366eb4ad,
    0x53090815, 0x9a4e721d, 0xff29cea5, 0x11867bb7, 0x74e1c70f,
    0xcdd91092, 0xa8beac2a, 0x46111938, 0x2376a580, 0x7566c6d8,
    0x10017a60, 0xfeaecf72, 0x9bc973ca, 0x22f1a457, 0x479618ef,
    0xa939adfd, 0xcc5e1145, 0x06ee4d76, 0x6389f1ce, 0x8d2644dc,
    0xe841f864, 0x51792ff9, 0x341e9341, 0xdab12653, 0xbfd69aeb,
    0xe9c6f9b3, 0x8ca1450b, 0x620ef019, 0x07694ca1, 0xbe519b3c,
    0xdb362784, 0x35999296, 0x50fe2e2e, 0x99b95426, 0xfcdee89e,
    0x12715d8c, 0x7716e134, 0xce2e36a9, 0xab498a11, 0x45e63f03,
    0x208183bb, 0x7691e0e3, 0x13f65c5b, 0xfd59e949, 0x983e55f1,
    0x2106826c, 0x44613ed4, 0xaace8bc6, 0xcfa9377e, 0x38417fd6,
    0x5d26c36e, 0xb389767c, 0xd6eecac4, 0x6fd61d59, 0x0ab1a1e1,
    0xe41e14f3, 0x8179a84b, 0xd769cb13, 0xb20e77ab, 0x5ca1c2b9,
    0x39c67e01, 0x80fea99c, 0xe5991524, 0x0b36a036, 0x6e511c8e,
    0xa7166686, 0xc271da3e, 0x2cde6f2c, 0x49b9d394, 0xf0810409,
    0x95e6b8b1, 0x7b490da3, 0x1e2eb11b, 0x483ed243, 0x2d596efb,
    0xc3f6dbe9, 0xa6916751, 0x1fa9b0cc, 0x7ace0c74, 0x9461b966,
    0xf10605de}};

#endif

#endif

#if N == 2

#if W == 8

local const z_crc_t FAR crc_braid_table[][256] = {
   {0x00000000, 0xae689191, 0x87a02563, 0x29c8b4f2, 0xd4314c87,
    0x7a59dd16, 0x539169e4, 0xfdf9f875, 0x73139f4f, 0xdd7b0ede,
    0xf4b3ba2c, 0x5adb2bbd, 0xa722d3c8, 0x094a4259, 0x2082f6ab,
    0x8eea673a, 0xe6273e9e, 0x484faf0f, 0x61871bfd, 0xcfef8a6c,
    0x32167219, 0x9c7ee388, 0xb5b6577a, 0x1bdec6eb, 0x9534a1d1,
    0x3b5c3040, 0x129484b2, 0xbcfc1523, 0x4105ed56, 0xef6d7cc7,
    0xc6a5c835, 0x68cd59a4, 0x173f7b7d, 0xb957eaec, 0x909f5e1e,
    0x3ef7cf8f, 0xc30e37fa, 0x6d66a66b, 0x44ae1299, 0xeac68308,
    0x642ce432, 0xca4475a3, 0xe38cc151, 0x4de450c0, 0xb01da8b5,
    0x1e753924, 0x37bd8dd6, 0x99d51c47, 0xf11845e3, 0x5f70d472,
    0x76b86080, 0xd8d0f111, 0x25290964, 0x8b4198f5, 0xa2892c07,
    0x0ce1bd96, 0x820bdaac, 0x2c634b3d, 0x05abffcf, 0xabc36e5e,
    0x563a962b, 0xf85207ba, 0xd19ab348, 0x7ff222d9, 0x2e7ef6fa,
    0x8016676b, 0xa9ded399, 0x07b64208, 0xfa4fba7d, 0x54272bec,
    0x7def9f1e, 0xd3870e8f, 0x5d6d69b5, 0xf305f824, 0xdacd4cd6,
    0x74a5dd47, 0x895c2532, 0x2734b4a3, 0x0efc0051, 0xa09491c0,
    0xc859c864, 0x663159f5, 0x4ff9ed07, 0xe1917c96, 0x1c6884e3,
    0xb2001572, 0x9bc8a180, 0x35a03011, 0xbb4a572b, 0x1522c6ba,
    0x3cea7248, 0x9282e3d9, 0x6f7b1bac, 0xc1138a3d, 0xe8db3ecf,
    0x46b3af5e, 0x39418d87, 0x97291c16, 0xbee1a8e4, 0x10893975,
    0xed70c100, 0x43185091, 0x6ad0e463, 0xc4b875f2, 0x4a5212c8,
    0xe43a8359, 0xcdf237ab, 0x639aa63a, 0x9e635e4f, 0x300bcfde,
    0x19c37b2c, 0xb7abeabd, 0xdf66b319, 0x710e2288, 0x58c6967a,
    0xf6ae07eb, 0x0b57ff9e, 0xa53f6e0f, 0x8cf7dafd, 0x229f4b6c,
    0xac752c56, 0x021dbdc7, 0x2bd50935, 0x85bd98a4, 0x784460d1,
    0xd62cf140, 0xffe445b2, 0x518cd423, 0x5cfdedf4, 0xf2957c65,
    0xdb5dc897, 0x75355906, 0x88cca173, 0x26a430e2, 0x0f6c8410,
    0xa1041581, 0x2fee72bb, 0x8186e32a, 0xa84e57d8, 0x0626c649,
    0xfbdf3e3c, 0x55b7afad, 0x7c7f1b5f, 0xd2178ace, 0xbadad36a,
    0x14b242fb, 0x3d7af609, 0x93126798, 0x6eeb9fed, 0xc0830e7c,
    0xe94bba8e, 0x47232b1f, 0xc9c94c25, 0x67a1ddb4, 0x4e696946,
    0xe001f8d7, 0x1df800a2, 0xb3909133, 0x9a5825c1, 0x3430b450,
    0x4bc29689, 0xe5aa0718, 0xcc62b3ea, 0x620a227b, 0x9ff3da0e,
    0x319b4b9f, 0x1853ff6d, 0xb63b6efc, 0x38d109c6, 0x96b99857,
    0xbf712ca5, 0x1119bd34, 0xece04541, 0x4288d4d0, 0x6b406022,
    0xc528f1b3, 0xade5a817, 0x038d3986, 0x2a458d74, 0x842d1ce5,
    0x79d4e490, 0xd7bc7501, 0xfe74c1f3, 0x501c5062, 0xdef63758,
    0x709ea6c9, 0x5956123b, 0xf73e83aa, 0x0ac77bdf, 0xa4afea4e,
    0x8d675ebc, 0x230fcf2d, 0x72831b0e, 0xdceb8a9f, 0xf5233e6d,
    0x5b4baffc, 0xa6b25789, 0x08dac618, 0x211272ea, 0x8f7ae37b,
    0x01908441, 0xaff815d0, 0x8630a122, 0x285830b3, 0xd5a1c8c6,
    0x7bc95957, 0x5201eda5, 0xfc697c34, 0x94a42590, 0x3accb401,
    0x130400f3, 0xbd6c9162, 0x40956917, 0xeefdf886, 0xc7354c74,
    0x695ddde5, 0xe7b7badf, 0x49df2b4e, 0x60179fbc, 0xce7f0e2d,
    0x3386f658, 0x9dee67c9, 0xb426d33b, 0x1a4e42aa, 0x65bc6073,
    0xcbd4f1e2, 0xe21c4510, 0x4c74d481, 0xb18d2cf4, 0x1fe5bd65,
    0x362d0997, 0x98459806, 0x16afff3c, 0xb8c76ead, 0x910fda5f,
    0x3f674bce, 0xc29eb3bb, 0x6cf6222a, 0x453e96d8, 0xeb560749,
    0x839b5eed, 0x2df3cf7c, 0x043b7b8e, 0xaa53ea1f, 0x57aa126a,
    0xf9c283fb, 0xd00a3709, 0x7e62a698, 0xf088c1a2, 0x5ee05033,
    0x7728e4c1, 0xd9407550, 0x24b98d25, 0x8ad11cb4, 0xa319a846,
    0x0d7139d7},
   {0x00000000, 0xb9fbdbe8, 0xa886b191, 0x117d6a79, 0x8a7c6563,
    0x3387be8b, 0x22fad4f2, 0x9b010f1a, 0xcf89cc87, 0x7672176f,
    0x670f7d16, 0xdef4a6fe, 0x45f5a9e4, 0xfc0e720c, 0xed731875,
    0x5488c39d, 0x44629f4f, 0xfd9944a7, 0xece42ede, 0x551ff536,
    0xce1efa2c, 0x77e521c4, 0x66984bbd, 0xdf639055, 0x8beb53c8,
    0x32108820, 0x236de259, 0x9a9639b1, 0x019736ab, 0xb86ced43,
    0xa911873a, 0x10ea5cd2, 0x88c53e9e, 0x313ee576, 0x20438f0f,
    0x99b854e7, 0x02b95bfd, 0xbb428015, 0xaa3fea6c, 0x13c43184,
    0x474cf219, 0xfeb729f1, 0xefca4388, 0x56319860, 0xcd30977a,
    0x74cb4c92, 0x65b626eb, 0xdc4dfd03, 0xcca7a1d1, 0x755c7a39,
    0x64211040, 0xdddacba8, 0x46dbc4b2, 0xff201f5a, 0xee5d7523,
    0x57a6aecb, 0x032e6d56, 0xbad5b6be, 0xaba8dcc7, 0x1253072f,
    0x89520835, 0x30a9d3dd, 0x21d4b9a4, 0x982f624c, 0xcafb7b7d,
    0x7300a095, 0x627dcaec, 0xdb861104, 0x40871e1e, 0xf97cc5f6,
    0xe801af8f, 0x51fa7467, 0x0572b7fa, 0xbc896c12, 0xadf4066b,
    0x140fdd83, 0x8f0ed299, 0x36f50971, 0x27886308, 0x9e73b8e0,
    0x8e99e432, 0x37623fda, 0x261f55a3, 0x9fe48e4b, 0x04e58151,
    0xbd1e5ab9, 0xac6330c0, 0x1598eb28, 0x411028b5, 0xf8ebf35d,
    0xe9969924, 0x506d42cc, 0xcb6c4dd6, 0x7297963e, 0x63eafc47,
    0xda1127af, 0x423e45e3, 0xfbc59e0b, 0xeab8f472, 0x53432f9a,
    0xc8422080, 0x71b9fb68, 0x60c49111, 0xd93f4af9, 0x8db78964,
    0x344c528c, 0x253138f5, 0x9ccae31d, 0x07cbec07, 0xbe3037ef,
    0xaf4d5d96, 0x16b6867e, 0x065cdaac, 0xbfa70144, 0xaeda6b3d,
    0x1721b0d5, 0x8c20bfcf, 0x35db6427, 0x24a60e5e, 0x9d5dd5b6,
    0xc9d5162b, 0x702ecdc3, 0x6153a7ba, 0xd8a87c52, 0x43a97348,
    0xfa52a8a0, 0xeb2fc2d9, 0x52d41931, 0x4e87f0bb, 0xf77c2b53,
    0xe601412a, 0x5ffa9ac2, 0xc4fb95d8, 0x7d004e30, 0x6c7d2449,
    0xd586ffa1, 0x810e3c3c, 0x38f5e7d4, 0x29888dad, 0x90735645,
    0x0b72595f, 0xb28982b7, 0xa3f4e8ce, 0x1a0f3326, 0x0ae56ff4,
    0xb31eb41c, 0xa263de65, 0x1b98058d, 0x80990a97, 0x3962d17f,
    0x281fbb06, 0x91e460ee, 0xc56ca373, 0x7c97789b, 0x6dea12e2,
    0xd411c90a, 0x4f10c610, 0xf6eb1df8, 0xe7967781, 0x5e6dac69,
    0xc642ce25, 0x7fb915cd, 0x6ec47fb4, 0xd73fa45c, 0x4c3eab46,
    0xf5c570ae, 0xe4b81ad7, 0x5d43c13f, 0x09cb02a2, 0xb030d94a,
    0xa14db333, 0x18b668db, 0x83b767c1, 0x3a4cbc29, 0x2b31d650,
    0x92ca0db8, 0x8220516a, 0x3bdb8a82, 0x2aa6e0fb, 0x935d3b13,
    0x085c3409, 0xb1a7efe1, 0xa0da8598, 0x19215e70, 0x4da99ded,
    0xf4524605, 0xe52f2c7c, 0x5cd4f794, 0xc7d5f88e, 0x7e2e2366,
    0x6f53491f, 0xd6a892f7, 0x847c8bc6, 0x3d87502e, 0x2cfa3a57,
    0x9501e1bf, 0x0e00eea5, 0xb7fb354d, 0xa6865f34, 0x1f7d84dc,
    0x4bf54741, 0xf20e9ca9, 0xe373f6d0, 0x5a882d38, 0xc1892222,
    0x7872f9ca, 0x690f93b3, 0xd0f4485b, 0xc01e1489, 0x79e5cf61,
    0x6898a518, 0xd1637ef0, 0x4a6271ea, 0xf399aa02, 0xe2e4c07b,
    0x5b1f1b93, 0x0f97d80e, 0xb66c03e6, 0xa711699f, 0x1eeab277,
    0x85ebbd6d, 0x3c106685, 0x2d6d0cfc, 0x9496d714, 0x0cb9b558,
    0xb5426eb0, 0xa43f04c9, 0x1dc4df21, 0x86c5d03b, 0x3f3e0bd3,
    0x2e4361aa, 0x97b8ba42, 0xc33079df, 0x7acba237, 0x6bb6c84e,
    0xd24d13a6, 0x494c1cbc, 0xf0b7c754, 0xe1caad2d, 0x583176c5,
    0x48db2a17, 0xf120f1ff, 0xe05d9b86, 0x59a6406e, 0xc2a74f74,
    0x7b5c949c, 0x6a21fee5, 0xd3da250d, 0x8752e690, 0x3ea93d78,
    0x2fd45701, 0x962f8ce9, 0x0d2e83f3, 0xb4d5581b, 0xa5a83262,
    0x1c53e98a},
   {0x00000000, 0x9d0fe176, 0xe16ec4ad, 0x7c6125db, 0x19ac8f1b,
    0x84a36e6d, 0xf8c24bb6, 0x65cdaac0, 0x33591e36, 0xae56ff40,
    0xd237da9b, 0x4f383bed, 0x2af5912d, 0xb7fa705b, 0xcb9b5580,
    0x5694b4f6, 0x66b23c6c, 0xfbbddd1a, 0x87dcf8c1, 0x1ad319b7,
    0x7f1eb377, 0xe2115201, 0x9e7077da, 0x037f96ac, 0x55eb225a,
    0xc8e4c32c, 0xb485e6f7, 0x298a0781, 0x4c47ad41, 0xd1484c37,
    0xad2969ec, 0x3026889a, 0xcd6478d8, 0x506b99ae, 0x2c0abc75,
    0xb1055d03, 0xd4c8f7c3, 0x49c716b5, 0x35a6336e, 0xa8a9d218,
    0xfe3d66ee, 0x63328798, 0x1f53a243, 0x825c4335, 0xe791e9f5,
    0x7a9e0883, 0x06ff2d58, 0x9bf0cc2e, 0xabd644b4, 0x36d9a5c2,
    0x4ab88019, 0xd7b7616f, 0xb27acbaf, 0x2f752ad9, 0x53140f02,
    0xce1bee74, 0x988f5a82, 0x0580bbf4, 0x79e19e2f, 0xe4ee7f59,
    0x8123d599, 0x1c2c34ef, 0x604d1134, 0xfd42f042, 0x41b9f7f1,
    0xdcb61687, 0xa0d7335c, 0x3dd8d22a, 0x581578ea, 0xc51a999c,
    0xb97bbc47, 0x24745d31, 0x72e0e9c7, 0xefef08b1, 0x938e2d6a,
    0x0e81cc1c, 0x6b4c66dc, 0xf64387aa, 0x8a22a271, 0x172d4307,
    0x270bcb9d, 0xba042aeb, 0xc6650f30, 0x5b6aee46, 0x3ea74486,
    0xa3a8a5f0, 0xdfc9802b, 0x42c6615d, 0x1452d5ab, 0x895d34dd,
    0xf53c1106, 0x6833f070, 0x0dfe5ab0, 0x90f1bbc6, 0xec909e1d,
    0x719f7f6b, 0x8cdd8f29, 0x11d26e5f, 0x6db34b84, 0xf0bcaaf2,
    0x95710032, 0x087ee144, 0x741fc49f, 0xe91025e9, 0xbf84911f,
    0x228b7069, 0x5eea55b2, 0xc3e5b4c4, 0xa6281e04, 0x3b27ff72,
    0x4746daa9, 0xda493bdf, 0xea6fb345, 0x77605233, 0x0b0177e8,
    0x960e969e, 0xf3c33c5e, 0x6eccdd28, 0x12adf8f3, 0x8fa21985,
    0xd936ad73, 0x44394c05, 0x385869de, 0xa55788a8, 0xc09a2268,
    0x5d95c31e, 0x21f4e6c5, 0xbcfb07b3, 0x8373efe2, 0x1e7c0e94,
    0x621d2b4f, 0xff12ca39, 0x9adf60f9, 0x07d0818f, 0x7bb1a454,
    0xe6be4522, 0xb02af1d4, 0x2d2510a2, 0x51443579, 0xcc4bd40f,
    0xa9867ecf, 0x34899fb9, 0x48e8ba62, 0xd5e75b14, 0xe5c1d38e,
    0x78ce32f8, 0x04af1723, 0x99a0f655, 0xfc6d5c95, 0x6162bde3,
    0x1d039838, 0x800c794e, 0xd698cdb8, 0x4b972cce, 0x37f60915,
    0xaaf9e863, 0xcf3442a3, 0x523ba3d5, 0x2e5a860e, 0xb3556778,
    0x4e17973a, 0xd318764c, 0xaf795397, 0x3276b2e1, 0x57bb1821,
    0xcab4f957, 0xb6d5dc8c, 0x2bda3dfa, 0x7d4e890c, 0xe041687a,
    0x9c204da1, 0x012facd7, 0x64e20617, 0xf9ede761, 0x858cc2ba,
    0x188323cc, 0x28a5ab56, 0xb5aa4a20, 0xc9cb6ffb, 0x54c48e8d,
    0x3109244d, 0xac06c53b, 0xd067e0e0, 0x4d680196, 0x1bfcb560,
    0x86f35416, 0xfa9271cd, 0x679d90bb, 0x02503a7b, 0x9f5fdb0d,
    0xe33efed6, 0x7e311fa0, 0xc2ca1813, 0x5fc5f965, 0x23a4dcbe,
    0xbeab3dc8, 0xdb669708, 0x4669767e, 0x3a0853a5, 0xa707b2d3,
    0xf1930625, 0x6c9ce753, 0x10fdc288, 0x8df223fe, 0xe83f893e,
    0x75306848, 0x09514d93, 0x945eace5, 0xa478247f, 0x3977c509,
    0x4516e0d2, 0xd81901a4, 0xbdd4ab64, 0x20db4a12, 0x5cba6fc9,
    0xc1b58ebf, 0x97213a49, 0x0a2edb3f, 0x764ffee4, 0xeb401f92,
    0x8e8db552, 0x13825424, 0x6fe371ff, 0xf2ec9089, 0x0fae60cb,
    0x92a181bd, 0xeec0a466, 0x73cf4510, 0x1602efd0, 0x8b0d0ea6,
    0xf76c2b7d, 0x6a63ca0b, 0x3cf77efd, 0xa1f89f8b, 0xdd99ba50,
    0x40965b26, 0x255bf1e6, 0xb8541090, 0xc435354b, 0x593ad43d,
    0x691c5ca7, 0xf413bdd1, 0x8872980a, 0x157d797c, 0x70b0d3bc,
    0xedbf32ca, 0x91de1711, 0x0cd1f667, 0x5a454291, 0xc74aa3e7,
    0xbb2b863c, 0x2624674a, 0x43e9cd8a, 0xdee62cfc, 0xa2870927,
    0x3f88e851},
   {0x00000000, 0xdd96d985, 0x605cb54b, 0xbdca6cce, 0xc0b96a96,
    0x1d2fb313, 0xa0e5dfdd, 0x7d730658, 0x5a03d36d, 0x87950ae8,
    0x3a5f6626, 0xe7c9bfa3, 0x9abab9fb, 0x472c607e, 0xfae60cb0,
    0x2770d535, 0xb407a6da, 0x69917f5f, 0xd45b1391, 0x09cdca14,
    0x74becc4c, 0xa92815c9, 0x14e27907, 0xc974a082, 0xee0475b7,
    0x3392ac32, 0x8e58c0fc, 0x53ce1979, 0x2ebd1f21, 0xf32bc6a4,
    0x4ee1aa6a, 0x937773ef, 0xb37e4bf5, 0x6ee89270, 0xd322febe,
    0x0eb4273b, 0x73c72163, 0xae51f8e6, 0x139b9428, 0xce0d4dad,
    0xe97d9898, 0x34eb411d, 0x89212dd3, 0x54b7f456, 0x29c4f20e,
    0xf4522b8b, 0x49984745, 0x940e9ec0, 0x0779ed2f, 0xdaef34aa,
    0x67255864, 0xbab381e1, 0xc7c087b9, 0x1a565e3c, 0xa79c32f2,
    0x7a0aeb77, 0x5d7a3e42, 0x80ece7c7, 0x3d268b09, 0xe0b0528c,
    0x9dc354d4, 0x40558d51, 0xfd9fe19f, 0x2009381a, 0xbd8d91ab,
    0x601b482e, 0xddd124e0, 0x0047fd65, 0x7d34fb3d, 0xa0a222b8,
    0x1d684e76, 0xc0fe97f3, 0xe78e42c6, 0x3a189b43, 0x87d2f78d,
    0x5a442e08, 0x27372850, 0xfaa1f1d5, 0x476b9d1b, 0x9afd449e,
    0x098a3771, 0xd41ceef4, 0x69d6823a, 0xb4405bbf, 0xc9335de7,
    0x14a58462, 0xa96fe8ac, 0x74f93129, 0x5389e41c, 0x8e1f3d99,
    0x33d55157, 0xee4388d2, 0x93308e8a, 0x4ea6570f, 0xf36c3bc1,
    0x2efae244, 0x0ef3da5e, 0xd36503db, 0x6eaf6f15, 0xb339b690,
    0xce4ab0c8, 0x13dc694d, 0xae160583, 0x7380dc06, 0x54f00933,
    0x8966d0b6, 0x34acbc78, 0xe93a65fd, 0x944963a5, 0x49dfba20,
    0xf415d6ee, 0x29830f6b, 0xbaf47c84, 0x6762a501, 0xdaa8c9cf,
    0x073e104a, 0x7a4d1612, 0xa7dbcf97, 0x1a11a359, 0xc7877adc,
    0xe0f7afe9, 0x3d61766c, 0x80ab1aa2, 0x5d3dc327, 0x204ec57f,
    0xfdd81cfa, 0x40127034, 0x9d84a9b1, 0xa06a2517, 0x7dfcfc92,
    0xc036905c, 0x1da049d9, 0x60d34f81, 0xbd459604, 0x008ffaca,
    0xdd19234f, 0xfa69f67a, 0x27ff2fff, 0x9a354331, 0x47a39ab4,
    0x3ad09cec, 0xe7464569, 0x5a8c29a7, 0x871af022, 0x146d83cd,
    0xc9fb5a48, 0x74313686, 0xa9a7ef03, 0xd4d4e95b, 0x094230de,
    0xb4885c10, 0x691e8595, 0x4e6e50a0, 0x93f88925, 0x2e32e5eb,
    0xf3a43c6e, 0x8ed73a36, 0x5341e3b3, 0xee8b8f7d, 0x331d56f8,
    0x13146ee2, 0xce82b767, 0x7348dba9, 0xaede022c, 0xd3ad0474,
    0x0e3bddf1, 0xb3f1b13f, 0x6e6768ba, 0x4917bd8f, 0x9481640a,
    0x294b08c4, 0xf4ddd141, 0x89aed719, 0x54380e9c, 0xe9f26252,
    0x3464bbd7, 0xa713c838, 0x7a8511bd, 0xc74f7d73, 0x1ad9a4f6,
    0x67aaa2ae, 0xba3c7b2b, 0x07f617e5, 0xda60ce60, 0xfd101b55,
    0x2086c2d0, 0x9d4cae1e, 0x40da779b, 0x3da971c3, 0xe03fa846,
    0x5df5c488, 0x80631d0d, 0x1de7b4bc, 0xc0716d39, 0x7dbb01f7,
    0xa02dd872, 0xdd5ede2a, 0x00c807af, 0xbd026b61, 0x6094b2e4,
    0x47e467d1, 0x9a72be54, 0x27b8d29a, 0xfa2e0b1f, 0x875d0d47,
    0x5acbd4c2, 0xe701b80c, 0x3a976189, 0xa9e01266, 0x7476cbe3,
    0xc9bca72d, 0x142a7ea8, 0x695978f0, 0xb4cfa175, 0x0905cdbb,
    0xd493143e, 0xf3e3c10b, 0x2e75188e, 0x93bf7440, 0x4e29adc5,
    0x335aab9d, 0xeecc7218, 0x53061ed6, 0x8e90c753, 0xae99ff49,
    0x730f26cc, 0xcec54a02, 0x13539387, 0x6e2095df, 0xb3b64c5a,
    0x0e7c2094, 0xd3eaf911, 0xf49a2c24, 0x290cf5a1, 0x94c6996f,
    0x495040ea, 0x342346b2, 0xe9b59f37, 0x547ff3f9, 0x89e92a7c,
    0x1a9e5993, 0xc7088016, 0x7ac2ecd8, 0xa754355d, 0xda273305,
    0x07b1ea80, 0xba7b864e, 0x67ed5fcb, 0x409d8afe, 0x9d0b537b,
    0x20c13fb5, 0xfd57e630, 0x8024e068, 0x5db239ed, 0xe0785523,
    0x3dee8ca6},
   {0x00000000, 0x9ba54c6f, 0xec3b9e9f, 0x779ed2f0, 0x03063b7f,
    0x98a37710, 0xef3da5e0, 0x7498e98f, 0x060c76fe, 0x9da93a91,
    0xea37e861, 0x7192a40e, 0x050a4d81, 0x9eaf01ee, 0xe931d31e,
    0x72949f71, 0x0c18edfc, 0x97bda193, 0xe0237363, 0x7b863f0c,
    0x0f1ed683, 0x94bb9aec, 0xe325481c, 0x78800473, 0x0a149b02,
    0x91b1d76d, 0xe62f059d, 0x7d8a49f2, 0x0912a07d, 0x92b7ec12,
    0xe5293ee2, 0x7e8c728d, 0x1831dbf8, 0x83949797, 0xf40a4567,
    0x6faf0908, 0x1b37e087, 0x8092ace8, 0xf70c7e18, 0x6ca93277,
    0x1e3dad06, 0x8598e169, 0xf2063399, 0x69a37ff6, 0x1d3b9679,
    0x869eda16, 0xf10008e6, 0x6aa54489, 0x14293604, 0x8f8c7a6b,
    0xf812a89b, 0x63b7e4f4, 0x172f0d7b, 0x8c8a4114, 0xfb1493e4,
    0x60b1df8b, 0x122540fa, 0x89800c95, 0xfe1ede65, 0x65bb920a,
    0x11237b85, 0x8a8637ea, 0xfd18e51a, 0x66bda975, 0x3063b7f0,
    0xabc6fb9f, 0xdc58296f, 0x47fd6500, 0x33658c8f, 0xa8c0c0e0,
    0xdf5e1210, 0x44fb5e7f, 0x366fc10e, 0xadca8d61, 0xda545f91,
    0x41f113fe, 0x3569fa71, 0xaeccb61e, 0xd95264ee, 0x42f72881,
    0x3c7b5a0c, 0xa7de1663, 0xd040c493, 0x4be588fc, 0x3f7d6173,
    0xa4d82d1c, 0xd346ffec, 0x48e3b383, 0x3a772cf2, 0xa1d2609d,
    0xd64cb26d, 0x4de9fe02, 0x3971178d, 0xa2d45be2, 0xd54a8912,
    0x4eefc57d, 0x28526c08, 0xb3f72067, 0xc469f297, 0x5fccbef8,
    0x2b545777, 0xb0f11b18, 0xc76fc9e8, 0x5cca8587, 0x2e5e1af6,
    0xb5fb5699, 0xc2658469, 0x59c0c806, 0x2d582189, 0xb6fd6de6,
    0xc163bf16, 0x5ac6f379, 0x244a81f4, 0xbfefcd9b, 0xc8711f6b,
    0x53d45304, 0x274cba8b, 0xbce9f6e4, 0xcb772414, 0x50d2687b,
    0x2246f70a, 0xb9e3bb65, 0xce7d6995, 0x55d825fa, 0x2140cc75,
    0xbae5801a, 0xcd7b52ea, 0x56de1e85, 0x60c76fe0, 0xfb62238f,
    0x8cfcf17f, 0x1759bd10, 0x63c1549f, 0xf86418f0, 0x8ffaca00,
    0x145f866f, 0x66cb191e, 0xfd6e5571, 0x8af08781, 0x1155cbee,
    0x65cd2261, 0xfe686e0e, 0x89f6bcfe, 0x1253f091, 0x6cdf821c,
    0xf77ace73, 0x80e41c83, 0x1b4150ec, 0x6fd9b963, 0xf47cf50c,
    0x83e227fc, 0x18476b93, 0x6ad3f4e2, 0xf176b88d, 0x86e86a7d,
    0x1d4d2612, 0x69d5cf9d, 0xf27083f2, 0x85ee5102, 0x1e4b1d6d,
    0x78f6b418, 0xe353f877, 0x94cd2a87, 0x0f6866e8, 0x7bf08f67,
    0xe055c308, 0x97cb11f8, 0x0c6e5d97, 0x7efac2e6, 0xe55f8e89,
    0x92c15c79, 0x09641016, 0x7dfcf999, 0xe659b5f6, 0x91c76706,
    0x0a622b69, 0x74ee59e4, 0xef4b158b, 0x98d5c77b, 0x03708b14,
    0x77e8629b, 0xec4d2ef4, 0x9bd3fc04, 0x0076b06b, 0x72e22f1a,
    0xe9476375, 0x9ed9b185, 0x057cfdea, 0x71e41465, 0xea41580a,
    0x9ddf8afa, 0x067ac695, 0x50a4d810, 0xcb01947f, 0xbc9f468f,
    0x273a0ae0, 0x53a2e36f, 0xc807af00, 0xbf997df0, 0x243c319f,
    0x56a8aeee, 0xcd0de281, 0xba933071, 0x21367c1e, 0x55ae9591,
    0xce0bd9fe, 0xb9950b0e, 0x22304761, 0x5cbc35ec, 0xc7197983,
    0xb087ab73, 0x2b22e71c, 0x5fba0e93, 0xc41f42fc, 0xb381900c,
    0x2824dc63, 0x5ab04312, 0xc1150f7d, 0xb68bdd8d, 0x2d2e91e2,
    0x59b6786d, 0xc2133402, 0xb58de6f2, 0x2e28aa9d, 0x489503e8,
    0xd3304f87, 0xa4ae9d77, 0x3f0bd118, 0x4b933897, 0xd03674f8,
    0xa7a8a608, 0x3c0dea67, 0x4e997516, 0xd53c3979, 0xa2a2eb89,
    0x3907a7e6, 0x4d9f4e69, 0xd63a0206, 0xa1a4d0f6, 0x3a019c99,
    0x448dee14, 0xdf28a27b, 0xa8b6708b, 0x33133ce4, 0x478bd56b,
    0xdc2e9904, 0xabb04bf4, 0x3015079b, 0x428198ea, 0xd924d485,
    0xaeba0675, 0x351f4a1a, 0x4187a395, 0xda22effa, 0xadbc3d0a,
    0x36197165},
   {0x00000000, 0xc18edfc0, 0x586cb9c1, 0x99e26601, 0xb0d97382,
    0x7157ac42, 0xe8b5ca43, 0x293b1583, 0xbac3e145, 0x7b4d3e85,
    0xe2af5884, 0x23218744, 0x0a1a92c7, 0xcb944d07, 0x52762b06,
    0x93f8f4c6, 0xaef6c4cb, 0x6f781b0b, 0xf69a7d0a, 0x3714a2ca,
    0x1e2fb749, 0xdfa16889, 0x46430e88, 0x87cdd148, 0x1435258e,
    0xd5bbfa4e, 0x4c599c4f, 0x8dd7438f, 0xa4ec560c, 0x656289cc,
    0xfc80efcd, 0x3d0e300d, 0x869c8fd7, 0x47125017, 0xdef03616,
    0x1f7ee9d6, 0x3645fc55, 0xf7cb2395, 0x6e294594, 0xafa79a54,
    0x3c5f6e92, 0xfdd1b152, 0x6433d753, 0xa5bd0893, 0x8c861d10,
    0x4d08c2d0, 0xd4eaa4d1, 0x15647b11, 0x286a4b1c, 0xe9e494dc,
    0x7006f2dd, 0xb1882d1d, 0x98b3389e, 0x593de75e, 0xc0df815f,
    0x01515e9f, 0x92a9aa59, 0x53277599, 0xcac51398, 0x0b4bcc58,
    0x2270d9db, 0xe3fe061b, 0x7a1c601a, 0xbb92bfda, 0xd64819ef,
    0x17c6c62f, 0x8e24a02e, 0x4faa7fee, 0x66916a6d, 0xa71fb5ad,
    0x3efdd3ac, 0xff730c6c, 0x6c8bf8aa, 0xad05276a, 0x34e7416b,
    0xf5699eab, 0xdc528b28, 0x1ddc54e8, 0x843e32e9, 0x45b0ed29,
    0x78bedd24, 0xb93002e4, 0x20d264e5, 0xe15cbb25, 0xc867aea6,
    0x09e97166, 0x900b1767, 0x5185c8a7, 0xc27d3c61, 0x03f3e3a1,
    0x9a1185a0, 0x5b9f5a60, 0x72a44fe3, 0xb32a9023, 0x2ac8f622,
    0xeb4629e2, 0x50d49638, 0x915a49f8, 0x08b82ff9, 0xc936f039,
    0xe00de5ba, 0x21833a7a, 0xb8615c7b, 0x79ef83bb, 0xea17777d,
    0x2b99a8bd, 0xb27bcebc, 0x73f5117c, 0x5ace04ff, 0x9b40db3f,
    0x02a2bd3e, 0xc32c62fe, 0xfe2252f3, 0x3fac8d33, 0xa64eeb32,
    0x67c034f2, 0x4efb2171, 0x8f75feb1, 0x169798b0, 0xd7194770,
    0x44e1b3b6, 0x856f6c76, 0x1c8d0a77, 0xdd03d5b7, 0xf438c034,
    0x35b61ff4, 0xac5479f5, 0x6ddaa635, 0x77e1359f, 0xb66fea5f,
    0x2f8d8c5e, 0xee03539e, 0xc738461d, 0x06b699dd, 0x9f54ffdc,
    0x5eda201c, 0xcd22d4da, 0x0cac0b1a, 0x954e6d1b, 0x54c0b2db,
    0x7dfba758, 0xbc757898, 0x25971e99, 0xe419c159, 0xd917f154,
    0x18992e94, 0x817b4895, 0x40f59755, 0x69ce82d6, 0xa8405d16,
    0x31a23b17, 0xf02ce4d7, 0x63d41011, 0xa25acfd1, 0x3bb8a9d0,
    0xfa367610, 0xd30d6393, 0x1283bc53, 0x8b61da52, 0x4aef0592,
    0xf17dba48, 0x30f36588, 0xa9110389, 0x689fdc49, 0x41a4c9ca,
    0x802a160a, 0x19c8700b, 0xd846afcb, 0x4bbe5b0d, 0x8a3084cd,
    0x13d2e2cc, 0xd25c3d0c, 0xfb67288f, 0x3ae9f74f, 0xa30b914e,
    0x62854e8e, 0x5f8b7e83, 0x9e05a143, 0x07e7c742, 0xc6691882,
    0xef520d01, 0x2edcd2c1, 0xb73eb4c0, 0x76b06b00, 0xe5489fc6,
    0x24c64006, 0xbd242607, 0x7caaf9c7, 0x5591ec44, 0x941f3384,
    0x0dfd5585, 0xcc738a45, 0xa1a92c70, 0x6027f3b0, 0xf9c595b1,
    0x384b4a71, 0x11705ff2, 0xd0fe8032, 0x491ce633, 0x889239f3,
    0x1b6acd35, 0xdae412f5, 0x430674f4, 0x8288ab34, 0xabb3beb7,
    0x6a3d6177, 0xf3df0776, 0x3251d8b6, 0x0f5fe8bb, 0xced1377b,
    0x5733517a, 0x96bd8eba, 0xbf869b39, 0x7e0844f9, 0xe7ea22f8,
    0x2664fd38, 0xb59c09fe, 0x7412d63e, 0xedf0b03f, 0x2c7e6fff,
    0x05457a7c, 0xc4cba5bc, 0x5d29c3bd, 0x9ca71c7d, 0x2735a3a7,
    0xe6bb7c67, 0x7f591a66, 0xbed7c5a6, 0x97ecd025, 0x56620fe5,
    0xcf8069e4, 0x0e0eb624, 0x9df642e2, 0x5c789d22, 0xc59afb23,
    0x041424e3, 0x2d2f3160, 0xeca1eea0, 0x754388a1, 0xb4cd5761,
    0x89c3676c, 0x484db8ac, 0xd1afdead, 0x1021016d, 0x391a14ee,
    0xf894cb2e, 0x6176ad2f, 0xa0f872ef, 0x33008629, 0xf28e59e9,
    0x6b6c3fe8, 0xaae2e028, 0x83d9f5ab, 0x42572a6b, 0xdbb54c6a,
    0x1a3b93aa},
   {0x00000000, 0xefc26b3e, 0x04f5d03d, 0xeb37bb03, 0x09eba07a,
    0xe629cb44, 0x0d1e7047, 0xe2dc1b79, 0x13d740f4, 0xfc152bca,
    0x172290c9, 0xf8e0fbf7, 0x1a3ce08e, 0xf5fe8bb0, 0x1ec930b3,
    0xf10b5b8d, 0x27ae81e8, 0xc86cead6, 0x235b51d5, 0xcc993aeb,
    0x2e452192, 0xc1874aac, 0x2ab0f1af, 0xc5729a91, 0x3479c11c,
    0xdbbbaa22, 0x308c1121, 0xdf4e7a1f, 0x3d926166, 0xd2500a58,
    0x3967b15b, 0xd6a5da65, 0x4f5d03d0, 0xa09f68ee, 0x4ba8d3ed,
    0xa46ab8d3, 0x46b6a3aa, 0xa974c894, 0x42437397, 0xad8118a9,
    0x5c8a4324, 0xb348281a, 0x587f9319, 0xb7bdf827, 0x5561e35e,
    0xbaa38860, 0x51943363, 0xbe56585d, 0x68f38238, 0x8731e906,
    0x6c065205, 0x83c4393b, 0x61182242, 0x8eda497c, 0x65edf27f,
    0x8a2f9941, 0x7b24c2cc, 0x94e6a9f2, 0x7fd112f1, 0x901379cf,
    0x72cf62b6, 0x9d0d0988, 0x763ab28b, 0x99f8d9b5, 0x9eba07a0,
    0x71786c9e, 0x9a4fd79d, 0x758dbca3, 0x9751a7da, 0x7893cce4,
    0x93a477e7, 0x7c661cd9, 0x8d6d4754, 0x62af2c6a, 0x89989769,
    0x665afc57, 0x8486e72e, 0x6b448c10, 0x80733713, 0x6fb15c2d,
    0xb9148648, 0x56d6ed76, 0xbde15675, 0x52233d4b, 0xb0ff2632,
    0x5f3d4d0c, 0xb40af60f, 0x5bc89d31, 0xaac3c6bc, 0x4501ad82,
    0xae361681, 0x41f47dbf, 0xa32866c6, 0x4cea0df8, 0xa7ddb6fb,
    0x481fddc5, 0xd1e70470, 0x3e256f4e, 0xd512d44d, 0x3ad0bf73,
    0xd80ca40a, 0x37cecf34, 0xdcf97437, 0x333b1f09, 0xc2304484,
    0x2df22fba, 0xc6c594b9, 0x2907ff87, 0xcbdbe4fe, 0x24198fc0,
    0xcf2e34c3, 0x20ec5ffd, 0xf6498598, 0x198beea6, 0xf2bc55a5,
    0x1d7e3e9b, 0xffa225e2, 0x10604edc, 0xfb57f5df, 0x14959ee1,
    0xe59ec56c, 0x0a5cae52, 0xe16b1551, 0x0ea97e6f, 0xec756516,
    0x03b70e28, 0xe880b52b, 0x0742de15, 0xe6050901, 0x09c7623f,
    0xe2f0d93c, 0x0d32b202, 0xefeea97b, 0x002cc245, 0xeb1b7946,
    0x04d91278, 0xf5d249f5, 0x1a1022cb, 0xf12799c8, 0x1ee5f2f6,
    0xfc39e98f, 0x13fb82b1, 0xf8cc39b2, 0x170e528c, 0xc1ab88e9,
    0x2e69e3d7, 0xc55e58d4, 0x2a9c33ea, 0xc8402893, 0x278243ad,
    0xccb5f8ae, 0x23779390, 0xd27cc81d, 0x3dbea323, 0xd6891820,
    0x394b731e, 0xdb976867, 0x34550359, 0xdf62b85a, 0x30a0d364,
    0xa9580ad1, 0x469a61ef, 0xadaddaec, 0x426fb1d2, 0xa0b3aaab,
    0x4f71c195, 0xa4467a96, 0x4b8411a8, 0xba8f4a25, 0x554d211b,
    0xbe7a9a18, 0x51b8f126, 0xb364ea5f, 0x5ca68161, 0xb7913a62,
    0x5853515c, 0x8ef68b39, 0x6134e007, 0x8a035b04, 0x65c1303a,
    0x871d2b43, 0x68df407d, 0x83e8fb7e, 0x6c2a9040, 0x9d21cbcd,
    0x72e3a0f3, 0x99d41bf0, 0x761670ce, 0x94ca6bb7, 0x7b080089,
    0x903fbb8a, 0x7ffdd0b4, 0x78bf0ea1, 0x977d659f, 0x7c4ade9c,
    0x9388b5a2, 0x7154aedb, 0x9e96c5e5, 0x75a17ee6, 0x9a6315d8,
    0x6b684e55, 0x84aa256b, 0x6f9d9e68, 0x805ff556, 0x6283ee2f,
    0x8d418511, 0x66763e12, 0x89b4552c, 0x5f118f49, 0xb0d3e477,
    0x5be45f74, 0xb426344a, 0x56fa2f33, 0xb938440d, 0x520fff0e,
    0xbdcd9430, 0x4cc6cfbd, 0xa304a483, 0x48331f80, 0xa7f174be,
    0x452d6fc7, 0xaaef04f9, 0x41d8bffa, 0xae1ad4c4, 0x37e20d71,
    0xd820664f, 0x3317dd4c, 0xdcd5b672, 0x3e09ad0b, 0xd1cbc635,
    0x3afc7d36, 0xd53e1608, 0x24354d85, 0xcbf726bb, 0x20c09db8,
    0xcf02f686, 0x2ddeedff, 0xc21c86c1, 0x292b3dc2, 0xc6e956fc,
    0x104c8c99, 0xff8ee7a7, 0x14b95ca4, 0xfb7b379a, 0x19a72ce3,
    0xf66547dd, 0x1d52fcde, 0xf29097e0, 0x039bcc6d, 0xec59a753,
    0x076e1c50, 0xe8ac776e, 0x0a706c17, 0xe5b20729, 0x0e85bc2a,
    0xe147d714},
   {0x00000000, 0x177b1443, 0x2ef62886, 0x398d3cc5, 0x5dec510c,
    0x4a97454f, 0x731a798a, 0x64616dc9, 0xbbd8a218, 0xaca3b65b,
    0x952e8a9e, 0x82559edd, 0xe634f314, 0xf14fe757, 0xc8c2db92,
    0xdfb9cfd1, 0xacc04271, 0xbbbb5632, 0x82366af7, 0x954d7eb4,
    0xf12c137d, 0xe657073e, 0xdfda3bfb, 0xc8a12fb8, 0x1718e069,
    0x0063f42a, 0x39eec8ef, 0x2e95dcac, 0x4af4b165, 0x5d8fa526,
    0x640299e3, 0x73798da0, 0x82f182a3, 0x958a96e0, 0xac07aa25,
    0xbb7cbe66, 0xdf1dd3af, 0xc866c7ec, 0xf1ebfb29, 0xe690ef6a,
    0x392920bb, 0x2e5234f8, 0x17df083d, 0x00a41c7e, 0x64c571b7,
    0x73be65f4, 0x4a335931, 0x5d484d72, 0x2e31c0d2, 0x394ad491,
    0x00c7e854, 0x17bcfc17, 0x73dd91de, 0x64a6859d, 0x5d2bb958,
    0x4a50ad1b, 0x95e962ca, 0x82927689, 0xbb1f4a4c, 0xac645e0f,
    0xc80533c6, 0xdf7e2785, 0xe6f31b40, 0xf1880f03, 0xde920307,
    0xc9e91744, 0xf0642b81, 0xe71f3fc2, 0x837e520b, 0x94054648,
    0xad887a8d, 0xbaf36ece, 0x654aa11f, 0x7231b55c, 0x4bbc8999,
    0x5cc79dda, 0x38a6f013, 0x2fdde450, 0x1650d895, 0x012bccd6,
    0x72524176, 0x65295535, 0x5ca469f0, 0x4bdf7db3, 0x2fbe107a,
    0x38c50439, 0x014838fc, 0x16332cbf, 0xc98ae36e, 0xdef1f72d,
    0xe77ccbe8, 0xf007dfab, 0x9466b262, 0x831da621, 0xba909ae4,
    0xadeb8ea7, 0x5c6381a4, 0x4b1895e7, 0x7295a922, 0x65eebd61,
    0x018fd0a8, 0x16f4c4eb, 0x2f79f82e, 0x3802ec6d, 0xe7bb23bc,
    0xf0c037ff, 0xc94d0b3a, 0xde361f79, 0xba5772b0, 0xad2c66f3,
    0x94a15a36, 0x83da4e75, 0xf0a3c3d5, 0xe7d8d796, 0xde55eb53,
    0xc92eff10, 0xad4f92d9, 0xba34869a, 0x83b9ba5f, 0x94c2ae1c,
    0x4b7b61cd, 0x5c00758e, 0x658d494b, 0x72f65d08, 0x169730c1,
    0x01ec2482, 0x38611847, 0x2f1a0c04, 0x6655004f, 0x712e140c,
    0x48a328c9, 0x5fd83c8a, 0x3bb95143, 0x2cc24500, 0x154f79c5,
    0x02346d86, 0xdd8da257, 0xcaf6b614, 0xf37b8ad1, 0xe4009e92,
    0x8061f35b, 0x971ae718, 0xae97dbdd, 0xb9eccf9e, 0xca95423e,
    0xddee567d, 0xe4636ab8, 0xf3187efb, 0x97791332, 0x80020771,
    0xb98f3bb4, 0xaef42ff7, 0x714de026, 0x6636f465, 0x5fbbc8a0,
    0x48c0dce3, 0x2ca1b12a, 0x3bdaa569, 0x025799ac, 0x152c8def,
    0xe4a482ec, 0xf3df96af, 0xca52aa6a, 0xdd29be29, 0xb948d3e0,
    0xae33c7a3, 0x97befb66, 0x80c5ef25, 0x5f7c20f4, 0x480734b7,
    0x718a0872, 0x66f11c31, 0x029071f8, 0x15eb65bb, 0x2c66597e,
    0x3b1d4d3d, 0x4864c09d, 0x5f1fd4de, 0x6692e81b, 0x71e9fc58,
    0x15889191, 0x02f385d2, 0x3b7eb917, 0x2c05ad54, 0xf3bc6285,
    0xe4c776c6, 0xdd4a4a03, 0xca315e40, 0xae503389, 0xb92b27ca,
    0x80a61b0f, 0x97dd0f4c, 0xb8c70348, 0xafbc170b, 0x96312bce,
    0x814a3f8d, 0xe52b5244, 0xf2504607, 0xcbdd7ac2, 0xdca66e81,
    0x031fa150, 0x1464b513, 0x2de989d6, 0x3a929d95, 0x5ef3f05c,
    0x4988e41f, 0x7005d8da, 0x677ecc99, 0x14074139, 0x037c557a,
    0x3af169bf, 0x2d8a7dfc, 0x49eb1035, 0x5e900476, 0x671d38b3,
    0x70662cf0, 0xafdfe321, 0xb8a4f762, 0x8129cba7, 0x9652dfe4,
    0xf233b22d, 0xe548a66e, 0xdcc59aab, 0xcbbe8ee8, 0x3a3681eb,
    0x2d4d95a8, 0x14c0a96d, 0x03bbbd2e, 0x67dad0e7, 0x70a1c4a4,
    0x492cf861, 0x5e57ec22, 0x81ee23f3, 0x969537b0, 0xaf180b75,
    0xb8631f36, 0xdc0272ff, 0xcb7966bc, 0xf2f45a79, 0xe58f4e3a,
    0x96f6c39a, 0x818dd7d9, 0xb800eb1c, 0xaf7bff5f, 0xcb1a9296,
    0xdc6186d5, 0xe5ecba10, 0xf297ae53, 0x2d2e6182, 0x3a5575c1,
    0x03d84904, 0x14a35d47, 0x70c2308e, 0x67b924cd, 0x5e341808,
    0x494f0c4b}};

local const z_word_t FAR crc_braid_big_table[][256] = {
   {0x0000000000000000, 0x43147b1700000000, 0x8628f62e00000000,
    0xc53c8d3900000000, 0x0c51ec5d00000000, 0x4f45974a00000000,
    0x8a791a7300000000, 0xc96d616400000000, 0x18a2d8bb00000000,
    0x5bb6a3ac00000000, 0x9e8a2e9500000000, 0xdd9e558200000000,
    0x14f334e600000000, 0x57e74ff100000000, 0x92dbc2c800000000,
    0xd1cfb9df00000000, 0x7142c0ac00000000, 0x3256bbbb00000000,
    0xf76a368200000000, 0xb47e4d9500000000, 0x7d132cf100000000,
    0x3e0757e600000000, 0xfb3bdadf00000000, 0xb82fa1c800000000,
    0x69e0181700000000, 0x2af4630000000000, 0xefc8ee3900000000,
    0xacdc952e00000000, 0x65b1f44a00000000, 0x26a58f5d00000000,
    0xe399026400000000, 0xa08d797300000000, 0xa382f18200000000,
    0xe0968a9500000000, 0x25aa07ac00000000, 0x66be7cbb00000000,
    0xafd31ddf00000000, 0xecc766c800000000, 0x29fbebf100000000,
    0x6aef90e600000000, 0xbb20293900000000, 0xf834522e00000000,
    0x3d08df1700000000, 0x7e1ca40000000000, 0xb771c56400000000,
    0xf465be7300000000, 0x3159334a00000000, 0x724d485d00000000,
    0xd2c0312e00000000, 0x91d44a3900000000, 0x54e8c70000000000,
    0x17fcbc1700000000, 0xde91dd7300000000, 0x9d85a66400000000,
    0x58b92b5d00000000, 0x1bad504a00000000, 0xca62e99500000000,
    0x8976928200000000, 0x4c4a1fbb00000000, 0x0f5e64ac00000000,
    0xc63305c800000000, 0x85277edf00000000, 0x401bf3e600000000,
    0x030f88f100000000, 0x070392de00000000, 0x4417e9c900000000,
    0x812b64f000000000, 0xc23f1fe700000000, 0x0b527e8300000000,
    0x4846059400000000, 0x8d7a88ad00000000, 0xce6ef3ba00000000,
    0x1fa14a6500000000, 0x5cb5317200000000, 0x9989bc4b00000000,
    0xda9dc75c00000000, 0x13f0a63800000000, 0x50e4dd2f00000000,
    0x95d8501600000000, 0xd6cc2b0100000000, 0x7641527200000000,
    0x3555296500000000, 0xf069a45c00000000, 0xb37ddf4b00000000,
    0x7a10be2f00000000, 0x3904c53800000000, 0xfc38480100000000,
    0xbf2c331600000000, 0x6ee38ac900000000, 0x2df7f1de00000000,
    0xe8cb7ce700000000, 0xabdf07f000000000, 0x62b2669400000000,
    0x21a61d8300000000, 0xe49a90ba00000000, 0xa78eebad00000000,
    0xa481635c00000000, 0xe795184b00000000, 0x22a9957200000000,
    0x61bdee6500000000, 0xa8d08f0100000000, 0xebc4f41600000000,
    0x2ef8792f00000000, 0x6dec023800000000, 0xbc23bbe700000000,
    0xff37c0f000000000, 0x3a0b4dc900000000, 0x791f36de00000000,
    0xb07257ba00000000, 0xf3662cad00000000, 0x365aa19400000000,
    0x754eda8300000000, 0xd5c3a3f000000000, 0x96d7d8e700000000,
    0x53eb55de00000000, 0x10ff2ec900000000, 0xd9924fad00000000,
    0x9a8634ba00000000, 0x5fbab98300000000, 0x1caec29400000000,
    0xcd617b4b00000000, 0x8e75005c00000000, 0x4b498d6500000000,
    0x085df67200000000, 0xc130971600000000, 0x8224ec0100000000,
    0x4718613800000000, 0x040c1a2f00000000, 0x4f00556600000000,
    0x0c142e7100000000, 0xc928a34800000000, 0x8a3cd85f00000000,
    0x4351b93b00000000, 0x0045c22c00000000, 0xc5794f1500000000,
    0x866d340200000000, 0x57a28ddd00000000, 0x14b6f6ca00000000,
    0xd18a7bf300000000, 0x929e00e400000000, 0x5bf3618000000000,
    0x18e71a9700000000, 0xdddb97ae00000000, 0x9ecfecb900000000,
    0x3e4295ca00000000, 0x7d56eedd00000000, 0xb86a63e400000000,
    0xfb7e18f300000000, 0x3213799700000000, 0x7107028000000000,
    0xb43b8fb900000000, 0xf72ff4ae00000000, 0x26e04d7100000000,
    0x65f4366600000000, 0xa0c8bb5f00000000, 0xe3dcc04800000000,
    0x2ab1a12c00000000, 0x69a5da3b00000000, 0xac99570200000000,
    0xef8d2c1500000000, 0xec82a4e400000000, 0xaf96dff300000000,
    0x6aaa52ca00000000, 0x29be29dd00000000, 0xe0d348b900000000,
    0xa3c733ae00000000, 0x66fbbe9700000000, 0x25efc58000000000,
    0xf4207c5f00000000, 0xb734074800000000, 0x72088a7100000000,
    0x311cf16600000000, 0xf871900200000000, 0xbb65eb1500000000,
    0x7e59662c00000000, 0x3d4d1d3b00000000, 0x9dc0644800000000,
    0xded41f5f00000000, 0x1be8926600000000, 0x58fce97100000000,
    0x9191881500000000, 0xd285f30200000000, 0x17b97e3b00000000,
    0x54ad052c00000000, 0x8562bcf300000000, 0xc676c7e400000000,
    0x034a4add00000000, 0x405e31ca00000000, 0x893350ae00000000,
    0xca272bb900000000, 0x0f1ba68000000000, 0x4c0fdd9700000000,
    0x4803c7b800000000, 0x0b17bcaf00000000, 0xce2b319600000000,
    0x8d3f4a8100000000, 0x44522be500000000, 0x074650f200000000,
    0xc27addcb00000000, 0x816ea6dc00000000, 0x50a11f0300000000,
    0x13b5641400000000, 0xd689e92d00000000, 0x959d923a00000000,
    0x5cf0f35e00000000, 0x1fe4884900000000, 0xdad8057000000000,
    0x99cc7e6700000000, 0x3941071400000000, 0x7a557c0300000000,
    0xbf69f13a00000000, 0xfc7d8a2d00000000, 0x3510eb4900000000,
    0x7604905e00000000, 0xb3381d6700000000, 0xf02c667000000000,
    0x21e3dfaf00000000, 0x62f7a4b800000000, 0xa7cb298100000000,
    0xe4df529600000000, 0x2db233f200000000, 0x6ea648e500000000,
    0xab9ac5dc00000000, 0xe88ebecb00000000, 0xeb81363a00000000,
    0xa8954d2d00000000, 0x6da9c01400000000, 0x2ebdbb0300000000,
    0xe7d0da6700000000, 0xa4c4a17000000000, 0x61f82c4900000000,
    0x22ec575e00000000, 0xf323ee8100000000, 0xb037959600000000,
    0x750b18af00000000, 0x361f63b800000000, 0xff7202dc00000000,
    0xbc6679cb00000000, 0x795af4f200000000, 0x3a4e8fe500000000,
    0x9ac3f69600000000, 0xd9d78d8100000000, 0x1ceb00b800000000,
    0x5fff7baf00000000, 0x96921acb00000000, 0xd58661dc00000000,
    0x10baece500000000, 0x53ae97f200000000, 0x82612e2d00000000,
    0xc175553a00000000, 0x0449d80300000000, 0x475da31400000000,
    0x8e30c27000000000, 0xcd24b96700000000, 0x0818345e00000000,
    0x4b0c4f4900000000},
   {0x0000000000000000, 0x3e6bc2ef00000000, 0x3dd0f50400000000,
    0x03bb37eb00000000, 0x7aa0eb0900000000, 0x44cb29e600000000,
    0x47701e0d00000000, 0x791bdce200000000, 0xf440d71300000000,
    0xca2b15fc00000000, 0xc990221700000000, 0xf7fbe0f800000000,
    0x8ee03c1a00000000, 0xb08bfef500000000, 0xb330c91e00000000,
    0x8d5b0bf100000000, 0xe881ae2700000000, 0xd6ea6cc800000000,
    0xd5515b2300000000, 0xeb3a99cc00000000, 0x9221452e00000000,
    0xac4a87c100000000, 0xaff1b02a00000000, 0x919a72c500000000,
    0x1cc1793400000000, 0x22aabbdb00000000, 0x21118c3000000000,
    0x1f7a4edf00000000, 0x6661923d00000000, 0x580a50d200000000,
    0x5bb1673900000000, 0x65daa5d600000000, 0xd0035d4f00000000,
    0xee689fa000000000, 0xedd3a84b00000000, 0xd3b86aa400000000,
    0xaaa3b64600000000, 0x94c874a900000000, 0x9773434200000000,
    0xa91881ad00000000, 0x24438a5c00000000, 0x1a2848b300000000,
    0x19937f5800000000, 0x27f8bdb700000000, 0x5ee3615500000000,
    0x6088a3ba00000000, 0x6333945100000000, 0x5d5856be00000000,
    0x3882f36800000000, 0x06e9318700000000, 0x0552066c00000000,
    0x3b39c48300000000, 0x4222186100000000, 0x7c49da8e00000000,
    0x7ff2ed6500000000, 0x41992f8a00000000, 0xccc2247b00000000,
    0xf2a9e69400000000, 0xf112d17f00000000, 0xcf79139000000000,
    0xb662cf7200000000, 0x88090d9d00000000, 0x8bb23a7600000000,
    0xb5d9f89900000000, 0xa007ba9e00000000, 0x9e6c787100000000,
    0x9dd74f9a00000000, 0xa3bc8d7500000000, 0xdaa7519700000000,
    0xe4cc937800000000, 0xe777a49300000000, 0xd91c667c00000000,
    0x54476d8d00000000, 0x6a2caf6200000000, 0x6997988900000000,
    0x57fc5a6600000000, 0x2ee7868400000000, 0x108c446b00000000,
    0x1337738000000000, 0x2d5cb16f00000000, 0x488614b900000000,
    0x76edd65600000000, 0x7556e1bd00000000, 0x4b3d235200000000,
    0x3226ffb000000000, 0x0c4d3d5f00000000, 0x0ff60ab400000000,
    0x319dc85b00000000, 0xbcc6c3aa00000000, 0x82ad014500000000,
    0x811636ae00000000, 0xbf7df44100000000, 0xc66628a300000000,
    0xf80dea4c00000000, 0xfbb6dda700000000, 0xc5dd1f4800000000,
    0x7004e7d100000000, 0x4e6f253e00000000, 0x4dd412d500000000,
    0x73bfd03a00000000, 0x0aa40cd800000000, 0x34cfce3700000000,
    0x3774f9dc00000000, 0x091f3b3300000000, 0x844430c200000000,
    0xba2ff22d00000000, 0xb994c5c600000000, 0x87ff072900000000,
    0xfee4dbcb00000000, 0xc08f192400000000, 0xc3342ecf00000000,
    0xfd5fec2000000000, 0x988549f600000000, 0xa6ee8b1900000000,
    0xa555bcf200000000, 0x9b3e7e1d00000000, 0xe225a2ff00000000,
    0xdc4e601000000000, 0xdff557fb00000000, 0xe19e951400000000,
    0x6cc59ee500000000, 0x52ae5c0a00000000, 0x51156be100000000,
    0x6f7ea90e00000000, 0x166575ec00000000, 0x280eb70300000000,
    0x2bb580e800000000, 0x15de420700000000, 0x010905e600000000,
    0x3f62c70900000000, 0x3cd9f0e200000000, 0x02b2320d00000000,
    0x7ba9eeef00000000, 0x45c22c0000000000, 0x46791beb00000000,
    0x7812d90400000000, 0xf549d2f500000000, 0xcb22101a00000000,
    0xc89927f100000000, 0xf6f2e51e00000000, 0x8fe939fc00000000,
    0xb182fb1300000000, 0xb239ccf800000000, 0x8c520e1700000000,
    0xe988abc100000000, 0xd7e3692e00000000, 0xd4585ec500000000,
    0xea339c2a00000000, 0x932840c800000000, 0xad43822700000000,
    0xaef8b5cc00000000, 0x9093772300000000, 0x1dc87cd200000000,
    0x23a3be3d00000000, 0x201889d600000000, 0x1e734b3900000000,
    0x676897db00000000, 0x5903553400000000, 0x5ab862df00000000,
    0x64d3a03000000000, 0xd10a58a900000000, 0xef619a4600000000,
    0xecdaadad00000000, 0xd2b16f4200000000, 0xabaab3a000000000,
    0x95c1714f00000000, 0x967a46a400000000, 0xa811844b00000000,
    0x254a8fba00000000, 0x1b214d5500000000, 0x189a7abe00000000,
    0x26f1b85100000000, 0x5fea64b300000000, 0x6181a65c00000000,
    0x623a91b700000000, 0x5c51535800000000, 0x398bf68e00000000,
    0x07e0346100000000, 0x045b038a00000000, 0x3a30c16500000000,
    0x432b1d8700000000, 0x7d40df6800000000, 0x7efbe88300000000,
    0x40902a6c00000000, 0xcdcb219d00000000, 0xf3a0e37200000000,
    0xf01bd49900000000, 0xce70167600000000, 0xb76bca9400000000,
    0x8900087b00000000, 0x8abb3f9000000000, 0xb4d0fd7f00000000,
    0xa10ebf7800000000, 0x9f657d9700000000, 0x9cde4a7c00000000,
    0xa2b5889300000000, 0xdbae547100000000, 0xe5c5969e00000000,
    0xe67ea17500000000, 0xd815639a00000000, 0x554e686b00000000,
    0x6b25aa8400000000, 0x689e9d6f00000000, 0x56f55f8000000000,
    0x2fee836200000000, 0x1185418d00000000, 0x123e766600000000,
    0x2c55b48900000000, 0x498f115f00000000, 0x77e4d3b000000000,
    0x745fe45b00000000, 0x4a3426b400000000, 0x332ffa5600000000,
    0x0d4438b900000000, 0x0eff0f5200000000, 0x3094cdbd00000000,
    0xbdcfc64c00000000, 0x83a404a300000000, 0x801f334800000000,
    0xbe74f1a700000000, 0xc76f2d4500000000, 0xf904efaa00000000,
    0xfabfd84100000000, 0xc4d41aae00000000, 0x710de23700000000,
    0x4f6620d800000000, 0x4cdd173300000000, 0x72b6d5dc00000000,
    0x0bad093e00000000, 0x35c6cbd100000000, 0x367dfc3a00000000,
    0x08163ed500000000, 0x854d352400000000, 0xbb26f7cb00000000,
    0xb89dc02000000000, 0x86f602cf00000000, 0xffedde2d00000000,
    0xc1861cc200000000, 0xc23d2b2900000000, 0xfc56e9c600000000,
    0x998c4c1000000000, 0xa7e78eff00000000, 0xa45cb91400000000,
    0x9a377bfb00000000, 0xe32ca71900000000, 0xdd4765f600000000,
    0xdefc521d00000000, 0xe09790f200000000, 0x6dcc9b0300000000,
    0x53a759ec00000000, 0x501c6e0700000000, 0x6e77ace800000000,
    0x176c700a00000000, 0x2907b2e500000000, 0x2abc850e00000000,
    0x14d747e100000000},
   {0x0000000000000000, 0xc0df8ec100000000, 0xc1b96c5800000000,
    0x0166e29900000000, 0x8273d9b000000000, 0x42ac577100000000,
    0x43cab5e800000000, 0x83153b2900000000, 0x45e1c3ba00000000,
    0x853e4d7b00000000, 0x8458afe200000000, 0x4487212300000000,
    0xc7921a0a00000000, 0x074d94cb00000000, 0x062b765200000000,
    0xc6f4f89300000000, 0xcbc4f6ae00000000, 0x0b1b786f00000000,
    0x0a7d9af600000000, 0xcaa2143700000000, 0x49b72f1e00000000,
    0x8968a1df00000000, 0x880e434600000000, 0x48d1cd8700000000,
    0x8e25351400000000, 0x4efabbd500000000, 0x4f9c594c00000000,
    0x8f43d78d00000000, 0x0c56eca400000000, 0xcc89626500000000,
    0xcdef80fc00000000, 0x0d300e3d00000000, 0xd78f9c8600000000,
    0x1750124700000000, 0x1636f0de00000000, 0xd6e97e1f00000000,
    0x55fc453600000000, 0x9523cbf700000000, 0x9445296e00000000,
    0x549aa7af00000000, 0x926e5f3c00000000, 0x52b1d1fd00000000,
    0x53d7336400000000, 0x9308bda500000000, 0x101d868c00000000,
    0xd0c2084d00000000, 0xd1a4ead400000000, 0x117b641500000000,
    0x1c4b6a2800000000, 0xdc94e4e900000000, 0xddf2067000000000,
    0x1d2d88b100000000, 0x9e38b39800000000, 0x5ee73d5900000000,
    0x5f81dfc000000000, 0x9f5e510100000000, 0x59aaa99200000000,
    0x9975275300000000, 0x9813c5ca00000000, 0x58cc4b0b00000000,
    0xdbd9702200000000, 0x1b06fee300000000, 0x1a601c7a00000000,
    0xdabf92bb00000000, 0xef1948d600000000, 0x2fc6c61700000000,
    0x2ea0248e00000000, 0xee7faa4f00000000, 0x6d6a916600000000,
    0xadb51fa700000000, 0xacd3fd3e00000000, 0x6c0c73ff00000000,
    0xaaf88b6c00000000, 0x6a2705ad00000000, 0x6b41e73400000000,
    0xab9e69f500000000, 0x288b52dc00000000, 0xe854dc1d00000000,
    0xe9323e8400000000, 0x29edb04500000000, 0x24ddbe7800000000,
    0xe40230b900000000, 0xe564d22000000000, 0x25bb5ce100000000,
    0xa6ae67c800000000, 0x6671e90900000000, 0x67170b9000000000,
    0xa7c8855100000000, 0x613c7dc200000000, 0xa1e3f30300000000,
    0xa085119a00000000, 0x605a9f5b00000000, 0xe34fa47200000000,
    0x23902ab300000000, 0x22f6c82a00000000, 0xe22946eb00000000,
    0x3896d45000000000, 0xf8495a9100000000, 0xf92fb80800000000,
    0x39f036c900000000, 0xbae50de000000000, 0x7a3a832100000000,
    0x7b5c61b800000000, 0xbb83ef7900000000, 0x7d7717ea00000000,
    0xbda8992b00000000, 0xbcce7bb200000000, 0x7c11f57300000000,
    0xff04ce5a00000000, 0x3fdb409b00000000, 0x3ebda20200000000,
    0xfe622cc300000000, 0xf35222fe00000000, 0x338dac3f00000000,
    0x32eb4ea600000000, 0xf234c06700000000, 0x7121fb4e00000000,
    0xb1fe758f00000000, 0xb098971600000000, 0x704719d700000000,
    0xb6b3e14400000000, 0x766c6f8500000000, 0x770a8d1c00000000,
    0xb7d503dd00000000, 0x34c038f400000000, 0xf41fb63500000000,
    0xf57954ac00000000, 0x35a6da6d00000000, 0x9f35e17700000000,
    0x5fea6fb600000000, 0x5e8c8d2f00000000, 0x9e5303ee00000000,
    0x1d4638c700000000, 0xdd99b60600000000, 0xdcff549f00000000,
    0x1c20da5e00000000, 0xdad422cd00000000, 0x1a0bac0c00000000,
    0x1b6d4e9500000000, 0xdbb2c05400000000, 0x58a7fb7d00000000,
    0x987875bc00000000, 0x991e972500000000, 0x59c119e400000000,
    0x54f117d900000000, 0x942e991800000000, 0x95487b8100000000,
    0x5597f54000000000, 0xd682ce6900000000, 0x165d40a800000000,
    0x173ba23100000000, 0xd7e42cf000000000, 0x1110d46300000000,
    0xd1cf5aa200000000, 0xd0a9b83b00000000, 0x107636fa00000000,
    0x93630dd300000000, 0x53bc831200000000, 0x52da618b00000000,
    0x9205ef4a00000000, 0x48ba7df100000000, 0x8865f33000000000,
    0x890311a900000000, 0x49dc9f6800000000, 0xcac9a44100000000,
    0x0a162a8000000000, 0x0b70c81900000000, 0xcbaf46d800000000,
    0x0d5bbe4b00000000, 0xcd84308a00000000, 0xcce2d21300000000,
    0x0c3d5cd200000000, 0x8f2867fb00000000, 0x4ff7e93a00000000,
    0x4e910ba300000000, 0x8e4e856200000000, 0x837e8b5f00000000,
    0x43a1059e00000000, 0x42c7e70700000000, 0x821869c600000000,
    0x010d52ef00000000, 0xc1d2dc2e00000000, 0xc0b43eb700000000,
    0x006bb07600000000, 0xc69f48e500000000, 0x0640c62400000000,
    0x072624bd00000000, 0xc7f9aa7c00000000, 0x44ec915500000000,
    0x84331f9400000000, 0x8555fd0d00000000, 0x458a73cc00000000,
    0x702ca9a100000000, 0xb0f3276000000000, 0xb195c5f900000000,
    0x714a4b3800000000, 0xf25f701100000000, 0x3280fed000000000,
    0x33e61c4900000000, 0xf339928800000000, 0x35cd6a1b00000000,
    0xf512e4da00000000, 0xf474064300000000, 0x34ab888200000000,
    0xb7beb3ab00000000, 0x77613d6a00000000, 0x7607dff300000000,
    0xb6d8513200000000, 0xbbe85f0f00000000, 0x7b37d1ce00000000,
    0x7a51335700000000, 0xba8ebd9600000000, 0x399b86bf00000000,
    0xf944087e00000000, 0xf822eae700000000, 0x38fd642600000000,
    0xfe099cb500000000, 0x3ed6127400000000, 0x3fb0f0ed00000000,
    0xff6f7e2c00000000, 0x7c7a450500000000, 0xbca5cbc400000000,
    0xbdc3295d00000000, 0x7d1ca79c00000000, 0xa7a3352700000000,
    0x677cbbe600000000, 0x661a597f00000000, 0xa6c5d7be00000000,
    0x25d0ec9700000000, 0xe50f625600000000, 0xe46980cf00000000,
    0x24b60e0e00000000, 0xe242f69d00000000, 0x229d785c00000000,
    0x23fb9ac500000000, 0xe324140400000000, 0x60312f2d00000000,
    0xa0eea1ec00000000, 0xa188437500000000, 0x6157cdb400000000,
    0x6c67c38900000000, 0xacb84d4800000000, 0xaddeafd100000000,
    0x6d01211000000000, 0xee141a3900000000, 0x2ecb94f800000000,
    0x2fad766100000000, 0xef72f8a000000000, 0x2986003300000000,
    0xe9598ef200000000, 0xe83f6c6b00000000, 0x28e0e2aa00000000,
    0xabf5d98300000000, 0x6b2a574200000000, 0x6a4cb5db00000000,
    0xaa933b1a00000000},
   {0x0000000000000000, 0x6f4ca59b00000000, 0x9f9e3bec00000000,
    0xf0d29e7700000000, 0x7f3b060300000000, 0x1077a39800000000,
    0xe0a53def00000000, 0x8fe9987400000000, 0xfe760c0600000000,
    0x913aa99d00000000, 0x61e837ea00000000, 0x0ea4927100000000,
    0x814d0a0500000000, 0xee01af9e00000000, 0x1ed331e900000000,
    0x719f947200000000, 0xfced180c00000000, 0x93a1bd9700000000,
    0x637323e000000000, 0x0c3f867b00000000, 0x83d61e0f00000000,
    0xec9abb9400000000, 0x1c4825e300000000, 0x7304807800000000,
    0x029b140a00000000, 0x6dd7b19100000000, 0x9d052fe600000000,
    0xf2498a7d00000000, 0x7da0120900000000, 0x12ecb79200000000,
    0xe23e29e500000000, 0x8d728c7e00000000, 0xf8db311800000000,
    0x9797948300000000, 0x67450af400000000, 0x0809af6f00000000,
    0x87e0371b00000000, 0xe8ac928000000000, 0x187e0cf700000000,
    0x7732a96c00000000, 0x06ad3d1e00000000, 0x69e1988500000000,
    0x993306f200000000, 0xf67fa36900000000, 0x79963b1d00000000,
    0x16da9e8600000000, 0xe60800f100000000, 0x8944a56a00000000,
    0x0436291400000000, 0x6b7a8c8f00000000, 0x9ba812f800000000,
    0xf4e4b76300000000, 0x7b0d2f1700000000, 0x14418a8c00000000,
    0xe49314fb00000000, 0x8bdfb16000000000, 0xfa40251200000000,
    0x950c808900000000, 0x65de1efe00000000, 0x0a92bb6500000000,
    0x857b231100000000, 0xea37868a00000000, 0x1ae518fd00000000,
    0x75a9bd6600000000, 0xf0b7633000000000, 0x9ffbc6ab00000000,
    0x6f2958dc00000000, 0x0065fd4700000000, 0x8f8c653300000000,
    0xe0c0c0a800000000, 0x10125edf00000000, 0x7f5efb4400000000,
    0x0ec16f3600000000, 0x618dcaad00000000, 0x915f54da00000000,
    0xfe13f14100000000, 0x71fa693500000000, 0x1eb6ccae00000000,
    0xee6452d900000000, 0x8128f74200000000, 0x0c5a7b3c00000000,
    0x6316dea700000000, 0x93c440d000000000, 0xfc88e54b00000000,
    0x73617d3f00000000, 0x1c2dd8a400000000, 0xecff46d300000000,
    0x83b3e34800000000, 0xf22c773a00000000, 0x9d60d2a100000000,
    0x6db24cd600000000, 0x02fee94d00000000, 0x8d17713900000000,
    0xe25bd4a200000000, 0x12894ad500000000, 0x7dc5ef4e00000000,
    0x086c522800000000, 0x6720f7b300000000, 0x97f269c400000000,
    0xf8becc5f00000000, 0x7757542b00000000, 0x181bf1b000000000,
    0xe8c96fc700000000, 0x8785ca5c00000000, 0xf61a5e2e00000000,
    0x9956fbb500000000, 0x698465c200000000, 0x06c8c05900000000,
    0x8921582d00000000, 0xe66dfdb600000000, 0x16bf63c100000000,
    0x79f3c65a00000000, 0xf4814a2400000000, 0x9bcdefbf00000000,
    0x6b1f71c800000000, 0x0453d45300000000, 0x8bba4c2700000000,
    0xe4f6e9bc00000000, 0x142477cb00000000, 0x7b68d25000000000,
    0x0af7462200000000, 0x65bbe3b900000000, 0x95697dce00000000,
    0xfa25d85500000000, 0x75cc402100000000, 0x1a80e5ba00000000,
    0xea527bcd00000000, 0x851ede5600000000, 0xe06fc76000000000,
    0x8f2362fb00000000, 0x7ff1fc8c00000000, 0x10bd591700000000,
    0x9f54c16300000000, 0xf01864f800000000, 0x00cafa8f00000000,
    0x6f865f1400000000, 0x1e19cb6600000000, 0x71556efd00000000,
    0x8187f08a00000000, 0xeecb551100000000, 0x6122cd6500000000,
    0x0e6e68fe00000000, 0xfebcf68900000000, 0x91f0531200000000,
    0x1c82df6c00000000, 0x73ce7af700000000, 0x831ce48000000000,
    0xec50411b00000000, 0x63b9d96f00000000, 0x0cf57cf400000000,
    0xfc27e28300000000, 0x936b471800000000, 0xe2f4d36a00000000,
    0x8db876f100000000, 0x7d6ae88600000000, 0x12264d1d00000000,
    0x9dcfd56900000000, 0xf28370f200000000, 0x0251ee8500000000,
    0x6d1d4b1e00000000, 0x18b4f67800000000, 0x77f853e300000000,
    0x872acd9400000000, 0xe866680f00000000, 0x678ff07b00000000,
    0x08c355e000000000, 0xf811cb9700000000, 0x975d6e0c00000000,
    0xe6c2fa7e00000000, 0x898e5fe500000000, 0x795cc19200000000,
    0x1610640900000000, 0x99f9fc7d00000000, 0xf6b559e600000000,
    0x0667c79100000000, 0x692b620a00000000, 0xe459ee7400000000,
    0x8b154bef00000000, 0x7bc7d59800000000, 0x148b700300000000,
    0x9b62e87700000000, 0xf42e4dec00000000, 0x04fcd39b00000000,
    0x6bb0760000000000, 0x1a2fe27200000000, 0x756347e900000000,
    0x85b1d99e00000000, 0xeafd7c0500000000, 0x6514e47100000000,
    0x0a5841ea00000000, 0xfa8adf9d00000000, 0x95c67a0600000000,
    0x10d8a45000000000, 0x7f9401cb00000000, 0x8f469fbc00000000,
    0xe00a3a2700000000, 0x6fe3a25300000000, 0x00af07c800000000,
    0xf07d99bf00000000, 0x9f313c2400000000, 0xeeaea85600000000,
    0x81e20dcd00000000, 0x713093ba00000000, 0x1e7c362100000000,
    0x9195ae5500000000, 0xfed90bce00000000, 0x0e0b95b900000000,
    0x6147302200000000, 0xec35bc5c00000000, 0x837919c700000000,
    0x73ab87b000000000, 0x1ce7222b00000000, 0x930eba5f00000000,
    0xfc421fc400000000, 0x0c9081b300000000, 0x63dc242800000000,
    0x1243b05a00000000, 0x7d0f15c100000000, 0x8ddd8bb600000000,
    0xe2912e2d00000000, 0x6d78b65900000000, 0x023413c200000000,
    0xf2e68db500000000, 0x9daa282e00000000, 0xe803954800000000,
    0x874f30d300000000, 0x779daea400000000, 0x18d10b3f00000000,
    0x9738934b00000000, 0xf87436d000000000, 0x08a6a8a700000000,
    0x67ea0d3c00000000, 0x1675994e00000000, 0x79393cd500000000,
    0x89eba2a200000000, 0xe6a7073900000000, 0x694e9f4d00000000,
    0x06023ad600000000, 0xf6d0a4a100000000, 0x999c013a00000000,
    0x14ee8d4400000000, 0x7ba228df00000000, 0x8b70b6a800000000,
    0xe43c133300000000, 0x6bd58b4700000000, 0x04992edc00000000,
    0xf44bb0ab00000000, 0x9b07153000000000, 0xea98814200000000,
    0x85d424d900000000, 0x7506baae00000000, 0x1a4a1f3500000000,
    0x95a3874100000000, 0xfaef22da00000000, 0x0a3dbcad00000000,
    0x6571193600000000},
   {0x0000000000000000, 0x85d996dd00000000, 0x4bb55c6000000000,
    0xce6ccabd00000000, 0x966ab9c000000000, 0x13b32f1d00000000,
    0xdddfe5a000000000, 0x5806737d00000000, 0x6dd3035a00000000,
    0xe80a958700000000, 0x26665f3a00000000, 0xa3bfc9e700000000,
    0xfbb9ba9a00000000, 0x7e602c4700000000, 0xb00ce6fa00000000,
    0x35d5702700000000, 0xdaa607b400000000, 0x5f7f916900000000,
    0x91135bd400000000, 0x14cacd0900000000, 0x4cccbe7400000000,
    0xc91528a900000000, 0x0779e21400000000, 0x82a074c900000000,
    0xb77504ee00000000, 0x32ac923300000000, 0xfcc0588e00000000,
    0x7919ce5300000000, 0x211fbd2e00000000, 0xa4c62bf300000000,
    0x6aaae14e00000000, 0xef73779300000000, 0xf54b7eb300000000,
    0x7092e86e00000000, 0xbefe22d300000000, 0x3b27b40e00000000,
    0x6321c77300000000, 0xe6f851ae00000000, 0x28949b1300000000,
    0xad4d0dce00000000, 0x98987de900000000, 0x1d41eb3400000000,
    0xd32d218900000000, 0x56f4b75400000000, 0x0ef2c42900000000,
    0x8b2b52f400000000, 0x4547984900000000, 0xc09e0e9400000000,
    0x2fed790700000000, 0xaa34efda00000000, 0x6458256700000000,
    0xe181b3ba00000000, 0xb987c0c700000000, 0x3c5e561a00000000,
    0xf2329ca700000000, 0x77eb0a7a00000000, 0x423e7a5d00000000,
    0xc7e7ec8000000000, 0x098b263d00000000, 0x8c52b0e000000000,
    0xd454c39d00000000, 0x518d554000000000, 0x9fe19ffd00000000,
    0x1a38092000000000, 0xab918dbd00000000, 0x2e481b6000000000,
    0xe024d1dd00000000, 0x65fd470000000000, 0x3dfb347d00000000,
    0xb822a2a000000000, 0x764e681d00000000, 0xf397fec000000000,
    0xc6428ee700000000, 0x439b183a00000000, 0x8df7d28700000000,
    0x082e445a00000000, 0x5028372700000000, 0xd5f1a1fa00000000,
    0x1b9d6b4700000000, 0x9e44fd9a00000000, 0x71378a0900000000,
    0xf4ee1cd400000000, 0x3a82d66900000000, 0xbf5b40b400000000,
    0xe75d33c900000000, 0x6284a51400000000, 0xace86fa900000000,
    0x2931f97400000000, 0x1ce4895300000000, 0x993d1f8e00000000,
    0x5751d53300000000, 0xd28843ee00000000, 0x8a8e309300000000,
    0x0f57a64e00000000, 0xc13b6cf300000000, 0x44e2fa2e00000000,
    0x5edaf30e00000000, 0xdb0365d300000000, 0x156faf6e00000000,
    0x90b639b300000000, 0xc8b04ace00000000, 0x4d69dc1300000000,
    0x830516ae00000000, 0x06dc807300000000, 0x3309f05400000000,
    0xb6d0668900000000, 0x78bcac3400000000, 0xfd653ae900000000,
    0xa563499400000000, 0x20badf4900000000, 0xeed615f400000000,
    0x6b0f832900000000, 0x847cf4ba00000000, 0x01a5626700000000,
    0xcfc9a8da00000000, 0x4a103e0700000000, 0x12164d7a00000000,
    0x97cfdba700000000, 0x59a3111a00000000, 0xdc7a87c700000000,
    0xe9aff7e000000000, 0x6c76613d00000000, 0xa21aab8000000000,
    0x27c33d5d00000000, 0x7fc54e2000000000, 0xfa1cd8fd00000000,
    0x3470124000000000, 0xb1a9849d00000000, 0x17256aa000000000,
    0x92fcfc7d00000000, 0x5c9036c000000000, 0xd949a01d00000000,
    0x814fd36000000000, 0x049645bd00000000, 0xcafa8f0000000000,
    0x4f2319dd00000000, 0x7af669fa00000000, 0xff2fff2700000000,
    0x3143359a00000000, 0xb49aa34700000000, 0xec9cd03a00000000,
    0x694546e700000000, 0xa7298c5a00000000, 0x22f01a8700000000,
    0xcd836d1400000000, 0x485afbc900000000, 0x8636317400000000,
    0x03efa7a900000000, 0x5be9d4d400000000, 0xde30420900000000,
    0x105c88b400000000, 0x95851e6900000000, 0xa0506e4e00000000,
    0x2589f89300000000, 0xebe5322e00000000, 0x6e3ca4f300000000,
    0x363ad78e00000000, 0xb3e3415300000000, 0x7d8f8bee00000000,
    0xf8561d3300000000, 0xe26e141300000000, 0x67b782ce00000000,
    0xa9db487300000000, 0x2c02deae00000000, 0x7404add300000000,
    0xf1dd3b0e00000000, 0x3fb1f1b300000000, 0xba68676e00000000,
    0x8fbd174900000000, 0x0a64819400000000, 0xc4084b2900000000,
    0x41d1ddf400000000, 0x19d7ae8900000000, 0x9c0e385400000000,
    0x5262f2e900000000, 0xd7bb643400000000, 0x38c813a700000000,
    0xbd11857a00000000, 0x737d4fc700000000, 0xf6a4d91a00000000,
    0xaea2aa6700000000, 0x2b7b3cba00000000, 0xe517f60700000000,
    0x60ce60da00000000, 0x551b10fd00000000, 0xd0c2862000000000,
    0x1eae4c9d00000000, 0x9b77da4000000000, 0xc371a93d00000000,
    0x46a83fe000000000, 0x88c4f55d00000000, 0x0d1d638000000000,
    0xbcb4e71d00000000, 0x396d71c000000000, 0xf701bb7d00000000,
    0x72d82da000000000, 0x2ade5edd00000000, 0xaf07c80000000000,
    0x616b02bd00000000, 0xe4b2946000000000, 0xd167e44700000000,
    0x54be729a00000000, 0x9ad2b82700000000, 0x1f0b2efa00000000,
    0x470d5d8700000000, 0xc2d4cb5a00000000, 0x0cb801e700000000,
    0x8961973a00000000, 0x6612e0a900000000, 0xe3cb767400000000,
    0x2da7bcc900000000, 0xa87e2a1400000000, 0xf078596900000000,
    0x75a1cfb400000000, 0xbbcd050900000000, 0x3e1493d400000000,
    0x0bc1e3f300000000, 0x8e18752e00000000, 0x4074bf9300000000,
    0xc5ad294e00000000, 0x9dab5a3300000000, 0x1872ccee00000000,
    0xd61e065300000000, 0x53c7908e00000000, 0x49ff99ae00000000,
    0xcc260f7300000000, 0x024ac5ce00000000, 0x8793531300000000,
    0xdf95206e00000000, 0x5a4cb6b300000000, 0x94207c0e00000000,
    0x11f9ead300000000, 0x242c9af400000000, 0xa1f50c2900000000,
    0x6f99c69400000000, 0xea40504900000000, 0xb246233400000000,
    0x379fb5e900000000, 0xf9f37f5400000000, 0x7c2ae98900000000,
    0x93599e1a00000000, 0x168008c700000000, 0xd8ecc27a00000000,
    0x5d3554a700000000, 0x053327da00000000, 0x80eab10700000000,
    0x4e867bba00000000, 0xcb5fed6700000000, 0xfe8a9d4000000000,
    0x7b530b9d00000000, 0xb53fc12000000000, 0x30e657fd00000000,
    0x68e0248000000000, 0xed39b25d00000000, 0x235578e000000000,
    0xa68cee3d00000000},
   {0x0000000000000000, 0x76e10f9d00000000, 0xadc46ee100000000,
    0xdb25617c00000000, 0x1b8fac1900000000, 0x6d6ea38400000000,
    0xb64bc2f800000000, 0xc0aacd6500000000, 0x361e593300000000,
    0x40ff56ae00000000, 0x9bda37d200000000, 0xed3b384f00000000,
    0x2d91f52a00000000, 0x5b70fab700000000, 0x80559bcb00000000,
    0xf6b4945600000000, 0x6c3cb26600000000, 0x1addbdfb00000000,
    0xc1f8dc8700000000, 0xb719d31a00000000, 0x77b31e7f00000000,
    0x015211e200000000, 0xda77709e00000000, 0xac967f0300000000,
    0x5a22eb5500000000, 0x2cc3e4c800000000, 0xf7e685b400000000,
    0x81078a2900000000, 0x41ad474c00000000, 0x374c48d100000000,
    0xec6929ad00000000, 0x9a88263000000000, 0xd87864cd00000000,
    0xae996b5000000000, 0x75bc0a2c00000000, 0x035d05b100000000,
    0xc3f7c8d400000000, 0xb516c74900000000, 0x6e33a63500000000,
    0x18d2a9a800000000, 0xee663dfe00000000, 0x9887326300000000,
    0x43a2531f00000000, 0x35435c8200000000, 0xf5e991e700000000,
    0x83089e7a00000000, 0x582dff0600000000, 0x2eccf09b00000000,
    0xb444d6ab00000000, 0xc2a5d93600000000, 0x1980b84a00000000,
    0x6f61b7d700000000, 0xafcb7ab200000000, 0xd92a752f00000000,
    0x020f145300000000, 0x74ee1bce00000000, 0x825a8f9800000000,
    0xf4bb800500000000, 0x2f9ee17900000000, 0x597feee400000000,
    0x99d5238100000000, 0xef342c1c00000000, 0x34114d6000000000,
    0x42f042fd00000000, 0xf1f7b94100000000, 0x8716b6dc00000000,
    0x5c33d7a000000000, 0x2ad2d83d00000000, 0xea78155800000000,
    0x9c991ac500000000, 0x47bc7bb900000000, 0x315d742400000000,
    0xc7e9e07200000000, 0xb108efef00000000, 0x6a2d8e9300000000,
    0x1ccc810e00000000, 0xdc664c6b00000000, 0xaa8743f600000000,
    0x71a2228a00000000, 0x07432d1700000000, 0x9dcb0b2700000000,
    0xeb2a04ba00000000, 0x300f65c600000000, 0x46ee6a5b00000000,
    0x8644a73e00000000, 0xf0a5a8a300000000, 0x2b80c9df00000000,
    0x5d61c64200000000, 0xabd5521400000000, 0xdd345d8900000000,
    0x06113cf500000000, 0x70f0336800000000, 0xb05afe0d00000000,
    0xc6bbf19000000000, 0x1d9e90ec00000000, 0x6b7f9f7100000000,
    0x298fdd8c00000000, 0x5f6ed21100000000, 0x844bb36d00000000,
    0xf2aabcf000000000, 0x3200719500000000, 0x44e17e0800000000,
    0x9fc41f7400000000, 0xe92510e900000000, 0x1f9184bf00000000,
    0x69708b2200000000, 0xb255ea5e00000000, 0xc4b4e5c300000000,
    0x041e28a600000000, 0x72ff273b00000000, 0xa9da464700000000,
    0xdf3b49da00000000, 0x45b36fea00000000, 0x3352607700000000,
    0xe877010b00000000, 0x9e960e9600000000, 0x5e3cc3f300000000,
    0x28ddcc6e00000000, 0xf3f8ad1200000000, 0x8519a28f00000000,
    0x73ad36d900000000, 0x054c394400000000, 0xde69583800000000,
    0xa88857a500000000, 0x68229ac000000000, 0x1ec3955d00000000,
    0xc5e6f42100000000, 0xb307fbbc00000000, 0xe2ef738300000000,
    0x940e7c1e00000000, 0x4f2b1d6200000000, 0x39ca12ff00000000,
    0xf960df9a00000000, 0x8f81d00700000000, 0x54a4b17b00000000,
    0x2245bee600000000, 0xd4f12ab000000000, 0xa210252d00000000,
    0x7935445100000000, 0x0fd44bcc00000000, 0xcf7e86a900000000,
    0xb99f893400000000, 0x62bae84800000000, 0x145be7d500000000,
    0x8ed3c1e500000000, 0xf832ce7800000000, 0x2317af0400000000,
    0x55f6a09900000000, 0x955c6dfc00000000, 0xe3bd626100000000,
    0x3898031d00000000, 0x4e790c8000000000, 0xb8cd98d600000000,
    0xce2c974b00000000, 0x1509f63700000000, 0x63e8f9aa00000000,
    0xa34234cf00000000, 0xd5a33b5200000000, 0x0e865a2e00000000,
    0x786755b300000000, 0x3a97174e00000000, 0x4c7618d300000000,
    0x975379af00000000, 0xe1b2763200000000, 0x2118bb5700000000,
    0x57f9b4ca00000000, 0x8cdcd5b600000000, 0xfa3dda2b00000000,
    0x0c894e7d00000000, 0x7a6841e000000000, 0xa14d209c00000000,
    0xd7ac2f0100000000, 0x1706e26400000000, 0x61e7edf900000000,
    0xbac28c8500000000, 0xcc23831800000000, 0x56aba52800000000,
    0x204aaab500000000, 0xfb6fcbc900000000, 0x8d8ec45400000000,
    0x4d24093100000000, 0x3bc506ac00000000, 0xe0e067d000000000,
    0x9601684d00000000, 0x60b5fc1b00000000, 0x1654f38600000000,
    0xcd7192fa00000000, 0xbb909d6700000000, 0x7b3a500200000000,
    0x0ddb5f9f00000000, 0xd6fe3ee300000000, 0xa01f317e00000000,
    0x1318cac200000000, 0x65f9c55f00000000, 0xbedca42300000000,
    0xc83dabbe00000000, 0x089766db00000000, 0x7e76694600000000,
    0xa553083a00000000, 0xd3b207a700000000, 0x250693f100000000,
    0x53e79c6c00000000, 0x88c2fd1000000000, 0xfe23f28d00000000,
    0x3e893fe800000000, 0x4868307500000000, 0x934d510900000000,
    0xe5ac5e9400000000, 0x7f2478a400000000, 0x09c5773900000000,
    0xd2e0164500000000, 0xa40119d800000000, 0x64abd4bd00000000,
    0x124adb2000000000, 0xc96fba5c00000000, 0xbf8eb5c100000000,
    0x493a219700000000, 0x3fdb2e0a00000000, 0xe4fe4f7600000000,
    0x921f40eb00000000, 0x52b58d8e00000000, 0x2454821300000000,
    0xff71e36f00000000, 0x8990ecf200000000, 0xcb60ae0f00000000,
    0xbd81a19200000000, 0x66a4c0ee00000000, 0x1045cf7300000000,
    0xd0ef021600000000, 0xa60e0d8b00000000, 0x7d2b6cf700000000,
    0x0bca636a00000000, 0xfd7ef73c00000000, 0x8b9ff8a100000000,
    0x50ba99dd00000000, 0x265b964000000000, 0xe6f15b2500000000,
    0x901054b800000000, 0x4b3535c400000000, 0x3dd43a5900000000,
    0xa75c1c6900000000, 0xd1bd13f400000000, 0x0a98728800000000,
    0x7c797d1500000000, 0xbcd3b07000000000, 0xca32bfed00000000,
    0x1117de9100000000, 0x67f6d10c00000000, 0x9142455a00000000,
    0xe7a34ac700000000, 0x3c862bbb00000000, 0x4a67242600000000,
    0x8acde94300000000, 0xfc2ce6de00000000, 0x270987a200000000,
    0x51e8883f00000000},
   {0x0000000000000000, 0xe8dbfbb900000000, 0x91b186a800000000,
    0x796a7d1100000000, 0x63657c8a00000000, 0x8bbe873300000000,
    0xf2d4fa2200000000, 0x1a0f019b00000000, 0x87cc89cf00000000,
    0x6f17727600000000, 0x167d0f6700000000, 0xfea6f4de00000000,
    0xe4a9f54500000000, 0x0c720efc00000000, 0x751873ed00000000,
    0x9dc3885400000000, 0x4f9f624400000000, 0xa74499fd00000000,
    0xde2ee4ec00000000, 0x36f51f5500000000, 0x2cfa1ece00000000,
    0xc421e57700000000, 0xbd4b986600000000, 0x559063df00000000,
    0xc853eb8b00000000, 0x2088103200000000, 0x59e26d2300000000,
    0xb139969a00000000, 0xab36970100000000, 0x43ed6cb800000000,
    0x3a8711a900000000, 0xd25cea1000000000, 0x9e3ec58800000000,
    0x76e53e3100000000, 0x0f8f432000000000, 0xe754b89900000000,
    0xfd5bb90200000000, 0x158042bb00000000, 0x6cea3faa00000000,
    0x8431c41300000000, 0x19f24c4700000000, 0xf129b7fe00000000,
    0x8843caef00000000, 0x6098315600000000, 0x7a9730cd00000000,
    0x924ccb7400000000, 0xeb26b66500000000, 0x03fd4ddc00000000,
    0xd1a1a7cc00000000, 0x397a5c7500000000, 0x4010216400000000,
    0xa8cbdadd00000000, 0xb2c4db4600000000, 0x5a1f20ff00000000,
    0x23755dee00000000, 0xcbaea65700000000, 0x566d2e0300000000,
    0xbeb6d5ba00000000, 0xc7dca8ab00000000, 0x2f07531200000000,
    0x3508528900000000, 0xddd3a93000000000, 0xa4b9d42100000000,
    0x4c622f9800000000, 0x7d7bfbca00000000, 0x95a0007300000000,
    0xecca7d6200000000, 0x041186db00000000, 0x1e1e874000000000,
    0xf6c57cf900000000, 0x8faf01e800000000, 0x6774fa5100000000,
    0xfab7720500000000, 0x126c89bc00000000, 0x6b06f4ad00000000,
    0x83dd0f1400000000, 0x99d20e8f00000000, 0x7109f53600000000,
    0x0863882700000000, 0xe0b8739e00000000, 0x32e4998e00000000,
    0xda3f623700000000, 0xa3551f2600000000, 0x4b8ee49f00000000,
    0x5181e50400000000, 0xb95a1ebd00000000, 0xc03063ac00000000,
    0x28eb981500000000, 0xb528104100000000, 0x5df3ebf800000000,
    0x249996e900000000, 0xcc426d5000000000, 0xd64d6ccb00000000,
    0x3e96977200000000, 0x47fcea6300000000, 0xaf2711da00000000,
    0xe3453e4200000000, 0x0b9ec5fb00000000, 0x72f4b8ea00000000,
    0x9a2f435300000000, 0x802042c800000000, 0x68fbb97100000000,
    0x1191c46000000000, 0xf94a3fd900000000, 0x6489b78d00000000,
    0x8c524c3400000000, 0xf538312500000000, 0x1de3ca9c00000000,
    0x07eccb0700000000, 0xef3730be00000000, 0x965d4daf00000000,
    0x7e86b61600000000, 0xacda5c0600000000, 0x4401a7bf00000000,
    0x3d6bdaae00000000, 0xd5b0211700000000, 0xcfbf208c00000000,
    0x2764db3500000000, 0x5e0ea62400000000, 0xb6d55d9d00000000,
    0x2b16d5c900000000, 0xc3cd2e7000000000, 0xbaa7536100000000,
    0x527ca8d800000000, 0x4873a94300000000, 0xa0a852fa00000000,
    0xd9c22feb00000000, 0x3119d45200000000, 0xbbf0874e00000000,
    0x532b7cf700000000, 0x2a4101e600000000, 0xc29afa5f00000000,
    0xd895fbc400000000, 0x304e007d00000000, 0x49247d6c00000000,
    0xa1ff86d500000000, 0x3c3c0e8100000000, 0xd4e7f53800000000,
    0xad8d882900000000, 0x4556739000000000, 0x5f59720b00000000,
    0xb78289b200000000, 0xcee8f4a300000000, 0x26330f1a00000000,
    0xf46fe50a00000000, 0x1cb41eb300000000, 0x65de63a200000000,
    0x8d05981b00000000, 0x970a998000000000, 0x7fd1623900000000,
    0x06bb1f2800000000, 0xee60e49100000000, 0x73a36cc500000000,
    0x9b78977c00000000, 0xe212ea6d00000000, 0x0ac911d400000000,
    0x10c6104f00000000, 0xf81debf600000000, 0x817796e700000000,
    0x69ac6d5e00000000, 0x25ce42c600000000, 0xcd15b97f00000000,
    0xb47fc46e00000000, 0x5ca43fd700000000, 0x46ab3e4c00000000,
    0xae70c5f500000000, 0xd71ab8e400000000, 0x3fc1435d00000000,
    0xa202cb0900000000, 0x4ad930b000000000, 0x33b34da100000000,
    0xdb68b61800000000, 0xc167b78300000000, 0x29bc4c3a00000000,
    0x50d6312b00000000, 0xb80dca9200000000, 0x6a51208200000000,
    0x828adb3b00000000, 0xfbe0a62a00000000, 0x133b5d9300000000,
    0x09345c0800000000, 0xe1efa7b100000000, 0x9885daa000000000,
    0x705e211900000000, 0xed9da94d00000000, 0x054652f400000000,
    0x7c2c2fe500000000, 0x94f7d45c00000000, 0x8ef8d5c700000000,
    0x66232e7e00000000, 0x1f49536f00000000, 0xf792a8d600000000,
    0xc68b7c8400000000, 0x2e50873d00000000, 0x573afa2c00000000,
    0xbfe1019500000000, 0xa5ee000e00000000, 0x4d35fbb700000000,
    0x345f86a600000000, 0xdc847d1f00000000, 0x4147f54b00000000,
    0xa99c0ef200000000, 0xd0f673e300000000, 0x382d885a00000000,
    0x222289c100000000, 0xcaf9727800000000, 0xb3930f6900000000,
    0x5b48f4d000000000, 0x89141ec000000000, 0x61cfe57900000000,
    0x18a5986800000000, 0xf07e63d100000000, 0xea71624a00000000,
    0x02aa99f300000000, 0x7bc0e4e200000000, 0x931b1f5b00000000,
    0x0ed8970f00000000, 0xe6036cb600000000, 0x9f6911a700000000,
    0x77b2ea1e00000000, 0x6dbdeb8500000000, 0x8566103c00000000,
    0xfc0c6d2d00000000, 0x14d7969400000000, 0x58b5b90c00000000,
    0xb06e42b500000000, 0xc9043fa400000000, 0x21dfc41d00000000,
    0x3bd0c58600000000, 0xd30b3e3f00000000, 0xaa61432e00000000,
    0x42bab89700000000, 0xdf7930c300000000, 0x37a2cb7a00000000,
    0x4ec8b66b00000000, 0xa6134dd200000000, 0xbc1c4c4900000000,
    0x54c7b7f000000000, 0x2dadcae100000000, 0xc576315800000000,
    0x172adb4800000000, 0xfff120f100000000, 0x869b5de000000000,
    0x6e40a65900000000, 0x744fa7c200000000, 0x9c945c7b00000000,
    0xe5fe216a00000000, 0x0d25dad300000000, 0x90e6528700000000,
    0x783da93e00000000, 0x0157d42f00000000, 0xe98c2f9600000000,
    0xf3832e0d00000000, 0x1b58d5b400000000, 0x6232a8a500000000,
    0x8ae9531c00000000},
   {0x0000000000000000, 0x919168ae00000000, 0x6325a08700000000,
    0xf2b4c82900000000, 0x874c31d400000000, 0x16dd597a00000000,
    0xe469915300000000, 0x75f8f9fd00000000, 0x4f9f137300000000,
    0xde0e7bdd00000000, 0x2cbab3f400000000, 0xbd2bdb5a00000000,
    0xc8d322a700000000, 0x59424a0900000000, 0xabf6822000000000,
    0x3a67ea8e00000000, 0x9e3e27e600000000, 0x0faf4f4800000000,
    0xfd1b876100000000, 0x6c8aefcf00000000, 0x1972163200000000,
    0x88e37e9c00000000, 0x7a57b6b500000000, 0xebc6de1b00000000,
    0xd1a1349500000000, 0x40305c3b00000000, 0xb284941200000000,
    0x2315fcbc00000000, 0x56ed054100000000, 0xc77c6def00000000,
    0x35c8a5c600000000, 0xa459cd6800000000, 0x7d7b3f1700000000,
    0xecea57b900000000, 0x1e5e9f9000000000, 0x8fcff73e00000000,
    0xfa370ec300000000, 0x6ba6666d00000000, 0x9912ae4400000000,
    0x0883c6ea00000000, 0x32e42c6400000000, 0xa37544ca00000000,
    0x51c18ce300000000, 0xc050e44d00000000, 0xb5a81db000000000,
    0x2439751e00000000, 0xd68dbd3700000000, 0x471cd59900000000,
    0xe34518f100000000, 0x72d4705f00000000, 0x8060b87600000000,
    0x11f1d0d800000000, 0x6409292500000000, 0xf598418b00000000,
    0x072c89a200000000, 0x96bde10c00000000, 0xacda0b8200000000,
    0x3d4b632c00000000, 0xcfffab0500000000, 0x5e6ec3ab00000000,
    0x2b963a5600000000, 0xba0752f800000000, 0x48b39ad100000000,
    0xd922f27f00000000, 0xfaf67e2e00000000, 0x6b67168000000000,
    0x99d3dea900000000, 0x0842b60700000000, 0x7dba4ffa00000000,
    0xec2b275400000000, 0x1e9fef7d00000000, 0x8f0e87d300000000,
    0xb5696d5d00000000, 0x24f805f300000000, 0xd64ccdda00000000,
    0x47dda57400000000, 0x32255c8900000000, 0xa3b4342700000000,
    0x5100fc0e00000000, 0xc09194a000000000, 0x64c859c800000000,
    0xf559316600000000, 0x07edf94f00000000, 0x967c91e100000000,
    0xe384681c00000000, 0x721500b200000000, 0x80a1c89b00000000,
    0x1130a03500000000, 0x2b574abb00000000, 0xbac6221500000000,
    0x4872ea3c00000000, 0xd9e3829200000000, 0xac1b7b6f00000000,
    0x3d8a13c100000000, 0xcf3edbe800000000, 0x5eafb34600000000,
    0x878d413900000000, 0x161c299700000000, 0xe4a8e1be00000000,
    0x7539891000000000, 0x00c170ed00000000, 0x9150184300000000,
    0x63e4d06a00000000, 0xf275b8c400000000, 0xc812524a00000000,
    0x59833ae400000000, 0xab37f2cd00000000, 0x3aa69a6300000000,
    0x4f5e639e00000000, 0xdecf0b3000000000, 0x2c7bc31900000000,
    0xbdeaabb700000000, 0x19b366df00000000, 0x88220e7100000000,
    0x7a96c65800000000, 0xeb07aef600000000, 0x9eff570b00000000,
    0x0f6e3fa500000000, 0xfddaf78c00000000, 0x6c4b9f2200000000,
    0x562c75ac00000000, 0xc7bd1d0200000000, 0x3509d52b00000000,
    0xa498bd8500000000, 0xd160447800000000, 0x40f12cd600000000,
    0xb245e4ff00000000, 0x23d48c5100000000, 0xf4edfd5c00000000,
    0x657c95f200000000, 0x97c85ddb00000000, 0x0659357500000000,
    0x73a1cc8800000000, 0xe230a42600000000, 0x10846c0f00000000,
    0x811504a100000000, 0xbb72ee2f00000000, 0x2ae3868100000000,
    0xd8574ea800000000, 0x49c6260600000000, 0x3c3edffb00000000,
    0xadafb75500000000, 0x5f1b7f7c00000000, 0xce8a17d200000000,
    0x6ad3daba00000000, 0xfb42b21400000000, 0x09f67a3d00000000,
    0x9867129300000000, 0xed9feb6e00000000, 0x7c0e83c000000000,
    0x8eba4be900000000, 0x1f2b234700000000, 0x254cc9c900000000,
    0xb4dda16700000000, 0x4669694e00000000, 0xd7f801e000000000,
    0xa200f81d00000000, 0x339190b300000000, 0xc125589a00000000,
    0x50b4303400000000, 0x8996c24b00000000, 0x1807aae500000000,
    0xeab362cc00000000, 0x7b220a6200000000, 0x0edaf39f00000000,
    0x9f4b9b3100000000, 0x6dff531800000000, 0xfc6e3bb600000000,
    0xc609d13800000000, 0x5798b99600000000, 0xa52c71bf00000000,
    0x34bd191100000000, 0x4145e0ec00000000, 0xd0d4884200000000,
    0x2260406b00000000, 0xb3f128c500000000, 0x17a8e5ad00000000,
    0x86398d0300000000, 0x748d452a00000000, 0xe51c2d8400000000,
    0x90e4d47900000000, 0x0175bcd700000000, 0xf3c174fe00000000,
    0x62501c5000000000, 0x5837f6de00000000, 0xc9a69e7000000000,
    0x3b12565900000000, 0xaa833ef700000000, 0xdf7bc70a00000000,
    0x4eeaafa400000000, 0xbc5e678d00000000, 0x2dcf0f2300000000,
    0x0e1b837200000000, 0x9f8aebdc00000000, 0x6d3e23f500000000,
    0xfcaf4b5b00000000, 0x8957b2a600000000, 0x18c6da0800000000,
    0xea72122100000000, 0x7be37a8f00000000, 0x4184900100000000,
    0xd015f8af00000000, 0x22a1308600000000, 0xb330582800000000,
    0xc6c8a1d500000000, 0x5759c97b00000000, 0xa5ed015200000000,
    0x347c69fc00000000, 0x9025a49400000000, 0x01b4cc3a00000000,
    0xf300041300000000, 0x62916cbd00000000, 0x1769954000000000,
    0x86f8fdee00000000, 0x744c35c700000000, 0xe5dd5d6900000000,
    0xdfbab7e700000000, 0x4e2bdf4900000000, 0xbc9f176000000000,
    0x2d0e7fce00000000, 0x58f6863300000000, 0xc967ee9d00000000,
    0x3bd326b400000000, 0xaa424e1a00000000, 0x7360bc6500000000,
    0xe2f1d4cb00000000, 0x10451ce200000000, 0x81d4744c00000000,
    0xf42c8db100000000, 0x65bde51f00000000, 0x97092d3600000000,
    0x0698459800000000, 0x3cffaf1600000000, 0xad6ec7b800000000,
    0x5fda0f9100000000, 0xce4b673f00000000, 0xbbb39ec200000000,
    0x2a22f66c00000000, 0xd8963e4500000000, 0x490756eb00000000,
    0xed5e9b8300000000, 0x7ccff32d00000000, 0x8e7b3b0400000000,
    0x1fea53aa00000000, 0x6a12aa5700000000, 0xfb83c2f900000000,
    0x09370ad000000000, 0x98a6627e00000000, 0xa2c188f000000000,
    0x3350e05e00000000, 0xc1e4287700000000, 0x507540d900000000,
    0x258db92400000000, 0xb41cd18a00000000, 0x46a819a300000000,
    0xd739710d00000000}};

#else /* W == 4 */

local const z_crc_t FAR crc_braid_table[][256] = {
   {0x00000000, 0xccaa009e, 0x4225077d, 0x8e8f07e3, 0x844a0efa,
    0x48e00e64, 0xc66f0987, 0x0ac50919, 0xd3e51bb5, 0x1f4f1b2b,
    0x91c01cc8, 0x5d6a1c56, 0x57af154f, 0x9b0515d1, 0x158a1232,
    0xd92012ac, 0x7cbb312b, 0xb01131b5, 0x3e9e3656, 0xf23436c8,
    0xf8f13fd1, 0x345b3f4f, 0xbad438ac, 0x767e3832, 0xaf5e2a9e,
    0x63f42a00, 0xed7b2de3, 0x21d12d7d, 0x2b142464, 0xe7be24fa,
    0x69312319, 0xa59b2387, 0xf9766256, 0x35dc62c8, 0xbb53652b,
    0x77f965b5, 0x7d3c6cac, 0xb1966c32, 0x3f196bd1, 0xf3b36b4f,
    0x2a9379e3, 0xe639797d, 0x68b67e9e, 0xa41c7e00, 0xaed97719,
    0x62737787, 0xecfc7064, 0x205670fa, 0x85cd537d, 0x496753e3,
    0xc7e85400, 0x0b42549e, 0x01875d87, 0xcd2d5d19, 0x43a25afa,
    0x8f085a64, 0x562848c8, 0x9a824856, 0x140d4fb5, 0xd8a74f2b,
    0xd2624632, 0x1ec846ac, 0x9047414f, 0x5ced41d1, 0x299dc2ed,
    0xe537c273, 0x6bb8c590, 0xa712c50e, 0xadd7cc17, 0x617dcc89,
    0xeff2cb6a, 0x2358cbf4, 0xfa78d958, 0x36d2d9c6, 0xb85dde25,
    0x74f7debb, 0x7e32d7a2, 0xb298d73c, 0x3c17d0df, 0xf0bdd041,
    0x5526f3c6, 0x998cf358, 0x1703f4bb, 0xdba9f425, 0xd16cfd3c,
    0x1dc6fda2, 0x9349fa41, 0x5fe3fadf, 0x86c3e873, 0x4a69e8ed,
    0xc4e6ef0e, 0x084cef90, 0x0289e689, 0xce23e617, 0x40ace1f4,
    0x8c06e16a, 0xd0eba0bb, 0x1c41a025, 0x92cea7c6, 0x5e64a758,
    0x54a1ae41, 0x980baedf, 0x1684a93c, 0xda2ea9a2, 0x030ebb0e,
    0xcfa4bb90, 0x412bbc73, 0x8d81bced, 0x8744b5f4, 0x4beeb56a,
    0xc561b289, 0x09cbb217, 0xac509190, 0x60fa910e, 0xee7596ed,
    0x22df9673, 0x281a9f6a, 0xe4b09ff4, 0x6a3f9817, 0xa6959889,
    0x7fb58a25, 0xb31f8abb, 0x3d908d58, 0xf13a8dc6, 0xfbff84df,
    0x37558441, 0xb9da83a2, 0x7570833c, 0x533b85da, 0x9f918544,
    0x111e82a7, 0xddb48239, 0xd7718b20, 0x1bdb8bbe, 0x95548c5d,
    0x59fe8cc3, 0x80de9e6f, 0x4c749ef1, 0xc2fb9912, 0x0e51998c,
    0x04949095, 0xc83e900b, 0x46b197e8, 0x8a1b9776, 0x2f80b4f1,
    0xe32ab46f, 0x6da5b38c, 0xa10fb312, 0xabcaba0b, 0x6760ba95,
    0xe9efbd76, 0x2545bde8, 0xfc65af44, 0x30cfafda, 0xbe40a839,
    0x72eaa8a7, 0x782fa1be, 0xb485a120, 0x3a0aa6c3, 0xf6a0a65d,
    0xaa4de78c, 0x66e7e712, 0xe868e0f1, 0x24c2e06f, 0x2e07e976,
    0xe2ade9e8, 0x6c22ee0b, 0xa088ee95, 0x79a8fc39, 0xb502fca7,
    0x3b8dfb44, 0xf727fbda, 0xfde2f2c3, 0x3148f25d, 0xbfc7f5be,
    0x736df520, 0xd6f6d6a7, 0x1a5cd639, 0x94d3d1da, 0x5879d144,
    0x52bcd85d, 0x9e16d8c3, 0x1099df20, 0xdc33dfbe, 0x0513cd12,
    0xc9b9cd8c, 0x4736ca6f, 0x8b9ccaf1, 0x8159c3e8, 0x4df3c376,
    0xc37cc495, 0x0fd6c40b, 0x7aa64737, 0xb60c47a9, 0x3883404a,
    0xf42940d4, 0xfeec49cd, 0x32464953, 0xbcc94eb0, 0x70634e2e,
    0xa9435c82, 0x65e95c1c, 0xeb665bff, 0x27cc5b61, 0x2d095278,
    0xe1a352e6, 0x6f2c5505, 0xa386559b, 0x061d761c, 0xcab77682,
    0x44387161, 0x889271ff, 0x825778e6, 0x4efd7878, 0xc0727f9b,
    0x0cd87f05, 0xd5f86da9, 0x19526d37, 0x97dd6ad4, 0x5b776a4a,
    0x51b26353, 0x9d1863cd, 0x1397642e, 0xdf3d64b0, 0x83d02561,
    0x4f7a25ff, 0xc1f5221c, 0x0d5f2282, 0x079a2b9b, 0xcb302b05,
    0x45bf2ce6, 0x89152c78, 0x50353ed4, 0x9c9f3e4a, 0x121039a9,
    0xdeba3937, 0xd47f302e, 0x18d530b0, 0x965a3753, 0x5af037cd,
    0xff6b144a, 0x33c114d4, 0xbd4e1337, 0x71e413a9, 0x7b211ab0,
    0xb78b1a2e, 0x39041dcd, 0xf5ae1d53, 0x2c8e0fff, 0xe0240f61,
    0x6eab0882, 0xa201081c, 0xa8c40105, 0x646e019b, 0xeae10678,
    0x264b06e6},
   {0x00000000, 0xa6770bb4, 0x979f1129, 0x31e81a9d, 0xf44f2413,
    0x52382fa7, 0x63d0353a, 0xc5a73e8e, 0x33ef4e67, 0x959845d3,
    0xa4705f4e, 0x020754fa, 0xc7a06a74, 0x61d761c0, 0x503f7b5d,
    0xf64870e9, 0x67de9cce, 0xc1a9977a, 0xf0418de7, 0x56368653,
    0x9391b8dd, 0x35e6b369, 0x040ea9f4, 0xa279a240, 0x5431d2a9,
    0xf246d91d, 0xc3aec380, 0x65d9c834, 0xa07ef6ba, 0x0609fd0e,
    0x37e1e793, 0x9196ec27, 0xcfbd399c, 0x69ca3228, 0x582228b5,
    0xfe552301, 0x3bf21d8f, 0x9d85163b, 0xac6d0ca6, 0x0a1a0712,
    0xfc5277fb, 0x5a257c4f, 0x6bcd66d2, 0xcdba6d66, 0x081d53e8,
    0xae6a585c, 0x9f8242c1, 0x39f54975, 0xa863a552, 0x0e14aee6,
    0x3ffcb47b, 0x998bbfcf, 0x5c2c8141, 0xfa5b8af5, 0xcbb39068,
    0x6dc49bdc, 0x9b8ceb35, 0x3dfbe081, 0x0c13fa1c, 0xaa64f1a8,
    0x6fc3cf26, 0xc9b4c492, 0xf85cde0f, 0x5e2bd5bb, 0x440b7579,
    0xe27c7ecd, 0xd3946450, 0x75e36fe4, 0xb044516a, 0x16335ade,
    0x27db4043, 0x81ac4bf7, 0x77e43b1e, 0xd19330aa, 0xe07b2a37,
    0x460c2183, 0x83ab1f0d, 0x25dc14b9, 0x14340e24, 0xb2430590,
    0x23d5e9b7, 0x85a2e203, 0xb44af89e, 0x123df32a, 0xd79acda4,
    0x71edc610, 0x4005dc8d, 0xe672d739, 0x103aa7d0, 0xb64dac64,
    0x87a5b6f9, 0x21d2bd4d, 0xe47583c3, 0x42028877, 0x73ea92ea,
    0xd59d995e, 0x8bb64ce5, 0x2dc14751, 0x1c295dcc, 0xba5e5678,
    0x7ff968f6, 0xd98e6342, 0xe86679df, 0x4e11726b, 0xb8590282,
    0x1e2e0936, 0x2fc613ab, 0x89b1181f, 0x4c162691, 0xea612d25,
    0xdb8937b8, 0x7dfe3c0c, 0xec68d02b, 0x4a1fdb9f, 0x7bf7c102,
    0xdd80cab6, 0x1827f438, 0xbe50ff8c, 0x8fb8e511, 0x29cfeea5,
    0xdf879e4c, 0x79f095f8, 0x48188f65, 0xee6f84d1, 0x2bc8ba5f,
    0x8dbfb1eb, 0xbc57ab76, 0x1a20a0c2, 0x8816eaf2, 0x2e61e146,
    0x1f89fbdb, 0xb9fef06f, 0x7c59cee1, 0xda2ec555, 0xebc6dfc8,
    0x4db1d47c, 0xbbf9a495, 0x1d8eaf21, 0x2c66b5bc, 0x8a11be08,
    0x4fb68086, 0xe9c18b32, 0xd82991af, 0x7e5e9a1b, 0xefc8763c,
    0x49bf7d88, 0x78576715, 0xde206ca1, 0x1b87522f, 0xbdf0599b,
    0x8c184306, 0x2a6f48b2, 0xdc27385b, 0x7a5033ef, 0x4bb82972,
    0xedcf22c6, 0x28681c48, 0x8e1f17fc, 0xbff70d61, 0x198006d5,
    0x47abd36e, 0xe1dcd8da, 0xd034c247, 0x7643c9f3, 0xb3e4f77d,
    0x1593fcc9, 0x247be654, 0x820cede0, 0x74449d09, 0xd23396bd,
    0xe3db8c20, 0x45ac8794, 0x800bb91a, 0x267cb2ae, 0x1794a833,
    0xb1e3a387, 0x20754fa0, 0x86024414, 0xb7ea5e89, 0x119d553d,
    0xd43a6bb3, 0x724d6007, 0x43a57a9a, 0xe5d2712e, 0x139a01c7,
    0xb5ed0a73, 0x840510ee, 0x22721b5a, 0xe7d525d4, 0x41a22e60,
    0x704a34fd, 0xd63d3f49, 0xcc1d9f8b, 0x6a6a943f, 0x5b828ea2,
    0xfdf58516, 0x3852bb98, 0x9e25b02c, 0xafcdaab1, 0x09baa105,
    0xfff2d1ec, 0x5985da58, 0x686dc0c5, 0xce1acb71, 0x0bbdf5ff,
    0xadcafe4b, 0x9c22e4d6, 0x3a55ef62, 0xabc30345, 0x0db408f1,
    0x3c5c126c, 0x9a2b19d8, 0x5f8c2756, 0xf9fb2ce2, 0xc813367f,
    0x6e643dcb, 0x982c4d22, 0x3e5b4696, 0x0fb35c0b, 0xa9c457bf,
    0x6c636931, 0xca146285, 0xfbfc7818, 0x5d8b73ac, 0x03a0a617,
    0xa5d7ada3, 0x943fb73e, 0x3248bc8a, 0xf7ef8204, 0x519889b0,
    0x6070932d, 0xc6079899, 0x304fe870, 0x9638e3c4, 0xa7d0f959,
    0x01a7f2ed, 0xc400cc63, 0x6277c7d7, 0x539fdd4a, 0xf5e8d6fe,
    0x647e3ad9, 0xc209316d, 0xf3e12bf0, 0x55962044, 0x90311eca,
    0x3646157e, 0x07ae0fe3, 0xa1d90457, 0x579174be, 0xf1e67f0a,
    0xc00e6597, 0x66796e23, 0xa3de50ad, 0x05a95b19, 0x34414184,
    0x92364a30},
   {0x00000000, 0xcb5cd3a5, 0x4dc8a10b, 0x869472ae, 0x9b914216,
    0x50cd91b3, 0xd659e31d, 0x1d0530b8, 0xec53826d, 0x270f51c8,
    0xa19b2366, 0x6ac7f0c3, 0x77c2c07b, 0xbc9e13de, 0x3a0a6170,
    0xf156b2d5, 0x03d6029b, 0xc88ad13e, 0x4e1ea390, 0x85427035,
    0x9847408d, 0x531b9328, 0xd58fe186, 0x1ed33223, 0xef8580f6,
    0x24d95353, 0xa24d21fd, 0x6911f258, 0x7414c2e0, 0xbf481145,
    0x39dc63eb, 0xf280b04e, 0x07ac0536, 0xccf0d693, 0x4a64a43d,
    0x81387798, 0x9c3d4720, 0x57619485, 0xd1f5e62b, 0x1aa9358e,
    0xebff875b, 0x20a354fe, 0xa6372650, 0x6d6bf5f5, 0x706ec54d,
    0xbb3216e8, 0x3da66446, 0xf6fab7e3, 0x047a07ad, 0xcf26d408,
    0x49b2a6a6, 0x82ee7503, 0x9feb45bb, 0x54b7961e, 0xd223e4b0,
    0x197f3715, 0xe82985c0, 0x23755665, 0xa5e124cb, 0x6ebdf76e,
    0x73b8c7d6, 0xb8e41473, 0x3e7066dd, 0xf52cb578, 0x0f580a6c,
    0xc404d9c9, 0x4290ab67, 0x89cc78c2, 0x94c9487a, 0x5f959bdf,
    0xd901e971, 0x125d3ad4, 0xe30b8801, 0x28575ba4, 0xaec3290a,
    0x659ffaaf, 0x789aca17, 0xb3c619b2, 0x35526b1c, 0xfe0eb8b9,
    0x0c8e08f7, 0xc7d2db52, 0x4146a9fc, 0x8a1a7a59, 0x971f4ae1,
    0x5c439944, 0xdad7ebea, 0x118b384f, 0xe0dd8a9a, 0x2b81593f,
    0xad152b91, 0x6649f834, 0x7b4cc88c, 0xb0101b29, 0x36846987,
    0xfdd8ba22, 0x08f40f5a, 0xc3a8dcff, 0x453cae51, 0x8e607df4,
    0x93654d4c, 0x58399ee9, 0xdeadec47, 0x15f13fe2, 0xe4a78d37,
    0x2ffb5e92, 0xa96f2c3c, 0x6233ff99, 0x7f36cf21, 0xb46a1c84,
    0x32fe6e2a, 0xf9a2bd8f, 0x0b220dc1, 0xc07ede64, 0x46eaacca,
    0x8db67f6f, 0x90b34fd7, 0x5bef9c72, 0xdd7beedc, 0x16273d79,
    0xe7718fac, 0x2c2d5c09, 0xaab92ea7, 0x61e5fd02, 0x7ce0cdba,
    0xb7bc1e1f, 0x31286cb1, 0xfa74bf14, 0x1eb014d8, 0xd5ecc77d,
    0x5378b5d3, 0x98246676, 0x852156ce, 0x4e7d856b, 0xc8e9f7c5,
    0x03b52460, 0xf2e396b5, 0x39bf4510, 0xbf2b37be, 0x7477e41b,
    0x6972d4a3, 0xa22e0706, 0x24ba75a8, 0xefe6a60d, 0x1d661643,
    0xd63ac5e6, 0x50aeb748, 0x9bf264ed, 0x86f75455, 0x4dab87f0,
    0xcb3ff55e, 0x006326fb, 0xf135942e, 0x3a69478b, 0xbcfd3525,
    0x77a1e680, 0x6aa4d638, 0xa1f8059d, 0x276c7733, 0xec30a496,
    0x191c11ee, 0xd240c24b, 0x54d4b0e5, 0x9f886340, 0x828d53f8,
    0x49d1805d, 0xcf45f2f3, 0x04192156, 0xf54f9383, 0x3e134026,
    0xb8873288, 0x73dbe12d, 0x6eded195, 0xa5820230, 0x2316709e,
    0xe84aa33b, 0x1aca1375, 0xd196c0d0, 0x5702b27e, 0x9c5e61db,
    0x815b5163, 0x4a0782c6, 0xcc93f068, 0x07cf23cd, 0xf6999118,
    0x3dc542bd, 0xbb513013, 0x700de3b6, 0x6d08d30e, 0xa65400ab,
    0x20c07205, 0xeb9ca1a0, 0x11e81eb4, 0xdab4cd11, 0x5c20bfbf,
    0x977c6c1a, 0x8a795ca2, 0x41258f07, 0xc7b1fda9, 0x0ced2e0c,
    0xfdbb9cd9, 0x36e74f7c, 0xb0733dd2, 0x7b2fee77, 0x662adecf,
    0xad760d6a, 0x2be27fc4, 0xe0beac61, 0x123e1c2f, 0xd962cf8a,
    0x5ff6bd24, 0x94aa6e81, 0x89af5e39, 0x42f38d9c, 0xc467ff32,
    0x0f3b2c97, 0xfe6d9e42, 0x35314de7, 0xb3a53f49, 0x78f9ecec,
    0x65fcdc54, 0xaea00ff1, 0x28347d5f, 0xe368aefa, 0x16441b82,
    0xdd18c827, 0x5b8cba89, 0x90d0692c, 0x8dd55994, 0x46898a31,
    0xc01df89f, 0x0b412b3a, 0xfa1799ef, 0x314b4a4a, 0xb7df38e4,
    0x7c83eb41, 0x6186dbf9, 0xaada085c, 0x2c4e7af2, 0xe712a957,
    0x15921919, 0xdececabc, 0x585ab812, 0x93066bb7, 0x8e035b0f,
    0x455f88aa, 0xc3cbfa04, 0x089729a1, 0xf9c19b74, 0x329d48d1,
    0xb4093a7f, 0x7f55e9da, 0x6250d962, 0xa90c0ac7, 0x2f987869,
    0xe4c4abcc},
   {0x00000000, 0x3d6029b0, 0x7ac05360, 0x47a07ad0, 0xf580a6c0,
    0xc8e08f70, 0x8f40f5a0, 0xb220dc10, 0x30704bc1, 0x0d106271,
    0x4ab018a1, 0x77d03111, 0xc5f0ed01, 0xf890c4b1, 0xbf30be61,
    0x825097d1, 0x60e09782, 0x5d80be32, 0x1a20c4e2, 0x2740ed52,
    0x95603142, 0xa80018f2, 0xefa06222, 0xd2c04b92, 0x5090dc43,
    0x6df0f5f3, 0x2a508f23, 0x1730a693, 0xa5107a83, 0x98705333,
    0xdfd029e3, 0xe2b00053, 0xc1c12f04, 0xfca106b4, 0xbb017c64,
    0x866155d4, 0x344189c4, 0x0921a074, 0x4e81daa4, 0x73e1f314,
    0xf1b164c5, 0xccd14d75, 0x8b7137a5, 0xb6111e15, 0x0431c205,
    0x3951ebb5, 0x7ef19165, 0x4391b8d5, 0xa121b886, 0x9c419136,
    0xdbe1ebe6, 0xe681c256, 0x54a11e46, 0x69c137f6, 0x2e614d26,
    0x13016496, 0x9151f347, 0xac31daf7, 0xeb91a027, 0xd6f18997,
    0x64d15587, 0x59b17c37, 0x1e1106e7, 0x23712f57, 0x58f35849,
    0x659371f9, 0x22330b29, 0x1f532299, 0xad73fe89, 0x9013d739,
    0xd7b3ade9, 0xead38459, 0x68831388, 0x55e33a38, 0x124340e8,
    0x2f236958, 0x9d03b548, 0xa0639cf8, 0xe7c3e628, 0xdaa3cf98,
    0x3813cfcb, 0x0573e67b, 0x42d39cab, 0x7fb3b51b, 0xcd93690b,
    0xf0f340bb, 0xb7533a6b, 0x8a3313db, 0x0863840a, 0x3503adba,
    0x72a3d76a, 0x4fc3feda, 0xfde322ca, 0xc0830b7a, 0x872371aa,
    0xba43581a, 0x9932774d, 0xa4525efd, 0xe3f2242d, 0xde920d9d,
    0x6cb2d18d, 0x51d2f83d, 0x167282ed, 0x2b12ab5d, 0xa9423c8c,
    0x9422153c, 0xd3826fec, 0xeee2465c, 0x5cc29a4c, 0x61a2b3fc,
    0x2602c92c, 0x1b62e09c, 0xf9d2e0cf, 0xc4b2c97f, 0x8312b3af,
    0xbe729a1f, 0x0c52460f, 0x31326fbf, 0x7692156f, 0x4bf23cdf,
    0xc9a2ab0e, 0xf4c282be, 0xb362f86e, 0x8e02d1de, 0x3c220dce,
    0x0142247e, 0x46e25eae, 0x7b82771e, 0xb1e6b092, 0x8c869922,
    0xcb26e3f2, 0xf646ca42, 0x44661652, 0x79063fe2, 0x3ea64532,
    0x03c66c82, 0x8196fb53, 0xbcf6d2e3, 0xfb56a833, 0xc6368183,
    0x74165d93, 0x49767423, 0x0ed60ef3, 0x33b62743, 0xd1062710,
    0xec660ea0, 0xabc67470, 0x96a65dc0, 0x248681d0, 0x19e6a860,
    0x5e46d2b0, 0x6326fb00, 0xe1766cd1, 0xdc164561, 0x9bb63fb1,
    0xa6d61601, 0x14f6ca11, 0x2996e3a1, 0x6e369971, 0x5356b0c1,
    0x70279f96, 0x4d47b626, 0x0ae7ccf6, 0x3787e546, 0x85a73956,
    0xb8c710e6, 0xff676a36, 0xc2074386, 0x4057d457, 0x7d37fde7,
    0x3a978737, 0x07f7ae87, 0xb5d77297, 0x88b75b27, 0xcf1721f7,
    0xf2770847, 0x10c70814, 0x2da721a4, 0x6a075b74, 0x576772c4,
    0xe547aed4, 0xd8278764, 0x9f87fdb4, 0xa2e7d404, 0x20b743d5,
    0x1dd76a65, 0x5a7710b5, 0x67173905, 0xd537e515, 0xe857cca5,
    0xaff7b675, 0x92979fc5, 0xe915e8db, 0xd475c16b, 0x93d5bbbb,
    0xaeb5920b, 0x1c954e1b, 0x21f567ab, 0x66551d7b, 0x5b3534cb,
    0xd965a31a, 0xe4058aaa, 0xa3a5f07a, 0x9ec5d9ca, 0x2ce505da,
    0x11852c6a, 0x562556ba, 0x6b457f0a, 0x89f57f59, 0xb49556e9,
    0xf3352c39, 0xce550589, 0x7c75d999, 0x4115f029, 0x06b58af9,
    0x3bd5a349, 0xb9853498, 0x84e51d28, 0xc34567f8, 0xfe254e48,
    0x4c059258, 0x7165bbe8, 0x36c5c138, 0x0ba5e888, 0x28d4c7df,
    0x15b4ee6f, 0x521494bf, 0x6f74bd0f, 0xdd54611f, 0xe03448af,
    0xa794327f, 0x9af41bcf, 0x18a48c1e, 0x25c4a5ae, 0x6264df7e,
    0x5f04f6ce, 0xed242ade, 0xd044036e, 0x97e479be, 0xaa84500e,
    0x4834505d, 0x755479ed, 0x32f4033d, 0x0f942a8d, 0xbdb4f69d,
    0x80d4df2d, 0xc774a5fd, 0xfa148c4d, 0x78441b9c, 0x4524322c,
    0x028448fc, 0x3fe4614c, 0x8dc4bd5c, 0xb0a494ec, 0xf704ee3c,
    0xca64c78c}};

local const z_word_t FAR crc_braid_big_table[][256] = {
   {0x00000000, 0xb029603d, 0x6053c07a, 0xd07aa047, 0xc0a680f5,
    0x708fe0c8, 0xa0f5408f, 0x10dc20b2, 0xc14b7030, 0x7162100d,
    0xa118b04a, 0x1131d077, 0x01edf0c5, 0xb1c490f8, 0x61be30bf,
    0xd1975082, 0x8297e060, 0x32be805d, 0xe2c4201a, 0x52ed4027,
    0x42316095, 0xf21800a8, 0x2262a0ef, 0x924bc0d2, 0x43dc9050,
    0xf3f5f06d, 0x238f502a, 0x93a63017, 0x837a10a5, 0x33537098,
    0xe329d0df, 0x5300b0e2, 0x042fc1c1, 0xb406a1fc, 0x647c01bb,
    0xd4556186, 0xc4894134, 0x74a02109, 0xa4da814e, 0x14f3e173,
    0xc564b1f1, 0x754dd1cc, 0xa537718b, 0x151e11b6, 0x05c23104,
    0xb5eb5139, 0x6591f17e, 0xd5b89143, 0x86b821a1, 0x3691419c,
    0xe6ebe1db, 0x56c281e6, 0x461ea154, 0xf637c169, 0x264d612e,
    0x96640113, 0x47f35191, 0xf7da31ac, 0x27a091eb, 0x9789f1d6,
    0x8755d164, 0x377cb159, 0xe706111e, 0x572f7123, 0x4958f358,
    0xf9719365, 0x290b3322, 0x9922531f, 0x89fe73ad, 0x39d71390,
    0xe9adb3d7, 0x5984d3ea, 0x88138368, 0x383ae355, 0xe8404312,
    0x5869232f, 0x48b5039d, 0xf89c63a0, 0x28e6c3e7, 0x98cfa3da,
    0xcbcf1338, 0x7be67305, 0xab9cd342, 0x1bb5b37f, 0x0b6993cd,
    0xbb40f3f0, 0x6b3a53b7, 0xdb13338a, 0x0a846308, 0xbaad0335,
    0x6ad7a372, 0xdafec34f, 0xca22e3fd, 0x7a0b83c0, 0xaa712387,
    0x1a5843ba, 0x4d773299, 0xfd5e52a4, 0x2d24f2e3, 0x9d0d92de,
    0x8dd1b26c, 0x3df8d251, 0xed827216, 0x5dab122b, 0x8c3c42a9,
    0x3c152294, 0xec6f82d3, 0x5c46e2ee, 0x4c9ac25c, 0xfcb3a261,
    0x2cc90226, 0x9ce0621b, 0xcfe0d2f9, 0x7fc9b2c4, 0xafb31283,
    0x1f9a72be, 0x0f46520c, 0xbf6f3231, 0x6f159276, 0xdf3cf24b,
    0x0eaba2c9, 0xbe82c2f4, 0x6ef862b3, 0xded1028e, 0xce0d223c,
    0x7e244201, 0xae5ee246, 0x1e77827b, 0x92b0e6b1, 0x2299868c,
    0xf2e326cb, 0x42ca46f6, 0x52166644, 0xe23f0679, 0x3245a63e,
    0x826cc603, 0x53fb9681, 0xe3d2f6bc, 0x33a856fb, 0x838136c6,
    0x935d1674, 0x23747649, 0xf30ed60e, 0x4327b633, 0x102706d1,
    0xa00e66ec, 0x7074c6ab, 0xc05da696, 0xd0818624, 0x60a8e619,
    0xb0d2465e, 0x00fb2663, 0xd16c76e1, 0x614516dc, 0xb13fb69b,
    0x0116d6a6, 0x11caf614, 0xa1e39629, 0x7199366e, 0xc1b05653,
    0x969f2770, 0x26b6474d, 0xf6cce70a, 0x46e58737, 0x5639a785,
    0xe610c7b8, 0x366a67ff, 0x864307c2, 0x57d45740, 0xe7fd377d,
    0x3787973a, 0x87aef707, 0x9772d7b5, 0x275bb788, 0xf72117cf,
    0x470877f2, 0x1408c710, 0xa421a72d, 0x745b076a, 0xc4726757,
    0xd4ae47e5, 0x648727d8, 0xb4fd879f, 0x04d4e7a2, 0xd543b720,
    0x656ad71d, 0xb510775a, 0x05391767, 0x15e537d5, 0xa5cc57e8,
    0x75b6f7af, 0xc59f9792, 0xdbe815e9, 0x6bc175d4, 0xbbbbd593,
    0x0b92b5ae, 0x1b4e951c, 0xab67f521, 0x7b1d5566, 0xcb34355b,
    0x1aa365d9, 0xaa8a05e4, 0x7af0a5a3, 0xcad9c59e, 0xda05e52c,
    0x6a2c8511, 0xba562556, 0x0a7f456b, 0x597ff589, 0xe95695b4,
    0x392c35f3, 0x890555ce, 0x99d9757c, 0x29f01541, 0xf98ab506,
    0x49a3d53b, 0x983485b9, 0x281de584, 0xf86745c3, 0x484e25fe,
    0x5892054c, 0xe8bb6571, 0x38c1c536, 0x88e8a50b, 0xdfc7d428,
    0x6feeb415, 0xbf941452, 0x0fbd746f, 0x1f6154dd, 0xaf4834e0,
    0x7f3294a7, 0xcf1bf49a, 0x1e8ca418, 0xaea5c425, 0x7edf6462,
    0xcef6045f, 0xde2a24ed, 0x6e0344d0, 0xbe79e497, 0x0e5084aa,
    0x5d503448, 0xed795475, 0x3d03f432, 0x8d2a940f, 0x9df6b4bd,
    0x2ddfd480, 0xfda574c7, 0x4d8c14fa, 0x9c1b4478, 0x2c322445,
    0xfc488402, 0x4c61e43f, 0x5cbdc48d, 0xec94a4b0, 0x3cee04f7,
    0x8cc764ca},
   {0x00000000, 0xa5d35ccb, 0x0ba1c84d, 0xae729486, 0x1642919b,
    0xb391cd50, 0x1de359d6, 0xb830051d, 0x6d8253ec, 0xc8510f27,
    0x66239ba1, 0xc3f0c76a, 0x7bc0c277, 0xde139ebc, 0x70610a3a,
    0xd5b256f1, 0x9b02d603, 0x3ed18ac8, 0x90a31e4e, 0x35704285,
    0x8d404798, 0x28931b53, 0x86e18fd5, 0x2332d31e, 0xf68085ef,
    0x5353d924, 0xfd214da2, 0x58f21169, 0xe0c21474, 0x451148bf,
    0xeb63dc39, 0x4eb080f2, 0x3605ac07, 0x93d6f0cc, 0x3da4644a,
    0x98773881, 0x20473d9c, 0x85946157, 0x2be6f5d1, 0x8e35a91a,
    0x5b87ffeb, 0xfe54a320, 0x502637a6, 0xf5f56b6d, 0x4dc56e70,
    0xe81632bb, 0x4664a63d, 0xe3b7faf6, 0xad077a04, 0x08d426cf,
    0xa6a6b249, 0x0375ee82, 0xbb45eb9f, 0x1e96b754, 0xb0e423d2,
    0x15377f19, 0xc08529e8, 0x65567523, 0xcb24e1a5, 0x6ef7bd6e,
    0xd6c7b873, 0x7314e4b8, 0xdd66703e, 0x78b52cf5, 0x6c0a580f,
    0xc9d904c4, 0x67ab9042, 0xc278cc89, 0x7a48c994, 0xdf9b955f,
    0x71e901d9, 0xd43a5d12, 0x01880be3, 0xa45b5728, 0x0a29c3ae,
    0xaffa9f65, 0x17ca9a78, 0xb219c6b3, 0x1c6b5235, 0xb9b80efe,
    0xf7088e0c, 0x52dbd2c7, 0xfca94641, 0x597a1a8a, 0xe14a1f97,
    0x4499435c, 0xeaebd7da, 0x4f388b11, 0x9a8adde0, 0x3f59812b,
    0x912b15ad, 0x34f84966, 0x8cc84c7b, 0x291b10b0, 0x87698436,
    0x22bad8fd, 0x5a0ff408, 0xffdca8c3, 0x51ae3c45, 0xf47d608e,
    0x4c4d6593, 0xe99e3958, 0x47ecadde, 0xe23ff115, 0x378da7e4,
    0x925efb2f, 0x3c2c6fa9, 0x99ff3362, 0x21cf367f, 0x841c6ab4,
    0x2a6efe32, 0x8fbda2f9, 0xc10d220b, 0x64de7ec0, 0xcaacea46,
    0x6f7fb68d, 0xd74fb390, 0x729cef5b, 0xdcee7bdd, 0x793d2716,
    0xac8f71e7, 0x095c2d2c, 0xa72eb9aa, 0x02fde561, 0xbacde07c,
    0x1f1ebcb7, 0xb16c2831, 0x14bf74fa, 0xd814b01e, 0x7dc7ecd5,
    0xd3b57853, 0x76662498, 0xce562185, 0x6b857d4e, 0xc5f7e9c8,
    0x6024b503, 0xb596e3f2, 0x1045bf39, 0xbe372bbf, 0x1be47774,
    0xa3d47269, 0x06072ea2, 0xa875ba24, 0x0da6e6ef, 0x4316661d,
    0xe6c53ad6, 0x48b7ae50, 0xed64f29b, 0x5554f786, 0xf087ab4d,
    0x5ef53fcb, 0xfb266300, 0x2e9435f1, 0x8b47693a, 0x2535fdbc,
    0x80e6a177, 0x38d6a46a, 0x9d05f8a1, 0x33776c27, 0x96a430ec,
    0xee111c19, 0x4bc240d2, 0xe5b0d454, 0x4063889f, 0xf8538d82,
    0x5d80d149, 0xf3f245cf, 0x56211904, 0x83934ff5, 0x2640133e,
    0x883287b8, 0x2de1db73, 0x95d1de6e, 0x300282a5, 0x9e701623,
    0x3ba34ae8, 0x7513ca1a, 0xd0c096d1, 0x7eb20257, 0xdb615e9c,
    0x63515b81, 0xc682074a, 0x68f093cc, 0xcd23cf07, 0x189199f6,
    0xbd42c53d, 0x133051bb, 0xb6e30d70, 0x0ed3086d, 0xab0054a6,
    0x0572c020, 0xa0a19ceb, 0xb41ee811, 0x11cdb4da, 0xbfbf205c,
    0x1a6c7c97, 0xa25c798a, 0x078f2541, 0xa9fdb1c7, 0x0c2eed0c,
    0xd99cbbfd, 0x7c4fe736, 0xd23d73b0, 0x77ee2f7b, 0xcfde2a66,
    0x6a0d76ad, 0xc47fe22b, 0x61acbee0, 0x2f1c3e12, 0x8acf62d9,
    0x24bdf65f, 0x816eaa94, 0x395eaf89, 0x9c8df342, 0x32ff67c4,
    0x972c3b0f, 0x429e6dfe, 0xe74d3135, 0x493fa5b3, 0xececf978,
    0x54dcfc65, 0xf10fa0ae, 0x5f7d3428, 0xfaae68e3, 0x821b4416,
    0x27c818dd, 0x89ba8c5b, 0x2c69d090, 0x9459d58d, 0x318a8946,
    0x9ff81dc0, 0x3a2b410b, 0xef9917fa, 0x4a4a4b31, 0xe438dfb7,
    0x41eb837c, 0xf9db8661, 0x5c08daaa, 0xf27a4e2c, 0x57a912e7,
    0x19199215, 0xbccacede, 0x12b85a58, 0xb76b0693, 0x0f5b038e,
    0xaa885f45, 0x04facbc3, 0xa1299708, 0x749bc1f9, 0xd1489d32,
    0x7f3a09b4, 0xdae9557f, 0x62d95062, 0xc70a0ca9, 0x6978982f,
    0xccabc4e4},
   {0x00000000, 0xb40b77a6, 0x29119f97, 0x9d1ae831, 0x13244ff4,
    0xa72f3852, 0x3a35d063, 0x8e3ea7c5, 0x674eef33, 0xd3459895,
    0x4e5f70a4, 0xfa540702, 0x746aa0c7, 0xc061d761, 0x5d7b3f50,
    0xe97048f6, 0xce9cde67, 0x7a97a9c1, 0xe78d41f0, 0x53863656,
    0xddb89193, 0x69b3e635, 0xf4a90e04, 0x40a279a2, 0xa9d23154,
    0x1dd946f2, 0x80c3aec3, 0x34c8d965, 0xbaf67ea0, 0x0efd0906,
    0x93e7e137, 0x27ec9691, 0x9c39bdcf, 0x2832ca69, 0xb5282258,
    0x012355fe, 0x8f1df23b, 0x3b16859d, 0xa60c6dac, 0x12071a0a,
    0xfb7752fc, 0x4f7c255a, 0xd266cd6b, 0x666dbacd, 0xe8531d08,
    0x5c586aae, 0xc142829f, 0x7549f539, 0x52a563a8, 0xe6ae140e,
    0x7bb4fc3f, 0xcfbf8b99, 0x41812c5c, 0xf58a5bfa, 0x6890b3cb,
    0xdc9bc46d, 0x35eb8c9b, 0x81e0fb3d, 0x1cfa130c, 0xa8f164aa,
    0x26cfc36f, 0x92c4b4c9, 0x0fde5cf8, 0xbbd52b5e, 0x79750b44,
    0xcd7e7ce2, 0x506494d3, 0xe46fe375, 0x6a5144b0, 0xde5a3316,
    0x4340db27, 0xf74bac81, 0x1e3be477, 0xaa3093d1, 0x372a7be0,
    0x83210c46, 0x0d1fab83, 0xb914dc25, 0x240e3414, 0x900543b2,
    0xb7e9d523, 0x03e2a285, 0x9ef84ab4, 0x2af33d12, 0xa4cd9ad7,
    0x10c6ed71, 0x8ddc0540, 0x39d772e6, 0xd0a73a10, 0x64ac4db6,
    0xf9b6a587, 0x4dbdd221, 0xc38375e4, 0x77880242, 0xea92ea73,
    0x5e999dd5, 0xe54cb68b, 0x5147c12d, 0xcc5d291c, 0x78565eba,
    0xf668f97f, 0x42638ed9, 0xdf7966e8, 0x6b72114e, 0x820259b8,
    0x36092e1e, 0xab13c62f, 0x1f18b189, 0x9126164c, 0x252d61ea,
    0xb83789db, 0x0c3cfe7d, 0x2bd068ec, 0x9fdb1f4a, 0x02c1f77b,
    0xb6ca80dd, 0x38f42718, 0x8cff50be, 0x11e5b88f, 0xa5eecf29,
    0x4c9e87df, 0xf895f079, 0x658f1848, 0xd1846fee, 0x5fbac82b,
    0xebb1bf8d, 0x76ab57bc, 0xc2a0201a, 0xf2ea1688, 0x46e1612e,
    0xdbfb891f, 0x6ff0feb9, 0xe1ce597c, 0x55c52eda, 0xc8dfc6eb,
    0x7cd4b14d, 0x95a4f9bb, 0x21af8e1d, 0xbcb5662c, 0x08be118a,
    0x8680b64f, 0x328bc1e9, 0xaf9129d8, 0x1b9a5e7e, 0x3c76c8ef,
    0x887dbf49, 0x15675778, 0xa16c20de, 0x2f52871b, 0x9b59f0bd,
    0x0643188c, 0xb2486f2a, 0x5b3827dc, 0xef33507a, 0x7229b84b,
    0xc622cfed, 0x481c6828, 0xfc171f8e, 0x610df7bf, 0xd5068019,
    0x6ed3ab47, 0xdad8dce1, 0x47c234d0, 0xf3c94376, 0x7df7e4b3,
    0xc9fc9315, 0x54e67b24, 0xe0ed0c82, 0x099d4474, 0xbd9633d2,
    0x208cdbe3, 0x9487ac45, 0x1ab90b80, 0xaeb27c26, 0x33a89417,
    0x87a3e3b1, 0xa04f7520, 0x14440286, 0x895eeab7, 0x3d559d11,
    0xb36b3ad4, 0x07604d72, 0x9a7aa543, 0x2e71d2e5, 0xc7019a13,
    0x730aedb5, 0xee100584, 0x5a1b7222, 0xd425d5e7, 0x602ea241,
    0xfd344a70, 0x493f3dd6, 0x8b9f1dcc, 0x3f946a6a, 0xa28e825b,
    0x1685f5fd, 0x98bb5238, 0x2cb0259e, 0xb1aacdaf, 0x05a1ba09,
    0xecd1f2ff, 0x58da8559, 0xc5c06d68, 0x71cb1ace, 0xfff5bd0b,
    0x4bfecaad, 0xd6e4229c, 0x62ef553a, 0x4503c3ab, 0xf108b40d,
    0x6c125c3c, 0xd8192b9a, 0x56278c5f, 0xe22cfbf9, 0x7f3613c8,
    0xcb3d646e, 0x224d2c98, 0x96465b3e, 0x0b5cb30f, 0xbf57c4a9,
    0x3169636c, 0x856214ca, 0x1878fcfb, 0xac738b5d, 0x17a6a003,
    0xa3add7a5, 0x3eb73f94, 0x8abc4832, 0x0482eff7, 0xb0899851,
    0x2d937060, 0x999807c6, 0x70e84f30, 0xc4e33896, 0x59f9d0a7,
    0xedf2a701, 0x63cc00c4, 0xd7c77762, 0x4add9f53, 0xfed6e8f5,
    0xd93a7e64, 0x6d3109c2, 0xf02be1f3, 0x44209655, 0xca1e3190,
    0x7e154636, 0xe30fae07, 0x5704d9a1, 0xbe749157, 0x0a7fe6f1,
    0x97650ec0, 0x236e7966, 0xad50dea3, 0x195ba905, 0x84414134,
    0x304a3692},
   {0x00000000, 0x9e00aacc, 0x7d072542, 0xe3078f8e, 0xfa0e4a84,
    0x640ee048, 0x87096fc6, 0x1909c50a, 0xb51be5d3, 0x2b1b4f1f,
    0xc81cc091, 0x561c6a5d, 0x4f15af57, 0xd115059b, 0x32128a15,
    0xac1220d9, 0x2b31bb7c, 0xb53111b0, 0x56369e3e, 0xc83634f2,
    0xd13ff1f8, 0x4f3f5b34, 0xac38d4ba, 0x32387e76, 0x9e2a5eaf,
    0x002af463, 0xe32d7bed, 0x7d2dd121, 0x6424142b, 0xfa24bee7,
    0x19233169, 0x87239ba5, 0x566276f9, 0xc862dc35, 0x2b6553bb,
    0xb565f977, 0xac6c3c7d, 0x326c96b1, 0xd16b193f, 0x4f6bb3f3,
    0xe379932a, 0x7d7939e6, 0x9e7eb668, 0x007e1ca4, 0x1977d9ae,
    0x87777362, 0x6470fcec, 0xfa705620, 0x7d53cd85, 0xe3536749,
    0x0054e8c7, 0x9e54420b, 0x875d8701, 0x195d2dcd, 0xfa5aa243,
    0x645a088f, 0xc8482856, 0x5648829a, 0xb54f0d14, 0x2b4fa7d8,
    0x324662d2, 0xac46c81e, 0x4f414790, 0xd141ed5c, 0xedc29d29,
    0x73c237e5, 0x90c5b86b, 0x0ec512a7, 0x17ccd7ad, 0x89cc7d61,
    0x6acbf2ef, 0xf4cb5823, 0x58d978fa, 0xc6d9d236, 0x25de5db8,
    0xbbdef774, 0xa2d7327e, 0x3cd798b2, 0xdfd0173c, 0x41d0bdf0,
    0xc6f32655, 0x58f38c99, 0xbbf40317, 0x25f4a9db, 0x3cfd6cd1,
    0xa2fdc61d, 0x41fa4993, 0xdffae35f, 0x73e8c386, 0xede8694a,
    0x0eefe6c4, 0x90ef4c08, 0x89e68902, 0x17e623ce, 0xf4e1ac40,
    0x6ae1068c, 0xbba0ebd0, 0x25a0411c, 0xc6a7ce92, 0x58a7645e,
    0x41aea154, 0xdfae0b98, 0x3ca98416, 0xa2a92eda, 0x0ebb0e03,
    0x90bba4cf, 0x73bc2b41, 0xedbc818d, 0xf4b54487, 0x6ab5ee4b,
    0x89b261c5, 0x17b2cb09, 0x909150ac, 0x0e91fa60, 0xed9675ee,
    0x7396df22, 0x6a9f1a28, 0xf49fb0e4, 0x17983f6a, 0x899895a6,
    0x258ab57f, 0xbb8a1fb3, 0x588d903d, 0xc68d3af1, 0xdf84fffb,
    0x41845537, 0xa283dab9, 0x3c837075, 0xda853b53, 0x4485919f,
    0xa7821e11, 0x3982b4dd, 0x208b71d7, 0xbe8bdb1b, 0x5d8c5495,
    0xc38cfe59, 0x6f9ede80, 0xf19e744c, 0x1299fbc2, 0x8c99510e,
    0x95909404, 0x0b903ec8, 0xe897b146, 0x76971b8a, 0xf1b4802f,
    0x6fb42ae3, 0x8cb3a56d, 0x12b30fa1, 0x0bbacaab, 0x95ba6067,
    0x76bdefe9, 0xe8bd4525, 0x44af65fc, 0xdaafcf30, 0x39a840be,
    0xa7a8ea72, 0xbea12f78, 0x20a185b4, 0xc3a60a3a, 0x5da6a0f6,
    0x8ce74daa, 0x12e7e766, 0xf1e068e8, 0x6fe0c224, 0x76e9072e,
    0xe8e9ade2, 0x0bee226c, 0x95ee88a0, 0x39fca879, 0xa7fc02b5,
    0x44fb8d3b, 0xdafb27f7, 0xc3f2e2fd, 0x5df24831, 0xbef5c7bf,
    0x20f56d73, 0xa7d6f6d6, 0x39d65c1a, 0xdad1d394, 0x44d17958,
    0x5dd8bc52, 0xc3d8169e, 0x20df9910, 0xbedf33dc, 0x12cd1305,
    0x8ccdb9c9, 0x6fca3647, 0xf1ca9c8b, 0xe8c35981, 0x76c3f34d,
    0x95c47cc3, 0x0bc4d60f, 0x3747a67a, 0xa9470cb6, 0x4a408338,
    0xd44029f4, 0xcd49ecfe, 0x53494632, 0xb04ec9bc, 0x2e4e6370,
    0x825c43a9, 0x1c5ce965, 0xff5b66eb, 0x615bcc27, 0x7852092d,
    0xe652a3e1, 0x05552c6f, 0x9b5586a3, 0x1c761d06, 0x8276b7ca,
    0x61713844, 0xff719288, 0xe6785782, 0x7878fd4e, 0x9b7f72c0,
    0x057fd80c, 0xa96df8d5, 0x376d5219, 0xd46add97, 0x4a6a775b,
    0x5363b251, 0xcd63189d, 0x2e649713, 0xb0643ddf, 0x6125d083,
    0xff257a4f, 0x1c22f5c1, 0x82225f0d, 0x9b2b9a07, 0x052b30cb,
    0xe62cbf45, 0x782c1589, 0xd43e3550, 0x4a3e9f9c, 0xa9391012,
    0x3739bade, 0x2e307fd4, 0xb030d518, 0x53375a96, 0xcd37f05a,
    0x4a146bff, 0xd414c133, 0x37134ebd, 0xa913e471, 0xb01a217b,
    0x2e1a8bb7, 0xcd1d0439, 0x531daef5, 0xff0f8e2c, 0x610f24e0,
    0x8208ab6e, 0x1c0801a2, 0x0501c4a8, 0x9b016e64, 0x7806e1ea,
    0xe6064b26}};

#endif

#endif

#if N == 3

#if W == 8

local const z_crc_t FAR crc_braid_table[][256] = {
   {0x00000000, 0x81256527, 0xd93bcc0f, 0x581ea928, 0x69069e5f,
    0xe823fb78, 0xb03d5250, 0x31183777, 0xd20d3cbe, 0x53285999,
    0x0b36f0b1, 0x8a139596, 0xbb0ba2e1, 0x3a2ec7c6, 0x62306eee,
    0xe3150bc9, 0x7f6b7f3d, 0xfe4e1a1a, 0xa650b332, 0x2775d615,
    0x166de162, 0x97488445, 0xcf562d6d, 0x4e73484a, 0xad664383,
    0x2c4326a4, 0x745d8f8c, 0xf578eaab, 0xc460dddc, 0x4545b8fb,
    0x1d5b11d3, 0x9c7e74f4, 0xfed6fe7a, 0x7ff39b5d, 0x27ed3275,
    0xa6c85752, 0x97d06025, 0x16f50502, 0x4eebac2a, 0xcfcec90d,
    0x2cdbc2c4, 0xadfea7e3, 0xf5e00ecb, 0x74c56bec, 0x45dd5c9b,
    0xc4f839bc, 0x9ce69094, 0x1dc3f5b3, 0x81bd8147, 0x0098e460,
    0x58864d48, 0xd9a3286f, 0xe8bb1f18, 0x699e7a3f, 0x3180d317,
    0xb0a5b630, 0x53b0bdf9, 0xd295d8de, 0x8a8b71f6, 0x0bae14d1,
    0x3ab623a6, 0xbb934681, 0xe38defa9, 0x62a88a8e, 0x26dcfab5,
    0xa7f99f92, 0xffe736ba, 0x7ec2539d, 0x4fda64ea, 0xceff01cd,
    0x96e1a8e5, 0x17c4cdc2, 0xf4d1c60b, 0x75f4a32c, 0x2dea0a04,
    0xaccf6f23, 0x9dd75854, 0x1cf23d73, 0x44ec945b, 0xc5c9f17c,
    0x59b78588, 0xd892e0af, 0x808c4987, 0x01a92ca0, 0x30b11bd7,
    0xb1947ef0, 0xe98ad7d8, 0x68afb2ff, 0x8bbab936, 0x0a9fdc11,
    0x52817539, 0xd3a4101e, 0xe2bc2769, 0x6399424e, 0x3b87eb66,
    0xbaa28e41, 0xd80a04cf, 0x592f61e8, 0x0131c8c0, 0x8014ade7,
    0xb10c9a90, 0x3029ffb7, 0x6837569f, 0xe91233b8, 0x0a073871,
    0x8b225d56, 0xd33cf47e, 0x52199159, 0x6301a62e, 0xe224c309,
    0xba3a6a21, 0x3b1f0f06, 0xa7617bf2, 0x26441ed5, 0x7e5ab7fd,
    0xff7fd2da, 0xce67e5ad, 0x4f42808a, 0x175c29a2, 0x96794c85,
    0x756c474c, 0xf449226b, 0xac578b43, 0x2d72ee64, 0x1c6ad913,
    0x9d4fbc34, 0xc551151c, 0x4474703b, 0x4db9f56a, 0xcc9c904d,
    0x94823965, 0x15a75c42, 0x24bf6b35, 0xa59a0e12, 0xfd84a73a,
    0x7ca1c21d, 0x9fb4c9d4, 0x1e91acf3, 0x468f05db, 0xc7aa60fc,
    0xf6b2578b, 0x779732ac, 0x2f899b84, 0xaeacfea3, 0x32d28a57,
    0xb3f7ef70, 0xebe94658, 0x6acc237f, 0x5bd41408, 0xdaf1712f,
    0x82efd807, 0x03cabd20, 0xe0dfb6e9, 0x61fad3ce, 0x39e47ae6,
    0xb8c11fc1, 0x89d928b6, 0x08fc4d91, 0x50e2e4b9, 0xd1c7819e,
    0xb36f0b10, 0x324a6e37, 0x6a54c71f, 0xeb71a238, 0xda69954f,
    0x5b4cf068, 0x03525940, 0x82773c67, 0x616237ae, 0xe0475289,
    0xb859fba1, 0x397c9e86, 0x0864a9f1, 0x8941ccd6, 0xd15f65fe,
    0x507a00d9, 0xcc04742d, 0x4d21110a, 0x153fb822, 0x941add05,
    0xa502ea72, 0x24278f55, 0x7c39267d, 0xfd1c435a, 0x1e094893,
    0x9f2c2db4, 0xc732849c, 0x4617e1bb, 0x770fd6cc, 0xf62ab3eb,
    0xae341ac3, 0x2f117fe4, 0x6b650fdf, 0xea406af8, 0xb25ec3d0,
    0x337ba6f7, 0x02639180, 0x8346f4a7, 0xdb585d8f, 0x5a7d38a8,
    0xb9683361, 0x384d5646, 0x6053ff6e, 0xe1769a49, 0xd06ead3e,
    0x514bc819, 0x09556131, 0x88700416, 0x140e70e2, 0x952b15c5,
    0xcd35bced, 0x4c10d9ca, 0x7d08eebd, 0xfc2d8b9a, 0xa43322b2,
    0x25164795, 0xc6034c5c, 0x4726297b, 0x1f388053, 0x9e1de574,
    0xaf05d203, 0x2e20b724, 0x763e1e0c, 0xf71b7b2b, 0x95b3f1a5,
    0x14969482, 0x4c883daa, 0xcdad588d, 0xfcb56ffa, 0x7d900add,
    0x258ea3f5, 0xa4abc6d2, 0x47becd1b, 0xc69ba83c, 0x9e850114,
    0x1fa06433, 0x2eb85344, 0xaf9d3663, 0xf7839f4b, 0x76a6fa6c,
    0xead88e98, 0x6bfdebbf, 0x33e34297, 0xb2c627b0, 0x83de10c7,
    0x02fb75e0, 0x5ae5dcc8, 0xdbc0b9ef, 0x38d5b226, 0xb9f0d701,
    0xe1ee7e29, 0x60cb1b0e, 0x51d32c79, 0xd0f6495e, 0x88e8e076,
    0x09cd8551},
   {0x00000000, 0x9b73ead4, 0xed96d3e9, 0x76e5393d, 0x005ca193,
    0x9b2f4b47, 0xedca727a, 0x76b998ae, 0x00b94326, 0x9bcaa9f2,
    0xed2f90cf, 0x765c7a1b, 0x00e5e2b5, 0x9b960861, 0xed73315c,
    0x7600db88, 0x0172864c, 0x9a016c98, 0xece455a5, 0x7797bf71,
    0x012e27df, 0x9a5dcd0b, 0xecb8f436, 0x77cb1ee2, 0x01cbc56a,
    0x9ab82fbe, 0xec5d1683, 0x772efc57, 0x019764f9, 0x9ae48e2d,
    0xec01b710, 0x77725dc4, 0x02e50c98, 0x9996e64c, 0xef73df71,
    0x740035a5, 0x02b9ad0b, 0x99ca47df, 0xef2f7ee2, 0x745c9436,
    0x025c4fbe, 0x992fa56a, 0xefca9c57, 0x74b97683, 0x0200ee2d,
    0x997304f9, 0xef963dc4, 0x74e5d710, 0x03978ad4, 0x98e46000,
    0xee01593d, 0x7572b3e9, 0x03cb2b47, 0x98b8c193, 0xee5df8ae,
    0x752e127a, 0x032ec9f2, 0x985d2326, 0xeeb81a1b, 0x75cbf0cf,
    0x03726861, 0x980182b5, 0xeee4bb88, 0x7597515c, 0x05ca1930,
    0x9eb9f3e4, 0xe85ccad9, 0x732f200d, 0x0596b8a3, 0x9ee55277,
    0xe8006b4a, 0x7373819e, 0x05735a16, 0x9e00b0c2, 0xe8e589ff,
    0x7396632b, 0x052ffb85, 0x9e5c1151, 0xe8b9286c, 0x73cac2b8,
    0x04b89f7c, 0x9fcb75a8, 0xe92e4c95, 0x725da641, 0x04e43eef,
    0x9f97d43b, 0xe972ed06, 0x720107d2, 0x0401dc5a, 0x9f72368e,
    0xe9970fb3, 0x72e4e567, 0x045d7dc9, 0x9f2e971d, 0xe9cbae20,
    0x72b844f4, 0x072f15a8, 0x9c5cff7c, 0xeab9c641, 0x71ca2c95,
    0x0773b43b, 0x9c005eef, 0xeae567d2, 0x71968d06, 0x0796568e,
    0x9ce5bc5a, 0xea008567, 0x71736fb3, 0x07caf71d, 0x9cb91dc9,
    0xea5c24f4, 0x712fce20, 0x065d93e4, 0x9d2e7930, 0xebcb400d,
    0x70b8aad9, 0x06013277, 0x9d72d8a3, 0xeb97e19e, 0x70e40b4a,
    0x06e4d0c2, 0x9d973a16, 0xeb72032b, 0x7001e9ff, 0x06b87151,
    0x9dcb9b85, 0xeb2ea2b8, 0x705d486c, 0x0b943260, 0x90e7d8b4,
    0xe602e189, 0x7d710b5d, 0x0bc893f3, 0x90bb7927, 0xe65e401a,
    0x7d2daace, 0x0b2d7146, 0x905e9b92, 0xe6bba2af, 0x7dc8487b,
    0x0b71d0d5, 0x90023a01, 0xe6e7033c, 0x7d94e9e8, 0x0ae6b42c,
    0x91955ef8, 0xe77067c5, 0x7c038d11, 0x0aba15bf, 0x91c9ff6b,
    0xe72cc656, 0x7c5f2c82, 0x0a5ff70a, 0x912c1dde, 0xe7c924e3,
    0x7cbace37, 0x0a035699, 0x9170bc4d, 0xe7958570, 0x7ce66fa4,
    0x09713ef8, 0x9202d42c, 0xe4e7ed11, 0x7f9407c5, 0x092d9f6b,
    0x925e75bf, 0xe4bb4c82, 0x7fc8a656, 0x09c87dde, 0x92bb970a,
    0xe45eae37, 0x7f2d44e3, 0x0994dc4d, 0x92e73699, 0xe4020fa4,
    0x7f71e570, 0x0803b8b4, 0x93705260, 0xe5956b5d, 0x7ee68189,
    0x085f1927, 0x932cf3f3, 0xe5c9cace, 0x7eba201a, 0x08bafb92,
    0x93c91146, 0xe52c287b, 0x7e5fc2af, 0x08e65a01, 0x9395b0d5,
    0xe57089e8, 0x7e03633c, 0x0e5e2b50, 0x952dc184, 0xe3c8f8b9,
    0x78bb126d, 0x0e028ac3, 0x95716017, 0xe394592a, 0x78e7b3fe,
    0x0ee76876, 0x959482a2, 0xe371bb9f, 0x7802514b, 0x0ebbc9e5,
    0x95c82331, 0xe32d1a0c, 0x785ef0d8, 0x0f2cad1c, 0x945f47c8,
    0xe2ba7ef5, 0x79c99421, 0x0f700c8f, 0x9403e65b, 0xe2e6df66,
    0x799535b2, 0x0f95ee3a, 0x94e604ee, 0xe2033dd3, 0x7970d707,
    0x0fc94fa9, 0x94baa57d, 0xe25f9c40, 0x792c7694, 0x0cbb27c8,
    0x97c8cd1c, 0xe12df421, 0x7a5e1ef5, 0x0ce7865b, 0x97946c8f,
    0xe17155b2, 0x7a02bf66, 0x0c0264ee, 0x97718e3a, 0xe194b707,
    0x7ae75dd3, 0x0c5ec57d, 0x972d2fa9, 0xe1c81694, 0x7abbfc40,
    0x0dc9a184, 0x96ba4b50, 0xe05f726d, 0x7b2c98b9, 0x0d950017,
    0x96e6eac3, 0xe003d3fe, 0x7b70392a, 0x0d70e2a2, 0x96030876,
    0xe0e6314b, 0x7b95db9f, 0x0d2c4331, 0x965fa9e5, 0xe0ba90d8,
    0x7bc97a0c},
   {0x00000000, 0x172864c0, 0x2e50c980, 0x3978ad40, 0x5ca19300,
    0x4b89f7c0, 0x72f15a80, 0x65d93e40, 0xb9432600, 0xae6b42c0,
    0x9713ef80, 0x803b8b40, 0xe5e2b500, 0xf2cad1c0, 0xcbb27c80,
    0xdc9a1840, 0xa9f74a41, 0xbedf2e81, 0x87a783c1, 0x908fe701,
    0xf556d941, 0xe27ebd81, 0xdb0610c1, 0xcc2e7401, 0x10b46c41,
    0x079c0881, 0x3ee4a5c1, 0x29ccc101, 0x4c15ff41, 0x5b3d9b81,
    0x624536c1, 0x756d5201, 0x889f92c3, 0x9fb7f603, 0xa6cf5b43,
    0xb1e73f83, 0xd43e01c3, 0xc3166503, 0xfa6ec843, 0xed46ac83,
    0x31dcb4c3, 0x26f4d003, 0x1f8c7d43, 0x08a41983, 0x6d7d27c3,
    0x7a554303, 0x432dee43, 0x54058a83, 0x2168d882, 0x3640bc42,
    0x0f381102, 0x181075c2, 0x7dc94b82, 0x6ae12f42, 0x53998202,
    0x44b1e6c2, 0x982bfe82, 0x8f039a42, 0xb67b3702, 0xa15353c2,
    0xc48a6d82, 0xd3a20942, 0xeadaa402, 0xfdf2c0c2, 0xca4e23c7,
    0xdd664707, 0xe41eea47, 0xf3368e87, 0x96efb0c7, 0x81c7d407,
    0xb8bf7947, 0xaf971d87, 0x730d05c7, 0x64256107, 0x5d5dcc47,
    0x4a75a887, 0x2fac96c7, 0x3884f207, 0x01fc5f47, 0x16d43b87,
    0x63b96986, 0x74910d46, 0x4de9a006, 0x5ac1c4c6, 0x3f18fa86,
    0x28309e46, 0x11483306, 0x066057c6, 0xdafa4f86, 0xcdd22b46,
    0xf4aa8606, 0xe382e2c6, 0x865bdc86, 0x9173b846, 0xa80b1506,
    0xbf2371c6, 0x42d1b104, 0x55f9d5c4, 0x6c817884, 0x7ba91c44,
    0x1e702204, 0x095846c4, 0x3020eb84, 0x27088f44, 0xfb929704,
    0xecbaf3c4, 0xd5c25e84, 0xc2ea3a44, 0xa7330404, 0xb01b60c4,
    0x8963cd84, 0x9e4ba944, 0xeb26fb45, 0xfc0e9f85, 0xc57632c5,
    0xd25e5605, 0xb7876845, 0xa0af0c85, 0x99d7a1c5, 0x8effc505,
    0x5265dd45, 0x454db985, 0x7c3514c5, 0x6b1d7005, 0x0ec44e45,
    0x19ec2a85, 0x209487c5, 0x37bce305, 0x4fed41cf, 0x58c5250f,
    0x61bd884f, 0x7695ec8f, 0x134cd2cf, 0x0464b60f, 0x3d1c1b4f,
    0x2a347f8f, 0xf6ae67cf, 0xe186030f, 0xd8feae4f, 0xcfd6ca8f,
    0xaa0ff4cf, 0xbd27900f, 0x845f3d4f, 0x9377598f, 0xe61a0b8e,
    0xf1326f4e, 0xc84ac20e, 0xdf62a6ce, 0xbabb988e, 0xad93fc4e,
    0x94eb510e, 0x83c335ce, 0x5f592d8e, 0x4871494e, 0x7109e40e,
    0x662180ce, 0x03f8be8e, 0x14d0da4e, 0x2da8770e, 0x3a8013ce,
    0xc772d30c, 0xd05ab7cc, 0xe9221a8c, 0xfe0a7e4c, 0x9bd3400c,
    0x8cfb24cc, 0xb583898c, 0xa2abed4c, 0x7e31f50c, 0x691991cc,
    0x50613c8c, 0x4749584c, 0x2290660c, 0x35b802cc, 0x0cc0af8c,
    0x1be8cb4c, 0x6e85994d, 0x79adfd8d, 0x40d550cd, 0x57fd340d,
    0x32240a4d, 0x250c6e8d, 0x1c74c3cd, 0x0b5ca70d, 0xd7c6bf4d,
    0xc0eedb8d, 0xf99676cd, 0xeebe120d, 0x8b672c4d, 0x9c4f488d,
    0xa537e5cd, 0xb21f810d, 0x85a36208, 0x928b06c8, 0xabf3ab88,
    0xbcdbcf48, 0xd902f108, 0xce2a95c8, 0xf7523888, 0xe07a5c48,
    0x3ce04408, 0x2bc820c8, 0x12b08d88, 0x0598e948, 0x6041d708,
    0x7769b3c8, 0x4e111e88, 0x59397a48, 0x2c542849, 0x3b7c4c89,
    0x0204e1c9, 0x152c8509, 0x70f5bb49, 0x67dddf89, 0x5ea572c9,
    0x498d1609, 0x95170e49, 0x823f6a89, 0xbb47c7c9, 0xac6fa309,
    0xc9b69d49, 0xde9ef989, 0xe7e654c9, 0xf0ce3009, 0x0d3cf0cb,
    0x1a14940b, 0x236c394b, 0x34445d8b, 0x519d63cb, 0x46b5070b,
    0x7fcdaa4b, 0x68e5ce8b, 0xb47fd6cb, 0xa357b20b, 0x9a2f1f4b,
    0x8d077b8b, 0xe8de45cb, 0xfff6210b, 0xc68e8c4b, 0xd1a6e88b,
    0xa4cbba8a, 0xb3e3de4a, 0x8a9b730a, 0x9db317ca, 0xf86a298a,
    0xef424d4a, 0xd63ae00a, 0xc11284ca, 0x1d889c8a, 0x0aa0f84a,
    0x33d8550a, 0x24f031ca, 0x41290f8a, 0x56016b4a, 0x6f79c60a,
    0x7851a2ca},
   {0x00000000, 0x9fda839e, 0xe4c4017d, 0x7b1e82e3, 0x12f904bb,
    0x8d238725, 0xf63d05c6, 0x69e78658, 0x25f20976, 0xba288ae8,
    0xc136080b, 0x5eec8b95, 0x370b0dcd, 0xa8d18e53, 0xd3cf0cb0,
    0x4c158f2e, 0x4be412ec, 0xd43e9172, 0xaf201391, 0x30fa900f,
    0x591d1657, 0xc6c795c9, 0xbdd9172a, 0x220394b4, 0x6e161b9a,
    0xf1cc9804, 0x8ad21ae7, 0x15089979, 0x7cef1f21, 0xe3359cbf,
    0x982b1e5c, 0x07f19dc2, 0x97c825d8, 0x0812a646, 0x730c24a5,
    0xecd6a73b, 0x85312163, 0x1aeba2fd, 0x61f5201e, 0xfe2fa380,
    0xb23a2cae, 0x2de0af30, 0x56fe2dd3, 0xc924ae4d, 0xa0c32815,
    0x3f19ab8b, 0x44072968, 0xdbddaaf6, 0xdc2c3734, 0x43f6b4aa,
    0x38e83649, 0xa732b5d7, 0xced5338f, 0x510fb011, 0x2a1132f2,
    0xb5cbb16c, 0xf9de3e42, 0x6604bddc, 0x1d1a3f3f, 0x82c0bca1,
    0xeb273af9, 0x74fdb967, 0x0fe33b84, 0x9039b81a, 0xf4e14df1,
    0x6b3bce6f, 0x10254c8c, 0x8fffcf12, 0xe618494a, 0x79c2cad4,
    0x02dc4837, 0x9d06cba9, 0xd1134487, 0x4ec9c719, 0x35d745fa,
    0xaa0dc664, 0xc3ea403c, 0x5c30c3a2, 0x272e4141, 0xb8f4c2df,
    0xbf055f1d, 0x20dfdc83, 0x5bc15e60, 0xc41bddfe, 0xadfc5ba6,
    0x3226d838, 0x49385adb, 0xd6e2d945, 0x9af7566b, 0x052dd5f5,
    0x7e335716, 0xe1e9d488, 0x880e52d0, 0x17d4d14e, 0x6cca53ad,
    0xf310d033, 0x63296829, 0xfcf3ebb7, 0x87ed6954, 0x1837eaca,
    0x71d06c92, 0xee0aef0c, 0x95146def, 0x0aceee71, 0x46db615f,
    0xd901e2c1, 0xa21f6022, 0x3dc5e3bc, 0x542265e4, 0xcbf8e67a,
    0xb0e66499, 0x2f3ce707, 0x28cd7ac5, 0xb717f95b, 0xcc097bb8,
    0x53d3f826, 0x3a347e7e, 0xa5eefde0, 0xdef07f03, 0x412afc9d,
    0x0d3f73b3, 0x92e5f02d, 0xe9fb72ce, 0x7621f150, 0x1fc67708,
    0x801cf496, 0xfb027675, 0x64d8f5eb, 0x32b39da3, 0xad691e3d,
    0xd6779cde, 0x49ad1f40, 0x204a9918, 0xbf901a86, 0xc48e9865,
    0x5b541bfb, 0x174194d5, 0x889b174b, 0xf38595a8, 0x6c5f1636,
    0x05b8906e, 0x9a6213f0, 0xe17c9113, 0x7ea6128d, 0x79578f4f,
    0xe68d0cd1, 0x9d938e32, 0x02490dac, 0x6bae8bf4, 0xf474086a,
    0x8f6a8a89, 0x10b00917, 0x5ca58639, 0xc37f05a7, 0xb8618744,
    0x27bb04da, 0x4e5c8282, 0xd186011c, 0xaa9883ff, 0x35420061,
    0xa57bb87b, 0x3aa13be5, 0x41bfb906, 0xde653a98, 0xb782bcc0,
    0x28583f5e, 0x5346bdbd, 0xcc9c3e23, 0x8089b10d, 0x1f533293,
    0x644db070, 0xfb9733ee, 0x9270b5b6, 0x0daa3628, 0x76b4b4cb,
    0xe96e3755, 0xee9faa97, 0x71452909, 0x0a5babea, 0x95812874,
    0xfc66ae2c, 0x63bc2db2, 0x18a2af51, 0x87782ccf, 0xcb6da3e1,
    0x54b7207f, 0x2fa9a29c, 0xb0732102, 0xd994a75a, 0x464e24c4,
    0x3d50a627, 0xa28a25b9, 0xc652d052, 0x598853cc, 0x2296d12f,
    0xbd4c52b1, 0xd4abd4e9, 0x4b715777, 0x306fd594, 0xafb5560a,
    0xe3a0d924, 0x7c7a5aba, 0x0764d859, 0x98be5bc7, 0xf159dd9f,
    0x6e835e01, 0x159ddce2, 0x8a475f7c, 0x8db6c2be, 0x126c4120,
    0x6972c3c3, 0xf6a8405d, 0x9f4fc605, 0x0095459b, 0x7b8bc778,
    0xe45144e6, 0xa844cbc8, 0x379e4856, 0x4c80cab5, 0xd35a492b,
    0xbabdcf73, 0x25674ced, 0x5e79ce0e, 0xc1a34d90, 0x519af58a,
    0xce407614, 0xb55ef4f7, 0x2a847769, 0x4363f131, 0xdcb972af,
    0xa7a7f04c, 0x387d73d2, 0x7468fcfc, 0xebb27f62, 0x90acfd81,
    0x0f767e1f, 0x6691f847, 0xf94b7bd9, 0x8255f93a, 0x1d8f7aa4,
    0x1a7ee766, 0x85a464f8, 0xfebae61b, 0x61606585, 0x0887e3dd,
    0x975d6043, 0xec43e2a0, 0x7399613e, 0x3f8cee10, 0xa0566d8e,
    0xdb48ef6d, 0x44926cf3, 0x2d75eaab, 0xb2af6935, 0xc9b1ebd6,
    0x566b6848},
   {0x00000000, 0x65673b46, 0xcace768c, 0xafa94dca, 0x4eedeb59,
    0x2b8ad01f, 0x84239dd5, 0xe144a693, 0x9ddbd6b2, 0xf8bcedf4,
    0x5715a03e, 0x32729b78, 0xd3363deb, 0xb65106ad, 0x19f84b67,
    0x7c9f7021, 0xe0c6ab25, 0x85a19063, 0x2a08dda9, 0x4f6fe6ef,
    0xae2b407c, 0xcb4c7b3a, 0x64e536f0, 0x01820db6, 0x7d1d7d97,
    0x187a46d1, 0xb7d30b1b, 0xd2b4305d, 0x33f096ce, 0x5697ad88,
    0xf93ee042, 0x9c59db04, 0x1afc500b, 0x7f9b6b4d, 0xd0322687,
    0xb5551dc1, 0x5411bb52, 0x31768014, 0x9edfcdde, 0xfbb8f698,
    0x872786b9, 0xe240bdff, 0x4de9f035, 0x288ecb73, 0xc9ca6de0,
    0xacad56a6, 0x03041b6c, 0x6663202a, 0xfa3afb2e, 0x9f5dc068,
    0x30f48da2, 0x5593b6e4, 0xb4d71077, 0xd1b02b31, 0x7e1966fb,
    0x1b7e5dbd, 0x67e12d9c, 0x028616da, 0xad2f5b10, 0xc8486056,
    0x290cc6c5, 0x4c6bfd83, 0xe3c2b049, 0x86a58b0f, 0x35f8a016,
    0x509f9b50, 0xff36d69a, 0x9a51eddc, 0x7b154b4f, 0x1e727009,
    0xb1db3dc3, 0xd4bc0685, 0xa82376a4, 0xcd444de2, 0x62ed0028,
    0x078a3b6e, 0xe6ce9dfd, 0x83a9a6bb, 0x2c00eb71, 0x4967d037,
    0xd53e0b33, 0xb0593075, 0x1ff07dbf, 0x7a9746f9, 0x9bd3e06a,
    0xfeb4db2c, 0x511d96e6, 0x347aada0, 0x48e5dd81, 0x2d82e6c7,
    0x822bab0d, 0xe74c904b, 0x060836d8, 0x636f0d9e, 0xccc64054,
    0xa9a17b12, 0x2f04f01d, 0x4a63cb5b, 0xe5ca8691, 0x80adbdd7,
    0x61e91b44, 0x048e2002, 0xab276dc8, 0xce40568e, 0xb2df26af,
    0xd7b81de9, 0x78115023, 0x1d766b65, 0xfc32cdf6, 0x9955f6b0,
    0x36fcbb7a, 0x539b803c, 0xcfc25b38, 0xaaa5607e, 0x050c2db4,
    0x606b16f2, 0x812fb061, 0xe4488b27, 0x4be1c6ed, 0x2e86fdab,
    0x52198d8a, 0x377eb6cc, 0x98d7fb06, 0xfdb0c040, 0x1cf466d3,
    0x79935d95, 0xd63a105f, 0xb35d2b19, 0x6bf1402c, 0x0e967b6a,
    0xa13f36a0, 0xc4580de6, 0x251cab75, 0x407b9033, 0xefd2ddf9,
    0x8ab5e6bf, 0xf62a969e, 0x934dadd8, 0x3ce4e012, 0x5983db54,
    0xb8c77dc7, 0xdda04681, 0x72090b4b, 0x176e300d, 0x8b37eb09,
    0xee50d04f, 0x41f99d85, 0x249ea6c3, 0xc5da0050, 0xa0bd3b16,
    0x0f1476dc, 0x6a734d9a, 0x16ec3dbb, 0x738b06fd, 0xdc224b37,
    0xb9457071, 0x5801d6e2, 0x3d66eda4, 0x92cfa06e, 0xf7a89b28,
    0x710d1027, 0x146a2b61, 0xbbc366ab, 0xdea45ded, 0x3fe0fb7e,
    0x5a87c038, 0xf52e8df2, 0x9049b6b4, 0xecd6c695, 0x89b1fdd3,
    0x2618b019, 0x437f8b5f, 0xa23b2dcc, 0xc75c168a, 0x68f55b40,
    0x0d926006, 0x91cbbb02, 0xf4ac8044, 0x5b05cd8e, 0x3e62f6c8,
    0xdf26505b, 0xba416b1d, 0x15e826d7, 0x708f1d91, 0x0c106db0,
    0x697756f6, 0xc6de1b3c, 0xa3b9207a, 0x42fd86e9, 0x279abdaf,
    0x8833f065, 0xed54cb23, 0x5e09e03a, 0x3b6edb7c, 0x94c796b6,
    0xf1a0adf0, 0x10e40b63, 0x75833025, 0xda2a7def, 0xbf4d46a9,
    0xc3d23688, 0xa6b50dce, 0x091c4004, 0x6c7b7b42, 0x8d3fddd1,
    0xe858e697, 0x47f1ab5d, 0x2296901b, 0xbecf4b1f, 0xdba87059,
    0x74013d93, 0x116606d5, 0xf022a046, 0x95459b00, 0x3aecd6ca,
    0x5f8bed8c, 0x23149dad, 0x4673a6eb, 0xe9daeb21, 0x8cbdd067,
    0x6df976f4, 0x089e4db2, 0xa7370078, 0xc2503b3e, 0x44f5b031,
    0x21928b77, 0x8e3bc6bd, 0xeb5cfdfb, 0x0a185b68, 0x6f7f602e,
    0xc0d62de4, 0xa5b116a2, 0xd92e6683, 0xbc495dc5, 0x13e0100f,
    0x76872b49, 0x97c38dda, 0xf2a4b69c, 0x5d0dfb56, 0x386ac010,
    0xa4331b14, 0xc1542052, 0x6efd6d98, 0x0b9a56de, 0xeadef04d,
    0x8fb9cb0b, 0x201086c1, 0x4577bd87, 0x39e8cda6, 0x5c8ff6e0,
    0xf326bb2a, 0x9641806c, 0x770526ff, 0x12621db9, 0xbdcb5073,
    0xd8ac6b35},
   {0x00000000, 0xd7e28058, 0x74b406f1, 0xa35686a9, 0xe9680de2,
    0x3e8a8dba, 0x9ddc0b13, 0x4a3e8b4b, 0x09a11d85, 0xde439ddd,
    0x7d151b74, 0xaaf79b2c, 0xe0c91067, 0x372b903f, 0x947d1696,
    0x439f96ce, 0x13423b0a, 0xc4a0bb52, 0x67f63dfb, 0xb014bda3,
    0xfa2a36e8, 0x2dc8b6b0, 0x8e9e3019, 0x597cb041, 0x1ae3268f,
    0xcd01a6d7, 0x6e57207e, 0xb9b5a026, 0xf38b2b6d, 0x2469ab35,
    0x873f2d9c, 0x50ddadc4, 0x26847614, 0xf166f64c, 0x523070e5,
    0x85d2f0bd, 0xcfec7bf6, 0x180efbae, 0xbb587d07, 0x6cbafd5f,
    0x2f256b91, 0xf8c7ebc9, 0x5b916d60, 0x8c73ed38, 0xc64d6673,
    0x11afe62b, 0xb2f96082, 0x651be0da, 0x35c64d1e, 0xe224cd46,
    0x41724bef, 0x9690cbb7, 0xdcae40fc, 0x0b4cc0a4, 0xa81a460d,
    0x7ff8c655, 0x3c67509b, 0xeb85d0c3, 0x48d3566a, 0x9f31d632,
    0xd50f5d79, 0x02eddd21, 0xa1bb5b88, 0x7659dbd0, 0x4d08ec28,
    0x9aea6c70, 0x39bcead9, 0xee5e6a81, 0xa460e1ca, 0x73826192,
    0xd0d4e73b, 0x07366763, 0x44a9f1ad, 0x934b71f5, 0x301df75c,
    0xe7ff7704, 0xadc1fc4f, 0x7a237c17, 0xd975fabe, 0x0e977ae6,
    0x5e4ad722, 0x89a8577a, 0x2afed1d3, 0xfd1c518b, 0xb722dac0,
    0x60c05a98, 0xc396dc31, 0x14745c69, 0x57ebcaa7, 0x80094aff,
    0x235fcc56, 0xf4bd4c0e, 0xbe83c745, 0x6961471d, 0xca37c1b4,
    0x1dd541ec, 0x6b8c9a3c, 0xbc6e1a64, 0x1f389ccd, 0xc8da1c95,
    0x82e497de, 0x55061786, 0xf650912f, 0x21b21177, 0x622d87b9,
    0xb5cf07e1, 0x16998148, 0xc17b0110, 0x8b458a5b, 0x5ca70a03,
    0xfff18caa, 0x28130cf2, 0x78cea136, 0xaf2c216e, 0x0c7aa7c7,
    0xdb98279f, 0x91a6acd4, 0x46442c8c, 0xe512aa25, 0x32f02a7d,
    0x716fbcb3, 0xa68d3ceb, 0x05dbba42, 0xd2393a1a, 0x9807b151,
    0x4fe53109, 0xecb3b7a0, 0x3b5137f8, 0x9a11d850, 0x4df35808,
    0xeea5dea1, 0x39475ef9, 0x7379d5b2, 0xa49b55ea, 0x07cdd343,
    0xd02f531b, 0x93b0c5d5, 0x4452458d, 0xe704c324, 0x30e6437c,
    0x7ad8c837, 0xad3a486f, 0x0e6ccec6, 0xd98e4e9e, 0x8953e35a,
    0x5eb16302, 0xfde7e5ab, 0x2a0565f3, 0x603beeb8, 0xb7d96ee0,
    0x148fe849, 0xc36d6811, 0x80f2fedf, 0x57107e87, 0xf446f82e,
    0x23a47876, 0x699af33d, 0xbe787365, 0x1d2ef5cc, 0xcacc7594,
    0xbc95ae44, 0x6b772e1c, 0xc821a8b5, 0x1fc328ed, 0x55fda3a6,
    0x821f23fe, 0x2149a557, 0xf6ab250f, 0xb534b3c1, 0x62d63399,
    0xc180b530, 0x16623568, 0x5c5cbe23, 0x8bbe3e7b, 0x28e8b8d2,
    0xff0a388a, 0xafd7954e, 0x78351516, 0xdb6393bf, 0x0c8113e7,
    0x46bf98ac, 0x915d18f4, 0x320b9e5d, 0xe5e91e05, 0xa67688cb,
    0x71940893, 0xd2c28e3a, 0x05200e62, 0x4f1e8529, 0x98fc0571,
    0x3baa83d8, 0xec480380, 0xd7193478, 0x00fbb420, 0xa3ad3289,
    0x744fb2d1, 0x3e71399a, 0xe993b9c2, 0x4ac53f6b, 0x9d27bf33,
    0xdeb829fd, 0x095aa9a5, 0xaa0c2f0c, 0x7deeaf54, 0x37d0241f,
    0xe032a447, 0x436422ee, 0x9486a2b6, 0xc45b0f72, 0x13b98f2a,
    0xb0ef0983, 0x670d89db, 0x2d330290, 0xfad182c8, 0x59870461,
    0x8e658439, 0xcdfa12f7, 0x1a1892af, 0xb94e1406, 0x6eac945e,
    0x24921f15, 0xf3709f4d, 0x502619e4, 0x87c499bc, 0xf19d426c,
    0x267fc234, 0x8529449d, 0x52cbc4c5, 0x18f54f8e, 0xcf17cfd6,
    0x6c41497f, 0xbba3c927, 0xf83c5fe9, 0x2fdedfb1, 0x8c885918,
    0x5b6ad940, 0x1154520b, 0xc6b6d253, 0x65e054fa, 0xb202d4a2,
    0xe2df7966, 0x353df93e, 0x966b7f97, 0x4189ffcf, 0x0bb77484,
    0xdc55f4dc, 0x7f037275, 0xa8e1f22d, 0xeb7e64e3, 0x3c9ce4bb,
    0x9fca6212, 0x4828e24a, 0x02166901, 0xd5f4e959, 0x76a26ff0,
    0xa140efa8},
   {0x00000000, 0xef52b6e1, 0x05d46b83, 0xea86dd62, 0x0ba8d706,
    0xe4fa61e7, 0x0e7cbc85, 0xe12e0a64, 0x1751ae0c, 0xf80318ed,
    0x1285c58f, 0xfdd7736e, 0x1cf9790a, 0xf3abcfeb, 0x192d1289,
    0xf67fa468, 0x2ea35c18, 0xc1f1eaf9, 0x2b77379b, 0xc425817a,
    0x250b8b1e, 0xca593dff, 0x20dfe09d, 0xcf8d567c, 0x39f2f214,
    0xd6a044f5, 0x3c269997, 0xd3742f76, 0x325a2512, 0xdd0893f3,
    0x378e4e91, 0xd8dcf870, 0x5d46b830, 0xb2140ed1, 0x5892d3b3,
    0xb7c06552, 0x56ee6f36, 0xb9bcd9d7, 0x533a04b5, 0xbc68b254,
    0x4a17163c, 0xa545a0dd, 0x4fc37dbf, 0xa091cb5e, 0x41bfc13a,
    0xaeed77db, 0x446baab9, 0xab391c58, 0x73e5e428, 0x9cb752c9,
    0x76318fab, 0x9963394a, 0x784d332e, 0x971f85cf, 0x7d9958ad,
    0x92cbee4c, 0x64b44a24, 0x8be6fcc5, 0x616021a7, 0x8e329746,
    0x6f1c9d22, 0x804e2bc3, 0x6ac8f6a1, 0x859a4040, 0xba8d7060,
    0x55dfc681, 0xbf591be3, 0x500bad02, 0xb125a766, 0x5e771187,
    0xb4f1cce5, 0x5ba37a04, 0xaddcde6c, 0x428e688d, 0xa808b5ef,
    0x475a030e, 0xa674096a, 0x4926bf8b, 0xa3a062e9, 0x4cf2d408,
    0x942e2c78, 0x7b7c9a99, 0x91fa47fb, 0x7ea8f11a, 0x9f86fb7e,
    0x70d44d9f, 0x9a5290fd, 0x7500261c, 0x837f8274, 0x6c2d3495,
    0x86abe9f7, 0x69f95f16, 0x88d75572, 0x6785e393, 0x8d033ef1,
    0x62518810, 0xe7cbc850, 0x08997eb1, 0xe21fa3d3, 0x0d4d1532,
    0xec631f56, 0x0331a9b7, 0xe9b774d5, 0x06e5c234, 0xf09a665c,
    0x1fc8d0bd, 0xf54e0ddf, 0x1a1cbb3e, 0xfb32b15a, 0x146007bb,
    0xfee6dad9, 0x11b46c38, 0xc9689448, 0x263a22a9, 0xccbcffcb,
    0x23ee492a, 0xc2c0434e, 0x2d92f5af, 0xc71428cd, 0x28469e2c,
    0xde393a44, 0x316b8ca5, 0xdbed51c7, 0x34bfe726, 0xd591ed42,
    0x3ac35ba3, 0xd04586c1, 0x3f173020, 0xae6be681, 0x41395060,
    0xabbf8d02, 0x44ed3be3, 0xa5c33187, 0x4a918766, 0xa0175a04,
    0x4f45ece5, 0xb93a488d, 0x5668fe6c, 0xbcee230e, 0x53bc95ef,
    0xb2929f8b, 0x5dc0296a, 0xb746f408, 0x581442e9, 0x80c8ba99,
    0x6f9a0c78, 0x851cd11a, 0x6a4e67fb, 0x8b606d9f, 0x6432db7e,
    0x8eb4061c, 0x61e6b0fd, 0x97991495, 0x78cba274, 0x924d7f16,
    0x7d1fc9f7, 0x9c31c393, 0x73637572, 0x99e5a810, 0x76b71ef1,
    0xf32d5eb1, 0x1c7fe850, 0xf6f93532, 0x19ab83d3, 0xf88589b7,
    0x17d73f56, 0xfd51e234, 0x120354d5, 0xe47cf0bd, 0x0b2e465c,
    0xe1a89b3e, 0x0efa2ddf, 0xefd427bb, 0x0086915a, 0xea004c38,
    0x0552fad9, 0xdd8e02a9, 0x32dcb448, 0xd85a692a, 0x3708dfcb,
    0xd626d5af, 0x3974634e, 0xd3f2be2c, 0x3ca008cd, 0xcadfaca5,
    0x258d1a44, 0xcf0bc726, 0x205971c7, 0xc1777ba3, 0x2e25cd42,
    0xc4a31020, 0x2bf1a6c1, 0x14e696e1, 0xfbb42000, 0x1132fd62,
    0xfe604b83, 0x1f4e41e7, 0xf01cf706, 0x1a9a2a64, 0xf5c89c85,
    0x03b738ed, 0xece58e0c, 0x0663536e, 0xe931e58f, 0x081fefeb,
    0xe74d590a, 0x0dcb8468, 0xe2993289, 0x3a45caf9, 0xd5177c18,
    0x3f91a17a, 0xd0c3179b, 0x31ed1dff, 0xdebfab1e, 0x3439767c,
    0xdb6bc09d, 0x2d1464f5, 0xc246d214, 0x28c00f76, 0xc792b997,
    0x26bcb3f3, 0xc9ee0512, 0x2368d870, 0xcc3a6e91, 0x49a02ed1,
    0xa6f29830, 0x4c744552, 0xa326f3b3, 0x4208f9d7, 0xad5a4f36,
    0x47dc9254, 0xa88e24b5, 0x5ef180dd, 0xb1a3363c, 0x5b25eb5e,
    0xb4775dbf, 0x555957db, 0xba0be13a, 0x508d3c58, 0xbfdf8ab9,
    0x670372c9, 0x8851c428, 0x62d7194a, 0x8d85afab, 0x6caba5cf,
    0x83f9132e, 0x697fce4c, 0x862d78ad, 0x7052dcc5, 0x9f006a24,
    0x7586b746, 0x9ad401a7, 0x7bfa0bc3, 0x94a8bd22, 0x7e2e6040,
    0x917cd6a1},
   {0x00000000, 0x87a6cb43, 0xd43c90c7, 0x539a5b84, 0x730827cf,
    0xf4aeec8c, 0xa734b708, 0x20927c4b, 0xe6104f9e, 0x61b684dd,
    0x322cdf59, 0xb58a141a, 0x95186851, 0x12bea312, 0x4124f896,
    0xc68233d5, 0x1751997d, 0x90f7523e, 0xc36d09ba, 0x44cbc2f9,
    0x6459beb2, 0xe3ff75f1, 0xb0652e75, 0x37c3e536, 0xf141d6e3,
    0x76e71da0, 0x257d4624, 0xa2db8d67, 0x8249f12c, 0x05ef3a6f,
    0x567561eb, 0xd1d3aaa8, 0x2ea332fa, 0xa905f9b9, 0xfa9fa23d,
    0x7d39697e, 0x5dab1535, 0xda0dde76, 0x899785f2, 0x0e314eb1,
    0xc8b37d64, 0x4f15b627, 0x1c8feda3, 0x9b2926e0, 0xbbbb5aab,
    0x3c1d91e8, 0x6f87ca6c, 0xe821012f, 0x39f2ab87, 0xbe5460c4,
    0xedce3b40, 0x6a68f003, 0x4afa8c48, 0xcd5c470b, 0x9ec61c8f,
    0x1960d7cc, 0xdfe2e419, 0x58442f5a, 0x0bde74de, 0x8c78bf9d,
    0xaceac3d6, 0x2b4c0895, 0x78d65311, 0xff709852, 0x5d4665f4,
    0xdae0aeb7, 0x897af533, 0x0edc3e70, 0x2e4e423b, 0xa9e88978,
    0xfa72d2fc, 0x7dd419bf, 0xbb562a6a, 0x3cf0e129, 0x6f6abaad,
    0xe8cc71ee, 0xc85e0da5, 0x4ff8c6e6, 0x1c629d62, 0x9bc45621,
    0x4a17fc89, 0xcdb137ca, 0x9e2b6c4e, 0x198da70d, 0x391fdb46,
    0xbeb91005, 0xed234b81, 0x6a8580c2, 0xac07b317, 0x2ba17854,
    0x783b23d0, 0xff9de893, 0xdf0f94d8, 0x58a95f9b, 0x0b33041f,
    0x8c95cf5c, 0x73e5570e, 0xf4439c4d, 0xa7d9c7c9, 0x207f0c8a,
    0x00ed70c1, 0x874bbb82, 0xd4d1e006, 0x53772b45, 0x95f51890,
    0x1253d3d3, 0x41c98857, 0xc66f4314, 0xe6fd3f5f, 0x615bf41c,
    0x32c1af98, 0xb56764db, 0x64b4ce73, 0xe3120530, 0xb0885eb4,
    0x372e95f7, 0x17bce9bc, 0x901a22ff, 0xc380797b, 0x4426b238,
    0x82a481ed, 0x05024aae, 0x5698112a, 0xd13eda69, 0xf1aca622,
    0x760a6d61, 0x259036e5, 0xa236fda6, 0xba8ccbe8, 0x3d2a00ab,
    0x6eb05b2f, 0xe916906c, 0xc984ec27, 0x4e222764, 0x1db87ce0,
    0x9a1eb7a3, 0x5c9c8476, 0xdb3a4f35, 0x88a014b1, 0x0f06dff2,
    0x2f94a3b9, 0xa83268fa, 0xfba8337e, 0x7c0ef83d, 0xaddd5295,
    0x2a7b99d6, 0x79e1c252, 0xfe470911, 0xded5755a, 0x5973be19,
    0x0ae9e59d, 0x8d4f2ede, 0x4bcd1d0b, 0xcc6bd648, 0x9ff18dcc,
    0x1857468f, 0x38c53ac4, 0xbf63f187, 0xecf9aa03, 0x6b5f6140,
    0x942ff912, 0x13893251, 0x401369d5, 0xc7b5a296, 0xe727dedd,
    0x6081159e, 0x331b4e1a, 0xb4bd8559, 0x723fb68c, 0xf5997dcf,
    0xa603264b, 0x21a5ed08, 0x01379143, 0x86915a00, 0xd50b0184,
    0x52adcac7, 0x837e606f, 0x04d8ab2c, 0x5742f0a8, 0xd0e43beb,
    0xf07647a0, 0x77d08ce3, 0x244ad767, 0xa3ec1c24, 0x656e2ff1,
    0xe2c8e4b2, 0xb152bf36, 0x36f47475, 0x1666083e, 0x91c0c37d,
    0xc25a98f9, 0x45fc53ba, 0xe7caae1c, 0x606c655f, 0x33f63edb,
    0xb450f598, 0x94c289d3, 0x13644290, 0x40fe1914, 0xc758d257,
    0x01dae182, 0x867c2ac1, 0xd5e67145, 0x5240ba06, 0x72d2c64d,
    0xf5740d0e, 0xa6ee568a, 0x21489dc9, 0xf09b3761, 0x773dfc22,
    0x24a7a7a6, 0xa3016ce5, 0x839310ae, 0x0435dbed, 0x57af8069,
    0xd0094b2a, 0x168b78ff, 0x912db3bc, 0xc2b7e838, 0x4511237b,
    0x65835f30, 0xe2259473, 0xb1bfcff7, 0x361904b4, 0xc9699ce6,
    0x4ecf57a5, 0x1d550c21, 0x9af3c762, 0xba61bb29, 0x3dc7706a,
    0x6e5d2bee, 0xe9fbe0ad, 0x2f79d378, 0xa8df183b, 0xfb4543bf,
    0x7ce388fc, 0x5c71f4b7, 0xdbd73ff4, 0x884d6470, 0x0febaf33,
    0xde38059b, 0x599eced8, 0x0a04955c, 0x8da25e1f, 0xad302254,
    0x2a96e917, 0x790cb293, 0xfeaa79d0, 0x38284a05, 0xbf8e8146,
    0xec14dac2, 0x6bb21181, 0x4b206dca, 0xcc86a689, 0x9f1cfd0d,
    0x18ba364e}};

local const z_word_t FAR crc_braid_big_table[][256] = {
   {0x0000000000000000, 0x43cba68700000000, 0xc7903cd400000000,
    0x845b9a5300000000, 0xcf27087300000000, 0x8cecaef400000000,
    0x08b734a700000000, 0x4b7c922000000000, 0x9e4f10e600000000,
    0xdd84b66100000000, 0x59df2c3200000000, 0x1a148ab500000000,
    0x5168189500000000, 0x12a3be1200000000, 0x96f8244100000000,
    0xd53382c600000000, 0x7d99511700000000, 0x3e52f79000000000,
    0xba096dc300000000, 0xf9c2cb4400000000, 0xb2be596400000000,
    0xf175ffe300000000, 0x752e65b000000000, 0x36e5c33700000000,
    0xe3d641f100000000, 0xa01de77600000000, 0x24467d2500000000,
    0x678ddba200000000, 0x2cf1498200000000, 0x6f3aef0500000000,
    0xeb61755600000000, 0xa8aad3d100000000, 0xfa32a32e00000000,
    0xb9f905a900000000, 0x3da29ffa00000000, 0x7e69397d00000000,
    0x3515ab5d00000000, 0x76de0dda00000000, 0xf285978900000000,
    0xb14e310e00000000, 0x647db3c800000000, 0x27b6154f00000000,
    0xa3ed8f1c00000000, 0xe026299b00000000, 0xab5abbbb00000000,
    0xe8911d3c00000000, 0x6cca876f00000000, 0x2f0121e800000000,
    0x87abf23900000000, 0xc46054be00000000, 0x403bceed00000000,
    0x03f0686a00000000, 0x488cfa4a00000000, 0x0b475ccd00000000,
    0x8f1cc69e00000000, 0xccd7601900000000, 0x19e4e2df00000000,
    0x5a2f445800000000, 0xde74de0b00000000, 0x9dbf788c00000000,
    0xd6c3eaac00000000, 0x95084c2b00000000, 0x1153d67800000000,
    0x529870ff00000000, 0xf465465d00000000, 0xb7aee0da00000000,
    0x33f57a8900000000, 0x703edc0e00000000, 0x3b424e2e00000000,
    0x7889e8a900000000, 0xfcd272fa00000000, 0xbf19d47d00000000,
    0x6a2a56bb00000000, 0x29e1f03c00000000, 0xadba6a6f00000000,
    0xee71cce800000000, 0xa50d5ec800000000, 0xe6c6f84f00000000,
    0x629d621c00000000, 0x2156c49b00000000, 0x89fc174a00000000,
    0xca37b1cd00000000, 0x4e6c2b9e00000000, 0x0da78d1900000000,
    0x46db1f3900000000, 0x0510b9be00000000, 0x814b23ed00000000,
    0xc280856a00000000, 0x17b307ac00000000, 0x5478a12b00000000,
    0xd0233b7800000000, 0x93e89dff00000000, 0xd8940fdf00000000,
    0x9b5fa95800000000, 0x1f04330b00000000, 0x5ccf958c00000000,
    0x0e57e57300000000, 0x4d9c43f400000000, 0xc9c7d9a700000000,
    0x8a0c7f2000000000, 0xc170ed0000000000, 0x82bb4b8700000000,
    0x06e0d1d400000000, 0x452b775300000000, 0x9018f59500000000,
    0xd3d3531200000000, 0x5788c94100000000, 0x14436fc600000000,
    0x5f3ffde600000000, 0x1cf45b6100000000, 0x98afc13200000000,
    0xdb6467b500000000, 0x73ceb46400000000, 0x300512e300000000,
    0xb45e88b000000000, 0xf7952e3700000000, 0xbce9bc1700000000,
    0xff221a9000000000, 0x7b7980c300000000, 0x38b2264400000000,
    0xed81a48200000000, 0xae4a020500000000, 0x2a11985600000000,
    0x69da3ed100000000, 0x22a6acf100000000, 0x616d0a7600000000,
    0xe536902500000000, 0xa6fd36a200000000, 0xe8cb8cba00000000,
    0xab002a3d00000000, 0x2f5bb06e00000000, 0x6c9016e900000000,
    0x27ec84c900000000, 0x6427224e00000000, 0xe07cb81d00000000,
    0xa3b71e9a00000000, 0x76849c5c00000000, 0x354f3adb00000000,
    0xb114a08800000000, 0xf2df060f00000000, 0xb9a3942f00000000,
    0xfa6832a800000000, 0x7e33a8fb00000000, 0x3df80e7c00000000,
    0x9552ddad00000000, 0xd6997b2a00000000, 0x52c2e17900000000,
    0x110947fe00000000, 0x5a75d5de00000000, 0x19be735900000000,
    0x9de5e90a00000000, 0xde2e4f8d00000000, 0x0b1dcd4b00000000,
    0x48d66bcc00000000, 0xcc8df19f00000000, 0x8f46571800000000,
    0xc43ac53800000000, 0x87f163bf00000000, 0x03aaf9ec00000000,
    0x40615f6b00000000, 0x12f92f9400000000, 0x5132891300000000,
    0xd569134000000000, 0x96a2b5c700000000, 0xddde27e700000000,
    0x9e15816000000000, 0x1a4e1b3300000000, 0x5985bdb400000000,
    0x8cb63f7200000000, 0xcf7d99f500000000, 0x4b2603a600000000,
    0x08eda52100000000, 0x4391370100000000, 0x005a918600000000,
    0x84010bd500000000, 0xc7caad5200000000, 0x6f607e8300000000,
    0x2cabd80400000000, 0xa8f0425700000000, 0xeb3be4d000000000,
    0xa04776f000000000, 0xe38cd07700000000, 0x67d74a2400000000,
    0x241ceca300000000, 0xf12f6e6500000000, 0xb2e4c8e200000000,
    0x36bf52b100000000, 0x7574f43600000000, 0x3e08661600000000,
    0x7dc3c09100000000, 0xf9985ac200000000, 0xba53fc4500000000,
    0x1caecae700000000, 0x5f656c6000000000, 0xdb3ef63300000000,
    0x98f550b400000000, 0xd389c29400000000, 0x9042641300000000,
    0x1419fe4000000000, 0x57d258c700000000, 0x82e1da0100000000,
    0xc12a7c8600000000, 0x4571e6d500000000, 0x06ba405200000000,
    0x4dc6d27200000000, 0x0e0d74f500000000, 0x8a56eea600000000,
    0xc99d482100000000, 0x61379bf000000000, 0x22fc3d7700000000,
    0xa6a7a72400000000, 0xe56c01a300000000, 0xae10938300000000,
    0xeddb350400000000, 0x6980af5700000000, 0x2a4b09d000000000,
    0xff788b1600000000, 0xbcb32d9100000000, 0x38e8b7c200000000,
    0x7b23114500000000, 0x305f836500000000, 0x739425e200000000,
    0xf7cfbfb100000000, 0xb404193600000000, 0xe69c69c900000000,
    0xa557cf4e00000000, 0x210c551d00000000, 0x62c7f39a00000000,
    0x29bb61ba00000000, 0x6a70c73d00000000, 0xee2b5d6e00000000,
    0xade0fbe900000000, 0x78d3792f00000000, 0x3b18dfa800000000,
    0xbf4345fb00000000, 0xfc88e37c00000000, 0xb7f4715c00000000,
    0xf43fd7db00000000, 0x70644d8800000000, 0x33afeb0f00000000,
    0x9b0538de00000000, 0xd8ce9e5900000000, 0x5c95040a00000000,
    0x1f5ea28d00000000, 0x542230ad00000000, 0x17e9962a00000000,
    0x93b20c7900000000, 0xd079aafe00000000, 0x054a283800000000,
    0x46818ebf00000000, 0xc2da14ec00000000, 0x8111b26b00000000,
    0xca6d204b00000000, 0x89a686cc00000000, 0x0dfd1c9f00000000,
    0x4e36ba1800000000},
   {0x0000000000000000, 0xe1b652ef00000000, 0x836bd40500000000,
    0x62dd86ea00000000, 0x06d7a80b00000000, 0xe761fae400000000,
    0x85bc7c0e00000000, 0x640a2ee100000000, 0x0cae511700000000,
    0xed1803f800000000, 0x8fc5851200000000, 0x6e73d7fd00000000,
    0x0a79f91c00000000, 0xebcfabf300000000, 0x89122d1900000000,
    0x68a47ff600000000, 0x185ca32e00000000, 0xf9eaf1c100000000,
    0x9b37772b00000000, 0x7a8125c400000000, 0x1e8b0b2500000000,
    0xff3d59ca00000000, 0x9de0df2000000000, 0x7c568dcf00000000,
    0x14f2f23900000000, 0xf544a0d600000000, 0x9799263c00000000,
    0x762f74d300000000, 0x12255a3200000000, 0xf39308dd00000000,
    0x914e8e3700000000, 0x70f8dcd800000000, 0x30b8465d00000000,
    0xd10e14b200000000, 0xb3d3925800000000, 0x5265c0b700000000,
    0x366fee5600000000, 0xd7d9bcb900000000, 0xb5043a5300000000,
    0x54b268bc00000000, 0x3c16174a00000000, 0xdda045a500000000,
    0xbf7dc34f00000000, 0x5ecb91a000000000, 0x3ac1bf4100000000,
    0xdb77edae00000000, 0xb9aa6b4400000000, 0x581c39ab00000000,
    0x28e4e57300000000, 0xc952b79c00000000, 0xab8f317600000000,
    0x4a39639900000000, 0x2e334d7800000000, 0xcf851f9700000000,
    0xad58997d00000000, 0x4ceecb9200000000, 0x244ab46400000000,
    0xc5fce68b00000000, 0xa721606100000000, 0x4697328e00000000,
    0x229d1c6f00000000, 0xc32b4e8000000000, 0xa1f6c86a00000000,
    0x40409a8500000000, 0x60708dba00000000, 0x81c6df5500000000,
    0xe31b59bf00000000, 0x02ad0b5000000000, 0x66a725b100000000,
    0x8711775e00000000, 0xe5ccf1b400000000, 0x047aa35b00000000,
    0x6cdedcad00000000, 0x8d688e4200000000, 0xefb508a800000000,
    0x0e035a4700000000, 0x6a0974a600000000, 0x8bbf264900000000,
    0xe962a0a300000000, 0x08d4f24c00000000, 0x782c2e9400000000,
    0x999a7c7b00000000, 0xfb47fa9100000000, 0x1af1a87e00000000,
    0x7efb869f00000000, 0x9f4dd47000000000, 0xfd90529a00000000,
    0x1c26007500000000, 0x74827f8300000000, 0x95342d6c00000000,
    0xf7e9ab8600000000, 0x165ff96900000000, 0x7255d78800000000,
    0x93e3856700000000, 0xf13e038d00000000, 0x1088516200000000,
    0x50c8cbe700000000, 0xb17e990800000000, 0xd3a31fe200000000,
    0x32154d0d00000000, 0x561f63ec00000000, 0xb7a9310300000000,
    0xd574b7e900000000, 0x34c2e50600000000, 0x5c669af000000000,
    0xbdd0c81f00000000, 0xdf0d4ef500000000, 0x3ebb1c1a00000000,
    0x5ab132fb00000000, 0xbb07601400000000, 0xd9dae6fe00000000,
    0x386cb41100000000, 0x489468c900000000, 0xa9223a2600000000,
    0xcbffbccc00000000, 0x2a49ee2300000000, 0x4e43c0c200000000,
    0xaff5922d00000000, 0xcd2814c700000000, 0x2c9e462800000000,
    0x443a39de00000000, 0xa58c6b3100000000, 0xc751eddb00000000,
    0x26e7bf3400000000, 0x42ed91d500000000, 0xa35bc33a00000000,
    0xc18645d000000000, 0x2030173f00000000, 0x81e66bae00000000,
    0x6050394100000000, 0x028dbfab00000000, 0xe33bed4400000000,
    0x8731c3a500000000, 0x6687914a00000000, 0x045a17a000000000,
    0xe5ec454f00000000, 0x8d483ab900000000, 0x6cfe685600000000,
    0x0e23eebc00000000, 0xef95bc5300000000, 0x8b9f92b200000000,
    0x6a29c05d00000000, 0x08f446b700000000, 0xe942145800000000,
    0x99bac88000000000, 0x780c9a6f00000000, 0x1ad11c8500000000,
    0xfb674e6a00000000, 0x9f6d608b00000000, 0x7edb326400000000,
    0x1c06b48e00000000, 0xfdb0e66100000000, 0x9514999700000000,
    0x74a2cb7800000000, 0x167f4d9200000000, 0xf7c91f7d00000000,
    0x93c3319c00000000, 0x7275637300000000, 0x10a8e59900000000,
    0xf11eb77600000000, 0xb15e2df300000000, 0x50e87f1c00000000,
    0x3235f9f600000000, 0xd383ab1900000000, 0xb78985f800000000,
    0x563fd71700000000, 0x34e251fd00000000, 0xd554031200000000,
    0xbdf07ce400000000, 0x5c462e0b00000000, 0x3e9ba8e100000000,
    0xdf2dfa0e00000000, 0xbb27d4ef00000000, 0x5a91860000000000,
    0x384c00ea00000000, 0xd9fa520500000000, 0xa9028edd00000000,
    0x48b4dc3200000000, 0x2a695ad800000000, 0xcbdf083700000000,
    0xafd526d600000000, 0x4e63743900000000, 0x2cbef2d300000000,
    0xcd08a03c00000000, 0xa5acdfca00000000, 0x441a8d2500000000,
    0x26c70bcf00000000, 0xc771592000000000, 0xa37b77c100000000,
    0x42cd252e00000000, 0x2010a3c400000000, 0xc1a6f12b00000000,
    0xe196e61400000000, 0x0020b4fb00000000, 0x62fd321100000000,
    0x834b60fe00000000, 0xe7414e1f00000000, 0x06f71cf000000000,
    0x642a9a1a00000000, 0x859cc8f500000000, 0xed38b70300000000,
    0x0c8ee5ec00000000, 0x6e53630600000000, 0x8fe531e900000000,
    0xebef1f0800000000, 0x0a594de700000000, 0x6884cb0d00000000,
    0x893299e200000000, 0xf9ca453a00000000, 0x187c17d500000000,
    0x7aa1913f00000000, 0x9b17c3d000000000, 0xff1ded3100000000,
    0x1eabbfde00000000, 0x7c76393400000000, 0x9dc06bdb00000000,
    0xf564142d00000000, 0x14d246c200000000, 0x760fc02800000000,
    0x97b992c700000000, 0xf3b3bc2600000000, 0x1205eec900000000,
    0x70d8682300000000, 0x916e3acc00000000, 0xd12ea04900000000,
    0x3098f2a600000000, 0x5245744c00000000, 0xb3f326a300000000,
    0xd7f9084200000000, 0x364f5aad00000000, 0x5492dc4700000000,
    0xb5248ea800000000, 0xdd80f15e00000000, 0x3c36a3b100000000,
    0x5eeb255b00000000, 0xbf5d77b400000000, 0xdb57595500000000,
    0x3ae10bba00000000, 0x583c8d5000000000, 0xb98adfbf00000000,
    0xc972036700000000, 0x28c4518800000000, 0x4a19d76200000000,
    0xabaf858d00000000, 0xcfa5ab6c00000000, 0x2e13f98300000000,
    0x4cce7f6900000000, 0xad782d8600000000, 0xc5dc527000000000,
    0x246a009f00000000, 0x46b7867500000000, 0xa701d49a00000000,
    0xc30bfa7b00000000, 0x22bda89400000000, 0x40602e7e00000000,
    0xa1d67c9100000000},
   {0x0000000000000000, 0x5880e2d700000000, 0xf106b47400000000,
    0xa98656a300000000, 0xe20d68e900000000, 0xba8d8a3e00000000,
    0x130bdc9d00000000, 0x4b8b3e4a00000000, 0x851da10900000000,
    0xdd9d43de00000000, 0x741b157d00000000, 0x2c9bf7aa00000000,
    0x6710c9e000000000, 0x3f902b3700000000, 0x96167d9400000000,
    0xce969f4300000000, 0x0a3b421300000000, 0x52bba0c400000000,
    0xfb3df66700000000, 0xa3bd14b000000000, 0xe8362afa00000000,
    0xb0b6c82d00000000, 0x19309e8e00000000, 0x41b07c5900000000,
    0x8f26e31a00000000, 0xd7a601cd00000000, 0x7e20576e00000000,
    0x26a0b5b900000000, 0x6d2b8bf300000000, 0x35ab692400000000,
    0x9c2d3f8700000000, 0xc4addd5000000000, 0x1476842600000000,
    0x4cf666f100000000, 0xe570305200000000, 0xbdf0d28500000000,
    0xf67beccf00000000, 0xaefb0e1800000000, 0x077d58bb00000000,
    0x5ffdba6c00000000, 0x916b252f00000000, 0xc9ebc7f800000000,
    0x606d915b00000000, 0x38ed738c00000000, 0x73664dc600000000,
    0x2be6af1100000000, 0x8260f9b200000000, 0xdae01b6500000000,
    0x1e4dc63500000000, 0x46cd24e200000000, 0xef4b724100000000,
    0xb7cb909600000000, 0xfc40aedc00000000, 0xa4c04c0b00000000,
    0x0d461aa800000000, 0x55c6f87f00000000, 0x9b50673c00000000,
    0xc3d085eb00000000, 0x6a56d34800000000, 0x32d6319f00000000,
    0x795d0fd500000000, 0x21dded0200000000, 0x885bbba100000000,
    0xd0db597600000000, 0x28ec084d00000000, 0x706cea9a00000000,
    0xd9eabc3900000000, 0x816a5eee00000000, 0xcae160a400000000,
    0x9261827300000000, 0x3be7d4d000000000, 0x6367360700000000,
    0xadf1a94400000000, 0xf5714b9300000000, 0x5cf71d3000000000,
    0x0477ffe700000000, 0x4ffcc1ad00000000, 0x177c237a00000000,
    0xbefa75d900000000, 0xe67a970e00000000, 0x22d74a5e00000000,
    0x7a57a88900000000, 0xd3d1fe2a00000000, 0x8b511cfd00000000,
    0xc0da22b700000000, 0x985ac06000000000, 0x31dc96c300000000,
    0x695c741400000000, 0xa7caeb5700000000, 0xff4a098000000000,
    0x56cc5f2300000000, 0x0e4cbdf400000000, 0x45c783be00000000,
    0x1d47616900000000, 0xb4c137ca00000000, 0xec41d51d00000000,
    0x3c9a8c6b00000000, 0x641a6ebc00000000, 0xcd9c381f00000000,
    0x951cdac800000000, 0xde97e48200000000, 0x8617065500000000,
    0x2f9150f600000000, 0x7711b22100000000, 0xb9872d6200000000,
    0xe107cfb500000000, 0x4881991600000000, 0x10017bc100000000,
    0x5b8a458b00000000, 0x030aa75c00000000, 0xaa8cf1ff00000000,
    0xf20c132800000000, 0x36a1ce7800000000, 0x6e212caf00000000,
    0xc7a77a0c00000000, 0x9f2798db00000000, 0xd4aca69100000000,
    0x8c2c444600000000, 0x25aa12e500000000, 0x7d2af03200000000,
    0xb3bc6f7100000000, 0xeb3c8da600000000, 0x42badb0500000000,
    0x1a3a39d200000000, 0x51b1079800000000, 0x0931e54f00000000,
    0xa0b7b3ec00000000, 0xf837513b00000000, 0x50d8119a00000000,
    0x0858f34d00000000, 0xa1dea5ee00000000, 0xf95e473900000000,
    0xb2d5797300000000, 0xea559ba400000000, 0x43d3cd0700000000,
    0x1b532fd000000000, 0xd5c5b09300000000, 0x8d45524400000000,
    0x24c304e700000000, 0x7c43e63000000000, 0x37c8d87a00000000,
    0x6f483aad00000000, 0xc6ce6c0e00000000, 0x9e4e8ed900000000,
    0x5ae3538900000000, 0x0263b15e00000000, 0xabe5e7fd00000000,
    0xf365052a00000000, 0xb8ee3b6000000000, 0xe06ed9b700000000,
    0x49e88f1400000000, 0x11686dc300000000, 0xdffef28000000000,
    0x877e105700000000, 0x2ef846f400000000, 0x7678a42300000000,
    0x3df39a6900000000, 0x657378be00000000, 0xccf52e1d00000000,
    0x9475ccca00000000, 0x44ae95bc00000000, 0x1c2e776b00000000,
    0xb5a821c800000000, 0xed28c31f00000000, 0xa6a3fd5500000000,
    0xfe231f8200000000, 0x57a5492100000000, 0x0f25abf600000000,
    0xc1b334b500000000, 0x9933d66200000000, 0x30b580c100000000,
    0x6835621600000000, 0x23be5c5c00000000, 0x7b3ebe8b00000000,
    0xd2b8e82800000000, 0x8a380aff00000000, 0x4e95d7af00000000,
    0x1615357800000000, 0xbf9363db00000000, 0xe713810c00000000,
    0xac98bf4600000000, 0xf4185d9100000000, 0x5d9e0b3200000000,
    0x051ee9e500000000, 0xcb8876a600000000, 0x9308947100000000,
    0x3a8ec2d200000000, 0x620e200500000000, 0x29851e4f00000000,
    0x7105fc9800000000, 0xd883aa3b00000000, 0x800348ec00000000,
    0x783419d700000000, 0x20b4fb0000000000, 0x8932ada300000000,
    0xd1b24f7400000000, 0x9a39713e00000000, 0xc2b993e900000000,
    0x6b3fc54a00000000, 0x33bf279d00000000, 0xfd29b8de00000000,
    0xa5a95a0900000000, 0x0c2f0caa00000000, 0x54afee7d00000000,
    0x1f24d03700000000, 0x47a432e000000000, 0xee22644300000000,
    0xb6a2869400000000, 0x720f5bc400000000, 0x2a8fb91300000000,
    0x8309efb000000000, 0xdb890d6700000000, 0x9002332d00000000,
    0xc882d1fa00000000, 0x6104875900000000, 0x3984658e00000000,
    0xf712facd00000000, 0xaf92181a00000000, 0x06144eb900000000,
    0x5e94ac6e00000000, 0x151f922400000000, 0x4d9f70f300000000,
    0xe419265000000000, 0xbc99c48700000000, 0x6c429df100000000,
    0x34c27f2600000000, 0x9d44298500000000, 0xc5c4cb5200000000,
    0x8e4ff51800000000, 0xd6cf17cf00000000, 0x7f49416c00000000,
    0x27c9a3bb00000000, 0xe95f3cf800000000, 0xb1dfde2f00000000,
    0x1859888c00000000, 0x40d96a5b00000000, 0x0b52541100000000,
    0x53d2b6c600000000, 0xfa54e06500000000, 0xa2d402b200000000,
    0x6679dfe200000000, 0x3ef93d3500000000, 0x977f6b9600000000,
    0xcfff894100000000, 0x8474b70b00000000, 0xdcf455dc00000000,
    0x7572037f00000000, 0x2df2e1a800000000, 0xe3647eeb00000000,
    0xbbe49c3c00000000, 0x1262ca9f00000000, 0x4ae2284800000000,
    0x0169160200000000, 0x59e9f4d500000000, 0xf06fa27600000000,
    0xa8ef40a100000000},
   {0x0000000000000000, 0x463b676500000000, 0x8c76ceca00000000,
    0xca4da9af00000000, 0x59ebed4e00000000, 0x1fd08a2b00000000,
    0xd59d238400000000, 0x93a644e100000000, 0xb2d6db9d00000000,
    0xf4edbcf800000000, 0x3ea0155700000000, 0x789b723200000000,
    0xeb3d36d300000000, 0xad0651b600000000, 0x674bf81900000000,
    0x21709f7c00000000, 0x25abc6e000000000, 0x6390a18500000000,
    0xa9dd082a00000000, 0xefe66f4f00000000, 0x7c402bae00000000,
    0x3a7b4ccb00000000, 0xf036e56400000000, 0xb60d820100000000,
    0x977d1d7d00000000, 0xd1467a1800000000, 0x1b0bd3b700000000,
    0x5d30b4d200000000, 0xce96f03300000000, 0x88ad975600000000,
    0x42e03ef900000000, 0x04db599c00000000, 0x0b50fc1a00000000,
    0x4d6b9b7f00000000, 0x872632d000000000, 0xc11d55b500000000,
    0x52bb115400000000, 0x1480763100000000, 0xdecddf9e00000000,
    0x98f6b8fb00000000, 0xb986278700000000, 0xffbd40e200000000,
    0x35f0e94d00000000, 0x73cb8e2800000000, 0xe06dcac900000000,
    0xa656adac00000000, 0x6c1b040300000000, 0x2a20636600000000,
    0x2efb3afa00000000, 0x68c05d9f00000000, 0xa28df43000000000,
    0xe4b6935500000000, 0x7710d7b400000000, 0x312bb0d100000000,
    0xfb66197e00000000, 0xbd5d7e1b00000000, 0x9c2de16700000000,
    0xda16860200000000, 0x105b2fad00000000, 0x566048c800000000,
    0xc5c60c2900000000, 0x83fd6b4c00000000, 0x49b0c2e300000000,
    0x0f8ba58600000000, 0x16a0f83500000000, 0x509b9f5000000000,
    0x9ad636ff00000000, 0xdced519a00000000, 0x4f4b157b00000000,
    0x0970721e00000000, 0xc33ddbb100000000, 0x8506bcd400000000,
    0xa47623a800000000, 0xe24d44cd00000000, 0x2800ed6200000000,
    0x6e3b8a0700000000, 0xfd9dcee600000000, 0xbba6a98300000000,
    0x71eb002c00000000, 0x37d0674900000000, 0x330b3ed500000000,
    0x753059b000000000, 0xbf7df01f00000000, 0xf946977a00000000,
    0x6ae0d39b00000000, 0x2cdbb4fe00000000, 0xe6961d5100000000,
    0xa0ad7a3400000000, 0x81dde54800000000, 0xc7e6822d00000000,
    0x0dab2b8200000000, 0x4b904ce700000000, 0xd836080600000000,
    0x9e0d6f6300000000, 0x5440c6cc00000000, 0x127ba1a900000000,
    0x1df0042f00000000, 0x5bcb634a00000000, 0x9186cae500000000,
    0xd7bdad8000000000, 0x441be96100000000, 0x02208e0400000000,
    0xc86d27ab00000000, 0x8e5640ce00000000, 0xaf26dfb200000000,
    0xe91db8d700000000, 0x2350117800000000, 0x656b761d00000000,
    0xf6cd32fc00000000, 0xb0f6559900000000, 0x7abbfc3600000000,
    0x3c809b5300000000, 0x385bc2cf00000000, 0x7e60a5aa00000000,
    0xb42d0c0500000000, 0xf2166b6000000000, 0x61b02f8100000000,
    0x278b48e400000000, 0xedc6e14b00000000, 0xabfd862e00000000,
    0x8a8d195200000000, 0xccb67e3700000000, 0x06fbd79800000000,
    0x40c0b0fd00000000, 0xd366f41c00000000, 0x955d937900000000,
    0x5f103ad600000000, 0x192b5db300000000, 0x2c40f16b00000000,
    0x6a7b960e00000000, 0xa0363fa100000000, 0xe60d58c400000000,
    0x75ab1c2500000000, 0x33907b4000000000, 0xf9ddd2ef00000000,
    0xbfe6b58a00000000, 0x9e962af600000000, 0xd8ad4d9300000000,
    0x12e0e43c00000000, 0x54db835900000000, 0xc77dc7b800000000,
    0x8146a0dd00000000, 0x4b0b097200000000, 0x0d306e1700000000,
    0x09eb378b00000000, 0x4fd050ee00000000, 0x859df94100000000,
    0xc3a69e2400000000, 0x5000dac500000000, 0x163bbda000000000,
    0xdc76140f00000000, 0x9a4d736a00000000, 0xbb3dec1600000000,
    0xfd068b7300000000, 0x374b22dc00000000, 0x717045b900000000,
    0xe2d6015800000000, 0xa4ed663d00000000, 0x6ea0cf9200000000,
    0x289ba8f700000000, 0x27100d7100000000, 0x612b6a1400000000,
    0xab66c3bb00000000, 0xed5da4de00000000, 0x7efbe03f00000000,
    0x38c0875a00000000, 0xf28d2ef500000000, 0xb4b6499000000000,
    0x95c6d6ec00000000, 0xd3fdb18900000000, 0x19b0182600000000,
    0x5f8b7f4300000000, 0xcc2d3ba200000000, 0x8a165cc700000000,
    0x405bf56800000000, 0x0660920d00000000, 0x02bbcb9100000000,
    0x4480acf400000000, 0x8ecd055b00000000, 0xc8f6623e00000000,
    0x5b5026df00000000, 0x1d6b41ba00000000, 0xd726e81500000000,
    0x911d8f7000000000, 0xb06d100c00000000, 0xf656776900000000,
    0x3c1bdec600000000, 0x7a20b9a300000000, 0xe986fd4200000000,
    0xafbd9a2700000000, 0x65f0338800000000, 0x23cb54ed00000000,
    0x3ae0095e00000000, 0x7cdb6e3b00000000, 0xb696c79400000000,
    0xf0ada0f100000000, 0x630be41000000000, 0x2530837500000000,
    0xef7d2ada00000000, 0xa9464dbf00000000, 0x8836d2c300000000,
    0xce0db5a600000000, 0x04401c0900000000, 0x427b7b6c00000000,
    0xd1dd3f8d00000000, 0x97e658e800000000, 0x5dabf14700000000,
    0x1b90962200000000, 0x1f4bcfbe00000000, 0x5970a8db00000000,
    0x933d017400000000, 0xd506661100000000, 0x46a022f000000000,
    0x009b459500000000, 0xcad6ec3a00000000, 0x8ced8b5f00000000,
    0xad9d142300000000, 0xeba6734600000000, 0x21ebdae900000000,
    0x67d0bd8c00000000, 0xf476f96d00000000, 0xb24d9e0800000000,
    0x780037a700000000, 0x3e3b50c200000000, 0x31b0f54400000000,
    0x778b922100000000, 0xbdc63b8e00000000, 0xfbfd5ceb00000000,
    0x685b180a00000000, 0x2e607f6f00000000, 0xe42dd6c000000000,
    0xa216b1a500000000, 0x83662ed900000000, 0xc55d49bc00000000,
    0x0f10e01300000000, 0x492b877600000000, 0xda8dc39700000000,
    0x9cb6a4f200000000, 0x56fb0d5d00000000, 0x10c06a3800000000,
    0x141b33a400000000, 0x522054c100000000, 0x986dfd6e00000000,
    0xde569a0b00000000, 0x4df0deea00000000, 0x0bcbb98f00000000,
    0xc186102000000000, 0x87bd774500000000, 0xa6cde83900000000,
    0xe0f68f5c00000000, 0x2abb26f300000000, 0x6c80419600000000,
    0xff26057700000000, 0xb91d621200000000, 0x7350cbbd00000000,
    0x356bacd800000000},
   {0x0000000000000000, 0x9e83da9f00000000, 0x7d01c4e400000000,
    0xe3821e7b00000000, 0xbb04f91200000000, 0x2587238d00000000,
    0xc6053df600000000, 0x5886e76900000000, 0x7609f22500000000,
    0xe88a28ba00000000, 0x0b0836c100000000, 0x958bec5e00000000,
    0xcd0d0b3700000000, 0x538ed1a800000000, 0xb00ccfd300000000,
    0x2e8f154c00000000, 0xec12e44b00000000, 0x72913ed400000000,
    0x911320af00000000, 0x0f90fa3000000000, 0x57161d5900000000,
    0xc995c7c600000000, 0x2a17d9bd00000000, 0xb494032200000000,
    0x9a1b166e00000000, 0x0498ccf100000000, 0xe71ad28a00000000,
    0x7999081500000000, 0x211fef7c00000000, 0xbf9c35e300000000,
    0x5c1e2b9800000000, 0xc29df10700000000, 0xd825c89700000000,
    0x46a6120800000000, 0xa5240c7300000000, 0x3ba7d6ec00000000,
    0x6321318500000000, 0xfda2eb1a00000000, 0x1e20f56100000000,
    0x80a32ffe00000000, 0xae2c3ab200000000, 0x30afe02d00000000,
    0xd32dfe5600000000, 0x4dae24c900000000, 0x1528c3a000000000,
    0x8bab193f00000000, 0x6829074400000000, 0xf6aadddb00000000,
    0x34372cdc00000000, 0xaab4f64300000000, 0x4936e83800000000,
    0xd7b532a700000000, 0x8f33d5ce00000000, 0x11b00f5100000000,
    0xf232112a00000000, 0x6cb1cbb500000000, 0x423edef900000000,
    0xdcbd046600000000, 0x3f3f1a1d00000000, 0xa1bcc08200000000,
    0xf93a27eb00000000, 0x67b9fd7400000000, 0x843be30f00000000,
    0x1ab8399000000000, 0xf14de1f400000000, 0x6fce3b6b00000000,
    0x8c4c251000000000, 0x12cfff8f00000000, 0x4a4918e600000000,
    0xd4cac27900000000, 0x3748dc0200000000, 0xa9cb069d00000000,
    0x874413d100000000, 0x19c7c94e00000000, 0xfa45d73500000000,
    0x64c60daa00000000, 0x3c40eac300000000, 0xa2c3305c00000000,
    0x41412e2700000000, 0xdfc2f4b800000000, 0x1d5f05bf00000000,
    0x83dcdf2000000000, 0x605ec15b00000000, 0xfedd1bc400000000,
    0xa65bfcad00000000, 0x38d8263200000000, 0xdb5a384900000000,
    0x45d9e2d600000000, 0x6b56f79a00000000, 0xf5d52d0500000000,
    0x1657337e00000000, 0x88d4e9e100000000, 0xd0520e8800000000,
    0x4ed1d41700000000, 0xad53ca6c00000000, 0x33d010f300000000,
    0x2968296300000000, 0xb7ebf3fc00000000, 0x5469ed8700000000,
    0xcaea371800000000, 0x926cd07100000000, 0x0cef0aee00000000,
    0xef6d149500000000, 0x71eece0a00000000, 0x5f61db4600000000,
    0xc1e201d900000000, 0x22601fa200000000, 0xbce3c53d00000000,
    0xe465225400000000, 0x7ae6f8cb00000000, 0x9964e6b000000000,
    0x07e73c2f00000000, 0xc57acd2800000000, 0x5bf917b700000000,
    0xb87b09cc00000000, 0x26f8d35300000000, 0x7e7e343a00000000,
    0xe0fdeea500000000, 0x037ff0de00000000, 0x9dfc2a4100000000,
    0xb3733f0d00000000, 0x2df0e59200000000, 0xce72fbe900000000,
    0x50f1217600000000, 0x0877c61f00000000, 0x96f41c8000000000,
    0x757602fb00000000, 0xebf5d86400000000, 0xa39db33200000000,
    0x3d1e69ad00000000, 0xde9c77d600000000, 0x401fad4900000000,
    0x18994a2000000000, 0x861a90bf00000000, 0x65988ec400000000,
    0xfb1b545b00000000, 0xd594411700000000, 0x4b179b8800000000,
    0xa89585f300000000, 0x36165f6c00000000, 0x6e90b80500000000,
    0xf013629a00000000, 0x13917ce100000000, 0x8d12a67e00000000,
    0x4f8f577900000000, 0xd10c8de600000000, 0x328e939d00000000,
    0xac0d490200000000, 0xf48bae6b00000000, 0x6a0874f400000000,
    0x898a6a8f00000000, 0x1709b01000000000, 0x3986a55c00000000,
    0xa7057fc300000000, 0x448761b800000000, 0xda04bb2700000000,
    0x82825c4e00000000, 0x1c0186d100000000, 0xff8398aa00000000,
    0x6100423500000000, 0x7bb87ba500000000, 0xe53ba13a00000000,
    0x06b9bf4100000000, 0x983a65de00000000, 0xc0bc82b700000000,
    0x5e3f582800000000, 0xbdbd465300000000, 0x233e9ccc00000000,
    0x0db1898000000000, 0x9332531f00000000, 0x70b04d6400000000,
    0xee3397fb00000000, 0xb6b5709200000000, 0x2836aa0d00000000,
    0xcbb4b47600000000, 0x55376ee900000000, 0x97aa9fee00000000,
    0x0929457100000000, 0xeaab5b0a00000000, 0x7428819500000000,
    0x2cae66fc00000000, 0xb22dbc6300000000, 0x51afa21800000000,
    0xcf2c788700000000, 0xe1a36dcb00000000, 0x7f20b75400000000,
    0x9ca2a92f00000000, 0x022173b000000000, 0x5aa794d900000000,
    0xc4244e4600000000, 0x27a6503d00000000, 0xb9258aa200000000,
    0x52d052c600000000, 0xcc53885900000000, 0x2fd1962200000000,
    0xb1524cbd00000000, 0xe9d4abd400000000, 0x7757714b00000000,
    0x94d56f3000000000, 0x0a56b5af00000000, 0x24d9a0e300000000,
    0xba5a7a7c00000000, 0x59d8640700000000, 0xc75bbe9800000000,
    0x9fdd59f100000000, 0x015e836e00000000, 0xe2dc9d1500000000,
    0x7c5f478a00000000, 0xbec2b68d00000000, 0x20416c1200000000,
    0xc3c3726900000000, 0x5d40a8f600000000, 0x05c64f9f00000000,
    0x9b45950000000000, 0x78c78b7b00000000, 0xe64451e400000000,
    0xc8cb44a800000000, 0x56489e3700000000, 0xb5ca804c00000000,
    0x2b495ad300000000, 0x73cfbdba00000000, 0xed4c672500000000,
    0x0ece795e00000000, 0x904da3c100000000, 0x8af59a5100000000,
    0x147640ce00000000, 0xf7f45eb500000000, 0x6977842a00000000,
    0x31f1634300000000, 0xaf72b9dc00000000, 0x4cf0a7a700000000,
    0xd2737d3800000000, 0xfcfc687400000000, 0x627fb2eb00000000,
    0x81fdac9000000000, 0x1f7e760f00000000, 0x47f8916600000000,
    0xd97b4bf900000000, 0x3af9558200000000, 0xa47a8f1d00000000,
    0x66e77e1a00000000, 0xf864a48500000000, 0x1be6bafe00000000,
    0x8565606100000000, 0xdde3870800000000, 0x43605d9700000000,
    0xa0e243ec00000000, 0x3e61997300000000, 0x10ee8c3f00000000,
    0x8e6d56a000000000, 0x6def48db00000000, 0xf36c924400000000,
    0xabea752d00000000, 0x3569afb200000000, 0xd6ebb1c900000000,
    0x48686b5600000000},
   {0x0000000000000000, 0xc064281700000000, 0x80c9502e00000000,
    0x40ad783900000000, 0x0093a15c00000000, 0xc0f7894b00000000,
    0x805af17200000000, 0x403ed96500000000, 0x002643b900000000,
    0xc0426bae00000000, 0x80ef139700000000, 0x408b3b8000000000,
    0x00b5e2e500000000, 0xc0d1caf200000000, 0x807cb2cb00000000,
    0x40189adc00000000, 0x414af7a900000000, 0x812edfbe00000000,
    0xc183a78700000000, 0x01e78f9000000000, 0x41d956f500000000,
    0x81bd7ee200000000, 0xc11006db00000000, 0x01742ecc00000000,
    0x416cb41000000000, 0x81089c0700000000, 0xc1a5e43e00000000,
    0x01c1cc2900000000, 0x41ff154c00000000, 0x819b3d5b00000000,
    0xc136456200000000, 0x01526d7500000000, 0xc3929f8800000000,
    0x03f6b79f00000000, 0x435bcfa600000000, 0x833fe7b100000000,
    0xc3013ed400000000, 0x036516c300000000, 0x43c86efa00000000,
    0x83ac46ed00000000, 0xc3b4dc3100000000, 0x03d0f42600000000,
    0x437d8c1f00000000, 0x8319a40800000000, 0xc3277d6d00000000,
    0x0343557a00000000, 0x43ee2d4300000000, 0x838a055400000000,
    0x82d8682100000000, 0x42bc403600000000, 0x0211380f00000000,
    0xc275101800000000, 0x824bc97d00000000, 0x422fe16a00000000,
    0x0282995300000000, 0xc2e6b14400000000, 0x82fe2b9800000000,
    0x429a038f00000000, 0x02377bb600000000, 0xc25353a100000000,
    0x826d8ac400000000, 0x4209a2d300000000, 0x02a4daea00000000,
    0xc2c0f2fd00000000, 0xc7234eca00000000, 0x074766dd00000000,
    0x47ea1ee400000000, 0x878e36f300000000, 0xc7b0ef9600000000,
    0x07d4c78100000000, 0x4779bfb800000000, 0x871d97af00000000,
    0xc7050d7300000000, 0x0761256400000000, 0x47cc5d5d00000000,
    0x87a8754a00000000, 0xc796ac2f00000000, 0x07f2843800000000,
    0x475ffc0100000000, 0x873bd41600000000, 0x8669b96300000000,
    0x460d917400000000, 0x06a0e94d00000000, 0xc6c4c15a00000000,
    0x86fa183f00000000, 0x469e302800000000, 0x0633481100000000,
    0xc657600600000000, 0x864ffada00000000, 0x462bd2cd00000000,
    0x0686aaf400000000, 0xc6e282e300000000, 0x86dc5b8600000000,
    0x46b8739100000000, 0x06150ba800000000, 0xc67123bf00000000,
    0x04b1d14200000000, 0xc4d5f95500000000, 0x8478816c00000000,
    0x441ca97b00000000, 0x0422701e00000000, 0xc446580900000000,
    0x84eb203000000000, 0x448f082700000000, 0x049792fb00000000,
    0xc4f3baec00000000, 0x845ec2d500000000, 0x443aeac200000000,
    0x040433a700000000, 0xc4601bb000000000, 0x84cd638900000000,
    0x44a94b9e00000000, 0x45fb26eb00000000, 0x859f0efc00000000,
    0xc53276c500000000, 0x05565ed200000000, 0x456887b700000000,
    0x850cafa000000000, 0xc5a1d79900000000, 0x05c5ff8e00000000,
    0x45dd655200000000, 0x85b94d4500000000, 0xc514357c00000000,
    0x05701d6b00000000, 0x454ec40e00000000, 0x852aec1900000000,
    0xc587942000000000, 0x05e3bc3700000000, 0xcf41ed4f00000000,
    0x0f25c55800000000, 0x4f88bd6100000000, 0x8fec957600000000,
    0xcfd24c1300000000, 0x0fb6640400000000, 0x4f1b1c3d00000000,
    0x8f7f342a00000000, 0xcf67aef600000000, 0x0f0386e100000000,
    0x4faefed800000000, 0x8fcad6cf00000000, 0xcff40faa00000000,
    0x0f9027bd00000000, 0x4f3d5f8400000000, 0x8f59779300000000,
    0x8e0b1ae600000000, 0x4e6f32f100000000, 0x0ec24ac800000000,
    0xcea662df00000000, 0x8e98bbba00000000, 0x4efc93ad00000000,
    0x0e51eb9400000000, 0xce35c38300000000, 0x8e2d595f00000000,
    0x4e49714800000000, 0x0ee4097100000000, 0xce80216600000000,
    0x8ebef80300000000, 0x4edad01400000000, 0x0e77a82d00000000,
    0xce13803a00000000, 0x0cd372c700000000, 0xccb75ad000000000,
    0x8c1a22e900000000, 0x4c7e0afe00000000, 0x0c40d39b00000000,
    0xcc24fb8c00000000, 0x8c8983b500000000, 0x4cedaba200000000,
    0x0cf5317e00000000, 0xcc91196900000000, 0x8c3c615000000000,
    0x4c58494700000000, 0x0c66902200000000, 0xcc02b83500000000,
    0x8cafc00c00000000, 0x4ccbe81b00000000, 0x4d99856e00000000,
    0x8dfdad7900000000, 0xcd50d54000000000, 0x0d34fd5700000000,
    0x4d0a243200000000, 0x8d6e0c2500000000, 0xcdc3741c00000000,
    0x0da75c0b00000000, 0x4dbfc6d700000000, 0x8ddbeec000000000,
    0xcd7696f900000000, 0x0d12beee00000000, 0x4d2c678b00000000,
    0x8d484f9c00000000, 0xcde537a500000000, 0x0d811fb200000000,
    0x0862a38500000000, 0xc8068b9200000000, 0x88abf3ab00000000,
    0x48cfdbbc00000000, 0x08f102d900000000, 0xc8952ace00000000,
    0x883852f700000000, 0x485c7ae000000000, 0x0844e03c00000000,
    0xc820c82b00000000, 0x888db01200000000, 0x48e9980500000000,
    0x08d7416000000000, 0xc8b3697700000000, 0x881e114e00000000,
    0x487a395900000000, 0x4928542c00000000, 0x894c7c3b00000000,
    0xc9e1040200000000, 0x09852c1500000000, 0x49bbf57000000000,
    0x89dfdd6700000000, 0xc972a55e00000000, 0x09168d4900000000,
    0x490e179500000000, 0x896a3f8200000000, 0xc9c747bb00000000,
    0x09a36fac00000000, 0x499db6c900000000, 0x89f99ede00000000,
    0xc954e6e700000000, 0x0930cef000000000, 0xcbf03c0d00000000,
    0x0b94141a00000000, 0x4b396c2300000000, 0x8b5d443400000000,
    0xcb639d5100000000, 0x0b07b54600000000, 0x4baacd7f00000000,
    0x8bcee56800000000, 0xcbd67fb400000000, 0x0bb257a300000000,
    0x4b1f2f9a00000000, 0x8b7b078d00000000, 0xcb45dee800000000,
    0x0b21f6ff00000000, 0x4b8c8ec600000000, 0x8be8a6d100000000,
    0x8abacba400000000, 0x4adee3b300000000, 0x0a739b8a00000000,
    0xca17b39d00000000, 0x8a296af800000000, 0x4a4d42ef00000000,
    0x0ae03ad600000000, 0xca8412c100000000, 0x8a9c881d00000000,
    0x4af8a00a00000000, 0x0a55d83300000000, 0xca31f02400000000,
    0x8a0f294100000000, 0x4a6b015600000000, 0x0ac6796f00000000,
    0xcaa2517800000000},
   {0x0000000000000000, 0xd4ea739b00000000, 0xe9d396ed00000000,
    0x3d39e57600000000, 0x93a15c0000000000, 0x474b2f9b00000000,
    0x7a72caed00000000, 0xae98b97600000000, 0x2643b90000000000,
    0xf2a9ca9b00000000, 0xcf902fed00000000, 0x1b7a5c7600000000,
    0xb5e2e50000000000, 0x6108969b00000000, 0x5c3173ed00000000,
    0x88db007600000000, 0x4c86720100000000, 0x986c019a00000000,
    0xa555e4ec00000000, 0x71bf977700000000, 0xdf272e0100000000,
    0x0bcd5d9a00000000, 0x36f4b8ec00000000, 0xe21ecb7700000000,
    0x6ac5cb0100000000, 0xbe2fb89a00000000, 0x83165dec00000000,
    0x57fc2e7700000000, 0xf964970100000000, 0x2d8ee49a00000000,
    0x10b701ec00000000, 0xc45d727700000000, 0x980ce50200000000,
    0x4ce6969900000000, 0x71df73ef00000000, 0xa535007400000000,
    0x0badb90200000000, 0xdf47ca9900000000, 0xe27e2fef00000000,
    0x36945c7400000000, 0xbe4f5c0200000000, 0x6aa52f9900000000,
    0x579ccaef00000000, 0x8376b97400000000, 0x2dee000200000000,
    0xf904739900000000, 0xc43d96ef00000000, 0x10d7e57400000000,
    0xd48a970300000000, 0x0060e49800000000, 0x3d5901ee00000000,
    0xe9b3727500000000, 0x472bcb0300000000, 0x93c1b89800000000,
    0xaef85dee00000000, 0x7a122e7500000000, 0xf2c92e0300000000,
    0x26235d9800000000, 0x1b1ab8ee00000000, 0xcff0cb7500000000,
    0x6168720300000000, 0xb582019800000000, 0x88bbe4ee00000000,
    0x5c51977500000000, 0x3019ca0500000000, 0xe4f3b99e00000000,
    0xd9ca5ce800000000, 0x0d202f7300000000, 0xa3b8960500000000,
    0x7752e59e00000000, 0x4a6b00e800000000, 0x9e81737300000000,
    0x165a730500000000, 0xc2b0009e00000000, 0xff89e5e800000000,
    0x2b63967300000000, 0x85fb2f0500000000, 0x51115c9e00000000,
    0x6c28b9e800000000, 0xb8c2ca7300000000, 0x7c9fb80400000000,
    0xa875cb9f00000000, 0x954c2ee900000000, 0x41a65d7200000000,
    0xef3ee40400000000, 0x3bd4979f00000000, 0x06ed72e900000000,
    0xd207017200000000, 0x5adc010400000000, 0x8e36729f00000000,
    0xb30f97e900000000, 0x67e5e47200000000, 0xc97d5d0400000000,
    0x1d972e9f00000000, 0x20aecbe900000000, 0xf444b87200000000,
    0xa8152f0700000000, 0x7cff5c9c00000000, 0x41c6b9ea00000000,
    0x952cca7100000000, 0x3bb4730700000000, 0xef5e009c00000000,
    0xd267e5ea00000000, 0x068d967100000000, 0x8e56960700000000,
    0x5abce59c00000000, 0x678500ea00000000, 0xb36f737100000000,
    0x1df7ca0700000000, 0xc91db99c00000000, 0xf4245cea00000000,
    0x20ce2f7100000000, 0xe4935d0600000000, 0x30792e9d00000000,
    0x0d40cbeb00000000, 0xd9aab87000000000, 0x7732010600000000,
    0xa3d8729d00000000, 0x9ee197eb00000000, 0x4a0be47000000000,
    0xc2d0e40600000000, 0x163a979d00000000, 0x2b0372eb00000000,
    0xffe9017000000000, 0x5171b80600000000, 0x859bcb9d00000000,
    0xb8a22eeb00000000, 0x6c485d7000000000, 0x6032940b00000000,
    0xb4d8e79000000000, 0x89e102e600000000, 0x5d0b717d00000000,
    0xf393c80b00000000, 0x2779bb9000000000, 0x1a405ee600000000,
    0xceaa2d7d00000000, 0x46712d0b00000000, 0x929b5e9000000000,
    0xafa2bbe600000000, 0x7b48c87d00000000, 0xd5d0710b00000000,
    0x013a029000000000, 0x3c03e7e600000000, 0xe8e9947d00000000,
    0x2cb4e60a00000000, 0xf85e959100000000, 0xc56770e700000000,
    0x118d037c00000000, 0xbf15ba0a00000000, 0x6bffc99100000000,
    0x56c62ce700000000, 0x822c5f7c00000000, 0x0af75f0a00000000,
    0xde1d2c9100000000, 0xe324c9e700000000, 0x37ceba7c00000000,
    0x9956030a00000000, 0x4dbc709100000000, 0x708595e700000000,
    0xa46fe67c00000000, 0xf83e710900000000, 0x2cd4029200000000,
    0x11ede7e400000000, 0xc507947f00000000, 0x6b9f2d0900000000,
    0xbf755e9200000000, 0x824cbbe400000000, 0x56a6c87f00000000,
    0xde7dc80900000000, 0x0a97bb9200000000, 0x37ae5ee400000000,
    0xe3442d7f00000000, 0x4ddc940900000000, 0x9936e79200000000,
    0xa40f02e400000000, 0x70e5717f00000000, 0xb4b8030800000000,
    0x6052709300000000, 0x5d6b95e500000000, 0x8981e67e00000000,
    0x27195f0800000000, 0xf3f32c9300000000, 0xcecac9e500000000,
    0x1a20ba7e00000000, 0x92fbba0800000000, 0x4611c99300000000,
    0x7b282ce500000000, 0xafc25f7e00000000, 0x015ae60800000000,
    0xd5b0959300000000, 0xe88970e500000000, 0x3c63037e00000000,
    0x502b5e0e00000000, 0x84c12d9500000000, 0xb9f8c8e300000000,
    0x6d12bb7800000000, 0xc38a020e00000000, 0x1760719500000000,
    0x2a5994e300000000, 0xfeb3e77800000000, 0x7668e70e00000000,
    0xa282949500000000, 0x9fbb71e300000000, 0x4b51027800000000,
    0xe5c9bb0e00000000, 0x3123c89500000000, 0x0c1a2de300000000,
    0xd8f05e7800000000, 0x1cad2c0f00000000, 0xc8475f9400000000,
    0xf57ebae200000000, 0x2194c97900000000, 0x8f0c700f00000000,
    0x5be6039400000000, 0x66dfe6e200000000, 0xb235957900000000,
    0x3aee950f00000000, 0xee04e69400000000, 0xd33d03e200000000,
    0x07d7707900000000, 0xa94fc90f00000000, 0x7da5ba9400000000,
    0x409c5fe200000000, 0x94762c7900000000, 0xc827bb0c00000000,
    0x1ccdc89700000000, 0x21f42de100000000, 0xf51e5e7a00000000,
    0x5b86e70c00000000, 0x8f6c949700000000, 0xb25571e100000000,
    0x66bf027a00000000, 0xee64020c00000000, 0x3a8e719700000000,
    0x07b794e100000000, 0xd35de77a00000000, 0x7dc55e0c00000000,
    0xa92f2d9700000000, 0x9416c8e100000000, 0x40fcbb7a00000000,
    0x84a1c90d00000000, 0x504bba9600000000, 0x6d725fe000000000,
    0xb9982c7b00000000, 0x1700950d00000000, 0xc3eae69600000000,
    0xfed303e000000000, 0x2a39707b00000000, 0xa2e2700d00000000,
    0x7608039600000000, 0x4b31e6e000000000, 0x9fdb957b00000000,
    0x31432c0d00000000, 0xe5a95f9600000000, 0xd890bae000000000,
    0x0c7ac97b00000000},
   {0x0000000000000000, 0x2765258100000000, 0x0fcc3bd900000000,
    0x28a91e5800000000, 0x5f9e066900000000, 0x78fb23e800000000,
    0x50523db000000000, 0x7737183100000000, 0xbe3c0dd200000000,
    0x9959285300000000, 0xb1f0360b00000000, 0x9695138a00000000,
    0xe1a20bbb00000000, 0xc6c72e3a00000000, 0xee6e306200000000,
    0xc90b15e300000000, 0x3d7f6b7f00000000, 0x1a1a4efe00000000,
    0x32b350a600000000, 0x15d6752700000000, 0x62e16d1600000000,
    0x4584489700000000, 0x6d2d56cf00000000, 0x4a48734e00000000,
    0x834366ad00000000, 0xa426432c00000000, 0x8c8f5d7400000000,
    0xabea78f500000000, 0xdcdd60c400000000, 0xfbb8454500000000,
    0xd3115b1d00000000, 0xf4747e9c00000000, 0x7afed6fe00000000,
    0x5d9bf37f00000000, 0x7532ed2700000000, 0x5257c8a600000000,
    0x2560d09700000000, 0x0205f51600000000, 0x2aaceb4e00000000,
    0x0dc9cecf00000000, 0xc4c2db2c00000000, 0xe3a7fead00000000,
    0xcb0ee0f500000000, 0xec6bc57400000000, 0x9b5cdd4500000000,
    0xbc39f8c400000000, 0x9490e69c00000000, 0xb3f5c31d00000000,
    0x4781bd8100000000, 0x60e4980000000000, 0x484d865800000000,
    0x6f28a3d900000000, 0x181fbbe800000000, 0x3f7a9e6900000000,
    0x17d3803100000000, 0x30b6a5b000000000, 0xf9bdb05300000000,
    0xded895d200000000, 0xf6718b8a00000000, 0xd114ae0b00000000,
    0xa623b63a00000000, 0x814693bb00000000, 0xa9ef8de300000000,
    0x8e8aa86200000000, 0xb5fadc2600000000, 0x929ff9a700000000,
    0xba36e7ff00000000, 0x9d53c27e00000000, 0xea64da4f00000000,
    0xcd01ffce00000000, 0xe5a8e19600000000, 0xc2cdc41700000000,
    0x0bc6d1f400000000, 0x2ca3f47500000000, 0x040aea2d00000000,
    0x236fcfac00000000, 0x5458d79d00000000, 0x733df21c00000000,
    0x5b94ec4400000000, 0x7cf1c9c500000000, 0x8885b75900000000,
    0xafe092d800000000, 0x87498c8000000000, 0xa02ca90100000000,
    0xd71bb13000000000, 0xf07e94b100000000, 0xd8d78ae900000000,
    0xffb2af6800000000, 0x36b9ba8b00000000, 0x11dc9f0a00000000,
    0x3975815200000000, 0x1e10a4d300000000, 0x6927bce200000000,
    0x4e42996300000000, 0x66eb873b00000000, 0x418ea2ba00000000,
    0xcf040ad800000000, 0xe8612f5900000000, 0xc0c8310100000000,
    0xe7ad148000000000, 0x909a0cb100000000, 0xb7ff293000000000,
    0x9f56376800000000, 0xb83312e900000000, 0x7138070a00000000,
    0x565d228b00000000, 0x7ef43cd300000000, 0x5991195200000000,
    0x2ea6016300000000, 0x09c324e200000000, 0x216a3aba00000000,
    0x060f1f3b00000000, 0xf27b61a700000000, 0xd51e442600000000,
    0xfdb75a7e00000000, 0xdad27fff00000000, 0xade567ce00000000,
    0x8a80424f00000000, 0xa2295c1700000000, 0x854c799600000000,
    0x4c476c7500000000, 0x6b2249f400000000, 0x438b57ac00000000,
    0x64ee722d00000000, 0x13d96a1c00000000, 0x34bc4f9d00000000,
    0x1c1551c500000000, 0x3b70744400000000, 0x6af5b94d00000000,
    0x4d909ccc00000000, 0x6539829400000000, 0x425ca71500000000,
    0x356bbf2400000000, 0x120e9aa500000000, 0x3aa784fd00000000,
    0x1dc2a17c00000000, 0xd4c9b49f00000000, 0xf3ac911e00000000,
    0xdb058f4600000000, 0xfc60aac700000000, 0x8b57b2f600000000,
    0xac32977700000000, 0x849b892f00000000, 0xa3feacae00000000,
    0x578ad23200000000, 0x70eff7b300000000, 0x5846e9eb00000000,
    0x7f23cc6a00000000, 0x0814d45b00000000, 0x2f71f1da00000000,
    0x07d8ef8200000000, 0x20bdca0300000000, 0xe9b6dfe000000000,
    0xced3fa6100000000, 0xe67ae43900000000, 0xc11fc1b800000000,
    0xb628d98900000000, 0x914dfc0800000000, 0xb9e4e25000000000,
    0x9e81c7d100000000, 0x100b6fb300000000, 0x376e4a3200000000,
    0x1fc7546a00000000, 0x38a271eb00000000, 0x4f9569da00000000,
    0x68f04c5b00000000, 0x4059520300000000, 0x673c778200000000,
    0xae37626100000000, 0x895247e000000000, 0xa1fb59b800000000,
    0x869e7c3900000000, 0xf1a9640800000000, 0xd6cc418900000000,
    0xfe655fd100000000, 0xd9007a5000000000, 0x2d7404cc00000000,
    0x0a11214d00000000, 0x22b83f1500000000, 0x05dd1a9400000000,
    0x72ea02a500000000, 0x558f272400000000, 0x7d26397c00000000,
    0x5a431cfd00000000, 0x9348091e00000000, 0xb42d2c9f00000000,
    0x9c8432c700000000, 0xbbe1174600000000, 0xccd60f7700000000,
    0xebb32af600000000, 0xc31a34ae00000000, 0xe47f112f00000000,
    0xdf0f656b00000000, 0xf86a40ea00000000, 0xd0c35eb200000000,
    0xf7a67b3300000000, 0x8091630200000000, 0xa7f4468300000000,
    0x8f5d58db00000000, 0xa8387d5a00000000, 0x613368b900000000,
    0x46564d3800000000, 0x6eff536000000000, 0x499a76e100000000,
    0x3ead6ed000000000, 0x19c84b5100000000, 0x3161550900000000,
    0x1604708800000000, 0xe2700e1400000000, 0xc5152b9500000000,
    0xedbc35cd00000000, 0xcad9104c00000000, 0xbdee087d00000000,
    0x9a8b2dfc00000000, 0xb22233a400000000, 0x9547162500000000,
    0x5c4c03c600000000, 0x7b29264700000000, 0x5380381f00000000,
    0x74e51d9e00000000, 0x03d205af00000000, 0x24b7202e00000000,
    0x0c1e3e7600000000, 0x2b7b1bf700000000, 0xa5f1b39500000000,
    0x8294961400000000, 0xaa3d884c00000000, 0x8d58adcd00000000,
    0xfa6fb5fc00000000, 0xdd0a907d00000000, 0xf5a38e2500000000,
    0xd2c6aba400000000, 0x1bcdbe4700000000, 0x3ca89bc600000000,
    0x1401859e00000000, 0x3364a01f00000000, 0x4453b82e00000000,
    0x63369daf00000000, 0x4b9f83f700000000, 0x6cfaa67600000000,
    0x988ed8ea00000000, 0xbfebfd6b00000000, 0x9742e33300000000,
    0xb027c6b200000000, 0xc710de8300000000, 0xe075fb0200000000,
    0xc8dce55a00000000, 0xefb9c0db00000000, 0x26b2d53800000000,
    0x01d7f0b900000000, 0x297eeee100000000, 0x0e1bcb6000000000,
    0x792cd35100000000, 0x5e49f6d000000000, 0x76e0e88800000000,
    0x5185cd0900000000}};

#else /* W == 4 */

local const z_crc_t FAR crc_braid_table[][256] = {
   {0x00000000, 0x9ba54c6f, 0xec3b9e9f, 0x779ed2f0, 0x03063b7f,
    0x98a37710, 0xef3da5e0, 0x7498e98f, 0x060c76fe, 0x9da93a91,
    0xea37e861, 0x7192a40e, 0x050a4d81, 0x9eaf01ee, 0xe931d31e,
    0x72949f71, 0x0c18edfc, 0x97bda193, 0xe0237363, 0x7b863f0c,
    0x0f1ed683, 0x94bb9aec, 0xe325481c, 0x78800473, 0x0a149b02,
    0x91b1d76d, 0xe62f059d, 0x7d8a49f2, 0x0912a07d, 0x92b7ec12,
    0xe5293ee2, 0x7e8c728d, 0x1831dbf8, 0x83949797, 0xf40a4567,
    0x6faf0908, 0x1b37e087, 0x8092ace8, 0xf70c7e18, 0x6ca93277,
    0x1e3dad06, 0x8598e169, 0xf2063399, 0x69a37ff6, 0x1d3b9679,
    0x869eda16, 0xf10008e6, 0x6aa54489, 0x14293604, 0x8f8c7a6b,
    0xf812a89b, 0x63b7e4f4, 0x172f0d7b, 0x8c8a4114, 0xfb1493e4,
    0x60b1df8b, 0x122540fa, 0x89800c95, 0xfe1ede65, 0x65bb920a,
    0x11237b85, 0x8a8637ea, 0xfd18e51a, 0x66bda975, 0x3063b7f0,
    0xabc6fb9f, 0xdc58296f, 0x47fd6500, 0x33658c8f, 0xa8c0c0e0,
    0xdf5e1210, 0x44fb5e7f, 0x366fc10e, 0xadca8d61, 0xda545f91,
    0x41f113fe, 0x3569fa71, 0xaeccb61e, 0xd95264ee, 0x42f72881,
    0x3c7b5a0c, 0xa7de1663, 0xd040c493, 0x4be588fc, 0x3f7d6173,
    0xa4d82d1c, 0xd346ffec, 0x48e3b383, 0x3a772cf2, 0xa1d2609d,
    0xd64cb26d, 0x4de9fe02, 0x3971178d, 0xa2d45be2, 0xd54a8912,
    0x4eefc57d, 0x28526c08, 0xb3f72067, 0xc469f297, 0x5fccbef8,
    0x2b545777, 0xb0f11b18, 0xc76fc9e8, 0x5cca8587, 0x2e5e1af6,
    0xb5fb5699, 0xc2658469, 0x59c0c806, 0x2d582189, 0xb6fd6de6,
    0xc163bf16, 0x5ac6f379, 0x244a81f4, 0xbfefcd9b, 0xc8711f6b,
    0x53d45304, 0x274cba8b, 0xbce9f6e4, 0xcb772414, 0x50d2687b,
    0x2246f70a, 0xb9e3bb65, 0xce7d6995, 0x55d825fa, 0x2140cc75,
    0xbae5801a, 0xcd7b52ea, 0x56de1e85, 0x60c76fe0, 0xfb62238f,
    0x8cfcf17f, 0x1759bd10, 0x63c1549f, 0xf86418f0, 0x8ffaca00,
    0x145f866f, 0x66cb191e, 0xfd6e5571, 0x8af08781, 0x1155cbee,
    0x65cd2261, 0xfe686e0e, 0x89f6bcfe, 0x1253f091, 0x6cdf821c,
    0xf77ace73, 0x80e41c83, 0x1b4150ec, 0x6fd9b963, 0xf47cf50c,
    0x83e227fc, 0x18476b93, 0x6ad3f4e2, 0xf176b88d, 0x86e86a7d,
    0x1d4d2612, 0x69d5cf9d, 0xf27083f2, 0x85ee5102, 0x1e4b1d6d,
    0x78f6b418, 0xe353f877, 0x94cd2a87, 0x0f6866e8, 0x7bf08f67,
    0xe055c308, 0x97cb11f8, 0x0c6e5d97, 0x7efac2e6, 0xe55f8e89,
    0x92c15c79, 0x09641016, 0x7dfcf999, 0xe659b5f6, 0x91c76706,
    0x0a622b69, 0x74ee59e4, 0xef4b158b, 0x98d5c77b, 0x03708b14,
    0x77e8629b, 0xec4d2ef4, 0x9bd3fc04, 0x0076b06b, 0x72e22f1a,
    0xe9476375, 0x9ed9b185, 0x057cfdea, 0x71e41465, 0xea41580a,
    0x9ddf8afa, 0x067ac695, 0x50a4d810, 0xcb01947f, 0xbc9f468f,
    0x273a0ae0, 0x53a2e36f, 0xc807af00, 0xbf997df0, 0x243c319f,
    0x56a8aeee, 0xcd0de281, 0xba933071, 0x21367c1e, 0x55ae9591,
    0xce0bd9fe, 0xb9950b0e, 0x22304761, 0x5cbc35ec, 0xc7197983,
    0xb087ab73, 0x2b22e71c, 0x5fba0e93, 0xc41f42fc, 0xb381900c,
    0x2824dc63, 0x5ab04312, 0xc1150f7d, 0xb68bdd8d, 0x2d2e91e2,
    0x59b6786d, 0xc2133402, 0xb58de6f2, 0x2e28aa9d, 0x489503e8,
    0xd3304f87, 0xa4ae9d77, 0x3f0bd118, 0x4b933897, 0xd03674f8,
    0xa7a8a608, 0x3c0dea67, 0x4e997516, 0xd53c3979, 0xa2a2eb89,
    0x3907a7e6, 0x4d9f4e69, 0xd63a0206, 0xa1a4d0f6, 0x3a019c99,
    0x448dee14, 0xdf28a27b, 0xa8b6708b, 0x33133ce4, 0x478bd56b,
    0xdc2e9904, 0xabb04bf4, 0x3015079b, 0x428198ea, 0xd924d485,
    0xaeba0675, 0x351f4a1a, 0x4187a395, 0xda22effa, 0xadbc3d0a,
    0x36197165},
   {0x00000000, 0xc18edfc0, 0x586cb9c1, 0x99e26601, 0xb0d97382,
    0x7157ac42, 0xe8b5ca43, 0x293b1583, 0xbac3e145, 0x7b4d3e85,
    0xe2af5884, 0x23218744, 0x0a1a92c7, 0xcb944d07, 0x52762b06,
    0x93f8f4c6, 0xaef6c4cb, 0x6f781b0b, 0xf69a7d0a, 0x3714a2ca,
    0x1e2fb749, 0xdfa16889, 0x46430e88, 0x87cdd148, 0x1435258e,
    0xd5bbfa4e, 0x4c599c4f, 0x8dd7438f, 0xa4ec560c, 0x656289cc,
    0xfc80efcd, 0x3d0e300d, 0x869c8fd7, 0x47125017, 0xdef03616,
    0x1f7ee9d6, 0x3645fc55, 0xf7cb2395, 0x6e294594, 0xafa79a54,
    0x3c5f6e92, 0xfdd1b152, 0x6433d753, 0xa5bd0893, 0x8c861d10,
    0x4d08c2d0, 0xd4eaa4d1, 0x15647b11, 0x286a4b1c, 0xe9e494dc,
    0x7006f2dd, 0xb1882d1d, 0x98b3389e, 0x593de75e, 0xc0df815f,
    0x01515e9f, 0x92a9aa59, 0x53277599, 0xcac51398, 0x0b4bcc58,
    0x2270d9db, 0xe3fe061b, 0x7a1c601a, 0xbb92bfda, 0xd64819ef,
    0x17c6c62f, 0x8e24a02e, 0x4faa7fee, 0x66916a6d, 0xa71fb5ad,
    0x3efdd3ac, 0xff730c6c, 0x6c8bf8aa, 0xad05276a, 0x34e7416b,
    0xf5699eab, 0xdc528b28, 0x1ddc54e8, 0x843e32e9, 0x45b0ed29,
    0x78bedd24, 0xb93002e4, 0x20d264e5, 0xe15cbb25, 0xc867aea6,
    0x09e97166, 0x900b1767, 0x5185c8a7, 0xc27d3c61, 0x03f3e3a1,
    0x9a1185a0, 0x5b9f5a60, 0x72a44fe3, 0xb32a9023, 0x2ac8f622,
    0xeb4629e2, 0x50d49638, 0x915a49f8, 0x08b82ff9, 0xc936f039,
    0xe00de5ba, 0x21833a7a, 0xb8615c7b, 0x79ef83bb, 0xea17777d,
    0x2b99a8bd, 0xb27bcebc, 0x73f5117c, 0x5ace04ff, 0x9b40db3f,
    0x02a2bd3e, 0xc32c62fe, 0xfe2252f3, 0x3fac8d33, 0xa64eeb32,
    0x67c034f2, 0x4efb2171, 0x8f75feb1, 0x169798b0, 0xd7194770,
    0x44e1b3b6, 0x856f6c76, 0x1c8d0a77, 0xdd03d5b7, 0xf438c034,
    0x35b61ff4, 0xac5479f5, 0x6ddaa635, 0x77e1359f, 0xb66fea5f,
    0x2f8d8c5e, 0xee03539e, 0xc738461d, 0x06b699dd, 0x9f54ffdc,
    0x5eda201c, 0xcd22d4da, 0x0cac0b1a, 0x954e6d1b, 0x54c0b2db,
    0x7dfba758, 0xbc757898, 0x25971e99, 0xe419c159, 0xd917f154,
    0x18992e94, 0x817b4895, 0x40f59755, 0x69ce82d6, 0xa8405d16,
    0x31a23b17, 0xf02ce4d7, 0x63d41011, 0xa25acfd1, 0x3bb8a9d0,
    0xfa367610, 0xd30d6393, 0x1283bc53, 0x8b61da52, 0x4aef0592,
    0xf17dba48, 0x30f36588, 0xa9110389, 0x689fdc49, 0x41a4c9ca,
    0x802a160a, 0x19c8700b, 0xd846afcb, 0x4bbe5b0d, 0x8a3084cd,
    0x13d2e2cc, 0xd25c3d0c, 0xfb67288f, 0x3ae9f74f, 0xa30b914e,
    0x62854e8e, 0x5f8b7e83, 0x9e05a143, 0x07e7c742, 0xc6691882,
    0xef520d01, 0x2edcd2c1, 0xb73eb4c0, 0x76b06b00, 0xe5489fc6,
    0x24c64006, 0xbd242607, 0x7caaf9c7, 0x5591ec44, 0x941f3384,
    0x0dfd5585, 0xcc738a45, 0xa1a92c70, 0x6027f3b0, 0xf9c595b1,
    0x384b4a71, 0x11705ff2, 0xd0fe8032, 0x491ce633, 0x889239f3,
    0x1b6acd35, 0xdae412f5, 0x430674f4, 0x8288ab34, 0xabb3beb7,
    0x6a3d6177, 0xf3df0776, 0x3251d8b6, 0x0f5fe8bb, 0xced1377b,
    0x5733517a, 0x96bd8eba, 0xbf869b39, 0x7e0844f9, 0xe7ea22f8,
    0x2664fd38, 0xb59c09fe, 0x7412d63e, 0xedf0b03f, 0x2c7e6fff,
    0x05457a7c, 0xc4cba5bc, 0x5d29c3bd, 0x9ca71c7d, 0x2735a3a7,
    0xe6bb7c67, 0x7f591a66, 0xbed7c5a6, 0x97ecd025, 0x56620fe5,
    0xcf8069e4, 0x0e0eb624, 0x9df642e2, 0x5c789d22, 0xc59afb23,
    0x041424e3, 0x2d2f3160, 0xeca1eea0, 0x754388a1, 0xb4cd5761,
    0x89c3676c, 0x484db8ac, 0xd1afdead, 0x1021016d, 0x391a14ee,
    0xf894cb2e, 0x6176ad2f, 0xa0f872ef, 0x33008629, 0xf28e59e9,
    0x6b6c3fe8, 0xaae2e028, 0x83d9f5ab, 0x42572a6b, 0xdbb54c6a,
    0x1a3b93aa},
   {0x00000000, 0xefc26b3e, 0x04f5d03d, 0xeb37bb03, 0x09eba07a,
    0xe629cb44, 0x0d1e7047, 0xe2dc1b79, 0x13d740f4, 0xfc152bca,
    0x172290c9, 0xf8e0fbf7, 0x1a3ce08e, 0xf5fe8bb0, 0x1ec930b3,
    0xf10b5b8d, 0x27ae81e8, 0xc86cead6, 0x235b51d5, 0xcc993aeb,
    0x2e452192, 0xc1874aac, 0x2ab0f1af, 0xc5729a91, 0x3479c11c,
    0xdbbbaa22, 0x308c1121, 0xdf4e7a1f, 0x3d926166, 0xd2500a58,
    0x3967b15b, 0xd6a5da65, 0x4f5d03d0, 0xa09f68ee, 0x4ba8d3ed,
    0xa46ab8d3, 0x46b6a3aa, 0xa974c894, 0x42437397, 0xad8118a9,
    0x5c8a4324, 0xb348281a, 0x587f9319, 0xb7bdf827, 0x5561e35e,
    0xbaa38860, 0x51943363, 0xbe56585d, 0x68f38238, 0x8731e906,
    0x6c065205, 0x83c4393b, 0x61182242, 0x8eda497c, 0x65edf27f,
    0x8a2f9941, 0x7b24c2cc, 0x94e6a9f2, 0x7fd112f1, 0x901379cf,
    0x72cf62b6, 0x9d0d0988, 0x763ab28b, 0x99f8d9b5, 0x9eba07a0,
    0x71786c9e, 0x9a4fd79d, 0x758dbca3, 0x9751a7da, 0x7893cce4,
    0x93a477e7, 0x7c661cd9, 0x8d6d4754, 0x62af2c6a, 0x89989769,
    0x665afc57, 0x8486e72e, 0x6b448c10, 0x80733713, 0x6fb15c2d,
    0xb9148648, 0x56d6ed76, 0xbde15675, 0x52233d4b, 0xb0ff2632,
    0x5f3d4d0c, 0xb40af60f, 0x5bc89d31, 0xaac3c6bc, 0x4501ad82,
    0xae361681, 0x41f47dbf, 0xa32866c6, 0x4cea0df8, 0xa7ddb6fb,
    0x481fddc5, 0xd1e70470, 0x3e256f4e, 0xd512d44d, 0x3ad0bf73,
    0xd80ca40a, 0x37cecf34, 0xdcf97437, 0x333b1f09, 0xc2304484,
    0x2df22fba, 0xc6c594b9, 0x2907ff87, 0xcbdbe4fe, 0x24198fc0,
    0xcf2e34c3, 0x20ec5ffd, 0xf6498598, 0x198beea6, 0xf2bc55a5,
    0x1d7e3e9b, 0xffa225e2, 0x10604edc, 0xfb57f5df, 0x14959ee1,
    0xe59ec56c, 0x0a5cae52, 0xe16b1551, 0x0ea97e6f, 0xec756516,
    0x03b70e28, 0xe880b52b, 0x0742de15, 0xe6050901, 0x09c7623f,
    0xe2f0d93c, 0x0d32b202, 0xefeea97b, 0x002cc245, 0xeb1b7946,
    0x04d91278, 0xf5d249f5, 0x1a1022cb, 0xf12799c8, 0x1ee5f2f6,
    0xfc39e98f, 0x13fb82b1, 0xf8cc39b2, 0x170e528c, 0xc1ab88e9,
    0x2e69e3d7, 0xc55e58d4, 0x2a9c33ea, 0xc8402893, 0x278243ad,
    0xccb5f8ae, 0x23779390, 0xd27cc81d, 0x3dbea323, 0xd6891820,
    0x394b731e, 0xdb976867, 0x34550359, 0xdf62b85a, 0x30a0d364,
    0xa9580ad1, 0x469a61ef, 0xadaddaec, 0x426fb1d2, 0xa0b3aaab,
    0x4f71c195, 0xa4467a96, 0x4b8411a8, 0xba8f4a25, 0x554d211b,
    0xbe7a9a18, 0x51b8f126, 0xb364ea5f, 0x5ca68161, 0xb7913a62,
    0x5853515c, 0x8ef68b39, 0x6134e007, 0x8a035b04, 0x65c1303a,
    0x871d2b43, 0x68df407d, 0x83e8fb7e, 0x6c2a9040, 0x9d21cbcd,
    0x72e3a0f3, 0x99d41bf0, 0x761670ce, 0x94ca6bb7, 0x7b080089,
    0x903fbb8a, 0x7ffdd0b4, 0x78bf0ea1, 0x977d659f, 0x7c4ade9c,
    0x9388b5a2, 0x7154aedb, 0x9e96c5e5, 0x75a17ee6, 0x9a6315d8,
    0x6b684e55, 0x84aa256b, 0x6f9d9e68, 0x805ff556, 0x6283ee2f,
    0x8d418511, 0x66763e12, 0x89b4552c, 0x5f118f49, 0xb0d3e477,
    0x5be45f74, 0xb426344a, 0x56fa2f33, 0xb938440d, 0x520fff0e,
    0xbdcd9430, 0x4cc6cfbd, 0xa304a483, 0x48331f80, 0xa7f174be,
    0x452d6fc7, 0xaaef04f9, 0x41d8bffa, 0xae1ad4c4, 0x37e20d71,
    0xd820664f, 0x3317dd4c, 0xdcd5b672, 0x3e09ad0b, 0xd1cbc635,
    0x3afc7d36, 0xd53e1608, 0x24354d85, 0xcbf726bb, 0x20c09db8,
    0xcf02f686, 0x2ddeedff, 0xc21c86c1, 0x292b3dc2, 0xc6e956fc,
    0x104c8c99, 0xff8ee7a7, 0x14b95ca4, 0xfb7b379a, 0x19a72ce3,
    0xf66547dd, 0x1d52fcde, 0xf29097e0, 0x039bcc6d, 0xec59a753,
    0x076e1c50, 0xe8ac776e, 0x0a706c17, 0xe5b20729, 0x0e85bc2a,
    0xe147d714},
   {0x00000000, 0x177b1443, 0x2ef62886, 0x398d3cc5, 0x5dec510c,
    0x4a97454f, 0x731a798a, 0x64616dc9, 0xbbd8a218, 0xaca3b65b,
    0x952e8a9e, 0x82559edd, 0xe634f314, 0xf14fe757, 0xc8c2db92,
    0xdfb9cfd1, 0xacc04271, 0xbbbb5632, 0x82366af7, 0x954d7eb4,
    0xf12c137d, 0xe657073e, 0xdfda3bfb, 0xc8a12fb8, 0x1718e069,
    0x0063f42a, 0x39eec8ef, 0x2e95dcac, 0x4af4b165, 0x5d8fa526,
    0x640299e3, 0x73798da0, 0x82f182a3, 0x958a96e0, 0xac07aa25,
    0xbb7cbe66, 0xdf1dd3af, 0xc866c7ec, 0xf1ebfb29, 0xe690ef6a,
    0x392920bb, 0x2e5234f8, 0x17df083d, 0x00a41c7e, 0x64c571b7,
    0x73be65f4, 0x4a335931, 0x5d484d72, 0x2e31c0d2, 0x394ad491,
    0x00c7e854, 0x17bcfc17, 0x73dd91de, 0x64a6859d, 0x5d2bb958,
    0x4a50ad1b, 0x95e962ca, 0x82927689, 0xbb1f4a4c, 0xac645e0f,
    0xc80533c6, 0xdf7e2785, 0xe6f31b40, 0xf1880f03, 0xde920307,
    0xc9e91744, 0xf0642b81, 0xe71f3fc2, 0x837e520b, 0x94054648,
    0xad887a8d, 0xbaf36ece, 0x654aa11f, 0x7231b55c, 0x4bbc8999,
    0x5cc79dda, 0x38a6f013, 0x2fdde450, 0x1650d895, 0x012bccd6,
    0x72524176, 0x65295535, 0x5ca469f0, 0x4bdf7db3, 0x2fbe107a,
    0x38c50439, 0x014838fc, 0x16332cbf, 0xc98ae36e, 0xdef1f72d,
    0xe77ccbe8, 0xf007dfab, 0x9466b262, 0x831da621, 0xba909ae4,
    0xadeb8ea7, 0x5c6381a4, 0x4b1895e7, 0x7295a922, 0x65eebd61,
    0x018fd0a8, 0x16f4c4eb, 0x2f79f82e, 0x3802ec6d, 0xe7bb23bc,
    0xf0c037ff, 0xc94d0b3a, 0xde361f79, 0xba5772b0, 0xad2c66f3,
    0x94a15a36, 0x83da4e75, 0xf0a3c3d5, 0xe7d8d796, 0xde55eb53,
    0xc92eff10, 0xad4f92d9, 0xba34869a, 0x83b9ba5f, 0x94c2ae1c,
    0x4b7b61cd, 0x5c00758e, 0x658d494b, 0x72f65d08, 0x169730c1,
    0x01ec2482, 0x38611847, 0x2f1a0c04, 0x6655004f, 0x712e140c,
    0x48a328c9, 0x5fd83c8a, 0x3bb95143, 0x2cc24500, 0x154f79c5,
    0x02346d86, 0xdd8da257, 0xcaf6b614, 0xf37b8ad1, 0xe4009e92,
    0x8061f35b, 0x971ae718, 0xae97dbdd, 0xb9eccf9e, 0xca95423e,
    0xddee567d, 0xe4636ab8, 0xf3187efb, 0x97791332, 0x80020771,
    0xb98f3bb4, 0xaef42ff7, 0x714de026, 0x6636f465, 0x5fbbc8a0,
    0x48c0dce3, 0x2ca1b12a, 0x3bdaa569, 0x025799ac, 0x152c8def,
    0xe4a482ec, 0xf3df96af, 0xca52aa6a, 0xdd29be29, 0xb948d3e0,
    0xae33c7a3, 0x97befb66, 0x80c5ef25, 0x5f7c20f4, 0x480734b7,
    0x718a0872, 0x66f11c31, 0x029071f8, 0x15eb65bb, 0x2c66597e,
    0x3b1d4d3d, 0x4864c09d, 0x5f1fd4de, 0x6692e81b, 0x71e9fc58,
    0x15889191, 0x02f385d2, 0x3b7eb917, 0x2c05ad54, 0xf3bc6285,
    0xe4c776c6, 0xdd4a4a03, 0xca315e40, 0xae503389, 0xb92b27ca,
    0x80a61b0f, 0x97dd0f4c, 0xb8c70348, 0xafbc170b, 0x96312bce,
    0x814a3f8d, 0xe52b5244, 0xf2504607, 0xcbdd7ac2, 0xdca66e81,
    0x031fa150, 0x1464b513, 0x2de989d6, 0x3a929d95, 0x5ef3f05c,
    0x4988e41f, 0x7005d8da, 0x677ecc99, 0x14074139, 0x037c557a,
    0x3af169bf, 0x2d8a7dfc, 0x49eb1035, 0x5e900476, 0x671d38b3,
    0x70662cf0, 0xafdfe321, 0xb8a4f762, 0x8129cba7, 0x9652dfe4,
    0xf233b22d, 0xe548a66e, 0xdcc59aab, 0xcbbe8ee8, 0x3a3681eb,
    0x2d4d95a8, 0x14c0a96d, 0x03bbbd2e, 0x67dad0e7, 0x70a1c4a4,
    0x492cf861, 0x5e57ec22, 0x81ee23f3, 0x969537b0, 0xaf180b75,
    0xb8631f36, 0xdc0272ff, 0xcb7966bc, 0xf2f45a79, 0xe58f4e3a,
    0x96f6c39a, 0x818dd7d9, 0xb800eb1c, 0xaf7bff5f, 0xcb1a9296,
    0xdc6186d5, 0xe5ecba10, 0xf297ae53, 0x2d2e6182, 0x3a5575c1,
    0x03d84904, 0x14a35d47, 0x70c2308e, 0x67b924cd, 0x5e341808,
    0x494f0c4b}};

local const z_word_t FAR crc_braid_big_table[][256] = {
   {0x00000000, 0x43147b17, 0x8628f62e, 0xc53c8d39, 0x0c51ec5d,
    0x4f45974a, 0x8a791a73, 0xc96d6164, 0x18a2d8bb, 0x5bb6a3ac,
    0x9e8a2e95, 0xdd9e5582, 0x14f334e6, 0x57e74ff1, 0x92dbc2c8,
    0xd1cfb9df, 0x7142c0ac, 0x3256bbbb, 0xf76a3682, 0xb47e4d95,
    0x7d132cf1, 0x3e0757e6, 0xfb3bdadf, 0xb82fa1c8, 0x69e01817,
    0x2af46300, 0xefc8ee39, 0xacdc952e, 0x65b1f44a, 0x26a58f5d,
    0xe3990264, 0xa08d7973, 0xa382f182, 0xe0968a95, 0x25aa07ac,
    0x66be7cbb, 0xafd31ddf, 0xecc766c8, 0x29fbebf1, 0x6aef90e6,
    0xbb202939, 0xf834522e, 0x3d08df17, 0x7e1ca400, 0xb771c564,
    0xf465be73, 0x3159334a, 0x724d485d, 0xd2c0312e, 0x91d44a39,
    0x54e8c700, 0x17fcbc17, 0xde91dd73, 0x9d85a664, 0x58b92b5d,
    0x1bad504a, 0xca62e995, 0x89769282, 0x4c4a1fbb, 0x0f5e64ac,
    0xc63305c8, 0x85277edf, 0x401bf3e6, 0x030f88f1, 0x070392de,
    0x4417e9c9, 0x812b64f0, 0xc23f1fe7, 0x0b527e83, 0x48460594,
    0x8d7a88ad, 0xce6ef3ba, 0x1fa14a65, 0x5cb53172, 0x9989bc4b,
    0xda9dc75c, 0x13f0a638, 0x50e4dd2f, 0x95d85016, 0xd6cc2b01,
    0x76415272, 0x35552965, 0xf069a45c, 0xb37ddf4b, 0x7a10be2f,
    0x3904c538, 0xfc384801, 0xbf2c3316, 0x6ee38ac9, 0x2df7f1de,
    0xe8cb7ce7, 0xabdf07f0, 0x62b26694, 0x21a61d83, 0xe49a90ba,
    0xa78eebad, 0xa481635c, 0xe795184b, 0x22a99572, 0x61bdee65,
    0xa8d08f01, 0xebc4f416, 0x2ef8792f, 0x6dec0238, 0xbc23bbe7,
    0xff37c0f0, 0x3a0b4dc9, 0x791f36de, 0xb07257ba, 0xf3662cad,
    0x365aa194, 0x754eda83, 0xd5c3a3f0, 0x96d7d8e7, 0x53eb55de,
    0x10ff2ec9, 0xd9924fad, 0x9a8634ba, 0x5fbab983, 0x1caec294,
    0xcd617b4b, 0x8e75005c, 0x4b498d65, 0x085df672, 0xc1309716,
    0x8224ec01, 0x47186138, 0x040c1a2f, 0x4f005566, 0x0c142e71,
    0xc928a348, 0x8a3cd85f, 0x4351b93b, 0x0045c22c, 0xc5794f15,
    0x866d3402, 0x57a28ddd, 0x14b6f6ca, 0xd18a7bf3, 0x929e00e4,
    0x5bf36180, 0x18e71a97, 0xdddb97ae, 0x9ecfecb9, 0x3e4295ca,
    0x7d56eedd, 0xb86a63e4, 0xfb7e18f3, 0x32137997, 0x71070280,
    0xb43b8fb9, 0xf72ff4ae, 0x26e04d71, 0x65f43666, 0xa0c8bb5f,
    0xe3dcc048, 0x2ab1a12c, 0x69a5da3b, 0xac995702, 0xef8d2c15,
    0xec82a4e4, 0xaf96dff3, 0x6aaa52ca, 0x29be29dd, 0xe0d348b9,
    0xa3c733ae, 0x66fbbe97, 0x25efc580, 0xf4207c5f, 0xb7340748,
    0x72088a71, 0x311cf166, 0xf8719002, 0xbb65eb15, 0x7e59662c,
    0x3d4d1d3b, 0x9dc06448, 0xded41f5f, 0x1be89266, 0x58fce971,
    0x91918815, 0xd285f302, 0x17b97e3b, 0x54ad052c, 0x8562bcf3,
    0xc676c7e4, 0x034a4add, 0x405e31ca, 0x893350ae, 0xca272bb9,
    0x0f1ba680, 0x4c0fdd97, 0x4803c7b8, 0x0b17bcaf, 0xce2b3196,
    0x8d3f4a81, 0x44522be5, 0x074650f2, 0xc27addcb, 0x816ea6dc,
    0x50a11f03, 0x13b56414, 0xd689e92d, 0x959d923a, 0x5cf0f35e,
    0x1fe48849, 0xdad80570, 0x99cc7e67, 0x39410714, 0x7a557c03,
    0xbf69f13a, 0xfc7d8a2d, 0x3510eb49, 0x7604905e, 0xb3381d67,
    0xf02c6670, 0x21e3dfaf, 0x62f7a4b8, 0xa7cb2981, 0xe4df5296,
    0x2db233f2, 0x6ea648e5, 0xab9ac5dc, 0xe88ebecb, 0xeb81363a,
    0xa8954d2d, 0x6da9c014, 0x2ebdbb03, 0xe7d0da67, 0xa4c4a170,
    0x61f82c49, 0x22ec575e, 0xf323ee81, 0xb0379596, 0x750b18af,
    0x361f63b8, 0xff7202dc, 0xbc6679cb, 0x795af4f2, 0x3a4e8fe5,
    0x9ac3f696, 0xd9d78d81, 0x1ceb00b8, 0x5fff7baf, 0x96921acb,
    0xd58661dc, 0x10baece5, 0x53ae97f2, 0x82612e2d, 0xc175553a,
    0x0449d803, 0x475da314, 0x8e30c270, 0xcd24b967, 0x0818345e,
    0x4b0c4f49},
   {0x00000000, 0x3e6bc2ef, 0x3dd0f504, 0x03bb37eb, 0x7aa0eb09,
    0x44cb29e6, 0x47701e0d, 0x791bdce2, 0xf440d713, 0xca2b15fc,
    0xc9902217, 0xf7fbe0f8, 0x8ee03c1a, 0xb08bfef5, 0xb330c91e,
    0x8d5b0bf1, 0xe881ae27, 0xd6ea6cc8, 0xd5515b23, 0xeb3a99cc,
    0x9221452e, 0xac4a87c1, 0xaff1b02a, 0x919a72c5, 0x1cc17934,
    0x22aabbdb, 0x21118c30, 0x1f7a4edf, 0x6661923d, 0x580a50d2,
    0x5bb16739, 0x65daa5d6, 0xd0035d4f, 0xee689fa0, 0xedd3a84b,
    0xd3b86aa4, 0xaaa3b646, 0x94c874a9, 0x97734342, 0xa91881ad,
    0x24438a5c, 0x1a2848b3, 0x19937f58, 0x27f8bdb7, 0x5ee36155,
    0x6088a3ba, 0x63339451, 0x5d5856be, 0x3882f368, 0x06e93187,
    0x0552066c, 0x3b39c483, 0x42221861, 0x7c49da8e, 0x7ff2ed65,
    0x41992f8a, 0xccc2247b, 0xf2a9e694, 0xf112d17f, 0xcf791390,
    0xb662cf72, 0x88090d9d, 0x8bb23a76, 0xb5d9f899, 0xa007ba9e,
    0x9e6c7871, 0x9dd74f9a, 0xa3bc8d75, 0xdaa75197, 0xe4cc9378,
    0xe777a493, 0xd91c667c, 0x54476d8d, 0x6a2caf62, 0x69979889,
    0x57fc5a66, 0x2ee78684, 0x108c446b, 0x13377380, 0x2d5cb16f,
    0x488614b9, 0x76edd656, 0x7556e1bd, 0x4b3d2352, 0x3226ffb0,
    0x0c4d3d5f, 0x0ff60ab4, 0x319dc85b, 0xbcc6c3aa, 0x82ad0145,
    0x811636ae, 0xbf7df441, 0xc66628a3, 0xf80dea4c, 0xfbb6dda7,
    0xc5dd1f48, 0x7004e7d1, 0x4e6f253e, 0x4dd412d5, 0x73bfd03a,
    0x0aa40cd8, 0x34cfce37, 0x3774f9dc, 0x091f3b33, 0x844430c2,
    0xba2ff22d, 0xb994c5c6, 0x87ff0729, 0xfee4dbcb, 0xc08f1924,
    0xc3342ecf, 0xfd5fec20, 0x988549f6, 0xa6ee8b19, 0xa555bcf2,
    0x9b3e7e1d, 0xe225a2ff, 0xdc4e6010, 0xdff557fb, 0xe19e9514,
    0x6cc59ee5, 0x52ae5c0a, 0x51156be1, 0x6f7ea90e, 0x166575ec,
    0x280eb703, 0x2bb580e8, 0x15de4207, 0x010905e6, 0x3f62c709,
    0x3cd9f0e2, 0x02b2320d, 0x7ba9eeef, 0x45c22c00, 0x46791beb,
    0x7812d904, 0xf549d2f5, 0xcb22101a, 0xc89927f1, 0xf6f2e51e,
    0x8fe939fc, 0xb182fb13, 0xb239ccf8, 0x8c520e17, 0xe988abc1,
    0xd7e3692e, 0xd4585ec5, 0xea339c2a, 0x932840c8, 0xad438227,
    0xaef8b5cc, 0x90937723, 0x1dc87cd2, 0x23a3be3d, 0x201889d6,
    0x1e734b39, 0x676897db, 0x59035534, 0x5ab862df, 0x64d3a030,
    0xd10a58a9, 0xef619a46, 0xecdaadad, 0xd2b16f42, 0xabaab3a0,
    0x95c1714f, 0x967a46a4, 0xa811844b, 0x254a8fba, 0x1b214d55,
    0x189a7abe, 0x26f1b851, 0x5fea64b3, 0x6181a65c, 0x623a91b7,
    0x5c515358, 0x398bf68e, 0x07e03461, 0x045b038a, 0x3a30c165,
    0x432b1d87, 0x7d40df68, 0x7efbe883, 0x40902a6c, 0xcdcb219d,
    0xf3a0e372, 0xf01bd499, 0xce701676, 0xb76bca94, 0x8900087b,
    0x8abb3f90, 0xb4d0fd7f, 0xa10ebf78, 0x9f657d97, 0x9cde4a7c,
    0xa2b58893, 0xdbae5471, 0xe5c5969e, 0xe67ea175, 0xd815639a,
    0x554e686b, 0x6b25aa84, 0x689e9d6f, 0x56f55f80, 0x2fee8362,
    0x1185418d, 0x123e7666, 0x2c55b489, 0x498f115f, 0x77e4d3b0,
    0x745fe45b, 0x4a3426b4, 0x332ffa56, 0x0d4438b9, 0x0eff0f52,
    0x3094cdbd, 0xbdcfc64c, 0x83a404a3, 0x801f3348, 0xbe74f1a7,
    0xc76f2d45, 0xf904efaa, 0xfabfd841, 0xc4d41aae, 0x710de237,
    0x4f6620d8, 0x4cdd1733, 0x72b6d5dc, 0x0bad093e, 0x35c6cbd1,
    0x367dfc3a, 0x08163ed5, 0x854d3524, 0xbb26f7cb, 0xb89dc020,
    0x86f602cf, 0xffedde2d, 0xc1861cc2, 0xc23d2b29, 0xfc56e9c6,
    0x998c4c10, 0xa7e78eff, 0xa45cb914, 0x9a377bfb, 0xe32ca719,
    0xdd4765f6, 0xdefc521d, 0xe09790f2, 0x6dcc9b03, 0x53a759ec,
    0x501c6e07, 0x6e77ace8, 0x176c700a, 0x2907b2e5, 0x2abc850e,
    0x14d747e1},
   {0x00000000, 0xc0df8ec1, 0xc1b96c58, 0x0166e299, 0x8273d9b0,
    0x42ac5771, 0x43cab5e8, 0x83153b29, 0x45e1c3ba, 0x853e4d7b,
    0x8458afe2, 0x44872123, 0xc7921a0a, 0x074d94cb, 0x062b7652,
    0xc6f4f893, 0xcbc4f6ae, 0x0b1b786f, 0x0a7d9af6, 0xcaa21437,
    0x49b72f1e, 0x8968a1df, 0x880e4346, 0x48d1cd87, 0x8e253514,
    0x4efabbd5, 0x4f9c594c, 0x8f43d78d, 0x0c56eca4, 0xcc896265,
    0xcdef80fc, 0x0d300e3d, 0xd78f9c86, 0x17501247, 0x1636f0de,
    0xd6e97e1f, 0x55fc4536, 0x9523cbf7, 0x9445296e, 0x549aa7af,
    0x926e5f3c, 0x52b1d1fd, 0x53d73364, 0x9308bda5, 0x101d868c,
    0xd0c2084d, 0xd1a4ead4, 0x117b6415, 0x1c4b6a28, 0xdc94e4e9,
    0xddf20670, 0x1d2d88b1, 0x9e38b398, 0x5ee73d59, 0x5f81dfc0,
    0x9f5e5101, 0x59aaa992, 0x99752753, 0x9813c5ca, 0x58cc4b0b,
    0xdbd97022, 0x1b06fee3, 0x1a601c7a, 0xdabf92bb, 0xef1948d6,
    0x2fc6c617, 0x2ea0248e, 0xee7faa4f, 0x6d6a9166, 0xadb51fa7,
    0xacd3fd3e, 0x6c0c73ff, 0xaaf88b6c, 0x6a2705ad, 0x6b41e734,
    0xab9e69f5, 0x288b52dc, 0xe854dc1d, 0xe9323e84, 0x29edb045,
    0x24ddbe78, 0xe40230b9, 0xe564d220, 0x25bb5ce1, 0xa6ae67c8,
    0x6671e909, 0x67170b90, 0xa7c88551, 0x613c7dc2, 0xa1e3f303,
    0xa085119a, 0x605a9f5b, 0xe34fa472, 0x23902ab3, 0x22f6c82a,
    0xe22946eb, 0x3896d450, 0xf8495a91, 0xf92fb808, 0x39f036c9,
    0xbae50de0, 0x7a3a8321, 0x7b5c61b8, 0xbb83ef79, 0x7d7717ea,
    0xbda8992b, 0xbcce7bb2, 0x7c11f573, 0xff04ce5a, 0x3fdb409b,
    0x3ebda202, 0xfe622cc3, 0xf35222fe, 0x338dac3f, 0x32eb4ea6,
    0xf234c067, 0x7121fb4e, 0xb1fe758f, 0xb0989716, 0x704719d7,
    0xb6b3e144, 0x766c6f85, 0x770a8d1c, 0xb7d503dd, 0x34c038f4,
    0xf41fb635, 0xf57954ac, 0x35a6da6d, 0x9f35e177, 0x5fea6fb6,
    0x5e8c8d2f, 0x9e5303ee, 0x1d4638c7, 0xdd99b606, 0xdcff549f,
    0x1c20da5e, 0xdad422cd, 0x1a0bac0c, 0x1b6d4e95, 0xdbb2c054,
    0x58a7fb7d, 0x987875bc, 0x991e9725, 0x59c119e4, 0x54f117d9,
    0x942e9918, 0x95487b81, 0x5597f540, 0xd682ce69, 0x165d40a8,
    0x173ba231, 0xd7e42cf0, 0x1110d463, 0xd1cf5aa2, 0xd0a9b83b,
    0x107636fa, 0x93630dd3, 0x53bc8312, 0x52da618b, 0x9205ef4a,
    0x48ba7df1, 0x8865f330, 0x890311a9, 0x49dc9f68, 0xcac9a441,
    0x0a162a80, 0x0b70c819, 0xcbaf46d8, 0x0d5bbe4b, 0xcd84308a,
    0xcce2d213, 0x0c3d5cd2, 0x8f2867fb, 0x4ff7e93a, 0x4e910ba3,
    0x8e4e8562, 0x837e8b5f, 0x43a1059e, 0x42c7e707, 0x821869c6,
    0x010d52ef, 0xc1d2dc2e, 0xc0b43eb7, 0x006bb076, 0xc69f48e5,
    0x0640c624, 0x072624bd, 0xc7f9aa7c, 0x44ec9155, 0x84331f94,
    0x8555fd0d, 0x458a73cc, 0x702ca9a1, 0xb0f32760, 0xb195c5f9,
    0x714a4b38, 0xf25f7011, 0x3280fed0, 0x33e61c49, 0xf3399288,
    0x35cd6a1b, 0xf512e4da, 0xf4740643, 0x34ab8882, 0xb7beb3ab,
    0x77613d6a, 0x7607dff3, 0xb6d85132, 0xbbe85f0f, 0x7b37d1ce,
    0x7a513357, 0xba8ebd96, 0x399b86bf, 0xf944087e, 0xf822eae7,
    0x38fd6426, 0xfe099cb5, 0x3ed61274, 0x3fb0f0ed, 0xff6f7e2c,
    0x7c7a4505, 0xbca5cbc4, 0xbdc3295d, 0x7d1ca79c, 0xa7a33527,
    0x677cbbe6, 0x661a597f, 0xa6c5d7be, 0x25d0ec97, 0xe50f6256,
    0xe46980cf, 0x24b60e0e, 0xe242f69d, 0x229d785c, 0x23fb9ac5,
    0xe3241404, 0x60312f2d, 0xa0eea1ec, 0xa1884375, 0x6157cdb4,
    0x6c67c389, 0xacb84d48, 0xaddeafd1, 0x6d012110, 0xee141a39,
    0x2ecb94f8, 0x2fad7661, 0xef72f8a0, 0x29860033, 0xe9598ef2,
    0xe83f6c6b, 0x28e0e2aa, 0xabf5d983, 0x6b2a5742, 0x6a4cb5db,
    0xaa933b1a},
   {0x00000000, 0x6f4ca59b, 0x9f9e3bec, 0xf0d29e77, 0x7f3b0603,
    0x1077a398, 0xe0a53def, 0x8fe99874, 0xfe760c06, 0x913aa99d,
    0x61e837ea, 0x0ea49271, 0x814d0a05, 0xee01af9e, 0x1ed331e9,
    0x719f9472, 0xfced180c, 0x93a1bd97, 0x637323e0, 0x0c3f867b,
    0x83d61e0f, 0xec9abb94, 0x1c4825e3, 0x73048078, 0x029b140a,
    0x6dd7b191, 0x9d052fe6, 0xf2498a7d, 0x7da01209, 0x12ecb792,
    0xe23e29e5, 0x8d728c7e, 0xf8db3118, 0x97979483, 0x67450af4,
    0x0809af6f, 0x87e0371b, 0xe8ac9280, 0x187e0cf7, 0x7732a96c,
    0x06ad3d1e, 0x69e19885, 0x993306f2, 0xf67fa369, 0x79963b1d,
    0x16da9e86, 0xe60800f1, 0x8944a56a, 0x04362914, 0x6b7a8c8f,
    0x9ba812f8, 0xf4e4b763, 0x7b0d2f17, 0x14418a8c, 0xe49314fb,
    0x8bdfb160, 0xfa402512, 0x950c8089, 0x65de1efe, 0x0a92bb65,
    0x857b2311, 0xea37868a, 0x1ae518fd, 0x75a9bd66, 0xf0b76330,
    0x9ffbc6ab, 0x6f2958dc, 0x0065fd47, 0x8f8c6533, 0xe0c0c0a8,
    0x10125edf, 0x7f5efb44, 0x0ec16f36, 0x618dcaad, 0x915f54da,
    0xfe13f141, 0x71fa6935, 0x1eb6ccae, 0xee6452d9, 0x8128f742,
    0x0c5a7b3c, 0x6316dea7, 0x93c440d0, 0xfc88e54b, 0x73617d3f,
    0x1c2dd8a4, 0xecff46d3, 0x83b3e348, 0xf22c773a, 0x9d60d2a1,
    0x6db24cd6, 0x02fee94d, 0x8d177139, 0xe25bd4a2, 0x12894ad5,
    0x7dc5ef4e, 0x086c5228, 0x6720f7b3, 0x97f269c4, 0xf8becc5f,
    0x7757542b, 0x181bf1b0, 0xe8c96fc7, 0x8785ca5c, 0xf61a5e2e,
    0x9956fbb5, 0x698465c2, 0x06c8c059, 0x8921582d, 0xe66dfdb6,
    0x16bf63c1, 0x79f3c65a, 0xf4814a24, 0x9bcdefbf, 0x6b1f71c8,
    0x0453d453, 0x8bba4c27, 0xe4f6e9bc, 0x142477cb, 0x7b68d250,
    0x0af74622, 0x65bbe3b9, 0x95697dce, 0xfa25d855, 0x75cc4021,
    0x1a80e5ba, 0xea527bcd, 0x851ede56, 0xe06fc760, 0x8f2362fb,
    0x7ff1fc8c, 0x10bd5917, 0x9f54c163, 0xf01864f8, 0x00cafa8f,
    0x6f865f14, 0x1e19cb66, 0x71556efd, 0x8187f08a, 0xeecb5511,
    0x6122cd65, 0x0e6e68fe, 0xfebcf689, 0x91f05312, 0x1c82df6c,
    0x73ce7af7, 0x831ce480, 0xec50411b, 0x63b9d96f, 0x0cf57cf4,
    0xfc27e283, 0x936b4718, 0xe2f4d36a, 0x8db876f1, 0x7d6ae886,
    0x12264d1d, 0x9dcfd569, 0xf28370f2, 0x0251ee85, 0x6d1d4b1e,
    0x18b4f678, 0x77f853e3, 0x872acd94, 0xe866680f, 0x678ff07b,
    0x08c355e0, 0xf811cb97, 0x975d6e0c, 0xe6c2fa7e, 0x898e5fe5,
    0x795cc192, 0x16106409, 0x99f9fc7d, 0xf6b559e6, 0x0667c791,
    0x692b620a, 0xe459ee74, 0x8b154bef, 0x7bc7d598, 0x148b7003,
    0x9b62e877, 0xf42e4dec, 0x04fcd39b, 0x6bb07600, 0x1a2fe272,
    0x756347e9, 0x85b1d99e, 0xeafd7c05, 0x6514e471, 0x0a5841ea,
    0xfa8adf9d, 0x95c67a06, 0x10d8a450, 0x7f9401cb, 0x8f469fbc,
    0xe00a3a27, 0x6fe3a253, 0x00af07c8, 0xf07d99bf, 0x9f313c24,
    0xeeaea856, 0x81e20dcd, 0x713093ba, 0x1e7c3621, 0x9195ae55,
    0xfed90bce, 0x0e0b95b9, 0x61473022, 0xec35bc5c, 0x837919c7,
    0x73ab87b0, 0x1ce7222b, 0x930eba5f, 0xfc421fc4, 0x0c9081b3,
    0x63dc2428, 0x1243b05a, 0x7d0f15c1, 0x8ddd8bb6, 0xe2912e2d,
    0x6d78b659, 0x023413c2, 0xf2e68db5, 0x9daa282e, 0xe8039548,
    0x874f30d3, 0x779daea4, 0x18d10b3f, 0x9738934b, 0xf87436d0,
    0x08a6a8a7, 0x67ea0d3c, 0x1675994e, 0x79393cd5, 0x89eba2a2,
    0xe6a70739, 0x694e9f4d, 0x06023ad6, 0xf6d0a4a1, 0x999c013a,
    0x14ee8d44, 0x7ba228df, 0x8b70b6a8, 0xe43c1333, 0x6bd58b47,
    0x04992edc, 0xf44bb0ab, 0x9b071530, 0xea988142, 0x85d424d9,
    0x7506baae, 0x1a4a1f35, 0x95a38741, 0xfaef22da, 0x0a3dbcad,
    0x65711936}};

#endif

#endif

#if N == 4

#if W == 8

local const z_crc_t FAR crc_braid_table[][256] = {
   {0x00000000, 0xf1da05aa, 0x38c50d15, 0xc91f08bf, 0x718a1a2a,
    0x80501f80, 0x494f173f, 0xb8951295, 0xe3143454, 0x12ce31fe,
    0xdbd13941, 0x2a0b3ceb, 0x929e2e7e, 0x63442bd4, 0xaa5b236b,
    0x5b8126c1, 0x1d596ee9, 0xec836b43, 0x259c63fc, 0xd4466656,
    0x6cd374c3, 0x9d097169, 0x541679d6, 0xa5cc7c7c, 0xfe4d5abd,
    0x0f975f17, 0xc68857a8, 0x37525202, 0x8fc74097, 0x7e1d453d,
    0xb7024d82, 0x46d84828, 0x3ab2ddd2, 0xcb68d878, 0x0277d0c7,
    0xf3add56d, 0x4b38c7f8, 0xbae2c252, 0x73fdcaed, 0x8227cf47,
    0xd9a6e986, 0x287cec2c, 0xe163e493, 0x10b9e139, 0xa82cf3ac,
    0x59f6f606, 0x90e9feb9, 0x6133fb13, 0x27ebb33b, 0xd631b691,
    0x1f2ebe2e, 0xeef4bb84, 0x5661a911, 0xa7bbacbb, 0x6ea4a404,
    0x9f7ea1ae, 0xc4ff876f, 0x352582c5, 0xfc3a8a7a, 0x0de08fd0,
    0xb5759d45, 0x44af98ef, 0x8db09050, 0x7c6a95fa, 0x7565bba4,
    0x84bfbe0e, 0x4da0b6b1, 0xbc7ab31b, 0x04efa18e, 0xf535a424,
    0x3c2aac9b, 0xcdf0a931, 0x96718ff0, 0x67ab8a5a, 0xaeb482e5,
    0x5f6e874f, 0xe7fb95da, 0x16219070, 0xdf3e98cf, 0x2ee49d65,
    0x683cd54d, 0x99e6d0e7, 0x50f9d858, 0xa123ddf2, 0x19b6cf67,
    0xe86ccacd, 0x2173c272, 0xd0a9c7d8, 0x8b28e119, 0x7af2e4b3,
    0xb3edec0c, 0x4237e9a6, 0xfaa2fb33, 0x0b78fe99, 0xc267f626,
    0x33bdf38c, 0x4fd76676, 0xbe0d63dc, 0x77126b63, 0x86c86ec9,
    0x3e5d7c5c, 0xcf8779f6, 0x06987149, 0xf74274e3, 0xacc35222,
    0x5d195788, 0x94065f37, 0x65dc5a9d, 0xdd494808, 0x2c934da2,
    0xe58c451d, 0x145640b7, 0x528e089f, 0xa3540d35, 0x6a4b058a,
    0x9b910020, 0x230412b5, 0xd2de171f, 0x1bc11fa0, 0xea1b1a0a,
    0xb19a3ccb, 0x40403961, 0x895f31de, 0x78853474, 0xc01026e1,
    0x31ca234b, 0xf8d52bf4, 0x090f2e5e, 0xeacb7748, 0x1b1172e2,
    0xd20e7a5d, 0x23d47ff7, 0x9b416d62, 0x6a9b68c8, 0xa3846077,
    0x525e65dd, 0x09df431c, 0xf80546b6, 0x311a4e09, 0xc0c04ba3,
    0x78555936, 0x898f5c9c, 0x40905423, 0xb14a5189, 0xf79219a1,
    0x06481c0b, 0xcf5714b4, 0x3e8d111e, 0x8618038b, 0x77c20621,
    0xbedd0e9e, 0x4f070b34, 0x14862df5, 0xe55c285f, 0x2c4320e0,
    0xdd99254a, 0x650c37df, 0x94d63275, 0x5dc93aca, 0xac133f60,
    0xd079aa9a, 0x21a3af30, 0xe8bca78f, 0x1966a225, 0xa1f3b0b0,
    0x5029b51a, 0x9936bda5, 0x68ecb80f, 0x336d9ece, 0xc2b79b64,
    0x0ba893db, 0xfa729671, 0x42e784e4, 0xb33d814e, 0x7a2289f1,
    0x8bf88c5b, 0xcd20c473, 0x3cfac1d9, 0xf5e5c966, 0x043fcccc,
    0xbcaade59, 0x4d70dbf3, 0x846fd34c, 0x75b5d6e6, 0x2e34f027,
    0xdfeef58d, 0x16f1fd32, 0xe72bf898, 0x5fbeea0d, 0xae64efa7,
    0x677be718, 0x96a1e2b2, 0x9faeccec, 0x6e74c946, 0xa76bc1f9,
    0x56b1c453, 0xee24d6c6, 0x1ffed36c, 0xd6e1dbd3, 0x273bde79,
    0x7cbaf8b8, 0x8d60fd12, 0x447ff5ad, 0xb5a5f007, 0x0d30e292,
    0xfceae738, 0x35f5ef87, 0xc42fea2d, 0x82f7a205, 0x732da7af,
    0xba32af10, 0x4be8aaba, 0xf37db82f, 0x02a7bd85, 0xcbb8b53a,
    0x3a62b090, 0x61e39651, 0x903993fb, 0x59269b44, 0xa8fc9eee,
    0x10698c7b, 0xe1b389d1, 0x28ac816e, 0xd97684c4, 0xa51c113e,
    0x54c61494, 0x9dd91c2b, 0x6c031981, 0xd4960b14, 0x254c0ebe,
    0xec530601, 0x1d8903ab, 0x4608256a, 0xb7d220c0, 0x7ecd287f,
    0x8f172dd5, 0x37823f40, 0xc6583aea, 0x0f473255, 0xfe9d37ff,
    0xb8457fd7, 0x499f7a7d, 0x808072c2, 0x715a7768, 0xc9cf65fd,
    0x38156057, 0xf10a68e8, 0x00d06d42, 0x5b514b83, 0xaa8b4e29,
    0x63944696, 0x924e433c, 0x2adb51a9, 0xdb015403, 0x121e5cbc,
    0xe3c45916},
   {0x00000000, 0x0ee7e8d1, 0x1dcfd1a2, 0x13283973, 0x3b9fa344,
    0x35784b95, 0x265072e6, 0x28b79a37, 0x773f4688, 0x79d8ae59,
    0x6af0972a, 0x64177ffb, 0x4ca0e5cc, 0x42470d1d, 0x516f346e,
    0x5f88dcbf, 0xee7e8d10, 0xe09965c1, 0xf3b15cb2, 0xfd56b463,
    0xd5e12e54, 0xdb06c685, 0xc82efff6, 0xc6c91727, 0x9941cb98,
    0x97a62349, 0x848e1a3a, 0x8a69f2eb, 0xa2de68dc, 0xac39800d,
    0xbf11b97e, 0xb1f651af, 0x078c1c61, 0x096bf4b0, 0x1a43cdc3,
    0x14a42512, 0x3c13bf25, 0x32f457f4, 0x21dc6e87, 0x2f3b8656,
    0x70b35ae9, 0x7e54b238, 0x6d7c8b4b, 0x639b639a, 0x4b2cf9ad,
    0x45cb117c, 0x56e3280f, 0x5804c0de, 0xe9f29171, 0xe71579a0,
    0xf43d40d3, 0xfadaa802, 0xd26d3235, 0xdc8adae4, 0xcfa2e397,
    0xc1450b46, 0x9ecdd7f9, 0x902a3f28, 0x8302065b, 0x8de5ee8a,
    0xa55274bd, 0xabb59c6c, 0xb89da51f, 0xb67a4dce, 0x0f1838c2,
    0x01ffd013, 0x12d7e960, 0x1c3001b1, 0x34879b86, 0x3a607357,
    0x29484a24, 0x27afa2f5, 0x78277e4a, 0x76c0969b, 0x65e8afe8,
    0x6b0f4739, 0x43b8dd0e, 0x4d5f35df, 0x5e770cac, 0x5090e47d,
    0xe166b5d2, 0xef815d03, 0xfca96470, 0xf24e8ca1, 0xdaf91696,
    0xd41efe47, 0xc736c734, 0xc9d12fe5, 0x9659f35a, 0x98be1b8b,
    0x8b9622f8, 0x8571ca29, 0xadc6501e, 0xa321b8cf, 0xb00981bc,
    0xbeee696d, 0x089424a3, 0x0673cc72, 0x155bf501, 0x1bbc1dd0,
    0x330b87e7, 0x3dec6f36, 0x2ec45645, 0x2023be94, 0x7fab622b,
    0x714c8afa, 0x6264b389, 0x6c835b58, 0x4434c16f, 0x4ad329be,
    0x59fb10cd, 0x571cf81c, 0xe6eaa9b3, 0xe80d4162, 0xfb257811,
    0xf5c290c0, 0xdd750af7, 0xd392e226, 0xc0badb55, 0xce5d3384,
    0x91d5ef3b, 0x9f3207ea, 0x8c1a3e99, 0x82fdd648, 0xaa4a4c7f,
    0xa4ada4ae, 0xb7859ddd, 0xb962750c, 0x1e307184, 0x10d79955,
    0x03ffa026, 0x0d1848f7, 0x25afd2c0, 0x2b483a11, 0x38600362,
    0x3687ebb3, 0x690f370c, 0x67e8dfdd, 0x74c0e6ae, 0x7a270e7f,
    0x52909448, 0x5c777c99, 0x4f5f45ea, 0x41b8ad3b, 0xf04efc94,
    0xfea91445, 0xed812d36, 0xe366c5e7, 0xcbd15fd0, 0xc536b701,
    0xd61e8e72, 0xd8f966a3, 0x8771ba1c, 0x899652cd, 0x9abe6bbe,
    0x9459836f, 0xbcee1958, 0xb209f189, 0xa121c8fa, 0xafc6202b,
    0x19bc6de5, 0x175b8534, 0x0473bc47, 0x0a945496, 0x2223cea1,
    0x2cc42670, 0x3fec1f03, 0x310bf7d2, 0x6e832b6d, 0x6064c3bc,
    0x734cfacf, 0x7dab121e, 0x551c8829, 0x5bfb60f8, 0x48d3598b,
    0x4634b15a, 0xf7c2e0f5, 0xf9250824, 0xea0d3157, 0xe4ead986,
    0xcc5d43b1, 0xc2baab60, 0xd1929213, 0xdf757ac2, 0x80fda67d,
    0x8e1a4eac, 0x9d3277df, 0x93d59f0e, 0xbb620539, 0xb585ede8,
    0xa6add49b, 0xa84a3c4a, 0x11284946, 0x1fcfa197, 0x0ce798e4,
    0x02007035, 0x2ab7ea02, 0x245002d3, 0x37783ba0, 0x399fd371,
    0x66170fce, 0x68f0e71f, 0x7bd8de6c, 0x753f36bd, 0x5d88ac8a,
    0x536f445b, 0x40477d28, 0x4ea095f9, 0xff56c456, 0xf1b12c87,
    0xe29915f4, 0xec7efd25, 0xc4c96712, 0xca2e8fc3, 0xd906b6b0,
    0xd7e15e61, 0x886982de, 0x868e6a0f, 0x95a6537c, 0x9b41bbad,
    0xb3f6219a, 0xbd11c94b, 0xae39f038, 0xa0de18e9, 0x16a45527,
    0x1843bdf6, 0x0b6b8485, 0x058c6c54, 0x2d3bf663, 0x23dc1eb2,
    0x30f427c1, 0x3e13cf10, 0x619b13af, 0x6f7cfb7e, 0x7c54c20d,
    0x72b32adc, 0x5a04b0eb, 0x54e3583a, 0x47cb6149, 0x492c8998,
    0xf8dad837, 0xf63d30e6, 0xe5150995, 0xebf2e144, 0xc3457b73,
    0xcda293a2, 0xde8aaad1, 0xd06d4200, 0x8fe59ebf, 0x8102766e,
    0x922a4f1d, 0x9ccda7cc, 0xb47a3dfb, 0xba9dd52a, 0xa9b5ec59,
    0xa7520488},
   {0x00000000, 0x3c60e308, 0x78c1c610, 0x44a12518, 0xf1838c20,
    0xcde36f28, 0x89424a30, 0xb522a938, 0x38761e01, 0x0416fd09,
    0x40b7d811, 0x7cd73b19, 0xc9f59221, 0xf5957129, 0xb1345431,
    0x8d54b739, 0x70ec3c02, 0x4c8cdf0a, 0x082dfa12, 0x344d191a,
    0x816fb022, 0xbd0f532a, 0xf9ae7632, 0xc5ce953a, 0x489a2203,
    0x74fac10b, 0x305be413, 0x0c3b071b, 0xb919ae23, 0x85794d2b,
    0xc1d86833, 0xfdb88b3b, 0xe1d87804, 0xddb89b0c, 0x9919be14,
    0xa5795d1c, 0x105bf424, 0x2c3b172c, 0x689a3234, 0x54fad13c,
    0xd9ae6605, 0xe5ce850d, 0xa16fa015, 0x9d0f431d, 0x282dea25,
    0x144d092d, 0x50ec2c35, 0x6c8ccf3d, 0x91344406, 0xad54a70e,
    0xe9f58216, 0xd595611e, 0x60b7c826, 0x5cd72b2e, 0x18760e36,
    0x2416ed3e, 0xa9425a07, 0x9522b90f, 0xd1839c17, 0xede37f1f,
    0x58c1d627, 0x64a1352f, 0x20001037, 0x1c60f33f, 0x18c1f649,
    0x24a11541, 0x60003059, 0x5c60d351, 0xe9427a69, 0xd5229961,
    0x9183bc79, 0xade35f71, 0x20b7e848, 0x1cd70b40, 0x58762e58,
    0x6416cd50, 0xd1346468, 0xed548760, 0xa9f5a278, 0x95954170,
    0x682dca4b, 0x544d2943, 0x10ec0c5b, 0x2c8cef53, 0x99ae466b,
    0xa5cea563, 0xe16f807b, 0xdd0f6373, 0x505bd44a, 0x6c3b3742,
    0x289a125a, 0x14faf152, 0xa1d8586a, 0x9db8bb62, 0xd9199e7a,
    0xe5797d72, 0xf9198e4d, 0xc5796d45, 0x81d8485d, 0xbdb8ab55,
    0x089a026d, 0x34fae165, 0x705bc47d, 0x4c3b2775, 0xc16f904c,
    0xfd0f7344, 0xb9ae565c, 0x85ceb554, 0x30ec1c6c, 0x0c8cff64,
    0x482dda7c, 0x744d3974, 0x89f5b24f, 0xb5955147, 0xf134745f,
    0xcd549757, 0x78763e6f, 0x4416dd67, 0x00b7f87f, 0x3cd71b77,
    0xb183ac4e, 0x8de34f46, 0xc9426a5e, 0xf5228956, 0x4000206e,
    0x7c60c366, 0x38c1e67e, 0x04a10576, 0x3183ec92, 0x0de30f9a,
    0x49422a82, 0x7522c98a, 0xc00060b2, 0xfc6083ba, 0xb8c1a6a2,
    0x84a145aa, 0x09f5f293, 0x3595119b, 0x71343483, 0x4d54d78b,
    0xf8767eb3, 0xc4169dbb, 0x80b7b8a3, 0xbcd75bab, 0x416fd090,
    0x7d0f3398, 0x39ae1680, 0x05cef588, 0xb0ec5cb0, 0x8c8cbfb8,
    0xc82d9aa0, 0xf44d79a8, 0x7919ce91, 0x45792d99, 0x01d80881,
    0x3db8eb89, 0x889a42b1, 0xb4faa1b9, 0xf05b84a1, 0xcc3b67a9,
    0xd05b9496, 0xec3b779e, 0xa89a5286, 0x94fab18e, 0x21d818b6,
    0x1db8fbbe, 0x5919dea6, 0x65793dae, 0xe82d8a97, 0xd44d699f,
    0x90ec4c87, 0xac8caf8f, 0x19ae06b7, 0x25cee5bf, 0x616fc0a7,
    0x5d0f23af, 0xa0b7a894, 0x9cd74b9c, 0xd8766e84, 0xe4168d8c,
    0x513424b4, 0x6d54c7bc, 0x29f5e2a4, 0x159501ac, 0x98c1b695,
    0xa4a1559d, 0xe0007085, 0xdc60938d, 0x69423ab5, 0x5522d9bd,
    0x1183fca5, 0x2de31fad, 0x29421adb, 0x1522f9d3, 0x5183dccb,
    0x6de33fc3, 0xd8c196fb, 0xe4a175f3, 0xa00050eb, 0x9c60b3e3,
    0x113404da, 0x2d54e7d2, 0x69f5c2ca, 0x559521c2, 0xe0b788fa,
    0xdcd76bf2, 0x98764eea, 0xa416ade2, 0x59ae26d9, 0x65cec5d1,
    0x216fe0c9, 0x1d0f03c1, 0xa82daaf9, 0x944d49f1, 0xd0ec6ce9,
    0xec8c8fe1, 0x61d838d8, 0x5db8dbd0, 0x1919fec8, 0x25791dc0,
    0x905bb4f8, 0xac3b57f0, 0xe89a72e8, 0xd4fa91e0, 0xc89a62df,
    0xf4fa81d7, 0xb05ba4cf, 0x8c3b47c7, 0x3919eeff, 0x05790df7,
    0x41d828ef, 0x7db8cbe7, 0xf0ec7cde, 0xcc8c9fd6, 0x882dbace,
    0xb44d59c6, 0x016ff0fe, 0x3d0f13f6, 0x79ae36ee, 0x45ced5e6,
    0xb8765edd, 0x8416bdd5, 0xc0b798cd, 0xfcd77bc5, 0x49f5d2fd,
    0x759531f5, 0x313414ed, 0x0d54f7e5, 0x800040dc, 0xbc60a3d4,
    0xf8c186cc, 0xc4a165c4, 0x7183ccfc, 0x4de32ff4, 0x09420aec,
    0x3522e9e4},
   {0x00000000, 0x6307d924, 0xc60fb248, 0xa5086b6c, 0x576e62d1,
    0x3469bbf5, 0x9161d099, 0xf26609bd, 0xaedcc5a2, 0xcddb1c86,
    0x68d377ea, 0x0bd4aece, 0xf9b2a773, 0x9ab57e57, 0x3fbd153b,
    0x5cbacc1f, 0x86c88d05, 0xe5cf5421, 0x40c73f4d, 0x23c0e669,
    0xd1a6efd4, 0xb2a136f0, 0x17a95d9c, 0x74ae84b8, 0x281448a7,
    0x4b139183, 0xee1bfaef, 0x8d1c23cb, 0x7f7a2a76, 0x1c7df352,
    0xb975983e, 0xda72411a, 0xd6e01c4b, 0xb5e7c56f, 0x10efae03,
    0x73e87727, 0x818e7e9a, 0xe289a7be, 0x4781ccd2, 0x248615f6,
    0x783cd9e9, 0x1b3b00cd, 0xbe336ba1, 0xdd34b285, 0x2f52bb38,
    0x4c55621c, 0xe95d0970, 0x8a5ad054, 0x5028914e, 0x332f486a,
    0x96272306, 0xf520fa22, 0x0746f39f, 0x64412abb, 0xc14941d7,
    0xa24e98f3, 0xfef454ec, 0x9df38dc8, 0x38fbe6a4, 0x5bfc3f80,
    0xa99a363d, 0xca9def19, 0x6f958475, 0x0c925d51, 0x76b13ed7,
    0x15b6e7f3, 0xb0be8c9f, 0xd3b955bb, 0x21df5c06, 0x42d88522,
    0xe7d0ee4e, 0x84d7376a, 0xd86dfb75, 0xbb6a2251, 0x1e62493d,
    0x7d659019, 0x8f0399a4, 0xec044080, 0x490c2bec, 0x2a0bf2c8,
    0xf079b3d2, 0x937e6af6, 0x3676019a, 0x5571d8be, 0xa717d103,
    0xc4100827, 0x6118634b, 0x021fba6f, 0x5ea57670, 0x3da2af54,
    0x98aac438, 0xfbad1d1c, 0x09cb14a1, 0x6acccd85, 0xcfc4a6e9,
    0xacc37fcd, 0xa051229c, 0xc356fbb8, 0x665e90d4, 0x055949f0,
    0xf73f404d, 0x94389969, 0x3130f205, 0x52372b21, 0x0e8de73e,
    0x6d8a3e1a, 0xc8825576, 0xab858c52, 0x59e385ef, 0x3ae45ccb,
    0x9fec37a7, 0xfcebee83, 0x2699af99, 0x459e76bd, 0xe0961dd1,
    0x8391c4f5, 0x71f7cd48, 0x12f0146c, 0xb7f87f00, 0xd4ffa624,
    0x88456a3b, 0xeb42b31f, 0x4e4ad873, 0x2d4d0157, 0xdf2b08ea,
    0xbc2cd1ce, 0x1924baa2, 0x7a236386, 0xed627dae, 0x8e65a48a,
    0x2b6dcfe6, 0x486a16c2, 0xba0c1f7f, 0xd90bc65b, 0x7c03ad37,
    0x1f047413, 0x43beb80c, 0x20b96128, 0x85b10a44, 0xe6b6d360,
    0x14d0dadd, 0x77d703f9, 0xd2df6895, 0xb1d8b1b1, 0x6baaf0ab,
    0x08ad298f, 0xada542e3, 0xcea29bc7, 0x3cc4927a, 0x5fc34b5e,
    0xfacb2032, 0x99ccf916, 0xc5763509, 0xa671ec2d, 0x03798741,
    0x607e5e65, 0x921857d8, 0xf11f8efc, 0x5417e590, 0x37103cb4,
    0x3b8261e5, 0x5885b8c1, 0xfd8dd3ad, 0x9e8a0a89, 0x6cec0334,
    0x0febda10, 0xaae3b17c, 0xc9e46858, 0x955ea447, 0xf6597d63,
    0x5351160f, 0x3056cf2b, 0xc230c696, 0xa1371fb2, 0x043f74de,
    0x6738adfa, 0xbd4aece0, 0xde4d35c4, 0x7b455ea8, 0x1842878c,
    0xea248e31, 0x89235715, 0x2c2b3c79, 0x4f2ce55d, 0x13962942,
    0x7091f066, 0xd5999b0a, 0xb69e422e, 0x44f84b93, 0x27ff92b7,
    0x82f7f9db, 0xe1f020ff, 0x9bd34379, 0xf8d49a5d, 0x5ddcf131,
    0x3edb2815, 0xccbd21a8, 0xafbaf88c, 0x0ab293e0, 0x69b54ac4,
    0x350f86db, 0x56085fff, 0xf3003493, 0x9007edb7, 0x6261e40a,
    0x01663d2e, 0xa46e5642, 0xc7698f66, 0x1d1bce7c, 0x7e1c1758,
    0xdb147c34, 0xb813a510, 0x4a75acad, 0x29727589, 0x8c7a1ee5,
    0xef7dc7c1, 0xb3c70bde, 0xd0c0d2fa, 0x75c8b996, 0x16cf60b2,
    0xe4a9690f, 0x87aeb02b, 0x22a6db47, 0x41a10263, 0x4d335f32,
    0x2e348616, 0x8b3ced7a, 0xe83b345e, 0x1a5d3de3, 0x795ae4c7,
    0xdc528fab, 0xbf55568f, 0xe3ef9a90, 0x80e843b4, 0x25e028d8,
    0x46e7f1fc, 0xb481f841, 0xd7862165, 0x728e4a09, 0x1189932d,
    0xcbfbd237, 0xa8fc0b13, 0x0df4607f, 0x6ef3b95b, 0x9c95b0e6,
    0xff9269c2, 0x5a9a02ae, 0x399ddb8a, 0x65271795, 0x0620ceb1,
    0xa328a5dd, 0xc02f7cf9, 0x32497544, 0x514eac60, 0xf446c70c,
    0x97411e28},
   {0x00000000, 0x01b5fd1d, 0x036bfa3a, 0x02de0727, 0x06d7f474,
    0x07620969, 0x05bc0e4e, 0x0409f353, 0x0dafe8e8, 0x0c1a15f5,
    0x0ec412d2, 0x0f71efcf, 0x0b781c9c, 0x0acde181, 0x0813e6a6,
    0x09a61bbb, 0x1b5fd1d0, 0x1aea2ccd, 0x18342bea, 0x1981d6f7,
    0x1d8825a4, 0x1c3dd8b9, 0x1ee3df9e, 0x1f562283, 0x16f03938,
    0x1745c425, 0x159bc302, 0x142e3e1f, 0x1027cd4c, 0x11923051,
    0x134c3776, 0x12f9ca6b, 0x36bfa3a0, 0x370a5ebd, 0x35d4599a,
    0x3461a487, 0x306857d4, 0x31ddaac9, 0x3303adee, 0x32b650f3,
    0x3b104b48, 0x3aa5b655, 0x387bb172, 0x39ce4c6f, 0x3dc7bf3c,
    0x3c724221, 0x3eac4506, 0x3f19b81b, 0x2de07270, 0x2c558f6d,
    0x2e8b884a, 0x2f3e7557, 0x2b378604, 0x2a827b19, 0x285c7c3e,
    0x29e98123, 0x204f9a98, 0x21fa6785, 0x232460a2, 0x22919dbf,
    0x26986eec, 0x272d93f1, 0x25f394d6, 0x244669cb, 0x6d7f4740,
    0x6ccaba5d, 0x6e14bd7a, 0x6fa14067, 0x6ba8b334, 0x6a1d4e29,
    0x68c3490e, 0x6976b413, 0x60d0afa8, 0x616552b5, 0x63bb5592,
    0x620ea88f, 0x66075bdc, 0x67b2a6c1, 0x656ca1e6, 0x64d95cfb,
    0x76209690, 0x77956b8d, 0x754b6caa, 0x74fe91b7, 0x70f762e4,
    0x71429ff9, 0x739c98de, 0x722965c3, 0x7b8f7e78, 0x7a3a8365,
    0x78e48442, 0x7951795f, 0x7d588a0c, 0x7ced7711, 0x7e337036,
    0x7f868d2b, 0x5bc0e4e0, 0x5a7519fd, 0x58ab1eda, 0x591ee3c7,
    0x5d171094, 0x5ca2ed89, 0x5e7ceaae, 0x5fc917b3, 0x566f0c08,
    0x57daf115, 0x5504f632, 0x54b10b2f, 0x50b8f87c, 0x510d0561,
    0x53d30246, 0x5266ff5b, 0x409f3530, 0x412ac82d, 0x43f4cf0a,
    0x42413217, 0x4648c144, 0x47fd3c59, 0x45233b7e, 0x4496c663,
    0x4d30ddd8, 0x4c8520c5, 0x4e5b27e2, 0x4feedaff, 0x4be729ac,
    0x4a52d4b1, 0x488cd396, 0x49392e8b, 0xdafe8e80, 0xdb4b739d,
    0xd99574ba, 0xd82089a7, 0xdc297af4, 0xdd9c87e9, 0xdf4280ce,
    0xdef77dd3, 0xd7516668, 0xd6e49b75, 0xd43a9c52, 0xd58f614f,
    0xd186921c, 0xd0336f01, 0xd2ed6826, 0xd358953b, 0xc1a15f50,
    0xc014a24d, 0xc2caa56a, 0xc37f5877, 0xc776ab24, 0xc6c35639,
    0xc41d511e, 0xc5a8ac03, 0xcc0eb7b8, 0xcdbb4aa5, 0xcf654d82,
    0xced0b09f, 0xcad943cc, 0xcb6cbed1, 0xc9b2b9f6, 0xc80744eb,
    0xec412d20, 0xedf4d03d, 0xef2ad71a, 0xee9f2a07, 0xea96d954,
    0xeb232449, 0xe9fd236e, 0xe848de73, 0xe1eec5c8, 0xe05b38d5,
    0xe2853ff2, 0xe330c2ef, 0xe73931bc, 0xe68ccca1, 0xe452cb86,
    0xe5e7369b, 0xf71efcf0, 0xf6ab01ed, 0xf47506ca, 0xf5c0fbd7,
    0xf1c90884, 0xf07cf599, 0xf2a2f2be, 0xf3170fa3, 0xfab11418,
    0xfb04e905, 0xf9daee22, 0xf86f133f, 0xfc66e06c, 0xfdd31d71,
    0xff0d1a56, 0xfeb8e74b, 0xb781c9c0, 0xb63434dd, 0xb4ea33fa,
    0xb55fcee7, 0xb1563db4, 0xb0e3c0a9, 0xb23dc78e, 0xb3883a93,
    0xba2e2128, 0xbb9bdc35, 0xb945db12, 0xb8f0260f, 0xbcf9d55c,
    0xbd4c2841, 0xbf922f66, 0xbe27d27b, 0xacde1810, 0xad6be50d,
    0xafb5e22a, 0xae001f37, 0xaa09ec64, 0xabbc1179, 0xa962165e,
    0xa8d7eb43, 0xa171f0f8, 0xa0c40de5, 0xa21a0ac2, 0xa3aff7df,
    0xa7a6048c, 0xa613f991, 0xa4cdfeb6, 0xa57803ab, 0x813e6a60,
    0x808b977d, 0x8255905a, 0x83e06d47, 0x87e99e14, 0x865c6309,
    0x8482642e, 0x85379933, 0x8c918288, 0x8d247f95, 0x8ffa78b2,
    0x8e4f85af, 0x8a4676fc, 0x8bf38be1, 0x892d8cc6, 0x889871db,
    0x9a61bbb0, 0x9bd446ad, 0x990a418a, 0x98bfbc97, 0x9cb64fc4,
    0x9d03b2d9, 0x9fddb5fe, 0x9e6848e3, 0x97ce5358, 0x967bae45,
    0x94a5a962, 0x9510547f, 0x9119a72c, 0x90ac5a31, 0x92725d16,
    0x93c7a00b},
   {0x00000000, 0x6e8c1b41, 0xdd183682, 0xb3942dc3, 0x61416b45,
    0x0fcd7004, 0xbc595dc7, 0xd2d54686, 0xc282d68a, 0xac0ecdcb,
    0x1f9ae008, 0x7116fb49, 0xa3c3bdcf, 0xcd4fa68e, 0x7edb8b4d,
    0x1057900c, 0x5e74ab55, 0x30f8b014, 0x836c9dd7, 0xede08696,
    0x3f35c010, 0x51b9db51, 0xe22df692, 0x8ca1edd3, 0x9cf67ddf,
    0xf27a669e, 0x41ee4b5d, 0x2f62501c, 0xfdb7169a, 0x933b0ddb,
    0x20af2018, 0x4e233b59, 0xbce956aa, 0xd2654deb, 0x61f16028,
    0x0f7d7b69, 0xdda83def, 0xb32426ae, 0x00b00b6d, 0x6e3c102c,
    0x7e6b8020, 0x10e79b61, 0xa373b6a2, 0xcdffade3, 0x1f2aeb65,
    0x71a6f024, 0xc232dde7, 0xacbec6a6, 0xe29dfdff, 0x8c11e6be,
    0x3f85cb7d, 0x5109d03c, 0x83dc96ba, 0xed508dfb, 0x5ec4a038,
    0x3048bb79, 0x201f2b75, 0x4e933034, 0xfd071df7, 0x938b06b6,
    0x415e4030, 0x2fd25b71, 0x9c4676b2, 0xf2ca6df3, 0xa2a3ab15,
    0xcc2fb054, 0x7fbb9d97, 0x113786d6, 0xc3e2c050, 0xad6edb11,
    0x1efaf6d2, 0x7076ed93, 0x60217d9f, 0x0ead66de, 0xbd394b1d,
    0xd3b5505c, 0x016016da, 0x6fec0d9b, 0xdc782058, 0xb2f43b19,
    0xfcd70040, 0x925b1b01, 0x21cf36c2, 0x4f432d83, 0x9d966b05,
    0xf31a7044, 0x408e5d87, 0x2e0246c6, 0x3e55d6ca, 0x50d9cd8b,
    0xe34de048, 0x8dc1fb09, 0x5f14bd8f, 0x3198a6ce, 0x820c8b0d,
    0xec80904c, 0x1e4afdbf, 0x70c6e6fe, 0xc352cb3d, 0xadded07c,
    0x7f0b96fa, 0x11878dbb, 0xa213a078, 0xcc9fbb39, 0xdcc82b35,
    0xb2443074, 0x01d01db7, 0x6f5c06f6, 0xbd894070, 0xd3055b31,
    0x609176f2, 0x0e1d6db3, 0x403e56ea, 0x2eb24dab, 0x9d266068,
    0xf3aa7b29, 0x217f3daf, 0x4ff326ee, 0xfc670b2d, 0x92eb106c,
    0x82bc8060, 0xec309b21, 0x5fa4b6e2, 0x3128ada3, 0xe3fdeb25,
    0x8d71f064, 0x3ee5dda7, 0x5069c6e6, 0x9e36506b, 0xf0ba4b2a,
    0x432e66e9, 0x2da27da8, 0xff773b2e, 0x91fb206f, 0x226f0dac,
    0x4ce316ed, 0x5cb486e1, 0x32389da0, 0x81acb063, 0xef20ab22,
    0x3df5eda4, 0x5379f6e5, 0xe0eddb26, 0x8e61c067, 0xc042fb3e,
    0xaecee07f, 0x1d5acdbc, 0x73d6d6fd, 0xa103907b, 0xcf8f8b3a,
    0x7c1ba6f9, 0x1297bdb8, 0x02c02db4, 0x6c4c36f5, 0xdfd81b36,
    0xb1540077, 0x638146f1, 0x0d0d5db0, 0xbe997073, 0xd0156b32,
    0x22df06c1, 0x4c531d80, 0xffc73043, 0x914b2b02, 0x439e6d84,
    0x2d1276c5, 0x9e865b06, 0xf00a4047, 0xe05dd04b, 0x8ed1cb0a,
    0x3d45e6c9, 0x53c9fd88, 0x811cbb0e, 0xef90a04f, 0x5c048d8c,
    0x328896cd, 0x7cabad94, 0x1227b6d5, 0xa1b39b16, 0xcf3f8057,
    0x1deac6d1, 0x7366dd90, 0xc0f2f053, 0xae7eeb12, 0xbe297b1e,
    0xd0a5605f, 0x63314d9c, 0x0dbd56dd, 0xdf68105b, 0xb1e40b1a,
    0x027026d9, 0x6cfc3d98, 0x3c95fb7e, 0x5219e03f, 0xe18dcdfc,
    0x8f01d6bd, 0x5dd4903b, 0x33588b7a, 0x80cca6b9, 0xee40bdf8,
    0xfe172df4, 0x909b36b5, 0x230f1b76, 0x4d830037, 0x9f5646b1,
    0xf1da5df0, 0x424e7033, 0x2cc26b72, 0x62e1502b, 0x0c6d4b6a,
    0xbff966a9, 0xd1757de8, 0x03a03b6e, 0x6d2c202f, 0xdeb80dec,
    0xb03416ad, 0xa06386a1, 0xceef9de0, 0x7d7bb023, 0x13f7ab62,
    0xc122ede4, 0xafaef6a5, 0x1c3adb66, 0x72b6c027, 0x807cadd4,
    0xeef0b695, 0x5d649b56, 0x33e88017, 0xe13dc691, 0x8fb1ddd0,
    0x3c25f013, 0x52a9eb52, 0x42fe7b5e, 0x2c72601f, 0x9fe64ddc,
    0xf16a569d, 0x23bf101b, 0x4d330b5a, 0xfea72699, 0x902b3dd8,
    0xde080681, 0xb0841dc0, 0x03103003, 0x6d9c2b42, 0xbf496dc4,
    0xd1c57685, 0x62515b46, 0x0cdd4007, 0x1c8ad00b, 0x7206cb4a,
    0xc192e689, 0xaf1efdc8, 0x7dcbbb4e, 0x1347a00f, 0xa0d38dcc,
    0xce5f968d},
   {0x00000000, 0xe71da697, 0x154a4b6f, 0xf257edf8, 0x2a9496de,
    0xcd893049, 0x3fdeddb1, 0xd8c37b26, 0x55292dbc, 0xb2348b2b,
    0x406366d3, 0xa77ec044, 0x7fbdbb62, 0x98a01df5, 0x6af7f00d,
    0x8dea569a, 0xaa525b78, 0x4d4ffdef, 0xbf181017, 0x5805b680,
    0x80c6cda6, 0x67db6b31, 0x958c86c9, 0x7291205e, 0xff7b76c4,
    0x1866d053, 0xea313dab, 0x0d2c9b3c, 0xd5efe01a, 0x32f2468d,
    0xc0a5ab75, 0x27b80de2, 0x8fd5b0b1, 0x68c81626, 0x9a9ffbde,
    0x7d825d49, 0xa541266f, 0x425c80f8, 0xb00b6d00, 0x5716cb97,
    0xdafc9d0d, 0x3de13b9a, 0xcfb6d662, 0x28ab70f5, 0xf0680bd3,
    0x1775ad44, 0xe52240bc, 0x023fe62b, 0x2587ebc9, 0xc29a4d5e,
    0x30cda0a6, 0xd7d00631, 0x0f137d17, 0xe80edb80, 0x1a593678,
    0xfd4490ef, 0x70aec675, 0x97b360e2, 0x65e48d1a, 0x82f92b8d,
    0x5a3a50ab, 0xbd27f63c, 0x4f701bc4, 0xa86dbd53, 0xc4da6723,
    0x23c7c1b4, 0xd1902c4c, 0x368d8adb, 0xee4ef1fd, 0x0953576a,
    0xfb04ba92, 0x1c191c05, 0x91f34a9f, 0x76eeec08, 0x84b901f0,
    0x63a4a767, 0xbb67dc41, 0x5c7a7ad6, 0xae2d972e, 0x493031b9,
    0x6e883c5b, 0x89959acc, 0x7bc27734, 0x9cdfd1a3, 0x441caa85,
    0xa3010c12, 0x5156e1ea, 0xb64b477d, 0x3ba111e7, 0xdcbcb770,
    0x2eeb5a88, 0xc9f6fc1f, 0x11358739, 0xf62821ae, 0x047fcc56,
    0xe3626ac1, 0x4b0fd792, 0xac127105, 0x5e459cfd, 0xb9583a6a,
    0x619b414c, 0x8686e7db, 0x74d10a23, 0x93ccacb4, 0x1e26fa2e,
    0xf93b5cb9, 0x0b6cb141, 0xec7117d6, 0x34b26cf0, 0xd3afca67,
    0x21f8279f, 0xc6e58108, 0xe15d8cea, 0x06402a7d, 0xf417c785,
    0x130a6112, 0xcbc91a34, 0x2cd4bca3, 0xde83515b, 0x399ef7cc,
    0xb474a156, 0x536907c1, 0xa13eea39, 0x46234cae, 0x9ee03788,
    0x79fd911f, 0x8baa7ce7, 0x6cb7da70, 0x52c5c807, 0xb5d86e90,
    0x478f8368, 0xa09225ff, 0x78515ed9, 0x9f4cf84e, 0x6d1b15b6,
    0x8a06b321, 0x07ece5bb, 0xe0f1432c, 0x12a6aed4, 0xf5bb0843,
    0x2d787365, 0xca65d5f2, 0x3832380a, 0xdf2f9e9d, 0xf897937f,
    0x1f8a35e8, 0xedddd810, 0x0ac07e87, 0xd20305a1, 0x351ea336,
    0xc7494ece, 0x2054e859, 0xadbebec3, 0x4aa31854, 0xb8f4f5ac,
    0x5fe9533b, 0x872a281d, 0x60378e8a, 0x92606372, 0x757dc5e5,
    0xdd1078b6, 0x3a0dde21, 0xc85a33d9, 0x2f47954e, 0xf784ee68,
    0x109948ff, 0xe2cea507, 0x05d30390, 0x8839550a, 0x6f24f39d,
    0x9d731e65, 0x7a6eb8f2, 0xa2adc3d4, 0x45b06543, 0xb7e788bb,
    0x50fa2e2c, 0x774223ce, 0x905f8559, 0x620868a1, 0x8515ce36,
    0x5dd6b510, 0xbacb1387, 0x489cfe7f, 0xaf8158e8, 0x226b0e72,
    0xc576a8e5, 0x3721451d, 0xd03ce38a, 0x08ff98ac, 0xefe23e3b,
    0x1db5d3c3, 0xfaa87554, 0x961faf24, 0x710209b3, 0x8355e44b,
    0x644842dc, 0xbc8b39fa, 0x5b969f6d, 0xa9c17295, 0x4edcd402,
    0xc3368298, 0x242b240f, 0xd67cc9f7, 0x31616f60, 0xe9a21446,
    0x0ebfb2d1, 0xfce85f29, 0x1bf5f9be, 0x3c4df45c, 0xdb5052cb,
    0x2907bf33, 0xce1a19a4, 0x16d96282, 0xf1c4c415, 0x039329ed,
    0xe48e8f7a, 0x6964d9e0, 0x8e797f77, 0x7c2e928f, 0x9b333418,
    0x43f04f3e, 0xa4ede9a9, 0x56ba0451, 0xb1a7a2c6, 0x19ca1f95,
    0xfed7b902, 0x0c8054fa, 0xeb9df26d, 0x335e894b, 0xd4432fdc,
    0x2614c224, 0xc10964b3, 0x4ce33229, 0xabfe94be, 0x59a97946,
    0xbeb4dfd1, 0x6677a4f7, 0x816a0260, 0x733def98, 0x9420490f,
    0xb39844ed, 0x5485e27a, 0xa6d20f82, 0x41cfa915, 0x990cd233,
    0x7e1174a4, 0x8c46995c, 0x6b5b3fcb, 0xe6b16951, 0x01accfc6,
    0xf3fb223e, 0x14e684a9, 0xcc25ff8f, 0x2b385918, 0xd96fb4e0,
    0x3e721277},
   {0x00000000, 0xa58b900e, 0x9066265d, 0x35edb653, 0xfbbd4afb,
    0x5e36daf5, 0x6bdb6ca6, 0xce50fca8, 0x2c0b93b7, 0x898003b9,
    0xbc6db5ea, 0x19e625e4, 0xd7b6d94c, 0x723d4942, 0x47d0ff11,
    0xe25b6f1f, 0x5817276e, 0xfd9cb760, 0xc8710133, 0x6dfa913d,
    0xa3aa6d95, 0x0621fd9b, 0x33cc4bc8, 0x9647dbc6, 0x741cb4d9,
    0xd19724d7, 0xe47a9284, 0x41f1028a, 0x8fa1fe22, 0x2a2a6e2c,
    0x1fc7d87f, 0xba4c4871, 0xb02e4edc, 0x15a5ded2, 0x20486881,
    0x85c3f88f, 0x4b930427, 0xee189429, 0xdbf5227a, 0x7e7eb274,
    0x9c25dd6b, 0x39ae4d65, 0x0c43fb36, 0xa9c86b38, 0x67989790,
    0xc213079e, 0xf7feb1cd, 0x527521c3, 0xe83969b2, 0x4db2f9bc,
    0x785f4fef, 0xddd4dfe1, 0x13842349, 0xb60fb347, 0x83e20514,
    0x2669951a, 0xc432fa05, 0x61b96a0b, 0x5454dc58, 0xf1df4c56,
    0x3f8fb0fe, 0x9a0420f0, 0xafe996a3, 0x0a6206ad, 0xbb2d9bf9,
    0x1ea60bf7, 0x2b4bbda4, 0x8ec02daa, 0x4090d102, 0xe51b410c,
    0xd0f6f75f, 0x757d6751, 0x9726084e, 0x32ad9840, 0x07402e13,
    0xa2cbbe1d, 0x6c9b42b5, 0xc910d2bb, 0xfcfd64e8, 0x5976f4e6,
    0xe33abc97, 0x46b12c99, 0x735c9aca, 0xd6d70ac4, 0x1887f66c,
    0xbd0c6662, 0x88e1d031, 0x2d6a403f, 0xcf312f20, 0x6ababf2e,
    0x5f57097d, 0xfadc9973, 0x348c65db, 0x9107f5d5, 0xa4ea4386,
    0x0161d388, 0x0b03d525, 0xae88452b, 0x9b65f378, 0x3eee6376,
    0xf0be9fde, 0x55350fd0, 0x60d8b983, 0xc553298d, 0x27084692,
    0x8283d69c, 0xb76e60cf, 0x12e5f0c1, 0xdcb50c69, 0x793e9c67,
    0x4cd32a34, 0xe958ba3a, 0x5314f24b, 0xf69f6245, 0xc372d416,
    0x66f94418, 0xa8a9b8b0, 0x0d2228be, 0x38cf9eed, 0x9d440ee3,
    0x7f1f61fc, 0xda94f1f2, 0xef7947a1, 0x4af2d7af, 0x84a22b07,
    0x2129bb09, 0x14c40d5a, 0xb14f9d54, 0xad2a31b3, 0x08a1a1bd,
    0x3d4c17ee, 0x98c787e0, 0x56977b48, 0xf31ceb46, 0xc6f15d15,
    0x637acd1b, 0x8121a204, 0x24aa320a, 0x11478459, 0xb4cc1457,
    0x7a9ce8ff, 0xdf1778f1, 0xeafacea2, 0x4f715eac, 0xf53d16dd,
    0x50b686d3, 0x655b3080, 0xc0d0a08e, 0x0e805c26, 0xab0bcc28,
    0x9ee67a7b, 0x3b6dea75, 0xd936856a, 0x7cbd1564, 0x4950a337,
    0xecdb3339, 0x228bcf91, 0x87005f9f, 0xb2ede9cc, 0x176679c2,
    0x1d047f6f, 0xb88fef61, 0x8d625932, 0x28e9c93c, 0xe6b93594,
    0x4332a59a, 0x76df13c9, 0xd35483c7, 0x310fecd8, 0x94847cd6,
    0xa169ca85, 0x04e25a8b, 0xcab2a623, 0x6f39362d, 0x5ad4807e,
    0xff5f1070, 0x45135801, 0xe098c80f, 0xd5757e5c, 0x70feee52,
    0xbeae12fa, 0x1b2582f4, 0x2ec834a7, 0x8b43a4a9, 0x6918cbb6,
    0xcc935bb8, 0xf97eedeb, 0x5cf57de5, 0x92a5814d, 0x372e1143,
    0x02c3a710, 0xa748371e, 0x1607aa4a, 0xb38c3a44, 0x86618c17,
    0x23ea1c19, 0xedbae0b1, 0x483170bf, 0x7ddcc6ec, 0xd85756e2,
    0x3a0c39fd, 0x9f87a9f3, 0xaa6a1fa0, 0x0fe18fae, 0xc1b17306,
    0x643ae308, 0x51d7555b, 0xf45cc555, 0x4e108d24, 0xeb9b1d2a,
    0xde76ab79, 0x7bfd3b77, 0xb5adc7df, 0x102657d1, 0x25cbe182,
    0x8040718c, 0x621b1e93, 0xc7908e9d, 0xf27d38ce, 0x57f6a8c0,
    0x99a65468, 0x3c2dc466, 0x09c07235, 0xac4be23b, 0xa629e496,
    0x03a27498, 0x364fc2cb, 0x93c452c5, 0x5d94ae6d, 0xf81f3e63,
    0xcdf28830, 0x6879183e, 0x8a227721, 0x2fa9e72f, 0x1a44517c,
    0xbfcfc172, 0x719f3dda, 0xd414add4, 0xe1f91b87, 0x44728b89,
    0xfe3ec3f8, 0x5bb553f6, 0x6e58e5a5, 0xcbd375ab, 0x05838903,
    0xa008190d, 0x95e5af5e, 0x306e3f50, 0xd235504f, 0x77bec041,
    0x42537612, 0xe7d8e61c, 0x29881ab4, 0x8c038aba, 0xb9ee3ce9,
    0x1c65ace7}};

local const z_word_t FAR crc_braid_big_table[][256] = {
   {0x0000000000000000, 0x0e908ba500000000, 0x5d26669000000000,
    0x53b6ed3500000000, 0xfb4abdfb00000000, 0xf5da365e00000000,
    0xa66cdb6b00000000, 0xa8fc50ce00000000, 0xb7930b2c00000000,
    0xb903808900000000, 0xeab56dbc00000000, 0xe425e61900000000,
    0x4cd9b6d700000000, 0x42493d7200000000, 0x11ffd04700000000,
    0x1f6f5be200000000, 0x6e27175800000000, 0x60b79cfd00000000,
    0x330171c800000000, 0x3d91fa6d00000000, 0x956daaa300000000,
    0x9bfd210600000000, 0xc84bcc3300000000, 0xc6db479600000000,
    0xd9b41c7400000000, 0xd72497d100000000, 0x84927ae400000000,
    0x8a02f14100000000, 0x22fea18f00000000, 0x2c6e2a2a00000000,
    0x7fd8c71f00000000, 0x71484cba00000000, 0xdc4e2eb000000000,
    0xd2dea51500000000, 0x8168482000000000, 0x8ff8c38500000000,
    0x2704934b00000000, 0x299418ee00000000, 0x7a22f5db00000000,
    0x74b27e7e00000000, 0x6bdd259c00000000, 0x654dae3900000000,
    0x36fb430c00000000, 0x386bc8a900000000, 0x9097986700000000,
    0x9e0713c200000000, 0xcdb1fef700000000, 0xc321755200000000,
    0xb26939e800000000, 0xbcf9b24d00000000, 0xef4f5f7800000000,
    0xe1dfd4dd00000000, 0x4923841300000000, 0x47b30fb600000000,
    0x1405e28300000000, 0x1a95692600000000, 0x05fa32c400000000,
    0x0b6ab96100000000, 0x58dc545400000000, 0x564cdff100000000,
    0xfeb08f3f00000000, 0xf020049a00000000, 0xa396e9af00000000,
    0xad06620a00000000, 0xf99b2dbb00000000, 0xf70ba61e00000000,
    0xa4bd4b2b00000000, 0xaa2dc08e00000000, 0x02d1904000000000,
    0x0c411be500000000, 0x5ff7f6d000000000, 0x51677d7500000000,
    0x4e08269700000000, 0x4098ad3200000000, 0x132e400700000000,
    0x1dbecba200000000, 0xb5429b6c00000000, 0xbbd210c900000000,
    0xe864fdfc00000000, 0xe6f4765900000000, 0x97bc3ae300000000,
    0x992cb14600000000, 0xca9a5c7300000000, 0xc40ad7d600000000,
    0x6cf6871800000000, 0x62660cbd00000000, 0x31d0e18800000000,
    0x3f406a2d00000000, 0x202f31cf00000000, 0x2ebfba6a00000000,
    0x7d09575f00000000, 0x7399dcfa00000000, 0xdb658c3400000000,
    0xd5f5079100000000, 0x8643eaa400000000, 0x88d3610100000000,
    0x25d5030b00000000, 0x2b4588ae00000000, 0x78f3659b00000000,
    0x7663ee3e00000000, 0xde9fbef000000000, 0xd00f355500000000,
    0x83b9d86000000000, 0x8d2953c500000000, 0x9246082700000000,
    0x9cd6838200000000, 0xcf606eb700000000, 0xc1f0e51200000000,
    0x690cb5dc00000000, 0x679c3e7900000000, 0x342ad34c00000000,
    0x3aba58e900000000, 0x4bf2145300000000, 0x45629ff600000000,
    0x16d472c300000000, 0x1844f96600000000, 0xb0b8a9a800000000,
    0xbe28220d00000000, 0xed9ecf3800000000, 0xe30e449d00000000,
    0xfc611f7f00000000, 0xf2f194da00000000, 0xa14779ef00000000,
    0xafd7f24a00000000, 0x072ba28400000000, 0x09bb292100000000,
    0x5a0dc41400000000, 0x549d4fb100000000, 0xb3312aad00000000,
    0xbda1a10800000000, 0xee174c3d00000000, 0xe087c79800000000,
    0x487b975600000000, 0x46eb1cf300000000, 0x155df1c600000000,
    0x1bcd7a6300000000, 0x04a2218100000000, 0x0a32aa2400000000,
    0x5984471100000000, 0x5714ccb400000000, 0xffe89c7a00000000,
    0xf17817df00000000, 0xa2cefaea00000000, 0xac5e714f00000000,
    0xdd163df500000000, 0xd386b65000000000, 0x80305b6500000000,
    0x8ea0d0c000000000, 0x265c800e00000000, 0x28cc0bab00000000,
    0x7b7ae69e00000000, 0x75ea6d3b00000000, 0x6a8536d900000000,
    0x6415bd7c00000000, 0x37a3504900000000, 0x3933dbec00000000,
    0x91cf8b2200000000, 0x9f5f008700000000, 0xcce9edb200000000,
    0xc279661700000000, 0x6f7f041d00000000, 0x61ef8fb800000000,
    0x3259628d00000000, 0x3cc9e92800000000, 0x9435b9e600000000,
    0x9aa5324300000000, 0xc913df7600000000, 0xc78354d300000000,
    0xd8ec0f3100000000, 0xd67c849400000000, 0x85ca69a100000000,
    0x8b5ae20400000000, 0x23a6b2ca00000000, 0x2d36396f00000000,
    0x7e80d45a00000000, 0x70105fff00000000, 0x0158134500000000,
    0x0fc898e000000000, 0x5c7e75d500000000, 0x52eefe7000000000,
    0xfa12aebe00000000, 0xf482251b00000000, 0xa734c82e00000000,
    0xa9a4438b00000000, 0xb6cb186900000000, 0xb85b93cc00000000,
    0xebed7ef900000000, 0xe57df55c00000000, 0x4d81a59200000000,
    0x43112e3700000000, 0x10a7c30200000000, 0x1e3748a700000000,
    0x4aaa071600000000, 0x443a8cb300000000, 0x178c618600000000,
    0x191cea2300000000, 0xb1e0baed00000000, 0xbf70314800000000,
    0xecc6dc7d00000000, 0xe25657d800000000, 0xfd390c3a00000000,
    0xf3a9879f00000000, 0xa01f6aaa00000000, 0xae8fe10f00000000,
    0x0673b1c100000000, 0x08e33a6400000000, 0x5b55d75100000000,
    0x55c55cf400000000, 0x248d104e00000000, 0x2a1d9beb00000000,
    0x79ab76de00000000, 0x773bfd7b00000000, 0xdfc7adb500000000,
    0xd157261000000000, 0x82e1cb2500000000, 0x8c71408000000000,
    0x931e1b6200000000, 0x9d8e90c700000000, 0xce387df200000000,
    0xc0a8f65700000000, 0x6854a69900000000, 0x66c42d3c00000000,
    0x3572c00900000000, 0x3be24bac00000000, 0x96e429a600000000,
    0x9874a20300000000, 0xcbc24f3600000000, 0xc552c49300000000,
    0x6dae945d00000000, 0x633e1ff800000000, 0x3088f2cd00000000,
    0x3e18796800000000, 0x2177228a00000000, 0x2fe7a92f00000000,
    0x7c51441a00000000, 0x72c1cfbf00000000, 0xda3d9f7100000000,
    0xd4ad14d400000000, 0x871bf9e100000000, 0x898b724400000000,
    0xf8c33efe00000000, 0xf653b55b00000000, 0xa5e5586e00000000,
    0xab75d3cb00000000, 0x0389830500000000, 0x0d1908a000000000,
    0x5eafe59500000000, 0x503f6e3000000000, 0x4f5035d200000000,
    0x41c0be7700000000, 0x1276534200000000, 0x1ce6d8e700000000,
    0xb41a882900000000, 0xba8a038c00000000, 0xe93ceeb900000000,
    0xe7ac651c00000000},
   {0x0000000000000000, 0x97a61de700000000, 0x6f4b4a1500000000,
    0xf8ed57f200000000, 0xde96942a00000000, 0x493089cd00000000,
    0xb1ddde3f00000000, 0x267bc3d800000000, 0xbc2d295500000000,
    0x2b8b34b200000000, 0xd366634000000000, 0x44c07ea700000000,
    0x62bbbd7f00000000, 0xf51da09800000000, 0x0df0f76a00000000,
    0x9a56ea8d00000000, 0x785b52aa00000000, 0xeffd4f4d00000000,
    0x171018bf00000000, 0x80b6055800000000, 0xa6cdc68000000000,
    0x316bdb6700000000, 0xc9868c9500000000, 0x5e20917200000000,
    0xc4767bff00000000, 0x53d0661800000000, 0xab3d31ea00000000,
    0x3c9b2c0d00000000, 0x1ae0efd500000000, 0x8d46f23200000000,
    0x75aba5c000000000, 0xe20db82700000000, 0xb1b0d58f00000000,
    0x2616c86800000000, 0xdefb9f9a00000000, 0x495d827d00000000,
    0x6f2641a500000000, 0xf8805c4200000000, 0x006d0bb000000000,
    0x97cb165700000000, 0x0d9dfcda00000000, 0x9a3be13d00000000,
    0x62d6b6cf00000000, 0xf570ab2800000000, 0xd30b68f000000000,
    0x44ad751700000000, 0xbc4022e500000000, 0x2be63f0200000000,
    0xc9eb872500000000, 0x5e4d9ac200000000, 0xa6a0cd3000000000,
    0x3106d0d700000000, 0x177d130f00000000, 0x80db0ee800000000,
    0x7836591a00000000, 0xef9044fd00000000, 0x75c6ae7000000000,
    0xe260b39700000000, 0x1a8de46500000000, 0x8d2bf98200000000,
    0xab503a5a00000000, 0x3cf627bd00000000, 0xc41b704f00000000,
    0x53bd6da800000000, 0x2367dac400000000, 0xb4c1c72300000000,
    0x4c2c90d100000000, 0xdb8a8d3600000000, 0xfdf14eee00000000,
    0x6a57530900000000, 0x92ba04fb00000000, 0x051c191c00000000,
    0x9f4af39100000000, 0x08ecee7600000000, 0xf001b98400000000,
    0x67a7a46300000000, 0x41dc67bb00000000, 0xd67a7a5c00000000,
    0x2e972dae00000000, 0xb931304900000000, 0x5b3c886e00000000,
    0xcc9a958900000000, 0x3477c27b00000000, 0xa3d1df9c00000000,
    0x85aa1c4400000000, 0x120c01a300000000, 0xeae1565100000000,
    0x7d474bb600000000, 0xe711a13b00000000, 0x70b7bcdc00000000,
    0x885aeb2e00000000, 0x1ffcf6c900000000, 0x3987351100000000,
    0xae2128f600000000, 0x56cc7f0400000000, 0xc16a62e300000000,
    0x92d70f4b00000000, 0x057112ac00000000, 0xfd9c455e00000000,
    0x6a3a58b900000000, 0x4c419b6100000000, 0xdbe7868600000000,
    0x230ad17400000000, 0xb4accc9300000000, 0x2efa261e00000000,
    0xb95c3bf900000000, 0x41b16c0b00000000, 0xd61771ec00000000,
    0xf06cb23400000000, 0x67caafd300000000, 0x9f27f82100000000,
    0x0881e5c600000000, 0xea8c5de100000000, 0x7d2a400600000000,
    0x85c717f400000000, 0x12610a1300000000, 0x341ac9cb00000000,
    0xa3bcd42c00000000, 0x5b5183de00000000, 0xccf79e3900000000,
    0x56a174b400000000, 0xc107695300000000, 0x39ea3ea100000000,
    0xae4c234600000000, 0x8837e09e00000000, 0x1f91fd7900000000,
    0xe77caa8b00000000, 0x70dab76c00000000, 0x07c8c55200000000,
    0x906ed8b500000000, 0x68838f4700000000, 0xff2592a000000000,
    0xd95e517800000000, 0x4ef84c9f00000000, 0xb6151b6d00000000,
    0x21b3068a00000000, 0xbbe5ec0700000000, 0x2c43f1e000000000,
    0xd4aea61200000000, 0x4308bbf500000000, 0x6573782d00000000,
    0xf2d565ca00000000, 0x0a38323800000000, 0x9d9e2fdf00000000,
    0x7f9397f800000000, 0xe8358a1f00000000, 0x10d8dded00000000,
    0x877ec00a00000000, 0xa10503d200000000, 0x36a31e3500000000,
    0xce4e49c700000000, 0x59e8542000000000, 0xc3bebead00000000,
    0x5418a34a00000000, 0xacf5f4b800000000, 0x3b53e95f00000000,
    0x1d282a8700000000, 0x8a8e376000000000, 0x7263609200000000,
    0xe5c57d7500000000, 0xb67810dd00000000, 0x21de0d3a00000000,
    0xd9335ac800000000, 0x4e95472f00000000, 0x68ee84f700000000,
    0xff48991000000000, 0x07a5cee200000000, 0x9003d30500000000,
    0x0a55398800000000, 0x9df3246f00000000, 0x651e739d00000000,
    0xf2b86e7a00000000, 0xd4c3ada200000000, 0x4365b04500000000,
    0xbb88e7b700000000, 0x2c2efa5000000000, 0xce23427700000000,
    0x59855f9000000000, 0xa168086200000000, 0x36ce158500000000,
    0x10b5d65d00000000, 0x8713cbba00000000, 0x7ffe9c4800000000,
    0xe85881af00000000, 0x720e6b2200000000, 0xe5a876c500000000,
    0x1d45213700000000, 0x8ae33cd000000000, 0xac98ff0800000000,
    0x3b3ee2ef00000000, 0xc3d3b51d00000000, 0x5475a8fa00000000,
    0x24af1f9600000000, 0xb309027100000000, 0x4be4558300000000,
    0xdc42486400000000, 0xfa398bbc00000000, 0x6d9f965b00000000,
    0x9572c1a900000000, 0x02d4dc4e00000000, 0x988236c300000000,
    0x0f242b2400000000, 0xf7c97cd600000000, 0x606f613100000000,
    0x4614a2e900000000, 0xd1b2bf0e00000000, 0x295fe8fc00000000,
    0xbef9f51b00000000, 0x5cf44d3c00000000, 0xcb5250db00000000,
    0x33bf072900000000, 0xa4191ace00000000, 0x8262d91600000000,
    0x15c4c4f100000000, 0xed29930300000000, 0x7a8f8ee400000000,
    0xe0d9646900000000, 0x777f798e00000000, 0x8f922e7c00000000,
    0x1834339b00000000, 0x3e4ff04300000000, 0xa9e9eda400000000,
    0x5104ba5600000000, 0xc6a2a7b100000000, 0x951fca1900000000,
    0x02b9d7fe00000000, 0xfa54800c00000000, 0x6df29deb00000000,
    0x4b895e3300000000, 0xdc2f43d400000000, 0x24c2142600000000,
    0xb36409c100000000, 0x2932e34c00000000, 0xbe94feab00000000,
    0x4679a95900000000, 0xd1dfb4be00000000, 0xf7a4776600000000,
    0x60026a8100000000, 0x98ef3d7300000000, 0x0f49209400000000,
    0xed4498b300000000, 0x7ae2855400000000, 0x820fd2a600000000,
    0x15a9cf4100000000, 0x33d20c9900000000, 0xa474117e00000000,
    0x5c99468c00000000, 0xcb3f5b6b00000000, 0x5169b1e600000000,
    0xc6cfac0100000000, 0x3e22fbf300000000, 0xa984e61400000000,
    0x8fff25cc00000000, 0x1859382b00000000, 0xe0b46fd900000000,
    0x7712723e00000000},
   {0x0000000000000000, 0x411b8c6e00000000, 0x823618dd00000000,
    0xc32d94b300000000, 0x456b416100000000, 0x0470cd0f00000000,
    0xc75d59bc00000000, 0x8646d5d200000000, 0x8ad682c200000000,
    0xcbcd0eac00000000, 0x08e09a1f00000000, 0x49fb167100000000,
    0xcfbdc3a300000000, 0x8ea64fcd00000000, 0x4d8bdb7e00000000,
    0x0c90571000000000, 0x55ab745e00000000, 0x14b0f83000000000,
    0xd79d6c8300000000, 0x9686e0ed00000000, 0x10c0353f00000000,
    0x51dbb95100000000, 0x92f62de200000000, 0xd3eda18c00000000,
    0xdf7df69c00000000, 0x9e667af200000000, 0x5d4bee4100000000,
    0x1c50622f00000000, 0x9a16b7fd00000000, 0xdb0d3b9300000000,
    0x1820af2000000000, 0x593b234e00000000, 0xaa56e9bc00000000,
    0xeb4d65d200000000, 0x2860f16100000000, 0x697b7d0f00000000,
    0xef3da8dd00000000, 0xae2624b300000000, 0x6d0bb00000000000,
    0x2c103c6e00000000, 0x20806b7e00000000, 0x619be71000000000,
    0xa2b673a300000000, 0xe3adffcd00000000, 0x65eb2a1f00000000,
    0x24f0a67100000000, 0xe7dd32c200000000, 0xa6c6beac00000000,
    0xfffd9de200000000, 0xbee6118c00000000, 0x7dcb853f00000000,
    0x3cd0095100000000, 0xba96dc8300000000, 0xfb8d50ed00000000,
    0x38a0c45e00000000, 0x79bb483000000000, 0x752b1f2000000000,
    0x3430934e00000000, 0xf71d07fd00000000, 0xb6068b9300000000,
    0x30405e4100000000, 0x715bd22f00000000, 0xb276469c00000000,
    0xf36dcaf200000000, 0x15aba3a200000000, 0x54b02fcc00000000,
    0x979dbb7f00000000, 0xd686371100000000, 0x50c0e2c300000000,
    0x11db6ead00000000, 0xd2f6fa1e00000000, 0x93ed767000000000,
    0x9f7d216000000000, 0xde66ad0e00000000, 0x1d4b39bd00000000,
    0x5c50b5d300000000, 0xda16600100000000, 0x9b0dec6f00000000,
    0x582078dc00000000, 0x193bf4b200000000, 0x4000d7fc00000000,
    0x011b5b9200000000, 0xc236cf2100000000, 0x832d434f00000000,
    0x056b969d00000000, 0x44701af300000000, 0x875d8e4000000000,
    0xc646022e00000000, 0xcad6553e00000000, 0x8bcdd95000000000,
    0x48e04de300000000, 0x09fbc18d00000000, 0x8fbd145f00000000,
    0xcea6983100000000, 0x0d8b0c8200000000, 0x4c9080ec00000000,
    0xbffd4a1e00000000, 0xfee6c67000000000, 0x3dcb52c300000000,
    0x7cd0dead00000000, 0xfa960b7f00000000, 0xbb8d871100000000,
    0x78a013a200000000, 0x39bb9fcc00000000, 0x352bc8dc00000000,
    0x743044b200000000, 0xb71dd00100000000, 0xf6065c6f00000000,
    0x704089bd00000000, 0x315b05d300000000, 0xf276916000000000,
    0xb36d1d0e00000000, 0xea563e4000000000, 0xab4db22e00000000,
    0x6860269d00000000, 0x297baaf300000000, 0xaf3d7f2100000000,
    0xee26f34f00000000, 0x2d0b67fc00000000, 0x6c10eb9200000000,
    0x6080bc8200000000, 0x219b30ec00000000, 0xe2b6a45f00000000,
    0xa3ad283100000000, 0x25ebfde300000000, 0x64f0718d00000000,
    0xa7dde53e00000000, 0xe6c6695000000000, 0x6b50369e00000000,
    0x2a4bbaf000000000, 0xe9662e4300000000, 0xa87da22d00000000,
    0x2e3b77ff00000000, 0x6f20fb9100000000, 0xac0d6f2200000000,
    0xed16e34c00000000, 0xe186b45c00000000, 0xa09d383200000000,
    0x63b0ac8100000000, 0x22ab20ef00000000, 0xa4edf53d00000000,
    0xe5f6795300000000, 0x26dbede000000000, 0x67c0618e00000000,
    0x3efb42c000000000, 0x7fe0ceae00000000, 0xbccd5a1d00000000,
    0xfdd6d67300000000, 0x7b9003a100000000, 0x3a8b8fcf00000000,
    0xf9a61b7c00000000, 0xb8bd971200000000, 0xb42dc00200000000,
    0xf5364c6c00000000, 0x361bd8df00000000, 0x770054b100000000,
    0xf146816300000000, 0xb05d0d0d00000000, 0x737099be00000000,
    0x326b15d000000000, 0xc106df2200000000, 0x801d534c00000000,
    0x4330c7ff00000000, 0x022b4b9100000000, 0x846d9e4300000000,
    0xc576122d00000000, 0x065b869e00000000, 0x47400af000000000,
    0x4bd05de000000000, 0x0acbd18e00000000, 0xc9e6453d00000000,
    0x88fdc95300000000, 0x0ebb1c8100000000, 0x4fa090ef00000000,
    0x8c8d045c00000000, 0xcd96883200000000, 0x94adab7c00000000,
    0xd5b6271200000000, 0x169bb3a100000000, 0x57803fcf00000000,
    0xd1c6ea1d00000000, 0x90dd667300000000, 0x53f0f2c000000000,
    0x12eb7eae00000000, 0x1e7b29be00000000, 0x5f60a5d000000000,
    0x9c4d316300000000, 0xdd56bd0d00000000, 0x5b1068df00000000,
    0x1a0be4b100000000, 0xd926700200000000, 0x983dfc6c00000000,
    0x7efb953c00000000, 0x3fe0195200000000, 0xfccd8de100000000,
    0xbdd6018f00000000, 0x3b90d45d00000000, 0x7a8b583300000000,
    0xb9a6cc8000000000, 0xf8bd40ee00000000, 0xf42d17fe00000000,
    0xb5369b9000000000, 0x761b0f2300000000, 0x3700834d00000000,
    0xb146569f00000000, 0xf05ddaf100000000, 0x33704e4200000000,
    0x726bc22c00000000, 0x2b50e16200000000, 0x6a4b6d0c00000000,
    0xa966f9bf00000000, 0xe87d75d100000000, 0x6e3ba00300000000,
    0x2f202c6d00000000, 0xec0db8de00000000, 0xad1634b000000000,
    0xa18663a000000000, 0xe09defce00000000, 0x23b07b7d00000000,
    0x62abf71300000000, 0xe4ed22c100000000, 0xa5f6aeaf00000000,
    0x66db3a1c00000000, 0x27c0b67200000000, 0xd4ad7c8000000000,
    0x95b6f0ee00000000, 0x569b645d00000000, 0x1780e83300000000,
    0x91c63de100000000, 0xd0ddb18f00000000, 0x13f0253c00000000,
    0x52eba95200000000, 0x5e7bfe4200000000, 0x1f60722c00000000,
    0xdc4de69f00000000, 0x9d566af100000000, 0x1b10bf2300000000,
    0x5a0b334d00000000, 0x9926a7fe00000000, 0xd83d2b9000000000,
    0x810608de00000000, 0xc01d84b000000000, 0x0330100300000000,
    0x422b9c6d00000000, 0xc46d49bf00000000, 0x8576c5d100000000,
    0x465b516200000000, 0x0740dd0c00000000, 0x0bd08a1c00000000,
    0x4acb067200000000, 0x89e692c100000000, 0xc8fd1eaf00000000,
    0x4ebbcb7d00000000, 0x0fa0471300000000, 0xcc8dd3a000000000,
    0x8d965fce00000000},
   {0x0000000000000000, 0x1dfdb50100000000, 0x3afa6b0300000000,
    0x2707de0200000000, 0x74f4d70600000000, 0x6909620700000000,
    0x4e0ebc0500000000, 0x53f3090400000000, 0xe8e8af0d00000000,
    0xf5151a0c00000000, 0xd212c40e00000000, 0xcfef710f00000000,
    0x9c1c780b00000000, 0x81e1cd0a00000000, 0xa6e6130800000000,
    0xbb1ba60900000000, 0xd0d15f1b00000000, 0xcd2cea1a00000000,
    0xea2b341800000000, 0xf7d6811900000000, 0xa425881d00000000,
    0xb9d83d1c00000000, 0x9edfe31e00000000, 0x8322561f00000000,
    0x3839f01600000000, 0x25c4451700000000, 0x02c39b1500000000,
    0x1f3e2e1400000000, 0x4ccd271000000000, 0x5130921100000000,
    0x76374c1300000000, 0x6bcaf91200000000, 0xa0a3bf3600000000,
    0xbd5e0a3700000000, 0x9a59d43500000000, 0x87a4613400000000,
    0xd457683000000000, 0xc9aadd3100000000, 0xeead033300000000,
    0xf350b63200000000, 0x484b103b00000000, 0x55b6a53a00000000,
    0x72b17b3800000000, 0x6f4cce3900000000, 0x3cbfc73d00000000,
    0x2142723c00000000, 0x0645ac3e00000000, 0x1bb8193f00000000,
    0x7072e02d00000000, 0x6d8f552c00000000, 0x4a888b2e00000000,
    0x57753e2f00000000, 0x0486372b00000000, 0x197b822a00000000,
    0x3e7c5c2800000000, 0x2381e92900000000, 0x989a4f2000000000,
    0x8567fa2100000000, 0xa260242300000000, 0xbf9d912200000000,
    0xec6e982600000000, 0xf1932d2700000000, 0xd694f32500000000,
    0xcb69462400000000, 0x40477f6d00000000, 0x5dbaca6c00000000,
    0x7abd146e00000000, 0x6740a16f00000000, 0x34b3a86b00000000,
    0x294e1d6a00000000, 0x0e49c36800000000, 0x13b4766900000000,
    0xa8afd06000000000, 0xb552656100000000, 0x9255bb6300000000,
    0x8fa80e6200000000, 0xdc5b076600000000, 0xc1a6b26700000000,
    0xe6a16c6500000000, 0xfb5cd96400000000, 0x9096207600000000,
    0x8d6b957700000000, 0xaa6c4b7500000000, 0xb791fe7400000000,
    0xe462f77000000000, 0xf99f427100000000, 0xde989c7300000000,
    0xc365297200000000, 0x787e8f7b00000000, 0x65833a7a00000000,
    0x4284e47800000000, 0x5f79517900000000, 0x0c8a587d00000000,
    0x1177ed7c00000000, 0x3670337e00000000, 0x2b8d867f00000000,
    0xe0e4c05b00000000, 0xfd19755a00000000, 0xda1eab5800000000,
    0xc7e31e5900000000, 0x9410175d00000000, 0x89eda25c00000000,
    0xaeea7c5e00000000, 0xb317c95f00000000, 0x080c6f5600000000,
    0x15f1da5700000000, 0x32f6045500000000, 0x2f0bb15400000000,
    0x7cf8b85000000000, 0x61050d5100000000, 0x4602d35300000000,
    0x5bff665200000000, 0x30359f4000000000, 0x2dc82a4100000000,
    0x0acff44300000000, 0x1732414200000000, 0x44c1484600000000,
    0x593cfd4700000000, 0x7e3b234500000000, 0x63c6964400000000,
    0xd8dd304d00000000, 0xc520854c00000000, 0xe2275b4e00000000,
    0xffdaee4f00000000, 0xac29e74b00000000, 0xb1d4524a00000000,
    0x96d38c4800000000, 0x8b2e394900000000, 0x808efeda00000000,
    0x9d734bdb00000000, 0xba7495d900000000, 0xa78920d800000000,
    0xf47a29dc00000000, 0xe9879cdd00000000, 0xce8042df00000000,
    0xd37df7de00000000, 0x686651d700000000, 0x759be4d600000000,
    0x529c3ad400000000, 0x4f618fd500000000, 0x1c9286d100000000,
    0x016f33d000000000, 0x2668edd200000000, 0x3b9558d300000000,
    0x505fa1c100000000, 0x4da214c000000000, 0x6aa5cac200000000,
    0x77587fc300000000, 0x24ab76c700000000, 0x3956c3c600000000,
    0x1e511dc400000000, 0x03aca8c500000000, 0xb8b70ecc00000000,
    0xa54abbcd00000000, 0x824d65cf00000000, 0x9fb0d0ce00000000,
    0xcc43d9ca00000000, 0xd1be6ccb00000000, 0xf6b9b2c900000000,
    0xeb4407c800000000, 0x202d41ec00000000, 0x3dd0f4ed00000000,
    0x1ad72aef00000000, 0x072a9fee00000000, 0x54d996ea00000000,
    0x492423eb00000000, 0x6e23fde900000000, 0x73de48e800000000,
    0xc8c5eee100000000, 0xd5385be000000000, 0xf23f85e200000000,
    0xefc230e300000000, 0xbc3139e700000000, 0xa1cc8ce600000000,
    0x86cb52e400000000, 0x9b36e7e500000000, 0xf0fc1ef700000000,
    0xed01abf600000000, 0xca0675f400000000, 0xd7fbc0f500000000,
    0x8408c9f100000000, 0x99f57cf000000000, 0xbef2a2f200000000,
    0xa30f17f300000000, 0x1814b1fa00000000, 0x05e904fb00000000,
    0x22eedaf900000000, 0x3f136ff800000000, 0x6ce066fc00000000,
    0x711dd3fd00000000, 0x561a0dff00000000, 0x4be7b8fe00000000,
    0xc0c981b700000000, 0xdd3434b600000000, 0xfa33eab400000000,
    0xe7ce5fb500000000, 0xb43d56b100000000, 0xa9c0e3b000000000,
    0x8ec73db200000000, 0x933a88b300000000, 0x28212eba00000000,
    0x35dc9bbb00000000, 0x12db45b900000000, 0x0f26f0b800000000,
    0x5cd5f9bc00000000, 0x41284cbd00000000, 0x662f92bf00000000,
    0x7bd227be00000000, 0x1018deac00000000, 0x0de56bad00000000,
    0x2ae2b5af00000000, 0x371f00ae00000000, 0x64ec09aa00000000,
    0x7911bcab00000000, 0x5e1662a900000000, 0x43ebd7a800000000,
    0xf8f071a100000000, 0xe50dc4a000000000, 0xc20a1aa200000000,
    0xdff7afa300000000, 0x8c04a6a700000000, 0x91f913a600000000,
    0xb6fecda400000000, 0xab0378a500000000, 0x606a3e8100000000,
    0x7d978b8000000000, 0x5a90558200000000, 0x476de08300000000,
    0x149ee98700000000, 0x09635c8600000000, 0x2e64828400000000,
    0x3399378500000000, 0x8882918c00000000, 0x957f248d00000000,
    0xb278fa8f00000000, 0xaf854f8e00000000, 0xfc76468a00000000,
    0xe18bf38b00000000, 0xc68c2d8900000000, 0xdb71988800000000,
    0xb0bb619a00000000, 0xad46d49b00000000, 0x8a410a9900000000,
    0x97bcbf9800000000, 0xc44fb69c00000000, 0xd9b2039d00000000,
    0xfeb5dd9f00000000, 0xe348689e00000000, 0x5853ce9700000000,
    0x45ae7b9600000000, 0x62a9a59400000000, 0x7f54109500000000,
    0x2ca7199100000000, 0x315aac9000000000, 0x165d729200000000,
    0x0ba0c79300000000},
   {0x0000000000000000, 0x24d9076300000000, 0x48b20fc600000000,
    0x6c6b08a500000000, 0xd1626e5700000000, 0xf5bb693400000000,
    0x99d0619100000000, 0xbd0966f200000000, 0xa2c5dcae00000000,
    0x861cdbcd00000000, 0xea77d36800000000, 0xceaed40b00000000,
    0x73a7b2f900000000, 0x577eb59a00000000, 0x3b15bd3f00000000,
    0x1fccba5c00000000, 0x058dc88600000000, 0x2154cfe500000000,
    0x4d3fc74000000000, 0x69e6c02300000000, 0xd4efa6d100000000,
    0xf036a1b200000000, 0x9c5da91700000000, 0xb884ae7400000000,
    0xa748142800000000, 0x8391134b00000000, 0xeffa1bee00000000,
    0xcb231c8d00000000, 0x762a7a7f00000000, 0x52f37d1c00000000,
    0x3e9875b900000000, 0x1a4172da00000000, 0x4b1ce0d600000000,
    0x6fc5e7b500000000, 0x03aeef1000000000, 0x2777e87300000000,
    0x9a7e8e8100000000, 0xbea789e200000000, 0xd2cc814700000000,
    0xf615862400000000, 0xe9d93c7800000000, 0xcd003b1b00000000,
    0xa16b33be00000000, 0x85b234dd00000000, 0x38bb522f00000000,
    0x1c62554c00000000, 0x70095de900000000, 0x54d05a8a00000000,
    0x4e91285000000000, 0x6a482f3300000000, 0x0623279600000000,
    0x22fa20f500000000, 0x9ff3460700000000, 0xbb2a416400000000,
    0xd74149c100000000, 0xf3984ea200000000, 0xec54f4fe00000000,
    0xc88df39d00000000, 0xa4e6fb3800000000, 0x803ffc5b00000000,
    0x3d369aa900000000, 0x19ef9dca00000000, 0x7584956f00000000,
    0x515d920c00000000, 0xd73eb17600000000, 0xf3e7b61500000000,
    0x9f8cbeb000000000, 0xbb55b9d300000000, 0x065cdf2100000000,
    0x2285d84200000000, 0x4eeed0e700000000, 0x6a37d78400000000,
    0x75fb6dd800000000, 0x51226abb00000000, 0x3d49621e00000000,
    0x1990657d00000000, 0xa499038f00000000, 0x804004ec00000000,
    0xec2b0c4900000000, 0xc8f20b2a00000000, 0xd2b379f000000000,
    0xf66a7e9300000000, 0x9a01763600000000, 0xbed8715500000000,
    0x03d117a700000000, 0x270810c400000000, 0x4b63186100000000,
    0x6fba1f0200000000, 0x7076a55e00000000, 0x54afa23d00000000,
    0x38c4aa9800000000, 0x1c1dadfb00000000, 0xa114cb0900000000,
    0x85cdcc6a00000000, 0xe9a6c4cf00000000, 0xcd7fc3ac00000000,
    0x9c2251a000000000, 0xb8fb56c300000000, 0xd4905e6600000000,
    0xf049590500000000, 0x4d403ff700000000, 0x6999389400000000,
    0x05f2303100000000, 0x212b375200000000, 0x3ee78d0e00000000,
    0x1a3e8a6d00000000, 0x765582c800000000, 0x528c85ab00000000,
    0xef85e35900000000, 0xcb5ce43a00000000, 0xa737ec9f00000000,
    0x83eeebfc00000000, 0x99af992600000000, 0xbd769e4500000000,
    0xd11d96e000000000, 0xf5c4918300000000, 0x48cdf77100000000,
    0x6c14f01200000000, 0x007ff8b700000000, 0x24a6ffd400000000,
    0x3b6a458800000000, 0x1fb342eb00000000, 0x73d84a4e00000000,
    0x57014d2d00000000, 0xea082bdf00000000, 0xced12cbc00000000,
    0xa2ba241900000000, 0x8663237a00000000, 0xae7d62ed00000000,
    0x8aa4658e00000000, 0xe6cf6d2b00000000, 0xc2166a4800000000,
    0x7f1f0cba00000000, 0x5bc60bd900000000, 0x37ad037c00000000,
    0x1374041f00000000, 0x0cb8be4300000000, 0x2861b92000000000,
    0x440ab18500000000, 0x60d3b6e600000000, 0xdddad01400000000,
    0xf903d77700000000, 0x9568dfd200000000, 0xb1b1d8b100000000,
    0xabf0aa6b00000000, 0x8f29ad0800000000, 0xe342a5ad00000000,
    0xc79ba2ce00000000, 0x7a92c43c00000000, 0x5e4bc35f00000000,
    0x3220cbfa00000000, 0x16f9cc9900000000, 0x093576c500000000,
    0x2dec71a600000000, 0x4187790300000000, 0x655e7e6000000000,
    0xd857189200000000, 0xfc8e1ff100000000, 0x90e5175400000000,
    0xb43c103700000000, 0xe561823b00000000, 0xc1b8855800000000,
    0xadd38dfd00000000, 0x890a8a9e00000000, 0x3403ec6c00000000,
    0x10daeb0f00000000, 0x7cb1e3aa00000000, 0x5868e4c900000000,
    0x47a45e9500000000, 0x637d59f600000000, 0x0f16515300000000,
    0x2bcf563000000000, 0x96c630c200000000, 0xb21f37a100000000,
    0xde743f0400000000, 0xfaad386700000000, 0xe0ec4abd00000000,
    0xc4354dde00000000, 0xa85e457b00000000, 0x8c87421800000000,
    0x318e24ea00000000, 0x1557238900000000, 0x793c2b2c00000000,
    0x5de52c4f00000000, 0x4229961300000000, 0x66f0917000000000,
    0x0a9b99d500000000, 0x2e429eb600000000, 0x934bf84400000000,
    0xb792ff2700000000, 0xdbf9f78200000000, 0xff20f0e100000000,
    0x7943d39b00000000, 0x5d9ad4f800000000, 0x31f1dc5d00000000,
    0x1528db3e00000000, 0xa821bdcc00000000, 0x8cf8baaf00000000,
    0xe093b20a00000000, 0xc44ab56900000000, 0xdb860f3500000000,
    0xff5f085600000000, 0x933400f300000000, 0xb7ed079000000000,
    0x0ae4616200000000, 0x2e3d660100000000, 0x42566ea400000000,
    0x668f69c700000000, 0x7cce1b1d00000000, 0x58171c7e00000000,
    0x347c14db00000000, 0x10a513b800000000, 0xadac754a00000000,
    0x8975722900000000, 0xe51e7a8c00000000, 0xc1c77def00000000,
    0xde0bc7b300000000, 0xfad2c0d000000000, 0x96b9c87500000000,
    0xb260cf1600000000, 0x0f69a9e400000000, 0x2bb0ae8700000000,
    0x47dba62200000000, 0x6302a14100000000, 0x325f334d00000000,
    0x1686342e00000000, 0x7aed3c8b00000000, 0x5e343be800000000,
    0xe33d5d1a00000000, 0xc7e45a7900000000, 0xab8f52dc00000000,
    0x8f5655bf00000000, 0x909aefe300000000, 0xb443e88000000000,
    0xd828e02500000000, 0xfcf1e74600000000, 0x41f881b400000000,
    0x652186d700000000, 0x094a8e7200000000, 0x2d93891100000000,
    0x37d2fbcb00000000, 0x130bfca800000000, 0x7f60f40d00000000,
    0x5bb9f36e00000000, 0xe6b0959c00000000, 0xc26992ff00000000,
    0xae029a5a00000000, 0x8adb9d3900000000, 0x9517276500000000,
    0xb1ce200600000000, 0xdda528a300000000, 0xf97c2fc000000000,
    0x4475493200000000, 0x60ac4e5100000000, 0x0cc746f400000000,
    0x281e419700000000},
   {0x0000000000000000, 0x08e3603c00000000, 0x10c6c17800000000,
    0x1825a14400000000, 0x208c83f100000000, 0x286fe3cd00000000,
    0x304a428900000000, 0x38a922b500000000, 0x011e763800000000,
    0x09fd160400000000, 0x11d8b74000000000, 0x193bd77c00000000,
    0x2192f5c900000000, 0x297195f500000000, 0x315434b100000000,
    0x39b7548d00000000, 0x023cec7000000000, 0x0adf8c4c00000000,
    0x12fa2d0800000000, 0x1a194d3400000000, 0x22b06f8100000000,
    0x2a530fbd00000000, 0x3276aef900000000, 0x3a95cec500000000,
    0x03229a4800000000, 0x0bc1fa7400000000, 0x13e45b3000000000,
    0x1b073b0c00000000, 0x23ae19b900000000, 0x2b4d798500000000,
    0x3368d8c100000000, 0x3b8bb8fd00000000, 0x0478d8e100000000,
    0x0c9bb8dd00000000, 0x14be199900000000, 0x1c5d79a500000000,
    0x24f45b1000000000, 0x2c173b2c00000000, 0x34329a6800000000,
    0x3cd1fa5400000000, 0x0566aed900000000, 0x0d85cee500000000,
    0x15a06fa100000000, 0x1d430f9d00000000, 0x25ea2d2800000000,
    0x2d094d1400000000, 0x352cec5000000000, 0x3dcf8c6c00000000,
    0x0644349100000000, 0x0ea754ad00000000, 0x1682f5e900000000,
    0x1e6195d500000000, 0x26c8b76000000000, 0x2e2bd75c00000000,
    0x360e761800000000, 0x3eed162400000000, 0x075a42a900000000,
    0x0fb9229500000000, 0x179c83d100000000, 0x1f7fe3ed00000000,
    0x27d6c15800000000, 0x2f35a16400000000, 0x3710002000000000,
    0x3ff3601c00000000, 0x49f6c11800000000, 0x4115a12400000000,
    0x5930006000000000, 0x51d3605c00000000, 0x697a42e900000000,
    0x619922d500000000, 0x79bc839100000000, 0x715fe3ad00000000,
    0x48e8b72000000000, 0x400bd71c00000000, 0x582e765800000000,
    0x50cd166400000000, 0x686434d100000000, 0x608754ed00000000,
    0x78a2f5a900000000, 0x7041959500000000, 0x4bca2d6800000000,
    0x43294d5400000000, 0x5b0cec1000000000, 0x53ef8c2c00000000,
    0x6b46ae9900000000, 0x63a5cea500000000, 0x7b806fe100000000,
    0x73630fdd00000000, 0x4ad45b5000000000, 0x42373b6c00000000,
    0x5a129a2800000000, 0x52f1fa1400000000, 0x6a58d8a100000000,
    0x62bbb89d00000000, 0x7a9e19d900000000, 0x727d79e500000000,
    0x4d8e19f900000000, 0x456d79c500000000, 0x5d48d88100000000,
    0x55abb8bd00000000, 0x6d029a0800000000, 0x65e1fa3400000000,
    0x7dc45b7000000000, 0x75273b4c00000000, 0x4c906fc100000000,
    0x44730ffd00000000, 0x5c56aeb900000000, 0x54b5ce8500000000,
    0x6c1cec3000000000, 0x64ff8c0c00000000, 0x7cda2d4800000000,
    0x74394d7400000000, 0x4fb2f58900000000, 0x475195b500000000,
    0x5f7434f100000000, 0x579754cd00000000, 0x6f3e767800000000,
    0x67dd164400000000, 0x7ff8b70000000000, 0x771bd73c00000000,
    0x4eac83b100000000, 0x464fe38d00000000, 0x5e6a42c900000000,
    0x568922f500000000, 0x6e20004000000000, 0x66c3607c00000000,
    0x7ee6c13800000000, 0x7605a10400000000, 0x92ec833100000000,
    0x9a0fe30d00000000, 0x822a424900000000, 0x8ac9227500000000,
    0xb26000c000000000, 0xba8360fc00000000, 0xa2a6c1b800000000,
    0xaa45a18400000000, 0x93f2f50900000000, 0x9b11953500000000,
    0x8334347100000000, 0x8bd7544d00000000, 0xb37e76f800000000,
    0xbb9d16c400000000, 0xa3b8b78000000000, 0xab5bd7bc00000000,
    0x90d06f4100000000, 0x98330f7d00000000, 0x8016ae3900000000,
    0x88f5ce0500000000, 0xb05cecb000000000, 0xb8bf8c8c00000000,
    0xa09a2dc800000000, 0xa8794df400000000, 0x91ce197900000000,
    0x992d794500000000, 0x8108d80100000000, 0x89ebb83d00000000,
    0xb1429a8800000000, 0xb9a1fab400000000, 0xa1845bf000000000,
    0xa9673bcc00000000, 0x96945bd000000000, 0x9e773bec00000000,
    0x86529aa800000000, 0x8eb1fa9400000000, 0xb618d82100000000,
    0xbefbb81d00000000, 0xa6de195900000000, 0xae3d796500000000,
    0x978a2de800000000, 0x9f694dd400000000, 0x874cec9000000000,
    0x8faf8cac00000000, 0xb706ae1900000000, 0xbfe5ce2500000000,
    0xa7c06f6100000000, 0xaf230f5d00000000, 0x94a8b7a000000000,
    0x9c4bd79c00000000, 0x846e76d800000000, 0x8c8d16e400000000,
    0xb424345100000000, 0xbcc7546d00000000, 0xa4e2f52900000000,
    0xac01951500000000, 0x95b6c19800000000, 0x9d55a1a400000000,
    0x857000e000000000, 0x8d9360dc00000000, 0xb53a426900000000,
    0xbdd9225500000000, 0xa5fc831100000000, 0xad1fe32d00000000,
    0xdb1a422900000000, 0xd3f9221500000000, 0xcbdc835100000000,
    0xc33fe36d00000000, 0xfb96c1d800000000, 0xf375a1e400000000,
    0xeb5000a000000000, 0xe3b3609c00000000, 0xda04341100000000,
    0xd2e7542d00000000, 0xcac2f56900000000, 0xc221955500000000,
    0xfa88b7e000000000, 0xf26bd7dc00000000, 0xea4e769800000000,
    0xe2ad16a400000000, 0xd926ae5900000000, 0xd1c5ce6500000000,
    0xc9e06f2100000000, 0xc1030f1d00000000, 0xf9aa2da800000000,
    0xf1494d9400000000, 0xe96cecd000000000, 0xe18f8cec00000000,
    0xd838d86100000000, 0xd0dbb85d00000000, 0xc8fe191900000000,
    0xc01d792500000000, 0xf8b45b9000000000, 0xf0573bac00000000,
    0xe8729ae800000000, 0xe091fad400000000, 0xdf629ac800000000,
    0xd781faf400000000, 0xcfa45bb000000000, 0xc7473b8c00000000,
    0xffee193900000000, 0xf70d790500000000, 0xef28d84100000000,
    0xe7cbb87d00000000, 0xde7cecf000000000, 0xd69f8ccc00000000,
    0xceba2d8800000000, 0xc6594db400000000, 0xfef06f0100000000,
    0xf6130f3d00000000, 0xee36ae7900000000, 0xe6d5ce4500000000,
    0xdd5e76b800000000, 0xd5bd168400000000, 0xcd98b7c000000000,
    0xc57bd7fc00000000, 0xfdd2f54900000000, 0xf531957500000000,
    0xed14343100000000, 0xe5f7540d00000000, 0xdc40008000000000,
    0xd4a360bc00000000, 0xcc86c1f800000000, 0xc465a1c400000000,
    0xfccc837100000000, 0xf42fe34d00000000, 0xec0a420900000000,
    0xe4e9223500000000},
   {0x0000000000000000, 0xd1e8e70e00000000, 0xa2d1cf1d00000000,
    0x7339281300000000, 0x44a39f3b00000000, 0x954b783500000000,
    0xe672502600000000, 0x379ab72800000000, 0x88463f7700000000,
    0x59aed87900000000, 0x2a97f06a00000000, 0xfb7f176400000000,
    0xcce5a04c00000000, 0x1d0d474200000000, 0x6e346f5100000000,
    0xbfdc885f00000000, 0x108d7eee00000000, 0xc16599e000000000,
    0xb25cb1f300000000, 0x63b456fd00000000, 0x542ee1d500000000,
    0x85c606db00000000, 0xf6ff2ec800000000, 0x2717c9c600000000,
    0x98cb419900000000, 0x4923a69700000000, 0x3a1a8e8400000000,
    0xebf2698a00000000, 0xdc68dea200000000, 0x0d8039ac00000000,
    0x7eb911bf00000000, 0xaf51f6b100000000, 0x611c8c0700000000,
    0xb0f46b0900000000, 0xc3cd431a00000000, 0x1225a41400000000,
    0x25bf133c00000000, 0xf457f43200000000, 0x876edc2100000000,
    0x56863b2f00000000, 0xe95ab37000000000, 0x38b2547e00000000,
    0x4b8b7c6d00000000, 0x9a639b6300000000, 0xadf92c4b00000000,
    0x7c11cb4500000000, 0x0f28e35600000000, 0xdec0045800000000,
    0x7191f2e900000000, 0xa07915e700000000, 0xd3403df400000000,
    0x02a8dafa00000000, 0x35326dd200000000, 0xe4da8adc00000000,
    0x97e3a2cf00000000, 0x460b45c100000000, 0xf9d7cd9e00000000,
    0x283f2a9000000000, 0x5b06028300000000, 0x8aeee58d00000000,
    0xbd7452a500000000, 0x6c9cb5ab00000000, 0x1fa59db800000000,
    0xce4d7ab600000000, 0xc238180f00000000, 0x13d0ff0100000000,
    0x60e9d71200000000, 0xb101301c00000000, 0x869b873400000000,
    0x5773603a00000000, 0x244a482900000000, 0xf5a2af2700000000,
    0x4a7e277800000000, 0x9b96c07600000000, 0xe8afe86500000000,
    0x39470f6b00000000, 0x0eddb84300000000, 0xdf355f4d00000000,
    0xac0c775e00000000, 0x7de4905000000000, 0xd2b566e100000000,
    0x035d81ef00000000, 0x7064a9fc00000000, 0xa18c4ef200000000,
    0x9616f9da00000000, 0x47fe1ed400000000, 0x34c736c700000000,
    0xe52fd1c900000000, 0x5af3599600000000, 0x8b1bbe9800000000,
    0xf822968b00000000, 0x29ca718500000000, 0x1e50c6ad00000000,
    0xcfb821a300000000, 0xbc8109b000000000, 0x6d69eebe00000000,
    0xa324940800000000, 0x72cc730600000000, 0x01f55b1500000000,
    0xd01dbc1b00000000, 0xe7870b3300000000, 0x366fec3d00000000,
    0x4556c42e00000000, 0x94be232000000000, 0x2b62ab7f00000000,
    0xfa8a4c7100000000, 0x89b3646200000000, 0x585b836c00000000,
    0x6fc1344400000000, 0xbe29d34a00000000, 0xcd10fb5900000000,
    0x1cf81c5700000000, 0xb3a9eae600000000, 0x62410de800000000,
    0x117825fb00000000, 0xc090c2f500000000, 0xf70a75dd00000000,
    0x26e292d300000000, 0x55dbbac000000000, 0x84335dce00000000,
    0x3befd59100000000, 0xea07329f00000000, 0x993e1a8c00000000,
    0x48d6fd8200000000, 0x7f4c4aaa00000000, 0xaea4ada400000000,
    0xdd9d85b700000000, 0x0c7562b900000000, 0x8471301e00000000,
    0x5599d71000000000, 0x26a0ff0300000000, 0xf748180d00000000,
    0xc0d2af2500000000, 0x113a482b00000000, 0x6203603800000000,
    0xb3eb873600000000, 0x0c370f6900000000, 0xdddfe86700000000,
    0xaee6c07400000000, 0x7f0e277a00000000, 0x4894905200000000,
    0x997c775c00000000, 0xea455f4f00000000, 0x3badb84100000000,
    0x94fc4ef000000000, 0x4514a9fe00000000, 0x362d81ed00000000,
    0xe7c566e300000000, 0xd05fd1cb00000000, 0x01b736c500000000,
    0x728e1ed600000000, 0xa366f9d800000000, 0x1cba718700000000,
    0xcd52968900000000, 0xbe6bbe9a00000000, 0x6f83599400000000,
    0x5819eebc00000000, 0x89f109b200000000, 0xfac821a100000000,
    0x2b20c6af00000000, 0xe56dbc1900000000, 0x34855b1700000000,
    0x47bc730400000000, 0x9654940a00000000, 0xa1ce232200000000,
    0x7026c42c00000000, 0x031fec3f00000000, 0xd2f70b3100000000,
    0x6d2b836e00000000, 0xbcc3646000000000, 0xcffa4c7300000000,
    0x1e12ab7d00000000, 0x29881c5500000000, 0xf860fb5b00000000,
    0x8b59d34800000000, 0x5ab1344600000000, 0xf5e0c2f700000000,
    0x240825f900000000, 0x57310dea00000000, 0x86d9eae400000000,
    0xb1435dcc00000000, 0x60abbac200000000, 0x139292d100000000,
    0xc27a75df00000000, 0x7da6fd8000000000, 0xac4e1a8e00000000,
    0xdf77329d00000000, 0x0e9fd59300000000, 0x390562bb00000000,
    0xe8ed85b500000000, 0x9bd4ada600000000, 0x4a3c4aa800000000,
    0x4649281100000000, 0x97a1cf1f00000000, 0xe498e70c00000000,
    0x3570000200000000, 0x02eab72a00000000, 0xd302502400000000,
    0xa03b783700000000, 0x71d39f3900000000, 0xce0f176600000000,
    0x1fe7f06800000000, 0x6cded87b00000000, 0xbd363f7500000000,
    0x8aac885d00000000, 0x5b446f5300000000, 0x287d474000000000,
    0xf995a04e00000000, 0x56c456ff00000000, 0x872cb1f100000000,
    0xf41599e200000000, 0x25fd7eec00000000, 0x1267c9c400000000,
    0xc38f2eca00000000, 0xb0b606d900000000, 0x615ee1d700000000,
    0xde82698800000000, 0x0f6a8e8600000000, 0x7c53a69500000000,
    0xadbb419b00000000, 0x9a21f6b300000000, 0x4bc911bd00000000,
    0x38f039ae00000000, 0xe918dea000000000, 0x2755a41600000000,
    0xf6bd431800000000, 0x85846b0b00000000, 0x546c8c0500000000,
    0x63f63b2d00000000, 0xb21edc2300000000, 0xc127f43000000000,
    0x10cf133e00000000, 0xaf139b6100000000, 0x7efb7c6f00000000,
    0x0dc2547c00000000, 0xdc2ab37200000000, 0xebb0045a00000000,
    0x3a58e35400000000, 0x4961cb4700000000, 0x98892c4900000000,
    0x37d8daf800000000, 0xe6303df600000000, 0x950915e500000000,
    0x44e1f2eb00000000, 0x737b45c300000000, 0xa293a2cd00000000,
    0xd1aa8ade00000000, 0x00426dd000000000, 0xbf9ee58f00000000,
    0x6e76028100000000, 0x1d4f2a9200000000, 0xcca7cd9c00000000,
    0xfb3d7ab400000000, 0x2ad59dba00000000, 0x59ecb5a900000000,
    0x880452a700000000},
   {0x0000000000000000, 0xaa05daf100000000, 0x150dc53800000000,
    0xbf081fc900000000, 0x2a1a8a7100000000, 0x801f508000000000,
    0x3f174f4900000000, 0x951295b800000000, 0x543414e300000000,
    0xfe31ce1200000000, 0x4139d1db00000000, 0xeb3c0b2a00000000,
    0x7e2e9e9200000000, 0xd42b446300000000, 0x6b235baa00000000,
    0xc126815b00000000, 0xe96e591d00000000, 0x436b83ec00000000,
    0xfc639c2500000000, 0x566646d400000000, 0xc374d36c00000000,
    0x6971099d00000000, 0xd679165400000000, 0x7c7ccca500000000,
    0xbd5a4dfe00000000, 0x175f970f00000000, 0xa85788c600000000,
    0x0252523700000000, 0x9740c78f00000000, 0x3d451d7e00000000,
    0x824d02b700000000, 0x2848d84600000000, 0xd2ddb23a00000000,
    0x78d868cb00000000, 0xc7d0770200000000, 0x6dd5adf300000000,
    0xf8c7384b00000000, 0x52c2e2ba00000000, 0xedcafd7300000000,
    0x47cf278200000000, 0x86e9a6d900000000, 0x2cec7c2800000000,
    0x93e463e100000000, 0x39e1b91000000000, 0xacf32ca800000000,
    0x06f6f65900000000, 0xb9fee99000000000, 0x13fb336100000000,
    0x3bb3eb2700000000, 0x91b631d600000000, 0x2ebe2e1f00000000,
    0x84bbf4ee00000000, 0x11a9615600000000, 0xbbacbba700000000,
    0x04a4a46e00000000, 0xaea17e9f00000000, 0x6f87ffc400000000,
    0xc582253500000000, 0x7a8a3afc00000000, 0xd08fe00d00000000,
    0x459d75b500000000, 0xef98af4400000000, 0x5090b08d00000000,
    0xfa956a7c00000000, 0xa4bb657500000000, 0x0ebebf8400000000,
    0xb1b6a04d00000000, 0x1bb37abc00000000, 0x8ea1ef0400000000,
    0x24a435f500000000, 0x9bac2a3c00000000, 0x31a9f0cd00000000,
    0xf08f719600000000, 0x5a8aab6700000000, 0xe582b4ae00000000,
    0x4f876e5f00000000, 0xda95fbe700000000, 0x7090211600000000,
    0xcf983edf00000000, 0x659de42e00000000, 0x4dd53c6800000000,
    0xe7d0e69900000000, 0x58d8f95000000000, 0xf2dd23a100000000,
    0x67cfb61900000000, 0xcdca6ce800000000, 0x72c2732100000000,
    0xd8c7a9d000000000, 0x19e1288b00000000, 0xb3e4f27a00000000,
    0x0cecedb300000000, 0xa6e9374200000000, 0x33fba2fa00000000,
    0x99fe780b00000000, 0x26f667c200000000, 0x8cf3bd3300000000,
    0x7666d74f00000000, 0xdc630dbe00000000, 0x636b127700000000,
    0xc96ec88600000000, 0x5c7c5d3e00000000, 0xf67987cf00000000,
    0x4971980600000000, 0xe37442f700000000, 0x2252c3ac00000000,
    0x8857195d00000000, 0x375f069400000000, 0x9d5adc6500000000,
    0x084849dd00000000, 0xa24d932c00000000, 0x1d458ce500000000,
    0xb740561400000000, 0x9f088e5200000000, 0x350d54a300000000,
    0x8a054b6a00000000, 0x2000919b00000000, 0xb512042300000000,
    0x1f17ded200000000, 0xa01fc11b00000000, 0x0a1a1bea00000000,
    0xcb3c9ab100000000, 0x6139404000000000, 0xde315f8900000000,
    0x7434857800000000, 0xe12610c000000000, 0x4b23ca3100000000,
    0xf42bd5f800000000, 0x5e2e0f0900000000, 0x4877cbea00000000,
    0xe272111b00000000, 0x5d7a0ed200000000, 0xf77fd42300000000,
    0x626d419b00000000, 0xc8689b6a00000000, 0x776084a300000000,
    0xdd655e5200000000, 0x1c43df0900000000, 0xb64605f800000000,
    0x094e1a3100000000, 0xa34bc0c000000000, 0x3659557800000000,
    0x9c5c8f8900000000, 0x2354904000000000, 0x89514ab100000000,
    0xa11992f700000000, 0x0b1c480600000000, 0xb41457cf00000000,
    0x1e118d3e00000000, 0x8b03188600000000, 0x2106c27700000000,
    0x9e0eddbe00000000, 0x340b074f00000000, 0xf52d861400000000,
    0x5f285ce500000000, 0xe020432c00000000, 0x4a2599dd00000000,
    0xdf370c6500000000, 0x7532d69400000000, 0xca3ac95d00000000,
    0x603f13ac00000000, 0x9aaa79d000000000, 0x30afa32100000000,
    0x8fa7bce800000000, 0x25a2661900000000, 0xb0b0f3a100000000,
    0x1ab5295000000000, 0xa5bd369900000000, 0x0fb8ec6800000000,
    0xce9e6d3300000000, 0x649bb7c200000000, 0xdb93a80b00000000,
    0x719672fa00000000, 0xe484e74200000000, 0x4e813db300000000,
    0xf189227a00000000, 0x5b8cf88b00000000, 0x73c420cd00000000,
    0xd9c1fa3c00000000, 0x66c9e5f500000000, 0xcccc3f0400000000,
    0x59deaabc00000000, 0xf3db704d00000000, 0x4cd36f8400000000,
    0xe6d6b57500000000, 0x27f0342e00000000, 0x8df5eedf00000000,
    0x32fdf11600000000, 0x98f82be700000000, 0x0deabe5f00000000,
    0xa7ef64ae00000000, 0x18e77b6700000000, 0xb2e2a19600000000,
    0xecccae9f00000000, 0x46c9746e00000000, 0xf9c16ba700000000,
    0x53c4b15600000000, 0xc6d624ee00000000, 0x6cd3fe1f00000000,
    0xd3dbe1d600000000, 0x79de3b2700000000, 0xb8f8ba7c00000000,
    0x12fd608d00000000, 0xadf57f4400000000, 0x07f0a5b500000000,
    0x92e2300d00000000, 0x38e7eafc00000000, 0x87eff53500000000,
    0x2dea2fc400000000, 0x05a2f78200000000, 0xafa72d7300000000,
    0x10af32ba00000000, 0xbaaae84b00000000, 0x2fb87df300000000,
    0x85bda70200000000, 0x3ab5b8cb00000000, 0x90b0623a00000000,
    0x5196e36100000000, 0xfb93399000000000, 0x449b265900000000,
    0xee9efca800000000, 0x7b8c691000000000, 0xd189b3e100000000,
    0x6e81ac2800000000, 0xc48476d900000000, 0x3e111ca500000000,
    0x9414c65400000000, 0x2b1cd99d00000000, 0x8119036c00000000,
    0x140b96d400000000, 0xbe0e4c2500000000, 0x010653ec00000000,
    0xab03891d00000000, 0x6a25084600000000, 0xc020d2b700000000,
    0x7f28cd7e00000000, 0xd52d178f00000000, 0x403f823700000000,
    0xea3a58c600000000, 0x5532470f00000000, 0xff379dfe00000000,
    0xd77f45b800000000, 0x7d7a9f4900000000, 0xc272808000000000,
    0x68775a7100000000, 0xfd65cfc900000000, 0x5760153800000000,
    0xe8680af100000000, 0x426dd00000000000, 0x834b515b00000000,
    0x294e8baa00000000, 0x9646946300000000, 0x3c434e9200000000,
    0xa951db2a00000000, 0x035401db00000000, 0xbc5c1e1200000000,
    0x1659c4e300000000}};

#else /* W == 4 */

local const z_crc_t FAR crc_braid_table[][256] = {
   {0x00000000, 0xae689191, 0x87a02563, 0x29c8b4f2, 0xd4314c87,
    0x7a59dd16, 0x539169e4, 0xfdf9f875, 0x73139f4f, 0xdd7b0ede,
    0xf4b3ba2c, 0x5adb2bbd, 0xa722d3c8, 0x094a4259, 0x2082f6ab,
    0x8eea673a, 0xe6273e9e, 0x484faf0f, 0x61871bfd, 0xcfef8a6c,
    0x32167219, 0x9c7ee388, 0xb5b6577a, 0x1bdec6eb, 0x9534a1d1,
    0x3b5c3040, 0x129484b2, 0xbcfc1523, 0x4105ed56, 0xef6d7cc7,
    0xc6a5c835, 0x68cd59a4, 0x173f7b7d, 0xb957eaec, 0x909f5e1e,
    0x3ef7cf8f, 0xc30e37fa, 0x6d66a66b, 0x44ae1299, 0xeac68308,
    0x642ce432, 0xca4475a3, 0xe38cc151, 0x4de450c0, 0xb01da8b5,
    0x1e753924, 0x37bd8dd6, 0x99d51c47, 0xf11845e3, 0x5f70d472,
    0x76b86080, 0xd8d0f111, 0x25290964, 0x8b4198f5, 0xa2892c07,
    0x0ce1bd96, 0x820bdaac, 0x2c634b3d, 0x05abffcf, 0xabc36e5e,
    0x563a962b, 0xf85207ba, 0xd19ab348, 0x7ff222d9, 0x2e7ef6fa,
    0x8016676b, 0xa9ded399, 0x07b64208, 0xfa4fba7d, 0x54272bec,
    0x7def9f1e, 0xd3870e8f, 0x5d6d69b5, 0xf305f824, 0xdacd4cd6,
    0x74a5dd47, 0x895c2532, 0x2734b4a3, 0x0efc0051, 0xa09491c0,
    0xc859c864, 0x663159f5, 0x4ff9ed07, 0xe1917c96, 0x1c6884e3,
    0xb2001572, 0x9bc8a180, 0x35a03011, 0xbb4a572b, 0x1522c6ba,
    0x3cea7248, 0x9282e3d9, 0x6f7b1bac, 0xc1138a3d, 0xe8db3ecf,
    0x46b3af5e, 0x39418d87, 0x97291c16, 0xbee1a8e4, 0x10893975,
    0xed70c100, 0x43185091, 0x6ad0e463, 0xc4b875f2, 0x4a5212c8,
    0xe43a8359, 0xcdf237ab, 0x639aa63a, 0x9e635e4f, 0x300bcfde,
    0x19c37b2c, 0xb7abeabd, 0xdf66b319, 0x710e2288, 0x58c6967a,
    0xf6ae07eb, 0x0b57ff9e, 0xa53f6e0f, 0x8cf7dafd, 0x229f4b6c,
    0xac752c56, 0x021dbdc7, 0x2bd50935, 0x85bd98a4, 0x784460d1,
    0xd62cf140, 0xffe445b2, 0x518cd423, 0x5cfdedf4, 0xf2957c65,
    0xdb5dc897, 0x75355906, 0x88cca173, 0x26a430e2, 0x0f6c8410,
    0xa1041581, 0x2fee72bb, 0x8186e32a, 0xa84e57d8, 0x0626c649,
    0xfbdf3e3c, 0x55b7afad, 0x7c7f1b5f, 0xd2178ace, 0xbadad36a,
    0x14b242fb, 0x3d7af609, 0x93126798, 0x6eeb9fed, 0xc0830e7c,
    0xe94bba8e, 0x47232b1f, 0xc9c94c25, 0x67a1ddb4, 0x4e696946,
    0xe001f8d7, 0x1df800a2, 0xb3909133, 0x9a5825c1, 0x3430b450,
    0x4bc29689, 0xe5aa0718, 0xcc62b3ea, 0x620a227b, 0x9ff3da0e,
    0x319b4b9f, 0x1853ff6d, 0xb63b6efc, 0x38d109c6, 0x96b99857,
    0xbf712ca5, 0x1119bd34, 0xece04541, 0x4288d4d0, 0x6b406022,
    0xc528f1b3, 0xade5a817, 0x038d3986, 0x2a458d74, 0x842d1ce5,
    0x79d4e490, 0xd7bc7501, 0xfe74c1f3, 0x501c5062, 0xdef63758,
    0x709ea6c9, 0x5956123b, 0xf73e83aa, 0x0ac77bdf, 0xa4afea4e,
    0x8d675ebc, 0x230fcf2d, 0x72831b0e, 0xdceb8a9f, 0xf5233e6d,
    0x5b4baffc, 0xa6b25789, 0x08dac618, 0x211272ea, 0x8f7ae37b,
    0x01908441, 0xaff815d0, 0x8630a122, 0x285830b3, 0xd5a1c8c6,
    0x7bc95957, 0x5201eda5, 0xfc697c34, 0x94a42590, 0x3accb401,
    0x130400f3, 0xbd6c9162, 0x40956917, 0xeefdf886, 0xc7354c74,
    0x695ddde5, 0xe7b7badf, 0x49df2b4e, 0x60179fbc, 0xce7f0e2d,
    0x3386f658, 0x9dee67c9, 0xb426d33b, 0x1a4e42aa, 0x65bc6073,
    0xcbd4f1e2, 0xe21c4510, 0x4c74d481, 0xb18d2cf4, 0x1fe5bd65,
    0x362d0997, 0x98459806, 0x16afff3c, 0xb8c76ead, 0x910fda5f,
    0x3f674bce, 0xc29eb3bb, 0x6cf6222a, 0x453e96d8, 0xeb560749,
    0x839b5eed, 0x2df3cf7c, 0x043b7b8e, 0xaa53ea1f, 0x57aa126a,
    0xf9c283fb, 0xd00a3709, 0x7e62a698, 0xf088c1a2, 0x5ee05033,
    0x7728e4c1, 0xd9407550, 0x24b98d25, 0x8ad11cb4, 0xa319a846,
    0x0d7139d7},
   {0x00000000, 0xb9fbdbe8, 0xa886b191, 0x117d6a79, 0x8a7c6563,
    0x3387be8b, 0x22fad4f2, 0x9b010f1a, 0xcf89cc87, 0x7672176f,
    0x670f7d16, 0xdef4a6fe, 0x45f5a9e4, 0xfc0e720c, 0xed731875,
    0x5488c39d, 0x44629f4f, 0xfd9944a7, 0xece42ede, 0x551ff536,
    0xce1efa2c, 0x77e521c4, 0x66984bbd, 0xdf639055, 0x8beb53c8,
    0x32108820, 0x236de259, 0x9a9639b1, 0x019736ab, 0xb86ced43,
    0xa911873a, 0x10ea5cd2, 0x88c53e9e, 0x313ee576, 0x20438f0f,
    0x99b854e7, 0x02b95bfd, 0xbb428015, 0xaa3fea6c, 0x13c43184,
    0x474cf219, 0xfeb729f1, 0xefca4388, 0x56319860, 0xcd30977a,
    0x74cb4c92, 0x65b626eb, 0xdc4dfd03, 0xcca7a1d1, 0x755c7a39,
    0x64211040, 0xdddacba8, 0x46dbc4b2, 0xff201f5a, 0xee5d7523,
    0x57a6aecb, 0x032e6d56, 0xbad5b6be, 0xaba8dcc7, 0x1253072f,
    0x89520835, 0x30a9d3dd, 0x21d4b9a4, 0x982f624c, 0xcafb7b7d,
    0x7300a095, 0x627dcaec, 0xdb861104, 0x40871e1e, 0xf97cc5f6,
    0xe801af8f, 0x51fa7467, 0x0572b7fa, 0xbc896c12, 0xadf4066b,
    0x140fdd83, 0x8f0ed299, 0x36f50971, 0x27886308, 0x9e73b8e0,
    0x8e99e432, 0x37623fda, 0x261f55a3, 0x9fe48e4b, 0x04e58151,
    0xbd1e5ab9, 0xac6330c0, 0x1598eb28, 0x411028b5, 0xf8ebf35d,
    0xe9969924, 0x506d42cc, 0xcb6c4dd6, 0x7297963e, 0x63eafc47,
    0xda1127af, 0x423e45e3, 0xfbc59e0b, 0xeab8f472, 0x53432f9a,
    0xc8422080, 0x71b9fb68, 0x60c49111, 0xd93f4af9, 0x8db78964,
    0x344c528c, 0x253138f5, 0x9ccae31d, 0x07cbec07, 0xbe3037ef,
    0xaf4d5d96, 0x16b6867e, 0x065cdaac, 0xbfa70144, 0xaeda6b3d,
    0x1721b0d5, 0x8c20bfcf, 0x35db6427, 0x24a60e5e, 0x9d5dd5b6,
    0xc9d5162b, 0x702ecdc3, 0x6153a7ba, 0xd8a87c52, 0x43a97348,
    0xfa52a8a0, 0xeb2fc2d9, 0x52d41931, 0x4e87f0bb, 0xf77c2b53,
    0xe601412a, 0x5ffa9ac2, 0xc4fb95d8, 0x7d004e30, 0x6c7d2449,
    0xd586ffa1, 0x810e3c3c, 0x38f5e7d4, 0x29888dad, 0x90735645,
    0x0b72595f, 0xb28982b7, 0xa3f4e8ce, 0x1a0f3326, 0x0ae56ff4,
    0xb31eb41c, 0xa263de65, 0x1b98058d, 0x80990a97, 0x3962d17f,
    0x281fbb06, 0x91e460ee, 0xc56ca373, 0x7c97789b, 0x6dea12e2,
    0xd411c90a, 0x4f10c610, 0xf6eb1df8, 0xe7967781, 0x5e6dac69,
    0xc642ce25, 0x7fb915cd, 0x6ec47fb4, 0xd73fa45c, 0x4c3eab46,
    0xf5c570ae, 0xe4b81ad7, 0x5d43c13f, 0x09cb02a2, 0xb030d94a,
    0xa14db333, 0x18b668db, 0x83b767c1, 0x3a4cbc29, 0x2b31d650,
    0x92ca0db8, 0x8220516a, 0x3bdb8a82, 0x2aa6e0fb, 0x935d3b13,
    0x085c3409, 0xb1a7efe1, 0xa0da8598, 0x19215e70, 0x4da99ded,
    0xf4524605, 0xe52f2c7c, 0x5cd4f794, 0xc7d5f88e, 0x7e2e2366,
    0x6f53491f, 0xd6a892f7, 0x847c8bc6, 0x3d87502e, 0x2cfa3a57,
    0x9501e1bf, 0x0e00eea5, 0xb7fb354d, 0xa6865f34, 0x1f7d84dc,
    0x4bf54741, 0xf20e9ca9, 0xe373f6d0, 0x5a882d38, 0xc1892222,
    0x7872f9ca, 0x690f93b3, 0xd0f4485b, 0xc01e1489, 0x79e5cf61,
    0x6898a518, 0xd1637ef0, 0x4a6271ea, 0xf399aa02, 0xe2e4c07b,
    0x5b1f1b93, 0x0f97d80e, 0xb66c03e6, 0xa711699f, 0x1eeab277,
    0x85ebbd6d, 0x3c106685, 0x2d6d0cfc, 0x9496d714, 0x0cb9b558,
    0xb5426eb0, 0xa43f04c9, 0x1dc4df21, 0x86c5d03b, 0x3f3e0bd3,
    0x2e4361aa, 0x97b8ba42, 0xc33079df, 0x7acba237, 0x6bb6c84e,
    0xd24d13a6, 0x494c1cbc, 0xf0b7c754, 0xe1caad2d, 0x583176c5,
    0x48db2a17, 0xf120f1ff, 0xe05d9b86, 0x59a6406e, 0xc2a74f74,
    0x7b5c949c, 0x6a21fee5, 0xd3da250d, 0x8752e690, 0x3ea93d78,
    0x2fd45701, 0x962f8ce9, 0x0d2e83f3, 0xb4d5581b, 0xa5a83262,
    0x1c53e98a},
   {0x00000000, 0x9d0fe176, 0xe16ec4ad, 0x7c6125db, 0x19ac8f1b,
    0x84a36e6d, 0xf8c24bb6, 0x65cdaac0, 0x33591e36, 0xae56ff40,
    0xd237da9b, 0x4f383bed, 0x2af5912d, 0xb7fa705b, 0xcb9b5580,
    0x5694b4f6, 0x66b23c6c, 0xfbbddd1a, 0x87dcf8c1, 0x1ad319b7,
    0x7f1eb377, 0xe2115201, 0x9e7077da, 0x037f96ac, 0x55eb225a,
    0xc8e4c32c, 0xb485e6f7, 0x298a0781, 0x4c47ad41, 0xd1484c37,
    0xad2969ec, 0x3026889a, 0xcd6478d8, 0x506b99ae, 0x2c0abc75,
    0xb1055d03, 0xd4c8f7c3, 0x49c716b5, 0x35a6336e, 0xa8a9d218,
    0xfe3d66ee, 0x63328798, 0x1f53a243, 0x825c4335, 0xe791e9f5,
    0x7a9e0883, 0x06ff2d58, 0x9bf0cc2e, 0xabd644b4, 0x36d9a5c2,
    0x4ab88019, 0xd7b7616f, 0xb27acbaf, 0x2f752ad9, 0x53140f02,
    0xce1bee74, 0x988f5a82, 0x0580bbf4, 0x79e19e2f, 0xe4ee7f59,
    0x8123d599, 0x1c2c34ef, 0x604d1134, 0xfd42f042, 0x41b9f7f1,
    0xdcb61687, 0xa0d7335c, 0x3dd8d22a, 0x581578ea, 0xc51a999c,
    0xb97bbc47, 0x24745d31, 0x72e0e9c7, 0xefef08b1, 0x938e2d6a,
    0x0e81cc1c, 0x6b4c66dc, 0xf64387aa, 0x8a22a271, 0x172d4307,
    0x270bcb9d, 0xba042aeb, 0xc6650f30, 0x5b6aee46, 0x3ea74486,
    0xa3a8a5f0, 0xdfc9802b, 0x42c6615d, 0x1452d5ab, 0x895d34dd,
    0xf53c1106, 0x6833f070, 0x0dfe5ab0, 0x90f1bbc6, 0xec909e1d,
    0x719f7f6b, 0x8cdd8f29, 0x11d26e5f, 0x6db34b84, 0xf0bcaaf2,
    0x95710032, 0x087ee144, 0x741fc49f, 0xe91025e9, 0xbf84911f,
    0x228b7069, 0x5eea55b2, 0xc3e5b4c4, 0xa6281e04, 0x3b27ff72,
    0x4746daa9, 0xda493bdf, 0xea6fb345, 0x77605233, 0x0b0177e8,
    0x960e969e, 0xf3c33c5e, 0x6eccdd28, 0x12adf8f3, 0x8fa21985,
    0xd936ad73, 0x44394c05, 0x385869de, 0xa55788a8, 0xc09a2268,
    0x5d95c31e, 0x21f4e6c5, 0xbcfb07b3, 0x8373efe2, 0x1e7c0e94,
    0x621d2b4f, 0xff12ca39, 0x9adf60f9, 0x07d0818f, 0x7bb1a454,
    0xe6be4522, 0xb02af1d4, 0x2d2510a2, 0x51443579, 0xcc4bd40f,
    0xa9867ecf, 0x34899fb9, 0x48e8ba62, 0xd5e75b14, 0xe5c1d38e,
    0x78ce32f8, 0x04af1723, 0x99a0f655, 0xfc6d5c95, 0x6162bde3,
    0x1d039838, 0x800c794e, 0xd698cdb8, 0x4b972cce, 0x37f60915,
    0xaaf9e863, 0xcf3442a3, 0x523ba3d5, 0x2e5a860e, 0xb3556778,
    0x4e17973a, 0xd318764c, 0xaf795397, 0x3276b2e1, 0x57bb1821,
    0xcab4f957, 0xb6d5dc8c, 0x2bda3dfa, 0x7d4e890c, 0xe041687a,
    0x9c204da1, 0x012facd7, 0x64e20617, 0xf9ede761, 0x858cc2ba,
    0x188323cc, 0x28a5ab56, 0xb5aa4a20, 0xc9cb6ffb, 0x54c48e8d,
    0x3109244d, 0xac06c53b, 0xd067e0e0, 0x4d680196, 0x1bfcb560,
    0x86f35416, 0xfa9271cd, 0x679d90bb, 0x02503a7b, 0x9f5fdb0d,
    0xe33efed6, 0x7e311fa0, 0xc2ca1813, 0x5fc5f965, 0x23a4dcbe,
    0xbeab3dc8, 0xdb669708, 0x4669767e, 0x3a0853a5, 0xa707b2d3,
    0xf1930625, 0x6c9ce753, 0x10fdc288, 0x8df223fe, 0xe83f893e,
    0x75306848, 0x09514d93, 0x945eace5, 0xa478247f, 0x3977c509,
    0x4516e0d2, 0xd81901a4, 0xbdd4ab64, 0x20db4a12, 0x5cba6fc9,
    0xc1b58ebf, 0x97213a49, 0x0a2edb3f, 0x764ffee4, 0xeb401f92,
    0x8e8db552, 0x13825424, 0x6fe371ff, 0xf2ec9089, 0x0fae60cb,
    0x92a181bd, 0xeec0a466, 0x73cf4510, 0x1602efd0, 0x8b0d0ea6,
    0xf76c2b7d, 0x6a63ca0b, 0x3cf77efd, 0xa1f89f8b, 0xdd99ba50,
    0x40965b26, 0x255bf1e6, 0xb8541090, 0xc435354b, 0x593ad43d,
    0x691c5ca7, 0xf413bdd1, 0x8872980a, 0x157d797c, 0x70b0d3bc,
    0xedbf32ca, 0x91de1711, 0x0cd1f667, 0x5a454291, 0xc74aa3e7,
    0xbb2b863c, 0x2624674a, 0x43e9cd8a, 0xdee62cfc, 0xa2870927,
    0x3f88e851},
   {0x00000000, 0xdd96d985, 0x605cb54b, 0xbdca6cce, 0xc0b96a96,
    0x1d2fb313, 0xa0e5dfdd, 0x7d730658, 0x5a03d36d, 0x87950ae8,
    0x3a5f6626, 0xe7c9bfa3, 0x9abab9fb, 0x472c607e, 0xfae60cb0,
    0x2770d535, 0xb407a6da, 0x69917f5f, 0xd45b1391, 0x09cdca14,
    0x74becc4c, 0xa92815c9, 0x14e27907, 0xc974a082, 0xee0475b7,
    0x3392ac32, 0x8e58c0fc, 0x53ce1979, 0x2ebd1f21, 0xf32bc6a4,
    0x4ee1aa6a, 0x937773ef, 0xb37e4bf5, 0x6ee89270, 0xd322febe,
    0x0eb4273b, 0x73c72163, 0xae51f8e6, 0x139b9428, 0xce0d4dad,
    0xe97d9898, 0x34eb411d, 0x89212dd3, 0x54b7f456, 0x29c4f20e,
    0xf4522b8b, 0x49984745, 0x940e9ec0, 0x0779ed2f, 0xdaef34aa,
    0x67255864, 0xbab381e1, 0xc7c087b9, 0x1a565e3c, 0xa79c32f2,
    0x7a0aeb77, 0x5d7a3e42, 0x80ece7c7, 0x3d268b09, 0xe0b0528c,
    0x9dc354d4, 0x40558d51, 0xfd9fe19f, 0x2009381a, 0xbd8d91ab,
    0x601b482e, 0xddd124e0, 0x0047fd65, 0x7d34fb3d, 0xa0a222b8,
    0x1d684e76, 0xc0fe97f3, 0xe78e42c6, 0x3a189b43, 0x87d2f78d,
    0x5a442e08, 0x27372850, 0xfaa1f1d5, 0x476b9d1b, 0x9afd449e,
    0x098a3771, 0xd41ceef4, 0x69d6823a, 0xb4405bbf, 0xc9335de7,
    0x14a58462, 0xa96fe8ac, 0x74f93129, 0x5389e41c, 0x8e1f3d99,
    0x33d55157, 0xee4388d2, 0x93308e8a, 0x4ea6570f, 0xf36c3bc1,
    0x2efae244, 0x0ef3da5e, 0xd36503db, 0x6eaf6f15, 0xb339b690,
    0xce4ab0c8, 0x13dc694d, 0xae160583, 0x7380dc06, 0x54f00933,
    0x8966d0b6, 0x34acbc78, 0xe93a65fd, 0x944963a5, 0x49dfba20,
    0xf415d6ee, 0x29830f6b, 0xbaf47c84, 0x6762a501, 0xdaa8c9cf,
    0x073e104a, 0x7a4d1612, 0xa7dbcf97, 0x1a11a359, 0xc7877adc,
    0xe0f7afe9, 0x3d61766c, 0x80ab1aa2, 0x5d3dc327, 0x204ec57f,
    0xfdd81cfa, 0x40127034, 0x9d84a9b1, 0xa06a2517, 0x7dfcfc92,
    0xc036905c, 0x1da049d9, 0x60d34f81, 0xbd459604, 0x008ffaca,
    0xdd19234f, 0xfa69f67a, 0x27ff2fff, 0x9a354331, 0x47a39ab4,
    0x3ad09cec, 0xe7464569, 0x5a8c29a7, 0x871af022, 0x146d83cd,
    0xc9fb5a48, 0x74313686, 0xa9a7ef03, 0xd4d4e95b, 0x094230de,
    0xb4885c10, 0x691e8595, 0x4e6e50a0, 0x93f88925, 0x2e32e5eb,
    0xf3a43c6e, 0x8ed73a36, 0x5341e3b3, 0xee8b8f7d, 0x331d56f8,
    0x13146ee2, 0xce82b767, 0x7348dba9, 0xaede022c, 0xd3ad0474,
    0x0e3bddf1, 0xb3f1b13f, 0x6e6768ba, 0x4917bd8f, 0x9481640a,
    0x294b08c4, 0xf4ddd141, 0x89aed719, 0x54380e9c, 0xe9f26252,
    0x3464bbd7, 0xa713c838, 0x7a8511bd, 0xc74f7d73, 0x1ad9a4f6,
    0x67aaa2ae, 0xba3c7b2b, 0x07f617e5, 0xda60ce60, 0xfd101b55,
    0x2086c2d0, 0x9d4cae1e, 0x40da779b, 0x3da971c3, 0xe03fa846,
    0x5df5c488, 0x80631d0d, 0x1de7b4bc, 0xc0716d39, 0x7dbb01f7,
    0xa02dd872, 0xdd5ede2a, 0x00c807af, 0xbd026b61, 0x6094b2e4,
    0x47e467d1, 0x9a72be54, 0x27b8d29a, 0xfa2e0b1f, 0x875d0d47,
    0x5acbd4c2, 0xe701b80c, 0x3a976189, 0xa9e01266, 0x7476cbe3,
    0xc9bca72d, 0x142a7ea8, 0x695978f0, 0xb4cfa175, 0x0905cdbb,
    0xd493143e, 0xf3e3c10b, 0x2e75188e, 0x93bf7440, 0x4e29adc5,
    0x335aab9d, 0xeecc7218, 0x53061ed6, 0x8e90c753, 0xae99ff49,
    0x730f26cc, 0xcec54a02, 0x13539387, 0x6e2095df, 0xb3b64c5a,
    0x0e7c2094, 0xd3eaf911, 0xf49a2c24, 0x290cf5a1, 0x94c6996f,
    0x495040ea, 0x342346b2, 0xe9b59f37, 0x547ff3f9, 0x89e92a7c,
    0x1a9e5993, 0xc7088016, 0x7ac2ecd8, 0xa754355d, 0xda273305,
    0x07b1ea80, 0xba7b864e, 0x67ed5fcb, 0x409d8afe, 0x9d0b537b,
    0x20c13fb5, 0xfd57e630, 0x8024e068, 0x5db239ed, 0xe0785523,
    0x3dee8ca6}};

local const z_word_t FAR crc_braid_big_table[][256] = {
   {0x00000000, 0x85d996dd, 0x4bb55c60, 0xce6ccabd, 0x966ab9c0,
    0x13b32f1d, 0xdddfe5a0, 0x5806737d, 0x6dd3035a, 0xe80a9587,
    0x26665f3a, 0xa3bfc9e7, 0xfbb9ba9a, 0x7e602c47, 0xb00ce6fa,
    0x35d57027, 0xdaa607b4, 0x5f7f9169, 0x91135bd4, 0x14cacd09,
    0x4cccbe74, 0xc91528a9, 0x0779e214, 0x82a074c9, 0xb77504ee,
    0x32ac9233, 0xfcc0588e, 0x7919ce53, 0x211fbd2e, 0xa4c62bf3,
    0x6aaae14e, 0xef737793, 0xf54b7eb3, 0x7092e86e, 0xbefe22d3,
    0x3b27b40e, 0x6321c773, 0xe6f851ae, 0x28949b13, 0xad4d0dce,
    0x98987de9, 0x1d41eb34, 0xd32d2189, 0x56f4b754, 0x0ef2c429,
    0x8b2b52f4, 0x45479849, 0xc09e0e94, 0x2fed7907, 0xaa34efda,
    0x64582567, 0xe181b3ba, 0xb987c0c7, 0x3c5e561a, 0xf2329ca7,
    0x77eb0a7a, 0x423e7a5d, 0xc7e7ec80, 0x098b263d, 0x8c52b0e0,
    0xd454c39d, 0x518d5540, 0x9fe19ffd, 0x1a380920, 0xab918dbd,
    0x2e481b60, 0xe024d1dd, 0x65fd4700, 0x3dfb347d, 0xb822a2a0,
    0x764e681d, 0xf397fec0, 0xc6428ee7, 0x439b183a, 0x8df7d287,
    0x082e445a, 0x50283727, 0xd5f1a1fa, 0x1b9d6b47, 0x9e44fd9a,
    0x71378a09, 0xf4ee1cd4, 0x3a82d669, 0xbf5b40b4, 0xe75d33c9,
    0x6284a514, 0xace86fa9, 0x2931f974, 0x1ce48953, 0x993d1f8e,
    0x5751d533, 0xd28843ee, 0x8a8e3093, 0x0f57a64e, 0xc13b6cf3,
    0x44e2fa2e, 0x5edaf30e, 0xdb0365d3, 0x156faf6e, 0x90b639b3,
    0xc8b04ace, 0x4d69dc13, 0x830516ae, 0x06dc8073, 0x3309f054,
    0xb6d06689, 0x78bcac34, 0xfd653ae9, 0xa5634994, 0x20badf49,
    0xeed615f4, 0x6b0f8329, 0x847cf4ba, 0x01a56267, 0xcfc9a8da,
    0x4a103e07, 0x12164d7a, 0x97cfdba7, 0x59a3111a, 0xdc7a87c7,
    0xe9aff7e0, 0x6c76613d, 0xa21aab80, 0x27c33d5d, 0x7fc54e20,
    0xfa1cd8fd, 0x34701240, 0xb1a9849d, 0x17256aa0, 0x92fcfc7d,
    0x5c9036c0, 0xd949a01d, 0x814fd360, 0x049645bd, 0xcafa8f00,
    0x4f2319dd, 0x7af669fa, 0xff2fff27, 0x3143359a, 0xb49aa347,
    0xec9cd03a, 0x694546e7, 0xa7298c5a, 0x22f01a87, 0xcd836d14,
    0x485afbc9, 0x86363174, 0x03efa7a9, 0x5be9d4d4, 0xde304209,
    0x105c88b4, 0x95851e69, 0xa0506e4e, 0x2589f893, 0xebe5322e,
    0x6e3ca4f3, 0x363ad78e, 0xb3e34153, 0x7d8f8bee, 0xf8561d33,
    0xe26e1413, 0x67b782ce, 0xa9db4873, 0x2c02deae, 0x7404add3,
    0xf1dd3b0e, 0x3fb1f1b3, 0xba68676e, 0x8fbd1749, 0x0a648194,
    0xc4084b29, 0x41d1ddf4, 0x19d7ae89, 0x9c0e3854, 0x5262f2e9,
    0xd7bb6434, 0x38c813a7, 0xbd11857a, 0x737d4fc7, 0xf6a4d91a,
    0xaea2aa67, 0x2b7b3cba, 0xe517f607, 0x60ce60da, 0x551b10fd,
    0xd0c28620, 0x1eae4c9d, 0x9b77da40, 0xc371a93d, 0x46a83fe0,
    0x88c4f55d, 0x0d1d6380, 0xbcb4e71d, 0x396d71c0, 0xf701bb7d,
    0x72d82da0, 0x2ade5edd, 0xaf07c800, 0x616b02bd, 0xe4b29460,
    0xd167e447, 0x54be729a, 0x9ad2b827, 0x1f0b2efa, 0x470d5d87,
    0xc2d4cb5a, 0x0cb801e7, 0x8961973a, 0x6612e0a9, 0xe3cb7674,
    0x2da7bcc9, 0xa87e2a14, 0xf0785969, 0x75a1cfb4, 0xbbcd0509,
    0x3e1493d4, 0x0bc1e3f3, 0x8e18752e, 0x4074bf93, 0xc5ad294e,
    0x9dab5a33, 0x1872ccee, 0xd61e0653, 0x53c7908e, 0x49ff99ae,
    0xcc260f73, 0x024ac5ce, 0x87935313, 0xdf95206e, 0x5a4cb6b3,
    0x94207c0e, 0x11f9ead3, 0x242c9af4, 0xa1f50c29, 0x6f99c694,
    0xea405049, 0xb2462334, 0x379fb5e9, 0xf9f37f54, 0x7c2ae989,
    0x93599e1a, 0x168008c7, 0xd8ecc27a, 0x5d3554a7, 0x053327da,
    0x80eab107, 0x4e867bba, 0xcb5fed67, 0xfe8a9d40, 0x7b530b9d,
    0xb53fc120, 0x30e657fd, 0x68e02480, 0xed39b25d, 0x235578e0,
    0xa68cee3d},
   {0x00000000, 0x76e10f9d, 0xadc46ee1, 0xdb25617c, 0x1b8fac19,
    0x6d6ea384, 0xb64bc2f8, 0xc0aacd65, 0x361e5933, 0x40ff56ae,
    0x9bda37d2, 0xed3b384f, 0x2d91f52a, 0x5b70fab7, 0x80559bcb,
    0xf6b49456, 0x6c3cb266, 0x1addbdfb, 0xc1f8dc87, 0xb719d31a,
    0x77b31e7f, 0x015211e2, 0xda77709e, 0xac967f03, 0x5a22eb55,
    0x2cc3e4c8, 0xf7e685b4, 0x81078a29, 0x41ad474c, 0x374c48d1,
    0xec6929ad, 0x9a882630, 0xd87864cd, 0xae996b50, 0x75bc0a2c,
    0x035d05b1, 0xc3f7c8d4, 0xb516c749, 0x6e33a635, 0x18d2a9a8,
    0xee663dfe, 0x98873263, 0x43a2531f, 0x35435c82, 0xf5e991e7,
    0x83089e7a, 0x582dff06, 0x2eccf09b, 0xb444d6ab, 0xc2a5d936,
    0x1980b84a, 0x6f61b7d7, 0xafcb7ab2, 0xd92a752f, 0x020f1453,
    0x74ee1bce, 0x825a8f98, 0xf4bb8005, 0x2f9ee179, 0x597feee4,
    0x99d52381, 0xef342c1c, 0x34114d60, 0x42f042fd, 0xf1f7b941,
    0x8716b6dc, 0x5c33d7a0, 0x2ad2d83d, 0xea781558, 0x9c991ac5,
    0x47bc7bb9, 0x315d7424, 0xc7e9e072, 0xb108efef, 0x6a2d8e93,
    0x1ccc810e, 0xdc664c6b, 0xaa8743f6, 0x71a2228a, 0x07432d17,
    0x9dcb0b27, 0xeb2a04ba, 0x300f65c6, 0x46ee6a5b, 0x8644a73e,
    0xf0a5a8a3, 0x2b80c9df, 0x5d61c642, 0xabd55214, 0xdd345d89,
    0x06113cf5, 0x70f03368, 0xb05afe0d, 0xc6bbf190, 0x1d9e90ec,
    0x6b7f9f71, 0x298fdd8c, 0x5f6ed211, 0x844bb36d, 0xf2aabcf0,
    0x32007195, 0x44e17e08, 0x9fc41f74, 0xe92510e9, 0x1f9184bf,
    0x69708b22, 0xb255ea5e, 0xc4b4e5c3, 0x041e28a6, 0x72ff273b,
    0xa9da4647, 0xdf3b49da, 0x45b36fea, 0x33526077, 0xe877010b,
    0x9e960e96, 0x5e3cc3f3, 0x28ddcc6e, 0xf3f8ad12, 0x8519a28f,
    0x73ad36d9, 0x054c3944, 0xde695838, 0xa88857a5, 0x68229ac0,
    0x1ec3955d, 0xc5e6f421, 0xb307fbbc, 0xe2ef7383, 0x940e7c1e,
    0x4f2b1d62, 0x39ca12ff, 0xf960df9a, 0x8f81d007, 0x54a4b17b,
    0x2245bee6, 0xd4f12ab0, 0xa210252d, 0x79354451, 0x0fd44bcc,
    0xcf7e86a9, 0xb99f8934, 0x62bae848, 0x145be7d5, 0x8ed3c1e5,
    0xf832ce78, 0x2317af04, 0x55f6a099, 0x955c6dfc, 0xe3bd6261,
    0x3898031d, 0x4e790c80, 0xb8cd98d6, 0xce2c974b, 0x1509f637,
    0x63e8f9aa, 0xa34234cf, 0xd5a33b52, 0x0e865a2e, 0x786755b3,
    0x3a97174e, 0x4c7618d3, 0x975379af, 0xe1b27632, 0x2118bb57,
    0x57f9b4ca, 0x8cdcd5b6, 0xfa3dda2b, 0x0c894e7d, 0x7a6841e0,
    0xa14d209c, 0xd7ac2f01, 0x1706e264, 0x61e7edf9, 0xbac28c85,
    0xcc238318, 0x56aba528, 0x204aaab5, 0xfb6fcbc9, 0x8d8ec454,
    0x4d240931, 0x3bc506ac, 0xe0e067d0, 0x9601684d, 0x60b5fc1b,
    0x1654f386, 0xcd7192fa, 0xbb909d67, 0x7b3a5002, 0x0ddb5f9f,
    0xd6fe3ee3, 0xa01f317e, 0x1318cac2, 0x65f9c55f, 0xbedca423,
    0xc83dabbe, 0x089766db, 0x7e766946, 0xa553083a, 0xd3b207a7,
    0x250693f1, 0x53e79c6c, 0x88c2fd10, 0xfe23f28d, 0x3e893fe8,
    0x48683075, 0x934d5109, 0xe5ac5e94, 0x7f2478a4, 0x09c57739,
    0xd2e01645, 0xa40119d8, 0x64abd4bd, 0x124adb20, 0xc96fba5c,
    0xbf8eb5c1, 0x493a2197, 0x3fdb2e0a, 0xe4fe4f76, 0x921f40eb,
    0x52b58d8e, 0x24548213, 0xff71e36f, 0x8990ecf2, 0xcb60ae0f,
    0xbd81a192, 0x66a4c0ee, 0x1045cf73, 0xd0ef0216, 0xa60e0d8b,
    0x7d2b6cf7, 0x0bca636a, 0xfd7ef73c, 0x8b9ff8a1, 0x50ba99dd,
    0x265b9640, 0xe6f15b25, 0x901054b8, 0x4b3535c4, 0x3dd43a59,
    0xa75c1c69, 0xd1bd13f4, 0x0a987288, 0x7c797d15, 0xbcd3b070,
    0xca32bfed, 0x1117de91, 0x67f6d10c, 0x9142455a, 0xe7a34ac7,
    0x3c862bbb, 0x4a672426, 0x8acde943, 0xfc2ce6de, 0x270987a2,
    0x51e8883f},
   {0x00000000, 0xe8dbfbb9, 0x91b186a8, 0x796a7d11, 0x63657c8a,
    0x8bbe8733, 0xf2d4fa22, 0x1a0f019b, 0x87cc89cf, 0x6f177276,
    0x167d0f67, 0xfea6f4de, 0xe4a9f545, 0x0c720efc, 0x751873ed,
    0x9dc38854, 0x4f9f6244, 0xa74499fd, 0xde2ee4ec, 0x36f51f55,
    0x2cfa1ece, 0xc421e577, 0xbd4b9866, 0x559063df, 0xc853eb8b,
    0x20881032, 0x59e26d23, 0xb139969a, 0xab369701, 0x43ed6cb8,
    0x3a8711a9, 0xd25cea10, 0x9e3ec588, 0x76e53e31, 0x0f8f4320,
    0xe754b899, 0xfd5bb902, 0x158042bb, 0x6cea3faa, 0x8431c413,
    0x19f24c47, 0xf129b7fe, 0x8843caef, 0x60983156, 0x7a9730cd,
    0x924ccb74, 0xeb26b665, 0x03fd4ddc, 0xd1a1a7cc, 0x397a5c75,
    0x40102164, 0xa8cbdadd, 0xb2c4db46, 0x5a1f20ff, 0x23755dee,
    0xcbaea657, 0x566d2e03, 0xbeb6d5ba, 0xc7dca8ab, 0x2f075312,
    0x35085289, 0xddd3a930, 0xa4b9d421, 0x4c622f98, 0x7d7bfbca,
    0x95a00073, 0xecca7d62, 0x041186db, 0x1e1e8740, 0xf6c57cf9,
    0x8faf01e8, 0x6774fa51, 0xfab77205, 0x126c89bc, 0x6b06f4ad,
    0x83dd0f14, 0x99d20e8f, 0x7109f536, 0x08638827, 0xe0b8739e,
    0x32e4998e, 0xda3f6237, 0xa3551f26, 0x4b8ee49f, 0x5181e504,
    0xb95a1ebd, 0xc03063ac, 0x28eb9815, 0xb5281041, 0x5df3ebf8,
    0x249996e9, 0xcc426d50, 0xd64d6ccb, 0x3e969772, 0x47fcea63,
    0xaf2711da, 0xe3453e42, 0x0b9ec5fb, 0x72f4b8ea, 0x9a2f4353,
    0x802042c8, 0x68fbb971, 0x1191c460, 0xf94a3fd9, 0x6489b78d,
    0x8c524c34, 0xf5383125, 0x1de3ca9c, 0x07eccb07, 0xef3730be,
    0x965d4daf, 0x7e86b616, 0xacda5c06, 0x4401a7bf, 0x3d6bdaae,
    0xd5b02117, 0xcfbf208c, 0x2764db35, 0x5e0ea624, 0xb6d55d9d,
    0x2b16d5c9, 0xc3cd2e70, 0xbaa75361, 0x527ca8d8, 0x4873a943,
    0xa0a852fa, 0xd9c22feb, 0x3119d452, 0xbbf0874e, 0x532b7cf7,
    0x2a4101e6, 0xc29afa5f, 0xd895fbc4, 0x304e007d, 0x49247d6c,
    0xa1ff86d5, 0x3c3c0e81, 0xd4e7f538, 0xad8d8829, 0x45567390,
    0x5f59720b, 0xb78289b2, 0xcee8f4a3, 0x26330f1a, 0xf46fe50a,
    0x1cb41eb3, 0x65de63a2, 0x8d05981b, 0x970a9980, 0x7fd16239,
    0x06bb1f28, 0xee60e491, 0x73a36cc5, 0x9b78977c, 0xe212ea6d,
    0x0ac911d4, 0x10c6104f, 0xf81debf6, 0x817796e7, 0x69ac6d5e,
    0x25ce42c6, 0xcd15b97f, 0xb47fc46e, 0x5ca43fd7, 0x46ab3e4c,
    0xae70c5f5, 0xd71ab8e4, 0x3fc1435d, 0xa202cb09, 0x4ad930b0,
    0x33b34da1, 0xdb68b618, 0xc167b783, 0x29bc4c3a, 0x50d6312b,
    0xb80dca92, 0x6a512082, 0x828adb3b, 0xfbe0a62a, 0x133b5d93,
    0x09345c08, 0xe1efa7b1, 0x9885daa0, 0x705e2119, 0xed9da94d,
    0x054652f4, 0x7c2c2fe5, 0x94f7d45c, 0x8ef8d5c7, 0x66232e7e,
    0x1f49536f, 0xf792a8d6, 0xc68b7c84, 0x2e50873d, 0x573afa2c,
    0xbfe10195, 0xa5ee000e, 0x4d35fbb7, 0x345f86a6, 0xdc847d1f,
    0x4147f54b, 0xa99c0ef2, 0xd0f673e3, 0x382d885a, 0x222289c1,
    0xcaf97278, 0xb3930f69, 0x5b48f4d0, 0x89141ec0, 0x61cfe579,
    0x18a59868, 0xf07e63d1, 0xea71624a, 0x02aa99f3, 0x7bc0e4e2,
    0x931b1f5b, 0x0ed8970f, 0xe6036cb6, 0x9f6911a7, 0x77b2ea1e,
    0x6dbdeb85, 0x8566103c, 0xfc0c6d2d, 0x14d79694, 0x58b5b90c,
    0xb06e42b5, 0xc9043fa4, 0x21dfc41d, 0x3bd0c586, 0xd30b3e3f,
    0xaa61432e, 0x42bab897, 0xdf7930c3, 0x37a2cb7a, 0x4ec8b66b,
    0xa6134dd2, 0xbc1c4c49, 0x54c7b7f0, 0x2dadcae1, 0xc5763158,
    0x172adb48, 0xfff120f1, 0x869b5de0, 0x6e40a659, 0x744fa7c2,
    0x9c945c7b, 0xe5fe216a, 0x0d25dad3, 0x90e65287, 0x783da93e,
    0x0157d42f, 0xe98c2f96, 0xf3832e0d, 0x1b58d5b4, 0x6232a8a5,
    0x8ae9531c},
   {0x00000000, 0x919168ae, 0x6325a087, 0xf2b4c829, 0x874c31d4,
    0x16dd597a, 0xe4699153, 0x75f8f9fd, 0x4f9f1373, 0xde0e7bdd,
    0x2cbab3f4, 0xbd2bdb5a, 0xc8d322a7, 0x59424a09, 0xabf68220,
    0x3a67ea8e, 0x9e3e27e6, 0x0faf4f48, 0xfd1b8761, 0x6c8aefcf,
    0x19721632, 0x88e37e9c, 0x7a57b6b5, 0xebc6de1b, 0xd1a13495,
    0x40305c3b, 0xb2849412, 0x2315fcbc, 0x56ed0541, 0xc77c6def,
    0x35c8a5c6, 0xa459cd68, 0x7d7b3f17, 0xecea57b9, 0x1e5e9f90,
    0x8fcff73e, 0xfa370ec3, 0x6ba6666d, 0x9912ae44, 0x0883c6ea,
    0x32e42c64, 0xa37544ca, 0x51c18ce3, 0xc050e44d, 0xb5a81db0,
    0x2439751e, 0xd68dbd37, 0x471cd599, 0xe34518f1, 0x72d4705f,
    0x8060b876, 0x11f1d0d8, 0x64092925, 0xf598418b, 0x072c89a2,
    0x96bde10c, 0xacda0b82, 0x3d4b632c, 0xcfffab05, 0x5e6ec3ab,
    0x2b963a56, 0xba0752f8, 0x48b39ad1, 0xd922f27f, 0xfaf67e2e,
    0x6b671680, 0x99d3dea9, 0x0842b607, 0x7dba4ffa, 0xec2b2754,
    0x1e9fef7d, 0x8f0e87d3, 0xb5696d5d, 0x24f805f3, 0xd64ccdda,
    0x47dda574, 0x32255c89, 0xa3b43427, 0x5100fc0e, 0xc09194a0,
    0x64c859c8, 0xf5593166, 0x07edf94f, 0x967c91e1, 0xe384681c,
    0x721500b2, 0x80a1c89b, 0x1130a035, 0x2b574abb, 0xbac62215,
    0x4872ea3c, 0xd9e38292, 0xac1b7b6f, 0x3d8a13c1, 0xcf3edbe8,
    0x5eafb346, 0x878d4139, 0x161c2997, 0xe4a8e1be, 0x75398910,
    0x00c170ed, 0x91501843, 0x63e4d06a, 0xf275b8c4, 0xc812524a,
    0x59833ae4, 0xab37f2cd, 0x3aa69a63, 0x4f5e639e, 0xdecf0b30,
    0x2c7bc319, 0xbdeaabb7, 0x19b366df, 0x88220e71, 0x7a96c658,
    0xeb07aef6, 0x9eff570b, 0x0f6e3fa5, 0xfddaf78c, 0x6c4b9f22,
    0x562c75ac, 0xc7bd1d02, 0x3509d52b, 0xa498bd85, 0xd1604478,
    0x40f12cd6, 0xb245e4ff, 0x23d48c51, 0xf4edfd5c, 0x657c95f2,
    0x97c85ddb, 0x06593575, 0x73a1cc88, 0xe230a426, 0x10846c0f,
    0x811504a1, 0xbb72ee2f, 0x2ae38681, 0xd8574ea8, 0x49c62606,
    0x3c3edffb, 0xadafb755, 0x5f1b7f7c, 0xce8a17d2, 0x6ad3daba,
    0xfb42b214, 0x09f67a3d, 0x98671293, 0xed9feb6e, 0x7c0e83c0,
    0x8eba4be9, 0x1f2b2347, 0x254cc9c9, 0xb4dda167, 0x4669694e,
    0xd7f801e0, 0xa200f81d, 0x339190b3, 0xc125589a, 0x50b43034,
    0x8996c24b, 0x1807aae5, 0xeab362cc, 0x7b220a62, 0x0edaf39f,
    0x9f4b9b31, 0x6dff5318, 0xfc6e3bb6, 0xc609d138, 0x5798b996,
    0xa52c71bf, 0x34bd1911, 0x4145e0ec, 0xd0d48842, 0x2260406b,
    0xb3f128c5, 0x17a8e5ad, 0x86398d03, 0x748d452a, 0xe51c2d84,
    0x90e4d479, 0x0175bcd7, 0xf3c174fe, 0x62501c50, 0x5837f6de,
    0xc9a69e70, 0x3b125659, 0xaa833ef7, 0xdf7bc70a, 0x4eeaafa4,
    0xbc5e678d, 0x2dcf0f23, 0x0e1b8372, 0x9f8aebdc, 0x6d3e23f5,
    0xfcaf4b5b, 0x8957b2a6, 0x18c6da08, 0xea721221, 0x7be37a8f,
    0x41849001, 0xd015f8af, 0x22a13086, 0xb3305828, 0xc6c8a1d5,
    0x5759c97b, 0xa5ed0152, 0x347c69fc, 0x9025a494, 0x01b4cc3a,
    0xf3000413, 0x62916cbd, 0x17699540, 0x86f8fdee, 0x744c35c7,
    0xe5dd5d69, 0xdfbab7e7, 0x4e2bdf49, 0xbc9f1760, 0x2d0e7fce,
    0x58f68633, 0xc967ee9d, 0x3bd326b4, 0xaa424e1a, 0x7360bc65,
    0xe2f1d4cb, 0x10451ce2, 0x81d4744c, 0xf42c8db1, 0x65bde51f,
    0x97092d36, 0x06984598, 0x3cffaf16, 0xad6ec7b8, 0x5fda0f91,
    0xce4b673f, 0xbbb39ec2, 0x2a22f66c, 0xd8963e45, 0x490756eb,
    0xed5e9b83, 0x7ccff32d, 0x8e7b3b04, 0x1fea53aa, 0x6a12aa57,
    0xfb83c2f9, 0x09370ad0, 0x98a6627e, 0xa2c188f0, 0x3350e05e,
    0xc1e42877, 0x507540d9, 0x258db924, 0xb41cd18a, 0x46a819a3,
    0xd739710d}};

#endif

#endif

#if N == 5

#if W == 8

local const z_crc_t FAR crc_braid_table[][256] = {
   {0x00000000, 0xaf449247, 0x85f822cf, 0x2abcb088, 0xd08143df,
    0x7fc5d198, 0x55796110, 0xfa3df357, 0x7a7381ff, 0xd53713b8,
    0xff8ba330, 0x50cf3177, 0xaaf2c220, 0x05b65067, 0x2f0ae0ef,
    0x804e72a8, 0xf4e703fe, 0x5ba391b9, 0x711f2131, 0xde5bb376,
    0x24664021, 0x8b22d266, 0xa19e62ee, 0x0edaf0a9, 0x8e948201,
    0x21d01046, 0x0b6ca0ce, 0xa4283289, 0x5e15c1de, 0xf1515399,
    0xdbede311, 0x74a97156, 0x32bf01bd, 0x9dfb93fa, 0xb7472372,
    0x1803b135, 0xe23e4262, 0x4d7ad025, 0x67c660ad, 0xc882f2ea,
    0x48cc8042, 0xe7881205, 0xcd34a28d, 0x627030ca, 0x984dc39d,
    0x370951da, 0x1db5e152, 0xb2f17315, 0xc6580243, 0x691c9004,
    0x43a0208c, 0xece4b2cb, 0x16d9419c, 0xb99dd3db, 0x93216353,
    0x3c65f114, 0xbc2b83bc, 0x136f11fb, 0x39d3a173, 0x96973334,
    0x6caac063, 0xc3ee5224, 0xe952e2ac, 0x461670eb, 0x657e037a,
    0xca3a913d, 0xe08621b5, 0x4fc2b3f2, 0xb5ff40a5, 0x1abbd2e2,
    0x3007626a, 0x9f43f02d, 0x1f0d8285, 0xb04910c2, 0x9af5a04a,
    0x35b1320d, 0xcf8cc15a, 0x60c8531d, 0x4a74e395, 0xe53071d2,
    0x91990084, 0x3edd92c3, 0x1461224b, 0xbb25b00c, 0x4118435b,
    0xee5cd11c, 0xc4e06194, 0x6ba4f3d3, 0xebea817b, 0x44ae133c,
    0x6e12a3b4, 0xc15631f3, 0x3b6bc2a4, 0x942f50e3, 0xbe93e06b,
    0x11d7722c, 0x57c102c7, 0xf8859080, 0xd2392008, 0x7d7db24f,
    0x87404118, 0x2804d35f, 0x02b863d7, 0xadfcf190, 0x2db28338,
    0x82f6117f, 0xa84aa1f7, 0x070e33b0, 0xfd33c0e7, 0x527752a0,
    0x78cbe228, 0xd78f706f, 0xa3260139, 0x0c62937e, 0x26de23f6,
    0x899ab1b1, 0x73a742e6, 0xdce3d0a1, 0xf65f6029, 0x591bf26e,
    0xd95580c6, 0x76111281, 0x5cada209, 0xf3e9304e, 0x09d4c319,
    0xa690515e, 0x8c2ce1d6, 0x23687391, 0xcafc06f4, 0x65b894b3,
    0x4f04243b, 0xe040b67c, 0x1a7d452b, 0xb539d76c, 0x9f8567e4,
    0x30c1f5a3, 0xb08f870b, 0x1fcb154c, 0x3577a5c4, 0x9a333783,
    0x600ec4d4, 0xcf4a5693, 0xe5f6e61b, 0x4ab2745c, 0x3e1b050a,
    0x915f974d, 0xbbe327c5, 0x14a7b582, 0xee9a46d5, 0x41ded492,
    0x6b62641a, 0xc426f65d, 0x446884f5, 0xeb2c16b2, 0xc190a63a,
    0x6ed4347d, 0x94e9c72a, 0x3bad556d, 0x1111e5e5, 0xbe5577a2,
    0xf8430749, 0x5707950e, 0x7dbb2586, 0xd2ffb7c1, 0x28c24496,
    0x8786d6d1, 0xad3a6659, 0x027ef41e, 0x823086b6, 0x2d7414f1,
    0x07c8a479, 0xa88c363e, 0x52b1c569, 0xfdf5572e, 0xd749e7a6,
    0x780d75e1, 0x0ca404b7, 0xa3e096f0, 0x895c2678, 0x2618b43f,
    0xdc254768, 0x7361d52f, 0x59dd65a7, 0xf699f7e0, 0x76d78548,
    0xd993170f, 0xf32fa787, 0x5c6b35c0, 0xa656c697, 0x091254d0,
    0x23aee458, 0x8cea761f, 0xaf82058e, 0x00c697c9, 0x2a7a2741,
    0x853eb506, 0x7f034651, 0xd047d416, 0xfafb649e, 0x55bff6d9,
    0xd5f18471, 0x7ab51636, 0x5009a6be, 0xff4d34f9, 0x0570c7ae,
    0xaa3455e9, 0x8088e561, 0x2fcc7726, 0x5b650670, 0xf4219437,
    0xde9d24bf, 0x71d9b6f8, 0x8be445af, 0x24a0d7e8, 0x0e1c6760,
    0xa158f527, 0x2116878f, 0x8e5215c8, 0xa4eea540, 0x0baa3707,
    0xf197c450, 0x5ed35617, 0x746fe69f, 0xdb2b74d8, 0x9d3d0433,
    0x32799674, 0x18c526fc, 0xb781b4bb, 0x4dbc47ec, 0xe2f8d5ab,
    0xc8446523, 0x6700f764, 0xe74e85cc, 0x480a178b, 0x62b6a703,
    0xcdf23544, 0x37cfc613, 0x988b5454, 0xb237e4dc, 0x1d73769b,
    0x69da07cd, 0xc69e958a, 0xec222502, 0x4366b745, 0xb95b4412,
    0x161fd655, 0x3ca366dd, 0x93e7f49a, 0x13a98632, 0xbced1475,
    0x9651a4fd, 0x391536ba, 0xc328c5ed, 0x6c6c57aa, 0x46d0e722,
    0xe9947565},
   {0x00000000, 0x4e890ba9, 0x9d121752, 0xd39b1cfb, 0xe15528e5,
    0xafdc234c, 0x7c473fb7, 0x32ce341e, 0x19db578b, 0x57525c22,
    0x84c940d9, 0xca404b70, 0xf88e7f6e, 0xb60774c7, 0x659c683c,
    0x2b156395, 0x33b6af16, 0x7d3fa4bf, 0xaea4b844, 0xe02db3ed,
    0xd2e387f3, 0x9c6a8c5a, 0x4ff190a1, 0x01789b08, 0x2a6df89d,
    0x64e4f334, 0xb77fefcf, 0xf9f6e466, 0xcb38d078, 0x85b1dbd1,
    0x562ac72a, 0x18a3cc83, 0x676d5e2c, 0x29e45585, 0xfa7f497e,
    0xb4f642d7, 0x863876c9, 0xc8b17d60, 0x1b2a619b, 0x55a36a32,
    0x7eb609a7, 0x303f020e, 0xe3a41ef5, 0xad2d155c, 0x9fe32142,
    0xd16a2aeb, 0x02f13610, 0x4c783db9, 0x54dbf13a, 0x1a52fa93,
    0xc9c9e668, 0x8740edc1, 0xb58ed9df, 0xfb07d276, 0x289cce8d,
    0x6615c524, 0x4d00a6b1, 0x0389ad18, 0xd012b1e3, 0x9e9bba4a,
    0xac558e54, 0xe2dc85fd, 0x31479906, 0x7fce92af, 0xcedabc58,
    0x8053b7f1, 0x53c8ab0a, 0x1d41a0a3, 0x2f8f94bd, 0x61069f14,
    0xb29d83ef, 0xfc148846, 0xd701ebd3, 0x9988e07a, 0x4a13fc81,
    0x049af728, 0x3654c336, 0x78ddc89f, 0xab46d464, 0xe5cfdfcd,
    0xfd6c134e, 0xb3e518e7, 0x607e041c, 0x2ef70fb5, 0x1c393bab,
    0x52b03002, 0x812b2cf9, 0xcfa22750, 0xe4b744c5, 0xaa3e4f6c,
    0x79a55397, 0x372c583e, 0x05e26c20, 0x4b6b6789, 0x98f07b72,
    0xd67970db, 0xa9b7e274, 0xe73ee9dd, 0x34a5f526, 0x7a2cfe8f,
    0x48e2ca91, 0x066bc138, 0xd5f0ddc3, 0x9b79d66a, 0xb06cb5ff,
    0xfee5be56, 0x2d7ea2ad, 0x63f7a904, 0x51399d1a, 0x1fb096b3,
    0xcc2b8a48, 0x82a281e1, 0x9a014d62, 0xd48846cb, 0x07135a30,
    0x499a5199, 0x7b546587, 0x35dd6e2e, 0xe64672d5, 0xa8cf797c,
    0x83da1ae9, 0xcd531140, 0x1ec80dbb, 0x50410612, 0x628f320c,
    0x2c0639a5, 0xff9d255e, 0xb1142ef7, 0x46c47ef1, 0x084d7558,
    0xdbd669a3, 0x955f620a, 0xa7915614, 0xe9185dbd, 0x3a834146,
    0x740a4aef, 0x5f1f297a, 0x119622d3, 0xc20d3e28, 0x8c843581,
    0xbe4a019f, 0xf0c30a36, 0x235816cd, 0x6dd11d64, 0x7572d1e7,
    0x3bfbda4e, 0xe860c6b5, 0xa6e9cd1c, 0x9427f902, 0xdaaef2ab,
    0x0935ee50, 0x47bce5f9, 0x6ca9866c, 0x22208dc5, 0xf1bb913e,
    0xbf329a97, 0x8dfcae89, 0xc375a520, 0x10eeb9db, 0x5e67b272,
    0x21a920dd, 0x6f202b74, 0xbcbb378f, 0xf2323c26, 0xc0fc0838,
    0x8e750391, 0x5dee1f6a, 0x136714c3, 0x38727756, 0x76fb7cff,
    0xa5606004, 0xebe96bad, 0xd9275fb3, 0x97ae541a, 0x443548e1,
    0x0abc4348, 0x121f8fcb, 0x5c968462, 0x8f0d9899, 0xc1849330,
    0xf34aa72e, 0xbdc3ac87, 0x6e58b07c, 0x20d1bbd5, 0x0bc4d840,
    0x454dd3e9, 0x96d6cf12, 0xd85fc4bb, 0xea91f0a5, 0xa418fb0c,
    0x7783e7f7, 0x390aec5e, 0x881ec2a9, 0xc697c900, 0x150cd5fb,
    0x5b85de52, 0x694bea4c, 0x27c2e1e5, 0xf459fd1e, 0xbad0f6b7,
    0x91c59522, 0xdf4c9e8b, 0x0cd78270, 0x425e89d9, 0x7090bdc7,
    0x3e19b66e, 0xed82aa95, 0xa30ba13c, 0xbba86dbf, 0xf5216616,
    0x26ba7aed, 0x68337144, 0x5afd455a, 0x14744ef3, 0xc7ef5208,
    0x896659a1, 0xa2733a34, 0xecfa319d, 0x3f612d66, 0x71e826cf,
    0x432612d1, 0x0daf1978, 0xde340583, 0x90bd0e2a, 0xef739c85,
    0xa1fa972c, 0x72618bd7, 0x3ce8807e, 0x0e26b460, 0x40afbfc9,
    0x9334a332, 0xddbda89b, 0xf6a8cb0e, 0xb821c0a7, 0x6bbadc5c,
    0x2533d7f5, 0x17fde3eb, 0x5974e842, 0x8aeff4b9, 0xc466ff10,
    0xdcc53393, 0x924c383a, 0x41d724c1, 0x0f5e2f68, 0x3d901b76,
    0x731910df, 0xa0820c24, 0xee0b078d, 0xc51e6418, 0x8b976fb1,
    0x580c734a, 0x168578e3, 0x244b4cfd, 0x6ac24754, 0xb9595baf,
    0xf7d05006},
   {0x00000000, 0x8d88fde2, 0xc060fd85, 0x4de80067, 0x5bb0fd4b,
    0xd63800a9, 0x9bd000ce, 0x1658fd2c, 0xb761fa96, 0x3ae90774,
    0x77010713, 0xfa89faf1, 0xecd107dd, 0x6159fa3f, 0x2cb1fa58,
    0xa13907ba, 0xb5b2f36d, 0x383a0e8f, 0x75d20ee8, 0xf85af30a,
    0xee020e26, 0x638af3c4, 0x2e62f3a3, 0xa3ea0e41, 0x02d309fb,
    0x8f5bf419, 0xc2b3f47e, 0x4f3b099c, 0x5963f4b0, 0xd4eb0952,
    0x99030935, 0x148bf4d7, 0xb014e09b, 0x3d9c1d79, 0x70741d1e,
    0xfdfce0fc, 0xeba41dd0, 0x662ce032, 0x2bc4e055, 0xa64c1db7,
    0x07751a0d, 0x8afde7ef, 0xc715e788, 0x4a9d1a6a, 0x5cc5e746,
    0xd14d1aa4, 0x9ca51ac3, 0x112de721, 0x05a613f6, 0x882eee14,
    0xc5c6ee73, 0x484e1391, 0x5e16eebd, 0xd39e135f, 0x9e761338,
    0x13feeeda, 0xb2c7e960, 0x3f4f1482, 0x72a714e5, 0xff2fe907,
    0xe977142b, 0x64ffe9c9, 0x2917e9ae, 0xa49f144c, 0xbb58c777,
    0x36d03a95, 0x7b383af2, 0xf6b0c710, 0xe0e83a3c, 0x6d60c7de,
    0x2088c7b9, 0xad003a5b, 0x0c393de1, 0x81b1c003, 0xcc59c064,
    0x41d13d86, 0x5789c0aa, 0xda013d48, 0x97e93d2f, 0x1a61c0cd,
    0x0eea341a, 0x8362c9f8, 0xce8ac99f, 0x4302347d, 0x555ac951,
    0xd8d234b3, 0x953a34d4, 0x18b2c936, 0xb98bce8c, 0x3403336e,
    0x79eb3309, 0xf463ceeb, 0xe23b33c7, 0x6fb3ce25, 0x225bce42,
    0xafd333a0, 0x0b4c27ec, 0x86c4da0e, 0xcb2cda69, 0x46a4278b,
    0x50fcdaa7, 0xdd742745, 0x909c2722, 0x1d14dac0, 0xbc2ddd7a,
    0x31a52098, 0x7c4d20ff, 0xf1c5dd1d, 0xe79d2031, 0x6a15ddd3,
    0x27fdddb4, 0xaa752056, 0xbefed481, 0x33762963, 0x7e9e2904,
    0xf316d4e6, 0xe54e29ca, 0x68c6d428, 0x252ed44f, 0xa8a629ad,
    0x099f2e17, 0x8417d3f5, 0xc9ffd392, 0x44772e70, 0x522fd35c,
    0xdfa72ebe, 0x924f2ed9, 0x1fc7d33b, 0xadc088af, 0x2048754d,
    0x6da0752a, 0xe02888c8, 0xf67075e4, 0x7bf88806, 0x36108861,
    0xbb987583, 0x1aa17239, 0x97298fdb, 0xdac18fbc, 0x5749725e,
    0x41118f72, 0xcc997290, 0x817172f7, 0x0cf98f15, 0x18727bc2,
    0x95fa8620, 0xd8128647, 0x559a7ba5, 0x43c28689, 0xce4a7b6b,
    0x83a27b0c, 0x0e2a86ee, 0xaf138154, 0x229b7cb6, 0x6f737cd1,
    0xe2fb8133, 0xf4a37c1f, 0x792b81fd, 0x34c3819a, 0xb94b7c78,
    0x1dd46834, 0x905c95d6, 0xddb495b1, 0x503c6853, 0x4664957f,
    0xcbec689d, 0x860468fa, 0x0b8c9518, 0xaab592a2, 0x273d6f40,
    0x6ad56f27, 0xe75d92c5, 0xf1056fe9, 0x7c8d920b, 0x3165926c,
    0xbced6f8e, 0xa8669b59, 0x25ee66bb, 0x680666dc, 0xe58e9b3e,
    0xf3d66612, 0x7e5e9bf0, 0x33b69b97, 0xbe3e6675, 0x1f0761cf,
    0x928f9c2d, 0xdf679c4a, 0x52ef61a8, 0x44b79c84, 0xc93f6166,
    0x84d76101, 0x095f9ce3, 0x16984fd8, 0x9b10b23a, 0xd6f8b25d,
    0x5b704fbf, 0x4d28b293, 0xc0a04f71, 0x8d484f16, 0x00c0b2f4,
    0xa1f9b54e, 0x2c7148ac, 0x619948cb, 0xec11b529, 0xfa494805,
    0x77c1b5e7, 0x3a29b580, 0xb7a14862, 0xa32abcb5, 0x2ea24157,
    0x634a4130, 0xeec2bcd2, 0xf89a41fe, 0x7512bc1c, 0x38fabc7b,
    0xb5724199, 0x144b4623, 0x99c3bbc1, 0xd42bbba6, 0x59a34644,
    0x4ffbbb68, 0xc273468a, 0x8f9b46ed, 0x0213bb0f, 0xa68caf43,
    0x2b0452a1, 0x66ec52c6, 0xeb64af24, 0xfd3c5208, 0x70b4afea,
    0x3d5caf8d, 0xb0d4526f, 0x11ed55d5, 0x9c65a837, 0xd18da850,
    0x5c0555b2, 0x4a5da89e, 0xc7d5557c, 0x8a3d551b, 0x07b5a8f9,
    0x133e5c2e, 0x9eb6a1cc, 0xd35ea1ab, 0x5ed65c49, 0x488ea165,
    0xc5065c87, 0x88ee5ce0, 0x0566a102, 0xa45fa6b8, 0x29d75b5a,
    0x643f5b3d, 0xe9b7a6df, 0xffef5bf3, 0x7267a611, 0x3f8fa676,
    0xb2075b94},
   {0x00000000, 0x80f0171f, 0xda91287f, 0x5a613f60, 0x6e5356bf,
    0xeea341a0, 0xb4c27ec0, 0x343269df, 0xdca6ad7e, 0x5c56ba61,
    0x06378501, 0x86c7921e, 0xb2f5fbc1, 0x3205ecde, 0x6864d3be,
    0xe894c4a1, 0x623c5cbd, 0xe2cc4ba2, 0xb8ad74c2, 0x385d63dd,
    0x0c6f0a02, 0x8c9f1d1d, 0xd6fe227d, 0x560e3562, 0xbe9af1c3,
    0x3e6ae6dc, 0x640bd9bc, 0xe4fbcea3, 0xd0c9a77c, 0x5039b063,
    0x0a588f03, 0x8aa8981c, 0xc478b97a, 0x4488ae65, 0x1ee99105,
    0x9e19861a, 0xaa2befc5, 0x2adbf8da, 0x70bac7ba, 0xf04ad0a5,
    0x18de1404, 0x982e031b, 0xc24f3c7b, 0x42bf2b64, 0x768d42bb,
    0xf67d55a4, 0xac1c6ac4, 0x2cec7ddb, 0xa644e5c7, 0x26b4f2d8,
    0x7cd5cdb8, 0xfc25daa7, 0xc817b378, 0x48e7a467, 0x12869b07,
    0x92768c18, 0x7ae248b9, 0xfa125fa6, 0xa07360c6, 0x208377d9,
    0x14b11e06, 0x94410919, 0xce203679, 0x4ed02166, 0x538074b5,
    0xd37063aa, 0x89115cca, 0x09e14bd5, 0x3dd3220a, 0xbd233515,
    0xe7420a75, 0x67b21d6a, 0x8f26d9cb, 0x0fd6ced4, 0x55b7f1b4,
    0xd547e6ab, 0xe1758f74, 0x6185986b, 0x3be4a70b, 0xbb14b014,
    0x31bc2808, 0xb14c3f17, 0xeb2d0077, 0x6bdd1768, 0x5fef7eb7,
    0xdf1f69a8, 0x857e56c8, 0x058e41d7, 0xed1a8576, 0x6dea9269,
    0x378bad09, 0xb77bba16, 0x8349d3c9, 0x03b9c4d6, 0x59d8fbb6,
    0xd928eca9, 0x97f8cdcf, 0x1708dad0, 0x4d69e5b0, 0xcd99f2af,
    0xf9ab9b70, 0x795b8c6f, 0x233ab30f, 0xa3caa410, 0x4b5e60b1,
    0xcbae77ae, 0x91cf48ce, 0x113f5fd1, 0x250d360e, 0xa5fd2111,
    0xff9c1e71, 0x7f6c096e, 0xf5c49172, 0x7534866d, 0x2f55b90d,
    0xafa5ae12, 0x9b97c7cd, 0x1b67d0d2, 0x4106efb2, 0xc1f6f8ad,
    0x29623c0c, 0xa9922b13, 0xf3f31473, 0x7303036c, 0x47316ab3,
    0xc7c17dac, 0x9da042cc, 0x1d5055d3, 0xa700e96a, 0x27f0fe75,
    0x7d91c115, 0xfd61d60a, 0xc953bfd5, 0x49a3a8ca, 0x13c297aa,
    0x933280b5, 0x7ba64414, 0xfb56530b, 0xa1376c6b, 0x21c77b74,
    0x15f512ab, 0x950505b4, 0xcf643ad4, 0x4f942dcb, 0xc53cb5d7,
    0x45cca2c8, 0x1fad9da8, 0x9f5d8ab7, 0xab6fe368, 0x2b9ff477,
    0x71fecb17, 0xf10edc08, 0x199a18a9, 0x996a0fb6, 0xc30b30d6,
    0x43fb27c9, 0x77c94e16, 0xf7395909, 0xad586669, 0x2da87176,
    0x63785010, 0xe388470f, 0xb9e9786f, 0x39196f70, 0x0d2b06af,
    0x8ddb11b0, 0xd7ba2ed0, 0x574a39cf, 0xbfdefd6e, 0x3f2eea71,
    0x654fd511, 0xe5bfc20e, 0xd18dabd1, 0x517dbcce, 0x0b1c83ae,
    0x8bec94b1, 0x01440cad, 0x81b41bb2, 0xdbd524d2, 0x5b2533cd,
    0x6f175a12, 0xefe74d0d, 0xb586726d, 0x35766572, 0xdde2a1d3,
    0x5d12b6cc, 0x077389ac, 0x87839eb3, 0xb3b1f76c, 0x3341e073,
    0x6920df13, 0xe9d0c80c, 0xf4809ddf, 0x74708ac0, 0x2e11b5a0,
    0xaee1a2bf, 0x9ad3cb60, 0x1a23dc7f, 0x4042e31f, 0xc0b2f400,
    0x282630a1, 0xa8d627be, 0xf2b718de, 0x72470fc1, 0x4675661e,
    0xc6857101, 0x9ce44e61, 0x1c14597e, 0x96bcc162, 0x164cd67d,
    0x4c2de91d, 0xccddfe02, 0xf8ef97dd, 0x781f80c2, 0x227ebfa2,
    0xa28ea8bd, 0x4a1a6c1c, 0xcaea7b03, 0x908b4463, 0x107b537c,
    0x24493aa3, 0xa4b92dbc, 0xfed812dc, 0x7e2805c3, 0x30f824a5,
    0xb00833ba, 0xea690cda, 0x6a991bc5, 0x5eab721a, 0xde5b6505,
    0x843a5a65, 0x04ca4d7a, 0xec5e89db, 0x6cae9ec4, 0x36cfa1a4,
    0xb63fb6bb, 0x820ddf64, 0x02fdc87b, 0x589cf71b, 0xd86ce004,
    0x52c47818, 0xd2346f07, 0x88555067, 0x08a54778, 0x3c972ea7,
    0xbc6739b8, 0xe60606d8, 0x66f611c7, 0x8e62d566, 0x0e92c279,
    0x54f3fd19, 0xd403ea06, 0xe03183d9, 0x60c194c6, 0x3aa0aba6,
    0xba50bcb9},
   {0x00000000, 0x9570d495, 0xf190af6b, 0x64e07bfe, 0x38505897,
    0xad208c02, 0xc9c0f7fc, 0x5cb02369, 0x70a0b12e, 0xe5d065bb,
    0x81301e45, 0x1440cad0, 0x48f0e9b9, 0xdd803d2c, 0xb96046d2,
    0x2c109247, 0xe141625c, 0x7431b6c9, 0x10d1cd37, 0x85a119a2,
    0xd9113acb, 0x4c61ee5e, 0x288195a0, 0xbdf14135, 0x91e1d372,
    0x049107e7, 0x60717c19, 0xf501a88c, 0xa9b18be5, 0x3cc15f70,
    0x5821248e, 0xcd51f01b, 0x19f3c2f9, 0x8c83166c, 0xe8636d92,
    0x7d13b907, 0x21a39a6e, 0xb4d34efb, 0xd0333505, 0x4543e190,
    0x695373d7, 0xfc23a742, 0x98c3dcbc, 0x0db30829, 0x51032b40,
    0xc473ffd5, 0xa093842b, 0x35e350be, 0xf8b2a0a5, 0x6dc27430,
    0x09220fce, 0x9c52db5b, 0xc0e2f832, 0x55922ca7, 0x31725759,
    0xa40283cc, 0x8812118b, 0x1d62c51e, 0x7982bee0, 0xecf26a75,
    0xb042491c, 0x25329d89, 0x41d2e677, 0xd4a232e2, 0x33e785f2,
    0xa6975167, 0xc2772a99, 0x5707fe0c, 0x0bb7dd65, 0x9ec709f0,
    0xfa27720e, 0x6f57a69b, 0x434734dc, 0xd637e049, 0xb2d79bb7,
    0x27a74f22, 0x7b176c4b, 0xee67b8de, 0x8a87c320, 0x1ff717b5,
    0xd2a6e7ae, 0x47d6333b, 0x233648c5, 0xb6469c50, 0xeaf6bf39,
    0x7f866bac, 0x1b661052, 0x8e16c4c7, 0xa2065680, 0x37768215,
    0x5396f9eb, 0xc6e62d7e, 0x9a560e17, 0x0f26da82, 0x6bc6a17c,
    0xfeb675e9, 0x2a14470b, 0xbf64939e, 0xdb84e860, 0x4ef43cf5,
    0x12441f9c, 0x8734cb09, 0xe3d4b0f7, 0x76a46462, 0x5ab4f625,
    0xcfc422b0, 0xab24594e, 0x3e548ddb, 0x62e4aeb2, 0xf7947a27,
    0x937401d9, 0x0604d54c, 0xcb552557, 0x5e25f1c2, 0x3ac58a3c,
    0xafb55ea9, 0xf3057dc0, 0x6675a955, 0x0295d2ab, 0x97e5063e,
    0xbbf59479, 0x2e8540ec, 0x4a653b12, 0xdf15ef87, 0x83a5ccee,
    0x16d5187b, 0x72356385, 0xe745b710, 0x67cf0be4, 0xf2bfdf71,
    0x965fa48f, 0x032f701a, 0x5f9f5373, 0xcaef87e6, 0xae0ffc18,
    0x3b7f288d, 0x176fbaca, 0x821f6e5f, 0xe6ff15a1, 0x738fc134,
    0x2f3fe25d, 0xba4f36c8, 0xdeaf4d36, 0x4bdf99a3, 0x868e69b8,
    0x13febd2d, 0x771ec6d3, 0xe26e1246, 0xbede312f, 0x2baee5ba,
    0x4f4e9e44, 0xda3e4ad1, 0xf62ed896, 0x635e0c03, 0x07be77fd,
    0x92cea368, 0xce7e8001, 0x5b0e5494, 0x3fee2f6a, 0xaa9efbff,
    0x7e3cc91d, 0xeb4c1d88, 0x8fac6676, 0x1adcb2e3, 0x466c918a,
    0xd31c451f, 0xb7fc3ee1, 0x228cea74, 0x0e9c7833, 0x9becaca6,
    0xff0cd758, 0x6a7c03cd, 0x36cc20a4, 0xa3bcf431, 0xc75c8fcf,
    0x522c5b5a, 0x9f7dab41, 0x0a0d7fd4, 0x6eed042a, 0xfb9dd0bf,
    0xa72df3d6, 0x325d2743, 0x56bd5cbd, 0xc3cd8828, 0xefdd1a6f,
    0x7aadcefa, 0x1e4db504, 0x8b3d6191, 0xd78d42f8, 0x42fd966d,
    0x261ded93, 0xb36d3906, 0x54288e16, 0xc1585a83, 0xa5b8217d,
    0x30c8f5e8, 0x6c78d681, 0xf9080214, 0x9de879ea, 0x0898ad7f,
    0x24883f38, 0xb1f8ebad, 0xd5189053, 0x406844c6, 0x1cd867af,
    0x89a8b33a, 0xed48c8c4, 0x78381c51, 0xb569ec4a, 0x201938df,
    0x44f94321, 0xd18997b4, 0x8d39b4dd, 0x18496048, 0x7ca91bb6,
    0xe9d9cf23, 0xc5c95d64, 0x50b989f1, 0x3459f20f, 0xa129269a,
    0xfd9905f3, 0x68e9d166, 0x0c09aa98, 0x99797e0d, 0x4ddb4cef,
    0xd8ab987a, 0xbc4be384, 0x293b3711, 0x758b1478, 0xe0fbc0ed,
    0x841bbb13, 0x116b6f86, 0x3d7bfdc1, 0xa80b2954, 0xcceb52aa,
    0x599b863f, 0x052ba556, 0x905b71c3, 0xf4bb0a3d, 0x61cbdea8,
    0xac9a2eb3, 0x39eafa26, 0x5d0a81d8, 0xc87a554d, 0x94ca7624,
    0x01baa2b1, 0x655ad94f, 0xf02a0dda, 0xdc3a9f9d, 0x494a4b08,
    0x2daa30f6, 0xb8dae463, 0xe46ac70a, 0x711a139f, 0x15fa6861,
    0x808abcf4},
   {0x00000000, 0xcf9e17c8, 0x444d29d1, 0x8bd33e19, 0x889a53a2,
    0x4704446a, 0xccd77a73, 0x03496dbb, 0xca45a105, 0x05dbb6cd,
    0x8e0888d4, 0x41969f1c, 0x42dff2a7, 0x8d41e56f, 0x0692db76,
    0xc90cccbe, 0x4ffa444b, 0x80645383, 0x0bb76d9a, 0xc4297a52,
    0xc76017e9, 0x08fe0021, 0x832d3e38, 0x4cb329f0, 0x85bfe54e,
    0x4a21f286, 0xc1f2cc9f, 0x0e6cdb57, 0x0d25b6ec, 0xc2bba124,
    0x49689f3d, 0x86f688f5, 0x9ff48896, 0x506a9f5e, 0xdbb9a147,
    0x1427b68f, 0x176edb34, 0xd8f0ccfc, 0x5323f2e5, 0x9cbde52d,
    0x55b12993, 0x9a2f3e5b, 0x11fc0042, 0xde62178a, 0xdd2b7a31,
    0x12b56df9, 0x996653e0, 0x56f84428, 0xd00eccdd, 0x1f90db15,
    0x9443e50c, 0x5bddf2c4, 0x58949f7f, 0x970a88b7, 0x1cd9b6ae,
    0xd347a166, 0x1a4b6dd8, 0xd5d57a10, 0x5e064409, 0x919853c1,
    0x92d13e7a, 0x5d4f29b2, 0xd69c17ab, 0x19020063, 0xe498176d,
    0x2b0600a5, 0xa0d53ebc, 0x6f4b2974, 0x6c0244cf, 0xa39c5307,
    0x284f6d1e, 0xe7d17ad6, 0x2eddb668, 0xe143a1a0, 0x6a909fb9,
    0xa50e8871, 0xa647e5ca, 0x69d9f202, 0xe20acc1b, 0x2d94dbd3,
    0xab625326, 0x64fc44ee, 0xef2f7af7, 0x20b16d3f, 0x23f80084,
    0xec66174c, 0x67b52955, 0xa82b3e9d, 0x6127f223, 0xaeb9e5eb,
    0x256adbf2, 0xeaf4cc3a, 0xe9bda181, 0x2623b649, 0xadf08850,
    0x626e9f98, 0x7b6c9ffb, 0xb4f28833, 0x3f21b62a, 0xf0bfa1e2,
    0xf3f6cc59, 0x3c68db91, 0xb7bbe588, 0x7825f240, 0xb1293efe,
    0x7eb72936, 0xf564172f, 0x3afa00e7, 0x39b36d5c, 0xf62d7a94,
    0x7dfe448d, 0xb2605345, 0x3496dbb0, 0xfb08cc78, 0x70dbf261,
    0xbf45e5a9, 0xbc0c8812, 0x73929fda, 0xf841a1c3, 0x37dfb60b,
    0xfed37ab5, 0x314d6d7d, 0xba9e5364, 0x750044ac, 0x76492917,
    0xb9d73edf, 0x320400c6, 0xfd9a170e, 0x1241289b, 0xdddf3f53,
    0x560c014a, 0x99921682, 0x9adb7b39, 0x55456cf1, 0xde9652e8,
    0x11084520, 0xd804899e, 0x179a9e56, 0x9c49a04f, 0x53d7b787,
    0x509eda3c, 0x9f00cdf4, 0x14d3f3ed, 0xdb4de425, 0x5dbb6cd0,
    0x92257b18, 0x19f64501, 0xd66852c9, 0xd5213f72, 0x1abf28ba,
    0x916c16a3, 0x5ef2016b, 0x97fecdd5, 0x5860da1d, 0xd3b3e404,
    0x1c2df3cc, 0x1f649e77, 0xd0fa89bf, 0x5b29b7a6, 0x94b7a06e,
    0x8db5a00d, 0x422bb7c5, 0xc9f889dc, 0x06669e14, 0x052ff3af,
    0xcab1e467, 0x4162da7e, 0x8efccdb6, 0x47f00108, 0x886e16c0,
    0x03bd28d9, 0xcc233f11, 0xcf6a52aa, 0x00f44562, 0x8b277b7b,
    0x44b96cb3, 0xc24fe446, 0x0dd1f38e, 0x8602cd97, 0x499cda5f,
    0x4ad5b7e4, 0x854ba02c, 0x0e989e35, 0xc10689fd, 0x080a4543,
    0xc794528b, 0x4c476c92, 0x83d97b5a, 0x809016e1, 0x4f0e0129,
    0xc4dd3f30, 0x0b4328f8, 0xf6d93ff6, 0x3947283e, 0xb2941627,
    0x7d0a01ef, 0x7e436c54, 0xb1dd7b9c, 0x3a0e4585, 0xf590524d,
    0x3c9c9ef3, 0xf302893b, 0x78d1b722, 0xb74fa0ea, 0xb406cd51,
    0x7b98da99, 0xf04be480, 0x3fd5f348, 0xb9237bbd, 0x76bd6c75,
    0xfd6e526c, 0x32f045a4, 0x31b9281f, 0xfe273fd7, 0x75f401ce,
    0xba6a1606, 0x7366dab8, 0xbcf8cd70, 0x372bf369, 0xf8b5e4a1,
    0xfbfc891a, 0x34629ed2, 0xbfb1a0cb, 0x702fb703, 0x692db760,
    0xa6b3a0a8, 0x2d609eb1, 0xe2fe8979, 0xe1b7e4c2, 0x2e29f30a,
    0xa5facd13, 0x6a64dadb, 0xa3681665, 0x6cf601ad, 0xe7253fb4,
    0x28bb287c, 0x2bf245c7, 0xe46c520f, 0x6fbf6c16, 0xa0217bde,
    0x26d7f32b, 0xe949e4e3, 0x629adafa, 0xad04cd32, 0xae4da089,
    0x61d3b741, 0xea008958, 0x259e9e90, 0xec92522e, 0x230c45e6,
    0xa8df7bff, 0x67416c37, 0x6408018c, 0xab961644, 0x2045285d,
    0xefdb3f95},
   {0x00000000, 0x24825136, 0x4904a26c, 0x6d86f35a, 0x920944d8,
    0xb68b15ee, 0xdb0de6b4, 0xff8fb782, 0xff638ff1, 0xdbe1dec7,
    0xb6672d9d, 0x92e57cab, 0x6d6acb29, 0x49e89a1f, 0x246e6945,
    0x00ec3873, 0x25b619a3, 0x01344895, 0x6cb2bbcf, 0x4830eaf9,
    0xb7bf5d7b, 0x933d0c4d, 0xfebbff17, 0xda39ae21, 0xdad59652,
    0xfe57c764, 0x93d1343e, 0xb7536508, 0x48dcd28a, 0x6c5e83bc,
    0x01d870e6, 0x255a21d0, 0x4b6c3346, 0x6fee6270, 0x0268912a,
    0x26eac01c, 0xd965779e, 0xfde726a8, 0x9061d5f2, 0xb4e384c4,
    0xb40fbcb7, 0x908ded81, 0xfd0b1edb, 0xd9894fed, 0x2606f86f,
    0x0284a959, 0x6f025a03, 0x4b800b35, 0x6eda2ae5, 0x4a587bd3,
    0x27de8889, 0x035cd9bf, 0xfcd36e3d, 0xd8513f0b, 0xb5d7cc51,
    0x91559d67, 0x91b9a514, 0xb53bf422, 0xd8bd0778, 0xfc3f564e,
    0x03b0e1cc, 0x2732b0fa, 0x4ab443a0, 0x6e361296, 0x96d8668c,
    0xb25a37ba, 0xdfdcc4e0, 0xfb5e95d6, 0x04d12254, 0x20537362,
    0x4dd58038, 0x6957d10e, 0x69bbe97d, 0x4d39b84b, 0x20bf4b11,
    0x043d1a27, 0xfbb2ada5, 0xdf30fc93, 0xb2b60fc9, 0x96345eff,
    0xb36e7f2f, 0x97ec2e19, 0xfa6add43, 0xdee88c75, 0x21673bf7,
    0x05e56ac1, 0x6863999b, 0x4ce1c8ad, 0x4c0df0de, 0x688fa1e8,
    0x050952b2, 0x218b0384, 0xde04b406, 0xfa86e530, 0x9700166a,
    0xb382475c, 0xddb455ca, 0xf93604fc, 0x94b0f7a6, 0xb032a690,
    0x4fbd1112, 0x6b3f4024, 0x06b9b37e, 0x223be248, 0x22d7da3b,
    0x06558b0d, 0x6bd37857, 0x4f512961, 0xb0de9ee3, 0x945ccfd5,
    0xf9da3c8f, 0xdd586db9, 0xf8024c69, 0xdc801d5f, 0xb106ee05,
    0x9584bf33, 0x6a0b08b1, 0x4e895987, 0x230faadd, 0x078dfbeb,
    0x0761c398, 0x23e392ae, 0x4e6561f4, 0x6ae730c2, 0x95688740,
    0xb1ead676, 0xdc6c252c, 0xf8ee741a, 0xf6c1cb59, 0xd2439a6f,
    0xbfc56935, 0x9b473803, 0x64c88f81, 0x404adeb7, 0x2dcc2ded,
    0x094e7cdb, 0x09a244a8, 0x2d20159e, 0x40a6e6c4, 0x6424b7f2,
    0x9bab0070, 0xbf295146, 0xd2afa21c, 0xf62df32a, 0xd377d2fa,
    0xf7f583cc, 0x9a737096, 0xbef121a0, 0x417e9622, 0x65fcc714,
    0x087a344e, 0x2cf86578, 0x2c145d0b, 0x08960c3d, 0x6510ff67,
    0x4192ae51, 0xbe1d19d3, 0x9a9f48e5, 0xf719bbbf, 0xd39bea89,
    0xbdadf81f, 0x992fa929, 0xf4a95a73, 0xd02b0b45, 0x2fa4bcc7,
    0x0b26edf1, 0x66a01eab, 0x42224f9d, 0x42ce77ee, 0x664c26d8,
    0x0bcad582, 0x2f4884b4, 0xd0c73336, 0xf4456200, 0x99c3915a,
    0xbd41c06c, 0x981be1bc, 0xbc99b08a, 0xd11f43d0, 0xf59d12e6,
    0x0a12a564, 0x2e90f452, 0x43160708, 0x6794563e, 0x67786e4d,
    0x43fa3f7b, 0x2e7ccc21, 0x0afe9d17, 0xf5712a95, 0xd1f37ba3,
    0xbc7588f9, 0x98f7d9cf, 0x6019add5, 0x449bfce3, 0x291d0fb9,
    0x0d9f5e8f, 0xf210e90d, 0xd692b83b, 0xbb144b61, 0x9f961a57,
    0x9f7a2224, 0xbbf87312, 0xd67e8048, 0xf2fcd17e, 0x0d7366fc,
    0x29f137ca, 0x4477c490, 0x60f595a6, 0x45afb476, 0x612de540,
    0x0cab161a, 0x2829472c, 0xd7a6f0ae, 0xf324a198, 0x9ea252c2,
    0xba2003f4, 0xbacc3b87, 0x9e4e6ab1, 0xf3c899eb, 0xd74ac8dd,
    0x28c57f5f, 0x0c472e69, 0x61c1dd33, 0x45438c05, 0x2b759e93,
    0x0ff7cfa5, 0x62713cff, 0x46f36dc9, 0xb97cda4b, 0x9dfe8b7d,
    0xf0787827, 0xd4fa2911, 0xd4161162, 0xf0944054, 0x9d12b30e,
    0xb990e238, 0x461f55ba, 0x629d048c, 0x0f1bf7d6, 0x2b99a6e0,
    0x0ec38730, 0x2a41d606, 0x47c7255c, 0x6345746a, 0x9ccac3e8,
    0xb84892de, 0xd5ce6184, 0xf14c30b2, 0xf1a008c1, 0xd52259f7,
    0xb8a4aaad, 0x9c26fb9b, 0x63a94c19, 0x472b1d2f, 0x2aadee75,
    0x0e2fbf43},
   {0x00000000, 0x36f290f3, 0x6de521e6, 0x5b17b115, 0xdbca43cc,
    0xed38d33f, 0xb62f622a, 0x80ddf2d9, 0x6ce581d9, 0x5a17112a,
    0x0100a03f, 0x37f230cc, 0xb72fc215, 0x81dd52e6, 0xdacae3f3,
    0xec387300, 0xd9cb03b2, 0xef399341, 0xb42e2254, 0x82dcb2a7,
    0x0201407e, 0x34f3d08d, 0x6fe46198, 0x5916f16b, 0xb52e826b,
    0x83dc1298, 0xd8cba38d, 0xee39337e, 0x6ee4c1a7, 0x58165154,
    0x0301e041, 0x35f370b2, 0x68e70125, 0x5e1591d6, 0x050220c3,
    0x33f0b030, 0xb32d42e9, 0x85dfd21a, 0xdec8630f, 0xe83af3fc,
    0x040280fc, 0x32f0100f, 0x69e7a11a, 0x5f1531e9, 0xdfc8c330,
    0xe93a53c3, 0xb22de2d6, 0x84df7225, 0xb12c0297, 0x87de9264,
    0xdcc92371, 0xea3bb382, 0x6ae6415b, 0x5c14d1a8, 0x070360bd,
    0x31f1f04e, 0xddc9834e, 0xeb3b13bd, 0xb02ca2a8, 0x86de325b,
    0x0603c082, 0x30f15071, 0x6be6e164, 0x5d147197, 0xd1ce024a,
    0xe73c92b9, 0xbc2b23ac, 0x8ad9b35f, 0x0a044186, 0x3cf6d175,
    0x67e16060, 0x5113f093, 0xbd2b8393, 0x8bd91360, 0xd0cea275,
    0xe63c3286, 0x66e1c05f, 0x501350ac, 0x0b04e1b9, 0x3df6714a,
    0x080501f8, 0x3ef7910b, 0x65e0201e, 0x5312b0ed, 0xd3cf4234,
    0xe53dd2c7, 0xbe2a63d2, 0x88d8f321, 0x64e08021, 0x521210d2,
    0x0905a1c7, 0x3ff73134, 0xbf2ac3ed, 0x89d8531e, 0xd2cfe20b,
    0xe43d72f8, 0xb929036f, 0x8fdb939c, 0xd4cc2289, 0xe23eb27a,
    0x62e340a3, 0x5411d050, 0x0f066145, 0x39f4f1b6, 0xd5cc82b6,
    0xe33e1245, 0xb829a350, 0x8edb33a3, 0x0e06c17a, 0x38f45189,
    0x63e3e09c, 0x5511706f, 0x60e200dd, 0x5610902e, 0x0d07213b,
    0x3bf5b1c8, 0xbb284311, 0x8ddad3e2, 0xd6cd62f7, 0xe03ff204,
    0x0c078104, 0x3af511f7, 0x61e2a0e2, 0x57103011, 0xd7cdc2c8,
    0xe13f523b, 0xba28e32e, 0x8cda73dd, 0x78ed02d5, 0x4e1f9226,
    0x15082333, 0x23fab3c0, 0xa3274119, 0x95d5d1ea, 0xcec260ff,
    0xf830f00c, 0x1408830c, 0x22fa13ff, 0x79eda2ea, 0x4f1f3219,
    0xcfc2c0c0, 0xf9305033, 0xa227e126, 0x94d571d5, 0xa1260167,
    0x97d49194, 0xccc32081, 0xfa31b072, 0x7aec42ab, 0x4c1ed258,
    0x1709634d, 0x21fbf3be, 0xcdc380be, 0xfb31104d, 0xa026a158,
    0x96d431ab, 0x1609c372, 0x20fb5381, 0x7bece294, 0x4d1e7267,
    0x100a03f0, 0x26f89303, 0x7def2216, 0x4b1db2e5, 0xcbc0403c,
    0xfd32d0cf, 0xa62561da, 0x90d7f129, 0x7cef8229, 0x4a1d12da,
    0x110aa3cf, 0x27f8333c, 0xa725c1e5, 0x91d75116, 0xcac0e003,
    0xfc3270f0, 0xc9c10042, 0xff3390b1, 0xa42421a4, 0x92d6b157,
    0x120b438e, 0x24f9d37d, 0x7fee6268, 0x491cf29b, 0xa524819b,
    0x93d61168, 0xc8c1a07d, 0xfe33308e, 0x7eeec257, 0x481c52a4,
    0x130be3b1, 0x25f97342, 0xa923009f, 0x9fd1906c, 0xc4c62179,
    0xf234b18a, 0x72e94353, 0x441bd3a0, 0x1f0c62b5, 0x29fef246,
    0xc5c68146, 0xf33411b5, 0xa823a0a0, 0x9ed13053, 0x1e0cc28a,
    0x28fe5279, 0x73e9e36c, 0x451b739f, 0x70e8032d, 0x461a93de,
    0x1d0d22cb, 0x2bffb238, 0xab2240e1, 0x9dd0d012, 0xc6c76107,
    0xf035f1f4, 0x1c0d82f4, 0x2aff1207, 0x71e8a312, 0x471a33e1,
    0xc7c7c138, 0xf13551cb, 0xaa22e0de, 0x9cd0702d, 0xc1c401ba,
    0xf7369149, 0xac21205c, 0x9ad3b0af, 0x1a0e4276, 0x2cfcd285,
    0x77eb6390, 0x4119f363, 0xad218063, 0x9bd31090, 0xc0c4a185,
    0xf6363176, 0x76ebc3af, 0x4019535c, 0x1b0ee249, 0x2dfc72ba,
    0x180f0208, 0x2efd92fb, 0x75ea23ee, 0x4318b31d, 0xc3c541c4,
    0xf537d137, 0xae206022, 0x98d2f0d1, 0x74ea83d1, 0x42181322,
    0x190fa237, 0x2ffd32c4, 0xaf20c01d, 0x99d250ee, 0xc2c5e1fb,
    0xf4377108}};

local const z_word_t FAR crc_braid_big_table[][256] = {
   {0x0000000000000000, 0xf390f23600000000, 0xe621e56d00000000,
    0x15b1175b00000000, 0xcc43cadb00000000, 0x3fd338ed00000000,
    0x2a622fb600000000, 0xd9f2dd8000000000, 0xd981e56c00000000,
    0x2a11175a00000000, 0x3fa0000100000000, 0xcc30f23700000000,
    0x15c22fb700000000, 0xe652dd8100000000, 0xf3e3cada00000000,
    0x007338ec00000000, 0xb203cbd900000000, 0x419339ef00000000,
    0x54222eb400000000, 0xa7b2dc8200000000, 0x7e40010200000000,
    0x8dd0f33400000000, 0x9861e46f00000000, 0x6bf1165900000000,
    0x6b822eb500000000, 0x9812dc8300000000, 0x8da3cbd800000000,
    0x7e3339ee00000000, 0xa7c1e46e00000000, 0x5451165800000000,
    0x41e0010300000000, 0xb270f33500000000, 0x2501e76800000000,
    0xd691155e00000000, 0xc320020500000000, 0x30b0f03300000000,
    0xe9422db300000000, 0x1ad2df8500000000, 0x0f63c8de00000000,
    0xfcf33ae800000000, 0xfc80020400000000, 0x0f10f03200000000,
    0x1aa1e76900000000, 0xe931155f00000000, 0x30c3c8df00000000,
    0xc3533ae900000000, 0xd6e22db200000000, 0x2572df8400000000,
    0x97022cb100000000, 0x6492de8700000000, 0x7123c9dc00000000,
    0x82b33bea00000000, 0x5b41e66a00000000, 0xa8d1145c00000000,
    0xbd60030700000000, 0x4ef0f13100000000, 0x4e83c9dd00000000,
    0xbd133beb00000000, 0xa8a22cb000000000, 0x5b32de8600000000,
    0x82c0030600000000, 0x7150f13000000000, 0x64e1e66b00000000,
    0x9771145d00000000, 0x4a02ced100000000, 0xb9923ce700000000,
    0xac232bbc00000000, 0x5fb3d98a00000000, 0x8641040a00000000,
    0x75d1f63c00000000, 0x6060e16700000000, 0x93f0135100000000,
    0x93832bbd00000000, 0x6013d98b00000000, 0x75a2ced000000000,
    0x86323ce600000000, 0x5fc0e16600000000, 0xac50135000000000,
    0xb9e1040b00000000, 0x4a71f63d00000000, 0xf801050800000000,
    0x0b91f73e00000000, 0x1e20e06500000000, 0xedb0125300000000,
    0x3442cfd300000000, 0xc7d23de500000000, 0xd2632abe00000000,
    0x21f3d88800000000, 0x2180e06400000000, 0xd210125200000000,
    0xc7a1050900000000, 0x3431f73f00000000, 0xedc32abf00000000,
    0x1e53d88900000000, 0x0be2cfd200000000, 0xf8723de400000000,
    0x6f0329b900000000, 0x9c93db8f00000000, 0x8922ccd400000000,
    0x7ab23ee200000000, 0xa340e36200000000, 0x50d0115400000000,
    0x4561060f00000000, 0xb6f1f43900000000, 0xb682ccd500000000,
    0x45123ee300000000, 0x50a329b800000000, 0xa333db8e00000000,
    0x7ac1060e00000000, 0x8951f43800000000, 0x9ce0e36300000000,
    0x6f70115500000000, 0xdd00e26000000000, 0x2e90105600000000,
    0x3b21070d00000000, 0xc8b1f53b00000000, 0x114328bb00000000,
    0xe2d3da8d00000000, 0xf762cdd600000000, 0x04f23fe000000000,
    0x0481070c00000000, 0xf711f53a00000000, 0xe2a0e26100000000,
    0x1130105700000000, 0xc8c2cdd700000000, 0x3b523fe100000000,
    0x2ee328ba00000000, 0xdd73da8c00000000, 0xd502ed7800000000,
    0x26921f4e00000000, 0x3323081500000000, 0xc0b3fa2300000000,
    0x194127a300000000, 0xead1d59500000000, 0xff60c2ce00000000,
    0x0cf030f800000000, 0x0c83081400000000, 0xff13fa2200000000,
    0xeaa2ed7900000000, 0x19321f4f00000000, 0xc0c0c2cf00000000,
    0x335030f900000000, 0x26e127a200000000, 0xd571d59400000000,
    0x670126a100000000, 0x9491d49700000000, 0x8120c3cc00000000,
    0x72b031fa00000000, 0xab42ec7a00000000, 0x58d21e4c00000000,
    0x4d63091700000000, 0xbef3fb2100000000, 0xbe80c3cd00000000,
    0x4d1031fb00000000, 0x58a126a000000000, 0xab31d49600000000,
    0x72c3091600000000, 0x8153fb2000000000, 0x94e2ec7b00000000,
    0x67721e4d00000000, 0xf0030a1000000000, 0x0393f82600000000,
    0x1622ef7d00000000, 0xe5b21d4b00000000, 0x3c40c0cb00000000,
    0xcfd032fd00000000, 0xda6125a600000000, 0x29f1d79000000000,
    0x2982ef7c00000000, 0xda121d4a00000000, 0xcfa30a1100000000,
    0x3c33f82700000000, 0xe5c125a700000000, 0x1651d79100000000,
    0x03e0c0ca00000000, 0xf07032fc00000000, 0x4200c1c900000000,
    0xb19033ff00000000, 0xa42124a400000000, 0x57b1d69200000000,
    0x8e430b1200000000, 0x7dd3f92400000000, 0x6862ee7f00000000,
    0x9bf21c4900000000, 0x9b8124a500000000, 0x6811d69300000000,
    0x7da0c1c800000000, 0x8e3033fe00000000, 0x57c2ee7e00000000,
    0xa4521c4800000000, 0xb1e30b1300000000, 0x4273f92500000000,
    0x9f0023a900000000, 0x6c90d19f00000000, 0x7921c6c400000000,
    0x8ab134f200000000, 0x5343e97200000000, 0xa0d31b4400000000,
    0xb5620c1f00000000, 0x46f2fe2900000000, 0x4681c6c500000000,
    0xb51134f300000000, 0xa0a023a800000000, 0x5330d19e00000000,
    0x8ac20c1e00000000, 0x7952fe2800000000, 0x6ce3e97300000000,
    0x9f731b4500000000, 0x2d03e87000000000, 0xde931a4600000000,
    0xcb220d1d00000000, 0x38b2ff2b00000000, 0xe14022ab00000000,
    0x12d0d09d00000000, 0x0761c7c600000000, 0xf4f135f000000000,
    0xf4820d1c00000000, 0x0712ff2a00000000, 0x12a3e87100000000,
    0xe1331a4700000000, 0x38c1c7c700000000, 0xcb5135f100000000,
    0xdee022aa00000000, 0x2d70d09c00000000, 0xba01c4c100000000,
    0x499136f700000000, 0x5c2021ac00000000, 0xafb0d39a00000000,
    0x76420e1a00000000, 0x85d2fc2c00000000, 0x9063eb7700000000,
    0x63f3194100000000, 0x638021ad00000000, 0x9010d39b00000000,
    0x85a1c4c000000000, 0x763136f600000000, 0xafc3eb7600000000,
    0x5c53194000000000, 0x49e20e1b00000000, 0xba72fc2d00000000,
    0x08020f1800000000, 0xfb92fd2e00000000, 0xee23ea7500000000,
    0x1db3184300000000, 0xc441c5c300000000, 0x37d137f500000000,
    0x226020ae00000000, 0xd1f0d29800000000, 0xd183ea7400000000,
    0x2213184200000000, 0x37a20f1900000000, 0xc432fd2f00000000,
    0x1dc020af00000000, 0xee50d29900000000, 0xfbe1c5c200000000,
    0x087137f400000000},
   {0x0000000000000000, 0x3651822400000000, 0x6ca2044900000000,
    0x5af3866d00000000, 0xd844099200000000, 0xee158bb600000000,
    0xb4e60ddb00000000, 0x82b78fff00000000, 0xf18f63ff00000000,
    0xc7dee1db00000000, 0x9d2d67b600000000, 0xab7ce59200000000,
    0x29cb6a6d00000000, 0x1f9ae84900000000, 0x45696e2400000000,
    0x7338ec0000000000, 0xa319b62500000000, 0x9548340100000000,
    0xcfbbb26c00000000, 0xf9ea304800000000, 0x7b5dbfb700000000,
    0x4d0c3d9300000000, 0x17ffbbfe00000000, 0x21ae39da00000000,
    0x5296d5da00000000, 0x64c757fe00000000, 0x3e34d19300000000,
    0x086553b700000000, 0x8ad2dc4800000000, 0xbc835e6c00000000,
    0xe670d80100000000, 0xd0215a2500000000, 0x46336c4b00000000,
    0x7062ee6f00000000, 0x2a91680200000000, 0x1cc0ea2600000000,
    0x9e7765d900000000, 0xa826e7fd00000000, 0xf2d5619000000000,
    0xc484e3b400000000, 0xb7bc0fb400000000, 0x81ed8d9000000000,
    0xdb1e0bfd00000000, 0xed4f89d900000000, 0x6ff8062600000000,
    0x59a9840200000000, 0x035a026f00000000, 0x350b804b00000000,
    0xe52ada6e00000000, 0xd37b584a00000000, 0x8988de2700000000,
    0xbfd95c0300000000, 0x3d6ed3fc00000000, 0x0b3f51d800000000,
    0x51ccd7b500000000, 0x679d559100000000, 0x14a5b99100000000,
    0x22f43bb500000000, 0x7807bdd800000000, 0x4e563ffc00000000,
    0xcce1b00300000000, 0xfab0322700000000, 0xa043b44a00000000,
    0x9612366e00000000, 0x8c66d89600000000, 0xba375ab200000000,
    0xe0c4dcdf00000000, 0xd6955efb00000000, 0x5422d10400000000,
    0x6273532000000000, 0x3880d54d00000000, 0x0ed1576900000000,
    0x7de9bb6900000000, 0x4bb8394d00000000, 0x114bbf2000000000,
    0x271a3d0400000000, 0xa5adb2fb00000000, 0x93fc30df00000000,
    0xc90fb6b200000000, 0xff5e349600000000, 0x2f7f6eb300000000,
    0x192eec9700000000, 0x43dd6afa00000000, 0x758ce8de00000000,
    0xf73b672100000000, 0xc16ae50500000000, 0x9b99636800000000,
    0xadc8e14c00000000, 0xdef00d4c00000000, 0xe8a18f6800000000,
    0xb252090500000000, 0x84038b2100000000, 0x06b404de00000000,
    0x30e586fa00000000, 0x6a16009700000000, 0x5c4782b300000000,
    0xca55b4dd00000000, 0xfc0436f900000000, 0xa6f7b09400000000,
    0x90a632b000000000, 0x1211bd4f00000000, 0x24403f6b00000000,
    0x7eb3b90600000000, 0x48e23b2200000000, 0x3bdad72200000000,
    0x0d8b550600000000, 0x5778d36b00000000, 0x6129514f00000000,
    0xe39edeb000000000, 0xd5cf5c9400000000, 0x8f3cdaf900000000,
    0xb96d58dd00000000, 0x694c02f800000000, 0x5f1d80dc00000000,
    0x05ee06b100000000, 0x33bf849500000000, 0xb1080b6a00000000,
    0x8759894e00000000, 0xddaa0f2300000000, 0xebfb8d0700000000,
    0x98c3610700000000, 0xae92e32300000000, 0xf461654e00000000,
    0xc230e76a00000000, 0x4087689500000000, 0x76d6eab100000000,
    0x2c256cdc00000000, 0x1a74eef800000000, 0x59cbc1f600000000,
    0x6f9a43d200000000, 0x3569c5bf00000000, 0x0338479b00000000,
    0x818fc86400000000, 0xb7de4a4000000000, 0xed2dcc2d00000000,
    0xdb7c4e0900000000, 0xa844a20900000000, 0x9e15202d00000000,
    0xc4e6a64000000000, 0xf2b7246400000000, 0x7000ab9b00000000,
    0x465129bf00000000, 0x1ca2afd200000000, 0x2af32df600000000,
    0xfad277d300000000, 0xcc83f5f700000000, 0x9670739a00000000,
    0xa021f1be00000000, 0x22967e4100000000, 0x14c7fc6500000000,
    0x4e347a0800000000, 0x7865f82c00000000, 0x0b5d142c00000000,
    0x3d0c960800000000, 0x67ff106500000000, 0x51ae924100000000,
    0xd3191dbe00000000, 0xe5489f9a00000000, 0xbfbb19f700000000,
    0x89ea9bd300000000, 0x1ff8adbd00000000, 0x29a92f9900000000,
    0x735aa9f400000000, 0x450b2bd000000000, 0xc7bca42f00000000,
    0xf1ed260b00000000, 0xab1ea06600000000, 0x9d4f224200000000,
    0xee77ce4200000000, 0xd8264c6600000000, 0x82d5ca0b00000000,
    0xb484482f00000000, 0x3633c7d000000000, 0x006245f400000000,
    0x5a91c39900000000, 0x6cc041bd00000000, 0xbce11b9800000000,
    0x8ab099bc00000000, 0xd0431fd100000000, 0xe6129df500000000,
    0x64a5120a00000000, 0x52f4902e00000000, 0x0807164300000000,
    0x3e56946700000000, 0x4d6e786700000000, 0x7b3ffa4300000000,
    0x21cc7c2e00000000, 0x179dfe0a00000000, 0x952a71f500000000,
    0xa37bf3d100000000, 0xf98875bc00000000, 0xcfd9f79800000000,
    0xd5ad196000000000, 0xe3fc9b4400000000, 0xb90f1d2900000000,
    0x8f5e9f0d00000000, 0x0de910f200000000, 0x3bb892d600000000,
    0x614b14bb00000000, 0x571a969f00000000, 0x24227a9f00000000,
    0x1273f8bb00000000, 0x48807ed600000000, 0x7ed1fcf200000000,
    0xfc66730d00000000, 0xca37f12900000000, 0x90c4774400000000,
    0xa695f56000000000, 0x76b4af4500000000, 0x40e52d6100000000,
    0x1a16ab0c00000000, 0x2c47292800000000, 0xaef0a6d700000000,
    0x98a124f300000000, 0xc252a29e00000000, 0xf40320ba00000000,
    0x873bccba00000000, 0xb16a4e9e00000000, 0xeb99c8f300000000,
    0xddc84ad700000000, 0x5f7fc52800000000, 0x692e470c00000000,
    0x33ddc16100000000, 0x058c434500000000, 0x939e752b00000000,
    0xa5cff70f00000000, 0xff3c716200000000, 0xc96df34600000000,
    0x4bda7cb900000000, 0x7d8bfe9d00000000, 0x277878f000000000,
    0x1129fad400000000, 0x621116d400000000, 0x544094f000000000,
    0x0eb3129d00000000, 0x38e290b900000000, 0xba551f4600000000,
    0x8c049d6200000000, 0xd6f71b0f00000000, 0xe0a6992b00000000,
    0x3087c30e00000000, 0x06d6412a00000000, 0x5c25c74700000000,
    0x6a74456300000000, 0xe8c3ca9c00000000, 0xde9248b800000000,
    0x8461ced500000000, 0xb2304cf100000000, 0xc108a0f100000000,
    0xf75922d500000000, 0xadaaa4b800000000, 0x9bfb269c00000000,
    0x194ca96300000000, 0x2f1d2b4700000000, 0x75eead2a00000000,
    0x43bf2f0e00000000},
   {0x0000000000000000, 0xc8179ecf00000000, 0xd1294d4400000000,
    0x193ed38b00000000, 0xa2539a8800000000, 0x6a44044700000000,
    0x737ad7cc00000000, 0xbb6d490300000000, 0x05a145ca00000000,
    0xcdb6db0500000000, 0xd488088e00000000, 0x1c9f964100000000,
    0xa7f2df4200000000, 0x6fe5418d00000000, 0x76db920600000000,
    0xbecc0cc900000000, 0x4b44fa4f00000000, 0x8353648000000000,
    0x9a6db70b00000000, 0x527a29c400000000, 0xe91760c700000000,
    0x2100fe0800000000, 0x383e2d8300000000, 0xf029b34c00000000,
    0x4ee5bf8500000000, 0x86f2214a00000000, 0x9fccf2c100000000,
    0x57db6c0e00000000, 0xecb6250d00000000, 0x24a1bbc200000000,
    0x3d9f684900000000, 0xf588f68600000000, 0x9688f49f00000000,
    0x5e9f6a5000000000, 0x47a1b9db00000000, 0x8fb6271400000000,
    0x34db6e1700000000, 0xfcccf0d800000000, 0xe5f2235300000000,
    0x2de5bd9c00000000, 0x9329b15500000000, 0x5b3e2f9a00000000,
    0x4200fc1100000000, 0x8a1762de00000000, 0x317a2bdd00000000,
    0xf96db51200000000, 0xe053669900000000, 0x2844f85600000000,
    0xddcc0ed000000000, 0x15db901f00000000, 0x0ce5439400000000,
    0xc4f2dd5b00000000, 0x7f9f945800000000, 0xb7880a9700000000,
    0xaeb6d91c00000000, 0x66a147d300000000, 0xd86d4b1a00000000,
    0x107ad5d500000000, 0x0944065e00000000, 0xc153989100000000,
    0x7a3ed19200000000, 0xb2294f5d00000000, 0xab179cd600000000,
    0x6300021900000000, 0x6d1798e400000000, 0xa500062b00000000,
    0xbc3ed5a000000000, 0x74294b6f00000000, 0xcf44026c00000000,
    0x07539ca300000000, 0x1e6d4f2800000000, 0xd67ad1e700000000,
    0x68b6dd2e00000000, 0xa0a143e100000000, 0xb99f906a00000000,
    0x71880ea500000000, 0xcae547a600000000, 0x02f2d96900000000,
    0x1bcc0ae200000000, 0xd3db942d00000000, 0x265362ab00000000,
    0xee44fc6400000000, 0xf77a2fef00000000, 0x3f6db12000000000,
    0x8400f82300000000, 0x4c1766ec00000000, 0x5529b56700000000,
    0x9d3e2ba800000000, 0x23f2276100000000, 0xebe5b9ae00000000,
    0xf2db6a2500000000, 0x3accf4ea00000000, 0x81a1bde900000000,
    0x49b6232600000000, 0x5088f0ad00000000, 0x989f6e6200000000,
    0xfb9f6c7b00000000, 0x3388f2b400000000, 0x2ab6213f00000000,
    0xe2a1bff000000000, 0x59ccf6f300000000, 0x91db683c00000000,
    0x88e5bbb700000000, 0x40f2257800000000, 0xfe3e29b100000000,
    0x3629b77e00000000, 0x2f1764f500000000, 0xe700fa3a00000000,
    0x5c6db33900000000, 0x947a2df600000000, 0x8d44fe7d00000000,
    0x455360b200000000, 0xb0db963400000000, 0x78cc08fb00000000,
    0x61f2db7000000000, 0xa9e545bf00000000, 0x12880cbc00000000,
    0xda9f927300000000, 0xc3a141f800000000, 0x0bb6df3700000000,
    0xb57ad3fe00000000, 0x7d6d4d3100000000, 0x64539eba00000000,
    0xac44007500000000, 0x1729497600000000, 0xdf3ed7b900000000,
    0xc600043200000000, 0x0e179afd00000000, 0x9b28411200000000,
    0x533fdfdd00000000, 0x4a010c5600000000, 0x8216929900000000,
    0x397bdb9a00000000, 0xf16c455500000000, 0xe85296de00000000,
    0x2045081100000000, 0x9e8904d800000000, 0x569e9a1700000000,
    0x4fa0499c00000000, 0x87b7d75300000000, 0x3cda9e5000000000,
    0xf4cd009f00000000, 0xedf3d31400000000, 0x25e44ddb00000000,
    0xd06cbb5d00000000, 0x187b259200000000, 0x0145f61900000000,
    0xc95268d600000000, 0x723f21d500000000, 0xba28bf1a00000000,
    0xa3166c9100000000, 0x6b01f25e00000000, 0xd5cdfe9700000000,
    0x1dda605800000000, 0x04e4b3d300000000, 0xccf32d1c00000000,
    0x779e641f00000000, 0xbf89fad000000000, 0xa6b7295b00000000,
    0x6ea0b79400000000, 0x0da0b58d00000000, 0xc5b72b4200000000,
    0xdc89f8c900000000, 0x149e660600000000, 0xaff32f0500000000,
    0x67e4b1ca00000000, 0x7eda624100000000, 0xb6cdfc8e00000000,
    0x0801f04700000000, 0xc0166e8800000000, 0xd928bd0300000000,
    0x113f23cc00000000, 0xaa526acf00000000, 0x6245f40000000000,
    0x7b7b278b00000000, 0xb36cb94400000000, 0x46e44fc200000000,
    0x8ef3d10d00000000, 0x97cd028600000000, 0x5fda9c4900000000,
    0xe4b7d54a00000000, 0x2ca04b8500000000, 0x359e980e00000000,
    0xfd8906c100000000, 0x43450a0800000000, 0x8b5294c700000000,
    0x926c474c00000000, 0x5a7bd98300000000, 0xe116908000000000,
    0x29010e4f00000000, 0x303fddc400000000, 0xf828430b00000000,
    0xf63fd9f600000000, 0x3e28473900000000, 0x271694b200000000,
    0xef010a7d00000000, 0x546c437e00000000, 0x9c7bddb100000000,
    0x85450e3a00000000, 0x4d5290f500000000, 0xf39e9c3c00000000,
    0x3b8902f300000000, 0x22b7d17800000000, 0xeaa04fb700000000,
    0x51cd06b400000000, 0x99da987b00000000, 0x80e44bf000000000,
    0x48f3d53f00000000, 0xbd7b23b900000000, 0x756cbd7600000000,
    0x6c526efd00000000, 0xa445f03200000000, 0x1f28b93100000000,
    0xd73f27fe00000000, 0xce01f47500000000, 0x06166aba00000000,
    0xb8da667300000000, 0x70cdf8bc00000000, 0x69f32b3700000000,
    0xa1e4b5f800000000, 0x1a89fcfb00000000, 0xd29e623400000000,
    0xcba0b1bf00000000, 0x03b72f7000000000, 0x60b72d6900000000,
    0xa8a0b3a600000000, 0xb19e602d00000000, 0x7989fee200000000,
    0xc2e4b7e100000000, 0x0af3292e00000000, 0x13cdfaa500000000,
    0xdbda646a00000000, 0x651668a300000000, 0xad01f66c00000000,
    0xb43f25e700000000, 0x7c28bb2800000000, 0xc745f22b00000000,
    0x0f526ce400000000, 0x166cbf6f00000000, 0xde7b21a000000000,
    0x2bf3d72600000000, 0xe3e449e900000000, 0xfada9a6200000000,
    0x32cd04ad00000000, 0x89a04dae00000000, 0x41b7d36100000000,
    0x588900ea00000000, 0x909e9e2500000000, 0x2e5292ec00000000,
    0xe6450c2300000000, 0xff7bdfa800000000, 0x376c416700000000,
    0x8c01086400000000, 0x441696ab00000000, 0x5d28452000000000,
    0x953fdbef00000000},
   {0x0000000000000000, 0x95d4709500000000, 0x6baf90f100000000,
    0xfe7be06400000000, 0x9758503800000000, 0x028c20ad00000000,
    0xfcf7c0c900000000, 0x6923b05c00000000, 0x2eb1a07000000000,
    0xbb65d0e500000000, 0x451e308100000000, 0xd0ca401400000000,
    0xb9e9f04800000000, 0x2c3d80dd00000000, 0xd24660b900000000,
    0x4792102c00000000, 0x5c6241e100000000, 0xc9b6317400000000,
    0x37cdd11000000000, 0xa219a18500000000, 0xcb3a11d900000000,
    0x5eee614c00000000, 0xa095812800000000, 0x3541f1bd00000000,
    0x72d3e19100000000, 0xe707910400000000, 0x197c716000000000,
    0x8ca801f500000000, 0xe58bb1a900000000, 0x705fc13c00000000,
    0x8e24215800000000, 0x1bf051cd00000000, 0xf9c2f31900000000,
    0x6c16838c00000000, 0x926d63e800000000, 0x07b9137d00000000,
    0x6e9aa32100000000, 0xfb4ed3b400000000, 0x053533d000000000,
    0x90e1434500000000, 0xd773536900000000, 0x42a723fc00000000,
    0xbcdcc39800000000, 0x2908b30d00000000, 0x402b035100000000,
    0xd5ff73c400000000, 0x2b8493a000000000, 0xbe50e33500000000,
    0xa5a0b2f800000000, 0x3074c26d00000000, 0xce0f220900000000,
    0x5bdb529c00000000, 0x32f8e2c000000000, 0xa72c925500000000,
    0x5957723100000000, 0xcc8302a400000000, 0x8b11128800000000,
    0x1ec5621d00000000, 0xe0be827900000000, 0x756af2ec00000000,
    0x1c4942b000000000, 0x899d322500000000, 0x77e6d24100000000,
    0xe232a2d400000000, 0xf285e73300000000, 0x675197a600000000,
    0x992a77c200000000, 0x0cfe075700000000, 0x65ddb70b00000000,
    0xf009c79e00000000, 0x0e7227fa00000000, 0x9ba6576f00000000,
    0xdc34474300000000, 0x49e037d600000000, 0xb79bd7b200000000,
    0x224fa72700000000, 0x4b6c177b00000000, 0xdeb867ee00000000,
    0x20c3878a00000000, 0xb517f71f00000000, 0xaee7a6d200000000,
    0x3b33d64700000000, 0xc548362300000000, 0x509c46b600000000,
    0x39bff6ea00000000, 0xac6b867f00000000, 0x5210661b00000000,
    0xc7c4168e00000000, 0x805606a200000000, 0x1582763700000000,
    0xebf9965300000000, 0x7e2de6c600000000, 0x170e569a00000000,
    0x82da260f00000000, 0x7ca1c66b00000000, 0xe975b6fe00000000,
    0x0b47142a00000000, 0x9e9364bf00000000, 0x60e884db00000000,
    0xf53cf44e00000000, 0x9c1f441200000000, 0x09cb348700000000,
    0xf7b0d4e300000000, 0x6264a47600000000, 0x25f6b45a00000000,
    0xb022c4cf00000000, 0x4e5924ab00000000, 0xdb8d543e00000000,
    0xb2aee46200000000, 0x277a94f700000000, 0xd901749300000000,
    0x4cd5040600000000, 0x572555cb00000000, 0xc2f1255e00000000,
    0x3c8ac53a00000000, 0xa95eb5af00000000, 0xc07d05f300000000,
    0x55a9756600000000, 0xabd2950200000000, 0x3e06e59700000000,
    0x7994f5bb00000000, 0xec40852e00000000, 0x123b654a00000000,
    0x87ef15df00000000, 0xeecca58300000000, 0x7b18d51600000000,
    0x8563357200000000, 0x10b745e700000000, 0xe40bcf6700000000,
    0x71dfbff200000000, 0x8fa45f9600000000, 0x1a702f0300000000,
    0x73539f5f00000000, 0xe687efca00000000, 0x18fc0fae00000000,
    0x8d287f3b00000000, 0xcaba6f1700000000, 0x5f6e1f8200000000,
    0xa115ffe600000000, 0x34c18f7300000000, 0x5de23f2f00000000,
    0xc8364fba00000000, 0x364dafde00000000, 0xa399df4b00000000,
    0xb8698e8600000000, 0x2dbdfe1300000000, 0xd3c61e7700000000,
    0x46126ee200000000, 0x2f31debe00000000, 0xbae5ae2b00000000,
    0x449e4e4f00000000, 0xd14a3eda00000000, 0x96d82ef600000000,
    0x030c5e6300000000, 0xfd77be0700000000, 0x68a3ce9200000000,
    0x01807ece00000000, 0x94540e5b00000000, 0x6a2fee3f00000000,
    0xfffb9eaa00000000, 0x1dc93c7e00000000, 0x881d4ceb00000000,
    0x7666ac8f00000000, 0xe3b2dc1a00000000, 0x8a916c4600000000,
    0x1f451cd300000000, 0xe13efcb700000000, 0x74ea8c2200000000,
    0x33789c0e00000000, 0xa6acec9b00000000, 0x58d70cff00000000,
    0xcd037c6a00000000, 0xa420cc3600000000, 0x31f4bca300000000,
    0xcf8f5cc700000000, 0x5a5b2c5200000000, 0x41ab7d9f00000000,
    0xd47f0d0a00000000, 0x2a04ed6e00000000, 0xbfd09dfb00000000,
    0xd6f32da700000000, 0x43275d3200000000, 0xbd5cbd5600000000,
    0x2888cdc300000000, 0x6f1addef00000000, 0xfacead7a00000000,
    0x04b54d1e00000000, 0x91613d8b00000000, 0xf8428dd700000000,
    0x6d96fd4200000000, 0x93ed1d2600000000, 0x06396db300000000,
    0x168e285400000000, 0x835a58c100000000, 0x7d21b8a500000000,
    0xe8f5c83000000000, 0x81d6786c00000000, 0x140208f900000000,
    0xea79e89d00000000, 0x7fad980800000000, 0x383f882400000000,
    0xadebf8b100000000, 0x539018d500000000, 0xc644684000000000,
    0xaf67d81c00000000, 0x3ab3a88900000000, 0xc4c848ed00000000,
    0x511c387800000000, 0x4aec69b500000000, 0xdf38192000000000,
    0x2143f94400000000, 0xb49789d100000000, 0xddb4398d00000000,
    0x4860491800000000, 0xb61ba97c00000000, 0x23cfd9e900000000,
    0x645dc9c500000000, 0xf189b95000000000, 0x0ff2593400000000,
    0x9a2629a100000000, 0xf30599fd00000000, 0x66d1e96800000000,
    0x98aa090c00000000, 0x0d7e799900000000, 0xef4cdb4d00000000,
    0x7a98abd800000000, 0x84e34bbc00000000, 0x11373b2900000000,
    0x78148b7500000000, 0xedc0fbe000000000, 0x13bb1b8400000000,
    0x866f6b1100000000, 0xc1fd7b3d00000000, 0x54290ba800000000,
    0xaa52ebcc00000000, 0x3f869b5900000000, 0x56a52b0500000000,
    0xc3715b9000000000, 0x3d0abbf400000000, 0xa8decb6100000000,
    0xb32e9aac00000000, 0x26faea3900000000, 0xd8810a5d00000000,
    0x4d557ac800000000, 0x2476ca9400000000, 0xb1a2ba0100000000,
    0x4fd95a6500000000, 0xda0d2af000000000, 0x9d9f3adc00000000,
    0x084b4a4900000000, 0xf630aa2d00000000, 0x63e4dab800000000,
    0x0ac76ae400000000, 0x9f131a7100000000, 0x6168fa1500000000,
    0xf4bc8a8000000000},
   {0x0000000000000000, 0x1f17f08000000000, 0x7f2891da00000000,
    0x603f615a00000000, 0xbf56536e00000000, 0xa041a3ee00000000,
    0xc07ec2b400000000, 0xdf69323400000000, 0x7eada6dc00000000,
    0x61ba565c00000000, 0x0185370600000000, 0x1e92c78600000000,
    0xc1fbf5b200000000, 0xdeec053200000000, 0xbed3646800000000,
    0xa1c494e800000000, 0xbd5c3c6200000000, 0xa24bcce200000000,
    0xc274adb800000000, 0xdd635d3800000000, 0x020a6f0c00000000,
    0x1d1d9f8c00000000, 0x7d22fed600000000, 0x62350e5600000000,
    0xc3f19abe00000000, 0xdce66a3e00000000, 0xbcd90b6400000000,
    0xa3cefbe400000000, 0x7ca7c9d000000000, 0x63b0395000000000,
    0x038f580a00000000, 0x1c98a88a00000000, 0x7ab978c400000000,
    0x65ae884400000000, 0x0591e91e00000000, 0x1a86199e00000000,
    0xc5ef2baa00000000, 0xdaf8db2a00000000, 0xbac7ba7000000000,
    0xa5d04af000000000, 0x0414de1800000000, 0x1b032e9800000000,
    0x7b3c4fc200000000, 0x642bbf4200000000, 0xbb428d7600000000,
    0xa4557df600000000, 0xc46a1cac00000000, 0xdb7dec2c00000000,
    0xc7e544a600000000, 0xd8f2b42600000000, 0xb8cdd57c00000000,
    0xa7da25fc00000000, 0x78b317c800000000, 0x67a4e74800000000,
    0x079b861200000000, 0x188c769200000000, 0xb948e27a00000000,
    0xa65f12fa00000000, 0xc66073a000000000, 0xd977832000000000,
    0x061eb11400000000, 0x1909419400000000, 0x793620ce00000000,
    0x6621d04e00000000, 0xb574805300000000, 0xaa6370d300000000,
    0xca5c118900000000, 0xd54be10900000000, 0x0a22d33d00000000,
    0x153523bd00000000, 0x750a42e700000000, 0x6a1db26700000000,
    0xcbd9268f00000000, 0xd4ced60f00000000, 0xb4f1b75500000000,
    0xabe647d500000000, 0x748f75e100000000, 0x6b98856100000000,
    0x0ba7e43b00000000, 0x14b014bb00000000, 0x0828bc3100000000,
    0x173f4cb100000000, 0x77002deb00000000, 0x6817dd6b00000000,
    0xb77eef5f00000000, 0xa8691fdf00000000, 0xc8567e8500000000,
    0xd7418e0500000000, 0x76851aed00000000, 0x6992ea6d00000000,
    0x09ad8b3700000000, 0x16ba7bb700000000, 0xc9d3498300000000,
    0xd6c4b90300000000, 0xb6fbd85900000000, 0xa9ec28d900000000,
    0xcfcdf89700000000, 0xd0da081700000000, 0xb0e5694d00000000,
    0xaff299cd00000000, 0x709babf900000000, 0x6f8c5b7900000000,
    0x0fb33a2300000000, 0x10a4caa300000000, 0xb1605e4b00000000,
    0xae77aecb00000000, 0xce48cf9100000000, 0xd15f3f1100000000,
    0x0e360d2500000000, 0x1121fda500000000, 0x711e9cff00000000,
    0x6e096c7f00000000, 0x7291c4f500000000, 0x6d86347500000000,
    0x0db9552f00000000, 0x12aea5af00000000, 0xcdc7979b00000000,
    0xd2d0671b00000000, 0xb2ef064100000000, 0xadf8f6c100000000,
    0x0c3c622900000000, 0x132b92a900000000, 0x7314f3f300000000,
    0x6c03037300000000, 0xb36a314700000000, 0xac7dc1c700000000,
    0xcc42a09d00000000, 0xd355501d00000000, 0x6ae900a700000000,
    0x75fef02700000000, 0x15c1917d00000000, 0x0ad661fd00000000,
    0xd5bf53c900000000, 0xcaa8a34900000000, 0xaa97c21300000000,
    0xb580329300000000, 0x1444a67b00000000, 0x0b5356fb00000000,
    0x6b6c37a100000000, 0x747bc72100000000, 0xab12f51500000000,
    0xb405059500000000, 0xd43a64cf00000000, 0xcb2d944f00000000,
    0xd7b53cc500000000, 0xc8a2cc4500000000, 0xa89dad1f00000000,
    0xb78a5d9f00000000, 0x68e36fab00000000, 0x77f49f2b00000000,
    0x17cbfe7100000000, 0x08dc0ef100000000, 0xa9189a1900000000,
    0xb60f6a9900000000, 0xd6300bc300000000, 0xc927fb4300000000,
    0x164ec97700000000, 0x095939f700000000, 0x696658ad00000000,
    0x7671a82d00000000, 0x1050786300000000, 0x0f4788e300000000,
    0x6f78e9b900000000, 0x706f193900000000, 0xaf062b0d00000000,
    0xb011db8d00000000, 0xd02ebad700000000, 0xcf394a5700000000,
    0x6efddebf00000000, 0x71ea2e3f00000000, 0x11d54f6500000000,
    0x0ec2bfe500000000, 0xd1ab8dd100000000, 0xcebc7d5100000000,
    0xae831c0b00000000, 0xb194ec8b00000000, 0xad0c440100000000,
    0xb21bb48100000000, 0xd224d5db00000000, 0xcd33255b00000000,
    0x125a176f00000000, 0x0d4de7ef00000000, 0x6d7286b500000000,
    0x7265763500000000, 0xd3a1e2dd00000000, 0xccb6125d00000000,
    0xac89730700000000, 0xb39e838700000000, 0x6cf7b1b300000000,
    0x73e0413300000000, 0x13df206900000000, 0x0cc8d0e900000000,
    0xdf9d80f400000000, 0xc08a707400000000, 0xa0b5112e00000000,
    0xbfa2e1ae00000000, 0x60cbd39a00000000, 0x7fdc231a00000000,
    0x1fe3424000000000, 0x00f4b2c000000000, 0xa130262800000000,
    0xbe27d6a800000000, 0xde18b7f200000000, 0xc10f477200000000,
    0x1e66754600000000, 0x017185c600000000, 0x614ee49c00000000,
    0x7e59141c00000000, 0x62c1bc9600000000, 0x7dd64c1600000000,
    0x1de92d4c00000000, 0x02feddcc00000000, 0xdd97eff800000000,
    0xc2801f7800000000, 0xa2bf7e2200000000, 0xbda88ea200000000,
    0x1c6c1a4a00000000, 0x037beaca00000000, 0x63448b9000000000,
    0x7c537b1000000000, 0xa33a492400000000, 0xbc2db9a400000000,
    0xdc12d8fe00000000, 0xc305287e00000000, 0xa524f83000000000,
    0xba3308b000000000, 0xda0c69ea00000000, 0xc51b996a00000000,
    0x1a72ab5e00000000, 0x05655bde00000000, 0x655a3a8400000000,
    0x7a4dca0400000000, 0xdb895eec00000000, 0xc49eae6c00000000,
    0xa4a1cf3600000000, 0xbbb63fb600000000, 0x64df0d8200000000,
    0x7bc8fd0200000000, 0x1bf79c5800000000, 0x04e06cd800000000,
    0x1878c45200000000, 0x076f34d200000000, 0x6750558800000000,
    0x7847a50800000000, 0xa72e973c00000000, 0xb83967bc00000000,
    0xd80606e600000000, 0xc711f66600000000, 0x66d5628e00000000,
    0x79c2920e00000000, 0x19fdf35400000000, 0x06ea03d400000000,
    0xd98331e000000000, 0xc694c16000000000, 0xa6aba03a00000000,
    0xb9bc50ba00000000},
   {0x0000000000000000, 0xe2fd888d00000000, 0x85fd60c000000000,
    0x6700e84d00000000, 0x4bfdb05b00000000, 0xa90038d600000000,
    0xce00d09b00000000, 0x2cfd581600000000, 0x96fa61b700000000,
    0x7407e93a00000000, 0x1307017700000000, 0xf1fa89fa00000000,
    0xdd07d1ec00000000, 0x3ffa596100000000, 0x58fab12c00000000,
    0xba0739a100000000, 0x6df3b2b500000000, 0x8f0e3a3800000000,
    0xe80ed27500000000, 0x0af35af800000000, 0x260e02ee00000000,
    0xc4f38a6300000000, 0xa3f3622e00000000, 0x410eeaa300000000,
    0xfb09d30200000000, 0x19f45b8f00000000, 0x7ef4b3c200000000,
    0x9c093b4f00000000, 0xb0f4635900000000, 0x5209ebd400000000,
    0x3509039900000000, 0xd7f48b1400000000, 0x9be014b000000000,
    0x791d9c3d00000000, 0x1e1d747000000000, 0xfce0fcfd00000000,
    0xd01da4eb00000000, 0x32e02c6600000000, 0x55e0c42b00000000,
    0xb71d4ca600000000, 0x0d1a750700000000, 0xefe7fd8a00000000,
    0x88e715c700000000, 0x6a1a9d4a00000000, 0x46e7c55c00000000,
    0xa41a4dd100000000, 0xc31aa59c00000000, 0x21e72d1100000000,
    0xf613a60500000000, 0x14ee2e8800000000, 0x73eec6c500000000,
    0x91134e4800000000, 0xbdee165e00000000, 0x5f139ed300000000,
    0x3813769e00000000, 0xdaeefe1300000000, 0x60e9c7b200000000,
    0x82144f3f00000000, 0xe514a77200000000, 0x07e92fff00000000,
    0x2b1477e900000000, 0xc9e9ff6400000000, 0xaee9172900000000,
    0x4c149fa400000000, 0x77c758bb00000000, 0x953ad03600000000,
    0xf23a387b00000000, 0x10c7b0f600000000, 0x3c3ae8e000000000,
    0xdec7606d00000000, 0xb9c7882000000000, 0x5b3a00ad00000000,
    0xe13d390c00000000, 0x03c0b18100000000, 0x64c059cc00000000,
    0x863dd14100000000, 0xaac0895700000000, 0x483d01da00000000,
    0x2f3de99700000000, 0xcdc0611a00000000, 0x1a34ea0e00000000,
    0xf8c9628300000000, 0x9fc98ace00000000, 0x7d34024300000000,
    0x51c95a5500000000, 0xb334d2d800000000, 0xd4343a9500000000,
    0x36c9b21800000000, 0x8cce8bb900000000, 0x6e33033400000000,
    0x0933eb7900000000, 0xebce63f400000000, 0xc7333be200000000,
    0x25ceb36f00000000, 0x42ce5b2200000000, 0xa033d3af00000000,
    0xec274c0b00000000, 0x0edac48600000000, 0x69da2ccb00000000,
    0x8b27a44600000000, 0xa7dafc5000000000, 0x452774dd00000000,
    0x22279c9000000000, 0xc0da141d00000000, 0x7add2dbc00000000,
    0x9820a53100000000, 0xff204d7c00000000, 0x1dddc5f100000000,
    0x31209de700000000, 0xd3dd156a00000000, 0xb4ddfd2700000000,
    0x562075aa00000000, 0x81d4febe00000000, 0x6329763300000000,
    0x04299e7e00000000, 0xe6d416f300000000, 0xca294ee500000000,
    0x28d4c66800000000, 0x4fd42e2500000000, 0xad29a6a800000000,
    0x172e9f0900000000, 0xf5d3178400000000, 0x92d3ffc900000000,
    0x702e774400000000, 0x5cd32f5200000000, 0xbe2ea7df00000000,
    0xd92e4f9200000000, 0x3bd3c71f00000000, 0xaf88c0ad00000000,
    0x4d75482000000000, 0x2a75a06d00000000, 0xc88828e000000000,
    0xe47570f600000000, 0x0688f87b00000000, 0x6188103600000000,
    0x837598bb00000000, 0x3972a11a00000000, 0xdb8f299700000000,
    0xbc8fc1da00000000, 0x5e72495700000000, 0x728f114100000000,
    0x907299cc00000000, 0xf772718100000000, 0x158ff90c00000000,
    0xc27b721800000000, 0x2086fa9500000000, 0x478612d800000000,
    0xa57b9a5500000000, 0x8986c24300000000, 0x6b7b4ace00000000,
    0x0c7ba28300000000, 0xee862a0e00000000, 0x548113af00000000,
    0xb67c9b2200000000, 0xd17c736f00000000, 0x3381fbe200000000,
    0x1f7ca3f400000000, 0xfd812b7900000000, 0x9a81c33400000000,
    0x787c4bb900000000, 0x3468d41d00000000, 0xd6955c9000000000,
    0xb195b4dd00000000, 0x53683c5000000000, 0x7f95644600000000,
    0x9d68eccb00000000, 0xfa68048600000000, 0x18958c0b00000000,
    0xa292b5aa00000000, 0x406f3d2700000000, 0x276fd56a00000000,
    0xc5925de700000000, 0xe96f05f100000000, 0x0b928d7c00000000,
    0x6c92653100000000, 0x8e6fedbc00000000, 0x599b66a800000000,
    0xbb66ee2500000000, 0xdc66066800000000, 0x3e9b8ee500000000,
    0x1266d6f300000000, 0xf09b5e7e00000000, 0x979bb63300000000,
    0x75663ebe00000000, 0xcf61071f00000000, 0x2d9c8f9200000000,
    0x4a9c67df00000000, 0xa861ef5200000000, 0x849cb74400000000,
    0x66613fc900000000, 0x0161d78400000000, 0xe39c5f0900000000,
    0xd84f981600000000, 0x3ab2109b00000000, 0x5db2f8d600000000,
    0xbf4f705b00000000, 0x93b2284d00000000, 0x714fa0c000000000,
    0x164f488d00000000, 0xf4b2c00000000000, 0x4eb5f9a100000000,
    0xac48712c00000000, 0xcb48996100000000, 0x29b511ec00000000,
    0x054849fa00000000, 0xe7b5c17700000000, 0x80b5293a00000000,
    0x6248a1b700000000, 0xb5bc2aa300000000, 0x5741a22e00000000,
    0x30414a6300000000, 0xd2bcc2ee00000000, 0xfe419af800000000,
    0x1cbc127500000000, 0x7bbcfa3800000000, 0x994172b500000000,
    0x23464b1400000000, 0xc1bbc39900000000, 0xa6bb2bd400000000,
    0x4446a35900000000, 0x68bbfb4f00000000, 0x8a4673c200000000,
    0xed469b8f00000000, 0x0fbb130200000000, 0x43af8ca600000000,
    0xa152042b00000000, 0xc652ec6600000000, 0x24af64eb00000000,
    0x08523cfd00000000, 0xeaafb47000000000, 0x8daf5c3d00000000,
    0x6f52d4b000000000, 0xd555ed1100000000, 0x37a8659c00000000,
    0x50a88dd100000000, 0xb255055c00000000, 0x9ea85d4a00000000,
    0x7c55d5c700000000, 0x1b553d8a00000000, 0xf9a8b50700000000,
    0x2e5c3e1300000000, 0xcca1b69e00000000, 0xaba15ed300000000,
    0x495cd65e00000000, 0x65a18e4800000000, 0x875c06c500000000,
    0xe05cee8800000000, 0x02a1660500000000, 0xb8a65fa400000000,
    0x5a5bd72900000000, 0x3d5b3f6400000000, 0xdfa6b7e900000000,
    0xf35befff00000000, 0x11a6677200000000, 0x76a68f3f00000000,
    0x945b07b200000000},
   {0x0000000000000000, 0xa90b894e00000000, 0x5217129d00000000,
    0xfb1c9bd300000000, 0xe52855e100000000, 0x4c23dcaf00000000,
    0xb73f477c00000000, 0x1e34ce3200000000, 0x8b57db1900000000,
    0x225c525700000000, 0xd940c98400000000, 0x704b40ca00000000,
    0x6e7f8ef800000000, 0xc77407b600000000, 0x3c689c6500000000,
    0x9563152b00000000, 0x16afb63300000000, 0xbfa43f7d00000000,
    0x44b8a4ae00000000, 0xedb32de000000000, 0xf387e3d200000000,
    0x5a8c6a9c00000000, 0xa190f14f00000000, 0x089b780100000000,
    0x9df86d2a00000000, 0x34f3e46400000000, 0xcfef7fb700000000,
    0x66e4f6f900000000, 0x78d038cb00000000, 0xd1dbb18500000000,
    0x2ac72a5600000000, 0x83cca31800000000, 0x2c5e6d6700000000,
    0x8555e42900000000, 0x7e497ffa00000000, 0xd742f6b400000000,
    0xc976388600000000, 0x607db1c800000000, 0x9b612a1b00000000,
    0x326aa35500000000, 0xa709b67e00000000, 0x0e023f3000000000,
    0xf51ea4e300000000, 0x5c152dad00000000, 0x4221e39f00000000,
    0xeb2a6ad100000000, 0x1036f10200000000, 0xb93d784c00000000,
    0x3af1db5400000000, 0x93fa521a00000000, 0x68e6c9c900000000,
    0xc1ed408700000000, 0xdfd98eb500000000, 0x76d207fb00000000,
    0x8dce9c2800000000, 0x24c5156600000000, 0xb1a6004d00000000,
    0x18ad890300000000, 0xe3b112d000000000, 0x4aba9b9e00000000,
    0x548e55ac00000000, 0xfd85dce200000000, 0x0699473100000000,
    0xaf92ce7f00000000, 0x58bcdace00000000, 0xf1b7538000000000,
    0x0aabc85300000000, 0xa3a0411d00000000, 0xbd948f2f00000000,
    0x149f066100000000, 0xef839db200000000, 0x468814fc00000000,
    0xd3eb01d700000000, 0x7ae0889900000000, 0x81fc134a00000000,
    0x28f79a0400000000, 0x36c3543600000000, 0x9fc8dd7800000000,
    0x64d446ab00000000, 0xcddfcfe500000000, 0x4e136cfd00000000,
    0xe718e5b300000000, 0x1c047e6000000000, 0xb50ff72e00000000,
    0xab3b391c00000000, 0x0230b05200000000, 0xf92c2b8100000000,
    0x5027a2cf00000000, 0xc544b7e400000000, 0x6c4f3eaa00000000,
    0x9753a57900000000, 0x3e582c3700000000, 0x206ce20500000000,
    0x89676b4b00000000, 0x727bf09800000000, 0xdb7079d600000000,
    0x74e2b7a900000000, 0xdde93ee700000000, 0x26f5a53400000000,
    0x8ffe2c7a00000000, 0x91cae24800000000, 0x38c16b0600000000,
    0xc3ddf0d500000000, 0x6ad6799b00000000, 0xffb56cb000000000,
    0x56bee5fe00000000, 0xada27e2d00000000, 0x04a9f76300000000,
    0x1a9d395100000000, 0xb396b01f00000000, 0x488a2bcc00000000,
    0xe181a28200000000, 0x624d019a00000000, 0xcb4688d400000000,
    0x305a130700000000, 0x99519a4900000000, 0x8765547b00000000,
    0x2e6edd3500000000, 0xd57246e600000000, 0x7c79cfa800000000,
    0xe91ada8300000000, 0x401153cd00000000, 0xbb0dc81e00000000,
    0x1206415000000000, 0x0c328f6200000000, 0xa539062c00000000,
    0x5e259dff00000000, 0xf72e14b100000000, 0xf17ec44600000000,
    0x58754d0800000000, 0xa369d6db00000000, 0x0a625f9500000000,
    0x145691a700000000, 0xbd5d18e900000000, 0x4641833a00000000,
    0xef4a0a7400000000, 0x7a291f5f00000000, 0xd322961100000000,
    0x283e0dc200000000, 0x8135848c00000000, 0x9f014abe00000000,
    0x360ac3f000000000, 0xcd16582300000000, 0x641dd16d00000000,
    0xe7d1727500000000, 0x4edafb3b00000000, 0xb5c660e800000000,
    0x1ccde9a600000000, 0x02f9279400000000, 0xabf2aeda00000000,
    0x50ee350900000000, 0xf9e5bc4700000000, 0x6c86a96c00000000,
    0xc58d202200000000, 0x3e91bbf100000000, 0x979a32bf00000000,
    0x89aefc8d00000000, 0x20a575c300000000, 0xdbb9ee1000000000,
    0x72b2675e00000000, 0xdd20a92100000000, 0x742b206f00000000,
    0x8f37bbbc00000000, 0x263c32f200000000, 0x3808fcc000000000,
    0x9103758e00000000, 0x6a1fee5d00000000, 0xc314671300000000,
    0x5677723800000000, 0xff7cfb7600000000, 0x046060a500000000,
    0xad6be9eb00000000, 0xb35f27d900000000, 0x1a54ae9700000000,
    0xe148354400000000, 0x4843bc0a00000000, 0xcb8f1f1200000000,
    0x6284965c00000000, 0x99980d8f00000000, 0x309384c100000000,
    0x2ea74af300000000, 0x87acc3bd00000000, 0x7cb0586e00000000,
    0xd5bbd12000000000, 0x40d8c40b00000000, 0xe9d34d4500000000,
    0x12cfd69600000000, 0xbbc45fd800000000, 0xa5f091ea00000000,
    0x0cfb18a400000000, 0xf7e7837700000000, 0x5eec0a3900000000,
    0xa9c21e8800000000, 0x00c997c600000000, 0xfbd50c1500000000,
    0x52de855b00000000, 0x4cea4b6900000000, 0xe5e1c22700000000,
    0x1efd59f400000000, 0xb7f6d0ba00000000, 0x2295c59100000000,
    0x8b9e4cdf00000000, 0x7082d70c00000000, 0xd9895e4200000000,
    0xc7bd907000000000, 0x6eb6193e00000000, 0x95aa82ed00000000,
    0x3ca10ba300000000, 0xbf6da8bb00000000, 0x166621f500000000,
    0xed7aba2600000000, 0x4471336800000000, 0x5a45fd5a00000000,
    0xf34e741400000000, 0x0852efc700000000, 0xa159668900000000,
    0x343a73a200000000, 0x9d31faec00000000, 0x662d613f00000000,
    0xcf26e87100000000, 0xd112264300000000, 0x7819af0d00000000,
    0x830534de00000000, 0x2a0ebd9000000000, 0x859c73ef00000000,
    0x2c97faa100000000, 0xd78b617200000000, 0x7e80e83c00000000,
    0x60b4260e00000000, 0xc9bfaf4000000000, 0x32a3349300000000,
    0x9ba8bddd00000000, 0x0ecba8f600000000, 0xa7c021b800000000,
    0x5cdcba6b00000000, 0xf5d7332500000000, 0xebe3fd1700000000,
    0x42e8745900000000, 0xb9f4ef8a00000000, 0x10ff66c400000000,
    0x9333c5dc00000000, 0x3a384c9200000000, 0xc124d74100000000,
    0x682f5e0f00000000, 0x761b903d00000000, 0xdf10197300000000,
    0x240c82a000000000, 0x8d070bee00000000, 0x18641ec500000000,
    0xb16f978b00000000, 0x4a730c5800000000, 0xe378851600000000,
    0xfd4c4b2400000000, 0x5447c26a00000000, 0xaf5b59b900000000,
    0x0650d0f700000000},
   {0x0000000000000000, 0x479244af00000000, 0xcf22f88500000000,
    0x88b0bc2a00000000, 0xdf4381d000000000, 0x98d1c57f00000000,
    0x1061795500000000, 0x57f33dfa00000000, 0xff81737a00000000,
    0xb81337d500000000, 0x30a38bff00000000, 0x7731cf5000000000,
    0x20c2f2aa00000000, 0x6750b60500000000, 0xefe00a2f00000000,
    0xa8724e8000000000, 0xfe03e7f400000000, 0xb991a35b00000000,
    0x31211f7100000000, 0x76b35bde00000000, 0x2140662400000000,
    0x66d2228b00000000, 0xee629ea100000000, 0xa9f0da0e00000000,
    0x0182948e00000000, 0x4610d02100000000, 0xcea06c0b00000000,
    0x893228a400000000, 0xdec1155e00000000, 0x995351f100000000,
    0x11e3eddb00000000, 0x5671a97400000000, 0xbd01bf3200000000,
    0xfa93fb9d00000000, 0x722347b700000000, 0x35b1031800000000,
    0x62423ee200000000, 0x25d07a4d00000000, 0xad60c66700000000,
    0xeaf282c800000000, 0x4280cc4800000000, 0x051288e700000000,
    0x8da234cd00000000, 0xca30706200000000, 0x9dc34d9800000000,
    0xda51093700000000, 0x52e1b51d00000000, 0x1573f1b200000000,
    0x430258c600000000, 0x04901c6900000000, 0x8c20a04300000000,
    0xcbb2e4ec00000000, 0x9c41d91600000000, 0xdbd39db900000000,
    0x5363219300000000, 0x14f1653c00000000, 0xbc832bbc00000000,
    0xfb116f1300000000, 0x73a1d33900000000, 0x3433979600000000,
    0x63c0aa6c00000000, 0x2452eec300000000, 0xace252e900000000,
    0xeb70164600000000, 0x7a037e6500000000, 0x3d913aca00000000,
    0xb52186e000000000, 0xf2b3c24f00000000, 0xa540ffb500000000,
    0xe2d2bb1a00000000, 0x6a62073000000000, 0x2df0439f00000000,
    0x85820d1f00000000, 0xc21049b000000000, 0x4aa0f59a00000000,
    0x0d32b13500000000, 0x5ac18ccf00000000, 0x1d53c86000000000,
    0x95e3744a00000000, 0xd27130e500000000, 0x8400999100000000,
    0xc392dd3e00000000, 0x4b22611400000000, 0x0cb025bb00000000,
    0x5b43184100000000, 0x1cd15cee00000000, 0x9461e0c400000000,
    0xd3f3a46b00000000, 0x7b81eaeb00000000, 0x3c13ae4400000000,
    0xb4a3126e00000000, 0xf33156c100000000, 0xa4c26b3b00000000,
    0xe3502f9400000000, 0x6be093be00000000, 0x2c72d71100000000,
    0xc702c15700000000, 0x809085f800000000, 0x082039d200000000,
    0x4fb27d7d00000000, 0x1841408700000000, 0x5fd3042800000000,
    0xd763b80200000000, 0x90f1fcad00000000, 0x3883b22d00000000,
    0x7f11f68200000000, 0xf7a14aa800000000, 0xb0330e0700000000,
    0xe7c033fd00000000, 0xa052775200000000, 0x28e2cb7800000000,
    0x6f708fd700000000, 0x390126a300000000, 0x7e93620c00000000,
    0xf623de2600000000, 0xb1b19a8900000000, 0xe642a77300000000,
    0xa1d0e3dc00000000, 0x29605ff600000000, 0x6ef21b5900000000,
    0xc68055d900000000, 0x8112117600000000, 0x09a2ad5c00000000,
    0x4e30e9f300000000, 0x19c3d40900000000, 0x5e5190a600000000,
    0xd6e12c8c00000000, 0x9173682300000000, 0xf406fcca00000000,
    0xb394b86500000000, 0x3b24044f00000000, 0x7cb640e000000000,
    0x2b457d1a00000000, 0x6cd739b500000000, 0xe467859f00000000,
    0xa3f5c13000000000, 0x0b878fb000000000, 0x4c15cb1f00000000,
    0xc4a5773500000000, 0x8337339a00000000, 0xd4c40e6000000000,
    0x93564acf00000000, 0x1be6f6e500000000, 0x5c74b24a00000000,
    0x0a051b3e00000000, 0x4d975f9100000000, 0xc527e3bb00000000,
    0x82b5a71400000000, 0xd5469aee00000000, 0x92d4de4100000000,
    0x1a64626b00000000, 0x5df626c400000000, 0xf584684400000000,
    0xb2162ceb00000000, 0x3aa690c100000000, 0x7d34d46e00000000,
    0x2ac7e99400000000, 0x6d55ad3b00000000, 0xe5e5111100000000,
    0xa27755be00000000, 0x490743f800000000, 0x0e95075700000000,
    0x8625bb7d00000000, 0xc1b7ffd200000000, 0x9644c22800000000,
    0xd1d6868700000000, 0x59663aad00000000, 0x1ef47e0200000000,
    0xb686308200000000, 0xf114742d00000000, 0x79a4c80700000000,
    0x3e368ca800000000, 0x69c5b15200000000, 0x2e57f5fd00000000,
    0xa6e749d700000000, 0xe1750d7800000000, 0xb704a40c00000000,
    0xf096e0a300000000, 0x78265c8900000000, 0x3fb4182600000000,
    0x684725dc00000000, 0x2fd5617300000000, 0xa765dd5900000000,
    0xe0f799f600000000, 0x4885d77600000000, 0x0f1793d900000000,
    0x87a72ff300000000, 0xc0356b5c00000000, 0x97c656a600000000,
    0xd054120900000000, 0x58e4ae2300000000, 0x1f76ea8c00000000,
    0x8e0582af00000000, 0xc997c60000000000, 0x41277a2a00000000,
    0x06b53e8500000000, 0x5146037f00000000, 0x16d447d000000000,
    0x9e64fbfa00000000, 0xd9f6bf5500000000, 0x7184f1d500000000,
    0x3616b57a00000000, 0xbea6095000000000, 0xf9344dff00000000,
    0xaec7700500000000, 0xe95534aa00000000, 0x61e5888000000000,
    0x2677cc2f00000000, 0x7006655b00000000, 0x379421f400000000,
    0xbf249dde00000000, 0xf8b6d97100000000, 0xaf45e48b00000000,
    0xe8d7a02400000000, 0x60671c0e00000000, 0x27f558a100000000,
    0x8f87162100000000, 0xc815528e00000000, 0x40a5eea400000000,
    0x0737aa0b00000000, 0x50c497f100000000, 0x1756d35e00000000,
    0x9fe66f7400000000, 0xd8742bdb00000000, 0x33043d9d00000000,
    0x7496793200000000, 0xfc26c51800000000, 0xbbb481b700000000,
    0xec47bc4d00000000, 0xabd5f8e200000000, 0x236544c800000000,
    0x64f7006700000000, 0xcc854ee700000000, 0x8b170a4800000000,
    0x03a7b66200000000, 0x4435f2cd00000000, 0x13c6cf3700000000,
    0x54548b9800000000, 0xdce437b200000000, 0x9b76731d00000000,
    0xcd07da6900000000, 0x8a959ec600000000, 0x022522ec00000000,
    0x45b7664300000000, 0x12445bb900000000, 0x55d61f1600000000,
    0xdd66a33c00000000, 0x9af4e79300000000, 0x3286a91300000000,
    0x7514edbc00000000, 0xfda4519600000000, 0xba36153900000000,
    0xedc528c300000000, 0xaa576c6c00000000, 0x22e7d04600000000,
    0x657594e900000000}};

#else /* W == 4 */

local const z_crc_t FAR crc_braid_table[][256] = {
   {0x00000000, 0x65673b46, 0xcace768c, 0xafa94dca, 0x4eedeb59,
    0x2b8ad01f, 0x84239dd5, 0xe144a693, 0x9ddbd6b2, 0xf8bcedf4,
    0x5715a03e, 0x32729b78, 0xd3363deb, 0xb65106ad, 0x19f84b67,
    0x7c9f7021, 0xe0c6ab25, 0x85a19063, 0x2a08dda9, 0x4f6fe6ef,
    0xae2b407c, 0xcb4c7b3a, 0x64e536f0, 0x01820db6, 0x7d1d7d97,
    0x187a46d1, 0xb7d30b1b, 0xd2b4305d, 0x33f096ce, 0x5697ad88,
    0xf93ee042, 0x9c59db04, 0x1afc500b, 0x7f9b6b4d, 0xd0322687,
    0xb5551dc1, 0x5411bb52, 0x31768014, 0x9edfcdde, 0xfbb8f698,
    0x872786b9, 0xe240bdff, 0x4de9f035, 0x288ecb73, 0xc9ca6de0,
    0xacad56a6, 0x03041b6c, 0x6663202a, 0xfa3afb2e, 0x9f5dc068,
    0x30f48da2, 0x5593b6e4, 0xb4d71077, 0xd1b02b31, 0x7e1966fb,
    0x1b7e5dbd, 0x67e12d9c, 0x028616da, 0xad2f5b10, 0xc8486056,
    0x290cc6c5, 0x4c6bfd83, 0xe3c2b049, 0x86a58b0f, 0x35f8a016,
    0x509f9b50, 0xff36d69a, 0x9a51eddc, 0x7b154b4f, 0x1e727009,
    0xb1db3dc3, 0xd4bc0685, 0xa82376a4, 0xcd444de2, 0x62ed0028,
    0x078a3b6e, 0xe6ce9dfd, 0x83a9a6bb, 0x2c00eb71, 0x4967d037,
    0xd53e0b33, 0xb0593075, 0x1ff07dbf, 0x7a9746f9, 0x9bd3e06a,
    0xfeb4db2c, 0x511d96e6, 0x347aada0, 0x48e5dd81, 0x2d82e6c7,
    0x822bab0d, 0xe74c904b, 0x060836d8, 0x636f0d9e, 0xccc64054,
    0xa9a17b12, 0x2f04f01d, 0x4a63cb5b, 0xe5ca8691, 0x80adbdd7,
    0x61e91b44, 0x048e2002, 0xab276dc8, 0xce40568e, 0xb2df26af,
    0xd7b81de9, 0x78115023, 0x1d766b65, 0xfc32cdf6, 0x9955f6b0,
    0x36fcbb7a, 0x539b803c, 0xcfc25b38, 0xaaa5607e, 0x050c2db4,
    0x606b16f2, 0x812fb061, 0xe4488b27, 0x4be1c6ed, 0x2e86fdab,
    0x52198d8a, 0x377eb6cc, 0x98d7fb06, 0xfdb0c040, 0x1cf466d3,
    0x79935d95, 0xd63a105f, 0xb35d2b19, 0x6bf1402c, 0x0e967b6a,
    0xa13f36a0, 0xc4580de6, 0x251cab75, 0x407b9033, 0xefd2ddf9,
    0x8ab5e6bf, 0xf62a969e, 0x934dadd8, 0x3ce4e012, 0x5983db54,
    0xb8c77dc7, 0xdda04681, 0x72090b4b, 0x176e300d, 0x8b37eb09,
    0xee50d04f, 0x41f99d85, 0x249ea6c3, 0xc5da0050, 0xa0bd3b16,
    0x0f1476dc, 0x6a734d9a, 0x16ec3dbb, 0x738b06fd, 0xdc224b37,
    0xb9457071, 0x5801d6e2, 0x3d66eda4, 0x92cfa06e, 0xf7a89b28,
    0x710d1027, 0x146a2b61, 0xbbc366ab, 0xdea45ded, 0x3fe0fb7e,
    0x5a87c038, 0xf52e8df2, 0x9049b6b4, 0xecd6c695, 0x89b1fdd3,
    0x2618b019, 0x437f8b5f, 0xa23b2dcc, 0xc75c168a, 0x68f55b40,
    0x0d926006, 0x91cbbb02, 0xf4ac8044, 0x5b05cd8e, 0x3e62f6c8,
    0xdf26505b, 0xba416b1d, 0x15e826d7, 0x708f1d91, 0x0c106db0,
    0x697756f6, 0xc6de1b3c, 0xa3b9207a, 0x42fd86e9, 0x279abdaf,
    0x8833f065, 0xed54cb23, 0x5e09e03a, 0x3b6edb7c, 0x94c796b6,
    0xf1a0adf0, 0x10e40b63, 0x75833025, 0xda2a7def, 0xbf4d46a9,
    0xc3d23688, 0xa6b50dce, 0x091c4004, 0x6c7b7b42, 0x8d3fddd1,
    0xe858e697, 0x47f1ab5d, 0x2296901b, 0xbecf4b1f, 0xdba87059,
    0x74013d93, 0x116606d5, 0xf022a046, 0x95459b00, 0x3aecd6ca,
    0x5f8bed8c, 0x23149dad, 0x4673a6eb, 0xe9daeb21, 0x8cbdd067,
    0x6df976f4, 0x089e4db2, 0xa7370078, 0xc2503b3e, 0x44f5b031,
    0x21928b77, 0x8e3bc6bd, 0xeb5cfdfb, 0x0a185b68, 0x6f7f602e,
    0xc0d62de4, 0xa5b116a2, 0xd92e6683, 0xbc495dc5, 0x13e0100f,
    0x76872b49, 0x97c38dda, 0xf2a4b69c, 0x5d0dfb56, 0x386ac010,
    0xa4331b14, 0xc1542052, 0x6efd6d98, 0x0b9a56de, 0xeadef04d,
    0x8fb9cb0b, 0x201086c1, 0x4577bd87, 0x39e8cda6, 0x5c8ff6e0,
    0xf326bb2a, 0x9641806c, 0x770526ff, 0x12621db9, 0xbdcb5073,
    0xd8ac6b35},
   {0x00000000, 0xd7e28058, 0x74b406f1, 0xa35686a9, 0xe9680de2,
    0x3e8a8dba, 0x9ddc0b13, 0x4a3e8b4b, 0x09a11d85, 0xde439ddd,
    0x7d151b74, 0xaaf79b2c, 0xe0c91067, 0x372b903f, 0x947d1696,
    0x439f96ce, 0x13423b0a, 0xc4a0bb52, 0x67f63dfb, 0xb014bda3,
    0xfa2a36e8, 0x2dc8b6b0, 0x8e9e3019, 0x597cb041, 0x1ae3268f,
    0xcd01a6d7, 0x6e57207e, 0xb9b5a026, 0xf38b2b6d, 0x2469ab35,
    0x873f2d9c, 0x50ddadc4, 0x26847614, 0xf166f64c, 0x523070e5,
    0x85d2f0bd, 0xcfec7bf6, 0x180efbae, 0xbb587d07, 0x6cbafd5f,
    0x2f256b91, 0xf8c7ebc9, 0x5b916d60, 0x8c73ed38, 0xc64d6673,
    0x11afe62b, 0xb2f96082, 0x651be0da, 0x35c64d1e, 0xe224cd46,
    0x41724bef, 0x9690cbb7, 0xdcae40fc, 0x0b4cc0a4, 0xa81a460d,
    0x7ff8c655, 0x3c67509b, 0xeb85d0c3, 0x48d3566a, 0x9f31d632,
    0xd50f5d79, 0x02eddd21, 0xa1bb5b88, 0x7659dbd0, 0x4d08ec28,
    0x9aea6c70, 0x39bcead9, 0xee5e6a81, 0xa460e1ca, 0x73826192,
    0xd0d4e73b, 0x07366763, 0x44a9f1ad, 0x934b71f5, 0x301df75c,
    0xe7ff7704, 0xadc1fc4f, 0x7a237c17, 0xd975fabe, 0x0e977ae6,
    0x5e4ad722, 0x89a8577a, 0x2afed1d3, 0xfd1c518b, 0xb722dac0,
    0x60c05a98, 0xc396dc31, 0x14745c69, 0x57ebcaa7, 0x80094aff,
    0x235fcc56, 0xf4bd4c0e, 0xbe83c745, 0x6961471d, 0xca37c1b4,
    0x1dd541ec, 0x6b8c9a3c, 0xbc6e1a64, 0x1f389ccd, 0xc8da1c95,
    0x82e497de, 0x55061786, 0xf650912f, 0x21b21177, 0x622d87b9,
    0xb5cf07e1, 0x16998148, 0xc17b0110, 0x8b458a5b, 0x5ca70a03,
    0xfff18caa, 0x28130cf2, 0x78cea136, 0xaf2c216e, 0x0c7aa7c7,
    0xdb98279f, 0x91a6acd4, 0x46442c8c, 0xe512aa25, 0x32f02a7d,
    0x716fbcb3, 0xa68d3ceb, 0x05dbba42, 0xd2393a1a, 0x9807b151,
    0x4fe53109, 0xecb3b7a0, 0x3b5137f8, 0x9a11d850, 0x4df35808,
    0xeea5dea1, 0x39475ef9, 0x7379d5b2, 0xa49b55ea, 0x07cdd343,
    0xd02f531b, 0x93b0c5d5, 0x4452458d, 0xe704c324, 0x30e6437c,
    0x7ad8c837, 0xad3a486f, 0x0e6ccec6, 0xd98e4e9e, 0x8953e35a,
    0x5eb16302, 0xfde7e5ab, 0x2a0565f3, 0x603beeb8, 0xb7d96ee0,
    0x148fe849, 0xc36d6811, 0x80f2fedf, 0x57107e87, 0xf446f82e,
    0x23a47876, 0x699af33d, 0xbe787365, 0x1d2ef5cc, 0xcacc7594,
    0xbc95ae44, 0x6b772e1c, 0xc821a8b5, 0x1fc328ed, 0x55fda3a6,
    0x821f23fe, 0x2149a557, 0xf6ab250f, 0xb534b3c1, 0x62d63399,
    0xc180b530, 0x16623568, 0x5c5cbe23, 0x8bbe3e7b, 0x28e8b8d2,
    0xff0a388a, 0xafd7954e, 0x78351516, 0xdb6393bf, 0x0c8113e7,
    0x46bf98ac, 0x915d18f4, 0x320b9e5d, 0xe5e91e05, 0xa67688cb,
    0x71940893, 0xd2c28e3a, 0x05200e62, 0x4f1e8529, 0x98fc0571,
    0x3baa83d8, 0xec480380, 0xd7193478, 0x00fbb420, 0xa3ad3289,
    0x744fb2d1, 0x3e71399a, 0xe993b9c2, 0x4ac53f6b, 0x9d27bf33,
    0xdeb829fd, 0x095aa9a5, 0xaa0c2f0c, 0x7deeaf54, 0x37d0241f,
    0xe032a447, 0x436422ee, 0x9486a2b6, 0xc45b0f72, 0x13b98f2a,
    0xb0ef0983, 0x670d89db, 0x2d330290, 0xfad182c8, 0x59870461,
    0x8e658439, 0xcdfa12f7, 0x1a1892af, 0xb94e1406, 0x6eac945e,
    0x24921f15, 0xf3709f4d, 0x502619e4, 0x87c499bc, 0xf19d426c,
    0x267fc234, 0x8529449d, 0x52cbc4c5, 0x18f54f8e, 0xcf17cfd6,
    0x6c41497f, 0xbba3c927, 0xf83c5fe9, 0x2fdedfb1, 0x8c885918,
    0x5b6ad940, 0x1154520b, 0xc6b6d253, 0x65e054fa, 0xb202d4a2,
    0xe2df7966, 0x353df93e, 0x966b7f97, 0x4189ffcf, 0x0bb77484,
    0xdc55f4dc, 0x7f037275, 0xa8e1f22d, 0xeb7e64e3, 0x3c9ce4bb,
    0x9fca6212, 0x4828e24a, 0x02166901, 0xd5f4e959, 0x76a26ff0,
    0xa140efa8},
   {0x00000000, 0xef52b6e1, 0x05d46b83, 0xea86dd62, 0x0ba8d706,
    0xe4fa61e7, 0x0e7cbc85, 0xe12e0a64, 0x1751ae0c, 0xf80318ed,
    0x1285c58f, 0xfdd7736e, 0x1cf9790a, 0xf3abcfeb, 0x192d1289,
    0xf67fa468, 0x2ea35c18, 0xc1f1eaf9, 0x2b77379b, 0xc425817a,
    0x250b8b1e, 0xca593dff, 0x20dfe09d, 0xcf8d567c, 0x39f2f214,
    0xd6a044f5, 0x3c269997, 0xd3742f76, 0x325a2512, 0xdd0893f3,
    0x378e4e91, 0xd8dcf870, 0x5d46b830, 0xb2140ed1, 0x5892d3b3,
    0xb7c06552, 0x56ee6f36, 0xb9bcd9d7, 0x533a04b5, 0xbc68b254,
    0x4a17163c, 0xa545a0dd, 0x4fc37dbf, 0xa091cb5e, 0x41bfc13a,
    0xaeed77db, 0x446baab9, 0xab391c58, 0x73e5e428, 0x9cb752c9,
    0x76318fab, 0x9963394a, 0x784d332e, 0x971f85cf, 0x7d9958ad,
    0x92cbee4c, 0x64b44a24, 0x8be6fcc5, 0x616021a7, 0x8e329746,
    0x6f1c9d22, 0x804e2bc3, 0x6ac8f6a1, 0x859a4040, 0xba8d7060,
    0x55dfc681, 0xbf591be3, 0x500bad02, 0xb125a766, 0x5e771187,
    0xb4f1cce5, 0x5ba37a04, 0xaddcde6c, 0x428e688d, 0xa808b5ef,
    0x475a030e, 0xa674096a, 0x4926bf8b, 0xa3a062e9, 0x4cf2d408,
    0x942e2c78, 0x7b7c9a99, 0x91fa47fb, 0x7ea8f11a, 0x9f86fb7e,
    0x70d44d9f, 0x9a5290fd, 0x7500261c, 0x837f8274, 0x6c2d3495,
    0x86abe9f7, 0x69f95f16, 0x88d75572, 0x6785e393, 0x8d033ef1,
    0x62518810, 0xe7cbc850, 0x08997eb1, 0xe21fa3d3, 0x0d4d1532,
    0xec631f56, 0x0331a9b7, 0xe9b774d5, 0x06e5c234, 0xf09a665c,
    0x1fc8d0bd, 0xf54e0ddf, 0x1a1cbb3e, 0xfb32b15a, 0x146007bb,
    0xfee6dad9, 0x11b46c38, 0xc9689448, 0x263a22a9, 0xccbcffcb,
    0x23ee492a, 0xc2c0434e, 0x2d92f5af, 0xc71428cd, 0x28469e2c,
    0xde393a44, 0x316b8ca5, 0xdbed51c7, 0x34bfe726, 0xd591ed42,
    0x3ac35ba3, 0xd04586c1, 0x3f173020, 0xae6be681, 0x41395060,
    0xabbf8d02, 0x44ed3be3, 0xa5c33187, 0x4a918766, 0xa0175a04,
    0x4f45ece5, 0xb93a488d, 0x5668fe6c, 0xbcee230e, 0x53bc95ef,
    0xb2929f8b, 0x5dc0296a, 0xb746f408, 0x581442e9, 0x80c8ba99,
    0x6f9a0c78, 0x851cd11a, 0x6a4e67fb, 0x8b606d9f, 0x6432db7e,
    0x8eb4061c, 0x61e6b0fd, 0x97991495, 0x78cba274, 0x924d7f16,
    0x7d1fc9f7, 0x9c31c393, 0x73637572, 0x99e5a810, 0x76b71ef1,
    0xf32d5eb1, 0x1c7fe850, 0xf6f93532, 0x19ab83d3, 0xf88589b7,
    0x17d73f56, 0xfd51e234, 0x120354d5, 0xe47cf0bd, 0x0b2e465c,
    0xe1a89b3e, 0x0efa2ddf, 0xefd427bb, 0x0086915a, 0xea004c38,
    0x0552fad9, 0xdd8e02a9, 0x32dcb448, 0xd85a692a, 0x3708dfcb,
    0xd626d5af, 0x3974634e, 0xd3f2be2c, 0x3ca008cd, 0xcadfaca5,
    0x258d1a44, 0xcf0bc726, 0x205971c7, 0xc1777ba3, 0x2e25cd42,
    0xc4a31020, 0x2bf1a6c1, 0x14e696e1, 0xfbb42000, 0x1132fd62,
    0xfe604b83, 0x1f4e41e7, 0xf01cf706, 0x1a9a2a64, 0xf5c89c85,
    0x03b738ed, 0xece58e0c, 0x0663536e, 0xe931e58f, 0x081fefeb,
    0xe74d590a, 0x0dcb8468, 0xe2993289, 0x3a45caf9, 0xd5177c18,
    0x3f91a17a, 0xd0c3179b, 0x31ed1dff, 0xdebfab1e, 0x3439767c,
    0xdb6bc09d, 0x2d1464f5, 0xc246d214, 0x28c00f76, 0xc792b997,
    0x26bcb3f3, 0xc9ee0512, 0x2368d870, 0xcc3a6e91, 0x49a02ed1,
    0xa6f29830, 0x4c744552, 0xa326f3b3, 0x4208f9d7, 0xad5a4f36,
    0x47dc9254, 0xa88e24b5, 0x5ef180dd, 0xb1a3363c, 0x5b25eb5e,
    0xb4775dbf, 0x555957db, 0xba0be13a, 0x508d3c58, 0xbfdf8ab9,
    0x670372c9, 0x8851c428, 0x62d7194a, 0x8d85afab, 0x6caba5cf,
    0x83f9132e, 0x697fce4c, 0x862d78ad, 0x7052dcc5, 0x9f006a24,
    0x7586b746, 0x9ad401a7, 0x7bfa0bc3, 0x94a8bd22, 0x7e2e6040,
    0x917cd6a1},
   {0x00000000, 0x87a6cb43, 0xd43c90c7, 0x539a5b84, 0x730827cf,
    0xf4aeec8c, 0xa734b708, 0x20927c4b, 0xe6104f9e, 0x61b684dd,
    0x322cdf59, 0xb58a141a, 0x95186851, 0x12bea312, 0x4124f896,
    0xc68233d5, 0x1751997d, 0x90f7523e, 0xc36d09ba, 0x44cbc2f9,
    0x6459beb2, 0xe3ff75f1, 0xb0652e75, 0x37c3e536, 0xf141d6e3,
    0x76e71da0, 0x257d4624, 0xa2db8d67, 0x8249f12c, 0x05ef3a6f,
    0x567561eb, 0xd1d3aaa8, 0x2ea332fa, 0xa905f9b9, 0xfa9fa23d,
    0x7d39697e, 0x5dab1535, 0xda0dde76, 0x899785f2, 0x0e314eb1,
    0xc8b37d64, 0x4f15b627, 0x1c8feda3, 0x9b2926e0, 0xbbbb5aab,
    0x3c1d91e8, 0x6f87ca6c, 0xe821012f, 0x39f2ab87, 0xbe5460c4,
    0xedce3b40, 0x6a68f003, 0x4afa8c48, 0xcd5c470b, 0x9ec61c8f,
    0x1960d7cc, 0xdfe2e419, 0x58442f5a, 0x0bde74de, 0x8c78bf9d,
    0xaceac3d6, 0x2b4c0895, 0x78d65311, 0xff709852, 0x5d4665f4,
    0xdae0aeb7, 0x897af533, 0x0edc3e70, 0x2e4e423b, 0xa9e88978,
    0xfa72d2fc, 0x7dd419bf, 0xbb562a6a, 0x3cf0e129, 0x6f6abaad,
    0xe8cc71ee, 0xc85e0da5, 0x4ff8c6e6, 0x1c629d62, 0x9bc45621,
    0x4a17fc89, 0xcdb137ca, 0x9e2b6c4e, 0x198da70d, 0x391fdb46,
    0xbeb91005, 0xed234b81, 0x6a8580c2, 0xac07b317, 0x2ba17854,
    0x783b23d0, 0xff9de893, 0xdf0f94d8, 0x58a95f9b, 0x0b33041f,
    0x8c95cf5c, 0x73e5570e, 0xf4439c4d, 0xa7d9c7c9, 0x207f0c8a,
    0x00ed70c1, 0x874bbb82, 0xd4d1e006, 0x53772b45, 0x95f51890,
    0x1253d3d3, 0x41c98857, 0xc66f4314, 0xe6fd3f5f, 0x615bf41c,
    0x32c1af98, 0xb56764db, 0x64b4ce73, 0xe3120530, 0xb0885eb4,
    0x372e95f7, 0x17bce9bc, 0x901a22ff, 0xc380797b, 0x4426b238,
    0x82a481ed, 0x05024aae, 0x5698112a, 0xd13eda69, 0xf1aca622,
    0x760a6d61, 0x259036e5, 0xa236fda6, 0xba8ccbe8, 0x3d2a00ab,
    0x6eb05b2f, 0xe916906c, 0xc984ec27, 0x4e222764, 0x1db87ce0,
    0x9a1eb7a3, 0x5c9c8476, 0xdb3a4f35, 0x88a014b1, 0x0f06dff2,
    0x2f94a3b9, 0xa83268fa, 0xfba8337e, 0x7c0ef83d, 0xaddd5295,
    0x2a7b99d6, 0x79e1c252, 0xfe470911, 0xded5755a, 0x5973be19,
    0x0ae9e59d, 0x8d4f2ede, 0x4bcd1d0b, 0xcc6bd648, 0x9ff18dcc,
    0x1857468f, 0x38c53ac4, 0xbf63f187, 0xecf9aa03, 0x6b5f6140,
    0x942ff912, 0x13893251, 0x401369d5, 0xc7b5a296, 0xe727dedd,
    0x6081159e, 0x331b4e1a, 0xb4bd8559, 0x723fb68c, 0xf5997dcf,
    0xa603264b, 0x21a5ed08, 0x01379143, 0x86915a00, 0xd50b0184,
    0x52adcac7, 0x837e606f, 0x04d8ab2c, 0x5742f0a8, 0xd0e43beb,
    0xf07647a0, 0x77d08ce3, 0x244ad767, 0xa3ec1c24, 0x656e2ff1,
    0xe2c8e4b2, 0xb152bf36, 0x36f47475, 0x1666083e, 0x91c0c37d,
    0xc25a98f9, 0x45fc53ba, 0xe7caae1c, 0x606c655f, 0x33f63edb,
    0xb450f598, 0x94c289d3, 0x13644290, 0x40fe1914, 0xc758d257,
    0x01dae182, 0x867c2ac1, 0xd5e67145, 0x5240ba06, 0x72d2c64d,
    0xf5740d0e, 0xa6ee568a, 0x21489dc9, 0xf09b3761, 0x773dfc22,
    0x24a7a7a6, 0xa3016ce5, 0x839310ae, 0x0435dbed, 0x57af8069,
    0xd0094b2a, 0x168b78ff, 0x912db3bc, 0xc2b7e838, 0x4511237b,
    0x65835f30, 0xe2259473, 0xb1bfcff7, 0x361904b4, 0xc9699ce6,
    0x4ecf57a5, 0x1d550c21, 0x9af3c762, 0xba61bb29, 0x3dc7706a,
    0x6e5d2bee, 0xe9fbe0ad, 0x2f79d378, 0xa8df183b, 0xfb4543bf,
    0x7ce388fc, 0x5c71f4b7, 0xdbd73ff4, 0x884d6470, 0x0febaf33,
    0xde38059b, 0x599eced8, 0x0a04955c, 0x8da25e1f, 0xad302254,
    0x2a96e917, 0x790cb293, 0xfeaa79d0, 0x38284a05, 0xbf8e8146,
    0xec14dac2, 0x6bb21181, 0x4b206dca, 0xcc86a689, 0x9f1cfd0d,
    0x18ba364e}};

local const z_word_t FAR crc_braid_big_table[][256] = {
   {0x00000000, 0x43cba687, 0xc7903cd4, 0x845b9a53, 0xcf270873,
    0x8cecaef4, 0x08b734a7, 0x4b7c9220, 0x9e4f10e6, 0xdd84b661,
    0x59df2c32, 0x1a148ab5, 0x51681895, 0x12a3be12, 0x96f82441,
    0xd53382c6, 0x7d995117, 0x3e52f790, 0xba096dc3, 0xf9c2cb44,
    0xb2be5964, 0xf175ffe3, 0x752e65b0, 0x36e5c337, 0xe3d641f1,
    0xa01de776, 0x24467d25, 0x678ddba2, 0x2cf14982, 0x6f3aef05,
    0xeb617556, 0xa8aad3d1, 0xfa32a32e, 0xb9f905a9, 0x3da29ffa,
    0x7e69397d, 0x3515ab5d, 0x76de0dda, 0xf2859789, 0xb14e310e,
    0x647db3c8, 0x27b6154f, 0xa3ed8f1c, 0xe026299b, 0xab5abbbb,
    0xe8911d3c, 0x6cca876f, 0x2f0121e8, 0x87abf239, 0xc46054be,
    0x403bceed, 0x03f0686a, 0x488cfa4a, 0x0b475ccd, 0x8f1cc69e,
    0xccd76019, 0x19e4e2df, 0x5a2f4458, 0xde74de0b, 0x9dbf788c,
    0xd6c3eaac, 0x95084c2b, 0x1153d678, 0x529870ff, 0xf465465d,
    0xb7aee0da, 0x33f57a89, 0x703edc0e, 0x3b424e2e, 0x7889e8a9,
    0xfcd272fa, 0xbf19d47d, 0x6a2a56bb, 0x29e1f03c, 0xadba6a6f,
    0xee71cce8, 0xa50d5ec8, 0xe6c6f84f, 0x629d621c, 0x2156c49b,
    0x89fc174a, 0xca37b1cd, 0x4e6c2b9e, 0x0da78d19, 0x46db1f39,
    0x0510b9be, 0x814b23ed, 0xc280856a, 0x17b307ac, 0x5478a12b,
    0xd0233b78, 0x93e89dff, 0xd8940fdf, 0x9b5fa958, 0x1f04330b,
    0x5ccf958c, 0x0e57e573, 0x4d9c43f4, 0xc9c7d9a7, 0x8a0c7f20,
    0xc170ed00, 0x82bb4b87, 0x06e0d1d4, 0x452b7753, 0x9018f595,
    0xd3d35312, 0x5788c941, 0x14436fc6, 0x5f3ffde6, 0x1cf45b61,
    0x98afc132, 0xdb6467b5, 0x73ceb464, 0x300512e3, 0xb45e88b0,
    0xf7952e37, 0xbce9bc17, 0xff221a90, 0x7b7980c3, 0x38b22644,
    0xed81a482, 0xae4a0205, 0x2a119856, 0x69da3ed1, 0x22a6acf1,
    0x616d0a76, 0xe5369025, 0xa6fd36a2, 0xe8cb8cba, 0xab002a3d,
    0x2f5bb06e, 0x6c9016e9, 0x27ec84c9, 0x6427224e, 0xe07cb81d,
    0xa3b71e9a, 0x76849c5c, 0x354f3adb, 0xb114a088, 0xf2df060f,
    0xb9a3942f, 0xfa6832a8, 0x7e33a8fb, 0x3df80e7c, 0x9552ddad,
    0xd6997b2a, 0x52c2e179, 0x110947fe, 0x5a75d5de, 0x19be7359,
    0x9de5e90a, 0xde2e4f8d, 0x0b1dcd4b, 0x48d66bcc, 0xcc8df19f,
    0x8f465718, 0xc43ac538, 0x87f163bf, 0x03aaf9ec, 0x40615f6b,
    0x12f92f94, 0x51328913, 0xd5691340, 0x96a2b5c7, 0xddde27e7,
    0x9e158160, 0x1a4e1b33, 0x5985bdb4, 0x8cb63f72, 0xcf7d99f5,
    0x4b2603a6, 0x08eda521, 0x43913701, 0x005a9186, 0x84010bd5,
    0xc7caad52, 0x6f607e83, 0x2cabd804, 0xa8f04257, 0xeb3be4d0,
    0xa04776f0, 0xe38cd077, 0x67d74a24, 0x241ceca3, 0xf12f6e65,
    0xb2e4c8e2, 0x36bf52b1, 0x7574f436, 0x3e086616, 0x7dc3c091,
    0xf9985ac2, 0xba53fc45, 0x1caecae7, 0x5f656c60, 0xdb3ef633,
    0x98f550b4, 0xd389c294, 0x90426413, 0x1419fe40, 0x57d258c7,
    0x82e1da01, 0xc12a7c86, 0x4571e6d5, 0x06ba4052, 0x4dc6d272,
    0x0e0d74f5, 0x8a56eea6, 0xc99d4821, 0x61379bf0, 0x22fc3d77,
    0xa6a7a724, 0xe56c01a3, 0xae109383, 0xeddb3504, 0x6980af57,
    0x2a4b09d0, 0xff788b16, 0xbcb32d91, 0x38e8b7c2, 0x7b231145,
    0x305f8365, 0x739425e2, 0xf7cfbfb1, 0xb4041936, 0xe69c69c9,
    0xa557cf4e, 0x210c551d, 0x62c7f39a, 0x29bb61ba, 0x6a70c73d,
    0xee2b5d6e, 0xade0fbe9, 0x78d3792f, 0x3b18dfa8, 0xbf4345fb,
    0xfc88e37c, 0xb7f4715c, 0xf43fd7db, 0x70644d88, 0x33afeb0f,
    0x9b0538de, 0xd8ce9e59, 0x5c95040a, 0x1f5ea28d, 0x542230ad,
    0x17e9962a, 0x93b20c79, 0xd079aafe, 0x054a2838, 0x46818ebf,
    0xc2da14ec, 0x8111b26b, 0xca6d204b, 0x89a686cc, 0x0dfd1c9f,
    0x4e36ba18},
   {0x00000000, 0xe1b652ef, 0x836bd405, 0x62dd86ea, 0x06d7a80b,
    0xe761fae4, 0x85bc7c0e, 0x640a2ee1, 0x0cae5117, 0xed1803f8,
    0x8fc58512, 0x6e73d7fd, 0x0a79f91c, 0xebcfabf3, 0x89122d19,
    0x68a47ff6, 0x185ca32e, 0xf9eaf1c1, 0x9b37772b, 0x7a8125c4,
    0x1e8b0b25, 0xff3d59ca, 0x9de0df20, 0x7c568dcf, 0x14f2f239,
    0xf544a0d6, 0x9799263c, 0x762f74d3, 0x12255a32, 0xf39308dd,
    0x914e8e37, 0x70f8dcd8, 0x30b8465d, 0xd10e14b2, 0xb3d39258,
    0x5265c0b7, 0x366fee56, 0xd7d9bcb9, 0xb5043a53, 0x54b268bc,
    0x3c16174a, 0xdda045a5, 0xbf7dc34f, 0x5ecb91a0, 0x3ac1bf41,
    0xdb77edae, 0xb9aa6b44, 0x581c39ab, 0x28e4e573, 0xc952b79c,
    0xab8f3176, 0x4a396399, 0x2e334d78, 0xcf851f97, 0xad58997d,
    0x4ceecb92, 0x244ab464, 0xc5fce68b, 0xa7216061, 0x4697328e,
    0x229d1c6f, 0xc32b4e80, 0xa1f6c86a, 0x40409a85, 0x60708dba,
    0x81c6df55, 0xe31b59bf, 0x02ad0b50, 0x66a725b1, 0x8711775e,
    0xe5ccf1b4, 0x047aa35b, 0x6cdedcad, 0x8d688e42, 0xefb508a8,
    0x0e035a47, 0x6a0974a6, 0x8bbf2649, 0xe962a0a3, 0x08d4f24c,
    0x782c2e94, 0x999a7c7b, 0xfb47fa91, 0x1af1a87e, 0x7efb869f,
    0x9f4dd470, 0xfd90529a, 0x1c260075, 0x74827f83, 0x95342d6c,
    0xf7e9ab86, 0x165ff969, 0x7255d788, 0x93e38567, 0xf13e038d,
    0x10885162, 0x50c8cbe7, 0xb17e9908, 0xd3a31fe2, 0x32154d0d,
    0x561f63ec, 0xb7a93103, 0xd574b7e9, 0x34c2e506, 0x5c669af0,
    0xbdd0c81f, 0xdf0d4ef5, 0x3ebb1c1a, 0x5ab132fb, 0xbb076014,
    0xd9dae6fe, 0x386cb411, 0x489468c9, 0xa9223a26, 0xcbffbccc,
    0x2a49ee23, 0x4e43c0c2, 0xaff5922d, 0xcd2814c7, 0x2c9e4628,
    0x443a39de, 0xa58c6b31, 0xc751eddb, 0x26e7bf34, 0x42ed91d5,
    0xa35bc33a, 0xc18645d0, 0x2030173f, 0x81e66bae, 0x60503941,
    0x028dbfab, 0xe33bed44, 0x8731c3a5, 0x6687914a, 0x045a17a0,
    0xe5ec454f, 0x8d483ab9, 0x6cfe6856, 0x0e23eebc, 0xef95bc53,
    0x8b9f92b2, 0x6a29c05d, 0x08f446b7, 0xe9421458, 0x99bac880,
    0x780c9a6f, 0x1ad11c85, 0xfb674e6a, 0x9f6d608b, 0x7edb3264,
    0x1c06b48e, 0xfdb0e661, 0x95149997, 0x74a2cb78, 0x167f4d92,
    0xf7c91f7d, 0x93c3319c, 0x72756373, 0x10a8e599, 0xf11eb776,
    0xb15e2df3, 0x50e87f1c, 0x3235f9f6, 0xd383ab19, 0xb78985f8,
    0x563fd717, 0x34e251fd, 0xd5540312, 0xbdf07ce4, 0x5c462e0b,
    0x3e9ba8e1, 0xdf2dfa0e, 0xbb27d4ef, 0x5a918600, 0x384c00ea,
    0xd9fa5205, 0xa9028edd, 0x48b4dc32, 0x2a695ad8, 0xcbdf0837,
    0xafd526d6, 0x4e637439, 0x2cbef2d3, 0xcd08a03c, 0xa5acdfca,
    0x441a8d25, 0x26c70bcf, 0xc7715920, 0xa37b77c1, 0x42cd252e,
    0x2010a3c4, 0xc1a6f12b, 0xe196e614, 0x0020b4fb, 0x62fd3211,
    0x834b60fe, 0xe7414e1f, 0x06f71cf0, 0x642a9a1a, 0x859cc8f5,
    0xed38b703, 0x0c8ee5ec, 0x6e536306, 0x8fe531e9, 0xebef1f08,
    0x0a594de7, 0x6884cb0d, 0x893299e2, 0xf9ca453a, 0x187c17d5,
    0x7aa1913f, 0x9b17c3d0, 0xff1ded31, 0x1eabbfde, 0x7c763934,
    0x9dc06bdb, 0xf564142d, 0x14d246c2, 0x760fc028, 0x97b992c7,
    0xf3b3bc26, 0x1205eec9, 0x70d86823, 0x916e3acc, 0xd12ea049,
    0x3098f2a6, 0x5245744c, 0xb3f326a3, 0xd7f90842, 0x364f5aad,
    0x5492dc47, 0xb5248ea8, 0xdd80f15e, 0x3c36a3b1, 0x5eeb255b,
    0xbf5d77b4, 0xdb575955, 0x3ae10bba, 0x583c8d50, 0xb98adfbf,
    0xc9720367, 0x28c45188, 0x4a19d762, 0xabaf858d, 0xcfa5ab6c,
    0x2e13f983, 0x4cce7f69, 0xad782d86, 0xc5dc5270, 0x246a009f,
    0x46b78675, 0xa701d49a, 0xc30bfa7b, 0x22bda894, 0x40602e7e,
    0xa1d67c91},
   {0x00000000, 0x5880e2d7, 0xf106b474, 0xa98656a3, 0xe20d68e9,
    0xba8d8a3e, 0x130bdc9d, 0x4b8b3e4a, 0x851da109, 0xdd9d43de,
    0x741b157d, 0x2c9bf7aa, 0x6710c9e0, 0x3f902b37, 0x96167d94,
    0xce969f43, 0x0a3b4213, 0x52bba0c4, 0xfb3df667, 0xa3bd14b0,
    0xe8362afa, 0xb0b6c82d, 0x19309e8e, 0x41b07c59, 0x8f26e31a,
    0xd7a601cd, 0x7e20576e, 0x26a0b5b9, 0x6d2b8bf3, 0x35ab6924,
    0x9c2d3f87, 0xc4addd50, 0x14768426, 0x4cf666f1, 0xe5703052,
    0xbdf0d285, 0xf67beccf, 0xaefb0e18, 0x077d58bb, 0x5ffdba6c,
    0x916b252f, 0xc9ebc7f8, 0x606d915b, 0x38ed738c, 0x73664dc6,
    0x2be6af11, 0x8260f9b2, 0xdae01b65, 0x1e4dc635, 0x46cd24e2,
    0xef4b7241, 0xb7cb9096, 0xfc40aedc, 0xa4c04c0b, 0x0d461aa8,
    0x55c6f87f, 0x9b50673c, 0xc3d085eb, 0x6a56d348, 0x32d6319f,
    0x795d0fd5, 0x21dded02, 0x885bbba1, 0xd0db5976, 0x28ec084d,
    0x706cea9a, 0xd9eabc39, 0x816a5eee, 0xcae160a4, 0x92618273,
    0x3be7d4d0, 0x63673607, 0xadf1a944, 0xf5714b93, 0x5cf71d30,
    0x0477ffe7, 0x4ffcc1ad, 0x177c237a, 0xbefa75d9, 0xe67a970e,
    0x22d74a5e, 0x7a57a889, 0xd3d1fe2a, 0x8b511cfd, 0xc0da22b7,
    0x985ac060, 0x31dc96c3, 0x695c7414, 0xa7caeb57, 0xff4a0980,
    0x56cc5f23, 0x0e4cbdf4, 0x45c783be, 0x1d476169, 0xb4c137ca,
    0xec41d51d, 0x3c9a8c6b, 0x641a6ebc, 0xcd9c381f, 0x951cdac8,
    0xde97e482, 0x86170655, 0x2f9150f6, 0x7711b221, 0xb9872d62,
    0xe107cfb5, 0x48819916, 0x10017bc1, 0x5b8a458b, 0x030aa75c,
    0xaa8cf1ff, 0xf20c1328, 0x36a1ce78, 0x6e212caf, 0xc7a77a0c,
    0x9f2798db, 0xd4aca691, 0x8c2c4446, 0x25aa12e5, 0x7d2af032,
    0xb3bc6f71, 0xeb3c8da6, 0x42badb05, 0x1a3a39d2, 0x51b10798,
    0x0931e54f, 0xa0b7b3ec, 0xf837513b, 0x50d8119a, 0x0858f34d,
    0xa1dea5ee, 0xf95e4739, 0xb2d57973, 0xea559ba4, 0x43d3cd07,
    0x1b532fd0, 0xd5c5b093, 0x8d455244, 0x24c304e7, 0x7c43e630,
    0x37c8d87a, 0x6f483aad, 0xc6ce6c0e, 0x9e4e8ed9, 0x5ae35389,
    0x0263b15e, 0xabe5e7fd, 0xf365052a, 0xb8ee3b60, 0xe06ed9b7,
    0x49e88f14, 0x11686dc3, 0xdffef280, 0x877e1057, 0x2ef846f4,
    0x7678a423, 0x3df39a69, 0x657378be, 0xccf52e1d, 0x9475ccca,
    0x44ae95bc, 0x1c2e776b, 0xb5a821c8, 0xed28c31f, 0xa6a3fd55,
    0xfe231f82, 0x57a54921, 0x0f25abf6, 0xc1b334b5, 0x9933d662,
    0x30b580c1, 0x68356216, 0x23be5c5c, 0x7b3ebe8b, 0xd2b8e828,
    0x8a380aff, 0x4e95d7af, 0x16153578, 0xbf9363db, 0xe713810c,
    0xac98bf46, 0xf4185d91, 0x5d9e0b32, 0x051ee9e5, 0xcb8876a6,
    0x93089471, 0x3a8ec2d2, 0x620e2005, 0x29851e4f, 0x7105fc98,
    0xd883aa3b, 0x800348ec, 0x783419d7, 0x20b4fb00, 0x8932ada3,
    0xd1b24f74, 0x9a39713e, 0xc2b993e9, 0x6b3fc54a, 0x33bf279d,
    0xfd29b8de, 0xa5a95a09, 0x0c2f0caa, 0x54afee7d, 0x1f24d037,
    0x47a432e0, 0xee226443, 0xb6a28694, 0x720f5bc4, 0x2a8fb913,
    0x8309efb0, 0xdb890d67, 0x9002332d, 0xc882d1fa, 0x61048759,
    0x3984658e, 0xf712facd, 0xaf92181a, 0x06144eb9, 0x5e94ac6e,
    0x151f9224, 0x4d9f70f3, 0xe4192650, 0xbc99c487, 0x6c429df1,
    0x34c27f26, 0x9d442985, 0xc5c4cb52, 0x8e4ff518, 0xd6cf17cf,
    0x7f49416c, 0x27c9a3bb, 0xe95f3cf8, 0xb1dfde2f, 0x1859888c,
    0x40d96a5b, 0x0b525411, 0x53d2b6c6, 0xfa54e065, 0xa2d402b2,
    0x6679dfe2, 0x3ef93d35, 0x977f6b96, 0xcfff8941, 0x8474b70b,
    0xdcf455dc, 0x7572037f, 0x2df2e1a8, 0xe3647eeb, 0xbbe49c3c,
    0x1262ca9f, 0x4ae22848, 0x01691602, 0x59e9f4d5, 0xf06fa276,
    0xa8ef40a1},
   {0x00000000, 0x463b6765, 0x8c76ceca, 0xca4da9af, 0x59ebed4e,
    0x1fd08a2b, 0xd59d2384, 0x93a644e1, 0xb2d6db9d, 0xf4edbcf8,
    0x3ea01557, 0x789b7232, 0xeb3d36d3, 0xad0651b6, 0x674bf819,
    0x21709f7c, 0x25abc6e0, 0x6390a185, 0xa9dd082a, 0xefe66f4f,
    0x7c402bae, 0x3a7b4ccb, 0xf036e564, 0xb60d8201, 0x977d1d7d,
    0xd1467a18, 0x1b0bd3b7, 0x5d30b4d2, 0xce96f033, 0x88ad9756,
    0x42e03ef9, 0x04db599c, 0x0b50fc1a, 0x4d6b9b7f, 0x872632d0,
    0xc11d55b5, 0x52bb1154, 0x14807631, 0xdecddf9e, 0x98f6b8fb,
    0xb9862787, 0xffbd40e2, 0x35f0e94d, 0x73cb8e28, 0xe06dcac9,
    0xa656adac, 0x6c1b0403, 0x2a206366, 0x2efb3afa, 0x68c05d9f,
    0xa28df430, 0xe4b69355, 0x7710d7b4, 0x312bb0d1, 0xfb66197e,
    0xbd5d7e1b, 0x9c2de167, 0xda168602, 0x105b2fad, 0x566048c8,
    0xc5c60c29, 0x83fd6b4c, 0x49b0c2e3, 0x0f8ba586, 0x16a0f835,
    0x509b9f50, 0x9ad636ff, 0xdced519a, 0x4f4b157b, 0x0970721e,
    0xc33ddbb1, 0x8506bcd4, 0xa47623a8, 0xe24d44cd, 0x2800ed62,
    0x6e3b8a07, 0xfd9dcee6, 0xbba6a983, 0x71eb002c, 0x37d06749,
    0x330b3ed5, 0x753059b0, 0xbf7df01f, 0xf946977a, 0x6ae0d39b,
    0x2cdbb4fe, 0xe6961d51, 0xa0ad7a34, 0x81dde548, 0xc7e6822d,
    0x0dab2b82, 0x4b904ce7, 0xd8360806, 0x9e0d6f63, 0x5440c6cc,
    0x127ba1a9, 0x1df0042f, 0x5bcb634a, 0x9186cae5, 0xd7bdad80,
    0x441be961, 0x02208e04, 0xc86d27ab, 0x8e5640ce, 0xaf26dfb2,
    0xe91db8d7, 0x23501178, 0x656b761d, 0xf6cd32fc, 0xb0f65599,
    0x7abbfc36, 0x3c809b53, 0x385bc2cf, 0x7e60a5aa, 0xb42d0c05,
    0xf2166b60, 0x61b02f81, 0x278b48e4, 0xedc6e14b, 0xabfd862e,
    0x8a8d1952, 0xccb67e37, 0x06fbd798, 0x40c0b0fd, 0xd366f41c,
    0x955d9379, 0x5f103ad6, 0x192b5db3, 0x2c40f16b, 0x6a7b960e,
    0xa0363fa1, 0xe60d58c4, 0x75ab1c25, 0x33907b40, 0xf9ddd2ef,
    0xbfe6b58a, 0x9e962af6, 0xd8ad4d93, 0x12e0e43c, 0x54db8359,
    0xc77dc7b8, 0x8146a0dd, 0x4b0b0972, 0x0d306e17, 0x09eb378b,
    0x4fd050ee, 0x859df941, 0xc3a69e24, 0x5000dac5, 0x163bbda0,
    0xdc76140f, 0x9a4d736a, 0xbb3dec16, 0xfd068b73, 0x374b22dc,
    0x717045b9, 0xe2d60158, 0xa4ed663d, 0x6ea0cf92, 0x289ba8f7,
    0x27100d71, 0x612b6a14, 0xab66c3bb, 0xed5da4de, 0x7efbe03f,
    0x38c0875a, 0xf28d2ef5, 0xb4b64990, 0x95c6d6ec, 0xd3fdb189,
    0x19b01826, 0x5f8b7f43, 0xcc2d3ba2, 0x8a165cc7, 0x405bf568,
    0x0660920d, 0x02bbcb91, 0x4480acf4, 0x8ecd055b, 0xc8f6623e,
    0x5b5026df, 0x1d6b41ba, 0xd726e815, 0x911d8f70, 0xb06d100c,
    0xf6567769, 0x3c1bdec6, 0x7a20b9a3, 0xe986fd42, 0xafbd9a27,
    0x65f03388, 0x23cb54ed, 0x3ae0095e, 0x7cdb6e3b, 0xb696c794,
    0xf0ada0f1, 0x630be410, 0x25308375, 0xef7d2ada, 0xa9464dbf,
    0x8836d2c3, 0xce0db5a6, 0x04401c09, 0x427b7b6c, 0xd1dd3f8d,
    0x97e658e8, 0x5dabf147, 0x1b909622, 0x1f4bcfbe, 0x5970a8db,
    0x933d0174, 0xd5066611, 0x46a022f0, 0x009b4595, 0xcad6ec3a,
    0x8ced8b5f, 0xad9d1423, 0xeba67346, 0x21ebdae9, 0x67d0bd8c,
    0xf476f96d, 0xb24d9e08, 0x780037a7, 0x3e3b50c2, 0x31b0f544,
    0x778b9221, 0xbdc63b8e, 0xfbfd5ceb, 0x685b180a, 0x2e607f6f,
    0xe42dd6c0, 0xa216b1a5, 0x83662ed9, 0xc55d49bc, 0x0f10e013,
    0x492b8776, 0xda8dc397, 0x9cb6a4f2, 0x56fb0d5d, 0x10c06a38,
    0x141b33a4, 0x522054c1, 0x986dfd6e, 0xde569a0b, 0x4df0deea,
    0x0bcbb98f, 0xc1861020, 0x87bd7745, 0xa6cde839, 0xe0f68f5c,
    0x2abb26f3, 0x6c804196, 0xff260577, 0xb91d6212, 0x7350cbbd,
    0x356bacd8}};

#endif

#endif

#if N == 6

#if W == 8

local const z_crc_t FAR crc_braid_table[][256] = {
   {0x00000000, 0x3db1ecdc, 0x7b63d9b8, 0x46d23564, 0xf6c7b370,
    0xcb765fac, 0x8da46ac8, 0xb0158614, 0x36fe60a1, 0x0b4f8c7d,
    0x4d9db919, 0x702c55c5, 0xc039d3d1, 0xfd883f0d, 0xbb5a0a69,
    0x86ebe6b5, 0x6dfcc142, 0x504d2d9e, 0x169f18fa, 0x2b2ef426,
    0x9b3b7232, 0xa68a9eee, 0xe058ab8a, 0xdde94756, 0x5b02a1e3,
    0x66b34d3f, 0x2061785b, 0x1dd09487, 0xadc51293, 0x9074fe4f,
    0xd6a6cb2b, 0xeb1727f7, 0xdbf98284, 0xe6486e58, 0xa09a5b3c,
    0x9d2bb7e0, 0x2d3e31f4, 0x108fdd28, 0x565de84c, 0x6bec0490,
    0xed07e225, 0xd0b60ef9, 0x96643b9d, 0xabd5d741, 0x1bc05155,
    0x2671bd89, 0x60a388ed, 0x5d126431, 0xb60543c6, 0x8bb4af1a,
    0xcd669a7e, 0xf0d776a2, 0x40c2f0b6, 0x7d731c6a, 0x3ba1290e,
    0x0610c5d2, 0x80fb2367, 0xbd4acfbb, 0xfb98fadf, 0xc6291603,
    0x763c9017, 0x4b8d7ccb, 0x0d5f49af, 0x30eea573, 0x6c820349,
    0x5133ef95, 0x17e1daf1, 0x2a50362d, 0x9a45b039, 0xa7f45ce5,
    0xe1266981, 0xdc97855d, 0x5a7c63e8, 0x67cd8f34, 0x211fba50,
    0x1cae568c, 0xacbbd098, 0x910a3c44, 0xd7d80920, 0xea69e5fc,
    0x017ec20b, 0x3ccf2ed7, 0x7a1d1bb3, 0x47acf76f, 0xf7b9717b,
    0xca089da7, 0x8cdaa8c3, 0xb16b441f, 0x3780a2aa, 0x0a314e76,
    0x4ce37b12, 0x715297ce, 0xc14711da, 0xfcf6fd06, 0xba24c862,
    0x879524be, 0xb77b81cd, 0x8aca6d11, 0xcc185875, 0xf1a9b4a9,
    0x41bc32bd, 0x7c0dde61, 0x3adfeb05, 0x076e07d9, 0x8185e16c,
    0xbc340db0, 0xfae638d4, 0xc757d408, 0x7742521c, 0x4af3bec0,
    0x0c218ba4, 0x31906778, 0xda87408f, 0xe736ac53, 0xa1e49937,
    0x9c5575eb, 0x2c40f3ff, 0x11f11f23, 0x57232a47, 0x6a92c69b,
    0xec79202e, 0xd1c8ccf2, 0x971af996, 0xaaab154a, 0x1abe935e,
    0x270f7f82, 0x61dd4ae6, 0x5c6ca63a, 0xd9040692, 0xe4b5ea4e,
    0xa267df2a, 0x9fd633f6, 0x2fc3b5e2, 0x1272593e, 0x54a06c5a,
    0x69118086, 0xeffa6633, 0xd24b8aef, 0x9499bf8b, 0xa9285357,
    0x193dd543, 0x248c399f, 0x625e0cfb, 0x5fefe027, 0xb4f8c7d0,
    0x89492b0c, 0xcf9b1e68, 0xf22af2b4, 0x423f74a0, 0x7f8e987c,
    0x395cad18, 0x04ed41c4, 0x8206a771, 0xbfb74bad, 0xf9657ec9,
    0xc4d49215, 0x74c11401, 0x4970f8dd, 0x0fa2cdb9, 0x32132165,
    0x02fd8416, 0x3f4c68ca, 0x799e5dae, 0x442fb172, 0xf43a3766,
    0xc98bdbba, 0x8f59eede, 0xb2e80202, 0x3403e4b7, 0x09b2086b,
    0x4f603d0f, 0x72d1d1d3, 0xc2c457c7, 0xff75bb1b, 0xb9a78e7f,
    0x841662a3, 0x6f014554, 0x52b0a988, 0x14629cec, 0x29d37030,
    0x99c6f624, 0xa4771af8, 0xe2a52f9c, 0xdf14c340, 0x59ff25f5,
    0x644ec929, 0x229cfc4d, 0x1f2d1091, 0xaf389685, 0x92897a59,
    0xd45b4f3d, 0xe9eaa3e1, 0xb58605db, 0x8837e907, 0xcee5dc63,
    0xf35430bf, 0x4341b6ab, 0x7ef05a77, 0x38226f13, 0x059383cf,
    0x8378657a, 0xbec989a6, 0xf81bbcc2, 0xc5aa501e, 0x75bfd60a,
    0x480e3ad6, 0x0edc0fb2, 0x336de36e, 0xd87ac499, 0xe5cb2845,
    0xa3191d21, 0x9ea8f1fd, 0x2ebd77e9, 0x130c9b35, 0x55deae51,
    0x686f428d, 0xee84a438, 0xd33548e4, 0x95e77d80, 0xa856915c,
    0x18431748, 0x25f2fb94, 0x6320cef0, 0x5e91222c, 0x6e7f875f,
    0x53ce6b83, 0x151c5ee7, 0x28adb23b, 0x98b8342f, 0xa509d8f3,
    0xe3dbed97, 0xde6a014b, 0x5881e7fe, 0x65300b22, 0x23e23e46,
    0x1e53d29a, 0xae46548e, 0x93f7b852, 0xd5258d36, 0xe89461ea,
    0x0383461d, 0x3e32aac1, 0x78e09fa5, 0x45517379, 0xf544f56d,
    0xc8f519b1, 0x8e272cd5, 0xb396c009, 0x357d26bc, 0x08ccca60,
    0x4e1eff04, 0x73af13d8, 0xc3ba95cc, 0xfe0b7910, 0xb8d94c74,
    0x8568a0a8},
   {0x00000000, 0x69790b65, 0xd2f216ca, 0xbb8b1daf, 0x7e952bd5,
    0x17ec20b0, 0xac673d1f, 0xc51e367a, 0xfd2a57aa, 0x94535ccf,
    0x2fd84160, 0x46a14a05, 0x83bf7c7f, 0xeac6771a, 0x514d6ab5,
    0x383461d0, 0x2125a915, 0x485ca270, 0xf3d7bfdf, 0x9aaeb4ba,
    0x5fb082c0, 0x36c989a5, 0x8d42940a, 0xe43b9f6f, 0xdc0ffebf,
    0xb576f5da, 0x0efde875, 0x6784e310, 0xa29ad56a, 0xcbe3de0f,
    0x7068c3a0, 0x1911c8c5, 0x424b522a, 0x2b32594f, 0x90b944e0,
    0xf9c04f85, 0x3cde79ff, 0x55a7729a, 0xee2c6f35, 0x87556450,
    0xbf610580, 0xd6180ee5, 0x6d93134a, 0x04ea182f, 0xc1f42e55,
    0xa88d2530, 0x1306389f, 0x7a7f33fa, 0x636efb3f, 0x0a17f05a,
    0xb19cedf5, 0xd8e5e690, 0x1dfbd0ea, 0x7482db8f, 0xcf09c620,
    0xa670cd45, 0x9e44ac95, 0xf73da7f0, 0x4cb6ba5f, 0x25cfb13a,
    0xe0d18740, 0x89a88c25, 0x3223918a, 0x5b5a9aef, 0x8496a454,
    0xedefaf31, 0x5664b29e, 0x3f1db9fb, 0xfa038f81, 0x937a84e4,
    0x28f1994b, 0x4188922e, 0x79bcf3fe, 0x10c5f89b, 0xab4ee534,
    0xc237ee51, 0x0729d82b, 0x6e50d34e, 0xd5dbcee1, 0xbca2c584,
    0xa5b30d41, 0xccca0624, 0x77411b8b, 0x1e3810ee, 0xdb262694,
    0xb25f2df1, 0x09d4305e, 0x60ad3b3b, 0x58995aeb, 0x31e0518e,
    0x8a6b4c21, 0xe3124744, 0x260c713e, 0x4f757a5b, 0xf4fe67f4,
    0x9d876c91, 0xc6ddf67e, 0xafa4fd1b, 0x142fe0b4, 0x7d56ebd1,
    0xb848ddab, 0xd131d6ce, 0x6abacb61, 0x03c3c004, 0x3bf7a1d4,
    0x528eaab1, 0xe905b71e, 0x807cbc7b, 0x45628a01, 0x2c1b8164,
    0x97909ccb, 0xfee997ae, 0xe7f85f6b, 0x8e81540e, 0x350a49a1,
    0x5c7342c4, 0x996d74be, 0xf0147fdb, 0x4b9f6274, 0x22e66911,
    0x1ad208c1, 0x73ab03a4, 0xc8201e0b, 0xa159156e, 0x64472314,
    0x0d3e2871, 0xb6b535de, 0xdfcc3ebb, 0xd25c4ee9, 0xbb25458c,
    0x00ae5823, 0x69d75346, 0xacc9653c, 0xc5b06e59, 0x7e3b73f6,
    0x17427893, 0x2f761943, 0x460f1226, 0xfd840f89, 0x94fd04ec,
    0x51e33296, 0x389a39f3, 0x8311245c, 0xea682f39, 0xf379e7fc,
    0x9a00ec99, 0x218bf136, 0x48f2fa53, 0x8deccc29, 0xe495c74c,
    0x5f1edae3, 0x3667d186, 0x0e53b056, 0x672abb33, 0xdca1a69c,
    0xb5d8adf9, 0x70c69b83, 0x19bf90e6, 0xa2348d49, 0xcb4d862c,
    0x90171cc3, 0xf96e17a6, 0x42e50a09, 0x2b9c016c, 0xee823716,
    0x87fb3c73, 0x3c7021dc, 0x55092ab9, 0x6d3d4b69, 0x0444400c,
    0xbfcf5da3, 0xd6b656c6, 0x13a860bc, 0x7ad16bd9, 0xc15a7676,
    0xa8237d13, 0xb132b5d6, 0xd84bbeb3, 0x63c0a31c, 0x0ab9a879,
    0xcfa79e03, 0xa6de9566, 0x1d5588c9, 0x742c83ac, 0x4c18e27c,
    0x2561e919, 0x9eeaf4b6, 0xf793ffd3, 0x328dc9a9, 0x5bf4c2cc,
    0xe07fdf63, 0x8906d406, 0x56caeabd, 0x3fb3e1d8, 0x8438fc77,
    0xed41f712, 0x285fc168, 0x4126ca0d, 0xfaadd7a2, 0x93d4dcc7,
    0xabe0bd17, 0xc299b672, 0x7912abdd, 0x106ba0b8, 0xd57596c2,
    0xbc0c9da7, 0x07878008, 0x6efe8b6d, 0x77ef43a8, 0x1e9648cd,
    0xa51d5562, 0xcc645e07, 0x097a687d, 0x60036318, 0xdb887eb7,
    0xb2f175d2, 0x8ac51402, 0xe3bc1f67, 0x583702c8, 0x314e09ad,
    0xf4503fd7, 0x9d2934b2, 0x26a2291d, 0x4fdb2278, 0x1481b897,
    0x7df8b3f2, 0xc673ae5d, 0xaf0aa538, 0x6a149342, 0x036d9827,
    0xb8e68588, 0xd19f8eed, 0xe9abef3d, 0x80d2e458, 0x3b59f9f7,
    0x5220f292, 0x973ec4e8, 0xfe47cf8d, 0x45ccd222, 0x2cb5d947,
    0x35a41182, 0x5cdd1ae7, 0xe7560748, 0x8e2f0c2d, 0x4b313a57,
    0x22483132, 0x99c32c9d, 0xf0ba27f8, 0xc88e4628, 0xa1f74d4d,
    0x1a7c50e2, 0x73055b87, 0xb61b6dfd, 0xdf626698, 0x64e97b37,
    0x0d907052},
   {0x00000000, 0x7fc99b93, 0xff933726, 0x805aacb5, 0x2457680d,
    0x5b9ef39e, 0xdbc45f2b, 0xa40dc4b8, 0x48aed01a, 0x37674b89,
    0xb73de73c, 0xc8f47caf, 0x6cf9b817, 0x13302384, 0x936a8f31,
    0xeca314a2, 0x915da034, 0xee943ba7, 0x6ece9712, 0x11070c81,
    0xb50ac839, 0xcac353aa, 0x4a99ff1f, 0x3550648c, 0xd9f3702e,
    0xa63aebbd, 0x26604708, 0x59a9dc9b, 0xfda41823, 0x826d83b0,
    0x02372f05, 0x7dfeb496, 0xf9ca4629, 0x8603ddba, 0x0659710f,
    0x7990ea9c, 0xdd9d2e24, 0xa254b5b7, 0x220e1902, 0x5dc78291,
    0xb1649633, 0xcead0da0, 0x4ef7a115, 0x313e3a86, 0x9533fe3e,
    0xeafa65ad, 0x6aa0c918, 0x1569528b, 0x6897e61d, 0x175e7d8e,
    0x9704d13b, 0xe8cd4aa8, 0x4cc08e10, 0x33091583, 0xb353b936,
    0xcc9a22a5, 0x20393607, 0x5ff0ad94, 0xdfaa0121, 0xa0639ab2,
    0x046e5e0a, 0x7ba7c599, 0xfbfd692c, 0x8434f2bf, 0x28e58a13,
    0x572c1180, 0xd776bd35, 0xa8bf26a6, 0x0cb2e21e, 0x737b798d,
    0xf321d538, 0x8ce84eab, 0x604b5a09, 0x1f82c19a, 0x9fd86d2f,
    0xe011f6bc, 0x441c3204, 0x3bd5a997, 0xbb8f0522, 0xc4469eb1,
    0xb9b82a27, 0xc671b1b4, 0x462b1d01, 0x39e28692, 0x9def422a,
    0xe226d9b9, 0x627c750c, 0x1db5ee9f, 0xf116fa3d, 0x8edf61ae,
    0x0e85cd1b, 0x714c5688, 0xd5419230, 0xaa8809a3, 0x2ad2a516,
    0x551b3e85, 0xd12fcc3a, 0xaee657a9, 0x2ebcfb1c, 0x5175608f,
    0xf578a437, 0x8ab13fa4, 0x0aeb9311, 0x75220882, 0x99811c20,
    0xe64887b3, 0x66122b06, 0x19dbb095, 0xbdd6742d, 0xc21fefbe,
    0x4245430b, 0x3d8cd898, 0x40726c0e, 0x3fbbf79d, 0xbfe15b28,
    0xc028c0bb, 0x64250403, 0x1bec9f90, 0x9bb63325, 0xe47fa8b6,
    0x08dcbc14, 0x77152787, 0xf74f8b32, 0x888610a1, 0x2c8bd419,
    0x53424f8a, 0xd318e33f, 0xacd178ac, 0x51cb1426, 0x2e028fb5,
    0xae582300, 0xd191b893, 0x759c7c2b, 0x0a55e7b8, 0x8a0f4b0d,
    0xf5c6d09e, 0x1965c43c, 0x66ac5faf, 0xe6f6f31a, 0x993f6889,
    0x3d32ac31, 0x42fb37a2, 0xc2a19b17, 0xbd680084, 0xc096b412,
    0xbf5f2f81, 0x3f058334, 0x40cc18a7, 0xe4c1dc1f, 0x9b08478c,
    0x1b52eb39, 0x649b70aa, 0x88386408, 0xf7f1ff9b, 0x77ab532e,
    0x0862c8bd, 0xac6f0c05, 0xd3a69796, 0x53fc3b23, 0x2c35a0b0,
    0xa801520f, 0xd7c8c99c, 0x57926529, 0x285bfeba, 0x8c563a02,
    0xf39fa191, 0x73c50d24, 0x0c0c96b7, 0xe0af8215, 0x9f661986,
    0x1f3cb533, 0x60f52ea0, 0xc4f8ea18, 0xbb31718b, 0x3b6bdd3e,
    0x44a246ad, 0x395cf23b, 0x469569a8, 0xc6cfc51d, 0xb9065e8e,
    0x1d0b9a36, 0x62c201a5, 0xe298ad10, 0x9d513683, 0x71f22221,
    0x0e3bb9b2, 0x8e611507, 0xf1a88e94, 0x55a54a2c, 0x2a6cd1bf,
    0xaa367d0a, 0xd5ffe699, 0x792e9e35, 0x06e705a6, 0x86bda913,
    0xf9743280, 0x5d79f638, 0x22b06dab, 0xa2eac11e, 0xdd235a8d,
    0x31804e2f, 0x4e49d5bc, 0xce137909, 0xb1dae29a, 0x15d72622,
    0x6a1ebdb1, 0xea441104, 0x958d8a97, 0xe8733e01, 0x97baa592,
    0x17e00927, 0x682992b4, 0xcc24560c, 0xb3edcd9f, 0x33b7612a,
    0x4c7efab9, 0xa0ddee1b, 0xdf147588, 0x5f4ed93d, 0x208742ae,
    0x848a8616, 0xfb431d85, 0x7b19b130, 0x04d02aa3, 0x80e4d81c,
    0xff2d438f, 0x7f77ef3a, 0x00be74a9, 0xa4b3b011, 0xdb7a2b82,
    0x5b208737, 0x24e91ca4, 0xc84a0806, 0xb7839395, 0x37d93f20,
    0x4810a4b3, 0xec1d600b, 0x93d4fb98, 0x138e572d, 0x6c47ccbe,
    0x11b97828, 0x6e70e3bb, 0xee2a4f0e, 0x91e3d49d, 0x35ee1025,
    0x4a278bb6, 0xca7d2703, 0xb5b4bc90, 0x5917a832, 0x26de33a1,
    0xa6849f14, 0xd94d0487, 0x7d40c03f, 0x02895bac, 0x82d3f719,
    0xfd1a6c8a},
   {0x00000000, 0xa396284c, 0x9c5d56d9, 0x3fcb7e95, 0xe3cbabf3,
    0x405d83bf, 0x7f96fd2a, 0xdc00d566, 0x1ce651a7, 0xbf7079eb,
    0x80bb077e, 0x232d2f32, 0xff2dfa54, 0x5cbbd218, 0x6370ac8d,
    0xc0e684c1, 0x39cca34e, 0x9a5a8b02, 0xa591f597, 0x0607dddb,
    0xda0708bd, 0x799120f1, 0x465a5e64, 0xe5cc7628, 0x252af2e9,
    0x86bcdaa5, 0xb977a430, 0x1ae18c7c, 0xc6e1591a, 0x65777156,
    0x5abc0fc3, 0xf92a278f, 0x7399469c, 0xd00f6ed0, 0xefc41045,
    0x4c523809, 0x9052ed6f, 0x33c4c523, 0x0c0fbbb6, 0xaf9993fa,
    0x6f7f173b, 0xcce93f77, 0xf32241e2, 0x50b469ae, 0x8cb4bcc8,
    0x2f229484, 0x10e9ea11, 0xb37fc25d, 0x4a55e5d2, 0xe9c3cd9e,
    0xd608b30b, 0x759e9b47, 0xa99e4e21, 0x0a08666d, 0x35c318f8,
    0x965530b4, 0x56b3b475, 0xf5259c39, 0xcaeee2ac, 0x6978cae0,
    0xb5781f86, 0x16ee37ca, 0x2925495f, 0x8ab36113, 0xe7328d38,
    0x44a4a574, 0x7b6fdbe1, 0xd8f9f3ad, 0x04f926cb, 0xa76f0e87,
    0x98a47012, 0x3b32585e, 0xfbd4dc9f, 0x5842f4d3, 0x67898a46,
    0xc41fa20a, 0x181f776c, 0xbb895f20, 0x844221b5, 0x27d409f9,
    0xdefe2e76, 0x7d68063a, 0x42a378af, 0xe13550e3, 0x3d358585,
    0x9ea3adc9, 0xa168d35c, 0x02fefb10, 0xc2187fd1, 0x618e579d,
    0x5e452908, 0xfdd30144, 0x21d3d422, 0x8245fc6e, 0xbd8e82fb,
    0x1e18aab7, 0x94abcba4, 0x373de3e8, 0x08f69d7d, 0xab60b531,
    0x77606057, 0xd4f6481b, 0xeb3d368e, 0x48ab1ec2, 0x884d9a03,
    0x2bdbb24f, 0x1410ccda, 0xb786e496, 0x6b8631f0, 0xc81019bc,
    0xf7db6729, 0x544d4f65, 0xad6768ea, 0x0ef140a6, 0x313a3e33,
    0x92ac167f, 0x4eacc319, 0xed3aeb55, 0xd2f195c0, 0x7167bd8c,
    0xb181394d, 0x12171101, 0x2ddc6f94, 0x8e4a47d8, 0x524a92be,
    0xf1dcbaf2, 0xce17c467, 0x6d81ec2b, 0x15141c31, 0xb682347d,
    0x89494ae8, 0x2adf62a4, 0xf6dfb7c2, 0x55499f8e, 0x6a82e11b,
    0xc914c957, 0x09f24d96, 0xaa6465da, 0x95af1b4f, 0x36393303,
    0xea39e665, 0x49afce29, 0x7664b0bc, 0xd5f298f0, 0x2cd8bf7f,
    0x8f4e9733, 0xb085e9a6, 0x1313c1ea, 0xcf13148c, 0x6c853cc0,
    0x534e4255, 0xf0d86a19, 0x303eeed8, 0x93a8c694, 0xac63b801,
    0x0ff5904d, 0xd3f5452b, 0x70636d67, 0x4fa813f2, 0xec3e3bbe,
    0x668d5aad, 0xc51b72e1, 0xfad00c74, 0x59462438, 0x8546f15e,
    0x26d0d912, 0x191ba787, 0xba8d8fcb, 0x7a6b0b0a, 0xd9fd2346,
    0xe6365dd3, 0x45a0759f, 0x99a0a0f9, 0x3a3688b5, 0x05fdf620,
    0xa66bde6c, 0x5f41f9e3, 0xfcd7d1af, 0xc31caf3a, 0x608a8776,
    0xbc8a5210, 0x1f1c7a5c, 0x20d704c9, 0x83412c85, 0x43a7a844,
    0xe0318008, 0xdffafe9d, 0x7c6cd6d1, 0xa06c03b7, 0x03fa2bfb,
    0x3c31556e, 0x9fa77d22, 0xf2269109, 0x51b0b945, 0x6e7bc7d0,
    0xcdedef9c, 0x11ed3afa, 0xb27b12b6, 0x8db06c23, 0x2e26446f,
    0xeec0c0ae, 0x4d56e8e2, 0x729d9677, 0xd10bbe3b, 0x0d0b6b5d,
    0xae9d4311, 0x91563d84, 0x32c015c8, 0xcbea3247, 0x687c1a0b,
    0x57b7649e, 0xf4214cd2, 0x282199b4, 0x8bb7b1f8, 0xb47ccf6d,
    0x17eae721, 0xd70c63e0, 0x749a4bac, 0x4b513539, 0xe8c71d75,
    0x34c7c813, 0x9751e05f, 0xa89a9eca, 0x0b0cb686, 0x81bfd795,
    0x2229ffd9, 0x1de2814c, 0xbe74a900, 0x62747c66, 0xc1e2542a,
    0xfe292abf, 0x5dbf02f3, 0x9d598632, 0x3ecfae7e, 0x0104d0eb,
    0xa292f8a7, 0x7e922dc1, 0xdd04058d, 0xe2cf7b18, 0x41595354,
    0xb87374db, 0x1be55c97, 0x242e2202, 0x87b80a4e, 0x5bb8df28,
    0xf82ef764, 0xc7e589f1, 0x6473a1bd, 0xa495257c, 0x07030d30,
    0x38c873a5, 0x9b5e5be9, 0x475e8e8f, 0xe4c8a6c3, 0xdb03d856,
    0x7895f01a},
   {0x00000000, 0x2a283862, 0x545070c4, 0x7e7848a6, 0xa8a0e188,
    0x8288d9ea, 0xfcf0914c, 0xd6d8a92e, 0x8a30c551, 0xa018fd33,
    0xde60b595, 0xf4488df7, 0x229024d9, 0x08b81cbb, 0x76c0541d,
    0x5ce86c7f, 0xcf108ce3, 0xe538b481, 0x9b40fc27, 0xb168c445,
    0x67b06d6b, 0x4d985509, 0x33e01daf, 0x19c825cd, 0x452049b2,
    0x6f0871d0, 0x11703976, 0x3b580114, 0xed80a83a, 0xc7a89058,
    0xb9d0d8fe, 0x93f8e09c, 0x45501f87, 0x6f7827e5, 0x11006f43,
    0x3b285721, 0xedf0fe0f, 0xc7d8c66d, 0xb9a08ecb, 0x9388b6a9,
    0xcf60dad6, 0xe548e2b4, 0x9b30aa12, 0xb1189270, 0x67c03b5e,
    0x4de8033c, 0x33904b9a, 0x19b873f8, 0x8a409364, 0xa068ab06,
    0xde10e3a0, 0xf438dbc2, 0x22e072ec, 0x08c84a8e, 0x76b00228,
    0x5c983a4a, 0x00705635, 0x2a586e57, 0x542026f1, 0x7e081e93,
    0xa8d0b7bd, 0x82f88fdf, 0xfc80c779, 0xd6a8ff1b, 0x8aa03f0e,
    0xa088076c, 0xdef04fca, 0xf4d877a8, 0x2200de86, 0x0828e6e4,
    0x7650ae42, 0x5c789620, 0x0090fa5f, 0x2ab8c23d, 0x54c08a9b,
    0x7ee8b2f9, 0xa8301bd7, 0x821823b5, 0xfc606b13, 0xd6485371,
    0x45b0b3ed, 0x6f988b8f, 0x11e0c329, 0x3bc8fb4b, 0xed105265,
    0xc7386a07, 0xb94022a1, 0x93681ac3, 0xcf8076bc, 0xe5a84ede,
    0x9bd00678, 0xb1f83e1a, 0x67209734, 0x4d08af56, 0x3370e7f0,
    0x1958df92, 0xcff02089, 0xe5d818eb, 0x9ba0504d, 0xb188682f,
    0x6750c101, 0x4d78f963, 0x3300b1c5, 0x192889a7, 0x45c0e5d8,
    0x6fe8ddba, 0x1190951c, 0x3bb8ad7e, 0xed600450, 0xc7483c32,
    0xb9307494, 0x93184cf6, 0x00e0ac6a, 0x2ac89408, 0x54b0dcae,
    0x7e98e4cc, 0xa8404de2, 0x82687580, 0xfc103d26, 0xd6380544,
    0x8ad0693b, 0xa0f85159, 0xde8019ff, 0xf4a8219d, 0x227088b3,
    0x0858b0d1, 0x7620f877, 0x5c08c015, 0xce31785d, 0xe419403f,
    0x9a610899, 0xb04930fb, 0x669199d5, 0x4cb9a1b7, 0x32c1e911,
    0x18e9d173, 0x4401bd0c, 0x6e29856e, 0x1051cdc8, 0x3a79f5aa,
    0xeca15c84, 0xc68964e6, 0xb8f12c40, 0x92d91422, 0x0121f4be,
    0x2b09ccdc, 0x5571847a, 0x7f59bc18, 0xa9811536, 0x83a92d54,
    0xfdd165f2, 0xd7f95d90, 0x8b1131ef, 0xa139098d, 0xdf41412b,
    0xf5697949, 0x23b1d067, 0x0999e805, 0x77e1a0a3, 0x5dc998c1,
    0x8b6167da, 0xa1495fb8, 0xdf31171e, 0xf5192f7c, 0x23c18652,
    0x09e9be30, 0x7791f696, 0x5db9cef4, 0x0151a28b, 0x2b799ae9,
    0x5501d24f, 0x7f29ea2d, 0xa9f14303, 0x83d97b61, 0xfda133c7,
    0xd7890ba5, 0x4471eb39, 0x6e59d35b, 0x10219bfd, 0x3a09a39f,
    0xecd10ab1, 0xc6f932d3, 0xb8817a75, 0x92a94217, 0xce412e68,
    0xe469160a, 0x9a115eac, 0xb03966ce, 0x66e1cfe0, 0x4cc9f782,
    0x32b1bf24, 0x18998746, 0x44914753, 0x6eb97f31, 0x10c13797,
    0x3ae90ff5, 0xec31a6db, 0xc6199eb9, 0xb861d61f, 0x9249ee7d,
    0xcea18202, 0xe489ba60, 0x9af1f2c6, 0xb0d9caa4, 0x6601638a,
    0x4c295be8, 0x3251134e, 0x18792b2c, 0x8b81cbb0, 0xa1a9f3d2,
    0xdfd1bb74, 0xf5f98316, 0x23212a38, 0x0909125a, 0x77715afc,
    0x5d59629e, 0x01b10ee1, 0x2b993683, 0x55e17e25, 0x7fc94647,
    0xa911ef69, 0x8339d70b, 0xfd419fad, 0xd769a7cf, 0x01c158d4,
    0x2be960b6, 0x55912810, 0x7fb91072, 0xa961b95c, 0x8349813e,
    0xfd31c998, 0xd719f1fa, 0x8bf19d85, 0xa1d9a5e7, 0xdfa1ed41,
    0xf589d523, 0x23517c0d, 0x0979446f, 0x77010cc9, 0x5d2934ab,
    0xced1d437, 0xe4f9ec55, 0x9a81a4f3, 0xb0a99c91, 0x667135bf,
    0x4c590ddd, 0x3221457b, 0x18097d19, 0x44e11166, 0x6ec92904,
    0x10b161a2, 0x3a9959c0, 0xec41f0ee, 0xc669c88c, 0xb811802a,
    0x9239b848},
   {0x00000000, 0x4713f6fb, 0x8e27edf6, 0xc9341b0d, 0xc73eddad,
    0x802d2b56, 0x4919305b, 0x0e0ac6a0, 0x550cbd1b, 0x121f4be0,
    0xdb2b50ed, 0x9c38a616, 0x923260b6, 0xd521964d, 0x1c158d40,
    0x5b067bbb, 0xaa197a36, 0xed0a8ccd, 0x243e97c0, 0x632d613b,
    0x6d27a79b, 0x2a345160, 0xe3004a6d, 0xa413bc96, 0xff15c72d,
    0xb80631d6, 0x71322adb, 0x3621dc20, 0x382b1a80, 0x7f38ec7b,
    0xb60cf776, 0xf11f018d, 0x8f43f22d, 0xc85004d6, 0x01641fdb,
    0x4677e920, 0x487d2f80, 0x0f6ed97b, 0xc65ac276, 0x8149348d,
    0xda4f4f36, 0x9d5cb9cd, 0x5468a2c0, 0x137b543b, 0x1d71929b,
    0x5a626460, 0x93567f6d, 0xd4458996, 0x255a881b, 0x62497ee0,
    0xab7d65ed, 0xec6e9316, 0xe26455b6, 0xa577a34d, 0x6c43b840,
    0x2b504ebb, 0x70563500, 0x3745c3fb, 0xfe71d8f6, 0xb9622e0d,
    0xb768e8ad, 0xf07b1e56, 0x394f055b, 0x7e5cf3a0, 0xc5f6e21b,
    0x82e514e0, 0x4bd10fed, 0x0cc2f916, 0x02c83fb6, 0x45dbc94d,
    0x8cefd240, 0xcbfc24bb, 0x90fa5f00, 0xd7e9a9fb, 0x1eddb2f6,
    0x59ce440d, 0x57c482ad, 0x10d77456, 0xd9e36f5b, 0x9ef099a0,
    0x6fef982d, 0x28fc6ed6, 0xe1c875db, 0xa6db8320, 0xa8d14580,
    0xefc2b37b, 0x26f6a876, 0x61e55e8d, 0x3ae32536, 0x7df0d3cd,
    0xb4c4c8c0, 0xf3d73e3b, 0xfdddf89b, 0xbace0e60, 0x73fa156d,
    0x34e9e396, 0x4ab51036, 0x0da6e6cd, 0xc492fdc0, 0x83810b3b,
    0x8d8bcd9b, 0xca983b60, 0x03ac206d, 0x44bfd696, 0x1fb9ad2d,
    0x58aa5bd6, 0x919e40db, 0xd68db620, 0xd8877080, 0x9f94867b,
    0x56a09d76, 0x11b36b8d, 0xe0ac6a00, 0xa7bf9cfb, 0x6e8b87f6,
    0x2998710d, 0x2792b7ad, 0x60814156, 0xa9b55a5b, 0xeea6aca0,
    0xb5a0d71b, 0xf2b321e0, 0x3b873aed, 0x7c94cc16, 0x729e0ab6,
    0x358dfc4d, 0xfcb9e740, 0xbbaa11bb, 0x509cc277, 0x178f348c,
    0xdebb2f81, 0x99a8d97a, 0x97a21fda, 0xd0b1e921, 0x1985f22c,
    0x5e9604d7, 0x05907f6c, 0x42838997, 0x8bb7929a, 0xcca46461,
    0xc2aea2c1, 0x85bd543a, 0x4c894f37, 0x0b9ab9cc, 0xfa85b841,
    0xbd964eba, 0x74a255b7, 0x33b1a34c, 0x3dbb65ec, 0x7aa89317,
    0xb39c881a, 0xf48f7ee1, 0xaf89055a, 0xe89af3a1, 0x21aee8ac,
    0x66bd1e57, 0x68b7d8f7, 0x2fa42e0c, 0xe6903501, 0xa183c3fa,
    0xdfdf305a, 0x98ccc6a1, 0x51f8ddac, 0x16eb2b57, 0x18e1edf7,
    0x5ff21b0c, 0x96c60001, 0xd1d5f6fa, 0x8ad38d41, 0xcdc07bba,
    0x04f460b7, 0x43e7964c, 0x4ded50ec, 0x0afea617, 0xc3cabd1a,
    0x84d94be1, 0x75c64a6c, 0x32d5bc97, 0xfbe1a79a, 0xbcf25161,
    0xb2f897c1, 0xf5eb613a, 0x3cdf7a37, 0x7bcc8ccc, 0x20caf777,
    0x67d9018c, 0xaeed1a81, 0xe9feec7a, 0xe7f42ada, 0xa0e7dc21,
    0x69d3c72c, 0x2ec031d7, 0x956a206c, 0xd279d697, 0x1b4dcd9a,
    0x5c5e3b61, 0x5254fdc1, 0x15470b3a, 0xdc731037, 0x9b60e6cc,
    0xc0669d77, 0x87756b8c, 0x4e417081, 0x0952867a, 0x075840da,
    0x404bb621, 0x897fad2c, 0xce6c5bd7, 0x3f735a5a, 0x7860aca1,
    0xb154b7ac, 0xf6474157, 0xf84d87f7, 0xbf5e710c, 0x766a6a01,
    0x31799cfa, 0x6a7fe741, 0x2d6c11ba, 0xe4580ab7, 0xa34bfc4c,
    0xad413aec, 0xea52cc17, 0x2366d71a, 0x647521e1, 0x1a29d241,
    0x5d3a24ba, 0x940e3fb7, 0xd31dc94c, 0xdd170fec, 0x9a04f917,
    0x5330e21a, 0x142314e1, 0x4f256f5a, 0x083699a1, 0xc10282ac,
    0x86117457, 0x881bb2f7, 0xcf08440c, 0x063c5f01, 0x412fa9fa,
    0xb030a877, 0xf7235e8c, 0x3e174581, 0x7904b37a, 0x770e75da,
    0x301d8321, 0xf929982c, 0xbe3a6ed7, 0xe53c156c, 0xa22fe397,
    0x6b1bf89a, 0x2c080e61, 0x2202c8c1, 0x65113e3a, 0xac252537,
    0xeb36d3cc},
   {0x00000000, 0xa13984ee, 0x99020f9d, 0x383b8b73, 0xe975197b,
    0x484c9d95, 0x707716e6, 0xd14e9208, 0x099b34b7, 0xa8a2b059,
    0x90993b2a, 0x31a0bfc4, 0xe0ee2dcc, 0x41d7a922, 0x79ec2251,
    0xd8d5a6bf, 0x1336696e, 0xb20fed80, 0x8a3466f3, 0x2b0de21d,
    0xfa437015, 0x5b7af4fb, 0x63417f88, 0xc278fb66, 0x1aad5dd9,
    0xbb94d937, 0x83af5244, 0x2296d6aa, 0xf3d844a2, 0x52e1c04c,
    0x6ada4b3f, 0xcbe3cfd1, 0x266cd2dc, 0x87555632, 0xbf6edd41,
    0x1e5759af, 0xcf19cba7, 0x6e204f49, 0x561bc43a, 0xf72240d4,
    0x2ff7e66b, 0x8ece6285, 0xb6f5e9f6, 0x17cc6d18, 0xc682ff10,
    0x67bb7bfe, 0x5f80f08d, 0xfeb97463, 0x355abbb2, 0x94633f5c,
    0xac58b42f, 0x0d6130c1, 0xdc2fa2c9, 0x7d162627, 0x452dad54,
    0xe41429ba, 0x3cc18f05, 0x9df80beb, 0xa5c38098, 0x04fa0476,
    0xd5b4967e, 0x748d1290, 0x4cb699e3, 0xed8f1d0d, 0x4cd9a5b8,
    0xede02156, 0xd5dbaa25, 0x74e22ecb, 0xa5acbcc3, 0x0495382d,
    0x3caeb35e, 0x9d9737b0, 0x4542910f, 0xe47b15e1, 0xdc409e92,
    0x7d791a7c, 0xac378874, 0x0d0e0c9a, 0x353587e9, 0x940c0307,
    0x5fefccd6, 0xfed64838, 0xc6edc34b, 0x67d447a5, 0xb69ad5ad,
    0x17a35143, 0x2f98da30, 0x8ea15ede, 0x5674f861, 0xf74d7c8f,
    0xcf76f7fc, 0x6e4f7312, 0xbf01e11a, 0x1e3865f4, 0x2603ee87,
    0x873a6a69, 0x6ab57764, 0xcb8cf38a, 0xf3b778f9, 0x528efc17,
    0x83c06e1f, 0x22f9eaf1, 0x1ac26182, 0xbbfbe56c, 0x632e43d3,
    0xc217c73d, 0xfa2c4c4e, 0x5b15c8a0, 0x8a5b5aa8, 0x2b62de46,
    0x13595535, 0xb260d1db, 0x79831e0a, 0xd8ba9ae4, 0xe0811197,
    0x41b89579, 0x90f60771, 0x31cf839f, 0x09f408ec, 0xa8cd8c02,
    0x70182abd, 0xd121ae53, 0xe91a2520, 0x4823a1ce, 0x996d33c6,
    0x3854b728, 0x006f3c5b, 0xa156b8b5, 0x99b34b70, 0x388acf9e,
    0x00b144ed, 0xa188c003, 0x70c6520b, 0xd1ffd6e5, 0xe9c45d96,
    0x48fdd978, 0x90287fc7, 0x3111fb29, 0x092a705a, 0xa813f4b4,
    0x795d66bc, 0xd864e252, 0xe05f6921, 0x4166edcf, 0x8a85221e,
    0x2bbca6f0, 0x13872d83, 0xb2bea96d, 0x63f03b65, 0xc2c9bf8b,
    0xfaf234f8, 0x5bcbb016, 0x831e16a9, 0x22279247, 0x1a1c1934,
    0xbb259dda, 0x6a6b0fd2, 0xcb528b3c, 0xf369004f, 0x525084a1,
    0xbfdf99ac, 0x1ee61d42, 0x26dd9631, 0x87e412df, 0x56aa80d7,
    0xf7930439, 0xcfa88f4a, 0x6e910ba4, 0xb644ad1b, 0x177d29f5,
    0x2f46a286, 0x8e7f2668, 0x5f31b460, 0xfe08308e, 0xc633bbfd,
    0x670a3f13, 0xace9f0c2, 0x0dd0742c, 0x35ebff5f, 0x94d27bb1,
    0x459ce9b9, 0xe4a56d57, 0xdc9ee624, 0x7da762ca, 0xa572c475,
    0x044b409b, 0x3c70cbe8, 0x9d494f06, 0x4c07dd0e, 0xed3e59e0,
    0xd505d293, 0x743c567d, 0xd56aeec8, 0x74536a26, 0x4c68e155,
    0xed5165bb, 0x3c1ff7b3, 0x9d26735d, 0xa51df82e, 0x04247cc0,
    0xdcf1da7f, 0x7dc85e91, 0x45f3d5e2, 0xe4ca510c, 0x3584c304,
    0x94bd47ea, 0xac86cc99, 0x0dbf4877, 0xc65c87a6, 0x67650348,
    0x5f5e883b, 0xfe670cd5, 0x2f299edd, 0x8e101a33, 0xb62b9140,
    0x171215ae, 0xcfc7b311, 0x6efe37ff, 0x56c5bc8c, 0xf7fc3862,
    0x26b2aa6a, 0x878b2e84, 0xbfb0a5f7, 0x1e892119, 0xf3063c14,
    0x523fb8fa, 0x6a043389, 0xcb3db767, 0x1a73256f, 0xbb4aa181,
    0x83712af2, 0x2248ae1c, 0xfa9d08a3, 0x5ba48c4d, 0x639f073e,
    0xc2a683d0, 0x13e811d8, 0xb2d19536, 0x8aea1e45, 0x2bd39aab,
    0xe030557a, 0x4109d194, 0x79325ae7, 0xd80bde09, 0x09454c01,
    0xa87cc8ef, 0x9047439c, 0x317ec772, 0xe9ab61cd, 0x4892e523,
    0x70a96e50, 0xd190eabe, 0x00de78b6, 0xa1e7fc58, 0x99dc772b,
    0x38e5f3c5},
   {0x00000000, 0xe81790a1, 0x0b5e2703, 0xe349b7a2, 0x16bc4e06,
    0xfeabdea7, 0x1de26905, 0xf5f5f9a4, 0x2d789c0c, 0xc56f0cad,
    0x2626bb0f, 0xce312bae, 0x3bc4d20a, 0xd3d342ab, 0x309af509,
    0xd88d65a8, 0x5af13818, 0xb2e6a8b9, 0x51af1f1b, 0xb9b88fba,
    0x4c4d761e, 0xa45ae6bf, 0x4713511d, 0xaf04c1bc, 0x7789a414,
    0x9f9e34b5, 0x7cd78317, 0x94c013b6, 0x6135ea12, 0x89227ab3,
    0x6a6bcd11, 0x827c5db0, 0xb5e27030, 0x5df5e091, 0xbebc5733,
    0x56abc792, 0xa35e3e36, 0x4b49ae97, 0xa8001935, 0x40178994,
    0x989aec3c, 0x708d7c9d, 0x93c4cb3f, 0x7bd35b9e, 0x8e26a23a,
    0x6631329b, 0x85788539, 0x6d6f1598, 0xef134828, 0x0704d889,
    0xe44d6f2b, 0x0c5aff8a, 0xf9af062e, 0x11b8968f, 0xf2f1212d,
    0x1ae6b18c, 0xc26bd424, 0x2a7c4485, 0xc935f327, 0x21226386,
    0xd4d79a22, 0x3cc00a83, 0xdf89bd21, 0x379e2d80, 0xb0b5e621,
    0x58a27680, 0xbbebc122, 0x53fc5183, 0xa609a827, 0x4e1e3886,
    0xad578f24, 0x45401f85, 0x9dcd7a2d, 0x75daea8c, 0x96935d2e,
    0x7e84cd8f, 0x8b71342b, 0x6366a48a, 0x802f1328, 0x68388389,
    0xea44de39, 0x02534e98, 0xe11af93a, 0x090d699b, 0xfcf8903f,
    0x14ef009e, 0xf7a6b73c, 0x1fb1279d, 0xc73c4235, 0x2f2bd294,
    0xcc626536, 0x2475f597, 0xd1800c33, 0x39979c92, 0xdade2b30,
    0x32c9bb91, 0x05579611, 0xed4006b0, 0x0e09b112, 0xe61e21b3,
    0x13ebd817, 0xfbfc48b6, 0x18b5ff14, 0xf0a26fb5, 0x282f0a1d,
    0xc0389abc, 0x23712d1e, 0xcb66bdbf, 0x3e93441b, 0xd684d4ba,
    0x35cd6318, 0xdddaf3b9, 0x5fa6ae09, 0xb7b13ea8, 0x54f8890a,
    0xbcef19ab, 0x491ae00f, 0xa10d70ae, 0x4244c70c, 0xaa5357ad,
    0x72de3205, 0x9ac9a2a4, 0x79801506, 0x919785a7, 0x64627c03,
    0x8c75eca2, 0x6f3c5b00, 0x872bcba1, 0xba1aca03, 0x520d5aa2,
    0xb144ed00, 0x59537da1, 0xaca68405, 0x44b114a4, 0xa7f8a306,
    0x4fef33a7, 0x9762560f, 0x7f75c6ae, 0x9c3c710c, 0x742be1ad,
    0x81de1809, 0x69c988a8, 0x8a803f0a, 0x6297afab, 0xe0ebf21b,
    0x08fc62ba, 0xebb5d518, 0x03a245b9, 0xf657bc1d, 0x1e402cbc,
    0xfd099b1e, 0x151e0bbf, 0xcd936e17, 0x2584feb6, 0xc6cd4914,
    0x2edad9b5, 0xdb2f2011, 0x3338b0b0, 0xd0710712, 0x386697b3,
    0x0ff8ba33, 0xe7ef2a92, 0x04a69d30, 0xecb10d91, 0x1944f435,
    0xf1536494, 0x121ad336, 0xfa0d4397, 0x2280263f, 0xca97b69e,
    0x29de013c, 0xc1c9919d, 0x343c6839, 0xdc2bf898, 0x3f624f3a,
    0xd775df9b, 0x5509822b, 0xbd1e128a, 0x5e57a528, 0xb6403589,
    0x43b5cc2d, 0xaba25c8c, 0x48ebeb2e, 0xa0fc7b8f, 0x78711e27,
    0x90668e86, 0x732f3924, 0x9b38a985, 0x6ecd5021, 0x86dac080,
    0x65937722, 0x8d84e783, 0x0aaf2c22, 0xe2b8bc83, 0x01f10b21,
    0xe9e69b80, 0x1c136224, 0xf404f285, 0x174d4527, 0xff5ad586,
    0x27d7b02e, 0xcfc0208f, 0x2c89972d, 0xc49e078c, 0x316bfe28,
    0xd97c6e89, 0x3a35d92b, 0xd222498a, 0x505e143a, 0xb849849b,
    0x5b003339, 0xb317a398, 0x46e25a3c, 0xaef5ca9d, 0x4dbc7d3f,
    0xa5abed9e, 0x7d268836, 0x95311897, 0x7678af35, 0x9e6f3f94,
    0x6b9ac630, 0x838d5691, 0x60c4e133, 0x88d37192, 0xbf4d5c12,
    0x575accb3, 0xb4137b11, 0x5c04ebb0, 0xa9f11214, 0x41e682b5,
    0xa2af3517, 0x4ab8a5b6, 0x9235c01e, 0x7a2250bf, 0x996be71d,
    0x717c77bc, 0x84898e18, 0x6c9e1eb9, 0x8fd7a91b, 0x67c039ba,
    0xe5bc640a, 0x0dabf4ab, 0xeee24309, 0x06f5d3a8, 0xf3002a0c,
    0x1b17baad, 0xf85e0d0f, 0x10499dae, 0xc8c4f806, 0x20d368a7,
    0xc39adf05, 0x2b8d4fa4, 0xde78b600, 0x366f26a1, 0xd5269103,
    0x3d3101a2}};

local const z_word_t FAR crc_braid_big_table[][256] = {
   {0x0000000000000000, 0xa19017e800000000, 0x03275e0b00000000,
    0xa2b749e300000000, 0x064ebc1600000000, 0xa7deabfe00000000,
    0x0569e21d00000000, 0xa4f9f5f500000000, 0x0c9c782d00000000,
    0xad0c6fc500000000, 0x0fbb262600000000, 0xae2b31ce00000000,
    0x0ad2c43b00000000, 0xab42d3d300000000, 0x09f59a3000000000,
    0xa8658dd800000000, 0x1838f15a00000000, 0xb9a8e6b200000000,
    0x1b1faf5100000000, 0xba8fb8b900000000, 0x1e764d4c00000000,
    0xbfe65aa400000000, 0x1d51134700000000, 0xbcc104af00000000,
    0x14a4897700000000, 0xb5349e9f00000000, 0x1783d77c00000000,
    0xb613c09400000000, 0x12ea356100000000, 0xb37a228900000000,
    0x11cd6b6a00000000, 0xb05d7c8200000000, 0x3070e2b500000000,
    0x91e0f55d00000000, 0x3357bcbe00000000, 0x92c7ab5600000000,
    0x363e5ea300000000, 0x97ae494b00000000, 0x351900a800000000,
    0x9489174000000000, 0x3cec9a9800000000, 0x9d7c8d7000000000,
    0x3fcbc49300000000, 0x9e5bd37b00000000, 0x3aa2268e00000000,
    0x9b32316600000000, 0x3985788500000000, 0x98156f6d00000000,
    0x284813ef00000000, 0x89d8040700000000, 0x2b6f4de400000000,
    0x8aff5a0c00000000, 0x2e06aff900000000, 0x8f96b81100000000,
    0x2d21f1f200000000, 0x8cb1e61a00000000, 0x24d46bc200000000,
    0x85447c2a00000000, 0x27f335c900000000, 0x8663222100000000,
    0x229ad7d400000000, 0x830ac03c00000000, 0x21bd89df00000000,
    0x802d9e3700000000, 0x21e6b5b000000000, 0x8076a25800000000,
    0x22c1ebbb00000000, 0x8351fc5300000000, 0x27a809a600000000,
    0x86381e4e00000000, 0x248f57ad00000000, 0x851f404500000000,
    0x2d7acd9d00000000, 0x8ceada7500000000, 0x2e5d939600000000,
    0x8fcd847e00000000, 0x2b34718b00000000, 0x8aa4666300000000,
    0x28132f8000000000, 0x8983386800000000, 0x39de44ea00000000,
    0x984e530200000000, 0x3af91ae100000000, 0x9b690d0900000000,
    0x3f90f8fc00000000, 0x9e00ef1400000000, 0x3cb7a6f700000000,
    0x9d27b11f00000000, 0x35423cc700000000, 0x94d22b2f00000000,
    0x366562cc00000000, 0x97f5752400000000, 0x330c80d100000000,
    0x929c973900000000, 0x302bdeda00000000, 0x91bbc93200000000,
    0x1196570500000000, 0xb00640ed00000000, 0x12b1090e00000000,
    0xb3211ee600000000, 0x17d8eb1300000000, 0xb648fcfb00000000,
    0x14ffb51800000000, 0xb56fa2f000000000, 0x1d0a2f2800000000,
    0xbc9a38c000000000, 0x1e2d712300000000, 0xbfbd66cb00000000,
    0x1b44933e00000000, 0xbad484d600000000, 0x1863cd3500000000,
    0xb9f3dadd00000000, 0x09aea65f00000000, 0xa83eb1b700000000,
    0x0a89f85400000000, 0xab19efbc00000000, 0x0fe01a4900000000,
    0xae700da100000000, 0x0cc7444200000000, 0xad5753aa00000000,
    0x0532de7200000000, 0xa4a2c99a00000000, 0x0615807900000000,
    0xa785979100000000, 0x037c626400000000, 0xa2ec758c00000000,
    0x005b3c6f00000000, 0xa1cb2b8700000000, 0x03ca1aba00000000,
    0xa25a0d5200000000, 0x00ed44b100000000, 0xa17d535900000000,
    0x0584a6ac00000000, 0xa414b14400000000, 0x06a3f8a700000000,
    0xa733ef4f00000000, 0x0f56629700000000, 0xaec6757f00000000,
    0x0c713c9c00000000, 0xade12b7400000000, 0x0918de8100000000,
    0xa888c96900000000, 0x0a3f808a00000000, 0xabaf976200000000,
    0x1bf2ebe000000000, 0xba62fc0800000000, 0x18d5b5eb00000000,
    0xb945a20300000000, 0x1dbc57f600000000, 0xbc2c401e00000000,
    0x1e9b09fd00000000, 0xbf0b1e1500000000, 0x176e93cd00000000,
    0xb6fe842500000000, 0x1449cdc600000000, 0xb5d9da2e00000000,
    0x11202fdb00000000, 0xb0b0383300000000, 0x120771d000000000,
    0xb397663800000000, 0x33baf80f00000000, 0x922aefe700000000,
    0x309da60400000000, 0x910db1ec00000000, 0x35f4441900000000,
    0x946453f100000000, 0x36d31a1200000000, 0x97430dfa00000000,
    0x3f26802200000000, 0x9eb697ca00000000, 0x3c01de2900000000,
    0x9d91c9c100000000, 0x39683c3400000000, 0x98f82bdc00000000,
    0x3a4f623f00000000, 0x9bdf75d700000000, 0x2b82095500000000,
    0x8a121ebd00000000, 0x28a5575e00000000, 0x893540b600000000,
    0x2dccb54300000000, 0x8c5ca2ab00000000, 0x2eebeb4800000000,
    0x8f7bfca000000000, 0x271e717800000000, 0x868e669000000000,
    0x24392f7300000000, 0x85a9389b00000000, 0x2150cd6e00000000,
    0x80c0da8600000000, 0x2277936500000000, 0x83e7848d00000000,
    0x222caf0a00000000, 0x83bcb8e200000000, 0x210bf10100000000,
    0x809be6e900000000, 0x2462131c00000000, 0x85f204f400000000,
    0x27454d1700000000, 0x86d55aff00000000, 0x2eb0d72700000000,
    0x8f20c0cf00000000, 0x2d97892c00000000, 0x8c079ec400000000,
    0x28fe6b3100000000, 0x896e7cd900000000, 0x2bd9353a00000000,
    0x8a4922d200000000, 0x3a145e5000000000, 0x9b8449b800000000,
    0x3933005b00000000, 0x98a317b300000000, 0x3c5ae24600000000,
    0x9dcaf5ae00000000, 0x3f7dbc4d00000000, 0x9eedaba500000000,
    0x3688267d00000000, 0x9718319500000000, 0x35af787600000000,
    0x943f6f9e00000000, 0x30c69a6b00000000, 0x91568d8300000000,
    0x33e1c46000000000, 0x9271d38800000000, 0x125c4dbf00000000,
    0xb3cc5a5700000000, 0x117b13b400000000, 0xb0eb045c00000000,
    0x1412f1a900000000, 0xb582e64100000000, 0x1735afa200000000,
    0xb6a5b84a00000000, 0x1ec0359200000000, 0xbf50227a00000000,
    0x1de76b9900000000, 0xbc777c7100000000, 0x188e898400000000,
    0xb91e9e6c00000000, 0x1ba9d78f00000000, 0xba39c06700000000,
    0x0a64bce500000000, 0xabf4ab0d00000000, 0x0943e2ee00000000,
    0xa8d3f50600000000, 0x0c2a00f300000000, 0xadba171b00000000,
    0x0f0d5ef800000000, 0xae9d491000000000, 0x06f8c4c800000000,
    0xa768d32000000000, 0x05df9ac300000000, 0xa44f8d2b00000000,
    0x00b678de00000000, 0xa1266f3600000000, 0x039126d500000000,
    0xa201313d00000000},
   {0x0000000000000000, 0xee8439a100000000, 0x9d0f029900000000,
    0x738b3b3800000000, 0x7b1975e900000000, 0x959d4c4800000000,
    0xe616777000000000, 0x08924ed100000000, 0xb7349b0900000000,
    0x59b0a2a800000000, 0x2a3b999000000000, 0xc4bfa03100000000,
    0xcc2deee000000000, 0x22a9d74100000000, 0x5122ec7900000000,
    0xbfa6d5d800000000, 0x6e69361300000000, 0x80ed0fb200000000,
    0xf366348a00000000, 0x1de20d2b00000000, 0x157043fa00000000,
    0xfbf47a5b00000000, 0x887f416300000000, 0x66fb78c200000000,
    0xd95dad1a00000000, 0x37d994bb00000000, 0x4452af8300000000,
    0xaad6962200000000, 0xa244d8f300000000, 0x4cc0e15200000000,
    0x3f4bda6a00000000, 0xd1cfe3cb00000000, 0xdcd26c2600000000,
    0x3256558700000000, 0x41dd6ebf00000000, 0xaf59571e00000000,
    0xa7cb19cf00000000, 0x494f206e00000000, 0x3ac41b5600000000,
    0xd44022f700000000, 0x6be6f72f00000000, 0x8562ce8e00000000,
    0xf6e9f5b600000000, 0x186dcc1700000000, 0x10ff82c600000000,
    0xfe7bbb6700000000, 0x8df0805f00000000, 0x6374b9fe00000000,
    0xb2bb5a3500000000, 0x5c3f639400000000, 0x2fb458ac00000000,
    0xc130610d00000000, 0xc9a22fdc00000000, 0x2726167d00000000,
    0x54ad2d4500000000, 0xba2914e400000000, 0x058fc13c00000000,
    0xeb0bf89d00000000, 0x9880c3a500000000, 0x7604fa0400000000,
    0x7e96b4d500000000, 0x90128d7400000000, 0xe399b64c00000000,
    0x0d1d8fed00000000, 0xb8a5d94c00000000, 0x5621e0ed00000000,
    0x25aadbd500000000, 0xcb2ee27400000000, 0xc3bcaca500000000,
    0x2d38950400000000, 0x5eb3ae3c00000000, 0xb037979d00000000,
    0x0f91424500000000, 0xe1157be400000000, 0x929e40dc00000000,
    0x7c1a797d00000000, 0x748837ac00000000, 0x9a0c0e0d00000000,
    0xe987353500000000, 0x07030c9400000000, 0xd6ccef5f00000000,
    0x3848d6fe00000000, 0x4bc3edc600000000, 0xa547d46700000000,
    0xadd59ab600000000, 0x4351a31700000000, 0x30da982f00000000,
    0xde5ea18e00000000, 0x61f8745600000000, 0x8f7c4df700000000,
    0xfcf776cf00000000, 0x12734f6e00000000, 0x1ae101bf00000000,
    0xf465381e00000000, 0x87ee032600000000, 0x696a3a8700000000,
    0x6477b56a00000000, 0x8af38ccb00000000, 0xf978b7f300000000,
    0x17fc8e5200000000, 0x1f6ec08300000000, 0xf1eaf92200000000,
    0x8261c21a00000000, 0x6ce5fbbb00000000, 0xd3432e6300000000,
    0x3dc717c200000000, 0x4e4c2cfa00000000, 0xa0c8155b00000000,
    0xa85a5b8a00000000, 0x46de622b00000000, 0x3555591300000000,
    0xdbd160b200000000, 0x0a1e837900000000, 0xe49abad800000000,
    0x971181e000000000, 0x7995b84100000000, 0x7107f69000000000,
    0x9f83cf3100000000, 0xec08f40900000000, 0x028ccda800000000,
    0xbd2a187000000000, 0x53ae21d100000000, 0x20251ae900000000,
    0xcea1234800000000, 0xc6336d9900000000, 0x28b7543800000000,
    0x5b3c6f0000000000, 0xb5b856a100000000, 0x704bb39900000000,
    0x9ecf8a3800000000, 0xed44b10000000000, 0x03c088a100000000,
    0x0b52c67000000000, 0xe5d6ffd100000000, 0x965dc4e900000000,
    0x78d9fd4800000000, 0xc77f289000000000, 0x29fb113100000000,
    0x5a702a0900000000, 0xb4f413a800000000, 0xbc665d7900000000,
    0x52e264d800000000, 0x21695fe000000000, 0xcfed664100000000,
    0x1e22858a00000000, 0xf0a6bc2b00000000, 0x832d871300000000,
    0x6da9beb200000000, 0x653bf06300000000, 0x8bbfc9c200000000,
    0xf834f2fa00000000, 0x16b0cb5b00000000, 0xa9161e8300000000,
    0x4792272200000000, 0x34191c1a00000000, 0xda9d25bb00000000,
    0xd20f6b6a00000000, 0x3c8b52cb00000000, 0x4f0069f300000000,
    0xa184505200000000, 0xac99dfbf00000000, 0x421de61e00000000,
    0x3196dd2600000000, 0xdf12e48700000000, 0xd780aa5600000000,
    0x390493f700000000, 0x4a8fa8cf00000000, 0xa40b916e00000000,
    0x1bad44b600000000, 0xf5297d1700000000, 0x86a2462f00000000,
    0x68267f8e00000000, 0x60b4315f00000000, 0x8e3008fe00000000,
    0xfdbb33c600000000, 0x133f0a6700000000, 0xc2f0e9ac00000000,
    0x2c74d00d00000000, 0x5fffeb3500000000, 0xb17bd29400000000,
    0xb9e99c4500000000, 0x576da5e400000000, 0x24e69edc00000000,
    0xca62a77d00000000, 0x75c472a500000000, 0x9b404b0400000000,
    0xe8cb703c00000000, 0x064f499d00000000, 0x0edd074c00000000,
    0xe0593eed00000000, 0x93d205d500000000, 0x7d563c7400000000,
    0xc8ee6ad500000000, 0x266a537400000000, 0x55e1684c00000000,
    0xbb6551ed00000000, 0xb3f71f3c00000000, 0x5d73269d00000000,
    0x2ef81da500000000, 0xc07c240400000000, 0x7fdaf1dc00000000,
    0x915ec87d00000000, 0xe2d5f34500000000, 0x0c51cae400000000,
    0x04c3843500000000, 0xea47bd9400000000, 0x99cc86ac00000000,
    0x7748bf0d00000000, 0xa6875cc600000000, 0x4803656700000000,
    0x3b885e5f00000000, 0xd50c67fe00000000, 0xdd9e292f00000000,
    0x331a108e00000000, 0x40912bb600000000, 0xae15121700000000,
    0x11b3c7cf00000000, 0xff37fe6e00000000, 0x8cbcc55600000000,
    0x6238fcf700000000, 0x6aaab22600000000, 0x842e8b8700000000,
    0xf7a5b0bf00000000, 0x1921891e00000000, 0x143c06f300000000,
    0xfab83f5200000000, 0x8933046a00000000, 0x67b73dcb00000000,
    0x6f25731a00000000, 0x81a14abb00000000, 0xf22a718300000000,
    0x1cae482200000000, 0xa3089dfa00000000, 0x4d8ca45b00000000,
    0x3e079f6300000000, 0xd083a6c200000000, 0xd811e81300000000,
    0x3695d1b200000000, 0x451eea8a00000000, 0xab9ad32b00000000,
    0x7a5530e000000000, 0x94d1094100000000, 0xe75a327900000000,
    0x09de0bd800000000, 0x014c450900000000, 0xefc87ca800000000,
    0x9c43479000000000, 0x72c77e3100000000, 0xcd61abe900000000,
    0x23e5924800000000, 0x506ea97000000000, 0xbeea90d100000000,
    0xb678de0000000000, 0x58fce7a100000000, 0x2b77dc9900000000,
    0xc5f3e53800000000},
   {0x0000000000000000, 0xfbf6134700000000, 0xf6ed278e00000000,
    0x0d1b34c900000000, 0xaddd3ec700000000, 0x562b2d8000000000,
    0x5b30194900000000, 0xa0c60a0e00000000, 0x1bbd0c5500000000,
    0xe04b1f1200000000, 0xed502bdb00000000, 0x16a6389c00000000,
    0xb660329200000000, 0x4d9621d500000000, 0x408d151c00000000,
    0xbb7b065b00000000, 0x367a19aa00000000, 0xcd8c0aed00000000,
    0xc0973e2400000000, 0x3b612d6300000000, 0x9ba7276d00000000,
    0x6051342a00000000, 0x6d4a00e300000000, 0x96bc13a400000000,
    0x2dc715ff00000000, 0xd63106b800000000, 0xdb2a327100000000,
    0x20dc213600000000, 0x801a2b3800000000, 0x7bec387f00000000,
    0x76f70cb600000000, 0x8d011ff100000000, 0x2df2438f00000000,
    0xd60450c800000000, 0xdb1f640100000000, 0x20e9774600000000,
    0x802f7d4800000000, 0x7bd96e0f00000000, 0x76c25ac600000000,
    0x8d34498100000000, 0x364f4fda00000000, 0xcdb95c9d00000000,
    0xc0a2685400000000, 0x3b547b1300000000, 0x9b92711d00000000,
    0x6064625a00000000, 0x6d7f569300000000, 0x968945d400000000,
    0x1b885a2500000000, 0xe07e496200000000, 0xed657dab00000000,
    0x16936eec00000000, 0xb65564e200000000, 0x4da377a500000000,
    0x40b8436c00000000, 0xbb4e502b00000000, 0x0035567000000000,
    0xfbc3453700000000, 0xf6d871fe00000000, 0x0d2e62b900000000,
    0xade868b700000000, 0x561e7bf000000000, 0x5b054f3900000000,
    0xa0f35c7e00000000, 0x1be2f6c500000000, 0xe014e58200000000,
    0xed0fd14b00000000, 0x16f9c20c00000000, 0xb63fc80200000000,
    0x4dc9db4500000000, 0x40d2ef8c00000000, 0xbb24fccb00000000,
    0x005ffa9000000000, 0xfba9e9d700000000, 0xf6b2dd1e00000000,
    0x0d44ce5900000000, 0xad82c45700000000, 0x5674d71000000000,
    0x5b6fe3d900000000, 0xa099f09e00000000, 0x2d98ef6f00000000,
    0xd66efc2800000000, 0xdb75c8e100000000, 0x2083dba600000000,
    0x8045d1a800000000, 0x7bb3c2ef00000000, 0x76a8f62600000000,
    0x8d5ee56100000000, 0x3625e33a00000000, 0xcdd3f07d00000000,
    0xc0c8c4b400000000, 0x3b3ed7f300000000, 0x9bf8ddfd00000000,
    0x600eceba00000000, 0x6d15fa7300000000, 0x96e3e93400000000,
    0x3610b54a00000000, 0xcde6a60d00000000, 0xc0fd92c400000000,
    0x3b0b818300000000, 0x9bcd8b8d00000000, 0x603b98ca00000000,
    0x6d20ac0300000000, 0x96d6bf4400000000, 0x2dadb91f00000000,
    0xd65baa5800000000, 0xdb409e9100000000, 0x20b68dd600000000,
    0x807087d800000000, 0x7b86949f00000000, 0x769da05600000000,
    0x8d6bb31100000000, 0x006aace000000000, 0xfb9cbfa700000000,
    0xf6878b6e00000000, 0x0d71982900000000, 0xadb7922700000000,
    0x5641816000000000, 0x5b5ab5a900000000, 0xa0aca6ee00000000,
    0x1bd7a0b500000000, 0xe021b3f200000000, 0xed3a873b00000000,
    0x16cc947c00000000, 0xb60a9e7200000000, 0x4dfc8d3500000000,
    0x40e7b9fc00000000, 0xbb11aabb00000000, 0x77c29c5000000000,
    0x8c348f1700000000, 0x812fbbde00000000, 0x7ad9a89900000000,
    0xda1fa29700000000, 0x21e9b1d000000000, 0x2cf2851900000000,
    0xd704965e00000000, 0x6c7f900500000000, 0x9789834200000000,
    0x9a92b78b00000000, 0x6164a4cc00000000, 0xc1a2aec200000000,
    0x3a54bd8500000000, 0x374f894c00000000, 0xccb99a0b00000000,
    0x41b885fa00000000, 0xba4e96bd00000000, 0xb755a27400000000,
    0x4ca3b13300000000, 0xec65bb3d00000000, 0x1793a87a00000000,
    0x1a889cb300000000, 0xe17e8ff400000000, 0x5a0589af00000000,
    0xa1f39ae800000000, 0xace8ae2100000000, 0x571ebd6600000000,
    0xf7d8b76800000000, 0x0c2ea42f00000000, 0x013590e600000000,
    0xfac383a100000000, 0x5a30dfdf00000000, 0xa1c6cc9800000000,
    0xacddf85100000000, 0x572beb1600000000, 0xf7ede11800000000,
    0x0c1bf25f00000000, 0x0100c69600000000, 0xfaf6d5d100000000,
    0x418dd38a00000000, 0xba7bc0cd00000000, 0xb760f40400000000,
    0x4c96e74300000000, 0xec50ed4d00000000, 0x17a6fe0a00000000,
    0x1abdcac300000000, 0xe14bd98400000000, 0x6c4ac67500000000,
    0x97bcd53200000000, 0x9aa7e1fb00000000, 0x6151f2bc00000000,
    0xc197f8b200000000, 0x3a61ebf500000000, 0x377adf3c00000000,
    0xcc8ccc7b00000000, 0x77f7ca2000000000, 0x8c01d96700000000,
    0x811aedae00000000, 0x7aecfee900000000, 0xda2af4e700000000,
    0x21dce7a000000000, 0x2cc7d36900000000, 0xd731c02e00000000,
    0x6c206a9500000000, 0x97d679d200000000, 0x9acd4d1b00000000,
    0x613b5e5c00000000, 0xc1fd545200000000, 0x3a0b471500000000,
    0x371073dc00000000, 0xcce6609b00000000, 0x779d66c000000000,
    0x8c6b758700000000, 0x8170414e00000000, 0x7a86520900000000,
    0xda40580700000000, 0x21b64b4000000000, 0x2cad7f8900000000,
    0xd75b6cce00000000, 0x5a5a733f00000000, 0xa1ac607800000000,
    0xacb754b100000000, 0x574147f600000000, 0xf7874df800000000,
    0x0c715ebf00000000, 0x016a6a7600000000, 0xfa9c793100000000,
    0x41e77f6a00000000, 0xba116c2d00000000, 0xb70a58e400000000,
    0x4cfc4ba300000000, 0xec3a41ad00000000, 0x17cc52ea00000000,
    0x1ad7662300000000, 0xe121756400000000, 0x41d2291a00000000,
    0xba243a5d00000000, 0xb73f0e9400000000, 0x4cc91dd300000000,
    0xec0f17dd00000000, 0x17f9049a00000000, 0x1ae2305300000000,
    0xe114231400000000, 0x5a6f254f00000000, 0xa199360800000000,
    0xac8202c100000000, 0x5774118600000000, 0xf7b21b8800000000,
    0x0c4408cf00000000, 0x015f3c0600000000, 0xfaa92f4100000000,
    0x77a830b000000000, 0x8c5e23f700000000, 0x8145173e00000000,
    0x7ab3047900000000, 0xda750e7700000000, 0x21831d3000000000,
    0x2c9829f900000000, 0xd76e3abe00000000, 0x6c153ce500000000,
    0x97e32fa200000000, 0x9af81b6b00000000, 0x610e082c00000000,
    0xc1c8022200000000, 0x3a3e116500000000, 0x372525ac00000000,
    0xccd336eb00000000},
   {0x0000000000000000, 0x6238282a00000000, 0xc470505400000000,
    0xa648787e00000000, 0x88e1a0a800000000, 0xead9888200000000,
    0x4c91f0fc00000000, 0x2ea9d8d600000000, 0x51c5308a00000000,
    0x33fd18a000000000, 0x95b560de00000000, 0xf78d48f400000000,
    0xd924902200000000, 0xbb1cb80800000000, 0x1d54c07600000000,
    0x7f6ce85c00000000, 0xe38c10cf00000000, 0x81b438e500000000,
    0x27fc409b00000000, 0x45c468b100000000, 0x6b6db06700000000,
    0x0955984d00000000, 0xaf1de03300000000, 0xcd25c81900000000,
    0xb249204500000000, 0xd071086f00000000, 0x7639701100000000,
    0x1401583b00000000, 0x3aa880ed00000000, 0x5890a8c700000000,
    0xfed8d0b900000000, 0x9ce0f89300000000, 0x871f504500000000,
    0xe527786f00000000, 0x436f001100000000, 0x2157283b00000000,
    0x0ffef0ed00000000, 0x6dc6d8c700000000, 0xcb8ea0b900000000,
    0xa9b6889300000000, 0xd6da60cf00000000, 0xb4e248e500000000,
    0x12aa309b00000000, 0x709218b100000000, 0x5e3bc06700000000,
    0x3c03e84d00000000, 0x9a4b903300000000, 0xf873b81900000000,
    0x6493408a00000000, 0x06ab68a000000000, 0xa0e310de00000000,
    0xc2db38f400000000, 0xec72e02200000000, 0x8e4ac80800000000,
    0x2802b07600000000, 0x4a3a985c00000000, 0x3556700000000000,
    0x576e582a00000000, 0xf126205400000000, 0x931e087e00000000,
    0xbdb7d0a800000000, 0xdf8ff88200000000, 0x79c780fc00000000,
    0x1bffa8d600000000, 0x0e3fa08a00000000, 0x6c0788a000000000,
    0xca4ff0de00000000, 0xa877d8f400000000, 0x86de002200000000,
    0xe4e6280800000000, 0x42ae507600000000, 0x2096785c00000000,
    0x5ffa900000000000, 0x3dc2b82a00000000, 0x9b8ac05400000000,
    0xf9b2e87e00000000, 0xd71b30a800000000, 0xb523188200000000,
    0x136b60fc00000000, 0x715348d600000000, 0xedb3b04500000000,
    0x8f8b986f00000000, 0x29c3e01100000000, 0x4bfbc83b00000000,
    0x655210ed00000000, 0x076a38c700000000, 0xa12240b900000000,
    0xc31a689300000000, 0xbc7680cf00000000, 0xde4ea8e500000000,
    0x7806d09b00000000, 0x1a3ef8b100000000, 0x3497206700000000,
    0x56af084d00000000, 0xf0e7703300000000, 0x92df581900000000,
    0x8920f0cf00000000, 0xeb18d8e500000000, 0x4d50a09b00000000,
    0x2f6888b100000000, 0x01c1506700000000, 0x63f9784d00000000,
    0xc5b1003300000000, 0xa789281900000000, 0xd8e5c04500000000,
    0xbadde86f00000000, 0x1c95901100000000, 0x7eadb83b00000000,
    0x500460ed00000000, 0x323c48c700000000, 0x947430b900000000,
    0xf64c189300000000, 0x6aace00000000000, 0x0894c82a00000000,
    0xaedcb05400000000, 0xcce4987e00000000, 0xe24d40a800000000,
    0x8075688200000000, 0x263d10fc00000000, 0x440538d600000000,
    0x3b69d08a00000000, 0x5951f8a000000000, 0xff1980de00000000,
    0x9d21a8f400000000, 0xb388702200000000, 0xd1b0580800000000,
    0x77f8207600000000, 0x15c0085c00000000, 0x5d7831ce00000000,
    0x3f4019e400000000, 0x9908619a00000000, 0xfb3049b000000000,
    0xd599916600000000, 0xb7a1b94c00000000, 0x11e9c13200000000,
    0x73d1e91800000000, 0x0cbd014400000000, 0x6e85296e00000000,
    0xc8cd511000000000, 0xaaf5793a00000000, 0x845ca1ec00000000,
    0xe66489c600000000, 0x402cf1b800000000, 0x2214d99200000000,
    0xbef4210100000000, 0xdccc092b00000000, 0x7a84715500000000,
    0x18bc597f00000000, 0x361581a900000000, 0x542da98300000000,
    0xf265d1fd00000000, 0x905df9d700000000, 0xef31118b00000000,
    0x8d0939a100000000, 0x2b4141df00000000, 0x497969f500000000,
    0x67d0b12300000000, 0x05e8990900000000, 0xa3a0e17700000000,
    0xc198c95d00000000, 0xda67618b00000000, 0xb85f49a100000000,
    0x1e1731df00000000, 0x7c2f19f500000000, 0x5286c12300000000,
    0x30bee90900000000, 0x96f6917700000000, 0xf4ceb95d00000000,
    0x8ba2510100000000, 0xe99a792b00000000, 0x4fd2015500000000,
    0x2dea297f00000000, 0x0343f1a900000000, 0x617bd98300000000,
    0xc733a1fd00000000, 0xa50b89d700000000, 0x39eb714400000000,
    0x5bd3596e00000000, 0xfd9b211000000000, 0x9fa3093a00000000,
    0xb10ad1ec00000000, 0xd332f9c600000000, 0x757a81b800000000,
    0x1742a99200000000, 0x682e41ce00000000, 0x0a1669e400000000,
    0xac5e119a00000000, 0xce6639b000000000, 0xe0cfe16600000000,
    0x82f7c94c00000000, 0x24bfb13200000000, 0x4687991800000000,
    0x5347914400000000, 0x317fb96e00000000, 0x9737c11000000000,
    0xf50fe93a00000000, 0xdba631ec00000000, 0xb99e19c600000000,
    0x1fd661b800000000, 0x7dee499200000000, 0x0282a1ce00000000,
    0x60ba89e400000000, 0xc6f2f19a00000000, 0xa4cad9b000000000,
    0x8a63016600000000, 0xe85b294c00000000, 0x4e13513200000000,
    0x2c2b791800000000, 0xb0cb818b00000000, 0xd2f3a9a100000000,
    0x74bbd1df00000000, 0x1683f9f500000000, 0x382a212300000000,
    0x5a12090900000000, 0xfc5a717700000000, 0x9e62595d00000000,
    0xe10eb10100000000, 0x8336992b00000000, 0x257ee15500000000,
    0x4746c97f00000000, 0x69ef11a900000000, 0x0bd7398300000000,
    0xad9f41fd00000000, 0xcfa769d700000000, 0xd458c10100000000,
    0xb660e92b00000000, 0x1028915500000000, 0x7210b97f00000000,
    0x5cb961a900000000, 0x3e81498300000000, 0x98c931fd00000000,
    0xfaf119d700000000, 0x859df18b00000000, 0xe7a5d9a100000000,
    0x41eda1df00000000, 0x23d589f500000000, 0x0d7c512300000000,
    0x6f44790900000000, 0xc90c017700000000, 0xab34295d00000000,
    0x37d4d1ce00000000, 0x55ecf9e400000000, 0xf3a4819a00000000,
    0x919ca9b000000000, 0xbf35716600000000, 0xdd0d594c00000000,
    0x7b45213200000000, 0x197d091800000000, 0x6611e14400000000,
    0x0429c96e00000000, 0xa261b11000000000, 0xc059993a00000000,
    0xeef041ec00000000, 0x8cc869c600000000, 0x2a8011b800000000,
    0x48b8399200000000},
   {0x0000000000000000, 0x4c2896a300000000, 0xd9565d9c00000000,
    0x957ecb3f00000000, 0xf3abcbe300000000, 0xbf835d4000000000,
    0x2afd967f00000000, 0x66d500dc00000000, 0xa751e61c00000000,
    0xeb7970bf00000000, 0x7e07bb8000000000, 0x322f2d2300000000,
    0x54fa2dff00000000, 0x18d2bb5c00000000, 0x8dac706300000000,
    0xc184e6c000000000, 0x4ea3cc3900000000, 0x028b5a9a00000000,
    0x97f591a500000000, 0xdbdd070600000000, 0xbd0807da00000000,
    0xf120917900000000, 0x645e5a4600000000, 0x2876cce500000000,
    0xe9f22a2500000000, 0xa5dabc8600000000, 0x30a477b900000000,
    0x7c8ce11a00000000, 0x1a59e1c600000000, 0x5671776500000000,
    0xc30fbc5a00000000, 0x8f272af900000000, 0x9c46997300000000,
    0xd06e0fd000000000, 0x4510c4ef00000000, 0x0938524c00000000,
    0x6fed529000000000, 0x23c5c43300000000, 0xb6bb0f0c00000000,
    0xfa9399af00000000, 0x3b177f6f00000000, 0x773fe9cc00000000,
    0xe24122f300000000, 0xae69b45000000000, 0xc8bcb48c00000000,
    0x8494222f00000000, 0x11eae91000000000, 0x5dc27fb300000000,
    0xd2e5554a00000000, 0x9ecdc3e900000000, 0x0bb308d600000000,
    0x479b9e7500000000, 0x214e9ea900000000, 0x6d66080a00000000,
    0xf818c33500000000, 0xb430559600000000, 0x75b4b35600000000,
    0x399c25f500000000, 0xace2eeca00000000, 0xe0ca786900000000,
    0x861f78b500000000, 0xca37ee1600000000, 0x5f49252900000000,
    0x1361b38a00000000, 0x388d32e700000000, 0x74a5a44400000000,
    0xe1db6f7b00000000, 0xadf3f9d800000000, 0xcb26f90400000000,
    0x870e6fa700000000, 0x1270a49800000000, 0x5e58323b00000000,
    0x9fdcd4fb00000000, 0xd3f4425800000000, 0x468a896700000000,
    0x0aa21fc400000000, 0x6c771f1800000000, 0x205f89bb00000000,
    0xb521428400000000, 0xf909d42700000000, 0x762efede00000000,
    0x3a06687d00000000, 0xaf78a34200000000, 0xe35035e100000000,
    0x8585353d00000000, 0xc9ada39e00000000, 0x5cd368a100000000,
    0x10fbfe0200000000, 0xd17f18c200000000, 0x9d578e6100000000,
    0x0829455e00000000, 0x4401d3fd00000000, 0x22d4d32100000000,
    0x6efc458200000000, 0xfb828ebd00000000, 0xb7aa181e00000000,
    0xa4cbab9400000000, 0xe8e33d3700000000, 0x7d9df60800000000,
    0x31b560ab00000000, 0x5760607700000000, 0x1b48f6d400000000,
    0x8e363deb00000000, 0xc21eab4800000000, 0x039a4d8800000000,
    0x4fb2db2b00000000, 0xdacc101400000000, 0x96e486b700000000,
    0xf031866b00000000, 0xbc1910c800000000, 0x2967dbf700000000,
    0x654f4d5400000000, 0xea6867ad00000000, 0xa640f10e00000000,
    0x333e3a3100000000, 0x7f16ac9200000000, 0x19c3ac4e00000000,
    0x55eb3aed00000000, 0xc095f1d200000000, 0x8cbd677100000000,
    0x4d3981b100000000, 0x0111171200000000, 0x946fdc2d00000000,
    0xd8474a8e00000000, 0xbe924a5200000000, 0xf2badcf100000000,
    0x67c417ce00000000, 0x2bec816d00000000, 0x311c141500000000,
    0x7d3482b600000000, 0xe84a498900000000, 0xa462df2a00000000,
    0xc2b7dff600000000, 0x8e9f495500000000, 0x1be1826a00000000,
    0x57c914c900000000, 0x964df20900000000, 0xda6564aa00000000,
    0x4f1baf9500000000, 0x0333393600000000, 0x65e639ea00000000,
    0x29ceaf4900000000, 0xbcb0647600000000, 0xf098f2d500000000,
    0x7fbfd82c00000000, 0x33974e8f00000000, 0xa6e985b000000000,
    0xeac1131300000000, 0x8c1413cf00000000, 0xc03c856c00000000,
    0x55424e5300000000, 0x196ad8f000000000, 0xd8ee3e3000000000,
    0x94c6a89300000000, 0x01b863ac00000000, 0x4d90f50f00000000,
    0x2b45f5d300000000, 0x676d637000000000, 0xf213a84f00000000,
    0xbe3b3eec00000000, 0xad5a8d6600000000, 0xe1721bc500000000,
    0x740cd0fa00000000, 0x3824465900000000, 0x5ef1468500000000,
    0x12d9d02600000000, 0x87a71b1900000000, 0xcb8f8dba00000000,
    0x0a0b6b7a00000000, 0x4623fdd900000000, 0xd35d36e600000000,
    0x9f75a04500000000, 0xf9a0a09900000000, 0xb588363a00000000,
    0x20f6fd0500000000, 0x6cde6ba600000000, 0xe3f9415f00000000,
    0xafd1d7fc00000000, 0x3aaf1cc300000000, 0x76878a6000000000,
    0x10528abc00000000, 0x5c7a1c1f00000000, 0xc904d72000000000,
    0x852c418300000000, 0x44a8a74300000000, 0x088031e000000000,
    0x9dfefadf00000000, 0xd1d66c7c00000000, 0xb7036ca000000000,
    0xfb2bfa0300000000, 0x6e55313c00000000, 0x227da79f00000000,
    0x099126f200000000, 0x45b9b05100000000, 0xd0c77b6e00000000,
    0x9cefedcd00000000, 0xfa3aed1100000000, 0xb6127bb200000000,
    0x236cb08d00000000, 0x6f44262e00000000, 0xaec0c0ee00000000,
    0xe2e8564d00000000, 0x77969d7200000000, 0x3bbe0bd100000000,
    0x5d6b0b0d00000000, 0x11439dae00000000, 0x843d569100000000,
    0xc815c03200000000, 0x4732eacb00000000, 0x0b1a7c6800000000,
    0x9e64b75700000000, 0xd24c21f400000000, 0xb499212800000000,
    0xf8b1b78b00000000, 0x6dcf7cb400000000, 0x21e7ea1700000000,
    0xe0630cd700000000, 0xac4b9a7400000000, 0x3935514b00000000,
    0x751dc7e800000000, 0x13c8c73400000000, 0x5fe0519700000000,
    0xca9e9aa800000000, 0x86b60c0b00000000, 0x95d7bf8100000000,
    0xd9ff292200000000, 0x4c81e21d00000000, 0x00a974be00000000,
    0x667c746200000000, 0x2a54e2c100000000, 0xbf2a29fe00000000,
    0xf302bf5d00000000, 0x3286599d00000000, 0x7eaecf3e00000000,
    0xebd0040100000000, 0xa7f892a200000000, 0xc12d927e00000000,
    0x8d0504dd00000000, 0x187bcfe200000000, 0x5453594100000000,
    0xdb7473b800000000, 0x975ce51b00000000, 0x02222e2400000000,
    0x4e0ab88700000000, 0x28dfb85b00000000, 0x64f72ef800000000,
    0xf189e5c700000000, 0xbda1736400000000, 0x7c2595a400000000,
    0x300d030700000000, 0xa573c83800000000, 0xe95b5e9b00000000,
    0x8f8e5e4700000000, 0xc3a6c8e400000000, 0x56d803db00000000,
    0x1af0957800000000},
   {0x0000000000000000, 0x939bc97f00000000, 0x263793ff00000000,
    0xb5ac5a8000000000, 0x0d68572400000000, 0x9ef39e5b00000000,
    0x2b5fc4db00000000, 0xb8c40da400000000, 0x1ad0ae4800000000,
    0x894b673700000000, 0x3ce73db700000000, 0xaf7cf4c800000000,
    0x17b8f96c00000000, 0x8423301300000000, 0x318f6a9300000000,
    0xa214a3ec00000000, 0x34a05d9100000000, 0xa73b94ee00000000,
    0x1297ce6e00000000, 0x810c071100000000, 0x39c80ab500000000,
    0xaa53c3ca00000000, 0x1fff994a00000000, 0x8c64503500000000,
    0x2e70f3d900000000, 0xbdeb3aa600000000, 0x0847602600000000,
    0x9bdca95900000000, 0x2318a4fd00000000, 0xb0836d8200000000,
    0x052f370200000000, 0x96b4fe7d00000000, 0x2946caf900000000,
    0xbadd038600000000, 0x0f71590600000000, 0x9cea907900000000,
    0x242e9ddd00000000, 0xb7b554a200000000, 0x02190e2200000000,
    0x9182c75d00000000, 0x339664b100000000, 0xa00dadce00000000,
    0x15a1f74e00000000, 0x863a3e3100000000, 0x3efe339500000000,
    0xad65faea00000000, 0x18c9a06a00000000, 0x8b52691500000000,
    0x1de6976800000000, 0x8e7d5e1700000000, 0x3bd1049700000000,
    0xa84acde800000000, 0x108ec04c00000000, 0x8315093300000000,
    0x36b953b300000000, 0xa5229acc00000000, 0x0736392000000000,
    0x94adf05f00000000, 0x2101aadf00000000, 0xb29a63a000000000,
    0x0a5e6e0400000000, 0x99c5a77b00000000, 0x2c69fdfb00000000,
    0xbff2348400000000, 0x138ae52800000000, 0x80112c5700000000,
    0x35bd76d700000000, 0xa626bfa800000000, 0x1ee2b20c00000000,
    0x8d797b7300000000, 0x38d521f300000000, 0xab4ee88c00000000,
    0x095a4b6000000000, 0x9ac1821f00000000, 0x2f6dd89f00000000,
    0xbcf611e000000000, 0x04321c4400000000, 0x97a9d53b00000000,
    0x22058fbb00000000, 0xb19e46c400000000, 0x272ab8b900000000,
    0xb4b171c600000000, 0x011d2b4600000000, 0x9286e23900000000,
    0x2a42ef9d00000000, 0xb9d926e200000000, 0x0c757c6200000000,
    0x9feeb51d00000000, 0x3dfa16f100000000, 0xae61df8e00000000,
    0x1bcd850e00000000, 0x88564c7100000000, 0x309241d500000000,
    0xa30988aa00000000, 0x16a5d22a00000000, 0x853e1b5500000000,
    0x3acc2fd100000000, 0xa957e6ae00000000, 0x1cfbbc2e00000000,
    0x8f60755100000000, 0x37a478f500000000, 0xa43fb18a00000000,
    0x1193eb0a00000000, 0x8208227500000000, 0x201c819900000000,
    0xb38748e600000000, 0x062b126600000000, 0x95b0db1900000000,
    0x2d74d6bd00000000, 0xbeef1fc200000000, 0x0b43454200000000,
    0x98d88c3d00000000, 0x0e6c724000000000, 0x9df7bb3f00000000,
    0x285be1bf00000000, 0xbbc028c000000000, 0x0304256400000000,
    0x909fec1b00000000, 0x2533b69b00000000, 0xb6a87fe400000000,
    0x14bcdc0800000000, 0x8727157700000000, 0x328b4ff700000000,
    0xa110868800000000, 0x19d48b2c00000000, 0x8a4f425300000000,
    0x3fe318d300000000, 0xac78d1ac00000000, 0x2614cb5100000000,
    0xb58f022e00000000, 0x002358ae00000000, 0x93b891d100000000,
    0x2b7c9c7500000000, 0xb8e7550a00000000, 0x0d4b0f8a00000000,
    0x9ed0c6f500000000, 0x3cc4651900000000, 0xaf5fac6600000000,
    0x1af3f6e600000000, 0x89683f9900000000, 0x31ac323d00000000,
    0xa237fb4200000000, 0x179ba1c200000000, 0x840068bd00000000,
    0x12b496c000000000, 0x812f5fbf00000000, 0x3483053f00000000,
    0xa718cc4000000000, 0x1fdcc1e400000000, 0x8c47089b00000000,
    0x39eb521b00000000, 0xaa709b6400000000, 0x0864388800000000,
    0x9bfff1f700000000, 0x2e53ab7700000000, 0xbdc8620800000000,
    0x050c6fac00000000, 0x9697a6d300000000, 0x233bfc5300000000,
    0xb0a0352c00000000, 0x0f5201a800000000, 0x9cc9c8d700000000,
    0x2965925700000000, 0xbafe5b2800000000, 0x023a568c00000000,
    0x91a19ff300000000, 0x240dc57300000000, 0xb7960c0c00000000,
    0x1582afe000000000, 0x8619669f00000000, 0x33b53c1f00000000,
    0xa02ef56000000000, 0x18eaf8c400000000, 0x8b7131bb00000000,
    0x3edd6b3b00000000, 0xad46a24400000000, 0x3bf25c3900000000,
    0xa869954600000000, 0x1dc5cfc600000000, 0x8e5e06b900000000,
    0x369a0b1d00000000, 0xa501c26200000000, 0x10ad98e200000000,
    0x8336519d00000000, 0x2122f27100000000, 0xb2b93b0e00000000,
    0x0715618e00000000, 0x948ea8f100000000, 0x2c4aa55500000000,
    0xbfd16c2a00000000, 0x0a7d36aa00000000, 0x99e6ffd500000000,
    0x359e2e7900000000, 0xa605e70600000000, 0x13a9bd8600000000,
    0x803274f900000000, 0x38f6795d00000000, 0xab6db02200000000,
    0x1ec1eaa200000000, 0x8d5a23dd00000000, 0x2f4e803100000000,
    0xbcd5494e00000000, 0x097913ce00000000, 0x9ae2dab100000000,
    0x2226d71500000000, 0xb1bd1e6a00000000, 0x041144ea00000000,
    0x978a8d9500000000, 0x013e73e800000000, 0x92a5ba9700000000,
    0x2709e01700000000, 0xb492296800000000, 0x0c5624cc00000000,
    0x9fcdedb300000000, 0x2a61b73300000000, 0xb9fa7e4c00000000,
    0x1beedda000000000, 0x887514df00000000, 0x3dd94e5f00000000,
    0xae42872000000000, 0x16868a8400000000, 0x851d43fb00000000,
    0x30b1197b00000000, 0xa32ad00400000000, 0x1cd8e48000000000,
    0x8f432dff00000000, 0x3aef777f00000000, 0xa974be0000000000,
    0x11b0b3a400000000, 0x822b7adb00000000, 0x3787205b00000000,
    0xa41ce92400000000, 0x06084ac800000000, 0x959383b700000000,
    0x203fd93700000000, 0xb3a4104800000000, 0x0b601dec00000000,
    0x98fbd49300000000, 0x2d578e1300000000, 0xbecc476c00000000,
    0x2878b91100000000, 0xbbe3706e00000000, 0x0e4f2aee00000000,
    0x9dd4e39100000000, 0x2510ee3500000000, 0xb68b274a00000000,
    0x03277dca00000000, 0x90bcb4b500000000, 0x32a8175900000000,
    0xa133de2600000000, 0x149f84a600000000, 0x87044dd900000000,
    0x3fc0407d00000000, 0xac5b890200000000, 0x19f7d38200000000,
    0x8a6c1afd00000000},
   {0x0000000000000000, 0x650b796900000000, 0xca16f2d200000000,
    0xaf1d8bbb00000000, 0xd52b957e00000000, 0xb020ec1700000000,
    0x1f3d67ac00000000, 0x7a361ec500000000, 0xaa572afd00000000,
    0xcf5c539400000000, 0x6041d82f00000000, 0x054aa14600000000,
    0x7f7cbf8300000000, 0x1a77c6ea00000000, 0xb56a4d5100000000,
    0xd061343800000000, 0x15a9252100000000, 0x70a25c4800000000,
    0xdfbfd7f300000000, 0xbab4ae9a00000000, 0xc082b05f00000000,
    0xa589c93600000000, 0x0a94428d00000000, 0x6f9f3be400000000,
    0xbffe0fdc00000000, 0xdaf576b500000000, 0x75e8fd0e00000000,
    0x10e3846700000000, 0x6ad59aa200000000, 0x0fdee3cb00000000,
    0xa0c3687000000000, 0xc5c8111900000000, 0x2a524b4200000000,
    0x4f59322b00000000, 0xe044b99000000000, 0x854fc0f900000000,
    0xff79de3c00000000, 0x9a72a75500000000, 0x356f2cee00000000,
    0x5064558700000000, 0x800561bf00000000, 0xe50e18d600000000,
    0x4a13936d00000000, 0x2f18ea0400000000, 0x552ef4c100000000,
    0x30258da800000000, 0x9f38061300000000, 0xfa337f7a00000000,
    0x3ffb6e6300000000, 0x5af0170a00000000, 0xf5ed9cb100000000,
    0x90e6e5d800000000, 0xead0fb1d00000000, 0x8fdb827400000000,
    0x20c609cf00000000, 0x45cd70a600000000, 0x95ac449e00000000,
    0xf0a73df700000000, 0x5fbab64c00000000, 0x3ab1cf2500000000,
    0x4087d1e000000000, 0x258ca88900000000, 0x8a91233200000000,
    0xef9a5a5b00000000, 0x54a4968400000000, 0x31afefed00000000,
    0x9eb2645600000000, 0xfbb91d3f00000000, 0x818f03fa00000000,
    0xe4847a9300000000, 0x4b99f12800000000, 0x2e92884100000000,
    0xfef3bc7900000000, 0x9bf8c51000000000, 0x34e54eab00000000,
    0x51ee37c200000000, 0x2bd8290700000000, 0x4ed3506e00000000,
    0xe1cedbd500000000, 0x84c5a2bc00000000, 0x410db3a500000000,
    0x2406cacc00000000, 0x8b1b417700000000, 0xee10381e00000000,
    0x942626db00000000, 0xf12d5fb200000000, 0x5e30d40900000000,
    0x3b3bad6000000000, 0xeb5a995800000000, 0x8e51e03100000000,
    0x214c6b8a00000000, 0x444712e300000000, 0x3e710c2600000000,
    0x5b7a754f00000000, 0xf467fef400000000, 0x916c879d00000000,
    0x7ef6ddc600000000, 0x1bfda4af00000000, 0xb4e02f1400000000,
    0xd1eb567d00000000, 0xabdd48b800000000, 0xced631d100000000,
    0x61cbba6a00000000, 0x04c0c30300000000, 0xd4a1f73b00000000,
    0xb1aa8e5200000000, 0x1eb705e900000000, 0x7bbc7c8000000000,
    0x018a624500000000, 0x64811b2c00000000, 0xcb9c909700000000,
    0xae97e9fe00000000, 0x6b5ff8e700000000, 0x0e54818e00000000,
    0xa1490a3500000000, 0xc442735c00000000, 0xbe746d9900000000,
    0xdb7f14f000000000, 0x74629f4b00000000, 0x1169e62200000000,
    0xc108d21a00000000, 0xa403ab7300000000, 0x0b1e20c800000000,
    0x6e1559a100000000, 0x1423476400000000, 0x71283e0d00000000,
    0xde35b5b600000000, 0xbb3eccdf00000000, 0xe94e5cd200000000,
    0x8c4525bb00000000, 0x2358ae0000000000, 0x4653d76900000000,
    0x3c65c9ac00000000, 0x596eb0c500000000, 0xf6733b7e00000000,
    0x9378421700000000, 0x4319762f00000000, 0x26120f4600000000,
    0x890f84fd00000000, 0xec04fd9400000000, 0x9632e35100000000,
    0xf3399a3800000000, 0x5c24118300000000, 0x392f68ea00000000,
    0xfce779f300000000, 0x99ec009a00000000, 0x36f18b2100000000,
    0x53faf24800000000, 0x29ccec8d00000000, 0x4cc795e400000000,
    0xe3da1e5f00000000, 0x86d1673600000000, 0x56b0530e00000000,
    0x33bb2a6700000000, 0x9ca6a1dc00000000, 0xf9add8b500000000,
    0x839bc67000000000, 0xe690bf1900000000, 0x498d34a200000000,
    0x2c864dcb00000000, 0xc31c179000000000, 0xa6176ef900000000,
    0x090ae54200000000, 0x6c019c2b00000000, 0x163782ee00000000,
    0x733cfb8700000000, 0xdc21703c00000000, 0xb92a095500000000,
    0x694b3d6d00000000, 0x0c40440400000000, 0xa35dcfbf00000000,
    0xc656b6d600000000, 0xbc60a81300000000, 0xd96bd17a00000000,
    0x76765ac100000000, 0x137d23a800000000, 0xd6b532b100000000,
    0xb3be4bd800000000, 0x1ca3c06300000000, 0x79a8b90a00000000,
    0x039ea7cf00000000, 0x6695dea600000000, 0xc988551d00000000,
    0xac832c7400000000, 0x7ce2184c00000000, 0x19e9612500000000,
    0xb6f4ea9e00000000, 0xd3ff93f700000000, 0xa9c98d3200000000,
    0xccc2f45b00000000, 0x63df7fe000000000, 0x06d4068900000000,
    0xbdeaca5600000000, 0xd8e1b33f00000000, 0x77fc388400000000,
    0x12f741ed00000000, 0x68c15f2800000000, 0x0dca264100000000,
    0xa2d7adfa00000000, 0xc7dcd49300000000, 0x17bde0ab00000000,
    0x72b699c200000000, 0xddab127900000000, 0xb8a06b1000000000,
    0xc29675d500000000, 0xa79d0cbc00000000, 0x0880870700000000,
    0x6d8bfe6e00000000, 0xa843ef7700000000, 0xcd48961e00000000,
    0x62551da500000000, 0x075e64cc00000000, 0x7d687a0900000000,
    0x1863036000000000, 0xb77e88db00000000, 0xd275f1b200000000,
    0x0214c58a00000000, 0x671fbce300000000, 0xc802375800000000,
    0xad094e3100000000, 0xd73f50f400000000, 0xb234299d00000000,
    0x1d29a22600000000, 0x7822db4f00000000, 0x97b8811400000000,
    0xf2b3f87d00000000, 0x5dae73c600000000, 0x38a50aaf00000000,
    0x4293146a00000000, 0x27986d0300000000, 0x8885e6b800000000,
    0xed8e9fd100000000, 0x3defabe900000000, 0x58e4d28000000000,
    0xf7f9593b00000000, 0x92f2205200000000, 0xe8c43e9700000000,
    0x8dcf47fe00000000, 0x22d2cc4500000000, 0x47d9b52c00000000,
    0x8211a43500000000, 0xe71add5c00000000, 0x480756e700000000,
    0x2d0c2f8e00000000, 0x573a314b00000000, 0x3231482200000000,
    0x9d2cc39900000000, 0xf827baf000000000, 0x28468ec800000000,
    0x4d4df7a100000000, 0xe2507c1a00000000, 0x875b057300000000,
    0xfd6d1bb600000000, 0x986662df00000000, 0x377be96400000000,
    0x5270900d00000000},
   {0x0000000000000000, 0xdcecb13d00000000, 0xb8d9637b00000000,
    0x6435d24600000000, 0x70b3c7f600000000, 0xac5f76cb00000000,
    0xc86aa48d00000000, 0x148615b000000000, 0xa160fe3600000000,
    0x7d8c4f0b00000000, 0x19b99d4d00000000, 0xc5552c7000000000,
    0xd1d339c000000000, 0x0d3f88fd00000000, 0x690a5abb00000000,
    0xb5e6eb8600000000, 0x42c1fc6d00000000, 0x9e2d4d5000000000,
    0xfa189f1600000000, 0x26f42e2b00000000, 0x32723b9b00000000,
    0xee9e8aa600000000, 0x8aab58e000000000, 0x5647e9dd00000000,
    0xe3a1025b00000000, 0x3f4db36600000000, 0x5b78612000000000,
    0x8794d01d00000000, 0x9312c5ad00000000, 0x4ffe749000000000,
    0x2bcba6d600000000, 0xf72717eb00000000, 0x8482f9db00000000,
    0x586e48e600000000, 0x3c5b9aa000000000, 0xe0b72b9d00000000,
    0xf4313e2d00000000, 0x28dd8f1000000000, 0x4ce85d5600000000,
    0x9004ec6b00000000, 0x25e207ed00000000, 0xf90eb6d000000000,
    0x9d3b649600000000, 0x41d7d5ab00000000, 0x5551c01b00000000,
    0x89bd712600000000, 0xed88a36000000000, 0x3164125d00000000,
    0xc64305b600000000, 0x1aafb48b00000000, 0x7e9a66cd00000000,
    0xa276d7f000000000, 0xb6f0c24000000000, 0x6a1c737d00000000,
    0x0e29a13b00000000, 0xd2c5100600000000, 0x6723fb8000000000,
    0xbbcf4abd00000000, 0xdffa98fb00000000, 0x031629c600000000,
    0x17903c7600000000, 0xcb7c8d4b00000000, 0xaf495f0d00000000,
    0x73a5ee3000000000, 0x4903826c00000000, 0x95ef335100000000,
    0xf1dae11700000000, 0x2d36502a00000000, 0x39b0459a00000000,
    0xe55cf4a700000000, 0x816926e100000000, 0x5d8597dc00000000,
    0xe8637c5a00000000, 0x348fcd6700000000, 0x50ba1f2100000000,
    0x8c56ae1c00000000, 0x98d0bbac00000000, 0x443c0a9100000000,
    0x2009d8d700000000, 0xfce569ea00000000, 0x0bc27e0100000000,
    0xd72ecf3c00000000, 0xb31b1d7a00000000, 0x6ff7ac4700000000,
    0x7b71b9f700000000, 0xa79d08ca00000000, 0xc3a8da8c00000000,
    0x1f446bb100000000, 0xaaa2803700000000, 0x764e310a00000000,
    0x127be34c00000000, 0xce97527100000000, 0xda1147c100000000,
    0x06fdf6fc00000000, 0x62c824ba00000000, 0xbe24958700000000,
    0xcd817bb700000000, 0x116dca8a00000000, 0x755818cc00000000,
    0xa9b4a9f100000000, 0xbd32bc4100000000, 0x61de0d7c00000000,
    0x05ebdf3a00000000, 0xd9076e0700000000, 0x6ce1858100000000,
    0xb00d34bc00000000, 0xd438e6fa00000000, 0x08d457c700000000,
    0x1c52427700000000, 0xc0bef34a00000000, 0xa48b210c00000000,
    0x7867903100000000, 0x8f4087da00000000, 0x53ac36e700000000,
    0x3799e4a100000000, 0xeb75559c00000000, 0xfff3402c00000000,
    0x231ff11100000000, 0x472a235700000000, 0x9bc6926a00000000,
    0x2e2079ec00000000, 0xf2ccc8d100000000, 0x96f91a9700000000,
    0x4a15abaa00000000, 0x5e93be1a00000000, 0x827f0f2700000000,
    0xe64add6100000000, 0x3aa66c5c00000000, 0x920604d900000000,
    0x4eeab5e400000000, 0x2adf67a200000000, 0xf633d69f00000000,
    0xe2b5c32f00000000, 0x3e59721200000000, 0x5a6ca05400000000,
    0x8680116900000000, 0x3366faef00000000, 0xef8a4bd200000000,
    0x8bbf999400000000, 0x575328a900000000, 0x43d53d1900000000,
    0x9f398c2400000000, 0xfb0c5e6200000000, 0x27e0ef5f00000000,
    0xd0c7f8b400000000, 0x0c2b498900000000, 0x681e9bcf00000000,
    0xb4f22af200000000, 0xa0743f4200000000, 0x7c988e7f00000000,
    0x18ad5c3900000000, 0xc441ed0400000000, 0x71a7068200000000,
    0xad4bb7bf00000000, 0xc97e65f900000000, 0x1592d4c400000000,
    0x0114c17400000000, 0xddf8704900000000, 0xb9cda20f00000000,
    0x6521133200000000, 0x1684fd0200000000, 0xca684c3f00000000,
    0xae5d9e7900000000, 0x72b12f4400000000, 0x66373af400000000,
    0xbadb8bc900000000, 0xdeee598f00000000, 0x0202e8b200000000,
    0xb7e4033400000000, 0x6b08b20900000000, 0x0f3d604f00000000,
    0xd3d1d17200000000, 0xc757c4c200000000, 0x1bbb75ff00000000,
    0x7f8ea7b900000000, 0xa362168400000000, 0x5445016f00000000,
    0x88a9b05200000000, 0xec9c621400000000, 0x3070d32900000000,
    0x24f6c69900000000, 0xf81a77a400000000, 0x9c2fa5e200000000,
    0x40c314df00000000, 0xf525ff5900000000, 0x29c94e6400000000,
    0x4dfc9c2200000000, 0x91102d1f00000000, 0x859638af00000000,
    0x597a899200000000, 0x3d4f5bd400000000, 0xe1a3eae900000000,
    0xdb0586b500000000, 0x07e9378800000000, 0x63dce5ce00000000,
    0xbf3054f300000000, 0xabb6414300000000, 0x775af07e00000000,
    0x136f223800000000, 0xcf83930500000000, 0x7a65788300000000,
    0xa689c9be00000000, 0xc2bc1bf800000000, 0x1e50aac500000000,
    0x0ad6bf7500000000, 0xd63a0e4800000000, 0xb20fdc0e00000000,
    0x6ee36d3300000000, 0x99c47ad800000000, 0x4528cbe500000000,
    0x211d19a300000000, 0xfdf1a89e00000000, 0xe977bd2e00000000,
    0x359b0c1300000000, 0x51aede5500000000, 0x8d426f6800000000,
    0x38a484ee00000000, 0xe44835d300000000, 0x807de79500000000,
    0x5c9156a800000000, 0x4817431800000000, 0x94fbf22500000000,
    0xf0ce206300000000, 0x2c22915e00000000, 0x5f877f6e00000000,
    0x836bce5300000000, 0xe75e1c1500000000, 0x3bb2ad2800000000,
    0x2f34b89800000000, 0xf3d809a500000000, 0x97eddbe300000000,
    0x4b016ade00000000, 0xfee7815800000000, 0x220b306500000000,
    0x463ee22300000000, 0x9ad2531e00000000, 0x8e5446ae00000000,
    0x52b8f79300000000, 0x368d25d500000000, 0xea6194e800000000,
    0x1d46830300000000, 0xc1aa323e00000000, 0xa59fe07800000000,
    0x7973514500000000, 0x6df544f500000000, 0xb119f5c800000000,
    0xd52c278e00000000, 0x09c096b300000000, 0xbc267d3500000000,
    0x60cacc0800000000, 0x04ff1e4e00000000, 0xd813af7300000000,
    0xcc95bac300000000, 0x10790bfe00000000, 0x744cd9b800000000,
    0xa8a0688500000000}};

#else /* W == 4 */

local const z_crc_t FAR crc_braid_table[][256] = {
   {0x00000000, 0x81256527, 0xd93bcc0f, 0x581ea928, 0x69069e5f,
    0xe823fb78, 0xb03d5250, 0x31183777, 0xd20d3cbe, 0x53285999,
    0x0b36f0b1, 0x8a139596, 0xbb0ba2e1, 0x3a2ec7c6, 0x62306eee,
    0xe3150bc9, 0x7f6b7f3d, 0xfe4e1a1a, 0xa650b332, 0x2775d615,
    0x166de162, 0x97488445, 0xcf562d6d, 0x4e73484a, 0xad664383,
    0x2c4326a4, 0x745d8f8c, 0xf578eaab, 0xc460dddc, 0x4545b8fb,
    0x1d5b11d3, 0x9c7e74f4, 0xfed6fe7a, 0x7ff39b5d, 0x27ed3275,
    0xa6c85752, 0x97d06025, 0x16f50502, 0x4eebac2a, 0xcfcec90d,
    0x2cdbc2c4, 0xadfea7e3, 0xf5e00ecb, 0x74c56bec, 0x45dd5c9b,
    0xc4f839bc, 0x9ce69094, 0x1dc3f5b3, 0x81bd8147, 0x0098e460,
    0x58864d48, 0xd9a3286f, 0xe8bb1f18, 0x699e7a3f, 0x3180d317,
    0xb0a5b630, 0x53b0bdf9, 0xd295d8de, 0x8a8b71f6, 0x0bae14d1,
    0x3ab623a6, 0xbb934681, 0xe38defa9, 0x62a88a8e, 0x26dcfab5,
    0xa7f99f92, 0xffe736ba, 0x7ec2539d, 0x4fda64ea, 0xceff01cd,
    0x96e1a8e5, 0x17c4cdc2, 0xf4d1c60b, 0x75f4a32c, 0x2dea0a04,
    0xaccf6f23, 0x9dd75854, 0x1cf23d73, 0x44ec945b, 0xc5c9f17c,
    0x59b78588, 0xd892e0af, 0x808c4987, 0x01a92ca0, 0x30b11bd7,
    0xb1947ef0, 0xe98ad7d8, 0x68afb2ff, 0x8bbab936, 0x0a9fdc11,
    0x52817539, 0xd3a4101e, 0xe2bc2769, 0x6399424e, 0x3b87eb66,
    0xbaa28e41, 0xd80a04cf, 0x592f61e8, 0x0131c8c0, 0x8014ade7,
    0xb10c9a90, 0x3029ffb7, 0x6837569f, 0xe91233b8, 0x0a073871,
    0x8b225d56, 0xd33cf47e, 0x52199159, 0x6301a62e, 0xe224c309,
    0xba3a6a21, 0x3b1f0f06, 0xa7617bf2, 0x26441ed5, 0x7e5ab7fd,
    0xff7fd2da, 0xce67e5ad, 0x4f42808a, 0x175c29a2, 0x96794c85,
    0x756c474c, 0xf449226b, 0xac578b43, 0x2d72ee64, 0x1c6ad913,
    0x9d4fbc34, 0xc551151c, 0x4474703b, 0x4db9f56a, 0xcc9c904d,
    0x94823965, 0x15a75c42, 0x24bf6b35, 0xa59a0e12, 0xfd84a73a,
    0x7ca1c21d, 0x9fb4c9d4, 0x1e91acf3, 0x468f05db, 0xc7aa60fc,
    0xf6b2578b, 0x779732ac, 0x2f899b84, 0xaeacfea3, 0x32d28a57,
    0xb3f7ef70, 0xebe94658, 0x6acc237f, 0x5bd41408, 0xdaf1712f,
    0x82efd807, 0x03cabd20, 0xe0dfb6e9, 0x61fad3ce, 0x39e47ae6,
    0xb8c11fc1, 0x89d928b6, 0x08fc4d91, 0x50e2e4b9, 0xd1c7819e,
    0xb36f0b10, 0x324a6e37, 0x6a54c71f, 0xeb71a238, 0xda69954f,
    0x5b4cf068, 0x03525940, 0x82773c67, 0x616237ae, 0xe0475289,
    0xb859fba1, 0x397c9e86, 0x0864a9f1, 0x8941ccd6, 0xd15f65fe,
    0x507a00d9, 0xcc04742d, 0x4d21110a, 0x153fb822, 0x941add05,
    0xa502ea72, 0x24278f55, 0x7c39267d, 0xfd1c435a, 0x1e094893,
    0x9f2c2db4, 0xc732849c, 0x4617e1bb, 0x770fd6cc, 0xf62ab3eb,
    0xae341ac3, 0x2f117fe4, 0x6b650fdf, 0xea406af8, 0xb25ec3d0,
    0x337ba6f7, 0x02639180, 0x8346f4a7, 0xdb585d8f, 0x5a7d38a8,
    0xb9683361, 0x384d5646, 0x6053ff6e, 0xe1769a49, 0xd06ead3e,
    0x514bc819, 0x09556131, 0x88700416, 0x140e70e2, 0x952b15c5,
    0xcd35bced, 0x4c10d9ca, 0x7d08eebd, 0xfc2d8b9a, 0xa43322b2,
    0x25164795, 0xc6034c5c, 0x4726297b, 0x1f388053, 0x9e1de574,
    0xaf05d203, 0x2e20b724, 0x763e1e0c, 0xf71b7b2b, 0x95b3f1a5,
    0x14969482, 0x4c883daa, 0xcdad588d, 0xfcb56ffa, 0x7d900add,
    0x258ea3f5, 0xa4abc6d2, 0x47becd1b, 0xc69ba83c, 0x9e850114,
    0x1fa06433, 0x2eb85344, 0xaf9d3663, 0xf7839f4b, 0x76a6fa6c,
    0xead88e98, 0x6bfdebbf, 0x33e34297, 0xb2c627b0, 0x83de10c7,
    0x02fb75e0, 0x5ae5dcc8, 0xdbc0b9ef, 0x38d5b226, 0xb9f0d701,
    0xe1ee7e29, 0x60cb1b0e, 0x51d32c79, 0xd0f6495e, 0x88e8e076,
    0x09cd8551},
   {0x00000000, 0x9b73ead4, 0xed96d3e9, 0x76e5393d, 0x005ca193,
    0x9b2f4b47, 0xedca727a, 0x76b998ae, 0x00b94326, 0x9bcaa9f2,
    0xed2f90cf, 0x765c7a1b, 0x00e5e2b5, 0x9b960861, 0xed73315c,
    0x7600db88, 0x0172864c, 0x9a016c98, 0xece455a5, 0x7797bf71,
    0x012e27df, 0x9a5dcd0b, 0xecb8f436, 0x77cb1ee2, 0x01cbc56a,
    0x9ab82fbe, 0xec5d1683, 0x772efc57, 0x019764f9, 0x9ae48e2d,
    0xec01b710, 0x77725dc4, 0x02e50c98, 0x9996e64c, 0xef73df71,
    0x740035a5, 0x02b9ad0b, 0x99ca47df, 0xef2f7ee2, 0x745c9436,
    0x025c4fbe, 0x992fa56a, 0xefca9c57, 0x74b97683, 0x0200ee2d,
    0x997304f9, 0xef963dc4, 0x74e5d710, 0x03978ad4, 0x98e46000,
    0xee01593d, 0x7572b3e9, 0x03cb2b47, 0x98b8c193, 0xee5df8ae,
    0x752e127a, 0x032ec9f2, 0x985d2326, 0xeeb81a1b, 0x75cbf0cf,
    0x03726861, 0x980182b5, 0xeee4bb88, 0x7597515c, 0x05ca1930,
    0x9eb9f3e4, 0xe85ccad9, 0x732f200d, 0x0596b8a3, 0x9ee55277,
    0xe8006b4a, 0x7373819e, 0x05735a16, 0x9e00b0c2, 0xe8e589ff,
    0x7396632b, 0x052ffb85, 0x9e5c1151, 0xe8b9286c, 0x73cac2b8,
    0x04b89f7c, 0x9fcb75a8, 0xe92e4c95, 0x725da641, 0x04e43eef,
    0x9f97d43b, 0xe972ed06, 0x720107d2, 0x0401dc5a, 0x9f72368e,
    0xe9970fb3, 0x72e4e567, 0x045d7dc9, 0x9f2e971d, 0xe9cbae20,
    0x72b844f4, 0x072f15a8, 0x9c5cff7c, 0xeab9c641, 0x71ca2c95,
    0x0773b43b, 0x9c005eef, 0xeae567d2, 0x71968d06, 0x0796568e,
    0x9ce5bc5a, 0xea008567, 0x71736fb3, 0x07caf71d, 0x9cb91dc9,
    0xea5c24f4, 0x712fce20, 0x065d93e4, 0x9d2e7930, 0xebcb400d,
    0x70b8aad9, 0x06013277, 0x9d72d8a3, 0xeb97e19e, 0x70e40b4a,
    0x06e4d0c2, 0x9d973a16, 0xeb72032b, 0x7001e9ff, 0x06b87151,
    0x9dcb9b85, 0xeb2ea2b8, 0x705d486c, 0x0b943260, 0x90e7d8b4,
    0xe602e189, 0x7d710b5d, 0x0bc893f3, 0x90bb7927, 0xe65e401a,
    0x7d2daace, 0x0b2d7146, 0x905e9b92, 0xe6bba2af, 0x7dc8487b,
    0x0b71d0d5, 0x90023a01, 0xe6e7033c, 0x7d94e9e8, 0x0ae6b42c,
    0x91955ef8, 0xe77067c5, 0x7c038d11, 0x0aba15bf, 0x91c9ff6b,
    0xe72cc656, 0x7c5f2c82, 0x0a5ff70a, 0x912c1dde, 0xe7c924e3,
    0x7cbace37, 0x0a035699, 0x9170bc4d, 0xe7958570, 0x7ce66fa4,
    0x09713ef8, 0x9202d42c, 0xe4e7ed11, 0x7f9407c5, 0x092d9f6b,
    0x925e75bf, 0xe4bb4c82, 0x7fc8a656, 0x09c87dde, 0x92bb970a,
    0xe45eae37, 0x7f2d44e3, 0x0994dc4d, 0x92e73699, 0xe4020fa4,
    0x7f71e570, 0x0803b8b4, 0x93705260, 0xe5956b5d, 0x7ee68189,
    0x085f1927, 0x932cf3f3, 0xe5c9cace, 0x7eba201a, 0x08bafb92,
    0x93c91146, 0xe52c287b, 0x7e5fc2af, 0x08e65a01, 0x9395b0d5,
    0xe57089e8, 0x7e03633c, 0x0e5e2b50, 0x952dc184, 0xe3c8f8b9,
    0x78bb126d, 0x0e028ac3, 0x95716017, 0xe394592a, 0x78e7b3fe,
    0x0ee76876, 0x959482a2, 0xe371bb9f, 0x7802514b, 0x0ebbc9e5,
    0x95c82331, 0xe32d1a0c, 0x785ef0d8, 0x0f2cad1c, 0x945f47c8,
    0xe2ba7ef5, 0x79c99421, 0x0f700c8f, 0x9403e65b, 0xe2e6df66,
    0x799535b2, 0x0f95ee3a, 0x94e604ee, 0xe2033dd3, 0x7970d707,
    0x0fc94fa9, 0x94baa57d, 0xe25f9c40, 0x792c7694, 0x0cbb27c8,
    0x97c8cd1c, 0xe12df421, 0x7a5e1ef5, 0x0ce7865b, 0x97946c8f,
    0xe17155b2, 0x7a02bf66, 0x0c0264ee, 0x97718e3a, 0xe194b707,
    0x7ae75dd3, 0x0c5ec57d, 0x972d2fa9, 0xe1c81694, 0x7abbfc40,
    0x0dc9a184, 0x96ba4b50, 0xe05f726d, 0x7b2c98b9, 0x0d950017,
    0x96e6eac3, 0xe003d3fe, 0x7b70392a, 0x0d70e2a2, 0x96030876,
    0xe0e6314b, 0x7b95db9f, 0x0d2c4331, 0x965fa9e5, 0xe0ba90d8,
    0x7bc97a0c},
   {0x00000000, 0x172864c0, 0x2e50c980, 0x3978ad40, 0x5ca19300,
    0x4b89f7c0, 0x72f15a80, 0x65d93e40, 0xb9432600, 0xae6b42c0,
    0x9713ef80, 0x803b8b40, 0xe5e2b500, 0xf2cad1c0, 0xcbb27c80,
    0xdc9a1840, 0xa9f74a41, 0xbedf2e81, 0x87a783c1, 0x908fe701,
    0xf556d941, 0xe27ebd81, 0xdb0610c1, 0xcc2e7401, 0x10b46c41,
    0x079c0881, 0x3ee4a5c1, 0x29ccc101, 0x4c15ff41, 0x5b3d9b81,
    0x624536c1, 0x756d5201, 0x889f92c3, 0x9fb7f603, 0xa6cf5b43,
    0xb1e73f83, 0xd43e01c3, 0xc3166503, 0xfa6ec843, 0xed46ac83,
    0x31dcb4c3, 0x26f4d003, 0x1f8c7d43, 0x08a41983, 0x6d7d27c3,
    0x7a554303, 0x432dee43, 0x54058a83, 0x2168d882, 0x3640bc42,
    0x0f381102, 0x181075c2, 0x7dc94b82, 0x6ae12f42, 0x53998202,
    0x44b1e6c2, 0x982bfe82, 0x8f039a42, 0xb67b3702, 0xa15353c2,
    0xc48a6d82, 0xd3a20942, 0xeadaa402, 0xfdf2c0c2, 0xca4e23c7,
    0xdd664707, 0xe41eea47, 0xf3368e87, 0x96efb0c7, 0x81c7d407,
    0xb8bf7947, 0xaf971d87, 0x730d05c7, 0x64256107, 0x5d5dcc47,
    0x4a75a887, 0x2fac96c7, 0x3884f207, 0x01fc5f47, 0x16d43b87,
    0x63b96986, 0x74910d46, 0x4de9a006, 0x5ac1c4c6, 0x3f18fa86,
    0x28309e46, 0x11483306, 0x066057c6, 0xdafa4f86, 0xcdd22b46,
    0xf4aa8606, 0xe382e2c6, 0x865bdc86, 0x9173b846, 0xa80b1506,
    0xbf2371c6, 0x42d1b104, 0x55f9d5c4, 0x6c817884, 0x7ba91c44,
    0x1e702204, 0x095846c4, 0x3020eb84, 0x27088f44, 0xfb929704,
    0xecbaf3c4, 0xd5c25e84, 0xc2ea3a44, 0xa7330404, 0xb01b60c4,
    0x8963cd84, 0x9e4ba944, 0xeb26fb45, 0xfc0e9f85, 0xc57632c5,
    0xd25e5605, 0xb7876845, 0xa0af0c85, 0x99d7a1c5, 0x8effc505,
    0x5265dd45, 0x454db985, 0x7c3514c5, 0x6b1d7005, 0x0ec44e45,
    0x19ec2a85, 0x209487c5, 0x37bce305, 0x4fed41cf, 0x58c5250f,
    0x61bd884f, 0x7695ec8f, 0x134cd2cf, 0x0464b60f, 0x3d1c1b4f,
    0x2a347f8f, 0xf6ae67cf, 0xe186030f, 0xd8feae4f, 0xcfd6ca8f,
    0xaa0ff4cf, 0xbd27900f, 0x845f3d4f, 0x9377598f, 0xe61a0b8e,
    0xf1326f4e, 0xc84ac20e, 0xdf62a6ce, 0xbabb988e, 0xad93fc4e,
    0x94eb510e, 0x83c335ce, 0x5f592d8e, 0x4871494e, 0x7109e40e,
    0x662180ce, 0x03f8be8e, 0x14d0da4e, 0x2da8770e, 0x3a8013ce,
    0xc772d30c, 0xd05ab7cc, 0xe9221a8c, 0xfe0a7e4c, 0x9bd3400c,
    0x8cfb24cc, 0xb583898c, 0xa2abed4c, 0x7e31f50c, 0x691991cc,
    0x50613c8c, 0x4749584c, 0x2290660c, 0x35b802cc, 0x0cc0af8c,
    0x1be8cb4c, 0x6e85994d, 0x79adfd8d, 0x40d550cd, 0x57fd340d,
    0x32240a4d, 0x250c6e8d, 0x1c74c3cd, 0x0b5ca70d, 0xd7c6bf4d,
    0xc0eedb8d, 0xf99676cd, 0xeebe120d, 0x8b672c4d, 0x9c4f488d,
    0xa537e5cd, 0xb21f810d, 0x85a36208, 0x928b06c8, 0xabf3ab88,
    0xbcdbcf48, 0xd902f108, 0xce2a95c8, 0xf7523888, 0xe07a5c48,
    0x3ce04408, 0x2bc820c8, 0x12b08d88, 0x0598e948, 0x6041d708,
    0x7769b3c8, 0x4e111e88, 0x59397a48, 0x2c542849, 0x3b7c4c89,
    0x0204e1c9, 0x152c8509, 0x70f5bb49, 0x67dddf89, 0x5ea572c9,
    0x498d1609, 0x95170e49, 0x823f6a89, 0xbb47c7c9, 0xac6fa309,
    0xc9b69d49, 0xde9ef989, 0xe7e654c9, 0xf0ce3009, 0x0d3cf0cb,
    0x1a14940b, 0x236c394b, 0x34445d8b, 0x519d63cb, 0x46b5070b,
    0x7fcdaa4b, 0x68e5ce8b, 0xb47fd6cb, 0xa357b20b, 0x9a2f1f4b,
    0x8d077b8b, 0xe8de45cb, 0xfff6210b, 0xc68e8c4b, 0xd1a6e88b,
    0xa4cbba8a, 0xb3e3de4a, 0x8a9b730a, 0x9db317ca, 0xf86a298a,
    0xef424d4a, 0xd63ae00a, 0xc11284ca, 0x1d889c8a, 0x0aa0f84a,
    0x33d8550a, 0x24f031ca, 0x41290f8a, 0x56016b4a, 0x6f79c60a,
    0x7851a2ca},
   {0x00000000, 0x9fda839e, 0xe4c4017d, 0x7b1e82e3, 0x12f904bb,
    0x8d238725, 0xf63d05c6, 0x69e78658, 0x25f20976, 0xba288ae8,
    0xc136080b, 0x5eec8b95, 0x370b0dcd, 0xa8d18e53, 0xd3cf0cb0,
    0x4c158f2e, 0x4be412ec, 0xd43e9172, 0xaf201391, 0x30fa900f,
    0x591d1657, 0xc6c795c9, 0xbdd9172a, 0x220394b4, 0x6e161b9a,
    0xf1cc9804, 0x8ad21ae7, 0x15089979, 0x7cef1f21, 0xe3359cbf,
    0x982b1e5c, 0x07f19dc2, 0x97c825d8, 0x0812a646, 0x730c24a5,
    0xecd6a73b, 0x85312163, 0x1aeba2fd, 0x61f5201e, 0xfe2fa380,
    0xb23a2cae, 0x2de0af30, 0x56fe2dd3, 0xc924ae4d, 0xa0c32815,
    0x3f19ab8b, 0x44072968, 0xdbddaaf6, 0xdc2c3734, 0x43f6b4aa,
    0x38e83649, 0xa732b5d7, 0xced5338f, 0x510fb011, 0x2a1132f2,
    0xb5cbb16c, 0xf9de3e42, 0x6604bddc, 0x1d1a3f3f, 0x82c0bca1,
    0xeb273af9, 0x74fdb967, 0x0fe33b84, 0x9039b81a, 0xf4e14df1,
    0x6b3bce6f, 0x10254c8c, 0x8fffcf12, 0xe618494a, 0x79c2cad4,
    0x02dc4837, 0x9d06cba9, 0xd1134487, 0x4ec9c719, 0x35d745fa,
    0xaa0dc664, 0xc3ea403c, 0x5c30c3a2, 0x272e4141, 0xb8f4c2df,
    0xbf055f1d, 0x20dfdc83, 0x5bc15e60, 0xc41bddfe, 0xadfc5ba6,
    0x3226d838, 0x49385adb, 0xd6e2d945, 0x9af7566b, 0x052dd5f5,
    0x7e335716, 0xe1e9d488, 0x880e52d0, 0x17d4d14e, 0x6cca53ad,
    0xf310d033, 0x63296829, 0xfcf3ebb7, 0x87ed6954, 0x1837eaca,
    0x71d06c92, 0xee0aef0c, 0x95146def, 0x0aceee71, 0x46db615f,
    0xd901e2c1, 0xa21f6022, 0x3dc5e3bc, 0x542265e4, 0xcbf8e67a,
    0xb0e66499, 0x2f3ce707, 0x28cd7ac5, 0xb717f95b, 0xcc097bb8,
    0x53d3f826, 0x3a347e7e, 0xa5eefde0, 0xdef07f03, 0x412afc9d,
    0x0d3f73b3, 0x92e5f02d, 0xe9fb72ce, 0x7621f150, 0x1fc67708,
    0x801cf496, 0xfb027675, 0x64d8f5eb, 0x32b39da3, 0xad691e3d,
    0xd6779cde, 0x49ad1f40, 0x204a9918, 0xbf901a86, 0xc48e9865,
    0x5b541bfb, 0x174194d5, 0x889b174b, 0xf38595a8, 0x6c5f1636,
    0x05b8906e, 0x9a6213f0, 0xe17c9113, 0x7ea6128d, 0x79578f4f,
    0xe68d0cd1, 0x9d938e32, 0x02490dac, 0x6bae8bf4, 0xf474086a,
    0x8f6a8a89, 0x10b00917, 0x5ca58639, 0xc37f05a7, 0xb8618744,
    0x27bb04da, 0x4e5c8282, 0xd186011c, 0xaa9883ff, 0x35420061,
    0xa57bb87b, 0x3aa13be5, 0x41bfb906, 0xde653a98, 0xb782bcc0,
    0x28583f5e, 0x5346bdbd, 0xcc9c3e23, 0x8089b10d, 0x1f533293,
    0x644db070, 0xfb9733ee, 0x9270b5b6, 0x0daa3628, 0x76b4b4cb,
    0xe96e3755, 0xee9faa97, 0x71452909, 0x0a5babea, 0x95812874,
    0xfc66ae2c, 0x63bc2db2, 0x18a2af51, 0x87782ccf, 0xcb6da3e1,
    0x54b7207f, 0x2fa9a29c, 0xb0732102, 0xd994a75a, 0x464e24c4,
    0x3d50a627, 0xa28a25b9, 0xc652d052, 0x598853cc, 0x2296d12f,
    0xbd4c52b1, 0xd4abd4e9, 0x4b715777, 0x306fd594, 0xafb5560a,
    0xe3a0d924, 0x7c7a5aba, 0x0764d859, 0x98be5bc7, 0xf159dd9f,
    0x6e835e01, 0x159ddce2, 0x8a475f7c, 0x8db6c2be, 0x126c4120,
    0x6972c3c3, 0xf6a8405d, 0x9f4fc605, 0x0095459b, 0x7b8bc778,
    0xe45144e6, 0xa844cbc8, 0x379e4856, 0x4c80cab5, 0xd35a492b,
    0xbabdcf73, 0x25674ced, 0x5e79ce0e, 0xc1a34d90, 0x519af58a,
    0xce407614, 0xb55ef4f7, 0x2a847769, 0x4363f131, 0xdcb972af,
    0xa7a7f04c, 0x387d73d2, 0x7468fcfc, 0xebb27f62, 0x90acfd81,
    0x0f767e1f, 0x6691f847, 0xf94b7bd9, 0x8255f93a, 0x1d8f7aa4,
    0x1a7ee766, 0x85a464f8, 0xfebae61b, 0x61606585, 0x0887e3dd,
    0x975d6043, 0xec43e2a0, 0x7399613e, 0x3f8cee10, 0xa0566d8e,
    0xdb48ef6d, 0x44926cf3, 0x2d75eaab, 0xb2af6935, 0xc9b1ebd6,
    0x566b6848}};

local const z_word_t FAR crc_braid_big_table[][256] = {
   {0x00000000, 0x9e83da9f, 0x7d01c4e4, 0xe3821e7b, 0xbb04f912,
    0x2587238d, 0xc6053df6, 0x5886e769, 0x7609f225, 0xe88a28ba,
    0x0b0836c1, 0x958bec5e, 0xcd0d0b37, 0x538ed1a8, 0xb00ccfd3,
    0x2e8f154c, 0xec12e44b, 0x72913ed4, 0x911320af, 0x0f90fa30,
    0x57161d59, 0xc995c7c6, 0x2a17d9bd, 0xb4940322, 0x9a1b166e,
    0x0498ccf1, 0xe71ad28a, 0x79990815, 0x211fef7c, 0xbf9c35e3,
    0x5c1e2b98, 0xc29df107, 0xd825c897, 0x46a61208, 0xa5240c73,
    0x3ba7d6ec, 0x63213185, 0xfda2eb1a, 0x1e20f561, 0x80a32ffe,
    0xae2c3ab2, 0x30afe02d, 0xd32dfe56, 0x4dae24c9, 0x1528c3a0,
    0x8bab193f, 0x68290744, 0xf6aadddb, 0x34372cdc, 0xaab4f643,
    0x4936e838, 0xd7b532a7, 0x8f33d5ce, 0x11b00f51, 0xf232112a,
    0x6cb1cbb5, 0x423edef9, 0xdcbd0466, 0x3f3f1a1d, 0xa1bcc082,
    0xf93a27eb, 0x67b9fd74, 0x843be30f, 0x1ab83990, 0xf14de1f4,
    0x6fce3b6b, 0x8c4c2510, 0x12cfff8f, 0x4a4918e6, 0xd4cac279,
    0x3748dc02, 0xa9cb069d, 0x874413d1, 0x19c7c94e, 0xfa45d735,
    0x64c60daa, 0x3c40eac3, 0xa2c3305c, 0x41412e27, 0xdfc2f4b8,
    0x1d5f05bf, 0x83dcdf20, 0x605ec15b, 0xfedd1bc4, 0xa65bfcad,
    0x38d82632, 0xdb5a3849, 0x45d9e2d6, 0x6b56f79a, 0xf5d52d05,
    0x1657337e, 0x88d4e9e1, 0xd0520e88, 0x4ed1d417, 0xad53ca6c,
    0x33d010f3, 0x29682963, 0xb7ebf3fc, 0x5469ed87, 0xcaea3718,
    0x926cd071, 0x0cef0aee, 0xef6d1495, 0x71eece0a, 0x5f61db46,
    0xc1e201d9, 0x22601fa2, 0xbce3c53d, 0xe4652254, 0x7ae6f8cb,
    0x9964e6b0, 0x07e73c2f, 0xc57acd28, 0x5bf917b7, 0xb87b09cc,
    0x26f8d353, 0x7e7e343a, 0xe0fdeea5, 0x037ff0de, 0x9dfc2a41,
    0xb3733f0d, 0x2df0e592, 0xce72fbe9, 0x50f12176, 0x0877c61f,
    0x96f41c80, 0x757602fb, 0xebf5d864, 0xa39db332, 0x3d1e69ad,
    0xde9c77d6, 0x401fad49, 0x18994a20, 0x861a90bf, 0x65988ec4,
    0xfb1b545b, 0xd5944117, 0x4b179b88, 0xa89585f3, 0x36165f6c,
    0x6e90b805, 0xf013629a, 0x13917ce1, 0x8d12a67e, 0x4f8f5779,
    0xd10c8de6, 0x328e939d, 0xac0d4902, 0xf48bae6b, 0x6a0874f4,
    0x898a6a8f, 0x1709b010, 0x3986a55c, 0xa7057fc3, 0x448761b8,
    0xda04bb27, 0x82825c4e, 0x1c0186d1, 0xff8398aa, 0x61004235,
    0x7bb87ba5, 0xe53ba13a, 0x06b9bf41, 0x983a65de, 0xc0bc82b7,
    0x5e3f5828, 0xbdbd4653, 0x233e9ccc, 0x0db18980, 0x9332531f,
    0x70b04d64, 0xee3397fb, 0xb6b57092, 0x2836aa0d, 0xcbb4b476,
    0x55376ee9, 0x97aa9fee, 0x09294571, 0xeaab5b0a, 0x74288195,
    0x2cae66fc, 0xb22dbc63, 0x51afa218, 0xcf2c7887, 0xe1a36dcb,
    0x7f20b754, 0x9ca2a92f, 0x022173b0, 0x5aa794d9, 0xc4244e46,
    0x27a6503d, 0xb9258aa2, 0x52d052c6, 0xcc538859, 0x2fd19622,
    0xb1524cbd, 0xe9d4abd4, 0x7757714b, 0x94d56f30, 0x0a56b5af,
    0x24d9a0e3, 0xba5a7a7c, 0x59d86407, 0xc75bbe98, 0x9fdd59f1,
    0x015e836e, 0xe2dc9d15, 0x7c5f478a, 0xbec2b68d, 0x20416c12,
    0xc3c37269, 0x5d40a8f6, 0x05c64f9f, 0x9b459500, 0x78c78b7b,
    0xe64451e4, 0xc8cb44a8, 0x56489e37, 0xb5ca804c, 0x2b495ad3,
    0x73cfbdba, 0xed4c6725, 0x0ece795e, 0x904da3c1, 0x8af59a51,
    0x147640ce, 0xf7f45eb5, 0x6977842a, 0x31f16343, 0xaf72b9dc,
    0x4cf0a7a7, 0xd2737d38, 0xfcfc6874, 0x627fb2eb, 0x81fdac90,
    0x1f7e760f, 0x47f89166, 0xd97b4bf9, 0x3af95582, 0xa47a8f1d,
    0x66e77e1a, 0xf864a485, 0x1be6bafe, 0x85656061, 0xdde38708,
    0x43605d97, 0xa0e243ec, 0x3e619973, 0x10ee8c3f, 0x8e6d56a0,
    0x6def48db, 0xf36c9244, 0xabea752d, 0x3569afb2, 0xd6ebb1c9,
    0x48686b56},
   {0x00000000, 0xc0642817, 0x80c9502e, 0x40ad7839, 0x0093a15c,
    0xc0f7894b, 0x805af172, 0x403ed965, 0x002643b9, 0xc0426bae,
    0x80ef1397, 0x408b3b80, 0x00b5e2e5, 0xc0d1caf2, 0x807cb2cb,
    0x40189adc, 0x414af7a9, 0x812edfbe, 0xc183a787, 0x01e78f90,
    0x41d956f5, 0x81bd7ee2, 0xc11006db, 0x01742ecc, 0x416cb410,
    0x81089c07, 0xc1a5e43e, 0x01c1cc29, 0x41ff154c, 0x819b3d5b,
    0xc1364562, 0x01526d75, 0xc3929f88, 0x03f6b79f, 0x435bcfa6,
    0x833fe7b1, 0xc3013ed4, 0x036516c3, 0x43c86efa, 0x83ac46ed,
    0xc3b4dc31, 0x03d0f426, 0x437d8c1f, 0x8319a408, 0xc3277d6d,
    0x0343557a, 0x43ee2d43, 0x838a0554, 0x82d86821, 0x42bc4036,
    0x0211380f, 0xc2751018, 0x824bc97d, 0x422fe16a, 0x02829953,
    0xc2e6b144, 0x82fe2b98, 0x429a038f, 0x02377bb6, 0xc25353a1,
    0x826d8ac4, 0x4209a2d3, 0x02a4daea, 0xc2c0f2fd, 0xc7234eca,
    0x074766dd, 0x47ea1ee4, 0x878e36f3, 0xc7b0ef96, 0x07d4c781,
    0x4779bfb8, 0x871d97af, 0xc7050d73, 0x07612564, 0x47cc5d5d,
    0x87a8754a, 0xc796ac2f, 0x07f28438, 0x475ffc01, 0x873bd416,
    0x8669b963, 0x460d9174, 0x06a0e94d, 0xc6c4c15a, 0x86fa183f,
    0x469e3028, 0x06334811, 0xc6576006, 0x864ffada, 0x462bd2cd,
    0x0686aaf4, 0xc6e282e3, 0x86dc5b86, 0x46b87391, 0x06150ba8,
    0xc67123bf, 0x04b1d142, 0xc4d5f955, 0x8478816c, 0x441ca97b,
    0x0422701e, 0xc4465809, 0x84eb2030, 0x448f0827, 0x049792fb,
    0xc4f3baec, 0x845ec2d5, 0x443aeac2, 0x040433a7, 0xc4601bb0,
    0x84cd6389, 0x44a94b9e, 0x45fb26eb, 0x859f0efc, 0xc53276c5,
    0x05565ed2, 0x456887b7, 0x850cafa0, 0xc5a1d799, 0x05c5ff8e,
    0x45dd6552, 0x85b94d45, 0xc514357c, 0x05701d6b, 0x454ec40e,
    0x852aec19, 0xc5879420, 0x05e3bc37, 0xcf41ed4f, 0x0f25c558,
    0x4f88bd61, 0x8fec9576, 0xcfd24c13, 0x0fb66404, 0x4f1b1c3d,
    0x8f7f342a, 0xcf67aef6, 0x0f0386e1, 0x4faefed8, 0x8fcad6cf,
    0xcff40faa, 0x0f9027bd, 0x4f3d5f84, 0x8f597793, 0x8e0b1ae6,
    0x4e6f32f1, 0x0ec24ac8, 0xcea662df, 0x8e98bbba, 0x4efc93ad,
    0x0e51eb94, 0xce35c383, 0x8e2d595f, 0x4e497148, 0x0ee40971,
    0xce802166, 0x8ebef803, 0x4edad014, 0x0e77a82d, 0xce13803a,
    0x0cd372c7, 0xccb75ad0, 0x8c1a22e9, 0x4c7e0afe, 0x0c40d39b,
    0xcc24fb8c, 0x8c8983b5, 0x4cedaba2, 0x0cf5317e, 0xcc911969,
    0x8c3c6150, 0x4c584947, 0x0c669022, 0xcc02b835, 0x8cafc00c,
    0x4ccbe81b, 0x4d99856e, 0x8dfdad79, 0xcd50d540, 0x0d34fd57,
    0x4d0a2432, 0x8d6e0c25, 0xcdc3741c, 0x0da75c0b, 0x4dbfc6d7,
    0x8ddbeec0, 0xcd7696f9, 0x0d12beee, 0x4d2c678b, 0x8d484f9c,
    0xcde537a5, 0x0d811fb2, 0x0862a385, 0xc8068b92, 0x88abf3ab,
    0x48cfdbbc, 0x08f102d9, 0xc8952ace, 0x883852f7, 0x485c7ae0,
    0x0844e03c, 0xc820c82b, 0x888db012, 0x48e99805, 0x08d74160,
    0xc8b36977, 0x881e114e, 0x487a3959, 0x4928542c, 0x894c7c3b,
    0xc9e10402, 0x09852c15, 0x49bbf570, 0x89dfdd67, 0xc972a55e,
    0x09168d49, 0x490e1795, 0x896a3f82, 0xc9c747bb, 0x09a36fac,
    0x499db6c9, 0x89f99ede, 0xc954e6e7, 0x0930cef0, 0xcbf03c0d,
    0x0b94141a, 0x4b396c23, 0x8b5d4434, 0xcb639d51, 0x0b07b546,
    0x4baacd7f, 0x8bcee568, 0xcbd67fb4, 0x0bb257a3, 0x4b1f2f9a,
    0x8b7b078d, 0xcb45dee8, 0x0b21f6ff, 0x4b8c8ec6, 0x8be8a6d1,
    0x8abacba4, 0x4adee3b3, 0x0a739b8a, 0xca17b39d, 0x8a296af8,
    0x4a4d42ef, 0x0ae03ad6, 0xca8412c1, 0x8a9c881d, 0x4af8a00a,
    0x0a55d833, 0xca31f024, 0x8a0f2941, 0x4a6b0156, 0x0ac6796f,
    0xcaa25178},
   {0x00000000, 0xd4ea739b, 0xe9d396ed, 0x3d39e576, 0x93a15c00,
    0x474b2f9b, 0x7a72caed, 0xae98b976, 0x2643b900, 0xf2a9ca9b,
    0xcf902fed, 0x1b7a5c76, 0xb5e2e500, 0x6108969b, 0x5c3173ed,
    0x88db0076, 0x4c867201, 0x986c019a, 0xa555e4ec, 0x71bf9777,
    0xdf272e01, 0x0bcd5d9a, 0x36f4b8ec, 0xe21ecb77, 0x6ac5cb01,
    0xbe2fb89a, 0x83165dec, 0x57fc2e77, 0xf9649701, 0x2d8ee49a,
    0x10b701ec, 0xc45d7277, 0x980ce502, 0x4ce69699, 0x71df73ef,
    0xa5350074, 0x0badb902, 0xdf47ca99, 0xe27e2fef, 0x36945c74,
    0xbe4f5c02, 0x6aa52f99, 0x579ccaef, 0x8376b974, 0x2dee0002,
    0xf9047399, 0xc43d96ef, 0x10d7e574, 0xd48a9703, 0x0060e498,
    0x3d5901ee, 0xe9b37275, 0x472bcb03, 0x93c1b898, 0xaef85dee,
    0x7a122e75, 0xf2c92e03, 0x26235d98, 0x1b1ab8ee, 0xcff0cb75,
    0x61687203, 0xb5820198, 0x88bbe4ee, 0x5c519775, 0x3019ca05,
    0xe4f3b99e, 0xd9ca5ce8, 0x0d202f73, 0xa3b89605, 0x7752e59e,
    0x4a6b00e8, 0x9e817373, 0x165a7305, 0xc2b0009e, 0xff89e5e8,
    0x2b639673, 0x85fb2f05, 0x51115c9e, 0x6c28b9e8, 0xb8c2ca73,
    0x7c9fb804, 0xa875cb9f, 0x954c2ee9, 0x41a65d72, 0xef3ee404,
    0x3bd4979f, 0x06ed72e9, 0xd2070172, 0x5adc0104, 0x8e36729f,
    0xb30f97e9, 0x67e5e472, 0xc97d5d04, 0x1d972e9f, 0x20aecbe9,
    0xf444b872, 0xa8152f07, 0x7cff5c9c, 0x41c6b9ea, 0x952cca71,
    0x3bb47307, 0xef5e009c, 0xd267e5ea, 0x068d9671, 0x8e569607,
    0x5abce59c, 0x678500ea, 0xb36f7371, 0x1df7ca07, 0xc91db99c,
    0xf4245cea, 0x20ce2f71, 0xe4935d06, 0x30792e9d, 0x0d40cbeb,
    0xd9aab870, 0x77320106, 0xa3d8729d, 0x9ee197eb, 0x4a0be470,
    0xc2d0e406, 0x163a979d, 0x2b0372eb, 0xffe90170, 0x5171b806,
    0x859bcb9d, 0xb8a22eeb, 0x6c485d70, 0x6032940b, 0xb4d8e790,
    0x89e102e6, 0x5d0b717d, 0xf393c80b, 0x2779bb90, 0x1a405ee6,
    0xceaa2d7d, 0x46712d0b, 0x929b5e90, 0xafa2bbe6, 0x7b48c87d,
    0xd5d0710b, 0x013a0290, 0x3c03e7e6, 0xe8e9947d, 0x2cb4e60a,
    0xf85e9591, 0xc56770e7, 0x118d037c, 0xbf15ba0a, 0x6bffc991,
    0x56c62ce7, 0x822c5f7c, 0x0af75f0a, 0xde1d2c91, 0xe324c9e7,
    0x37ceba7c, 0x9956030a, 0x4dbc7091, 0x708595e7, 0xa46fe67c,
    0xf83e7109, 0x2cd40292, 0x11ede7e4, 0xc507947f, 0x6b9f2d09,
    0xbf755e92, 0x824cbbe4, 0x56a6c87f, 0xde7dc809, 0x0a97bb92,
    0x37ae5ee4, 0xe3442d7f, 0x4ddc9409, 0x9936e792, 0xa40f02e4,
    0x70e5717f, 0xb4b80308, 0x60527093, 0x5d6b95e5, 0x8981e67e,
    0x27195f08, 0xf3f32c93, 0xcecac9e5, 0x1a20ba7e, 0x92fbba08,
    0x4611c993, 0x7b282ce5, 0xafc25f7e, 0x015ae608, 0xd5b09593,
    0xe88970e5, 0x3c63037e, 0x502b5e0e, 0x84c12d95, 0xb9f8c8e3,
    0x6d12bb78, 0xc38a020e, 0x17607195, 0x2a5994e3, 0xfeb3e778,
    0x7668e70e, 0xa2829495, 0x9fbb71e3, 0x4b510278, 0xe5c9bb0e,
    0x3123c895, 0x0c1a2de3, 0xd8f05e78, 0x1cad2c0f, 0xc8475f94,
    0xf57ebae2, 0x2194c979, 0x8f0c700f, 0x5be60394, 0x66dfe6e2,
    0xb2359579, 0x3aee950f, 0xee04e694, 0xd33d03e2, 0x07d77079,
    0xa94fc90f, 0x7da5ba94, 0x409c5fe2, 0x94762c79, 0xc827bb0c,
    0x1ccdc897, 0x21f42de1, 0xf51e5e7a, 0x5b86e70c, 0x8f6c9497,
    0xb25571e1, 0x66bf027a, 0xee64020c, 0x3a8e7197, 0x07b794e1,
    0xd35de77a, 0x7dc55e0c, 0xa92f2d97, 0x9416c8e1, 0x40fcbb7a,
    0x84a1c90d, 0x504bba96, 0x6d725fe0, 0xb9982c7b, 0x1700950d,
    0xc3eae696, 0xfed303e0, 0x2a39707b, 0xa2e2700d, 0x76080396,
    0x4b31e6e0, 0x9fdb957b, 0x31432c0d, 0xe5a95f96, 0xd890bae0,
    0x0c7ac97b},
   {0x00000000, 0x27652581, 0x0fcc3bd9, 0x28a91e58, 0x5f9e0669,
    0x78fb23e8, 0x50523db0, 0x77371831, 0xbe3c0dd2, 0x99592853,
    0xb1f0360b, 0x9695138a, 0xe1a20bbb, 0xc6c72e3a, 0xee6e3062,
    0xc90b15e3, 0x3d7f6b7f, 0x1a1a4efe, 0x32b350a6, 0x15d67527,
    0x62e16d16, 0x45844897, 0x6d2d56cf, 0x4a48734e, 0x834366ad,
    0xa426432c, 0x8c8f5d74, 0xabea78f5, 0xdcdd60c4, 0xfbb84545,
    0xd3115b1d, 0xf4747e9c, 0x7afed6fe, 0x5d9bf37f, 0x7532ed27,
    0x5257c8a6, 0x2560d097, 0x0205f516, 0x2aaceb4e, 0x0dc9cecf,
    0xc4c2db2c, 0xe3a7fead, 0xcb0ee0f5, 0xec6bc574, 0x9b5cdd45,
    0xbc39f8c4, 0x9490e69c, 0xb3f5c31d, 0x4781bd81, 0x60e49800,
    0x484d8658, 0x6f28a3d9, 0x181fbbe8, 0x3f7a9e69, 0x17d38031,
    0x30b6a5b0, 0xf9bdb053, 0xded895d2, 0xf6718b8a, 0xd114ae0b,
    0xa623b63a, 0x814693bb, 0xa9ef8de3, 0x8e8aa862, 0xb5fadc26,
    0x929ff9a7, 0xba36e7ff, 0x9d53c27e, 0xea64da4f, 0xcd01ffce,
    0xe5a8e196, 0xc2cdc417, 0x0bc6d1f4, 0x2ca3f475, 0x040aea2d,
    0x236fcfac, 0x5458d79d, 0x733df21c, 0x5b94ec44, 0x7cf1c9c5,
    0x8885b759, 0xafe092d8, 0x87498c80, 0xa02ca901, 0xd71bb130,
    0xf07e94b1, 0xd8d78ae9, 0xffb2af68, 0x36b9ba8b, 0x11dc9f0a,
    0x39758152, 0x1e10a4d3, 0x6927bce2, 0x4e429963, 0x66eb873b,
    0x418ea2ba, 0xcf040ad8, 0xe8612f59, 0xc0c83101, 0xe7ad1480,
    0x909a0cb1, 0xb7ff2930, 0x9f563768, 0xb83312e9, 0x7138070a,
    0x565d228b, 0x7ef43cd3, 0x59911952, 0x2ea60163, 0x09c324e2,
    0x216a3aba, 0x060f1f3b, 0xf27b61a7, 0xd51e4426, 0xfdb75a7e,
    0xdad27fff, 0xade567ce, 0x8a80424f, 0xa2295c17, 0x854c7996,
    0x4c476c75, 0x6b2249f4, 0x438b57ac, 0x64ee722d, 0x13d96a1c,
    0x34bc4f9d, 0x1c1551c5, 0x3b707444, 0x6af5b94d, 0x4d909ccc,
    0x65398294, 0x425ca715, 0x356bbf24, 0x120e9aa5, 0x3aa784fd,
    0x1dc2a17c, 0xd4c9b49f, 0xf3ac911e, 0xdb058f46, 0xfc60aac7,
    0x8b57b2f6, 0xac329777, 0x849b892f, 0xa3feacae, 0x578ad232,
    0x70eff7b3, 0x5846e9eb, 0x7f23cc6a, 0x0814d45b, 0x2f71f1da,
    0x07d8ef82, 0x20bdca03, 0xe9b6dfe0, 0xced3fa61, 0xe67ae439,
    0xc11fc1b8, 0xb628d989, 0x914dfc08, 0xb9e4e250, 0x9e81c7d1,
    0x100b6fb3, 0x376e4a32, 0x1fc7546a, 0x38a271eb, 0x4f9569da,
    0x68f04c5b, 0x40595203, 0x673c7782, 0xae376261, 0x895247e0,
    0xa1fb59b8, 0x869e7c39, 0xf1a96408, 0xd6cc4189, 0xfe655fd1,
    0xd9007a50, 0x2d7404cc, 0x0a11214d, 0x22b83f15, 0x05dd1a94,
    0x72ea02a5, 0x558f2724, 0x7d26397c, 0x5a431cfd, 0x9348091e,
    0xb42d2c9f, 0x9c8432c7, 0xbbe11746, 0xccd60f77, 0xebb32af6,
    0xc31a34ae, 0xe47f112f, 0xdf0f656b, 0xf86a40ea, 0xd0c35eb2,
    0xf7a67b33, 0x80916302, 0xa7f44683, 0x8f5d58db, 0xa8387d5a,
    0x613368b9, 0x46564d38, 0x6eff5360, 0x499a76e1, 0x3ead6ed0,
    0x19c84b51, 0x31615509, 0x16047088, 0xe2700e14, 0xc5152b95,
    0xedbc35cd, 0xcad9104c, 0xbdee087d, 0x9a8b2dfc, 0xb22233a4,
    0x95471625, 0x5c4c03c6, 0x7b292647, 0x5380381f, 0x74e51d9e,
    0x03d205af, 0x24b7202e, 0x0c1e3e76, 0x2b7b1bf7, 0xa5f1b395,
    0x82949614, 0xaa3d884c, 0x8d58adcd, 0xfa6fb5fc, 0xdd0a907d,
    0xf5a38e25, 0xd2c6aba4, 0x1bcdbe47, 0x3ca89bc6, 0x1401859e,
    0x3364a01f, 0x4453b82e, 0x63369daf, 0x4b9f83f7, 0x6cfaa676,
    0x988ed8ea, 0xbfebfd6b, 0x9742e333, 0xb027c6b2, 0xc710de83,
    0xe075fb02, 0xc8dce55a, 0xefb9c0db, 0x26b2d538, 0x01d7f0b9,
    0x297eeee1, 0x0e1bcb60, 0x792cd351, 0x5e49f6d0, 0x76e0e888,
    0x5185cd09}};

#endif

#endif

#endif

local const z_crc_t FAR x2n_table[] = {
    0x40000000, 0x20000000, 0x08000000, 0x00800000, 0x00008000,
    0xedb88320, 0xb1e6b092, 0xa06a2517, 0xed627dae, 0x88d14467,
    0xd7bbfe6a, 0xec447f11, 0x8e7ea170, 0x6427800e, 0x4d47bae0,
    0x09fe548f, 0x83852d0f, 0x30362f1a, 0x7b5a9cc3, 0x31fec169,
    0x9fec022a, 0x6c8dedc4, 0x15d6874d, 0x5fde7a4e, 0xbad90e37,
    0x2e4e5eef, 0x4eaba214, 0xa8a472c0, 0x429a969e, 0x148d302a,
    0xc40ba6d0, 0xc4e22c3c};

#endif

/* CRC polynomial. */
#define POLY 0xedb88320         /* p(x) reflected, with x^32 implied */

/*
  Return a(x) multiplied by b(x) modulo p(x), where p(x) is the CRC polynomial,
  reflected. For speed, this requires that a not be zero.
 */
local z_crc_t multmodp(z_crc_t a, z_crc_t b) {
    z_crc_t m, p;

    m = (z_crc_t)1 << 31;
    p = 0;
    for (;;) {
        if (a & m) {
            p ^= b;
            if ((a & (m - 1)) == 0)
                break;
        }
        m >>= 1;
        b = b & 1 ? (b >> 1) ^ POLY : b >> 1;
    }
    return p;
}

/*
  Return x^(n * 2^k) modulo p(x). Requires that x2n_table[] has been
  initialized.
 */
local z_crc_t x2nmodp(z_off64_t n, unsigned k) {
    z_crc_t p;

    p = (z_crc_t)1 << 31;           /* x^0 == 1 */
    while (n) {
        if (n & 1)
            p = multmodp(x2n_table[k & 31], p);
        n >>= 1;
        k++;
    }
    return p;
}

#ifdef DYNAMIC_CRC_TABLE
/* =========================================================================
 * Build the tables for byte-wise and braided CRC-32 calculations, and a table
 * of powers of x for combining CRC-32s.
 */
local z_crc_t FAR crc_table[256];
#ifdef W
   local z_word_t FAR crc_big_table[256];
   local z_crc_t FAR crc_braid_table[W][256];
   local z_word_t FAR crc_braid_big_table[W][256];
   local void braid(z_crc_t [][256], z_word_t [][256], int, int);
#endif
#ifdef MAKECRCH
   local void write_table(FILE *, const z_crc_t FAR *, int);
   local void write_table32hi(FILE *, const z_word_t FAR *, int);
   local void write_table64(FILE *, const z_word_t FAR *, int);
#endif /* MAKECRCH */

/*
  Define a once() function depending on the availability of atomics. If this is
  compiled with DYNAMIC_CRC_TABLE defined, and if CRCs will be computed in
  multiple threads, and if atomics are not available, then get_crc_table() must
  be called to initialize the tables and must return before any threads are
  allowed to compute or combine CRCs.
 */

/* Definition of once functionality. */
typedef struct once_s once_t;

/* Check for the availability of atomics. */
#if defined(__STDC__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_ATOMICS__)

#include <stdatomic.h>

/* Structure for once(), which must be initialized with ONCE_INIT. */
struct once_s {
    atomic_flag begun;
    atomic_int done;
};
#define ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  ONCE_INIT.
 */
local void once(once_t *state, void (*init)(void)) {
    if (!atomic_load(&state->done)) {
        if (atomic_flag_test_and_set(&state->begun))
            while (!atomic_load(&state->done))
                ;
        else {
            init();
            atomic_store(&state->done, 1);
        }
    }
}

#else   /* no atomics */

/* Structure for once(), which must be initialized with ONCE_INIT. */
struct once_s {
    volatile int begun;
    volatile int done;
};
#define ONCE_INIT {0, 0}

/* Test and set. Alas, not atomic, but tries to minimize the period of
   vulnerability. */
local int test_and_set(int volatile *flag) {
    int was;

    was = *flag;
    *flag = 1;
    return was;
}

/* Run the provided init() function once. This is not thread-safe. */
local void once(once_t *state, void (*init)(void)) {
    if (!state->done) {
        if (test_and_set(&state->begun))
            while (!state->done)
                ;
        else {
            init();
            state->done = 1;
        }
    }
}

#endif

/* State for once(). */
local once_t made = ONCE_INIT;

/*
  Generate tables for a byte-wise 32-bit CRC calculation on the polynomial:
  x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1.

  Polynomials over GF(2) are represented in binary, one bit per coefficient,
  with the lowest powers in the most significant bit. Then adding polynomials
  is just exclusive-or, and multiplying a polynomial by x is a right shift by
  one. If we call the above polynomial p, and represent a byte as the
  polynomial q, also with the lowest power in the most significant bit (so the
  byte 0xb1 is the polynomial x^7+x^3+x^2+1), then the CRC is (q*x^32) mod p,
  where a mod b means the remainder after dividing a by b.

  This calculation is done using the shift-register method of multiplying and
  taking the remainder. The register is initialized to zero, and for each
  incoming bit, x^32 is added mod p to the register if the bit is a one (where
  x^32 mod p is p+x^32 = x^26+...+1), and the register is multiplied mod p by x
  (which is shifting right by one and adding x^32 mod p if the bit shifted out
  is a one). We start with the highest power (least significant bit) of q and
  repeat for all eight bits of q.

  The table is simply the CRC of all possible eight bit values. This is all the
  information needed to generate CRCs on data a byte at a time for all
  combinations of CRC register values and incoming bytes.
 */

local void make_crc_table(void) {
    unsigned i, j, n;
    z_crc_t p;

    /* initialize the CRC of bytes tables */
    for (i = 0; i < 256; i++) {
        p = i;
        for (j = 0; j < 8; j++)
            p = p & 1 ? (p >> 1) ^ POLY : p >> 1;
        crc_table[i] = p;
#ifdef W
        crc_big_table[i] = byte_swap(p);
#endif
    }

    /* initialize the x^2^n mod p(x) table */
    p = (z_crc_t)1 << 30;         /* x^1 */
    x2n_table[0] = p;
    for (n = 1; n < 32; n++)
        x2n_table[n] = p = multmodp(p, p);

#ifdef W
    /* initialize the braiding tables -- needs x2n_table[] */
    braid(crc_braid_table, crc_braid_big_table, N, W);
#endif

#ifdef MAKECRCH
    {
        /*
          The crc32.h header file contains tables for both 32-bit and 64-bit
          z_word_t's, and so requires a 64-bit type be available. In that case,
          z_word_t must be defined to be 64-bits. This code then also generates
          and writes out the tables for the case that z_word_t is 32 bits.
         */
#if !defined(W) || W != 8
#  error Need a 64-bit integer type in order to generate crc32.h.
#endif
        FILE *out;
        int k, n;
        z_crc_t ltl[8][256];
        z_word_t big[8][256];

        out = fopen("crc32.h", "w");
        if (out == NULL) return;

        /* write out little-endian CRC table to crc32.h */
        fprintf(out,
            "/* crc32.h -- tables for rapid CRC calculation\n"
            " * Generated automatically by crc32.c\n */\n"
            "\n"
            "local const z_crc_t FAR crc_table[] = {\n"
            "    ");
        write_table(out, crc_table, 256);
        fprintf(out,
            "};\n");

        /* write out big-endian CRC table for 64-bit z_word_t to crc32.h */
        fprintf(out,
            "\n"
            "#ifdef W\n"
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table64(out, crc_big_table, 256);
        fprintf(out,
            "};\n");

        /* write out big-endian CRC table for 32-bit z_word_t to crc32.h */
        fprintf(out,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table32hi(out, crc_big_table, 256);
        fprintf(out,
            "};\n"
            "\n"
            "#endif\n");

        /* write out braid tables for each value of N */
        for (n = 1; n <= 6; n++) {
            fprintf(out,
            "\n"
            "#if N == %d\n", n);

            /* compute braid tables for this N and 64-bit word_t */
            braid(ltl, big, n, 8);

            /* write out braid tables for 64-bit z_word_t to crc32.h */
            fprintf(out,
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            for (k = 0; k < 8; k++) {
                fprintf(out, "   {");
                write_table(out, ltl[k], 256);
                fprintf(out, "}%s", k < 7 ? ",\n" : "");
            }
            fprintf(out,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            for (k = 0; k < 8; k++) {
                fprintf(out, "   {");
                write_table64(out, big[k], 256);
                fprintf(out, "}%s", k < 7 ? ",\n" : "");
            }
            fprintf(out,
            "};\n");

            /* compute braid tables for this N and 32-bit word_t */
            braid(ltl, big, n, 4);

            /* write out braid tables for 32-bit z_word_t to crc32.h */
            fprintf(out,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            for (k = 0; k < 4; k++) {
                fprintf(out, "   {");
                write_table(out, ltl[k], 256);
                fprintf(out, "}%s", k < 3 ? ",\n" : "");
            }
            fprintf(out,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            for (k = 0; k < 4; k++) {
                fprintf(out, "   {");
                write_table32hi(out, big[k], 256);
                fprintf(out, "}%s", k < 3 ? ",\n" : "");
            }
            fprintf(out,
            "};\n"
            "\n"
            "#endif\n"
            "\n"
            "#endif\n");
        }
        fprintf(out,
            "\n"
            "#endif\n");

        /* write out zeros operator table to crc32.h */
        fprintf(out,
            "\n"
            "local const z_crc_t FAR x2n_table[] = {\n"
            "    ");
        write_table(out, x2n_table, 32);
        fprintf(out,
            "};\n");
        fclose(out);
    }
#endif /* MAKECRCH */
}

#ifdef MAKECRCH

/*
   Write the 32-bit values in table[0..k-1] to out, five per line in
   hexadecimal separated by commas.
 */
local void write_table(FILE *out, const z_crc_t FAR *table, int k) {
    int n;

    for (n = 0; n < k; n++)
        fprintf(out, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (unsigned long)(table[n]),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
   Write the high 32-bits of each value in table[0..k-1] to out, five per line
   in hexadecimal separated by commas.
 */
local void write_table32hi(FILE *out, const z_word_t FAR *table, int k) {
    int n;

    for (n = 0; n < k; n++)
        fprintf(out, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (unsigned long)(table[n] >> 32),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
  Write the 64-bit values in table[0..k-1] to out, three per line in
  hexadecimal separated by commas. This assumes that if there is a 64-bit
  type, then there is also a long long integer type, and it is at least 64
  bits. If not, then the type cast and format string can be adjusted
  accordingly.
 */
local void write_table64(FILE *out, const z_word_t FAR *table, int k) {
    int n;

    for (n = 0; n < k; n++)
        fprintf(out, "%s0x%016llx%s", n == 0 || n % 3 ? "" : "    ",
                (unsigned long long)(table[n]),
                n == k - 1 ? "" : (n % 3 == 2 ? ",\n" : ", "));
}

/* Actually do the deed. */
int main(void) {
    make_crc_table();
    return 0;
}

#endif /* MAKECRCH */

#ifdef W
/*
  Generate the little and big-endian braid tables for the given n and z_word_t
  size w. Each array must have room for w blocks of 256 elements.
 */
local void braid(z_crc_t ltl[][256], z_word_t big[][256], int n, int w) {
    int k;
    z_crc_t i, p, q;
    for (k = 0; k < w; k++) {
        p = x2nmodp((n * w + 3 - k) << 3, 0);
        ltl[k][0] = 0;
        big[w - 1 - k][0] = 0;
        for (i = 1; i < 256; i++) {
            ltl[k][i] = q = multmodp(i << 24, p);
            big[w - 1 - k][i] = byte_swap(q);
        }
    }
}
#endif

#endif /* DYNAMIC_CRC_TABLE */

/* =========================================================================
 * This function can be used by asm versions of crc32(), and to force the
 * generation of the CRC tables in a threaded application.
 */
const z_crc_t FAR * ZEXPORT get_crc_table(void) {
#ifdef DYNAMIC_CRC_TABLE
    once(&made, make_crc_table);
#endif /* DYNAMIC_CRC_TABLE */
    return (const z_crc_t FAR *)crc_table;
}

/* =========================================================================
 * Use ARM machine instructions if available. This will compute the CRC about
 * ten times faster than the braided calculation. This code does not check for
 * the presence of the CRC instruction at run time. __ARM_FEATURE_CRC32 will
 * only be defined if the compilation specifies an ARM processor architecture
 * that has the instructions. For example, compiling with -march=armv8.1-a or
 * -march=armv8-a+crc, or -march=native if the compile machine has the crc32
 * instructions.
 */
#ifdef ARMCRC32

/*
   Constants empirically determined to maximize speed. These values are from
   measurements on a Cortex-A57. Your mileage may vary.
 */
#define Z_BATCH 3990                /* number of words in a batch */
#define Z_BATCH_ZEROS 0xa10d3d0c    /* computed from Z_BATCH = 3990 */
#define Z_BATCH_MIN 800             /* fewest words in a final batch */

unsigned long ZEXPORT crc32_z(unsigned long crc, const unsigned char FAR *buf,
                              z_size_t len) {
    z_crc_t val;
    z_word_t crc1, crc2;
    const z_word_t *word;
    z_word_t val0, val1, val2;
    z_size_t last, last2, i;
    z_size_t num;

    /* Return initial CRC, if requested. */
    if (buf == Z_NULL) return 0;

#ifdef DYNAMIC_CRC_TABLE
    once(&made, make_crc_table);
#endif /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    crc = (~crc) & 0xffffffff;

    /* Compute the CRC up to a word boundary. */
    while (len && ((z_size_t)buf & 7) != 0) {
        len--;
        val = *buf++;
        __asm__ volatile("crc32b %w0, %w0, %w1" : "+r"(crc) : "r"(val));
    }

    /* Prepare to compute the CRC on full 64-bit words word[0..num-1]. */
    word = (z_word_t const *)buf;
    num = len >> 3;
    len &= 7;

    /* Do three interleaved CRCs to realize the throughput of one crc32x
       instruction per cycle. Each CRC is calculated on Z_BATCH words. The
       three CRCs are combined into a single CRC after each set of batches. */
    while (num >= 3 * Z_BATCH) {
        crc1 = 0;
        crc2 = 0;
        for (i = 0; i < Z_BATCH; i++) {
            val0 = word[i];
            val1 = word[i + Z_BATCH];
            val2 = word[i + 2 * Z_BATCH];
            __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc) : "r"(val0));
            __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        word += 3 * Z_BATCH;
        num -= 3 * Z_BATCH;
        crc = multmodp(Z_BATCH_ZEROS, crc) ^ crc1;
        crc = multmodp(Z_BATCH_ZEROS, crc) ^ crc2;
    }

    /* Do one last smaller batch with the remaining words, if there are enough
       to pay for the combination of CRCs. */
    last = num / 3;
    if (last >= Z_BATCH_MIN) {
        last2 = last << 1;
        crc1 = 0;
        crc2 = 0;
        for (i = 0; i < last; i++) {
            val0 = word[i];
            val1 = word[i + last];
            val2 = word[i + last2];
            __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc) : "r"(val0));
            __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        word += 3 * last;
        num -= 3 * last;
        val = x2nmodp(last, 6);
        crc = multmodp(val, crc) ^ crc1;
        crc = multmodp(val, crc) ^ crc2;
    }

    /* Compute the CRC on any remaining words. */
    for (i = 0; i < num; i++) {
        val0 = word[i];
        __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc) : "r"(val0));
    }
    word += num;

    /* Complete the CRC on any remaining bytes. */
    buf = (const unsigned char FAR *)word;
    while (len) {
        len--;
        val = *buf++;
        __asm__ volatile("crc32b %w0, %w0, %w1" : "+r"(crc) : "r"(val));
    }

    /* Return the CRC, post-conditioned. */
    return crc ^ 0xffffffff;
}

#else

#ifdef W

/*
  Return the CRC of the W bytes in the word_t data, taking the
  least-significant byte of the word as the first byte of data, without any pre
  or post conditioning. This is used to combine the CRCs of each braid.
 */
local z_crc_t crc_word(z_word_t data) {
    int k;
    for (k = 0; k < W; k++)
        data = (data >> 8) ^ crc_table[data & 0xff];
    return (z_crc_t)data;
}

local z_word_t crc_word_big(z_word_t data) {
    int k;
    for (k = 0; k < W; k++)
        data = (data << 8) ^
            crc_big_table[(data >> ((W - 1) << 3)) & 0xff];
    return data;
}

#endif

/* ========================================================================= */
unsigned long ZEXPORT crc32_z(unsigned long crc, const unsigned char FAR *buf,
                              z_size_t len) {
    /* Return initial CRC, if requested. */
    if (buf == Z_NULL) return 0;

#ifdef DYNAMIC_CRC_TABLE
    once(&made, make_crc_table);
#endif /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    crc = (~crc) & 0xffffffff;

#ifdef W

    /* If provided enough bytes, do a braided CRC calculation. */
    if (len >= N * W + W - 1) {
        z_size_t blks;
        z_word_t const *words;
        unsigned endian;
        int k;

        /* Compute the CRC up to a z_word_t boundary. */
        while (len && ((z_size_t)buf & (W - 1)) != 0) {
            len--;
            crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        }

        /* Compute the CRC on as many N z_word_t blocks as are available. */
        blks = len / (N * W);
        len -= blks * N * W;
        words = (z_word_t const *)buf;

        /* Do endian check at execution time instead of compile time, since ARM
           processors can change the endianness at execution time. If the
           compiler knows what the endianness will be, it can optimize out the
           check and the unused branch. */
        endian = 1;
        if (*(unsigned char *)&endian) {
            /* Little endian. */

            z_crc_t crc0;
            z_word_t word0;
#if N > 1
            z_crc_t crc1;
            z_word_t word1;
#if N > 2
            z_crc_t crc2;
            z_word_t word2;
#if N > 3
            z_crc_t crc3;
            z_word_t word3;
#if N > 4
            z_crc_t crc4;
            z_word_t word4;
#if N > 5
            z_crc_t crc5;
            z_word_t word5;
#endif
#endif
#endif
#endif
#endif

            /* Initialize the CRC for each braid. */
            crc0 = crc;
#if N > 1
            crc1 = 0;
#if N > 2
            crc2 = 0;
#if N > 3
            crc3 = 0;
#if N > 4
            crc4 = 0;
#if N > 5
            crc5 = 0;
#endif
#endif
#endif
#endif
#endif

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            while (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ words[0];
#if N > 1
                word1 = crc1 ^ words[1];
#if N > 2
                word2 = crc2 ^ words[2];
#if N > 3
                word3 = crc3 ^ words[3];
#if N > 4
                word4 = crc4 ^ words[4];
#if N > 5
                word5 = crc5 ^ words[5];
#endif
#endif
#endif
#endif
#endif
                words += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = crc_braid_table[0][word0 & 0xff];
#if N > 1
                crc1 = crc_braid_table[0][word1 & 0xff];
#if N > 2
                crc2 = crc_braid_table[0][word2 & 0xff];
#if N > 3
                crc3 = crc_braid_table[0][word3 & 0xff];
#if N > 4
                crc4 = crc_braid_table[0][word4 & 0xff];
#if N > 5
                crc5 = crc_braid_table[0][word5 & 0xff];
#endif
#endif
#endif
#endif
#endif
                for (k = 1; k < W; k++) {
                    crc0 ^= crc_braid_table[k][(word0 >> (k << 3)) & 0xff];
#if N > 1
                    crc1 ^= crc_braid_table[k][(word1 >> (k << 3)) & 0xff];
#if N > 2
                    crc2 ^= crc_braid_table[k][(word2 >> (k << 3)) & 0xff];
#if N > 3
                    crc3 ^= crc_braid_table[k][(word3 >> (k << 3)) & 0xff];
#if N > 4
                    crc4 ^= crc_braid_table[k][(word4 >> (k << 3)) & 0xff];
#if N > 5
                    crc5 ^= crc_braid_table[k][(word5 >> (k << 3)) & 0xff];
#endif
#endif
#endif
#endif
#endif
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            crc = crc_word(crc0 ^ words[0]);
#if N > 1
            crc = crc_word(crc1 ^ words[1] ^ crc);
#if N > 2
            crc = crc_word(crc2 ^ words[2] ^ crc);
#if N > 3
            crc = crc_word(crc3 ^ words[3] ^ crc);
#if N > 4
            crc = crc_word(crc4 ^ words[4] ^ crc);
#if N > 5
            crc = crc_word(crc5 ^ words[5] ^ crc);
#endif
#endif
#endif
#endif
#endif
            words += N;
        }
        else {
            /* Big endian. */

            z_word_t crc0, word0, comb;
#if N > 1
            z_word_t crc1, word1;
#if N > 2
            z_word_t crc2, word2;
#if N > 3
            z_word_t crc3, word3;
#if N > 4
            z_word_t crc4, word4;
#if N > 5
            z_word_t crc5, word5;
#endif
#endif
#endif
#endif
#endif

            /* Initialize the CRC for each braid. */
            crc0 = byte_swap(crc);
#if N > 1
            crc1 = 0;
#if N > 2
            crc2 = 0;
#if N > 3
            crc3 = 0;
#if N > 4
            crc4 = 0;
#if N > 5
            crc5 = 0;
#endif
#endif
#endif
#endif
#endif

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            while (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ words[0];
#if N > 1
                word1 = crc1 ^ words[1];
#if N > 2
                word2 = crc2 ^ words[2];
#if N > 3
                word3 = crc3 ^ words[3];
#if N > 4
                word4 = crc4 ^ words[4];
#if N > 5
                word5 = crc5 ^ words[5];
#endif
#endif
#endif
#endif
#endif
                words += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = crc_braid_big_table[0][word0 & 0xff];
#if N > 1
                crc1 = crc_braid_big_table[0][word1 & 0xff];
#if N > 2
                crc2 = crc_braid_big_table[0][word2 & 0xff];
#if N > 3
                crc3 = crc_braid_big_table[0][word3 & 0xff];
#if N > 4
                crc4 = crc_braid_big_table[0][word4 & 0xff];
#if N > 5
                crc5 = crc_braid_big_table[0][word5 & 0xff];
#endif
#endif
#endif
#endif
#endif
                for (k = 1; k < W; k++) {
                    crc0 ^= crc_braid_big_table[k][(word0 >> (k << 3)) & 0xff];
#if N > 1
                    crc1 ^= crc_braid_big_table[k][(word1 >> (k << 3)) & 0xff];
#if N > 2
                    crc2 ^= crc_braid_big_table[k][(word2 >> (k << 3)) & 0xff];
#if N > 3
                    crc3 ^= crc_braid_big_table[k][(word3 >> (k << 3)) & 0xff];
#if N > 4
                    crc4 ^= crc_braid_big_table[k][(word4 >> (k << 3)) & 0xff];
#if N > 5
                    crc5 ^= crc_braid_big_table[k][(word5 >> (k << 3)) & 0xff];
#endif
#endif
#endif
#endif
#endif
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            comb = crc_word_big(crc0 ^ words[0]);
#if N > 1
            comb = crc_word_big(crc1 ^ words[1] ^ comb);
#if N > 2
            comb = crc_word_big(crc2 ^ words[2] ^ comb);
#if N > 3
            comb = crc_word_big(crc3 ^ words[3] ^ comb);
#if N > 4
            comb = crc_word_big(crc4 ^ words[4] ^ comb);
#if N > 5
            comb = crc_word_big(crc5 ^ words[5] ^ comb);
#endif
#endif
#endif
#endif
#endif
            words += N;
            crc = byte_swap(comb);
        }

        /*
          Update the pointer to the remaining bytes to process.
         */
        buf = (unsigned char const *)words;
    }

#endif /* W */

    /* Complete the computation of the CRC on any remaining bytes. */
    while (len >= 8) {
        len -= 8;
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
    }
    while (len) {
        len--;
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
    }

    /* Return the CRC, post-conditioned. */
    return crc ^ 0xffffffff;
}

#endif

/* ========================================================================= */
unsigned long ZEXPORT crc32(unsigned long crc, const unsigned char FAR *buf,
                            uInt len) {
    return crc32_z(crc, buf, len);
}

/* ========================================================================= */
uLong ZEXPORT crc32_combine64(uLong crc1, uLong crc2, z_off64_t len2) {
#ifdef DYNAMIC_CRC_TABLE
    once(&made, make_crc_table);
#endif /* DYNAMIC_CRC_TABLE */
    return multmodp(x2nmodp(len2, 3), crc1) ^ (crc2 & 0xffffffff);
}

/* ========================================================================= */
uLong ZEXPORT crc32_combine(uLong crc1, uLong crc2, z_off_t len2) {
    return crc32_combine64(crc1, crc2, (z_off64_t)len2);
}

/* ========================================================================= */
uLong ZEXPORT crc32_combine_gen64(z_off64_t len2) {
#ifdef DYNAMIC_CRC_TABLE
    once(&made, make_crc_table);
#endif /* DYNAMIC_CRC_TABLE */
    return x2nmodp(len2, 3);
}

/* ========================================================================= */
uLong ZEXPORT crc32_combine_gen(z_off_t len2) {
    return crc32_combine_gen64((z_off64_t)len2);
}

/* ========================================================================= */
uLong ZEXPORT crc32_combine_op(uLong crc1, uLong crc2, uLong op) {
    return multmodp(op, crc1) ^ (crc2 & 0xffffffff);
}
/* adler32.c -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-2011, 2016 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

//REBOL: #include "zutil.h"

#define BASE 65521U     /* largest prime smaller than 65536 */
#define NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

/* use NO_DIVIDE if your processor does not do division in hardware --
   try it both ways to see which is faster */
#ifdef NO_DIVIDE
/* note that this assumes BASE is 65521, where 65536 % 65521 == 15
   (thank you to John Reiser for pointing this out) */
#  define CHOP(a) \
    do { \
        unsigned long tmp = a >> 16; \
        a &= 0xffffUL; \
        a += (tmp << 4) - tmp; \
    } while (0)
#  define MOD28(a) \
    do { \
        CHOP(a); \
        if (a >= BASE) a -= BASE; \
    } while (0)
#  define MOD(a) \
    do { \
        CHOP(a); \
        MOD28(a); \
    } while (0)
#  define MOD63(a) \
    do { /* this assumes a is not negative */ \
        z_off64_t tmp = a >> 32; \
        a &= 0xffffffffL; \
        a += (tmp << 8) - (tmp << 5) + tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        if (a >= BASE) a -= BASE; \
    } while (0)
#else
#  define MOD(a) a %= BASE
#  define MOD28(a) a %= BASE
#  define MOD63(a) a %= BASE
#endif

/* ========================================================================= */
uLong ZEXPORT adler32_z(uLong adler, const Bytef *buf, z_size_t len) {
    unsigned long sum2;
    unsigned n;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    /* in case user likes doing a byte at a time, keep it fast */
    if (len == 1) {
        adler += buf[0];
        if (adler >= BASE)
            adler -= BASE;
        sum2 += adler;
        if (sum2 >= BASE)
            sum2 -= BASE;
        return adler | (sum2 << 16);
    }

    /* initial Adler-32 value (deferred check for len == 1 speed) */
    if (buf == Z_NULL)
        return 1L;

    /* in case short lengths are provided, keep it somewhat fast */
    if (len < 16) {
        while (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        if (adler >= BASE)
            adler -= BASE;
        MOD28(sum2);            /* only added so many BASE's */
        return adler | (sum2 << 16);
    }

    /* do length NMAX blocks -- requires just one modulo operation */
    while (len >= NMAX) {
        len -= NMAX;
        n = NMAX / 16;          /* NMAX is divisible by 16 */
        do {
            DO16(buf);          /* 16 sums unrolled */
            buf += 16;
        } while (--n);
        MOD(adler);
        MOD(sum2);
    }

    /* do remaining bytes (less than NMAX, still just one modulo) */
    if (len) {                  /* avoid modulos if none remaining */
        while (len >= 16) {
            len -= 16;
            DO16(buf);
            buf += 16;
        }
        while (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        MOD(adler);
        MOD(sum2);
    }

    /* return recombined sums */
    return adler | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32(uLong adler, const Bytef *buf, uInt len) {
    return adler32_z(adler, buf, len);
}

/* ========================================================================= */
local uLong adler32_combine_(uLong adler1, uLong adler2, z_off64_t len2) {
    unsigned long sum1;
    unsigned long sum2;
    unsigned rem;

    /* for negative len, return invalid adler32 as a clue for debugging */
    if (len2 < 0)
        return 0xffffffffUL;

    /* the derivation of this formula is left as an exercise for the reader */
    MOD63(len2);                /* assumes len2 >= 0 */
    rem = (unsigned)len2;
    sum1 = adler1 & 0xffff;
    sum2 = rem * sum1;
    MOD(sum2);
    sum1 += (adler2 & 0xffff) + BASE - 1;
    sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + BASE - rem;
    if (sum1 >= BASE) sum1 -= BASE;
    if (sum1 >= BASE) sum1 -= BASE;
    if (sum2 >= ((unsigned long)BASE << 1)) sum2 -= ((unsigned long)BASE << 1);
    if (sum2 >= BASE) sum2 -= BASE;
    return sum1 | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32_combine(uLong adler1, uLong adler2, z_off_t len2) {
    return adler32_combine_(adler1, adler2, len2);
}

uLong ZEXPORT adler32_combine64(uLong adler1, uLong adler2, z_off64_t len2) {
    return adler32_combine_(adler1, adler2, len2);
}
/* deflate.c -- compress data using the deflation algorithm
 * Copyright (C) 1995-2023 Jean-loup Gailly and Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/*
 *  ALGORITHM
 *
 *      The "deflation" process depends on being able to identify portions
 *      of the input text which are identical to earlier input (within a
 *      sliding window trailing behind the input currently being processed).
 *
 *      The most straightforward technique turns out to be the fastest for
 *      most input files: try all possible matches and select the longest.
 *      The key feature of this algorithm is that insertions into the string
 *      dictionary are very simple and thus fast, and deletions are avoided
 *      completely. Insertions are performed at each input character, whereas
 *      string matches are performed only when the previous match ends. So it
 *      is preferable to spend more time in matches to allow very fast string
 *      insertions and avoid deletions. The matching algorithm for small
 *      strings is inspired from that of Rabin & Karp. A brute force approach
 *      is used to find longer strings when a small match has been found.
 *      A similar algorithm is used in comic (by Jan-Mark Wams) and freeze
 *      (by Leonid Broukhis).
 *         A previous version of this file used a more sophisticated algorithm
 *      (by Fiala and Greene) which is guaranteed to run in linear amortized
 *      time, but has a larger average cost, uses more memory and is patented.
 *      However the F&G algorithm may be faster for some highly redundant
 *      files if the parameter max_chain_length (described below) is too large.
 *
 *  ACKNOWLEDGEMENTS
 *
 *      The idea of lazy evaluation of matches is due to Jan-Mark Wams, and
 *      I found it in 'freeze' written by Leonid Broukhis.
 *      Thanks to many people for bug reports and testing.
 *
 *  REFERENCES
 *
 *      Deutsch, L.P.,"DEFLATE Compressed Data Format Specification".
 *      Available in http://tools.ietf.org/html/rfc1951
 *
 *      A description of the Rabin and Karp algorithm is given in the book
 *         "Algorithms" by R. Sedgewick, Addison-Wesley, p252.
 *
 *      Fiala,E.R., and Greene,D.H.
 *         Data Compression with Finite Windows, Comm.ACM, 32,4 (1989) 490-595
 *
 */

/* @(#) $Id$ */

//REBOL: #include "deflate.h"

const char deflate_copyright[] =
   " deflate 1.3.0.1 Copyright 1995-2023 Jean-loup Gailly and Mark Adler ";
/*
  If you use the zlib library in a product, an acknowledgment is welcome
  in the documentation of your product. If for some reason you cannot
  include such an acknowledgment, I would appreciate that you keep this
  copyright string in the executable of your product.
 */

typedef enum {
    need_more,      /* block not completed, need more input or more output */
    block_done,     /* block flush performed */
    finish_started, /* finish started, need only more output at next deflate */
    finish_done     /* finish done, accept no more input or output */
} block_state;

typedef block_state (*compress_func)(deflate_state *s, int flush);
/* Compression function. Returns the block state after the call. */

local block_state deflate_stored(deflate_state *s, int flush);
local block_state deflate_fast(deflate_state *s, int flush);
#ifndef FASTEST
local block_state deflate_slow(deflate_state *s, int flush);
#endif
local block_state deflate_rle(deflate_state *s, int flush);
local block_state deflate_huff(deflate_state *s, int flush);

/* ===========================================================================
 * Local data
 */

#define NIL 0
/* Tail of hash chains */

#ifndef TOO_FAR
#  define TOO_FAR 4096
#endif
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */
typedef struct config_s {
   ush good_length; /* reduce lazy search above this match length */
   ush max_lazy;    /* do not perform lazy search above this match length */
   ush nice_length; /* quit search above this match length */
   ush max_chain;
   compress_func func;
} config;

#ifdef FASTEST
local const config configuration_table[2] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0, deflate_stored},  /* store only */
/* 1 */ {4,    4,  8,    4, deflate_fast}}; /* max speed, no lazy matches */
#else
local const config configuration_table[10] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0, deflate_stored},  /* store only */
/* 1 */ {4,    4,  8,    4, deflate_fast}, /* max speed, no lazy matches */
/* 2 */ {4,    5, 16,    8, deflate_fast},
/* 3 */ {4,    6, 32,   32, deflate_fast},

/* 4 */ {4,    4, 16,   16, deflate_slow},  /* lazy matches */
/* 5 */ {8,   16, 32,   32, deflate_slow},
/* 6 */ {8,   16, 128, 128, deflate_slow},
/* 7 */ {8,   32, 128, 256, deflate_slow},
/* 8 */ {32, 128, 258, 1024, deflate_slow},
/* 9 */ {32, 258, 258, 4096, deflate_slow}}; /* max compression */
#endif

/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
 * For deflate_fast() (levels <= 3) good is ignored and lazy has a different
 * meaning.
 */

/* rank Z_BLOCK between Z_NO_FLUSH and Z_PARTIAL_FLUSH */
#define RANK(f) (((f) * 2) - ((f) > 4 ? 9 : 0))

/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to UPDATE_HASH are made with consecutive input
 *    characters, so that a running hash key can be computed from the previous
 *    key instead of complete recalculation each time.
 */
#define UPDATE_HASH(s,h,c) (h = (((h) << s->hash_shift) ^ (c)) & s->hash_mask)


/* ===========================================================================
 * Insert string str in the dictionary and set match_head to the previous head
 * of the hash chain (the most recent string with same hash key). Return
 * the previous length of the hash chain.
 * If this file is compiled with -DFASTEST, the compression level is forced
 * to 1, and no hash chains are maintained.
 * IN  assertion: all calls to INSERT_STRING are made with consecutive input
 *    characters and the first MIN_MATCH bytes of str are valid (except for
 *    the last MIN_MATCH-1 bytes of the input file).
 */
#ifdef FASTEST
#define INSERT_STRING(s, str, match_head) \
   (UPDATE_HASH(s, s->ins_h, s->window[(str) + (MIN_MATCH-1)]), \
    match_head = s->head[s->ins_h], \
    s->head[s->ins_h] = (Pos)(str))
#else
#define INSERT_STRING(s, str, match_head) \
   (UPDATE_HASH(s, s->ins_h, s->window[(str) + (MIN_MATCH-1)]), \
    match_head = s->prev[(str) & s->w_mask] = s->head[s->ins_h], \
    s->head[s->ins_h] = (Pos)(str))
#endif

/* ===========================================================================
 * Initialize the hash table (avoiding 64K overflow for 16 bit systems).
 * prev[] will be initialized on the fly.
 */
#define CLEAR_HASH(s) \
    do { \
        s->head[s->hash_size - 1] = NIL; \
        zmemzero((Bytef *)s->head, \
                 (unsigned)(s->hash_size - 1)*sizeof(*s->head)); \
    } while (0)

/* ===========================================================================
 * Slide the hash table when sliding the window down (could be avoided with 32
 * bit values at the expense of memory usage). We slide even when level == 0 to
 * keep the hash table consistent if we switch back to level > 0 later.
 */
#if defined(__has_feature)
#  if __has_feature(memory_sanitizer)
     __attribute__((no_sanitize("memory")))
#  endif
#endif
local void slide_hash(deflate_state *s) {
    unsigned n, m;
    Posf *p;
    uInt wsize = s->w_size;

    n = s->hash_size;
    p = &s->head[n];
    do {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
    } while (--n);
    n = wsize;
#ifndef FASTEST
    p = &s->prev[n];
    do {
        m = *--p;
        *p = (Pos)(m >= wsize ? m - wsize : NIL);
        /* If n is not on any hash chain, prev[n] is garbage but
         * its value will never be used.
         */
    } while (--n);
#endif
}

/* ===========================================================================
 * Read a new buffer from the current input stream, update the adler32
 * and total number of bytes read.  All deflate() input goes through
 * this function so some applications may wish to modify it to avoid
 * allocating a large strm->next_in buffer and copying from it.
 * (See also flush_pending()).
 */
local unsigned read_buf(z_streamp strm, Bytef *buf, unsigned size) {
    unsigned len = strm->avail_in;

    if (len > size) len = size;
    if (len == 0) return 0;

    strm->avail_in  -= len;

    zmemcpy(buf, strm->next_in, len);
    if (strm->state->wrap == 1) {
        strm->adler = adler32(strm->adler, buf, len);
    }
#ifdef GZIP
    else if (strm->state->wrap == 2) {
        strm->adler = crc32(strm->adler, buf, len);
    }
#endif
    strm->next_in  += len;
    strm->total_in += len;

    return len;
}

/* ===========================================================================
 * Fill the window when the lookahead becomes insufficient.
 * Updates strstart and lookahead.
 *
 * IN assertion: lookahead < MIN_LOOKAHEAD
 * OUT assertions: strstart <= window_size-MIN_LOOKAHEAD
 *    At least one byte has been read, or avail_in == 0; reads are
 *    performed for at least two bytes (required for the zip translate_eol
 *    option -- not supported here).
 */
local void fill_window(deflate_state *s) {
    unsigned n;
    unsigned more;    /* Amount of free space at the end of the window. */
    uInt wsize = s->w_size;

    Assert(s->lookahead < MIN_LOOKAHEAD, "already enough lookahead");

    do {
        more = (unsigned)(s->window_size -(ulg)s->lookahead -(ulg)s->strstart);

        /* Deal with !@#$% 64K limit: */
        if (sizeof(int) <= 2) {
            if (more == 0 && s->strstart == 0 && s->lookahead == 0) {
                more = wsize;

            } else if (more == (unsigned)(-1)) {
                /* Very unlikely, but possible on 16 bit machine if
                 * strstart == 0 && lookahead == 1 (input done a byte at time)
                 */
                more--;
            }
        }

        /* If the window is almost full and there is insufficient lookahead,
         * move the upper half to the lower one to make room in the upper half.
         */
        if (s->strstart >= wsize + MAX_DIST(s)) {

            zmemcpy(s->window, s->window + wsize, (unsigned)wsize - more);
            s->match_start -= wsize;
            s->strstart    -= wsize; /* we now have strstart >= MAX_DIST */
            s->block_start -= (long) wsize;
            if (s->insert > s->strstart)
                s->insert = s->strstart;
            slide_hash(s);
            more += wsize;
        }
        if (s->strm->avail_in == 0) break;

        /* If there was no sliding:
         *    strstart <= WSIZE+MAX_DIST-1 && lookahead <= MIN_LOOKAHEAD - 1 &&
         *    more == window_size - lookahead - strstart
         * => more >= window_size - (MIN_LOOKAHEAD-1 + WSIZE + MAX_DIST-1)
         * => more >= window_size - 2*WSIZE + 2
         * In the BIG_MEM or MMAP case (not yet supported),
         *   window_size == input_size + MIN_LOOKAHEAD  &&
         *   strstart + s->lookahead <= input_size => more >= MIN_LOOKAHEAD.
         * Otherwise, window_size == 2*WSIZE so more >= 2.
         * If there was sliding, more >= WSIZE. So in all cases, more >= 2.
         */
        Assert(more >= 2, "more < 2");

        n = read_buf(s->strm, s->window + s->strstart + s->lookahead, more);
        s->lookahead += n;

        /* Initialize the hash value now that we have some input: */
        if (s->lookahead + s->insert >= MIN_MATCH) {
            uInt str = s->strstart - s->insert;
            s->ins_h = s->window[str];
            UPDATE_HASH(s, s->ins_h, s->window[str + 1]);
#if MIN_MATCH != 3
            Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
            while (s->insert) {
                UPDATE_HASH(s, s->ins_h, s->window[str + MIN_MATCH-1]);
#ifndef FASTEST
                s->prev[str & s->w_mask] = s->head[s->ins_h];
#endif
                s->head[s->ins_h] = (Pos)str;
                str++;
                s->insert--;
                if (s->lookahead + s->insert < MIN_MATCH)
                    break;
            }
        }
        /* If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
         * but this is not important since only literal bytes will be emitted.
         */

    } while (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_in != 0);

    /* If the WIN_INIT bytes after the end of the current data have never been
     * written, then zero those bytes in order to avoid memory check reports of
     * the use of uninitialized (or uninitialised as Julian writes) bytes by
     * the longest match routines.  Update the high water mark for the next
     * time through here.  WIN_INIT is set to MAX_MATCH since the longest match
     * routines allow scanning to strstart + MAX_MATCH, ignoring lookahead.
     */
    if (s->high_water < s->window_size) {
        ulg curr = s->strstart + (ulg)(s->lookahead);
        ulg init;

        if (s->high_water < curr) {
            /* Previous high water mark below current data -- zero WIN_INIT
             * bytes or up to end of window, whichever is less.
             */
            init = s->window_size - curr;
            if (init > WIN_INIT)
                init = WIN_INIT;
            zmemzero(s->window + curr, (unsigned)init);
            s->high_water = curr + init;
        }
        else if (s->high_water < (ulg)curr + WIN_INIT) {
            /* High water mark at or above current data, but below current data
             * plus WIN_INIT -- zero out to current data plus WIN_INIT, or up
             * to end of window, whichever is less.
             */
            init = (ulg)curr + WIN_INIT - s->high_water;
            if (init > s->window_size - s->high_water)
                init = s->window_size - s->high_water;
            zmemzero(s->window + s->high_water, (unsigned)init);
            s->high_water += init;
        }
    }

    Assert((ulg)s->strstart <= s->window_size - MIN_LOOKAHEAD,
           "not enough room for search");
}

/* ========================================================================= */
int ZEXPORT deflateInit_(z_streamp strm, int level, const char *version,
                         int stream_size) {
    return deflateInit2_(strm, level, Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL,
                         Z_DEFAULT_STRATEGY, version, stream_size);
    /* To do: ignore strm->next_in if we use it as window */
}

/* ========================================================================= */
int ZEXPORT deflateInit2_(z_streamp strm, int level, int method,
                          int windowBits, int memLevel, int strategy,
                          const char *version, int stream_size) {
    deflate_state *s;
    int wrap = 1;
    static const char my_version[] = ZLIB_VERSION;

    if (version == Z_NULL || version[0] != my_version[0] ||
        stream_size != sizeof(z_stream)) {
        return Z_VERSION_ERROR;
    }
    if (strm == Z_NULL) return Z_STREAM_ERROR;

    strm->msg = Z_NULL;
    if (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        return Z_STREAM_ERROR;
#else
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    if (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        return Z_STREAM_ERROR;
#else
        strm->zfree = zcfree;
#endif

#ifdef FASTEST
    if (level != 0) level = 1;
#else
    if (level == Z_DEFAULT_COMPRESSION) level = 6;
#endif

    if (windowBits < 0) { /* suppress zlib wrapper */
        wrap = 0;
        if (windowBits < -15)
            return Z_STREAM_ERROR;
        windowBits = -windowBits;
    }
#ifdef GZIP
    else if (windowBits > 15) {
        wrap = 2;       /* write gzip wrapper instead */
        windowBits -= 16;
    }
#endif
    if (memLevel < 1 || memLevel > MAX_MEM_LEVEL || method != Z_DEFLATED ||
        windowBits < 8 || windowBits > 15 || level < 0 || level > 9 ||
        strategy < 0 || strategy > Z_FIXED || (windowBits == 8 && wrap != 1)) {
        return Z_STREAM_ERROR;
    }
    if (windowBits == 8) windowBits = 9;  /* until 256-byte window bug fixed */
    s = (deflate_state *) ZALLOC(strm, 1, sizeof(deflate_state));
    if (s == Z_NULL) return Z_MEM_ERROR;
    strm->state = (struct internal_state FAR *)s;
    s->strm = strm;
    s->status = INIT_STATE;     /* to pass state test in deflateReset() */

    s->wrap = wrap;
    s->gzhead = Z_NULL;
    s->w_bits = (uInt)windowBits;
    s->w_size = 1 << s->w_bits;
    s->w_mask = s->w_size - 1;

    s->hash_bits = (uInt)memLevel + 7;
    s->hash_size = 1 << s->hash_bits;
    s->hash_mask = s->hash_size - 1;
    s->hash_shift =  ((s->hash_bits + MIN_MATCH-1) / MIN_MATCH);

    s->window = (Bytef *) ZALLOC(strm, s->w_size, 2*sizeof(Byte));
    s->prev   = (Posf *)  ZALLOC(strm, s->w_size, sizeof(Pos));
    s->head   = (Posf *)  ZALLOC(strm, s->hash_size, sizeof(Pos));

    s->high_water = 0;      /* nothing written to s->window yet */

    s->lit_bufsize = 1 << (memLevel + 6); /* 16K elements by default */

    /* We overlay pending_buf and sym_buf. This works since the average size
     * for length/distance pairs over any compressed block is assured to be 31
     * bits or less.
     *
     * Analysis: The longest fixed codes are a length code of 8 bits plus 5
     * extra bits, for lengths 131 to 257. The longest fixed distance codes are
     * 5 bits plus 13 extra bits, for distances 16385 to 32768. The longest
     * possible fixed-codes length/distance pair is then 31 bits total.
     *
     * sym_buf starts one-fourth of the way into pending_buf. So there are
     * three bytes in sym_buf for every four bytes in pending_buf. Each symbol
     * in sym_buf is three bytes -- two for the distance and one for the
     * literal/length. As each symbol is consumed, the pointer to the next
     * sym_buf value to read moves forward three bytes. From that symbol, up to
     * 31 bits are written to pending_buf. The closest the written pending_buf
     * bits gets to the next sym_buf symbol to read is just before the last
     * code is written. At that time, 31*(n - 2) bits have been written, just
     * after 24*(n - 2) bits have been consumed from sym_buf. sym_buf starts at
     * 8*n bits into pending_buf. (Note that the symbol buffer fills when n - 1
     * symbols are written.) The closest the writing gets to what is unread is
     * then n + 14 bits. Here n is lit_bufsize, which is 16384 by default, and
     * can range from 128 to 32768.
     *
     * Therefore, at a minimum, there are 142 bits of space between what is
     * written and what is read in the overlain buffers, so the symbols cannot
     * be overwritten by the compressed data. That space is actually 139 bits,
     * due to the three-bit fixed-code block header.
     *
     * That covers the case where either Z_FIXED is specified, forcing fixed
     * codes, or when the use of fixed codes is chosen, because that choice
     * results in a smaller compressed block than dynamic codes. That latter
     * condition then assures that the above analysis also covers all dynamic
     * blocks. A dynamic-code block will only be chosen to be emitted if it has
     * fewer bits than a fixed-code block would for the same set of symbols.
     * Therefore its average symbol length is assured to be less than 31. So
     * the compressed data for a dynamic block also cannot overwrite the
     * symbols from which it is being constructed.
     */

#ifdef LIT_MEM
    s->pending_buf = (uchf *) ZALLOC(strm, s->lit_bufsize, 5);
#else
    s->pending_buf = (uchf *) ZALLOC(strm, s->lit_bufsize, 4);
#endif
    s->pending_buf_size = (ulg)s->lit_bufsize * 4;

    if (s->window == Z_NULL || s->prev == Z_NULL || s->head == Z_NULL ||
        s->pending_buf == Z_NULL) {
        s->status = FINISH_STATE;
        strm->msg = ERR_MSG(Z_MEM_ERROR);
        deflateEnd (strm);
        return Z_MEM_ERROR;
    }
#ifdef LIT_MEM
    s->d_buf = (ushf *)(s->pending_buf + (s->lit_bufsize << 1));
    s->l_buf = s->pending_buf + (s->lit_bufsize << 2);
    s->sym_end = s->lit_bufsize - 1;
#else
    s->sym_buf = s->pending_buf + s->lit_bufsize;
    s->sym_end = (s->lit_bufsize - 1) * 3;
#endif
    /* We avoid equality with lit_bufsize*3 because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K-1 bytes.
     */

    s->level = level;
    s->strategy = strategy;
    s->method = (Byte)method;

    return deflateReset(strm);
}

/* =========================================================================
 * Check for a valid deflate stream state. Return 0 if ok, 1 if not.
 */
local int deflateStateCheck(z_streamp strm) {
    deflate_state *s;
    if (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        return 1;
    s = strm->state;
    if (s == Z_NULL || s->strm != strm || (s->status != INIT_STATE &&
#ifdef GZIP
                                           s->status != GZIP_STATE &&
#endif
                                           s->status != EXTRA_STATE &&
                                           s->status != NAME_STATE &&
                                           s->status != COMMENT_STATE &&
                                           s->status != HCRC_STATE &&
                                           s->status != BUSY_STATE &&
                                           s->status != FINISH_STATE))
        return 1;
    return 0;
}

/* ========================================================================= */
int ZEXPORT deflateSetDictionary(z_streamp strm, const Bytef *dictionary,
                                 uInt  dictLength) {
    deflate_state *s;
    uInt str, n;
    int wrap;
    unsigned avail;
    z_const unsigned char *next;

    if (deflateStateCheck(strm) || dictionary == Z_NULL)
        return Z_STREAM_ERROR;
    s = strm->state;
    wrap = s->wrap;
    if (wrap == 2 || (wrap == 1 && s->status != INIT_STATE) || s->lookahead)
        return Z_STREAM_ERROR;

    /* when using zlib wrappers, compute Adler-32 for provided dictionary */
    if (wrap == 1)
        strm->adler = adler32(strm->adler, dictionary, dictLength);
    s->wrap = 0;                    /* avoid computing Adler-32 in read_buf */

    /* if dictionary would fill window, just replace the history */
    if (dictLength >= s->w_size) {
        if (wrap == 0) {            /* already empty otherwise */
            CLEAR_HASH(s);
            s->strstart = 0;
            s->block_start = 0L;
            s->insert = 0;
        }
        dictionary += dictLength - s->w_size;  /* use the tail */
        dictLength = s->w_size;
    }

    /* insert dictionary into window and hash */
    avail = strm->avail_in;
    next = strm->next_in;
    strm->avail_in = dictLength;
    strm->next_in = (z_const Bytef *)dictionary;
    fill_window(s);
    while (s->lookahead >= MIN_MATCH) {
        str = s->strstart;
        n = s->lookahead - (MIN_MATCH-1);
        do {
            UPDATE_HASH(s, s->ins_h, s->window[str + MIN_MATCH-1]);
#ifndef FASTEST
            s->prev[str & s->w_mask] = s->head[s->ins_h];
#endif
            s->head[s->ins_h] = (Pos)str;
            str++;
        } while (--n);
        s->strstart = str;
        s->lookahead = MIN_MATCH-1;
        fill_window(s);
    }
    s->strstart += s->lookahead;
    s->block_start = (long)s->strstart;
    s->insert = s->lookahead;
    s->lookahead = 0;
    s->match_length = s->prev_length = MIN_MATCH-1;
    s->match_available = 0;
    strm->next_in = next;
    strm->avail_in = avail;
    s->wrap = wrap;
    return Z_OK;
}

/* ========================================================================= */
int ZEXPORT deflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dictLength) {
    deflate_state *s;
    uInt len;

    if (deflateStateCheck(strm))
        return Z_STREAM_ERROR;
    s = strm->state;
    len = s->strstart + s->lookahead;
    if (len > s->w_size)
        len = s->w_size;
    if (dictionary != Z_NULL && len)
        zmemcpy(dictionary, s->window + s->strstart + s->lookahead - len, len);
    if (dictLength != Z_NULL)
        *dictLength = len;
    return Z_OK;
}

/* ========================================================================= */
int ZEXPORT deflateResetKeep(z_streamp strm) {
    deflate_state *s;

    if (deflateStateCheck(strm)) {
        return Z_STREAM_ERROR;
    }

    strm->total_in = strm->total_out = 0;
    strm->msg = Z_NULL; /* use zfree if we ever allocate msg dynamically */
    strm->data_type = Z_UNKNOWN;

    s = (deflate_state *)strm->state;
    s->pending = 0;
    s->pending_out = s->pending_buf;

    if (s->wrap < 0) {
        s->wrap = -s->wrap; /* was made negative by deflate(..., Z_FINISH); */
    }
    s->status =
#ifdef GZIP
        s->wrap == 2 ? GZIP_STATE :
#endif
        INIT_STATE;
    strm->adler =
#ifdef GZIP
        s->wrap == 2 ? crc32(0L, Z_NULL, 0) :
#endif
        adler32(0L, Z_NULL, 0);
    s->last_flush = -2;

    _tr_init(s);

    return Z_OK;
}

/* ===========================================================================
 * Initialize the "longest match" routines for a new zlib stream
 */
local void lm_init(deflate_state *s) {
    s->window_size = (ulg)2L*s->w_size;

    CLEAR_HASH(s);

    /* Set the default configuration parameters:
     */
    s->max_lazy_match   = configuration_table[s->level].max_lazy;
    s->good_match       = configuration_table[s->level].good_length;
    s->nice_match       = configuration_table[s->level].nice_length;
    s->max_chain_length = configuration_table[s->level].max_chain;

    s->strstart = 0;
    s->block_start = 0L;
    s->lookahead = 0;
    s->insert = 0;
    s->match_length = s->prev_length = MIN_MATCH-1;
    s->match_available = 0;
    s->ins_h = 0;
}

/* ========================================================================= */
int ZEXPORT deflateReset(z_streamp strm) {
    int ret;

    ret = deflateResetKeep(strm);
    if (ret == Z_OK)
        lm_init(strm->state);
    return ret;
}

/* ========================================================================= */
int ZEXPORT deflateSetHeader(z_streamp strm, gz_headerp head) {
    if (deflateStateCheck(strm) || strm->state->wrap != 2)
        return Z_STREAM_ERROR;
    strm->state->gzhead = head;
    return Z_OK;
}

/* ========================================================================= */
int ZEXPORT deflatePending(z_streamp strm, unsigned *pending, int *bits) {
    if (deflateStateCheck(strm)) return Z_STREAM_ERROR;
    if (pending != Z_NULL)
        *pending = strm->state->pending;
    if (bits != Z_NULL)
        *bits = strm->state->bi_valid;
    return Z_OK;
}

/* ========================================================================= */
int ZEXPORT deflatePrime(z_streamp strm, int bits, int value) {
    deflate_state *s;
    int put;

    if (deflateStateCheck(strm)) return Z_STREAM_ERROR;
    s = strm->state;
#ifdef LIT_MEM
    if (bits < 0 || bits > 16 ||
        (uchf *)s->d_buf < s->pending_out + ((Buf_size + 7) >> 3))
        return Z_BUF_ERROR;
#else
    if (bits < 0 || bits > 16 ||
        s->sym_buf < s->pending_out + ((Buf_size + 7) >> 3))
        return Z_BUF_ERROR;
#endif
    do {
        put = Buf_size - s->bi_valid;
        if (put > bits)
            put = bits;
        s->bi_buf |= (ush)((value & ((1 << put) - 1)) << s->bi_valid);
        s->bi_valid += put;
        _tr_flush_bits(s);
        value >>= put;
        bits -= put;
    } while (bits);
    return Z_OK;
}

/* ========================================================================= */
int ZEXPORT deflateParams(z_streamp strm, int level, int strategy) {
    deflate_state *s;
    compress_func func;

    if (deflateStateCheck(strm)) return Z_STREAM_ERROR;
    s = strm->state;

#ifdef FASTEST
    if (level != 0) level = 1;
#else
    if (level == Z_DEFAULT_COMPRESSION) level = 6;
#endif
    if (level < 0 || level > 9 || strategy < 0 || strategy > Z_FIXED) {
        return Z_STREAM_ERROR;
    }
    func = configuration_table[s->level].func;

    if ((strategy != s->strategy || func != configuration_table[level].func) &&
        s->last_flush != -2) {
        /* Flush the last buffer: */
        int err = deflate(strm, Z_BLOCK);
        if (err == Z_STREAM_ERROR)
            return err;
        if (strm->avail_in || (s->strstart - s->block_start) + s->lookahead)
            return Z_BUF_ERROR;
    }
    if (s->level != level) {
        if (s->level == 0 && s->matches != 0) {
            if (s->matches == 1)
                slide_hash(s);
            else
                CLEAR_HASH(s);
            s->matches = 0;
        }
        s->level = level;
        s->max_lazy_match   = configuration_table[level].max_lazy;
        s->good_match       = configuration_table[level].good_length;
        s->nice_match       = configuration_table[level].nice_length;
        s->max_chain_length = configuration_table[level].max_chain;
    }
    s->strategy = strategy;
    return Z_OK;
}

/* ========================================================================= */
int ZEXPORT deflateTune(z_streamp strm, int good_length, int max_lazy,
                        int nice_length, int max_chain) {
    deflate_state *s;

    if (deflateStateCheck(strm)) return Z_STREAM_ERROR;
    s = strm->state;
    s->good_match = (uInt)good_length;
    s->max_lazy_match = (uInt)max_lazy;
    s->nice_match = nice_length;
    s->max_chain_length = (uInt)max_chain;
    return Z_OK;
}

/* =========================================================================
 * For the default windowBits of 15 and memLevel of 8, this function returns a
 * close to exact, as well as small, upper bound on the compressed size. This
 * is an expansion of ~0.03%, plus a small constant.
 *
 * For any setting other than those defaults for windowBits and memLevel, one
 * of two worst case bounds is returned. This is at most an expansion of ~4% or
 * ~13%, plus a small constant.
 *
 * Both the 0.03% and 4% derive from the overhead of stored blocks. The first
 * one is for stored blocks of 16383 bytes (memLevel == 8), whereas the second
 * is for stored blocks of 127 bytes (the worst case memLevel == 1). The
 * expansion results from five bytes of header for each stored block.
 *
 * The larger expansion of 13% results from a window size less than or equal to
 * the symbols buffer size (windowBits <= memLevel + 7). In that case some of
 * the data being compressed may have slid out of the sliding window, impeding
 * a stored block from being emitted. Then the only choice is a fixed or
 * dynamic block, where a fixed block limits the maximum expansion to 9 bits
 * per 8-bit byte, plus 10 bits for every block. The smallest block size for
 * which this can occur is 255 (memLevel == 2).
 *
 * Shifts are used to approximate divisions, for speed.
 */
uLong ZEXPORT deflateBound(z_streamp strm, uLong sourceLen) {
    deflate_state *s;
    uLong fixedlen, storelen, wraplen;

    /* upper bound for fixed blocks with 9-bit literals and length 255
       (memLevel == 2, which is the lowest that may not use stored blocks) --
       ~13% overhead plus a small constant */
    fixedlen = sourceLen + (sourceLen >> 3) + (sourceLen >> 8) +
               (sourceLen >> 9) + 4;

    /* upper bound for stored blocks with length 127 (memLevel == 1) --
       ~4% overhead plus a small constant */
    storelen = sourceLen + (sourceLen >> 5) + (sourceLen >> 7) +
               (sourceLen >> 11) + 7;

    /* if can't get parameters, return larger bound plus a zlib wrapper */
    if (deflateStateCheck(strm))
        return (fixedlen > storelen ? fixedlen : storelen) + 6;

    /* compute wrapper length */
    s = strm->state;
    switch (s->wrap) {
    case 0:                                 /* raw deflate */
        wraplen = 0;
        break;
    case 1:                                 /* zlib wrapper */
        wraplen = 6 + (s->strstart ? 4 : 0);
        break;
#ifdef GZIP
    case 2:                                 /* gzip wrapper */
        wraplen = 18;
        if (s->gzhead != Z_NULL) {          /* user-supplied gzip header */
            Bytef *str;
            if (s->gzhead->extra != Z_NULL)
                wraplen += 2 + s->gzhead->extra_len;
            str = s->gzhead->name;
            if (str != Z_NULL)
                do {
                    wraplen++;
                } while (*str++);
            str = s->gzhead->comment;
            if (str != Z_NULL)
                do {
                    wraplen++;
                } while (*str++);
            if (s->gzhead->hcrc)
                wraplen += 2;
        }
        break;
#endif
    default:                                /* for compiler happiness */
        wraplen = 6;
    }

    /* if not default parameters, return one of the conservative bounds */
    if (s->w_bits != 15 || s->hash_bits != 8 + 7)
        return (s->w_bits <= s->hash_bits && s->level ? fixedlen : storelen) +
               wraplen;

    /* default settings: return tight bound for that case -- ~0.03% overhead
       plus a small constant */
    return sourceLen + (sourceLen >> 12) + (sourceLen >> 14) +
           (sourceLen >> 25) + 13 - 6 + wraplen;
}

/* =========================================================================
 * Put a short in the pending buffer. The 16-bit value is put in MSB order.
 * IN assertion: the stream state is correct and there is enough room in
 * pending_buf.
 */
local void putShortMSB(deflate_state *s, uInt b) {
    put_byte(s, (Byte)(b >> 8));
    put_byte(s, (Byte)(b & 0xff));
}

/* =========================================================================
 * Flush as much pending output as possible. All deflate() output, except for
 * some deflate_stored() output, goes through this function so some
 * applications may wish to modify it to avoid allocating a large
 * strm->next_out buffer and copying into it. (See also read_buf()).
 */
local void flush_pending(z_streamp strm) {
    unsigned len;
    deflate_state *s = strm->state;

    _tr_flush_bits(s);
    len = s->pending;
    if (len > strm->avail_out) len = strm->avail_out;
    if (len == 0) return;

    zmemcpy(strm->next_out, s->pending_out, len);
    strm->next_out  += len;
    s->pending_out  += len;
    strm->total_out += len;
    strm->avail_out -= len;
    s->pending      -= len;
    if (s->pending == 0) {
        s->pending_out = s->pending_buf;
    }
}

/* ===========================================================================
 * Update the header CRC with the bytes s->pending_buf[beg..s->pending - 1].
 */
#define HCRC_UPDATE(beg) \
    do { \
        if (s->gzhead->hcrc && s->pending > (beg)) \
            strm->adler = crc32(strm->adler, s->pending_buf + (beg), \
                                s->pending - (beg)); \
    } while (0)

/* ========================================================================= */
int ZEXPORT deflate(z_streamp strm, int flush) {
    int old_flush; /* value of flush param for previous deflate call */
    deflate_state *s;

    if (deflateStateCheck(strm) || flush > Z_BLOCK || flush < 0) {
        return Z_STREAM_ERROR;
    }
    s = strm->state;

    if (strm->next_out == Z_NULL ||
        (strm->avail_in != 0 && strm->next_in == Z_NULL) ||
        (s->status == FINISH_STATE && flush != Z_FINISH)) {
        ERR_RETURN(strm, Z_STREAM_ERROR);
    }
    if (strm->avail_out == 0) ERR_RETURN(strm, Z_BUF_ERROR);

    old_flush = s->last_flush;
    s->last_flush = flush;

    /* Flush as much pending output as possible */
    if (s->pending != 0) {
        flush_pending(strm);
        if (strm->avail_out == 0) {
            /* Since avail_out is 0, deflate will be called again with
             * more output space, but possibly with both pending and
             * avail_in equal to zero. There won't be anything to do,
             * but this is not an error situation so make sure we
             * return OK instead of BUF_ERROR at next call of deflate:
             */
            s->last_flush = -1;
            return Z_OK;
        }

    /* Make sure there is something to do and avoid duplicate consecutive
     * flushes. For repeated and useless calls with Z_FINISH, we keep
     * returning Z_STREAM_END instead of Z_BUF_ERROR.
     */
    } else if (strm->avail_in == 0 && RANK(flush) <= RANK(old_flush) &&
               flush != Z_FINISH) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* User must not provide more input after the first FINISH: */
    if (s->status == FINISH_STATE && strm->avail_in != 0) {
        ERR_RETURN(strm, Z_BUF_ERROR);
    }

    /* Write the header */
    if (s->status == INIT_STATE && s->wrap == 0)
        s->status = BUSY_STATE;
    if (s->status == INIT_STATE) {
        /* zlib header */
        uInt header = (Z_DEFLATED + ((s->w_bits - 8) << 4)) << 8;
        uInt level_flags;

        if (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2)
            level_flags = 0;
        else if (s->level < 6)
            level_flags = 1;
        else if (s->level == 6)
            level_flags = 2;
        else
            level_flags = 3;
        header |= (level_flags << 6);
        if (s->strstart != 0) header |= PRESET_DICT;
        header += 31 - (header % 31);

        putShortMSB(s, header);

        /* Save the adler32 of the preset dictionary: */
        if (s->strstart != 0) {
            putShortMSB(s, (uInt)(strm->adler >> 16));
            putShortMSB(s, (uInt)(strm->adler & 0xffff));
        }
        strm->adler = adler32(0L, Z_NULL, 0);
        s->status = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        if (s->pending != 0) {
            s->last_flush = -1;
            return Z_OK;
        }
    }
#ifdef GZIP
    if (s->status == GZIP_STATE) {
        /* gzip header */
        strm->adler = crc32(0L, Z_NULL, 0);
        put_byte(s, 31);
        put_byte(s, 139);
        put_byte(s, 8);
        if (s->gzhead == Z_NULL) {
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, 0);
            put_byte(s, s->level == 9 ? 2 :
                     (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2 ?
                      4 : 0));
            put_byte(s, OS_CODE);
            s->status = BUSY_STATE;

            /* Compression must start with an empty pending buffer */
            flush_pending(strm);
            if (s->pending != 0) {
                s->last_flush = -1;
                return Z_OK;
            }
        }
        else {
            put_byte(s, (s->gzhead->text ? 1 : 0) +
                     (s->gzhead->hcrc ? 2 : 0) +
                     (s->gzhead->extra == Z_NULL ? 0 : 4) +
                     (s->gzhead->name == Z_NULL ? 0 : 8) +
                     (s->gzhead->comment == Z_NULL ? 0 : 16)
                     );
            put_byte(s, (Byte)(s->gzhead->time & 0xff));
            put_byte(s, (Byte)((s->gzhead->time >> 8) & 0xff));
            put_byte(s, (Byte)((s->gzhead->time >> 16) & 0xff));
            put_byte(s, (Byte)((s->gzhead->time >> 24) & 0xff));
            put_byte(s, s->level == 9 ? 2 :
                     (s->strategy >= Z_HUFFMAN_ONLY || s->level < 2 ?
                      4 : 0));
            put_byte(s, s->gzhead->os & 0xff);
            if (s->gzhead->extra != Z_NULL) {
                put_byte(s, s->gzhead->extra_len & 0xff);
                put_byte(s, (s->gzhead->extra_len >> 8) & 0xff);
            }
            if (s->gzhead->hcrc)
                strm->adler = crc32(strm->adler, s->pending_buf,
                                    s->pending);
            s->gzindex = 0;
            s->status = EXTRA_STATE;
        }
    }
    if (s->status == EXTRA_STATE) {
        if (s->gzhead->extra != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            uInt left = (s->gzhead->extra_len & 0xffff) - s->gzindex;
            while (s->pending + left > s->pending_buf_size) {
                uInt copy = s->pending_buf_size - s->pending;
                zmemcpy(s->pending_buf + s->pending,
                        s->gzhead->extra + s->gzindex, copy);
                s->pending = s->pending_buf_size;
                HCRC_UPDATE(beg);
                s->gzindex += copy;
                flush_pending(strm);
                if (s->pending != 0) {
                    s->last_flush = -1;
                    return Z_OK;
                }
                beg = 0;
                left -= copy;
            }
            zmemcpy(s->pending_buf + s->pending,
                    s->gzhead->extra + s->gzindex, left);
            s->pending += left;
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->status = NAME_STATE;
    }
    if (s->status == NAME_STATE) {
        if (s->gzhead->name != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            int val;
            do {
                if (s->pending == s->pending_buf_size) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    if (s->pending != 0) {
                        s->last_flush = -1;
                        return Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->name[s->gzindex++];
                put_byte(s, val);
            } while (val != 0);
            HCRC_UPDATE(beg);
            s->gzindex = 0;
        }
        s->status = COMMENT_STATE;
    }
    if (s->status == COMMENT_STATE) {
        if (s->gzhead->comment != Z_NULL) {
            ulg beg = s->pending;   /* start of bytes to update crc */
            int val;
            do {
                if (s->pending == s->pending_buf_size) {
                    HCRC_UPDATE(beg);
                    flush_pending(strm);
                    if (s->pending != 0) {
                        s->last_flush = -1;
                        return Z_OK;
                    }
                    beg = 0;
                }
                val = s->gzhead->comment[s->gzindex++];
                put_byte(s, val);
            } while (val != 0);
            HCRC_UPDATE(beg);
        }
        s->status = HCRC_STATE;
    }
    if (s->status == HCRC_STATE) {
        if (s->gzhead->hcrc) {
            if (s->pending + 2 > s->pending_buf_size) {
                flush_pending(strm);
                if (s->pending != 0) {
                    s->last_flush = -1;
                    return Z_OK;
                }
            }
            put_byte(s, (Byte)(strm->adler & 0xff));
            put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
            strm->adler = crc32(0L, Z_NULL, 0);
        }
        s->status = BUSY_STATE;

        /* Compression must start with an empty pending buffer */
        flush_pending(strm);
        if (s->pending != 0) {
            s->last_flush = -1;
            return Z_OK;
        }
    }
#endif

    /* Start a new block or continue the current one.
     */
    if (strm->avail_in != 0 || s->lookahead != 0 ||
        (flush != Z_NO_FLUSH && s->status != FINISH_STATE)) {
        block_state bstate;

        bstate = s->level == 0 ? deflate_stored(s, flush) :
                 s->strategy == Z_HUFFMAN_ONLY ? deflate_huff(s, flush) :
                 s->strategy == Z_RLE ? deflate_rle(s, flush) :
                 (*(configuration_table[s->level].func))(s, flush);

        if (bstate == finish_started || bstate == finish_done) {
            s->status = FINISH_STATE;
        }
        if (bstate == need_more || bstate == finish_started) {
            if (strm->avail_out == 0) {
                s->last_flush = -1; /* avoid BUF_ERROR next call, see above */
            }
            return Z_OK;
            /* If flush != Z_NO_FLUSH && avail_out == 0, the next call
             * of deflate should use the same flush parameter to make sure
             * that the flush is complete. So we don't have to output an
             * empty block here, this will be done at next call. This also
             * ensures that for a very small output buffer, we emit at most
             * one empty block.
             */
        }
        if (bstate == block_done) {
            if (flush == Z_PARTIAL_FLUSH) {
                _tr_align(s);
            } else if (flush != Z_BLOCK) { /* FULL_FLUSH or SYNC_FLUSH */
                _tr_stored_block(s, (char*)0, 0L, 0);
                /* For a full flush, this empty block will be recognized
                 * as a special marker by inflate_sync().
                 */
                if (flush == Z_FULL_FLUSH) {
                    CLEAR_HASH(s);             /* forget history */
                    if (s->lookahead == 0) {
                        s->strstart = 0;
                        s->block_start = 0L;
                        s->insert = 0;
                    }
                }
            }
            flush_pending(strm);
            if (strm->avail_out == 0) {
              s->last_flush = -1; /* avoid BUF_ERROR at next call, see above */
              return Z_OK;
            }
        }
    }

    if (flush != Z_FINISH) return Z_OK;
    if (s->wrap <= 0) return Z_STREAM_END;

    /* Write the trailer */
#ifdef GZIP
    if (s->wrap == 2) {
        put_byte(s, (Byte)(strm->adler & 0xff));
        put_byte(s, (Byte)((strm->adler >> 8) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 16) & 0xff));
        put_byte(s, (Byte)((strm->adler >> 24) & 0xff));
        put_byte(s, (Byte)(strm->total_in & 0xff));
        put_byte(s, (Byte)((strm->total_in >> 8) & 0xff));
        put_byte(s, (Byte)((strm->total_in >> 16) & 0xff));
        put_byte(s, (Byte)((strm->total_in >> 24) & 0xff));
    }
    else
#endif
    {
        putShortMSB(s, (uInt)(strm->adler >> 16));
        putShortMSB(s, (uInt)(strm->adler & 0xffff));
    }
    flush_pending(strm);
    /* If avail_out is zero, the application will call deflate again
     * to flush the rest.
     */
    if (s->wrap > 0) s->wrap = -s->wrap; /* write the trailer only once! */
    return s->pending != 0 ? Z_OK : Z_STREAM_END;
}

/* ========================================================================= */
int ZEXPORT deflateEnd(z_streamp strm) {
    int status;

    if (deflateStateCheck(strm)) return Z_STREAM_ERROR;

    status = strm->state->status;

    /* Deallocate in reverse order of allocations: */
    TRY_FREE(strm, strm->state->pending_buf);
    TRY_FREE(strm, strm->state->head);
    TRY_FREE(strm, strm->state->prev);
    TRY_FREE(strm, strm->state->window);

    ZFREE(strm, strm->state);
    strm->state = Z_NULL;

    return status == BUSY_STATE ? Z_DATA_ERROR : Z_OK;
}

/* =========================================================================
 * Copy the source state to the destination state.
 * To simplify the source, this is not supported for 16-bit MSDOS (which
 * doesn't have enough memory anyway to duplicate compression states).
 */
int ZEXPORT deflateCopy(z_streamp dest, z_streamp source) {
#ifdef MAXSEG_64K
    (void)dest;
    (void)source;
    return Z_STREAM_ERROR;
#else
    deflate_state *ds;
    deflate_state *ss;


    if (deflateStateCheck(source) || dest == Z_NULL) {
        return Z_STREAM_ERROR;
    }

    ss = source->state;

    zmemcpy((voidpf)dest, (voidpf)source, sizeof(z_stream));

    ds = (deflate_state *) ZALLOC(dest, 1, sizeof(deflate_state));
    if (ds == Z_NULL) return Z_MEM_ERROR;
    dest->state = (struct internal_state FAR *) ds;
    zmemcpy((voidpf)ds, (voidpf)ss, sizeof(deflate_state));
    ds->strm = dest;

    ds->window = (Bytef *) ZALLOC(dest, ds->w_size, 2*sizeof(Byte));
    ds->prev   = (Posf *)  ZALLOC(dest, ds->w_size, sizeof(Pos));
    ds->head   = (Posf *)  ZALLOC(dest, ds->hash_size, sizeof(Pos));
    ds->pending_buf = (uchf *) ZALLOC(dest, ds->lit_bufsize, 4);

    if (ds->window == Z_NULL || ds->prev == Z_NULL || ds->head == Z_NULL ||
        ds->pending_buf == Z_NULL) {
        deflateEnd (dest);
        return Z_MEM_ERROR;
    }
    /* following zmemcpy do not work for 16-bit MSDOS */
    zmemcpy(ds->window, ss->window, ds->w_size * 2 * sizeof(Byte));
    zmemcpy((voidpf)ds->prev, (voidpf)ss->prev, ds->w_size * sizeof(Pos));
    zmemcpy((voidpf)ds->head, (voidpf)ss->head, ds->hash_size * sizeof(Pos));
    zmemcpy(ds->pending_buf, ss->pending_buf, (uInt)ds->pending_buf_size);

    ds->pending_out = ds->pending_buf + (ss->pending_out - ss->pending_buf);
#ifdef LIT_MEM
    ds->d_buf = (ushf *)(ds->pending_buf + (ds->lit_bufsize << 1));
    ds->l_buf = ds->pending_buf + (ds->lit_bufsize << 2);
#else
    ds->sym_buf = ds->pending_buf + ds->lit_bufsize;
#endif

    ds->l_desc.dyn_tree = ds->dyn_ltree;
    ds->d_desc.dyn_tree = ds->dyn_dtree;
    ds->bl_desc.dyn_tree = ds->bl_tree;

    return Z_OK;
#endif /* MAXSEG_64K */
}

#ifndef FASTEST
/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 * OUT assertion: the match length is not greater than s->lookahead.
 */
local uInt longest_match(deflate_state *s, IPos cur_match) {
    unsigned chain_length = s->max_chain_length;/* max hash chain length */
    register Bytef *scan = s->window + s->strstart; /* current string */
    register Bytef *match;                      /* matched string */
    register int len;                           /* length of current match */
    int best_len = (int)s->prev_length;         /* best match length so far */
    int nice_match = s->nice_match;             /* stop if match long enough */
    IPos limit = s->strstart > (IPos)MAX_DIST(s) ?
        s->strstart - (IPos)MAX_DIST(s) : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */
    Posf *prev = s->prev;
    uInt wmask = s->w_mask;

#ifdef UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    register Bytef *strend = s->window + s->strstart + MAX_MATCH - 1;
    register ush scan_start = *(ushf*)scan;
    register ush scan_end   = *(ushf*)(scan + best_len - 1);
#else
    register Bytef *strend = s->window + s->strstart + MAX_MATCH;
    register Byte scan_end1  = scan[best_len - 1];
    register Byte scan_end   = scan[best_len];
#endif

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_bits >= 8 && MAX_MATCH == 258, "Code too clever");

    /* Do not waste too much time if we already have a good match: */
    if (s->prev_length >= s->good_match) {
        chain_length >>= 2;
    }
    /* Do not look for matches beyond the end of the input. This is necessary
     * to make deflate deterministic.
     */
    if ((uInt)nice_match > s->lookahead) nice_match = (int)s->lookahead;

    Assert((ulg)s->strstart <= s->window_size - MIN_LOOKAHEAD,
           "need lookahead");

    do {
        Assert(cur_match < s->strstart, "no future");
        match = s->window + cur_match;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2.  Note that the checks below
         * for insufficient lookahead only occur occasionally for performance
         * reasons.  Therefore uninitialized memory will be accessed, and
         * conditional jumps will be made that depend on those values.
         * However the length of the match is limited to the lookahead, so
         * the output of deflate is not affected by the uninitialized values.
         */
#if (defined(UNALIGNED_OK) && MAX_MATCH == 258)
        /* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
        if (*(ushf*)(match + best_len - 1) != scan_end ||
            *(ushf*)match != scan_start) continue;

        /* It is not necessary to compare scan[2] and match[2] since they are
         * always equal when the other bytes match, given that the hash keys
         * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
         * strstart + 3, + 5, up to strstart + 257. We check for insufficient
         * lookahead only every 4th comparison; the 128th check will be made
         * at strstart + 257. If MAX_MATCH-2 is not a multiple of 8, it is
         * necessary to put more guard bytes at the end of the window, or
         * to check more often for insufficient lookahead.
         */
        Assert(scan[2] == match[2], "scan[2]?");
        scan++, match++;
        do {
        } while (*(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 *(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 *(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 *(ushf*)(scan += 2) == *(ushf*)(match += 2) &&
                 scan < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window + strstart + 257 */
        Assert(scan <= s->window + (unsigned)(s->window_size - 1),
               "wild scan");
        if (*scan == *match) scan++;

        len = (MAX_MATCH - 1) - (int)(strend - scan);
        scan = strend - (MAX_MATCH-1);

#else /* UNALIGNED_OK */

        if (match[best_len]     != scan_end  ||
            match[best_len - 1] != scan_end1 ||
            *match              != *scan     ||
            *++match            != scan[1])      continue;

        /* The check at best_len - 1 can be removed because it will be made
         * again later. (This heuristic is not always a win.)
         * It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.
         */
        scan += 2, match++;
        Assert(*scan == *match, "match[2]?");

        /* We check for insufficient lookahead only every 8th comparison;
         * the 256th check will be made at strstart + 258.
         */
        do {
        } while (*++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 scan < strend);

        Assert(scan <= s->window + (unsigned)(s->window_size - 1),
               "wild scan");

        len = MAX_MATCH - (int)(strend - scan);
        scan = strend - MAX_MATCH;

#endif /* UNALIGNED_OK */

        if (len > best_len) {
            s->match_start = cur_match;
            best_len = len;
            if (len >= nice_match) break;
#ifdef UNALIGNED_OK
            scan_end = *(ushf*)(scan + best_len - 1);
#else
            scan_end1  = scan[best_len - 1];
            scan_end   = scan[best_len];
#endif
        }
    } while ((cur_match = prev[cur_match & wmask]) > limit
             && --chain_length != 0);

    if ((uInt)best_len <= s->lookahead) return (uInt)best_len;
    return s->lookahead;
}

#else /* FASTEST */

/* ---------------------------------------------------------------------------
 * Optimized version for FASTEST only
 */
local uInt longest_match(deflate_state *s, IPos cur_match) {
    register Bytef *scan = s->window + s->strstart; /* current string */
    register Bytef *match;                       /* matched string */
    register int len;                           /* length of current match */
    register Bytef *strend = s->window + s->strstart + MAX_MATCH;

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary.
     */
    Assert(s->hash_bits >= 8 && MAX_MATCH == 258, "Code too clever");

    Assert((ulg)s->strstart <= s->window_size - MIN_LOOKAHEAD,
           "need lookahead");

    Assert(cur_match < s->strstart, "no future");

    match = s->window + cur_match;

    /* Return failure if the match length is less than 2:
     */
    if (match[0] != scan[0] || match[1] != scan[1]) return MIN_MATCH-1;

    /* The check at best_len - 1 can be removed because it will be made
     * again later. (This heuristic is not always a win.)
     * It is not necessary to compare scan[2] and match[2] since they
     * are always equal when the other bytes match, given that
     * the hash keys are equal and that HASH_BITS >= 8.
     */
    scan += 2, match += 2;
    Assert(*scan == *match, "match[2]?");

    /* We check for insufficient lookahead only every 8th comparison;
     * the 256th check will be made at strstart + 258.
     */
    do {
    } while (*++scan == *++match && *++scan == *++match &&
             *++scan == *++match && *++scan == *++match &&
             *++scan == *++match && *++scan == *++match &&
             *++scan == *++match && *++scan == *++match &&
             scan < strend);

    Assert(scan <= s->window + (unsigned)(s->window_size - 1), "wild scan");

    len = MAX_MATCH - (int)(strend - scan);

    if (len < MIN_MATCH) return MIN_MATCH - 1;

    s->match_start = cur_match;
    return (uInt)len <= s->lookahead ? (uInt)len : s->lookahead;
}

#endif /* FASTEST */

#ifdef ZLIB_DEBUG

#define EQUAL 0
/* result of memcmp for equal strings */

/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
local void check_match(deflate_state *s, IPos start, IPos match, int length) {
    /* check that the match is indeed a match */
    if (zmemcmp(s->window + match,
                s->window + start, length) != EQUAL) {
        fprintf(stderr, " start %u, match %u, length %d\n",
                start, match, length);
        do {
            fprintf(stderr, "%c%c", s->window[match++], s->window[start++]);
        } while (--length != 0);
        z_error("invalid match");
    }
    if (z_verbose > 1) {
        fprintf(stderr,"\\[%d,%d]", start - match, length);
        do { putc(s->window[start++], stderr); } while (--length != 0);
    }
}
#else
#  define check_match(s, start, match, length)
#endif /* ZLIB_DEBUG */

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#define FLUSH_BLOCK_ONLY(s, last) { \
   _tr_flush_block(s, (s->block_start >= 0L ? \
                   (charf *)&s->window[(unsigned)s->block_start] : \
                   (charf *)Z_NULL), \
                (ulg)((long)s->strstart - s->block_start), \
                (last)); \
   s->block_start = s->strstart; \
   flush_pending(s->strm); \
   Tracev((stderr,"[FLUSH]")); \
}

/* Same but force premature exit if necessary. */
#define FLUSH_BLOCK(s, last) { \
   FLUSH_BLOCK_ONLY(s, last); \
   if (s->strm->avail_out == 0) return (last) ? finish_started : need_more; \
}

/* Maximum stored block length in deflate format (not including header). */
#define MAX_STORED 65535

/* Minimum of a and b. */
#define MIN(a, b) ((a) > (b) ? (b) : (a))

/* ===========================================================================
 * Copy without compression as much as possible from the input stream, return
 * the current block state.
 *
 * In case deflateParams() is used to later switch to a non-zero compression
 * level, s->matches (otherwise unused when storing) keeps track of the number
 * of hash table slides to perform. If s->matches is 1, then one hash table
 * slide will be done when switching. If s->matches is 2, the maximum value
 * allowed here, then the hash table will be cleared, since two or more slides
 * is the same as a clear.
 *
 * deflate_stored() is written to minimize the number of times an input byte is
 * copied. It is most efficient with large input and output buffers, which
 * maximizes the opportunities to have a single copy from next_in to next_out.
 */
local block_state deflate_stored(deflate_state *s, int flush) {
    /* Smallest worthy block size when not flushing or finishing. By default
     * this is 32K. This can be as small as 507 bytes for memLevel == 1. For
     * large input and output buffers, the stored block size will be larger.
     */
    unsigned min_block = MIN(s->pending_buf_size - 5, s->w_size);

    /* Copy as many min_block or larger stored blocks directly to next_out as
     * possible. If flushing, copy the remaining available input to next_out as
     * stored blocks, if there is enough space.
     */
    unsigned len, left, have, last = 0;
    unsigned used = s->strm->avail_in;
    do {
        /* Set len to the maximum size block that we can copy directly with the
         * available input data and output space. Set left to how much of that
         * would be copied from what's left in the window.
         */
        len = MAX_STORED;       /* maximum deflate stored block length */
        have = (s->bi_valid + 42) >> 3;         /* number of header bytes */
        if (s->strm->avail_out < have)          /* need room for header */
            break;
            /* maximum stored block length that will fit in avail_out: */
        have = s->strm->avail_out - have;
        left = s->strstart - s->block_start;    /* bytes left in window */
        if (len > (ulg)left + s->strm->avail_in)
            len = left + s->strm->avail_in;     /* limit len to the input */
        if (len > have)
            len = have;                         /* limit len to the output */

        /* If the stored block would be less than min_block in length, or if
         * unable to copy all of the available input when flushing, then try
         * copying to the window and the pending buffer instead. Also don't
         * write an empty block when flushing -- deflate() does that.
         */
        if (len < min_block && ((len == 0 && flush != Z_FINISH) ||
                                flush == Z_NO_FLUSH ||
                                len != left + s->strm->avail_in))
            break;

        /* Make a dummy stored block in pending to get the header bytes,
         * including any pending bits. This also updates the debugging counts.
         */
        last = flush == Z_FINISH && len == left + s->strm->avail_in ? 1 : 0;
        _tr_stored_block(s, (char *)0, 0L, last);

        /* Replace the lengths in the dummy stored block with len. */
        s->pending_buf[s->pending - 4] = len;
        s->pending_buf[s->pending - 3] = len >> 8;
        s->pending_buf[s->pending - 2] = ~len;
        s->pending_buf[s->pending - 1] = ~len >> 8;

        /* Write the stored block header bytes. */
        flush_pending(s->strm);

#ifdef ZLIB_DEBUG
        /* Update debugging counts for the data about to be copied. */
        s->compressed_len += len << 3;
        s->bits_sent += len << 3;
#endif

        /* Copy uncompressed bytes from the window to next_out. */
        if (left) {
            if (left > len)
                left = len;
            zmemcpy(s->strm->next_out, s->window + s->block_start, left);
            s->strm->next_out += left;
            s->strm->avail_out -= left;
            s->strm->total_out += left;
            s->block_start += left;
            len -= left;
        }

        /* Copy uncompressed bytes directly from next_in to next_out, updating
         * the check value.
         */
        if (len) {
            read_buf(s->strm, s->strm->next_out, len);
            s->strm->next_out += len;
            s->strm->avail_out -= len;
            s->strm->total_out += len;
        }
    } while (last == 0);

    /* Update the sliding window with the last s->w_size bytes of the copied
     * data, or append all of the copied data to the existing window if less
     * than s->w_size bytes were copied. Also update the number of bytes to
     * insert in the hash tables, in the event that deflateParams() switches to
     * a non-zero compression level.
     */
    used -= s->strm->avail_in;      /* number of input bytes directly copied */
    if (used) {
        /* If any input was used, then no unused input remains in the window,
         * therefore s->block_start == s->strstart.
         */
        if (used >= s->w_size) {    /* supplant the previous history */
            s->matches = 2;         /* clear hash */
            zmemcpy(s->window, s->strm->next_in - s->w_size, s->w_size);
            s->strstart = s->w_size;
            s->insert = s->strstart;
        }
        else {
            if (s->window_size - s->strstart <= used) {
                /* Slide the window down. */
                s->strstart -= s->w_size;
                zmemcpy(s->window, s->window + s->w_size, s->strstart);
                if (s->matches < 2)
                    s->matches++;   /* add a pending slide_hash() */
                if (s->insert > s->strstart)
                    s->insert = s->strstart;
            }
            zmemcpy(s->window + s->strstart, s->strm->next_in - used, used);
            s->strstart += used;
            s->insert += MIN(used, s->w_size - s->insert);
        }
        s->block_start = s->strstart;
    }
    if (s->high_water < s->strstart)
        s->high_water = s->strstart;

    /* If the last block was written to next_out, then done. */
    if (last)
        return finish_done;

    /* If flushing and all input has been consumed, then done. */
    if (flush != Z_NO_FLUSH && flush != Z_FINISH &&
        s->strm->avail_in == 0 && (long)s->strstart == s->block_start)
        return block_done;

    /* Fill the window with any remaining input. */
    have = s->window_size - s->strstart;
    if (s->strm->avail_in > have && s->block_start >= (long)s->w_size) {
        /* Slide the window down. */
        s->block_start -= s->w_size;
        s->strstart -= s->w_size;
        zmemcpy(s->window, s->window + s->w_size, s->strstart);
        if (s->matches < 2)
            s->matches++;           /* add a pending slide_hash() */
        have += s->w_size;          /* more space now */
        if (s->insert > s->strstart)
            s->insert = s->strstart;
    }
    if (have > s->strm->avail_in)
        have = s->strm->avail_in;
    if (have) {
        read_buf(s->strm, s->window + s->strstart, have);
        s->strstart += have;
        s->insert += MIN(have, s->w_size - s->insert);
    }
    if (s->high_water < s->strstart)
        s->high_water = s->strstart;

    /* There was not enough avail_out to write a complete worthy or flushed
     * stored block to next_out. Write a stored block to pending instead, if we
     * have enough input for a worthy block, or if flushing and there is enough
     * room for the remaining input as a stored block in the pending buffer.
     */
    have = (s->bi_valid + 42) >> 3;         /* number of header bytes */
        /* maximum stored block length that will fit in pending: */
    have = MIN(s->pending_buf_size - have, MAX_STORED);
    min_block = MIN(have, s->w_size);
    left = s->strstart - s->block_start;
    if (left >= min_block ||
        ((left || flush == Z_FINISH) && flush != Z_NO_FLUSH &&
         s->strm->avail_in == 0 && left <= have)) {
        len = MIN(left, have);
        last = flush == Z_FINISH && s->strm->avail_in == 0 &&
               len == left ? 1 : 0;
        _tr_stored_block(s, (charf *)s->window + s->block_start, len, last);
        s->block_start += len;
        flush_pending(s->strm);
    }

    /* We've done all we can with the available input and output. */
    return last ? finish_started : need_more;
}

/* ===========================================================================
 * Compress as much as possible from the input stream, return the current
 * block state.
 * This function does not perform lazy evaluation of matches and inserts
 * new strings in the dictionary only for unmatched strings or for short
 * matches. It is used only for the fast compression options.
 */
local block_state deflate_fast(deflate_state *s, int flush) {
    IPos hash_head;       /* head of the hash chain */
    int bflush;           /* set if current block must be flushed */

    for (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        if (s->lookahead < MIN_LOOKAHEAD) {
            fill_window(s);
            if (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                return need_more;
            }
            if (s->lookahead == 0) break; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_head = NIL;
        if (s->lookahead >= MIN_MATCH) {
            INSERT_STRING(s, s->strstart, hash_head);
        }

        /* Find the longest match, discarding those <= prev_length.
         * At this point we have always match_length < MIN_MATCH
         */
        if (hash_head != NIL && s->strstart - hash_head <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->match_length = longest_match (s, hash_head);
            /* longest_match() sets match_start */
        }
        if (s->match_length >= MIN_MATCH) {
            check_match(s, s->strstart, s->match_start, s->match_length);

            _tr_tally_dist(s, s->strstart - s->match_start,
                           s->match_length - MIN_MATCH, bflush);

            s->lookahead -= s->match_length;

            /* Insert new strings in the hash table only if the match length
             * is not too large. This saves time but degrades compression.
             */
#ifndef FASTEST
            if (s->match_length <= s->max_insert_length &&
                s->lookahead >= MIN_MATCH) {
                s->match_length--; /* string at strstart already in table */
                do {
                    s->strstart++;
                    INSERT_STRING(s, s->strstart, hash_head);
                    /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                     * always MIN_MATCH bytes ahead.
                     */
                } while (--s->match_length != 0);
                s->strstart++;
            } else
#endif
            {
                s->strstart += s->match_length;
                s->match_length = 0;
                s->ins_h = s->window[s->strstart];
                UPDATE_HASH(s, s->ins_h, s->window[s->strstart + 1]);
#if MIN_MATCH != 3
                Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
                /* If lookahead < MIN_MATCH, ins_h is garbage, but it does not
                 * matter since it will be recomputed at next deflate call.
                 */
            }
        } else {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->window[s->strstart]));
            _tr_tally_lit(s, s->window[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        if (bflush) FLUSH_BLOCK(s, 0);
    }
    s->insert = s->strstart < MIN_MATCH-1 ? s->strstart : MIN_MATCH-1;
    if (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        return finish_done;
    }
    if (s->sym_next)
        FLUSH_BLOCK(s, 0);
    return block_done;
}

#ifndef FASTEST
/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
local block_state deflate_slow(deflate_state *s, int flush) {
    IPos hash_head;          /* head of hash chain */
    int bflush;              /* set if current block must be flushed */

    /* Process the input block. */
    for (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        if (s->lookahead < MIN_LOOKAHEAD) {
            fill_window(s);
            if (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) {
                return need_more;
            }
            if (s->lookahead == 0) break; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart + 2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        hash_head = NIL;
        if (s->lookahead >= MIN_MATCH) {
            INSERT_STRING(s, s->strstart, hash_head);
        }

        /* Find the longest match, discarding those <= prev_length.
         */
        s->prev_length = s->match_length, s->prev_match = s->match_start;
        s->match_length = MIN_MATCH-1;

        if (hash_head != NIL && s->prev_length < s->max_lazy_match &&
            s->strstart - hash_head <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            s->match_length = longest_match (s, hash_head);
            /* longest_match() sets match_start */

            if (s->match_length <= 5 && (s->strategy == Z_FILTERED
#if TOO_FAR <= 32767
                || (s->match_length == MIN_MATCH &&
                    s->strstart - s->match_start > TOO_FAR)
#endif
                )) {

                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                s->match_length = MIN_MATCH-1;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        if (s->prev_length >= MIN_MATCH && s->match_length <= s->prev_length) {
            uInt max_insert = s->strstart + s->lookahead - MIN_MATCH;
            /* Do not insert strings in hash table beyond this. */

            check_match(s, s->strstart - 1, s->prev_match, s->prev_length);

            _tr_tally_dist(s, s->strstart - 1 - s->prev_match,
                           s->prev_length - MIN_MATCH, bflush);

            /* Insert in hash table all strings up to the end of the match.
             * strstart - 1 and strstart are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            s->lookahead -= s->prev_length - 1;
            s->prev_length -= 2;
            do {
                if (++s->strstart <= max_insert) {
                    INSERT_STRING(s, s->strstart, hash_head);
                }
            } while (--s->prev_length != 0);
            s->match_available = 0;
            s->match_length = MIN_MATCH-1;
            s->strstart++;

            if (bflush) FLUSH_BLOCK(s, 0);

        } else if (s->match_available) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c", s->window[s->strstart - 1]));
            _tr_tally_lit(s, s->window[s->strstart - 1], bflush);
            if (bflush) {
                FLUSH_BLOCK_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            if (s->strm->avail_out == 0) return need_more;
        } else {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            s->match_available = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    Assert (flush != Z_NO_FLUSH, "no flush?");
    if (s->match_available) {
        Tracevv((stderr,"%c", s->window[s->strstart - 1]));
        _tr_tally_lit(s, s->window[s->strstart - 1], bflush);
        s->match_available = 0;
    }
    s->insert = s->strstart < MIN_MATCH-1 ? s->strstart : MIN_MATCH-1;
    if (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        return finish_done;
    }
    if (s->sym_next)
        FLUSH_BLOCK(s, 0);
    return block_done;
}
#endif /* FASTEST */

/* ===========================================================================
 * For Z_RLE, simply look for runs of bytes, generate matches only of distance
 * one.  Do not maintain a hash table.  (It will be regenerated if this run of
 * deflate switches away from Z_RLE.)
 */
local block_state deflate_rle(deflate_state *s, int flush) {
    int bflush;             /* set if current block must be flushed */
    uInt prev;              /* byte at distance one to match */
    Bytef *scan, *strend;   /* scan goes up to strend for length of run */

    for (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the longest run, plus one for the unrolled loop.
         */
        if (s->lookahead <= MAX_MATCH) {
            fill_window(s);
            if (s->lookahead <= MAX_MATCH && flush == Z_NO_FLUSH) {
                return need_more;
            }
            if (s->lookahead == 0) break; /* flush the current block */
        }

        /* See how many times the previous byte repeats */
        s->match_length = 0;
        if (s->lookahead >= MIN_MATCH && s->strstart > 0) {
            scan = s->window + s->strstart - 1;
            prev = *scan;
            if (prev == *++scan && prev == *++scan && prev == *++scan) {
                strend = s->window + s->strstart + MAX_MATCH;
                do {
                } while (prev == *++scan && prev == *++scan &&
                         prev == *++scan && prev == *++scan &&
                         prev == *++scan && prev == *++scan &&
                         prev == *++scan && prev == *++scan &&
                         scan < strend);
                s->match_length = MAX_MATCH - (uInt)(strend - scan);
                if (s->match_length > s->lookahead)
                    s->match_length = s->lookahead;
            }
            Assert(scan <= s->window + (uInt)(s->window_size - 1),
                   "wild scan");
        }

        /* Emit match if have run of MIN_MATCH or longer, else emit literal */
        if (s->match_length >= MIN_MATCH) {
            check_match(s, s->strstart, s->strstart - 1, s->match_length);

            _tr_tally_dist(s, 1, s->match_length - MIN_MATCH, bflush);

            s->lookahead -= s->match_length;
            s->strstart += s->match_length;
            s->match_length = 0;
        } else {
            /* No match, output a literal byte */
            Tracevv((stderr,"%c", s->window[s->strstart]));
            _tr_tally_lit(s, s->window[s->strstart], bflush);
            s->lookahead--;
            s->strstart++;
        }
        if (bflush) FLUSH_BLOCK(s, 0);
    }
    s->insert = 0;
    if (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        return finish_done;
    }
    if (s->sym_next)
        FLUSH_BLOCK(s, 0);
    return block_done;
}

/* ===========================================================================
 * For Z_HUFFMAN_ONLY, do not look for matches.  Do not maintain a hash table.
 * (It will be regenerated if this run of deflate switches away from Huffman.)
 */
local block_state deflate_huff(deflate_state *s, int flush) {
    int bflush;             /* set if current block must be flushed */

    for (;;) {
        /* Make sure that we have a literal to write. */
        if (s->lookahead == 0) {
            fill_window(s);
            if (s->lookahead == 0) {
                if (flush == Z_NO_FLUSH)
                    return need_more;
                break;      /* flush the current block */
            }
        }

        /* Output a literal byte */
        s->match_length = 0;
        Tracevv((stderr,"%c", s->window[s->strstart]));
        _tr_tally_lit(s, s->window[s->strstart], bflush);
        s->lookahead--;
        s->strstart++;
        if (bflush) FLUSH_BLOCK(s, 0);
    }
    s->insert = 0;
    if (flush == Z_FINISH) {
        FLUSH_BLOCK(s, 1);
        return finish_done;
    }
    if (s->sym_next)
        FLUSH_BLOCK(s, 0);
    return block_done;
}
/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2017 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

//REBOL: #include "zutil.h"
#ifndef Z_SOLO
//REBOL: #  include "gzguts.h"
#endif

z_const char * const z_errmsg[10] = {
    (z_const char *)"need dictionary",     /* Z_NEED_DICT       2  */
    (z_const char *)"stream end",          /* Z_STREAM_END      1  */
    (z_const char *)"",                    /* Z_OK              0  */
    (z_const char *)"file error",          /* Z_ERRNO         (-1) */
    (z_const char *)"stream error",        /* Z_STREAM_ERROR  (-2) */
    (z_const char *)"data error",          /* Z_DATA_ERROR    (-3) */
    (z_const char *)"insufficient memory", /* Z_MEM_ERROR     (-4) */
    (z_const char *)"buffer error",        /* Z_BUF_ERROR     (-5) */
    (z_const char *)"incompatible version",/* Z_VERSION_ERROR (-6) */
    (z_const char *)""
};


const char * ZEXPORT zlibVersion(void) {
    return ZLIB_VERSION;
}

uLong ZEXPORT zlibCompileFlags(void) {
    uLong flags;

    flags = 0;
    switch ((int)(sizeof(uInt))) {
    case 2:     break;
    case 4:     flags += 1;     break;
    case 8:     flags += 2;     break;
    default:    flags += 3;
    }
    switch ((int)(sizeof(uLong))) {
    case 2:     break;
    case 4:     flags += 1 << 2;        break;
    case 8:     flags += 2 << 2;        break;
    default:    flags += 3 << 2;
    }
    switch ((int)(sizeof(voidpf))) {
    case 2:     break;
    case 4:     flags += 1 << 4;        break;
    case 8:     flags += 2 << 4;        break;
    default:    flags += 3 << 4;
    }
    switch ((int)(sizeof(z_off_t))) {
    case 2:     break;
    case 4:     flags += 1 << 6;        break;
    case 8:     flags += 2 << 6;        break;
    default:    flags += 3 << 6;
    }
#ifdef ZLIB_DEBUG
    flags += 1 << 8;
#endif
    /*
#if defined(ASMV) || defined(ASMINF)
    flags += 1 << 9;
#endif
     */
#ifdef ZLIB_WINAPI
    flags += 1 << 10;
#endif
#ifdef BUILDFIXED
    flags += 1 << 12;
#endif
#ifdef DYNAMIC_CRC_TABLE
    flags += 1 << 13;
#endif
#ifdef NO_GZCOMPRESS
    flags += 1L << 16;
#endif
#ifdef NO_GZIP
    flags += 1L << 17;
#endif
#ifdef PKZIP_BUG_WORKAROUND
    flags += 1L << 20;
#endif
#ifdef FASTEST
    flags += 1L << 21;
#endif
#if defined(STDC) || defined(Z_HAVE_STDARG_H)
#  ifdef NO_vsnprintf
    flags += 1L << 25;
#    ifdef HAS_vsprintf_void
    flags += 1L << 26;
#    endif
#  else
#    ifdef HAS_vsnprintf_void
    flags += 1L << 26;
#    endif
#  endif
#else
    flags += 1L << 24;
#  ifdef NO_snprintf
    flags += 1L << 25;
#    ifdef HAS_sprintf_void
    flags += 1L << 26;
#    endif
#  else
#    ifdef HAS_snprintf_void
    flags += 1L << 26;
#    endif
#  endif
#endif
    return flags;
}

#ifdef ZLIB_DEBUG
#include <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
int ZLIB_INTERNAL z_verbose = verbose;

void ZLIB_INTERNAL z_error(char *m) {
    fprintf(stderr, "%s\n", m);
    exit(1);
}
#endif

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
const char * ZEXPORT zError(int err) {
    return ERR_MSG(err);
}

#if defined(_WIN32_WCE) && _WIN32_WCE < 0x800
    /* The older Microsoft C Run-Time Library for Windows CE doesn't have
     * errno.  We define it as a global variable to simplify porting.
     * Its value is always 0 and should not be used.
     */
    int errno = 0;
#endif

#ifndef HAVE_MEMCPY

void ZLIB_INTERNAL zmemcpy(Bytef* dest, const Bytef* source, uInt len) {
    if (len == 0) return;
    do {
        *dest++ = *source++; /* ??? to be unrolled */
    } while (--len != 0);
}

int ZLIB_INTERNAL zmemcmp(const Bytef* s1, const Bytef* s2, uInt len) {
    uInt j;

    for (j = 0; j < len; j++) {
        if (s1[j] != s2[j]) return 2*(s1[j] > s2[j])-1;
    }
    return 0;
}

void ZLIB_INTERNAL zmemzero(Bytef* dest, uInt len) {
    if (len == 0) return;
    do {
        *dest++ = 0;  /* ??? to be unrolled */
    } while (--len != 0);
}
#endif

#ifndef Z_SOLO

#ifdef SYS16BIT

#ifdef __TURBOC__
/* Turbo C in 16-bit mode */

#  define MY_ZCALLOC

/* Turbo C malloc() does not allow dynamic allocation of 64K bytes
 * and farmalloc(64K) returns a pointer with an offset of 8, so we
 * must fix the pointer. Warning: the pointer must be put back to its
 * original form in order to free it, use zcfree().
 */

#define MAX_PTR 10
/* 10*64K = 640K */

local int next_ptr = 0;

typedef struct ptr_table_s {
    voidpf org_ptr;
    voidpf new_ptr;
} ptr_table;

local ptr_table table[MAX_PTR];
/* This table is used to remember the original form of pointers
 * to large buffers (64K). Such pointers are normalized with a zero offset.
 * Since MSDOS is not a preemptive multitasking OS, this table is not
 * protected from concurrent access. This hack doesn't work anyway on
 * a protected system like OS/2. Use Microsoft C instead.
 */

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned items, unsigned size) {
    voidpf buf;
    ulg bsize = (ulg)items*size;

    (void)opaque;

    /* If we allocate less than 65520 bytes, we assume that farmalloc
     * will return a usable pointer which doesn't have to be normalized.
     */
    if (bsize < 65520L) {
        buf = farmalloc(bsize);
        if (*(ush*)&buf != 0) return buf;
    } else {
        buf = farmalloc(bsize + 16L);
    }
    if (buf == NULL || next_ptr >= MAX_PTR) return NULL;
    table[next_ptr].org_ptr = buf;

    /* Normalize the pointer to seg:0 */
    *((ush*)&buf+1) += ((ush)((uch*)buf-0) + 15) >> 4;
    *(ush*)&buf = 0;
    table[next_ptr++].new_ptr = buf;
    return buf;
}

void ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    int n;

    (void)opaque;

    if (*(ush*)&ptr != 0) { /* object < 64K */
        farfree(ptr);
        return;
    }
    /* Find the original pointer */
    for (n = 0; n < next_ptr; n++) {
        if (ptr != table[n].new_ptr) continue;

        farfree(table[n].org_ptr);
        while (++n < next_ptr) {
            table[n-1] = table[n];
        }
        next_ptr--;
        return;
    }
    Assert(0, "zcfree: ptr not found");
}

#endif /* __TURBOC__ */


#ifdef M_I86
/* Microsoft C in 16-bit mode */

#  define MY_ZCALLOC

#if (!defined(_MSC_VER) || (_MSC_VER <= 600))
#  define _halloc  halloc
#  define _hfree   hfree
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, uInt items, uInt size) {
    (void)opaque;
    return _halloc((long)items, size);
}

void ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (void)opaque;
    _hfree(ptr);
}

#endif /* M_I86 */

#endif /* SYS16BIT */


#ifndef MY_ZCALLOC /* Any system without a special alloc function */

#ifndef STDC
extern voidp malloc(uInt size);
extern voidp calloc(uInt items, uInt size);
extern void free(voidpf ptr);
#endif

voidpf ZLIB_INTERNAL zcalloc(voidpf opaque, unsigned items, unsigned size) {
    (void)opaque;
    return sizeof(uInt) > 2 ? (voidpf)malloc(items * size) :
                              (voidpf)calloc(items, size);
}

void ZLIB_INTERNAL zcfree(voidpf opaque, voidpf ptr) {
    (void)opaque;
    free(ptr);
}

#endif /* MY_ZCALLOC */

#endif /* !Z_SOLO */
/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2005, 2014, 2016 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#define ZLIB_INTERNAL
//REBOL: #include "zlib.h"

/* ===========================================================================
     Compresses the source buffer into the destination buffer. The level
   parameter has the same meaning as in deflateInit.  sourceLen is the byte
   length of the source buffer. Upon entry, destLen is the total size of the
   destination buffer, which must be at least 0.1% larger than sourceLen plus
   12 bytes. Upon exit, destLen is the actual size of the compressed buffer.

     compress2 returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_BUF_ERROR if there was not enough room in the output buffer,
   Z_STREAM_ERROR if the level parameter is invalid.
*/
int ZEXPORT compress2(Bytef *dest, uLongf *destLen, const Bytef *source,
                      uLong sourceLen, int level) {
    z_stream stream;
    int err;
    const uInt max = (uInt)-1;
    uLong left;

    left = *destLen;
    *destLen = 0;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit(&stream, level);
    if (err != Z_OK) return err;

    stream.next_out = dest;
    stream.avail_out = 0;
    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;

    do {
        if (stream.avail_out == 0) {
            stream.avail_out = left > (uLong)max ? max : (uInt)left;
            left -= stream.avail_out;
        }
        if (stream.avail_in == 0) {
            stream.avail_in = sourceLen > (uLong)max ? max : (uInt)sourceLen;
            sourceLen -= stream.avail_in;
        }
        err = deflate(&stream, sourceLen ? Z_NO_FLUSH : Z_FINISH);
    } while (err == Z_OK);

    *destLen = stream.total_out;
    deflateEnd(&stream);
    return err == Z_STREAM_END ? Z_OK : err;
}

/* ===========================================================================
 */
int ZEXPORT compress(Bytef *dest, uLongf *destLen, const Bytef *source,
                     uLong sourceLen) {
    return compress2(dest, destLen, source, sourceLen, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
uLong ZEXPORT compressBound(uLong sourceLen) {
    return sourceLen + (sourceLen >> 12) + (sourceLen >> 14) +
           (sourceLen >> 25) + 13;
}
/* uncompr.c -- decompress a memory buffer
 * Copyright (C) 1995-2003, 2010, 2014, 2016 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#define ZLIB_INTERNAL
//REBOL: #include "zlib.h"

/* ===========================================================================
     Decompresses the source buffer into the destination buffer.  *sourceLen is
   the byte length of the source buffer. Upon entry, *destLen is the total size
   of the destination buffer, which must be large enough to hold the entire
   uncompressed data. (The size of the uncompressed data must have been saved
   previously by the compressor and transmitted to the decompressor by some
   mechanism outside the scope of this compression library.) Upon exit,
   *destLen is the size of the decompressed data and *sourceLen is the number
   of source bytes consumed. Upon return, source + *sourceLen points to the
   first unused input byte.

     uncompress returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_BUF_ERROR if there was not enough room in the output buffer, or
   Z_DATA_ERROR if the input data was corrupted, including if the input data is
   an incomplete zlib stream.
*/
int ZEXPORT uncompress2(Bytef *dest, uLongf *destLen, const Bytef *source,
                        uLong *sourceLen) {
    z_stream stream;
    int err;
    const uInt max = (uInt)-1;
    uLong len, left;
    Byte buf[1];    /* for detection of incomplete stream when *destLen == 0 */

    len = *sourceLen;
    if (*destLen) {
        left = *destLen;
        *destLen = 0;
    }
    else {
        left = 1;
        dest = buf;
    }

    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = inflateInit(&stream);
    if (err != Z_OK) return err;

    stream.next_out = dest;
    stream.avail_out = 0;

    do {
        if (stream.avail_out == 0) {
            stream.avail_out = left > (uLong)max ? max : (uInt)left;
            left -= stream.avail_out;
        }
        if (stream.avail_in == 0) {
            stream.avail_in = len > (uLong)max ? max : (uInt)len;
            len -= stream.avail_in;
        }
        err = inflate(&stream, Z_NO_FLUSH);
    } while (err == Z_OK);

    *sourceLen -= len + stream.avail_in;
    if (dest != buf)
        *destLen = stream.total_out;
    else if (stream.total_out && err == Z_BUF_ERROR)
        left = 1;

    inflateEnd(&stream);
    return err == Z_STREAM_END ? Z_OK :
           err == Z_NEED_DICT ? Z_DATA_ERROR  :
           err == Z_BUF_ERROR && left + stream.avail_out ? Z_DATA_ERROR :
           err;
}

int ZEXPORT uncompress(Bytef *dest, uLongf *destLen, const Bytef *source,
                       uLong sourceLen) {
    return uncompress2(dest, destLen, source, &sourceLen);
}
/* trees.c -- output deflated data using Huffman coding
 * Copyright (C) 1995-2021 Jean-loup Gailly
 * detect_data_type() function provided freely by Cosmin Truta, 2006
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/*
 *  ALGORITHM
 *
 *      The "deflation" process uses several Huffman trees. The more
 *      common source values are represented by shorter bit sequences.
 *
 *      Each code tree is stored in a compressed form which is itself
 * a Huffman encoding of the lengths of all the code strings (in
 * ascending order by source values).  The actual code strings are
 * reconstructed from the lengths in the inflate process, as described
 * in the deflate specification.
 *
 *  REFERENCES
 *
 *      Deutsch, L.P.,"'Deflate' Compressed Data Format Specification".
 *      Available in ftp.uu.net:/pub/archiving/zip/doc/deflate-1.1.doc
 *
 *      Storer, James A.
 *          Data Compression:  Methods and Theory, pp. 49-50.
 *          Computer Science Press, 1988.  ISBN 0-7167-8156-5.
 *
 *      Sedgewick, R.
 *          Algorithms, p290.
 *          Addison-Wesley, 1983. ISBN 0-201-06672-6.
 */

/* @(#) $Id$ */

/* #define GEN_TREES_H */

//REBOL: #include "deflate.h"

#ifdef ZLIB_DEBUG
#  include <ctype.h>
#endif

/* ===========================================================================
 * Constants
 */

#define MAX_BL_BITS 7
/* Bit length codes must not exceed MAX_BL_BITS bits */

#define END_BLOCK 256
/* end of block literal code */

#define REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#define REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#define REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

local const int extra_lbits[LENGTH_CODES] /* extra bits for each length code */
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

local const int extra_dbits[D_CODES] /* extra bits for each distance code */
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

local const int extra_blbits[BL_CODES]/* extra bits for each bit length code */
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

local const uch bl_order[BL_CODES]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
/* The lengths of the bit length codes are sent in order of decreasing
 * probability, to avoid transmitting the lengths for unused bit length codes.
 */

/* ===========================================================================
 * Local data. These are initialized only once.
 */

#define DIST_CODE_LEN  512 /* see definition of array dist_code below */

#if defined(GEN_TREES_H) || !defined(STDC)
/* non ANSI compilers may not accept trees.h */

local ct_data static_ltree[L_CODES+2];
/* The static literal tree. Since the bit lengths are imposed, there is no
 * need for the L_CODES extra codes used during heap construction. However
 * The codes 286 and 287 are needed to build a canonical tree (see _tr_init
 * below).
 */

local ct_data static_dtree[D_CODES];
/* The static distance tree. (Actually a trivial tree since all codes use
 * 5 bits.)
 */

uch _dist_code[DIST_CODE_LEN];
/* Distance codes. The first 256 values correspond to the distances
 * 3 .. 258, the last 256 values correspond to the top 8 bits of
 * the 15 bit distances.
 */

uch _length_code[MAX_MATCH-MIN_MATCH+1];
/* length code for each normalized match length (0 == MIN_MATCH) */

local int base_length[LENGTH_CODES];
/* First normalized length for each code (0 = MIN_MATCH) */

local int base_dist[D_CODES];
/* First normalized distance for each code (0 = distance of 1) */

#else
//REBOL: #  include "trees.h"
/* header created automatically with -DGEN_TREES_H */

local const ct_data static_ltree[L_CODES+2] = {
{{ 12},{  8}}, {{140},{  8}}, {{ 76},{  8}}, {{204},{  8}}, {{ 44},{  8}},
{{172},{  8}}, {{108},{  8}}, {{236},{  8}}, {{ 28},{  8}}, {{156},{  8}},
{{ 92},{  8}}, {{220},{  8}}, {{ 60},{  8}}, {{188},{  8}}, {{124},{  8}},
{{252},{  8}}, {{  2},{  8}}, {{130},{  8}}, {{ 66},{  8}}, {{194},{  8}},
{{ 34},{  8}}, {{162},{  8}}, {{ 98},{  8}}, {{226},{  8}}, {{ 18},{  8}},
{{146},{  8}}, {{ 82},{  8}}, {{210},{  8}}, {{ 50},{  8}}, {{178},{  8}},
{{114},{  8}}, {{242},{  8}}, {{ 10},{  8}}, {{138},{  8}}, {{ 74},{  8}},
{{202},{  8}}, {{ 42},{  8}}, {{170},{  8}}, {{106},{  8}}, {{234},{  8}},
{{ 26},{  8}}, {{154},{  8}}, {{ 90},{  8}}, {{218},{  8}}, {{ 58},{  8}},
{{186},{  8}}, {{122},{  8}}, {{250},{  8}}, {{  6},{  8}}, {{134},{  8}},
{{ 70},{  8}}, {{198},{  8}}, {{ 38},{  8}}, {{166},{  8}}, {{102},{  8}},
{{230},{  8}}, {{ 22},{  8}}, {{150},{  8}}, {{ 86},{  8}}, {{214},{  8}},
{{ 54},{  8}}, {{182},{  8}}, {{118},{  8}}, {{246},{  8}}, {{ 14},{  8}},
{{142},{  8}}, {{ 78},{  8}}, {{206},{  8}}, {{ 46},{  8}}, {{174},{  8}},
{{110},{  8}}, {{238},{  8}}, {{ 30},{  8}}, {{158},{  8}}, {{ 94},{  8}},
{{222},{  8}}, {{ 62},{  8}}, {{190},{  8}}, {{126},{  8}}, {{254},{  8}},
{{  1},{  8}}, {{129},{  8}}, {{ 65},{  8}}, {{193},{  8}}, {{ 33},{  8}},
{{161},{  8}}, {{ 97},{  8}}, {{225},{  8}}, {{ 17},{  8}}, {{145},{  8}},
{{ 81},{  8}}, {{209},{  8}}, {{ 49},{  8}}, {{177},{  8}}, {{113},{  8}},
{{241},{  8}}, {{  9},{  8}}, {{137},{  8}}, {{ 73},{  8}}, {{201},{  8}},
{{ 41},{  8}}, {{169},{  8}}, {{105},{  8}}, {{233},{  8}}, {{ 25},{  8}},
{{153},{  8}}, {{ 89},{  8}}, {{217},{  8}}, {{ 57},{  8}}, {{185},{  8}},
{{121},{  8}}, {{249},{  8}}, {{  5},{  8}}, {{133},{  8}}, {{ 69},{  8}},
{{197},{  8}}, {{ 37},{  8}}, {{165},{  8}}, {{101},{  8}}, {{229},{  8}},
{{ 21},{  8}}, {{149},{  8}}, {{ 85},{  8}}, {{213},{  8}}, {{ 53},{  8}},
{{181},{  8}}, {{117},{  8}}, {{245},{  8}}, {{ 13},{  8}}, {{141},{  8}},
{{ 77},{  8}}, {{205},{  8}}, {{ 45},{  8}}, {{173},{  8}}, {{109},{  8}},
{{237},{  8}}, {{ 29},{  8}}, {{157},{  8}}, {{ 93},{  8}}, {{221},{  8}},
{{ 61},{  8}}, {{189},{  8}}, {{125},{  8}}, {{253},{  8}}, {{ 19},{  9}},
{{275},{  9}}, {{147},{  9}}, {{403},{  9}}, {{ 83},{  9}}, {{339},{  9}},
{{211},{  9}}, {{467},{  9}}, {{ 51},{  9}}, {{307},{  9}}, {{179},{  9}},
{{435},{  9}}, {{115},{  9}}, {{371},{  9}}, {{243},{  9}}, {{499},{  9}},
{{ 11},{  9}}, {{267},{  9}}, {{139},{  9}}, {{395},{  9}}, {{ 75},{  9}},
{{331},{  9}}, {{203},{  9}}, {{459},{  9}}, {{ 43},{  9}}, {{299},{  9}},
{{171},{  9}}, {{427},{  9}}, {{107},{  9}}, {{363},{  9}}, {{235},{  9}},
{{491},{  9}}, {{ 27},{  9}}, {{283},{  9}}, {{155},{  9}}, {{411},{  9}},
{{ 91},{  9}}, {{347},{  9}}, {{219},{  9}}, {{475},{  9}}, {{ 59},{  9}},
{{315},{  9}}, {{187},{  9}}, {{443},{  9}}, {{123},{  9}}, {{379},{  9}},
{{251},{  9}}, {{507},{  9}}, {{  7},{  9}}, {{263},{  9}}, {{135},{  9}},
{{391},{  9}}, {{ 71},{  9}}, {{327},{  9}}, {{199},{  9}}, {{455},{  9}},
{{ 39},{  9}}, {{295},{  9}}, {{167},{  9}}, {{423},{  9}}, {{103},{  9}},
{{359},{  9}}, {{231},{  9}}, {{487},{  9}}, {{ 23},{  9}}, {{279},{  9}},
{{151},{  9}}, {{407},{  9}}, {{ 87},{  9}}, {{343},{  9}}, {{215},{  9}},
{{471},{  9}}, {{ 55},{  9}}, {{311},{  9}}, {{183},{  9}}, {{439},{  9}},
{{119},{  9}}, {{375},{  9}}, {{247},{  9}}, {{503},{  9}}, {{ 15},{  9}},
{{271},{  9}}, {{143},{  9}}, {{399},{  9}}, {{ 79},{  9}}, {{335},{  9}},
{{207},{  9}}, {{463},{  9}}, {{ 47},{  9}}, {{303},{  9}}, {{175},{  9}},
{{431},{  9}}, {{111},{  9}}, {{367},{  9}}, {{239},{  9}}, {{495},{  9}},
{{ 31},{  9}}, {{287},{  9}}, {{159},{  9}}, {{415},{  9}}, {{ 95},{  9}},
{{351},{  9}}, {{223},{  9}}, {{479},{  9}}, {{ 63},{  9}}, {{319},{  9}},
{{191},{  9}}, {{447},{  9}}, {{127},{  9}}, {{383},{  9}}, {{255},{  9}},
{{511},{  9}}, {{  0},{  7}}, {{ 64},{  7}}, {{ 32},{  7}}, {{ 96},{  7}},
{{ 16},{  7}}, {{ 80},{  7}}, {{ 48},{  7}}, {{112},{  7}}, {{  8},{  7}},
{{ 72},{  7}}, {{ 40},{  7}}, {{104},{  7}}, {{ 24},{  7}}, {{ 88},{  7}},
{{ 56},{  7}}, {{120},{  7}}, {{  4},{  7}}, {{ 68},{  7}}, {{ 36},{  7}},
{{100},{  7}}, {{ 20},{  7}}, {{ 84},{  7}}, {{ 52},{  7}}, {{116},{  7}},
{{  3},{  8}}, {{131},{  8}}, {{ 67},{  8}}, {{195},{  8}}, {{ 35},{  8}},
{{163},{  8}}, {{ 99},{  8}}, {{227},{  8}}
};

local const ct_data static_dtree[D_CODES] = {
{{ 0},{ 5}}, {{16},{ 5}}, {{ 8},{ 5}}, {{24},{ 5}}, {{ 4},{ 5}},
{{20},{ 5}}, {{12},{ 5}}, {{28},{ 5}}, {{ 2},{ 5}}, {{18},{ 5}},
{{10},{ 5}}, {{26},{ 5}}, {{ 6},{ 5}}, {{22},{ 5}}, {{14},{ 5}},
{{30},{ 5}}, {{ 1},{ 5}}, {{17},{ 5}}, {{ 9},{ 5}}, {{25},{ 5}},
{{ 5},{ 5}}, {{21},{ 5}}, {{13},{ 5}}, {{29},{ 5}}, {{ 3},{ 5}},
{{19},{ 5}}, {{11},{ 5}}, {{27},{ 5}}, {{ 7},{ 5}}, {{23},{ 5}}
};

const uch ZLIB_INTERNAL _dist_code[DIST_CODE_LEN] = {
 0,  1,  2,  3,  4,  4,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8,
 8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10, 10, 10, 10, 10,
10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13,
13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15,
15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  0,  0, 16, 17,
18, 18, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22,
23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27,
27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29
};

const uch ZLIB_INTERNAL _length_code[MAX_MATCH-MIN_MATCH+1]= {
 0,  1,  2,  3,  4,  5,  6,  7,  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 12, 12,
13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16,
17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19,
19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22,
22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23,
23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26,
26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 28
};

local const int base_length[LENGTH_CODES] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 32, 40, 48, 56,
64, 80, 96, 112, 128, 160, 192, 224, 0
};

local const int base_dist[D_CODES] = {
    0,     1,     2,     3,     4,     6,     8,    12,    16,    24,
   32,    48,    64,    96,   128,   192,   256,   384,   512,   768,
 1024,  1536,  2048,  3072,  4096,  6144,  8192, 12288, 16384, 24576
};


#endif /* GEN_TREES_H */

struct static_tree_desc_s {
    const ct_data *static_tree;  /* static tree or NULL */
    const intf *extra_bits;      /* extra bits for each code or NULL */
    int     extra_base;          /* base index for extra_bits */
    int     elems;               /* max number of elements in the tree */
    int     max_length;          /* max bit length for the codes */
};

#ifdef NO_INIT_GLOBAL_POINTERS
#  define TCONST
#else
#  define TCONST const
#endif

local TCONST static_tree_desc static_l_desc =
{static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_BITS};

local TCONST static_tree_desc static_d_desc =
{static_dtree, extra_dbits, 0,          D_CODES, MAX_BITS};

local TCONST static_tree_desc static_bl_desc =
{(const ct_data *)0, extra_blbits, 0,   BL_CODES, MAX_BL_BITS};

/* ===========================================================================
 * Output a short LSB first on the stream.
 * IN assertion: there is enough room in pendingBuf.
 */
#define put_short(s, w) { \
    put_byte(s, (uch)((w) & 0xff)); \
    put_byte(s, (uch)((ush)(w) >> 8)); \
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
local unsigned bi_reverse(unsigned code, int len) {
    register unsigned res = 0;
    do {
        res |= code & 1;
        code >>= 1, res <<= 1;
    } while (--len > 0);
    return res >> 1;
}

/* ===========================================================================
 * Flush the bit buffer, keeping at most 7 bits in it.
 */
local void bi_flush(deflate_state *s) {
    if (s->bi_valid == 16) {
        put_short(s, s->bi_buf);
        s->bi_buf = 0;
        s->bi_valid = 0;
    } else if (s->bi_valid >= 8) {
        put_byte(s, (Byte)s->bi_buf);
        s->bi_buf >>= 8;
        s->bi_valid -= 8;
    }
}

/* ===========================================================================
 * Flush the bit buffer and align the output on a byte boundary
 */
local void bi_windup(deflate_state *s) {
    if (s->bi_valid > 8) {
        put_short(s, s->bi_buf);
    } else if (s->bi_valid > 0) {
        put_byte(s, (Byte)s->bi_buf);
    }
    s->bi_buf = 0;
    s->bi_valid = 0;
#ifdef ZLIB_DEBUG
    s->bits_sent = (s->bits_sent + 7) & ~7;
#endif
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
local void gen_codes(ct_data *tree, int max_code, ushf *bl_count) {
    ush next_code[MAX_BITS+1]; /* next code value for each bit length */
    unsigned code = 0;         /* running code value */
    int bits;                  /* bit index */
    int n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    for (bits = 1; bits <= MAX_BITS; bits++) {
        code = (code + bl_count[bits - 1]) << 1;
        next_code[bits] = (ush)code;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    Assert (code + bl_count[MAX_BITS] - 1 == (1 << MAX_BITS) - 1,
            "inconsistent bit counts");
    Tracev((stderr,"\ngen_codes: max_code %d ", max_code));

    for (n = 0;  n <= max_code; n++) {
        int len = tree[n].Len;
        if (len == 0) continue;
        /* Now reverse the bits */
        tree[n].Code = (ush)bi_reverse(next_code[len]++, len);

        Tracecv(tree != static_ltree, (stderr,"\nn %3d %c l %2d c %4x (%x) ",
            n, (isgraph(n) ? n : ' '), len, tree[n].Code, next_code[len] - 1));
    }
}

#ifdef GEN_TREES_H
local void gen_trees_header(void);
#endif

#ifndef ZLIB_DEBUG
#  define send_code(s, c, tree) send_bits(s, tree[c].Code, tree[c].Len)
   /* Send a code of the given tree. c and tree must not have side effects */

#else /* !ZLIB_DEBUG */
#  define send_code(s, c, tree) \
     { if (z_verbose>2) fprintf(stderr,"\ncd %3d ",(c)); \
       send_bits(s, tree[c].Code, tree[c].Len); }
#endif

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
#ifdef ZLIB_DEBUG
local void send_bits(deflate_state *s, int value, int length) {
    Tracevv((stderr," l %2d v %4x ", length, value));
    Assert(length > 0 && length <= 15, "invalid length");
    s->bits_sent += (ulg)length;

    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
     * (16 - bi_valid) bits from value, leaving (width - (16 - bi_valid))
     * unused bits in value.
     */
    if (s->bi_valid > (int)Buf_size - length) {
        s->bi_buf |= (ush)value << s->bi_valid;
        put_short(s, s->bi_buf);
        s->bi_buf = (ush)value >> (Buf_size - s->bi_valid);
        s->bi_valid += length - Buf_size;
    } else {
        s->bi_buf |= (ush)value << s->bi_valid;
        s->bi_valid += length;
    }
}
#else /* !ZLIB_DEBUG */

#define send_bits(s, value, length) \
{ int len = length;\
  if (s->bi_valid > (int)Buf_size - len) {\
    int val = (int)value;\
    s->bi_buf |= (ush)val << s->bi_valid;\
    put_short(s, s->bi_buf);\
    s->bi_buf = (ush)val >> (Buf_size - s->bi_valid);\
    s->bi_valid += len - Buf_size;\
  } else {\
    s->bi_buf |= (ush)(value) << s->bi_valid;\
    s->bi_valid += len;\
  }\
}
#endif /* ZLIB_DEBUG */


/* the arguments must not have side effects */

/* ===========================================================================
 * Initialize the various 'constant' tables.
 */
local void tr_static_init(void) {
#if defined(GEN_TREES_H) || !defined(STDC)
    static int static_init_done = 0;
    int n;        /* iterates over tree elements */
    int bits;     /* bit counter */
    int length;   /* length value */
    int code;     /* code value */
    int dist;     /* distance index */
    ush bl_count[MAX_BITS+1];
    /* number of codes at each bit length for an optimal tree */

    if (static_init_done) return;

    /* For some embedded targets, global variables are not initialized: */
#ifdef NO_INIT_GLOBAL_POINTERS
    static_l_desc.static_tree = static_ltree;
    static_l_desc.extra_bits = extra_lbits;
    static_d_desc.static_tree = static_dtree;
    static_d_desc.extra_bits = extra_dbits;
    static_bl_desc.extra_bits = extra_blbits;
#endif

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    length = 0;
    for (code = 0; code < LENGTH_CODES-1; code++) {
        base_length[code] = length;
        for (n = 0; n < (1 << extra_lbits[code]); n++) {
            _length_code[length++] = (uch)code;
        }
    }
    Assert (length == 256, "tr_static_init: length != 256");
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    _length_code[length - 1] = (uch)code;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    for (code = 0 ; code < 16; code++) {
        base_dist[code] = dist;
        for (n = 0; n < (1 << extra_dbits[code]); n++) {
            _dist_code[dist++] = (uch)code;
        }
    }
    Assert (dist == 256, "tr_static_init: dist != 256");
    dist >>= 7; /* from now on, all distances are divided by 128 */
    for ( ; code < D_CODES; code++) {
        base_dist[code] = dist << 7;
        for (n = 0; n < (1 << (extra_dbits[code] - 7)); n++) {
            _dist_code[256 + dist++] = (uch)code;
        }
    }
    Assert (dist == 256, "tr_static_init: 256 + dist != 512");

    /* Construct the codes of the static literal tree */
    for (bits = 0; bits <= MAX_BITS; bits++) bl_count[bits] = 0;
    n = 0;
    while (n <= 143) static_ltree[n++].Len = 8, bl_count[8]++;
    while (n <= 255) static_ltree[n++].Len = 9, bl_count[9]++;
    while (n <= 279) static_ltree[n++].Len = 7, bl_count[7]++;
    while (n <= 287) static_ltree[n++].Len = 8, bl_count[8]++;
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes((ct_data *)static_ltree, L_CODES+1, bl_count);

    /* The static distance tree is trivial: */
    for (n = 0; n < D_CODES; n++) {
        static_dtree[n].Len = 5;
        static_dtree[n].Code = bi_reverse((unsigned)n, 5);
    }
    static_init_done = 1;

#  ifdef GEN_TREES_H
    gen_trees_header();
#  endif
#endif /* defined(GEN_TREES_H) || !defined(STDC) */
}

/* ===========================================================================
 * Generate the file trees.h describing the static trees.
 */
#ifdef GEN_TREES_H
#  ifndef ZLIB_DEBUG
#    include <stdio.h>
#  endif

#  define SEPARATOR(i, last, width) \
      ((i) == (last)? "\n};\n\n" :    \
       ((i) % (width) == (width) - 1 ? ",\n" : ", "))

void gen_trees_header(void) {
    FILE *header = fopen("trees.h", "w");
    int i;

    Assert (header != NULL, "Can't open trees.h");
    fprintf(header,
            "/* header created automatically with -DGEN_TREES_H */\n\n");

    fprintf(header, "local const ct_data static_ltree[L_CODES+2] = {\n");
    for (i = 0; i < L_CODES+2; i++) {
        fprintf(header, "{{%3u},{%3u}}%s", static_ltree[i].Code,
                static_ltree[i].Len, SEPARATOR(i, L_CODES+1, 5));
    }

    fprintf(header, "local const ct_data static_dtree[D_CODES] = {\n");
    for (i = 0; i < D_CODES; i++) {
        fprintf(header, "{{%2u},{%2u}}%s", static_dtree[i].Code,
                static_dtree[i].Len, SEPARATOR(i, D_CODES-1, 5));
    }

    fprintf(header, "const uch ZLIB_INTERNAL _dist_code[DIST_CODE_LEN] = {\n");
    for (i = 0; i < DIST_CODE_LEN; i++) {
        fprintf(header, "%2u%s", _dist_code[i],
                SEPARATOR(i, DIST_CODE_LEN-1, 20));
    }

    fprintf(header,
        "const uch ZLIB_INTERNAL _length_code[MAX_MATCH-MIN_MATCH+1]= {\n");
    for (i = 0; i < MAX_MATCH-MIN_MATCH+1; i++) {
        fprintf(header, "%2u%s", _length_code[i],
                SEPARATOR(i, MAX_MATCH-MIN_MATCH, 20));
    }

    fprintf(header, "local const int base_length[LENGTH_CODES] = {\n");
    for (i = 0; i < LENGTH_CODES; i++) {
        fprintf(header, "%1u%s", base_length[i],
                SEPARATOR(i, LENGTH_CODES-1, 20));
    }

    fprintf(header, "local const int base_dist[D_CODES] = {\n");
    for (i = 0; i < D_CODES; i++) {
        fprintf(header, "%5u%s", base_dist[i],
                SEPARATOR(i, D_CODES-1, 10));
    }

    fclose(header);
}
#endif /* GEN_TREES_H */

/* ===========================================================================
 * Initialize a new block.
 */
local void init_block(deflate_state *s) {
    int n; /* iterates over tree elements */

    /* Initialize the trees. */
    for (n = 0; n < L_CODES;  n++) s->dyn_ltree[n].Freq = 0;
    for (n = 0; n < D_CODES;  n++) s->dyn_dtree[n].Freq = 0;
    for (n = 0; n < BL_CODES; n++) s->bl_tree[n].Freq = 0;

    s->dyn_ltree[END_BLOCK].Freq = 1;
    s->opt_len = s->static_len = 0L;
    s->sym_next = s->matches = 0;
}

/* ===========================================================================
 * Initialize the tree data structures for a new zlib stream.
 */
void ZLIB_INTERNAL _tr_init(deflate_state *s) {
    tr_static_init();

    s->l_desc.dyn_tree = s->dyn_ltree;
    s->l_desc.stat_desc = &static_l_desc;

    s->d_desc.dyn_tree = s->dyn_dtree;
    s->d_desc.stat_desc = &static_d_desc;

    s->bl_desc.dyn_tree = s->bl_tree;
    s->bl_desc.stat_desc = &static_bl_desc;

    s->bi_buf = 0;
    s->bi_valid = 0;
#ifdef ZLIB_DEBUG
    s->compressed_len = 0L;
    s->bits_sent = 0L;
#endif

    /* Initialize the first block of the first file: */
    init_block(s);
}

#define SMALLEST 1
/* Index within the heap array of least frequent node in the Huffman tree */


/* ===========================================================================
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#define pqremove(s, tree, top) \
{\
    top = s->heap[SMALLEST]; \
    s->heap[SMALLEST] = s->heap[s->heap_len--]; \
    pqdownheap(s, tree, SMALLEST); \
}

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#define smaller(tree, n, m, depth) \
   (tree[n].Freq < tree[m].Freq || \
   (tree[n].Freq == tree[m].Freq && depth[n] <= depth[m]))

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
local void pqdownheap(deflate_state *s, ct_data *tree, int k) {
    int v = s->heap[k];
    int j = k << 1;  /* left son of k */
    while (j <= s->heap_len) {
        /* Set j to the smallest of the two sons: */
        if (j < s->heap_len &&
            smaller(tree, s->heap[j + 1], s->heap[j], s->depth)) {
            j++;
        }
        /* Exit if v is smaller than both sons */
        if (smaller(tree, v, s->heap[j], s->depth)) break;

        /* Exchange v with the smallest son */
        s->heap[k] = s->heap[j];  k = j;

        /* And continue down the tree, setting j to the left son of k */
        j <<= 1;
    }
    s->heap[k] = v;
}

/* ===========================================================================
 * Compute the optimal bit lengths for a tree and update the total bit length
 * for the current block.
 * IN assertion: the fields freq and dad are set, heap[heap_max] and
 *    above are the tree nodes sorted by increasing frequency.
 * OUT assertions: the field len is set to the optimal bit length, the
 *     array bl_count contains the frequencies for each bit length.
 *     The length opt_len is updated; static_len is also updated if stree is
 *     not null.
 */
local void gen_bitlen(deflate_state *s, tree_desc *desc) {
    ct_data *tree        = desc->dyn_tree;
    int max_code         = desc->max_code;
    const ct_data *stree = desc->stat_desc->static_tree;
    const intf *extra    = desc->stat_desc->extra_bits;
    int base             = desc->stat_desc->extra_base;
    int max_length       = desc->stat_desc->max_length;
    int h;              /* heap index */
    int n, m;           /* iterate over the tree elements */
    int bits;           /* bit length */
    int xbits;          /* extra bits */
    ush f;              /* frequency */
    int overflow = 0;   /* number of elements with bit length too large */

    for (bits = 0; bits <= MAX_BITS; bits++) s->bl_count[bits] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    tree[s->heap[s->heap_max]].Len = 0; /* root of the heap */

    for (h = s->heap_max + 1; h < HEAP_SIZE; h++) {
        n = s->heap[h];
        bits = tree[tree[n].Dad].Len + 1;
        if (bits > max_length) bits = max_length, overflow++;
        tree[n].Len = (ush)bits;
        /* We overwrite tree[n].Dad which is no longer needed */

        if (n > max_code) continue; /* not a leaf node */

        s->bl_count[bits]++;
        xbits = 0;
        if (n >= base) xbits = extra[n - base];
        f = tree[n].Freq;
        s->opt_len += (ulg)f * (unsigned)(bits + xbits);
        if (stree) s->static_len += (ulg)f * (unsigned)(stree[n].Len + xbits);
    }
    if (overflow == 0) return;

    Tracev((stderr,"\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    do {
        bits = max_length - 1;
        while (s->bl_count[bits] == 0) bits--;
        s->bl_count[bits]--;        /* move one leaf down the tree */
        s->bl_count[bits + 1] += 2; /* move one overflow item as its brother */
        s->bl_count[max_length]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        overflow -= 2;
    } while (overflow > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    for (bits = max_length; bits != 0; bits--) {
        n = s->bl_count[bits];
        while (n != 0) {
            m = s->heap[--h];
            if (m > max_code) continue;
            if ((unsigned) tree[m].Len != (unsigned) bits) {
                Tracev((stderr,"code %d bits %d->%d\n", m, tree[m].Len, bits));
                s->opt_len += ((ulg)bits - tree[m].Len) * tree[m].Freq;
                tree[m].Len = (ush)bits;
            }
            n--;
        }
    }
}

#ifdef DUMP_BL_TREE
#  include <stdio.h>
#endif

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
local void build_tree(deflate_state *s, tree_desc *desc) {
    ct_data *tree         = desc->dyn_tree;
    const ct_data *stree  = desc->stat_desc->static_tree;
    int elems             = desc->stat_desc->elems;
    int n, m;          /* iterate over heap elements */
    int max_code = -1; /* largest code with non zero frequency */
    int node;          /* new node being created */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n + 1].
     * heap[0] is not used.
     */
    s->heap_len = 0, s->heap_max = HEAP_SIZE;

    for (n = 0; n < elems; n++) {
        if (tree[n].Freq != 0) {
            s->heap[++(s->heap_len)] = max_code = n;
            s->depth[n] = 0;
        } else {
            tree[n].Len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    while (s->heap_len < 2) {
        node = s->heap[++(s->heap_len)] = (max_code < 2 ? ++max_code : 0);
        tree[node].Freq = 1;
        s->depth[node] = 0;
        s->opt_len--; if (stree) s->static_len -= stree[node].Len;
        /* node is 0 or 1 so it does not have extra bits */
    }
    desc->max_code = max_code;

    /* The elements heap[heap_len/2 + 1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    for (n = s->heap_len/2; n >= 1; n--) pqdownheap(s, tree, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    node = elems;              /* next internal node of the tree */
    do {
        pqremove(s, tree, n);  /* n = node of least frequency */
        m = s->heap[SMALLEST]; /* m = node of next least frequency */

        s->heap[--(s->heap_max)] = n; /* keep the nodes sorted by frequency */
        s->heap[--(s->heap_max)] = m;

        /* Create a new node father of n and m */
        tree[node].Freq = tree[n].Freq + tree[m].Freq;
        s->depth[node] = (uch)((s->depth[n] >= s->depth[m] ?
                                s->depth[n] : s->depth[m]) + 1);
        tree[n].Dad = tree[m].Dad = (ush)node;
#ifdef DUMP_BL_TREE
        if (tree == s->bl_tree) {
            fprintf(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
                    node, tree[node].Freq, n, tree[n].Freq, m, tree[m].Freq);
        }
#endif
        /* and insert the new node in the heap */
        s->heap[SMALLEST] = node++;
        pqdownheap(s, tree, SMALLEST);

    } while (s->heap_len >= 2);

    s->heap[--(s->heap_max)] = s->heap[SMALLEST];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen(s, (tree_desc *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes ((ct_data *)tree, max_code, s->bl_count);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree.
 */
local void scan_tree(deflate_state *s, ct_data *tree, int max_code) {
    int n;                     /* iterates over all tree elements */
    int prevlen = -1;          /* last emitted length */
    int curlen;                /* length of current code */
    int nextlen = tree[0].Len; /* length of next code */
    int count = 0;             /* repeat count of the current code */
    int max_count = 7;         /* max repeat count */
    int min_count = 4;         /* min repeat count */

    if (nextlen == 0) max_count = 138, min_count = 3;
    tree[max_code + 1].Len = (ush)0xffff; /* guard */

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = tree[n + 1].Len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            s->bl_tree[curlen].Freq += count;
        } else if (curlen != 0) {
            if (curlen != prevlen) s->bl_tree[curlen].Freq++;
            s->bl_tree[REP_3_6].Freq++;
        } else if (count <= 10) {
            s->bl_tree[REPZ_3_10].Freq++;
        } else {
            s->bl_tree[REPZ_11_138].Freq++;
        }
        count = 0; prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
        } else {
            max_count = 7, min_count = 4;
        }
    }
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
local void send_tree(deflate_state *s, ct_data *tree, int max_code) {
    int n;                     /* iterates over all tree elements */
    int prevlen = -1;          /* last emitted length */
    int curlen;                /* length of current code */
    int nextlen = tree[0].Len; /* length of next code */
    int count = 0;             /* repeat count of the current code */
    int max_count = 7;         /* max repeat count */
    int min_count = 4;         /* min repeat count */

    /* tree[max_code + 1].Len = -1; */  /* guard already set */
    if (nextlen == 0) max_count = 138, min_count = 3;

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = tree[n + 1].Len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            do { send_code(s, curlen, s->bl_tree); } while (--count != 0);

        } else if (curlen != 0) {
            if (curlen != prevlen) {
                send_code(s, curlen, s->bl_tree); count--;
            }
            Assert(count >= 3 && count <= 6, " 3_6?");
            send_code(s, REP_3_6, s->bl_tree); send_bits(s, count - 3, 2);

        } else if (count <= 10) {
            send_code(s, REPZ_3_10, s->bl_tree); send_bits(s, count - 3, 3);

        } else {
            send_code(s, REPZ_11_138, s->bl_tree); send_bits(s, count - 11, 7);
        }
        count = 0; prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
        } else {
            max_count = 7, min_count = 4;
        }
    }
}

/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
local int build_bl_tree(deflate_state *s) {
    int max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    scan_tree(s, (ct_data *)s->dyn_ltree, s->l_desc.max_code);
    scan_tree(s, (ct_data *)s->dyn_dtree, s->d_desc.max_code);

    /* Build the bit length tree: */
    build_tree(s, (tree_desc *)(&(s->bl_desc)));
    /* opt_len now includes the length of the tree representations, except the
     * lengths of the bit lengths codes and the 5 + 5 + 4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    for (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        if (s->bl_tree[bl_order[max_blindex]].Len != 0) break;
    }
    /* Update opt_len to include the bit length tree and counts */
    s->opt_len += 3*((ulg)max_blindex + 1) + 5 + 5 + 4;
    Tracev((stderr, "\ndyn trees: dyn %ld, stat %ld",
            s->opt_len, s->static_len));

    return max_blindex;
}

/* ===========================================================================
 * Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
 */
local void send_all_trees(deflate_state *s, int lcodes, int dcodes,
                          int blcodes) {
    int rank;                    /* index in bl_order */

    Assert (lcodes >= 257 && dcodes >= 1 && blcodes >= 4, "not enough codes");
    Assert (lcodes <= L_CODES && dcodes <= D_CODES && blcodes <= BL_CODES,
            "too many codes");
    Tracev((stderr, "\nbl counts: "));
    send_bits(s, lcodes - 257, 5);  /* not +255 as stated in appnote.txt */
    send_bits(s, dcodes - 1,   5);
    send_bits(s, blcodes - 4,  4);  /* not -3 as stated in appnote.txt */
    for (rank = 0; rank < blcodes; rank++) {
        Tracev((stderr, "\nbl code %2d ", bl_order[rank]));
        send_bits(s, s->bl_tree[bl_order[rank]].Len, 3);
    }
    Tracev((stderr, "\nbl tree: sent %ld", s->bits_sent));

    send_tree(s, (ct_data *)s->dyn_ltree, lcodes - 1);  /* literal tree */
    Tracev((stderr, "\nlit tree: sent %ld", s->bits_sent));

    send_tree(s, (ct_data *)s->dyn_dtree, dcodes - 1);  /* distance tree */
    Tracev((stderr, "\ndist tree: sent %ld", s->bits_sent));
}

/* ===========================================================================
 * Send a stored block
 */
void ZLIB_INTERNAL _tr_stored_block(deflate_state *s, charf *buf,
                                    ulg stored_len, int last) {
    send_bits(s, (STORED_BLOCK<<1) + last, 3);  /* send block type */
    bi_windup(s);        /* align on byte boundary */
    put_short(s, (ush)stored_len);
    put_short(s, (ush)~stored_len);
    if (stored_len)
        zmemcpy(s->pending_buf + s->pending, (Bytef *)buf, stored_len);
    s->pending += stored_len;
#ifdef ZLIB_DEBUG
    s->compressed_len = (s->compressed_len + 3 + 7) & (ulg)~7L;
    s->compressed_len += (stored_len + 4) << 3;
    s->bits_sent += 2*16;
    s->bits_sent += stored_len << 3;
#endif
}

/* ===========================================================================
 * Flush the bits in the bit buffer to pending output (leaves at most 7 bits)
 */
void ZLIB_INTERNAL _tr_flush_bits(deflate_state *s) {
    bi_flush(s);
}

/* ===========================================================================
 * Send one empty static block to give enough lookahead for inflate.
 * This takes 10 bits, of which 7 may remain in the bit buffer.
 */
void ZLIB_INTERNAL _tr_align(deflate_state *s) {
    send_bits(s, STATIC_TREES<<1, 3);
    send_code(s, END_BLOCK, static_ltree);
#ifdef ZLIB_DEBUG
    s->compressed_len += 10L; /* 3 for block type, 7 for EOB */
#endif
    bi_flush(s);
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
local void compress_block(deflate_state *s, const ct_data *ltree,
                          const ct_data *dtree) {
    unsigned dist;      /* distance of matched string */
    int lc;             /* match length or unmatched char (if dist == 0) */
    unsigned sx = 0;    /* running index in symbol buffers */
    unsigned code;      /* the code to send */
    int extra;          /* number of extra bits to send */

    if (s->sym_next != 0) do {
#ifdef LIT_MEM
        dist = s->d_buf[sx];
        lc = s->l_buf[sx++];
#else
        dist = s->sym_buf[sx++] & 0xff;
        dist += (unsigned)(s->sym_buf[sx++] & 0xff) << 8;
        lc = s->sym_buf[sx++];
#endif
        if (dist == 0) {
            send_code(s, lc, ltree); /* send a literal byte */
            Tracecv(isgraph(lc), (stderr," '%c' ", lc));
        } else {
            /* Here, lc is the match length - MIN_MATCH */
            code = _length_code[lc];
            send_code(s, code + LITERALS + 1, ltree);   /* send length code */
            extra = extra_lbits[code];
            if (extra != 0) {
                lc -= base_length[code];
                send_bits(s, lc, extra);       /* send the extra length bits */
            }
            dist--; /* dist is now the match distance - 1 */
            code = d_code(dist);
            Assert (code < D_CODES, "bad d_code");

            send_code(s, code, dtree);       /* send the distance code */
            extra = extra_dbits[code];
            if (extra != 0) {
                dist -= (unsigned)base_dist[code];
                send_bits(s, dist, extra);   /* send the extra distance bits */
            }
        } /* literal or match pair ? */

        /* Check for no overlay of pending_buf on needed symbols */
#ifdef LIT_MEM
        Assert(s->pending < (s->lit_bufsize << 1) + sx, "pendingBuf overflow");
#else
        Assert(s->pending < s->lit_bufsize + sx, "pendingBuf overflow");
#endif

    } while (sx < s->sym_next);

    send_code(s, END_BLOCK, ltree);
}

/* ===========================================================================
 * Check if the data type is TEXT or BINARY, using the following algorithm:
 * - TEXT if the two conditions below are satisfied:
 *    a) There are no non-portable control characters belonging to the
 *       "block list" (0..6, 14..25, 28..31).
 *    b) There is at least one printable character belonging to the
 *       "allow list" (9 {TAB}, 10 {LF}, 13 {CR}, 32..255).
 * - BINARY otherwise.
 * - The following partially-portable control characters form a
 *   "gray list" that is ignored in this detection algorithm:
 *   (7 {BEL}, 8 {BS}, 11 {VT}, 12 {FF}, 26 {SUB}, 27 {ESC}).
 * IN assertion: the fields Freq of dyn_ltree are set.
 */
local int detect_data_type(deflate_state *s) {
    /* block_mask is the bit mask of block-listed bytes
     * set bits 0..6, 14..25, and 28..31
     * 0xf3ffc07f = binary 11110011111111111100000001111111
     */
    unsigned long block_mask = 0xf3ffc07fUL;
    int n;

    /* Check for non-textual ("block-listed") bytes. */
    for (n = 0; n <= 31; n++, block_mask >>= 1)
        if ((block_mask & 1) && (s->dyn_ltree[n].Freq != 0))
            return Z_BINARY;

    /* Check for textual ("allow-listed") bytes. */
    if (s->dyn_ltree[9].Freq != 0 || s->dyn_ltree[10].Freq != 0
            || s->dyn_ltree[13].Freq != 0)
        return Z_TEXT;
    for (n = 32; n < LITERALS; n++)
        if (s->dyn_ltree[n].Freq != 0)
            return Z_TEXT;

    /* There are no "block-listed" or "allow-listed" bytes:
     * this stream either is empty or has tolerated ("gray-listed") bytes only.
     */
    return Z_BINARY;
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and write out the encoded block.
 */
void ZLIB_INTERNAL _tr_flush_block(deflate_state *s, charf *buf,
                                   ulg stored_len, int last) {
    ulg opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    int max_blindex = 0;  /* index of last bit length code of non zero freq */

    /* Build the Huffman trees unless a stored block is forced */
    if (s->level > 0) {

        /* Check if the file is binary or text */
        if (s->strm->data_type == Z_UNKNOWN)
            s->strm->data_type = detect_data_type(s);

        /* Construct the literal and distance trees */
        build_tree(s, (tree_desc *)(&(s->l_desc)));
        Tracev((stderr, "\nlit data: dyn %ld, stat %ld", s->opt_len,
                s->static_len));

        build_tree(s, (tree_desc *)(&(s->d_desc)));
        Tracev((stderr, "\ndist data: dyn %ld, stat %ld", s->opt_len,
                s->static_len));
        /* At this point, opt_len and static_len are the total bit lengths of
         * the compressed block data, excluding the tree representations.
         */

        /* Build the bit length tree for the above two trees, and get the index
         * in bl_order of the last bit length code to send.
         */
        max_blindex = build_bl_tree(s);

        /* Determine the best encoding. Compute the block lengths in bytes. */
        opt_lenb = (s->opt_len + 3 + 7) >> 3;
        static_lenb = (s->static_len + 3 + 7) >> 3;

        Tracev((stderr, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u ",
                opt_lenb, s->opt_len, static_lenb, s->static_len, stored_len,
                s->sym_next / 3));

#ifndef FORCE_STATIC
        if (static_lenb <= opt_lenb || s->strategy == Z_FIXED)
#endif
            opt_lenb = static_lenb;

    } else {
        Assert(buf != (char*)0, "lost buf");
        opt_lenb = static_lenb = stored_len + 5; /* force a stored block */
    }

#ifdef FORCE_STORED
    if (buf != (char*)0) { /* force stored block */
#else
    if (stored_len + 4 <= opt_lenb && buf != (char*)0) {
                       /* 4: two words for the lengths */
#endif
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        _tr_stored_block(s, buf, stored_len, last);

    } else if (static_lenb == opt_lenb) {
        send_bits(s, (STATIC_TREES<<1) + last, 3);
        compress_block(s, (const ct_data *)static_ltree,
                       (const ct_data *)static_dtree);
#ifdef ZLIB_DEBUG
        s->compressed_len += 3 + s->static_len;
#endif
    } else {
        send_bits(s, (DYN_TREES<<1) + last, 3);
        send_all_trees(s, s->l_desc.max_code + 1, s->d_desc.max_code + 1,
                       max_blindex + 1);
        compress_block(s, (const ct_data *)s->dyn_ltree,
                       (const ct_data *)s->dyn_dtree);
#ifdef ZLIB_DEBUG
        s->compressed_len += 3 + s->opt_len;
#endif
    }
    Assert (s->compressed_len == s->bits_sent, "bad compressed size");
    /* The above check is made mod 2^32, for files larger than 512 MB
     * and uLong implemented on 32 bits.
     */
    init_block(s);

    if (last) {
        bi_windup(s);
#ifdef ZLIB_DEBUG
        s->compressed_len += 7;  /* align on byte boundary */
#endif
    }
    Tracev((stderr,"\ncomprlen %lu(%lu) ", s->compressed_len >> 3,
           s->compressed_len - 7*last));
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 */
int ZLIB_INTERNAL _tr_tally(deflate_state *s, unsigned dist, unsigned lc) {
#ifdef LIT_MEM
    s->d_buf[s->sym_next] = (ush)dist;
    s->l_buf[s->sym_next++] = (uch)lc;
#else
    s->sym_buf[s->sym_next++] = (uch)dist;
    s->sym_buf[s->sym_next++] = (uch)(dist >> 8);
    s->sym_buf[s->sym_next++] = (uch)lc;
#endif
    if (dist == 0) {
        /* lc is the unmatched char */
        s->dyn_ltree[lc].Freq++;
    } else {
        s->matches++;
        /* Here, lc is the match length - MIN_MATCH */
        dist--;             /* dist = match distance - 1 */
        Assert((ush)dist < (ush)MAX_DIST(s) &&
               (ush)lc <= (ush)(MAX_MATCH-MIN_MATCH) &&
               (ush)d_code(dist) < (ush)D_CODES,  "_tr_tally: bad match");

        s->dyn_ltree[_length_code[lc] + LITERALS + 1].Freq++;
        s->dyn_dtree[d_code(dist)].Freq++;
    }
    return (s->sym_next == s->sym_end);
}
/* inftrees.h -- header to use inftrees.c
 * Copyright (C) 1995-2005, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* Structure for decoding tables.  Each entry provides either the
   information needed to do the operation requested by the code that
   indexed that table entry, or it provides a pointer to another
   table that indexes more bits of the code.  op indicates whether
   the entry is a pointer to another table, a literal, a length or
   distance, an end-of-block, or an invalid code.  For a table
   pointer, the low four bits of op is the number of index bits of
   that table.  For a length or distance, the low four bits of op
   is the number of extra bits to get after the code.  bits is
   the number of bits in this code or part of the code to drop off
   of the bit buffer.  val is the actual byte to output in the case
   of a literal, the base length or distance, or the offset from
   the current table to the next table.  Each entry is four bytes. */
typedef struct {
    unsigned char op;           /* operation, extra bits, table bits */
    unsigned char bits;         /* bits in this part of the code */
    unsigned short val;         /* offset in table or code value */
} code;

/* op values as set by inflate_table():
    00000000 - literal
    0000tttt - table link, tttt != 0 is the number of table index bits
    0001eeee - length or distance, eeee is the number of extra bits
    01100000 - end of block
    01000000 - invalid code
 */

/* Maximum size of the dynamic table.  The maximum number of code structures is
   1444, which is the sum of 852 for literal/length codes and 592 for distance
   codes.  These values were found by exhaustive searches using the program
   examples/enough.c found in the zlib distribution.  The arguments to that
   program are the number of symbols, the initial root table size, and the
   maximum bit length of a code.  "enough 286 9 15" for literal/length codes
   returns 852, and "enough 30 6 15" for distance codes returns 592. The
   initial root table size (9 or 6) is found in the fifth argument of the
   inflate_table() calls in inflate.c and infback.c.  If the root table size is
   changed, then these maximum sizes would be need to be recalculated and
   updated. */
#define ENOUGH_LENS 852
#define ENOUGH_DISTS 592
#define ENOUGH (ENOUGH_LENS+ENOUGH_DISTS)

/* Type of code to build for inflate_table() */
typedef enum {
    CODES,
    LENS,
    DISTS
} codetype;

int ZLIB_INTERNAL inflate_table(codetype type, unsigned short FAR *lens,
                                unsigned codes, code FAR * FAR *table,
                                unsigned FAR *bits, unsigned short FAR *work);
/* inftrees.c -- generate Huffman trees for efficient decoding
 * Copyright (C) 1995-2023 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

//REBOL: #include "zutil.h"
//REBOL: #include "inftrees.h"

#define MAXBITS 15

const char inflate_copyright[] =
   " inflate 1.3.0.1 Copyright 1995-2023 Mark Adler ";
/*
  If you use the zlib library in a product, an acknowledgment is welcome
  in the documentation of your product. If for some reason you cannot
  include such an acknowledgment, I would appreciate that you keep this
  copyright string in the executable of your product.
 */

/*
   Build a set of tables to decode the provided canonical Huffman code.
   The code lengths are lens[0..codes-1].  The result starts at *table,
   whose indices are 0..2^bits-1.  work is a writable array of at least
   lens shorts, which is used as a work area.  type is the type of code
   to be generated, CODES, LENS, or DISTS.  On return, zero is success,
   -1 is an invalid code, and +1 means that ENOUGH isn't enough.  table
   on return points to the next available entry's address.  bits is the
   requested root table index bits, and on return it is the actual root
   table index bits.  It will differ if the request is greater than the
   longest code or if it is less than the shortest code.
 */
int ZLIB_INTERNAL inflate_table(codetype type, unsigned short FAR *lens,
                                unsigned codes, code FAR * FAR *table,
                                unsigned FAR *bits, unsigned short FAR *work) {
    unsigned len;               /* a code's length in bits */
    unsigned sym;               /* index of code symbols */
    unsigned min, max;          /* minimum and maximum code lengths */
    unsigned root;              /* number of index bits for root table */
    unsigned curr;              /* number of index bits for current table */
    unsigned drop;              /* code bits to drop for sub-table */
    int left;                   /* number of prefix codes available */
    unsigned used;              /* code entries in table used */
    unsigned huff;              /* Huffman code */
    unsigned incr;              /* for incrementing code, index */
    unsigned fill;              /* index for replicating entries */
    unsigned low;               /* low bits for current root entry */
    unsigned mask;              /* mask for low root bits */
    code here;                  /* table entry for duplication */
    code FAR *next;             /* next available space in table */
    const unsigned short FAR *base;     /* base value table to use */
    const unsigned short FAR *extra;    /* extra bits table to use */
    unsigned match;             /* use base and extra for symbol >= match */
    unsigned short count[MAXBITS+1];    /* number of codes of each length */
    unsigned short offs[MAXBITS+1];     /* offsets in table for each length */
    static const unsigned short lbase[31] = { /* Length codes 257..285 base */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
    static const unsigned short lext[31] = { /* Length codes 257..285 extra */
        16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 18,
        19, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21, 16, 70, 200};
    static const unsigned short dbase[32] = { /* Distance codes 0..29 base */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577, 0, 0};
    static const unsigned short dext[32] = { /* Distance codes 0..29 extra */
        16, 16, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22,
        23, 23, 24, 24, 25, 25, 26, 26, 27, 27,
        28, 28, 29, 29, 64, 64};

    /*
       Process a set of code lengths to create a canonical Huffman code.  The
       code lengths are lens[0..codes-1].  Each length corresponds to the
       symbols 0..codes-1.  The Huffman code is generated by first sorting the
       symbols by length from short to long, and retaining the symbol order
       for codes with equal lengths.  Then the code starts with all zero bits
       for the first code of the shortest length, and the codes are integer
       increments for the same length, and zeros are appended as the length
       increases.  For the deflate format, these bits are stored backwards
       from their more natural integer increment ordering, and so when the
       decoding tables are built in the large loop below, the integer codes
       are incremented backwards.

       This routine assumes, but does not check, that all of the entries in
       lens[] are in the range 0..MAXBITS.  The caller must assure this.
       1..MAXBITS is interpreted as that code length.  zero means that that
       symbol does not occur in this code.

       The codes are sorted by computing a count of codes for each length,
       creating from that a table of starting indices for each length in the
       sorted table, and then entering the symbols in order in the sorted
       table.  The sorted table is work[], with that space being provided by
       the caller.

       The length counts are used for other purposes as well, i.e. finding
       the minimum and maximum length codes, determining if there are any
       codes at all, checking for a valid set of lengths, and looking ahead
       at length counts to determine sub-table sizes when building the
       decoding tables.
     */

    /* accumulate lengths for codes (assumes lens[] all in 0..MAXBITS) */
    for (len = 0; len <= MAXBITS; len++)
        count[len] = 0;
    for (sym = 0; sym < codes; sym++)
        count[lens[sym]]++;

    /* bound code lengths, force root to be within code lengths */
    root = *bits;
    for (max = MAXBITS; max >= 1; max--)
        if (count[max] != 0) break;
    if (root > max) root = max;
    if (max == 0) {                     /* no symbols to code at all */
        here.op = (unsigned char)64;    /* invalid code marker */
        here.bits = (unsigned char)1;
        here.val = (unsigned short)0;
        *(*table)++ = here;             /* make a table to force an error */
        *(*table)++ = here;
        *bits = 1;
        return 0;     /* no symbols, but wait for decoding to report error */
    }
    for (min = 1; min < max; min++)
        if (count[min] != 0) break;
    if (root < min) root = min;

    /* check for an over-subscribed or incomplete set of lengths */
    left = 1;
    for (len = 1; len <= MAXBITS; len++) {
        left <<= 1;
        left -= count[len];
        if (left < 0) return -1;        /* over-subscribed */
    }
    if (left > 0 && (type == CODES || max != 1))
        return -1;                      /* incomplete set */

    /* generate offsets into symbol table for each length for sorting */
    offs[1] = 0;
    for (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + count[len];

    /* sort symbols by length, by symbol order within each length */
    for (sym = 0; sym < codes; sym++)
        if (lens[sym] != 0) work[offs[lens[sym]]++] = (unsigned short)sym;

    /*
       Create and fill in decoding tables.  In this loop, the table being
       filled is at next and has curr index bits.  The code being used is huff
       with length len.  That code is converted to an index by dropping drop
       bits off of the bottom.  For codes where len is less than drop + curr,
       those top drop + curr - len bits are incremented through all values to
       fill the table with replicated entries.

       root is the number of index bits for the root table.  When len exceeds
       root, sub-tables are created pointed to by the root entry with an index
       of the low root bits of huff.  This is saved in low to check for when a
       new sub-table should be started.  drop is zero when the root table is
       being filled, and drop is root when sub-tables are being filled.

       When a new sub-table is needed, it is necessary to look ahead in the
       code lengths to determine what size sub-table is needed.  The length
       counts are used for this, and so count[] is decremented as codes are
       entered in the tables.

       used keeps track of how many table entries have been allocated from the
       provided *table space.  It is checked for LENS and DIST tables against
       the constants ENOUGH_LENS and ENOUGH_DISTS to guard against changes in
       the initial root table size constants.  See the comments in inftrees.h
       for more information.

       sym increments through all symbols, and the loop terminates when
       all codes of length max, i.e. all codes, have been processed.  This
       routine permits incomplete codes, so another loop after this one fills
       in the rest of the decoding tables with invalid code markers.
     */

    /* set up for code type */
    switch (type) {
    case CODES:
        base = extra = work;    /* dummy value--not used */
        match = 20;
        break;
    case LENS:
        base = lbase;
        extra = lext;
        match = 257;
        break;
    default:    /* DISTS */
        base = dbase;
        extra = dext;
        match = 0;
    }

    /* initialize state for loop */
    huff = 0;                   /* starting code */
    sym = 0;                    /* starting code symbol */
    len = min;                  /* starting code length */
    next = *table;              /* current table to fill in */
    curr = root;                /* current table index bits */
    drop = 0;                   /* current bits to drop from code for index */
    low = (unsigned)(-1);       /* trigger new sub-table when len > root */
    used = 1U << root;          /* use root table entries */
    mask = used - 1;            /* mask for comparing low */

    /* check available table space */
    if ((type == LENS && used > ENOUGH_LENS) ||
        (type == DISTS && used > ENOUGH_DISTS))
        return 1;

    /* process all codes and make table entries */
    for (;;) {
        /* create table entry */
        here.bits = (unsigned char)(len - drop);
        if (work[sym] + 1U < match) {
            here.op = (unsigned char)0;
            here.val = work[sym];
        }
        else if (work[sym] >= match) {
            here.op = (unsigned char)(extra[work[sym] - match]);
            here.val = base[work[sym] - match];
        }
        else {
            here.op = (unsigned char)(32 + 64);         /* end of block */
            here.val = 0;
        }

        /* replicate for those indices with low len bits equal to huff */
        incr = 1U << (len - drop);
        fill = 1U << curr;
        min = fill;                 /* save offset to next table */
        do {
            fill -= incr;
            next[(huff >> drop) + fill] = here;
        } while (fill != 0);

        /* backwards increment the len-bit code huff */
        incr = 1U << (len - 1);
        while (huff & incr)
            incr >>= 1;
        if (incr != 0) {
            huff &= incr - 1;
            huff += incr;
        }
        else
            huff = 0;

        /* go to next symbol, update count, len */
        sym++;
        if (--(count[len]) == 0) {
            if (len == max) break;
            len = lens[work[sym]];
        }

        /* create new sub-table if needed */
        if (len > root && (huff & mask) != low) {
            /* if first time, transition to sub-tables */
            if (drop == 0)
                drop = root;

            /* increment past last table */
            next += min;            /* here min is 1 << curr */

            /* determine length of next table */
            curr = len - drop;
            left = (int)(1 << curr);
            while (curr + drop < max) {
                left -= count[curr + drop];
                if (left <= 0) break;
                curr++;
                left <<= 1;
            }

            /* check for enough space */
            used += 1U << curr;
            if ((type == LENS && used > ENOUGH_LENS) ||
                (type == DISTS && used > ENOUGH_DISTS))
                return 1;

            /* point entry in root table to sub-table */
            low = huff & mask;
            (*table)[low].op = (unsigned char)curr;
            (*table)[low].bits = (unsigned char)root;
            (*table)[low].val = (unsigned short)(next - *table);
        }
    }

    /* fill in remaining table entry if code is incomplete (guaranteed to have
       at most one remaining entry, since if the code is incomplete, the
       maximum code length that was allowed to get this far is one bit) */
    if (huff != 0) {
        here.op = (unsigned char)64;            /* invalid code marker */
        here.bits = (unsigned char)(len - drop);
        here.val = (unsigned short)0;
        next[huff] = here;
    }

    /* set return parameters */
    *table += used;
    *bits = root;
    return 0;
}
/* inffast.h -- header to use inffast.c
 * Copyright (C) 1995-2003, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

void ZLIB_INTERNAL inflate_fast(z_streamp strm, unsigned start);
/* inflate.h -- internal inflate state definition
 * Copyright (C) 1995-2019 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* define NO_GZIP when compiling if you want to disable gzip header and
   trailer decoding by inflate().  NO_GZIP would be used to avoid linking in
   the crc code when it is not needed.  For shared libraries, gzip decoding
   should be left enabled. */
#ifndef NO_GZIP
#  define GUNZIP
#endif

/* Possible inflate modes between inflate() calls */
typedef enum {
    HEAD = 16180,   /* i: waiting for magic header */
    FLAGS,      /* i: waiting for method and flags (gzip) */
    TIME,       /* i: waiting for modification time (gzip) */
    OS,         /* i: waiting for extra flags and operating system (gzip) */
    EXLEN,      /* i: waiting for extra length (gzip) */
    EXTRA,      /* i: waiting for extra bytes (gzip) */
    NAME,       /* i: waiting for end of file name (gzip) */
    COMMENT,    /* i: waiting for end of comment (gzip) */
    HCRC,       /* i: waiting for header crc (gzip) */
    DICTID,     /* i: waiting for dictionary check value */
    DICT,       /* waiting for inflateSetDictionary() call */
        TYPE,       /* i: waiting for type bits, including last-flag bit */
        TYPEDO,     /* i: same, but skip check to exit inflate on new block */
        STORED,     /* i: waiting for stored size (length and complement) */
        COPY_,      /* i/o: same as COPY below, but only first time in */
        COPY,       /* i/o: waiting for input or output to copy stored block */
        TABLE,      /* i: waiting for dynamic block table lengths */
        LENLENS,    /* i: waiting for code length code lengths */
        CODELENS,   /* i: waiting for length/lit and distance code lengths */
            LEN_,       /* i: same as LEN below, but only first time in */
            LEN,        /* i: waiting for length/lit/eob code */
            LENEXT,     /* i: waiting for length extra bits */
            DIST,       /* i: waiting for distance code */
            DISTEXT,    /* i: waiting for distance extra bits */
            MATCH,      /* o: waiting for output space to copy string */
            LIT,        /* o: waiting for output space to write literal */
    CHECK,      /* i: waiting for 32-bit check value */
    LENGTH,     /* i: waiting for 32-bit length (gzip) */
    DONE,       /* finished check, done -- remain here until reset */
    BAD,        /* got a data error -- remain here until reset */
    MEM,        /* got an inflate() memory error -- remain here until reset */
    SYNC        /* looking for synchronization bytes to restart inflate() */
} inflate_mode;

/*
    State transitions between above modes -

    (most modes can go to BAD or MEM on error -- not shown for clarity)

    Process header:
        HEAD -> (gzip) or (zlib) or (raw)
        (gzip) -> FLAGS -> TIME -> OS -> EXLEN -> EXTRA -> NAME -> COMMENT ->
                  HCRC -> TYPE
        (zlib) -> DICTID or TYPE
        DICTID -> DICT -> TYPE
        (raw) -> TYPEDO
    Read deflate blocks:
            TYPE -> TYPEDO -> STORED or TABLE or LEN_ or CHECK
            STORED -> COPY_ -> COPY -> TYPE
            TABLE -> LENLENS -> CODELENS -> LEN_
            LEN_ -> LEN
    Read deflate codes in fixed or dynamic block:
                LEN -> LENEXT or LIT or TYPE
                LENEXT -> DIST -> DISTEXT -> MATCH -> LEN
                LIT -> LEN
    Process trailer:
        CHECK -> LENGTH -> DONE
 */

/* State maintained between inflate() calls -- approximately 7K bytes, not
   including the allocated sliding window, which is up to 32K bytes. */
struct inflate_state {
    z_streamp strm;             /* pointer back to this zlib stream */
    inflate_mode mode;          /* current inflate mode */
    int last;                   /* true if processing last block */
    int wrap;                   /* bit 0 true for zlib, bit 1 true for gzip,
                                   bit 2 true to validate check value */
    int havedict;               /* true if dictionary provided */
    int flags;                  /* gzip header method and flags, 0 if zlib, or
                                   -1 if raw or no header yet */
    unsigned dmax;              /* zlib header max distance (INFLATE_STRICT) */
    unsigned long check;        /* protected copy of check value */
    unsigned long total;        /* protected copy of output count */
    gz_headerp head;            /* where to save gzip header information */
        /* sliding window */
    unsigned wbits;             /* log base 2 of requested window size */
    unsigned wsize;             /* window size or zero if not using window */
    unsigned whave;             /* valid bytes in the window */
    unsigned wnext;             /* window write index */
    unsigned char FAR *window;  /* allocated sliding window, if needed */
        /* bit accumulator */
    unsigned long hold;         /* input bit accumulator */
    unsigned bits;              /* number of bits in "in" */
        /* for string and stored block copying */
    unsigned length;            /* literal or length of data to copy */
    unsigned offset;            /* distance back to copy string from */
        /* for table and code decoding */
    unsigned extra;             /* extra bits needed */
        /* fixed and dynamic code tables */
    code const FAR *lencode;    /* starting table for length/literal codes */
    code const FAR *distcode;   /* starting table for distance codes */
    unsigned lenbits;           /* index bits for lencode */
    unsigned distbits;          /* index bits for distcode */
        /* dynamic table building */
    unsigned ncode;             /* number of code length code lengths */
    unsigned nlen;              /* number of length code lengths */
    unsigned ndist;             /* number of distance code lengths */
    unsigned have;              /* number of code lengths in lens[] */
    code FAR *next;             /* next available space in codes[] */
    unsigned short lens[320];   /* temporary storage for code lengths */
    unsigned short work[288];   /* work area for code table building */
    code codes[ENOUGH];         /* space for code tables */
    int sane;                   /* if false, allow invalid distance too far */
    int back;                   /* bits back of last unprocessed length/lit */
    unsigned was;               /* initial length of match */
};
/* inffast.c -- fast decoding
 * Copyright (C) 1995-2017 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

//REBOL: #include "zutil.h"
//REBOL: #include "inftrees.h"
//REBOL: #include "inflate.h"
//REBOL: #include "inffast.h"

#ifdef ASMINF
#  pragma message("Assembler code may have bugs -- use at your own risk")
#else

/*
   Decode literal, length, and distance codes and write out the resulting
   literal and match bytes until either not enough input or output is
   available, an end-of-block is encountered, or a data error is encountered.
   When large enough input and output buffers are supplied to inflate(), for
   example, a 16K input buffer and a 64K output buffer, more than 95% of the
   inflate execution time is spent in this routine.

   Entry assumptions:

        state->mode == LEN
        strm->avail_in >= 6
        strm->avail_out >= 258
        start >= strm->avail_out
        state->bits < 8

   On return, state->mode is one of:

        LEN -- ran out of enough output space or enough available input
        TYPE -- reached end of block code, inflate() to interpret next block
        BAD -- error in block data

   Notes:

    - The maximum input bits used by a length/distance pair is 15 bits for the
      length code, 5 bits for the length extra, 15 bits for the distance code,
      and 13 bits for the distance extra.  This totals 48 bits, or six bytes.
      Therefore if strm->avail_in >= 6, then there is enough input to avoid
      checking for available input while decoding.

    - The maximum bytes that a single length/distance pair can output is 258
      bytes, which is the maximum length that can be coded.  inflate_fast()
      requires strm->avail_out >= 258 for each loop to avoid checking for
      output space.
 */
void ZLIB_INTERNAL inflate_fast(z_streamp strm, unsigned start) {
    struct inflate_state FAR *state;
    z_const unsigned char FAR *in;      /* local strm->next_in */
    z_const unsigned char FAR *last;    /* have enough input while in < last */
    unsigned char FAR *out;     /* local strm->next_out */
    unsigned char FAR *beg;     /* inflate()'s initial strm->next_out */
    unsigned char FAR *end;     /* while out < end, enough space available */
#ifdef INFLATE_STRICT
    unsigned dmax;              /* maximum distance from zlib header */
#endif
    unsigned wsize;             /* window size or zero if not using window */
    unsigned whave;             /* valid bytes in the window */
    unsigned wnext;             /* window write index */
    unsigned char FAR *window;  /* allocated sliding window, if wsize != 0 */
    unsigned long hold;         /* local strm->hold */
    unsigned bits;              /* local strm->bits */
    code const FAR *lcode;      /* local strm->lencode */
    code const FAR *dcode;      /* local strm->distcode */
    unsigned lmask;             /* mask for first level of length codes */
    unsigned dmask;             /* mask for first level of distance codes */
    code const *here;           /* retrieved table entry */
    unsigned op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    unsigned len;               /* match length, unused bytes */
    unsigned dist;              /* match distance */
    unsigned char FAR *from;    /* where to copy match from */

    /* copy state to local variables */
    state = (struct inflate_state FAR *)strm->state;
    in = strm->next_in;
    last = in + (strm->avail_in - 5);
    out = strm->next_out;
    beg = out - (start - strm->avail_out);
    end = out + (strm->avail_out - 257);
#ifdef INFLATE_STRICT
    dmax = state->dmax;
#endif
    wsize = state->wsize;
    whave = state->whave;
    wnext = state->wnext;
    window = state->window;
    hold = state->hold;
    bits = state->bits;
    lcode = state->lencode;
    dcode = state->distcode;
    lmask = (1U << state->lenbits) - 1;
    dmask = (1U << state->distbits) - 1;

    /* decode literals and length/distances until end-of-block or not enough
       input data or output space */
    do {
        if (bits < 15) {
            hold += (unsigned long)(*in++) << bits;
            bits += 8;
            hold += (unsigned long)(*in++) << bits;
            bits += 8;
        }
        here = lcode + (hold & lmask);
      dolen:
        op = (unsigned)(here->bits);
        hold >>= op;
        bits -= op;
        op = (unsigned)(here->op);
        if (op == 0) {                          /* literal */
            Tracevv((stderr, here->val >= 0x20 && here->val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", here->val));
            *out++ = (unsigned char)(here->val);
        }
        else if (op & 16) {                     /* length base */
            len = (unsigned)(here->val);
            op &= 15;                           /* number of extra bits */
            if (op) {
                if (bits < op) {
                    hold += (unsigned long)(*in++) << bits;
                    bits += 8;
                }
                len += (unsigned)hold & ((1U << op) - 1);
                hold >>= op;
                bits -= op;
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            if (bits < 15) {
                hold += (unsigned long)(*in++) << bits;
                bits += 8;
                hold += (unsigned long)(*in++) << bits;
                bits += 8;
            }
            here = dcode + (hold & dmask);
          dodist:
            op = (unsigned)(here->bits);
            hold >>= op;
            bits -= op;
            op = (unsigned)(here->op);
            if (op & 16) {                      /* distance base */
                dist = (unsigned)(here->val);
                op &= 15;                       /* number of extra bits */
                if (bits < op) {
                    hold += (unsigned long)(*in++) << bits;
                    bits += 8;
                    if (bits < op) {
                        hold += (unsigned long)(*in++) << bits;
                        bits += 8;
                    }
                }
                dist += (unsigned)hold & ((1U << op) - 1);
#ifdef INFLATE_STRICT
                if (dist > dmax) {
                    strm->msg = (char *)"invalid distance too far back";
                    state->mode = BAD;
                    break;
                }
#endif
                hold >>= op;
                bits -= op;
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (unsigned)(out - beg);     /* max distance in output */
                if (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    if (op > whave) {
                        if (state->sane) {
                            strm->msg =
                                (char *)"invalid distance too far back";
                            state->mode = BAD;
                            break;
                        }
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                        if (len <= op - whave) {
                            do {
                                *out++ = 0;
                            } while (--len);
                            continue;
                        }
                        len -= op - whave;
                        do {
                            *out++ = 0;
                        } while (--op > whave);
                        if (op == 0) {
                            from = out - dist;
                            do {
                                *out++ = *from++;
                            } while (--len);
                            continue;
                        }
#endif
                    }
                    from = window;
                    if (wnext == 0) {           /* very common case */
                        from += wsize - op;
                        if (op < len) {         /* some from window */
                            len -= op;
                            do {
                                *out++ = *from++;
                            } while (--op);
                            from = out - dist;  /* rest from output */
                        }
                    }
                    else if (wnext < op) {      /* wrap around window */
                        from += wsize + wnext - op;
                        op -= wnext;
                        if (op < len) {         /* some from end of window */
                            len -= op;
                            do {
                                *out++ = *from++;
                            } while (--op);
                            from = window;
                            if (wnext < len) {  /* some from start of window */
                                op = wnext;
                                len -= op;
                                do {
                                    *out++ = *from++;
                                } while (--op);
                                from = out - dist;      /* rest from output */
                            }
                        }
                    }
                    else {                      /* contiguous in window */
                        from += wnext - op;
                        if (op < len) {         /* some from window */
                            len -= op;
                            do {
                                *out++ = *from++;
                            } while (--op);
                            from = out - dist;  /* rest from output */
                        }
                    }
                    while (len > 2) {
                        *out++ = *from++;
                        *out++ = *from++;
                        *out++ = *from++;
                        len -= 3;
                    }
                    if (len) {
                        *out++ = *from++;
                        if (len > 1)
                            *out++ = *from++;
                    }
                }
                else {
                    from = out - dist;          /* copy direct from output */
                    do {                        /* minimum length is three */
                        *out++ = *from++;
                        *out++ = *from++;
                        *out++ = *from++;
                        len -= 3;
                    } while (len > 2);
                    if (len) {
                        *out++ = *from++;
                        if (len > 1)
                            *out++ = *from++;
                    }
                }
            }
            else if ((op & 64) == 0) {          /* 2nd level distance code */
                here = dcode + here->val + (hold & ((1U << op) - 1));
                goto dodist;
            }
            else {
                strm->msg = (char *)"invalid distance code";
                state->mode = BAD;
                break;
            }
        }
        else if ((op & 64) == 0) {              /* 2nd level length code */
            here = lcode + here->val + (hold & ((1U << op) - 1));
            goto dolen;
        }
        else if (op & 32) {                     /* end-of-block */
            Tracevv((stderr, "inflate:         end of block\n"));
            state->mode = TYPE;
            break;
        }
        else {
            strm->msg = (char *)"invalid literal/length code";
            state->mode = BAD;
            break;
        }
    } while (in < last && out < end);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = bits >> 3;
    in -= len;
    bits -= len << 3;
    hold &= (1U << bits) - 1;

    /* update state and return */
    strm->next_in = in;
    strm->next_out = out;
    strm->avail_in = (unsigned)(in < last ? 5 + (last - in) : 5 - (in - last));
    strm->avail_out = (unsigned)(out < end ?
                                 257 + (end - out) : 257 - (out - end));
    state->hold = hold;
    state->bits = bits;
    return;
}

/*
   inflate_fast() speedups that turned out slower (on a PowerPC G3 750CXe):
   - Using bit fields for code structure
   - Different op definition to avoid & for extra bits (do & for table bits)
   - Three separate decoding do-loops for direct, window, and wnext == 0
   - Special case for distance > 1 copies to do overlapped load and store copy
   - Explicit branch predictions (based on measured branch probabilities)
   - Deferring match copy and interspersed it with decoding subsequent codes
   - Swapping literal/length else
   - Swapping window/direct else
   - Larger unrolled copy loops (three is about right)
   - Moving len -= 3 statement into middle of loop
 */

#endif /* !ASMINF */
/* inflate.c -- zlib decompression
 * Copyright (C) 1995-2022 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/*
 * Change history:
 *
 * 1.2.beta0    24 Nov 2002
 * - First version -- complete rewrite of inflate to simplify code, avoid
 *   creation of window when not needed, minimize use of window when it is
 *   needed, make inffast.c even faster, implement gzip decoding, and to
 *   improve code readability and style over the previous zlib inflate code
 *
 * 1.2.beta1    25 Nov 2002
 * - Use pointers for available input and output checking in inffast.c
 * - Remove input and output counters in inffast.c
 * - Change inffast.c entry and loop from avail_in >= 7 to >= 6
 * - Remove unnecessary second byte pull from length extra in inffast.c
 * - Unroll direct copy to three copies per loop in inffast.c
 *
 * 1.2.beta2    4 Dec 2002
 * - Change external routine names to reduce potential conflicts
 * - Correct filename to inffixed.h for fixed tables in inflate.c
 * - Make hbuf[] unsigned char to match parameter type in inflate.c
 * - Change strm->next_out[-state->offset] to *(strm->next_out - state->offset)
 *   to avoid negation problem on Alphas (64 bit) in inflate.c
 *
 * 1.2.beta3    22 Dec 2002
 * - Add comments on state->bits assertion in inffast.c
 * - Add comments on op field in inftrees.h
 * - Fix bug in reuse of allocated window after inflateReset()
 * - Remove bit fields--back to byte structure for speed
 * - Remove distance extra == 0 check in inflate_fast()--only helps for lengths
 * - Change post-increments to pre-increments in inflate_fast(), PPC biased?
 * - Add compile time option, POSTINC, to use post-increments instead (Intel?)
 * - Make MATCH copy in inflate() much faster for when inflate_fast() not used
 * - Use local copies of stream next and avail values, as well as local bit
 *   buffer and bit count in inflate()--for speed when inflate_fast() not used
 *
 * 1.2.beta4    1 Jan 2003
 * - Split ptr - 257 statements in inflate_table() to avoid compiler warnings
 * - Move a comment on output buffer sizes from inffast.c to inflate.c
 * - Add comments in inffast.c to introduce the inflate_fast() routine
 * - Rearrange window copies in inflate_fast() for speed and simplification
 * - Unroll last copy for window match in inflate_fast()
 * - Use local copies of window variables in inflate_fast() for speed
 * - Pull out common wnext == 0 case for speed in inflate_fast()
 * - Make op and len in inflate_fast() unsigned for consistency
 * - Add FAR to lcode and dcode declarations in inflate_fast()
 * - Simplified bad distance check in inflate_fast()
 * - Added inflateBackInit(), inflateBack(), and inflateBackEnd() in new
 *   source file infback.c to provide a call-back interface to inflate for
 *   programs like gzip and unzip -- uses window as output buffer to avoid
 *   window copying
 *
 * 1.2.beta5    1 Jan 2003
 * - Improved inflateBack() interface to allow the caller to provide initial
 *   input in strm.
 * - Fixed stored blocks bug in inflateBack()
 *
 * 1.2.beta6    4 Jan 2003
 * - Added comments in inffast.c on effectiveness of POSTINC
 * - Typecasting all around to reduce compiler warnings
 * - Changed loops from while (1) or do {} while (1) to for (;;), again to
 *   make compilers happy
 * - Changed type of window in inflateBackInit() to unsigned char *
 *
 * 1.2.beta7    27 Jan 2003
 * - Changed many types to unsigned or unsigned short to avoid warnings
 * - Added inflateCopy() function
 *
 * 1.2.0        9 Mar 2003
 * - Changed inflateBack() interface to provide separate opaque descriptors
 *   for the in() and out() functions
 * - Changed inflateBack() argument and in_func typedef to swap the length
 *   and buffer address return values for the input function
 * - Check next_in and next_out for Z_NULL on entry to inflate()
 *
 * The history for versions after 1.2.0 are in ChangeLog in zlib distribution.
 */

//REBOL: #include "zutil.h"
//REBOL: #include "inftrees.h"
//REBOL: #include "inflate.h"
//REBOL: #include "inffast.h"

#ifdef MAKEFIXED
#  ifndef BUILDFIXED
#    define BUILDFIXED
#  endif
#endif

local int inflateStateCheck(z_streamp strm) {
    struct inflate_state FAR *state;
    if (strm == Z_NULL ||
        strm->zalloc == (alloc_func)0 || strm->zfree == (free_func)0)
        return 1;
    state = (struct inflate_state FAR *)strm->state;
    if (state == Z_NULL || state->strm != strm ||
        state->mode < HEAD || state->mode > SYNC)
        return 1;
    return 0;
}

int ZEXPORT inflateResetKeep(z_streamp strm) {
    struct inflate_state FAR *state;

    if (inflateStateCheck(strm)) return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;
    strm->total_in = strm->total_out = state->total = 0;
    strm->msg = Z_NULL;
    if (state->wrap)        /* to support ill-conceived Java test suite */
        strm->adler = state->wrap & 1;
    state->mode = HEAD;
    state->last = 0;
    state->havedict = 0;
    state->flags = -1;
    state->dmax = 32768U;
    state->head = Z_NULL;
    state->hold = 0;
    state->bits = 0;
    state->lencode = state->distcode = state->next = state->codes;
    state->sane = 1;
    state->back = -1;
    Tracev((stderr, "inflate: reset\n"));
    return Z_OK;
}

int ZEXPORT inflateReset(z_streamp strm) {
    struct inflate_state FAR *state;

    if (inflateStateCheck(strm)) return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;
    state->wsize = 0;
    state->whave = 0;
    state->wnext = 0;
    return inflateResetKeep(strm);
}

int ZEXPORT inflateReset2(z_streamp strm, int windowBits) {
    int wrap;
    struct inflate_state FAR *state;

    /* get the state */
    if (inflateStateCheck(strm)) return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;

    /* extract wrap request from windowBits parameter */
    if (windowBits < 0) {
        if (windowBits < -15)
            return Z_STREAM_ERROR;
        wrap = 0;
        windowBits = -windowBits;
    }
    else {
        wrap = (windowBits >> 4) + 5;
#ifdef GUNZIP
        if (windowBits < 48)
            windowBits &= 15;
#endif
    }

    /* set number of window bits, free window if different */
    if (windowBits && (windowBits < 8 || windowBits > 15))
        return Z_STREAM_ERROR;
    if (state->window != Z_NULL && state->wbits != (unsigned)windowBits) {
        ZFREE(strm, state->window);
        state->window = Z_NULL;
    }

    /* update state and reset the rest of it */
    state->wrap = wrap;
    state->wbits = (unsigned)windowBits;
    return inflateReset(strm);
}

int ZEXPORT inflateInit2_(z_streamp strm, int windowBits,
                          const char *version, int stream_size) {
    int ret;
    struct inflate_state FAR *state;

    if (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (int)(sizeof(z_stream)))
        return Z_VERSION_ERROR;
    if (strm == Z_NULL) return Z_STREAM_ERROR;
    strm->msg = Z_NULL;                 /* in case we return an error */
    if (strm->zalloc == (alloc_func)0) {
#ifdef Z_SOLO
        return Z_STREAM_ERROR;
#else
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
#endif
    }
    if (strm->zfree == (free_func)0)
#ifdef Z_SOLO
        return Z_STREAM_ERROR;
#else
        strm->zfree = zcfree;
#endif
    state = (struct inflate_state FAR *)
            ZALLOC(strm, 1, sizeof(struct inflate_state));
    if (state == Z_NULL) return Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (struct internal_state FAR *)state;
    state->strm = strm;
    state->window = Z_NULL;
    state->mode = HEAD;     /* to pass state test in inflateReset2() */
    ret = inflateReset2(strm, windowBits);
    if (ret != Z_OK) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
    }
    return ret;
}

int ZEXPORT inflateInit_(z_streamp strm, const char *version,
                         int stream_size) {
    return inflateInit2_(strm, DEF_WBITS, version, stream_size);
}

int ZEXPORT inflatePrime(z_streamp strm, int bits, int value) {
    struct inflate_state FAR *state;

    if (inflateStateCheck(strm)) return Z_STREAM_ERROR;
    if (bits == 0)
        return Z_OK;
    state = (struct inflate_state FAR *)strm->state;
    if (bits < 0) {
        state->hold = 0;
        state->bits = 0;
        return Z_OK;
    }
    if (bits > 16 || state->bits + (uInt)bits > 32) return Z_STREAM_ERROR;
    value &= (1L << bits) - 1;
    state->hold += (unsigned)value << state->bits;
    state->bits += (uInt)bits;
    return Z_OK;
}

/*
   Return state with length and distance decoding tables and index sizes set to
   fixed code decoding.  Normally this returns fixed tables from inffixed.h.
   If BUILDFIXED is defined, then instead this routine builds the tables the
   first time it's called, and returns those tables the first time and
   thereafter.  This reduces the size of the code by about 2K bytes, in
   exchange for a little execution time.  However, BUILDFIXED should not be
   used for threaded applications, since the rewriting of the tables and virgin
   may not be thread-safe.
 */
local void fixedtables(struct inflate_state FAR *state) {
#ifdef BUILDFIXED
    static int virgin = 1;
    static code *lenfix, *distfix;
    static code fixed[544];

    /* build fixed huffman tables if first call (may not be thread safe) */
    if (virgin) {
        unsigned sym, bits;
        static code *next;

        /* literal/length table */
        sym = 0;
        while (sym < 144) state->lens[sym++] = 8;
        while (sym < 256) state->lens[sym++] = 9;
        while (sym < 280) state->lens[sym++] = 7;
        while (sym < 288) state->lens[sym++] = 8;
        next = fixed;
        lenfix = next;
        bits = 9;
        inflate_table(LENS, state->lens, 288, &(next), &(bits), state->work);

        /* distance table */
        sym = 0;
        while (sym < 32) state->lens[sym++] = 5;
        distfix = next;
        bits = 5;
        inflate_table(DISTS, state->lens, 32, &(next), &(bits), state->work);

        /* do this just once */
        virgin = 0;
    }
#else /* !BUILDFIXED */
//REBOL: #   include "inffixed.h"
    /* inffixed.h -- table for decoding fixed codes
     * Generated automatically by makefixed().
     */

    /* WARNING: this file should *not* be used by applications.
       It is part of the implementation of this library and is
       subject to change. Applications should only use zlib.h.
     */

    static const code lenfix[512] = {
        {96,7,0},{0,8,80},{0,8,16},{20,8,115},{18,7,31},{0,8,112},{0,8,48},
        {0,9,192},{16,7,10},{0,8,96},{0,8,32},{0,9,160},{0,8,0},{0,8,128},
        {0,8,64},{0,9,224},{16,7,6},{0,8,88},{0,8,24},{0,9,144},{19,7,59},
        {0,8,120},{0,8,56},{0,9,208},{17,7,17},{0,8,104},{0,8,40},{0,9,176},
        {0,8,8},{0,8,136},{0,8,72},{0,9,240},{16,7,4},{0,8,84},{0,8,20},
        {21,8,227},{19,7,43},{0,8,116},{0,8,52},{0,9,200},{17,7,13},{0,8,100},
        {0,8,36},{0,9,168},{0,8,4},{0,8,132},{0,8,68},{0,9,232},{16,7,8},
        {0,8,92},{0,8,28},{0,9,152},{20,7,83},{0,8,124},{0,8,60},{0,9,216},
        {18,7,23},{0,8,108},{0,8,44},{0,9,184},{0,8,12},{0,8,140},{0,8,76},
        {0,9,248},{16,7,3},{0,8,82},{0,8,18},{21,8,163},{19,7,35},{0,8,114},
        {0,8,50},{0,9,196},{17,7,11},{0,8,98},{0,8,34},{0,9,164},{0,8,2},
        {0,8,130},{0,8,66},{0,9,228},{16,7,7},{0,8,90},{0,8,26},{0,9,148},
        {20,7,67},{0,8,122},{0,8,58},{0,9,212},{18,7,19},{0,8,106},{0,8,42},
        {0,9,180},{0,8,10},{0,8,138},{0,8,74},{0,9,244},{16,7,5},{0,8,86},
        {0,8,22},{64,8,0},{19,7,51},{0,8,118},{0,8,54},{0,9,204},{17,7,15},
        {0,8,102},{0,8,38},{0,9,172},{0,8,6},{0,8,134},{0,8,70},{0,9,236},
        {16,7,9},{0,8,94},{0,8,30},{0,9,156},{20,7,99},{0,8,126},{0,8,62},
        {0,9,220},{18,7,27},{0,8,110},{0,8,46},{0,9,188},{0,8,14},{0,8,142},
        {0,8,78},{0,9,252},{96,7,0},{0,8,81},{0,8,17},{21,8,131},{18,7,31},
        {0,8,113},{0,8,49},{0,9,194},{16,7,10},{0,8,97},{0,8,33},{0,9,162},
        {0,8,1},{0,8,129},{0,8,65},{0,9,226},{16,7,6},{0,8,89},{0,8,25},
        {0,9,146},{19,7,59},{0,8,121},{0,8,57},{0,9,210},{17,7,17},{0,8,105},
        {0,8,41},{0,9,178},{0,8,9},{0,8,137},{0,8,73},{0,9,242},{16,7,4},
        {0,8,85},{0,8,21},{16,8,258},{19,7,43},{0,8,117},{0,8,53},{0,9,202},
        {17,7,13},{0,8,101},{0,8,37},{0,9,170},{0,8,5},{0,8,133},{0,8,69},
        {0,9,234},{16,7,8},{0,8,93},{0,8,29},{0,9,154},{20,7,83},{0,8,125},
        {0,8,61},{0,9,218},{18,7,23},{0,8,109},{0,8,45},{0,9,186},{0,8,13},
        {0,8,141},{0,8,77},{0,9,250},{16,7,3},{0,8,83},{0,8,19},{21,8,195},
        {19,7,35},{0,8,115},{0,8,51},{0,9,198},{17,7,11},{0,8,99},{0,8,35},
        {0,9,166},{0,8,3},{0,8,131},{0,8,67},{0,9,230},{16,7,7},{0,8,91},
        {0,8,27},{0,9,150},{20,7,67},{0,8,123},{0,8,59},{0,9,214},{18,7,19},
        {0,8,107},{0,8,43},{0,9,182},{0,8,11},{0,8,139},{0,8,75},{0,9,246},
        {16,7,5},{0,8,87},{0,8,23},{64,8,0},{19,7,51},{0,8,119},{0,8,55},
        {0,9,206},{17,7,15},{0,8,103},{0,8,39},{0,9,174},{0,8,7},{0,8,135},
        {0,8,71},{0,9,238},{16,7,9},{0,8,95},{0,8,31},{0,9,158},{20,7,99},
        {0,8,127},{0,8,63},{0,9,222},{18,7,27},{0,8,111},{0,8,47},{0,9,190},
        {0,8,15},{0,8,143},{0,8,79},{0,9,254},{96,7,0},{0,8,80},{0,8,16},
        {20,8,115},{18,7,31},{0,8,112},{0,8,48},{0,9,193},{16,7,10},{0,8,96},
        {0,8,32},{0,9,161},{0,8,0},{0,8,128},{0,8,64},{0,9,225},{16,7,6},
        {0,8,88},{0,8,24},{0,9,145},{19,7,59},{0,8,120},{0,8,56},{0,9,209},
        {17,7,17},{0,8,104},{0,8,40},{0,9,177},{0,8,8},{0,8,136},{0,8,72},
        {0,9,241},{16,7,4},{0,8,84},{0,8,20},{21,8,227},{19,7,43},{0,8,116},
        {0,8,52},{0,9,201},{17,7,13},{0,8,100},{0,8,36},{0,9,169},{0,8,4},
        {0,8,132},{0,8,68},{0,9,233},{16,7,8},{0,8,92},{0,8,28},{0,9,153},
        {20,7,83},{0,8,124},{0,8,60},{0,9,217},{18,7,23},{0,8,108},{0,8,44},
        {0,9,185},{0,8,12},{0,8,140},{0,8,76},{0,9,249},{16,7,3},{0,8,82},
        {0,8,18},{21,8,163},{19,7,35},{0,8,114},{0,8,50},{0,9,197},{17,7,11},
        {0,8,98},{0,8,34},{0,9,165},{0,8,2},{0,8,130},{0,8,66},{0,9,229},
        {16,7,7},{0,8,90},{0,8,26},{0,9,149},{20,7,67},{0,8,122},{0,8,58},
        {0,9,213},{18,7,19},{0,8,106},{0,8,42},{0,9,181},{0,8,10},{0,8,138},
        {0,8,74},{0,9,245},{16,7,5},{0,8,86},{0,8,22},{64,8,0},{19,7,51},
        {0,8,118},{0,8,54},{0,9,205},{17,7,15},{0,8,102},{0,8,38},{0,9,173},
        {0,8,6},{0,8,134},{0,8,70},{0,9,237},{16,7,9},{0,8,94},{0,8,30},
        {0,9,157},{20,7,99},{0,8,126},{0,8,62},{0,9,221},{18,7,27},{0,8,110},
        {0,8,46},{0,9,189},{0,8,14},{0,8,142},{0,8,78},{0,9,253},{96,7,0},
        {0,8,81},{0,8,17},{21,8,131},{18,7,31},{0,8,113},{0,8,49},{0,9,195},
        {16,7,10},{0,8,97},{0,8,33},{0,9,163},{0,8,1},{0,8,129},{0,8,65},
        {0,9,227},{16,7,6},{0,8,89},{0,8,25},{0,9,147},{19,7,59},{0,8,121},
        {0,8,57},{0,9,211},{17,7,17},{0,8,105},{0,8,41},{0,9,179},{0,8,9},
        {0,8,137},{0,8,73},{0,9,243},{16,7,4},{0,8,85},{0,8,21},{16,8,258},
        {19,7,43},{0,8,117},{0,8,53},{0,9,203},{17,7,13},{0,8,101},{0,8,37},
        {0,9,171},{0,8,5},{0,8,133},{0,8,69},{0,9,235},{16,7,8},{0,8,93},
        {0,8,29},{0,9,155},{20,7,83},{0,8,125},{0,8,61},{0,9,219},{18,7,23},
        {0,8,109},{0,8,45},{0,9,187},{0,8,13},{0,8,141},{0,8,77},{0,9,251},
        {16,7,3},{0,8,83},{0,8,19},{21,8,195},{19,7,35},{0,8,115},{0,8,51},
        {0,9,199},{17,7,11},{0,8,99},{0,8,35},{0,9,167},{0,8,3},{0,8,131},
        {0,8,67},{0,9,231},{16,7,7},{0,8,91},{0,8,27},{0,9,151},{20,7,67},
        {0,8,123},{0,8,59},{0,9,215},{18,7,19},{0,8,107},{0,8,43},{0,9,183},
        {0,8,11},{0,8,139},{0,8,75},{0,9,247},{16,7,5},{0,8,87},{0,8,23},
        {64,8,0},{19,7,51},{0,8,119},{0,8,55},{0,9,207},{17,7,15},{0,8,103},
        {0,8,39},{0,9,175},{0,8,7},{0,8,135},{0,8,71},{0,9,239},{16,7,9},
        {0,8,95},{0,8,31},{0,9,159},{20,7,99},{0,8,127},{0,8,63},{0,9,223},
        {18,7,27},{0,8,111},{0,8,47},{0,9,191},{0,8,15},{0,8,143},{0,8,79},
        {0,9,255}
    };

    static const code distfix[32] = {
        {16,5,1},{23,5,257},{19,5,17},{27,5,4097},{17,5,5},{25,5,1025},
        {21,5,65},{29,5,16385},{16,5,3},{24,5,513},{20,5,33},{28,5,8193},
        {18,5,9},{26,5,2049},{22,5,129},{64,5,0},{16,5,2},{23,5,385},
        {19,5,25},{27,5,6145},{17,5,7},{25,5,1537},{21,5,97},{29,5,24577},
        {16,5,4},{24,5,769},{20,5,49},{28,5,12289},{18,5,13},{26,5,3073},
        {22,5,193},{64,5,0}
    };

#endif /* BUILDFIXED */
    state->lencode = lenfix;
    state->lenbits = 9;
    state->distcode = distfix;
    state->distbits = 5;
}

#ifdef MAKEFIXED
#include <stdio.h>

/*
   Write out the inffixed.h that is #include'd above.  Defining MAKEFIXED also
   defines BUILDFIXED, so the tables are built on the fly.  makefixed() writes
   those tables to stdout, which would be piped to inffixed.h.  A small program
   can simply call makefixed to do this:

    void makefixed(void);

    int main(void)
    {
        makefixed();
        return 0;
    }

   Then that can be linked with zlib built with MAKEFIXED defined and run:

    a.out > inffixed.h
 */
void makefixed(void)
{
    unsigned low, size;
    struct inflate_state state;

    fixedtables(&state);
    puts("    /* inffixed.h -- table for decoding fixed codes");
    puts("     * Generated automatically by makefixed().");
    puts("     */");
    puts("");
    puts("    /* WARNING: this file should *not* be used by applications.");
    puts("       It is part of the implementation of this library and is");
    puts("       subject to change. Applications should only use zlib.h.");
    puts("     */");
    puts("");
    size = 1U << 9;
    printf("    static const code lenfix[%u] = {", size);
    low = 0;
    for (;;) {
        if ((low % 7) == 0) printf("\n        ");
        printf("{%u,%u,%d}", (low & 127) == 99 ? 64 : state.lencode[low].op,
               state.lencode[low].bits, state.lencode[low].val);
        if (++low == size) break;
        putchar(',');
    }
    puts("\n    };");
    size = 1U << 5;
    printf("\n    static const code distfix[%u] = {", size);
    low = 0;
    for (;;) {
        if ((low % 6) == 0) printf("\n        ");
        printf("{%u,%u,%d}", state.distcode[low].op, state.distcode[low].bits,
               state.distcode[low].val);
        if (++low == size) break;
        putchar(',');
    }
    puts("\n    };");
}
#endif /* MAKEFIXED */

/*
   Update the window with the last wsize (normally 32K) bytes written before
   returning.  If window does not exist yet, create it.  This is only called
   when a window is already in use, or when output has been written during this
   inflate call, but the end of the deflate stream has not been reached yet.
   It is also called to create a window for dictionary data when a dictionary
   is loaded.

   Providing output buffers larger than 32K to inflate() should provide a speed
   advantage, since only the last 32K of output is copied to the sliding window
   upon return from inflate(), and since all distances after the first 32K of
   output will fall in the output data, making match copies simpler and faster.
   The advantage may be dependent on the size of the processor's data caches.
 */
local int updatewindow(z_streamp strm, const Bytef *end, unsigned copy) {
    struct inflate_state FAR *state;
    unsigned dist;

    state = (struct inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    if (state->window == Z_NULL) {
        state->window = (unsigned char FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               sizeof(unsigned char));
        if (state->window == Z_NULL) return 1;
    }

    /* if window not in use yet, initialize */
    if (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->wnext = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    if (copy >= state->wsize) {
        zmemcpy(state->window, end - state->wsize, state->wsize);
        state->wnext = 0;
        state->whave = state->wsize;
    }
    else {
        dist = state->wsize - state->wnext;
        if (dist > copy) dist = copy;
        zmemcpy(state->window + state->wnext, end - copy, dist);
        copy -= dist;
        if (copy) {
            zmemcpy(state->window, end - copy, copy);
            state->wnext = copy;
            state->whave = state->wsize;
        }
        else {
            state->wnext += dist;
            if (state->wnext == state->wsize) state->wnext = 0;
            if (state->whave < state->wsize) state->whave += dist;
        }
    }
    return 0;
}

/* Macros for inflate(): */

/* check function to use adler32() for zlib or crc32() for gzip */
#ifdef GUNZIP
#  define UPDATE_CHECK(check, buf, len) \
    (state->flags ? crc32(check, buf, len) : adler32(check, buf, len))
#else
#  define UPDATE_CHECK(check, buf, len) adler32(check, buf, len)
#endif

/* check macros for header crc */
#ifdef GUNZIP
#  define CRC2(check, word) \
    do { \
        hbuf[0] = (unsigned char)(word); \
        hbuf[1] = (unsigned char)((word) >> 8); \
        check = crc32(check, hbuf, 2); \
    } while (0)

#  define CRC4(check, word) \
    do { \
        hbuf[0] = (unsigned char)(word); \
        hbuf[1] = (unsigned char)((word) >> 8); \
        hbuf[2] = (unsigned char)((word) >> 16); \
        hbuf[3] = (unsigned char)((word) >> 24); \
        check = crc32(check, hbuf, 4); \
    } while (0)
#endif

/* Load registers with state in inflate() for speed */
#define LOAD() \
    do { \
        put = strm->next_out; \
        left = strm->avail_out; \
        next = strm->next_in; \
        have = strm->avail_in; \
        hold = state->hold; \
        bits = state->bits; \
    } while (0)

/* Restore state from registers in inflate() */
#define RESTORE() \
    do { \
        strm->next_out = put; \
        strm->avail_out = left; \
        strm->next_in = next; \
        strm->avail_in = have; \
        state->hold = hold; \
        state->bits = bits; \
    } while (0)

/* Clear the input bit accumulator */
#define INITBITS() \
    do { \
        hold = 0; \
        bits = 0; \
    } while (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#define PULLBYTE() \
    do { \
        if (have == 0) goto inf_leave; \
        have--; \
        hold += (unsigned long)(*next++) << bits; \
        bits += 8; \
    } while (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#define NEEDBITS(n) \
    do { \
        while (bits < (unsigned)(n)) \
            PULLBYTE(); \
    } while (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
    ((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
    do { \
        hold >>= (n); \
        bits -= (unsigned)(n); \
    } while (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
    do { \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    } while (0)

/*
   inflate() uses a state machine to process as much input data and generate as
   much output data as possible before returning.  The state machine is
   structured roughly as follows:

    for (;;) switch (state) {
    ...
    case STATEn:
        if (not enough input data or output space to make progress)
            return;
        ... make progress ...
        state = STATEm;
        break;
    ...
    }

   so when inflate() is called again, the same case is attempted again, and
   if the appropriate resources are provided, the machine proceeds to the
   next state.  The NEEDBITS() macro is usually the way the state evaluates
   whether it can proceed or should return.  NEEDBITS() does the return if
   the requested bits are not available.  The typical use of the BITS macros
   is:

        NEEDBITS(n);
        ... do something with BITS(n) ...
        DROPBITS(n);

   where NEEDBITS(n) either returns from inflate() if there isn't enough
   input left to load n bits into the accumulator, or it continues.  BITS(n)
   gives the low n bits in the accumulator.  When done, DROPBITS(n) drops
   the low n bits off the accumulator.  INITBITS() clears the accumulator
   and sets the number of available bits to zero.  BYTEBITS() discards just
   enough bits to put the accumulator on a byte boundary.  After BYTEBITS()
   and a NEEDBITS(8), then BITS(8) would return the next byte in the stream.

   NEEDBITS(n) uses PULLBYTE() to get an available byte of input, or to return
   if there is no input available.  The decoding of variable length codes uses
   PULLBYTE() directly in order to pull just enough bytes to decode the next
   code, and no more.

   Some states loop until they get enough input, making sure that enough
   state information is maintained to continue the loop where it left off
   if NEEDBITS() returns in the loop.  For example, want, need, and keep
   would all have to actually be part of the saved state in case NEEDBITS()
   returns:

    case STATEw:
        while (want < need) {
            NEEDBITS(n);
            keep[want++] = BITS(n);
            DROPBITS(n);
        }
        state = STATEx;
    case STATEx:

   As shown above, if the next state is also the next case, then the break
   is omitted.

   A state may also return if there is not enough output space available to
   complete that state.  Those states are copying stored data, writing a
   literal byte, and copying a matching string.

   When returning, a "goto inf_leave" is used to update the total counters,
   update the check value, and determine whether any progress has been made
   during that inflate() call in order to return the proper return code.
   Progress is defined as a change in either strm->avail_in or strm->avail_out.
   When there is a window, goto inf_leave will update the window with the last
   output written.  If a goto inf_leave occurs in the middle of decompression
   and there is no window currently, goto inf_leave will create one and copy
   output to the window for the next call of inflate().

   In this implementation, the flush parameter of inflate() only affects the
   return code (per zlib.h).  inflate() always writes as much as possible to
   strm->next_out, given the space available and the provided input--the effect
   documented in zlib.h of Z_SYNC_FLUSH.  Furthermore, inflate() always defers
   the allocation of and copying into a sliding window until necessary, which
   provides the effect documented in zlib.h for Z_FINISH when the entire input
   stream available.  So the only thing the flush parameter actually does is:
   when flush is set to Z_FINISH, inflate() cannot return Z_OK.  Instead it
   will return Z_BUF_ERROR if it has not reached the end of the stream.
 */

int ZEXPORT inflate(z_streamp strm, int flush) {
    struct inflate_state FAR *state;
    z_const unsigned char FAR *next;    /* next input */
    unsigned char FAR *put;     /* next output */
    unsigned have, left;        /* available input and output */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned in, out;           /* save starting available input and output */
    unsigned copy;              /* number of stored or match bytes to copy */
    unsigned char FAR *from;    /* where to copy match bytes from */
    code here;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    int ret;                    /* return code */
#ifdef GUNZIP
    unsigned char hbuf[4];      /* buffer for gzip header crc calculation */
#endif
    static const unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    if (inflateStateCheck(strm) || strm->next_out == Z_NULL ||
        (strm->next_in == Z_NULL && strm->avail_in != 0))
        return Z_STREAM_ERROR;

    state = (struct inflate_state FAR *)strm->state;
    if (state->mode == TYPE) state->mode = TYPEDO;      /* skip check */
    LOAD();
    in = have;
    out = left;
    ret = Z_OK;
    for (;;)
        switch (state->mode) {
        case HEAD:
            if (state->wrap == 0) {
                state->mode = TYPEDO;
                break;
            }
            NEEDBITS(16);
#ifdef GUNZIP
            if ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                if (state->wbits == 0)
                    state->wbits = 15;
                state->check = crc32(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->mode = FLAGS;
                break;
            }
            if (state->head != Z_NULL)
                state->head->done = -1;
            if (!(state->wrap & 1) ||   /* check if zlib header allowed */
#else
            if (
#endif
                ((BITS(8) << 8) + (hold >> 8)) % 31) {
                strm->msg = (char *)"incorrect header check";
                state->mode = BAD;
                break;
            }
            if (BITS(4) != Z_DEFLATED) {
                strm->msg = (char *)"unknown compression method";
                state->mode = BAD;
                break;
            }
            DROPBITS(4);
            len = BITS(4) + 8;
            if (state->wbits == 0)
                state->wbits = len;
            if (len > 15 || len > state->wbits) {
                strm->msg = (char *)"invalid window size";
                state->mode = BAD;
                break;
            }
            state->dmax = 1U << len;
            state->flags = 0;               /* indicate zlib header */
            Tracev((stderr, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->mode = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            break;
#ifdef GUNZIP
        case FLAGS:
            NEEDBITS(16);
            state->flags = (int)(hold);
            if ((state->flags & 0xff) != Z_DEFLATED) {
                strm->msg = (char *)"unknown compression method";
                state->mode = BAD;
                break;
            }
            if (state->flags & 0xe000) {
                strm->msg = (char *)"unknown header flags set";
                state->mode = BAD;
                break;
            }
            if (state->head != Z_NULL)
                state->head->text = (int)((hold >> 8) & 1);
            if ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->mode = TIME;
                /* fallthrough */
        case TIME:
            NEEDBITS(32);
            if (state->head != Z_NULL)
                state->head->time = hold;
            if ((state->flags & 0x0200) && (state->wrap & 4))
                CRC4(state->check, hold);
            INITBITS();
            state->mode = OS;
                /* fallthrough */
        case OS:
            NEEDBITS(16);
            if (state->head != Z_NULL) {
                state->head->xflags = (int)(hold & 0xff);
                state->head->os = (int)(hold >> 8);
            }
            if ((state->flags & 0x0200) && (state->wrap & 4))
                CRC2(state->check, hold);
            INITBITS();
            state->mode = EXLEN;
                /* fallthrough */
        case EXLEN:
            if (state->flags & 0x0400) {
                NEEDBITS(16);
                state->length = (unsigned)(hold);
                if (state->head != Z_NULL)
                    state->head->extra_len = (unsigned)hold;
                if ((state->flags & 0x0200) && (state->wrap & 4))
                    CRC2(state->check, hold);
                INITBITS();
            }
            else if (state->head != Z_NULL)
                state->head->extra = Z_NULL;
            state->mode = EXTRA;
                /* fallthrough */
        case EXTRA:
            if (state->flags & 0x0400) {
                copy = state->length;
                if (copy > have) copy = have;
                if (copy) {
                    if (state->head != Z_NULL &&
                        state->head->extra != Z_NULL &&
                        (len = state->head->extra_len - state->length) <
                            state->head->extra_max) {
                        zmemcpy(state->head->extra + len, next,
                                len + copy > state->head->extra_max ?
                                state->head->extra_max - len : copy);
                    }
                    if ((state->flags & 0x0200) && (state->wrap & 4))
                        state->check = crc32(state->check, next, copy);
                    have -= copy;
                    next += copy;
                    state->length -= copy;
                }
                if (state->length) goto inf_leave;
            }
            state->length = 0;
            state->mode = NAME;
                /* fallthrough */
        case NAME:
            if (state->flags & 0x0800) {
                if (have == 0) goto inf_leave;
                copy = 0;
                do {
                    len = (unsigned)(next[copy++]);
                    if (state->head != Z_NULL &&
                            state->head->name != Z_NULL &&
                            state->length < state->head->name_max)
                        state->head->name[state->length++] = (Bytef)len;
                } while (len && copy < have);
                if ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = crc32(state->check, next, copy);
                have -= copy;
                next += copy;
                if (len) goto inf_leave;
            }
            else if (state->head != Z_NULL)
                state->head->name = Z_NULL;
            state->length = 0;
            state->mode = COMMENT;
                /* fallthrough */
        case COMMENT:
            if (state->flags & 0x1000) {
                if (have == 0) goto inf_leave;
                copy = 0;
                do {
                    len = (unsigned)(next[copy++]);
                    if (state->head != Z_NULL &&
                            state->head->comment != Z_NULL &&
                            state->length < state->head->comm_max)
                        state->head->comment[state->length++] = (Bytef)len;
                } while (len && copy < have);
                if ((state->flags & 0x0200) && (state->wrap & 4))
                    state->check = crc32(state->check, next, copy);
                have -= copy;
                next += copy;
                if (len) goto inf_leave;
            }
            else if (state->head != Z_NULL)
                state->head->comment = Z_NULL;
            state->mode = HCRC;
                /* fallthrough */
        case HCRC:
            if (state->flags & 0x0200) {
                NEEDBITS(16);
                if ((state->wrap & 4) && hold != (state->check & 0xffff)) {
                    strm->msg = (char *)"header crc mismatch";
                    state->mode = BAD;
                    break;
                }
                INITBITS();
            }
            if (state->head != Z_NULL) {
                state->head->hcrc = (int)((state->flags >> 9) & 1);
                state->head->done = 1;
            }
            strm->adler = state->check = crc32(0L, Z_NULL, 0);
            state->mode = TYPE;
            break;
#endif
        case DICTID:
            NEEDBITS(32);
            strm->adler = state->check = ZSWAP32(hold);
            INITBITS();
            state->mode = DICT;
                /* fallthrough */
        case DICT:
            if (state->havedict == 0) {
                RESTORE();
                return Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->mode = TYPE;
                /* fallthrough */
        case TYPE:
            if (flush == Z_BLOCK || flush == Z_TREES) goto inf_leave;
                /* fallthrough */
        case TYPEDO:
            if (state->last) {
                BYTEBITS();
                state->mode = CHECK;
                break;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            switch (BITS(2)) {
            case 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                break;
            case 1:                             /* fixed block */
                fixedtables(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN_;             /* decode codes */
                if (flush == Z_TREES) {
                    DROPBITS(2);
                    goto inf_leave;
                }
                break;
            case 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                break;
            case 3:
                strm->msg = (char *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            break;
        case STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            if ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->msg = (char *)"invalid stored block lengths";
                state->mode = BAD;
                break;
            }
            state->length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->length));
            INITBITS();
            state->mode = COPY_;
            if (flush == Z_TREES) goto inf_leave;
                /* fallthrough */
        case COPY_:
            state->mode = COPY;
                /* fallthrough */
        case COPY:
            copy = state->length;
            if (copy) {
                if (copy > have) copy = have;
                if (copy > left) copy = left;
                if (copy == 0) goto inf_leave;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                left -= copy;
                put += copy;
                state->length -= copy;
                break;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->mode = TYPE;
            break;
        case TABLE:
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
#ifndef PKZIP_BUG_WORKAROUND
            if (state->nlen > 286 || state->ndist > 30) {
                strm->msg = (char *)"too many length or distance symbols";
                state->mode = BAD;
                break;
            }
#endif
            Tracev((stderr, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->mode = LENLENS;
                /* fallthrough */
        case LENLENS:
            while (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            while (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            state->lencode = (const code FAR *)(state->next);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->next),
                                &(state->lenbits), state->work);
            if (ret) {
                strm->msg = (char *)"invalid code lengths set";
                state->mode = BAD;
                break;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->mode = CODELENS;
                /* fallthrough */
        case CODELENS:
            while (state->have < state->nlen + state->ndist) {
                for (;;) {
                    here = state->lencode[BITS(state->lenbits)];
                    if ((unsigned)(here.bits) <= bits) break;
                    PULLBYTE();
                }
                if (here.val < 16) {
                    DROPBITS(here.bits);
                    state->lens[state->have++] = here.val;
                }
                else {
                    if (here.val == 16) {
                        NEEDBITS(here.bits + 2);
                        DROPBITS(here.bits);
                        if (state->have == 0) {
                            strm->msg = (char *)"invalid bit length repeat";
                            state->mode = BAD;
                            break;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    else if (here.val == 17) {
                        NEEDBITS(here.bits + 3);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    else {
                        NEEDBITS(here.bits + 7);
                        DROPBITS(here.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    if (state->have + copy > state->nlen + state->ndist) {
                        strm->msg = (char *)"invalid bit length repeat";
                        state->mode = BAD;
                        break;
                    }
                    while (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            if (state->mode == BAD) break;

            /* check for end-of-block code (better have one) */
            if (state->lens[256] == 0) {
                strm->msg = (char *)"invalid code -- missing end-of-block";
                state->mode = BAD;
                break;
            }

            /* build code tables -- note: do not change the lenbits or distbits
               values here (9 and 6) without reading the comments in inftrees.h
               concerning the ENOUGH constants, which depend on those values */
            state->next = state->codes;
            state->lencode = (const code FAR *)(state->next);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->next),
                                &(state->lenbits), state->work);
            if (ret) {
                strm->msg = (char *)"invalid literal/lengths set";
                state->mode = BAD;
                break;
            }
            state->distcode = (const code FAR *)(state->next);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->next), &(state->distbits), state->work);
            if (ret) {
                strm->msg = (char *)"invalid distances set";
                state->mode = BAD;
                break;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->mode = LEN_;
            if (flush == Z_TREES) goto inf_leave;
                /* fallthrough */
        case LEN_:
            state->mode = LEN;
                /* fallthrough */
        case LEN:
            if (have >= 6 && left >= 258) {
                RESTORE();
                inflate_fast(strm, out);
                LOAD();
                if (state->mode == TYPE)
                    state->back = -1;
                break;
            }
            state->back = 0;
            for (;;) {
                here = state->lencode[BITS(state->lenbits)];
                if ((unsigned)(here.bits) <= bits) break;
                PULLBYTE();
            }
            if (here.op && (here.op & 0xf0) == 0) {
                last = here;
                for (;;) {
                    here = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    if ((unsigned)(last.bits + here.bits) <= bits) break;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
                state->back += last.bits;
            }
            DROPBITS(here.bits);
            state->back += here.bits;
            state->length = (unsigned)here.val;
            if ((int)(here.op) == 0) {
                Tracevv((stderr, here.val >= 0x20 && here.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", here.val));
                state->mode = LIT;
                break;
            }
            if (here.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->back = -1;
                state->mode = TYPE;
                break;
            }
            if (here.op & 64) {
                strm->msg = (char *)"invalid literal/length code";
                state->mode = BAD;
                break;
            }
            state->extra = (unsigned)(here.op) & 15;
            state->mode = LENEXT;
                /* fallthrough */
        case LENEXT:
            if (state->extra) {
                NEEDBITS(state->extra);
                state->length += BITS(state->extra);
                DROPBITS(state->extra);
                state->back += state->extra;
            }
            Tracevv((stderr, "inflate:         length %u\n", state->length));
            state->was = state->length;
            state->mode = DIST;
                /* fallthrough */
        case DIST:
            for (;;) {
                here = state->distcode[BITS(state->distbits)];
                if ((unsigned)(here.bits) <= bits) break;
                PULLBYTE();
            }
            if ((here.op & 0xf0) == 0) {
                last = here;
                for (;;) {
                    here = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    if ((unsigned)(last.bits + here.bits) <= bits) break;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
                state->back += last.bits;
            }
            DROPBITS(here.bits);
            state->back += here.bits;
            if (here.op & 64) {
                strm->msg = (char *)"invalid distance code";
                state->mode = BAD;
                break;
            }
            state->offset = (unsigned)here.val;
            state->extra = (unsigned)(here.op) & 15;
            state->mode = DISTEXT;
                /* fallthrough */
        case DISTEXT:
            if (state->extra) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
                state->back += state->extra;
            }
#ifdef INFLATE_STRICT
            if (state->offset > state->dmax) {
                strm->msg = (char *)"invalid distance too far back";
                state->mode = BAD;
                break;
            }
#endif
            Tracevv((stderr, "inflate:         distance %u\n", state->offset));
            state->mode = MATCH;
                /* fallthrough */
        case MATCH:
            if (left == 0) goto inf_leave;
            copy = out - left;
            if (state->offset > copy) {         /* copy from window */
                copy = state->offset - copy;
                if (copy > state->whave) {
                    if (state->sane) {
                        strm->msg = (char *)"invalid distance too far back";
                        state->mode = BAD;
                        break;
                    }
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
                    Trace((stderr, "inflate.c too far\n"));
                    copy -= state->whave;
                    if (copy > state->length) copy = state->length;
                    if (copy > left) copy = left;
                    left -= copy;
                    state->length -= copy;
                    do {
                        *put++ = 0;
                    } while (--copy);
                    if (state->length == 0) state->mode = LEN;
                    break;
#endif
                }
                if (copy > state->wnext) {
                    copy -= state->wnext;
                    from = state->window + (state->wsize - copy);
                }
                else
                    from = state->window + (state->wnext - copy);
                if (copy > state->length) copy = state->length;
            }
            else {                              /* copy from output */
                from = put - state->offset;
                copy = state->length;
            }
            if (copy > left) copy = left;
            left -= copy;
            state->length -= copy;
            do {
                *put++ = *from++;
            } while (--copy);
            if (state->length == 0) state->mode = LEN;
            break;
        case LIT:
            if (left == 0) goto inf_leave;
            *put++ = (unsigned char)(state->length);
            left--;
            state->mode = LEN;
            break;
        case CHECK:
            if (state->wrap) {
                NEEDBITS(32);
                out -= left;
                strm->total_out += out;
                state->total += out;
                if ((state->wrap & 4) && out)
                    strm->adler = state->check =
                        UPDATE_CHECK(state->check, put - out, out);
                out = left;
                if ((state->wrap & 4) && (
#ifdef GUNZIP
                     state->flags ? hold :
#endif
                     ZSWAP32(hold)) != state->check) {
                    strm->msg = (char *)"incorrect data check";
                    state->mode = BAD;
                    break;
                }
                INITBITS();
                Tracev((stderr, "inflate:   check matches trailer\n"));
            }
#ifdef GUNZIP
            state->mode = LENGTH;
                /* fallthrough */
        case LENGTH:
            if (state->wrap && state->flags) {
                NEEDBITS(32);
                if ((state->wrap & 4) && hold != (state->total & 0xffffffff)) {
                    strm->msg = (char *)"incorrect length check";
                    state->mode = BAD;
                    break;
                }
                INITBITS();
                Tracev((stderr, "inflate:   length matches trailer\n"));
            }
#endif
            state->mode = DONE;
                /* fallthrough */
        case DONE:
            ret = Z_STREAM_END;
            goto inf_leave;
        case BAD:
            ret = Z_DATA_ERROR;
            goto inf_leave;
        case MEM:
            return Z_MEM_ERROR;
        case SYNC:
                /* fallthrough */
        default:
            return Z_STREAM_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    if (state->wsize || (out != strm->avail_out && state->mode < BAD &&
            (state->mode < CHECK || flush != Z_FINISH)))
        if (updatewindow(strm, strm->next_out, out - strm->avail_out)) {
            state->mode = MEM;
            return Z_MEM_ERROR;
        }
    in -= strm->avail_in;
    out -= strm->avail_out;
    strm->total_in += in;
    strm->total_out += out;
    state->total += out;
    if ((state->wrap & 4) && out)
        strm->adler = state->check =
            UPDATE_CHECK(state->check, strm->next_out - out, out);
    strm->data_type = (int)state->bits + (state->last ? 64 : 0) +
                      (state->mode == TYPE ? 128 : 0) +
                      (state->mode == LEN_ || state->mode == COPY_ ? 256 : 0);
    if (((in == 0 && out == 0) || flush == Z_FINISH) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    return ret;
}

int ZEXPORT inflateEnd(z_streamp strm) {
    struct inflate_state FAR *state;
    if (inflateStateCheck(strm))
        return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;
    if (state->window != Z_NULL) ZFREE(strm, state->window);
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    return Z_OK;
}

int ZEXPORT inflateGetDictionary(z_streamp strm, Bytef *dictionary,
                                 uInt *dictLength) {
    struct inflate_state FAR *state;

    /* check state */
    if (inflateStateCheck(strm)) return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;

    /* copy dictionary */
    if (state->whave && dictionary != Z_NULL) {
        zmemcpy(dictionary, state->window + state->wnext,
                state->whave - state->wnext);
        zmemcpy(dictionary + state->whave - state->wnext,
                state->window, state->wnext);
    }
    if (dictLength != Z_NULL)
        *dictLength = state->whave;
    return Z_OK;
}

int ZEXPORT inflateSetDictionary(z_streamp strm, const Bytef *dictionary,
                                 uInt dictLength) {
    struct inflate_state FAR *state;
    unsigned long dictid;
    int ret;

    /* check state */
    if (inflateStateCheck(strm)) return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;
    if (state->wrap != 0 && state->mode != DICT)
        return Z_STREAM_ERROR;

    /* check for correct dictionary identifier */
    if (state->mode == DICT) {
        dictid = adler32(0L, Z_NULL, 0);
        dictid = adler32(dictid, dictionary, dictLength);
        if (dictid != state->check)
            return Z_DATA_ERROR;
    }

    /* copy dictionary to window using updatewindow(), which will amend the
       existing dictionary if appropriate */
    ret = updatewindow(strm, dictionary + dictLength, dictLength);
    if (ret) {
        state->mode = MEM;
        return Z_MEM_ERROR;
    }
    state->havedict = 1;
    Tracev((stderr, "inflate:   dictionary set\n"));
    return Z_OK;
}

int ZEXPORT inflateGetHeader(z_streamp strm, gz_headerp head) {
    struct inflate_state FAR *state;

    /* check state */
    if (inflateStateCheck(strm)) return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;
    if ((state->wrap & 2) == 0) return Z_STREAM_ERROR;

    /* save header structure */
    state->head = head;
    head->done = 0;
    return Z_OK;
}

/*
   Search buf[0..len-1] for the pattern: 0, 0, 0xff, 0xff.  Return when found
   or when out of input.  When called, *have is the number of pattern bytes
   found in order so far, in 0..3.  On return *have is updated to the new
   state.  If on return *have equals four, then the pattern was found and the
   return value is how many bytes were read including the last byte of the
   pattern.  If *have is less than four, then the pattern has not been found
   yet and the return value is len.  In the latter case, syncsearch() can be
   called again with more data and the *have state.  *have is initialized to
   zero for the first call.
 */
local unsigned syncsearch(unsigned FAR *have, const unsigned char FAR *buf,
                          unsigned len) {
    unsigned got;
    unsigned next;

    got = *have;
    next = 0;
    while (next < len && got < 4) {
        if ((int)(buf[next]) == (got < 2 ? 0 : 0xff))
            got++;
        else if (buf[next])
            got = 0;
        else
            got = 4 - got;
        next++;
    }
    *have = got;
    return next;
}

int ZEXPORT inflateSync(z_streamp strm) {
    unsigned len;               /* number of bytes to look at or looked at */
    int flags;                  /* temporary to save header status */
    unsigned long in, out;      /* temporary to save total_in and total_out */
    unsigned char buf[4];       /* to restore bit buffer to byte string */
    struct inflate_state FAR *state;

    /* check parameters */
    if (inflateStateCheck(strm)) return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;
    if (strm->avail_in == 0 && state->bits < 8) return Z_BUF_ERROR;

    /* if first time, start search in bit buffer */
    if (state->mode != SYNC) {
        state->mode = SYNC;
        state->hold >>= state->bits & 7;
        state->bits -= state->bits & 7;
        len = 0;
        while (state->bits >= 8) {
            buf[len++] = (unsigned char)(state->hold);
            state->hold >>= 8;
            state->bits -= 8;
        }
        state->have = 0;
        syncsearch(&(state->have), buf, len);
    }

    /* search available input */
    len = syncsearch(&(state->have), strm->next_in, strm->avail_in);
    strm->avail_in -= len;
    strm->next_in += len;
    strm->total_in += len;

    /* return no joy or set up to restart inflate() on a new block */
    if (state->have != 4) return Z_DATA_ERROR;
    if (state->flags == -1)
        state->wrap = 0;    /* if no header yet, treat as raw */
    else
        state->wrap &= ~4;  /* no point in computing a check value now */
    flags = state->flags;
    in = strm->total_in;  out = strm->total_out;
    inflateReset(strm);
    strm->total_in = in;  strm->total_out = out;
    state->flags = flags;
    state->mode = TYPE;
    return Z_OK;
}

/*
   Returns true if inflate is currently at the end of a block generated by
   Z_SYNC_FLUSH or Z_FULL_FLUSH. This function is used by one PPP
   implementation to provide an additional safety check. PPP uses
   Z_SYNC_FLUSH but removes the length bytes of the resulting empty stored
   block. When decompressing, PPP checks that at the end of input packet,
   inflate is waiting for these length bytes.
 */
int ZEXPORT inflateSyncPoint(z_streamp strm) {
    struct inflate_state FAR *state;

    if (inflateStateCheck(strm)) return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;
    return state->mode == STORED && state->bits == 0;
}

int ZEXPORT inflateCopy(z_streamp dest, z_streamp source) {
    struct inflate_state FAR *state;
    struct inflate_state FAR *copy;
    unsigned char FAR *window;
    unsigned wsize;

    /* check input */
    if (inflateStateCheck(source) || dest == Z_NULL)
        return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)source->state;

    /* allocate space */
    copy = (struct inflate_state FAR *)
           ZALLOC(source, 1, sizeof(struct inflate_state));
    if (copy == Z_NULL) return Z_MEM_ERROR;
    window = Z_NULL;
    if (state->window != Z_NULL) {
        window = (unsigned char FAR *)
                 ZALLOC(source, 1U << state->wbits, sizeof(unsigned char));
        if (window == Z_NULL) {
            ZFREE(source, copy);
            return Z_MEM_ERROR;
        }
    }

    /* copy state */
    zmemcpy((voidpf)dest, (voidpf)source, sizeof(z_stream));
    zmemcpy((voidpf)copy, (voidpf)state, sizeof(struct inflate_state));
    copy->strm = dest;
    if (state->lencode >= state->codes &&
        state->lencode <= state->codes + ENOUGH - 1) {
        copy->lencode = copy->codes + (state->lencode - state->codes);
        copy->distcode = copy->codes + (state->distcode - state->codes);
    }
    copy->next = copy->codes + (state->next - state->codes);
    if (window != Z_NULL) {
        wsize = 1U << state->wbits;
        zmemcpy(window, state->window, wsize);
    }
    copy->window = window;
    dest->state = (struct internal_state FAR *)copy;
    return Z_OK;
}

int ZEXPORT inflateUndermine(z_streamp strm, int subvert) {
    struct inflate_state FAR *state;

    if (inflateStateCheck(strm)) return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
    state->sane = !subvert;
    return Z_OK;
#else
    (void)subvert;
    state->sane = 1;
    return Z_DATA_ERROR;
#endif
}

int ZEXPORT inflateValidate(z_streamp strm, int check) {
    struct inflate_state FAR *state;

    if (inflateStateCheck(strm)) return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;
    if (check && state->wrap)
        state->wrap |= 4;
    else
        state->wrap &= ~4;
    return Z_OK;
}

long ZEXPORT inflateMark(z_streamp strm) {
    struct inflate_state FAR *state;

    if (inflateStateCheck(strm))
        return -(1L << 16);
    state = (struct inflate_state FAR *)strm->state;
    return (long)(((unsigned long)((long)state->back)) << 16) +
        (state->mode == COPY ? state->length :
            (state->mode == MATCH ? state->was - state->length : 0));
}

unsigned long ZEXPORT inflateCodesUsed(z_streamp strm) {
    struct inflate_state FAR *state;
    if (inflateStateCheck(strm)) return (unsigned long)-1;
    state = (struct inflate_state FAR *)strm->state;
    return (unsigned long)(state->next - state->codes);
}
