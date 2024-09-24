#ifndef __CUSTOMOBJECT_H
#define __CUSTOMOBJECT_H

#include <bfc/dispatch.h>
#include <bfc/common.h>

class ifc_window;

// {F5527A4F-C910-48c2-A80B-98A60D317F35}
const GUID customObjectGuid = 
{ 0xf5527a4f, 0xc910, 0x48c2, { 0xa8, 0xb, 0x98, 0xa6, 0xd, 0x31, 0x7f, 0x35 } };


/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class CustomObject : public Dispatchable {
public:
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  void customobject_setRootWnd(ifc_window *w);
  
  enum {
    CUSTOMOBJECT_SETROOTWND=10,
    CUSTOMOBJECT_GETROOTWND=20,
  };
};

inline void CustomObject::customobject_setRootWnd(ifc_window *w) {
  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  _voidcall(CUSTOMOBJECT_SETROOTWND, w);
}

/**
  Class 

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class CustomObjectI : public CustomObject {
public:

  
  /**
    Method
  
    @see 
    @ret 
    @param 
  */
  virtual void customobject_setRootWnd(ifc_window *w)=0;

protected:
  RECVS_DISPATCH;
};


#endif
