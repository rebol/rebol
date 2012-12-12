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

Amiga		[m68]
Macintosh	[mac-ppc mac-m68 mac-misc osx-ppc osx-x86]
Windows		[win32-x86]
Linux		[libc5-x86 libc6-x86 libc6f-x86 libc6-2-11-x86]
Linux-PPC	[libc6-ppc]
Linux-ARM	[libc6-arm]
FreeBSD		[x86 elf-x86]
NetBSD		[x86]
OpenBSD		[x86]
Sun			[sparc]
