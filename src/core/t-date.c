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

static const REBI64 DAYS_OF_JAN_1ST_1970 = 719468; // number of days for 1st January 1970


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
	REBOOL iso = GET_MOPT(mold, MOPT_MOLD_ALL);
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
	if (iso) {
		// use ISO8601 output
		bp = Form_Int_Pad(bp, (REBINT)VAL_YEAR(value), 6, -4, '0');
		*bp = '-';
		bp = Form_Int_Pad(++bp, (REBINT)VAL_MONTH(value), 2, -2, '0');
		*bp = '-';
		bp = Form_Int_Pad(++bp, (REBINT)VAL_DAY(value), 2, -2, '0');
		*bp = 0;
	} else {
		// use standard Rebol output
		bp = Form_Int(bp, (REBINT)VAL_DAY(value));
		*bp++ = dash;
		memcpy(bp, Month_Names[VAL_MONTH(value) - 1], 3);
		bp += 3;
		*bp++ = dash;
		bp = Form_Int_Pad(bp, (REBINT)VAL_YEAR(value), 6, -4, '0');
		*bp = 0;
	}
	Append_Bytes(mold->series, cs_cast(buf));

	if (VAL_TIME(value) != NO_TIME) {

		Append_Byte(mold->series, iso ? 'T' : '/');
		Emit_Time(mold, value, iso);

		if (VAL_ZONE(value) != 0) {

			bp = &buf[0];
			tz = VAL_ZONE(value);
			if (tz < 0) {
				*bp++ = '-';
				tz = -tz;
			}
			else
				*bp++ = '+';

			if(iso) {
				bp = Form_Int_Pad(bp, tz / 4, 2, -2, '0');
			} else {
				bp = Form_Int(bp, tz / 4);
			}
			*bp++ = ':';
			bp = Form_Int_Pad(bp, ((REBI64)tz & 3) * 15, 2, 2, '0');
			*bp = 0;

			Append_Bytes(mold->series, cs_cast(buf));
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
*/	REBI64 Days_Of_Date(REBINT day, REBINT month, REBINT year )
/*
**		Return number of days from given date parts
**
***********************************************************************/
{
	REBI64 m = (month + 9) % 12;
	REBI64 y = year - (m / 10);
	return (REBI64)(365 * y + (y / 4) - (y / 100) + (y / 400) + ((m * 306 + 5) / 10) + ((REBI64)day - 1));
}

/***********************************************************************
**
*/	void Date_Of_Days(REBI64 days, REBDAT *date)
/*
**		Return number of days from given date parts
**
***********************************************************************/
{
	REBI64 dd, y, mi;
	
	y = ((10000 * days) + 14780) / 3652425;
	dd = days - (365 * y + (y / 4) - (y / 100) + (y / 400));
	mi = (100 * dd + 52) / 3060;
	date->date.month = (mi + 2) % 12 + 1;
	date->date.year = y + ((mi + 2) / 12);
	date->date.day = dd - ((mi * 306 + 5) / 10) + 1;
}


/***********************************************************************
**
*/	static REBI64 Days_Of_Jan_1st(REBINT year)
/*
**		Return number of days for 1st January of given year
**
***********************************************************************/
{
	return Days_Of_Date(1, 1, year);
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
*/	REBCNT Date_To_Timestamp(REBVAL *date)
/*
**		Return the unix time stamp for a specific date value.
**
***********************************************************************/
{
	REBDAT d = VAL_DATE(date);
	REBI64 epoch = (Days_Of_Date(d.date.day, d.date.month, d.date.year) - DAYS_OF_JAN_1ST_1970) * SECS_IN_DAY;
	REBI64 time = VAL_TIME(date);
	if (time == NO_TIME) time = 0;
	return epoch + ((time + 500000000) / SEC_SEC);
}

/***********************************************************************
**
*/	void Timestamp_To_Date(REBVAL *date, REBI64 epoch)
/*
**		Set Rebol date from the unix time stamp epoch.
**
***********************************************************************/
{
	REBI64 days = (epoch / SECS_IN_DAY) + DAYS_OF_JAN_1ST_1970;

	VAL_SET(date, REB_DATE);
	Date_Of_Days(days, &VAL_DATE(date));
	VAL_TIME(date) = TIME_SEC((epoch % 86400));
	VAL_ZONE(date) = 0;
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
	day = cast(REBINT, secs / TIME_IN_DAY);
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
*/	REBDEC Gregorian_To_Julian_Date(REBDAT date, REB_TIMEF time)
/*
**		Given a Gregorian date and time, return Julian date
**		https://www.typecalendar.com/julian-date
**		https://pdc.ro.nu/jd-code.html
**
***********************************************************************/
{
	long jd;
	long d = date.date.day-1;
	long m = date.date.month-1;
	long y = date.date.year;

	//printf("%li-%li-%li %i:%i:%i\n", d, m, y, time.h, time.m, time.s);
	if (time.h <= 12) {
		d--;
		time.h += 12;
	} else {
		time.h -= 12;
	}
		date = Normalize_Date(d,m,y,0);
		d = date.date.day-1;
		m = date.date.month;
		y = date.date.year;
	//printf("%li-%li-%li %i:%i:%i\n", d, m, y, time.h, time.m, time.s);

	y += 8000;
	if (m < 3) { y--; m += 12; }
	jd  = (y*365) +(y/4) -(y/100) +(y/400) -1200820;
	jd += (m*153+3)/5-92;
    jd += d ;

    return (REBDEC)jd + ((double)time.h / 24.0 + (double)time.m / 1440.0 + (double)time.s / 86400.0);
}

/***********************************************************************
**
*/	void Julian_To_Gregorian_Date(REBDEC julian, REBINT *day, REBINT *month, REBINT *year, REBI64 *secs)
/*
**		Converts a Julian date to a Gregorian date and time.
**		https://www.typecalendar.com/julian-date
**		NOTE: month and day are 1-based!
**
***********************************************************************/
{
	REBINT z, w, x, a, b, c, d, e, f;
	double fp, ip;
	REBI64 h, m, s; 

	fp = modf(julian, &ip);           // The fractional part

	z = (REBINT)ip;                   // The integral part of the Julian day
	w = (z - 1867216.25) / 36524.25;  // The value used in the calculation to determine the leap years. It represents the number of leap years since 4713 BC
	x = w / 4;                        // The number of 4-year cycles (leap year groups) that have passed since the year 4713 BC.
	a = z + 1 + w - x;                // The adjusted Julian day number, taking into account leap years.
	b = a + 1524;                     // The Julian day number shifted by 122.1 to provide a suitable starting point for subsequent calculations.
	c = (b - 122.1) / 365.25;         // The estimated year of the Gregorian calendar.
	d = 365.25 * c;                   // The number of days that have passed in the year, excluding the current month.
	e = (b - d) / 30.6001;            // The month number.
	f = 30.6001 * e;                  // The number of days that have passed in the current month, excluding the current day.

	*day = b - d - f + fp;
	*month = (e < 14) ? e - 1 : e - 13;
	*year = (*month > 2) ? c - 4716 : c - 4715;


	fp *= 24;
	h = (REBI64)fp;
	fp = (fp - h) * 60;
	m = (REBI64)fp;
	fp = (fp - m) * 60;
	s = (REBI64)round(fp);

	//printf("--- %i-%i-%i %lli:%lli:%lli\n", *day, *month, *year, h, m, s);

	*secs = (h+12) * HR_SEC + m * MIN_SEC + s * SEC_SEC;
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
		if (IS_TIME(++arg)) {
			// make date! [1-1-2000 100:0]
			// we must get date parts here so can be used
			// for time normalization later
			day   = VAL_DAY(val) - 1;
			month = VAL_MONTH(val) - 1;
			year  = VAL_YEAR(val);
			goto set_time;
		}
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

	if (year > MAX_YEAR || day < 1 || day > (REBCNT)(Month_Lengths[month-1])) return FALSE;

	// Check February for leap year or century:
	if (month == 2 && day == 29) {
		if (((year % 4) != 0) ||		// not leap year
			((year % 100) == 0 && 		// century?
			(year % 400) != 0)) return FALSE; // not leap century
	}

	day--;
	month--;

set_time:
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

	Normalize_Time(&secs, (REBINT*)&day);
	date = Normalize_Date(day, month, year, tz);

	VAL_SET(val, REB_DATE);
	VAL_DATE(val) = date;
	VAL_TIME(val) = secs;
	Adjust_Date_Zone(val, TRUE);

	return TRUE;
}

/***********************************************************************
**
*/	static REBOOL Query_Date_Field(REBVAL *data, REBVAL *select, REBVAL *ret)
/*
**		Set a value with date data according specified mode
**
***********************************************************************/
{
	REBPVS pvs;
	pvs.value = data;
	pvs.select = select;
	pvs.setval = 0;
	pvs.store = ret;

	return (PE_BAD_SELECT > PD_Date(&pvs));
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
	REBINT sym = 0;
	REBINT n = 0;
	REBI64 secs;
	REBINT tz, tzp;
	REBDAT date;
	REBINT day, month, year;
	REBINT num;
	REBVAL dat;
	REB_TIMEF time = {0,0,0,0};
	REBOOL asTimezone = FALSE;

	if (!IS_DATE(data)) return PE_BAD_ARGUMENT;

	if (IS_WORD(arg) || IS_SET_WORD(arg)) {
		//!!! change this to an array!?
		sym = VAL_WORD_CANON(arg);
	}
	else if (IS_INTEGER(arg)) {
		sym =  SYM_YEAR + Int32(arg) - 1;
	}

	if (sym < SYM_YEAR || sym > SYM_JULIAN) {
		//@@ https://github.com/Oldes/Rebol-issues/issues/1375
		return (val) ? PE_BAD_SELECT : PE_NONE;
	}

	if (sym == SYM_TIMEZONE) {
		asTimezone = TRUE;
		sym = SYM_ZONE;
	}
	
	dat = *data; // recode!
	data = &dat;
	if (sym != SYM_UTC) Adjust_Date_Zone(data, FALSE); // adjust for timezone
	date  = VAL_DATE(data);
	day   = VAL_DAY(data) - 1;
	month = VAL_MONTH(data) - 1;
	year  = VAL_YEAR(data);
	secs  = VAL_TIME(data);
	tz    = VAL_ZONE(data);
	if (sym >= SYM_HOUR && sym <= SYM_SECOND) Split_Time(secs, &time);

	if (val == 0) {

		if (secs == NO_TIME	&& (
			sym == SYM_TIME ||
			(sym >= SYM_HOUR && sym <= SYM_SECOND) ||
			sym == SYM_ZONE)
		) return PE_NONE;
		
		val = pvs->store;
		switch(sym) {
		case SYM_YEAR:
			num = year;
			break;
		case SYM_MONTH:
			num = month + 1;
			break;
		case SYM_DAY:
			num = day + 1;
			break;
		case SYM_TIME:
			*val = *data;
			VAL_SET(val, REB_TIME);
			return PE_USE;
		case SYM_ZONE:
			*val = *data;
			VAL_TIME(val) = (i64)tz * ZONE_MINS * MIN_SEC;
			VAL_SET(val, REB_TIME);
			return PE_USE;
		case SYM_DATE:
			*val = *data;
			VAL_TIME(val) = NO_TIME;
			VAL_ZONE(val) = 0;
			return PE_USE;
		case SYM_WEEKDAY:
			num = Week_Day(date);
			break;
		case SYM_YEARDAY:
			num = (REBINT)Julian_Date(date);
			break;
		case SYM_JULIAN:
			if (secs == NO_TIME) {
				time.h = 12; // Julian date is counted from noon
			} else {
				// Julian date result is in universal time!
				Split_Time(secs - ((i64)tz) * ((i64)ZONE_SECS * SEC_SEC), &time);
			}
			SET_DECIMAL(val, Gregorian_To_Julian_Date(date, time));
			return PE_USE;
		case SYM_UTC:
			*val = *data;
			VAL_ZONE(val) = 0;
			return PE_USE;
		case SYM_HOUR:
			num = time.h;
			break;
		case SYM_MINUTE:
			num = time.m;
			break;
		case SYM_SECOND:
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

		if (IS_INTEGER(val) || IS_DECIMAL(val)) {
			// allow negative time zone
			n = (sym == SYM_ZONE || sym == SYM_YEARDAY) ? Int32(val) : Int32s(val, 0);
		}
		else if (IS_NONE(val)) n = 0;
		else if (IS_TIME(val) && (sym == SYM_TIME || sym == SYM_ZONE));
		else if (IS_DATE(val) && (sym == SYM_TIME || sym == SYM_DATE || sym == SYM_UTC));
		else return PE_BAD_SET_TYPE;

		if (secs == NO_TIME && ((sym >= SYM_HOUR && sym <= SYM_SECOND) || sym == SYM_TIME || sym == SYM_ZONE)) {
			// init time with 0:0:0.0 as we are going to set time related part
			time.h = 0;	time.m = 0;	time.s = 0;	time.n = 0;
		}

		switch(sym) {
		case SYM_YEAR:
			year = n;
			break;
		case SYM_MONTH:
			month = n - 1;
			break;
		case SYM_DAY:
			day = n - 1;
			break;
		case SYM_TIME:
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
		case SYM_ZONE:
			tzp = tz;
			if (IS_TIME(val)) tz = (REBINT)(VAL_TIME(val) / (ZONE_MINS * MIN_SEC));
			else if (IS_DATE(val)) tz = VAL_ZONE(val);
			else tz = n * (60 / ZONE_MINS);
			if (tz > MAX_ZONE || tz < -MAX_ZONE) return PE_BAD_RANGE;
			if (secs == NO_TIME) secs = 0;
			if (asTimezone) {
				secs += (((i64)tz - tzp) * ((i64)ZONE_SECS * SEC_SEC));
			}
			break;
		case SYM_DATE:
			if (!IS_DATE(val)) return PE_BAD_SET_TYPE;
			date = VAL_DATE(val);
			goto setDate;
		case SYM_HOUR:
			time.h = n;
			secs = Join_Time(&time);
			break;
		case SYM_MINUTE:
			time.m = n;
			secs = Join_Time(&time);
			break;
		case SYM_SECOND:
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
		case SYM_UTC:
			if (!IS_DATE(val)) return PE_BAD_SET_TYPE;
			 data = pvs->value;
			*data = *val;
			VAL_ZONE(data) = 0;
			return PE_USE;
		case SYM_YEARDAY:
			if (!IS_INTEGER(val)) return PE_BAD_SET_TYPE;
			Date_Of_Days( Days_Of_Jan_1st(year) + n - 1, &date);
			day   = date.date.day - 1;
			month = date.date.month - 1;
			year  = date.date.year;
			break;
		case SYM_JULIAN:
			if (!IS_DECIMAL(val)) return PE_BAD_SET_TYPE;
			Julian_To_Gregorian_Date(VAL_DECIMAL(val), &day, &month, &year, &secs);
			day--; month--; // The date/time normalization expects 0-based day and month
			tz = 0; // no timezone
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
	REBI64	secs = 0;
	REBDAT	date = {0};
	REBINT	day = 0, month = 0, year = 0, tz = 0;
	REBVAL	*val;
	REBVAL	*arg = D_RET; // using D_RET to silent compiler's warnings (it's redefined if needed)
	REBINT	num;
	REBVAL *spec;

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
		REBINT type = VAL_TYPE(arg);

		if (type == REB_DATE) {
			if (action == A_SUBTRACT) {
				if(!IS_DATE(val)) Trap_Math_Args(VAL_TYPE(val), A_SUBTRACT);
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
			else if (ANY_BLOCK(arg) && VAL_BLK_LEN(arg) >= 1) {
				if (MT_Date(D_RET, VAL_BLK_DATA(arg), REB_DATE)) {
					return R_RET;
				}
			}
			else if (IS_INTEGER(arg)) {
				Timestamp_To_Date(D_RET, VAL_INT64(arg));
				return R_RET;
			}
			else if (IS_DECIMAL(arg)) {
				Julian_To_Gregorian_Date(VAL_DECIMAL(arg) + 2400000.5, &day, &month, &year, &secs);
				day--; month--; // The date/time normalization expects 0-based day and month
				goto fixTime;
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

		case A_REFLECT:
			*D_ARG(3) = *D_ARG(2);
			// continue..
		case A_QUERY:
			spec = Get_System(SYS_STANDARD, STD_DATE_INFO);
			if (!IS_OBJECT(spec)) Trap_Arg(spec);
			if (D_REF(2)) { // query/mode refinement
				REBVAL *field = D_ARG(3);
				if(IS_WORD(field)) {
					switch(VAL_WORD_CANON(field)) {
					case SYM_WORDS:
						Set_Block(D_RET, Get_Object_Words(spec));
						return R_RET;
					case SYM_SPEC:
						return R_ARG1;
					}
					if (!Query_Date_Field(val, field, D_RET))
						Trap_Reflect(VAL_TYPE(val), field); // better error?
				}
				else if (IS_BLOCK(field)) {
					REBVAL *out = D_RET;
					REBSER *values = Make_Block(2 * BLK_LEN(VAL_SERIES(field)));
					REBVAL *word = VAL_BLK_DATA(field);
					for (; NOT_END(word); word++) {
						if (ANY_WORD(word)) {
							if (IS_SET_WORD(word)) {
								// keep the set-word in result
								out = Append_Value(values);
								*out = *word;
								VAL_SET_LINE(out);
							}
							out = Append_Value(values);
							if (!Query_Date_Field(val, word, out))
								Trap1(RE_INVALID_ARG, word);
						}
						else  Trap1(RE_INVALID_ARG, word);
					}
					Set_Series(REB_BLOCK, D_RET, values);
				}
				else {
					Set_Block(D_RET, Get_Object_Words(spec));
				}
			} else {
				REBSER *obj = CLONE_OBJECT(VAL_OBJ_FRAME(spec));
				REBSER *words = VAL_OBJ_WORDS(spec);
				REBVAL *word = BLK_HEAD(words);
				for (num=0; NOT_END(word); word++,num++) {
					Query_Date_Field(val, word, OFV(obj, num));
				}
				SET_OBJECT(D_RET, obj);
			}
			return R_RET;
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
