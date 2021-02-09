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
**  Title: Dialects
**  Build: A0
**  Date:  10-Feb-2021
**  File:  reb-dialect.h
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-boot.reb)
**
***********************************************************************/


enum REBOL_dialect_error {
	REB_DIALECT_END = 0,	// End of dialect block
	REB_DIALECT_MISSING,	// Requested dialect is missing or not valid
	REB_DIALECT_NO_CMD,		// Command needed before the arguments
	REB_DIALECT_BAD_SPEC,	// Dialect spec is not valid
	REB_DIALECT_BAD_ARG,	// The argument type does not match the dialect
	REB_DIALECT_EXTRA_ARG	// There are more args than the command needs
};

enum DIALECTS_object {
	DIALECTS_SELF = 0,
	DIALECTS_SECURE,
	DIALECTS_DRAW,
	DIALECTS_EFFECT,
	DIALECTS_TEXT,
	DIALECTS_REBCODE,
	DIALECTS_MAX
};

#define DIALECT_LIT_CMD 0x1000
