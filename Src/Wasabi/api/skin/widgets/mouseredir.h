//PORTABLE
#ifndef _MOUSEREDIR_H
#define _MOUSEREDIR_H

#include <api/script/script.h>
#include <api/script/objects/guiobj.h>
#include <bfc/string/StringW.h>
#include <api/wnd/wndclass/guiobjwnd.h>

#define MOUSEREDIR_PARENT GuiObjectWnd

// {9B2E341B-6C98-40fa-8B85-0C1B6EE89405}
static const GUID mouseredirGuid = 
{ 0x9b2e341b, 0x6c98, 0x40fa, { 0x8b, 0x85, 0xc, 0x1b, 0x6e, 0xe8, 0x94, 0x5 } };


class SMap;
class SRegion;

class MouseRedirScriptController : public GuiObjectScriptController {
  public:

    virtual const wchar_t *getClassName();
    virtual const wchar_t *getAncestorClassName();
    virtual ScriptObjectController *getAncestorController() { return guiController; }
    virtual int getNumFunctions();
    virtual const function_descriptor_struct *getExportedFunctions();
    virtual GUID getClassGuid();
    virtual ScriptObject *instantiate();
    virtual void *encapsulate(ScriptObject *o);
    virtual void destroy(ScriptObject *o);
    virtual void deencapsulate(void *o);

  private:

    static function_descriptor_struct exportedFunction[];
    
};

extern MouseRedirScriptController *mouseredirController;

class MouseRedir : public MOUSEREDIR_PARENT {
public:
  MouseRedir();
  virtual ~MouseRedir();

  virtual int onInit();

  void setTarget(const wchar_t *id);

  void setRedirection(GuiObject *o);
  GuiObject *getRedirection();
  void setRegionFromMap(SMap *map, int byte, int inversed);
  void setRegion(SRegion *reg);
  virtual int mouseInRegion(int x, int y);

  virtual ifc_window *getForwardWnd();
  virtual int setXuiParam(int _xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

protected:
/*static */void CreateXMLParameters(int master_handle);
  enum {
    MOUSEREDIR_TARGET=0,
  };


private:
  GuiObject *redirobject;
  StringW deferedredirobjectid;
  int xuihandle;
	static XMLParamPair params[];

  void doSetTarget(const wchar_t *id);
  RegionI *rgn;

public:
  static scriptVar script_vcpu_setRedirection(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar obj);
  static scriptVar script_vcpu_getRedirection(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_setRegionFromMap(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar map, scriptVar byte, scriptVar inv);
  static scriptVar script_vcpu_setRegion(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar reg);
};

extern const wchar_t mouseRedirXuiObjectStr[];
extern char mouseRedirXuiSvcName[];
class MouseRedirXuiSvc : public XuiObjectSvc<MouseRedir, mouseRedirXuiObjectStr, mouseRedirXuiSvcName> {};


#endif
