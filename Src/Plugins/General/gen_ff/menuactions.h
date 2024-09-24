#ifndef _MENUACTIONS_H
#define _MENUACTIONS_H

#include <api/service/svcs/svc_action.h>

extern int ffoptionstop;
extern int ffwoptionstop;
extern int in_menu;

class MenuActions : public svc_actionI {
  public :
    MenuActions();
    virtual ~MenuActions();

    static const char *getServiceName() { return "Menu Actions"; }
    virtual int onActionId(int pvtid, const wchar_t *action, const wchar_t *param=NULL, int p1=0, int p2=0, void *data=NULL, int datalen=0, ifc_window *source=NULL);

    static void installSkinOptions(HMENU menu=NULL);
    static void removeSkinOptions();
    static int toggleOption(int n, GUID g=INVALID_GUID, int *cmdoffset=NULL);

    static void installSkinWindowOptions();
    static void removeSkinWindowOptions();
    static int toggleWindowOption(int n, GUID g=INVALID_GUID, int *cmdoffset=NULL);

    static HMENU makeSkinOptionsSubMenu(GUID g, int *cmdoffset);

	static const wchar_t* localizeSkinWindowName(const wchar_t*);

    enum {
      _ACTION_MENU = 0,
      _ACTION_SYSMENU,
      _ACTION_CONTROLMENU,
      ACTION_WA5FILEMENU,
      ACTION_WA5PLAYMENU,
      ACTION_WA5OPTIONSMENU,
      ACTION_WA5WINDOWSMENU,
      ACTION_WA5HELPMENU,
      ACTION_WA5PEFILEMENU,
      ACTION_WA5PEPLAYLISTMENU,
      ACTION_WA5PESORTMENU,
      ACTION_WA5PEHELPMENU,
      ACTION_WA5MLFILEMENU,
      ACTION_WA5MLVIEWMENU,
      ACTION_WA5MLHELPMENU,
      ACTION_PEADD,
      ACTION_PEREM,
      ACTION_PESEL,
      ACTION_PEMISC,
      ACTION_PELIST,
      ACTION_PELISTOFLISTS,
      ACTION_VIDFS,
      ACTION_VID1X,
      ACTION_VID2X,
      ACTION_VIDTV,
      ACTION_VIDMISC,
      ACTION_VISNEXT,
      ACTION_VISPREV,
      ACTION_VISRANDOM,
      ACTION_VISFS,
      ACTION_VISCFG,
      ACTION_VISMENU,
      ACTION_TRACKINFO,
      ACTION_TRACKMENU,
      ACTION_SENDTO,
    };
};


class ColorThemeSlot 
{
public:
  ColorThemeSlot(const wchar_t *_name, int _entry) : name(_name), entry(_entry) {}
  virtual ~ColorThemeSlot() {}
  StringW name;
  int entry;
};

class ColorThemeSlotSort {
public:
  // comparator for sorting
  static int compareItem(ColorThemeSlot *p1, ColorThemeSlot *p2) {
    return wcscmp(p1->name, p2->name);
  }
  // comparator for searching
  static int compareAttrib(const wchar_t *attrib, ColorThemeSlot *item) {
    return wcscmp(attrib, item->name);
  }
};

#endif