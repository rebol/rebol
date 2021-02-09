/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**  Copyright 2012 REBOL Technologies
**  REBOL is a trademark of REBOL Technologies
**  Licensed under the Apache License, Version 2.0
**  This is a code-generated file.
**
************************************************************************
**
**  Title: Boot Definitions
**  Build: A0
**  Date:  10-Feb-2021
**  File:  bootdefs.h
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-boot.reb)
**
***********************************************************************/


#define REBOL_VER 3
#define REBOL_REV 4
#define REBOL_UPD 0
#define REBOL_SYS 4
#define REBOL_VAR 40

/***********************************************************************
**
**	REBOL Boot Strings
**
**		These are special strings required during boot and other
**		operations. Putting them here hides them from exe hackers.
**		These are all string offsets within a single string.
**
***********************************************************************/
#define RS_SCAN                0
#define RS_INFO                31
#define RS_TRACE               32
#define RS_STACK               43
#define RS_DUMP                45
#define RS_ERROR               59
#define RS_ERRS                72
#define RS_WATCH               76
#define RS_EXTENSION           79
#define RS_MAX	82
#define RS_SIZE	1401

/***********************************************************************
**
*/	enum REBOL_Symbols
/*
**		REBOL static canonical words (symbols) used with the code.
**
***********************************************************************/
{
	SYM_NOT_USED = 0,
	SYM_END_TYPE,                 // 1
	SYM_UNSET_TYPE,               // 2
	SYM_NONE_TYPE,                // 3
	SYM_LOGIC_TYPE,               // 4
	SYM_INTEGER_TYPE,             // 5
	SYM_DECIMAL_TYPE,             // 6
	SYM_PERCENT_TYPE,             // 7
	SYM_MONEY_TYPE,               // 8
	SYM_CHAR_TYPE,                // 9
	SYM_PAIR_TYPE,                // 10
	SYM_TUPLE_TYPE,               // 11
	SYM_TIME_TYPE,                // 12
	SYM_DATE_TYPE,                // 13
	SYM_BINARY_TYPE,              // 14
	SYM_STRING_TYPE,              // 15
	SYM_FILE_TYPE,                // 16
	SYM_EMAIL_TYPE,               // 17
	SYM_REF_TYPE,                 // 18
	SYM_URL_TYPE,                 // 19
	SYM_TAG_TYPE,                 // 20
	SYM_BITSET_TYPE,              // 21
	SYM_IMAGE_TYPE,               // 22
	SYM_VECTOR_TYPE,              // 23
	SYM_BLOCK_TYPE,               // 24
	SYM_PAREN_TYPE,               // 25
	SYM_PATH_TYPE,                // 26
	SYM_SET_PATH_TYPE,            // 27
	SYM_GET_PATH_TYPE,            // 28
	SYM_LIT_PATH_TYPE,            // 29
	SYM_MAP_TYPE,                 // 30
	SYM_DATATYPE_TYPE,            // 31
	SYM_TYPESET_TYPE,             // 32
	SYM_WORD_TYPE,                // 33
	SYM_SET_WORD_TYPE,            // 34
	SYM_GET_WORD_TYPE,            // 35
	SYM_LIT_WORD_TYPE,            // 36
	SYM_REFINEMENT_TYPE,          // 37
	SYM_ISSUE_TYPE,               // 38
	SYM_NATIVE_TYPE,              // 39
	SYM_ACTION_TYPE,              // 40
	SYM_REBCODE_TYPE,             // 41
	SYM_COMMAND_TYPE,             // 42
	SYM_OP_TYPE,                  // 43
	SYM_CLOSURE_TYPE,             // 44
	SYM_FUNCTION_TYPE,            // 45
	SYM_FRAME_TYPE,               // 46
	SYM_OBJECT_TYPE,              // 47
	SYM_MODULE_TYPE,              // 48
	SYM_ERROR_TYPE,               // 49
	SYM_TASK_TYPE,                // 50
	SYM_PORT_TYPE,                // 51
	SYM_GOB_TYPE,                 // 52
	SYM_EVENT_TYPE,               // 53
	SYM_HANDLE_TYPE,              // 54
	SYM_STRUCT_TYPE,              // 55
	SYM_LIBRARY_TYPE,             // 56
	SYM_UTYPE_TYPE,               // 57
	SYM_ANY_TYPEX,                // 58 - any-type!
	SYM_ANY_WORDX,                // 59 - any-word!
	SYM_ANY_PATHX,                // 60 - any-path!
	SYM_ANY_FUNCTIONX,            // 61 - any-function!
	SYM_NUMBERX,                  // 62 - number!
	SYM_SCALARX,                  // 63 - scalar!
	SYM_SERIESX,                  // 64 - series!
	SYM_ANY_STRINGX,              // 65 - any-string!
	SYM_ANY_OBJECTX,              // 66 - any-object!
	SYM_ANY_BLOCKX,               // 67 - any-block!
	SYM_SI8X,                     // 68 - si8!
	SYM_SI16X,                    // 69 - si16!
	SYM_SI32X,                    // 70 - si32!
	SYM_SI64X,                    // 71 - si64!
	SYM_UI8X,                     // 72 - ui8!
	SYM_UI16X,                    // 73 - ui16!
	SYM_UI32X,                    // 74 - ui32!
	SYM_UI64X,                    // 75 - ui64!
	SYM_F32X,                     // 76 - f32!
	SYM_F64X,                     // 77 - f64!
	SYM_DATATYPES,                // 78 - datatypes
	SYM_NATIVE,                   // 79 - native
	SYM_SELF,                     // 80 - self
	SYM_NONE,                     // 81 - none
	SYM_TRUE,                     // 82 - true
	SYM_FALSE,                    // 83 - false
	SYM_ON,                       // 84 - on
	SYM_OFF,                      // 85 - off
	SYM_YES,                      // 86 - yes
	SYM_NO,                       // 87 - no
	SYM_PI,                       // 88 - pi
	SYM_REBOL,                    // 89 - rebol
	SYM_SYSTEM,                   // 90 - system
	SYM_BASE,                     // 91 - base
	SYM_SYS,                      // 92 - sys
	SYM_MODS,                     // 93 - mods
	SYM_SPEC,                     // 94 - spec
	SYM_BODY,                     // 95 - body
	SYM_WORDS,                    // 96 - words
	SYM_VALUES,                   // 97 - values
	SYM_TYPES,                    // 98 - types
	SYM_TITLE,                    // 99 - title
	SYM_X,                        // 100 - x
	SYM_Y,                        // 101 - y
	SYM__ADD,                     // 102 - +
	SYM__,                        // 103 - -
	SYM__P,                       // 104 - *
	SYM_UNSIGNED,                 // 105 - unsigned
	SYM__UNNAMED_,                // 106 - -unnamed-
	SYM__APPLY_,                  // 107 - -apply-
	SYM_CODE,                     // 108 - code
	SYM_DELECT,                   // 109 - delect
	SYM_SECURE,                   // 110 - secure
	SYM_PROTECT,                  // 111 - protect
	SYM_NET,                      // 112 - net
	SYM_CALL,                     // 113 - call
	SYM_ENVR,                     // 114 - envr
	SYM_EVAL,                     // 115 - eval
	SYM_MEMORY,                   // 116 - memory
	SYM_DEBUG,                    // 117 - debug
	SYM_BROWSE,                   // 118 - browse
	SYM_EXTENSION,                // 119 - extension
	SYM_YEAR,                     // 120 - year
	SYM_MONTH,                    // 121 - month
	SYM_DAY,                      // 122 - day
	SYM_TIME,                     // 123 - time
	SYM_DATE,                     // 124 - date
	SYM_ZONE,                     // 125 - zone
	SYM_HOUR,                     // 126 - hour
	SYM_MINUTE,                   // 127 - minute
	SYM_SECOND,                   // 128 - second
	SYM_WEEKDAY,                  // 129 - weekday
	SYM_YEARDAY,                  // 130 - yearday
	SYM_TIMEZONE,                 // 131 - timezone
	SYM_UTC,                      // 132 - utc
	SYM_JULIAN,                   // 133 - julian
	SYM_PARSE,                    // 134 - parse
	SYM_OR_BAR,                   // 135 - |
	SYM_SET,                      // 136 - set
	SYM_COPY,                     // 137 - copy
	SYM_SOME,                     // 138 - some
	SYM_ANY,                      // 139 - any
	SYM_OPT,                      // 140 - opt
	SYM_NOT,                      // 141 - not
	SYM_AND,                      // 142 - and
	SYM_THEN,                     // 143 - then
	SYM_REMOVE,                   // 144 - remove
	SYM_INSERT,                   // 145 - insert
	SYM_CHANGE,                   // 146 - change
	SYM_IF,                       // 147 - if
	SYM_FAIL,                     // 148 - fail
	SYM_REJECT,                   // 149 - reject
	SYM_WHILE,                    // 150 - while
	SYM_RETURN,                   // 151 - return
	SYM_LIMIT,                    // 152 - limit
	SYM_QQ,                       // 153 - ??
	SYM_ACCEPT,                   // 154 - accept
	SYM_BREAK,                    // 155 - break
	SYM_SKIP,                     // 156 - skip
	SYM_TO,                       // 157 - to
	SYM_THRU,                     // 158 - thru
	SYM_QUOTE,                    // 159 - quote
	SYM_DO,                       // 160 - do
	SYM_INTO,                     // 161 - into
	SYM_ONLY,                     // 162 - only
	SYM_END,                      // 163 - end
	SYM_TYPE,                     // 164 - type
	SYM_KEY,                      // 165 - key
	SYM_PORT,                     // 166 - port
	SYM_MODE,                     // 167 - mode
	SYM_WINDOW,                   // 168 - window
	SYM_DOUBLE,                   // 169 - double
	SYM_CONTROL,                  // 170 - control
	SYM_SHIFT,                    // 171 - shift
	SYM_POINT,                    // 172 - point
	SYM_BOX,                      // 173 - box
	SYM_TRIANGLE,                 // 174 - triangle
	SYM_HERMITE,                  // 175 - hermite
	SYM_HANNING,                  // 176 - hanning
	SYM_HAMMING,                  // 177 - hamming
	SYM_BLACKMAN,                 // 178 - blackman
	SYM_GAUSSIAN,                 // 179 - gaussian
	SYM_QUADRATIC,                // 180 - quadratic
	SYM_CUBIC,                    // 181 - cubic
	SYM_CATROM,                   // 182 - catrom
	SYM_MITCHELL,                 // 183 - mitchell
	SYM_LANCZOS,                  // 184 - lanczos
	SYM_BESSEL,                   // 185 - bessel
	SYM_SINC,                     // 186 - sinc
	SYM_HASH,                     // 187 - hash
	SYM_ADLER32,                  // 188 - adler32
	SYM_CRC24,                    // 189 - crc24
	SYM_CRC32,                    // 190 - crc32
	SYM_MD4,                      // 191 - md4
	SYM_MD5,                      // 192 - md5
	SYM_RIPEMD160,                // 193 - ripemd160
	SYM_SHA1,                     // 194 - sha1
	SYM_SHA224,                   // 195 - sha224
	SYM_SHA256,                   // 196 - sha256
	SYM_SHA384,                   // 197 - sha384
	SYM_SHA512,                   // 198 - sha512
	SYM_IDENTIFY,                 // 199 - identify
	SYM_DECODE,                   // 200 - decode
	SYM_ENCODE,                   // 201 - encode
	SYM_CONSOLE,                  // 202 - console
	SYM_FILE,                     // 203 - file
	SYM_DIR,                      // 204 - dir
	SYM_EVENT,                    // 205 - event
	SYM_CALLBACK,                 // 206 - callback
	SYM_DNS,                      // 207 - dns
	SYM_TCP,                      // 208 - tcp
	SYM_UDP,                      // 209 - udp
	SYM_CLIPBOARD,                // 210 - clipboard
	SYM_GOB,                      // 211 - gob
	SYM_OFFSET,                   // 212 - offset
	SYM_SIZE,                     // 213 - size
	SYM_PANE,                     // 214 - pane
	SYM_PARENT,                   // 215 - parent
	SYM_IMAGE,                    // 216 - image
	SYM_DRAW,                     // 217 - draw
	SYM_TEXT,                     // 218 - text
	SYM_EFFECT,                   // 219 - effect
	SYM_COLOR,                    // 220 - color
	SYM_FLAGS,                    // 221 - flags
	SYM_RGB,                      // 222 - rgb
	SYM_ALPHA,                    // 223 - alpha
	SYM_DATA,                     // 224 - data
	SYM_RESIZE,                   // 225 - resize
	SYM_NO_TITLE,                 // 226 - no-title
	SYM_NO_BORDER,                // 227 - no-border
	SYM_DROPABLE,                 // 228 - dropable
	SYM_TRANSPARENT,              // 229 - transparent
	SYM_POPUP,                    // 230 - popup
	SYM_MODAL,                    // 231 - modal
	SYM_ON_TOP,                   // 232 - on-top
	SYM_HIDDEN,                   // 233 - hidden
	SYM_OWNER,                    // 234 - owner
	SYM_OWNER_READ,               // 235 - owner-read
	SYM_OWNER_WRITE,              // 236 - owner-write
	SYM_OWNER_EXECUTE,            // 237 - owner-execute
	SYM_GROUP_READ,               // 238 - group-read
	SYM_GROUP_WRITE,              // 239 - group-write
	SYM_GROUP_EXECUTE,            // 240 - group-execute
	SYM_WORLD_READ,               // 241 - world-read
	SYM_WORLD_WRITE,              // 242 - world-write
	SYM_WORLD_EXECUTE,            // 243 - world-execute
	SYM_ECHO,                     // 244 - echo
	SYM_LINE,                     // 245 - line
	SYM_BITS,                     // 246 - bits
	SYM_CRASH,                    // 247 - crash
	SYM_CRASH_DUMP,               // 248 - crash-dump
	SYM_WATCH_RECYCLE,            // 249 - watch-recycle
	SYM_WATCH_OBJ_COPY,           // 250 - watch-obj-copy
	SYM_STACK_SIZE,               // 251 - stack-size
	SYM_ID,                       // 252 - id
	SYM_EXIT_CODE,                // 253 - exit-code
	SYM_PUB_EXP,                  // 254 - pub-exp
	SYM_PRIV_EXP,                 // 255 - priv-exp
	SYM_R_BUFFER,                 // 256 - r-buffer
	SYM_W_BUFFER,                 // 257 - w-buffer
	SYM_R_MASK,                   // 258 - r-mask
	SYM_W_MASK,                   // 259 - w-mask
	SYM_NOT_BIT,                  // 260 - not-bit
	SYM_UNIXTIME_NOW,             // 261 - unixtime-now
	SYM_UNIXTIME_NOW_LE,          // 262 - unixtime-now-le
	SYM_RANDOM_BYTES,             // 263 - random-bytes
	SYM_LENGTHQ,                  // 264 - length?
	SYM_BUFFER_COLS,              // 265 - buffer-cols
	SYM_BUFFER_ROWS,              // 266 - buffer-rows
	SYM_WINDOW_COLS,              // 267 - window-cols
	SYM_WINDOW_ROWS,              // 268 - window-rows
	SYM_DEVICES_IN,               // 269 - devices-in
	SYM_DEVICES_OUT,              // 270 - devices-out
	SYM_MSDOS_DATETIME,           // 271 - msdos-datetime
	SYM_MSDOS_DATE,               // 272 - msdos-date
	SYM_MSDOS_TIME,               // 273 - msdos-time
	SYM_OCTAL_BYTES,              // 274 - octal-bytes
	SYM_STRING_BYTES,             // 275 - string-bytes

    // follows symbols used in C sources, but not defined in %words.reb list...
	SYM_ABGR,                     // 276
	SYM_AES,                      // 277
	SYM_ALIGN,                    // 278
	SYM_ARGB,                     // 279
	SYM_AT,                       // 280
	SYM_ATZ,                      // 281
	SYM_BGR,                      // 282
	SYM_BGRA,                     // 283
	SYM_BGRO,                     // 284
	SYM_BINCODE,                  // 285
	SYM_BIT,                      // 286
	SYM_BITSET16,                 // 287
	SYM_BITSET32,                 // 288
	SYM_BITSET8,                  // 289
	SYM_BMP,                      // 290
	SYM_BYTES,                    // 291
	SYM_CHACHA20,                 // 292
	SYM_CHACHA20POLY1305,         // 293
	SYM_CHECKSUM,                 // 294
	SYM_CODEC,                    // 295
	SYM_DDS,                      // 296
	SYM_DH,                       // 297
	SYM_DNG,                      // 298
	SYM_ECDH,                     // 299
	SYM_EGID,                     // 300
	SYM_ENCODEDU32,               // 301
	SYM_EUID,                     // 302
	SYM_FB,                       // 303
	SYM_FIXED16,                  // 304
	SYM_FIXED8,                   // 305
	SYM_FLOAT,                    // 306
	SYM_FLOAT16,                  // 307
	SYM_GID,                      // 308
	SYM_GIF,                      // 309
	SYM_HDP,                      // 310
	SYM_HEIF,                     // 311
	SYM_ICO,                      // 312
	SYM_INDEX,                    // 313
	SYM_INDEXZ,                   // 314
	SYM_JPEG,                     // 315
	SYM_JPEGXR,                   // 316
	SYM_JPG,                      // 317
	SYM_JXR,                      // 318
	SYM_LENGTH,                   // 319
	SYM_LOCAL,                    // 320
	SYM_MIDI,                     // 321
	SYM_NAME,                     // 322
	SYM_OBGR,                     // 323
	SYM_OPACITY,                  // 324
	SYM_ORGB,                     // 325
	SYM_PID,                      // 326
	SYM_PNG,                      // 327
	SYM_POLY1305,                 // 328
	SYM_RC4,                      // 329
	SYM_RGBA,                     // 330
	SYM_RGBO,                     // 331
	SYM_RSA,                      // 332
	SYM_SB,                       // 333
	SYM_SECP160R1,                // 334
	SYM_SECP192R1,                // 335
	SYM_SECP224R1,                // 336
	SYM_SECP256K1,                // 337
	SYM_SECP256R1,                // 338
	SYM_SI16,                     // 339
	SYM_SI16BE,                   // 340
	SYM_SI16LE,                   // 341
	SYM_SI24,                     // 342
	SYM_SI24BE,                   // 343
	SYM_SI32,                     // 344
	SYM_SI32BE,                   // 345
	SYM_SI32LE,                   // 346
	SYM_SI64,                     // 347
	SYM_SI8,                      // 348
	SYM_SIGNED,                   // 349
	SYM_SKIPBITS,                 // 350
	SYM_STRING,                   // 351
	SYM_TIFF,                     // 352
	SYM_TUPLE3,                   // 353
	SYM_TUPLE4,                   // 354
	SYM_UB,                       // 355
	SYM_UI16,                     // 356
	SYM_UI16BE,                   // 357
	SYM_UI16BEBYTES,              // 358
	SYM_UI16BYTES,                // 359
	SYM_UI16LE,                   // 360
	SYM_UI16LEBYTES,              // 361
	SYM_UI24,                     // 362
	SYM_UI24BE,                   // 363
	SYM_UI24BEBYTES,              // 364
	SYM_UI24BYTES,                // 365
	SYM_UI24LE,                   // 366
	SYM_UI24LEBYTES,              // 367
	SYM_UI32,                     // 368
	SYM_UI32BE,                   // 369
	SYM_UI32BEBYTES,              // 370
	SYM_UI32BYTES,                // 371
	SYM_UI32LE,                   // 372
	SYM_UI32LEBYTES,              // 373
	SYM_UI64,                     // 374
	SYM_UI64BE,                   // 375
	SYM_UI64LE,                   // 376
	SYM_UI8,                      // 377
	SYM_UI8BYTES,                 // 378
	SYM_UID,                      // 379
	SYM_WEBP,                     // 380
	SYM_WIDGET                    // 381
};

/***********************************************************************
**
*/	enum REBOL_Actions
/*
**		REBOL datatype action numbers.
**
***********************************************************************/
{
	A_TYPE = 0,                   // Handled by interpreter
	A_ADD,                        // 1
	A_SUBTRACT,                   // 2
	A_MULTIPLY,                   // 3
	A_DIVIDE,                     // 4
	A_REMAINDER,                  // 5
	A_POWER,                      // 6
	A_AND,                        // 7
	A_OR,                         // 8
	A_XOR,                        // 9
	A_NEGATE,                     // 10
	A_COMPLEMENT,                 // 11
	A_ABSOLUTE,                   // 12
	A_ROUND,                      // 13
	A_RANDOM,                     // 14
	A_ODDQ,                       // 15
	A_EVENQ,                      // 16
	A_HEAD,                       // 17
	A_TAIL,                       // 18
	A_HEADQ,                      // 19
	A_TAILQ,                      // 20
	A_PASTQ,                      // 21
	A_NEXT,                       // 22
	A_BACK,                       // 23
	A_SKIP,                       // 24
	A_AT,                         // 25
	A_INDEXQ,                     // 26
	A_LENGTHQ,                    // 27
	A_PICK,                       // 28
	A_FIND,                       // 29
	A_SELECT,                     // 30
	A_REFLECT,                    // 31
	A_MAKE,                       // 32
	A_TO,                         // 33
	A_COPY,                       // 34
	A_TAKE,                       // 35
	A_PUT,                        // 36
	A_INSERT,                     // 37
	A_APPEND,                     // 38
	A_REMOVE,                     // 39
	A_CHANGE,                     // 40
	A_POKE,                       // 41
	A_CLEAR,                      // 42
	A_TRIM,                       // 43
	A_SWAP,                       // 44
	A_REVERSE,                    // 45
	A_SORT,                       // 46
	A_CREATE,                     // 47
	A_DELETE,                     // 48
	A_OPEN,                       // 49
	A_CLOSE,                      // 50
	A_READ,                       // 51
	A_WRITE,                      // 52
	A_OPENQ,                      // 53
	A_QUERY,                      // 54
	A_MODIFY,                     // 55
	A_UPDATE,                     // 56
	A_RENAME,                     // 57
	A_MAX_ACTION
};

#define IS_BINARY_ACT(a) ((a) <= A_XOR)
