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
**  Title: Datatype Comparison Functions
**  Build: A0
**  Date:  14-Jan-2021
**  File:  comptypes.h
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-boot.reb)
**
***********************************************************************/


extern REBINT CT_None(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Logic(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Integer(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Decimal(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Money(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Char(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Pair(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Tuple(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Time(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Date(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_String(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Bitset(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Image(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Vector(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Block(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Map(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Datatype(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Typeset(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Word(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Function(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Frame(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Object(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Port(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Gob(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Event(REBVAL *, REBVAL *, REBINT);
extern REBINT CT_Utype(REBVAL *, REBVAL *, REBINT);

/***********************************************************************
**
*/	const REBCTF Compare_Types[REB_MAX] =
/*
**		Type comparision functions.
**
***********************************************************************/
{
	0,                            // end
	CT_None,                      // unset
	CT_None,                      // none
	CT_Logic,                     // logic
	CT_Integer,                   // integer
	CT_Decimal,                   // decimal
	CT_Decimal,                   // percent
	CT_Money,                     // money
	CT_Char,                      // char
	CT_Pair,                      // pair
	CT_Tuple,                     // tuple
	CT_Time,                      // time
	CT_Date,                      // date
	CT_String,                    // binary
	CT_String,                    // string
	CT_String,                    // file
	CT_String,                    // email
	CT_String,                    // ref
	CT_String,                    // url
	CT_String,                    // tag
	CT_Bitset,                    // bitset
	CT_Image,                     // image
	CT_Vector,                    // vector
	CT_Block,                     // block
	CT_Block,                     // paren
	CT_Block,                     // path
	CT_Block,                     // set-path
	CT_Block,                     // get-path
	CT_Block,                     // lit-path
	CT_Map,                       // map
	CT_Datatype,                  // datatype
	CT_Typeset,                   // typeset
	CT_Word,                      // word
	CT_Word,                      // set-word
	CT_Word,                      // get-word
	CT_Word,                      // lit-word
	CT_Word,                      // refinement
	CT_Word,                      // issue
	CT_Function,                  // native
	CT_Function,                  // action
	0,                            // rebcode
	CT_Function,                  // command
	CT_Function,                  // op
	CT_Function,                  // closure
	CT_Function,                  // function
	CT_Frame,                     // frame
	CT_Object,                    // object
	CT_Object,                    // module
	CT_Object,                    // error
	CT_Object,                    // task
	CT_Port,                      // port
	CT_Gob,                       // gob
	CT_Event,                     // event
	0,                            // handle
	0,                            // struct
	0,                            // library
	CT_Utype                      // utype
};
