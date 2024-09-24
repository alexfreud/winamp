#include "main.h"
#include "api__ml_wire.h"
#include "UpdateAutoDownload.h"

int UpdateAutoDownload::episodes[] = {0,           // AUTODOWNLOAD_NEVER
									  1,           // AUTODOWNLOAD_LAST_ONE
									  2,           // AUTODOWNLOAD_LAST_TWO
									  3,           // AUTODOWNLOAD_LAST_THREE
									  5,           // AUTODOWNLOAD_LAST_FIVE
};

const wchar_t *UpdateAutoDownload::GetTitle(int position, wchar_t *buffer, int bufferMax)
{
	if (NULL == buffer) 
		return NULL;

	INT stringId = IDS_ERROR_FYEO;
	switch (position)
	{
		case AUTODOWNLOAD_NEVER:			stringId = IDS_ATD_NEVER; break;
		case AUTODOWNLOAD_LAST_ONE:		stringId = IDS_ATD_LASTONE; break;
		case AUTODOWNLOAD_LAST_TWO:		stringId = IDS_ATD_LASTTWO; break;
		case AUTODOWNLOAD_LAST_THREE:	stringId = IDS_ATD_LASTTHREE; break;
		case AUTODOWNLOAD_LAST_FIVE:	stringId = IDS_ATD_LASTFIVE; break;
	}
	return WASABI_API_LNGSTRINGW_BUF(stringId, buffer, bufferMax);
}


bool UpdateAutoDownload::GetAutoDownload(int selection)
{
	if (selection == AUTODOWNLOAD_NEVER)
		return false;
	else
		return true;
}

int UpdateAutoDownload::GetAutoDownloadEpisodes(int selection)
{
	if (selection >= 0 && selection < AUTODOWNLOAD_NUMENTRIES)
		return episodes[selection];
	else
		return 0;
}

int UpdateAutoDownload::GetSelection(int selEpisodes, bool autoDownload)
{
	if (!autoDownload)
		return AUTODOWNLOAD_NEVER;

	for (int i = AUTODOWNLOAD_LAST_ONE;i < AUTODOWNLOAD_NUMENTRIES;i++)
		if (selEpisodes == episodes[i])
			return i;

	return AUTODOWNLOAD_LAST_ONE;

}