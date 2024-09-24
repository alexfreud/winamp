//  precomp__gen_ff.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef NULLSOFT_GEN_FF_PRECOMP_H
#define NULLSOFT_GEN_FF_PRECOMP_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#ifndef WINVER
#define WINVER 0x0501
#endif


// Most used wasabi headers

#include <wasabicfg.h>
//#include <api.h>
#include <api__gen_ff.h>
#include <bfc/platform/platform.h>
#include <bfc/assert.h>
#include <bfc/common.h>
#include <bfc/wasabi_std.h>
#include <bfc/ptrlist.h>
#include <bfc/stack.h>
#include <bfc/tlist.h>
#include <bfc/string/bfcstring.h>
#include <bfc/string/StringW.h>
#include <bfc/wasabi_std_wnd.h>
#include <bfc/std_string.h>
#include <bfc/dispatch.h>
#include <bfc/nsGUID.h>
#include <api/service/servicei.h>

#include <api/service/waservicefactory.h>

#include <api/config/cfgscriptobj.h>

#include <api/wnd/rootwnd.h>
#include <api/wnd/basewnd.h>
#include <api/wnd/wndclass/guiobjwnd.h>

#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/script/objects/guiobject.h>

#include <api/script/scriptobj.h>
#include <api/script/objcontroller.h>
#include <api/script/scriptvar.h>

#include "wa2core.h"
#include "wa2frontend.h"
#include "wa2wndembed.h"

#include <tataki/canvas/canvas.h>
#include <tataki/region/region.h>
#include <tataki/bitmap/bitmap.h>

#include <api/skin/skin.h>
#include <api/skin/skinparse.h>
#include <api/skin/widgets.h>

#include <api/timer/timerclient.h>

#include <api/skin/widgets.h>



#endif // _PRECOMP_H
