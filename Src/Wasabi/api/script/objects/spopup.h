//PORTABLE
#ifndef _SPOPUP_H
#define _SPOPUP_H

#include <api/wnd/popup.h>
#include <api/script/objects/rootobject.h>

// {F4787AF4-B2BB-4ef7-9CFB-E74BA9BEA88D}
static const GUID popupGuid = 
{ 0xf4787af4, 0xb2bb, 0x4ef7, { 0x9c, 0xfb, 0xe7, 0x4b, 0xa9, 0xbe, 0xa8, 0x8d } };

#define SPOPUP_PARENT PopupMenu

class PopupScriptController: public ScriptObjectControllerI {
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController() { return rootScriptObjectController; }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *o);

  private:

    static function_descriptor_struct exportedFunction[];
    
};

extern PopupScriptController *popupController;


#ifndef WANT_NEW_POPUPMENU
class SPopup : public SPOPUP_PARENT, public RootObjectInstance {
#else
class SPopup : public SPOPUP_PARENT {
#endif
public:
  SPopup();
	virtual ~SPopup();

private:

public:

  static scriptVar script_vcpu_addSubMenu(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar popup, scriptVar str);
  static scriptVar script_vcpu_addCommand(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar str, scriptVar cmd, scriptVar checked, scriptVar disabled);
  static scriptVar script_vcpu_addSeparator(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_popAtXY(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar x, scriptVar y);
  static scriptVar script_vcpu_popAtMouse(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getNumCommands(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_disableCommand(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar cmd, scriptVar disable);
  static scriptVar script_vcpu_checkCommand(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar cmd, scriptVar check);
};

#endif
