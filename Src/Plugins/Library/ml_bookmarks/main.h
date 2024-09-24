#ifndef NULLSOFT_BOOKMARKS_MAIN_H
#define NULLSOFT_BOOKMARKS_MAIN_H

#include <windows.h>
#include <windowsx.h>

#include "api__ml_bookmarks.h"

#include "../Plugins/General/gen_ml/ml.h"
#include "../nu/MediaLibraryInterface.h"

#include "resource.h"

#include "../winamp/wa_ipc.h"
//#include "../Plugins/General/gen_ml/ml.h"
#include "../Plugins/General/gen_ml/config.h"


#define WINAMP_EDIT_BOOKMARKS           40320

INT_PTR bm_pluginMessageProc( int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3 );

extern winampMediaLibraryPlugin plugin;
extern int bookmark_treeItem;

void bookmark_notifyAdd( wchar_t *filenametitle );

#endif  // !NULLSOFT_BOOKMARKS_MAIN_H
