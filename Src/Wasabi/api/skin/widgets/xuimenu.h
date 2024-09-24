#ifndef _XUIMENU_H
#define _XUIMENU_H

#include <api/wnd/wndclass/guiobjwnd.h>

/*<?<autoheader/>*/
#include "xuimenuso.h"
/*?>*/

#define XUIMENU_PARENT XuiMenuScriptObject

// {A0211C57-DCED-45ae-AEA6-56014B5898E8}
static const GUID xuiMenuGuid = 
{ 0xa0211c57, 0xdced, 0x45ae, { 0xae, 0xa6, 0x56, 0x1, 0x4b, 0x58, 0x98, 0xe8 } };

/*<?<classdecl name="XuiMenu" factory="ScriptObject" />*/
class XuiMenu : public XuiMenuScriptObject 
{
/*?>*/
  public:
    friend LRESULT CALLBACK xuimenu_KeyboardProc(int code, WPARAM wParam, LPARAM lParam);
    friend LRESULT CALLBACK xuimenu_msgProc(int code, WPARAM wParam, LPARAM lParam);

    XuiMenu();
    virtual ~XuiMenu();

    virtual int onInit();
    virtual int onLeftButtonDown(int x, int y);
    int setXuiParam(int _xuihandle, int xmlattrid, const wchar_t *name, const wchar_t *value);
    virtual void timerCallback(int c);
    virtual int onDeferredCallback(intptr_t p1, intptr_t p2);

    SCRIPT void setMenuGroup(const wchar_t *mg);
    SCRIPT const wchar_t *getMenuGroup() { return menugroup; }

    SCRIPT void setMenu(const wchar_t *m);
    SCRIPT const wchar_t *getMenu() { return menuid; }

    SCRIPT void spawnMenu(int monitor = 1);
    SCRIPT void cancelMenu();

    SCRIPT void setNormalId(const wchar_t *id);
    SCRIPT void setDownId(const wchar_t *id);
    SCRIPT void setHoverId(const wchar_t *id);

    SCRIPT EVENT virtual void onOpenMenu();
    SCRIPT EVENT virtual void onCloseMenu();

    virtual void onEnterArea();
    virtual void onLeaveArea();
    virtual int onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source);

    SCRIPT void nextMenu();
    SCRIPT void previousMenu();
    
    enum {
      MENU_MENU = 0,
      MENU_MENUGROUP,
      MENU_NORMALID,
      MENU_DOWNID,
      MENU_HOVERID,
      MENU_NEXT,
      MENU_PREV,
    };
		
protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:
		static XMLParamPair params[];
    void onMenuSelect(HWND hwnd, HMENU menu, int menuitem, int flags);
    void startKbdHook();
    void stopKbdHook();
    void updateObjects();
    void timerCheck();
    void switchToMenu(XuiMenu *menu);
    void _nextMenu();
    void _previousMenu();
    void openAction();
    void onTrappedLeft();
    void onTrappedRight();
    StringW menugroup;
    StringW menuid;
    int xuihandle;
    XuiMenu *nextinchain;
    int timerset;
    int disablespawn;
    GuiObject *normal;
    GuiObject *down;
    GuiObject *hover;
    StringW next, prev;
    int isspawned;
    StringW normalid, downid, hoverid;
    int inarea;
    int kbdhook;
    int orig_x, orig_y;
    int kbdlocktimer;
    int submenu_isselected;
    int submenu_selectedbymouse;
    HMENU submenu_selected;
    HWND menu_parent;
    HMENU first_hmenu;
    HMENU cur_hmenu;
};

// -----------------------------------------------------------------------
extern const wchar_t MenuXuiObjectStr[];
extern char MenuXuiSvcName[];
class MenuXuiSvc : public XuiObjectSvc<XuiMenu, MenuXuiObjectStr, MenuXuiSvcName> {};

#endif
