/* zconf.h -- configuration of the zlib compression library
 * Copyright (C) 1995-1998 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */
#ifndef _ZCONF_H
#define _ZCONF_H
#define Z_PREFIX
/*
 * If you *really* need a unique prefix for all types and library functions,
 * compile with -DZ_PREFIX. The "standard" zlib should be compiled without it.
 */
#ifdef Z_PREFIX
#  define deflateInit_	Z_deflateInit_
#  define deflate	Z_deflate
#  define deflateEnd	Z_deflateEnd
#  define inflateInit_ 	Z_inflateInit_
#  define inflate	Z_inflate
#  define inflateEnd	Z_inflateEnd
#  define deflateInit2_	Z_deflateInit2_
#  define deflateSetDictionary Z_deflateSetDictionary
#  define deflateCopy	Z_deflateCopy
#  define deflateReset	Z_deflateReset
#  define deflateParams	Z_deflateParams
#  define inflateInit2_	Z_inflateInit2_
#  define inflateSetDictionary Z_inflateSetDictionary
#  define inflateSync	Z_inflateSync
#  define inflateSyncPoint Z_inflateSyncPoint
#  define inflateReset	Z_inflateReset
#  define compress	Z_compress
#  define compress2	Z_compress2
#  define uncompress	Z_uncompress
#  define adler32	Z_adler32
#  define crc32		Z_crc32
#  define get_crc_table Z_get_crc_table
# define _dist_code	Z__dist_code
# define _length_code	Z__length_code
# define _tr_align	Z__tr_align
# define _tr_flush_block	Z__tr_flush_block
# define _tr_init	Z__tr_init
# define _tr_stored_block	Z__tr_stored_block
# define base_dist	Z_base_dist
# define base_length	Z_base_length
# define bi_flush	Z_bi_flush
# define bi_reverse	Z_bi_reverse
# define bi_windup	Z_bi_windup
# define bl_order	Z_bl_order
# define border	Z_border
# define build_bl_tree	Z_build_bl_tree
# define build_tree	Z_build_tree
# define compress_block	Z_compress_block
# define configuration_table	Z_configuration_table
# define copy_block	Z_copy_block
# define cpdext	Z_cpdext
# define cpdist	Z_cpdist
# define cplens	Z_cplens
# define cplext	Z_cplext
# define deflate_slow	Z_deflate_slow
# define deflate_stored	Z_deflate_stored
# define extra_blbits	Z_extra_blbits
# define extra_dbits	Z_extra_dbits
# define extra_lbits	Z_extra_lbits
# define fill_window	Z_fill_window
# define fixed_bd	Z_fixed_bd
# define fixed_bl	Z_fixed_bl
# define fixed_built	Z_fixed_built
# define fixed_mem	Z_fixed_mem
# define fixed_td	Z_fixed_td
# define fixed_tl	Z_fixed_tl
# define flush_pending	Z_flush_pending
# define gen_bitlen	Z_gen_bitlen
# define gen_codes	Z_gen_codes
# define huft_build	Z_huft_build
# define inflate_blocks	Z_inflate_blocks
# define inflate_blocks_free	Z_inflate_blocks_free
# define inflate_blocks_new	Z_inflate_blocks_new
# define inflate_blocks_reset	Z_inflate_blocks_reset
# define inflate_codes	Z_inflate_codes
# define inflate_codes_free	Z_inflate_codes_free
# define inflate_codes_new	Z_inflate_codes_new
# define inflate_flush	Z_inflate_flush
# define inflate_mask	Z_inflate_mask
# define inflate_trees_bits	Z_inflate_trees_bits
# define inflate_trees_dynamic	Z_inflate_trees_dynamic
# define inflate_trees_fixed	Z_inflate_trees_fixed
# define init_block	Z_init_block
# define lm_init	Z_lm_init
# define longest_match	Z_longest_match
# define pqdownheap	Z_pqdownheap
# define putShortMSB	Z_putShortMSB
# define read_buf	Z_read_buf
# define scan_tree	Z_scan_tree
# define send_all_trees	Z_send_all_trees
# define send_bits	Z_send_bits
# define send_tree	Z_send_tree
# define set_data_type	Z_set_data_type
# define static_bl_desc	Z_static_bl_desc
# define static_d_desc	Z_static_d_desc
# define static_dtree	Z_static_dtree
# define static_l_desc	Z_static_l_desc
# define static_ltree	Z_static_ltree
# define tr_static_init	Z_tr_static_init
# define z_errmsg	Z_z_errmsg
# define zcalloc	Z_zcalloc
# define zcfree	Z_zcfree
#  define Byte		Z_Byte
#  define uInt		Z_uInt
#  define uLong		Z_uLong
#  define Bytef	        Z_Bytef
#  define charf		Z_charf
#  define intf		Z_intf
#  define uIntf		Z_uIntf
#  define uLongf	Z_uLongf
#  define voidpf	Z_voidpf
#  define voidp		Z_voidp
#endif
#if (defined(_WIN32) || defined(__WIN32__)) && !defined(WIN32)
#  define WIN32
#endif
#if defined(__GNUC__) || defined(WIN32) || defined(__386__) || defined(i386)
#  ifndef __32BIT__
#    define __32BIT__
#  endif
#endif
#if defined(__MSDOS__) && !defined(MSDOS)
#  define MSDOS
#endif
/*
 * Compile with -DMAXSEG_64K if the alloc function cannot allocate more
 * than 64k bytes at a time (needed on systems with 16-bit int).
 */
/* #if defined(MSDOS) && !defined(__32BIT__) */
/* #  define MAXSEG_64K */
/* #endif */
/* #ifdef MSDOS */
/* #  define UNALIGNED_OK */
/* #endif */
#if (defined(MSDOS) || defined(_WINDOWS) || defined(WIN32))  && !defined(STDC)
#  define STDC
#endif
#if defined(__STDC__) || defined(__cplusplus) || defined(__OS2__)
#  ifndef STDC
#    define STDC
#  endif
#endif
#ifndef STDC
#  ifndef const /* cannot use !defined(STDC) && !defined(const) on Mac */
#    define const
#  endif
#endif
/* Some Mac compilers merge all .h files incorrectly: */
#if defined(__MWERKS__) || defined(applec) ||defined(THINK_C) ||defined(__SC__)
#  define NO_DUMMY_DECL
#endif
/* Borland C incorrectly complains about missing returns: */
#if defined(__BORLANDC__)
#  define NEED_DUMMY_RETURN
#endif
/* Maximum value for memLevel in deflateInit2 */
#ifndef MAX_MEM_LEVEL
#  ifdef MAXSEG_64K
#    define MAX_MEM_LEVEL 8
#  else
#    define MAX_MEM_LEVEL 9
#  endif
#endif
/* Maximum value for windowBits in deflateInit2 and inflateInit2.
 * WARNING: reducing MAX_WBITS makes minigzip unable to extract .gz files
 * created by gzip. (Files created by minigzip can still be extracted by
 * gzip.)
 */
#ifndef MAX_WBITS
#  define MAX_WBITS   15 /* 32K LZ77 window */
#endif
/* The memory requirements for deflate are (in bytes):
            (1 << (windowBits+2)) +  (1 << (memLevel+9))
 that is: 128K for windowBits=15  +  128K for memLevel = 8  (default values)
 plus a few kilobytes for small objects. For example, if you want to reduce
 the default memory requirements from 256K to 128K, compile with
     make CFLAGS="-O -DMAX_WBITS=14 -DMAX_MEM_LEVEL=7"
 Of course this will generally degrade compression (there's no free lunch).
   The memory requirements for inflate are (in bytes) 1 << windowBits
 that is, 32K for windowBits=15 (default value) plus a few kilobytes
 for small objects.
*/
                        /* Type declarations */
#ifndef OF /* function prototypes */
#  ifdef STDC
#    define OF(args)  args
#  else
#    define OF(args)  ()
#  endif
#endif
/* The following definitions for FAR are needed only for MSDOS mixed
 * model programming (small or medium model with some far allocations).
 * This was tested only with MSC; for other MSDOS compilers you may have
 * to define NO_MEMCPY in zutil.h.  If you don't need the mixed model,
 * just define FAR to be empty.
 */
#if (defined(M_I86SM) || defined(M_I86MM)) && !defined(__32BIT__)
   /* MSC small or medium model */
#  define SMALL_MEDIUM
#  ifdef _MSC_VER
#    define FAR __far
#  else
#    define FAR far
#  endif
#endif
#if defined(__BORLANDC__) && (defined(__SMALL__) || defined(__MEDIUM__))
#  ifndef __32BIT__
#    define SMALL_MEDIUM
#    define FAR __far
#  endif
#endif
/* Compile with -DZLIB_DLL for Windows DLL support */
#if (defined(_WINDOWS) || defined(WINDOWS)) && defined(ZLIB_DLL)
#  ifdef FAR
#    undef FAR
#  endif
#  include <windows.h>
#  define ZEXPORT  WINAPI
#  ifdef WIN32
#    define ZEXPORTVA  WINAPIV
#  else
#    define ZEXPORTVA  FAR _cdecl _export
#  endif
#else
#   if defined (__BORLANDC__) && defined (_Windows) && defined (__DLL__)
#       define ZEXPORT _export
#       define ZEXPORTVA _export
#   else
#       define ZEXPORT
#       define ZEXPORTVA
#   endif
#endif
#ifndef FAR
#   define FAR
#endif
//#if !defined(__MACTYPES__)
/* added MacTypes.h test JDJ */
typedef unsigned char  Byte;  /* 8 bits */
//#endif
typedef unsigned int   uInt;  /* 16 bits or more */
typedef unsigned long  uLong; /* 32 bits or more */
#if defined(__BORLANDC__) && defined(SMALL_MEDIUM)
   /* Borland C/C++ ignores FAR inside typedef */
#  define Bytef Byte FAR
#else
   typedef Byte  FAR Bytef;
#endif
typedef char  FAR charf;
typedef int   FAR intf;
typedef uInt  FAR uIntf;
typedef uLong FAR uLongf;
#ifdef STDC
   typedef void FAR *voidpf;
   typedef void     *voidp;
#else
   typedef Byte FAR *voidpf;
   typedef Byte     *voidp;
#endif
#if defined(HAVE_UNISTD_H)
#if !defined(TO_WINCE)
#  include <sys/types.h> /* for off_t */
#  include <unistd.h>    /* for SEEK_* and off_t */
#endif
#  define z_off_t  off_t
#endif
#ifndef SEEK_SET
#  define SEEK_SET        0       /* Seek from beginning of file.  */
#  define SEEK_CUR        1       /* Seek from current position.  */
#endif
#ifndef z_off_t
#  define  z_off_t long
#endif
/* MVS linker does not support external names larger than 8 bytes */
#if defined(__MVS__)
#   pragma map(deflateInit_,"DEIN")
#   pragma map(deflateInit2_,"DEIN2")
#   pragma map(deflateEnd,"DEEND")
#   pragma map(inflateInit_,"ININ")
#   pragma map(inflateInit2_,"ININ2")
#   pragma map(inflateEnd,"INEND")
#   pragma map(inflateSync,"INSY")
#   pragma map(inflateSetDictionary,"INSEDI")
#   pragma map(inflate_blocks,"INBL")
#   pragma map(inflate_blocks_new,"INBLNE")
#   pragma map(inflate_blocks_free,"INBLFR")
#   pragma map(inflate_blocks_reset,"INBLRE")
#   pragma map(inflate_codes_free,"INCOFR")
#   pragma map(inflate_codes,"INCO")
//#   pragma map(inflate_fast,"INFA")
#   pragma map(inflate_flush,"INFLU")
#   pragma map(inflate_mask,"INMA")
/* #   pragma map(inflate_set_dictionary,"INSEDI2") */
#   pragma map(inflate_copyright,"INCOPY")
#   pragma map(inflate_trees_bits,"INTRBI")
#   pragma map(inflate_trees_dynamic,"INTRDY")
#   pragma map(inflate_trees_fixed,"INTRFI")
#   pragma map(inflate_trees_free,"INTRFR")
#endif
#endif /* _ZCONF_H */
////////////////////////////////////////////////////////////////
/* zlib.h -- interface of the 'zlib' general purpose compression library
  version 1.1.2, March 19th, 1998
  Copyright (C) 1995-1998 Jean-loup Gailly and Mark Adler
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.
  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:
  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
  Jean-loup Gailly        Mark Adler
  jloup@gzip.org          madler@alumni.caltech.edu
  The data format used by the zlib library is described by RFCs (Request for
  Comments) 1950 to 1952 in the files ftp://ds.internic.net/rfc/rfc1950.txt
  (zlib format), rfc1951.txt (deflate format) and rfc1952.txt (gzip format).
*/
#ifndef _ZLIB_H
#define _ZLIB_H
#ifdef __cplusplus
extern "C" {
#endif
#define ZLIB_VERSION "1.1.2"
/* 
     The 'zlib' compression library provides in-memory compression and
  decompression functions, including integrity checks of the uncompressed
  data.  This version of the library supports only one compression method
  (deflation) but other algorithms will be added later and will have the same
  stream interface.
     Compression can be done in a single step if the buffers are large
  enough (for example if an input file is mmap'ed), or can be done by
  repeated calls of the compression function.  In the latter case, the
  application must provide more input and/or consume the output
  (providing more output space) before each call.
     The library also supports reading and writing files in gzip (.gz) format
  with an interface similar to that of stdio.
     The library does not install any signal handler. The decoder checks
  the consistency of the compressed data, so the library should never
  crash even in case of corrupted input.
*/
typedef uLong (ZEXPORT *check_func) OF((uLong check, const Bytef *buf, uInt len));
typedef voidpf (*alloc_func) OF((voidpf opaque, uInt items, uInt size));
typedef void   (*free_func)  OF((voidpf opaque, voidpf address));
struct internal_state;
typedef struct z_stream_s {
    Bytef    *next_in;  /* next input byte */
    uInt     avail_in;  /* number of bytes available at next_in */
    uLong    total_in;  /* total nb of input bytes read so far */
    Bytef    *next_out; /* next output byte should be put there */
    uInt     avail_out; /* remaining free space at next_out */
    uLong    total_out; /* total nb of bytes output so far */
    char     *msg;      /* last error message, NULL if no error */
    struct internal_state FAR *state; /* not visible by applications */
    alloc_func zalloc;  /* used to allocate the internal state */
    free_func  zfree;   /* used to free the internal state */
    voidpf     opaque;  /* private data object passed to zalloc and zfree */
    int     data_type;  /* best guess about the data type: ascii or binary */
    uLong   adler;      /* adler32 value of the uncompressed data */
    check_func checksum;   /* reserved for future use */
} z_stream;
typedef z_stream FAR *z_streamp;
/*
   The application must update next_in and avail_in when avail_in has
   dropped to zero. It must update next_out and avail_out when avail_out
   has dropped to zero. The application must initialize zalloc, zfree and
   opaque before calling the init function. All other fields are set by the
   compression library and must not be updated by the application.
   The opaque value provided by the application will be passed as the first
   parameter for calls of zalloc and zfree. This can be useful for custom
   memory management. The compression library attaches no meaning to the
   opaque value.
   zalloc must return Z_NULL if there is not enough memory for the object.
   If zlib is used in a multi-threaded application, zalloc and zfree must be
   thread safe.
   On 16-bit systems, the functions zalloc and zfree must be able to allocate
   exactly 65536 bytes, but will not be required to allocate more than this
   if the symbol MAXSEG_64K is defined (see zconf.h). WARNING: On MSDOS,
   pointers returned by zalloc for objects of exactly 65536 bytes *must*
   have their offset normalized to zero. The default allocation function
   provided by this library ensures this (see zutil.c). To reduce memory
   requirements and avoid any allocation of 64K objects, at the expense of
   compression ratio, compile the library with -DMAX_WBITS=14 (see zconf.h).
   The fields total_in and total_out can be used for statistics or
   progress reports. After compression, total_in holds the total size of
   the uncompressed data and may be saved for use in the decompressor
   (particularly if the decompressor wants to decompress everything in
   a single step).
*/
                        /* constants */
#define Z_NO_FLUSH      0
#define Z_PARTIAL_FLUSH 1 /* will be removed, use Z_SYNC_FLUSH instead */
#define Z_SYNC_FLUSH    2
#define Z_FULL_FLUSH    3
#define Z_FINISH        4
/* Allowed flush values; see deflate() below for details */
#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)
/* Return codes for the compression/decompression functions. Negative
 * values are errors, positive values are used for special but normal events.
 */
#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)
/* compression levels */
#define Z_FILTERED            1
#define Z_HUFFMAN_ONLY        2
#define Z_DEFAULT_STRATEGY    0
/* compression strategy; see deflateInit2() below for details */
#define Z_BINARY   0
#define Z_ASCII    1
#define Z_UNKNOWN  2
/* Possible values of the data_type field */
#define Z_DEFLATED   8
/* The deflate compression method (the only one supported in this version) */
#define Z_NULL  0  /* for initializing zalloc, zfree, opaque */
#define zlib_version zlibVersion()
/* for compatibility with versions < 1.0.2 */
                        /* basic functions */
extern const char * ZEXPORT zlibVersion OF((void));
/* The application can compare zlibVersion and ZLIB_VERSION for consistency.
   If the first character differs, the library code actually used is
   not compatible with the zlib.h header file used by the application.
   This check is automatically made by deflateInit and inflateInit.
 */
/* 
extern int ZEXPORT deflateInit OF((z_streamp strm, int level));
     Initializes the internal stream state for compression. The fields
   zalloc, zfree and opaque must be initialized before by the caller.
   If zalloc and zfree are set to Z_NULL, deflateInit updates them to
   use default allocation functions.
     The compression level must be Z_DEFAULT_COMPRESSION, or between 0 and 9:
   1 gives best speed, 9 gives best compression, 0 gives no compression at
   all (the input data is simply copied a block at a time).
   Z_DEFAULT_COMPRESSION requests a default compromise between speed and
   compression (currently equivalent to level 6).
     deflateInit returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_STREAM_ERROR if level is not a valid compression level,
   Z_VERSION_ERROR if the zlib library version (zlib_version) is incompatible
   with the version assumed by the caller (ZLIB_VERSION).
   msg is set to null if there is no error message.  deflateInit does not
   perform any compression: this will be done by deflate().
*/
extern int ZEXPORT deflate OF((z_streamp strm, int flush));
/*
    deflate compresses as much data as possible, and stops when the input
  buffer becomes empty or the output buffer becomes full. It may introduce some
  output latency (reading input without producing any output) except when
  forced to flush.
    The detailed semantics are as follows. deflate performs one or both of the
  following actions:
  - Compress more input starting at next_in and update next_in and avail_in
    accordingly. If not all input can be processed (because there is not
    enough room in the output buffer), next_in and avail_in are updated and
    processing will resume at this point for the next call of deflate().
  - Provide more output starting at next_out and update next_out and avail_out
    accordingly. This action is forced if the parameter flush is non zero.
    Forcing flush frequently degrades the compression ratio, so this parameter
    should be set only when necessary (in interactive applications).
    Some output may be provided even if flush is not set.
  Before the call of deflate(), the application should ensure that at least
  one of the actions is possible, by providing more input and/or consuming
  more output, and updating avail_in or avail_out accordingly; avail_out
  should never be zero before the call. The application can consume the
  compressed output when it wants, for example when the output buffer is full
  (avail_out == 0), or after each call of deflate(). If deflate returns Z_OK
  and with zero avail_out, it must be called again after making room in the
  output buffer because there might be more output pending.
    If the parameter flush is set to Z_SYNC_FLUSH, all pending output is
  flushed to the output buffer and the output is aligned on a byte boundary, so
  that the decompressor can get all input data available so far. (In particular
  avail_in is zero after the call if enough output space has been provided
  before the call.)  Flushing may degrade compression for some compression
  algorithms and so it should be used only when necessary.
    If flush is set to Z_FULL_FLUSH, all output is flushed as with
  Z_SYNC_FLUSH, and the compression state is reset so that decompression can
  restart from this point if previous compressed data has been damaged or if
  random access is desired. Using Z_FULL_FLUSH too often can seriously degrade
  the compression.
    If deflate returns with avail_out == 0, this function must be called again
  with the same value of the flush parameter and more output space (updated
  avail_out), until the flush is complete (deflate returns with non-zero
  avail_out).
    If the parameter flush is set to Z_FINISH, pending input is processed,
  pending output is flushed and deflate returns with Z_STREAM_END if there
  was enough output space; if deflate returns with Z_OK, this function must be
  called again with Z_FINISH and more output space (updated avail_out) but no
  more input data, until it returns with Z_STREAM_END or an error. After
  deflate has returned Z_STREAM_END, the only possible operations on the
  stream are deflateReset or deflateEnd.
  
    Z_FINISH can be used immediately after deflateInit if all the compression
  is to be done in a single step. In this case, avail_out must be at least
  0.1% larger than avail_in plus 12 bytes.  If deflate does not return
  Z_STREAM_END, then it must be called again as described above.
    deflate() sets strm->adler to the adler32 checksum of all input read
  so far (that is, total_in bytes).
    deflate() may update data_type if it can make a good guess about
  the input data type (Z_ASCII or Z_BINARY). In doubt, the data is considered
  binary. This field is only for information purposes and does not affect
  the compression algorithm in any manner.
    deflate() returns Z_OK if some progress has been made (more input
  processed or more output produced), Z_STREAM_END if all input has been
  consumed and all output has been produced (only when flush is set to
  Z_FINISH), Z_STREAM_ERROR if the stream state was inconsistent (for example
  if next_in or next_out was NULL), Z_BUF_ERROR if no progress is possible.
*/
extern int ZEXPORT deflateEnd OF((z_streamp strm));
/*
     All dynamically allocated data structures for this stream are freed.
   This function discards any unprocessed input and does not flush any
   pending output.
     deflateEnd returns Z_OK if success, Z_STREAM_ERROR if the
   stream state was inconsistent, Z_DATA_ERROR if the stream was freed
   prematurely (some input or output was discarded). In the error case,
   msg may be set but then points to a static string (which must not be
   deallocated).
*/
/* 
extern int ZEXPORT inflateInit OF((z_streamp strm));
     Initializes the internal stream state for decompression. The fields
   next_in, avail_in, zalloc, zfree and opaque must be initialized before by
   the caller. If next_in is not Z_NULL and avail_in is large enough (the exact
   value depends on the compression method), inflateInit determines the
   compression method from the zlib header and allocates all data structures
   accordingly; otherwise the allocation will be deferred to the first call of
   inflate.  If zalloc and zfree are set to Z_NULL, inflateInit updates them to
   use default allocation functions.
     inflateInit returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_VERSION_ERROR if the zlib library version is incompatible with the
   version assumed by the caller.  msg is set to null if there is no error
   message. inflateInit does not perform any decompression apart from reading
   the zlib header if present: this will be done by inflate().  (So next_in and
   avail_in may be modified, but next_out and avail_out are unchanged.)
*/
extern int ZEXPORT inflate OF((z_streamp strm, int flush));
/*
    inflate decompresses as much data as possible, and stops when the input
  buffer becomes empty or the output buffer becomes full. It may some
  introduce some output latency (reading input without producing any output)
  except when forced to flush.
  The detailed semantics are as follows. inflate performs one or both of the
  following actions:
  - Decompress more input starting at next_in and update next_in and avail_in
    accordingly. If not all input can be processed (because there is not
    enough room in the output buffer), next_in is updated and processing
    will resume at this point for the next call of inflate().
  - Provide more output starting at next_out and update next_out and avail_out
    accordingly.  inflate() provides as much output as possible, until there
    is no more input data or no more space in the output buffer (see below
    about the flush parameter).
  Before the call of inflate(), the application should ensure that at least
  one of the actions is possible, by providing more input and/or consuming
  more output, and updating the next_* and avail_* values accordingly.
  The application can consume the uncompressed output when it wants, for
  example when the output buffer is full (avail_out == 0), or after each
  call of inflate(). If inflate returns Z_OK and with zero avail_out, it
  must be called again after making room in the output buffer because there
  might be more output pending.
    If the parameter flush is set to Z_SYNC_FLUSH, inflate flushes as much
  output as possible to the output buffer. The flushing behavior of inflate is
  not specified for values of the flush parameter other than Z_SYNC_FLUSH
  and Z_FINISH, but the current implementation actually flushes as much output
  as possible anyway.
    inflate() should normally be called until it returns Z_STREAM_END or an
  error. However if all decompression is to be performed in a single step
  (a single call of inflate), the parameter flush should be set to
  Z_FINISH. In this case all pending input is processed and all pending
  output is flushed; avail_out must be large enough to hold all the
  uncompressed data. (The size of the uncompressed data may have been saved
  by the compressor for this purpose.) The next operation on this stream must
  be inflateEnd to deallocate the decompression state. The use of Z_FINISH
  is never required, but can be used to inform inflate that a faster routine
  may be used for the single inflate() call.
     If a preset dictionary is needed at this point (see inflateSetDictionary
  below), inflate sets strm-adler to the adler32 checksum of the
  dictionary chosen by the compressor and returns Z_NEED_DICT; otherwise 
  it sets strm->adler to the adler32 checksum of all output produced
  so far (that is, total_out bytes) and returns Z_OK, Z_STREAM_END or
  an error code as described below. At the end of the stream, inflate()
  checks that its computed adler32 checksum is equal to that saved by the
  compressor and returns Z_STREAM_END only if the checksum is correct.
    inflate() returns Z_OK if some progress has been made (more input processed
  or more output produced), Z_STREAM_END if the end of the compressed data has
  been reached and all uncompressed output has been produced, Z_NEED_DICT if a
  preset dictionary is needed at this point, Z_DATA_ERROR if the input data was
  corrupted (input stream not conforming to the zlib format or incorrect
  adler32 checksum), Z_STREAM_ERROR if the stream structure was inconsistent
  (for example if next_in or next_out was NULL), Z_MEM_ERROR if there was not
  enough memory, Z_BUF_ERROR if no progress is possible or if there was not
  enough room in the output buffer when Z_FINISH is used. In the Z_DATA_ERROR
  case, the application may then call inflateSync to look for a good
  compression block.
*/
extern int ZEXPORT inflateEnd OF((z_streamp strm));
/*
     All dynamically allocated data structures for this stream are freed.
   This function discards any unprocessed input and does not flush any
   pending output.
     inflateEnd returns Z_OK if success, Z_STREAM_ERROR if the stream state
   was inconsistent. In the error case, msg may be set but then points to a
   static string (which must not be deallocated).
*/
                        /* Advanced functions */
/*
    The following functions are needed only in some special applications.
*/
/*   
extern int ZEXPORT deflateInit2 OF((z_streamp strm,
                                    int  level,
                                    int  method,
                                    int  windowBits,
                                    int  memLevel,
                                    int  strategy));
     This is another version of deflateInit with more compression options. The
   fields next_in, zalloc, zfree and opaque must be initialized before by
   the caller.
     The method parameter is the compression method. It must be Z_DEFLATED in
   this version of the library.
     The windowBits parameter is the base two logarithm of the window size
   (the size of the history buffer).  It should be in the range 8..15 for this
   version of the library. Larger values of this parameter result in better
   compression at the expense of memory usage. The default value is 15 if
   deflateInit is used instead.
     The memLevel parameter specifies how much memory should be allocated
   for the internal compression state. memLevel=1 uses minimum memory but
   is slow and reduces compression ratio; memLevel=9 uses maximum memory
   for optimal speed. The default value is 8. See zconf.h for total memory
   usage as a function of windowBits and memLevel.
     The strategy parameter is used to tune the compression algorithm. Use the
   value Z_DEFAULT_STRATEGY for normal data, Z_FILTERED for data produced by a
   filter (or predictor), or Z_HUFFMAN_ONLY to force Huffman encoding only (no
   string match).  Filtered data consists mostly of small values with a
   somewhat random distribution. In this case, the compression algorithm is
   tuned to compress them better. The effect of Z_FILTERED is to force more
   Huffman coding and less string matching; it is somewhat intermediate
   between Z_DEFAULT and Z_HUFFMAN_ONLY. The strategy parameter only affects
   the compression ratio but not the correctness of the compressed output even
   if it is not set appropriately.
      deflateInit2 returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_STREAM_ERROR if a parameter is invalid (such as an invalid
   method). msg is set to null if there is no error message.  deflateInit2 does
   not perform any compression: this will be done by deflate().
*/
                            
/* extern int ZEXPORT deflateSetDictionary OF((z_streamp strm, */
/*                                             const Bytef *dictionary, */
/*                                             uInt  dictLength)); */
/*
     Initializes the compression dictionary from the given byte sequence
   without producing any compressed output. This function must be called
   immediately after deflateInit or deflateInit2, before any call of
   deflate. The compressor and decompressor must use exactly the same
   dictionary (see inflateSetDictionary).
     The dictionary should consist of strings (byte sequences) that are likely
   to be encountered later in the data to be compressed, with the most commonly
   used strings preferably put towards the end of the dictionary. Using a
   dictionary is most useful when the data to be compressed is short and can be
   predicted with good accuracy; the data can then be compressed better than
   with the default empty dictionary.
     Depending on the size of the compression data structures selected by
   deflateInit or deflateInit2, a part of the dictionary may in effect be
   discarded, for example if the dictionary is larger than the window size in
   deflate or deflate2. Thus the strings most likely to be useful should be
   put at the end of the dictionary, not at the front.
     Upon return of this function, strm->adler is set to the Adler32 value
   of the dictionary; the decompressor may later use this value to determine
   which dictionary has been used by the compressor. (The Adler32 value
   applies to the whole dictionary even if only a subset of the dictionary is
   actually used by the compressor.)
     deflateSetDictionary returns Z_OK if success, or Z_STREAM_ERROR if a
   parameter is invalid (such as NULL dictionary) or the stream state is
   inconsistent (for example if deflate has already been called for this stream
   or if the compression method is bsort). deflateSetDictionary does not
   perform any compression: this will be done by deflate().
*/
extern int ZEXPORT deflateCopy OF((z_streamp dest,
                                   z_streamp source));
/*
     Sets the destination stream as a complete copy of the source stream.
     This function can be useful when several compression strategies will be
   tried, for example when there are several ways of pre-processing the input
   data with a filter. The streams that will be discarded should then be freed
   by calling deflateEnd.  Note that deflateCopy duplicates the internal
   compression state which can be quite large, so this strategy is slow and
   can consume lots of memory.
     deflateCopy returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_STREAM_ERROR if the source stream state was inconsistent
   (such as zalloc being NULL). msg is left unchanged in both source and
   destination.
*/
extern int ZEXPORT deflateReset OF((z_streamp strm));
/*
     This function is equivalent to deflateEnd followed by deflateInit,
   but does not free and reallocate all the internal compression state.
   The stream will keep the same compression level and any other attributes
   that may have been set by deflateInit2.
      deflateReset returns Z_OK if success, or Z_STREAM_ERROR if the source
   stream state was inconsistent (such as zalloc or state being NULL).
*/
extern int ZEXPORT deflateParams OF((z_streamp strm, int level, int strategy));
/*
     Dynamically update the compression level and compression strategy.  The
   interpretation of level and strategy is as in deflateInit2.  This can be
   used to switch between compression and straight copy of the input data, or
   to switch to a different kind of input data requiring a different
   strategy. If the compression level is changed, the input available so far
   is compressed with the old level (and may be flushed); the new level will
   take effect only at the next call of deflate().
     Before the call of deflateParams, the stream state must be set as for
   a call of deflate(), since the currently available input may have to
   be compressed and flushed. In particular, strm->avail_out must be non-zero.
     deflateParams returns Z_OK if success, Z_STREAM_ERROR if the source
   stream state was inconsistent or if a parameter was invalid, Z_BUF_ERROR
   if strm->avail_out was zero.
*/
/*   
extern int ZEXPORT inflateInit2 OF((z_streamp strm,
                                    int  windowBits));
     This is another version of inflateInit with an extra parameter. The
   fields next_in, avail_in, zalloc, zfree and opaque must be initialized
   before by the caller.
     The windowBits parameter is the base two logarithm of the maximum window
   size (the size of the history buffer).  It should be in the range 8..15 for
   this version of the library. The default value is 15 if inflateInit is used
   instead. If a compressed stream with a larger window size is given as
   input, inflate() will return with the error code Z_DATA_ERROR instead of
   trying to allocate a larger window.
      inflateInit2 returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_STREAM_ERROR if a parameter is invalid (such as a negative
   memLevel). msg is set to null if there is no error message.  inflateInit2
   does not perform any decompression apart from reading the zlib header if
   present: this will be done by inflate(). (So next_in and avail_in may be
   modified, but next_out and avail_out are unchanged.)
*/
/* extern int ZEXPORT inflateSetDictionary OF((z_streamp strm, */
/*                                             const Bytef *dictionary, */
/*                                             uInt  dictLength)); */
/*
     Initializes the decompression dictionary from the given uncompressed byte
   sequence. This function must be called immediately after a call of inflate
   if this call returned Z_NEED_DICT. The dictionary chosen by the compressor
   can be determined from the Adler32 value returned by this call of
   inflate. The compressor and decompressor must use exactly the same
   dictionary (see deflateSetDictionary).
     inflateSetDictionary returns Z_OK if success, Z_STREAM_ERROR if a
   parameter is invalid (such as NULL dictionary) or the stream state is
   inconsistent, Z_DATA_ERROR if the given dictionary doesn't match the
   expected one (incorrect Adler32 value). inflateSetDictionary does not
   perform any decompression: this will be done by subsequent calls of
   inflate().
*/
extern int ZEXPORT inflateSync OF((z_streamp strm));
/* 
    Skips invalid compressed data until a full flush point (see above the
  description of deflate with Z_FULL_FLUSH) can be found, or until all
  available input is skipped. No output is provided.
    inflateSync returns Z_OK if a full flush point has been found, Z_BUF_ERROR
  if no more input was provided, Z_DATA_ERROR if no flush point has been found,
  or Z_STREAM_ERROR if the stream structure was inconsistent. In the success
  case, the application may save the current current value of total_in which
  indicates where valid compressed data was found. In the error case, the
  application may repeatedly call inflateSync, providing more input each time,
  until success or end of the input data.
*/
extern int ZEXPORT inflateReset OF((z_streamp strm));
/*
     This function is equivalent to inflateEnd followed by inflateInit,
   but does not free and reallocate all the internal decompression state.
   The stream will keep attributes that may have been set by inflateInit2.
      inflateReset returns Z_OK if success, or Z_STREAM_ERROR if the source
   stream state was inconsistent (such as zalloc or state being NULL).
*/
                        /* utility functions */
/*
     The following utility functions are implemented on top of the
   basic stream-oriented functions. To simplify the interface, some
   default options are assumed (compression level and memory usage,
   standard memory allocation functions). The source code of these
   utility functions can easily be modified if you need special options.
*/
extern int ZEXPORT compress OF((Bytef *dest,   uLongf *destLen,
                                const Bytef *source, uLong sourceLen));
/*
     Compresses the source buffer into the destination buffer.  sourceLen is
   the byte length of the source buffer. Upon entry, destLen is the total
   size of the destination buffer, which must be at least 0.1% larger than
   sourceLen plus 12 bytes. Upon exit, destLen is the actual size of the
   compressed buffer.
     This function can be used to compress a whole file at once if the
   input file is mmap'ed.
     compress returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_BUF_ERROR if there was not enough room in the output
   buffer.
*/
extern int ZEXPORT compress2 OF((Bytef *dest,   uLongf *destLen,
                                 const Bytef *source, uLong sourceLen,
                                 int level));
/*
     Compresses the source buffer into the destination buffer. The level
   parameter has the same meaning as in deflateInit.  sourceLen is the byte
   length of the source buffer. Upon entry, destLen is the total size of the
   destination buffer, which must be at least 0.1% larger than sourceLen plus
   12 bytes. Upon exit, destLen is the actual size of the compressed buffer.
     compress2 returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_BUF_ERROR if there was not enough room in the output buffer,
   Z_STREAM_ERROR if the level parameter is invalid.
*/
extern int ZEXPORT uncompress OF((Bytef *dest,   uLongf *destLen,
                                  const Bytef *source, uLong sourceLen, int flag));
/*
     Decompresses the source buffer into the destination buffer.  sourceLen is
   the byte length of the source buffer. Upon entry, destLen is the total
   size of the destination buffer, which must be large enough to hold the
   entire uncompressed data. (The size of the uncompressed data must have
   been saved previously by the compressor and transmitted to the decompressor
   by some mechanism outside the scope of this compression library.)
   Upon exit, destLen is the actual size of the compressed buffer.
     This function can be used to decompress a whole file at once if the
   input file is mmap'ed.
     uncompress returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_BUF_ERROR if there was not enough room in the output
   buffer, or Z_DATA_ERROR if the input data was corrupted.
*/
/* typedef voidp gzFile; */
/* extern gzFile ZEXPORT gzopen  OF((const char *path, const char *mode)); */
/*
     Opens a gzip (.gz) file for reading or writing. The mode parameter
   is as in fopen ("rb" or "wb") but can also include a compression level
   ("wb9") or a strategy: 'f' for filtered data as in "wb6f", 'h' for
   Huffman only compression as in "wb1h". (See the description
   of deflateInit2 for more information about the strategy parameter.)
     gzopen can be used to read a file which is not in gzip format; in this
   case gzread will directly read from the file without decompression.
     gzopen returns NULL if the file could not be opened or if there was
   insufficient memory to allocate the (de)compression state; errno
   can be checked to distinguish the two cases (if errno is zero, the
   zlib error is Z_MEM_ERROR).  */
/* extern gzFile ZEXPORT gzdopen  OF((int fd, const char *mode)); */
/*
     gzdopen() associates a gzFile with the file descriptor fd.  File
   descriptors are obtained from calls like open, dup, creat, pipe or
   fileno (in the file has been previously opened with fopen).
   The mode parameter is as in gzopen.
     The next call of gzclose on the returned gzFile will also close the
   file descriptor fd, just like fclose(fdopen(fd), mode) closes the file
   descriptor fd. If you want to keep fd open, use gzdopen(dup(fd), mode).
     gzdopen returns NULL if there was insufficient memory to allocate
   the (de)compression state.
*/
/* extern int ZEXPORT gzsetparams OF((gzFile file, int level, int strategy)); */
/*
     Dynamically update the compression level or strategy. See the description
   of deflateInit2 for the meaning of these parameters.
     gzsetparams returns Z_OK if success, or Z_STREAM_ERROR if the file was not
   opened for writing.
*/
/* extern int ZEXPORT    gzread  OF((gzFile file, voidp buf, unsigned len)); */
/*
     Reads the given number of uncompressed bytes from the compressed file.
   If the input file was not in gzip format, gzread copies the given number
   of bytes into the buffer.
     gzread returns the number of uncompressed bytes actually read (0 for
   end of file, -1 for error). */
/* extern int ZEXPORT    gzwrite OF((gzFile file, const voidp buf, unsigned len)); */
/*
     Writes the given number of uncompressed bytes into the compressed file.
   gzwrite returns the number of uncompressed bytes actually written
   (0 in case of error).
*/
/* extern int ZEXPORTVA   gzprintf OF((gzFile file, const char *format, ...)); */
/*
     Converts, formats, and writes the args to the compressed file under
   control of the format string, as in fprintf. gzprintf returns the number of
   uncompressed bytes actually written (0 in case of error).
*/
/* extern int ZEXPORT gzputs OF((gzFile file, const char *s)); */
/*
      Writes the given null-terminated string to the compressed file, excluding
   the terminating null character.
      gzputs returns the number of characters written, or -1 in case of error.
*/
/* extern char * ZEXPORT gzgets OF((gzFile file, char *buf, int len)); */
/*
      Reads bytes from the compressed file until len-1 characters are read, or
   a newline character is read and transferred to buf, or an end-of-file
   condition is encountered.  The string is then terminated with a null
   character.
      gzgets returns buf, or Z_NULL in case of error.
*/
/* extern int ZEXPORT    gzputc OF((gzFile file, int c)); */
/*
      Writes c, converted to an unsigned char, into the compressed file.
   gzputc returns the value that was written, or -1 in case of error.
*/
/* extern int ZEXPORT    gzgetc OF((gzFile file)); */
/*
      Reads one byte from the compressed file. gzgetc returns this byte
   or -1 in case of end of file or error.
*/
/* extern int ZEXPORT    gzflush OF((gzFile file, int flush)); */
/*
     Flushes all pending output into the compressed file. The parameter
   flush is as in the deflate() function. The return value is the zlib
   error number (see function gzerror below). gzflush returns Z_OK if
   the flush parameter is Z_FINISH and all output could be flushed.
     gzflush should be called only when strictly necessary because it can
   degrade compression.
*/
/* extern z_off_t ZEXPORT    gzseek OF((gzFile file, z_off_t offset, int whence)); */
/* 
      Sets the starting position for the next gzread or gzwrite on the given
   compressed file. The offset represents a number of bytes in the
   uncompressed data stream. The whence parameter is defined as in lseek(2);
   the value SEEK_END is not supported.
     If the file is opened for reading, this function is emulated but can be
   extremely slow. If the file is opened for writing, only forward seeks are
   supported; gzseek then compresses a sequence of zeroes up to the new
   starting position.
      gzseek returns the resulting offset location as measured in bytes from
   the beginning of the uncompressed stream, or -1 in case of error, in
   particular if the file is opened for writing and the new starting position
   would be before the current position.
*/
/* extern int ZEXPORT    gzrewind OF((gzFile file)); */
/*
     Rewinds the given file. This function is supported only for reading.
   gzrewind(file) is equivalent to (int)gzseek(file, 0L, SEEK_SET)
*/
/* extern z_off_t ZEXPORT    gztell OF((gzFile file)); */
/*
     Returns the starting position for the next gzread or gzwrite on the
   given compressed file. This position represents a number of bytes in the
   uncompressed data stream.
   gztell(file) is equivalent to gzseek(file, 0L, SEEK_CUR)
*/
/* extern int ZEXPORT gzeof OF((gzFile file)); */
/*
     Returns 1 when EOF has previously been detected reading the given
   input stream, otherwise zero.
*/
/* extern int ZEXPORT    gzclose OF((gzFile file)); */
/*
     Flushes all pending output if necessary, closes the compressed file
   and deallocates all the (de)compression state. The return value is the zlib
   error number (see function gzerror below).
*/
/* extern const char * ZEXPORT gzerror OF((gzFile file, int *errnum)); */
/*
     Returns the error message for the last error which occurred on the
   given compressed file. errnum is set to zlib error number. If an
   error occurred in the file system and not in the compression library,
   errnum is set to Z_ERRNO and the application may consult errno
   to get the exact error code.
*/
                        /* checksum functions */
/*
     These functions are not related to compression but are exported
   anyway because they might be useful in applications using the
   compression library.
*/
extern uLong ZEXPORT adler32 OF((uLong adler, const Bytef *buf, uInt len));
/*
     Update a running Adler-32 checksum with the bytes buf[0..len-1] and
   return the updated checksum. If buf is NULL, this function returns
   the required initial value for the checksum.
   An Adler-32 checksum is almost as reliable as a CRC32 but can be computed
   much faster. Usage example:
     uLong adler = adler32(0L, Z_NULL, 0);
     while (read_buffer(buffer, length) != EOF) {
       adler = adler32(adler, buffer, length);
     }
     if (adler != original_adler) error();
*/
/* extern uLong ZEXPORT crc32   OF((uLong crc, const Bytef *buf, uInt len)); */
/*
     Update a running crc with the bytes buf[0..len-1] and return the updated
   crc. If buf is NULL, this function returns the required initial value
   for the crc. Pre- and post-conditioning (one's complement) is performed
   within this function so it shouldn't be done by the application.
   Usage example:
     uLong crc = crc32(0L, Z_NULL, 0);
     while (read_buffer(buffer, length) != EOF) {
       crc = crc32(crc, buffer, length);
     }
     if (crc != original_crc) error();
*/
                        /* various hacks, don't look :) */
/* deflateInit and inflateInit are macros to allow checking the zlib version
 * and the compiler's view of z_stream:
 */
extern int ZEXPORT deflateInit_ OF((z_streamp strm, int level,
                                    const char *version, int stream_size));
extern int ZEXPORT inflateInit_ OF((z_streamp strm,
                                    const char *version, int stream_size));
extern int ZEXPORT deflateInit2_ OF((z_streamp strm, int  level, int  method,
                                     int windowBits, int memLevel,
                                     int strategy, const char *version,
                                     int stream_size));
extern int ZEXPORT inflateInit2_ OF((z_streamp strm, int  windowBits,
                                     const char *version, int stream_size));
#define deflateInit(strm, level) \
        deflateInit_((strm), (level),       ZLIB_VERSION, sizeof(z_stream))
#define inflateInit(strm) \
        inflateInit_((strm),                ZLIB_VERSION, sizeof(z_stream))
#define deflateInit2(strm, level, method, windowBits, memLevel, strategy) \
        deflateInit2_((strm),(level),(method),(windowBits),(memLevel),\
                      (strategy),           ZLIB_VERSION, sizeof(z_stream))
#define inflateInit2(strm, windowBits) \
        inflateInit2_((strm), (windowBits), ZLIB_VERSION, sizeof(z_stream))
#if !defined(_Z_UTIL_H) && !defined(NO_DUMMY_DECL)
/*     struct internal_state {int dummy;};  hack for buggy compilers */
#endif
extern const char   * ZEXPORT zError           OF((int err));
extern int            ZEXPORT inflateSyncPoint OF((z_streamp z));
extern const uLongf * ZEXPORT get_crc_table    OF((void));
#ifdef __cplusplus
}
#endif
#endif /* _ZLIB_H */
/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-1998 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */
/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */
#ifndef _Z_UTIL_H
#define _Z_UTIL_H
#ifdef STDC
#if !defined(TO_WINCE)
#  include <stddef.h>
#endif
#  include <string.h>
#  include <stdlib.h>
#endif
#ifdef NO_ERRNO_H
    extern int errno;
#else
#if !defined(TO_WINCE)
#   include <errno.h>
#endif
#endif
#ifndef local
#ifdef macintosh
#define local static
#else
#  define local /* static */
#endif
#endif
/* compile with -Dlocal if your debugger can't find static symbols */
typedef unsigned char  uch;
typedef uch FAR uchf;
typedef unsigned short ush;
typedef ush FAR ushf;
typedef unsigned long  ulg;
extern const char *z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */
#define ERR_MSG(err) z_errmsg[Z_NEED_DICT-(err)]
#define ERR_RETURN(strm,err) \
  return (strm->msg = (char*)ERR_MSG(err), (err))
/* To be used only when the state is known to be valid */
        /* common constants */
#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */
#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
/* default memLevel */
#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
/* The three kinds of block type */
#define MIN_MATCH  3
#define MAX_MATCH  258
/* The minimum and maximum match lengths */
#define PRESET_DICT 0x20 /* preset dictionary flag in zlib header */
        /* target dependencies */
#ifdef MSDOS
#  define OS_CODE  0x00
#  ifdef __TURBOC__
#    if(__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
       /* Allow compilation with ANSI keywords only enabled */
       void _Cdecl farfree( void *block );
       void *_Cdecl farmalloc( unsigned long nbytes );
#    else
#if !defined(TO_WINCE)
#     include <alloc.h>
#endif
#    endif
#  else /* MSC or DJGPP */
#    include <malloc.h>
#  endif
#endif
#ifdef OS2
#  define OS_CODE  0x06
#endif
#ifdef WIN32 /* Window 95 & Windows NT */
#  define OS_CODE  0x0b
#endif
/* #if defined(VAXC) || defined(VMS) */
/* #  define OS_CODE  0x02 */
/* #  define F_OPEN(name, mode) \ */
/*      fopen((name), (mode), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512") */
/* #endif */
#ifdef AMIGA
#  define OS_CODE  0x01
#endif
#if defined(ATARI) || defined(atarist)
#  define OS_CODE  0x05
#endif
/* #if defined(MACOS) || defined(TARGET_OS_MAC) */
/* #  define OS_CODE  0x07 */
/* #  ifndef fdopen */
/* #    define fdopen(fd,mode) NULL // No fdopen() */
/* #  endif */
/* #endif */
/* #if defined(__MWERKS__) && !defined(fdopen) */
/* #  if __dest_os != __be_os && __dest_os != __win32_os */
/* #    define fdopen(fd,mode) NULL */
/* #  endif */
/* #endif */
#ifdef __50SERIES /* Prime/PRIMOS */
#  define OS_CODE  0x0F
#endif
#ifdef TOPS20
#  define OS_CODE  0x0a
#endif
/* #if defined(_BEOS_) || defined(RISCOS) */
/* #  define fdopen(fd,mode) NULL // No fdopen() */
/* #endif */
/* #if (defined(_MSC_VER) && (_MSC_VER >= 600)) */
/* #  define fdopen(fd,type)  _fdopen(fd,type) */
/* #endif */
        /* Common defaults */
#ifndef OS_CODE
#  define OS_CODE  0x03  /* assume Unix */
#endif
/* #ifndef F_OPEN */
/* #  define F_OPEN(name, mode) fopen((name), (mode)) */
/* #endif */
         /* functions */
/* #ifdef HAVE_STRERROR */
/*    extern char *strerror OF((int)); */
/* #  define zstrerror(errnum) strerror(errnum) */
/* #else */
/* #  define zstrerror(errnum) "" */
/* #endif */
#if defined(pyr)
#  define NO_MEMCPY
#endif
#if defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#endif
#if defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#endif
#ifdef HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  else
#    define zmemcpy memcpy
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#else
   extern void zmemcpy  OF((Bytef* dest, Bytef* source, uInt len));
   extern int  zmemcmp  OF((Bytef* s1,   Bytef* s2, uInt len));
   extern void zmemzero OF((Bytef* dest, uInt len));
#endif
/* Diagnostic functions */
/* #ifdef DEBUG */
/* #  include <stdio.h> */
/*    extern int z_verbose; */
/*    extern void z_error    OF((char *m)); */
/* #  define Assert(cond,msg) {if(!(cond)) z_error(msg);} */
/* #  define Trace(x) {if (z_verbose>=0) fprintf x ;} */
/* #  define Tracev(x) {if (z_verbose>0) fprintf x ;} */
/* #  define Tracevv(x) {if (z_verbose>1) fprintf x ;} */
/* #  define Tracec(c,x) {if (z_verbose>0 && (c)) fprintf x ;} */
/* #  define Tracecv(c,x) {if (z_verbose>1 && (c)) fprintf x ;} */
/* #else */
/* #  define Assert(cond,msg) */
/* #  define Trace(x) */
/* #  define Tracev(x) */
/* #  define Tracevv(x) */
/* #  define Tracec(c,x) */
/* #  define Tracecv(c,x) */
/* #endif */
voidpf zcalloc OF((voidpf opaque, unsigned items, unsigned size));
void   zcfree  OF((voidpf opaque, voidpf ptr));
#define ZALLOC(strm, items, size) \
           (*((strm)->zalloc))((strm)->opaque, (items), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) {if (p) ZFREE(s, p);}
#if 0
INLINE voidpf zcalloc (voidpf opaque, unsigned items, unsigned size) {
    return (voidpf)calloc(items, size);
}
INLINE void  zcfree (voidpf opaque, voidpf ptr) {
    free(ptr);
}
#endif
#endif /* _Z_UTIL_H */
#ifndef INFTREES_H
#define INFTREES_H
/* inftrees.h -- header to use inftrees.c
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */
/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */
/* Huffman code lookup table entry--this entry is four bytes for machines
   that have 16-bit pointers (e.g. PC's in the small or medium model). */
typedef struct inflate_huft_s FAR inflate_huft;
struct inflate_huft_s {
  union {
    struct {
      Byte Exop;        /* number of extra bits or operation */
      Byte Bits;        /* number of bits in this code or subcode */
    } what;
    uInt pad;           /* pad structure to a power of 2 (4 bytes for */
  } word;               /*  16-bit, 8 bytes for 32-bit int's) */
  uInt base;            /* literal, length base, distance base,
                           or table offset */
};
/* Maximum size of dynamic tree.  The maximum found in a long but non-
   exhaustive search was 1004 huft structures (850 for length/literals
   and 154 for distances, the latter actually the result of an
   exhaustive search).  The actual maximum is not known, but the
   value below is more than safe. */
#define MANY 1440
extern int inflate_trees_bits OF((
    uIntf *,                    /* 19 code lengths */
    uIntf *,                    /* bits tree desired/actual depth */
    inflate_huft * FAR *,       /* bits tree result */
    inflate_huft *,             /* space for trees */
    z_streamp));                /* for messages */
extern int inflate_trees_dynamic OF((
    uInt,                       /* number of literal/length codes */
    uInt,                       /* number of distance codes */
    uIntf *,                    /* that many (total) code lengths */
    uIntf *,                    /* literal desired/actual bit depth */
    uIntf *,                    /* distance desired/actual bit depth */
    inflate_huft * FAR *,       /* literal/length tree result */
    inflate_huft * FAR *,       /* distance tree result */
    inflate_huft *,             /* space for trees */
    z_streamp));                /* for messages */
extern int inflate_trees_fixed OF((
    uIntf *,                    /* literal desired/actual bit depth */
    uIntf *,                    /* distance desired/actual bit depth */
    inflate_huft * FAR *,       /* literal/length tree result */
    inflate_huft * FAR *,       /* distance tree result */
    z_streamp));                /* for memory allocation */
#endif
#ifndef INFBLOCK_H
#define INFBLOCK_H
/* infblock.h -- header to use infblock.c
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */
/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */
struct inflate_blocks_state;
typedef struct inflate_blocks_state FAR inflate_blocks_statef;
extern inflate_blocks_statef * inflate_blocks_new OF((
    z_streamp z,
    check_func c,               /* check function */
    uInt w));                   /* window size */
extern int inflate_blocks OF((
    inflate_blocks_statef *,
    z_streamp ,
    int));                      /* initial return code */
extern void inflate_blocks_reset OF((
    inflate_blocks_statef *,
    z_streamp ,
    uLongf *));                  /* check value on output */
extern int inflate_blocks_free OF((
    inflate_blocks_statef *,
    z_streamp));
/* extern void inflate_set_dictionary OF(( */
/*     inflate_blocks_statef *s, */
/*     const Bytef *d,  dictionary */
/*     uInt  n));        dictionary length */
extern int inflate_blocks_sync_point OF((
    inflate_blocks_statef *s));
#endif
#ifndef INFCODES_H
#define INFCODES_H
/* infcodes.h -- header to use infcodes.c
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */
/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */
struct inflate_codes_state;
typedef struct inflate_codes_state FAR inflate_codes_statef;
extern inflate_codes_statef *inflate_codes_new OF((
    uInt, uInt,
    inflate_huft *, inflate_huft *,
    z_streamp ));
extern int inflate_codes OF((
    inflate_blocks_statef *,
    z_streamp ,
    int));
extern void inflate_codes_free OF((
    inflate_codes_statef *,
    z_streamp ));
#endif
/* infutil.h -- types and macros common to blocks and codes
 * Copyright (C) 1995-1998 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */
/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */
#ifndef _INFUTIL_H
#define _INFUTIL_H
typedef enum {
      TYPE,     /* get type bits (3, including end bit) */
      LENS,     /* get lengths for stored */
      STORED,   /* processing stored block */
      TABLE,    /* get table lengths */
      BTREE,    /* get bit lengths tree for a dynamic block */
      DTREE,    /* get length, distance trees for a dynamic block */
      CODES,    /* processing fixed or dynamic block */
      DRY,      /* output remaining window bytes */
      DONE,     /* finished last block, done */
      BAD}      /* got a data error--stuck here */
inflate_block_mode;
/* inflate blocks semi-private state */
struct inflate_blocks_state {
  /* mode */
  inflate_block_mode  mode;     /* current inflate_block mode */
  /* mode dependent information */
  union {
    uInt left;          /* if STORED, bytes left to copy */
    struct {
      uInt table;               /* table lengths (14 bits) */
      uInt index;               /* index into blens (or border) */
      uIntf *blens;             /* bit lengths of codes */
      uInt bb;                  /* bit length tree depth */
      inflate_huft *tb;         /* bit length decoding tree */
    } trees;            /* if DTREE, decoding info for trees */
    struct {
      inflate_codes_statef 
         *codes;
    } decode;           /* if CODES, current state */
  } sub;                /* submode */
  uInt last;            /* true if this block is the last block */
  /* mode independent information */
  uInt bitk;            /* bits in bit buffer */
  uLong bitb;           /* bit buffer */
  inflate_huft *hufts;  /* single malloc for tree space */
  Bytef *window;        /* sliding window */
  Bytef *end;           /* one byte after sliding window */
  Bytef *read;          /* window read pointer */
  Bytef *write;         /* window write pointer */
  check_func checkfn;   /* check function */
  uLong check;          /* check on output */
};
/* defines for inflate input/output */
/*   update pointers and return */
#define UPDBITS {s->bitb=b;s->bitk=k;}
#define UPDIN {z->avail_in=n;z->total_in+=p-z->next_in;z->next_in=p;}
#define UPDOUT {s->write=q;}
#define UPDATE {UPDBITS UPDIN UPDOUT}
#define LEAVE {UPDATE return inflate_flush(s,z,r);}
/*   get bytes and bits */
#define LOADIN {p=z->next_in;n=z->avail_in;b=s->bitb;k=s->bitk;}
#define NEEDBYTE {if(n)r=Z_OK;else LEAVE}
#define NEXTBYTE (n--,*p++)
#define NEEDBITS(j) {while(k<(j)){NEEDBYTE;b|=((uLong)NEXTBYTE)<<k;k+=8;}}
#define DUMPBITS(j) {b>>=(j);k-=(j);}
/*   output bytes */
#define WAVAIL (uInt)(q<s->read?s->read-q-1:s->end-q)
#define LOADOUT {q=s->write;m=(uInt)WAVAIL;}
#define WRAP {if(q==s->end&&s->read!=s->window){q=s->window;m=(uInt)WAVAIL;}}
#define FLUSH {UPDOUT r=inflate_flush(s,z,r); LOADOUT}
#define NEEDOUT {if(m==0){WRAP if(m==0){FLUSH WRAP if(m==0) LEAVE}}r=Z_OK;}
#define OUTBYTE(a) {*q++=(Byte)(a);m--;}
/*   load local pointers */
#define LOAD {LOADIN LOADOUT}
/* masks for lower bits (size given to avoid silly warnings with Visual C++) */
extern uInt inflate_mask[17];
/* copy as much as possible from the sliding window to the output area */
extern int inflate_flush OF((
    inflate_blocks_statef *,
    z_streamp ,
    int));
//rls struct internal_state      {int dummy;}; /* for buggy compilers */
#endif
/* deflate.h -- internal compression state
 * Copyright (C) 1995-1998 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */
/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */
#ifndef _DEFLATE_H
#define _DEFLATE_H
//rls#include "zutil.h"
/* ===========================================================================
 * Internal compression state.
 */
#define LENGTH_CODES 29
/* number of length codes, not counting the special END_BLOCK code */
#define LITERALS  256
/* number of literal bytes 0..255 */
#define L_CODES (LITERALS+1+LENGTH_CODES)
/* number of Literal or Length codes, including the END_BLOCK code */
#define D_CODES   30
/* number of distance codes */
#define BL_CODES  19
/* number of codes used to transfer the bit lengths */
#define HEAP_SIZE (2*L_CODES+1)
/* maximum heap size */
#define MAX_BITS 15
/* All codes must not exceed MAX_BITS bits */
#define INIT_STATE    42
#define BUSY_STATE   113
#define FINISH_STATE 666
/* Stream status */
/* Data structure describing a single value and its code string. */
typedef struct ct_data_s {
    union {
        ush  freq;       /* frequency count */
        ush  code;       /* bit string */
    } fc;
    union {
        ush  dad;        /* father node in Huffman tree */
        ush  len;        /* length of bit string */
    } dl;
} FAR ct_data;
#define Freq fc.freq
#define Code fc.code
#define Dad  dl.dad
#define Len  dl.len
typedef struct static_tree_desc_s  static_tree_desc;
typedef struct tree_desc_s {
    ct_data *dyn_tree;           /* the dynamic tree */
    int     max_code;            /* largest code with non zero frequency */
    static_tree_desc *stat_desc; /* the corresponding static tree */
} FAR tree_desc;
typedef ush Pos;
typedef Pos FAR Posf;
typedef unsigned IPos;
/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */
typedef struct internal_state {
    z_streamp strm;      /* pointer back to this zlib stream */
    int   status;        /* as the name implies */
    Bytef *pending_buf;  /* output still pending */
    ulg   pending_buf_size; /* size of pending_buf */
    Bytef *pending_out;  /* next pending byte to output to the stream */
    int   pending;       /* nb of bytes in the pending buffer */
    int   noheader;      /* suppress zlib header and adler32 */
    Byte  data_type;     /* UNKNOWN, BINARY or ASCII */
    Byte  method;        /* STORED (for zip only) or DEFLATED */
    int   last_flush;    /* value of flush param for previous deflate call */
                /* used by deflate.c: */
    uInt  w_size;        /* LZ77 window size (32K by default) */
    uInt  w_bits;        /* log2(w_size)  (8..16) */
    uInt  w_mask;        /* w_size - 1 */
    Bytef *window;
    /* Sliding window. Input bytes are read into the second half of the window,
     * and move to the first half later to keep a dictionary of at least wSize
     * bytes. With this organization, matches are limited to a distance of
     * wSize-MAX_MATCH bytes, but this ensures that IO is always
     * performed with a length multiple of the block size. Also, it limits
     * the window size to 64K, which is quite useful on MSDOS.
     * To do: use the user input buffer as sliding window.
     */
    ulg window_size;
    /* Actual size of window: 2*wSize, except when the user input buffer
     * is directly used as sliding window.
     */
    Posf *prev;
    /* Link to older string with same hash index. To limit the size of this
     * array to 64K, this link is maintained only for the last 32K strings.
     * An index in this array is thus a window index modulo 32K.
     */
    Posf *head; /* Heads of the hash chains or NIL. */
    uInt  ins_h;          /* hash index of string to be inserted */
    uInt  hash_size;      /* number of elements in hash table */
    uInt  hash_bits;      /* log2(hash_size) */
    uInt  hash_mask;      /* hash_size-1 */
    uInt  hash_shift;
    /* Number of bits by which ins_h must be shifted at each input
     * step. It must be such that after MIN_MATCH steps, the oldest
     * byte no longer takes part in the hash key, that is:
     *   hash_shift * MIN_MATCH >= hash_bits
     */
    long block_start;
    /* Window position at the beginning of the current output block. Gets
     * negative when the window is moved backwards.
     */
    uInt match_length;           /* length of best match */
    IPos prev_match;             /* previous match */
    int match_available;         /* set if previous match exists */
    uInt strstart;               /* start of string to insert */
    uInt match_start;            /* start of matching string */
    uInt lookahead;              /* number of valid bytes ahead in window */
    uInt prev_length;
    /* Length of the best match at previous step. Matches not greater than this
     * are discarded. This is used in the lazy match evaluation.
     */
    uInt max_chain_length;
    /* To speed up deflation, hash chains are never searched beyond this
     * length.  A higher limit improves compression ratio but degrades the
     * speed.
     */
    uInt max_lazy_match;
    /* Attempt to find a better match only when the current match is strictly
     * smaller than this value. This mechanism is used only for compression
     * levels >= 4.
     */
#   define max_insert_length  max_lazy_match
    /* Insert new strings in the hash table only if the match length is not
     * greater than this length. This saves time but degrades compression.
     * max_insert_length is used only for compression levels <= 3.
     */
    int level;    /* compression level (1..9) */
    int strategy; /* favor or force Huffman coding*/
    uInt good_match;
    /* Use a faster search when the previous match is longer than this */
    int nice_match; /* Stop searching when current match exceeds this */
                /* used by trees.c: */
    /* Didn't use ct_data typedef below to supress compiler warning */
    struct ct_data_s dyn_ltree[HEAP_SIZE];   /* literal and length tree */
    struct ct_data_s dyn_dtree[2*D_CODES+1]; /* distance tree */
    struct ct_data_s bl_tree[2*BL_CODES+1];  /* Huffman tree for bit lengths */
    struct tree_desc_s l_desc;               /* desc. for literal tree */
    struct tree_desc_s d_desc;               /* desc. for distance tree */
    struct tree_desc_s bl_desc;              /* desc. for bit length tree */
    ush bl_count[MAX_BITS+1];
    /* number of codes at each bit length for an optimal tree */
    int heap[2*L_CODES+1];      /* heap used to build the Huffman trees */
    int heap_len;               /* number of elements in the heap */
    int heap_max;               /* element of largest frequency */
    /* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
     * The same heap array is used to build all trees.
     */
    uch depth[2*L_CODES+1];
    /* Depth of each subtree used as tie breaker for trees of equal frequency
     */
    uchf *l_buf;          /* buffer for literals or lengths */
    uInt  lit_bufsize;
    /* Size of match buffer for literals/lengths.  There are 4 reasons for
     * limiting lit_bufsize to 64K:
     *   - frequencies can be kept in 16 bit counters
     *   - if compression is not successful for the first block, all input
     *     data is still in the window so we can still emit a stored block even
     *     when input comes from standard input.  (This can also be done for
     *     all blocks if lit_bufsize is not greater than 32K.)
     *   - if compression is not successful for a file smaller than 64K, we can
     *     even emit a stored file instead of a stored block (saving 5 bytes).
     *     This is applicable only for zip (not gzip or zlib).
     *   - creating new Huffman trees less frequently may not provide fast
     *     adaptation to changes in the input data statistics. (Take for
     *     example a binary file with poorly compressible code followed by
     *     a highly compressible string table.) Smaller buffer sizes give
     *     fast adaptation but have of course the overhead of transmitting
     *     trees more frequently.
     *   - I can't count above 4
     */
    uInt last_lit;      /* running index in l_buf */
    ushf *d_buf;
    /* Buffer for distances. To simplify the code, d_buf and l_buf have
     * the same number of elements. To use different lengths, an extra flag
     * array would be necessary.
     */
    ulg opt_len;        /* bit length of current block with optimal trees */
    ulg static_len;     /* bit length of current block with static trees */
    ulg compressed_len; /* total bit length of compressed file */
    uInt matches;       /* number of string matches in current block */
    int last_eob_len;   /* bit length of EOB code for last block */
#ifdef DEBUG
    ulg bits_sent;      /* bit length of the compressed data */
#endif
    ush bi_buf;
    /* Output buffer. bits are inserted starting at the bottom (least
     * significant bits).
     */
    int bi_valid;
    /* Number of valid bits in bi_buf.  All bits above the last valid bit
     * are always zero.
     */
} FAR deflate_state;
/* Output a byte on the stream.
 * IN assertion: there is enough room in pending_buf.
 */
#define put_byte(s, c) {s->pending_buf[s->pending++] = (c);}
#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */
#define MAX_DIST(s)  ((s)->w_size-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */
        /* in trees.c */
void _tr_init         OF((deflate_state *s));
int  _tr_tally        OF((deflate_state *s, unsigned dist, unsigned lc));
ulg  _tr_flush_block  OF((deflate_state *s, charf *buf, ulg stored_len,
			  int eof));
void _tr_align        OF((deflate_state *s));
void _tr_stored_block OF((deflate_state *s, charf *buf, ulg stored_len,
                          int eof));
#define d_code(dist) \
   ((dist) < 256 ? _dist_code[dist] : _dist_code[256+((dist)>>7)])
/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. _dist_code[256] and _dist_code[257] are never
 * used.
 */
#ifndef DEBUG
/* Inline versions of _tr_tally for speed: */
#if defined(GEN_TREES_H) || !defined(STDC)
  extern uch _length_code[];
  extern uch _dist_code[];
#else
  extern const uch _length_code[];
  extern const uch _dist_code[];
#endif
# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->d_buf[s->last_lit] = 0; \
    s->l_buf[s->last_lit++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->last_lit == s->lit_bufsize-1); \
   }
# define _tr_tally_dist(s, distance, length, flush) \
  { uch len = (length); \
    ush dist = (distance); \
    s->d_buf[s->last_lit] = dist; \
    s->l_buf[s->last_lit++] = len; \
    dist--; \
    s->dyn_ltree[_length_code[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_code(dist)].Freq++; \
    flush = (s->last_lit == s->lit_bufsize-1); \
  }
#else
# define _tr_tally_lit(s, c, flush) flush = _tr_tally(s, 0, c)
# define _tr_tally_dist(s, distance, length, flush) \
              flush = _tr_tally(s, distance, length) 
#endif
#endif
