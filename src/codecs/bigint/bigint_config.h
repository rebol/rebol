/*
 * Copyright (c) 2007, Cameron Rich
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * * Neither the name of the axTLS project nor the names of its contributors 
 *   may be used to endorse or promote products derived from this software 
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * BigInt Options
 */

/*
		CONFIG_BIGINT_DEBUG
		Enable diagnostics. Most of the extra size in this mode is
		due to the storage of various strings that are used.
*/
#undef CONFIG_BIGINT_DEBUG

/*
		CONFIG_BIGINT_CLASSICAL
        Classical uses standard division. It has no limitations and is
        theoretically the slowest due to the divisions used. For this particular
        implementation it is surprisingly quite fast. 
*/
#undef CONFIG_BIGINT_CLASSICAL

/*
		CONFIG_BIGINT_MONTGOMERY
        Montgomery uses simple addition and multiplication to achieve its
        performance.  It has the limitation that 0 <= x, y < m, and so is not 
        used when CRT is active. 
*/
#undef CONFIG_BIGINT_MONTGOMERY

/*
		CONFIG_BIGINT_BARRETT
        Barrett performs expensive precomputation before reduction and partial
        multiplies for computational speed.

        It is about 40% faster than Classical/Montgomery with the expense of
        about 2kB, and so this option is normally selected. 
*/
#define CONFIG_BIGINT_BARRETT 1

/*
		CONFIG_BIGINT_CRT
        Uses a number of extra coefficients from the private key to improve the
        performance of a decryption. This feature is one of the most 
        significant performance improvements (it reduces a decryption time by 
        over 3 times). 
		This option should be selected.
*/
#define CONFIG_BIGINT_CRT 1

/*
		CONFIG_BIGINT_KARATSUBA
        Uses 3 multiplications (plus a number of additions/subtractions) 
        instead of 4. Multiplications are O(N^2) but addition/subtraction 
        is O(N) hence for large numbers is beneficial. For this project, the 
        effect was only useful for 4096 bit keys (for 32 bit processors). For
        8 bit processors this option might be a possibility.
        It costs about 2kB to enable it. 
*/
#undef CONFIG_BIGINT_KARATSUBA

/*
		MUL_KARATSUBA_THRESH
        The minimum number of components needed before Karasuba muliplication
        is used.
 
        This is very dependent on the speed/implementation of bi_add()/
        bi_subtract(). There is a bit of trial and error here and will be
        at a different point for different architectures. 
*/
#define MUL_KARATSUBA_THRESH

/*
		SQU_KARATSUBA_THRESH
        The minimum number of components needed before Karatsuba squaring
        is used.
 
        This is very dependent on the speed/implementation of bi_add()/
        bi_subtract(). There is a bit of trial and error here and will be
        at a different point for different architectures. 
*/
#define SQU_KARATSUBA_THRESH

/*
		CONFIG_BIGINT_SLIDING_WINDOW
        Allow Sliding-Window Exponentiation to be used.
        Potentially processes more than 1 bit at a time when doing 
        exponentiation. The sliding-window technique reduces the number of 
        precomputations compared to other precomputed techniques.
        It results in a considerable performance improvement with it enabled
        (it halves the decryption time) and so should be selected. 
*/
//NOTE: the sliding window optimization doesn't work with Diffie-Hellman from some reason. Needs to be checked why --Cyphre
//#define CONFIG_BIGINT_SLIDING_WINDOW 1
#undef CONFIG_BIGINT_SLIDING_WINDOW

/*
		CONFIG_BIGINT_SQUARE
        Allow squaring to be used instead of a multiplication. It uses
        1/2 of the standard multiplies to obtain its performance.  
        It gives a 20% speed improvement overall and so should be selected.
*/
#define CONFIG_BIGINT_SQUARE 1

/*
		CONFIG_BIGINT_CHECK_ON
        This is used when developing bigint algorithms. It performs a sanity
        check on all operations at the expense of speed.
        This option is only selected when developing and should normally be
        turned off.
*/
#undef CONFIG_BIGINT_CHECK_ON

/*
	CONFIG_INTEGER_32BIT
	The native integer size is 32 bits or higher.
*/
#define CONFIG_INTEGER_32BIT 1

/*
	CONFIG_INTEGER_16BIT
	The native integer size is 16 bits.
*/
#undef CONFIG_INTEGER_16BIT

/*
	CONFIG_INTEGER_8BIT
	The native integer size is 8 bits.
*/
#undef CONFIG_INTEGER_8BIT
