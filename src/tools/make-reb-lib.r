REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Make Reb-Lib related files"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Author: "Carl Sassenrath"
	Needs: 2.100.100
]

print "--- Make Reb-Lib Headers ---"

verbose: true

lib-ver: 2

preface: "RL_"

src-dir: %../core/
reb-lib: src-dir/a-lib.c
ext-lib: src-dir/f-extension.c

out-dir: %../include/
reb-ext-lib:  out-dir/reb-lib.h   ; for Host usage
reb-ext-defs: out-dir/reb-lib-lib.h  ; for REBOL usage

ver: load %../boot/version.r

do %form-header.r

;-----------------------------------------------------------------------------
;-----------------------------------------------------------------------------

cnt: 0

xlib: make string! 20000
rlib: make string! 1000
mlib: make string! 1000
dlib: make string! 1000
cmts: make string! 1000
xsum: make string! 1000

emit:  func [d] [append repend xlib d newline]
remit: func [d] [append repend rlib d newline]
demit: func [d] [append repend dlib d newline]
cemit: func [d] [append repend cmts d newline]
memit: func [d /nol] [
	repend mlib d
	if not nol [append mlib newline]
]

count: func [s c /local n] [
	if find ["()" "(void)"] s [return "()"]
	out: copy "(a"
	n: 1
	while [s: find/tail s c][
		repend out [#"," #"a" + n]
		n: n + 1
	]
	append out ")"
]

in-sub: func [str pat /local f] [
	all [
		f: find cmt pat ":"
		insert f "^/:"
		f: find next f newline
		remove f
		insert f " - "
	]
]

gen-doc: func [fspec spec cmt] [
	replace/all cmt "**" "  "
	replace/all cmt "/*" "  "
	replace/all cmt "*/" "  "
	trim cmt
	append cmt newline

	insert find cmt "Arguments:" "^/:"
	bb: beg: find/tail cmt "Arguments:"
	insert any [find bb "notes:" tail bb] newline
	while [
		all [
			beg: find beg " - "
			positive? offset? beg any [find beg "notes:" tail beg]
		]
	][
		insert beg </tt>
		insert find/tail/reverse beg newline {<br><tt class=word>}
		beg: find/tail beg " - "
	]

	beg: insert bb { - } ;<div style="white-space: pre;">}
	remove find beg newline
	remove/part find beg "<br>" 4 ; extra <br>

	remove find cmt "^/Returns:"
	in-sub cmt "Returns:"
	in-sub cmt "Notes:"

	insert cmt reduce [
		":Function: - " <tt class=word> spec </tt>
		"^/^/:Summary: - "
	]
	cemit ["===" fspec newline newline cmt]
]

pads: func [start col] [
	col: col - offset? start tail start
	head insert/dup clear "" #" " col
]

func-header: [
	[
		thru "RL_API "
		copy spec to newline skip
		["/*" copy cmt thru "*/" | none]
		(
			if all [
				spec
				trim spec
				fn: find spec preface
				find spec #"("
			][
				emit ["RL_API " spec ";"] ;    // " the-file]
				append xsum spec
				p1: copy/part spec fn
				p3: find fn #"("
				p2: copy/part fn p3
				p2u: uppercase copy p2
				p2l: lowercase copy find/tail p2 preface
				demit [tab p2 ","]
				remit [tab p1 "(*" p2l ")" p3 ";"]
				args: count p3 #","
				m: tail mlib
				memit/nol ["#define " p2u args]
				memit [pads m 35 " RL->" p2l args]
				if w: find cmt "****" [append clear w "*/"]
				memit ["/*^/**^-" spec "^/**" cmt newline]

				gen-doc p2 spec cmt
				cnt: cnt + 1
			]
		)
		newline
		[
			"/*" ; must be in func header section, not file banner
			any [
				thru "**"
				[#" " | #"^-"]
				copy line thru newline
			]
			thru "*/"
			| 
			none
		]
	]
]

write-if: func [file data] [
	if data <> attempt [to string! read file][ ;R3
		print ["UPDATE:" file]
		write file data
	]
]

process: func [file] [
	if verbose [?? file]
	data: to string! read the-file: file ;R3
	parse/all data [
		any func-header
	]
]

;-----------------------------------------------------------------------------

remit {
typedef struct rebol_ext_api ^{}

cemit [{Host/Extension API

=r3

=*Updated for A} ver/3 { on } now/date {

=*Describes the functions of reb-lib, the REBOL API (both the DLL and extension library.)

=!This document is auto-generated and changes should not be made within this wiki.

=note WARNING: PRELIMINARY Documentation

=*This API is under development and subject to change. Various functions may be moved, removed, renamed, enhanced, etc.

Also note: the formatting of this document will be enhanced in future revisions.

=/note

==Concept

The REBOL API provides common API functions needed by the Host-Kit and also by
REBOL extension modules. This interface is commonly referred to as "reb-lib".

There are two methods of linking to this code:

*Direct calls as you would use functions within any DLL.

*Indirect calls through a set of macros (that use a structure pointer to the library.)

==Functions
}]

;-----------------------------------------------------------------------------

process reb-lib
process ext-lib

;-----------------------------------------------------------------------------

remit "} RL_LIB;"

out: to-string reduce [
form-header/gen "REBOL Host and Extension API" %reb-lib.r %make-reb-lib.r
{
// These constants are created by the release system and can be used to check
// for compatiblity with the reb-lib DLL (using RL_Version.)
#define RL_VER } ver/1 {
#define RL_REV } ver/2 {
#define RL_UPD } ver/3 {

// Compatiblity with the lib requires that structs are aligned using the same
// method. This is concrete, not abstract. The macro below uses struct
// sizes to inform the developer that something is wrong.
#define CHECK_STRUCT_ALIGN (sizeof(REBREQ) == 80 && sizeof(REBEVT) == 12)

// Function entry points for reb-lib (used for MACROS below):}
rlib
{
// Extension entry point functions:
#ifdef TO_WIN32
#ifdef __cplusplus
#define RXIEXT extern "C" __declspec(dllexport)
#else
#define RXIEXT __declspec(dllexport)
#endif
#else
#define RXIEXT extern
#endif

RXIEXT const char *RX_Init(int opts, RL_LIB *lib);
RXIEXT int RX_Quit(int opts);
RXIEXT int RX_Call(int cmd, RXIFRM *frm, void *data);

// The macros below will require this base pointer:
extern RL_LIB *RL;  // is passed to the RX_Init() function

// Macros to access reb-lib functions (from non-linked extensions):

}
mlib
{

#define RL_MAKE_BINARY(s) RL_MAKE_STRING(s, FALSE)

#ifndef REB_EXT // not extension lib, use direct calls to r3lib

}
xlib
{
#endif
}
]

write-if reb-ext-lib out

;-----------------------------------------------------------------------------

out: to-string reduce [
form-header/gen "REBOL Host/Extension API" %reb-lib-lib.r %make-reb-lib.r
{RL_LIB Ext_Lib = ^{
}
dlib
{^};
}
]

write-if reb-ext-defs out

write-if %../reb-lib-doc.txt cmts

;ask "Done"
print "   "
