/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2023 Rebol Open Source Contributors
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
**  Module:  n-oid.c
**  Summary: native function for object identifiers (OID)
**  Section: natives
**  Author:  Oldes
**  Notes:   Based on code from mbedTLS
**
***********************************************************************/

#include <stdio.h>
#include "sys-core.h"

/***********************************************************************
**
*/	REBNATIVE(form_oid)
/*
//  form-oid: native [
//		"Return the x.y.z.... style numeric string for the given OID"
//		oid [binary!]
//  ]
***********************************************************************/
{
	REBVAL *val_oid = D_ARG(1);
	REBSER *out;
	REBYTE *oid;
	REBCNT  len, value;
	int ret = 0;
	size_t i, n;
	char *p;

	oid = VAL_BIN_AT(val_oid);
	len = VAL_LEN(val_oid);
	out = Make_Binary(3 * len); // len * 3 should be enough to hold the result
	p = SERIES_DATA(out);
	n = SERIES_AVAIL(out);

	if (len > 0) {
		ret = snprintf(p, n, "%d.%d", oid[0] / 40, oid[0] % 40);
		//Minimal series is 8 bytes so it must be enough to hold first 2 integers
		//if (ret >= n) {
		//	Extend_Series(out, ret - n + 1); // may reallocate p!
		//	p = SERIES_DATA(out) + SERIES_TAIL(out);
		//	n = SERIES_REST(out) - SERIES_TAIL(out);
		//	ret = snprintf(p, n, "%d.%d", oid[0] / 40, oid[0] % 40);
		//}
		if (ret < 0) return R_ARG1; // error!
		SERIES_TAIL(out) += ret;
		n -= (size_t)ret;
		p += (size_t)ret;
	}

	value = 0;
	for (i = 1; i < len; i++) {
		/* Prevent overflow in value. */
		if (((value << 7) >> 7) != value)
			return R_ARG1; // error!

		value <<= 7;
		value += oid[i] & 0x7F;

		if (!(oid[i] & 0x80)) {
			/* Last byte */
			ret = snprintf(p, n, ".%u", value);
			if (ret < 0) return R_ARG1; // error!
			if ((size_t)ret >= n) {
				Extend_Series(out, (REBLEN)(ret - n + 1)); // may reallocate p!
				p = SERIES_DATA(out) + SERIES_TAIL(out);
				n = SERIES_AVAIL(out);
				ret = snprintf(p, n, ".%u", value);
				if (ret < 0) return R_ARG1; // error!
			}			
			SERIES_TAIL(out) += ret;
			n -= (size_t)ret;
			p += (size_t)ret;
			value = 0;
		}
	}
	SET_STRING(D_RET, out);
	return R_RET;
}