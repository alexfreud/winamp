#ifndef _SVC_MINIBROWSER_H
#define _SVC_MINIBROWSER_H

#include <bfc/dispatch.h>
#include <api/service/services.h>

class MiniBrowser;

#include <bfc/nsguid.h>

// {2E41D2E8-19A5-4029-9339-8FDF7481000A}
static const GUID GUID_MINIBROWSER_ANY = 
{ 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

// {C0A3D1AC-2430-45a7-B51B-AB04B74DD9EA}
static const GUID GUID_MINIBROWSER_IEACTIVEX = 
{ 0xc0a3d1ac, 0x2430, 0x45a7, { 0xb5, 0x1b, 0xab, 0x4, 0xb7, 0x4d, 0xd9, 0xea } };

class NOVTABLE svc_miniBrowser : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::MINIBROWSER; }

  int testGuid(GUID g);
  MiniBrowser *createMiniBrowser();
  void destroyMiniBrowser(MiniBrowser *b);

  enum {
    TESTGUID            =10,
    CREATEMINIBROWSER   =20,
    DESTROYMINIBROWSER  =30,
  };
};

inline int svc_miniBrowser::testGuid(GUID g) {
  return _call(TESTGUID, 0, g);
}

inline MiniBrowser *svc_miniBrowser::createMiniBrowser() {
  return _call(CREATEMINIBROWSER, (MiniBrowser *)0);
}

inline void svc_miniBrowser::destroyMiniBrowser(MiniBrowser *b) {
  _voidcall(DESTROYMINIBROWSER, b);
}

class NOVTABLE svc_miniBrowserI : public svc_miniBrowser {
public:
  virtual int testGuid(GUID g)=0;
  virtual MiniBrowser *createMiniBrowser()=0;
  virtual void destroyMiniBrowser(MiniBrowser *b)=0;

protected:
  RECVS_DISPATCH;
};

#include <api/service/svc_enum.h>

class MiniBrowserSvcEnum : public SvcEnumT<svc_miniBrowser> {
public:
  MiniBrowserSvcEnum(GUID g) : guid(g) {}
protected:
  virtual int testService(svc_miniBrowser *svc) {
    return (svc->testGuid(guid));
  }
private:
  GUID guid;
};

#endif
