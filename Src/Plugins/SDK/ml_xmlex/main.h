#ifndef _XMLEX_MAIN_H_
#define _XMLEX_MAIN_H_
#include <windows.h>
#include <windowsx.h>
#include "../nu/MediaLibraryInterface.h"
#include "..\..\General\gen_ml\ml.h"
#include "..\..\General\gen_ml\config.h"

extern winampMediaLibraryPlugin plugin;
INT_PTR xmlex_pluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);
extern UINT_PTR xmlex_treeItem;

#endif