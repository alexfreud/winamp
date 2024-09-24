#ifndef _PSLIDER_H
#define _PSLIDER_H

#include <api/wnd/wndclass/slider.h>
#include <api/script/objects/guiobj.h>
#include <api/skin/widgets.h>

#define PSLIDER_PARENT SliderWnd

class SliderScriptController : public GuiObjectScriptController {
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
    virtual void deencapsulate(void *o);

  private:

    static function_descriptor_struct exportedFunction[];
    
};

extern SliderScriptController *sliderController;



class PSliderWnd : public PSLIDER_PARENT {

public:

  PSliderWnd();
  virtual ~PSliderWnd();

  virtual int onSetPosition();
  virtual int onSetFinalPosition();
  virtual int onPostedPosition(int p);
  virtual void lock();
  virtual void unlock();

  virtual int setXuiParam(int _xuihandle, int attribid, const wchar_t *paramname, const wchar_t *strvalue);
  virtual const wchar_t *vcpu_getClassName();
  virtual ScriptObjectController *vcpu_getController() { return sliderController; }

/*  virtual int getAutoHeight();
  virtual int getAutoWidth();*/

  virtual int onInit();

#ifdef WASABI_COMPILE_CONFIG
  virtual int onReloadConfig();
  void reloadConfig();
#endif

  virtual int scriptDivisor() { return 1; }

  enum {
    PSLIDER_SETBARLEFT=0,
    PSLIDER_SETBARMIDDLE,
    PSLIDER_SETBARRIGHT,
    PSLIDER_SETTHUMB,
    PSLIDER_SETDOWNTHUMB,
    PSLIDER_SETHOVERTHUMB,
    PSLIDER_SETORIENTATION,
    PSLIDER_SETLOW,
    PSLIDER_SETHIGH,
    PSLIDER_SETHOTPOS,
    PSLIDER_SETHOTRANGE,
		PSLIDER_SETSTRETCHTHUMB,
		PSLIDER_NUMPARAMS,
  };
protected:
	/*static */void CreateXMLParameters(int master_handle);

public:
  static scriptVar script_setPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v);
  static scriptVar script_getPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_onSetPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar p);
  static scriptVar script_onPostedPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar p);
  static scriptVar script_onSetFinalPosition(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar p);
  static scriptVar script_lock(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_unlock(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);


private:
  int xuihandle;
	static XMLParamPair params[];
};

class SliderXuiSvc : public svc_xuiObjectI {

public:
  SliderXuiSvc() {};
  virtual ~SliderXuiSvc() {};

  static const char *getServiceName() { return "Slider xui object"; }
  static const wchar_t *xuisvc_getXmlTag() { return L"Slider"; }
  virtual int testTag(const wchar_t *xmltag) { return !WCSICMP(xmltag, L"Slider"); }
  virtual GuiObject *instantiate(const wchar_t *xmltag, ifc_xmlreaderparams *params=NULL);
  virtual void destroy(GuiObject *g);
};       


#endif
