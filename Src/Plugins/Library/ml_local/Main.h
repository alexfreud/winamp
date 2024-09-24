#ifndef NULLSOFT_MAINH
#define NULLSOFT_MAINH

#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include "..\..\General\gen_ml/ml.h"
#include "../nu/MediaLibraryInterface.h"
#include "../nu/DialogSkinner.h"
#include "../winamp/wa_ipc.h"
#include "ml_local.h"
#include <shlobj.h>
#include "../nde/nde.h"
#include "../nde/nde_c.h"
#include <time.h>
#include "../Agave/Language/api_language.h"
#include "..\..\General\gen_ml/config.h"
#include "api__ml_local.h"
#include "../replicant/nu/AutoWide.h"
#include "../replicant/nu/AutoChar.h"
#include "AlbumArtCache.h"
#include "./local_menu.h"
#include "../playlist/ifc_playlistloadercallback.h"

#define WM_QUERYFILEINFO	(WM_USER + 65) 
#define WM_SHOWFILEINFO		(WM_USER + 64) // wParam - bForceUpdate, lParam - pszFileName

extern winampMediaLibraryPlugin plugin;
extern int winampVersion;
extern int substantives;
extern int play_enq_rnd_alt;
extern bool nde_error;
extern HMENU wa_playlists_cmdmenu;
extern prefsDlgRecW preferences;
extern HWND hwndSearchGlobal;

void EatKeyboard();
void HookPlaylistEditor();
void UnhookPlaylistEditor();
extern bool skipTitleInfo;
extern wchar_t *recent_fn;

extern int IPC_GET_CLOUD_HINST, IPC_GET_CLOUD_ACTIVE, IPC_CLOUD_ENABLED;
extern LRESULT ML_IPC_MENUFUCKER_BUILD;
extern LRESULT ML_IPC_MENUFUCKER_RESULT;

extern int groupBtn, enqueuedef;
extern LARGE_INTEGER freq;

class PLCallBackW : public ifc_playlistloadercallback
{
public:
	PLCallBackW(void){};
	~PLCallBackW(void){};
public:
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info)
	{
		mediaLibrary.AddToMediaLibrary(filename);
	}
	RECVS_DISPATCH;
};

extern "C" void qsort_itemRecord(void *base, size_t num, const void *context,
int (__fastcall *comp)(const void *, const void *, const void *));

void MigrateArtCache();
void setCloudValue(itemRecordW *item, const wchar_t* value);

void onStartPlayFileTrack(const wchar_t *filename, bool resume);
	
#endif