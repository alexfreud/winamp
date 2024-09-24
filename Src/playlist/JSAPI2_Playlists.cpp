#include "JSAPI2_Playlists.h"
#include "../Winamp/JSAPI.h"
#include "api__playlist.h"
#include "../Winamp/JSAPI_ObjectArray.h"
#include "Playlists.h"
#include "JSAPI2_Playlist.h"
#include "PlaylistManager.h"
#include "../Winamp/JSAPI_CallbackParameters.h"

extern Playlists playlists;

JSAPI2::PlaylistsAPI::PlaylistsAPI(const wchar_t *_key, JSAPI::ifc_info *_info)
{
	info = _info;
	key  = _key;
}

#define DISP_TABLE \
	CHECK_ID(GetPlaylists)\
	CHECK_ID(OpenPlaylist)\
	CHECK_ID(SavePlaylist)\

#define CHECK_ID(str) JSAPI_DISP_ENUMIFY(str),
enum { 
	DISP_TABLE 
};

#undef CHECK_ID
#define CHECK_ID(str) 		if (wcscmp(rgszNames[i], L## #str) == 0)	{		rgdispid[i] = JSAPI_DISP_ENUMIFY(str); continue; }
HRESULT JSAPI2::PlaylistsAPI::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		DISP_TABLE

			rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT JSAPI2::PlaylistsAPI::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::PlaylistsAPI::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::PlaylistsAPI::GetPlaylists(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);

	VariantInit(pvarResult);
	V_VT(pvarResult) = VT_DISPATCH;
	if (AGAVE_API_JSAPI2_SECURITY->GetActionAuthorization(L"playlists", L"read", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		JSAPI::ObjectArray *objectArray = new JSAPI::ObjectArray;
		playlists.Lock();
		
		size_t count = playlists.GetCount();

		for (size_t i=0;i!=count;i++)
		{
			const PlaylistInfo &info = playlists.GetPlaylistInfo(i);
			JSAPI::CallbackParameters *playlistParams = new JSAPI::CallbackParameters;
			playlistParams->AddString(L"filename", info.filename);
			playlistParams->AddString(L"title", info.title);
			wchar_t guid_str[40];
			nsGUID::toCharW(info.guid, guid_str);
			playlistParams->AddString(L"playlistId", guid_str);
			playlistParams->AddLong(L"length", info.length);
			playlistParams->AddLong(L"numitems", info.numItems);
			objectArray->AddObject(playlistParams);
			playlistParams->Release();
		}
		playlists.Unlock();
		V_DISPATCH(pvarResult) = objectArray;
		return S_OK;
	}
	else
	{
		V_DISPATCH(pvarResult) = 0;
		return S_OK;
	}

	return E_FAIL;		
}

HRESULT JSAPI2::PlaylistsAPI::OpenPlaylist(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);

	VariantInit(pvarResult);
	V_VT(pvarResult) = VT_DISPATCH;
	V_DISPATCH(pvarResult) = 0;
	if (AGAVE_API_JSAPI2_SECURITY->GetActionAuthorization(L"playlists", L"read", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		GUID playlist_guid = nsGUID::fromCharW(JSAPI_PARAM(pdispparams, 1).bstrVal);
		playlists.Lock();
		size_t index;
		if (playlists.GetPosition(playlist_guid, &index) == API_PLAYLISTS_SUCCESS)
		{
			const wchar_t *filename = playlists.GetFilename(index);
			if (filename)
			{
				PlaylistObject *playlist = new PlaylistObject(key);
				if (playlistManager.Load(filename, &playlist->playlist) == PLAYLISTMANAGER_SUCCESS)
				{
					V_DISPATCH(pvarResult) = playlist;
				}
				else
				{
					delete playlist;
				}
			}
		}
		playlists.Unlock();			
		return S_OK;

	}
	else
	{
		return S_OK;
	}

	return E_FAIL;		
}

HRESULT JSAPI2::PlaylistsAPI::SavePlaylist(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 2);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 2, VT_DISPATCH, puArgErr);

	VariantInit(pvarResult);
	V_VT(pvarResult) = VT_BOOL;
	V_BOOL(pvarResult) = FALSE;
	if (AGAVE_API_JSAPI2_SECURITY->GetActionAuthorization(L"playlists", L"write", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		GUID playlist_guid = nsGUID::fromCharW(JSAPI_PARAM(pdispparams, 1).bstrVal);
		playlists.Lock();
		size_t index;
		if (playlists.GetPosition(playlist_guid, &index) == API_PLAYLISTS_SUCCESS)
		{
			const wchar_t *filename = playlists.GetFilename(index);
			if (filename)
			{
				IDispatch *dispPlaylist = JSAPI_PARAM(pdispparams, 2).pdispVal; 
				PlaylistObject *playlist = 0;
				dispPlaylist->QueryInterface(JSAPI2::IID_PlaylistObject, (void **)&playlist);
				if (playlistManager.Save(filename, &(playlist->playlist)) == PLAYLISTMANAGER_SUCCESS)
					V_BOOL(pvarResult) = TRUE;
			}
		}
		playlists.Unlock();			
		return S_OK;

	}
	else
	{
		return S_OK;
	}

	return E_FAIL;		
}

#undef CHECK_ID
#define CHECK_ID(str) 		case JSAPI_DISP_ENUMIFY(str): return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT JSAPI2::PlaylistsAPI::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		DISP_TABLE
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JSAPI2::PlaylistsAPI::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG JSAPI2::PlaylistsAPI::AddRef(void)
{
	return this->_refCount.fetch_add( 1 );
}

ULONG JSAPI2::PlaylistsAPI::Release( void )
{
	std::size_t l_Ref = this->_refCount.fetch_sub( 1 );
	if ( l_Ref == 0 )
		delete this;

	return l_Ref;
}
