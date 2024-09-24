//PORTABLE
#ifndef _COMPBUCK_H
#define _COMPBUCK_H

#include <api/wnd/wndclass/clickwnd.h>
#include <api/skin/widgets/text.h>
#include <api/wnd/wndclass/guiobjwnd.h>
#include <api/service/svc_enum.h>
#include <api/script/objects/guiobj.h>
#include <api/service/svcs/svc_wndcreate.h>

// {97AA3E4D-F4D0-4fa8-817B-0AF22A454983}
static const GUID cbucketGuid = 
{ 0x97aa3e4d, 0xf4d0, 0x4fa8, { 0x81, 0x7b, 0xa, 0xf2, 0x2a, 0x45, 0x49, 0x83 } };

#define COMPONENTBUCKET2_PARENT GuiObjectWnd
#define COMPONENTBUCKET2_XMLPARENT GuiObjectWnd

class CompBucketScriptController: public GuiObjectScriptController {
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

extern CompBucketScriptController *cbucketController;

class Layout;

class ServiceWndHolder;

class ComponentBucket2 : public COMPONENTBUCKET2_PARENT {
public:
  ComponentBucket2();
  virtual ~ComponentBucket2();

  virtual int onInit();
  virtual int setXuiParam(int _xuihandle, int id, const wchar_t *name, const wchar_t *strval);

/*  virtual int getAutoHeight();
  virtual int getAutoWidth();*/

  virtual void timerCallback(int id);
  virtual int childNotify(ifc_window *child, int msg, intptr_t p1, intptr_t p2);

  virtual int onResize();
  virtual void setLMargin(int i);
  virtual void setRMargin(int i);
  virtual void setSpacing(int i);
  virtual int getLMargin(void);
  virtual int getRMargin(void);
  virtual int getSpacing(void);

  void next_page();
  void prev_page();
  void next_down();
  void next_up();
  void prev_down();
  void prev_up();
  static void next_down(Group *l); // next_down on all compbucks in this group
  static void next_up(Group *l); // next_up on all compbucks in this group
  static void prev_down(Group *l); // prev_down on all compbucks in this group
  static void prev_up(Group *l); // prev_up on all compbucks in this group
  static void prev_page(Group *l); // prev_down on all compbucks in this group
  static void next_page(Group *l); // prev_up on all compbucks in this group

  void setText(const wchar_t *txt);
  static void setText(ifc_window *cb , const wchar_t *txt); // set this text for this compbuck's rootwnd

  static void registerText(Text *t, const wchar_t *id=NULL); // id=NULL => register for all compbucks in this group
  static void unRegisterText(Text *t, const wchar_t *id=NULL); // id=NULL => unregister for all compbucks in this group

  static ComponentBucket2 *getComponentBucket(const wchar_t *cb);

  int getMaxWidth();
  int getMaxHeight();
  void setVertical(int v);
  void setScroll(int v);
  int getScroll();
  int getNumChildren();
  GuiObject *enumChildren(int i);

protected:
/*static */void CreateXMLParameters(int master_handle);
  enum {
    COMPBUCK_LEFTMARGIN=0,
    COMPBUCK_RIGHTMARGIN,
    COMPBUCK_SPACING,
    COMPBUCK_VERTICAL,
    COMPBUCK_WNDTYPE,
  };

private:  

  void load();
  void addItems(svc_windowCreate *wc);
  void doRegisterText(Text *t);
  void doUnregisterText(Text *t);

  int timeron;
  static PtrList<ComponentBucket2> cblist;
  PtrList<Text> txtlist;
  StringW id;
  PtrList<ServiceWndHolder> myclients;
  int lmargin;
  int rmargin;
  int spacing;

  int xscroll;
  int direction;
  int timerset;

  void startScrollTimer();
  void stopScrollTimer();
  uint32_t lastticcount;
  int vertical;
  int xuihandle;
	static XMLParamPair params[];
  StringW wndtype;

  uint32_t scrollpage_starttime;
  int scrollpage_timerset;
  int scrollpage_start;
  int scrollpage_target;
  int scrollpage_speed;

public:

  static scriptVar script_vcpu_fake(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getMaxWidth(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getMaxHeight(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_getScroll(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_setScroll(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v);
  static scriptVar script_vcpu_getNumChildren(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_enumChildren(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar v);
};

extern const wchar_t componentBucketXuiObjectStr[];
extern char componentBucketXuiSvcName[];
class ComponentBucketXuiSvc : public XuiObjectSvc<ComponentBucket2, componentBucketXuiObjectStr, componentBucketXuiSvcName> {};


#endif
