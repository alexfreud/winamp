#include <precomp.h>
#include "pathmgr.h"
#include <api/api.h>
#include <bfc/util/inifile.h>

#if !defined(WIN32) && !defined(LINUX)
#error port me
#endif

#ifdef WIN32
#include <shlobj.h>
#endif


