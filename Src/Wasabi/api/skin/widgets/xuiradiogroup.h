#ifndef __SCRIPTRADIOGROUP_H
#define __SCRIPTRADIOGROUP_H

#include <api/skin/widgets/guiradiogroup.h>
#include <api/script/objects/c_script/h_guiobject.h>

#define  SCRIPTRADIOGROUP_PARENT GuiRadioGroup

// -----------------------------------------------------------------------
// Your wnd object class
class ScriptRadioGroup : public SCRIPTRADIOGROUP_PARENT {
  
  public:

    ScriptRadioGroup();
    virtual ~ScriptRadioGroup();

  private:

    int myxuihandle;
};

// -----------------------------------------------------------------------
// This defines the svc_xuiObject that exposes your wnd object

extern const wchar_t ScriptRadioGroupXuiObjectStr[];
extern char ScriptRadioGroupXuiSvcName[];
class ScriptRadioGroupXuiSvc : public XuiObjectSvc<ScriptRadioGroup, ScriptRadioGroupXuiObjectStr, ScriptRadioGroupXuiSvcName> {};

#endif
