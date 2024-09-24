#include "main.h"

#include "./amnwatcher.h"
#include <shlobj.h>
#include <strsafe.h>

#include "playlists.h"
#include "PlaylistManager.h"
#include "PlaylistView.h"

waServiceFactory *AMNWatcher::watcherFactory = NULL;

BOOL IsMusicNowWantUs(void)
{
	HKEY key;
	DWORD value = 0, length = sizeof(DWORD);
	if (ERROR_SUCCESS == RegOpenKeyExW( HKEY_CURRENT_USER, L"Software\\AOL Music Now\\UserPreferences", 0, KEY_QUERY_VALUE, &key ))
	{
		long retCode = RegQueryValueExW(key, L"IntegrateWithWinamp", NULL, NULL, (LPBYTE) & value, &length);
		if (ERROR_SUCCESS != retCode) value = TRUE;
		RegCloseKey(key);
	}
	return value;
}

BOOL GetMusicNowPlaylistPath(LPWSTR *pathNew, LPWSTR *pathOld) // CAUTION!!! this function will allocate memory for the string using malloc
{
	// first check rgistry
	HKEY key;
	*pathNew = NULL;
	*pathOld = NULL;

	if (ERROR_SUCCESS == RegOpenKeyExW( HKEY_CURRENT_USER, L"Software\\AOL Music Now\\UserPreferences", 0, KEY_QUERY_VALUE, &key ))
	{
		const wchar_t *rPath = L"PlaylistDirectory";
		DWORD length;
		if (ERROR_SUCCESS == RegQueryValueExW(key, rPath, NULL, NULL, NULL, &length)) // try registry ( for future)
		{
			*pathNew = (LPWSTR)malloc(length);
			if (ERROR_SUCCESS != RegQueryValueExW(key, rPath, NULL, NULL, (LPBYTE)*pathNew, &length)) { free(*pathNew); *pathNew = NULL; }
		}
		RegCloseKey(key);
		
		if (*pathNew && !PathFileExists(*pathNew)) { free(*pathNew); *pathNew = NULL; } //check that path is actualy exist
		
		LPITEMIDLIST pidl;
		if (S_OK == SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl))
		{
			wchar_t path[MAX_PATH*4] = {0};
			if(SHGetPathFromIDListW(pidl, path))
			{
				const wchar_t *suffix = L"Music Now Playlists";
				int cchLen = lstrlenW(path) + lstrlenW(suffix) + 32; 
				if (!*pathNew) // no luck - use deafult's
				{
					*pathNew = (LPWSTR)malloc(cchLen*sizeof(wchar_t));
					StringCchPrintfW(*pathNew, cchLen, L"%s\\%s", path, suffix);
					if (!PathFileExists(*pathNew)) {free(*pathNew); *pathNew = NULL; }// i give up - can't find anything
				}
				// now let's try old path
				*pathOld = (LPWSTR)malloc(cchLen*sizeof(wchar_t));
				StringCchPrintfW(*pathOld, cchLen, L"%s\\AOL %s", path, suffix);
				if (!PathFileExists(*pathOld)) { free(*pathOld); *pathOld = NULL; }
			}
			CoTaskMemFree(pidl);
		}
	} 
	return (NULL != *pathNew) || (NULL != *pathOld);
}

AMNWatcher::AMNWatcher(void) : watcherOld(NULL), watcherNew(NULL), dirty(FALSE)
{}

AMNWatcher::~AMNWatcher(void)
{
	//Destroy(); // have to just let it leak. you should call Destroy() manually
}

int AMNWatcher::Init(api_service *service, const wchar_t *trackPath)
{
	BOOL retCode = FALSE;
	if (!IsMusicNowWantUs()) return retCode;
	if (watcherFactory) return retCode;
	
	LPWSTR pathNew, pathOld;
	if (!GetMusicNowPlaylistPath(&pathNew, &pathOld)) return retCode;
	int cchLen = lstrlenW(pathNew);
	if (cchLen > 1 && pathNew[cchLen -1] == L'\\') pathNew[cchLen -1] = 0x00;
	cchLen = lstrlenW(pathOld);
	if (cchLen > 1 && pathOld[cchLen -1] == L'\\') pathOld[cchLen -1] = 0x00;
 
	watcherFactory = service->service_getServiceByGuid(watcherGUID);
	if (watcherFactory)
	{
		if (pathNew) 
		{
			watcherNew = (api_watcher*)watcherFactory->getInterface();
			watcherNew->Create( L"WAMN_PLS_NEW", pathNew, TRUE, WATCHER_TRACKMODE_CENTRAL, OnWatcherNotify);
		}
		if (pathOld)
		{
			watcherOld = (api_watcher*)watcherFactory->getInterface();
			watcherOld->Create( L"WAMN_PLS_OLD", pathOld, TRUE, WATCHER_TRACKMODE_CENTRAL, OnWatcherNotify);
		}
		for (int i = 0; i < 2; i++)
		{
			api_watcher *watcher = (i) ? watcherOld : watcherNew;
			if (!watcher) continue;

			watcher->SetExtensionFilter(L"WPL", WATCHER_FILTERTYPE_INCLUDE);
			watcher->SetUserData(this);
			watcher->SetCallBack(OnWatcherNotify);
			watcher->SetTrackingPath(trackPath);
			watcher->Start();
			watcher->ForceScan(FALSE, SCANPRIORITY_IDLE);
		}
	}
	free(pathNew);
	free(pathOld);
	return TRUE;
}

void AMNWatcher::Destroy(void)
{
	if (watcherNew)
	{
		watcherNew->Stop();
		watcherFactory->releaseInterface(watcherNew);
		watcherNew = NULL;
	}
	if (watcherOld)
	{
		watcherOld->Stop();
		watcherFactory->releaseInterface(watcherOld);
		watcherOld = NULL;
	}
}

int AMNWatcher::OnWatcherNotify(api_watcher *sender, UINT message, LONG_PTR param, void* userdata)
{
	UNREFERENCED_PARAMETER(sender);

	AMNWatcher *watcher = (AMNWatcher*)userdata;
	if (WATCHER_MSG_FILE == message)
	{
		WATCHERCHANGEINFO *info = (WATCHERCHANGEINFO*)param;
		
		// ignore some names
		if (info->cchFile >= 21) // can be "AOL Music Now - Auido.wpl" or "AOL Music Now - Video.wpl"
		{
			const wchar_t *testname = info->file + info->cchFile - 21;
			if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, NULL, testname, 9, L"Music Now", 9)) return 1;
		}
		else if ( 17 == info->cchFile && CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, NULL, info->file, 13, L"AOL Music Now", 13)) return 1;
		
		wchar_t fullName[MAX_PATH*4];
		PathCombineW(fullName, info->path, info->file);

		size_t i(0);

		switch (info->state)
		{
		case WATCHER_FILESTATE_ADDED:
		case WATCHER_FILESTATE_CHANGED:
			{
				if (!info->file)
					break;
				wchar_t *title = _wcsdup(info->file);
				unsigned int len = lstrlen(title);
				while (len && title[len] != '.') len--;
				if (len != 0) title[len] = 0x00;
				while (i != playlists.size() && lstrcmpW(fullName, playlists.at(i).filename)) i++;
				if ( i == playlists.size())
				{
					AddPlaylist(title, fullName, TRUE, playlistManager->CountItems(fullName));
					watcher->dirty = TRUE;
				}
				else
				{
					int length = playlistManager->GetLengthMilliseconds(fullName);
					int numItems = (int)playlistManager->CountItems(fullName);
					if (playlists.at(i).length != length || playlists.at(i).numItems != numItems)
					{
						StringCchCopy(playlists.at(i).title, 1024, title);
						playlists.at(i).length = length;
						playlists.at(i).numItems = numItems;

						HWND hwnd = (HWND) SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, 0, ML_IPC_GETCURRENTVIEW);
						if (hwnd)
						{
							wchar_t* title = (wchar_t*)GetPropW(hwnd, L"TITLE");
							if (title && 0 == lstrcmp(title, playlists[i].title)) SendMessage(hwnd, WM_PLAYLIST_RELOAD, 0, 0);
						}
						watcher->dirty = TRUE;
					}

				}
				free(title);
			}
			break;
		case WATCHER_FILESTATE_REMOVED:
			{
				for ( i = 0; i < playlists.size(); i++)
				{
					if (0 == lstrcmpW(fullName, playlists.at(i).filename))
					{
						HWND hwnd = (HWND) SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, 0, ML_IPC_GETCURRENTVIEW);
						if (hwnd)
						{
							wchar_t* title = (wchar_t *)GetPropW(hwnd, L"TITLE");
							if (title && 0 == lstrcmp(title, playlists[i].title)) SendMessage(hwnd, WM_PLAYLIST_UNLOAD, 0, 0);
						}
						if (playlists.at(i).treeId) 
							mediaLibrary.RemoveTreeItem(playlists.at(i).treeId);
						playlists.eraseAt(i);
						i--;
						watcher->dirty = TRUE;
						break;
					}
				}
			}
			break;
		}
	}
	else if (WATCHER_MSG_STATUS == message)
	{
		if (STATUS_SCANER_STOPPED == param || STATUS_SCANER_FINISHED == param)
		{
			if (watcher->dirty)
			{
				SavePlaylists();
				RefreshPlaylistsList();
				watcher->dirty = FALSE;
			}

		}
	}


	return 1;
}
