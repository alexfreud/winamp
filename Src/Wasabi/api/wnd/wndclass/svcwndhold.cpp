#include <precomp.h>

#include "svcwndhold.h"

#include <bfc/common.h>

#include <api/service/svcs/svc_wndcreate.h>

#include <api/wnd/wndclass/blankwnd.h>

ServiceWndHolder::ServiceWndHolder(ifc_window *_child, svc_windowCreate *_svc) :
 child(NULL), svc(NULL) 
 {
  setChild(_child, _svc);
}

ServiceWndHolder::~ServiceWndHolder() 
{
  if (svc != NULL) 
	{
    svc->destroyWindow(child);
    if (!svc->refcount()) 
      WASABI_API_SVC->service_release(svc);
  } else {
    delete static_cast<BaseWnd*>(child);
  }
}

int ServiceWndHolder::setChild(ifc_window *_child, svc_windowCreate *_svc) 
{
  if (child == _child && svc == _svc) return 0;

  if (child != NULL) {
    if (svc != NULL) {
      svc->destroyWindow(child);
      if (!svc->refcount()) 
        WASABI_API_SVC->service_release(svc);
      svc = NULL;
    } else {
      delete static_cast<BaseWnd*>(child);
    }
  child = NULL;
  } 
  
  child = _child;
  svc = _svc;

  return 1;
}

ifc_window *ServiceWndHolder::rootwndholder_getRootWnd() {
  return child ? child : SERVICEWNDHOLDER_PARENT::rootwndholder_getRootWnd();
}
