#include "pdtimer.h"

__int64 pdReadResolution(void)
{
  __int64 myfeq;
  LARGE_INTEGER feq;

  QueryPerformanceFrequency( &feq);
  myfeq = feq.QuadPart;

  return myfeq;
}

__int64 pdReadTimer(void)
{
  __int64 mynow;

  LARGE_INTEGER now;
  
  QueryPerformanceCounter( &now );
  mynow = now.QuadPart;  

  return mynow;
}