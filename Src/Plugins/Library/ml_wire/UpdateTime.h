#ifndef NULLSOFT_UPDATETIMEH
#define NULLSOFT_UPDATETIMEH

#include <time.h>

/* This header file is used by FeedsDialog.h
It provides a set of helper functions to deal with the combo box for update time
 
basically - converts between combo box choice and __time64_t
*/
namespace Update
{
	enum
	{
	    TIME_MANUALLY = 0,
	    TIME_WEEKLY,
	    TIME_DAILY,
	    TIME_HOURLY,
	    TIME_NUMENTRIES
	};

	extern __time64_t times[];
	
	const wchar_t *GetTitle(int position, wchar_t *buffer, int bufferMax);
	bool GetAutoUpdate(int selection);
	__time64_t GetTime(int selection);
	int GetSelection(__time64_t selTime, bool autoUpdate);
}
#endif
