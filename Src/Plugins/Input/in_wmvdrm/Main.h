#ifndef NULLSOFT_MAINH
#define NULLSOFT_MAINH

#define WMDRM_VERSION L"3.95"

#include "WinampInterface.h"
#include "../Winamp/in2.h"
#include <windows.h>
#include "WMDRMModule.h"
#include <shlwapi.h>
#include <wmsdk.h>
#include "config.h"
#include "WMHandler.h"
#include "util.h"
#include "FileTypes.h"
#include "TagAlias.h"
#include "WMInformation.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "vidutils.h"
#include "api.h"

extern WMInformation *setFileInfo;

struct IDispatch;
extern IDispatch *winampExternal;

extern WinampInterface winamp;
extern In_Module plugin;

extern WMDRM mod;

#ifdef _DEBUG
#define SHOW_CALLBACKS
#endif

//#define SHOW_CALLBACKS

#ifdef SHOW_CALLBACKS
#include <iostream>
#define WMTCASE(sw) case sw: 	std::cerr << #sw << std::endl;
#define WMT_SHOW_HR_CODE(hr) std::cerr << HRErrorCode(hr) << std::endl;
#else
#define WMTCASE(sw) case sw: 	
#define WMT_SHOW_HR_CODE(hr) 
#endif

// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID = 
{ 0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf } };

#endif