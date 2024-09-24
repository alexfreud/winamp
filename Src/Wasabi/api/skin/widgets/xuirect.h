#ifndef _XUIRECT_H
#define _XUIRECT_H

#include <api/wnd/wndclass/guiobjwnd.h>
#include <tataki/color/skinclr.h>
#include <tataki/canvas/bltcanvas.h>
#define SCRIPTRECT_PARENT GuiObjectWnd
class ScriptRect : public SCRIPTRECT_PARENT {
public:
  static const wchar_t *xuiobject_getXmlTag() { return L"Rect"; }
	static const char *xuiobject_getServiceName() { return "Rect XuiObject"; }

  ScriptRect();
  virtual ~ScriptRect();
  
  virtual int onInit();
  
  virtual int onPaint(Canvas *c);

  virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

protected:
	/*static */void CreateXMLParameters(int master_handle);
private:
	enum {
  SCRIPTRECT_SETCOLOR = 0,
  SCRIPTRECT_SETFILLED,
  SCRIPTRECT_EDGES,
  SCRIPTRECT_THICKNESS,
  SCRIPTRECT_GAMMAGROUP,
};
	static XMLParamPair params[];
  void resetPixel();

  int myxuihandle;
  SkinColor color;
  int filled, edges, thickness;
  BltCanvas pixel;
};

class ScriptRectXuiSvc : public XuiObjectSvc2<ScriptRect> {};

#endif
