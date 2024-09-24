#include "Main.h"
#include "playlistsXML.h"
#include "../nu/AutoChar.h"

void PlaylistsXML::StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, api_xmlreaderparams *params)
{
	const wchar_t *filename = params->getItemValue(L"filename");
	const wchar_t *title = params->getItemValue(L"title");
	const wchar_t *countString = params->getItemValue(L"songs");

	int numItems = 0;
	if (countString && *countString)
		numItems = _wtoi(countString);

	const wchar_t *lengthString = params->getItemValue(L"seconds");
	int length = -1000;
	if (lengthString && *lengthString)
		length = _wtoi(lengthString);

	AddPlaylist(title, filename, true, numItems, length);
}

void PlaylistsXML::LoadFile(const wchar_t *filename)
{
	if (!parser)
		return ; // no sense in continuing if there's no parser available

	HANDLE file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	if (file == INVALID_HANDLE_VALUE)
		return ;

	char data[1024] = {0};
	DWORD bytesRead;
	while (true)
	{
		if (ReadFile(file, data, 1024, &bytesRead, NULL) && bytesRead)
		{
			if (parser->xmlreader_feed(data, bytesRead) != API_XML_SUCCESS)
			{
				CloseHandle(file);
				return;
			}
		}
		else
			break;
	}

	CloseHandle(file);
	parser->xmlreader_feed(0, 0);
}

PlaylistsXML::PlaylistsXML(): parser(0), parserFactory(0)
{
	parserFactory = WASABI_API_SVC->service_getServiceByGuid(api_xmlGUID);
	if (parserFactory)
		parser = (api_xml *)parserFactory->getInterface();

	if (parser)
	{
		parser->xmlreader_registerCallback(L"Winamp:Playlists\fplaylist", this);
		parser->xmlreader_registerCallback(L"playlists\fplaylist", this);
		parser->xmlreader_open();
	}
}

PlaylistsXML::~PlaylistsXML()
{
	if (parser)
	{
		parser->xmlreader_unregisterCallback(this);
		parser->xmlreader_close();
	}

	if (parserFactory && parser)
		parserFactory->releaseInterface(parser);

	parserFactory = 0;
	parser = 0;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS PlaylistsXML
START_DISPATCH;
VCB(ONSTARTELEMENT, StartTag)
END_DISPATCH;
