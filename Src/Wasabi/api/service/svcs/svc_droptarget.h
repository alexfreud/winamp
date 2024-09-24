#ifndef _SVC_DROPTARGET_H
#define _SVC_DROPTARGET_H

#include <bfc/dispatch.h>
#include <api/service/services.h>

class DragInterface;	// see bfc/drag.h

class NOVTABLE svc_dropTarget : public Dispatchable 
{
public:
  static FOURCC getServiceType() { return WaSvc::DROPTARGET; }

  int testTarget(FOURCC type);

  DragInterface *getDragInterfaceForType(FOURCC type);
  int releaseDragInterface(DragInterface *di);

protected:
  enum {
    TESTTARGET=100,
    GETDRAGINTERFACEFORTYPE=200,
    RELEASEDRAGINTERFACE=210,
  };
};

inline
int svc_dropTarget::testTarget(FOURCC type) {
  return _call(TESTTARGET, 0, type);
}

inline
DragInterface *svc_dropTarget::getDragInterfaceForType(FOURCC type) {
  return _call(GETDRAGINTERFACEFORTYPE, (DragInterface*)NULL, type);
}

inline
int svc_dropTarget::releaseDragInterface(DragInterface *di) {
  return _call(RELEASEDRAGINTERFACE, 0, di);
}

class svc_dropTargetI : public svc_dropTarget {
public:
  virtual int testTarget(FOURCC type)=0;

  virtual DragInterface *getDragInterfaceForType(FOURCC type)=0;
  virtual int releaseDragInterface(DragInterface *di)=0;

protected:
  RECVS_DISPATCH;
};

#include <api/service/servicei.h>

template <class T>
class DropTargetCreator : public waServiceFactoryTSingle<svc_dropTarget, T> { };

#include <api/service/svc_enum.h>
#include <api/wnd/drag.h>

class DropTargetEnum : public SvcEnumT<svc_dropTarget> {
public:
  DropTargetEnum(FOURCC type) : dt_type(type) {}
  static int throwDrop(FOURCC type, ifc_window *sourceWnd, int x=0, int y=0) {
    DropTargetEnum dte(type);
    svc_dropTarget *sdt = dte.getFirst();
    if (sdt == NULL) return 0;
    DragInterface *di = sdt->getDragInterfaceForType(type);
    int r = 0;
    if (di != NULL) r = di->dragDrop(sourceWnd, 0, 0);
    sdt->releaseDragInterface(di);
    return r;
  }
protected:
  virtual int testService(svc_dropTarget *svc) {
    return (svc->testTarget(dt_type));
  }
private:
  FOURCC dt_type;
};

#endif
