#ifndef __CURSORMGR_H
#define __CURSORMGR_H

#include <bfc/platform/platform.h>
#include <api/wnd/cursor.h>

class ifc_window;

class CursorMgr 
{
  public:
    CursorMgr() {}
    virtual ~CursorMgr() {} 

    static OSCURSOR requestCursor(const wchar_t *id);
};

#endif
