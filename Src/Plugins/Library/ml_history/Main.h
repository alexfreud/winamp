#ifndef NULLSOFT_MAINH
#define NULLSOFT_MAINH

#include <windows.h>
#include "..\..\General\gen_ml/ml.h"
#include "resource.h"
#include "../nu/MediaLibraryInterface.h"
#include "..\..\General\gen_ml/menu.h"
#include <commctrl.h>
#include <shlwapi.h>
#include "ml_history.h"
#include <windowsx.h>
#include "..\..\General\gen_ml/ml.h"
#include "..\..\General\gen_ml/ml_ipc.h"
#include "../nde/nde_c.h"
#include "api__ml_history.h"

#include "../Winamp/JSAPI2_svc_apicreator.h"

#include <api/service/waservicefactory.h>
#include <api/service/services.h>

extern winampMediaLibraryPlugin plugin;
extern bool nde_error;
extern int history_fn_mode;
extern wchar_t *history_fn;
extern int timer;
extern HWND m_curview_hwnd;
extern int groupBtn, enqueuedef;
extern HMENU g_context_menus2;

void History_StartTracking(const wchar_t *filename, bool resume);

#endif