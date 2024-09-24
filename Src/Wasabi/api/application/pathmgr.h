#ifndef _PATHMGR_H
#define _PATHMGR_H

#include <bfc/std.h>

class PathMgr 
{
public:
  //static const char *getUserSettingsPath();
#ifdef WASABI_COMPILE_COMPONENTS
  static const wchar_t *getComponentDataPath(GUID g);
#endif
};

#endif
