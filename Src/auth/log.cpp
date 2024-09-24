#include "api.h"
#include <api/syscb/callbacks/consolecb.h>
#include <time.h>
#include <strsafe.h>
void Log(const wchar_t *format, ...)
{
	wchar_t buffer[2048] = {0}; // hope it's big enough :)
	va_list args;
	va_start (args, format);
	StringCbVPrintf(buffer, sizeof(buffer), format, args);
	WASABI_API_SYSCB->syscb_issueCallback(SysCallback::CONSOLE, ConsoleCallback::DEBUGMESSAGE, 0, reinterpret_cast<intptr_t>(buffer));
	va_end(args);
}

const wchar_t *MakeDateString(__time64_t convertTime)
{
	static wchar_t dest[2048];

	SYSTEMTIME sysTime;
	tm *newtime = _localtime64(&convertTime);
	if (newtime)
	{
		sysTime.wYear	= (WORD)(newtime->tm_year + 1900);
		sysTime.wMonth	= (WORD)(newtime->tm_mon + 1);
		sysTime.wDayOfWeek = (WORD)newtime->tm_wday;
		sysTime.wDay		= (WORD)newtime->tm_mday;
		sysTime.wHour	= (WORD)newtime->tm_hour;
		sysTime.wMinute	= (WORD)newtime->tm_min;
		sysTime.wSecond	= (WORD)newtime->tm_sec;
		sysTime.wMilliseconds = 0;

		GetTimeFormatW(LOCALE_USER_DEFAULT, 0, &sysTime, NULL, dest, 2048);

		//wcsftime(expire_time, 63, L"%b %d, %Y", _localtime64(&convertTime));
	}
	else
		dest[0] = 0;

	return dest;
}

