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
**  Title: Command line argument processing
**  Author: Carl Sassenrath
**  Caution: OS independent
**  Purpose:
**      Parses command line arguments and options, storing them
**      in a structure to be used by the REBOL library.
**
************************************************************************
**
**  NOTE to PROGRAMMERS:
**
**    1. Keep code clear and simple.
**    2. Document unusual code, reasoning, or gotchas.
**    3. Use same style for code, vars, indent(4), comments, etc.
**    4. Keep in mind Linux, OS X, BSD, big/little endian CPUs.
**    5. Test everything, then test it again.
**
***********************************************************************/

#include <stdlib.h>
#include <string.h>

#include "reb-config.h"
#include "reb-c.h"
#include "reb-args.h"

extern int OS_Get_Current_Dir(REBCHR **lp);
extern REBOOL OS_Get_Boot_Path(REBCHR **path);

// REBOL Option --Words:

const struct {const char *word; const int flag;} arg_words[] = {
	// Keep in Alpha order!
	{"args",		RO_ARGS | RO_EXT},
	{"boot",		RO_BOOT | RO_EXT},
	{"cgi",			RO_CGI | RO_QUIET},
	{"debug",		RO_DEBUG | RO_EXT},
	{"do",			RO_DO | RO_EXT},
	{"halt",		RO_HALT},
	{"help",		RO_HELP},
	{"import",		RO_IMPORT | RO_EXT},
	{"quiet",		RO_QUIET},
	{"script",		RO_SCRIPT | RO_EXT},
	{"secure",		RO_SECURE | RO_EXT},
	{"trace",		RO_TRACE},
	{"verbose",		RO_VERBOSE},
	{"version",		RO_VERSION | RO_EXT},
	{"",			0},
};

// REBOL Option -Characters (in alpha sorted order):

const struct arg_chr {const char cflg; const int flag;} arg_chars[] = {
	{'?',	RO_HELP},
	{'V',	RO_VERS},
	{'b',	RO_BOOT | RO_EXT},
	{'c',	RO_CGI | RO_QUIET},
	{'h',	RO_HALT},
	{'q',	RO_QUIET},
	{'s',	RO_SECURE_MIN},
	{'t',	RO_TRACE},
	{'v',	RO_VERS},
	{'w',	RO_NO_WINDOW},
	{'\0',	0},
};

// REBOL Option +Characters:

const struct arg_chr arg_chars2[] = {
	{'s',	RO_SECURE_MAX},
	{'\0',	0},
};


/***********************************************************************
**
*/	static int find_option_word(REBCHR *word)
/*
**		Scan options, return flag bits, else zero.
**
***********************************************************************/
{
	int n;
	int i;
	char buf[16];

	// Some shells will pass us the line terminator. Ignore it.
	if (word[0] == '\r' || word[0] == '\n') return RO_IGNORE;

	FROM_OS_STR(buf, word, 15);

	for (i = 0; arg_words[i].flag; i++) {
		n = strncmp(buf, arg_words[i].word, 15); // correct (bytes)
		if (n < 0) break;
		if (n == 0) return arg_words[i].flag;
	}
	return 0;
}


/***********************************************************************
**
*/	static int find_option_char(REBCHR chr, const struct arg_chr list[])
/*
**		Scan option char flags, return flag bits, else zero.
**
***********************************************************************/
{
	int i;

	// Some shells will pass us the line terminator. Ignore it.
	if (chr == '\r' || chr == '\n') return RO_IGNORE;

	for (i = 0; list[i].flag; i++) {
		if (chr < list[i].cflg) break;
		if (chr == list[i].cflg) return list[i].flag;
	}
	return 0;
}


/***********************************************************************
**
*/	static int Get_Ext_Arg(int flag, REBARGS *rargs, REBCHR *arg)
/*
**		Get extended argument field.
**
***********************************************************************/
{
	if (arg && arg[1] == (REBCHR)'-') return flag;

	flag &= ~RO_EXT;

	switch (flag) {

	case RO_VERSION:
		rargs->version = arg;
		break;

	case RO_SCRIPT:
		rargs->script = arg;
		break;

	case RO_ARGS:
		rargs->args = arg;
		break;

	case RO_DO:
		rargs->do_arg = arg;
		break;

	case RO_DEBUG:
		rargs->debug = arg;
		break;

	case RO_SECURE:
		rargs->secure = arg;
		break;

	case RO_IMPORT:
		rargs->import = arg;
		break;

	case RO_BOOT:
		rargs->boot = arg;
		break;
	}

	return flag;
}


/***********************************************************************
**
*/	void Parse_Args(int argc, REBCHR **argv, REBARGS *rargs)
/*
**		Parse REBOL's command line arguments, setting options
**		and values in the provided args structure.
**
**		If RAW_MAIN_ARGS is used, the arguments list is not being
**		parsed in this function, but instead will be converted to
**		block of strings and leaved on interpreter to process it.
**
***********************************************************************/
{
#ifdef RAW_MAIN_ARGS
	CLEARS(rargs);
	rargs->argc = argc;
	rargs->argv = argv;
	if (0 == OS_Get_Boot_Path(&rargs->exe_path)) {
		// First arg is path to executable (on most systems):
		if (argc > 0) rargs->exe_path = *argv;
	}
	OS_Get_Current_Dir(&rargs->current_dir);

#else

	REBCHR *arg;
	int flag, i;

	CLEARS(rargs);

	if (!OS_Get_Boot_Path(&rargs->exe_path)) {
		// In case of fail...
		// First arg is path to executable (on most systems):
		if (argc > 0) rargs->exe_path = *argv;
	}

	OS_Get_Current_Dir(&rargs->current_dir);

	// Parse each argument:
	for (i = 1; i < argc ; i++) {
		arg = argv[i];
		if (arg == 0) continue; // shell bug
		if (*arg == '-') {
			if (arg[1] == '-') {
				if (arg[2] == 0) {
					// --  stops options parsing for the interpreter
					break;
				}
				// --option words
				flag = find_option_word(arg+2);
				if (flag & RO_EXT) {
					flag = Get_Ext_Arg(flag, rargs, (i+1 >= argc) ? 0 : argv[i+1]);
					if ((flag & RO_EXT) == 0) i++; // used it
					else flag &= ~RO_EXT;
				}
				if (!flag) flag = RO_HELP;
				rargs->options |= flag;
			}
			else {
				// -x option chars
				while (*++arg) {
					flag = find_option_char(*arg, arg_chars);
					if (flag & RO_EXT) {
						flag = Get_Ext_Arg(flag, rargs, (i+1 >= argc) ? 0 : argv[i+1]);
						if ((flag & RO_EXT) == 0) i++; // used it
						else flag &= ~RO_EXT;
					}
					if (!flag) flag = RO_HELP;
					rargs->options |= flag;
				}
			}
		}
		else if (*arg == '+') {
			// +x option chars
			while (*++arg) {
				flag = find_option_char(*arg, arg_chars2);
				if (flag & RO_EXT) {
					flag = Get_Ext_Arg(flag, rargs, (i+1 >= argc) ? 0 : argv[i+1]);
					if ((flag & RO_EXT) == 0) i++; // used it
					else flag &= ~RO_EXT;
				}
				if (!flag) flag = RO_HELP;
				rargs->options |= flag;
			}
		}
		else {
			// script filename
			if (rargs->script) {
				// we already have the script from --script option
				// so this should be first arg instead.
				--i; // revert the counter so this value is collected later
			}
			else {
				rargs->script = arg;
				// after having processed a command-line argument as scriptname,
				// all remaining arguments are passed as-is to the script (via system/options/args)
			}
			break;
		}
	}
	// if there are still unprocessed args, pass them as a block (converted later in b-init.c)
	if (argc > i++) {
		rargs->argc = argc - i;
		rargs->argv = argv + i;
	}
#endif
}



