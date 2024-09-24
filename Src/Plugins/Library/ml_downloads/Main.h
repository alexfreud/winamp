#ifndef NULLSOFT_MAINH
#define NULLSOFT_MAINH

#include "Downloaded.h"

#define PLUGIN_VERSION_MAJOR		1
#define PLUGIN_VERSION_MINOR		33

extern int winampVersion, podcast_parent, dirty;

#define NAVITEM_UNIQUESTR	L"download_svc"
BOOL Navigation_Update(void);

bool AddDownloadData(const DownloadedFile &data);
void CloseDatabase();

#include "resource.h"
#include "../nu/DialogSkinner.h"
#include "../nu/MediaLibraryInterface.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoLock.h"
#include <windows.h>
#include <shlwapi.h>

extern ATOM VIEWPROP;
extern winampMediaLibraryPlugin plugin;
extern int downloads_treeItem;

#include "../Components/wac_downloadManager/wac_downloadManager_api.h"

#define ML_ENQDEF_VAL() (!!GetPrivateProfileInt(L"gen_ml_config", L"enqueuedef", 0, ml_cfg))
#define ML_GROUPBTN_VAL() (!!GetPrivateProfileInt(L"gen_ml_config", L"groupbtn", 1, ml_cfg))
extern wchar_t* ml_cfg;

#include "DownloadViewCallback.h"

extern DownloadViewCallback *downloadViewCallback;

#endif

extern HWND downloads_window;
extern int groupBtn, enqueuedef;