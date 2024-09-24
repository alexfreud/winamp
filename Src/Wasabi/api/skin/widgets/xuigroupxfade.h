#ifndef _XUIGROUPXFADE_H
#define _XUIGROUPXFADE_H

#include <api/wnd/wndclass/guiobjwnd.h>

#define GROUPXFADE_PARENT GuiObjectWnd
class GroupXFade : public GROUPXFADE_PARENT {
public:
  static const wchar_t *xuiobject_getXmlTag() { return L"GroupXFade"; }
	static const char *xuiobject_getServiceName() { return "GroupXFade XuiObject"; }
  GroupXFade();
  virtual ~GroupXFade();
  
  virtual int onInit();
  
  virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);
  virtual int onResize();

  enum {
    GROUPXFADE_SETGROUP,
    GROUPXFADE_SETSPEED,
  };

  void setNewGroup(const wchar_t *grp);
protected:
	/*static */void CreateXMLParameters(int master_handle);
private:
  int myxuihandle;
  GuiObject *child[2];
  StringW id[2];
  int curchild;
  double speed;

	static XMLParamPair params[];
};


class GroupXFadeXuiSvc : public XuiObjectSvc2<GroupXFade> {};

#endif
