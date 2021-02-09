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
**  Title: Bootstrap Structure and Root Module
**  Build: A0
**  Date:  14-Jan-2021
**  File:  boot.h
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-boot.reb)
**
***********************************************************************/


#define MAX_NATS      203
#define NAT_SPEC_SIZE 102659
#define CHECK_TITLE   13607632

extern const REBYTE Native_Specs[];
extern const REBFUN Native_Funcs[];

typedef struct REBOL_Boot_Block {
	REBVAL types;
	REBVAL words;
	REBVAL root;
	REBVAL task;
	REBVAL strings;
	REBVAL booters;
	REBVAL actions;
	REBVAL natives;
	REBVAL ops;
	REBVAL typespecs;
	REBVAL errors;
	REBVAL sysobj;
	REBVAL base;
	REBVAL sys;
	REBVAL mezz;
	REBVAL protocols;
} BOOT_BLK;

//**** ROOT Context (Root Module):

typedef struct REBOL_Root_Context {
	REBVAL self;
	REBVAL root;
	REBVAL system;
	REBVAL errobj;
	REBVAL strings;
	REBVAL typesets;
	REBVAL noneval;
	REBVAL noname;
	REBVAL boot;
} ROOT_CTX;

#define ROOT_SELF              (&Root_Context->self)
#define ROOT_ROOT              (&Root_Context->root)
#define ROOT_SYSTEM            (&Root_Context->system)
#define ROOT_ERROBJ            (&Root_Context->errobj)
#define ROOT_STRINGS           (&Root_Context->strings)
#define ROOT_TYPESETS          (&Root_Context->typesets)
#define ROOT_NONEVAL           (&Root_Context->noneval)
#define ROOT_NONAME            (&Root_Context->noname)
#define ROOT_BOOT              (&Root_Context->boot)
#define ROOT_MAX 9


//**** Task Context

typedef struct REBOL_Task_Context {
	REBVAL self;
	REBVAL stack;
	REBVAL ballast;
	REBVAL max_ballast;
	REBVAL this_error;
	REBVAL this_value;
	REBVAL stack_error;
	REBVAL this_context;
	REBVAL buf_emit;
	REBVAL buf_words;
	REBVAL buf_utf8;
	REBVAL buf_print;
	REBVAL buf_form;
	REBVAL buf_mold;
	REBVAL mold_loop;
	REBVAL err_temps;
} TASK_CTX;

#define TASK_SELF              (&Task_Context->self)
#define TASK_STACK             (&Task_Context->stack)
#define TASK_BALLAST           (&Task_Context->ballast)
#define TASK_MAX_BALLAST       (&Task_Context->max_ballast)
#define TASK_THIS_ERROR        (&Task_Context->this_error)
#define TASK_THIS_VALUE        (&Task_Context->this_value)
#define TASK_STACK_ERROR       (&Task_Context->stack_error)
#define TASK_THIS_CONTEXT      (&Task_Context->this_context)
#define TASK_BUF_EMIT          (&Task_Context->buf_emit)
#define TASK_BUF_WORDS         (&Task_Context->buf_words)
#define TASK_BUF_UTF8          (&Task_Context->buf_utf8)
#define TASK_BUF_PRINT         (&Task_Context->buf_print)
#define TASK_BUF_FORM          (&Task_Context->buf_form)
#define TASK_BUF_MOLD          (&Task_Context->buf_mold)
#define TASK_MOLD_LOOP         (&Task_Context->mold_loop)
#define TASK_ERR_TEMPS         (&Task_Context->err_temps)
#define TASK_MAX 16
