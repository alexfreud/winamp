#include "PlaylistSync.h"
#include "api.h"
#include "../nu/AutoUrl.h"
#include "main.h"
#include "../../..\Components\wac_network\wac_network_http_receiver_api.h"
#include <strsafe.h>

/* classes and utility functions to notifying the device of playlist modifications */
class SimpleCallback : public ifc_downloadManagerCallback
{
public:
	void OnInit(DownloadToken token)
	{
		api_httpreceiver *jnet = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
		if (jnet)
		{
			jnet->AddHeaderValue("X-Winamp-ID", winamp_id_str);
			jnet->AddHeaderValue("X-Winamp-Name", winamp_name);
		}
	}

	RECVS_DISPATCH;
};

#define CBCLASS SimpleCallback
START_DISPATCH;
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONINIT, OnInit)
END_DISPATCH;
#undef CBCLASS

class NewPlaylistCallback : public ifc_downloadManagerCallback
{
public:
	NewPlaylistCallback()
	{
		event = CreateEvent(NULL, TRUE, FALSE, NULL);
		location=0;
	}

	~NewPlaylistCallback()
	{
		CloseHandle(event);
		free(location);
	}

	void OnInit(DownloadToken token)
	{
		api_httpreceiver *jnet = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
		if (jnet)
		{
			jnet->AddHeaderValue("X-Winamp-ID", winamp_id_str);
			jnet->AddHeaderValue("X-Winamp-Name", winamp_name);
		}
	}
	void OnCancel(DownloadToken token)
	{
		SetEvent(event);
	}
	void OnError(DownloadToken token, int error)
	{
		SetEvent(event);
	}
	void OnFinish(DownloadToken token)
	{
		api_httpreceiver *jnet = WAC_API_DOWNLOADMANAGER->GetReceiver(token);
		if (jnet)
		{
			const char *jnet_location = jnet->getheader("Location");
			if (jnet_location)
				location = strdup(jnet_location);
		}
		SetEvent(event);
	}
	const char *Wait()
	{
		WaitForSingleObject(event, INFINITE);
		return location;
	}
	HANDLE event;
	char *location;
	RECVS_DISPATCH;
};

#define CBCLASS NewPlaylistCallback
START_DISPATCH;
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONINIT, OnInit)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONCANCEL, OnCancel)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONERROR, OnInit)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONFINISH, OnFinish)
END_DISPATCH;
#undef CBCLASS

static SimpleCallback simple_callback;
void Sync_AddToPlaylist(const char *root_url, const wchar_t *playlist_id, const wchar_t *song_id)
{
	if ( WAC_API_DOWNLOADMANAGER )
	{
		char url[1024] = {0};
		StringCbPrintfA(url, sizeof(url), "%s/playlist?action=add&id=%s&songid=%s", root_url, AutoUrl(playlist_id), AutoUrl(song_id));
		WAC_API_DOWNLOADMANAGER->DownloadEx(url, &simple_callback, api_downloadManager::DOWNLOADEX_BUFFER);
	}
}

void Sync_RemoveFromPlaylist(const char *root_url, const wchar_t *playlist_id, const wchar_t *song_id)
{
	if ( WAC_API_DOWNLOADMANAGER )
	{
		char url[1024] = {0};
		StringCbPrintfA(url, sizeof(url), "%s/playlist?action=remove&id=%s&songid=%s", root_url, AutoUrl(playlist_id), AutoUrl(song_id));
		WAC_API_DOWNLOADMANAGER->DownloadEx(url, &simple_callback, api_downloadManager::DOWNLOADEX_BUFFER);
	}
}

void Sync_DeletePlaylist(const char *root_url, const wchar_t *playlist_id)
{
	if ( WAC_API_DOWNLOADMANAGER )
	{
		char url[1024] = {0};
		StringCbPrintfA(url, sizeof(url), "%s/playlist?action=delete&id=%s", root_url, AutoUrl(playlist_id));
		WAC_API_DOWNLOADMANAGER->DownloadEx(url, &simple_callback, api_downloadManager::DOWNLOADEX_BUFFER);
	}
}

WifiPlaylist *Sync_NewPlaylist(const char *root_url, const wchar_t *playlist_name)
{
	if ( WAC_API_DOWNLOADMANAGER )
	{
		NewPlaylistCallback new_playlist_callback;
		char url[1024] = {0};
		StringCbPrintfA(url, sizeof(url), "%s/playlist?action=new&name=%s", root_url, AutoUrl(playlist_name));
		WAC_API_DOWNLOADMANAGER->DownloadEx(url, &new_playlist_callback, api_downloadManager::DOWNLOADEX_BUFFER);
		const char *playlist_id = new_playlist_callback.Wait();
		if (playlist_id)
		{
			return new WifiPlaylist(playlist_id, playlist_name);
		}
	}

	return 0;
}

void Sync_RenamePlaylist(const char *root_url, const wchar_t *playlist_id, const wchar_t *playlist_name)
{
	if ( WAC_API_DOWNLOADMANAGER )
	{
		char url[ 1024 ] = { 0 };
		StringCbPrintfA( url, sizeof( url ), "%s/playlist?action=rename&id=%s&name=%s", root_url, AutoUrl( playlist_id ), AutoUrl( playlist_name ) );
		WAC_API_DOWNLOADMANAGER->DownloadEx( url, &simple_callback, api_downloadManager::DOWNLOADEX_BUFFER );
	}
}