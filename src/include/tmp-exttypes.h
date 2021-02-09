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
**  Title: Extension Type Equates
**  Build: A0
**  Date:  10-Feb-2021
**  File:  tmp-exttypes.h
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-boot.reb)
**
***********************************************************************/


/***********************************************************************
**
*/	const REBYTE Reb_To_RXT[REB_MAX] =
/*
***********************************************************************/
{
	RXT_END,                      // end
	RXT_UNSET,                    // unset
	RXT_NONE,                     // none
	RXT_LOGIC,                    // logic
	RXT_INTEGER,                  // integer
	RXT_DECIMAL,                  // decimal
	RXT_PERCENT,                  // percent
	0,                            // money
	RXT_CHAR,                     // char
	RXT_PAIR,                     // pair
	RXT_TUPLE,                    // tuple
	RXT_TIME,                     // time
	RXT_DATE,                     // date
	RXT_BINARY,                   // binary
	RXT_STRING,                   // string
	RXT_FILE,                     // file
	RXT_EMAIL,                    // email
	RXT_REF,                      // ref
	RXT_URL,                      // url
	RXT_TAG,                      // tag
	RXT_BITSET,                   // bitset
	RXT_IMAGE,                    // image
	RXT_VECTOR,                   // vector
	RXT_BLOCK,                    // block
	RXT_PAREN,                    // paren
	RXT_PATH,                     // path
	RXT_SET_PATH,                 // set-path
	RXT_GET_PATH,                 // get-path
	RXT_LIT_PATH,                 // lit-path
	0,                            // map
	0,                            // datatype
	0,                            // typeset
	RXT_WORD,                     // word
	RXT_SET_WORD,                 // set-word
	RXT_GET_WORD,                 // get-word
	RXT_LIT_WORD,                 // lit-word
	RXT_REFINEMENT,               // refinement
	RXT_ISSUE,                    // issue
	0,                            // native
	0,                            // action
	0,                            // rebcode
	0,                            // command
	0,                            // op
	0,                            // closure
	0,                            // function
	0,                            // frame
	RXT_OBJECT,                   // object
	RXT_MODULE,                   // module
	0,                            // error
	0,                            // task
	0,                            // port
	RXT_GOB,                      // gob
	0,                            // event
	RXT_HANDLE,                   // handle
	0,                            // struct
	0,                            // library
	0                             // utype
};

/***********************************************************************
**
*/	const REBYTE RXT_To_Reb[RXT_MAX] =
/*
***********************************************************************/
{
	REB_END,                      // 0
	REB_UNSET,                    // 1
	REB_NONE,                     // 2
	REB_HANDLE,                   // 3
	REB_LOGIC,                    // 4
	REB_INTEGER,                  // 5
	REB_DECIMAL,                  // 6
	REB_PERCENT,                  // 7
	0,                            // 8
	0,                            // 9
	REB_CHAR,                     // 10
	REB_PAIR,                     // 11
	REB_TUPLE,                    // 12
	REB_TIME,                     // 13
	REB_DATE,                     // 14
	0,                            // 15
	REB_WORD,                     // 16
	REB_SET_WORD,                 // 17
	REB_GET_WORD,                 // 18
	REB_LIT_WORD,                 // 19
	REB_REFINEMENT,               // 20
	REB_ISSUE,                    // 21
	0,                            // 22
	0,                            // 23
	REB_STRING,                   // 24
	REB_FILE,                     // 25
	REB_EMAIL,                    // 26
	REB_REF,                      // 27
	REB_URL,                      // 28
	REB_TAG,                      // 29
	0,                            // 30
	0,                            // 31
	REB_BLOCK,                    // 32
	REB_PAREN,                    // 33
	REB_PATH,                     // 34
	REB_SET_PATH,                 // 35
	REB_GET_PATH,                 // 36
	REB_LIT_PATH,                 // 37
	0,                            // 38
	0,                            // 39
	REB_BINARY,                   // 40
	REB_BITSET,                   // 41
	REB_VECTOR,                   // 42
	REB_IMAGE,                    // 43
	0,                            // 44
	0,                            // 45
	0,                            // 46
	REB_GOB,                      // 47
	REB_OBJECT,                   // 48
	REB_MODULE                    // 49
};

/***********************************************************************
**
*/	const REBCNT RXT_Eval_Class[RXT_MAX] =
/*
***********************************************************************/
{
	0,                            // end
	RXE_NULL,                     // unset
	RXE_NULL,                     // none
	RXE_HANDLE,                   // handle
	RXE_32,                       // logic
	RXE_64,                       // integer
	RXE_64,                       // decimal
	RXE_64,                       // percent
	0,                            // 8
	0,                            // 9
	RXE_32,                       // char
	RXE_64,                       // pair
	RXE_64,                       // tuple
	RXE_64,                       // time
	RXE_DATE,                     // date
	0,                            // 15
	RXE_SYM,                      // word
	RXE_SYM,                      // set-word
	RXE_SYM,                      // get-word
	RXE_SYM,                      // lit-word
	RXE_SYM,                      // refinement
	RXE_SYM,                      // issue
	0,                            // 22
	0,                            // 23
	RXE_SER,                      // string
	RXE_SER,                      // file
	RXE_SER,                      // email
	RXE_SER,                      // ref
	RXE_SER,                      // url
	RXE_SER,                      // tag
	0,                            // 30
	0,                            // 31
	RXE_SER,                      // block
	RXE_SER,                      // paren
	RXE_SER,                      // path
	RXE_SER,                      // set-path
	RXE_SER,                      // get-path
	RXE_SER,                      // lit-path
	0,                            // 38
	0,                            // 39
	RXE_SER,                      // binary
	RXE_SER,                      // bitset
	RXE_SER,                      // vector
	RXE_IMAGE,                    // image
	0,                            // 44
	0,                            // 45
	0,                            // 46
	RXE_SER,                      // gob
	RXE_OBJECT,                   // object
	RXE_OBJECT                    // module
};

#define RXT_ALLOWED_TYPES (((u64)1<<REB_UNSET) \
|((u64)1<<REB_NONE) \
|((u64)1<<REB_HANDLE) \
|((u64)1<<REB_LOGIC) \
|((u64)1<<REB_INTEGER) \
|((u64)1<<REB_DECIMAL) \
|((u64)1<<REB_PERCENT) \
|((u64)1<<REB_CHAR) \
|((u64)1<<REB_PAIR) \
|((u64)1<<REB_TUPLE) \
|((u64)1<<REB_TIME) \
|((u64)1<<REB_DATE) \
|((u64)1<<REB_WORD) \
|((u64)1<<REB_SET_WORD) \
|((u64)1<<REB_GET_WORD) \
|((u64)1<<REB_LIT_WORD) \
|((u64)1<<REB_REFINEMENT) \
|((u64)1<<REB_ISSUE) \
|((u64)1<<REB_STRING) \
|((u64)1<<REB_FILE) \
|((u64)1<<REB_EMAIL) \
|((u64)1<<REB_REF) \
|((u64)1<<REB_URL) \
|((u64)1<<REB_TAG) \
|((u64)1<<REB_BLOCK) \
|((u64)1<<REB_PAREN) \
|((u64)1<<REB_PATH) \
|((u64)1<<REB_SET_PATH) \
|((u64)1<<REB_GET_PATH) \
|((u64)1<<REB_LIT_PATH) \
|((u64)1<<REB_BINARY) \
|((u64)1<<REB_BITSET) \
|((u64)1<<REB_VECTOR) \
|((u64)1<<REB_IMAGE) \
|((u64)1<<REB_GOB) \
|((u64)1<<REB_OBJECT) \
|((u64)1<<REB_MODULE) \
)
