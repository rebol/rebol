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
**  Title: Datatype Makers
**  Build: A0
**  Date:  10-Feb-2021
**  File:  maketypes.h
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-boot.reb)
**
***********************************************************************/


extern REBFLG MT_None(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Logic(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Decimal(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Pair(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Tuple(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Time(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Date(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_String(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Bitset(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Image(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Vector(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Block(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Map(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Datatype(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Typeset(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Function(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Object(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Gob(REBVAL *, REBVAL *, REBCNT);
extern REBFLG MT_Event(REBVAL *, REBVAL *, REBCNT);


/***********************************************************************
**
*/	const MAKE_FUNC Make_Dispatch[REB_MAX] =
/*
**		Specifies the make method used for each datatype.
**
***********************************************************************/
{
	0,                            // end
	MT_None,                      // unset
	MT_None,                      // none
	MT_Logic,                     // logic
	0,                            // integer
	MT_Decimal,                   // decimal
	MT_Decimal,                   // percent
	0,                            // money
	0,                            // char
	MT_Pair,                      // pair
	MT_Tuple,                     // tuple
	MT_Time,                      // time
	MT_Date,                      // date
	MT_String,                    // binary
	MT_String,                    // string
	MT_String,                    // file
	MT_String,                    // email
	MT_String,                    // ref
	MT_String,                    // url
	MT_String,                    // tag
	MT_Bitset,                    // bitset
	MT_Image,                     // image
	MT_Vector,                    // vector
	MT_Block,                     // block
	MT_Block,                     // paren
	MT_Block,                     // path
	MT_Block,                     // set-path
	MT_Block,                     // get-path
	MT_Block,                     // lit-path
	MT_Map,                       // map
	MT_Datatype,                  // datatype
	MT_Typeset,                   // typeset
	0,                            // word
	0,                            // set-word
	0,                            // get-word
	0,                            // lit-word
	0,                            // refinement
	0,                            // issue
	MT_Function,                  // native
	MT_Function,                  // action
	0,                            // rebcode
	MT_Function,                  // command
	MT_Function,                  // op
	MT_Function,                  // closure
	MT_Function,                  // function
	0,                            // frame
	MT_Object,                    // object
	MT_Object,                    // module
	MT_Object,                    // error
	MT_Object,                    // task
	0,                            // port
	MT_Gob,                       // gob
	MT_Event,                     // event
	0,                            // handle
	0,                            // struct
	0,                            // library
	0                             // utype
};
