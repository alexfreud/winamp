#ifndef _COMPONENTOBJ_H
#define _COMPONENTOBJ_H

#include <api/script/script.h>
#include <api/script/scriptobj.h>
#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/script/objects/guiobj.h>

class SMap;
class SRegion;
class Container;
class Layout;
class CompWnd;

// {403ABCC0-6F22-4bd6-8BA4-10C829932547}
static const GUID componentObjectGuid = 
{ 0x403abcc0, 0x6f22, 0x4bd6, { 0x8b, 0xa4, 0x10, 0xc8, 0x29, 0x93, 0x25, 0x47 } };

#define COMPONENTOBJECT_PARENT GuiObjectWnd

class CompoObjScriptController : public GuiObjectScriptController {
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController() { return guiController; }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual ScriptObject *instantiate();
    virtual void destroy(ScriptObject *o);
    virtual void *encapsulate(ScriptObject *o);
    virtual void deencapsulate(void *);

  private:

    static function_descriptor_struct exportedFunction[];
    
};

extern CompoObjScriptController *compoController;


#ifndef _NOSTUDIO

#include <api/wnd/virtualwnd.h>

class ComponentObject : public COMPONENTOBJECT_PARENT {
public:
  ComponentObject();
  virtual ~ComponentObject();

  virtual int setXmlParam(const wchar_t *name, const wchar_t *value);

	virtual int onResize();
	virtual void onSetVisible(int s);
  virtual int handleRatio();

  void deniedComponentCompWnd(CompWnd *c, GUID g);
  void grantedComponentCompWnd(CompWnd *c, GUID g);
  int wantGUID(GUID *g);

  void onReleaseComponent();
  void onGetComponent(GUID g);

  void onBeforeGetWac(GUID g, CompWnd *c);
  void onBeforeGiveUpWac(GUID g, CompWnd *c);

	void setGUID(GUID g);
	void setCompGUID(GUID g);
	GUID getGUID(void);
  int getAutoClose();
  void setAcceptWac(int a);
  int getAnimatedRects() { return !noanimrects; }
  virtual int getPreferences(int what);

	virtual void script_resetRegion();
	virtual void script_setRegionFromMap(SMap *map, int byte, int inv);
	virtual void script_setRegion(SRegion *r);
  virtual int handleDesktopAlpha();
  virtual int handleTransparency();

  // VCPU
  static scriptVar script_vcpu_getGUID(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getWac(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onShow(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onHide(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_onGetWac(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar wac);
  static scriptVar script_vcpu_onGiveUpWac(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar wac);
  static scriptVar script_vcpu_setRegionFromMap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar map, scriptVar byte, scriptVar inv);
  static scriptVar script_vcpu_setRegion(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar r);
  static scriptVar script_vcpu_setAcceptWac(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar on);
  // End VCPU

private:
  GUID myGUID;
  GUID myCompGuid;

  int deleting;

  CompWnd *compwnd;
  int noshowcmdbar;
  int noshowbtnbar;

  api_region *my_region_clone;
  int autoopen, autoclose;
  int accept;
  int denyDesktopAlpha;
  int denyTransparency;
  int noanimrects;

#else 
class ComponentObject : public COMPONENTOBJECT_SCRIPTPARENT {
#endif

};


#endif
