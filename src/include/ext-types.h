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
**  Title: Extension Types (Isolators)
**  Build: A0
**  Date:  10-Feb-2021
**  File:  ext-types.h
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-boot.reb)
**
***********************************************************************/


enum REBOL_Ext_Types
{
	RXT_END = 0,                  // 0
	RXT_UNSET,                    // 1
	RXT_NONE,                     // 2
	RXT_HANDLE,                   // 3
	RXT_LOGIC = 4,                // 4
	RXT_INTEGER,                  // 5
	RXT_DECIMAL,                  // 6
	RXT_PERCENT,                  // 7
	RXT_CHAR = 10,                // 8
	RXT_PAIR,                     // 9
	RXT_TUPLE,                    // 10
	RXT_TIME,                     // 11
	RXT_DATE,                     // 12
	RXT_WORD = 16,                // 13
	RXT_SET_WORD,                 // 14
	RXT_GET_WORD,                 // 15
	RXT_LIT_WORD,                 // 16
	RXT_REFINEMENT,               // 17
	RXT_ISSUE,                    // 18
	RXT_STRING = 24,              // 19
	RXT_FILE,                     // 20
	RXT_EMAIL,                    // 21
	RXT_REF,                      // 22
	RXT_URL,                      // 23
	RXT_TAG,                      // 24
	RXT_BLOCK = 32,               // 25
	RXT_PAREN,                    // 26
	RXT_PATH,                     // 27
	RXT_SET_PATH,                 // 28
	RXT_GET_PATH,                 // 29
	RXT_LIT_PATH,                 // 30
	RXT_BINARY = 40,              // 31
	RXT_BITSET,                   // 32
	RXT_VECTOR,                   // 33
	RXT_IMAGE,                    // 34
	RXT_GOB = 47,                 // 35
	RXT_OBJECT = 48,              // 36
	RXT_MODULE,                   // 37
    RXT_MAX
};
