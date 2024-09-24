#ifndef _SVCWNDHOLD_H
#define _SVCWNDHOLD_H

#include <api/wnd/wndclass/rootwndholder.h>
#include <bfc/common.h>

class svc_windowCreate;

// for some reason if this derives from virtualwnd typesheet won't show it
#define SERVICEWNDHOLDER_PARENT RootWndHolder

/**
  class ServiceWndHolder .

  @short 
  @author Nullsoft
  @ver 1.0
  @see 
*/
class ServiceWndHolder : public SERVICEWNDHOLDER_PARENT {
public:
  /**
    ServiceWndHolder constructor .

    @param _child A pointer to the child we want to set.
    @param _svc A pointer to the window creation service associated with the window we want to set as a child.
  */
  ServiceWndHolder(ifc_window *child=NULL, svc_windowCreate *svc=NULL);
  
  /**
    ServiceWndHolder destructor
  */
  virtual ~ServiceWndHolder();

  /**
    ServiceWndHolder method setChild .

    @ret 1
    @param _child A pointer to the child we want to set.
    @param _svc A pointer to the window creation service associated with the window we want to set as a child.
  */
  int setChild(ifc_window *child, svc_windowCreate *svc);

  virtual ifc_window *rootwndholder_getRootWnd();

private:
  ifc_window *child;
  svc_windowCreate *svc;
};

#endif
