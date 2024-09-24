#ifndef __HIDEOBJECT_H
#define __HIDEOBJECT_H

#include <api/skin/objectactuator.h>

#define  HIDEOBJECT_PARENT ObjectActuator

extern const wchar_t HideObjectXuiObjectStr[];
extern char HideObjectXuiSvcName[];
// -----------------------------------------------------------------------
// Your wnd object class
class HideObject: public HIDEOBJECT_PARENT {
  
  public:

    HideObject();

    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

    virtual void actuator_onPerform(GuiObject *target);
    virtual const wchar_t *getActuatorTag() { return HideObjectXuiObjectStr; } // for error msgs purposes
protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:

    int myxuihandle;

    enum {
      HIDEOBJECT_HIDE= 0,
    };
		static XMLParamPair params[];

};

// -----------------------------------------------------------------------
// This defines the svc_xuiObject that exposes your wnd object

class HideObjectXuiSvc : public XuiObjectSvc<HideObject, HideObjectXuiObjectStr, HideObjectXuiSvcName> {};

#endif
