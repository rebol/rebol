REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Make primary boot files"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Author: "Carl Sassenrath"
	Version: 2.100.0
	Needs: 2.100.100
	Purpose: {
		A lot of the REBOL system is built by REBOL, and this program
		does most of the serious work. It generates most of the C include
		files required to compile REBOL.
	}
]

print "--- Make Boot : System Embedded Script ---"

do %form-header.r

; Set platform TARGET
do %systems.r
target: config-system/define ; default

; Include graphics for these systems:
graphics-targets: [
	TO_WIN32
]
has-graphics: false ;not not find graphics-targets target

opts: system/options/args

if all [block? opts opts/1 = ">"] [opts: none] ; cw editor

if block? opts [
	if find opts "no-gfx" [
		has-graphics: false
		opts: next opts
	]
	if not tail? opts [
		opts: load first opts
		unless tuple? opts [print "Invalid version arg." wait 2 quit]
		target: config-system/platform opts
	]
]

write-if: func [file data] [
	if data <> attempt [read file][
		print ["UPDATE:" file]
		write file data
	]
]

;-- SETUP --------------------------------------------------------------

change-dir %../boot/
;dir: %../core/temp/  ; temporary definition
inc: %../include/
src: %../core/

version: load %version.r
either tuple? opts [
	version/4: opts/2
	version/5: opts/3
][
	version/4: system/version/4
	version/5: system/version/5
]

;-- Title string put into boot.h file checksum:
Title:
{REBOL
Copyright 2012 REBOL Technologies
REBOL is a trademark of REBOL Technologies
Licensed under the Apache License, Version 2.0
}

sections: [
	boot-types
	boot-words
	boot-root
	boot-task
	boot-strings
	boot-booters
	boot-actions
	boot-natives
	boot-ops
	boot-typespecs
	boot-errors
	boot-sysobj
	boot-base
	boot-sys
	boot-mezz
	boot-protocols
;	boot-script
]

include-protocols: false      ; include protocols in build

;-- Error handler:
error: func [msg arg] [print ["*** Make-boot error:" msg arg] halt]

;-- Args passed: platform, product
if none? args: system/options/args [error "No platform specified." ""]

if args/1 = ">" [args: ["Win32" "VIEW-PRO"]] ; for debugging only
product: to-word any [args/2  "core"]

platform-data: context [type: 'windows]
build: context [features: [help-strings]]

;-- Fetch platform specifications:
;init-build-objects/platform platform
;platform-data: platforms/:platform
;build: platform-data/builds/:product

;-- UTILITIES ----------------------------------------------------------

up-word: func [w] [
	w: uppercase form w
	foreach [f t] [
		#"-" #"_"
	][replace/all w f t]
	w
]

;-- Emit Function
out: make string! 100000
emit: func [data] [repend out data]

to-c-name: func [word] [
	word: form word
	foreach [f t] [
		#"-" #"_"
		#"." #"_"
		#"?" #"q"
		#"!" #"x"
		#"~" ""
		#"*" "_p"
		#"+" "_add"
		#"|" "or_bar"
	][replace/all word f t]
	word
]

emit-enum: func [word] [emit [tab to-c-name word "," newline]]

emit-line: func [prefix word cmt /var /define /code /decl /up1 /local str][

	str: to-c-name word

	if word = 0 [prefix: ""]
	
	if not any [code decl] [
		either var [uppercase/part str 1] [uppercase str]
	]

	if up1 [uppercase/part str 1]

	str: any [
		if define [rejoin [prefix str]]
		if code   [rejoin ["    " prefix str cmt]]
		if decl   [rejoin [prefix str cmt]]
		rejoin ["    " prefix str ","]
	]
	if any [code decl] [cmt: none]
	if cmt [
		len: 31 - length? str
		loop to-integer len / 4 [append str tab]
		any [
			if define [repend str cmt]
			if cmt [repend str ["// " cmt]]
		]
	]
	append str newline
	append out str
]

emit-head: func [title [string!] file [file!]] [
	clear out
	emit form-header/gen title file %make-boot.r
]

emit-end: func [/easy] [
	if not easy [remove find/last out #","]
	append out {^};^/}
]

binary-to-c: either system/version/4 = 3 [
	; Windows format:
	func [comp-data /local out] [
		out: make string! 4 * (length? comp-data)
		forall comp-data [
			out: insert out reduce [to-integer first comp-data ", "]
			if zero? ((index? comp-data) // 10) [out: insert out "^/^-"]
		]
;		remove/part out either (pick out -1) = #" " [-2][-4]
		head out
	]
][
	; Other formats (Linux, OpenBSD, etc.):
	func [comp-data /local out data] [
		out: make string! 4 * (length? comp-data)
		forall comp-data [
			data: copy/part comp-data 16
			comp-data: skip comp-data 15
			data: enbase/base data 16
			forall data [
				insert data "\x"
				data: skip data 3
			]
			data: tail data
			insert data {"^/}
			append out {"}
			append out head data
		]
		head out
	]
]

remove-tests: func [d] [
	while [d: find d #test][remove/part d 2]
]

;----------------------------------------------------------------------------
;
; Evaltypes.h - Evaluation Dispatch Maps
;
;----------------------------------------------------------------------------

boot-types: load %types.r
type-record: [type evalclass typeclass moldtype formtype haspath maker typesets]

emit-head "Evaluation Maps" %evaltypes.h
emit {
/***********************************************************************
**
*/	const REBINT Eval_Type_Map[REB_MAX] =
/*
**		Specifies the evaluation method used for each datatype.
**
***********************************************************************/
^{
}

foreach :type-record boot-types [
	emit-line "ET_" evalclass type
]
emit-end

emit {
/***********************************************************************
**
*/	const REBDOF Func_Dispatch[] =
/*
**		The function evaluation dispatchers.
**
***********************************************************************/
^{
}

foreach :type-record boot-types [
	if find [function operator] evalclass [
		emit-line/var "Do_" type none
	]
]
emit-end

emit {
/***********************************************************************
**
*/	const REBACT Value_Dispatch[REB_MAX] =
/*
**		The ACTION dispatch function for each datatype.
**
***********************************************************************/
^{
}

foreach :type-record boot-types [
	emit-line/var "T_" typeclass type
]
emit-end

emit {
/***********************************************************************
**
*/	const REBPEF Path_Dispatch[REB_MAX] =
/*
**		The path evaluator function for each datatype.
**
***********************************************************************/
^{
}

foreach :type-record boot-types [
	emit-line/var "PD_" switch/default haspath [
		* [typeclass]
		- [0]
	][haspath] type
]
emit-end

write inc/tmp-evaltypes.h out


;----------------------------------------------------------------------------
;
; Maketypes.h - Dispatchers for Make (used by construct)
;
;----------------------------------------------------------------------------

emit-head "Datatype Makers" %maketypes.h
emit newline

types-used: []

foreach :type-record boot-types [
	if all [
		maker = '*
		word? typeclass
		not find types-used typeclass
	][
		emit-line/up1/decl "extern REBFLG MT_" typeclass "(REBVAL *, REBVAL *, REBCNT);"
		append types-used typeclass
	]
]

emit {

/***********************************************************************
**
*/	const MAKE_FUNC Make_Dispatch[REB_MAX] =
/*
**		Specifies the make method used for each datatype.
**
***********************************************************************/
^{
}

foreach :type-record boot-types [
 	either maker = '* [
		emit-line/var "MT_" typeclass type
	][
		emit-line "" "0"  type
	]
]

emit-end

write inc/tmp-maketypes.h out

;----------------------------------------------------------------------------
;
; Comptypes.h - compare functions
;
;----------------------------------------------------------------------------

emit-head "Datatype Comparison Functions" %comptypes.h
emit newline

types-used: []

foreach :type-record boot-types [
	if all [
		word? typeclass
		not find types-used typeclass
	][
		emit-line/up1/decl "extern REBINT CT_" typeclass "(REBVAL *, REBVAL *, REBINT);"
		append types-used typeclass
	]
]

emit {
/***********************************************************************
**
*/	const REBCTF Compare_Types[REB_MAX] =
/*
**		Type comparision functions.
**
***********************************************************************/
^{
}

foreach :type-record boot-types [
	emit-line/var "CT_" typeclass type
]
emit-end

write inc/tmp-comptypes.h out


;----------------------------------------------------------------------------
;
; Moldtypes.h - Dispatchers for Mold and Form
;
;----------------------------------------------------------------------------

;emit-head "Mold Dispatchers"
;
;emit {
;/***********************************************************************
;**
;*/	const MOLD_FUNC Mold_Dispatch[REB_MAX] =
;/*
;**		The MOLD dispatch function for each datatype.
;**
;***********************************************************************/
;^{
;}
;
;foreach :type-record boot-types [
;	f: "Mold_"
;	switch/default moldtype [
;		* [t: typeclass]
;		+ [t: type]
;		- [t: 0]
;	][t: uppercase/part form moldtype 1]
;	emit [tab "case " uppercase join "REB_" type ":" tab "\\" t]
;	emit newline
;	;emit-line/var f t type
;]
;emit-end
;
;emit {
;/***********************************************************************
;**
;*/	const MOLD_FUNC Form_Dispatch[REB_MAX] =
;/*
;**		The FORM dispatch function for each datatype.
;**
;***********************************************************************/
;^{
;}
;foreach :type-record boot-types [
;	f: "Mold_"
;	switch/default formtype [
;		*  [t: typeclass]
;		f* [t: typeclass f: "Form_"]
;		+  [t: type]
;		f+ [t: type f: "Form_"]
;		-  [t: 0]
;	][t: uppercase/part form moldtype 1]
;	emit [tab "case " uppercase join "REB_" type ":" tab "\\" t]
;	emit newline
;	;emit-line/var f t type
;]
;emit-end
;
;write inc/tmp-moldtypes.h out

;----------------------------------------------------------------------------
;
; Bootdefs.h - Boot include file
;
;----------------------------------------------------------------------------

emit-head "Datatype Definitions" %reb-types.h

emit [
{
/***********************************************************************
**
*/	enum REBOL_Types
/*
**		Internal datatype numbers. These change. Do not export.
**
***********************************************************************/
^{
}
]

datatypes: []
n: 0
foreach :type-record boot-types [
	append datatypes type
	emit-line "REB_" type n
	n: n + 1
]
emit {    REB_MAX
^};
}

emit {
/***********************************************************************
**
**	REBOL Type Check Macros
**
***********************************************************************/
}

new-types: []
foreach :type-record boot-types [
	append new-types to-word join type "!"
	str: uppercase form type
	replace/all str #"-" #"_"
	def: join {#define IS_} [str "(v)"]
	len: 31 - length? def
	loop to-integer len / 4 [append def tab]
	emit [def "(VAL_TYPE(v)==REB_" str ")" newline]
]

emit {
/***********************************************************************
**
**	REBOL Typeset Defines
**
***********************************************************************/
}

typeset-sets: []

foreach :type-record boot-types [
	typesets: compose [(typesets)]
	foreach ts typesets [
		spot: any [
			select typeset-sets ts
			first back insert tail typeset-sets reduce [ts copy []]
		]
		append spot type
	]
]
remove/part typeset-sets 2 ; the - markers

foreach [ts types] typeset-sets [
	emit ["#define TS_" up-word ts " ("]
	foreach t types [
		emit ["((REBU64)1<<REB_" up-word t ")|"]
	]
	append remove back tail out ")^/"
]

write-if inc/reb-types.h out

;----------------------------------------------------------------------------
;
; Extension Related Tables
;
;----------------------------------------------------------------------------

ext-types: load %types-ext.r
rxt-record: [type offset size]

; Generate type table with necessary gaps
rxt-types: []
n: 0
foreach :rxt-record ext-types [
	if integer? offset [
		insert/dup tail rxt-types 0 offset - n
		n: offset
	]
	append rxt-types type
	n: n + 1
]


emit-head "Extension Types (Isolators)" %ext-types.h

emit [
{
enum REBOL_Ext_Types
^{
}
]
n: 0
foreach :rxt-record ext-types [
	either integer? offset [
		emit-line "RXT_" rejoin [type " = " offset] n
	][
		emit-line "RXT_" type n
	]
	n: n + 1
]
emit {    RXT_MAX
^};
}

write inc/ext-types.h out ; part of Host-Kit distro

emit-head "Extension Type Equates" %tmp-exttypes.h
emit {
/***********************************************************************
**
*/	const REBYTE Reb_To_RXT[REB_MAX] =
/*
***********************************************************************/
^{
}

foreach :type-record boot-types [
	either find ext-types type [
		emit-line "RXT_" type type
	][
		emit-line "" 0 type
	]
]
emit-end

emit {
/***********************************************************************
**
*/	const REBYTE RXT_To_Reb[RXT_MAX] =
/*
***********************************************************************/
^{
}

n: 0
foreach type rxt-types [
	either word? type [emit-line "REB_" type n][
		emit-line "" 0 n
	]
	n: n + 1
]
emit-end

emit {
/***********************************************************************
**
*/	const REBCNT RXT_Eval_Class[RXT_MAX] =
/*
***********************************************************************/
^{
}

n: 0
foreach type rxt-types [
	either all [
		word? type
		rec: find ext-types type
	][
		emit-line "RXE_" rec/3 rec/1
	][
		emit-line "" 0 n
	]
	n: n + 1
]
emit-end

emit {
#define RXT_ALLOWED_TYPES (}
foreach type next rxt-types [
	if word? type [
		emit replace join "((u64)" uppercase rejoin ["1<<REB_" type ") \^/"] #"-" #"_"
		emit "|"
	]
]
remove back tail out
emit ")^/"

write inc/tmp-exttypes.h out


;----------------------------------------------------------------------------
;
; Bootdefs.h - Boot include file
;
;----------------------------------------------------------------------------

emit-head "Boot Definitions" %bootdefs.h

emit [
{
#define REBOL_VER } version/1 {
#define REBOL_REV } version/2 {
#define REBOL_UPD } version/3 {
#define REBOL_SYS } version/4 {
#define REBOL_VAR } version/5 {
}
]

;-- Generate Lower-Level String Table ----------------------------------------

emit {
/***********************************************************************
**
**	REBOL Boot Strings
**
**		These are special strings required during boot and other
**		operations. Putting them here hides them from exe hackers.
**		These are all string offsets within a single string.
**
***********************************************************************/
}

boot-strings: load %strings.r

code: ""
n: 0
foreach str boot-strings [
	either set-word? :str [
		emit-line/define "#define RS_" to word! str n ;R3
	][
		n: n + 1
		append code str
		append code null
	]
]

emit ["#define RS_MAX" tab n lf]
emit ["#define RS_SIZE" tab length? out lf]
boot-strings: to-binary code

;-- Generate Canonical Words (must follow datatypes above!) ------------------

emit {
/***********************************************************************
**
*/	enum REBOL_Symbols
/*
**		REBOL static canonical words (symbols) used with the code.
**
***********************************************************************/
^{
	SYM_NOT_USED = 0,
}

n: 1
foreach :type-record boot-types [
	emit-line "SYM_" join type "_type" n
	n: n + 1
]

boot-words: load %words.r

replace boot-words '*port-modes* load %modes.r

foreach word boot-words [
	emit-line "SYM_" word reform [n "-" word]
	n: n + 1
]
emit-end

;-- Generate Action Constants ------------------------------------------------

emit {
/***********************************************************************
**
*/	enum REBOL_Actions
/*
**		REBOL datatype action numbers.
**
***********************************************************************/
^{
}

boot-actions: load %actions.r
n: 1
emit-line "A_" "type = 0" "Handled by interpreter"
foreach word boot-actions [
	if set-word? :word [
		emit-line "A_" to word! :word n ;R3
		n: n + 1
	]
]
emit [tab "A_MAX_ACTION" lf "};"]
emit {

#define IS_BINARY_ACT(a) ((a) <= A_XOR)
}
print [n "actions"]

write inc/tmp-bootdefs.h out

;----------------------------------------------------------------------------
;
; Sysobj.h - System Object Selectors
;
;----------------------------------------------------------------------------

emit-head "System Object" %sysobj.h
emit newline

at-value: func ['field] [next find boot-sysobj to-set-word field]

boot-sysobj: load %sysobj.r
change at-value version version
when: now
when: when - when/zone
when/zone: 0:00
change at-value build when
change at-value product to lit-word! product


plats: load %platforms.r

change/only at-value platform reduce [
	any [pick plats version/4 * 2 - 1 "Unknown"]
	any [select pick plats version/4 * 2 version/5 ""]
]

ob: context boot-sysobj

make-obj-defs: func [obj prefix depth /local f] [
	uppercase prefix
	emit ["enum " prefix "object {" newline]
	emit-line prefix "SELF = 0" none
	foreach field words-of obj [ ;R3
		emit-line prefix field none
	]
	emit [tab uppercase join prefix "MAX^/"]
	emit "};^/^/"

	if depth > 1 [
		foreach field words-of obj [ ;R3
			f: join prefix [field #"_"]
			replace/all f "-" "_"
			all [
				field <> 'standard
				object? get in obj field
				make-obj-defs obj/:field f depth - 1
			]
		]
	]
]

make-obj-defs ob "SYS_" 1
make-obj-defs ob/catalog "CAT_" 4
make-obj-defs ob/contexts "CTX_" 4
make-obj-defs ob/standard "STD_" 4
make-obj-defs ob/state "STATE_" 4
;make-obj-defs ob/network "NET_" 4
make-obj-defs ob/ports "PORTS_" 4
make-obj-defs ob/options "OPTIONS_" 4
;make-obj-defs ob/intrinsic "INTRINSIC_" 4
make-obj-defs ob/locale "LOCALE_" 4
make-obj-defs ob/view "VIEW_" 4

write inc/tmp-sysobj.h out

;----------------------------------------------------------------------------

emit-head "Dialects" %reb-dialect.h
emit {
enum REBOL_dialect_error {
	REB_DIALECT_END = 0,	// End of dialect block
	REB_DIALECT_MISSING,	// Requested dialect is missing or not valid
	REB_DIALECT_NO_CMD,		// Command needed before the arguments
	REB_DIALECT_BAD_SPEC,	// Dialect spec is not valid
	REB_DIALECT_BAD_ARG,	// The argument type does not match the dialect
	REB_DIALECT_EXTRA_ARG	// There are more args than the command needs
};

}
make-obj-defs ob/dialects "DIALECTS_" 4

emit {#define DIALECT_LIT_CMD 0x1000
}

write inc/reb-dialect.h out


;----------------------------------------------------------------------------
;
; Event Types
;
;----------------------------------------------------------------------------

emit-head "Event Types" %reb-evtypes.h
emit newline

emit ["enum event_types {" newline]
foreach field ob/view/event-types [
	emit-line "EVT_" field none
]
emit [tab "EVT_MAX^/"]
emit "};^/^/"

emit ["enum event_keys {" newline]
emit-line "EVK_" "NONE" none
foreach field ob/view/event-keys [
	emit-line "EVK_" field none
]
emit [tab "EVK_MAX^/"]
emit "};^/^/"

write inc/reb-evtypes.h out


;----------------------------------------------------------------------------
;
; Error Constants
;
;----------------------------------------------------------------------------

;-- Error Structure ----------------------------------------------------------

emit-head "Error Structure and Constants" %errnums.h

emit {
#ifdef VAL_TYPE
/***********************************************************************
**
*/	typedef struct REBOL_Error_Obj
/*
***********************************************************************/
^{
}
; Generate ERROR object and append it to bootdefs.h:
emit-line/code "REBVAL " 'self ";" ;R3
foreach word words-of ob/standard/error [ ;R3
	if word = 'near [word: 'nearest] ; prevents C problem
	emit-line/code "REBVAL " word ";"
]
emit {^} ERROR_OBJ;
#endif
}

emit {
/***********************************************************************
**
*/	enum REBOL_Errors
/*
***********************************************************************/
^{
}

boot-errors: load %errors.r
err-list: make block! 200
errs: false

foreach [cat msgs] boot-errors [
	code: second msgs
	new1: true
	foreach [word val] skip msgs 4 [
		err: uppercase form to word! word ;R3
		replace/all err "-" "_"
		if find err-list err [print ["DUPLICATE Error Constant:" err] errs: true]
		append err-list err
		either new1 [
			emit-line "RE_" reform [err "=" code] reform [code mold val]
			new1: false
		][
			emit-line "RE_" err reform [code mold val]
		]
		code: code + 1
	]
	emit-line "RE_" join to word! cat "_max" none ;R3
	emit newline
]
if errs [wait 3 quit]
emit-end

emit {
#define RE_NOTE RE_NO_LOAD
#define RE_USER RE_MESSAGE
}

write inc/tmp-errnums.h out

;-------------------------------------------------------------------------

emit-head "Port Modes" %port-modes.h

data: load %modes.r

emit {
enum port_modes ^{
}

foreach word data [
	emit-enum word
]
emit-end

write inc/tmp-portmodes.h out

;----------------------------------------------------------------------------
;
; Load Boot Mezzanine Functions - Base, Sys, and Plus
;
;----------------------------------------------------------------------------

;-- Add other MEZZ functions:
mezz-files: load %../mezz/boot-files.r ; base lib, sys, mezz

;append boot-mezz+ none ?? why was this needed?

foreach section [boot-base boot-sys boot-mezz] [
	set section make block! 200
	foreach file first mezz-files [
		append get section load join %../mezz/ file
	]
	remove-tests get section
	mezz-files: next mezz-files
]

boot-protocols: make block! 20
foreach file first mezz-files [
	m: load/all join %../mezz/ file ; not REBOL word
	append/only append/only boot-protocols m/2 skip m 2
]

emit-head "Sys Context" %sysctx.h
sctx: construct boot-sys
make-obj-defs sctx "SYS_CTX_" 1
write inc/tmp-sysctx.h out


;----------------------------------------------------------------------------
;
; b-boot.c - Boot data file
;
;----------------------------------------------------------------------------

;-- Build b-boot.c output file -------------------------------------------------


emit-head "Natives and Bootstrap" %b-boot.c
emit {
#include "sys-core.h"

}

externs: make string! 2000
boot-booters: load %booters.r
boot-natives: load %natives.r

if has-graphics [append boot-natives load %graphics.r]

nats: append copy boot-booters boot-natives

n: boot-sys
;while [n: find n 'native] [
;	if set-word? first back n [
;		print index? n
;		append nats copy/part back n 3
;	]
;	n: next next n
;]

nat-count: 0

foreach val nats [
	if set-word? val [
		emit-line/decl "REBNATIVE(" to word! val ");" ;R3
		nat-count: nat-count + 1
	]
]

print [nat-count "natives"]

emit [newline {const REBFUN Native_Funcs[} nat-count {] = ^{
}]
foreach val nats [
	if set-word? val [
		emit-line/code "N_" to word! val "," ;R3
	]
	;nat-count: nat-count + 1
]
emit-end
emit newline

;-- Embedded REBOL Tests:
;where: find boot/script 'tests
;if where [
;	remove where
;	foreach file sort load %../tests/ [
;		test: load join %../tests/ file
;		if test/1 <> 'skip-test [
;			where: insert where test
;		]
;	]
;]

;-- Build typespecs block (in same order as datatypes table):
boot-typespecs: make block! 100
specs: load %typespec.r
foreach type datatypes [
	append/only boot-typespecs select specs type
]

;-- Create main code section (compressed):
boot-types: new-types
boot-root: load %root.r
boot-task: load %task.r
boot-ops: load %ops.r
;boot-script: load %script.r

write %boot-code.r mold reduce sections
data: mold/flat reduce sections
insert data reduce ["; Copyright (C) REBOL Technologies " now newline]
insert tail data make char! 0 ; scanner requires zero termination

comp-data: compress data: to-binary data

emit [
{
	// This array contains 4 bytes encoding a 32-bit little endian value,
	// followed by data which is the DEFLATE-algorithm-compressed
	// representation of the textual function specs for Rebol's native
	// routines.  This textual representation is null-terminated.
	// The leading value represents the expected length of
	// the text after it is decompressed (this is redundant with
	// information saved by DEFLATE, but having it twice provides a
	// redundant sanity check on the compression and decompression)
}
]

emit ["const REBYTE Native_Specs[" 4 + length? comp-data "] = {^/^-"]

;-- Prefix with the length
data-len-bin: to binary! length? data
assert [parse data-len-bin [4 #{00} 4 skip]] ;-- See CC #2064
emit binary-to-c reverse (skip data-len-bin 4)

;-- Convert UTF-8 binary to C-encoded string:
emit binary-to-c comp-data
emit-end/easy

write src/b-boot.c out

;-- Output stats:
print [
	"Compressed" length? data "to" length? comp-data "bytes:"
	to-integer ((length? comp-data) / (length? data) * 100)
	"percent of original"
]

;-- Create platform string:
;platform: to-string platform
;lowercase platform
;if platform-data/type = 'windows [ ; Why?? Not sure.
;	product: to-string product
;	lowercase product
;	replace/all product "-" ""
;]
;;dir: to-file rejoin [%../to- platform "/" product "/temp/"]

;----------------------------------------------------------------------------
;
; Boot.h - Boot header file
;
;----------------------------------------------------------------------------

emit-head "Bootstrap Structure and Root Module" %boot.h

emit [
{
#define MAX_NATS      } nat-count {
#define NAT_SPEC_SIZE } length? comp-data {
#define CHECK_TITLE   } checksum to binary! title {

extern const REBYTE Native_Specs[];
extern const REBFUN Native_Funcs[];

typedef struct REBOL_Boot_Block ^{
}
]

foreach word sections [
	word: form word
	remove/part word 5 ; boot_
	emit-line/code "REBVAL " word ";"
]
emit "} BOOT_BLK;"

;-------------------

emit [
{

//**** ROOT Context (Root Module):

typedef struct REBOL_Root_Context ^{
}
]

foreach word boot-root [
	emit-line/code "REBVAL " word ";"
]
emit ["} ROOT_CTX;" lf lf]

n: 0
foreach word boot-root [
	emit-line/define "#define ROOT_" word join "(&Root_Context->" [lowercase replace/all form word #"-" #"_" ")"]
	n: n + 1
]
emit ["#define ROOT_MAX " n lf]

;-------------------

emit [
{

//**** Task Context

typedef struct REBOL_Task_Context ^{
}
]

foreach word boot-task [
	emit-line/code "REBVAL " word ";"
]
emit ["} TASK_CTX;" lf lf]

n: 0
foreach word boot-task [
	emit-line/define "#define TASK_" word join "(&Task_Context->" [lowercase replace/all form word #"-" #"_" ")"]
	n: n + 1
]
emit ["#define TASK_MAX " n lf]

write inc/tmp-boot.h out
;print ask "-DONE-"
;wait .3
print "   "
