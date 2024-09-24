#ifndef NULLSOFT_MAINH
#define NULLSOFT_MAINH

#include "Wire.h"
#include "Downloaded.h"

#define PLUGIN_VERSION_MAJOR	1
#define PLUGIN_VERSION_MINOR	80

#define SERVICE_PODCAST			720
#define SERVICE_SUBSCRIPTION	721
#define SERVICE_DOWNLOADS		722

#define BAD_CHANNEL		((size_t)-1)
#define BAD_ITEM		((size_t)-1)

void SaveChannels(ChannelList &channels);
void SaveAll(bool rss_only=false);
void HookTerminate();

void DestroyLoader(HWND);
void BuildLoader(HWND);
extern int winampVersion;
void addToLibrary(const DownloadedFile& d); // call in winamp main thread only
void addToLibrary_thread(const DownloadedFile& d); // call from any thread

bool AddPodcastData(const DownloadedFile &data);
bool IsPodcastDownloaded(const wchar_t *url);
void CloseDatabase();

#include "resource.h"
#include "../nu/DialogSkinner.h"
#include "../nu/MediaLibraryInterface.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoLock.h"
#include "..\..\General\gen_ml/menu.h"
#include <windows.h>
#include <shlwapi.h>

extern ATOM VIEWPROP;
extern winampMediaLibraryPlugin plugin;

#include "../Components/wac_downloadManager/wac_downloadManager_api.h"

#define ML_ENQDEF_VAL() (!!GetPrivateProfileInt(L"gen_ml_config", L"enqueuedef", 0, ml_cfg))
#define ML_GROUPBTN_VAL() (!!GetPrivateProfileInt(L"gen_ml_config", L"groupbtn", 1, ml_cfg))
extern wchar_t* ml_cfg;
wchar_t *urlencode(wchar_t *p);

extern HWND current_window;
extern int groupBtn, enqueuedef, customAllowed;
extern viewButtons view;

void SwapPlayEnqueueInMenu(HMENU listMenu);
void SyncMenuWithAccelerators(HWND hwndDlg, HMENU menu);
void Downloads_UpdateButtonText(HWND hwndDlg, int _enqueuedef);
void listbuild(wchar_t **buf, int &buf_size, int &buf_pos, const wchar_t *tbuf);

enum
{
	BPM_ECHO_WM_COMMAND=0x1, // send WM_COMMAND and return value
	BPM_WM_COMMAND = 0x2, // just send WM_COMMAND
};

BOOL Downloads_ButtonPopupMenu(HWND hwndDlg, int buttonId, HMENU menu, int flags=0);
void UpdateMenuItems(HWND hwndDlg, HMENU menu);

#endif