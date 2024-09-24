#include "precomp__gen_ff.h"
#include "main.h"
#include "../Agave/Language/api_language.h"
#include "../../..\Components\wac_network\wac_network_http_receiver_api.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoCharFn.h"
#include "wa2frontend.h"
#include "resource.h"
#include "../nu/ns_wc.h"
#include "../nu/refcount.h"
#include "api/skin/widgets/xuidownloadslist.h"

#include <shlobj.h>
#include <shlwapi.h>
#include <api/script/objects/systemobj.h>

extern wchar_t *INI_DIR;

static void createDirForFileW(wchar_t *str)
{
	wchar_t *p = str;
	if ((p[0] ==L'\\' || p[0] ==L'/') && (p[1] ==L'\\' || p[1] ==L'/'))
	{
		p += 2;
		while (p && *p && *p !=L'\\' && *p !=L'/') p++;
		if (!p || !*p) return ;
		p++;
		while (p && *p && *p !=L'\\' && *p !=L'/') p++;
	}
	else
	{
		while (p && *p && *p !=L'\\' && *p !=L'/') p++;
	}

	while (p && *p)
	{
		while (p && *p !=L'\\' && *p !=L'/' && *p) p = CharNextW(p);
		if (p && *p)
		{
			wchar_t lp = *p;
			*p = 0;
			CreateDirectoryW(str, NULL);
			*p++ = lp;
		}
	}
}

static wchar_t* defaultPathToStore()
{
	static wchar_t pathToStore[MAX_PATH] = {0};
	if(FAILED(SHGetFolderPathW(NULL, CSIDL_MYMUSIC, NULL, SHGFP_TYPE_CURRENT, pathToStore)))
	{
		if(FAILED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, pathToStore)))
		{
			lstrcpynW(pathToStore, L"C:\\My Music", MAX_PATH);
		}
	}
	return pathToStore;
}

static void GetPathToStore(wchar_t path_to_store[MAX_PATH])
{
	GetPrivateProfileStringW( L"gen_ml_config", L"extractpath", defaultPathToStore(), path_to_store, MAX_PATH, (const wchar_t *)SendMessageW( plugin.hwndParent, WM_WA_IPC, 0, IPC_GETMLINIFILEW ) );
}

void Winamp2FrontEnd::getDownloadPath(wchar_t path2store[MAX_PATH])
{
	GetPathToStore(path2store);
}

void Winamp2FrontEnd::setDownloadPath(const wchar_t * path2store)
{
	WritePrivateProfileStringA( "gen_ml_config", "extractpath", AutoChar( path2store, CP_UTF8 ), (const char *)SendMessageW( plugin.hwndParent, WM_WA_IPC, 0, IPC_GETMLINIFILE ) );
}

class DownloadCallback : public Countable<ifc_downloadManagerCallback>
{
public:
	DownloadCallback( const wchar_t *destination_filepath, bool storeInMl = true, bool notifyDownloadsList = true )
	{
		WCSCPYN( this->destination_filepath, destination_filepath, MAX_PATH );

		this->storeInMl           = storeInMl;
		this->notifyDownloadsList = notifyDownloadsList;
	}


	void OnFinish( DownloadToken token );

	void OnError( DownloadToken token, int error )
	{
		if ( notifyDownloadsList )
			DownloadsList::onDownloadError( token, error );

		Release();
	}

	void OnCancel( DownloadToken token )
	{
		if ( notifyDownloadsList )
			DownloadsList::onDownloadCancel( token );

		Release();
	}
	void OnTick( DownloadToken token )
	{
		if ( notifyDownloadsList )
			DownloadsList::onDownloadTick( token );
	}

	const wchar_t *getPreferredFilePath()
	{
		return destination_filepath;
	}

	bool getStoreInML()
	{
		return storeInMl;
	}
	REFERENCE_COUNT_IMPLEMENTATION;

protected:
	RECVS_DISPATCH;

private:
	bool    storeInMl;
	bool    notifyDownloadsList;
	wchar_t destination_filepath[ MAX_PATH ];
};

// TODO: benski> this is a big hack. we should have a MIME manager in Winamp
struct
{
	const char *mime; const char *ext;
}
hack_mimes[] = { {"audio/mpeg", ".mp3"} };

void DownloadCallback::OnFinish(DownloadToken token)
{
	api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
	if (http)
	{
		const char *extension = 0;
		const char *headers = http->getallheaders();
		//  we're trying to figure out wtf kind of file this is

		if (!extension) // just adding this if to make this easier to re-arrange
		{
			//   1) check if there's a content-disposition, it might have an extension
			const char *content_disposition = http->getheader("content-disposition");
			if (content_disposition)
			{
				const char *content_filename = strstr(content_disposition, "filename=");
				if (content_filename && *content_filename)
				{
					content_filename+=9;
					extension = PathFindExtensionA(content_filename);
				}
			}
		}

		if (!extension)
		{
			//   2) check MIME type for something good
			const char *content_type = http->getheader("content-type");
			if (content_type)
			{
				for (int i=0;i!=sizeof(hack_mimes)/sizeof(hack_mimes[0]);i++)
				{
					if (!strcmp(hack_mimes[i].mime, content_type))
					{
						extension=hack_mimes[i].ext;
					}
				}
			}
		}

		const char *url = http->get_url();
		if (!extension)
		{
			//   3) check if URL has an extension
		
			if (url) // umm, we better have a URL :)  but worth a check
			{
				extension = PathFindExtensionA(url);
			}
		}

		if (!extension)
		{
			//   4) ask for winamp's default extension and use that (most likely mp3)
			extension=".mp3"; // TODO: actually use the setting and not hardcode this :)
		}

		// then we rename the file
		wchar_t temppath[MAX_PATH-14] = {0};
		wchar_t filename[MAX_PATH]    = {0};

		GetTempPathW(MAX_PATH-14, temppath);
		GetTempFileNameW(temppath, L"atf", 0, filename);
		PathRemoveExtensionW(filename);
		PathAddExtensionW(filename, AutoWide(extension));
		
		const wchar_t *downloadDest = WAC_API_DOWNLOADMANAGER->GetLocation(token);
		MoveFileW(downloadDest, filename);
		
		// then we build a filename with ATF
		wchar_t formatted_filename[MAX_PATH] = {0}, path_to_store[MAX_PATH] = {0};
		if (!WCSICMP(this->getPreferredFilePath(), L""))
			GetPathToStore(path_to_store);
		else
			WCSCPYN(path_to_store, this->getPreferredFilePath(), MAX_PATH);

		// TODO: benski> this is very temporary
		char temp_filename[MAX_PATH] = {0}, *t = temp_filename,
			 *tfn = PathFindFileNameA(url);

		while(tfn && *tfn)
		{
			if(*tfn == '%')
			{
				if(_strnicmp(tfn,"%20",3))
				{
					*t = *tfn;
				}
				else{
					*t = ' ';
					tfn = CharNextA(tfn);
					tfn = CharNextA(tfn);
				}
			}
			else
			{
				*t = *tfn;
			}
			tfn = CharNextA(tfn);
			t = CharNextA(t);
		}

		*t = 0;

		PathCombineW(formatted_filename, path_to_store, AutoWide(temp_filename));
		createDirForFileW(formatted_filename);

		// then move the file there
		if (!MoveFileW(filename, formatted_filename))
		{
			CopyFileW(filename, formatted_filename, FALSE);
			DeleteFileW(filename);
		}

		//Std::messageBox(formatted_filename, filename, 0);
		if (this->getStoreInML() && PathFileExistsW(formatted_filename))
		{
			// then add to the media library :)
			// TOOD: benski> use api_mldb because it's more thread-friendly than SendMessageW
			HWND library = wa2.getMediaLibrary();
			LMDB_FILE_ADD_INFOW fi = {const_cast<wchar_t *>(formatted_filename), -1, -1};
			SendMessageW(library, WM_ML_IPC, (WPARAM)&fi, ML_IPC_DB_ADDORUPDATEFILEW);
			PostMessage(library, WM_ML_IPC, 0, ML_IPC_DB_SYNCDB);
		}

		if (notifyDownloadsList)
			DownloadsList::onDownloadEnd(token, const_cast<wchar_t *>(formatted_filename));
		
		SystemObject::onDownloadFinished(AutoWide(url), true, formatted_filename);

		Release();

		return;
	}

	if (notifyDownloadsList)
		DownloadsList::onDownloadEnd(token, NULL);

	Release();
}

int Winamp2FrontEnd::DownloadFile( const char *url, const wchar_t *destfilepath, bool addToMl, bool notifyDownloadsList )
{
	DownloadCallback *callback = new DownloadCallback( destfilepath, addToMl, notifyDownloadsList );
	DownloadToken     dt = WAC_API_DOWNLOADMANAGER->Download( url, callback );

	// Notify <DownloadsList/>
	if ( notifyDownloadsList )
		DownloadsList::onDownloadStart( url, dt );

	return 0;
	/*
	HTTPRETRIEVEFILEW func = (HTTPRETRIEVEFILEW) SendMessageW(hwnd_winamp, WM_WA_IPC, 0, IPC_GETHTTPGETTERW);
	if (func || func == (HTTPRETRIEVEFILEW)1)
		return func(NULL, url, destfilename, title);
	else
		return 0;
		*/
}

#define CBCLASS DownloadCallback
START_DISPATCH;
REFERENCE_COUNTED;
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONFINISH, OnFinish )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONTICK,   OnTick )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONERROR,  OnError )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONCANCEL, OnCancel )
END_DISPATCH;
#undef CBCLASS
