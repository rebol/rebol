REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Platform definitions"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Purpose: {
		Platform identification found in system object.
	}
]

Amiga		[1 m68k20+ 2 m68k 3 ppc]
Macintosh	[1 mac-ppc 2 mac-m68k 3 mac-misc 4 osx-ppc 5 osx-x86]
Windows		[1 win32-x86 2 dec-alpha]
Linux		[1 libc5-x86 2 libc6-2-3-x86 3 libc6-2-5-x86 4 libc6-2-11-x86 10 libc6-ppc 20 libc6-arm 21 bionic-arm 30 libc6-mips]
Haiku		[75 x86-32]
BSDi		[1 x86]
FreeBSD		[1 x86 2 elf-x86]
NetBSD		[1 x86 2 ppc 3 m68k 4 dec-alpha 5 sparc]
OpenBSD		[1 x86 2 ppc 3 m68k 4 elf-x86 5 sparc]
Sun			[1 sparc]
SGI			[]
HP			[]
Android		[1 arm]
free-slot	[]
WindowsCE	[1 sh3 2 mips 5 arm 6 sh4]
