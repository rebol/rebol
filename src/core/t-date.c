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
**  Module:  t-date.c
**  Summary: date datatype
**  Section: datatypes
**  Author:  Carl Sassenrath
**  Notes:
**    Date and time are stored in UTC format with an optional timezone.
**    The zone must be added when a date is exported or imported, but not
**    when date computations are performed.
**
***********************************************************************/

#include "sys-core.h"


/***********************************************************************
**
*/	void Set_Date_UTC(REBVAL *val, REBINT y, REBINT m, REBINT d, REBI64 t, REBINT z)
/*
**		Convert date/time/zone to UTC with zone.
**
***********************************************************************/
{
	// Adjust for zone....
	VAL_YEAR(val)  = y;
	VAL_MONTH(val) = m;
	VAL_DAY(val)   = d;
	VAL_TIME(val)  = t;
	VAL_ZONE(val)  = z;
	VAL_SET(val, REB_DATE);
	if (z) Adjust_Date_Zone(val, TRUE);
}


/***********************************************************************
**
*/	void Set_Date(REBVAL *val, REBOL_DAT *dat)
/*
**		Convert OS date struct to REBOL value struct.
**		NOTE: Input zone is in minutes.
**
***********************************************************************/
{
	VAL_YEAR(val)  = dat->year;
	VAL_MONTH(val) = dat->month;
	VAL_DAY(val)   = dat->day;
	VAL_ZONE(val)  = dat->zone / ZONE_MINS;
	VAL_TIME(val)  = TIME_SEC(dat->time) + dat->nano;
	VAL_SET(val, REB_DATE);
}


/***********************************************************************
**
*/	REBINT CT_Date(REBVAL *a, REBVAL *b, REBINT mode)
/*
***********************************************************************/
{
	REBINT num = Cmp_Date(a, b);
	if (mode >= 2)
		return VAL_DATE(a).bits == VAL_DATE(b).bits && VAL_TIME(a) == VAL_TIME(b);
	if (mode >= 0)  return (num == 0);
	if (mode == -1) return (num >= 0);
	return (num > 0);
}


/***********************************************************************
**
*/	void Emit_Date(REB_MOLD *mold, REBVAL *value)
/*
***********************************************************************/
{
	REBYTE buf[64];
	REBYTE *bp = &buf[0];
	REBINT tz;
	REBYTE dash = GET_MOPT(mold, MOPT_SLASH_DATE) ? '/' : '-';
	REBVAL val = *value;
	value = &val;

	if (
		VAL_MONTH(value) == 0
		|| VAL_MONTH(value) > 12
		|| VAL_DAY(value) == 0
		|| VAL_DAY(value) > 31
	) {
		Append_Bytes(mold->series, "?date?");
		return;
	}

	if (VAL_TIME(value) != NO_TIME) Adjust_Date_Zone(value, FALSE);

//	Punctuation[GET_MOPT(mold, MOPT_COMMA_PT) ? PUNCT_COMMA : PUNCT_DOT]

	bp = Form_Int(bp, (REBINT)VAL_DAY(value));
	*bp++ = dash;
	memcpy(bp, Month_Names[VAL_MONTH(value)-1], 3);
	bp += 3;
	*bp++ = dash;
	bp = Form_Int_Pad(bp, (REBINT)VAL_YEAR(value), 6, -4, '0');
	*bp = 0;

	Append_Bytes(mold->series, buf);

	if (VAL_TIME(value) != NO_TIME) {

		Append_Byte(mold->series, '/');
		Emit_Time(mold, value);

		if (VAL_ZONE(value) != 0) {

			bp = &buf[0];
			tz = VAL_ZONE(value);
			if (tz < 0) {
				*bp++ = '-';
				tz = -tz;
			}
			else
				*bp++ = '+';

			bp = Form_Int(bp, tz/4);
			*bp++ = ':';
			bp = Form_Int_Pad(bp, (tz&3) * 15, 2, 2, '0');
			*bp = 0;

			Append_Bytes(mold->series, buf);
		}
	}
}


/***********************************************************************
**
*/	static REBCNT Month_Length(REBCNT month, REBCNT year)
/*
**		Given a year, determine the number of days in the month.
**		Handles all leap year calculations.
**
***********************************************************************/
{
	if (month != 1)
		return (REBCNT)Month_Lengths[month];

	return (
		((year % 4) == 0) &&		// divisible by four is a leap year
		(
			((year % 100) != 0) ||	// except when divisible by 100
			((year % 400) == 0)		// but not when divisible by 400
		)
	) ? 29 : 28;
}


/***********************************************************************
**
*/	REBCNT Julian_Date(REBDAT date)
/*
**		Given a year, month and day, return the number of days since the
**		beginning of that year.
**
***********************************************************************/
{
	REBCNT days;
	REBCNT i;

	days = 0;

	for (i = 0; i < (date.date.month-1); i++)
		days += Month_Length(i, date.date.year);

	return date.date.day + days;
}


/***********************************************************************
**
*/	REBINT Diff_Date(REBDAT d1, REBDAT d2)
/*
**		Calculate the difference in days between two dates.
**
***********************************************************************/
{
	REBCNT days;
	REBINT sign;
	REBCNT m, y;
	REBDAT tmp;

	if (d1.bits == d2.bits) return 0;

	if (d1.bits < d2.bits) {
		sign = -1;
		tmp = d1;
		d1 = d2;
		d2 = tmp;
	}
	else
		sign = 1;

	// if not same year, calculate days to end of month, year and
	// days in between years plus days in end year
	if (d1.date.year > d2.date.year) {
		days = Month_Length(d2.date.month-1, d2.date.year) - d2.date.day;

		for (m = d2.date.month; m < 12; m++)
			days += Month_Length(m, d2.date.year);

		for (y = d2.date.year + 1; y < d1.date.year; y++) {
			days += (((y % 4) == 0) &&	// divisible by four is a leap year
				(((y % 100) != 0) ||	// except when divisible by 100
				((y % 400) == 0)))	// but not when divisible by 400
				? 366u : 365u;
		}
		return sign * (REBINT)(days + Julian_Date(d1));
	}
	return sign * (REBINT)(Julian_Date(d1) - Julian_Date(d2));
}


/***********************************************************************
**
*/	REBCNT Week_Day(REBDAT date)
/*
**		Return the day of the week for a specific date.
**
***********************************************************************/
{
	REBDAT year1 = {0};
	year1.date.day = 1;
	year1.date.month = 1;

	return ((Diff_Date(date, year1) + 5) % 7) + 1;
}


/***********************************************************************
**
*/	void Normalize_Time(REBI64 *sp, REBINT *dp)
/*
**		Adjust *dp by number of days and set secs to less than a day.
**
***********************************************************************/
{
	REBI64 secs = *sp;
	REBINT day;

	if (secs == NO_TIME) return;

	// how many days worth of seconds do we have
	day = (REBINT)(secs / TIME_IN_DAY);
	secs %= TIME_IN_DAY;

	if (secs < 0L) {
		day--;
		secs += TIME_IN_DAY;
	}

	*dp += day;
	*sp = secs;
}


/***********************************************************************
**
*/	static REBDAT Normalize_Date(REBINT day, REBINT month, REBINT year, REBINT tz)
/*
**		Given a year, month and day, normalize and combine to give a new
**		date value.
**
***********************************************************************/
{
	REBINT d;
	REBDAT dr;

	// First we normalize the month to get the right year
	if (month<0) {
		year-=(-month+11)/12;
		month=11-((-month+11)%12);
	}
	if (month >= 12) {
		year += month / 12;
		month %= 12;
	}

	// Now adjust the days by stepping through each month
	while (day >= (d = (REBINT)Month_Length(month, year))) {
		day -= d;
		if (++month >= 12) {
			month = 0;
			year++;
		}
	}
	while (day < 0) {
		if (month == 0) {
			month = 11;
			year--;
		}
		else
			month--;
		day += (REBINT)Month_Length(month, year);
	}

	if (year < 0 || year > MAX_YEAR) Trap1(RE_TYPE_LIMIT, Get_Type(REB_DATE));

	dr.date.year = year;
	dr.date.month = month+1;
	dr.date.day = day+1;
	dr.date.zone = tz;

	return dr;
}


/***********************************************************************
**
*/	void Adjust_Date_Zone(REBVAL *d, REBFLG to_utc)
/*
**		Adjust date and time for the timezone.
**		The result should be used for output, not stored.
**
***********************************************************************/
{
	REBI64 secs;
	REBCNT n;

	if (VAL_ZONE(d) == 0) return;

	if (VAL_TIME(d) == NO_TIME) {
		VAL_TIME(d) = VAL_ZONE(d) = 0;
		return;
	}

	// (compiler should fold the constant)
	secs = ((i64)VAL_ZONE(d) * ((i64)ZONE_SECS * SEC_SEC));
	if (to_utc) secs = -secs;
	secs += VAL_TIME(d);

	VAL_TIME(d) = (secs + TIME_IN_DAY) % TIME_IN_DAY;

	n = VAL_DAY(d) - 1;

	if (secs < 0) n--;
	else if (secs >= TIME_IN_DAY) n++;
	else return;

	VAL_DATE(d) = Normalize_Date(n, VAL_MONTH(d)-1, VAL_YEAR(d), VAL_ZONE(d));
}


/***********************************************************************
**
*/	void Subtract_Date(REBVAL *d1, REBVAL *d2, REBVAL *result)
/*
**		Called by DIFFERENCE function.
**
***********************************************************************/
{
	REBINT diff;
	REBI64 t1;
	REBI64 t2;

	diff  = Diff_Date(VAL_DATE(d1), VAL_DATE(d2));
	if (abs(diff) > (((1U << 31) - 1) / SECS_IN_DAY)) Trap0(RE_OVERFLOW);

	t1 = VAL_TIME(d1);
	if (t1 == NO_TIME) t1 = 0L;
	t2 = VAL_TIME(d2);
	if (t2 == NO_TIME) t2 = 0L;

	VAL_SET(result, REB_TIME);
	VAL_TIME(result) = (t1 - t2) + ((REBI64)diff * TIME_IN_DAY);
}


/***********************************************************************
**
*/	REBINT Cmp_Date(REBVAL *d1, REBVAL *d2)
/*
***********************************************************************/
{
	REBINT diff;

	diff  = Diff_Date(VAL_DATE(d1), VAL_DATE(d2));
	if (diff == 0) diff = Cmp_Time(d1, d2);

	return diff;
}


/***********************************************************************
**
*/	REBFLG MT_Date(REBVAL *val, REBVAL *arg, REBCNT type)
/*
**		Given a block of values, construct a date datatype.
**
***********************************************************************/
{
	REBI64 secs = NO_TIME;
	REBINT tz = 0;
	REBDAT date;
	REBCNT year, month, day;

	if (IS_DATE(arg)) {
		*val = *arg;
		return TRUE;
	}

	if (!IS_INTEGER(arg)) return FALSE;
	day = Int32s(arg++, 1);
	if (!IS_INTEGER(arg)) return FALSE;
	month = Int32s(arg++, 1);
	if (!IS_INTEGER(arg)) return FALSE;
	if (day > 99) {
		year = day;
		day = Int32s(arg++, 1);
	} else
		year = Int32s(arg++, 0);

	if (month < 1 || month > 12) return FALSE;

	if (year > MAX_YEAR || day < 1 || day > (REBINT)(Month_Lengths[month-1])) return FALSE;

	// Check February for leap year or century:
	if (month == 2 && day == 29) {
		if (((year % 4) != 0) ||		// not leap year
			((year % 100) == 0 && 		// century?
			(year % 400) != 0)) return FALSE; // not leap century
	}

	day--;
	month--;

	if (IS_TIME(arg)) {
		secs = VAL_TIME(arg);
		arg++;
	}

	if (IS_TIME(arg)) {
		tz = (REBINT)(VAL_TIME(arg) / (ZONE_MINS * MIN_SEC));
		if (tz < -MAX_ZONE || tz > MAX_ZONE) Trap_Range(arg);
		arg++;
	}

	if (!IS_END(arg)) return FALSE;

	Normalize_Time(&secs, &day);
	date = Normalize_Date(day, month, year, tz);

	VAL_SET(val, REB_DATE);
	VAL_DATE(val) = date;
	VAL_TIME(val) = secs;
	Adjust_Date_Zone(val, TRUE);

	return TRUE;
}


/***********************************************************************
**
*/	REBINT PD_Date(REBPVS *pvs)
/*
***********************************************************************/
{
	REBVAL *data = pvs->value;
	REBVAL *arg = pvs->select;
	REBVAL *val = pvs->setval;
	REBINT i;
	REBINT n;
	REBI64 secs;
	REBINT tz;
	REBDAT date;
	REBINT day, month, year;
	REBINT num;
	REBVAL dat;
	REB_TIMEF time;

	// !zone! - adjust date by zone (unless /utc given)

	if (IS_WORD(arg)) {
		//!!! change this to an array!?
		switch (VAL_WORD_CANON(arg)) {
		case SYM_YEAR:	i = 0; break;
		case SYM_MONTH:	i = 1; break;
		case SYM_DAY:	i = 2; break;
		case SYM_TIME:	i = 3; break;
		case SYM_ZONE:	i = 4; break;
		case SYM_DATE:	i = 5; break;
		case SYM_WEEKDAY: i = 6; break;
		case SYM_JULIAN:
		case SYM_YEARDAY: i = 7; break;
		case SYM_UTC:    i = 8; break;
		case SYM_HOUR:	 i = 9; break;
		case SYM_MINUTE: i = 10; break;
		case SYM_SECOND: i = 11; break;
		default: return PE_BAD_SELECT;
		}
	}
	else if (IS_INTEGER(arg)) {
		i = Int32(arg) - 1;
		if (i < 0 || i > 8) return PE_BAD_SELECT;
	}
	else
		return PE_BAD_SELECT;

	if (IS_DATE(data)) {
		dat = *data; // recode!
		data = &dat;
		if (i != 8) Adjust_Date_Zone(data, FALSE); // adjust for timezone
		date  = VAL_DATE(data);
		day   = VAL_DAY(data) - 1;
		month = VAL_MONTH(data) - 1;
		year  = VAL_YEAR(data);
		secs  = VAL_TIME(data);
		tz    = VAL_ZONE(data);
		if (i > 8) Split_Time(secs, &time);
	}

	if (val == 0) {
		val = pvs->store;
		switch(i) {
		case 0:
			num = year;
			break;
		case 1:
			num = month + 1;
			break;
		case 2:
			num = day + 1;
			break;
		case 3:
			if (secs == NO_TIME) return PE_NONE;
			*val = *data;
			VAL_SET(val, REB_TIME);
			return PE_USE;
		case 4:
			if (secs == NO_TIME) return PE_NONE;
			*val = *data;
			VAL_TIME(val) = (i64)tz * ZONE_MINS * MIN_SEC;
			VAL_SET(val, REB_TIME);
			return PE_USE;
		case 5:
			// date
			*val = *data;
			VAL_TIME(val) = NO_TIME;
			VAL_ZONE(val) = 0;
			return PE_USE;
		case 6:
			// weekday
			num = Week_Day(date);
			break;
		case 7:
			// yearday
			num = (REBINT)Julian_Date(date);
			break;
		case 8:
			// utc
			*val = *data;
			VAL_ZONE(val) = 0;
			return PE_USE;
		case 9:
			num = time.h;
			break;
		case 10:
			num = time.m;
			break;
		case 11:
			if (time.n == 0) num = time.s;
			else {
				SET_DECIMAL(val, (REBDEC)time.s + (time.n * NANO));
				return PE_USE;
			}
			break;

		default:
			return PE_NONE;
		}
		SET_INTEGER(val, num);
		return PE_USE;

	} else {

		if (IS_INTEGER(val) || IS_DECIMAL(val)) n = Int32s(val, 0);
		else if (IS_NONE(val)) n = 0;
		else if (IS_TIME(val) && (i == 3 || i == 4));
		else if (IS_DATE(val) && (i == 3 || i == 5));
		else return PE_BAD_SET_TYPE;

		switch(i) {
		case 0:
			year = n;
			break;
		case 1:
			month = n - 1;
			break;
		case 2:
			day = n - 1;
			break;
		case 3:
			// time
			if (IS_NONE(val)) {
				secs = NO_TIME;
				tz = 0;
				break;
			}
			else if (IS_TIME(val) || IS_DATE(val))
				secs = VAL_TIME(val);
			else if (IS_INTEGER(val))
				secs = n * SEC_SEC;
			else if (IS_DECIMAL(val))
				secs = DEC_TO_SECS(VAL_DECIMAL(val));
			else return PE_BAD_SET_TYPE;
			break;
		case 4:
			// zone
			if (IS_TIME(val)) tz = (REBINT)(VAL_TIME(val) / (ZONE_MINS * MIN_SEC));
			else if (IS_DATE(val)) tz = VAL_ZONE(val);
			else tz = n * (60 / ZONE_MINS);
			if (tz > MAX_ZONE || tz < -MAX_ZONE) return PE_BAD_RANGE;
			break;
		case 5:
			// date
			if (!IS_DATE(val)) return PE_BAD_SET_TYPE;
			date = VAL_DATE(val);
			goto setDate;
		case 9:
			time.h = n;
			secs = Join_Time(&time);
			break;
		case 10:
			time.m = n;
			secs = Join_Time(&time);
			break;
		case 11:
			if (IS_INTEGER(val)) {
				time.s = n;
				time.n = 0;
			}
			else {
				//if (f < 0.0) Trap_Range(val);
				time.s = (REBINT)VAL_DECIMAL(val);
				time.n = (REBINT)((VAL_DECIMAL(val) - time.s) * SEC_SEC);
			}
			secs = Join_Time(&time);
			break;

		default:
			return PE_BAD_SET;
		}

		Normalize_Time(&secs, &day);
		date = Normalize_Date(day, month, year, tz);

setDate:
		data = pvs->value;
		VAL_SET(data, REB_DATE);
		VAL_DATE(data) = date;
		VAL_TIME(data) = secs;
		Adjust_Date_Zone(data, TRUE);

		return PE_USE;
	}
}


/***********************************************************************
**
*/	REBTYPE(Date)
/*
***********************************************************************/
{
	REBI64	secs;
	REBINT  tz;
	REBDAT	date;
	REBINT	day, month, year;
	REBVAL	*val;
	REBVAL	*arg;
	REBINT	num;

	val = D_ARG(1);
	if (IS_DATE(val)) {
		date  = VAL_DATE(val);
		day   = VAL_DAY(val) - 1;
		month = VAL_MONTH(val) - 1;
		year  = VAL_YEAR(val);
		tz    = VAL_ZONE(val);
		secs  = VAL_TIME(val);
	}

	if (DS_ARGC > 1) arg = D_ARG(2);

	if (IS_BINARY_ACT(action)) {
		REBINT	type = VAL_TYPE(arg);

		if (type == REB_DATE) {
			if (action == A_SUBTRACT) {
				num = Diff_Date(date, VAL_DATE(arg));
				goto ret_int;
			}
		}
		else if (type == REB_TIME) {
			if (secs == NO_TIME) secs = 0;
			if (action == A_ADD) {
				secs += VAL_TIME(arg);
				goto fixTime;
			}
			if (action == A_SUBTRACT) {
				secs -= VAL_TIME(arg);
				goto fixTime;
			}
		}
		else if (type == REB_INTEGER) {
			num = Int32(arg);
			if (action == A_ADD) {
				day += num;
				goto fixDate;
			}
			if (action == A_SUBTRACT) {
				day -= num;
				goto fixDate;
			}
		}
		else if (type == REB_DECIMAL) {
			REBDEC dec = Dec64(arg);
			if (secs == NO_TIME) secs = 0;
			if (action == A_ADD) {
				secs += (REBI64)(dec * TIME_IN_DAY);
				goto fixTime;
			}
			if (action == A_SUBTRACT) {
				secs -= (REBI64)(dec * TIME_IN_DAY);
				goto fixTime;
			}
		}
	}
	else {
		switch(action) {
		case A_EVENQ: day = ~day;
		case A_ODDQ: DECIDE((day & 1) == 0);

		case A_PICK:
			Pick_Path(val, arg, 0);
			return R_TOS;

///		case A_POKE:
///			Pick_Path(val, arg, D_ARG(3));
///			return R_ARG3;

		case A_MAKE:
		case A_TO:
			if (IS_DATE(arg)) {
				val = arg;
				goto ret_val;
			}
			if (IS_STRING(arg)) {
				REBYTE *bp;
				REBCNT len;
				// 30-September-10000/12:34:56.123456789AM/12:34
				bp = Qualify_String(arg, 45, &len, FALSE); // can trap, ret diff str
				if (Scan_Date(bp, len, D_RET)) return R_RET;
			}
			else if (ANY_BLOCK(arg) && VAL_BLK_LEN(arg) >= 3) {
				if (MT_Date(D_RET, VAL_BLK_DATA(arg), REB_DATE)) {
					return R_RET;
				}
			}
//			else if (IS_NONE(arg)) {
//				secs = nsec = day = month = year = tz = 0;
//				goto fixTime; 
//			}
			Trap_Make(REB_DATE, arg);

		case A_RANDOM:	//!!! needs further definition ?  random/zero
			if (D_REF(2)) {
				// Note that nsecs not set often for dates (requires /precise)
				Set_Random(((REBI64)year << 48) + ((REBI64)Julian_Date(date) << 32) + secs);
				return R_UNSET;
			}
			if (year == 0) break;
			num = D_REF(3); // secure
			year = (REBCNT)Random_Range(year, num);
			month = (REBCNT)Random_Range(12, num);
			day = (REBCNT)Random_Range(31, num);
			if (secs != NO_TIME)
				secs = Random_Range(TIME_IN_DAY, num);
			goto fixDate;

		case A_ABSOLUTE:
			goto setDate;
		}
	}
	Trap_Action(REB_DATE, action);

fixTime:
	Normalize_Time(&secs, &day);

fixDate:
	date = Normalize_Date(day, month, year, tz);

setDate:
	VAL_SET(DS_RETURN, REB_DATE);
	VAL_DATE(DS_RETURN) = date;
	VAL_TIME(DS_RETURN) = secs;
	return R_RET;

ret_int:
	DS_RET_INT(num);
	return R_RET;

ret_val:
	*DS_RETURN = *val;
	return R_RET;

is_false:
	return R_FALSE;

is_true:
	return R_TRUE;
}
