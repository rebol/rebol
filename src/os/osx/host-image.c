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
**  Title: osx native codecs functionality (interface)
**  Author: Oldes
**  Purpose: 
**	Related: 
**
***********************************************************************/
#include "reb-host.h"
#include "sys-codecs.h"

//
//#include "host-lib.h"

//#include "reb-codec.h"

/***********************************************************************
**
*/	void OS_Load_Image(const char *uri, REBCNT frame, REBCDI *codi)
/*
**		Decodes image from specified file. If its multi-image type, it's possible
**		to specify which frame to load.
**
***********************************************************************/
{
	codi->error = DecodeImageFromFile(uri, frame, codi);
}

/***********************************************************************
**
*/	void OS_Save_Image(const char *uri, REBCDI *codi)
/*
**		Encodes image to specified file.
**
***********************************************************************/
{
	codi->error = EncodeImageToFile(uri, codi);
}

/***********************************************************************
**
*/	void OS_Release_Codecs(void)
/*
**		Release resources.
**
***********************************************************************/
{
	// nothing
}

