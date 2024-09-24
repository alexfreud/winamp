#ifndef NULLSOFT_FANZONE_MAIN_H
#define NULLSOFT_FANZONE_MAIN_H

#include <windows.h>
#include <strsafe.h>
#include <iostream>

#include "api__ml_fanzone.h"
#include "resource.h"
#include <windowsx.h>
#include "../Plugins/General/gen_ml/ml.h"
#include "../nu/MediaLibraryInterface.h"

#include "../winamp/wa_ipc.h"
#include "../Plugins/General/gen_ml/ml.h"
#include "../Plugins/General/gen_ml/config.h"

static HWND         m_hwnd;

extern winampMediaLibraryPlugin plugin;
INT_PTR fanzone_pluginMessageProc( int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3 );
extern int fanzone_treeItem;

#endif  // !NULLSOFT_FANZONE_MAIN_H