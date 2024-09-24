#ifndef __CURSOR_H
#define __CURSOR_H
#ifdef _WIN32 // PORT ME
#include <bfc/dispatch.h>
#include <api/syscb/callbacks/skincb.h>
#include <bfc/string/bfcstring.h>
#include <bfc/string/StringW.h>

class Cursor : public Dispatchable 
{
  public:
    OSCURSORHANDLE getOSHandle();

  enum {
    CURSOR_GETOSHANDLE = 0,
  };

};

inline OSCURSORHANDLE Cursor::getOSHandle() {
  return _call(CURSOR_GETOSHANDLE, (OSCURSORHANDLE)NULL);
}

class CursorI : public Cursor {

  public:

    CursorI() {}
    virtual ~CursorI() {}

    virtual OSCURSORHANDLE getOSHandle()=0;
  
  protected:

    RECVS_DISPATCH;
    
};

#ifdef WASABI_COMPILE_SKIN

class SkinCursor : public CursorI, public SkinCallbackI {

  public:

    SkinCursor();
    SkinCursor(const wchar_t *elementid);
    virtual ~SkinCursor();
  
    virtual void setCursorElementId(const wchar_t *id);
    virtual int skincb_onReset();
    virtual OSCURSORHANDLE getOSHandle();
    virtual void reset();

  private:
    
    StringW name;
    OSCURSORHANDLE cursor;
};

#endif
#endif
#endif