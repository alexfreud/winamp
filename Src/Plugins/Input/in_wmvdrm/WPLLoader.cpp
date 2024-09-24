#include "main.h"
#include "WPLLoader.h"
#include <stdio.h>
#include "../nu/AutoWide.h"
#include "../xml/ifc_xmlreadercallback.h"
#include "../xml/obj_xml.h"
#include "api.h"
#include <api/service/waservicefactory.h>
#include <shlwapi.h>
#include <strsafe.h>

class WPLXML : public ifc_xmlreadercallback
{
public:
	WPLXML(ifc_playlistloadercallback *_playlist, const wchar_t *wplFilename) : playlist(_playlist)
	{
		lstrcpynW(rootPath, wplFilename, MAX_PATH);
		PathRemoveFileSpecW(rootPath);
	}
	void StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	//not necessary YET, it will be if we register for more things: if (!_wcsicmp(xmlpath, L"smil\fbody\fseq\fmedia")) 
	{
		const wchar_t *track = params->getItemValue(L"src");
		
		if (track)
		{
			if (PathIsRootW(track) || PathIsURLW(track))
			{
        			playlist->OnFile(track, 0, -1, 0); // TODO: more info!!!
			}
			else
			{
					wchar_t fullPath[MAX_PATH] = {0}, canonicalizedPath[MAX_PATH] = {0};
					PathCombineW(fullPath, rootPath, track);
					PathCanonicalizeW(canonicalizedPath, fullPath);
					playlist->OnFile(canonicalizedPath, 0, -1, 0); // TODO: more info!!!
			}
		}
	}
}
ifc_playlistloadercallback *playlist;
wchar_t rootPath[MAX_PATH];
protected:
	RECVS_DISPATCH;
};

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS WPLXML
START_DISPATCH;
VCB(ONSTARTELEMENT, StartTag)
END_DISPATCH;


WPLLoader::WPLLoader() 
{
}

WPLLoader::~WPLLoader(void)
{
	//Close();
}

int WPLLoader::Load(const wchar_t *filename, ifc_playlistloadercallback *playlist)
{
	HANDLE file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

	if (file == INVALID_HANDLE_VALUE)
		return IFC_PLAYLISTLOADER_FAILED;

	obj_xml *parser=0;
	waServiceFactory *parserFactory=0;

	parserFactory = plugin.service->service_getServiceByGuid(obj_xmlGUID);
	if (parserFactory)
		parser = (obj_xml *)parserFactory->getInterface();

	if (parser)
	{
		WPLXML wplXml(playlist, filename);
		parser->xmlreader_registerCallback(L"smil\fbody\fseq\fmedia", &wplXml);
		parser->xmlreader_open();
		parser->xmlreader_setEncoding(L"UTF-8"); // WPL is always UTF-8, but doesn't explicitly have it

		while (true)
		{
			char data[1024] = {0};
			DWORD bytesRead = 0;
			if (ReadFile(file, data, 1024, &bytesRead, NULL) && bytesRead)
			{
				if (parser->xmlreader_feed(data, bytesRead) != API_XML_SUCCESS)
				{
					CloseHandle(file);
					parser->xmlreader_unregisterCallback(&wplXml);
					parser->xmlreader_close();
					parserFactory->releaseInterface(parser);
					return IFC_PLAYLISTLOADER_FAILED;
				}
			}
			else
				break;
		}

		CloseHandle(file);
		parser->xmlreader_feed(0, 0);

		parser->xmlreader_unregisterCallback(&wplXml);
		parser->xmlreader_close();
		parserFactory->releaseInterface(parser);
		return IFC_PLAYLISTLOADER_SUCCESS;
	}

	CloseHandle(file);
	return IFC_PLAYLISTLOADER_FAILED;
}


#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS WPLLoader
START_DISPATCH;
CB(IFC_PLAYLISTLOADER_LOAD, Load)
END_DISPATCH;
