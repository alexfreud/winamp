#ifndef NULLSOFT_CRASHER_MAIN_H
#define NULLSOFT_CRASHER_MAIN_H

#include <windows.h>
#include <dbghelp.h>
#include "resource.h"
#define NO_IVIDEO_DECLARE
#include "..\winamp\wa_ipc.h"
#include "settings.h"
#include "../winamp/gen.h"

extern Settings settings;
extern prefsDlgRecW prefItem;
extern char *winampVersion;
extern "C" winampGeneralPurposePlugin plugin;

extern "C" __declspec(dllexport) int StartHandler(wchar_t* iniPath);
extern "C" LONG WINAPI FeedBackFilter( struct _EXCEPTION_POINTERS *pExceptionInfo );

//typedef struct _EXCEPTION_POINTERS EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

static BOOL alreadyProccessing;

#endif // NULLSOFT_CRASHER_MAIN_H