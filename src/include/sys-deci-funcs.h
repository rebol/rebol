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
**  Summary: Deci Datatype Functions
**  Module:  sys-deci-funcs.h
**  Notes:
**
***********************************************************************/

/* unary operators - logic */
REBFLG deci_is_zero (const deci a);

/* unary operators - deci */
deci deci_abs (deci a);
deci deci_negate (deci a);

/* binary operators - logic */
REBFLG deci_is_equal (deci a, deci b);
REBFLG deci_is_lesser_or_equal (deci a, deci b);
REBFLG deci_is_same (deci a, deci b);

/* binary operators - deci */
deci deci_add (deci a, deci b);
deci deci_subtract (deci a, deci b);
deci deci_multiply (const deci a, const deci b);
deci deci_divide (deci a, deci b);
deci deci_mod (deci a, deci b);

/* conversion to deci */
deci int_to_deci (REBI64 a);
deci decimal_to_deci (REBDEC a);
deci string_to_deci (REBYTE *s, REBYTE **endptr);
deci binary_to_deci(REBYTE *s);

/* conversion to other datatypes */
REBI64 deci_to_int (const deci a);
REBDEC deci_to_decimal (const deci a);
REBINT deci_to_string(REBYTE *string, const deci a, const REBYTE symbol, const REBYTE point);
REBYTE *deci_to_binary(REBYTE binary[12], const deci a);

/* math functions */
deci deci_ldexp (deci a, REBINT e);
deci deci_truncate (deci a, deci b);
deci deci_away (deci a, deci b);
deci deci_floor (deci a, deci b);
deci deci_ceil (deci a, deci b);
deci deci_half_even (deci a, deci b);
deci deci_half_away (deci a, deci b);
deci deci_half_truncate (deci a, deci b);
deci deci_half_ceil (deci a, deci b);
deci deci_half_floor (deci a, deci b);
deci deci_sign (deci a);
