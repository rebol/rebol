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
**  Title: Evaluation Maps
**  Build: A0
**  Date:  10-Feb-2021
**  File:  evaltypes.h
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-boot.reb)
**
***********************************************************************/


/***********************************************************************
**
*/	const REBINT Eval_Type_Map[REB_MAX] =
/*
**		Specifies the evaluation method used for each datatype.
**
***********************************************************************/
{
	ET_END,                       // end
	ET_SELF,                      // unset
	ET_SELF,                      // none
	ET_SELF,                      // logic
	ET_SELF,                      // integer
	ET_SELF,                      // decimal
	ET_SELF,                      // percent
	ET_SELF,                      // money
	ET_SELF,                      // char
	ET_SELF,                      // pair
	ET_SELF,                      // tuple
	ET_SELF,                      // time
	ET_SELF,                      // date
	ET_SELF,                      // binary
	ET_SELF,                      // string
	ET_SELF,                      // file
	ET_SELF,                      // email
	ET_SELF,                      // ref
	ET_SELF,                      // url
	ET_SELF,                      // tag
	ET_SELF,                      // bitset
	ET_SELF,                      // image
	ET_SELF,                      // vector
	ET_SELF,                      // block
	ET_PAREN,                     // paren
	ET_PATH,                      // path
	ET_PATH,                      // set-path
	ET_PATH,                      // get-path
	ET_LIT_PATH,                  // lit-path
	ET_SELF,                      // map
	ET_SELF,                      // datatype
	ET_SELF,                      // typeset
	ET_WORD,                      // word
	ET_SET_WORD,                  // set-word
	ET_GET_WORD,                  // get-word
	ET_LIT_WORD,                  // lit-word
	ET_SELF,                      // refinement
	ET_SELF,                      // issue
	ET_FUNCTION,                  // native
	ET_FUNCTION,                  // action
	ET_FUNCTION,                  // rebcode
	ET_FUNCTION,                  // command
	ET_OPERATOR,                  // op
	ET_FUNCTION,                  // closure
	ET_FUNCTION,                  // function
	ET_INVALID,                   // frame
	ET_SELF,                      // object
	ET_SELF,                      // module
	ET_SELF,                      // error
	ET_SELF,                      // task
	ET_SELF,                      // port
	ET_SELF,                      // gob
	ET_SELF,                      // event
	ET_SELF,                      // handle
	ET_INVALID,                   // struct
	ET_INVALID,                   // library
	ET_SELF                       // utype
};

/***********************************************************************
**
*/	const REBDOF Func_Dispatch[] =
/*
**		The function evaluation dispatchers.
**
***********************************************************************/
{
	Do_Native,
	Do_Action,
	Do_Rebcode,
	Do_Command,
	Do_Op,
	Do_Closure,
	Do_Function 
};

/***********************************************************************
**
*/	const REBACT Value_Dispatch[REB_MAX] =
/*
**		The ACTION dispatch function for each datatype.
**
***********************************************************************/
{
	0,                            // end
	T_None,                       // unset
	T_None,                       // none
	T_Logic,                      // logic
	T_Integer,                    // integer
	T_Decimal,                    // decimal
	T_Decimal,                    // percent
	T_Money,                      // money
	T_Char,                       // char
	T_Pair,                       // pair
	T_Tuple,                      // tuple
	T_Time,                       // time
	T_Date,                       // date
	T_String,                     // binary
	T_String,                     // string
	T_String,                     // file
	T_String,                     // email
	T_String,                     // ref
	T_String,                     // url
	T_String,                     // tag
	T_Bitset,                     // bitset
	T_Image,                      // image
	T_Vector,                     // vector
	T_Block,                      // block
	T_Block,                      // paren
	T_Block,                      // path
	T_Block,                      // set-path
	T_Block,                      // get-path
	T_Block,                      // lit-path
	T_Map,                        // map
	T_Datatype,                   // datatype
	T_Typeset,                    // typeset
	T_Word,                       // word
	T_Word,                       // set-word
	T_Word,                       // get-word
	T_Word,                       // lit-word
	T_Word,                       // refinement
	T_Word,                       // issue
	T_Function,                   // native
	T_Function,                   // action
	0,                            // rebcode
	T_Function,                   // command
	T_Function,                   // op
	T_Function,                   // closure
	T_Function,                   // function
	T_Frame,                      // frame
	T_Object,                     // object
	T_Object,                     // module
	T_Object,                     // error
	T_Object,                     // task
	T_Port,                       // port
	T_Gob,                        // gob
	T_Event,                      // event
	0,                            // handle
	0,                            // struct
	0,                            // library
	T_Utype                       // utype
};

/***********************************************************************
**
*/	const REBPEF Path_Dispatch[REB_MAX] =
/*
**		The path evaluator function for each datatype.
**
***********************************************************************/
{
	0,                            // end
	0,                            // unset
	0,                            // none
	0,                            // logic
	0,                            // integer
	0,                            // decimal
	0,                            // percent
	0,                            // money
	0,                            // char
	PD_Pair,                      // pair
	PD_Tuple,                     // tuple
	PD_Time,                      // time
	PD_Date,                      // date
	PD_String,                    // binary
	PD_String,                    // string
	PD_File,                      // file
	PD_String,                    // email
	PD_String,                    // ref
	PD_File,                      // url
	PD_String,                    // tag
	PD_Bitset,                    // bitset
	PD_Image,                     // image
	PD_Vector,                    // vector
	PD_Block,                     // block
	PD_Block,                     // paren
	PD_Block,                     // path
	PD_Block,                     // set-path
	PD_Block,                     // get-path
	PD_Block,                     // lit-path
	PD_Map,                       // map
	0,                            // datatype
	0,                            // typeset
	0,                            // word
	0,                            // set-word
	0,                            // get-word
	0,                            // lit-word
	0,                            // refinement
	0,                            // issue
	0,                            // native
	0,                            // action
	0,                            // rebcode
	0,                            // command
	0,                            // op
	0,                            // closure
	0,                            // function
	PD_Frame,                     // frame
	PD_Object,                    // object
	PD_Object,                    // module
	PD_Object,                    // error
	PD_Object,                    // task
	PD_Object,                    // port
	PD_Gob,                       // gob
	PD_Event,                     // event
	0,                            // handle
	0,                            // struct
	0,                            // library
	0                             // utype
};
