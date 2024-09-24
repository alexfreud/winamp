#include "main.h"
#include "RFCDate.h"
#include <strsafe.h>

void MakeDateString(__time64_t convertTime, wchar_t *date_str, size_t len)
{
	SYSTEMTIME sysTime = {0};
	tm *newtime = _localtime64(&convertTime);

	sysTime.wYear = newtime->tm_year + 1900;
	sysTime.wMonth = newtime->tm_mon + 1;
	sysTime.wDayOfWeek = newtime->tm_wday;
	sysTime.wDay = newtime->tm_mday;
	sysTime.wHour = newtime->tm_hour;
	sysTime.wMinute = newtime->tm_min;
	sysTime.wSecond = newtime->tm_sec;

	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &sysTime, NULL, date_str, (int)len);

}

void MakeRFCDateString(__time64_t convertTime, wchar_t *data_str, size_t len)
{
	SYSTEMTIME sysTime = {0};
	tm *newtime = _gmtime64(&convertTime);

	sysTime.wYear = newtime->tm_year + 1900;
	sysTime.wMonth = newtime->tm_mon + 1;
	sysTime.wDayOfWeek = newtime->tm_wday;
	sysTime.wDay = newtime->tm_mday;
	sysTime.wHour = newtime->tm_hour;
	sysTime.wMinute = newtime->tm_min;
	sysTime.wSecond = newtime->tm_sec;

	wchar_t rfcDate[64] = {0}, rfcTime[64] = {0};
	GetDateFormat(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), 0, &sysTime, L"ddd',' d MMM yyyy ", rfcDate, 64);
	GetTimeFormat(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), 0, &sysTime, L"HH':'mm':'ss 'GMT'", rfcTime, 64);
	StringCchPrintf(data_str,len,L"%s%s",rfcDate,rfcTime);
}

static int getMonthOfYear(const wchar_t *str);
static int validateTime(struct tm *tmloc, int gmoffset);
static const wchar_t *getNextField(const wchar_t *pos);
static int getGMOffset(const wchar_t *str);

enum
{
    DAY_OF_MONTH = 0,
    MONTH_OF_YEAR,
    YEAR,
    TIME,
    TIMEZONE,
    DATE_END
};

__time64_t MakeRFCDate(const wchar_t *date)
{
	__time64_t result = 0;
	const wchar_t *pos = date;
	tm tmloc = {0};
	const wchar_t *strmin;
	const wchar_t *strsec;
	int gmoffset = 1;
	int state = DAY_OF_MONTH;
	tzset();
	/* skip weekday if present */
	while (pos && *pos && !iswdigit(*pos)) pos++;

	do
	{
		switch (state)
		{
		case DAY_OF_MONTH:
			tmloc.tm_mday = _wtoi(pos);
			break;
		case MONTH_OF_YEAR:
			tmloc.tm_mon = getMonthOfYear(pos);
			break;
		case YEAR:
			{
				/* TODO: we're only accepting 4-digit dates...*/
				const wchar_t *test = pos; int numDigits = 0;
				while (iswdigit(*test) && *test) { test++; numDigits++; }
				if (numDigits == 2)  // let's hope we never have 2 digit years!
					tmloc.tm_year = _wtoi(pos) + 100; // assume 2 digit years are 20xx
				else
					tmloc.tm_year = _wtoi(pos) - 1900;
			}
			break;
		case TIME:
			strmin = wcschr(pos, L':');
			strsec = strmin ? wcschr(strmin + 1, L':') : 0;

			tmloc.tm_hour = _wtoi(pos);
			if (!strmin)	return _time64(0);
			tmloc.tm_min = _wtoi(strmin + 1);
			if (!strsec)	return _time64(0);
			tmloc.tm_sec = _wtoi(strsec + 1);
			break;
		case TIMEZONE:
			gmoffset = getGMOffset(pos);
			break;
		case DATE_END:
			pos = 0;
			break;
		}

		state++;
	}
	while ((pos = getNextField(pos)));

	tmloc.tm_isdst = 0; //_daylight;

	if (validateTime(&tmloc, gmoffset))
	{
		result = _mktime64(&tmloc) - _timezone;
		//if (_daylight)
	}

	return result;
}

const wchar_t *getNextField(const wchar_t *pos)
{
	if (!pos)
		return 0;
	while (pos && *pos && !iswspace(*pos)) pos++;
	while (pos && *pos && iswspace(*pos)) pos++;

	return ((pos && *pos) ? pos : 0);
}

int validateTime(struct tm *tmloc, int gmoffset)
{
	int result = 1;

	if (tmloc->tm_mday < 1 || tmloc->tm_mday > 31 ||
	    tmloc->tm_mon < 0 || tmloc->tm_mon > 11 ||
	    tmloc->tm_year < 0 || tmloc->tm_year > 2000 ||
	    tmloc->tm_hour < 0 || tmloc->tm_hour > 23 ||
	    tmloc->tm_min < 0 || tmloc->tm_min > 59 ||
	    tmloc->tm_sec < 0 || tmloc->tm_sec > 59 ||
	    gmoffset == 1)
		result = 0;

	return result;
}

int getMonthOfYear(const wchar_t *str)
{
	int mon = -1;
	/* This is not the most efficient way to determine
	the month (we could use integer comparisons, for instance)
	but I don't think this will be a performance bottleneck :)
	*/

	if (!wcsnicmp(str, L"Jan", 3))
		mon = 0;
	else if (!wcsnicmp(str, L"Feb", 3))
		mon = 1;
	else if (!wcsnicmp(str, L"Mar", 3))
		mon = 2;
	else if (!wcsnicmp(str, L"Apr", 3))
		mon = 3;
	else if (!wcsnicmp(str, L"May", 3))
		mon = 4;
	else if (!wcsnicmp(str, L"Jun", 3))
		mon = 5;
	else if (!wcsnicmp(str, L"Jul", 3))
		mon = 6;
	else if (!wcsnicmp(str, L"Aug", 3))
		mon = 7;
	else if (!wcsnicmp(str, L"Sep", 3))
		mon = 8;
	else if (!wcsnicmp(str, L"Oct", 3))
		mon = 9;
	else if (!wcsnicmp(str, L"Nov", 3))
		mon = 10;
	else if (!wcsnicmp(str, L"Dec", 3))
		mon = 11;

	return mon;
}

int getGMOffset(const wchar_t *str)
{
	int secs = 0;
	/* See note in getMonthOfYear() */

	if (!wcsnicmp(str, L"UT", 2) || !wcsnicmp(str, L"GMT", 3))
		secs = 0;
	else if (!wcsnicmp(str, L"EDT", 3))
		secs = -4 * 3600;
	else if (!wcsnicmp(str, L"PST", 3))
		secs = -8 * 3600;
	else if (!wcsnicmp(str, L"EST", 3) || !wcsnicmp(str, L"CDT", 3))
		secs = -5 * 3600;
	else if (!wcsnicmp(str, L"MST", 3) || !wcsnicmp(str, L"PDT", 3))
		secs = -7 * 3600;
	else if (!wcsnicmp(str, L"CST", 3) || !wcsnicmp(str, L"MDT", 3))
		secs = -6 * 3600;
	else if ( (str[0] == L'+' || str[0] == L'-') &&
	          iswdigit(str[1]) && iswdigit(str[2]) &&
	          iswdigit(str[3]) && iswdigit(str[4]))
	{
		secs = 3600 * (10 * (str[1] - 48) + str[2] - 48);
		secs += 60 * (10 * (str[3] - 48) + str[4] - 48);

		if (str[0] == L'-')
			secs = -secs;
	}
	else
		secs = 1;

	return secs;
}