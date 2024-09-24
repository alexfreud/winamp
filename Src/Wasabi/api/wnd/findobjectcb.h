#ifndef __FINDOBJECTCALLBACK_H
#define __FINDOBJECTCALLBACK_H

#include <bfc/dispatch.h>
#include <bfc/common.h>

class ifc_window;

class FindObjectCallback : public Dispatchable {	
  
  public:
  
    int findobjectcb_matchObject(ifc_window *object);
  
  enum {
    FINDOBJECTCB_MATCHOBJECT = 0,
  };

};

inline int FindObjectCallback::findobjectcb_matchObject(ifc_window *object) {
  return _call(FINDOBJECTCB_MATCHOBJECT, 0, object);
}


class _FindObjectCallback : public FindObjectCallback {
  public:
    virtual int findobjectcb_matchObject(ifc_window *object)=0;

  protected:
    RECVS_DISPATCH;
};


class FindObjectCallbackI : public _FindObjectCallback {
  public:
    
    FindObjectCallbackI() {}
    virtual ~FindObjectCallbackI() {}

    virtual int findobjectcb_matchObject(ifc_window *object)=0;
};

#endif