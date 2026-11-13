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

1 Amiga [
	1 m68k20+
	2 m68k
	3 ppc
]

2 Macintosh [
	1 mac-ppc
	2 mac-m68k
	3 mac-misc
	4 osx-ppc
	5 osx-x86
	40 osx-x64
]

3 Windows [
	1 win32-x86
	2 dec-alpha
	40 win32-x64
]

4 Linux [
	1 libc5-x86
	2 libc6-2-3-x86
	3 libc6-2-5-x86
	4 libc6-2-11-x86
	10 libc6-ppc
	20 libc6-arm
	21 bionic-arm
	30 libc6-mips
	40 libc-x64
	46 musl-x64
]

5 Haiku [
	75 x86-32
]

6 BSDi [
	1 x86
]

7 FreeBSD [
	1 x86
	2 elf-x86
	40 elf-x64
]

8 NetBSD [
	1 x86
	2 ppc
	3 m68k
	4 dec-alpha
	5 sparc
]

9 OpenBSD [
	1 x86
	2 ppc
	3 m68k
	4 elf-x86
	5 sparc
]

10 Sun [
	1 sparc
]

11 SGI []

12 HP []

13 Android [
	1 arm
]

14 free-slot []

15 WindowsCE [
	1 sh3
	2 mips
	5 arm
	6 sh4
]
