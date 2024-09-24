#include "on2_timer.h"

#if defined(WIN32)
#include <windows.h>
#include <mmsystem.h>
#else
#include <sys/time.h>
#endif

#if defined( WIN32 ) 
const unsigned long MAX_BEFORE_ROLLOVER = (1000 * 60 * 60 * 24);
#else
const unsigned long MAX_BEFORE_ROLLOVER = 0xFFFFFFFF;
#endif

// full day in milliseconds
const unsigned long kDuckFullDayMilli = 86400000;


void
on2Timer_Init ( on2Timer* pTimer )
{
	pTimer->elapsedMilli = 0;
	pTimer->baseMilli = 0;
}

/* The DeInit function was in danger of trying to free statically allocated timers goofy */


unsigned long
on2Timer_Start ( on2Timer* pTimer )
{
	pTimer->baseMilli = on2Timer_GetCurrentTimeMilli();
    	pTimer->elapsedMilli = 0;
	return pTimer->baseMilli;
}

unsigned long
on2Timer_Stop ( on2Timer* pTimer )
{	
    unsigned long currentMilli = on2Timer_GetCurrentTimeMilli();
	
	if(currentMilli >= pTimer->baseMilli)
	{
		pTimer->elapsedMilli = currentMilli - pTimer->baseMilli;
		return pTimer->elapsedMilli;
	}
	// rollover condition, get milli before rollover, add to current milli
    // I think if there is a rollover during timing on win32 this will cause a crash 
    // when the addition of currentMilli and rollMilli results in overflowing the size of
    // and unsigned long int
	else
	{
		unsigned long rollMilli = MAX_BEFORE_ROLLOVER - pTimer->baseMilli;
		pTimer->elapsedMilli = currentMilli + rollMilli;
		return pTimer->elapsedMilli;	
	}		
}


unsigned long
on2Timer_GetCurrentElapsedMilli ( on2Timer* pTimer )
{
	unsigned long currentMilli = on2Timer_GetCurrentTimeMilli();
	
	if(currentMilli >= pTimer->baseMilli)
	{	
		return ( currentMilli - pTimer->baseMilli );
	}
	// rollover condition, get milli before rollover, add to current milli
	else
	{
		unsigned long rollMilli = MAX_BEFORE_ROLLOVER - pTimer->baseMilli;
		return ( currentMilli + rollMilli );
	}
}

unsigned long 
on2Timer_GetCurrentTimeSeconds ()
{
	unsigned long currentMilli = on2Timer_GetCurrentTimeMilli();
	return currentMilli / 1000;
}

unsigned long 
on2Timer_GetCurrentTimeMilli ()
{
#if !defined(WIN32)
    unsigned long currentMilli;
    struct timeval tv;
	struct timezone tz;
	gettimeofday ( &tv, &tz );
	currentMilli = (tv.tv_sec * 1000) + (tv.tv_usec/1000);
    return currentMilli;
#else
    return timeGetTime();
#endif
	
}

int
on2Timer_Sleep( int msec )
{
#ifdef _WIN32
	Sleep( msec );
#endif

	return 0;
}

