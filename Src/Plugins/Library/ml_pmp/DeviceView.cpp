//#define _WIN32_WINNT 0x0400
#include "../Winamp/buildType.h"
#include "main.h"
#include "DeviceView.h"

//#include <commctrl.h>
#include "nu/AutoWide.h"
#include "nu/AutoChar.h"
#include "../nu/AutoUrl.h"
#include "SkinnedListView.h"
#include "../playlist/api_playlistmanager.h"
#include "../playlist/ifc_playlistdirectorycallback.h"
#include "../playlist/ifc_playlistloadercallback.h"
#include "api__ml_pmp.h"
#include <shlwapi.h>
#include <time.h>
#include "metadata_utils.h"
#include "../ml_wire/ifc_podcast.h"
#include "./local_menu.h"
#include "IconStore.h"
#include "../devices/ifc_deviceevent.h"
#include "metadata_utils.h"
#include "../nu/sort.h"
#include "resource1.h"
#include <strsafe.h>
#include "../nu/MediaLibraryInterface.h"

extern C_ItemList devices;
extern C_Config * global_config;

extern INT_PTR CALLBACK pmp_artistalbum_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
extern INT_PTR CALLBACK pmp_playlist_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

int IPC_GET_CLOUD_HINST = -1, IPC_LIBRARY_PLAYLISTS_REFRESH = -1;
HINSTANCE cloud_hinst = 0;
int currentViewedPlaylist=0;
HNAVITEM cloudQueueTreeItem=NULL;
LinkedQueue cloudTransferQueue, cloudFinishedTransfers;
int cloudTransferProgress = 0;
DeviceView * currentViewedDevice=NULL;

volatile size_t TransferContext::paused_all = 0;

extern void UpdateTransfersListView(bool softUpdate, CopyInst * item=NULL);
extern void UpdateDevicesListView(bool softUpdate);
extern INT_PTR CALLBACK config_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
extern HWND mainMessageWindow;
extern prefsDlgRecW prefsPage;
extern int prefsPageLoaded;
static int thread_id;

static bool copySettings(wchar_t * ssrc, wchar_t * sdest);
static __int64 fileSize(wchar_t * filename);
static void removebadchars(wchar_t *s);

extern ThreadID *transfer_thread;
int TransferThreadPoolFunc(HANDLE handle, void *user_data, intptr_t id);

INT_PTR CALLBACK pmp_queue_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK pmp_video_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

DeviceView::DeviceView(Device *dev) 
	: activityRunning(FALSE), navigationItemCreated(FALSE), usedSpace(0), totalSpace(0)
{
	memset(name, 0, sizeof(name));
	queueActiveIcon = isCloudDevice = 0;
	treeItem = videoTreeItem = queueTreeItem = 0;
	connection_type = "USB";
	display_type = "Portable Media Player";
	metadata_fields = (int)dev->extraActions(DEVICE_SUPPORTED_METADATA,0,0,0);
	if(!metadata_fields)
		metadata_fields = -1;
	dev->extraActions(DEVICE_GET_CONNECTION_TYPE, (intptr_t)&connection_type, 0, 0);
	isCloudDevice = (!lstrcmpiA(connection_type, "cloud"));
	dev->extraActions(DEVICE_GET_DISPLAY_TYPE, (intptr_t)&display_type, 0, 0);
	ref_count = 1;
	if (dev->extraActions(DEVICE_GET_UNIQUE_ID, (intptr_t)name, sizeof(name), 0) == 0)
	{
		// fallback
		GUID name_guid;
		CoCreateGuid(&name_guid);
		StringCbPrintfA(name, sizeof(name), "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
			(int)name_guid.Data1, (int)name_guid.Data2, (int)name_guid.Data3,
			(int)name_guid.Data4[0], (int)name_guid.Data4[1],
			(int)name_guid.Data4[2], (int)name_guid.Data4[3],
			(int)name_guid.Data4[4], (int)name_guid.Data4[5],
			(int)name_guid.Data4[6], (int)name_guid.Data4[7] );
	}

	wchar_t inifile[MAX_PATH] = {0};
	dev->extraActions(DEVICE_GET_INI_FILE,(intptr_t)inifile,0,0);
	if(!inifile[0])
	{
		wchar_t name[256] = {0};
		dev->getPlaylistName(0,name,256);
		removebadchars(name);
		// build this slow so we make sure each directory exists
		PathCombine(inifile, WASABI_API_APP->path_getUserSettingsPath(), L"Plugins");
		CreateDirectory(inifile, NULL);
		PathAppend(inifile, L"ml");
		CreateDirectory(inifile, NULL);
		wchar_t ini_filespec[MAX_PATH] = {0};
		StringCchPrintf(ini_filespec, MAX_PATH, L"ml_pmp_device_%s.ini",name);
		PathAppend(inifile, ini_filespec);
	}

	if(fileSize(inifile) <= 0) copySettings(global_config->GetIniFile(),inifile); //  import old settings
	config = new C_Config(inifile,L"ml_pmp",global_config);

	currentTransferProgress = 0;
	transferRate=0;
	commitNeeded=false;
	this->dev = dev;
	wchar_t deviceName[256]=L"";
	dev->getPlaylistName(0,deviceName,sizeof(deviceName)/sizeof(wchar_t));

	if (!isCloudDevice) videoView = config->ReadInt(L"showVideoView",dev->extraActions(DEVICE_SUPPORTS_VIDEO,0,0,0));
	else videoView = 0;

	prefsDlgRecW *parentPrefs = (prefsDlgRecW *)dev->extraActions(DEVICE_GET_PREFS_PARENT, 0, 0, 0);
	if (!parentPrefs)
	{
		// only add it when we're using our own root page, otherwise skip this
		if (!prefsPageLoaded)
		{
			SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(intptr_t)&prefsPage,IPC_ADD_PREFS_DLGW);
		}
		prefsPageLoaded+=1;
	}

	if (lstrcmpi(deviceName, L"all_sources"))
	{
		devPrefsPage.hInst=WASABI_API_LNG_HINST;
		devPrefsPage.where=(parentPrefs ? (intptr_t)parentPrefs : (intptr_t)&prefsPage);
		devPrefsPage.dlgID=IDD_CONFIG;
		devPrefsPage.name=_wcsdup(deviceName);
		devPrefsPage.proc=config_dlgproc;
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(intptr_t)&devPrefsPage,IPC_ADD_PREFS_DLGW);
	}
	else
	{
		memset(&devPrefsPage, 0, sizeof(prefsDlgRecW));
	}

	UpdateSpaceInfo(TRUE, FALSE);

	threadKillswitch = 0;
	transferContext.transfer_thread = WASABI_API_THREADPOOL->ReserveThread(api_threadpool::FLAG_LONG_EXECUTION);
	transferContext.dev = this;
	WASABI_API_THREADPOOL->AddHandle(transferContext.transfer_thread, transferContext.notifier, TransferThreadPoolFunc, &transferContext, thread_id, api_threadpool::FLAG_LONG_EXECUTION);
	thread_id++;

	if (AGAVE_API_DEVICEMANAGER)
	{
		ifc_device *registered_device = this;
		AGAVE_API_DEVICEMANAGER->DeviceRegister(&registered_device, 1);
	}
	//hTransferThread = CreateThread(NULL, 0, ThreadFunc_Transfer, (LPVOID)this, 0, &dwThreadId);
	/*
	if(dev->getDeviceCapacityTotal() > L3000000000) SyncConnectionDefault=1;
	else SyncConnectionDefault=2;
	*/
	SyncConnectionDefault=0; // default off for now.
	if (!isCloudDevice)
	{
		// ok all started. Now do any "on connect" actions...
		time_t lastSync = (time_t)config->ReadInt(L"syncOnConnect_time",0);
		time_t now = time(NULL);
		//int diff = now - lastSync;
		double diff = difftime(now,lastSync);
		config->WriteInt(L"syncOnConnect_time",(int)now);
		if(diff > config->ReadInt(L"syncOnConnect_hours",12)*3600)
		{
			switch(config->ReadInt(L"syncOnConnect",SyncConnectionDefault))
			{
				case 1:
				{
					if (!isCloudDevice) Sync(true);
					//else CloudSync(true);
				}
				break;
				case 2: Autofill(); break;
			}
		}
	}
	if (!AGAVE_API_DEVICEMANAGER)
		RegisterViews(0);
}
HNAVITEM GetNavigationRoot(BOOL forceCreate);
HNAVITEM NavigationItem_Find(HNAVITEM root, const wchar_t *name, BOOL allow_root = 0);

void DeviceView::RegisterViews(HNAVITEM parent)
{
	NAVINSERTSTRUCT nis = {0};
	NAVITEM *item = 0;
	wchar_t buffer[128] = {0};

	item = &nis.item;

	if(!parent)
	{
		MLTREEIMAGE devIcon;
		wchar_t deviceName[256]=L"";

		devIcon.resourceId = IDR_DEVICE_ICON;
		devIcon.hinst = plugin.hDllInstance;
		dev->extraActions(DEVICE_SET_ICON,(intptr_t)&devIcon,0,0);

		dev->getPlaylistName(0,deviceName,sizeof(deviceName)/sizeof(wchar_t));

		nis.hParent = GetNavigationRoot(TRUE);
		nis.hInsertAfter = NCI_LAST;
		item->cbSize = sizeof(NAVITEM);
		item->mask = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_STYLE | NIMF_IMAGE | NIMF_IMAGESEL;

		item->pszText = deviceName;
		item->pszInvariant = nis.item.pszText;
		item->style = NIS_HASCHILDREN;
		item->styleMask = item->style,
		item->iImage = icon_store.GetResourceIcon(devIcon.hinst, MAKEINTRESOURCE(devIcon.resourceId));
		item->iSelectedImage = item->iImage;

		treeItem = parent = MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis);

		navigationItemCreated = TRUE;
	}
	else
	{
		treeItem = parent;
		navigationItemCreated = FALSE;

		item->cbSize = sizeof(NAVITEM);
		item->mask = NIMF_STYLE;
		item->hItem = treeItem;
		item->style = NIS_HASCHILDREN;
		item->styleMask = NIS_HASCHILDREN;

		MLNavItem_SetInfo(plugin.hwndLibraryParent, item);

		/* create transfer view */
		// TODO: create this view dynamically
		HNAVITEM cloud = 0;
		if (isCloudDevice)
		{
			cloud = NavigationItem_Find(0, L"cloud_sources", TRUE);
			if (cloud != NULL) parent = cloud;
		}

#if 0
		int mode = gen_mlconfig->ReadInt(L"txviewpos", 0);
		if (mode == 1)
		{
			nis.hParent = cloud;//parent;
			nis.hInsertAfter = NCI_FIRST;
		}
		else if (mode == 2)
		{
			nis.hParent = NavigationItem_Find(0, L"ml_devices_root", TRUE);
			nis.hInsertAfter = NCI_FIRST;
		}
#else
		nis.hParent = parent;
#endif

		item->cbSize = sizeof(NAVITEM);
		item->mask = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_IMAGE | NIMF_IMAGESEL;
		item->pszText = WASABI_API_LNGSTRINGW_BUF(IDS_TRANSFERS, buffer, 128);
		item->pszInvariant = (isCloudDevice ? L"cloud_transfers" : L"transfers");
		item->iImage = icon_store.GetQueueIcon();
		item->iSelectedImage = nis.item.iImage;

#if 0
		if (!cloudQueueTreeItem && isCloudDevice)
		{
			NAVINSERTSTRUCT nis2 = {0};
			nis2.item.cbSize = sizeof(NAVITEM);
			nis2.item.pszText = WASABI_API_LNGSTRINGW(IDS_ADD_SOURCE);
			nis2.item.pszInvariant = L"cloud_add_sources";
			nis2.item.mask = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_IMAGE | NIMF_IMAGESEL | NIMF_STYLE;
			nis2.item.iImage = nis2.item.iSelectedImage = mediaLibrary.AddTreeImageBmp(IDB_TREEITEM_CLOUD_ADD_SOURCE);
			nis2.hParent = parent;
			nis2.hInsertAfter = NCI_FIRST;
			HNAVITEM item = MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis2);
			if (mode == 1) nis.hInsertAfter = item;
			queueTreeItem = cloudQueueTreeItem = MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis);

#if 0
			nis2.item.pszText = L"BYOS";//WASABI_API_LNGSTRINGW(IDS_ADD_SOURCE);
			nis2.item.pszInvariant = L"cloud_byos";
			nis2.item.mask = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_IMAGE | NIMF_IMAGESEL | NIMF_STYLE;
			nis2.item.iImage = nis2.item.iSelectedImage = mediaLibrary.AddTreeImageBmp(IDB_TREEITEM_CLOUD_ADD_BYOS);
			nis2.hParent = parent;
			nis2.hInsertAfter = nis.hInsertAfter;
			MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis2);
#endif
		}
		else
			queueTreeItem = cloudQueueTreeItem;
#endif
			queueTreeItem = MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis);
	}

	/* create video view */
	if (videoView)
	{
		nis.hParent = parent;
		nis.hInsertAfter = NCI_LAST;
		item->cbSize = sizeof(NAVITEM);
		item->mask = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_IMAGE | NIMF_IMAGESEL;
		item->pszText =  WASABI_API_LNGSTRINGW_BUF(IDS_VIDEO, buffer, 128);
		item->pszInvariant = L"video";
		item->iImage = icon_store.GetVideoIcon();
		item->iSelectedImage = nis.item.iImage;
	
		videoTreeItem = MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis);
	}
	else
		videoTreeItem = 0;

	/* create playlists */
	int l = (dev ? dev->getPlaylistCount() : 0);
	for(int i=1; i<l; i++)
	{
		AddPlaylistNode(i);
	}
}

DeviceView::~DeviceView()
{
	if(configDevice == this) SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(intptr_t)&prefsPage,IPC_OPENPREFSTOPAGE);

	// remove it when we're removed what we added
	int lastPrefsPageLoaded = prefsPageLoaded;
	prefsPageLoaded-=1;
	if(lastPrefsPageLoaded == 1)
	{
		SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(intptr_t)&prefsPage,IPC_REMOVE_PREFS_DLG);
	}

	//OutputDebugString(L"device unloading started");
	// get rid of the transfer thread
	threadKillswitch=1;
	transferContext.WaitForKill();
	if(threadKillswitch != 100)
	{
		/*OutputDebugString(L"FUCKO");*/
	}

	SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(intptr_t)&devPrefsPage,IPC_REMOVE_PREFS_DLG);
	free(devPrefsPage.name);
	//OutputDebugString(L"device unloading finished");
	delete config;
}

void DeviceView::SetVideoView(BOOL enabled)
{
	videoView=enabled;
	config->WriteInt(L"showVideoView",videoView);
	if(videoView)
	{
		/* add video before the playlists */
		wchar_t buffer[128] = {0};
		NAVINSERTSTRUCT nis = {0};
		nis.item.cbSize = sizeof(NAVITEM);
		nis.item.pszText =  WASABI_API_LNGSTRINGW_BUF(IDS_VIDEO, buffer, 128);
		nis.item.pszInvariant = L"video";
		nis.item.mask = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_IMAGE | NIMF_IMAGESEL;
		nis.hParent = treeItem;
		nis.hInsertAfter = NCI_FIRST;
		nis.item.iImage = icon_store.GetVideoIcon();
		nis.item.iSelectedImage = nis.item.iImage;
		videoTreeItem = MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis);
	}
	else
	{
		MLNavCtrl_DeleteItem(plugin.hwndLibraryParent, videoTreeItem);
		videoTreeItem = 0;
	}
}

static void removebadchars(wchar_t *s)
{
	while (s && *s)
	{
		if (*s == L'?' || *s == L'/' || *s == L'\\' || *s == L':' || *s == L'*' || *s == L'\"' || *s == L'<' || *s == L'>' || *s == L'|') 
			*s = L'_';
		s = CharNextW(s);
	}
}

static __int64 fileSize(wchar_t * filename)
{
	WIN32_FIND_DATA f= {0};
	HANDLE h = FindFirstFileW(filename,&f);
	if(h == INVALID_HANDLE_VALUE) return -1;
	FindClose(h);
	ULARGE_INTEGER i;
	i.HighPart = f.nFileSizeHigh;
	i.LowPart = f.nFileSizeLow;
	return i.QuadPart;
}

static bool copySettings(wchar_t * ssrc, wchar_t * sdest)
{
	FILE * src, * dest;
	src=_wfopen(ssrc,L"rt");
	if(!src) return false;
	dest=_wfopen(sdest,L"wt");
	if(!dest)
	{
		fclose(src); return false;
	}
	wchar_t buf[1024]=L"";
	bool insection=false;
	while(fgetws(buf,1024,src))
	{
		if(buf[0]==L'[' && wcslen(buf)>1) if(buf[wcslen(buf)-2]==L']') insection=false;
		if(wcscmp(&buf[0],L"[ml_pmp]\n")==0) insection=true;
		if(insection) fputws(&buf[0],dest);
	}
	fclose(src);
	fclose(dest);
	return true;
}

HNAVITEM DeviceView::AddPlaylistNode(int id)
{
	NAVINSERTSTRUCT nis = {0};
	wchar_t title[256] = {0}, name[128] = {0};

	dev->getPlaylistName(id, title , ARRAYSIZE(title));

	StringCchPrintf(name, ARRAYSIZE(name), L"ml_pmp_playlist_%d", id);
	
	nis.hParent = treeItem;
	nis.hInsertAfter = NCI_LAST;

	memset(&nis.item, 0, sizeof(nis.item));
	nis.item.cbSize = sizeof(NAVITEM);
	nis.item.pszText = title;
	nis.item.pszInvariant = name;
	nis.item.mask = NIMF_TEXT | NIMF_TEXTINVARIANT | NIMF_IMAGE | NIMF_IMAGESEL | NIMF_STYLE;
	nis.item.iImage = icon_store.GetPlaylistIcon();
	nis.item.iSelectedImage = nis.item.iImage;
	nis.item.style = NIS_ALLOWEDIT;
	nis.item.styleMask = nis.item.style;
	
	HNAVITEM item = MLNavCtrl_InsertItem(plugin.hwndLibraryParent, &nis);
	playlistTreeItems.push_back(item);

	return item;
}

int DeviceView::CreatePlaylist(wchar_t * name, bool silent)
{
	HNAVITEM item;
	int playlistId;
	
	if (NULL == name)
	{
		int count, slot, length;
		wchar_t buffer[512] = {0}, buffer2[ARRAYSIZE(buffer)] = {0};
		BOOL foundMatch;

		name = WASABI_API_LNGSTRINGW_BUF(IDS_NEW_PLAYLIST, buffer, ARRAYSIZE(buffer));

		count = dev->getPlaylistCount();
		slot = 1;
		length = -1;

		do
		{
			foundMatch = FALSE;
			for (int i = 1; i < count; i++)
			{
				
				dev->getPlaylistName(i, buffer2, ARRAYSIZE(buffer2));
				if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, buffer2, -1, name, -1))
				{
					foundMatch = TRUE;
					
					if(name != buffer)
					{
						if (FAILED(StringCchCopy(buffer, ARRAYSIZE(buffer), name)))
							return -1;
					}

					if (-1 == length)
					{
						wchar_t *end;
						length = lstrlen(buffer);
						end = buffer + length;
						
						if(length > 2 && L')' == *(--end))
						{			
							unsigned short charType;

							for(wchar_t *begin = --end; begin != buffer; begin--)
							{
								if (L'(' == *begin)
								{
									if (begin > buffer && L' ' == *(--begin))
									{
										length = (int)(intptr_t)(begin - buffer);
										slot = 0;
									}
									break;
								}
								else if (FALSE == GetStringTypeW(CT_CTYPE1, begin, 1, &charType) ||
									0 == (C1_DIGIT & charType))
								{
									break;
								}
							}
						}
					}
					
					slot++;

					if (1 == slot)
						buffer[length] = L'\0';
					else if (FAILED(StringCchPrintf(buffer + length, ARRAYSIZE(buffer) - length, L" (%d)", slot)))
						return false;
					
					break;
				}
			}
		} while(FALSE != foundMatch);
	}

	playlistId = dev->newPlaylist(name);
	if(playlistId == -1) 
		return -1; // failed
	
	item = AddPlaylistNode(playlistId);
	if (NULL == item)
	{
		dev->deletePlaylist(playlistId);
		return -1;
	}

	DevicePropertiesChanges();

	if(!silent)
		MLNavItem_EditTitle(plugin.hwndLibraryParent, item);
		
	return playlistId;
}

void DeviceView::RenamePlaylist(int playlistId)
{
	HNAVITEM item;

	item = NULL;

	if (0 == playlistId)
	{
		if (0 != dev->extraActions(DEVICE_CAN_RENAME_DEVICE,0,0,0))
			item = treeItem;
	}
	else
	{
		if (playlistId > 0 && playlistId <= (int)playlistTreeItems.size())
			item = playlistTreeItems[playlistId - 1];
	}
	
	if (NULL != item)
		MLNavItem_EditTitle(plugin.hwndLibraryParent, item);
}

size_t DeviceView::GetPlaylistName(wchar_t *buffer, size_t bufferSize, int playlistId, 
								const wchar_t *defaultName, BOOL quoteSpaces)
{
	size_t length;

	if (NULL == buffer || 0 == bufferSize)
		return 0;

	buffer[0] = L'\0';
	if (NULL != dev)
		dev->getPlaylistName(playlistId, buffer, bufferSize);

	if (FAILED(StringCchLength(buffer, bufferSize, &length)))
		return 0;
	
	if (0 == length)
	{
		if (NULL != defaultName)
		{
			if (FALSE != IS_INTRESOURCE(defaultName))
				WASABI_API_LNGSTRINGW_BUF((int)(intptr_t)defaultName, buffer, bufferSize);
			else
			{
				if (FAILED(StringCchCopy(buffer, bufferSize, defaultName)))
					return 0;
			}

			if (FAILED(StringCchLength(buffer, bufferSize, &length)))
				return 0;
		}
	}
	else
	{
		if (FALSE != quoteSpaces &&	
			(L' ' == buffer[0] || L' ' == buffer[length-1]) &&
			(bufferSize - length) > 2)
		{
			memmove(buffer + 1, buffer, sizeof(wchar_t) * length);
			buffer[0] = L'\"';
			buffer[length++] = L'\"';
			buffer[length] = L'\0';
		}
	}
	return length;
}

bool DeviceView::DeletePlaylist(int playlistId, bool deleteFiles, bool verbal)
{
	int index;
	C_ItemList delList;
	
	if(playlistId < 1) 
		return false;

	index = playlistId - 1;
	
	if (false != deleteFiles)
	{
		int length;

		length = dev->getPlaylistLength(playlistId);
		for(int i = 0; i < length; i++) 
		{
			delList.Add((void*)dev->getPlaylistTrack(playlistId, i));
		}
	}

	if (false != verbal)
	{
		wchar_t message[1024] = {0}, title[1024] = {0}, playlistName[256] = {0}, deviceName[256] = {0};

		GetPlaylistName(playlistName, ARRAYSIZE(playlistName), playlistId, NULL, FALSE);
		GetPlaylistName(deviceName, ARRAYSIZE(deviceName), 0, MAKEINTRESOURCE(IDS_DEVICE_LOWERCASE), TRUE);

		if (0 != delList.GetSize())
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_PHYSICALLY_REMOVE_X_TRACKS, title, ARRAYSIZE(title));
			StringCchPrintf(message, ARRAYSIZE(message), title, delList.GetSize(), playlistName);

			WASABI_API_LNGSTRINGW_BUF(IDS_DELETE_PLAYLIST_TITLE, title, ARRAYSIZE(title));
		}
		else
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_DELETE_PLAYLIST, title, ARRAYSIZE(title));
			StringCchPrintf(message, ARRAYSIZE(message), title, playlistName, deviceName);

			WASABI_API_LNGSTRINGW_BUF(IDS_DELETE_PLAYLIST_TITLE, title, ARRAYSIZE(title));
		}
		
		if(IDYES != MessageBox(plugin.hwndLibraryParent, message, title, 
						MB_YESNO | MB_ICONWARNING))
		{
			return false;
		}
	}

	if (0 != delList.GetSize())
	{
		int result;
		result = DeleteTracks(&delList, CENTER_OVER_ML_VIEW);
		if (IDABORT == result) /* user abort */
			return false;

		if (IDOK != result) /* error */
		{
			
		}

	}
	
	HNAVITEM item = playlistTreeItems[index];
	playlistTreeItems.erase(playlistTreeItems.begin() + index);

	MLNavCtrl_DeleteItem(plugin.hwndLibraryParent, item);
	
	dev->deletePlaylist(playlistId);
	DevicePropertiesChanges();

	return true;
}

bool DeviceView::GetTransferFromMlSupported(int dataType)
{
	switch(dataType)
	{
		case ML_TYPE_ITEMRECORDLISTW:
		case ML_TYPE_ITEMRECORDLIST:
		case ML_TYPE_PLAYLIST:
		case ML_TYPE_PLAYLISTS:
		case ML_TYPE_FILENAMES:
		case ML_TYPE_FILENAMESW:
			return true;
	}
	return false;
}
intptr_t DeviceView::TransferFromML(int type, void* data, int unsupportedReturn, int supportedReturn, int playlist)
{
	int r;

	if (AGAVE_API_STATS)
	{
		wchar_t device_name[128] = {0};
		if(dev->extraActions(DEVICE_GET_MODEL, (intptr_t)device_name, 128, 0) == 1 && device_name[0])
		{
			AGAVE_API_STATS->SetString("pmp", device_name);
		}
	}

	if(type == ML_TYPE_ITEMRECORDLISTW)
	{
		r=AddItemListToTransferQueue((itemRecordListW*)data,playlist);
	}
	else if(type == ML_TYPE_ITEMRECORDLIST)
	{
		itemRecordListW list= {0};
		convertRecordList(&list,(itemRecordList*)data);
		r=AddItemListToTransferQueue(&list,playlist);
		freeRecordList(&list);
	}
	else if(type == ML_TYPE_FILENAMES)
	{
		C_ItemList fileList;
		char * filenames = (char *)data;
		while(filenames && *filenames)
		{
			fileList.Add(filenames);
			filenames+=strlen(filenames)+1;
		}
		r=AddFileListToTransferQueue((char**)fileList.GetAll(),fileList.GetSize(),playlist);
	}
	else if(type == ML_TYPE_FILENAMESW)
	{
		C_ItemList fileList;
		wchar_t * filenames = (wchar_t *)data;
		while(filenames && *filenames)
		{
			fileList.Add(filenames);
			filenames+=wcslen(filenames)+1;
		}
		r=AddFileListToTransferQueue((wchar_t**)fileList.GetAll(),fileList.GetSize(),playlist);
	}
	else if(type == ML_TYPE_PLAYLIST)
	{
		mlPlaylist * pl = (mlPlaylist *)data;
		TransferPlaylist((wchar_t*)pl->filename,(wchar_t*)pl->title);
		r=0;
	}
	else if(type == ML_TYPE_PLAYLISTS)
	{
		mlPlaylist **playlists = (mlPlaylist **)data;
		while(playlists && *playlists)
		{
			mlPlaylist *pl = *playlists;
			TransferPlaylist((wchar_t*)pl->filename,(wchar_t*)pl->title);
			playlists++;
		}
		r=0;
	}
	else return unsupportedReturn;

	wchar_t errStr[32] = {0};
	if(r==-1)
		MessageBox(plugin.hwndLibraryParent,
		           WASABI_API_LNGSTRINGW(IDS_DEVICE_OUT_OF_SPACE),
		           WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,errStr,32),0);
	else if(r==-2)
		MessageBox(plugin.hwndLibraryParent,
		           WASABI_API_LNGSTRINGW(IDS_INCOMPATABLE_FORMAT_NO_TX),
		           WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,errStr,32),0);

	return supportedReturn;
}


class ItemListRefLoader : public ifc_playlistloadercallback
{
public:
	ItemListRefLoader(C_ItemList &itemList, C_ItemList &playlistItemList) : fileList(itemList), playlistList(playlistItemList)
	{
	}
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
	{
		if(playlistManager->CanLoad(filename))
			playlistList.Add(_wcsdup(filename));
		else
			fileList.Add(_wcsdup(filename));
	}

	C_ItemList &fileList, &playlistList;
protected:
	RECVS_DISPATCH;
};

#define CBCLASS ItemListRefLoader
START_DISPATCH;
VCB( IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile )
END_DISPATCH;
#undef CBCLASS


class PlaylistDirectoryCallback : public ifc_playlistdirectorycallback
{
public:
	PlaylistDirectoryCallback(const wchar_t *_extlist) : extlist(_extlist)
	{
	}

	bool ShouldRecurse(const wchar_t *path)
	{
		// TODO: check for recursion?
		return true;
	}

	bool ShouldLoad(const wchar_t *filename)
	{
		if(playlistManager->CanLoad(filename))
			return true;
		const wchar_t *ext = PathFindExtensionW(filename);
		if(!*ext)
			return false;

		ext++;

		const wchar_t *a = extlist;
		while(a && *a)
		{
			if(!_wcsicmp(a, ext))
				return true;
			a += wcslen(a) + 1;
		}
		return false;
	}

	const wchar_t *extlist;

protected:
	RECVS_DISPATCH;
};

#define CBCLASS PlaylistDirectoryCallback
START_DISPATCH;
CB(IFC_PLAYLISTDIRECTORYCALLBACK_SHOULDRECURSE, ShouldRecurse)
CB(IFC_PLAYLISTDIRECTORYCALLBACK_SHOULDLOAD, ShouldLoad)
END_DISPATCH;
#undef CBCLASS

intptr_t DeviceView::TransferFromDrop(HDROP hDrop, int playlist)
{
	// benski> ugh. memory allocation hell.  oh well
	C_ItemList fileList;
	C_ItemList playlistList;
	const wchar_t *extListW = (const wchar_t *)SendMessageW(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_EXTLISTW);
	PlaylistDirectoryCallback dirCB(extListW);
	wchar_t temp[2048] = {0};
	int y = DragQueryFileW(hDrop, 0xffffffff, temp, 2048);

	for(int x = 0; x < y; x ++)
	{
		DragQueryFileW(hDrop, x, temp, 2048);
		// see if it's a directory
		bool isDir=false;
		if(!PathIsURLW(temp) && !PathIsNetworkPathW(temp))
		{
			HANDLE h;
			WIN32_FIND_DATAW d;

			h = FindFirstFileW(temp, &d);
			if(h != INVALID_HANDLE_VALUE)
			{
				FindClose(h);
				if(d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					ItemListRefLoader fileListCB(fileList, playlistList);
					playlistManager->LoadDirectory(temp, &fileListCB, &dirCB);
					isDir=true;
				}
			}
		}

		if(!isDir)
		{
			if(playlistManager->CanLoad(temp))
				playlistList.Add(_wcsdup(temp));
			else
				fileList.Add(_wcsdup(temp));
		}
	}

	int r=0, r2=0;
	if(fileList.GetSize())
		r=AddFileListToTransferQueue((wchar_t**)fileList.GetAll(),fileList.GetSize(),playlist);
#if 0
	if(playlistList.GetSize())
		r2=AddFileListToTransferQueue((wchar_t**)playlistList.GetAll(), playlistList.GetSize(),1/*playlists*/);
#endif
	wchar_t errStr[32] = {0};
	if(r==-1 || r2==-1)
		MessageBox(plugin.hwndLibraryParent,
		           WASABI_API_LNGSTRINGW(IDS_DEVICE_OUT_OF_SPACE),
		           WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,errStr,32),0);
	if(r==-2 || r2 == -2)
		MessageBox(plugin.hwndLibraryParent,
		           WASABI_API_LNGSTRINGW(IDS_INCOMPATABLE_FORMAT_NO_TX),
		           WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,errStr,32),0);

	// benski> my CS301 professor would be proud!
	fileList.for_all(free);
	playlistList.for_all(free);

	GlobalFree((HGLOBAL)extListW);
	return 0;
}

HWND DeviceView::CreateView(HWND parent)
{
	currentViewedDevice=this;
	currentViewedPlaylist=0;

	if (currentViewedDevice->config->ReadInt(L"media_numfilters", 2) == 1)
		return WASABI_API_CREATEDIALOGPARAMW((isCloudDevice ? IDD_VIEW_CLOUD_SIMPLE : IDD_VIEW_PMP_VIDEO),
											 parent, pmp_video_dlgproc, (currentViewedDevice->isCloudDevice ? 1 : 2));
	else
		return WASABI_API_CREATEDIALOGPARAMW((isCloudDevice ? IDD_VIEW_CLOUD_ARTISTALBUM : IDD_VIEW_PMP_ARTISTALBUM),
											 parent, pmp_artistalbum_dlgproc, 0);
}

BOOL DeviceView::DisplayDeviceContextMenu(HNAVITEM item, HWND hostWindow, POINT pt)
{
	HMENU menu = GetSubMenu(m_context_menus,3);
	if (NULL == menu)
		return FALSE;

	if(dev->extraActions(DEVICE_CAN_RENAME_DEVICE,0,0,0))
		AppendMenu(menu,0,ID_TREEPLAYLIST_RENAMEPLAYLIST,WASABI_API_LNGSTRINGW(IDS_RENAME_DEVICE));

	int r = Menu_TrackSkinnedPopup(menu, TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_NONOTIFY,
								   pt.x, pt.y, plugin.hwndLibraryParent, NULL);

	if(dev->extraActions(DEVICE_CAN_RENAME_DEVICE,0,0,0))
		DeleteMenu(menu,ID_TREEPLAYLIST_RENAMEPLAYLIST,MF_BYCOMMAND);

	switch(r)
	{
		case ID_TREEDEVICE_NEWPLAYLIST:
			CreatePlaylist();
			break;
		case ID_TREEDEVICE_EJECTDEVICE:
			Eject();
			break;
		case ID_TREEPLAYLIST_RENAMEPLAYLIST:
			MLNavItem_EditTitle(plugin.hwndLibraryParent, item);
			//RenamePlaylist(0);
			break;
	}

	return TRUE;
}

BOOL DeviceView::DisplayPlaylistContextMenu(int playlistId, HNAVITEM item, HWND hostWindow, POINT pt)
{
	HMENU menu = GetSubMenu(m_context_menus,4);
	if (NULL == menu)
		return FALSE;
							
	EnableMenuItem(menu,ID_TREEPLAYLIST_COPYPLAYLISTTOLOCALMEDIA, 
			MF_BYCOMMAND | (dev->copyToHardDriveSupported()? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));

	int r = Menu_TrackSkinnedPopup(menu,TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_LEFTBUTTON|TPM_NONOTIFY,
								   pt.x, pt.y, plugin.hwndLibraryParent, NULL);
	switch(r)
	{
		case ID_TREEPLAYLIST_RENAMEPLAYLIST:
			MLNavItem_EditTitle(plugin.hwndLibraryParent, item);
			break;
		case ID_TREEPLAYLIST_REMOVEPLAYLISTANDFILES:
			DeletePlaylist(playlistId, true, true);
			break;
		case ID_TREEPLAYLIST_REMOVEPLAYLIST:
			DeletePlaylist(playlistId, false, true);
			break;
		case ID_TREEPLAYLIST_COPYPLAYLISTTOLOCALMEDIA:
			CopyPlaylistToLibrary(playlistId);
			break;
	}

	return TRUE;
}
static HNAVITEM Navigation_GetItemFromMessage(INT msg, INT_PTR param)
{
	return (msg < ML_MSG_NAVIGATION_FIRST) ? 
			MLNavCtrl_FindItemById(plugin.hwndLibraryParent, param) : 
			(HNAVITEM)param;
}

BOOL DeviceView::Navigation_IsPlaylistItem(HNAVITEM item, int *playlistId)
{
	for(size_t i=0; i < playlistTreeItems.size(); i++)
	{
		if(item == playlistTreeItems[i])
		{
			if (NULL != playlistId)
				*playlistId = (i + 1);

			return TRUE;
		}
	}

	return FALSE;
}

HWND DeviceView::Navigation_CreateViewCb(HNAVITEM item, HWND parentWindow)
{
	if(item == treeItem)
	{
		if (FALSE != navigationItemCreated)
		{
			currentViewedDevice = this;
			currentViewedPlaylist = 0;
			return WASABI_API_CREATEDIALOGW((currentViewedDevice->isCloudDevice ? IDD_VIEW_CLOUD_ARTISTALBUM : IDD_VIEW_PMP_ARTISTALBUM), parentWindow, pmp_artistalbum_dlgproc);
		}
	}
	else if (item == queueTreeItem)
	{
		currentViewedDevice = this;
		currentViewedPlaylist = 0;
		return WASABI_API_CREATEDIALOGPARAMW((currentViewedDevice->isCloudDevice ? IDD_VIEW_CLOUD_QUEUE : IDD_VIEW_PMP_QUEUE), parentWindow, pmp_queue_dlgproc, (LPARAM)this);
	}
	else if(item == videoTreeItem)
	{
		currentViewedDevice = this;
		currentViewedPlaylist = 0;
		return WASABI_API_CREATEDIALOGPARAMW(IDD_VIEW_PMP_VIDEO, parentWindow, pmp_video_dlgproc, 0);
	}
	else
	{
		if (FALSE != Navigation_IsPlaylistItem(item, &currentViewedPlaylist))
		{
			currentViewedDevice = this;
			return WASABI_API_CREATEDIALOGW(IDD_VIEW_PMP_PLAYLIST, parentWindow, pmp_playlist_dlgproc);
		}
	}

	return NULL;
}

BOOL DeviceView::Navigation_ShowContextMenuCb(HNAVITEM item, HWND hostWindow, POINT pt)
{
	if (item == treeItem)
	{
		if (FALSE != navigationItemCreated)
			return DisplayDeviceContextMenu(item, hostWindow, pt);
	}
	else
	{
		int playlistId;
		if (FALSE != Navigation_IsPlaylistItem(item, &playlistId))
			return DisplayPlaylistContextMenu(playlistId, item, hostWindow, pt);
	}

	return FALSE;
}
BOOL DeviceView::Navigation_ClickCb(HNAVITEM item, int actionType, HWND hostWindow)
{
	int playlistId;

	switch(actionType)
	{
		case ML_ACTION_DBLCLICK:
		case ML_ACTION_ENTER:
			if (FALSE != Navigation_IsPlaylistItem(item, &playlistId))
			{	
				PlayPlaylist(playlistId, false, true, hostWindow);
				return TRUE;
			}
			break;
	}
	
	return FALSE;
}

int DeviceView::Navigation_DropTargetCb(HNAVITEM item, unsigned int dataType, void *data)
{
	if (item == treeItem)
	{
		if (FALSE != navigationItemCreated)
		{
			if(NULL == data)
				return (false != GetTransferFromMlSupported(dataType)) ? 1 : -1;
				
			return TransferFromML(dataType, data, -1, 1);
		}
	}
	else
	{
		int playlistId;
		if (FALSE != Navigation_IsPlaylistItem(item, &playlistId))
		{
			if(NULL == data)
				return (false != GetTransferFromMlSupported(dataType)) ? 1 : -1;
				
			return TransferFromML(dataType, data, -1, 1, playlistId);
		}
	}

	return FALSE;
}

BOOL DeviceView::Navigation_TitleEditBeginCb(HNAVITEM item)
{
	if (item == treeItem)
	{
		if (FALSE != navigationItemCreated && 
			FALSE == dev->extraActions(DEVICE_CAN_RENAME_DEVICE,0,0,0))
		{
			return TRUE;
		}
	}
	
	return FALSE;
}
BOOL DeviceView::Navigation_TitleEditEndCb(HNAVITEM item, const wchar_t *title)
{
	int playlistId = 0;
	wchar_t buffer[512] = {0};

	if (item == treeItem)
	{
		if (FALSE == navigationItemCreated)
			return FALSE;

		playlistId = 0;
	}
	else
	{		
		if (FALSE == Navigation_IsPlaylistItem(item, &playlistId))
			return FALSE;
	}

	if (NULL == title)
		return TRUE;

	buffer[0] = L'\0';
	dev->getPlaylistName(playlistId, buffer, ARRAYSIZE(buffer));
			
	if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, 0, buffer, -1, title, -1))
		return TRUE;

	dev->setPlaylistName(playlistId, title);
	DevicePropertiesChanges();

	buffer[0] = L'\0';
	dev->getPlaylistName(playlistId, buffer, ARRAYSIZE(buffer));

	if (0 == playlistId)
	{
		free(devPrefsPage.name);
		devPrefsPage.name = _wcsdup(buffer);

		UpdateDevicesListView(false);
		OnNameChanged(buffer);
	}

	if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, 0, buffer, -1, title, -1))
		return TRUE;

	NAVITEM itemInfo;
	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.pszText = buffer;
	itemInfo.hItem = item;
	itemInfo.mask = NIMF_TEXT;
	MLNavItem_SetInfo(plugin.hwndLibraryParent, &itemInfo);

	return FALSE;
}

BOOL DeviceView::Navigation_KeyDownCb(HNAVITEM item, NMTVKEYDOWN *keyData, HWND hwnd)
{
	int playlistId;
	if (item == treeItem)
	{
		if (FALSE == navigationItemCreated)
			return FALSE;

		switch(keyData->wVKey)
		{
			case VK_F2:
				MLNavItem_EditTitle(plugin.hwndLibraryParent, item);
				break;
		}
		return TRUE;
	}

	if (FALSE != Navigation_IsPlaylistItem(item, &playlistId))
	{
		switch(keyData->wVKey)
		{
			case VK_F2:
				MLNavItem_EditTitle(plugin.hwndLibraryParent, item);
				break;

			case VK_DELETE:
				DeletePlaylist(playlistId, false, true);
				break;
		}
		return TRUE;
	}

	return FALSE;
}

intptr_t DeviceView::MessageProc(int message_type, intptr_t param1, intptr_t param2, intptr_t param3)
{
	if(message_type >= ML_MSG_TREE_BEGIN && message_type <= ML_MSG_TREE_END)
	{		
		HNAVITEM item;

		switch(message_type)
		{
			case ML_MSG_TREE_ONCREATEVIEW: 
				item = Navigation_GetItemFromMessage(message_type, param1);
				return (INT_PTR)Navigation_CreateViewCb(item, (HWND)param2);
			case ML_MSG_NAVIGATION_CONTEXTMENU:
			{
				POINT pt;
				POINTSTOPOINT(pt, MAKEPOINTS(param3));
				item = Navigation_GetItemFromMessage(message_type, param1);
				return (INT_PTR)Navigation_ShowContextMenuCb(item, (HWND)param2, pt);
			}
			case ML_MSG_TREE_ONCLICK:
				item = Navigation_GetItemFromMessage(message_type, param1);
				return (INT_PTR)Navigation_ClickCb(item, (int)param2, (HWND)param3);
			case ML_MSG_TREE_ONDROPTARGET:
				item = Navigation_GetItemFromMessage(message_type, param1);
				return (INT_PTR)Navigation_DropTargetCb(item, (unsigned int)param2, (void*)param3);
			case ML_MSG_NAVIGATION_ONBEGINTITLEEDIT:
				item = Navigation_GetItemFromMessage(message_type, param1);
				return (INT_PTR)Navigation_TitleEditBeginCb(item);
			case ML_MSG_NAVIGATION_ONENDTITLEEDIT:
				item = Navigation_GetItemFromMessage(message_type, param1);
				return (INT_PTR)Navigation_TitleEditEndCb(item, (const wchar_t*)param2);
			case ML_MSG_TREE_ONKEYDOWN:
				item = Navigation_GetItemFromMessage(message_type, param1);
				return (INT_PTR)Navigation_KeyDownCb(item, (NMTVKEYDOWN*)param2, (HWND)param3);
		}
	}
	else if(message_type == ML_MSG_ONSENDTOBUILD)
	{
		if (!gen_mlconfig->ReadInt(L"pmp_send_to", DEFAULT_PMP_SEND_TO))
		{
			if (param1 == ML_TYPE_ITEMRECORDLIST || param1 == ML_TYPE_ITEMRECORDLISTW ||
				param1 == ML_TYPE_FILENAMES || param1 == ML_TYPE_FILENAMESW ||
				param1 == ML_TYPE_PLAYLIST || param1 == ML_TYPE_PLAYLISTS)
			{
				if (dev->extraActions(DEVICE_SENDTO_UNSUPPORTED, 0, 0, 0) == 0)
				{
					wchar_t buffer[128] = {0};
					dev->getPlaylistName(0, buffer, 128);
					mediaLibrary.AddToSendTo(buffer, param2, reinterpret_cast<INT_PTR>(this));
				}
			}
		}
	}
	else if(message_type == ML_MSG_ONSENDTOSELECT && param2 && param3 == (intptr_t)this)
	{
		// TODO!!!
		// if we get a playlist or playlist list and we can match it to a cloud device then
		// we check for 'hss' and if so then process as a cloud playlist else do as before
		if (this->isCloudDevice && (param1 == ML_TYPE_PLAYLIST || param1 == ML_TYPE_PLAYLISTS))
		{
			char name[128] = {0};
			if (dev->extraActions(DEVICE_GET_UNIQUE_ID, (intptr_t)name, sizeof(name), 0))
			{
				if (!strcmp(name, "hss"/*HSS_CLIENT*/))
				{
					if(param1 == ML_TYPE_PLAYLIST)
					{
						mlPlaylist * pl = (mlPlaylist *)param2;
						TransferAddCloudPlaylist((wchar_t*)pl->filename, (wchar_t*)pl->title);
					}
					else if(param1 == ML_TYPE_PLAYLISTS)
					{
						mlPlaylist **playlists = (mlPlaylist **)param2;
						while(playlists && *playlists)
						{
							mlPlaylist *pl = *playlists;
							TransferAddCloudPlaylist((wchar_t*)pl->filename, (wchar_t*)pl->title);
							playlists++;
						}
					}
					return 1;
				}
			}
		}

		UpdateActivityState();
		return TransferFromML(param1,(void*)param2,0,1);
	}
	return 0;
}

void DeviceView::DevicePropertiesChanges()
{
	commitNeeded=true;
	SetEvent(transferContext.notifier);
}

void DeviceView::Eject()
{
	LinkedQueue * txQueue = getTransferQueue(this);
	if(txQueue && txQueue->GetSize() == 0)
	{
		dev->Eject();
	}
	else
	{
		wchar_t titleStr[32] = {0};
		MessageBox(plugin.hwndLibraryParent,WASABI_API_LNGSTRINGW(IDS_SYNC_IS_IN_PROGRESS),
		           WASABI_API_LNGSTRINGW_BUF(IDS_CANNOT_EJECT,titleStr,32),0);
	}
}

Device * deleteTrackDev;
C_ItemList * deleteTracks;
extern HWND hwndMediaView;

static INT_PTR CALLBACK pmp_delete_progress_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	static int i;
	static songid_t s;
	switch(uMsg)
	{
		case WM_INITDIALOG:
			i=0;
			s=0;
			SetWindowText(hwndDlg,WASABI_API_LNGSTRINGW((!currentViewedDevice || currentViewedDevice && !currentViewedDevice->isCloudDevice ? IDS_DELETING_TRACKS : IDS_REMOVING_TRACKS)));
			SendDlgItemMessage(hwndDlg,IDC_PROGRESS,PBM_SETRANGE,0,MAKELPARAM(0, (!deleteTracks ? 0 : deleteTracks->GetSize())));
			SetTimer(hwndDlg,0,5,NULL);

			if (FALSE != CenterWindow(hwndDlg, (HWND)lParam))
				SendMessage(hwndDlg, DM_REPOSITION, 0, 0L);

			break;
		case WM_TIMER:
			if(wParam == 1)
			{
				KillTimer(hwndDlg,wParam);
				SendDlgItemMessage(hwndDlg,IDC_PROGRESS,PBM_SETPOS,i,0);
				if(i < deleteTracks->GetSize())
				{
					songid_t s2 = (songid_t)deleteTracks->Get(i++);
					if(s != s2)
					{
						if(hwndMediaView) SendMessage(hwndMediaView,WM_USER+1,(WPARAM)s2,0);
						deleteTrackDev->deleteTrack(s2);
						s=s2;
					}
					SetTimer(hwndDlg,1,5,NULL);
				}
				else EndDialog(hwndDlg, IDOK);
			}
			else if(wParam == 0)
			{
				KillTimer(hwndDlg,0);
				int s = deleteTracks->GetSize();
				for(int i=0; i<s; i++)
				{
					void * p = deleteTracks->Get(i);
					for(int j=i+1; j<s; j++)
					{
						if(p == deleteTracks->Get(j))
						{
							deleteTracks->Del(j--); s--;
						}
					}
				}
				SetTimer(hwndDlg,1,5,NULL);
			}
			break;
		case WM_COMMAND:
			if(LOWORD(wParam) == IDC_ABORT)
				EndDialog(hwndDlg, IDABORT);
			break;
	}
	return 0;
}
int DeviceView::DeleteTracks(C_ItemList * tracks, HWND centerWindow)
{
	LinkedQueue * txQueue = getTransferQueue(this);
	if(txQueue && txQueue->GetSize() > 0)
	{
		wchar_t sorry[32] = {0};
		MessageBox(plugin.hwndLibraryParent,WASABI_API_LNGSTRINGW(IDS_CANNOT_REMOVE_TRACKS_WHILE_TRANSFERING),
		           WASABI_API_LNGSTRINGW_BUF(IDS_SORRY,sorry,32),0);
		return -1;
	}
	if (dev && tracks)
	{
		deleteTrackDev = dev;
		deleteTracks = tracks;
		return WASABI_API_DIALOGBOXPARAMW(IDD_PROGRESS, plugin.hwndLibraryParent, pmp_delete_progress_dlgproc, (LPARAM)centerWindow);
	}
	return IDABORT;
}

int DeviceView::AddFileListToTransferQueue(char ** files, int num, int playlist)
{
	wchar_t ** filesW = (wchar_t**)calloc(num, sizeof(wchar_t*));
	for(int i=0; i<num; i++)
	{
		filesW[i] = AutoWideDup(files[i]);
	}
	int r = AddFileListToTransferQueue(filesW,num,playlist);
	for(int i=0; i<num; i++)
	{
		free(filesW[i]);
	}
	free(filesW);
	return r;
}

int DeviceView::AddFileListToTransferQueue(wchar_t ** files, int num, int playlist)
{
	C_ItemList * irs = fileListToItemRecords(files,num, CENTER_OVER_ML_VIEW);
	int r = AddItemListToTransferQueue(irs,playlist);
	for(int i=0; i < irs->GetSize(); i++)
	{
		itemRecordW * it = (itemRecordW *)irs->Get(i);
		freeRecord(it);
		free(it);
	}
	delete irs;
	return r;
}

void TransfersListPushPopItem(CopyInst * item, DeviceView *view);
int DeviceView::AddItemListToTransferQueue(C_ItemList * items, int playlist)
{
	if(playlist == 0)
	{
		if (!isCloudDevice)
		{
			int r=0;
			C_ItemList toSend, haveSent;
			ProcessDatabaseDifferences(dev,items,&haveSent,&toSend,NULL,NULL);
			LinkedQueue * txQueue = getTransferQueue(this);
			if (txQueue)
			{
				txQueue->lock();

				for(int i = 0; i < toSend.GetSize(); i++)
				{
					if((r = this->AddTrackToTransferQueue(this, (itemRecordW*)toSend.Get(i), true)) == -1) break;
				}
	
				txQueue->unlock();
			}
			return r;
		}
		else
		{
			int r=0;
			LinkedQueue * txQueue = getTransferQueue(this);
			if (txQueue)
			{
				txQueue->lock();
	
				for(int i = 0; i < items->GetSize(); i++)
				{
					if((r = this->AddTrackToTransferQueue(this, (itemRecordW*)items->Get(i), true)) == -1) break;
				}
	
				txQueue->unlock();
			}
			return r;
		}
	}
	else
	{
		return TransferTracksToPlaylist(items,playlist);
	}
}

int DeviceView::AddItemListToTransferQueue(itemRecordListW * items, int playlist)
{
	if(playlist == 0)
	{
		if (!isCloudDevice)
		{
			int r=0;
			C_ItemList toSend;
			ProcessDatabaseDifferences(dev,items,NULL,&toSend,NULL,NULL);
			LinkedQueue * txQueue = getTransferQueue(this);
			if (txQueue)
			{
				txQueue->lock();
				for (int i = 0; i < toSend.GetSize(); i++)
					if((r = this->AddTrackToTransferQueue(this, (itemRecordW*)toSend.Get(i), true)) == -1) break;
				txQueue->unlock();
			}
			return r;
		}
		else
		{
			int r=0;
			LinkedQueue * txQueue = getTransferQueue(this);
			if (txQueue)
			{
				txQueue->lock();
				for (int i = 0; i < items->Size; i++)
					if((r = this->AddTrackToTransferQueue(this, &items->Items[i], true)) == -1) break;
				txQueue->unlock();
			}
			return r;
		}
	}
	else
	{
		C_ItemList itemRecords;
		for (int i = 0; i < items->Size; i++) itemRecords.Add(&items->Items[i]);
		return TransferTracksToPlaylist(&itemRecords,playlist);
	}
}

class ItemListLoader : public ifc_playlistloadercallback
{
public:
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
	{
		fileList.Add(_wcsdup(filename));
	}

	void FreeAll()
	{
		fileList.for_all(free);
	}
	C_ItemList fileList;
protected:
	RECVS_DISPATCH;
};

#define CBCLASS ItemListLoader
START_DISPATCH;
VCB( IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile )
END_DISPATCH;
#undef CBCLASS

void DeviceView::TransferPlaylist(wchar_t * file, wchar_t * name0)
{
	// first sort the name out
	if(!file) return;
	wchar_t name[256] = {0};
	if(!name0)
	{
		wchar_t * s = wcsrchr(file,L'\\');
		if(!s) s = wcsrchr(file,L'/');
		if(!s) s = file;
		else s++;
		if(wcslen(s) >= 255) s[255]=0;
		StringCchCopy(name,256, s);
		wchar_t * e = wcsrchr(name,L'.');
		if(e) *e=0;
	}
	else lstrcpyn(name,name0,255);
	name[255]=0;
	// name sorted, parse m3u

	ItemListLoader fileList;
	playlistManager->Load(file, &fileList);
	C_ItemList *itemRecords = fileListToItemRecords(&fileList.fileList, CENTER_OVER_ML_VIEW);
	fileList.FreeAll();

	// now we have a list of itemRecords, lets try and add this playlist to the device!
	int plid = CreatePlaylist(name,true);
	if(plid != -1)
		TransferTracksToPlaylist(itemRecords,plid);
	delete itemRecords;
}

int DeviceView::TransferTracksToPlaylist(C_ItemList *itemRecords, int plid)
{
	wchar_t name[256] = {0};
	dev->getPlaylistName(plid,name,256);
	int i;
	for(i=0; i<itemRecords->GetSize(); i++)
	{
		itemRecordW * ice = (itemRecordW *)itemRecords->Get(i);
		wchar_t num[12] = {0};
		StringCchPrintf(num, 12, L"%x",i);
		setRecordExtendedItem(ice,L"PLN",num);
	}
	C_ItemList irAlreadyOn, siAlreadyOn;
	ProcessDatabaseDifferences(dev,itemRecords,&irAlreadyOn,NULL,&siAlreadyOn,NULL);
	// itemRecords_sort, irAlreadyOn, irTransfer and siAlreadyOn will NOT be in playlist order
	// we must get them into playlist order. In O(n) :/
	int l = itemRecords->GetSize();
	PlaylistAddItem * pl = (PlaylistAddItem*)calloc(l,sizeof(PlaylistAddItem));
	int on=0;
	for(i=0; i < l; i++)
	{
		itemRecordW * ice = (itemRecordW *)itemRecords->Get(i);
		int n;
		swscanf(getRecordExtendedItem(ice,L"PLN"),L"%x",&n);
		if(n >= l)
		{
			continue;
		}
		pl[n].item = ice;
		if(on < irAlreadyOn.GetSize()) if((itemRecordW*)irAlreadyOn.Get(on) == ice)   // this track is on the device!
			{
				pl[n].songid = (songid_t)siAlreadyOn.Get(on);
				on++;
			}
	}

	// awesome! pl now contains our playlist in proper order with the "songid" fields set if the track is on the device.
	C_ItemList * directAdd = new C_ItemList;
	int m = 0;
	LinkedQueue * txQueue = getTransferQueue(this);
	if (txQueue)
	{
		PlaylistCopyInst * inst = NULL;
		txQueue->lock();
		for(i=0; i < l; i++)
		{
			if(pl[i].songid)
			{
				directAdd->Add((void*)pl[i].songid);
			}
			else
			{
				int r = dev->trackAddedToTransferQueue(pl[i].item);
				if(r)
				{
					m |= (-r);
					freeRecord(pl[i].item);
					continue;
				}
				if(!inst)
				{
					if(plid != -1) for(int i=0; i<directAdd->GetSize(); i++)
							dev->addTrackToPlaylist(plid,(songid_t)directAdd->Get(i));
					delete directAdd;
				}
				else
				{
					inst->plAddSongs = directAdd;
					AddTrackToTransferQueue(inst);
				}
				directAdd = new C_ItemList;
				inst = new PlaylistCopyInst(this,pl[i].item,name,plid);
			}
			freeRecord(pl[i].item);
		}
		if(inst)
		{
			inst->plAddSongs = directAdd;
			AddTrackToTransferQueue(inst);
		}
		else     // NULL inst means no transfers!
		{
			if(plid != -1) for(int i=0; i<directAdd->GetSize(); i++)
					dev->addTrackToPlaylist(plid,(songid_t)directAdd->Get(i));
			delete directAdd;
		}
		txQueue->unlock();
	}
	if (pl) free(pl);

	wchar_t warnStr[32] = {0};
	WASABI_API_LNGSTRINGW_BUF(IDS_WARNING,warnStr,32);
	if(m == 1) MessageBox(plugin.hwndLibraryParent,WASABI_API_LNGSTRINGW(IDS_NATS_DEVICE_MAYBE_FULL),warnStr,0);
	else if(m == 2) MessageBox(plugin.hwndLibraryParent,WASABI_API_LNGSTRINGW(IDS_NATS_SOME_OF_INCOMPATABLE_FORMAT),warnStr,0);
	else if(m == 3) MessageBox(plugin.hwndLibraryParent,WASABI_API_LNGSTRINGW(IDS_NATS_MAYBE_FULL_AND_INCOMPATABLE_FORMAT),warnStr,0);
	return 0;
}

void DeviceView::TransferAddCloudPlaylist(wchar_t * file, wchar_t * name0)
{
	if (!file) return;

	AGAVE_API_PLAYLISTS->Lock();
	for (size_t index = 0; index < AGAVE_API_PLAYLISTS->GetCount(); index++)
	{
		const wchar_t* filename = AGAVE_API_PLAYLISTS->GetFilename(index);
		if (!lstrcmpiW(filename, file))
		{
			int cloud = 1;
			if (AGAVE_API_PLAYLISTS->GetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud)) == API_PLAYLISTS_SUCCESS)
			{
				// not set as a cloud playlist so we need to set and then announce
				if (!cloud)
				{
					cloud = 1;
					AGAVE_API_PLAYLISTS->SetInfo(index, api_playlists_cloud, &cloud, sizeof(cloud));
					AGAVE_API_PLAYLISTS->Flush();
				}

				if (!cloud_hinst) cloud_hinst = (HINSTANCE)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_HINST);
				if (cloud_hinst && cloud_hinst != (HINSTANCE)1)
				{
					winampMediaLibraryPlugin *(*gp)();
					gp = (winampMediaLibraryPlugin * (__cdecl *)(void))GetProcAddress(cloud_hinst, "winampGetMediaLibraryPlugin");
					if (gp)
					{
						winampMediaLibraryPlugin *mlplugin = gp();
						if (mlplugin && (mlplugin->version >= MLHDR_VER_OLD && mlplugin->version <= MLHDR_VER))
						{
							mlplugin->MessageProc(0x406, index, 0, 0);
						}
					}
				}
				PostMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_LIBRARY_PLAYLISTS_REFRESH);
			}
			else
			{
			}
			break;
		}
	}
	AGAVE_API_PLAYLISTS->Unlock();
}

extern MLTREEITEMW mainTreeItem;

HWND hwndToolTips=NULL;

int DeviceView::AddTrackToTransferQueue(CopyInst * inst)
{
	LinkedQueue * txQueue = getTransferQueue(this);
	if (txQueue)
	{
		txQueue->Offer(inst);
		if(txQueue->GetSize() == 1)
			SetEvent(transferContext.notifier);
		device_update_map[0] = true;
		device_update_map[inst->dev] = true;
	}
	return 0;
}

int DeviceView::AddTrackToTransferQueue(DeviceView * device, itemRecordW * item, bool dupeCheck, bool forceDupe)
{
	SongCopyInst * inst = new SongCopyInst(device, item);

	if (dupeCheck)
	{
		LinkedQueue * txQueue = getTransferQueue(this);
		if (txQueue)
		{
			txQueue->lock();
			// current queue dupe check
			for(int i = 0; i < txQueue->GetSize(); i++)
			{
				if(((CopyInst *)txQueue->Get(i))->Equals(inst))
				{
					delete inst;
					txQueue->unlock();
					return 0;
				}
			}
			txQueue->unlock();
		}
	}

	if (!forceDupe)
	{
		int r = dev->trackAddedToTransferQueue(&inst->song);
		if (r)
		{
			if (r == 2)
			{
				inst->status = STATUS_DONE;
				inst->res = 2;
				AddTrackToTransferQueue(inst);
				return 0;
			}
			else
			{
				delete inst;
			}
		}
		else AddTrackToTransferQueue(inst);
		return r;
	}
	else
	{
		inst->res = 2;
		AddTrackToTransferQueue(inst);
		return 0;
	}
}

class SyncItemListLoader : public ifc_playlistloadercallback
{
public:
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
	{
		if(pos < len)
		{
			songs[pos].filename = _wcsdup(filename);
			metaToGet->Add(&songs[pos].map);
			songMaps->Add(&songs[pos].pladd);
		}
		pos++;
	}
	int pos,len;
	songMapping * songs;
	C_ItemList * metaToGet, * songMaps;
protected:
	RECVS_DISPATCH;
};

#define CBCLASS SyncItemListLoader
START_DISPATCH;
VCB(IFC_PLAYLISTLOADERCALLBACK_ONFILE, OnFile)
END_DISPATCH;
#undef CBCLASS

typedef struct
{
	mlPlaylistInfo info;
	songMapping * songs;
} SyncPlaylist;

void TransfersListUpdateItem(CopyInst * item);
void TransfersListUpdateItem(CopyInst * item, DeviceView *view);
/*
static int setPlaylistTrack(Device * dev, int pl, int n, int len, songid_t song) {
if(n >= len) { while(n >= len) { dev->addTrackToPlaylist(pl,song); len++; } return len; }
else {
dev->addTrackToPlaylist(pl,song);
dev->playlistSwapItems(pl,len,n);
dev->removeTrackFromPlaylist(pl,len);
}
return len;
}
*/
class PlaylistSyncCopyInst : public CopyInst
{
public:
	bool memFreed;
	C_ItemList *songMaps;
	C_ItemList * playlists;
	PlaylistSyncCopyInst(DeviceView *dev, C_ItemList *songMaps, C_ItemList * playlists) : songMaps(songMaps), playlists(playlists)
	{
		usesPreCopy = false;
		usesPostCopy = true;
		this->dev = dev;
		equalsType = -1;
		status=STATUS_WAITING;
		// status caption
		WASABI_API_LNGSTRINGW_BUF(IDS_WAITING,statusCaption,sizeof(statusCaption)/sizeof(wchar_t));
		// track caption
		WASABI_API_LNGSTRINGW_BUF(IDS_PLAYLIST_SYNCRONIZATION,trackCaption,sizeof(trackCaption)/sizeof(wchar_t));
		// type caption
		WASABI_API_LNGSTRINGW_BUF(IDS_OTHER,typeCaption,sizeof(typeCaption)/sizeof(wchar_t));
		memFreed=false;
	}

	virtual ~PlaylistSyncCopyInst()
	{
		freeMemory();
	}

	virtual bool CopyAction()
	{
		return false;
	}

	virtual void PostCopyAction()
	{
		SyncPlaylists(); freeMemory();
	}
	virtual void Cancelled()
	{
		freeMemory();
	}
	virtual bool Equals(CopyInst * b)
	{
		return false;
	}
	void freeMemory()
	{
		if(memFreed) return;
		memFreed=true;
		if(songMaps) delete songMaps;
		songMaps = NULL;
		if(playlists)
		{
			int l = playlists->GetSize();
			for(int i=0; i<l; i++)
			{
				SyncPlaylist * playlist = (SyncPlaylist *)playlists->Get(i);
				if(playlist)
				{
					if(playlist->songs)
					{
						for(int j=0; j<playlist->info.numItems; j++)
						{
							if(playlist->songs[j].ice)
							{
								freeRecord(playlist->songs[j].ice);
								free(playlist->songs[j].ice);
							}
							if(playlist->songs[j].filename)
								free(playlist->songs[j].filename);
						}
						free(playlist->songs);
					}
					free(playlist);
				}
			}
			delete playlists;
			playlists = NULL;
		}
	}
	void SyncPlaylists()
	{
		if(memFreed)
			return;
		WASABI_API_LNGSTRINGW_BUF(IDS_WORKING,statusCaption,sizeof(statusCaption)/sizeof(wchar_t));
		TransfersListUpdateItem(this);
		TransfersListUpdateItem(this, dev);
		MapItemRecordsToSongs(dev->dev,(PlaylistAddItem **)songMaps->GetAll(),songMaps->GetSize());
		int numPlaylists = playlists->GetSize();
		for(int i=0; i<numPlaylists; i++)
		{
			SyncPlaylist * playlist = (SyncPlaylist *)playlists->Get(i);
			int plnum = -1;
			bool done = false;
			int l = dev->dev->getPlaylistCount();
			int j;
			for(j=0; j < l; j++)
			{
				wchar_t buf[128] = {0};
				dev->dev->getPlaylistName(j,buf,128);
				if(wcscmp(buf,playlist->info.playlistName)) continue;
				int plen = dev->dev->getPlaylistLength(j);
				if(plen != playlist->info.numItems)
				{
					plnum = j;
					break;
				}
				for(int k=0; k<plen; k++)
				{
					if(playlist->songs[k].song != dev->dev->getPlaylistTrack(j,k))
					{
						plnum = j;
						break;
					}
				}
				if(plnum == -1)
				{
					done = true;
					break;
				}
			}
			if(done) continue;
			if(plnum == -1)
			{
				plnum = dev->CreatePlaylist(playlist->info.playlistName,true);
				if(plnum == -1) continue;
			}
			int plen = dev->dev->getPlaylistLength(plnum);
			while(plen && ((plen % 4) != 1)) dev->dev->removeTrackFromPlaylist(plnum,--plen); // avoid granulation boundarys
			int n=0;
			for(j=0; j<playlist->info.numItems; j++)
			{
				songid_t s = playlist->songs[j].song;
				if(s && (n>=plen || s != dev->dev->getPlaylistTrack(plnum,n)))
				{
					// begin set item code...
					if(n >= plen) while(n >= plen)
						{
							dev->dev->addTrackToPlaylist(plnum,s);
							plen++;
						}
					else
					{
						dev->dev->addTrackToPlaylist(plnum,s);
						dev->dev->playlistSwapItems(plnum,plen,n);
						dev->dev->removeTrackFromPlaylist(plnum,plen);
					}
					// end set item code
				}
				if(s) n++;
			}
			plen = dev->dev->getPlaylistLength(plnum);
			while(plen > n) dev->dev->removeTrackFromPlaylist(plnum,--plen);

			if(_wcsicmp(playlist->info.playlistName,L"Podcasts")==0)
			{
				wchar_t *name=NULL;
				for(int j=playlist->info.numItems-1; j>=0; j--)
				{
					wchar_t *n = getRecordExtendedItem(playlist->songs[j].ice,L"podcastchannel");
					if(!name) name=n;
					if(name && n)
					{
						if(_wcsicmp(name,n))
						{
							dev->dev->extraActions(DEVICE_ADDPODCASTGROUP,plnum,j+1,(intptr_t)name);
							name=n;
						}
						if(j==0) dev->dev->extraActions(DEVICE_ADDPODCASTGROUP,plnum,0,(intptr_t)name);
					}
				}
				dev->dev->extraActions(DEVICE_ADDPODCASTGROUP_FINISH,plnum,0,0);
			}
		}
		dev->DevicePropertiesChanges();
		freeMemory();
		WASABI_API_LNGSTRINGW_BUF(IDS_DONE,statusCaption,sizeof(statusCaption)/sizeof(wchar_t));
		TransfersListUpdateItem(this);
		TransfersListUpdateItem(this, dev);
	}
};

static bool shouldSyncPlaylist(wchar_t * name, C_Config * config)
{
	wchar_t buf[150] = {0};
	StringCchPrintf(buf,150, L"sync-%s",name);
	return config->ReadInt(buf,0) == config->ReadInt(L"plsyncwhitelist",1);
}

static int sortfunc_podcastpubdate(const void *elem1, const void *elem2)
{
	itemRecordW *ar = (itemRecordW *)elem1;
	itemRecordW *br = (itemRecordW *)elem2;
	wchar_t *a = getRecordExtendedItem(ar,L"podcastpubdate");
	wchar_t *b = getRecordExtendedItem(br,L"podcastpubdate");
	if(!a) a = L"0";
	if(!b) b = L"0";
	return _wtoi(b) - _wtoi(a);
}

void DeviceView::OnActivityStarted()
{
	for ( ifc_deviceevent *l_event_handler : event_handlers )
		l_event_handler->ActivityStarted( this, this );
}

void DeviceView::OnActivityChanged()
{
	for ( ifc_deviceevent *l_event_handler : event_handlers )
		l_event_handler->ActivityChanged( this, this );
}

void DeviceView::OnActivityFinished()
{
	for ( ifc_deviceevent *l_event_handler : event_handlers )
		l_event_handler->ActivityFinished( this, this );
}

void DeviceView::UpdateActivityState()
{
	LinkedQueue * txQueue = getTransferQueue(this);
	if (txQueue && FALSE == activityRunning)
	{
		if (0 != txQueue->GetSize())
		{
			activityRunning = TRUE;

			if (FAILED(GetProgress(&currentProgress)))
				currentProgress = 0;
			 
			OnActivityStarted();
		}
	}
	else
	{
		if (txQueue && 0 == txQueue->GetSize())
		{
			activityRunning = FALSE;
			OnActivityFinished();
		}
		else
		{
			unsigned int percent;
			if (FAILED(GetProgress(&percent)) || 
				percent != currentProgress)
			{
				currentProgress = percent;
				OnActivityChanged();
			}
		}
	}
}

void DeviceView::UpdateSpaceInfo(BOOL updateUsedSpace, BOOL notifyChanges)
{
	uint64_t total, used;
	unsigned int changes;

	changes = 0;

	total = dev->getDeviceCapacityTotal();
	if (total != totalSpace)
	{
		totalSpace = total;
		changes |= (1 << 0);	
	}

	if (FALSE != updateUsedSpace)
	{
		used = dev->getDeviceCapacityAvailable();
		if (used > total)
			used = total;

		used = total - used;

		if (used != usedSpace)
		{
			usedSpace = used;
			changes |= (1 << 1);
		}
	}

	if (0 != changes && FALSE != notifyChanges)
	{
		for ( ifc_deviceevent *l_event_handler : event_handlers )
		{
			if (0 != ((1 << 0) & changes))
				l_event_handler->TotalSpaceChanged(this, totalSpace);
			if (0 != ((1 << 1) & changes))
				l_event_handler->TotalSpaceChanged(this, usedSpace);
		}
	}
}


void DeviceView::OnNameChanged(const wchar_t *new_name)
{
	for ( ifc_deviceevent *l_event_handler : event_handlers )
		l_event_handler->DisplayNameChanged( this, new_name );
}

void DeviceView::Sync(bool silent)
{
	// sync configuration settings....
	bool syncAllLibrary = config->ReadInt(L"syncAllLibrary",1)!=0;

	if (AGAVE_API_STATS)
	{
		wchar_t device_name[128] = {0};
		device_name[0] = 0;
		if(dev->extraActions(DEVICE_GET_MODEL, (intptr_t)device_name, 128, 0) == 1 && device_name[0])
		{
			AGAVE_API_STATS->SetString("pmp", device_name);
		}
	}

	HWND centerWindow = CENTER_OVER_ML_VIEW;
	UpdateActivityState();

	C_ItemList mllist;
	wchar_t * querystring=0;
	itemRecordListW *results = 0;
	if(syncAllLibrary)
	{
		querystring = _wcsdup(config->ReadString(L"SyncQuery",L"type=0"));
		results = (AGAVE_API_MLDB ? AGAVE_API_MLDB->Query(querystring) : NULL);
		if (results)
			for(int i = 0; i < results->Size; i++) mllist.Add(&results->Items[i]);
	}

	// read playlists/views and find out what else needs to be added
	PlaylistSyncCopyInst * sync = NULL;
	C_ItemList filenameMaps;
	C_ItemList *songMaps = new C_ItemList;
	C_ItemList * playlists = new C_ItemList;

	// first collect playlists without metadata
	SyncItemListLoader list;
	list.metaToGet = &filenameMaps;
	list.songMaps = songMaps;
	int playlistsnum = SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, 0, ML_IPC_PLAYLIST_COUNT);
	for(int i=0; i<playlistsnum; i++)
	{
		SyncPlaylist* playlist = (SyncPlaylist*)calloc(sizeof(SyncPlaylist),1);
		playlist->info.size = sizeof(mlPlaylistInfo);
		playlist->info.playlistNum = i;
		SendMessage(plugin.hwndLibraryParent, WM_ML_IPC, (WPARAM)&playlist->info, ML_IPC_PLAYLIST_INFO);
		if(shouldSyncPlaylist(playlist->info.playlistName, config))
		{
			playlists->Add(playlist);
		}
		else
		{
			free(playlist);
			playlist = 0;
			continue;
		}
		//if(playlist->info.numItems <= 1)
		{
			list.pos = list.len = 0;
			playlistManager->Load(playlist->info.filename, &list);
			playlist->info.numItems = list.pos;
		}
		list.pos = 0;
		list.len=playlist->info.numItems;
		list.songs = playlist->songs = (songMapping*)calloc(sizeof(songMapping), list.len);
		playlistManager->Load(playlist->info.filename, &list);
	}
	mapFilesToItemRecords((filenameMap **)filenameMaps.GetAll(), filenameMaps.GetSize(), centerWindow); // get metadata

	// now sync podcasts...
	if (dev->extraActions(DEVICE_SUPPORTS_PODCASTS, 0, 0, 0) == 0)
	{
		int podcasteps = config->ReadInt(L"podcast-sync_episodes",0);
		int podcastsnum = AGAVE_API_PODCASTS ? AGAVE_API_PODCASTS->GetNumPodcasts() : 0;
		if(podcasteps && podcastsnum > 0)
		{
			// if we want to sync podcasts and we have podcasts to sync
			bool all = !!config->ReadInt(L"podcast-sync_all", 1);
			SyncPlaylist * s = (SyncPlaylist *)calloc(sizeof(SyncPlaylist),1);
			lstrcpyn(s->info.playlistName, L"Podcasts", 128); //set the name of the playlist containing our podcasts
			int n = 0, alloc = 512;
			s->songs = (songMapping*)calloc(alloc, sizeof(songMapping));
			for(int i = 0; i < podcastsnum; i++)
			{
				ifc_podcast *podcast = AGAVE_API_PODCASTS->EnumPodcast(i);
				if(podcast)
				{
					wchar_t podcast_name[256] = {0};
					if(podcast->GetTitle(podcast_name, 256) == 0)
					{
						wchar_t buf[300] = {0};
						StringCchPrintf(buf, 300, L"podcast-sync-%s", podcast_name);
						if(podcast_name[0] && (all || config->ReadInt(buf,0)))   // if we have a podcast and we want to sync it
						{
							wchar_t query[300] = {0};
							StringCchPrintf(query, 300, L"podcastchannel = \"%s\"", podcast_name);
							itemRecordListW *podcasts = AGAVE_API_MLDB->Query(query);
							if(podcasts)
							{
								qsort(podcasts->Items,podcasts->Size,sizeof(itemRecordW),sortfunc_podcastpubdate); // sort the podcasts into publish date order
								for(int j=0; j<podcasts->Size && (podcasteps == -1 || j < podcasteps); j++)
								{
									// add podcast to playlist
									if(n >= alloc)
									{
										size_t old_alloc = alloc;
										alloc += 512;
										songMapping* new_songs = (songMapping*)realloc(s->songs,sizeof(songMapping) * alloc);
										if (new_songs)
										{
											s->songs = new_songs;
										}
										else
										{
											new_songs = (songMapping*)malloc(sizeof(songMapping) * alloc);
											if (new_songs)
											{
												memcpy(new_songs, s->songs, sizeof(songMapping) * old_alloc);
												free(s->songs);
												s->songs = new_songs;
											}
											else
											{
												alloc = old_alloc;
												continue;
											}
										}
									}
									ZeroMemory(&s->songs[n],sizeof(songMapping));
									s->songs[n].ice = (itemRecordW*)calloc(sizeof(itemRecordW), 1);
									copyRecord(s->songs[n].ice,&podcasts->Items[j]);
									mllist.Add(s->songs[n].ice);
									songMaps->Add(&s->songs[n].pladd);
									n++;
								}
								if(podcasts)
									AGAVE_API_MLDB->FreeRecordList(podcasts);
							}
						}
					}
				}
			}
			s->info.numItems = n;
			if(n)
				playlists->Add(s);
			else
			{
				free(s->songs);
				free(s);
			}
		}
	}
	// now collect playlists with metadata (i.e, smart views)
	// except the new ml_local isn't ready.
	// calloc a new SyncPlaylist, fill in playlist->info.numItems, playlist->info.playlistName and playlist->songs[].ice then add to playlists.

	// add tracks to be sync'd
	for(int i=0; i<filenameMaps.GetSize(); i++)
	{
		filenameMap* f = (filenameMap*)filenameMaps.Get(i);
		if(f->ice)
			mllist.Add(f->ice);
	}
	// prepare sync
	if(playlists->GetSize())
		sync = new PlaylistSyncCopyInst(this, songMaps, playlists);
	else
	{
		delete playlists;
		delete songMaps;
	}

	// work out the tracks to be sent and deleted...
	C_ItemList synclist,dellist;

	ProcessDatabaseDifferences(dev, &mllist, NULL, &synclist, NULL, &dellist);

	if(!synclist.GetSize() && !dellist.GetSize())
	{
		// nothing to do
		if(sync)
		{
			sync->SyncPlaylists();
			delete sync;
		}
		if(!silent)
		{
			wchar_t titleStr[32] = {0};
			MessageBox(plugin.hwndLibraryParent,
			           WASABI_API_LNGSTRINGW(IDS_NOTHING_TO_SYNC_UP_TO_DATE),
			           WASABI_API_LNGSTRINGW_BUF(IDS_SYNC, titleStr, 32),0);
		}
	}
	else
	{
		// need to sync some tracks
		if(IDOK == SyncDialog_Show(centerWindow, this, &synclist, &dellist, FALSE))
		{
			config->WriteInt(L"syncOnConnect_time",(int)time(NULL));
			if(dellist.GetSize())
			{
				switch(config->ReadInt(L"TrueSync",0))
				{
					case 1: this->DeleteTracks(&dellist, centerWindow); break;
					case 2: this->CopyTracksToHardDrive(&dellist); break;
				}
			}

			int i = 0, l = 0;
			LinkedQueue * txQueue = getTransferQueue(this);
			if (txQueue)
			{
				l = synclist.GetSize();
				txQueue->lock();
				for(i = 0; i < l; i++) if(AddTrackToTransferQueue(this, (itemRecordW*)synclist.Get(i), false) == -1) break;
				if(sync) AddTrackToTransferQueue(sync);
				txQueue->unlock();
			}

			if(i != l)
			{
				wchar_t titleStr[128] = {0};
				MessageBox(plugin.hwndLibraryParent,
				           WASABI_API_LNGSTRINGW(IDS_THERE_IS_NOT_ENOUGH_SPACE_ON_THE_DEVICE),
				           WASABI_API_LNGSTRINGW_BUF(IDS_NOT_ENOUGH_SPACE, titleStr, ARRAYSIZE(titleStr)),
						   MB_OK | MB_ICONWARNING);
			}
		}
		else
		{
			if(sync) delete sync;
		}
	}

	if(syncAllLibrary)
	{
		if(results)
			AGAVE_API_MLDB->FreeRecordList(results);
		free(querystring);
	}
}

void DeviceView::CloudSync(bool silent)
{
	if (AGAVE_API_STATS)
	{
		wchar_t device_name[128] = {0};
		if(dev->extraActions(DEVICE_GET_MODEL, (intptr_t)device_name, 128, 0) == 1 && device_name[0])
		{
			AGAVE_API_STATS->SetString("pmp", device_name);
		}
	}

	UpdateActivityState();

	// work out the tracks to be sent...
	C_ItemList *filenameMaps2 = new C_ItemList, synclist;
	DeviceView * hss = 0, * local = 0;

	if (!cloud_hinst) cloud_hinst = (HINSTANCE)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_CLOUD_HINST);
	if (cloud_hinst && cloud_hinst != (HINSTANCE)1)
	{
		winampMediaLibraryPlugin *(*gp)();
		gp = (winampMediaLibraryPlugin * (__cdecl *)(void))GetProcAddress(cloud_hinst, "winampGetMediaLibraryPlugin");
		if (gp)
		{
			winampMediaLibraryPlugin *mlplugin = gp();
			if (mlplugin && (mlplugin->version >= MLHDR_VER_OLD && mlplugin->version <= MLHDR_VER))
			{
				// determine the cloud device and alter the device
				// to be checked with as needed by the action done
				for(int i = 0; i < devices.GetSize(); i++) 
				{
					DeviceView * d = (DeviceView *)devices.Get(i);

					if (d->isCloudDevice)
					{
						char name[128] = {0};
						if (d->dev->extraActions(DEVICE_GET_UNIQUE_ID, (intptr_t)name, sizeof(name), 0))
						{
							if (!strcmp(name, "hss"/*HSS_CLIENT*/))
								hss = d;
							else if (!strcmp(name, "local_desktop"))
								local = d;
						}
					}
				}

				if (local && hss && local->dev == dev)
				{
					// just use the local library as the source to compare against
					mlplugin->MessageProc(0x403, /*source*/-1, /*dest*/hss->dev->extraActions(DEVICE_GET_CLOUD_DEVICE_ID,0,0,0), (INT_PTR)filenameMaps2);
				}
				else
				{
					// just use the local library as the source to compare against
					mlplugin->MessageProc(0x403, /*source*/-1, /*dest*/dev->extraActions(DEVICE_GET_CLOUD_DEVICE_ID,0,0,0), (INT_PTR)filenameMaps2);
				}
			}
		}
	}

	synclist = *fileListToItemRecords(filenameMaps2, CENTER_OVER_ML_VIEW);
	nu::qsort(synclist.GetAll(), synclist.GetSize(), sizeof(void*), dev, compareSongs);

	if(!synclist.GetSize())
	{
		if(!silent)
		{
			wchar_t titleStr[32] = {0};
			MessageBox(plugin.hwndLibraryParent,
			           WASABI_API_LNGSTRINGW(IDS_NOTHING_TO_SYNC_UP_TO_DATE),
			           WASABI_API_LNGSTRINGW_BUF(IDS_SYNC,titleStr,32),0);
		}
	}
	else
	{
		DeviceView * destDevice = (local && hss && local->dev == dev ? hss : this);
		// need to sync some tracks
		if(IDOK == SyncCloudDialog_Show(CENTER_OVER_ML_VIEW, destDevice, &synclist))
		{
			int l = synclist.GetSize();
			cloudTransferQueue.lock();
			int i = 0;
			for (; i < l; i++) if (AddTrackToTransferQueue(destDevice, (itemRecordW*)synclist.Get(i), false) == -1) break;
			cloudTransferQueue.unlock();

			if(i != l)
			{
				wchar_t titleStr[128] = {0};
				MessageBox(plugin.hwndLibraryParent,
				           WASABI_API_LNGSTRINGW(IDS_THERE_IS_NOT_ENOUGH_SPACE_ON_THE_DEVICE),
				           WASABI_API_LNGSTRINGW_BUF(IDS_NOT_ENOUGH_SPACE, titleStr, ARRAYSIZE(titleStr)),
						   MB_OK | MB_ICONWARNING);
			}
		}
	}
}

extern itemRecordListW * generateAutoFillList(DeviceView * dev, C_Config * config); // from autofill.cpp

void DeviceView::Autofill()
{
	HWND centerWindow;

	centerWindow = CENTER_OVER_ML_VIEW;

	if (AGAVE_API_STATS)
	{
		wchar_t device_name[128] = {0};
		if(dev->extraActions(DEVICE_GET_MODEL, (intptr_t)device_name, 128, 0) == 1 && device_name[0])
		{
			AGAVE_API_STATS->SetString("pmp", device_name);
		}
	}

	UpdateActivityState();

	C_ItemList delList,sendList;

	itemRecordListW * autofillList = generateAutoFillList(this,config);
	ProcessDatabaseDifferences(dev,autofillList,NULL,&sendList,NULL,&delList);
	
	if(IDOK == SyncDialog_Show(centerWindow, this, &sendList, &delList, TRUE))
	{
		config->WriteInt(L"syncOnConnect_time", (int)time(NULL));
		// delete all tracks in delList
		if(IDOK == DeleteTracks(&delList, centerWindow))
		{
			// not aborted
			// send all tracks in sendList
			LinkedQueue * txQueue = getTransferQueue(this);
			if (txQueue)
			{
				txQueue->lock();
				for(int i = 0; i < sendList.GetSize(); i++) AddTrackToTransferQueue(this, (itemRecordW*)sendList.Get(i), false);
				txQueue->unlock();
			}
		}
	}
	if(autofillList)
		freeRecordList(autofillList);
}

extern int serverPort;

bool DeviceView::PlayTracks(C_ItemList * tracks, int startPlaybackAt, bool enqueue, bool msgIfImpossible, HWND parent)
{
	if(tracks->GetSize() == 0) return true;
	// direct playback?
	if(dev->playTracks((songid_t*)tracks->GetAll(),tracks->GetSize(),startPlaybackAt,enqueue))
		return true;
	if(serverPort>0 && dev->copyToHardDriveSupported())
	{
		// indirect playback?
		if(!enqueue)
		{
			//clear playlist
			SendMessage(plugin.hwndWinampParent,WM_WA_IPC,0,IPC_DELETE);
		}

		wchar_t buf[2048] = {0};
		dev->getPlaylistName(0,buf,128);
		AutoUrl device(buf);
		for(int i=0; i<tracks->GetSize(); i++)
		{
			songid_t s = (songid_t)tracks->Get(i);
			//encode fields to url format
			wchar_t metadata[2048] = {0};
			dev->getTrackArtist(s,metadata,2048);
			AutoUrl artist(metadata);
			dev->getTrackAlbum(s,metadata,2048);
			AutoUrl album(metadata);
			dev->getTrackTitle(s,metadata,2048);
			AutoUrl title(metadata);

			// construct URL
			wchar_t ext[10]=L"";
			dev->getTrackExtraInfo(s,L"ext",ext,10);
			char buf[8192] = {0};
			StringCchPrintfA(buf,8192, "http://127.0.0.1:%d/?a=%s&l=%s&t=%s&d=%s%s%s",serverPort,artist,album,title,device,*ext?";.":"",(char*)AutoChar(ext));
			// get title
			AutoWide wideUrl(buf);

			wchar_t buf2[4096] = {0};
			getTitle(dev,s,wideUrl,buf2,4096);
			// enqueue file
			enqueueFileWithMetaStructW ef = { wideUrl, buf2, NULL, dev->getTrackLength( s ) / 1000 };
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&ef, IPC_PLAYFILEW);
		}
		if(!enqueue)   //play item startPlaybackAt
		{
			SendMessage(plugin.hwndWinampParent,WM_WA_IPC,startPlaybackAt,IPC_SETPLAYLISTPOS);
			SendMessage(plugin.hwndWinampParent,WM_COMMAND,40047,0); //stop
			SendMessage(plugin.hwndWinampParent,WM_COMMAND,40045,0); //play
		}
		return true;
	}
	if(msgIfImpossible)
	{
		wchar_t titleStr[32] = {0};
		MessageBox(parent,WASABI_API_LNGSTRINGW(IDS_DOES_NOT_SUPPORT_DIRECT_PLAYBACK),
		           WASABI_API_LNGSTRINGW_BUF(IDS_UNSUPPORTED,titleStr,32),0);
	}
	return false;
}

bool DeviceView::PlayPlaylist(int playlistId, bool enqueue, bool msgIfImpossible, HWND parent)
{
	int l = dev->getPlaylistLength(playlistId);
	C_ItemList tracks;
	for(int j=0; j<l; j++) 
		tracks.Add((void*)dev->getPlaylistTrack(playlistId,j));
	return PlayTracks(&tracks, 0, enqueue, msgIfImpossible, parent);
}

void DeviceView::CopyTracksToHardDrive(C_ItemList * tracks)
{
	CopyTracksToHardDrive((songid_t*)tracks->GetAll(),tracks->GetSize());
}

static void getReverseCopyFilenameFormat(wchar_t* filepath, wchar_t* format, int len, BOOL * uppercaseext)
{
	wchar_t m_def_extract_path[MAX_PATH] = L"C:\\My Music";
	wchar_t m_def_filename_fmt[MAX_PATH] = L"<Artist> - <Album>\\## - <Trackartist> - <Title>";
	GetDefaultSaveToFolder(m_def_extract_path);
	bool cdrip = !!global_config->ReadInt(L"extractusecdrip", 1);
	const wchar_t *mlinifile = (const wchar_t*)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETMLINIFILEW);

	wchar_t buf[2048] = {0};
	if(cdrip) GetPrivateProfileString(L"gen_ml_config",L"extractpath",m_def_extract_path,buf,2048,mlinifile);
	else lstrcpyn(buf,global_config->ReadString(L"extractpath",m_def_extract_path),2048);
	lstrcpyn(filepath,buf,len);
	int l = wcslen(filepath);
	if(*(filepath+l-1) != L'\\')
	{
		*(filepath+l) = L'\\';
		*(filepath+l+1)=0;
		l++;
	}
	if(cdrip) GetPrivateProfileString(L"gen_ml_config",L"extractfmt2",m_def_filename_fmt,buf,2048,mlinifile);
	else lstrcpyn(buf,global_config->ReadString(L"extractfmt2",m_def_filename_fmt),2048);
	if(l < len) lstrcpyn(format/*+l*/,buf,len - l);
	if(cdrip) *uppercaseext = GetPrivateProfileInt(L"gen_ml_config",L"extractucext",0,mlinifile);
	else *uppercaseext = global_config->ReadInt(L"extractucext",0);
}

void DeviceView::CopyTracksToHardDrive(songid_t * tracks, int numTracks)
{
	if(!dev->copyToHardDriveSupported()) return;
	BOOL uppercaseext=FALSE;
	wchar_t filepath[MAX_PATH] = {0}, format[2048] = {0};
	getReverseCopyFilenameFormat(filepath,format,2048,&uppercaseext);

	LinkedQueue * txQueue = getTransferQueue(this);
	if (txQueue)
	{
		txQueue->lock();
		for(int i=0; i<numTracks; i++)
		{
			AddTrackToTransferQueue(new ReverseCopyInst(this,filepath,format,tracks[i],true,!!uppercaseext));
		}
		txQueue->unlock();
	}
}

void DeviceView::CopyPlaylistToLibrary(int plnum)
{
	if(plnum==0) return;
	wchar_t name[128] = {0};
	dev->getPlaylistName(plnum,name,128);
	wchar_t filename[MAX_PATH] = {0};
	wchar_t dir[MAX_PATH] = {0};

	GetTempPath(MAX_PATH,dir);
	GetTempFileName(dir,L"pmppl",0,filename);
	_wunlink(filename);
	{
		wchar_t * ext = wcsrchr(filename,L'.');
		if(ext) *ext=0;
		StringCchCat(filename,MAX_PATH,L".m3u");
	}
	FILE * f = _wfopen(filename,L"wt"); if(f)
	{
		fputws(L"#EXTM3U\n",f);
		fclose(f);
	}
	/*
	mlMakePlaylistV2 a = {sizeof(mlMakePlaylistV2),name,ML_TYPE_FILENAMES,"\0\0",PL_FLAG_SHOW | PL_FLAG_FILL_FILENAME};
	SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&a,ML_IPC_PLAYLIST_MAKE);
	*/

	wchar_t filepath[MAX_PATH] = {0}, format[2048] = {0};
	BOOL uppercaseext=FALSE;
	getReverseCopyFilenameFormat(filepath,format,2048,&uppercaseext);
	int l = dev->getPlaylistLength(plnum);

	LinkedQueue * txQueue = getTransferQueue(this);
	if (txQueue)
	{
		txQueue->lock();
		for(int i=0; i<l; i++)
			AddTrackToTransferQueue(new ReversePlaylistCopyInst(this,filepath,format,dev->getPlaylistTrack(plnum,i),filename,name,i==l-1,true));
		txQueue->unlock();
	}
}

void DeviceView::Unregister()
{
	for(size_t i=0; i < playlistTreeItems.size(); i++)
	{
		HNAVITEM item = playlistTreeItems[i];
		// TODO: free memory associated with the text for item
		MLNavCtrl_DeleteItem(plugin.hwndLibraryParent, item);
	}
	playlistTreeItems.clear();

	if (videoTreeItem)
		MLNavCtrl_DeleteItem(plugin.hwndLibraryParent, videoTreeItem);
	videoTreeItem=0;

	if (AGAVE_API_DEVICEMANAGER)
		AGAVE_API_DEVICEMANAGER->DeviceUnregister(name);
	if (treeItem)
		MLNavCtrl_DeleteItem(plugin.hwndLibraryParent, treeItem);
	treeItem=0;
}