#ifndef __DEVICEVIEW_H_
#define __DEVICEVIEW_H_

//#define _WIN32_WINNT 0x0400

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <shlobj.h>
#include <shellapi.h>
#include "..\..\General\gen_ml/ml.h"
#include "..\..\General\gen_ml/itemlist.h"
#include "..\..\General\gen_ml/childwnd.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/ipc_pe.h"
#include "LinkedQueue.h"
#include "pmp.h"
#include "resource1.h"
#include "config.h"
#include "transfer_thread.h"
#include "api__ml_pmp.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"

#include <bfc/platform/types.h>
#include "../devices/ifc_device.h"
#include "../devices/ifc_deviceactivity.h"
#include "pmp.h"
#include <bfc/multipatch.h>
#include <vector>

#define COMMITTIMERID 0x2345

extern C_Config * gen_mlconfig;
extern LinkedQueue cloudTransferQueue, cloudFinishedTransfers;
extern int cloudTransferProgress;
extern winampMediaLibraryPlugin plugin;

wchar_t *guessTitles(const wchar_t *filename, int *tracknum,wchar_t **artist, wchar_t **album,wchar_t **title); // free result after using artist, etc
wchar_t* GetDefaultSaveToFolder(wchar_t* path_to_store);

#define AVERAGEBASIS 3
class TransferContext
{
public:
	TransferContext() : dev(0), transfer_thread(NULL)
	{
		numTransfers=0;
		start=0; 
		end=0;
		InitializeCriticalSection(&transfer_lock);
		killer = CreateEvent(NULL, TRUE, FALSE, 0);
		notifier = CreateEvent(NULL, FALSE, FALSE, 0);
		paused = 0;
		ZeroMemory(&times,sizeof(times));
	}

	~TransferContext()
	{
		DeleteCriticalSection(&transfer_lock);
		CloseHandle(killer);
		CloseHandle(notifier);
	}

	void WaitForKill()
	{
		SetEvent(notifier);
		WaitForSingleObject(killer, INFINITE);
	}
	void DoOneTransfer(HANDLE handle);
	bool IsPaused();
	void Pause();
	void Resume();
	
	static bool IsAllPaused();
	static void PauseAll();
	static void ResumeAll();
	
	int numTransfers;
	int times[AVERAGEBASIS];
	time_t start, end;
	DeviceView *dev;
	volatile size_t paused;
	HANDLE killer, notifier;
	CRITICAL_SECTION transfer_lock;
	ThreadID *transfer_thread;

	static volatile size_t paused_all;
};

enum
{
	PATCH_IFC_DEVICE,
	PATCH_IFC_DEVICEACTIVITY,
};

class DeviceView : public MultiPatch<PATCH_IFC_DEVICE, ifc_device>,
	public MultiPatch<PATCH_IFC_DEVICEACTIVITY, ifc_deviceactivity>
{
public://protected:
	HNAVITEM treeItem, videoTreeItem, queueTreeItem;
	int queueActiveIcon;
	int isCloudDevice;
	std::vector<HNAVITEM> playlistTreeItems;
	LinkedQueue finishedTransfers;
	
	HNAVITEM AddPlaylistNode(int id);
	int AddTrackToTransferQueue(DeviceView * device, itemRecordW * item, bool dupeCheck, bool forceDupe=false); // true on success
	int AddTrackToTransferQueue(CopyInst * inst);
public:
	TransferContext transferContext;
	int videoView;
	int SyncConnectionDefault;
	prefsDlgRecW devPrefsPage;
	int currentTransferProgress; // percentage
	double transferRate;
	int threadKillswitch;
	LinkedQueue transferQueue;
	bool commitNeeded;
	C_Config * config;
	Device * dev;
	int metadata_fields;
	DeviceView(Device *dev);

	void SetVideoView(BOOL enabled);
	bool GetTransferFromMlSupported(int dataType);
	intptr_t TransferFromML(int type,void* data, int unsupportedReturn, int supportedReturn, int playlist=0);
	intptr_t TransferFromDrop(HDROP hDrop, int playlist=0);
	intptr_t MessageProc(int message_type, intptr_t param1, intptr_t param2, intptr_t param3);
	void DevicePropertiesChanges();
	int CreatePlaylist(wchar_t * name=NULL, bool silent=false);
	void RenamePlaylist(int id);
	bool DeletePlaylist(int playlistId, bool deleteFiles, bool verbal);
	int AddFileListToTransferQueue(char ** files, int num, int playlist=0);
	int AddFileListToTransferQueue(wchar_t ** files, int num, int playlist=0);
	int AddItemListToTransferQueue(itemRecordListW * items, int playlist=0);
	int AddItemListToTransferQueue(C_ItemList * items, int playlist=0);
	void TransferPlaylist(wchar_t * file, wchar_t * name=NULL); // name=NULL when its from the ML and we must find out ourself...
	int TransferTracksToPlaylist(C_ItemList *itemRecords, int plid);
	void TransferAddCloudPlaylist(wchar_t * file, wchar_t * name0);
	int DeleteTracks(C_ItemList * tracks, HWND centerWindow);
	void Sync(bool silent = false);
	void CloudSync(bool silent = false);
	void Autofill();
	void Eject();
	bool PlayTracks(C_ItemList * tracks, int startPlaybackAt, bool enqueue, bool msgIfImpossible, HWND parent=NULL); // returns false if failed/unsupported
	bool PlayPlaylist(int playlistId, bool enqueue, bool msgIfImpossible, HWND parent=NULL);
	void CopyTracksToHardDrive(songid_t * tracks, int numTracks);
	void CopyTracksToHardDrive(C_ItemList * tracks);
	void CopyPlaylistToLibrary(int plnum);
	void OnActivityStarted();
	void OnActivityChanged();
	void OnActivityFinished();
	void OnNameChanged(const wchar_t *new_name);
//private:
	void RegisterViews(HNAVITEM parent);
	void Unregister();

	BOOL DisplayDeviceContextMenu(HNAVITEM item, HWND hostWindow, POINT pt);
	BOOL DisplayPlaylistContextMenu(int playlistId, HNAVITEM item, HWND hostWindow, POINT pt);

// navigation
	BOOL Navigation_IsPlaylistItem(HNAVITEM item, int *playlistId);
	HWND Navigation_CreateViewCb(HNAVITEM item, HWND parentWindow);
	BOOL Navigation_ShowContextMenuCb(HNAVITEM item, HWND hostWindow, POINT pt);
	BOOL Navigation_ClickCb(HNAVITEM item, int actionType, HWND hostWindow);
	int Navigation_DropTargetCb(HNAVITEM item, unsigned int dataType, void *data);
	BOOL Navigation_TitleEditBeginCb(HNAVITEM item);
	BOOL Navigation_TitleEditEndCb(HNAVITEM item, const wchar_t *title);
	BOOL Navigation_KeyDownCb(HNAVITEM item, NMTVKEYDOWN *keyData, HWND hwnd);

	size_t GetPlaylistName(wchar_t *buffer, size_t bufferSize, int playlistId, const wchar_t *defaultName, BOOL quoteSpaces);

public:
	/* ifc_device */
	int QueryInterface(GUID interface_guid, void **object);
	const char *GetName(); 
	HRESULT GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height);
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize);

	const char *GetType();
	const char *GetDisplayType();
	const char *GetConnection();

	BOOL GetHidden();
	
	HRESULT GetTotalSpace(uint64_t *size);
	HRESULT GetUsedSpace(uint64_t *size);

	BOOL GetAttached();
	HRESULT Attach(HWND hostWindow);
	HRESULT Detach(HWND hostWindow);
	
	HRESULT EnumerateCommands(ifc_devicesupportedcommandenum **enumerator, DeviceCommandContext context);
	HRESULT SendCommand(const char *command, HWND hostWindow, ULONG_PTR param);
	HRESULT GetCommandFlags(const char *command, DeviceCommandFlags *flags);

	HRESULT GetActivity(ifc_deviceactivity **activity);

	HRESULT Advise(ifc_deviceevent *handler);
	HRESULT Unadvise(ifc_deviceevent *handler);

	HWND CreateView(HWND parentWindow);
	void SetNavigationItem(void *navigationItem); 

	HRESULT GetDropSupported(unsigned int dataType);
	HRESULT Drop(void *data, unsigned int dataType);

	HRESULT SetDisplayName(const wchar_t *displayName, bool force);

	HRESULT GetModel(wchar_t *buffer, size_t bufferSize);
	HRESULT GetStatus(wchar_t *buffer, size_t bufferSize);

	void UpdateActivityState();
	void UpdateSpaceInfo(BOOL updateUsedSpace, BOOL notifyChanges);

	/* ifc_deviceactivity */
	BOOL GetActive();
	BOOL GetCancelable();
	HRESULT GetProgress(unsigned int *percentCompleted);
	HRESULT Activity_GetDisplayName(wchar_t *buffer, size_t bufferMax);
	HRESULT Activity_GetStatus(wchar_t *buffer, size_t bufferMax);
	HRESULT Cancel(HWND hostWindow);

	/* Dispatchable */
		size_t AddRef()
	{
		return InterlockedIncrement((LONG*)&ref_count);
	}

	size_t Release()
	{
		if (0 == ref_count)
			return ref_count;

		LONG r = InterlockedDecrement((LONG*)&ref_count);
		if (0 == r)
			delete(this);

		return r;
	}

private:
	RECVS_MULTIPATCH;
	std::vector<ifc_deviceevent*> event_handlers;
	char name[128];
	size_t ref_count;
	~DeviceView();
	const char *connection_type;
	const char *display_type;
	BOOL navigationItemCreated;
	BOOL activityRunning;
	unsigned int currentProgress;
	uint64_t usedSpace;
	uint64_t totalSpace;
};

#endif