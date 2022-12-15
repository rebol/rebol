/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2022 Rebol Open Source Contributors
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
**  Module:  u-lzw.c
**  Summary: LZW compression algorithm
**  Section: natives
**  Author:  Oldes, David Bryant
**  Notes:
**		This code was ported from the original David Bryant's code
**		https://github.com/dbry/lzw-ab
**
***********************************************************************/
// Original credits:
////////////////////////////////////////////////////////////////////////////
//                            **** LZW-AB ****                            //
//               Adjusted Binary LZW Compressor/Decompressor              //
//                  Copyright (c) 2016-2020 David Bryant                  //
//                           All Rights Reserved                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

#include "sys-core.h"
#if !defined(REBOL_OPTIONS_FILE)
#include "opt-config.h"
#else
#include REBOL_OPTIONS_FILE
#endif

#ifdef INCLUDE_LZW

/* This library implements the LZW general-purpose data compression algorithm.
 * The algorithm was originally described as a hardware implementation by
 * Terry Welsh here:
 *
 *   Welch, T.A. “A Technique for High-Performance Data Compression.”
 *   IEEE Computer 17,6 (June 1984), pp. 8-19.
 *
 * Since then there have been enumerable refinements and variations on the
 * basic technique, and this implementation is no different. The target of
 * the present implementation is embedded systems, and so emphasis was placed
 * on simplicity, fast execution, and minimal RAM usage.
 * 
 * This is a streaming compressor in that the data is not divided into blocks
 * and no context information like dictionaries or Huffman tables are sent
 * ahead of the compressed data (except for one byte to signal the maximum
 * bit depth). This limits the maximum possible compression ratio compared to
 * algorithms that significantly preprocess the data, but with the help of
 * some enhancements to the LZW algorithm (described below) it is able to
 * compress better than the UNIX "compress" utility (which is also LZW) and
 * is in fact closer to and sometimes beats the compression level of "gzip".
 *
 * The symbols are stored in "adjusted binary" which provides somewhat better
 * compression, with virtually no speed penalty, compared to the fixed word
 * sizes normally used. These are sometimes called "phased-in" binary codes
 * and their use in LZW is described here:
 *
 *   R. N. Horspool, "Improving LZW (data compression algorithm)", Data
 *   Compression Conference, pp. 332-341, 1991.
 *
 * Earlier versions of this compressor would reset as soon as the dictionary
 * became full to ensure good performance on heterogenous data (such as tar
 * files or executable images). While trivial to implement, this is not
 * particularly efficient with homogeneous data (or in general) because we
 * spend a lot of time sending short symbols where the compression is poor.
 *
 * This newer version utilizes a technique such that once the dictionary is
 * full, we restart at the beginning and recycle only those codes that were
 * seen only once. We know this because they are not referenced by longer
 * strings, and are easy to replace in the dictionary for the same reason.
 * Since they have only been seen once it's also more likely that we will
 * be replacing them with a more common string, and this is especially
 * true if the data characteristics are changing.
 *
 * Replacing string codes in this manner has the interesting side effect that
 * some older shorter strings that the removed strings were based on will
 * possibly become unreferenced themselves and be recycled on the next pass.
 * In this way, the entire dictionary constantly "churns" based on the
 * incoming stream, thereby improving and adapting to optimal compression.
 *
 * Even with this technique there is still a possibility that a sudden change
 * in the data characteristics will appear, resulting in significant negative
 * compression (up to 100% for 16-bit codes). To detect this case we generate
 * an exponentially decaying average of the current compression ratio and reset
 * when this hits about 1.06, which limits worst case inflation to about 8%.
 *
 * The maximum symbol size is configurable on the encode side (from 9 bits to
 * 16 bits) and determines the RAM footprint required by both sides and, to a
 * large extent, the compression performance. This information is communicated
 * to the decoder in the first stream byte so that it can allocate accordingly.
 * The RAM requirements are as follows:
 *
 *    maximum    encoder RAM   decoder RAM
 *  symbol size  requirement   requirement
 * -----------------------------------------
 *     9-bit      4096 bytes    2368 bytes
 *    10-bit      8192 bytes    4992 bytes
 *    11-bit     16384 bytes   10240 bytes
 *    12-bit     32768 bytes   20736 bytes
 *    13-bit     65536 bytes   41728 bytes
 *    14-bit    131072 bytes   83712 bytes
 *    15-bit    262144 bytes  167680 bytes
 *    16-bit    524288 bytes  335616 bytes
 *
 * This implementation uses malloc(), but obviously an embedded version could
 * use static arrays instead if desired (assuming that the maxbits was
 * controlled outside).
 */

#define NULL_CODE       65535   // indicates a NULL prefix (must be unsigned short)
#define CLEAR_CODE      256     // code to flush dictionary and restart decoder
#define FIRST_STRING    257     // code of first dictionary string

/* This macro determines the number of bits required to represent the given value,
 * not counting the implied MSB. For GNU C it will use the provided built-in,
 * otherwise a comparison tree is employed. Note that in the non-GNU case, only
 * values up to 65535 (15 bits) are supported.
 */

#ifdef __GNUC__
#define CODE_BITS(n) (31 - __builtin_clz(n))
#else
#define CODE_BITS(n) ((n) < 4096 ?                                      \
            ((n) < 1024  ? 8  + ((n) >= 512)  : 10 + ((n) >= 2048)) :   \
            ((n) < 16384 ? 12 + ((n) >= 8192) : 14 + ((n) >= 32768)))
#endif

/* This macro writes the adjusted-binary symbol "code" given the maximum
 * symbol "maxcode". A macro is used here just to avoid the duplication in
 * the lzw_compress() function. The idea is that if "maxcode" is not one
 * less than a power of two (which it rarely will be) then this code can
 * often send fewer bits that would be required with a fixed-sized code.
 *
 * For example, the first code we send will have a "maxcode" of 257, so
 * every "code" would normally consume 9 bits. But with adjusted binary we
 * can actually represent any code from 0 to 253 with just 8 bits -- only
 * the 4 codes from 254 to 257 take 9 bits.
 */

#define WRITE_CODE(code,maxcode) do {                               \
    unsigned int code_bits = CODE_BITS (maxcode);                   \
    unsigned int extras = (2 << code_bits) - (maxcode) - 1;         \
    if ((code) < extras) {                                          \
        shifter |= ((code) << bits);                                \
        bits += code_bits;                                          \
    }                                                               \
    else {                                                          \
        shifter |= ((((code) + extras) >> 1) << bits);              \
        bits += code_bits;                                          \
        shifter |= ((((code) + extras) & 1) << bits++);             \
    }                                                               \
    do { (*dst)(shifter,dstctx); shifter >>= 8;                     \
        output_bytes += 256;                                        \
    } while ((bits -= 8) >= 8);                                     \
} while (0)

/* LZW compression function. Bytes (8-bit) are read and written through callbacks and the
 * "maxbits" parameter specifies the maximum symbol size (9-16), which in turn determines
 * the RAM requirement and, to a large extent, the level of compression achievable. A return
 * value of EOF from the "src" callback terminates the compression process. A non-zero return
 * value indicates one of the two possible errors -- bad "maxbits" param or failed malloc().
 * There are contexts (void pointers) that are passed to the callbacks to easily facilitate
 * multiple instances of the compression operation (but simple applications can ignore these).
 */

typedef struct {
    unsigned short first_reference, next_reference, back_reference;
    unsigned char terminator;
} encoder_entry_t;

int lzw_compress (void (*dst)(int,void*), void *dstctx, int (*src)(void*), void *srcctx, int maxbits)
{
    unsigned int maxcode = FIRST_STRING, next_string = FIRST_STRING, prefix = NULL_CODE, total_codes;
    unsigned int dictionary_full = 0, available_entries, max_available_entries, max_available_code;
    unsigned int input_bytes = 65536, output_bytes = 65536;
    unsigned int shifter = 0, bits = 0;
    encoder_entry_t *dictionary;
    int c;
	REBSER *mem;

    if (maxbits < 9 || maxbits > 16)    // check for valid "maxbits" setting
        return 1;

    // based on the "maxbits" parameter, compute total codes and allocate dictionary storage

    total_codes = 1 << maxbits;

	mem = Make_Binary((total_codes * sizeof(encoder_entry_t)) - 1); // using -1, because Make_Binary adds 1!
	dictionary = (encoder_entry_t *)BIN_HEAD(mem);
    max_available_entries = total_codes - FIRST_STRING - 1;
    max_available_code = total_codes - 2;

    if (!dictionary)
        return 1;                       // failed malloc()

    // clear the dictionary

    available_entries = max_available_entries;
    memset (dictionary, 0, 256 * sizeof (encoder_entry_t));

    (*dst)(maxbits - 9, dstctx);    // first byte in output stream indicates the maximum symbol bits

    // This is the main loop where we read input bytes and compress them. We always keep track of the
    // "prefix", which represents a pending byte (if < 256) or string entry (if >= FIRST_STRING) that
    // has not been sent to the decoder yet. The output symbols are kept in the "shifter" and "bits"
    // variables and are sent to the output every time 8 bits are available (done in the macro).

    while ((c = (*src)(srcctx)) != NO_RESULT) {
        unsigned int cti;                   // coding table index

        input_bytes += 256;

        if (prefix == NULL_CODE) {          // this only happens the very first byte when we don't yet have a prefix
            prefix = c;
            continue;
        }

        memset (dictionary + next_string, 0, sizeof (encoder_entry_t));

        if ((cti = dictionary [prefix].first_reference)) {          // if any longer strings are built on the current prefix...
            while (1)
                if (dictionary [cti].terminator == c) {             // we found a matching string, so we just update the prefix
                    prefix = cti;                                   // to that string and continue without sending anything
                    break;
                }
                else if (!dictionary [cti].next_reference) {        // this string did not match the new character and
                    dictionary [cti].next_reference = next_string;  // there aren't any more, so we'll add a new string,
                                                                    // point to it with "next_reference", and also make the
                    dictionary [next_string].back_reference = cti;  // "back_reference" which is used for recycling entries
                    cti = 0;
                    break;
                }
                else
                    cti = dictionary [cti].next_reference;          // there are more possible matches to check, so loop back
        }
        else {                                                      // no longer strings are based on the current prefix, so now
            dictionary [prefix].first_reference = next_string;      // the current prefix plus the new byte will be the next string
            dictionary [next_string].back_reference = prefix;       // also make the back_reference used for recycling
            if (prefix >= FIRST_STRING) available_entries--;        // the codes 0-255 are never available for recycling
        }

        // If "cti" is zero, we could not simply extend our "prefix" to a longer string because we did not find a
        // dictionary match, so we send the symbol representing the current "prefix" and add the new string to the
        // dictionary. Since the current byte "c" was not included in the prefix, that now becomes our new prefix.

        if (!cti) {
            WRITE_CODE (prefix, maxcode);               // send symbol for current prefix (0 to maxcode-1)
            dictionary [next_string].terminator = c;    // newly created string has current byte as the terminator
            prefix = c;                                 // current byte also becomes new prefix for next string

            // If the dictionary is not full yet, we bump the maxcode and next_string and check to see if the
            // dictionary is now full. If it is we set the dictionary_full flag and leave maxcode set to two
            // less than total_codes because every string entry is now available for matching, but the actual
            // maximum code is reserved for EOF.

            if (!dictionary_full) {
                dictionary_full = (++next_string > max_available_code);
                maxcode++;
            }

            // If the dictionary is full we look for an entry to recycle starting at next_string (the one we
            // just created or recycled) plus one (with check for wrap check). We know there is one because at
            // a minimum the string we just added. This also takes care of removing the entry to be recycled
            // (which is possible/easy because no longer strings have been based on it).

            if (dictionary_full) {
                for (next_string++; next_string <= max_available_code || (next_string = FIRST_STRING); next_string++)
                    if (!dictionary [next_string].first_reference)
                        break;

                cti = dictionary [next_string].back_reference;  // dictionary [cti] references the entry we're
                                                                // trying to recycle (either as a first or a next)

                if (dictionary [cti].first_reference == next_string) {
                    dictionary [cti].first_reference = dictionary [next_string].next_reference;

                    // if we just cleared a first reference, and that string is not 0-255,
                    // then that's a newly available entry
                    if (!dictionary [cti].first_reference && cti >= FIRST_STRING)
                        available_entries++;
                }
                else if (dictionary [cti].next_reference == next_string)    // fixup a "next_reference"
                    dictionary [cti].next_reference = dictionary [next_string].next_reference;

                // If the entry we're recycling had a next reference, then update the back reference
                // so it's completely out of the chain. Of course we know it didn't have a first
                // reference because then we wouldn't be recycling it.

                if (dictionary [next_string].next_reference)
                    dictionary [dictionary [next_string].next_reference].back_reference = cti;

                // This check is technically not needed because there will always be an available entry
                // (the last string we added at a minimum) but we don't want to get in a situation where
                // we only have a few entries that we're cycling though. I pulled the limits (16 entries
                // or 1% of total) out of a hat.

                if (available_entries < 16 || available_entries * 100 < max_available_entries) {
                    // clear the dictionary and reset the byte counters -- basically everything starts over
                    // except that we keep the last pending "prefix" (which, of course, was never sent)

                    WRITE_CODE (CLEAR_CODE, maxcode);
                    memset (dictionary, 0, 256 * sizeof (encoder_entry_t));
                    available_entries = max_available_entries;
                    next_string = maxcode = FIRST_STRING;
                    input_bytes = output_bytes = 65536;
                    dictionary_full = 0;
                }
            }

            // This is similar to the above check, except that it's used whether the dictionary is full or not.
            // It uses an exponentially decaying average of the current compression ratio, so it can terminate
            // very early if the incoming data is uncompressible or it can terminate any later time that the
            // dictionary no longer compresses the incoming stream.

            if (output_bytes > input_bytes + (input_bytes >> 4)) {
                WRITE_CODE (CLEAR_CODE, maxcode);
                memset (dictionary, 0, 256 * sizeof (encoder_entry_t));
                available_entries = max_available_entries;
                next_string = maxcode = FIRST_STRING;
                input_bytes = output_bytes = 65536;
                dictionary_full = 0;
            }
            else {
                output_bytes -= output_bytes >> 8;
                input_bytes -= input_bytes >> 8;
            }
        }
    }

    // we're done with input, so if we've received anything we still need to send that pesky pending prefix...

    if (prefix != NULL_CODE) {
        WRITE_CODE (prefix, maxcode);

        if (!dictionary_full)
            maxcode++;
    }

    WRITE_CODE (maxcode, maxcode);  // the maximum possible code is always reserved for our END_CODE

    if (bits)                       // finally, flush any pending bits from the shifter
        (*dst)(shifter, dstctx);

	Free_Series(mem);
    return 0;
}

/* LZW decompression function. Bytes (8-bit) are read and written through callbacks. The
 * "maxbits" parameter is read as the first byte in the stream and controls how much memory
 * is allocated for decoding. A return value of EOF from the "src" callback terminates the
 * decompression process (although this should not normally occur). A non-zero return value
 * indicates an error, which in this case can be a bad "maxbits" read from the stream, a
 * failed malloc(), or if an EOF is read from the input stream before the decompression
 * terminates naturally with END_CODE. There are contexts (void pointers) that are passed
 * to the callbacks to easily facilitate multiple instances of the decompression operation
 * (but simple applications can ignore these).
 */

typedef struct {
    unsigned char terminator, extra_references;
    unsigned short prefix;
} decoder_entry_t;

int lzw_decompress (void (*dst)(int,void*), void *dstctx, int (*src)(void*), void *srcctx)
{
    unsigned int maxcode = FIRST_STRING, next_string = FIRST_STRING - 1, prefix = CLEAR_CODE;
    unsigned int dictionary_full = 0, max_available_code, total_codes;
    unsigned int shifter = 0, bits = 0, read_byte, i;
    unsigned char *reverse_buffer, *referenced;
	REBSER *mem;

    decoder_entry_t *dictionary;

    if ((read_byte = ((*src)(srcctx))) == NO_RESULT || (read_byte & 0xf8))  //sanitize first byte
        return 1;

    // based on the "maxbits" parameter, compute total codes and allocate dictionary storage

    total_codes = 512 << (read_byte & 0x7);
    max_available_code = total_codes - 2;


	mem = Make_Binary((total_codes * sizeof(decoder_entry_t)) + (total_codes - 256) + (total_codes / 8) - 1);

    dictionary = (decoder_entry_t *)BIN_HEAD(mem);
    reverse_buffer = BIN_SKIP(mem, (total_codes * sizeof(decoder_entry_t)));
    referenced = BIN_SKIP(mem, (total_codes * sizeof(decoder_entry_t)) + (total_codes - 256));  // bitfield indicating code is referenced at least once

    // Note that to implement the dictionary entry recycling we have to keep track of how many
    // longer strings are based on each string in the dictionary. This can be between 0 (no
    // references) to 256 (every possible next byte), but unfortunately that's one more value
    // than what can be stored in a byte. The solution is to have a single bit for each entry
    // indicating any references (i.e., the code cannot be recycled) and an additional byte
    // in the dictionary entry struct counting the "extra" references (beyond one).

    if (!reverse_buffer || !dictionary)         // check for malloc() failure
        return 1;

    for (i = 0; i < 256; ++i) {                 // these never change
        dictionary [i].prefix = NULL_CODE;
        dictionary [i].terminator = i;
    }

    // This is the main loop where we read input symbols. The values range from 0 to the code value
    // of the "next" string in the dictionary (although the actual "next" code cannot be used yet,
    // and so we reserve that code for the END_CODE). Note that receiving an EOF from the input
    // stream is actually an error because we should have gotten the END_CODE first.

    while (1) {
        unsigned int code_bits = CODE_BITS (maxcode), code;
        unsigned int extras = (2 << code_bits) - maxcode - 1;

        do {
            if ((read_byte = ((*src)(srcctx))) == NO_RESULT) {
				Free_Series(mem);
                return 1;
            }

            shifter |= read_byte << bits;
        } while ((bits += 8) < code_bits);

        // first we assume the code will fit in the minimum number of required bits

        code = shifter & ((1 << code_bits) - 1);
        shifter >>= code_bits;
        bits -= code_bits;

        // but if code >= extras, then we need to read another bit to calculate the real code
        // (this is the "adjusted binary" part)

        if (code >= extras) {
            if (!bits) {
                if ((read_byte = ((*src)(srcctx))) == NO_RESULT) {
					Free_Series(mem);
                    return 1;
                }

                shifter = read_byte;
                bits = 8;
            }

            code = (code << 1) - extras + (shifter & 1);
            shifter >>= 1;
            bits--;
        }

        if (code == maxcode)                // sending the maximum code is reserved for the end of the file
            break;
        else if (code == CLEAR_CODE) {      // otherwise check for a CLEAR_CODE to start over early
            next_string = FIRST_STRING - 1;
            maxcode = FIRST_STRING;
            dictionary_full = 0;
        }
        else if (prefix == CLEAR_CODE) {    // this only happens at the first symbol which is always sent
            (*dst)(code, dstctx);           // literally and becomes our initial prefix
            next_string++;
            maxcode++;
        }
        // Otherwise we have a valid prefix so we step through the string from end to beginning storing the
        // bytes in the "reverse_buffer", and then we send them out in the proper order. One corner-case
        // we have to handle here is that the string might be the same one that is actually being defined
        // now (code == next_string).
        else {
            unsigned int cti = (code == next_string) ? prefix : code;
            unsigned char *rbp = reverse_buffer, c;

            do {
                *rbp++ = dictionary [cti].terminator;
                if (rbp == reverse_buffer + total_codes - 256) {
					Free_Series(mem);
                    return 1;
                }
            } while ((cti = dictionary [cti].prefix) != NULL_CODE);

            c = *--rbp;     // the first byte in this string is the terminator for the last string, which is
                            // the one that we'll create a new dictionary entry for this time

            do      // send string in corrected order (except for the terminator which we don't know yet)
                (*dst)(*rbp, dstctx);
            while (rbp-- != reverse_buffer);

            if (code == next_string) {
                (*dst)(c,dstctx);
            }

            // This should always execute (the conditional is to catch corruptions) and is where we add a new string to
            // the dictionary, either at the end or elsewhere when we are "recycling" entries that were never referenced

            if (next_string >= FIRST_STRING && next_string < total_codes) {
                if (referenced [prefix >> 3] & (1 << (prefix & 7)))     // increment reference count on prefix
                    dictionary [prefix].extra_references++;
                else
                    referenced [prefix >> 3] |= 1 << (prefix & 7);

                dictionary [next_string].prefix = prefix;       // now update the next dictionary entry with the new string
                dictionary [next_string].terminator = c;        // (but we're always one behind, so it's not the string just sent)
                dictionary [next_string].extra_references = 0;  // newly created string has not been referenced
                referenced [next_string >> 3] &= ~(1 << (next_string & 7));
            }

            // If the dictionary is not full yet, we bump the maxcode and next_string and check to see if the
            // dictionary is now full. If it is we set the dictionary_full flag and set next_string back to the
            // beginning of the dictionary strings to start recycling them. Note that then maxcode will remain
            // two less than total_codes because every string entry is available for matching, and the actual
            // maximum code is reserved for EOF.

            if (!dictionary_full) {
                maxcode++;

                if (++next_string > max_available_code) {
                    dictionary_full = 1;
                    maxcode--;
                }
            }

            // If the dictionary is full we look for an entry to recycle starting at next_string (the one we
            // created or recycled) plus one. We know there is one because at a minimum the string we just added
            // has not been referenced). This also takes care of removing the entry to be recycled (which is
            // possible/easy because no longer strings have been based on it).

            if (dictionary_full) {
                for (next_string++; next_string <= max_available_code || (next_string = FIRST_STRING); next_string++)
                    if (!(referenced [next_string >> 3] & (1 << (next_string & 7))))
                        break;

                if (dictionary [dictionary [next_string].prefix].extra_references)
                    dictionary [dictionary [next_string].prefix].extra_references--;
                else
                    referenced [dictionary [next_string].prefix >> 3] &= ~(1 << (dictionary [next_string].prefix & 7));
            }
        }

        prefix = code;      // the code we just received becomes the prefix for the next dictionary string entry
                            // (which we'll create once we find out the terminator)
    }

	Free_Series(mem);
    return 0;
}

typedef struct {
	REBSER *buffer;
	REBCNT index;  // current buffer position
	REBCNT end;    // for early exit
} streamer;

static int read_buff(void *ctx) {
	streamer *stream = ctx;
	REBINT value;

	if (stream->index < stream->end) {
		value = BIN_DATA(stream->buffer)[stream->index++];
	}
	else
		value = NO_RESULT;

	return value;
}

static void write_buff(int value, void *ctx) {
	streamer *stream = ctx;

	if (stream->index >= stream->end || value == NO_RESULT) {
		return;
	}
	if (stream->index >= stream->buffer->rest) {
		// update the length, before expansion!
		SERIES_TAIL(stream->buffer) = stream->index;
		Expand_Series(stream->buffer, AT_TAIL, stream->buffer->rest);
	}
	BIN_DATA(stream->buffer)[stream->index++] = (REBYTE)value;
}


/***********************************************************************
**
*/  REBSER *CompressLzw(REBSER *input, REBINT index, REBCNT in_len, REBINT level)
/*
**      Compress a binary (only) using LZW compression.
**      data
**      /part
**      length
**
***********************************************************************/
{
    REBINT  err, maxbits = 16;
	streamer reader, writer;

	if (level >= 1 && level <= 7) {
		maxbits = 8 + level;
	}
	else if (level < 1)
		maxbits = 9;

	CLEARS(&reader);
	CLEARS(&writer);
	reader.buffer = input;
	reader.index = index;
	reader.end = index + in_len;
	writer.buffer = Make_Binary(in_len >> 1);
	writer.end = NO_LIMIT;

	err = lzw_compress(write_buff, &writer, read_buff, &reader, maxbits);
    if (err) {
		Free_Series(writer.buffer);
        SET_INTEGER(DS_RETURN, err);
        Trap1(RE_BAD_PRESS, DS_RETURN); //!!!provide error string descriptions
    }
    SERIES_TAIL(writer.buffer) = writer.index;
    return writer.buffer;
}

/***********************************************************************
**
*/  REBSER *DecompressLzw(REBSER *input, REBCNT index, REBINT in_len, REBCNT limit)
/*
**      Decompress a binary (only).
**
***********************************************************************/
{
	REBU64 out_len;
	REBINT err;
	streamer reader, writer;

	CLEARS(&reader);
	CLEARS(&writer);
	reader.buffer = input;
	reader.index = index;
	reader.end = in_len + index;
	
	if (limit > 0) {
		writer.end = out_len = limit;
	}
	else {
		out_len = (REBU64)in_len << 1;
		writer.end = NO_LIMIT;
	}
	writer.buffer = Make_Binary(out_len);

	err = lzw_decompress(write_buff, &writer, read_buff, &reader);
	if (err) {
		SET_INTEGER(DS_RETURN, err);
		Trap1(RE_BAD_PRESS, DS_RETURN); //!!!provide error string descriptions
	}
	SET_STR_END(writer.buffer, writer.index);
	SERIES_TAIL(writer.buffer) = writer.index;
	return writer.buffer;
}

#endif //INCLUDE_LZW