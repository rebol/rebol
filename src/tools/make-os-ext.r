REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Generate OS host API headers"
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

verbose: false

version: load %../boot/version.r

lib-version: version/3
print ["--- Make OS Ext Lib --- Version:" lib-version]

; Set platform TARGET
do %systems.r
target: config-system/os-dir

do %form-header.r

change-dir append %../os/ target

files: [
	%host-lib.c
	%../host-device.c
]

; If it is graphics enabled:
if all [
	not find any [system/options/args []] "no-gfx"
	find [3] system/version/4
][
	append files [%host-window.c]
]

cnt: 0

xlib: make string! 20000
rlib: make string! 1000
mlib: make string! 1000
dlib: make string! 1000
xsum: make string! 1000

emit:  func [d] [append repend xlib d newline]
remit: func [d] [append repend rlib d newline]
demit: func [d] [append repend dlib d newline]
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

pads: func [start col] [
	col: col - offset? start tail start
	head insert/dup clear "" #" " col
]

func-header: [
	[
		thru "/***" 10 100 "*" newline
		thru "*/"
		copy spec to newline
		(if all [
			spec
			trim spec
			not find spec "static"
			fn: find spec "OS_"
			find spec #"("
		][
			emit ["extern " spec ";    // " the-file]
			append xsum spec
			p1: copy/part spec fn
			p3: find fn #"("
			p2: copy/part fn p3
			p2u: uppercase copy p2
			p2l: lowercase copy p2
			demit [tab p2 ","]
			remit [tab p1 "(*" p2l ")" p3 ";"]
			args: count p3 #","
			m: tail mlib
			memit/nol ["#define " p2u args]
			memit [pads m 35 " Host_Lib->" p2l args]
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

process: func [file] [
	if verbose [?? file]
	data: read the-file: file
	data: to-string data ; R3
	parse/all data [
		any func-header
	]
]

;process %mem_string.c halt

remit {
typedef struct REBOL_Host_Lib ^{
	int size;
	unsigned int ver_sum;
	REBDEV **devices;}

memit {
extern	REBOL_HOST_LIB *Host_Lib;
}

foreach file files [
	print ["scanning" file]
	if all [
		%.c = suffix? file
	][process file]
]

remit "} REBOL_HOST_LIB;"

out: reduce [
	form-header/gen "Host Access Library" %host-lib.h %make-os-ext.r
{
#define HOST_LIB_VER } lib-version {
#define HOST_LIB_SUM } checksum/tcp to-binary xsum {
#define HOST_LIB_SIZE } cnt {

extern REBDEV *Devices[];
}
rlib
{
//** Included by HOST *********************************************

#ifndef REB_DEF

}
xlib
{
#ifdef OS_LIB_TABLE

REBOL_HOST_LIB *Host_Lib;

REBOL_HOST_LIB Host_Lib_Init = ^{  // Host library function vector table.
	HOST_LIB_SIZE,
	(HOST_LIB_VER << 16) + HOST_LIB_SUM,
	(REBDEV**)&Devices,
}
dlib
{^};

#endif //OS_LIB_TABLE 

#else //REB_DEF

//** Included by REBOL ********************************************
}
mlib
{
#endif //REB_DEF
}
]

;print out ;halt
;print ['checksum checksum/tcp xsum]
write %../../include/host-lib.h out
;ask "Done"
print "   "
