//PORTABLE
#ifndef _GROUPLIST_H
#define _GROUPLIST_H

#include <api/script/objects/guiobj.h>
#include <api/wnd/wndclass/guiobjwnd.h>
#include <bfc/ptrlist.h>

// {01E28CE1-B059-11d5-979F-E4DE6F51760A}
static const GUID grouplistGuid = 
{ 0x1e28ce1, 0xb059, 0x11d5, { 0x97, 0x9f, 0xe4, 0xde, 0x6f, 0x51, 0x76, 0xa } };

#define GROUPLIST_PARENT GuiObjectWnd

class GroupListScriptController: public GuiObjectScriptController {
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

extern GroupListScriptController *grouplistController;

class GroupList : public GROUPLIST_PARENT {
public:
  GroupList();
  virtual ~GroupList();

  Group *instantiate(const wchar_t *id, int n);
  void removeAll();
  Group *enumItem(int n);
  int getNumItems();
  void scrollToPercent(int p);
  void scrollTo(int y);
  void setRedraw(int i);
  void reposChildren();
  virtual int onResize();

  virtual int getPreferences(int what);

private:  

  void insert(const wchar_t *id, int where);

public:

  static scriptVar script_vcpu_instantiate(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar id, scriptVar n);
  static scriptVar script_vcpu_getNumItems(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_enumItem(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n);
  static scriptVar script_vcpu_removeAll(SCRIPT_FUNCTION_PARAMS, ScriptObject *o);
  static scriptVar script_vcpu_scrollToPercent(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n);
  static scriptVar script_vcpu_setRedraw(SCRIPT_FUNCTION_PARAMS, ScriptObject *o, scriptVar n);

  PtrList<Group> groups;
  int scrollY;
  int maxheight;
  int maxwidth;
  int redraw;
};

extern const wchar_t groupListXuiObjectStr[];
extern char groupListXuiSvcName[];
class GroupListXuiSvc : public XuiObjectSvc<GroupList, groupListXuiObjectStr, groupListXuiSvcName> {};


#endif
