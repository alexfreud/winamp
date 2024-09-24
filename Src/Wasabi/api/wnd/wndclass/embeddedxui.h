#ifndef __EMBEDDEDXUIOBJECT_H
#define __EMBEDDEDXUIOBJECT_H

#include <wasabicfg.h>
#include <api/script/api_maki.h>
#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/script/scriptguid.h>
#include <api/script/scriptobj.h>
#include <api/script/objcontroller.h>

#define EMBEDDEDXUIOBJECT_PARENT GuiObjectWnd


class EmbeddedXuiObjectParam {
  public:
    

    EmbeddedXuiObjectParam(const wchar_t *p, const wchar_t *v) : param(p), value(v) {}

    virtual ~EmbeddedXuiObjectParam() {}

    StringW param;
    StringW value;
};


class EmbeddedXuiObject : public EMBEDDEDXUIOBJECT_PARENT {
  public:

    EmbeddedXuiObject();
    
  
    virtual ~EmbeddedXuiObject();

    virtual void onNewContent();

    virtual int onInit();

 
    virtual void embeddedxui_onNewEmbeddedContent();
    virtual const wchar_t *embeddedxui_getContentId() { return NULL; }
    virtual const wchar_t *embeddedxui_getEmbeddedObjectId() { return NULL; }

    virtual int onUnknownXuiParam(const wchar_t *xmlattributename, const wchar_t *value);
    

#ifdef WASABI_COMPILE_CONFIG
    virtual int onReloadConfig();
#endif
    virtual GuiObject *embeddedxui_getEmbeddedObject() { return embedded; }

  private:

#ifdef WASABI_COMPILE_CONFIG
    void syncCfgAttrib();
#endif

    int myxuihandle;
    PtrList<EmbeddedXuiObjectParam> paramlist;
    GuiObject *embedded;
};

// -----------------------------------------------------------------------
class EmbeddedXuiScriptController: public ScriptObjectControllerI 
{
public:
  virtual const wchar_t *getClassName() { return L"ObjectEmbedder"; }
  virtual const wchar_t *getAncestorClassName() { return L"GuiObject"; }
  virtual ScriptObjectController *getAncestorController();
  virtual int getNumFunctions();
  virtual const function_descriptor_struct *getExportedFunctions();
  virtual GUID getClassGuid() { return embeddedXuiGuid; }
  virtual ScriptObject *instantiate();
  virtual void destroy(ScriptObject *o);
  virtual void *encapsulate(ScriptObject *o);
  virtual void deencapsulate(void *o);
  virtual ScriptObject *cast(ScriptObject *o, GUID g);

private:

  static function_descriptor_struct exportedFunction[];
  static scriptVar EmbeddedXui_getEmbeddedObject(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
};

extern COMEXP EmbeddedXuiScriptController *embeddedXuiController;


#endif