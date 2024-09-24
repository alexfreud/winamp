#ifndef NULLSOFT_UPDATEAUTODOWNLOADH
#define NULLSOFT_UPDATEAUTODOWNLOADH

/* This header file is used by FeedsDialog.h
It provides a set of helper functions to deal with the combo box for auto download 
 
basically - converts between combo box choice and int
*/
namespace UpdateAutoDownload
{
	enum
	{
	    AUTODOWNLOAD_NEVER = 0,
	    AUTODOWNLOAD_LAST_ONE,
	    AUTODOWNLOAD_LAST_TWO,
	    AUTODOWNLOAD_LAST_THREE,
	    AUTODOWNLOAD_LAST_FIVE,
	    AUTODOWNLOAD_NUMENTRIES
	};

	extern int episodes[];
	
	const wchar_t *GetTitle(int position, wchar_t *buffer, int bufferMax);
	bool GetAutoDownload(int selection);
	int GetAutoDownloadEpisodes(int selection);
	int GetSelection(int selEpisodes, bool autoDownload);
}
#endif
