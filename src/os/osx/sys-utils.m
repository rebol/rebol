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
**  Title: osx objective-c common code
**  Author: Oldes
**  Purpose: various common osx functions
**
************************************************************************/

#include "sys-utils.h"

CFURLRef urlFromCString(const char* cString){
	NSString *myNSString = [NSString stringWithUTF8String:cString];
	NSString *path = [myNSString stringByExpandingTildeInPath];
	return CFURLCreateWithFileSystemPath(NULL, (CFStringRef)path, 0, false);
}
