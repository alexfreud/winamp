#ifndef _SVC_XMLPROVIDER_H
#define _SVC_XMLPROVIDER_H

#include <bfc/dispatch.h>
#include <api/service/services.h>

class skin_xmlreaderparams;

class NOVTABLE svc_xmlProvider : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::XMLPROVIDER; }

  int testDesc(const wchar_t *desc);
  const wchar_t *getXmlData(const wchar_t *desc, const wchar_t *incpath, skin_xmlreaderparams *params=NULL);

  enum {
    TESTDESC=10,
    GETXMLDATA=20,
  };
};

inline int svc_xmlProvider::testDesc(const wchar_t *desc) {
  return _call(TESTDESC, 0, desc);
}

inline const wchar_t *svc_xmlProvider::getXmlData(const wchar_t *desc, const wchar_t *incpath, skin_xmlreaderparams *params) {
  return _call(GETXMLDATA, (const wchar_t *)0, desc, incpath, params);
}

// derive from this one
class NOVTABLE svc_xmlProviderI : public svc_xmlProvider {
public:
  virtual int testDesc(const wchar_t *desc)=0;
  virtual const wchar_t *getXmlData(const wchar_t *desc, const wchar_t *incpath, skin_xmlreaderparams *params=NULL)=0;

protected:
  RECVS_DISPATCH;
};

#include <api/service/svc_enum.h>
#include <bfc/string/StringW.h>

class XmlProviderEnum : public SvcEnumT<svc_xmlProvider> {
public:
  XmlProviderEnum(const wchar_t *_desc) : desc(_desc) { }

protected:
  virtual int testService(svc_xmlProvider *svc) {
    return svc->testDesc(desc);
  }

private:
  StringW desc;
};

#endif
