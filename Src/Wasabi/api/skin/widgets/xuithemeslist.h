#ifndef __THEMESLIST_H
#define __THEMESLIST_H

#include <api/wnd/wndclass/listwnd.h>
#include <api/skin/nakedobject.h>
#include <api/service/svcs/svc_action.h>

class ThemesSlotActionSvc : public svc_actionI {
public:
  ThemesSlotActionSvc() {
		registerAction(L"ThemesSlotsMenu", 0);
  }
  virtual ~ThemesSlotActionSvc() {}
  virtual int onActionId(int pvtid, const wchar_t *action, const wchar_t *param=NULL, int p1=0, int p2=0, void *data=NULL, int datalen=0, ifc_window *source=NULL);
  static const char *getServiceName() { return "ThemesSlotMenu Service"; }
};

#define THEMESLIST_PARENT ListWnd
#define THEMESLIST2_PARENT NakedObject

// -----------------------------------------------------------------------
class ColorThemesList : public THEMESLIST_PARENT, public SkinCallbackI 
{
  
  public:

    ColorThemesList();
    virtual ~ColorThemesList();

    virtual int onInit();
    virtual void onDoubleClick(int itemnum);
    virtual int onPaint(Canvas *canvas);
    virtual int onRightClick(int itemnum);

    virtual int onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source);
    virtual int onResize();
    virtual int wantResizeCols() { return 0; }
    virtual int setXuiParam(int _xuihandle, int xmlattrid, const wchar_t *name, const wchar_t *value);
    virtual int wantHScroll() { return !nohscroll; }

    virtual int getTextBold(LPARAM lParam);
    virtual void onSetVisible(int show);

    virtual int skincb_onColorThemesListChanged() { loadThemes(); return 1;}

    enum {
      CTLIST_NOHSCROLL = 0,
    };

    static void setSlot(int s, const wchar_t *set);
    static const wchar_t *getSlot(int s);
protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:
    static XMLParamPair params[];
    void colorthemes_switch();
    void colorthemes_next();
    void colorthemes_previous();
    void colorthemes_advance(int i);
    void loadThemes();
    int xuihandle;
    int nohscroll;
    int ensure_on_paint;
};

// -----------------------------------------------------------------------
class NakedItem 
{
  public:
    NakedItem(const wchar_t *_name, int _p) : name(_name), data(_p) {}
    virtual ~NakedItem() {}
    const wchar_t *getName() { return name; }
    int getData() { return data; }
  private:
    StringW name;
    int data;
};

// -----------------------------------------------------------------------
class NakedItemSort {
public:
  // comparator for sorting
  static int compareItem(NakedItem *p1, NakedItem *p2) {
    return _wcsicmp(p1->getName(), p2->getName());
  }
  // comparator for searching
  static int compareAttrib(const wchar_t *attrib, NakedItem *item) {
    return _wcsicmp(attrib, item->getName());
  }
};


// -----------------------------------------------------------------------
class NakedColorThemesList : public THEMESLIST2_PARENT, public SkinCallbackI 
{
  
  public:

    NakedColorThemesList();
    virtual ~NakedColorThemesList();

    virtual int onAction(const wchar_t *action, const wchar_t *param, int x, int y, intptr_t p1, intptr_t p2, void *data, size_t datalen, ifc_window *source);

    virtual void onSetVisible(int show);

    virtual int skincb_onColorThemesListChanged() { loadThemes(); return 1;}

  private:
    
    PtrListQuickSorted<NakedItem, NakedItemSort> items;
    void colorthemes_switch();
    void colorthemes_next();
    void colorthemes_previous();
    void colorthemes_advance(int i);
    void loadThemes();
    int xuihandle;
};


// -----------------------------------------------------------------------
extern const wchar_t ColorThemesListXuiObjectStr[];
extern char ColorThemesListXuiSvcName[];
class ColorThemesListXuiSvc : public XuiObjectSvc<ColorThemesList, ColorThemesListXuiObjectStr, ColorThemesListXuiSvcName> {};
extern const wchar_t NakedColorThemesListXuiObjectStr[];
extern char NakedColorThemesListXuiSvcName[];
class NakedColorThemesListXuiSvc : public XuiObjectSvc<NakedColorThemesList, NakedColorThemesListXuiObjectStr, NakedColorThemesListXuiSvcName> {};

#endif
