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
**  Summary: Decimal conversion wrapper
**  Author:  Carl Sassenrath
**  Notes:
**
***********************************************************************/

#ifdef HAS_ECVT
#define ECVT(value, ndig, dec, sign) ecvt(value, ndig, dec, sign)
#else
#ifdef HAS_LONG_DOUBLE
REBYTE *Ecvt(REBDEC value, int ndig, REBINT *dec, REBINT *sign);
#define ECVT(value, ndig, dec, sign) Ecvt(value, ndig, dec, sign)
#else
#error Need to emulate long double for ecvt()
#endif
#endif
