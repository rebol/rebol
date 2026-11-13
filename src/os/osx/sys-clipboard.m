/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**	Copyright 2021 Oldes
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
**  Title: Device: Clipboard access for macOS
**  Author: Oldes
**  Purpose:
**      Provides a very simple interface to the clipboard for text.
**      May be expanded in the future for images, etc.
**
************************************************************************/

#include "reb-host.h"
#include "host-lib.h" // for OS_Make
#include <AppKit/AppKit.h>

void EmptyClipboard(void){
	[[NSPasteboard generalPasteboard] clearContents];
}

REBOOL GetClipboardStringData(REBREQ *req){
	unsigned long len;
	NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
	NSArray *classes=[NSArray arrayWithObject:[NSString class]];
	NSDictionary *options=[NSDictionary dictionary];
	NSArray *clipboardContent=[pasteboard readObjectsForClasses:classes options:options];
	
	if(clipboardContent)
	{
		if(0<[clipboardContent count])
		{
			NSString *str=[clipboardContent objectAtIndex:0];
			if(NULL!=str)
			{
				len = strlen([str UTF8String]);
				// req->length is used when reading just part of the result
				if (req->length > 0 && req->length < len)
					len = req->length;
				
				req->data = (REBYTE*)OS_Make(len+1);
				req->actual = AS_REBLEN(len);
				COPY_STR(req->data, [str UTF8String], len);
				return TRUE;
			}
		}
	}
	return FALSE;
}

REBOOL SetClipboardStringData(REBREQ *req){
	const char *str = cs_cast(req->data);
	NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
	[pasteboard clearContents];
	
	NSString *nsstr = [NSString stringWithUTF8String:str];
	
	if (!nsstr) return FALSE;
	NSArray *arr = [NSArray arrayWithObject:[NSString stringWithUTF8String:str]];
	BOOL res = [pasteboard writeObjects:arr];
	req->actual = req->length;

	return res;
}
