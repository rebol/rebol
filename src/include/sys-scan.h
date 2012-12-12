/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
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
**  Summary: Lexical Scanner Definitions
**  Module:  sys-scan.h
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

/*
**  Tokens returned by the scanner.  Keep in sync with boot.r strings area.
*/
enum Value_Types {
	TOKEN_EOF = 0,
	TOKEN_LINE,
	TOKEN_BLOCK_END,
	TOKEN_PAREN_END,
	TOKEN_WORD,
	TOKEN_SET,
	TOKEN_GET,
	TOKEN_LIT,
	TOKEN_NONE,		// not needed
	TOKEN_LOGIC,	// not needed
	TOKEN_INTEGER,
	TOKEN_DECIMAL,
	TOKEN_PERCENT,
	TOKEN_MONEY,
	TOKEN_TIME,
	TOKEN_DATE,
	TOKEN_CHAR,
	TOKEN_BLOCK,
	TOKEN_PAREN,
	TOKEN_STRING,
	TOKEN_BINARY,
	TOKEN_PAIR,
	TOKEN_TUPLE,
	TOKEN_FILE,
	TOKEN_EMAIL,
	TOKEN_URL,
	TOKEN_ISSUE,
	TOKEN_TAG,
	TOKEN_PATH,
	TOKEN_REFINE,
	TOKEN_CONSTRUCT,
	TOKEN_MAX
};


/*
**  Lexical Table Entry Encoding
*/
#define LEX_SHIFT       5               /* shift for encoding classes */
#define LEX_CLASS       (3<<LEX_SHIFT)  /* class bit field */
#define LEX_VALUE       (0x1F)          /* value bit field */

#define GET_LEX_CLASS(c)  (Lex_Map[(REBYTE)c] >> LEX_SHIFT)
#define GET_LEX_VALUE(c)  (Lex_Map[(REBYTE)c] & LEX_VALUE)


/*
**	Delimiting Chars (encoded in the LEX_VALUE field)
*/
enum LEX_DELIMIT_ENUM {
	LEX_DELIMIT_SPACE,              /* 20 space */
	LEX_DELIMIT_END_FILE,           /* 00 EOF */
	LEX_DELIMIT_LINEFEED,           /* 0A line-feed */
	LEX_DELIMIT_RETURN,             /* 0D return */
	LEX_DELIMIT_LEFT_PAREN,         /* 28 ( */
	LEX_DELIMIT_RIGHT_PAREN,        /* 29 ) */
	LEX_DELIMIT_LEFT_BRACKET,       /* 5B [ */
	LEX_DELIMIT_RIGHT_BRACKET,      /* 5D ] */
	LEX_DELIMIT_LEFT_BRACE,         /* 7B } */
	LEX_DELIMIT_RIGHT_BRACE,        /* 7D } */
	LEX_DELIMIT_QUOTE,              /* 22 " */
	LEX_DELIMIT_SLASH,              /* 2F / - date, path, file */
	LEX_DELIMIT_SEMICOLON,          /* 3B ; */
	LEX_DELIMIT_UTF8_ERROR,
	LEX_DELIMIT_MAX
};


/*
**  General Lexical Classes (encoded in the LEX_CLASS field)
*/
enum LEX_CLASS_ENUM {
    LEX_CLASS_DELIMIT = 0,
    LEX_CLASS_SPECIAL,
    LEX_CLASS_WORD,
    LEX_CLASS_NUMBER
};

#define LEX_DELIMIT     (LEX_CLASS_DELIMIT<<LEX_SHIFT)
#define LEX_SPECIAL     (LEX_CLASS_SPECIAL<<LEX_SHIFT)
#define LEX_WORD        (LEX_CLASS_WORD<<LEX_SHIFT)
#define LEX_NUMBER      (LEX_CLASS_NUMBER<<LEX_SHIFT)

#define LEX_FLAG(n)             (1 << (n))
#define SET_LEX_FLAG(f,l)       (f = f | LEX_FLAG(l))
#define HAS_LEX_FLAGS(f,l)      (f & (l))
#define HAS_LEX_FLAG(f,l)       (f & LEX_FLAG(l))
#define ONLY_LEX_FLAG(f,l)      (f == LEX_FLAG(l))

#define MASK_LEX_CLASS(c)               (Lex_Map[(REBYTE)c] & LEX_CLASS)
#define IS_LEX_SPACE(c)                 (!Lex_Map[(REBYTE)c])
#define IS_LEX_ANY_SPACE(c)				(Lex_Map[(REBYTE)c]<=LEX_DELIMIT_RETURN)
#define IS_LEX_DELIMIT(c)               (MASK_LEX_CLASS(c) == LEX_DELIMIT)
#define IS_LEX_SPECIAL(c)               (MASK_LEX_CLASS(c) == LEX_SPECIAL)
#define IS_LEX_WORD(c)                  (MASK_LEX_CLASS(c) == LEX_WORD)
#define IS_LEX_NUMBER(c)                (MASK_LEX_CLASS(c) == LEX_NUMBER)

#define IS_LEX_AT_LEAST_SPECIAL(c)      (Lex_Map[(REBYTE)c] >= LEX_SPECIAL)
#define IS_LEX_AT_LEAST_WORD(c)         (Lex_Map[(REBYTE)c] >= LEX_WORD)
#define IS_LEX_AT_LEAST_NUMBER(c)       (Lex_Map[(REBYTE)c] >= LEX_NUMBER)

/*
**  Special Chars (encoded in the LEX_VALUE field)
*/
enum LEX_SPECIAL_ENUM {             /* The order is important! */
	LEX_SPECIAL_AT,                 /* 40 @ - email */
	LEX_SPECIAL_PERCENT,            /* 25 % - file name */
	LEX_SPECIAL_BACKSLASH,          /* 5C \  */
	LEX_SPECIAL_COLON,              /* 3A : - time, get, set */
	LEX_SPECIAL_TICK,               /* 27 ' - literal */
	LEX_SPECIAL_LESSER,				/* 3C < - compare or tag */
	LEX_SPECIAL_GREATER,			/* 3E > - compare or end tag */
	LEX_SPECIAL_PLUS,               /* 2B + - positive number */
	LEX_SPECIAL_MINUS,              /* 2D - - date, negative number */
	LEX_SPECIAL_TILDE,              /* 7E ~ - complement number */
									/** Any of these can follow - or ~ : */
	LEX_SPECIAL_PERIOD,             /* 2E . - decimal number */
	LEX_SPECIAL_COMMA,              /* 2C , - decimal number */
	LEX_SPECIAL_POUND,              /* 23 # - hex number */
	LEX_SPECIAL_DOLLAR,             /* 24 $ - money */
	LEX_SPECIAL_WORD,               /* SPECIAL - used for word chars (for nums) */
	LEX_SPECIAL_MAX
};

/*
**  Special Encodings
*/
#define LEX_DEFAULT (LEX_DELIMIT|LEX_DELIMIT_SPACE)     /* control chars = spaces */

/*
**  Characters not allowed in Words
*/
#define LEX_WORD_FLAGS (LEX_FLAG(LEX_SPECIAL_AT) |              \
                        LEX_FLAG(LEX_SPECIAL_PERCENT) |         \
                        LEX_FLAG(LEX_SPECIAL_BACKSLASH) |       \
                        LEX_FLAG(LEX_SPECIAL_COMMA) |           \
						LEX_FLAG(LEX_SPECIAL_POUND) |			\
						LEX_FLAG(LEX_SPECIAL_DOLLAR) |			\
                        LEX_FLAG(LEX_SPECIAL_COLON))

enum rebol_esc_codes {
	// Must match Esc_Names[]!
	ESC_LINE,
	ESC_TAB,
	ESC_PAGE,
	ESC_ESCAPE,
	ESC_ESC,
	ESC_BACK,
	ESC_DEL,
	ESC_NULL,
	ESC_MAX
};


/*
**  Scanner State Structure
*/

typedef struct rebol_scan_state {
    REBYTE *begin;
    REBYTE *end;
    REBYTE const *limit;    /* no chars after this point */
//    REBYTE const *error_id; /* id string for errors (file name or URL path) */
    REBCNT line_count;
	REBYTE *head_line;		// head of current line (used for errors)
	REBCNT opts;
	REBCNT errors;
} SCAN_STATE;

#define ACCEPT_TOKEN(s) ((s)->begin = (s)->end)

#define NOT_NEWLINE(c) ((c) && (c) != CR && (c) != LF)

enum {
	SCAN_NEXT,	// load/next feature
	SCAN_ONLY,  // only single value (no blocks)
	SCAN_RELAX,	// no error throw
};

/*
**  Externally Accessed Variables
*/
extern const REBYTE Lex_Map[256];

/***********************************************************************
**
*/  static INLINE REBYTE *Skip_To_Char(REBYTE *cp, REBYTE *ep, REBYTE chr)
/*
**		Skip to the specified character but not past the end
**		of the string.  Return zero if the char is not found.
**
***********************************************************************/
{
    while (cp != ep && *cp != chr) cp++;
    if (*cp == chr) return cp;
    return 0;
}
