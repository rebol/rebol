REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "System build targets"
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
		These are the target system definitions used to build REBOL
		with a variety of compilers and libraries. I prefer to keep it
		simple like this rather than using a complex configuration tool
		that could make it difficult to support REBOL on older platforms.
	}
]

systems: [
	[plat		os-name			os-base		build-flags]
	[0.1.03		"amiga"			posix		[BEN HID NPS +SC CMT COP -SP -LM]]
	[0.2.04		"osx"			posix		[BEN +OS NCM -LM UOP LIC]]				; OSX/PPC; no shared lib possible
	[0.2.05		"osxi"			posix		[LEN +O2 PIC NPS NCM HID STX -LM UOP ARC FCS FCM LIC]]
	[0.2.40		"osx_x64"		posix		[LEN +O2 PIC NPS NCM HID STX -LM UOP L64 FCS FCM LIC]]
	[0.3.01		"win32"			win32		[LEN +O2 UNI M32 W32 CON S4M EXE DIR -LM]]
	[0.3.40		"win32_x64"		win32		[LEN +O2 UNI M64 W32 CON S4M EXE DIR -LM P64]]
	[0.4.02		"linux"			posix		[LEN +O2 PIC LDL ST1 -LM]]			; libc 2.3
	[0.4.03		"linux"			posix		[LEN +O2 PIC LDL ST1 -LM HID]]		; libc 2.5
	[0.4.04		"linux"			posix		[LEN +O2 PIC LDL ST1 -LM HID M32]]	; libc 2.11
	[0.4.10		"linux_ppc"		posix		[BEN +O1 PIC LDL ST1 -LM HID]]
	[0.4.20		"linux_arm"		posix		[LEN +O2 PIC LDL ST1 -LM HID]]
	[0.4.21		"linux_arm"		posix		[LEN +O2 PIE LDL ST1 -LM HID]]	; bionic (Android)
	[0.4.30		"linux_mips"	posix		[LEN +O2 PIC LDL ST1 -LM HID]]
	[0.4.40		"linux_x64"		posix		[LEN +O2 PIC LDL ST1 -LM HID L64]] ; core version
	[0.4.46		"linux_x64_musl"posix		[LEN +O2 PIC LDL ST1 -LM HID L64]]
	[0.5.75		"haiku"			posix		[LEN +O2 ST1 NWK]]
	[0.7.02		"freebsd"		posix		[LEN +O1 ST1 -LM]]
	[0.7.40		"freebsd_x64"	posix		[LEN +O1 ST1 -LM L64]]
	[0.9.04		"openbsd"		posix		[LEN +O1 ST1 -LM]]
	[0.13.01	"android_arm"	android		[LEN HID F64 LDL LLOG -LM CST]]
]

compile-flags: [
	+OS:	"-Os"						; size optimize
	+O1:	"-O1"						; full optimize
	+O2:	"-O2"						; full optimize
	UNI:	"-DUNICODE -D_UNICODE"		; win32 wants it
	CST:	"-DCUSTOM_STARTUP"			; include custom startup script at host boot
	HID:	"-fvisibility=hidden"		; all syms are hidden
	F64:	"-D_FILE_OFFSET_BITS=64"	; allow larger files
	P64:    "-D__LLP64__"				; 64-bit, long (integer) has 32 bits and 'void *' is sizeof(long long)
	L64:	"-D__LP64__"				; same like LLP64, but long (integer) has 64 bits instead of 32
	NPS:	"-Wno-pointer-sign"			; OSX fix
	NSP:	"-fno-stack-protector"		; avoid insert of functions names
	PIC:	"-fPIC"						; position independent (used for libs)
	PIE:	"-fPIE"						; position independent (executables)
	DYN:	"-dynamic"					; optimize for dll??
	NCM:	"-fno-common"				; lib cannot have common vars
	PAK:	"-fpack-struct"				; pack structures
	ARC:	"-arch i386"				; x86 32 bit architecture (OSX)
	M32:	"-m32"						; use 32-bit memory model
	M64:	"-m64"						; use 64-bit memory model
	LEN:	"-DENDIAN_LITTLE"			; uses little endian byte order
	BEN:	"-DENDIAN_BIG"				; uses big endian byte order
	UOP:	"-DUSE_OLD_PIPE"			; use pipe() instead of pipe2(), which may not be supported
	GUI:	"-DREB_VIEW"				; view
]

linker-flags: [
	MAP:	"-Wl,-M"					; output a map
	STA:	"--strip-all"
	LDL:	"-ldl"						; link with dynamic lib lib
	LIC:	"-liconv"                   ; macOS (clang?) needs iconv to be explicitly linked (for iconv native function)
	LLOG:	"-llog"						; on Android, link with liblog.so
	ARC:	"-arch i386"				; x86 32 bit architecture (OSX)
	M32:	"-m32"						; use 32-bit memory model (Linux x64)
	W32:	"-lwsock32 -lcomdlg32 -lgdi32 -lwinmm -lComctl32 -lUxTheme -lopengl32 -lOle32"
	WIN:	"-mwindows"					; build as Windows GUI binary
	CON:	"-mconsole"					; build as Windows Console binary
	S4M:	"-Wl,--stack=4194300"
	-LM:	"-lm"						; HaikuOS has math in libroot, for instance
	NWK:	"-lnetwork"					; Needed by HaikuOS
	FCS:	"-framework CoreServices"
	FCM:	"-framework CoreMIDI"
]

other-flags: [
	+SC:	""							; has smart console
	-SP:	""							; non standard paths
	COP:	""							; use COPY as cp program
	DIR:	""							; use DIR as ls program
	ST1:	"-s"						; strip flags...
	STX:	"-x"
	ST2:	"-S -x -X"
	CMT:	"-R.comment"
	EXE:	""							; use %.exe as binary file suffix
]

config-system: func [
	"Return build configuration information"
	/fields "record variables"
	/define "the TO_TARGET define name"
	/os-dir "the %osname/ directory"
	/platform v [tuple!]
][
	if fields [return first systems]
	v: any [v to tuple! reduce [0 system/version/4 system/version/5]]
	foreach rec next systems [
		if rec/1 = v [
			if os-dir [return dirize to-file rec/3]
			if define [return to-word uppercase join "TO_" rec/2]
			return rec
		]
	]
	none
]
