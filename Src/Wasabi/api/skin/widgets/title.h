//PORTABLE
#ifndef _TITLE_H
#define _TITLE_H

#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/script/objects/guiobj.h>

// {7DFD3244-3751-4e7c-BF40-82AE5F3ADC33}
static const GUID titleGuid = 
{ 0x7dfd3244, 0x3751, 0x4e7c, { 0xbf, 0x40, 0x82, 0xae, 0x5f, 0x3a, 0xdc, 0x33 } };

#define TITLE_PARENT GuiObjectWnd

class TitleScriptController : public GuiObjectScriptController {
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

extern TitleScriptController *titleController;


class Title : public TITLE_PARENT {
public:
  
  Title();
  virtual ~Title();

	virtual int onPaint(Canvas *canvas);
  virtual void setTitle(const wchar_t *title);
  virtual const wchar_t *getTitle();
  virtual int onLeftButtonDblClk(int x, int y);
  virtual int getPreferences(int what);

  virtual int setXuiParam(int _xuihandle, int attrid, const wchar_t *name, const wchar_t *strval);
  virtual const wchar_t *vcpu_getClassName();
  virtual ScriptObjectController *vcpu_getController() { return titleController; }
  virtual int onDeferredCallback(intptr_t param1, intptr_t param2);

  void setBorder(int b);
  void setStreaks(int s);

  enum {
    TITLE_SETTITLE=0,
    TITLE_SETSTREAKS,
    TITLE_SETBORDER,
    TITLE_SETMAXIMIZE,
    TITLE_SETDBLCLKACTION,
  };
	
protected:
	/*static */void CreateXMLParameters(int master_handle);

private:
	static XMLParamPair params[];
  StringW title;
  int dostreaks, doborder;
  int m_maximize;
  StringW dblClickAction;
  int xuihandle;

public:

  static scriptVar script_vcpu_fake(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);

};

extern const wchar_t titleBarXuiObjectStr[];
extern char titleBarXuiSvcName[];
class TitleBarXuiSvc : public XuiObjectSvc<Title, titleBarXuiObjectStr, titleBarXuiSvcName> {};


#endif
