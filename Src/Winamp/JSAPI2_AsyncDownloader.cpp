#include "JSAPI2_AsyncDownloader.h"
#include "JSAPI2_Security.h"
#include "main.h"
#include "../Agave/Language/api_language.h"
#include "JSAPI.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoLock.h"
#include "api.h"
#include "..\Components\wac_network\wac_network_http_receiver_api.h"
#include "resource.h"
#include "../Plugins/General/gen_ml/ml.h"
#include <api/service/svcs/svc_imgload.h>
#include "JSAPI2_CallbackManager.h"

#define SCRIPT_E_REPORTED 	0x80020101

#define SIMULTANEOUS_ASYNCDOWNLOADS 2
std::vector<DownloadToken> asyncDownloads;
Nullsoft::Utility::LockGuard asyncDownloadsLock;


bool IsImage(const wchar_t *filename)
{
	FOURCC imgload = svc_imageLoader::getServiceType();
	int n = (int) WASABI_API_SVC->service_getNumServices(imgload);
	for (int i=0; i<n; i++)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgload,i);
		if (sf)
		{	
			svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
			if (l)
			{
				if (l->isMine(filename))
				{
					sf->releaseInterface(l);
					return true;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return false;
}

bool IsPlaylist(const wchar_t *filename)
{
	if (!AGAVE_API_PLAYLISTMANAGER || !AGAVE_API_PLAYLISTMANAGER->CanLoad(filename))
		return false;
	return true;
}

bool IsMedia( const wchar_t *filename )
{
	int a = 0;
	if ( !in_setmod_noplay( filename, &a ) )
	{
		return false;
	}
	return true;
}

namespace JSAPI2
{
	class AsyncDownloaderAPICallback : public ifc_downloadManagerCallback
	{
	public:
		AsyncDownloaderAPICallback( const wchar_t *url, const wchar_t *destination_filepath, const wchar_t *onlineServiceId, const wchar_t *onlineServiceName )
		{
			this->hFile = INVALID_HANDLE_VALUE;
			this->url = _wcsdup( url );
			this->destination_filepath = _wcsdup( destination_filepath );
			this->onlineServiceId = _wcsdup( onlineServiceId );
			if ( onlineServiceName )
				this->onlineServiceName = _wcsdup( onlineServiceName );
			else
				this->onlineServiceName = NULL;
			this->totalSize = 0;
			this->downloaded = 0;
			ref_count = 1;
		}
		
		void OnInit(DownloadToken token)
		{
			callbackManager.OnInit(this->url, this->onlineServiceId);
		}

		void OnConnect(DownloadToken token)
		{
			// ---- retrieve total size
			api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
			if (http) 
			{
				this->totalSize = http->content_length();
			}

			// ---- create file handle
			hFile = CreateFileW(destination_filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if ( hFile == INVALID_HANDLE_VALUE ) 
			{
				WAC_API_DOWNLOADMANAGER->CancelDownload(token);
			}

			callbackManager.OnConnect(this->url, this->onlineServiceId);
		}

		void OnData(DownloadToken token, void *data, size_t datalen)
		{
			// ---- OnConnect copied here due to dlmgr OnData called first
			// ---- retrieve total size
			api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
			if ( !this->totalSize && http ) 
			{
				this->totalSize = http->content_length();
			}

			if ( hFile == INVALID_HANDLE_VALUE )
			{
				// ---- create file handle
				hFile = CreateFileW(destination_filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if ( hFile == INVALID_HANDLE_VALUE ) 
				{
					WAC_API_DOWNLOADMANAGER->CancelDownload(token);
					return;
				}		
			}
			// ---- OnConnect to be removed once dlmgr is fixed

			// ---- OnData
			// ---- if file handle is invalid, then cancel download
			if ( hFile == INVALID_HANDLE_VALUE )
			{
				WAC_API_DOWNLOADMANAGER->CancelDownload(token);
				return;
			}

			this->downloaded = (size_t)WAC_API_DOWNLOADMANAGER->GetBytesDownloaded(token);

			if ( datalen > 0 ) 
			{
				// ---- hFile is valid handle, and write to disk
				DWORD numWritten = 0;
				WriteFile(hFile, data, (DWORD)datalen, &numWritten, FALSE);
				
				// ---- failed writing the number of datalen characters, cancel download
				if (numWritten != datalen) 
				{
					WAC_API_DOWNLOADMANAGER->CancelDownload(token);
					return;
				}
			}
		
			// TODO: if killswitch is turned on, then cancel download
			//if ( downloadStatus.UpdateStatus(p_token, this->downloaded, this->totalSize) )
			//{
			//	WAC_API_DOWNLOADMANAGER->CancelDownload(p_token);
			//}

			callbackManager.OnData(url, this->downloaded, this->totalSize, this->onlineServiceId);
		}

		void OnCancel( DownloadToken p_token )
		{
			if ( hFile != INVALID_HANDLE_VALUE )
			{
				CloseHandle( hFile );
				DeleteFileW( destination_filepath );
			}

			this->resumeNextPendingDownload( p_token );

			callbackManager.OnCancel( url, this->onlineServiceId );

			this->Release();
		}

		void OnError(DownloadToken p_token, int error)
		{
			if ( hFile != INVALID_HANDLE_VALUE ) 
			{
				CloseHandle(hFile);
				DeleteFileW(destination_filepath);
			}

			this->resumeNextPendingDownload( p_token );

			callbackManager.OnError(url, error, this->onlineServiceId);
			
			this->Release();
		}

		void OnFinish( DownloadToken p_token )
		{
			if ( hFile != INVALID_HANDLE_VALUE )
			{
				CloseHandle( hFile );

				if ( IsMedia( PathFindFileNameW( destination_filepath ) ) )
				{
					LMDB_FILE_ADD_INFOW fi = { const_cast<wchar_t *>( destination_filepath ), -1, -1 };
					sendMlIpc( ML_IPC_DB_ADDORUPDATEFILEW, (WPARAM)&fi );
					sendMlIpc( ML_IPC_DB_SYNCDB, 0 );
				}
			}

			this->resumeNextPendingDownload( p_token );


			callbackManager.OnFinish( url, destination_filepath, this->onlineServiceId );

			this->Release();
		}


		int GetSource( wchar_t *source, size_t source_cch )
		{
			if ( this->onlineServiceName )
				return wcscpy_s( source, source_cch, this->onlineServiceName );
			else
				return 1;
		}

		int GetTitle( wchar_t *title, size_t title_cch )
		{
			return wcscpy_s( title, title_cch, PathFindFileNameW( this->destination_filepath ) );
		}

		int GetLocation( wchar_t *location, size_t location_cch )
		{
			return wcscpy_s( location, location_cch, this->destination_filepath );
		}


		size_t AddRef()
		{
			return InterlockedIncrement( (LONG *)&ref_count );
		}

		size_t Release()
		{
			if ( ref_count == 0 )
				return ref_count;

			LONG r = InterlockedDecrement( (LONG *)&ref_count );
			if ( r == 0 )
				delete( this );

			return r;
		}

	private: // private destructor so no one accidentally calls delete directly on this reference counted object
		~AsyncDownloaderAPICallback()
		{
			if ( url )
				free( url );

			if ( destination_filepath )
				free( destination_filepath );

			if ( onlineServiceId )
				free( onlineServiceId );

			if ( onlineServiceName )
				free( onlineServiceName );
		}

		inline void resumeNextPendingDownload( DownloadToken p_token )
		{
			{
				Nullsoft::Utility::AutoLock lock( asyncDownloadsLock );

				size_t l_index = 0;
				for ( DownloadToken &l_download_token : asyncDownloads )
				{
					if ( l_download_token == p_token )
					{
						asyncDownloads.erase( asyncDownloads.begin() + l_index );
						break;
					}

					++l_index;
				}
			}

			for ( DownloadToken &l_download_token : asyncDownloads )
			{
				if ( WAC_API_DOWNLOADMANAGER->IsPending( l_download_token ) )
				{
					WAC_API_DOWNLOADMANAGER->ResumePendingDownload( l_download_token );
					break;
				}
			}
		}

	protected:
		RECVS_DISPATCH;

	private:
		HANDLE   hFile;
		wchar_t *url;
		wchar_t *destination_filepath;
		wchar_t *onlineServiceId;
		wchar_t *onlineServiceName;
		size_t   totalSize;
		size_t   downloaded;
		LONG     ref_count;
	};
}

#define CBCLASS JSAPI2::AsyncDownloaderAPICallback
START_DISPATCH;
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONFINISH,    OnFinish )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONCANCEL,    OnCancel )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONERROR,     OnError )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONDATA,      OnData )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONCONNECT,   OnConnect )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONINIT,      OnInit )
CB(  IFC_DOWNLOADMANAGERCALLBACK_GETSOURCE,   GetSource )
CB(  IFC_DOWNLOADMANAGERCALLBACK_GETTITLE,    GetTitle )
CB(  IFC_DOWNLOADMANAGERCALLBACK_GETLOCATION, GetLocation )
CB(  ADDREF,  AddRef )
CB(  RELEASE, Release )
END_DISPATCH;
	#undef CBCLASS

JSAPI2::AsyncDownloaderAPI::AsyncDownloaderAPI(const wchar_t *_key, JSAPI::ifc_info *_info)
{
	info = _info;
	key = _key;
	refCount = 1;
}

JSAPI2::AsyncDownloaderAPI::~AsyncDownloaderAPI()
{
	// just in case someone forgot
	JSAPI2::callbackManager.Deregister(this);

	size_t index = events.size();
	while(index--)
	{
		IDispatch *pEvent = events[index];
		if (NULL != pEvent)
			pEvent->Release();
	}
}


#define DISP_TABLE \
	CHECK_ID(DownloadMedia)\
	CHECK_ID(RegisterForEvents)\
	CHECK_ID(UnregisterFromEvents)\

#define CHECK_ID(str) JSAPI_DISP_ENUMIFY(str),
enum { 
	DISP_TABLE 
};

#undef CHECK_ID
#define CHECK_ID(str)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L## #str, -1))\
		{	rgdispid[i] = JSAPI_DISP_ENUMIFY(str); continue; }

HRESULT JSAPI2::AsyncDownloaderAPI::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		DISP_TABLE;

		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT JSAPI2::AsyncDownloaderAPI::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::AsyncDownloaderAPI::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

int CALLBACK WINAPI BrowseCallbackProc_Download(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		SendMessageW(hwnd, BFFM_SETSELECTIONW, 1, (LPARAM)lpData);

		// this is not nice but it fixes the selection not working correctly on all OSes
		EnumChildWindows(hwnd, browseEnumProc, 0);
	}
	if (uMsg == WM_CREATE) SetWindowTextW(hwnd,getStringW(IDS_SELDOWNLOADDIR,NULL,0));
	return 0;
}

void GetPathToStore(wchar_t path_to_store[MAX_PATH]);
bool GetOnlineDownloadPath(const wchar_t *key, const wchar_t *svcname, wchar_t path_to_store[MAX_PATH])
{
	//retrieve online service specific download path
	GetPrivateProfileStringW(key,L"downloadpath",NULL,path_to_store,MAX_PATH,JSAPI2_INIFILE);

	//if found then return, otherwise allow user to specify
	if (path_to_store && path_to_store[0]) return true;
	
	//default music folder
	GetPathToStore(path_to_store);

	//popup dialog to allow user select and specify online service download path
	BROWSEINFOW bi={0};
	wchar_t name[MAX_PATH] = {0};
	wchar_t title[256] = {0};
	bi.hwndOwner = g_dialog_box_parent?g_dialog_box_parent:hMainWindow;
	bi.pszDisplayName = name;
	StringCchPrintfW(title,256,getStringW(IDS_ONLINESERVICE_SELDOWNLOADDIR, 0, 0),svcname);
	bi.lpszTitle = title;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCallbackProc_Download;
	bi.lParam = (LPARAM)path_to_store;
	ITEMIDLIST *idlist = SHBrowseForFolderW(&bi);
	if (idlist)
	{
		SHGetPathFromIDListW(idlist, path_to_store);
		Shell_Free(idlist);
		WritePrivateProfileStringW(key,L"downloadpath",path_to_store,JSAPI2_INIFILE);
		return true;
	}
	
	return false;
}

void CleanNameForPath(wchar_t *name)
{

	while (name && *name)
	{
		switch(*name)
		{
		case L'?':
		case L'*':
		case  L'|':
			*name = L'_';
			break;
		case '/':
		case L'\\':
		case L':':
			*name =  L'-';
			break;
		case L'\"': 
			*name  = L'\'';
			break;
		case L'<':
			*name  = L'(';
			break;
		case L'>': *name = L')';
			break;
		}
		name++;			
	}
}

HRESULT JSAPI2::AsyncDownloaderAPI::DownloadMedia(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 1, 3);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr); //url
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 2, VT_BSTR, puArgErr); //destination file
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 3, VT_BOOL, puArgErr); //add to media library or not

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	if (security.GetActionAuthorization(L"downloader", L"downloadmedia", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);

		const wchar_t *url = JSAPI_PARAM(pdispparams, 1).bstrVal;
		wchar_t *destFileSpec=JSAPI_PARAM_OPTIONAL(pdispparams, 2, bstrVal, PathFindFileNameW(url));
		//filter reserved characters in file name
		CleanNameForPath(destFileSpec);

		// verify that passed-in URL is a valid media type
		if (!url || !destFileSpec || (!IsImage(destFileSpec) && !IsPlaylist(destFileSpec) && !IsMedia(destFileSpec)))
		{
			JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
			return S_OK;
		}

		wchar_t path_to_store[MAX_PATH] = {0};
		if (GetOnlineDownloadPath(this->key, this->info->GetName(), path_to_store))
		{
			CreateDirectoryW(path_to_store, NULL);

			wchar_t destfile[MAX_PATH] = {0};
			PathCombineW(destfile, path_to_store, destFileSpec);

			JSAPI2::AsyncDownloaderAPICallback *callback = new JSAPI2::AsyncDownloaderAPICallback(url, destfile, key, this->info->GetName());
			{
				Nullsoft::Utility::AutoLock lock(asyncDownloadsLock);
				if (asyncDownloads.size() < SIMULTANEOUS_ASYNCDOWNLOADS) 
				{
					DownloadToken dt = WAC_API_DOWNLOADMANAGER->DownloadEx(AutoChar(url), callback, api_downloadManager::DOWNLOADEX_CALLBACK | api_downloadManager::DOWNLOADEX_UI);
					asyncDownloads.push_back(dt);
				}
				else
				{
					DownloadToken dt = WAC_API_DOWNLOADMANAGER->DownloadEx(AutoChar(url), callback, api_downloadManager::DOWNLOADEX_CALLBACK | api_downloadManager::DOWNLOADEX_PENDING | api_downloadManager::DOWNLOADEX_UI);					
					asyncDownloads.push_back(dt);
				}
			}
		}
		else
		{
			JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
		}
	}
	else
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	}
	return S_OK;
}

HRESULT JSAPI2::AsyncDownloaderAPI::RegisterForEvents(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_DISPATCH, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	switch (security.GetActionAuthorization(L"downloader", L"events", key, info, JSAPI2::api_security::ACTION_PROMPT))
	{
	case JSAPI2::api_security::ACTION_DISALLOWED:
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
		break;
	case JSAPI2::api_security::ACTION_ALLOWED:
		{
			/** if this is the first time someone is registering an event
			** add ourselves to the callback manager
			*/
			if (events.empty())
				JSAPI2::callbackManager.Register(this);

			IDispatch *event = JSAPI_PARAM(pdispparams, 1).pdispVal;
			event->AddRef();
			// TODO: benski> not sure, but we might need to: event->AddRef(); 
			events.push_back(event);
			JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
		}
		break;
	}
	return S_OK;
}

HRESULT JSAPI2::AsyncDownloaderAPI::UnregisterFromEvents(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_DISPATCH, puArgErr);

	IDispatch *event = JSAPI_PARAM(pdispparams, 1).pdispVal;
	// TODO: benski> not sure, but we might need to: event->Release(); 

	size_t index = events.size();
	while(index--)
	{
		if (events[index] == event)
		{
			events.erase(events.begin() + index);
			event->Release();
		}
	}
	
	/** if we don't have any more event listeners
	** remove ourselves from the callback manager
	*/
	if (events.empty())
		JSAPI2::callbackManager.Deregister(this);

	return S_OK;
}

#undef CHECK_ID
#define CHECK_ID(str) 		case JSAPI_DISP_ENUMIFY(str): return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT JSAPI2::AsyncDownloaderAPI::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		DISP_TABLE
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JSAPI2::AsyncDownloaderAPI::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = (IDispatch *)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG JSAPI2::AsyncDownloaderAPI::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}


ULONG JSAPI2::AsyncDownloaderAPI::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}


void JSAPI2::AsyncDownloaderAPI::InvokeEvent(const wchar_t *eventName, JSAPI::CallbackParameters::PropertyTemplate *parameters, size_t parametersCount)
{
	size_t index  = events.size();
	if (0 == index) 
	{
		JSAPI2::callbackManager.Deregister(this);
		return;
	}

	JSAPI::CallbackParameters *eventData= new JSAPI::CallbackParameters;
	if (NULL == eventData) return;
		
	eventData->AddString(L"event", eventName);

	if (NULL != parameters && 0 != parametersCount)
		eventData->AddPropertyIndirect(parameters, parametersCount);
	
	HRESULT hr;
	while (index--)
	{
		IDispatch *pEvent = events[index];
		if (NULL != pEvent)
		{
			hr = JSAPI::InvokeEvent(eventData, pEvent);
			if (FAILED(hr) && SCRIPT_E_REPORTED != hr)
			{				
				events.erase(events.begin() + index);
				pEvent->Release();
			}
		}
	}

	if (events.empty())
		JSAPI2::callbackManager.Deregister(this);

	eventData->Release();
}


void JSAPI2::AsyncDownloaderAPI::OnInit(const wchar_t *url)
{
	JSAPI::CallbackParameters::PropertyTemplate parameter = 
			{JSAPI::CallbackParameters::typeString, L"url", (ULONG_PTR)url};
		
	InvokeEvent(L"OnInit", &parameter, 1);
}


void JSAPI2::AsyncDownloaderAPI::OnConnect(const wchar_t *url)
{
	JSAPI::CallbackParameters::PropertyTemplate parameter = 
			{JSAPI::CallbackParameters::typeString, L"url", (ULONG_PTR)url};
		
	InvokeEvent(L"OnConnect", &parameter, 1);
}


void JSAPI2::AsyncDownloaderAPI::OnData(const wchar_t *url, size_t downloadedlen, size_t totallen)
{
	JSAPI::CallbackParameters::PropertyTemplate parameter[3] = 
		{{JSAPI::CallbackParameters::typeString, L"url", (ULONG_PTR)url},
		{JSAPI::CallbackParameters::typeLong, L"downloadedlen", (ULONG_PTR)downloadedlen},
		{JSAPI::CallbackParameters::typeLong, L"totallen", (ULONG_PTR)totallen}};
		
	InvokeEvent(L"OnData", &parameter[0], 3);
}


void JSAPI2::AsyncDownloaderAPI::OnCancel(const wchar_t *url)
{
	JSAPI::CallbackParameters::PropertyTemplate parameter = 
			{JSAPI::CallbackParameters::typeString, L"url", (ULONG_PTR)url};
		
	InvokeEvent(L"OnCancel", &parameter, 1);
}


void JSAPI2::AsyncDownloaderAPI::OnError(const wchar_t *url, int error)
{
	JSAPI::CallbackParameters::PropertyTemplate parameter[2] = 
		{{JSAPI::CallbackParameters::typeString, L"url", (ULONG_PTR)url},
		{JSAPI::CallbackParameters::typeLong, L"error", (ULONG_PTR)error}};
		
	InvokeEvent(L"OnError", &parameter[0], 2);
}


void JSAPI2::AsyncDownloaderAPI::OnFinish(const wchar_t *url, const wchar_t *destfilename)
{
	JSAPI::CallbackParameters::PropertyTemplate parameter[2] = 
		{{JSAPI::CallbackParameters::typeString, L"url", (ULONG_PTR)url},
		{JSAPI::CallbackParameters::typeString, L"destfilename", (ULONG_PTR)destfilename}};
		
	InvokeEvent(L"OnFinish", &parameter[0], 2);
}

const wchar_t *JSAPI2::AsyncDownloaderAPI::GetKey()
{
	return this->key;
}