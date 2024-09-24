#ifndef __ADDPARAMS_H
#define __ADDPARAMS_H

#include <api/skin/objectactuator.h>
#include <bfc/pair.h>

#define  ADDPARAMS_PARENT ObjectActuator

extern const wchar_t AddParamsXuiObjectStr[];
extern char AddParamsXuiSvcName[];

// -----------------------------------------------------------------------
// Your wnd object class
class AddParams: public ADDPARAMS_PARENT {
public:

  AddParams();
  virtual ~AddParams();

  virtual int setXmlParam(const wchar_t *param, const wchar_t *value);
  virtual void actuator_onPerform(GuiObject *target);
  virtual const wchar_t *getActuatorTag() { return AddParamsXuiObjectStr; } // for error msgs purposes

private:
  int myxuihandle;
  PtrList< Pair<StringW, StringW> > pastlist;
};

// -----------------------------------------------------------------------
// This defines the svc_xuiObject that exposes your wnd object

class AddParamsXuiSvc : public XuiObjectSvc<AddParams, AddParamsXuiObjectStr, AddParamsXuiSvcName> {};

#endif
