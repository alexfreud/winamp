#ifndef __OBJECTACTUATOR_H
#define __OBJECTACTUATOR_H

#include <api/skin/nakedobject.h>

#define  OBJECTACTUATOR_PARENT NakedObject

// -----------------------------------------------------------------------
// Your wnd object class
class ObjectActuator : public OBJECTACTUATOR_PARENT {
  
  public:

    ObjectActuator();
    virtual ~ObjectActuator();

    virtual int onInit();
    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

    virtual void actuator_setTarget(const wchar_t *value);
    virtual void actuator_setGroup(const wchar_t *value);

    virtual int actuator_wantTargetParam() { return 1; }
    virtual int actuator_wantGroupParam() { return 1; }
    virtual int actuator_wantAutoPerform() { return 1; }
    virtual void actuator_onPerform(GuiObject *target) { } // called back n times for n targets found (separated by ';'), guaranteed non NULL

    virtual const wchar_t *getActuatorTag(); // for error msgs purposes


protected:
    void performActions();
/*static */void CreateXMLParameters(int master_handle);
  private:
		static XMLParamPair params[];
    int myxuihandle;

  
		  enum {
      OBJECTACTUATOR_TARGET= 0,
      OBJECTACTUATOR_GROUP,
    };

  StringW groupid;
  StringW objectsid;
};


#endif
