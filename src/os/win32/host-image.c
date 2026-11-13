/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**	Copyright 2019-2025 Oldes
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
**  Title: Windows native codecs functionality (interface)
**  Author: Oldes
**  Purpose: 
**	Related: 
**
***********************************************************************/
#include "reb-host.h"
#include "sys-codecs.h"
#include "sys-utils.h"

//
//#include "host-lib.h"

//#include "reb-codec.h"

/***********************************************************************
**
*/	OS_API void OS_Load_Image(REBUNI *uri, REBCNT frame, REBCDI *codi)
/*
**		Decodes image from specified file. If its multi-image type, it's possible
**		to specify which frame to load.
**
***********************************************************************/
{
	DecodeImageFromFile((PCWSTR*)uri, frame, codi);
}

/***********************************************************************
**
*/	OS_API void OS_Save_Image(REBUNI *uri, REBCDI *codi)
/*
**		Encodes image to specified file.
**
***********************************************************************/
{
	EncodeImageToFile((PCWSTR*)uri, codi);
}

/***********************************************************************
**
*/	OS_API void OS_Release_Codecs()
/*
**		Release WIC resources.
**
***********************************************************************/
{
	codecs_fini();
}

