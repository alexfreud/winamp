#include "main.h"
#include "api__ml_wire.h"
#include "UpdateTime.h"

__time64_t Update::times[] = {0,                                                 // TIME_MANUALLY
60 /* 1 minute */ * 60 /* 1 hour */ * 24 /* 1 day */ * 7 /* 1 week */,           // TIME_WEEKLY
60 /* 1 minute */ * 60 /* 1 hour */ * 24 /* 1 day */,                            // TIME_DAILY
60 /* 1 minute */ * 60 /* 1 hour */,                                             // TIME_HOURLY
};

const wchar_t *Update::GetTitle( int position, wchar_t *buffer, int bufferMax )
{
	if ( NULL == buffer )
		return NULL;

	INT stringId = IDS_ERROR_FYEO;
	switch ( position )
	{
		case TIME_MANUALLY:
			stringId = IDS_UPD_MANUALLY;
			break;
		case TIME_WEEKLY:
			stringId = IDS_UPD_WEEK;
			break;
		case TIME_DAILY:
			stringId = IDS_UPD_DAY;
			break;
		case TIME_HOURLY:
			stringId = IDS_UPD_HOUR;
			break;
	}
	return WASABI_API_LNGSTRINGW_BUF( stringId, buffer, bufferMax );
}

bool Update::GetAutoUpdate(int selection)
{
	if (selection == TIME_MANUALLY)
		return false;
	else
		return true;
}

__time64_t Update::GetTime(int selection)
{
	if (selection >= 0 && selection < TIME_NUMENTRIES)
		return times[selection];
	else
		return 0;
}

int Update::GetSelection(__time64_t selTime, bool autoUpdate)
{
	if (!autoUpdate)
		return TIME_MANUALLY;

	for (int i = TIME_WEEKLY;i < TIME_NUMENTRIES;i++)
		if (selTime >= times[i])
			return i;

	return TIME_DAILY;

}