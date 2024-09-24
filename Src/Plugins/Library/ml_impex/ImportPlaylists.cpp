#include "api__ml_impex.h"
#include "../xml/obj_xml.h"
#include <map>
#include <bfc/string/url.h>
#include "importer.h"
#include "resource.h"
#include "../plist/loader.h"
#include "../playlist/ifc_playlist.h"
#include "../Winamp/wa_ipc.h"
#include <shlwapi.h>

struct iTunesFileInfo
{
	iTunesFileInfo(const wchar_t *_filename, uint64_t _length)
	{
		filename = _wcsdup(_filename);
		length = _length;
	}
	~iTunesFileInfo()
	{
		free(filename);
	}
	wchar_t *filename;
	uint64_t length;
};
typedef std::map<int64_t, iTunesFileInfo*> FilesList;
extern winampMediaLibraryPlugin plugin;
int Load(const wchar_t *filename, obj_xml *parser);

class PlistPlaylist : public ifc_playlist
{
public:
	PlistPlaylist(const plistArray *_items, FilesList &_files) : items(_items), files(_files)
	{
		length_sum = 0;
	}
	size_t GetNumItems();
	size_t GetItem(size_t item, wchar_t *filename, size_t filenameCch);
	int GetItemLengthMilliseconds(size_t item);
	uint64_t length_sum;
protected:
	RECVS_DISPATCH;
	const plistArray *items;
	FilesList &files;	
};

size_t PlistPlaylist::GetNumItems()
{
	return items->getNumItems();
}

size_t PlistPlaylist::GetItem(size_t item, wchar_t *filename, size_t filenameCch)
{
	plistDict *item_dict = (plistDict *)items->enumItem((int)item);
	if (item_dict)
	{
		plistKey *id_key = item_dict->getKey(L"Track ID");
		if (id_key)
		{
			plistInteger *id_data = (plistInteger *)id_key->getData();
			if (id_data)
			{
				int64_t key = id_data->getValue();
				iTunesFileInfo *info = files[key];
				if (info)
				{
					const wchar_t *track_name = info->filename;
					if (track_name)
					{	
						length_sum += info->length;
						StringCchCopyW(filename, filenameCch, track_name);
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

int PlistPlaylist::GetItemLengthMilliseconds(size_t item)
{
 	plistDict *item_dict = (plistDict *)items->enumItem((int)item);
	if (item_dict)
	{
		plistKey *id_key = item_dict->getKey(L"Track ID");
		if (id_key)
		{
			plistInteger *id_data = (plistInteger *)id_key->getData();
			if (id_data)
			{
				int64_t key = id_data->getValue();
				iTunesFileInfo *info = files[key];
				if (info)
				{					
					return (int)info->length;				
				}
			}
		}
	}
	return 0;
}

#define CBCLASS PlistPlaylist
START_DISPATCH;
CB(IFC_PLAYLIST_GETNUMITEMS, GetNumItems)
CB(IFC_PLAYLIST_GETITEM, GetItem)
CB(IFC_PLAYLIST_GETITEMLENGTHMILLISECONDS, GetItemLengthMilliseconds)
END_DISPATCH;
#undef CBCLASS

static bool GetInteger(const plistDict *dict, const wchar_t *key_name, int64_t *int_val)
{
	plistKey *key = dict->getKey(key_name);
	if (!key)
		return false;

	plistData *data = key->getData();
	if (!data)
		return false;

	if (data->getType() != PLISTDATA_INTEGER)
		return false;

	plistInteger *data_int = static_cast<plistInteger *>(data);
	*int_val = data_int->getValue();
	return true;
}

static bool GetString(const plistDict *dict, const wchar_t *key_name, const wchar_t **str_val)
{
	plistKey *key = dict->getKey(key_name);
	if (!key)
		return false;

	plistData *data = key->getData();
	if (!data)
		return false;

	if (data->getType() != PLISTDATA_STRING)
		return false;

	plistString *data_str = static_cast<plistString *>(data);
	*str_val = data_str->getString();
	return true;
}

static bool GetArray(const plistDict *dict, const wchar_t *key_name, plistArray **array_val)
{
	plistKey *key = dict->getKey(key_name);
	if (!key)
		return false;

	plistData *data = key->getData();
	if (!data)
		return false;

	if (data->getType() != PLISTDATA_ARRAY)
		return false;

	*array_val = static_cast<plistArray *>(data);
	return true;
}

static bool CheckDuplicatePlaylist(uint64_t playlist_id64, GUID &dup_guid)
{
	AGAVE_API_PLAYLISTS->Lock();
	size_t numPlaylists = AGAVE_API_PLAYLISTS->GetCount();
	uint64_t compare_id64=0;
	for (size_t i=0;i!=numPlaylists;i++)
	{
		if (AGAVE_API_PLAYLISTS->GetInfo(i, api_playlists_iTunesID, &compare_id64, sizeof(compare_id64)) == API_PLAYLISTS_SUCCESS)
		{
			if (compare_id64 == playlist_id64)
			{
				dup_guid = AGAVE_API_PLAYLISTS->GetGUID(i);
				AGAVE_API_PLAYLISTS->Unlock();
				return true;
			}
		}
	}
	AGAVE_API_PLAYLISTS->Unlock();
	return false;
}

enum
{
	DUPLICATE_PLAYLIST_SKIP,
	DUPLICATE_PLAYLIST_REPLACE,
	DUPLICATE_PLAYLIST_NEW,
};

static int PromptReplaceSkipNew(GUID &dup_guid)
{
	/* TODO: 
	* get name and stuff from api_playlists
	* we'll need an HWND for the UI
	* we'll need some passed-in state variable to remember "do for all" choice
	*/
	return DUPLICATE_PLAYLIST_SKIP;
}

HINSTANCE cloud_hinst = 0;
int IPC_GET_CLOUD_HINST = -1, IPC_GET_CLOUD_ACTIVE = -1;
int cloudAvailable()
{
	if (IPC_GET_CLOUD_HINST == -1) IPC_GET_CLOUD_HINST = (INT)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"WinampCloud", IPC_REGISTER_WINAMP_IPCMESSAGE);
	if (IPC_GET_CLOUD_ACTIVE == -1) IPC_GET_CLOUD_ACTIVE = (INT)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"WinampCloudActive", IPC_REGISTER_WINAMP_IPCMESSAGE);
	if (!cloud_hinst) cloud_hinst = (HINSTANCE)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_HINST);

	return (/*0/*/!(!cloud_hinst || cloud_hinst == (HINSTANCE)1 || !SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_ACTIVE))/**/);
}

static void AddPlaylist(plistDict *playlist,  FilesList &files)
{
	const wchar_t *name;
	const wchar_t *playlist_persistent_id;
	int64_t playlist_id;
	int64_t visible;
	plistArray *items;
	uint64_t playlist_id64;

	if (GetString(playlist, L"Name", &name) 
		&& GetArray(playlist, L"Playlist Items", &items) 
		&& GetInteger(playlist, L"Playlist ID", &playlist_id)
		&& GetString(playlist, L"Playlist Persistent ID", &playlist_persistent_id)
		&& (!GetInteger(playlist, L"Visible", &visible) || visible))
	{
		playlist_id64 =_wcstoui64(playlist_persistent_id, 0, 16);
		// see if it's already in the database

		GUID dup_guid; // so we know the GUID we clash with, in case we want to replace it instead of skip it
		if (playlist_id64 && CheckDuplicatePlaylist(playlist_id64, dup_guid))
		{
			switch(PromptReplaceSkipNew(dup_guid))
			{
			case DUPLICATE_PLAYLIST_SKIP:
				break;
			case DUPLICATE_PLAYLIST_REPLACE:
				// TODO
				break;
			case DUPLICATE_PLAYLIST_NEW:
				// TODO
				break;
			}
		}
		else
		{		
			PlistPlaylist plist_playlist(items, files);

			const wchar_t *user_folder = WASABI_API_APP->path_getUserSettingsPath();
			wchar_t destination[MAX_PATH] = {0};
			PathCombineW(destination, user_folder, L"plugins\\ml\\playlists");

			wchar_t playlist_filename[MAX_PATH] = {0};
			StringCbPrintfW(playlist_filename, sizeof(playlist_filename), L"i_%I64u.m3u8", playlist_id);
			PathAppendW(destination, playlist_filename);

			static wchar_t ml_ini_file[MAX_PATH] = {0};
			if (!ml_ini_file[0]) lstrcpynW(ml_ini_file, (const wchar_t*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETMLINIFILEW), MAX_PATH);
			size_t cloud = (cloudAvailable() ? GetPrivateProfileIntW(L"gen_ml_config", L"cloud_always", 1, ml_ini_file) : 0);

			AGAVE_API_PLAYLISTMANAGER->Save(destination, &plist_playlist);
			AGAVE_API_PLAYLISTS->Lock();
			int new_index = (!cloud ? (int)AGAVE_API_PLAYLISTS->AddPlaylist(destination, name) : (int)AGAVE_API_PLAYLISTS->AddCloudPlaylist(destination, name));
			if (new_index >= 0)
			{
				uint32_t numItems = (uint32_t)plist_playlist.GetNumItems();
				uint64_t totalLength = plist_playlist.length_sum/1000;
				AGAVE_API_PLAYLISTS->SetInfo(new_index, api_playlists_totalTime, &totalLength, sizeof(totalLength));
				AGAVE_API_PLAYLISTS->SetInfo(new_index, api_playlists_itemCount, &numItems, sizeof(numItems));
				if (cloud) AGAVE_API_PLAYLISTS->SetInfo(new_index, api_playlists_cloud, &cloud, sizeof(cloud));
				AGAVE_API_PLAYLISTS->SetInfo(new_index, api_playlists_iTunesID, &playlist_id64, sizeof(playlist_id64));
			}
			AGAVE_API_PLAYLISTS->Unlock();

			PostMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&"ml_playlist_refresh", IPC_REGISTER_WINAMP_IPCMESSAGE));
		}
	}
}

void FixPath(const wchar_t *strdata, StringW &f);

int ImportPlaylists(HWND parent, const wchar_t *library_file)
{
	FilesList files;
	// create an XML parser
	obj_xml *parser=0;
	waServiceFactory *factory = plugin.service->service_getServiceByGuid(obj_xmlGUID);
	if (factory)
		parser = (obj_xml *)factory->getInterface();

	if (parser)
	{
		// create status window
		//HWND hwndDlg = WASABI_API_CREATEDIALOGW(IDD_INFODIALOG,plugin.hwndLibraryParent,import_dlgproc);

		// init it 
		//ShowWindow(hwndDlg, SW_NORMAL);
		//UpdateWindow(hwndDlg);

		// create an iTunes XML library reader
		plistLoader it;

		// load the XML, this creates an iTunes DB in memory, and returns the root key
		parser->xmlreader_open();
		parser->xmlreader_registerCallback(L"plist\f*", &it);
		Load(library_file, parser);
		parser->xmlreader_unregisterCallback(&it);
		parser->xmlreader_close();
		plistKey *root_key = &it;

		// show import progress controls
		//ShowWindow(GetDlgItem(hwndDlg, IDC_PROCESSING_STATE), SW_HIDE);
		//ShowWindow(GetDlgItem(hwndDlg, IDC_PROGRESS_PERCENT), SW_SHOWNORMAL);
		//ShowWindow(GetDlgItem(hwndDlg, IDC_TRACKS), SW_SHOWNORMAL);
		//UpdateWindow(hwndDlg);

		// we start at the root key
		if (root_key) 
		{
			// the root key contains a dictionary
			plistData *root_dict = root_key->getData();
			if (root_dict && root_dict->getType() == PLISTDATA_DICT) 
			{
				// that dictionary contains a number of keys, one of which contains a dictionary of tracks
				plistKey *tracks_key = ((plistDict*)root_dict)->getKey(L"Tracks");
				plistData *tracks_dict = tracks_key?tracks_key->getData():0;
				if (tracks_dict && tracks_dict->getType() == PLISTDATA_DICT) 
				{
					// we have the tracks dictionary ...
					plistDict *tracks = (plistDict *)tracks_dict;
					int n =tracks?tracks->getNumKeys():0;
					// ... now enumerate tracks
					for (int i=0;i<n;i++) 
					{
						// each track is a key in the tracks dictionary, and contains a dictionary of properties
						plistKey *track_key = tracks->enumKey(i);
						plistData *track_dict = track_key->getData();
						// prepare an item record

						if (track_dict->getType() == PLISTDATA_DICT) 
						{
							// we have the track's dictionary of properties...
							plistDict *track = (plistDict *)track_dict;
							int64_t id = 0;
							const wchar_t *location = 0;
							if (GetInteger(track, L"Track ID", &id) && GetString(track, L"Location", &location))
							{
									StringW f;
									FixPath(location, f);

									int64_t length = 0;
									GetInteger(track, L"Total Time", &length);
									// done
									wchar_t *filename = _wcsdup(f);
									files[id] = new iTunesFileInfo(filename, length);

								// show progress
								//SetDlgItemText(hwndDlg, IDC_TRACKS, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_TRACKS_IMPORTED_X), ++count));
								//SendDlgItemMessage(hwndDlg, IDC_PROGRESS_PERCENT, PBM_SETPOS, (int)((double)count/n*100.0), 0);
								//if (count % 10 == 0 || count == n) 
								//									UpdateWindow(hwndDlg);
							}
						}
					}
				}

				// ok we're done building the track list, now let's enumerate the playlists
				plistKey *playlists_key = ((plistDict*)root_dict)->getKey(L"Playlists");
				plistData *playlists_dict = playlists_key?playlists_key->getData():0;
				if (playlists_dict && playlists_dict->getType() == PLISTDATA_ARRAY) 
				{
					plistArray *playlists = (plistArray *)playlists_dict;
					int n =playlists?playlists->getNumItems():0;
					// ... now enumerate playlists
					for (int i=0;i<n;i++) 
					{
						// each playlist is a key in the playlists dictionary, and contains a dictionary of properties
						plistData *playlist_dict = playlists->enumItem(i);
						if (playlist_dict->getType() == PLISTDATA_DICT) 
						{
							// we have the playlist's dictionary of properties...
							plistDict *playlist = (plistDict *)playlist_dict;
							AddPlaylist(playlist, files);
						}
					}
				}
			}
		}
		//DestroyWindow(hwndDlg);
		factory->releaseInterface(parser);
	}
	else
		return DISPATCH_FAILURE;

	FilesList::iterator itr;
	for (itr = files.begin(); itr!= files.end(); itr++)
	{
		iTunesFileInfo *info = itr->second;
		delete info;
	}
	return DISPATCH_SUCCESS;
}