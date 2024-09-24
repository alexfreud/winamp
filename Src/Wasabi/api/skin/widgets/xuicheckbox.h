#ifndef __SCRIPTCHECKBOX_H
#define __SCRIPTCHECKBOX_H

#include <api/skin/widgets/checkbox.h>
#include <api/script/objects/c_script/h_guiobject.h>
#include <api/script/objcontroller.h>
#include <api/wnd/accessible.h>

#define  SCRIPTCHECKBOX_PARENT CheckBox

// -----------------------------------------------------------------------
// Your wnd object class
class ScriptCheckBox : public SCRIPTCHECKBOX_PARENT {
  
  public:

    ScriptCheckBox();
    virtual ~ScriptCheckBox();

    // XuiObject automatically calls this back for all parameters registered using addParam
    // encountered in the xml source
    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

    virtual void onToggle();

    virtual int accessibility_getState() { return CHECKBOX_PARENT::accessibility_getState() | (isActivated() ? STATE_SYSTEM_CHECKED : 0); }
protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:

    // a list of IDs for our xml attributes, we use them in addParam() in the constructor
    enum {
       SCRIPTCHECKBOX_TEXT = 0,
       SCRIPTCHECKBOX_RADIOID,
       SCRIPTCHECKBOX_RADIOVAL,
       SCRIPTCHECKBOX_ACTION,
       SCRIPTCHECKBOX_ACTIONPARAM,
       SCRIPTCHECKBOX_ACTIONTARGET,
    };
    int myxuihandle;
		static XMLParamPair params[];
};

// -----------------------------------------------------------------------------------------------------
class CheckBoxController : public ScriptObjectControllerI {
  public:

    virtual const wchar_t *getClassName() { return L"Checkbox"; }
    virtual const wchar_t *getAncestorClassName() { return L"GuiObject"; }
    virtual ScriptObjectController *getAncestorController() { return WASABI_API_MAKI->maki_getController(guiObjectGuid); }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid() { return checkBoxGuid; }
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  public:
    static scriptVar onToggle(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar newstate);
    static scriptVar setChecked(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar state);
    static scriptVar isChecked(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
    static scriptVar setText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar text);
    static scriptVar getText(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

  private:

    static function_descriptor_struct exportedFunction[];
};

extern CheckBoxController *checkBoxController;


// -----------------------------------------------------------------------
// This defines the svc_xuiObject that exposes your wnd object

extern const wchar_t ScriptCheckBoxXuiObjectStr[];
extern char ScriptCheckBoxXuiSvcName[];
class ScriptCheckBoxXuiSvc : public XuiObjectSvc<ScriptCheckBox, ScriptCheckBoxXuiObjectStr, ScriptCheckBoxXuiSvcName> {};

#endif
