REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Generate headers and boot code"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2021 Rebol Open Source Contributors
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Author: "Oldes"
	Needs: 3.6.0
	Note: {
		This script is comilation of multiple original Carl's make-* scripts.
	}
]

secure [ file allow ]

do %tools/form-header.reb
do %tools/utils.reb

args: system/script/args

spec-file: try/except [
	to-rebol-file either block? args [first args][args]
][ clean-path %spec-core.reb ]

unless 'file = exists? spec-file [
	do error ["Spec file not found:" mold spec-file]
]

print-info ["Using spec file:" as-yellow to-local-file spec-file ]
spec: load spec-file

;-- Options:
verbose: off

root-dir:        spec/root
c-core-files:    spec/core-files
c-host-files:    spec/host-files
mezz-base-files: spec/mezz-base-files
mezz-sys-files:  spec/mezz-sys-files
mezz-lib-files:  spec/mezz-lib-files
mezz-prot-files: spec/mezz-prot-files
boot-host-files: spec/boot-host-files

;either file? boot-script [
;	if #"/" <> first boot-script [boot-script: join root-dir boot-script]
;	unless exists? boot-script [
;		print-error ["Boot script not found:" as-red boot-script]
;		quit/return 3
;	]
;][
;	if boot-script [
;		print-error ["Boot script must be a file, but is:" as-red boot-script]
;		quit/return 3
;	]
;]


switch system/platform [
	Windows [ if block? spec/host-files-win32 [append c-host-files spec/host-files-win32] ]
]


assert [
	block? c-core-files
	block? c-host-files
	not empty? c-core-files
	not empty? c-host-files
	block? mezz-base-files
	block? mezz-sys-files
	block? mezz-lib-files
	block? mezz-prot-files
]

absolutize-path: func[file][
	if #"/" <> first file/1 [insert file src-dir]
	file
]
src-dir: dirize spec/source
if #"/" <> first src-dir [src-dir: join spec/root src-dir]

if file? src-dir [
	forall c-core-files    [absolutize-path c-core-files/1]
	forall c-host-files    [absolutize-path c-host-files/1]
	forall mezz-base-files [absolutize-path mezz-base-files/1]
	forall mezz-sys-files  [absolutize-path mezz-sys-files/1]
	forall mezz-lib-files  [absolutize-path mezz-lib-files/1]
	forall mezz-prot-files [absolutize-path mezz-prot-files/1]
	forall boot-host-files [absolutize-path boot-host-files/1]
]

c-core-files: unique c-core-files
c-host-files: unique c-host-files

mezz-files: reduce [
	;Boot Mezzanine Functions
	unique mezz-base-files
	unique mezz-sys-files
	unique mezz-lib-files
	unique mezz-prot-files
]


version:  any [spec/version 3.5.0]
platform: any [spec/platform system/platform]
target:   any [spec/target spec/os-target spec/configuration]
vendor:   any [
	;pc, apple, nvidia, ibm, etc.
	all [word? spec/vendor        spec/vendor       ]
	all [word? spec/manufacturer  spec/manufacturer ]
]
arch: any [
	; list of LLVM (6) targets/archs:
	;    aarch64    - AArch64 (little endian)
	;    aarch64_be - AArch64 (big endian)
	;    amdgcn     - AMD GCN GPUs
	;    arm        - ARM
	;    arm64      - ARM64 (little endian)
	;    armeb      - ARM (big endian)
	;    bpf        - BPF (host endian)
	;    bpfeb      - BPF (big endian)
	;    bpfel      - BPF (little endian)
	;    hexagon    - Hexagon
	;    lanai      - Lanai
	;    mips       - Mips
	;    mips64     - Mips64 [experimental]
	;    mips64el   - Mips64el [experimental]
	;    mipsel     - Mipsel
	;    msp430     - MSP430 [experimental]
	;    nvptx      - NVIDIA PTX 32-bit
	;    nvptx64    - NVIDIA PTX 64-bit
	;    ppc32      - PowerPC 32
	;    ppc64      - PowerPC 64
	;    ppc64le    - PowerPC 64 LE
	;    r600       - AMD GPUs HD2XXX-HD6XXX
	;    sparc      - Sparc
	;    sparcel    - Sparc LE
	;    sparcv9    - Sparc V9
	;    systemz    - SystemZ
	;    thumb      - Thumb
	;    thumbeb    - Thumb (big endian)
	;    x86        - 32-bit X86: Pentium-Pro and above
	;    x86_64     - 64-bit X86: EM64T and AMD64
	;    xcore      - XCore
	all [word? spec/arch          spec/arch       ]
	all [word? spec/target-arch   spec/target-arch]
]
os-info: get-os-info
os:  any [
	select os-info 'ID
	all [word? spec/os spec/os]
	select #(
		Macintosh: macos
		Windows:   windows
		Linux:     linux
		OpenBSD:   openbsd
	) platform	
]
syst: any [
	; none, linux, win32, darwin, cuda, etc.
	spec/sys
	spec/kernel
	select #(
		macos:   darwin
		ios:     darwin
		windows: win32
		linux:   linux
		openbsd: openbsd
	) os	
]
abi: any [
	; eabi, gnu, android, macho, elf, musl etc.
	spec/target-abi
	spec/abi
	select #(
		macos:   macho
		ios:     macho
		windows: pe
		reactos: pe
		beos:    pe
		linux:   elf
		;alpine:  musl
	) spec/os
]

product:  any [spec/product 'Core]
configs:  unique any [spec/config copy []]
stack-size: any [spec/stack-size 1048576] ;default 1MiB

unless target [
	; if configuration is not fully provided, try to compose it
	; EXAMPLES:
	;	x86_64-linux-musl
	;	ppc-apple-darwin
	;
	target: clear ""
	if arch   [append append target arch   #"-"]
	if vendor [append append target vendor #"-"]
	if syst   [append append target syst   #"-"]
	if abi    [append append target abi    #"-"]
	take/last target
]

;"Rebol Core 4.0.0 Windows x86_64 msvc-19 20-Feb-2021/17:31"
;"Rebol Core 4.0.0 Linux x86_64-linux-gnu gcc 20-Feb-2021/17:31"
;"Rebol Core 4.0.0 Linux x86_64-linux-musl gcc 20-Feb-2021/17:31"
;"Rebol Core 4.0.0 Linux arm-linux-gnueabihf gcc 20-Feb-2021/17:31
build-date: now/date
build-time: now/time build-time/second: 0
build-date/time: build-time


;- resolving current git commit
try/except [
	parse read/string %../.git/HEAD [thru "ref: " copy git-header to lf]
	git-commit: mold debase read/string join %../.git/ git-header 16
][	git-commit: none]

if string? select os-info 'VERSION_ID [
	;; make sure that it will be loadable
	os-info/VERSION_ID: mold os-info/VERSION_ID
]

str-version: reform [
	"Rebol"  ; probably always same
	product  ; like Core, View, etc...
	version  ; triple value product version
	platform ; Linux, Windows, macOS, Android...
	os
	arch
	vendor
	syst
	abi
	any [all [word? spec/compiler spec/compiler]] ; gcc, clang, msvc...
	target
	build-date
	git-commit
	get-libc-version ;; musl, glibc_2.28,... or just none
	select os-info 'VERSION_ID
]

;;format-date-time may not be available in older Builder tools!
;;format-date-time now/utc "(yyyy-MM-dd hh:mm:ss UTC)"

short-str-version: next ajoin [{
Rebol/} product SP version " (" build-date { UTC)
Copyright (c) 2012 REBOL Technologies
Copyright (c) 2012-} now/year { Rebol Open Source Contributors
Source:       https://github.com/Oldes/Rebol3}]

ver3: version ver3/4: none ; trimmed version to just 3 parts
lib-version: version/1

protect [spec version c-core-files c-host-files mezz-files configs]

;change-dir root-dir

; Output directory for generated files:
make-dir gen-dir: root-dir/src/generated

;------------------------------------------------------------------------------
print-title ["[1/6]" as-red "Building Rebol headers"]
do %tools/make-headers.reb

;------------------------------------------------------------------------------
print-title ["[2/6]" as-red "Make primary boot files" ]
do %tools/make-boot.reb

;------------------------------------------------------------------------------
print-title ["[3/6]" as-red "Make Host Init Code"]
; Options:
include-vid: off
; Files to include in the host program:
files: [
;	%mezz/prot-http.reb ;- included in core! (boot-files.reb)
;	%mezz/view-colors.reb
]
vid-files: [
	%mezz/dial-draw.reb
	%mezz/dial-text.reb
	%mezz/dial-effect.reb
	%mezz/view-funcs.reb
	%mezz/vid-face.reb
	%mezz/vid-events.reb
	%mezz/vid-styles.reb
	%mezz/mezz-splash.reb
]
if include-vid [append files vid-files]
do %tools/make-host-init.reb

;------------------------------------------------------------------------------
print-title ["[4/6]" as-red "Make Host Boot Extension"]
do %tools/make-host-ext.reb

;------------------------------------------------------------------------------
print-title ["[5/6]" as-red "Make OS Library Extension"]
do %tools/make-os-ext.reb

;------------------------------------------------------------------------------
print-title ["[6/6]" as-red "Make Reb-Lib Headers" ]
do %tools/make-reb-lib.reb

print-title "DONE"
