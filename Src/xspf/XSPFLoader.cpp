#include "XSPFLoader.h"
#include "../xml/obj_xml.h"
#include "../xml/ifc_xmlreadercallback.h"
#include "api__xspf.h"
#include "../winamp/wa_ipc.h"
#include <api/service/waservicefactory.h>
#include <shlwapi.h>
#include <strsafe.h>

// tries to retrieve the media library API (if it's not already retrieved
bool HasMediaLibraryAPI()
{
	// TODO: should critical section this
	if (!AGAVE_API_MLDB)
	{
		waServiceFactory *mldbFactory = WASABI_API_SVC->service_getServiceByGuid(mldbApiGuid);
		if (mldbFactory)
			AGAVE_API_MLDB = (api_mldb *)mldbFactory->getInterface(); // retrieve a pointer to the API object
	}
	return !!AGAVE_API_MLDB;
}

static bool HasTagzAPI()
{
	// TODO: should critical section this
	if (!AGAVE_API_TAGZ)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(tagzGUID);
		if (sf)
			AGAVE_API_TAGZ = (api_tagz *)sf->getInterface(); // retrieve a pointer to the API object
	}
	return !!AGAVE_API_TAGZ;
}

const wchar_t* ATFString()
{
	// TODO: should critical section this
	if (!WASABI_API_APP)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
		if (sf)
			WASABI_API_APP = (api_application *)sf->getInterface(); // retrieve a pointer to the API object
	}
	if (WASABI_API_APP)
	{
		const wchar_t *atf = WASABI_API_APP->getATFString();
		if (atf && *atf) return atf;
	}
	return L"%artist% - %title%";
}

// go check out XSPFLoader::Load before decyphering this class
class XSPFLoaderCallback : public ifc_xmlreadercallback
{
public:
	XSPFLoaderCallback(ifc_playlistloadercallback *_playlist);
	~XSPFLoaderCallback();
	void xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params);
	void xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag);
	void xmlReaderOnCharacterDataCallback(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str);

protected:
	RECVS_DISPATCH;

private:
	enum
	{
		ELEMENT_NONE=-1,
		ELEMENT_LOCATION=0,
		ELEMENT_IDENTIFIER,
		ELEMENT_TITLE,
		ELEMENT_CREATOR,
		ELEMENT_ALBUM,
		ELEMENT_TRACKNUM,
		NUM_ELEMENTS,
	};
	wchar_t *elements[NUM_ELEMENTS];
	int element;
	ifc_playlistloadercallback *playlist;
};

XSPFLoaderCallback::XSPFLoaderCallback(ifc_playlistloadercallback *_playlist)
{
	element = ELEMENT_NONE;
	for (int i=0;i<NUM_ELEMENTS;i++)
		elements[i]=0;

	playlist = _playlist;
}

XSPFLoaderCallback::~XSPFLoaderCallback()
{
	for (int i=0;i<NUM_ELEMENTS;i++)
	{
		free(elements[i]);
		elements[i]=0;
	}
}

// this gets called for every opening tag
// xmlpath is the full XML path up to this point, delimited with \f (form feed)
// xmltag is the actual opening tag
// params is an object which lets you query for the attribtues in the tag
void XSPFLoaderCallback::xmlReaderOnStartElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	if (!wcscmp(xmlpath, L"playlist\ftrackList\ftrack\flocation"))
		element=ELEMENT_LOCATION;
	else if (!wcscmp(xmlpath, L"playlist\ftrackList\ftrack\fidentifier"))
		element=ELEMENT_IDENTIFIER;
	else if (!wcscmp(xmlpath, L"playlist\ftrackList\ftrack\ftitle"))
		element=ELEMENT_TITLE;
	else if (!wcscmp(xmlpath, L"playlist\ftrackList\ftrack\fcreator"))
		element=ELEMENT_CREATOR;
	else if (!wcscmp(xmlpath, L"playlist\ftrackList\ftrack\falbum"))
		element=ELEMENT_ALBUM;
	else if (!wcscmp(xmlpath, L"playlist\ftrackList\ftrack\ftracknum"))
		element=ELEMENT_TRACKNUM;
	else
		element=ELEMENT_NONE;
}

static void QueryStringAdd(const wchar_t *value, wchar_t *&query, size_t &query_len) 
{
	if (!value || !*value) return;
	while (value && *value && query_len > 3) {
		if (*value == L'%') { *query++ = L'%'; *query++ = L'%'; query_len-=2;}
		else if (*value == L'\"') { *query++ = L'%'; *query++ = L'2'; *query++ = L'2'; query_len-=3;}
		else if (*value == L'\'') { *query++ = L'%'; *query++ = L'2'; *query++ = L'7'; query_len-=3;}
		else if (*value == L'[') { *query++ = L'%'; *query++ = L'5'; *query++ = L'B'; query_len-=3;}
		else if (*value == L']') { *query++ = L'%'; *query++ = L'5'; *query++ = L'D'; query_len-=3;}
		else if (*value == L'(') { *query++ = L'%'; *query++ = L'2'; *query++ = L'8'; query_len-=3;}
		else if (*value == L')') { *query++ = L'%'; *query++ = L'2'; *query++ = L'9'; query_len-=3;}
		else if (*value == L'#') { *query++ = L'%'; *query++ = L'2'; *query++ = L'3'; query_len-=3;}
		else { *query++ = *value; query_len--; }
		value++;
	}
	*query = 0;
}

class ItemRecordTagProvider : public ifc_tagprovider
{
public:
	ItemRecordTagProvider(itemRecordW *item) : t(item) {}
	wchar_t *GetTag(const wchar_t *name, ifc_tagparams *parameters);
	void FreeTag(wchar_t *Tag);
protected:
	RECVS_DISPATCH;
private:
	itemRecordW *t;
};

#define CBCLASS ItemRecordTagProvider
START_DISPATCH;
CB(IFC_TAGPROVIDER_GET_TAG, GetTag);
VCB(IFC_TAGPROVIDER_FREE_TAG, FreeTag);
END_DISPATCH;
#undef CBCLASS

wchar_t *ItemRecordTagProvider::GetTag(const wchar_t *tag, ifc_tagparams *parameters) 
{
	bool copy=false;
	wchar_t buf[128]={0};
	wchar_t *value = NULL;

	if (!_wcsicmp(tag, L"artist"))	value = t->artist;
	else if (!_wcsicmp(tag, L"album"))	value = t->album;
	else if (!_wcsicmp(tag, L"filename"))	value = t->filename;
	else if (!_wcsicmp(tag, L"title"))	value = t->title;
	else if (!_wcsicmp(tag, L"year"))
	{
		if (t->year > 0)
		{
			StringCchPrintfW(buf, 128, L"%04d", t->year);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"genre"))	value = t->genre;
	else if (!_wcsicmp(tag, L"comment")) value = t->comment;
	else if (!_wcsicmp(tag, L"tracknumber") || !_wcsicmp(tag, L"track"))
	{
		if (t->track > 0)
		{
			if (t->tracks > 0)
				StringCchPrintfW(buf, 128, L"%02d/%02d", t->track, t->tracks);
			else
				StringCchPrintfW(buf, 128, L"%02d", t->track);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"disc"))
	{
		if (t->disc > 0)
		{
			if (t->discs > 0)
				StringCchPrintfW(buf, 128, L"%d/%d", t->disc, t->discs);
			else
				StringCchPrintfW(buf, 128, L"%d", t->disc);

			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"rating"))
	{
		if (t->rating > 0)
		{
			StringCchPrintfW(buf, 128, L"%d", t->rating);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"playcount"))
	{
		if (t->playcount > 0)
		{
			StringCchPrintfW(buf, 128, L"%d", t->playcount);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"bitrate"))
	{
		if (t->bitrate > 0)
		{
			StringCchPrintfW(buf, 128, L"%d", t->bitrate);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"bpm"))
	{
		if (t->bpm > 0)
		{
			StringCchPrintfW(buf, 128, L"%d", t->bpm);
			value = buf;
		}
	}
	else if (!_wcsicmp(tag, L"albumartist"))	value = t->albumartist;
	else if (!_wcsicmp(tag, L"publisher"))	value = t->publisher;
	else if (!_wcsicmp(tag, L"composer"))	value = t->composer;
	else if (!_wcsicmp(tag, L"replaygain_album_gain"))	value = t->replaygain_album_gain;
	else if (!_wcsicmp(tag, L"replaygain_track_gain"))	value = t->replaygain_track_gain;
	else if (!_wcsicmp(tag, L"GracenoteFileID"))	
	{
		copy=true;
		value = getRecordExtendedItem(t, L"GracenoteFileID");
	}
	else if (!_wcsicmp(tag, L"GracenoteExtData"))
	{
		copy=true;
		value = getRecordExtendedItem(t, L"GracenoteExtData");
	}
	else
		return 0;

	if (!value)
		return 0;
	else
	{
		if (copy || value == buf)
			return AGAVE_API_MLDB->DuplicateString(value);
		else
		{
			AGAVE_API_MLDB->RetainString(value);
			return value;
		}
	}
}

void ItemRecordTagProvider::FreeTag(wchar_t *tag)
{
	AGAVE_API_MLDB->ReleaseString(tag);
}

// this gets called for every closing tag
void XSPFLoaderCallback::xmlReaderOnEndElementCallback(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	if (!wcscmp(xmlpath, L"playlist\ftrackList\ftrack"))
	{
		// end of track info
		if (elements[ELEMENT_LOCATION] // if we have a location
		&& (PathIsURL(elements[ELEMENT_LOCATION]) // and it's a URL
			|| GetFileAttributes(elements[ELEMENT_LOCATION])  != INVALID_FILE_ATTRIBUTES)) // or a file that exists
		{
			// location field seemed OK so go ahead and add
			// TODO try using the length if available...
			playlist->OnFile(elements[ELEMENT_LOCATION], 0, -1, 0);
		}
		else if (HasMediaLibraryAPI()) // we don't have a good location so let's hit the media library
		{
			// let's build a query out of what we have
			bool needs_and=false;
			wchar_t query[2048]={0};
			wchar_t *query_end=query; 
			size_t query_size=2048;

			struct {
				const wchar_t *field_name;
				int field_value;
			} fields[] =
			{
				{L"artist", ELEMENT_CREATOR},
				{L"title", ELEMENT_TITLE},
			};
			for(size_t i=0;i!=sizeof(fields)/sizeof(fields[0]);i++)
			{
				if (elements[fields[i].field_value])
				{
					if (needs_and)
						StringCchCopyEx(query_end, query_size, L" AND ", &query_end, &query_size, 0);
					StringCchPrintfEx(query_end, query_size, &query_end, &query_size, 0, L"(%s == \"", fields[i].field_name);
					QueryStringAdd(elements[fields[i].field_value], query_end, query_size);
					StringCchCopyEx(query_end, query_size, L"\")", &query_end, &query_size, 0);
					needs_and=true;
				}
			}

			if (query[0])
			{
				itemRecordListW *results = AGAVE_API_MLDB->QueryLimit(query, 1);
				if (results && results->Size >= 1 && results->Items[0].filename)
				{
					if (HasTagzAPI())
					{
						wchar_t title[2048]={0};
						ItemRecordTagProvider provider(&results->Items[0]);
						AGAVE_API_TAGZ->format(ATFString(), title, 2048, &provider, 0);
						int length = -1;
						if (results->Items[0].length > 0)
							length=results->Items[0].length*1000;
						playlist->OnFile(results->Items[0].filename, title, length, 0);
					}
					else
					{
						int length = -1;
						if (results->Items[0].length > 0)
							length=results->Items[0].length*1000;
						playlist->OnFile(results->Items[0].filename, 0, length, 0);
					}
				}
				AGAVE_API_MLDB->FreeRecordList(results);
			}
		}
		for (int i=0;i<NUM_ELEMENTS;i++)
		{
			free(elements[i]);
			elements[i]=0;
		}
	}
	else if (!wcscmp(xmlpath, L"playlist\ftrackList\ftrack\flocation"))
		element=ELEMENT_NONE;
	else if (!wcscmp(xmlpath, L"playlist\ftrackList\ftrack\fidentifier"))
		element=ELEMENT_NONE;
	else if (!wcscmp(xmlpath, L"playlist\ftrackList\ftrack\ftitle"))
		element=ELEMENT_NONE;
	else if (!wcscmp(xmlpath, L"playlist\ftrackList\ftrack\fcreator"))
		element=ELEMENT_NONE;
	else if (!wcscmp(xmlpath, L"playlist\ftrackList\ftrack\falbum"))
		element=ELEMENT_NONE;
	else if (!wcscmp(xmlpath, L"playlist\ftrackList\ftrack\ftracknum"))
		element=ELEMENT_NONE;
}

void XSPFLoaderCallback::xmlReaderOnCharacterDataCallback(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str)
{
	if (element!=ELEMENT_NONE)
	{
		// this certainly isn't the most memory efficient way to do this but it'll work for now :)
		if (elements[element] == 0)
		{
			elements[element] = wcsdup(str);
		}
		else
		{
			size_t elementlen = wcslen(elements[element]);
			size_t incominglen = wcslen(str);
			
			wchar_t *&newstr = elements[element], *oldstr = newstr;
			newstr = (wchar_t *)realloc(newstr, sizeof(wchar_t)*(elementlen+incominglen+1));
			if (newstr != NULL)
			{
				memcpy(newstr+elementlen, str, incominglen*sizeof(wchar_t));
				newstr[elementlen+incominglen] = 0;
			}
		}
	}
}

#define CBCLASS XSPFLoaderCallback
START_DISPATCH;
VCB(ONSTARTELEMENT, xmlReaderOnStartElementCallback)
VCB(ONENDELEMENT, xmlReaderOnEndElementCallback)
VCB(ONCHARDATA, xmlReaderOnCharacterDataCallback)
END_DISPATCH;
#undef CBCLASS

/* ---------------------------- */
static int LoadFile(obj_xml *parser, const wchar_t *filename)
{
	HANDLE file = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

	if (file == INVALID_HANDLE_VALUE)
		return IFC_PLAYLISTLOADER_FAILED;

	while (true)
	{
		char data[1024] = {0};
		DWORD bytesRead = 0;
		if (ReadFile(file, data, 1024, &bytesRead, NULL) && bytesRead)
		{
			if (parser->xmlreader_feed(data, bytesRead) != API_XML_SUCCESS)
			{
				CloseHandle(file);
				return IFC_PLAYLISTLOADER_FAILED;
			}
		}
		else
			break;
	}

	CloseHandle(file);
	if (parser->xmlreader_feed(0, 0) != API_XML_SUCCESS)
		return IFC_PLAYLISTLOADER_FAILED;

	return IFC_PLAYLISTLOADER_SUCCESS;
}

// this get called by Winamp
// you a method in the passed playlist loader callback object
// for each item in the playlist
int XSPFLoader::Load(const wchar_t *filename, ifc_playlistloadercallback *playlist)
{
	// first thing we'll need is an XML parser
	// we'll have to do the wasabi dance to get it

	// first, get the service factory for creating XML objects
	waServiceFactory *xmlFactory = WASABI_API_SVC->service_getServiceByGuid(obj_xmlGUID);
	if (xmlFactory)
	{
		obj_xml *parser = (obj_xml *)xmlFactory->getInterface(); // create an XML parser
		if (parser)
		{
			// ok now we can get down to biz'nes
			XSPFLoaderCallback xmlCallback(playlist);
			parser->xmlreader_registerCallback(L"*", &xmlCallback);
			parser->xmlreader_setCaseSensitive();
			parser->xmlreader_open();
			int ret = LoadFile(parser, filename);
			parser->xmlreader_unregisterCallback(&xmlCallback);
			parser->xmlreader_close();
			xmlFactory->releaseInterface(parser); // destroy the XML parser via the service factory

			return ret;
		}
	}

	return IFC_PLAYLISTLOADER_FAILED;
}

// Define the dispatch table
#define CBCLASS XSPFLoader
START_DISPATCH;
CB(IFC_PLAYLISTLOADER_LOAD, Load)
END_DISPATCH;
#undef CBCLASS