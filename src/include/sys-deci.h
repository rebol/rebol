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
**  Summary: Deci Datatype
**  Module:  sys-deci.h
**  Notes:
**
***********************************************************************/

typedef struct deci {
    unsigned m0:32;	/* significand, lowest part */
	unsigned m1:32;	/* significand, continuation */
    unsigned m2:23;	/* significand, highest part */
	unsigned s:1;	/* sign, 0 means nonnegative, 1 means nonpositive */
	int e:8;		/* exponent */
} deci;

