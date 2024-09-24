#ifndef __SENDPARAMS_H
#define __SENDPARAMS_H

#include <api/skin/objectactuator.h>
#include <bfc/pair.h>

#define  SENDPARAMS_PARENT ObjectActuator

extern const wchar_t SendParamsXuiObjectStr[];
extern char SendParamsXuiSvcName[];

// -----------------------------------------------------------------------
// Your wnd object class
class SendParams: public SENDPARAMS_PARENT {
  
  public:

    SendParams();
    virtual ~SendParams();

    virtual int setXmlParam(const wchar_t *param, const wchar_t *value);
    virtual void actuator_onPerform(GuiObject *target);
    virtual const wchar_t *getActuatorTag() { return SendParamsXuiObjectStr; } // for error msgs purposes

  private:

    int myxuihandle;
    PtrList< Pair<StringW, StringW> > pastlist;
};

// -----------------------------------------------------------------------
// This defines the svc_xuiObject that exposes your wnd object

class SendParamsXuiSvc : public XuiObjectSvc<SendParams, SendParamsXuiObjectStr, SendParamsXuiSvcName> {};

#endif
