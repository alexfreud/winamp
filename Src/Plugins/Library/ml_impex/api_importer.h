#pragma once

#include <bfc/dispatch.h>
#include <api/service/services.h>

// {A32C39BC-CDF7-4c9b-9EA2-9DF7E0D651D2}
static const GUID iTunesImporterGUID = 
{ 0xa32c39bc, 0xcdf7, 0x4c9b, { 0x9e, 0xa2, 0x9d, 0xf7, 0xe0, 0xd6, 0x51, 0xd2 } };

class api_itunes_importer : public Dispatchable
{
protected:
	api_itunes_importer() {}
	~api_itunes_importer() {}
public:
	static FOURCC getServiceType() { return WaSvc::UNIQUE; }
	static const char *getServiceName() { return "iTunes Importer Service"; }
	static GUID getServiceGuid() { return iTunesImporterGUID; }

	bool iTunesExists();
	int ImportFromFile(HWND parent, const wchar_t *library_file);
	int ImportFromiTunes(HWND parent);
	int ImportPlaylistsFromiTunes(HWND parent);

	enum
	{
		API_ITUNES_IMPORTER_ITUNESEXISTS = 0,
		API_ITUNES_IMPORTER_IMPORTFROMFILE = 1,
		API_ITUNES_IMPORTER_IMPORTFROMITUNES = 2,
		API_ITUNES_IMPORTER_IMPORTPLAYLISTSFROMITUNES = 3,
	};
};

inline bool api_itunes_importer::iTunesExists()
{
	return _call(API_ITUNES_IMPORTER_ITUNESEXISTS, (bool)false);
}

inline int api_itunes_importer::ImportFromFile(HWND parent, const wchar_t *library_file)
{
	return _call(API_ITUNES_IMPORTER_IMPORTFROMFILE, (int)DISPATCH_FAILURE, parent, library_file);
}

inline int api_itunes_importer::ImportFromiTunes(HWND parent)
{
	return _call(API_ITUNES_IMPORTER_IMPORTFROMITUNES, (int)DISPATCH_FAILURE, parent);
}

inline int api_itunes_importer::ImportPlaylistsFromiTunes(HWND parent)
{
		return _call(API_ITUNES_IMPORTER_IMPORTPLAYLISTSFROMITUNES, (int)DISPATCH_FAILURE, parent);
}
