/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2025 Rebol Open Source Contributors
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
**  Summary: Defines function pointer types used by runtime extensions.
**  Module:  reb-ext-handler.h
**  Author:  Oldes
**  Notes:
**
***********************************************************************/


/***********************************************************************
**  De/Compression runtime extension function pointer types.
**  Used by the RL_Register_Compress_Method function to register
**  compression methods from native extensions.
***********************************************************************/
typedef int (*COMPRESS_FUNC)(
    const REBYTE *input,
    REBLEN        in_len,
	REBCNT        level,
    REBSER      **output,
	REBINT       *error
);

typedef int (*DECOMPRESS_FUNC)(
    const REBYTE *input,
	REBLEN        in_len,
	REBLEN        out_len,
    REBSER      **output,
	REBINT       *error
);

