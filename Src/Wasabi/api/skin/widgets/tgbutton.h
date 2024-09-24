#ifndef _TOGGLEBUTTON_H
#define _TOGGLEBUTTON_H

#include <api/script/script.h>
#include <api/script/scriptobj.h>
#include <api/skin/widgets/button.h>

#define TOGGLEBUTTON_PARENT Wasabi::Button

class TgButtonScriptController : public ButtonScriptController {
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController() { return buttonController; }
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

extern TgButtonScriptController *tgbuttonController;


class ToggleButton : public TOGGLEBUTTON_PARENT {

public:

  ToggleButton();
  virtual ~ToggleButton();

  virtual void onLeftPush(int x, int y);
  virtual void onToggle(int i);

  virtual int setXuiParam(int _xuihandle, int xmlattributeid, const wchar_t *name, const wchar_t *value);
#ifdef WASABI_COMPILE_CONFIG
  virtual int onReloadConfig();
#endif
  virtual void autoToggle();

  virtual const wchar_t *vcpu_getClassName();
  virtual ScriptObjectController *vcpu_getController() { return tgbuttonController; }
  virtual int getCurCfgVal();

protected:
/*static */void CreateXMLParameters(int master_handle);
  enum {
    TOGGLEBUTTON_AUTOTOGGLE=0,
#ifdef WASABI_COMPILE_CONFIG
    TOGGLEBUTTON_CFGVAL,
#endif
  };
	

private:
	static XMLParamPair params[];
	wchar_t *param;
  int autotoggle;
#ifdef WASABI_COMPILE_CONFIG
  int cfgVal;
#endif
  int xuihandle;

public:
  static scriptVar script_onToggle(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar is);
  static scriptVar script_getCurCfgVal(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
};


extern const wchar_t toggleButtonXuiObjectStr[];
extern char toggleButtonXuiSvcName[];
class ToggleButtonXuiSvc : public XuiObjectSvc<ToggleButton, toggleButtonXuiObjectStr, toggleButtonXuiSvcName> {};


// {80F97426-9A10-472c-82E7-8309AA4789E7}
static const GUID NStatesTgButtonGuid = 
{ 0x80f97426, 0x9a10, 0x472c, { 0x82, 0xe7, 0x83, 0x9, 0xaa, 0x47, 0x89, 0xe7 } };

#define NSTATESTGBUTTON_PARENT ToggleButton

class NStatesTgButton : public NSTATESTGBUTTON_PARENT {
  public:
    NStatesTgButton();
    virtual ~NStatesTgButton(); 

    virtual int setXuiParam(int _xuihandle, int xmlattributeid, const wchar_t *paramname, const wchar_t *strvalue);
    void setNStates(int n) { nstates = n; }
    virtual int onInit();

    void setState(int n);
    virtual int getActivatedButton();
    virtual void autoToggle();
    int getState() { return state; }
    virtual void setActivatedButton(int a);
    virtual int getCurCfgVal();
    void setOneVisualState(int v);

  protected:
/*static */void CreateXMLParameters(int master_handle);
    virtual void setupBitmaps();

  enum {
    NSTATESTGBUTTON_NSTATES=0,
#ifdef WASABI_COMPILE_CONFIG
    NSTATESTGBUTTON_CFGVALS,
#endif
    NSTATESTGBUTTON_ONEVSTATE,
  };

  StringW image, hover, down, active;
  int nstates;
  int state;
  
  int onevstate;
#ifdef WASABI_COMPILE_CONFIG
  StringW cfgvals;
#endif
private:
	static XMLParamPair params[];
	int xuihandle;
};

extern const wchar_t nStatesTgButtonXuiObjectStr[];
extern char nStatesTgButtonXuiSvcName[];
class nStatesTgButtonXuiSvc : public XuiObjectSvc<NStatesTgButton, nStatesTgButtonXuiObjectStr, nStatesTgButtonXuiSvcName> {};


#endif