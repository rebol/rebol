REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Make primary boot files"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2023 Rebol Open Source Contributors
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Author: "Carl Sassenrath"
	Version: 2.100.0
	Needs: 3.5.0
	Purpose: {
		A lot of the REBOL system is built by REBOL, and this program
		does most of the serious work. It generates most of the C include
		files required to compile REBOL.
	}
	Note: {
		Some code may look strange, but that is because this code may be evaluated
		by older bootstrap executables, which may be not fully functional!
	}
]

context [ ; wrapped to prevent colisions with other build scripts

;platform-data: context [type: 'windows]
;build: context [features: [help-strings]]

;-- SETUP --------------------------------------------------------------

inc: root-dir/src/include
src: root-dir/src/core

;-- Title string put into boot.h file checksum:
Title:
{REBOL
Copyright 2012 REBOL Technologies
Copyright 2012-2021 Rebol Open Source Contributors
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


;-- Fetch platform specifications:
;init-build-objects/platform platform
;platform-data: platforms/:platform
;build: platform-data/builds/:product

load-boot: func[file][load-file rejoin [root-dir %src/boot/ file]]

;-- Emit Function
out: make string! 100000
emit: func [data] [repend out data]

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
		if code   [rejoin [tab prefix str cmt]]
		if decl   [rejoin [prefix str cmt]]
		rejoin [tab prefix str ","]
	]
	if any [code decl] [cmt: none]
	if cmt [
		append/dup str #" " (31 - length? str)
		any [
			if define [repend str cmt]
			if cmt    [repend str ["// " cmt]]
		]
	]
	append str newline
	append out str
]

emit-head: func [title [string!] file [file!]] [
	clear out
	emit form-header/gen title file %make-boot.reb
]

emit-end: func [/easy] [
	if not easy [change find/last out #"," #" "]
	append out {^};^/}
]

remove-tests: func [d] [
	while [d: find d #test][remove/part d 2]
]

;----------------------------------------------------------------------------
;- Evaltypes.h - Evaluation Dispatch Maps                                    
;----------------------------------------------------------------------------

boot-types: load-boot %types.reb
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

write-generated inc/gen-evaltypes.h out


;----------------------------------------------------------------------------
;- Maketypes.h - Dispatchers for Make (used by construct)                    
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

write-generated inc/gen-maketypes.h out

;----------------------------------------------------------------------------
;- Comptypes.h - compare functions                                           
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

write-generated inc/gen-comptypes.h out


;----------------------------------------------------------------------------
;- Moldtypes.h - Dispatchers for Mold and Form                               
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
;write-generated inc/gen-moldtypes.h out

;----------------------------------------------------------------------------
;- Bootdefs.h - Boot include file                                            
;----------------------------------------------------------------------------

emit-head "Datatype Definitions" %reb-types.h

emit {
/***********************************************************************
**
*/	enum REBOL_Types
/*
**		Internal datatype numbers. These change. Do not export.
**
***********************************************************************/
^{
}

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

write-generated inc/reb-types.h out

;----------------------------------------------------------------------------
;- Extension Related Tables                                                  
;----------------------------------------------------------------------------

ext-types: load-boot %types-ext.reb
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

emit {
enum REBOL_Ext_Types
^{
}

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

write-generated inc/ext-types.h out ; part of Host-Kit distro

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

write-generated inc/gen-exttypes.h out


;----------------------------------------------------------------------------
;- Bootdefs.h - Boot include file                                            
;----------------------------------------------------------------------------

emit-head "Boot Definitions" %bootdefs.h

emit [{
#define REBOL_VER } any [version/1 0] {
#define REBOL_REV } any [version/2 0] {
#define REBOL_UPD } any [version/3 0] {
#define REBOL_SYS } any [version/4 0] {
#define REBOL_VAR } any [version/5 0] {
// Version visible when used -v argument on start-up (before initialization)
#define REBOL_VERSION \} string-to-c short-str-version
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

boot-strings: load-boot %strings.reb

append boot-strings compose [
	version:
		(any [str-version "Rebol Core 3.5.0"])
]

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

used-words: copy []

n: 1
foreach :type-record boot-types [
	word: join type "_type"
	append used-words to-c-name word
	emit-line "SYM_" word n
	n: n + 1
]

boot-words: load-boot %words.reb

foreach [group words] load-boot %modes.reb [
	replace boot-words group words
]

foreach word boot-words [
	append used-words to-c-name word
	emit-line "SYM_" word reform [n "-" word]
	n: n + 1
]

;- gen-symbols.reb is file generated by make-headers.reb script!
if exists? gen-dir/gen-symbols.reb [
	emit {^/    // follows symbols used in C sources, but not defined in %words.reb list...^/}

	foreach word load-file gen-dir/gen-symbols.reb [
		if not find used-words word [
			append boot-words to word! lowercase word
			emit-line "SYM_" word form n
			n: n + 1
		]
	]

	;delete temp-dir/tmp-symbols.reb
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

boot-actions: load-boot %actions.reb
act-count: 1
emit-line "A_" "type = 0" "Handled by interpreter"
foreach word boot-actions [
	if set-word? :word [
		emit-line "A_" to word! :word act-count ;R3
		++ act-count
	]
]
emit [tab "A_MAX_ACTION" lf "};"]
emit {

#define IS_BINARY_ACT(a) ((a) <= A_XOR)
}

write-generated inc/gen-bootdefs.h out

;----------------------------------------------------------------------------
;- Sysobj.h - System Object Selectors                                        
;----------------------------------------------------------------------------

emit-head "System Object" %sysobj.h
emit newline

at-value: func ['field] [next find boot-sysobj to-set-word field]

get-git: function[][
	git: ref: none
	if exists? git-dir: root-dir/.git [
		;?? git-dir
		try [
			;ls/r :git-dir
			;try [print read/string  git-dir/config ]

			git-head: read-file git-dir/HEAD
			;?? git-head
			parse git-head [thru "ref:" any #" " copy ref to lf]
			
			git: object [repository: branch: commit: message: none]
			try [
				git/repository: trim/tail read/string git-dir/description
				if find git/repository "Unnamed repository;" [
					git/repository: none
				]
			]
			try [git/branch: find/reverse/tail tail ref #"/"]
			try [git/commit: trim/tail either ref [read/string git-dir/(ref)][git-head]]
			try [git/message: trim/tail read/string git-dir/COMMIT_EDITMSG]
			;?? git
		]
	]
	git
]

plats: load-boot %platforms.reb
boot-sysobj: load-boot %sysobj.reb
change at-value version version
when: now
when: when - when/zone
when/zone: 0:00
;change at-value build object [
;	os: any [select third any [find/skip plats version/4 3 []] version/5 ""]
;	date: when
;	git: get-git
;]
;change at-value product to lit-word! product
change/only at-value platform to lit-word! any [select plats version/4 'Unknown]

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

write-generated inc/gen-sysobj.h out

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

write-generated inc/reb-dialect.h out


;----------------------------------------------------------------------------
;- Event Types                                                               
;----------------------------------------------------------------------------

emit-head "Event Types" %reb-evtypes.h
emit newline

emit ["enum event_types {" newline]
foreach field ob/catalog/event-types [
	emit-line "EVT_" field none
]
emit [tab "EVT_MAX^/"]
emit "};^/^/"

emit ["enum event_keys {" newline]
emit-line "EVK_" "NONE" none
foreach field ob/catalog/event-keys [
	emit-line "EVK_" field none
]
emit [tab "EVK_MAX^/"]
emit "};^/^/"

write-generated inc/reb-evtypes.h out


;----------------------------------------------------------------------------
;- Error Constants                                                           
;----------------------------------------------------------------------------

;-- Error Structure ----------------------------------------------------- ---

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

boot-errors: load-boot %errors.reb
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

write-generated inc/gen-errnums.h out

;-------------------------------------------------------------------------

emit-head "Port Modes" %port-modes.h

data: load-boot %modes.reb

emit {^/enum reb_port_modes ^{^/}
foreach word select data '*port-modes* [ emit-enum join "MODE_PORT_" up-word word ]
emit-end

emit {^/enum reb_console_modes ^{^/}
foreach word select data '*console-modes* [	emit-enum join "MODE_CONSOLE_" up-word word ]
emit-end

emit {^/enum reb_audio_modes ^{^/}
foreach word select data '*audio-modes* [	emit-enum join "MODE_AUDIO_" up-word word ]
emit-end

write-generated inc/gen-portmodes.h out

;----------------------------------------------------------------------------
;- Load Boot Mezzanine Functions - Base, Sys, Lib, and Prot                  
;----------------------------------------------------------------------------

print-info "Load Boot Mezzanine Functions - Base, Sys, Lib and Prot"

;make sure that mezz-tail.reb is the last file in boot-mezz
if all [
	pos: find mezz-files/3 %mezz-tail.reb
	1 <> length? pos
][
	append mezz-files/3 take pos
]

foreach section [boot-base boot-sys boot-mezz] [
	set section make block! 200

	foreach file first mezz-files [
		code: load-file/header file
		 hdr: take code
		append get section either 'module = select hdr 'type [
			either find hdr/options 'delay [
				compose [
					sys/load-module/delay (
						append
						mold/only compose/deep/only [
							Rebol [
								Version: (any [select hdr 'version 0.0.0])
								Title:   (select hdr 'title)
								Name:    (select hdr 'name)
								Date:    (select hdr 'date)
								Author:  (select hdr 'author)
								Exports: (select hdr 'exports)
								Needs:   (select hdr 'needs)
							]
						]
						mold/only code
					)
				]
			][
				reduce [
					'import to paren! compose/deep/only [
						module [
							Title:   (select hdr 'title)
							Name:    (select hdr 'name)
							Version: (select hdr 'version)
							Date:    (select hdr 'date)
							Author:  (select hdr 'author)
							Exports: (select hdr 'exports)
							Needs:   (select hdr 'needs)
						] ( code )
					]
				]
			]
		][	code ]
	]
	remove-tests get section
	mezz-files: next mezz-files
]

boot-protocols: make block! 20
foreach file first mezz-files [
	code: load-file/header file
	hdr: to block! take code
	either all [
		;- if protocol exports some function, import must be used so
		;- the functions are available in user's context
		;select hdr 'exports
		select hdr 'name
		'module  = select hdr 'type
	][
		;- using boot-mezz as this section is binded into lib context
		append boot-mezz 
		either find hdr/options 'delay [
			compose [
				sys/load-module/delay (
					append
					mold/only compose/deep/only [
						Rebol [
							Version: (any [select hdr 'version 0.0.0])
							Title:   (select hdr 'title)
							Name:    (select hdr 'name)
							Date:    (select hdr 'date)
							Author:  (select hdr 'author)
							Exports: (select hdr 'exports)
							Needs:   (select hdr 'needs)
						]
					]
					mold/only code
				)
			]
		][ 
			reduce [
				'import to paren! compose/deep/only [
					module [
						Title:   (select hdr 'title)
						Name:    (select hdr 'name)
						Version: (select hdr 'version)
						Date:    (select hdr 'date)
						Author:  (select hdr 'author)
						Exports: (select hdr 'exports)
						Needs:   (select hdr 'needs)
					] ( code )
				]
			]
		]
	][
		;- else hidden module is used by default (see sys-start.reb)
		append/only append/only boot-protocols hdr code
	]
]

emit-head "Sys Context" %sysctx.h
sctx: construct boot-sys
make-obj-defs sctx "SYS_CTX_" 1
write-generated inc/gen-sysctx.h out


;----------------------------------------------------------------------------
;- b-boot.c - Boot data file                                                 
;----------------------------------------------------------------------------

;-- Build b-boot.c output file ----------------------------------------------


emit-head "Natives and Bootstrap" %b-boot.c
emit {
#include "sys-core.h"

}

externs: make string! 2000
boot-booters: load-boot %booters.reb
boot-natives: append load-boot %natives.reb load-file gen-dir/gen-natives.reb


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

emit [newline {const REBFUN Native_Funcs[} nat-count {] = ^{
}]
foreach val nats [
	if set-word? val [
		emit-line/code "N_" to word! val "," ;R3
	]
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
specs: load-boot %typespec.reb
foreach type datatypes [
	append/only boot-typespecs select specs type
]

;-- Create main code section (compressed):
boot-types: new-types
boot-root: load-boot %root.reb
boot-task: load-boot %task.reb
boot-ops:  load-boot %ops.reb
;boot-script: load-boot %script.reb

write-generated gen-dir/gen-boot-code.reb mold reduce sections

data: mold reduce sections
insert data reduce ["; Copyright (C) REBOL Technologies " now newline]
insert tail data make char! 0 ; scanner requires zero termination

data: to binary! data
comp-data: compress/level data 'zlib 9

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

write-generated src/b-boot.c out

;-- Create platform string:
;platform: to-string platform
;lowercase platform
;if platform-data/type = 'windows [ ; Why?? Not sure.
;	product: to-string product
;	lowercase product
;	replace/all product "-" ""
;]
;;dir: to-file rejoin [%../to- platform "/" product "/temp/"]

;-- Output stats:
print-info ["Natives:^[[33m" nat-count]
print-info ["Actions:^[[33m" act-count]
print-info ajoin [
	"^[[32mBoot code compressed from ^[[33m" length? data
	"^[[32m to ^[[33m" length? comp-data
	"^[[32m bytes (^[[33m" to-integer ((length? comp-data) / (length? data) * 100)
	"^[[32m percent)"
]
;print-info [
;	"Compressed" length? data "to" length? comp-data-zlib "bytes:"
;	to-integer ((length? comp-data-zlib) / (length? data) * 100)
;	"percent of original (ZLIB)"
;]

;----------------------------------------------------------------------------
;- Boot.h - Boot header file                                                 
;----------------------------------------------------------------------------

emit-head "Bootstrap Structure and Root Module" %boot.h

emit [
{
#define MAX_NATS      } nat-count {
#define NAT_SPEC_SIZE } length? comp-data {
#define CHECK_TITLE   } crc24 to binary! title {

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

write-generated inc/gen-boot.h out


;----------------------------------------------------------------------------
;- gen-config.h - system configuration header file                           
;----------------------------------------------------------------------------

emit-head "Build configuration" %config.h

emit {^/#ifndef REBOL_OPTIONS_H^/}

if stack-size [
	emit ajoin ["#define STACK_SIZE " stack-size lf lf]
]
foreach def configs [
	emit ajoin ["#define " def lf]
]
emit {
//**************************************************************//
#include "opt-dependencies.h" // checks for above options

#endif //REBOL_OPTIONS_H
}

write-generated inc/gen-config.h out

;----------------------------------------------------------------------------
;print ask "-DONE-"
;wait .3

] ; end of context
;print "[DONE boot]^/"
