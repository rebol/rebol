/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Summary: System definitions
**  Module:  sys-system.h
**  Author:  Carl Sassenrath
**  Notes:
**		Define the target system by prefacing its name with "TO_".
**		Special per-system settings also go here.
**
***********************************************************************/

#ifndef SYSTEM_H
#define SYSTEM_H

#if (defined(AMIGA) || defined(_AMIGA)) && !defined(AMIGA1)
	#define TO_AMIGA
	#define SYS "1.1"
	extern long near __stack;
	#define STACK_SPACE __stack
	#define STACK_PAD	(4 * 1024)
	#define BIG_MODEL
	#define ENDIAN_BIG
	#define CPU_68K
#endif

#ifdef AMIGA1
	#define	TO_AMIGA
	#define	SYS "1.2"
	extern long near __stack;
	#define STACK_SPACE __stack
	#define STACK_PAD	(4 * 1024)
	#define BIG_MODEL
	#define ENDIAN_BIG
	#define CPU_68K
#endif

#ifdef macintosh
	#define TO_MAC
	#define INLINE inline
#ifdef __powerc
	#define TO_MAC_PPC
	#define SYS "2.1"
	#define CPU_PPC
#else
	#define TO_MAC_68K
	#define SYS "2.2"
	#define CPU_68K
#define STACK_SPACE (64 * 1024)
#define STACK_PAD (8 * 1024)
#endif
	#define BIG_MODEL
	#define ENDIAN_BIG
#endif

#ifdef MAC_OSX
	#define TO_MAC_OSX
	#define TO_UNIX
	#define SYS "2.4"
	#define CONSOLE_BSD
	#define BIG_MODEL
	#define ENDIAN_BIG
	#define CPU_PPC
	#define HAS_CALLBACKS
	#define RTLD_LAZY 1
#endif

#if (defined(_WIN32) || defined(__WIN32__)) && !defined(WIN32_ALPHA) &&\
	!defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
	#define	TO_WIN32
	#define SYS "3.1"
	#define ENDIAN_LITTLE
	#define CPU_X86
	#define STACK_SPACE (3000000) // set in linker makefile
#endif

#ifdef WIN32_ALPHA
	#define	TO_WIN32
	#define SYS "3.2"
	#define ENDIAN_LITTLE
	#define CPU_ALPHA
#endif

#ifdef LINUX_SLACKWARE
	#define	LINUX_LIBC5
#endif
#ifdef LINUX_REDHAT
	#define	LINUX_LIBC6
#endif
#ifdef LINUX_REDHAT_ALPHA
	#define LINUX_ALPHA_LIBC6
#endif

#ifdef LINUX_LIBC5
	#define TO_LINUX
	#define	TO_UNIX
	#define	SYS "4.1"
	#define CONSOLE_UNIX
	#define ENDIAN_LITTLE
	#define CPU_X86
#endif

#ifdef LINUX_LIBC6
	#define TO_LINUX
	#define	TO_UNIX
	#define	SYS "4.2"
	#define CONSOLE_UNIX
	#define ENDIAN_LITTLE
	#define CPU_X86
#endif

#ifdef LINUX_ALPHA_LIBC6
	#define TO_LINUX
	#define	TO_UNIX
	#define	SYS "4.3"
	#define CONSOLE_UNIX
	#define BIG_MODEL
	#define ENDIAN_LITTLE
	#define CPU_ALPHA
	#define PTR_SIZE_64
	#define LONG_SIZE_64
#endif

#ifdef LINUX_PPC_LIBC6
	#define TO_LINUX
	#define TO_UNIX
	#define SYS "4.4"
	#define CONSOLE_UNIX
	#define BIG_MODEL
	#define ENDIAN_BIG
	#define CPU_PPC
#endif

#ifdef LINUX_68K_LIBC6
	#define TO_LINUX
	#define TO_UNIX
	#define SYS "4.5"
	#define CONSOLE_UNIX
	#define BIG_MODEL
	#define ENDIAN_BIG
	#define CPU_68K
#endif

#ifdef LINUX_SPARC_LIBC6
	#define TO_LINUX
	#define TO_UNIX
	#define SYS "4.6"
	#define CONSOLE_UNIX
	#define BIG_MODEL
	#define ENDIAN_BIG
	#define CPU_SPARC
#endif

#ifdef LINUX_ULTRA_LIBC6
	#define TO_LINUX
	#define TO_UNIX
	#define SYS "4.7"
	#define CONSOLE_UNIX
	#define BIG_MODEL
	#define ENDIAN_BIG
	#define CPU_ULTRA
#endif

#ifdef LINUX_ARM_LIBC6
	#define TO_LINUX
	#define	TO_UNIX
	#define	SYS "4.8"
	#define CONSOLE_UNIX
	#define ENDIAN_LITTLE
	#define CPU_ARM
#endif

#ifdef LINUX_MIPS_LIBC6
	#define TO_LINUX
	#define	TO_UNIX
	#define	SYS "4.9"
	#define CONSOLE_UNIX
	#define ENDIAN_LITTLE
	#define BIG_MODEL
	#define CPU_MIPS
#endif

#ifdef BEOS_PPC
	#define TO_BEOS
	#define SYS "5.1"
	#define CONSOLE_UNIX
	#define BIG_MODEL
	#define STACK_SPACE (256 * 1024)
	#define ENDIAN_BIG
	#define CPU_PPC
#endif

#ifdef BEOS_X86
	#define TO_BEOS
	#define SYS "5.2"
	#define CONSOLE_UNIX
	#define STACK_SPACE (256 * 1024)
	#define ENDIAN_LITTLE
	#define CPU_X86
#endif

#ifdef BSDI
	#define TO_UNIX
	#define SYS "6.1"
	#define CONSOLE_BSD
	#define ENDIAN_LITTLE
	#define CPU_X86
#endif

#ifdef FREEBSD4
	#define TO_FREEBSD
	#define TO_UNIX
	#define SYS "7.2"
	#define CONSOLE_BSD
	//#define SINGLE_CHAR_CONSOLE
	#define ENDIAN_LITTLE
	#define CPU_X86
#endif

#ifdef FREEBSD
	#define TO_FREEBSD
	#define TO_UNIX
	#define SYS "7.1"
	#define CONSOLE_BSD
	//#define SINGLE_CHAR_CONSOLE
	#define ENDIAN_LITTLE
	#define CPU_X86
#endif

#ifdef NETBSD_X86
	#define TO_NETBSD
	#define TO_UNIX
	#define SYS "8.1"
	#define CONSOLE_BSD
	#define ENDIAN_LITTLE
	#define CPU_X86
#endif

#ifdef NETBSD_PPC
	#define TO_NETBSD
	#define TO_UNIX
	#define SYS "8.2"
	#define CONSOLE_BSD
	#define BIG_MODEL
	#define SINGLE_CHAR_CONSOLE
	#define BIG_MODEL
	#define ENDIAN_BIG
	#define CPU_PPC
#endif

#ifdef NETBSD_68K
	#define TO_NETBSD
	#define TO_UNIX
	#define SYS "8.3"
	#define CONSOLE_BSD
	#define BIG_MODEL
	#define ENDIAN_BIG
	#define CPU_68K
#endif

#ifdef NETBSD_ALPHA
	#define TO_NETBSD
	#define TO_UNIX
	#define SYS "8.4"
	#define CONSOLE_BSD
	#define ENDIAN_LITTLE
	#define CPU_ALPHA
	#define BIG_MODEL
	#define PTR_SIZE_64
#endif

#ifdef NETBSD_SPARC
	#define TO_NETBSD
	#define TO_UNIX
	#define SYS "8.5"
	#define CONSOLE_BSD
	#define BIG_MODEL
	#define SINGLE_CHAR_CONSOLE
	#define ENDIAN_BIG
	#define CPU_SPARC
#endif

#ifdef OPENBSD_X86
	#define TO_OPENBSD
	#define TO_UNIX
	#define SYS "9.1"
	#define CONSOLE_BSD
	#define ENDIAN_LITTLE
	#define CPU_X86
#endif

#ifdef OBSD_X86
	#define TO_OPENBSD
	#define TO_OBSD
	#define TO_UNIX
	#define SYS "9.2"
	#define CONSOLE_BSD
	#define ENDIAN_LITTLE
	#define CPU_X86
#endif

#ifdef OPENBSD_PPC
	#define TO_OPENBSD
	#define TO_UNIX
	#define SYS "9.2"
	#define CONSOLE_BSD
	#define BIG_MODEL
	#define ENDIAN_BIG
	#define CPU_PPC
#endif

#ifdef OPENBSD_68K
	#define TO_OPENBSD
	#define TO_UNIX
	#define SYS "9.3"
	#define CONSOLE_BSD
	#define BIG_MODEL
	#define ENDIAN_BIG
	#define CPU_68K
#endif

#ifdef OPENBSD_ALPHA
	#define TO_OPENBSD
	#define TO_UNIX
	#define SYS "9.4"
	#define CONSOLE_BSD
	#define ENDIAN_LITTLE
	#define CPU_ALPHA
#endif

#ifdef OPENBSD_SPARC
	#define TO_OPENBSD
	#define TO_UNIX
	#define SYS "9.5"
	#define CONSOLE_BSD
      #define SINGLE_CHAR_CONSOLE
	#define BIG_MODEL
	#define ENDIAN_BIG
	#define CPU_SPARC
#endif

#ifdef SOLARIS_SPARC
	#define TO_UNIX
	#define TO_SOLARIS
	#define SYS "10.1"
	#define CONSOLE_UNIX
	#define BIG_MODEL
      #define SINGLE_CHAR_CONSOLE
	#define ENDIAN_BIG
	#define CPU_SPARC
#endif

#ifdef SOLARIS_X86
	#define TO_UNIX
	#define TO_SOLARIS
	#define SYS "10.2"
	#define CONSOLE_UNIX
	#define ENDIAN_LITTLE
	#define CPU_X86
#endif

#ifdef SGI_IRIX
	#define TO_UNIX
	#define SYS "11"
	#define CONSOLE_UNIX
	#define BIG_MODEL
	#define ENDIAN_BIG
#endif

#ifdef HP_UX
	#define TO_UNIX
	#define SYS "12"
	#define CONSOLE_UNIX
	#define BIG_MODEL
	#define ENDIAN_BIG
#endif


#ifdef PALM
	#define TO_PALM
	#define SYS "14"
#endif

#if defined(_WIN32_WCE)
	#undef DEBUG
	#define TO_WINCE
	#define BIG_MODEL
	#define STACK_SPACE (64 * 1024)
	#define STACK_PAD (16 * 1024)
	#if defined(SH3)
		#define SYS "15.1"
		#define BIG_MODEL
		#define ENDIAN_LITTLE
	#elif defined(SH4)
		#define SYS "15.6"
		#define BIG_MODEL
		#define ENDIAN_LITTLE
	#elif defined(MIPS)
		#define SYS "15.2"
		#define BIG_MODEL
		#define ENDIAN_LITTLE
		#define CPU_MIPS
	#elif defined(PPC)
		#define SYS "15.3"
		#define BIG_MODEL
		#define ENDIAN_BIG
		#define CPU_PPC
	#elif defined(i486)
		#define SYS "15.4"
		#define ENDIAN_LITTLE
		#define CPU_X86
	#elif defined(ARM)
		#define SYS "15.5"
		#define ENDIAN_LITTLE
		#define CPU_ARM
	#endif
#endif

#ifdef OS2
	#define TO_OS2
	#define SYS "16"
	#define TO_UNIX
	#define CONSOLE_UNIX
	#define ENDIAN_LITTLE
	#define CPU_X86
#endif

#ifdef AIX
	#define TO_UNIX
	#define SYS "17"
	#define CONSOLE_AIX
	#define BIG_MODEL
	#define ENDIAN_BIG
#endif

#ifdef AS400
	#define TO_AS400
	#define SYS "18"
#endif

#ifdef SCO_UNIXWARE
	#define TO_SCO
	#define TO_UNIX
	#define SYS "19"
	#define CONSOLE_BSD
	#define INLINE
	#define ENDIAN_LITTLE
	#define CPU_X86
#endif

#ifdef SCO_OPENSERVER
	#define TO_SCO
	#define TO_UNIX
	#define SYS "24"
	#define CONSOLE_BSD
	#define INLINE
	#define ENDIAN_LITTLE
	#define CPU_X86
#endif

#ifdef QNX_RTOS
	#define TO_QNX
	#define TO_UNIX
	#define SYS "22"
	#define CONSOLE_BSD
	#define INLINE
	#define STACK_SPACE (32 * 1024)
	#define STACK_PAD (16 * 1024)
	#define ENDIAN_LITTLE
	#define CPU_X86
#endif

#ifdef QNX_RTP
	#define TO_QNX
	#define TO_UNIX
	#define SYS "28"
	#define CONSOLE_BSD
	#define INLINE
	#define STACK_SPACE (32 * 1024)
	#define STACK_PAD (16 * 1024)
	#define ENDIAN_LITTLE
	#define CPU_X86
#endif

#ifdef ELATE_VP
	#define TO_UNIX
	#define TO_ELATE
	#define SYS "27.1"
	#define BIG_MODEL
	#define ENDIAN_LITTLE
	#define CPU_VP
#endif

#if defined(__SYMBIAN32__)
#if defined(__WINS__)
	#define PSION
	#define TO_PSION
	#define SYS "13.2"
	#define ENDIAN_LITTLE
#elif defined(__MARM__)
	#define PSION
	#define TO_PSION
	#define SYS "13.1"
	#define ENDIAN_LITTLE
#else
#error Unknown EPOC platform
#endif
#endif

/***********************************************************************
**
**	SYSTEM GROUPS
**
**		Define some common system groups, in addition to those already
**		defined under SYSTEM DEFINITIONS.
**
***********************************************************************/

#if defined(BSDI) || defined(TO_FREEBSD) || defined(TO_NETBSD) || defined(TO_OPENBSD)
	#define TO_BSD
#endif

#if defined(TO_WIN32) || defined(TO_WINCE)
	#define TO_WINDOWS
#endif

/***********************************************************************
**
**	HEADER FILES
**
**		For each system-specific header file define a corresponding
**		HAS_..._H symbol if the current system has that header file.
**		Sorted alphabetically.
**
***********************************************************************/

#if defined(TO_UNIX) && !defined(NETBSD_ALPHA)
	#define HAS_ARPA_INET_H
#endif

#if defined(TO_LINUX)
	#define HAS_ASM_IOCTLS_H
#endif

#if defined(TO_BSD) || defined(TO_LINUX) || defined(AIX) || defined(SGI_IRIX)
	#define HAS_CDEFS_H
#endif

#if defined(TO_WIN32)
	#define HAS_DIRECT_H
#endif

#if defined(TO_AMIGA)
	#define HAS_DIRENT_H
#endif

#if defined(HP_UX)
	#define HAS_DL_H
#endif

#if defined(TO_UNIX) && !defined(QNX_RTOS) && !defined(TO_ELATE) && !defined(HP_UX) && !defined(TO_MAC_OSX)
	#define HAS_DLFCN_H
#endif

#if !defined(TO_WINCE)
	#define HAS_ERRNO_H
#endif

#if !defined(TO_WINCE)
	#define HAS_FCNTL_H
#endif

#if !defined(TO_PSION)
	#define HAS_FLOAT_H
#endif

#if defined(TO_AMIGA)
	#define HAS_FUNCTIONS_H
#endif

#if defined(TO_WIN32)
	#define HAS_IO_H
#endif

#if (defined(TO_UNIX) && !defined(TO_ELATE)) || defined(TO_AMIGA)
	#define HAS_NET_IF_H
#endif

#if defined(TO_UNIX) && !defined(TO_ELATE)
	#define HAS_NET_ROUTE_H
#endif

#if defined(TO_BEOS)
	#define HAS_NET_SOCKET_H
#endif

#if !defined(TO_WIN32) && !defined(TO_WINCE)
	#define HAS_NETDB_H
#endif

#if defined(TO_UNIX) || defined(TO_BEOS) || defined(TO_AMIGA)
	#define HAS_NETINET_IN_H
#endif

#if defined(TO_UNIX)
	#define HAS_NETINET_TCP_H
#endif

#if defined(TO_WIN32)
	#define HAS_PROCESS_H
#endif

#if defined(TO_UNIX) || defined(TO_BEOS)
	#define HAS_SIGNAL_H
#endif

#if defined(TO_WIN32)
	#define HAS_SNMP_H
#endif

#if defined(TO_MAC) || defined(TO_AMIGA)
	#define HAS_STAT_H
#endif

#if defined(TO_UNIX) && !defined(TO_QNX) && !defined(TO_ELATE)
	#define HAS_SYS_ERRNO_H
#endif

#if defined(TO_BSD) || defined(SGI_IRIX) || defined(TO_SOLARIS) || defined(MAC_OSX) || defined(TO_AMIGA) || defined(TO_SCO)
	#define HAS_SYS_FILIO_H
#endif

#if defined(TO_UNIX) || defined(TO_BEOS)
	#define HAS_SYS_IOCTL_H
#endif

#if defined(HP_UX)
	#define HAS_SYS_IOCTLS_H
#endif

#if defined(TO_UNIX)
	#define HAS_SYS_PARAM_H
#endif

#if defined(TO_QNX)
	#define HAS_SYS_SELECT_H
#endif

#if defined(TO_UNIX) || defined(TO_BEOS) || defined(TO_AMIGA)
	#define HAS_SYS_SOCKET_H
#endif

#if defined(TO_BSD) || defined(TO_SCO) || defined(TO_SOLARIS) || defined(TO_AMIGA)
	#define HAS_SYS_SOCKIO_H
#endif

#if defined(LINUX_REDHAT)
	#define HAS_SYS_SOUNDCARD_H
#endif

#if defined(TO_BSD)
	#define HAS_SYS_SYSCTL_H
#endif

#if defined(TO_WIN32) || defined(TO_UNIX) || defined(TO_AMIGA)
	#define HAS_SYS_STAT_H
#endif

#if defined(TO_UNIX) || defined(TO_AMIGA)
	#define HAS_SYS_TIME_H
#endif

#if defined(TO_UNIX) || defined(TO_BEOS)
	#define HAS_SYS_TYPES_H
#endif

#if defined(TO_UNIX) && !defined(TO_ELATE)
	#define HAS_SYS_UN_H
#endif

#if defined(TO_UNIX)
	#define HAS_SYS_WAIT_H
#endif

#if defined(TO_BEOS)
	#define HAS_TERMCAP_H
#endif

#if defined(TO_UNIX) || defined(TO_BEOS)
	#define HAS_TERMIOS_H
#endif

#if !defined(TO_WINCE)
	#define HAS_TIME_H
#endif

#if defined(TO_MAC) || defined(TO_UNIX)
	#define HAS_UNISTD_H
#endif

#if defined(TO_WIN32)
	#define HAS_WINDOWS_H
#endif

#if defined(TO_WIN32)
	#define HAS_WINDOWSX_H
#endif

#if defined(TO_WIN32) || defined(TO_WINCE)
	#define HAS_WINSOCK_H
#endif



/***********************************************************************
**
**	SYSTEM SETTINGS
**
**		Define a few additional symbols describing system-specific
**		oddities or properties.
**
***********************************************************************/

#if defined(TO_BSD) || defined(MAC_OSX)
	#define HAS_TZ_IN_LOCAL_TM		// timezone can be obtained from (struct tm).tm_gmtoff
#endif

#if defined(TO_LINUX) || defined(AIX) || defined(TO_SCO) || defined(SGI_IRIX) ||\
 defined(HP_UX) || defined(TO_SOLARIS) || defined(TO_QNX) || defined(TO_ELATE)
	#define HAS_TZ_IN_ISDST			// timezone can be obtained from diffing two times.
									// Supports (struct tm).tm_isdst
#endif

#if defined(TO_UNIX) || defined(TO_BEOS)
	#define HAS_GET_TIME_OF_DAY		// Has function gettimeofday()
#endif

#if !defined(TO_BSD) && !defined(TO_MAC) && !defined(BEOS_PPC) && !defined(TO_QNX) \
 && !defined(TO_ELATE) && !defined(TO_PSION) && !defined(TO_MAC_OSX)
	#define HAS_GCVT				// Has function gcvt()
#endif

#if defined(TO_BSD)
	#define HAS_ROUTING_SYSCTL		// Has routing table access through sysctl()
#endif

#if defined(TO_WIN32) || (defined(TO_UNIX) && !defined(SGI_IRIX) && !defined(TO_SOLARIS) && \
 !defined(TO_QNX) && !defined(TO_ELATE)) || defined(TO_PSION)
#if !defined(TO_LINUX) || !defined(DEBUGMODE)
	#define HAS_ASYNC_DNS			// Has asynchronous DNS support (in some way)
#endif
#endif

#if !defined(TO_ELATE) && !defined(TO_BEOS) && !defined(TO_WINCE)
	#define HAS_INTERFACE_QUERY		// Allows network interface queries (in some way)
#endif

#if defined(NETBSD_ALPHA) || defined(LINUX_ALPHA_LIBC6)
	#define HAS_LONG_PTRS			// Pointers cast to numbers must use long (not int)
#endif

#if !defined(TO_LINUX) && !defined(TO_SOLARIS) && !defined(SGI_IRIX) && !defined(HP_UX) && !defined(TO_WIN32)
	#define HAS_SOCKADDR_SA_LEN		// Has sa_len member in struct sockaddr
#endif

#if !defined(OS2) && !defined(TO_AMIGA) && !defined(TO_QNX) && !defined(TO_ELATE) \
	&& !defined(TO_PSION)
	#define HAS_FINITE				// Has _finite function
#endif

#if !defined(SOLARIS_SPARC) && !defined(TO_WIN32)
	#define HAS_LONG_ALIGNED_DOUBLE	// Doubles are long-aligned (not double-aligned)
#endif

#if defined(TO_AMIGA)
	#define HAS_SHORT_ALIGNED_LONG	// Longs and pointers are short-aligned (not long-aligned)
#endif

#if defined(SOLARIS_SPARC)
	#define HAS_LONG_LONG_DOUBLE	// Must copy doubles using long longs
#endif

//#if defined(TO_FREEBSD) || defined(TO_OPENBSD) || defined(TO_LINUX)
	#define HAS_MIT_SHM				// Supports X11R6 MIT Shared Memory Extension
//#endif

#if !defined(TO_WINCE)
	#define HAS_GETENV				// Has getenv() call
#endif

#if defined(TO_UNIX) || defined(TO_BEOS)
	#define HAS_CASE_FILES			// Has case-sensitive file names
#endif

#if defined(TO_UNIX) && !defined(TO_ELATE)
	#define HAS_CTRLZ				// Has Ctrl-Z for job control
#endif

#if defined(TO_UNIX) || defined(TO_BEOS) || defined(TO_AMIGA)
	#define HAS_SHARED_LAUNCH_CONSOLE	// Multiple launched copies share the same console
#endif

#if defined(TO_UNIX) && !defined(TO_ELATE)
	#define HAS_TILDE_EXPANSION		// Uses ~ for the user's home directory
#endif

#if defined(TO_UNIX)
	#define HAS_WAITPID				// Has waitpid() call
#endif

#if defined(TO_BSD) || defined(TO_LINUX)
	#define HAS_BSD_FILE_LOCKS		// Supports BSD-style file locking
#endif

#if defined(TO_UNIX)
	#define HAS_SET_SPEED			// Has cfsetispeed()/cfsetospeed() calls to set the baud rate
#endif

#if defined(TO_UNIX)
	#define HAS_GETPID				// Has getpid()
#endif

#if defined(TO_WIN32) || defined(TO_UNIX)
	#define HAS_SHELL_PIPES			// Supports i/o pipes in Shell component
#endif

#if !defined(TO_AMIGA)
	#define HAS_INT64				// Has 64-bit integer datatype in C
#endif

#if defined(LINUX_REDHAT)
	#define HAS_SOUND_OSS			// Uses Open Sound System for sound
#endif

#if defined(TO_UNIX)
	#define HAS_FASTCGI_POSIX_STARTUP	// Has Posix-style FastCGI startup (passing listen socket in stdin)
#endif

#if defined(CPU_X86) || defined(CPU_68K)
	#define HAS_STACK_BASED_ARGS	// Arguments are passed on the stack
#endif

#if (defined(TO_UNIX) || defined(TO_WIN32)) && defined(HAS_STACK_BASED_ARGS)
	#define HAS_CALLBACKS			// Library API supports REBOL callbacks
#endif

#if defined(TO_UNIX) && defined(COMP_VIEW)
	#define HAS_OS_REQUEST
#endif

#endif

