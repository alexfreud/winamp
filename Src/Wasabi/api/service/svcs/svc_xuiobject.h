#ifndef _SVC_XUIOBJECT_H
#define _SVC_XUIOBJECT_H

#include <bfc/dispatch.h>
#include <api/service/services.h>
#include <bfc/platform/platform.h>
#include <bfc/string/StringW.h>
class GuiObject;
class skin_xmlreaderparams;

class NOVTABLE svc_xuiObject : public Dispatchable
{
public:
    static FOURCC getServiceType()
    {
        return WaSvc::XUIOBJECT;
    }
    int testTag( const wchar_t *xmltag );
    GuiObject *instantiate( const wchar_t *xmltag, ifc_xmlreaderparams *params = NULL );
    void destroy( GuiObject *g );

    enum
    {
        XUI_TESTTAG               = 10,
        //XUI_INSTANTIATE=20, // RETIRED
        XUI_INSTANTIATEWITHPARAMS = 25,
        XUI_DESTROY               = 30,
    };
};

inline int svc_xuiObject::testTag(const wchar_t *xmltag) {
  return _call(XUI_TESTTAG, 0, xmltag);
}

inline GuiObject *svc_xuiObject::instantiate(const wchar_t *xmltag, ifc_xmlreaderparams *params) {
  return _call(XUI_INSTANTIATEWITHPARAMS, (GuiObject *)NULL, xmltag, params);
}

inline void svc_xuiObject::destroy(GuiObject *o) {
  _voidcall(XUI_DESTROY, o);
}

// derive from this one
class svc_xuiObjectI : public svc_xuiObject 
{
public:
  virtual int testTag(const wchar_t *xmltag)=0;
  virtual GuiObject *instantiate(const wchar_t *xmltag, ifc_xmlreaderparams *params=NULL)=0;
  virtual void destroy(GuiObject *o)=0;

protected:
  RECVS_DISPATCH;
};

#include <api/service/servicei.h>
template <class T>
class XuiObjectCreator : public waServiceFactoryTSingle<svc_xuiObject, T> {
public:
  virtual const wchar_t *svc_getTestString() { return T::xuisvc_getXmlTag(); }
};

#include <api/service/svc_enum.h>
#include <bfc/string/StringW.h>

class XuiObjectSvcEnum : public SvcEnumT<svc_xuiObject> {
public:
  XuiObjectSvcEnum(const wchar_t *xmltag) : tag(xmltag) {}
protected:
  virtual int testService(svc_xuiObject *svc) {
    return (svc->testTag(tag));
  }
private:
  StringW tag;
};

#endif
