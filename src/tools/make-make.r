REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Make the R3 Core Makefile"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Author: "Carl Sassenrath"
	Purpose: {
		Build a new makefile for a given platform.
	}
	Note: [
		"This runs relative to ../tools directory."
		"Make OS-specific changes to the systems.r file."
	]
]

file-files:  %file-base.r
file-system: %systems.r

path-host:   %../os/
path-make:   %../../make/
path-incl:   %../../src/include/

;******************************************************************************

makefile-head:

{# REBOL Makefile -- Generated by make-make.r (do not edit) on !date
# This makefile is intentional kept simple to make builds possible on
# a wider range of target platforms.

# To cross compile or build for other target:
#    $TOOLS - should point to bin where gcc is found
#    $INCL  - should point to the dir for includes
# Example make:
#    make TOOLS=~/amiga/amiga/bin/ppc-amigaos- INCL=/SDK/newlib/include

# For the build toolchain:
CC=	$(TOOLS)gcc
NM=	$(TOOLS)nm
STRIP=	$(TOOLS)strip

# CP allows different copy progs:
CP=	
# UP - some systems do not use ../
UP=	
# CD - some systems do not use ./
CD=	
# Special tools:
T=	$(UP)/src/tools
# Paths used by make:
S=	../src
R=	$S/core

INCL ?= .
I= -I$(INCL) -I$S/include/

TO_OS=	
OS_ID=	
RAPI_FLAGS=	
HOST_FLAGS=	-DREB_EXE
RLIB_FLAGS=	

# Flags for core and for host:
RFLAGS= -c -D$(TO_OS) -DREB_API  $(RAPI_FLAGS) $I
HFLAGS= -c -D$(TO_OS) -DREB_CORE $(HOST_FLAGS) $I
CLIB=   -lc -lm

# REBOL builds various include files:
REBOL=	$(CD)r3-make -qs

# For running tests, ship, build, etc.
R3=	$(CD)r3 -qs

### Build targets:
top:
	make r3

update:
	-cd $(UP)/; cvs -q update src

make:
	$(REBOL) $T/make-make.r $(OS_ID)

clean:
	@-rm -rf libr3.so objs/

all:
	make clean
	make prep
	make r3
	make lib
	make host

prep:
	$(REBOL) $T/make-headers.r
	$(REBOL) $T/make-boot.r $(OS_ID)
	$(REBOL) $T/make-host-init.r
	$(REBOL) $T/make-os-ext.r # ok, but not always
	$(REBOL) $T/make-host-ext.r
	$(REBOL) $T/make-reb-lib.r

### Post build actions
purge:
	-rm libr3.*
	-rm host
	make lib
	make host

test:
	$(CP) r3 $(UP)/src/tests/
	$(R3) $S/tests/test.r

install:
	sudo cp r3 /usr/local/bin

ship:
	$(R3) $S/tools/upload.r

build:	libr3.so
	$(R3) $S/tools/make-build.r

cln:
	rm libr3.* r3.o

check:
	$(STRIP) -s -o r3.s r3
	$(STRIP) -x -o r3.x r3
	$(STRIP) -X -o r3.X r3
	ls -l r3*

}

;******************************************************************************

makefile-link: {
# Directly linked r3 executable:
r3:	objs $(OBJS) $(HOST)
	$(CC) -o r3 $(OBJS) $(HOST) $(CLIB)
	$(STRIP) r3
	-$(NM) -a r3
	ls -l r3

objs:
	mkdir -p objs
}

makefile-so: {
lib:	libr3.so

# PUBLIC: Shared library:
# NOTE: Did not use "-Wl,-soname,libr3.so" because won't find .so in local dir.
libr3.so:	$(OBJS)
	$(CC) -o libr3.so -shared $(OBJS) $(CLIB)
	$(STRIP) libr3.so
	-$(NM) -D libr3.so
	-$(NM) -a libr3.so | grep "Do_"
	ls -l libr3.so

# PUBLIC: Host using the shared lib:
host:	$(HOST)
	$(CC) -o host $(HOST) libr3.so $(CLIB)
	$(STRIP) libr3.lib
	$(STRIP) host
	ls -l host
	echo "export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH"
}

makefile-dyn: {
lib:	libr3.dylib

# Private static library (to be used below for OSX):
libr3.dylib:	$(OBJS)
	ld -r -o r3.o $(OBJS)
	$(CC) -dynamiclib -o libr3.dylib r3.o $(CLIB)
	$(STRIP) -x libr3.dylib
	-$(NM) -D libr3.dylib
	-$(NM) -a libr3.dylib | grep "Do_"
	ls -l libr3.dylib

# PUBLIC: Host using the shared lib:
host:	$(HOST)
	$(CC) -o host $(HOST) libr3.dylib $(CLIB)
	$(STRIP) host
	ls -l host
	echo "export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH"
}

not-used: {
# PUBLIC: Static library (to distrirbute) -- does not work!
libr3.lib:	r3.o
	ld -static -r -o libr3.lib r3.o
	$(STRIP) libr3.lib
	-$(NM) -a libr3.lib | grep "Do_"
	ls -l libr3.lib
}

;******************************************************************************
;** Options and Config
;******************************************************************************

opts: system/options/args
if block? opts [opts: first opts]
if opts = ">" [opts: "0.3.1"] ; bogus editor

do %systems.r

fb: make object! load %file-base.r

either opts [
	opts: load opts
	unless all [
		tuple? opts
		os: config-system/platform opts
	][
		print ["*** Expected platform id (tuple like 0.3.1), not:" opts]
		wait 2
		quit
	]
	print ["Option set for building:" os/1 os/2]
][
	unless os: config-system [
		print ["*** Platform for" system/version "not supported"]
		wait 2
		quit
	]
]

set [os-plat os-name os-base build-flags] os
compile-flags: context compile-flags
linker-flags:  context linker-flags
other-flags:   context other-flags

; Make plat id string:
plat-id: form os-plat/2
if tail? next plat-id [insert plat-id #"0"]
append plat-id os-plat/3

; Create TO-OSNAME to indicate target OS:
to-def: join "TO_" uppercase copy os-name

outdir: path-make
make-dir outdir
make-dir outdir/objs

nl2: "^/^/"
output: make string! 10000

;******************************************************************************
;** Functions
;******************************************************************************

flag?: func ['word] [found? find build-flags word]

macro+: func [
	"Appends value to end of macro= line"
	'name
	value
][
	name: find find makefile-head join name "=" newline
	if name/-1 != space [name: insert name space]
	insert name value
]

macro++: funct ['name obj [object!]] [
	out: make string! 10
	foreach n words-of obj [
		all [
			obj/:n
			flag? (n)
			repend out [space obj/:n]
		]
	]
	macro+ (name) out
]

emit: func [d] [repend output d]

pad: func [str] [head insert/dup copy "" " " 16 - length? str]

to-obj: func [
	"Create .o object filename (with no dir path)."
	file
][
	;?? file
	file: to-file file ;second split-path file
	head change back tail file "o"
]

emit-obj-files: func [
	"Output a line-wrapped list of object files."
	files [block!]
	/local cnt
][
	cnt: 1
	foreach file files [
		file: to-obj file
		emit [%objs/ file " "]
		if cnt // 4 = 0 [emit "\^/^-"]
		cnt: cnt + 1
	]
	if tab = last output [clear skip tail output -3]
	emit nl2
]

emit-file-deps: func [
	"Emit compiler and file dependency lines."
	files
	;flags
	/dir path  ; from path
	/local obj
][
	foreach src files [
		obj: to-obj src
		src: rejoin pick [["$R/" src]["$S/" path src]] not dir
		emit [
			%objs/ obj ":" pad obj src
			newline tab
			"$(CC) "
			src " "
			;flags " "
			pick ["$(RFLAGS)" "$(HFLAGS)"] not dir
			" -o " %objs/ obj ; " " src
			nl2
		]
	]
]

;******************************************************************************
;** Build
;******************************************************************************

replace makefile-head "!date" now

macro+ TO_OS to-def
macro+ OS_ID os-plat
macro+ CP pick [copy cp] flag? COP
unless flag? -SP [ ; Use standard paths:
	macro+ UP ".."
	macro+ CD "./"
]
macro++ CLIB linker-flags
macro++ RAPI_FLAGS compile-flags
macro++ HOST_FLAGS make compile-flags [PIC: NCM: none]
macro+  HOST_FLAGS compile-flags/f64 ; default for all

if flag? +SC [remove find fb/os-posix 'host-readline.c]

emit makefile-head
emit ["OBJS =" tab]
emit-obj-files fb/core
emit ["HOST =" tab]
emit-obj-files append copy fb/os fb/os-posix
emit makefile-link
emit get pick [makefile-dyn makefile-so] os-plat/2 = 2
emit {
### File build targets:
b-boot.c: $(SRC)/boot/boot.r
	$(REBOL) -sqw $(SRC)/tools/make-boot.r
}
emit newline
emit-file-deps fb/core
emit-file-deps/dir fb/os %os/
emit-file-deps/dir fb/os-posix %os/posix/

;print copy/part output 300 halt
print ["Created:" outdir/makefile]
write outdir/makefile output

