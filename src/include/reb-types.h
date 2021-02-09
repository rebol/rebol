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
**  Title: Datatype Definitions
**  Build: A0
**  Date:  14-Jan-2021
**  File:  reb-types.h
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-boot.reb)
**
***********************************************************************/


/***********************************************************************
**
*/	enum REBOL_Types
/*
**		Internal datatype numbers. These change. Do not export.
**
***********************************************************************/
{
	REB_END,                      // 0
	REB_UNSET,                    // 1
	REB_NONE,                     // 2
	REB_LOGIC,                    // 3
	REB_INTEGER,                  // 4
	REB_DECIMAL,                  // 5
	REB_PERCENT,                  // 6
	REB_MONEY,                    // 7
	REB_CHAR,                     // 8
	REB_PAIR,                     // 9
	REB_TUPLE,                    // 10
	REB_TIME,                     // 11
	REB_DATE,                     // 12
	REB_BINARY,                   // 13
	REB_STRING,                   // 14
	REB_FILE,                     // 15
	REB_EMAIL,                    // 16
	REB_REF,                      // 17
	REB_URL,                      // 18
	REB_TAG,                      // 19
	REB_BITSET,                   // 20
	REB_IMAGE,                    // 21
	REB_VECTOR,                   // 22
	REB_BLOCK,                    // 23
	REB_PAREN,                    // 24
	REB_PATH,                     // 25
	REB_SET_PATH,                 // 26
	REB_GET_PATH,                 // 27
	REB_LIT_PATH,                 // 28
	REB_MAP,                      // 29
	REB_DATATYPE,                 // 30
	REB_TYPESET,                  // 31
	REB_WORD,                     // 32
	REB_SET_WORD,                 // 33
	REB_GET_WORD,                 // 34
	REB_LIT_WORD,                 // 35
	REB_REFINEMENT,               // 36
	REB_ISSUE,                    // 37
	REB_NATIVE,                   // 38
	REB_ACTION,                   // 39
	REB_REBCODE,                  // 40
	REB_COMMAND,                  // 41
	REB_OP,                       // 42
	REB_CLOSURE,                  // 43
	REB_FUNCTION,                 // 44
	REB_FRAME,                    // 45
	REB_OBJECT,                   // 46
	REB_MODULE,                   // 47
	REB_ERROR,                    // 48
	REB_TASK,                     // 49
	REB_PORT,                     // 50
	REB_GOB,                      // 51
	REB_EVENT,                    // 52
	REB_HANDLE,                   // 53
	REB_STRUCT,                   // 54
	REB_LIBRARY,                  // 55
	REB_UTYPE,                    // 56
    REB_MAX
};

/***********************************************************************
**
**	REBOL Type Check Macros
**
***********************************************************************/
#define IS_END(v)			(VAL_TYPE(v)==REB_END)
#define IS_UNSET(v)			(VAL_TYPE(v)==REB_UNSET)
#define IS_NONE(v)			(VAL_TYPE(v)==REB_NONE)
#define IS_LOGIC(v)			(VAL_TYPE(v)==REB_LOGIC)
#define IS_INTEGER(v)		(VAL_TYPE(v)==REB_INTEGER)
#define IS_DECIMAL(v)		(VAL_TYPE(v)==REB_DECIMAL)
#define IS_PERCENT(v)		(VAL_TYPE(v)==REB_PERCENT)
#define IS_MONEY(v)			(VAL_TYPE(v)==REB_MONEY)
#define IS_CHAR(v)			(VAL_TYPE(v)==REB_CHAR)
#define IS_PAIR(v)			(VAL_TYPE(v)==REB_PAIR)
#define IS_TUPLE(v)			(VAL_TYPE(v)==REB_TUPLE)
#define IS_TIME(v)			(VAL_TYPE(v)==REB_TIME)
#define IS_DATE(v)			(VAL_TYPE(v)==REB_DATE)
#define IS_BINARY(v)		(VAL_TYPE(v)==REB_BINARY)
#define IS_STRING(v)		(VAL_TYPE(v)==REB_STRING)
#define IS_FILE(v)			(VAL_TYPE(v)==REB_FILE)
#define IS_EMAIL(v)			(VAL_TYPE(v)==REB_EMAIL)
#define IS_REF(v)			(VAL_TYPE(v)==REB_REF)
#define IS_URL(v)			(VAL_TYPE(v)==REB_URL)
#define IS_TAG(v)			(VAL_TYPE(v)==REB_TAG)
#define IS_BITSET(v)		(VAL_TYPE(v)==REB_BITSET)
#define IS_IMAGE(v)			(VAL_TYPE(v)==REB_IMAGE)
#define IS_VECTOR(v)		(VAL_TYPE(v)==REB_VECTOR)
#define IS_BLOCK(v)			(VAL_TYPE(v)==REB_BLOCK)
#define IS_PAREN(v)			(VAL_TYPE(v)==REB_PAREN)
#define IS_PATH(v)			(VAL_TYPE(v)==REB_PATH)
#define IS_SET_PATH(v)		(VAL_TYPE(v)==REB_SET_PATH)
#define IS_GET_PATH(v)		(VAL_TYPE(v)==REB_GET_PATH)
#define IS_LIT_PATH(v)		(VAL_TYPE(v)==REB_LIT_PATH)
#define IS_MAP(v)			(VAL_TYPE(v)==REB_MAP)
#define IS_DATATYPE(v)		(VAL_TYPE(v)==REB_DATATYPE)
#define IS_TYPESET(v)		(VAL_TYPE(v)==REB_TYPESET)
#define IS_WORD(v)			(VAL_TYPE(v)==REB_WORD)
#define IS_SET_WORD(v)		(VAL_TYPE(v)==REB_SET_WORD)
#define IS_GET_WORD(v)		(VAL_TYPE(v)==REB_GET_WORD)
#define IS_LIT_WORD(v)		(VAL_TYPE(v)==REB_LIT_WORD)
#define IS_REFINEMENT(v)	(VAL_TYPE(v)==REB_REFINEMENT)
#define IS_ISSUE(v)			(VAL_TYPE(v)==REB_ISSUE)
#define IS_NATIVE(v)		(VAL_TYPE(v)==REB_NATIVE)
#define IS_ACTION(v)		(VAL_TYPE(v)==REB_ACTION)
#define IS_REBCODE(v)		(VAL_TYPE(v)==REB_REBCODE)
#define IS_COMMAND(v)		(VAL_TYPE(v)==REB_COMMAND)
#define IS_OP(v)			(VAL_TYPE(v)==REB_OP)
#define IS_CLOSURE(v)		(VAL_TYPE(v)==REB_CLOSURE)
#define IS_FUNCTION(v)		(VAL_TYPE(v)==REB_FUNCTION)
#define IS_FRAME(v)			(VAL_TYPE(v)==REB_FRAME)
#define IS_OBJECT(v)		(VAL_TYPE(v)==REB_OBJECT)
#define IS_MODULE(v)		(VAL_TYPE(v)==REB_MODULE)
#define IS_ERROR(v)			(VAL_TYPE(v)==REB_ERROR)
#define IS_TASK(v)			(VAL_TYPE(v)==REB_TASK)
#define IS_PORT(v)			(VAL_TYPE(v)==REB_PORT)
#define IS_GOB(v)			(VAL_TYPE(v)==REB_GOB)
#define IS_EVENT(v)			(VAL_TYPE(v)==REB_EVENT)
#define IS_HANDLE(v)		(VAL_TYPE(v)==REB_HANDLE)
#define IS_STRUCT(v)		(VAL_TYPE(v)==REB_STRUCT)
#define IS_LIBRARY(v)		(VAL_TYPE(v)==REB_LIBRARY)
#define IS_UTYPE(v)			(VAL_TYPE(v)==REB_UTYPE)

/***********************************************************************
**
**	REBOL Typeset Defines
**
***********************************************************************/
#define TS_NUMBER (((REBU64)1<<REB_INTEGER)|((REBU64)1<<REB_DECIMAL)|((REBU64)1<<REB_PERCENT))
#define TS_SCALAR (((REBU64)1<<REB_INTEGER)|((REBU64)1<<REB_DECIMAL)|((REBU64)1<<REB_PERCENT)|((REBU64)1<<REB_MONEY)|((REBU64)1<<REB_CHAR)|((REBU64)1<<REB_PAIR)|((REBU64)1<<REB_TUPLE)|((REBU64)1<<REB_TIME)|((REBU64)1<<REB_DATE))
#define TS_SERIES (((REBU64)1<<REB_BINARY)|((REBU64)1<<REB_STRING)|((REBU64)1<<REB_FILE)|((REBU64)1<<REB_EMAIL)|((REBU64)1<<REB_REF)|((REBU64)1<<REB_URL)|((REBU64)1<<REB_TAG)|((REBU64)1<<REB_IMAGE)|((REBU64)1<<REB_VECTOR)|((REBU64)1<<REB_BLOCK)|((REBU64)1<<REB_PAREN)|((REBU64)1<<REB_PATH)|((REBU64)1<<REB_SET_PATH)|((REBU64)1<<REB_GET_PATH)|((REBU64)1<<REB_LIT_PATH))
#define TS_STRING (((REBU64)1<<REB_STRING)|((REBU64)1<<REB_FILE)|((REBU64)1<<REB_EMAIL)|((REBU64)1<<REB_REF)|((REBU64)1<<REB_URL)|((REBU64)1<<REB_TAG))
#define TS_BLOCK (((REBU64)1<<REB_BLOCK)|((REBU64)1<<REB_PAREN)|((REBU64)1<<REB_PATH)|((REBU64)1<<REB_SET_PATH)|((REBU64)1<<REB_GET_PATH)|((REBU64)1<<REB_LIT_PATH))
#define TS_PATH (((REBU64)1<<REB_PATH)|((REBU64)1<<REB_SET_PATH)|((REBU64)1<<REB_GET_PATH)|((REBU64)1<<REB_LIT_PATH))
#define TS_WORD (((REBU64)1<<REB_WORD)|((REBU64)1<<REB_SET_WORD)|((REBU64)1<<REB_GET_WORD)|((REBU64)1<<REB_LIT_WORD)|((REBU64)1<<REB_REFINEMENT)|((REBU64)1<<REB_ISSUE))
#define TS_FUNCTION (((REBU64)1<<REB_NATIVE)|((REBU64)1<<REB_ACTION)|((REBU64)1<<REB_REBCODE)|((REBU64)1<<REB_COMMAND)|((REBU64)1<<REB_OP)|((REBU64)1<<REB_CLOSURE)|((REBU64)1<<REB_FUNCTION))
#define TS_OBJECT (((REBU64)1<<REB_OBJECT)|((REBU64)1<<REB_MODULE)|((REBU64)1<<REB_ERROR)|((REBU64)1<<REB_TASK)|((REBU64)1<<REB_PORT))
