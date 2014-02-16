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
**  Title: Simple readline() line input handler
**  Author: Carl Sassenrath
**  Purpose:
**      Processes special keys for input line editing and recall.
**      Avoides use of complex OS libraries and GNU readline().
**      but hardcodes some parts only for the common standard.
**
**  Usage: This file is meant to be used in more than just REBOL, so
**  it does not include the normal REBOL header files, but rather
**  defines its own types and constants.
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
#include <stdio.h>
#include <string.h>

//#define TEST_MODE  // teset as stand-alone program

#ifdef NO_TTY_ATTRIBUTES
#ifdef TO_WIN32
#include <io.h>
#endif
#else
#include <termios.h>
#endif

#define FALSE 0
#define TRUE (0==0)

enum {
	BEL =   7,
	BS  =   8,
	LF  =  10,
	CR  =  13,
	ESC =  27,
	DEL = 127,
};

// Configuration:
#define TERM_BUF_LEN 4096	// chars allowed per line
#define READ_BUF_LEN 64		// chars per read()
#define MAX_HISTORY  300	// number of lines stored

// Macros: (does not use reb-c.h)
#define MAKE_STR(l) (char*)malloc(l)
#define WRITE_CHAR(s)    write(1, s, 1)
#define WRITE_CHARS(s,l) write(1, s, l)
#define WRITE_STR(s)     write(1, s, strlen(s))

#define DBG_INT(t,n) //printf("\r\ndbg[%s]: %d\r\n", t, (n));
#define DBG_STR(t,s) //printf("\r\ndbg[%s]: %s\r\n", t, (s));

typedef struct term_data {
	char *buffer;
	char *residue;
	char *out;
	int pos;
	int end;
	int hist;
} STD_TERM;

// Globals:
static int  Term_Init = 0;			// Terminal init was successful
static char **Line_History;		// Prior input lines
static int Line_Count;			// Number of prior lines

#ifndef NO_TTY_ATTRIBUTES
static struct termios Term_Attrs;	// Initial settings, restored on exit
#endif


/***********************************************************************
**
*/	STD_TERM *Init_Terminal(void)
/*
**		Change the terminal modes to those required for proper
**		REBOL console handling. Return TRUE on success.
**
***********************************************************************/
{
	STD_TERM *term;
#ifndef NO_TTY_ATTRIBUTES
	struct termios attrs;

	if (Term_Init || tcgetattr(0, &Term_Attrs)) return FALSE;

	attrs = Term_Attrs;

	// Local modes:
	attrs.c_lflag &= ~(ECHO | ICANON); // raw input

	// Input modes:
	attrs.c_iflag &= ~(ICRNL | INLCR); // leave CR an LF as is

	// Output modes:
	attrs.c_oflag |= ONLCR; // On output, emit CRLF

	// Special modes:
	attrs.c_cc[VMIN] = 1;	// min num of bytes for READ to return
	attrs.c_cc[VTIME] = 0;	// how long to wait for input

	tcsetattr(0, TCSADRAIN, &attrs);
#endif

	// Setup variables:
	Line_History = (char**)malloc((MAX_HISTORY+2) * sizeof(char*));
	Line_History[0] = "";
	Line_Count = 1;

	term = malloc(sizeof(*term));
	memset(term, 0, sizeof(*term));
	term->buffer = MAKE_STR(TERM_BUF_LEN);
	term->buffer[0] = 0;
	term->residue = MAKE_STR(TERM_BUF_LEN);
	term->residue[0] = 0;

	Term_Init = TRUE;

	return term;
}


/***********************************************************************
**
*/	void Quit_Terminal(STD_TERM *term)
/*
**		Restore the terminal modes original entry settings,
**		in preparation for exit from program.
**
***********************************************************************/
{
	int n;

	if (Term_Init) {
#ifndef NO_TTY_ATTRIBUTES
		tcsetattr(0, TCSADRAIN, &Term_Attrs);
#endif
		free(term->residue);
		free(term->buffer);
		free(term);
		for (n = 1; n < Line_Count; n++) free(Line_History[n]);
		free(Line_History);
	}

	Term_Init = FALSE;
}


/***********************************************************************
**
*/	static void Write_Char(char c, int n)
/*
**		Write out repeated number of chars.
**		Unicode: not used
**
***********************************************************************/
{
	char buf[4];

	buf[0] = c;
	for (; n > 0; n--) WRITE_CHAR(buf);
}


/***********************************************************************
**
*/	static void Store_Line(STD_TERM *term)
/*
**		Makes a copy of the current buffer and store it in the
**		history list. Returns the copied string.
**
***********************************************************************/
{
	term->buffer[term->end] = 0;
	term->out = MAKE_STR(term->end + 1);
	strcpy(term->out, term->buffer);

	// If max history, drop older lines (but not [0] empty line):
	if (Line_Count >= MAX_HISTORY) {
		free(Line_History[1]);
		memmove(Line_History+1, Line_History+2, (MAX_HISTORY-2)*sizeof(char*));
		Line_Count = MAX_HISTORY-1;
	}

	Line_History[Line_Count++] = term->out;
}


/***********************************************************************
**
*/	static void Recall_Line(STD_TERM *term)
/*
**		Set the current buffer to the contents of the history
**		list at its current position. Clip at the ends.
**		Return the history line index number.
**		Unicode: ok
**
***********************************************************************/
{
	if (term->hist < 0) term->hist = 0;

	if (term->hist == 0)
		Write_Char(BEL, 1); // bell

	if (term->hist >= Line_Count) {
		// Special case: no "next" line:
		term->hist = Line_Count;
		term->buffer[0] = 0;
		term->pos = term->end = 0;
	}
	else {
		// Fetch prior line:
		strcpy(term->buffer, Line_History[term->hist]);
		term->pos = term->end = strlen(term->buffer);
	}
}


/***********************************************************************
**
*/	static void Clear_Line(STD_TERM *term)
/*
**		Clear all the chars from the current position to the end.
**		Reset cursor to current position.
**		Unicode: not used
**
***********************************************************************/
{
	Write_Char(' ', term->end - term->pos); // wipe prior line
	Write_Char(BS, term->end - term->pos); // return to position
}


/***********************************************************************
**
*/	static void Home_Line(STD_TERM *term)
/*
**		Reset cursor to home position.
**		Unicode: not used
**
***********************************************************************/
{
	Write_Char(BS, term->pos);
	term->pos = 0;
}


/***********************************************************************
**
*/	static void End_Line(STD_TERM *term)
/*
**		Move cursor to end position.
**		Unicode: not used
**
***********************************************************************/
{
	int len = term->end - term->pos;

	if (len > 0) {
		WRITE_CHARS(term->buffer+term->pos, len);
		term->pos = term->end;
	}
}


/***********************************************************************
**
*/	static void Show_Line(STD_TERM *term, int blanks)
/*
**		Refresh a line from the current position to the end.
**		Extra blanks can be specified to erase chars off end.
**		If blanks is negative, stay at end of line.
**		Reset the cursor back to current position.
**		Unicode: ok
**
***********************************************************************/
{
	int len;

	//printf("\r\nsho pos: %d end: %d ==", term->pos, term->end);

	// Clip bounds:
	if (term->pos < 0) term->pos = 0;
	else if (term->pos > term->end) term->pos = term->end;

	if (blanks >= 0) {
		len = term->end - term->pos;
		WRITE_CHARS(term->buffer+term->pos, len);
	}
	else {
		WRITE_CHARS(term->buffer, term->end);
		blanks = -blanks;
		len = 0;
	}

	Write_Char(' ', blanks);
	Write_Char(BS, blanks + len); // return to position or end
}


/***********************************************************************
**
*/	static char *Insert_Char(STD_TERM *term, char *cp)
/*
**		Insert a char at the current position. Adjust end position.
**		Redisplay the line.
**		Unicode: not yet supported!
**
***********************************************************************/
{
	//printf("\r\nins pos: %d end: %d ==", term->pos, term->end);
	if (term->end < TERM_BUF_LEN-1) { // avoid buffer overrun

		if (term->pos < term->end) { // open space for it:
			memmove(term->buffer + term->pos + 1, term->buffer + term->pos, 1 + term->end - term->pos);
		}
		WRITE_CHAR(cp);
		term->buffer[term->pos] = *cp;
		term->end++;
		term->pos++;
		Show_Line(term, 0);
	}

	return ++cp;
}


/***********************************************************************
**
*/	static void Delete_Char(STD_TERM *term, int back)
/*
**		Delete a char at the current position. Adjust end position.
**		Redisplay the line. Blank out extra char at end.
**		Unicode: not yet supported!
**
***********************************************************************/
{
	int len;

	if ( (term->pos == term->end) && back == 0) return; //Ctrl-D at EOL
   
	if (back) term->pos--;

	len = 1 + term->end - term->pos;

	if (term->pos >= 0 && len > 0) {
		memmove(term->buffer + term->pos, term->buffer + term->pos + 1, len);
		if (back) Write_Char(BS, 1);
		term->end--;
		Show_Line(term, 1);
	}
	else term->pos = 0;
}


/***********************************************************************
**
*/	static void Move_Cursor(STD_TERM *term, int count)
/*
**		Move cursor right or left by one char.
**		Unicode: not yet supported!
**
***********************************************************************/
{
	if (count < 0) {
		if (term->pos > 0) {
			term->pos--;
			Write_Char(BS, 1);
		}
	}
	else {
		if (term->pos < term->end) {
			WRITE_CHAR(term->buffer + term->pos);
			term->pos++;
		}
	}
}


/***********************************************************************
**
*/	static char *Process_Key(STD_TERM *term, char *cp)
/*
**		Process the next key. If it's an edit key, perform the
**		necessary editing action. Return position of next char.
**		Unicode: not yet supported!
**
***********************************************************************/
{
	int len;

	if (*cp == 0) return cp;

	// No UTF-8 yet
	if (*cp < 0) *cp = '?';

	if (*cp == ESC) {
		// Escape sequence:
		cp++;
		if (*cp == '[' || *cp == 'O') {

			// Special key:
			switch (*++cp) {

			// Arrow keys:
			case 'A':	// up arrow
				term->hist -= 2;
			case 'B':	// down arrow
				term->hist++;
				len = term->end;
				Home_Line(term);
				Recall_Line(term);
				if (len <= term->end) len = 0;
				else len = term->end - len;
				Show_Line(term, len-1); // len < 0 (stay at end)
				break;

			case 'D':	// left arrow
				Move_Cursor(term, -1);
				break;
			case 'C':	// right arrow
				Move_Cursor(term, 1);
				break;

			// Other special keys:
			case '1':	// home
				Home_Line(term);
				cp++; // remove ~
				break;
			case '4':	// end
				End_Line(term);
				cp++; // remove ~
				break;
			case '3':	// delete
				Delete_Char(term, FALSE);
				cp++; // remove ~
				break;

			case 'H':	// home
				Home_Line(term);
				break;
			case 'F':	// end
				End_Line(term);
				break;

			default:
				WRITE_STR("[ESC]");
				cp--;
			}
		}
		else {
			switch (*++cp) {
			case 'H':	// home
				Home_Line(term);
				break;
			case 'F':	// end
				End_Line(term);
				break;
			default:
				// Q: what other keys do we want to support ?!
				WRITE_STR("[ESC]");
				cp--;
			}
		}
	}
	else {
		// ASCII char:
		switch (*cp) {

		case  BS:	// backspace
		case DEL:	// delete
			Delete_Char(term, TRUE);
			break;

		case CR:	// CR
			if (cp[1] == LF) cp++; // eat
		case LF:	// LF
			WRITE_STR("\r\n");
			Store_Line(term);
			break;

		case 1:	// CTRL-A
			Home_Line(term);
			break;
		case 2:	// CTRL-B
			Move_Cursor(term, -1);
			break;
		case 4:	// CTRL-D
			Delete_Char(term, FALSE);
			break;
		case 5:	// CTRL-E
			End_Line(term);
			break;
		case 6:	// CTRL-F
			Move_Cursor(term, 1);
			break;

		default:
			cp = Insert_Char(term, cp);
			cp--;
		}
	}

	return ++cp;
}


/***********************************************************************
**
*/	static int Read_Bytes(STD_TERM *term, char *buf, int len)
/*
**		Read the next "chunk" of data into the terminal buffer.
**
***********************************************************************/
{
	int end;

	// If we have leftovers:
	if (term->residue[0]) {
		end = strlen(term->residue);
		if (end < len) len = end;
		strncpy(buf, term->residue, len); // terminated below
		memmove(term->residue, term->residue+len, end-len); // remove
		term->residue[end-len] = 0;
	}
	else {
		// Read next few bytes. We don't know how many may be waiting.
		// We assume that escape-sequences are always complete in buf.
		// (No partial escapes.) If this is not true, then we will need
		// to add an additional "collection" loop here.
		if ((len = read(0, buf, len)) < 0) {
			WRITE_STR("\r\nI/O terminated\r\n");
			Quit_Terminal(term); // something went wrong
			exit(100);
		}
	}

	buf[len] = 0;
	buf[len+1] = 0;

	DBG_INT("read len", len);

	return len;
}


/***********************************************************************
**
*/	int Read_Line(STD_TERM *term, char *result, int limit)
/*
**		Read a line (as a sequence of bytes) from the terminal.
**		Handles line editing and line history recall.
**		Returns number of bytes in line.
**
***********************************************************************/
{
	char buf[READ_BUF_LEN];
	char *cp;
	int len;		// length of IO read

	term->pos = term->end = 0;
	term->hist = Line_Count;
	term->out = 0;
	term->buffer[0] = 0;

	do {
		Read_Bytes(term, buf, READ_BUF_LEN-2);
		for (cp = buf; *cp;) {
			cp = Process_Key(term, cp);
		}
	} while (!term->out);

	// Not at end of input? Save any unprocessed chars:
	if (*cp) {
		if (strlen(term->residue) + strlen(cp) < TERM_BUF_LEN-1) // avoid overrun
			strcat(term->residue, cp);
	}

	// Fill the output buffer:
	len = strlen(term->out);
	if (len >= limit-1) len = limit-2;
	strncpy(result, term->out, limit);
	result[len++] = LF;
	result[len] = 0;

	return len;
}

#ifdef TEST_MODE
test(STD_TERM *term, char *cp) {
	term->hist = Line_Count;
	term->pos = term->end = 0;
	term->out = 0;
	term->buffer[0] = 0;
	while (*cp) cp = Process_Key(term, cp);
}

main() {
	int i;
	char buf[1024];
	STD_TERM *term;

	term = Init_Terminal();

	Write_Char('-', 50);
	WRITE_STR("\r\n");

#ifdef WIN32
	test(term, "text\010\010st\n"); //bs bs
	test(term, "test\001xxxx\n"); // home
	test(term, "test\001\005xxxx\n"); // home
	test(term, "\033[A\n"); // up arrow
#endif

	do {
		WRITE_STR(">> ");
		i = Read_Line(term, buf, 1000);
		printf("len: %d %s\r\n", i, term->out);
	} while (i > 0);

	Quit_Terminal(term);
}
#endif
