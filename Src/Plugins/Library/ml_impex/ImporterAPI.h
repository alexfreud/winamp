#include "api_importer.h"

class ImporterAPI : public api_itunes_importer
{
public:
	bool iTunesExists();
	int ImportFromFile(HWND parent, const wchar_t *library_file);
	int ImportFromiTunes(HWND parent);
	int ImportPlaylistsFromiTunes(HWND parent);

protected:
	RECVS_DISPATCH;
};