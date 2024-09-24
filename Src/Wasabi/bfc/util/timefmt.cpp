#include <precomp.h>
#include <bfc/wasabi_std.h>
#include <time.h>
#include "timefmt.h"

void TimeFmt::printMinSec(int sec, wchar_t *buf, int buflen)
{
  int minutes, seconds;
  int negative = sec < 0;

  if (buf == NULL) return;

  if (sec == -1) 
  {
    *buf = 0;
    return;
  }

  seconds = sec % 60;
  sec /= 60;
  minutes = sec;

  StringPrintfW sp(L"%s%d:%02d", (minutes == 0 && negative) ? L"-" : L"", minutes, ABS(seconds));
  WCSCPYN(buf, sp, buflen);
}

void TimeFmt::printHourMinSec(int sec, wchar_t *buf, int buflen, int hoursonlyifneeded) 
{
  int hours, minutes, seconds;
  int negative = sec < 0;

  sec = ABS(sec);
  if (buf == NULL) return;

  if (sec == -1) {
    *buf = 0;
    return;
  }

  hours = sec / 3600;
  sec -= hours * 3600;
  seconds = sec % 60;
  sec /= 60;
  minutes = sec;

  StringW sp;
  if (hoursonlyifneeded && hours == 0)
    sp = StringPrintfW(L"%s%d:%02d", (minutes == 0 && negative) ? L"-" : L"", minutes, seconds);
  else
    sp = StringPrintfW(L"%s%d:%02d:%02d", (hours == 0 && negative) ? L"-" : L"", hours, minutes, seconds);

  WCSCPYN(buf, sp, buflen);
}

void TimeFmt::printTimeStamp(wchar_t *buf, int bufsize, int ts) 
{
  if (ts == 0) 
  {
    WCSCPYN(buf, L"Never", bufsize);	// FUCKO: load from lang pack
    return;
  }

  struct tm *tm_now;
  tm_now = localtime((const time_t *)&ts);
  if (tm_now == NULL)
  {
    *buf = 0;
    return;
  }
  wcsftime(buf, bufsize, L"%a %b %Y %d %I:%M:%S %p", tm_now);
}
