#ifndef __GRID_H
#define __GRID_H

#include <api/wnd/wndclass/guiobjwnd.h>
#include <tataki/bitmap/autobitmap.h>
#include <tataki/region/region.h>

#define GRID_PARENT GuiObjectWnd 

// -----------------------------------------------------------------------
class Grid : public GRID_PARENT {
  
  public:

    Grid();
    virtual ~Grid();

    virtual int onPaint(Canvas *c); 
    virtual int setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value);

    void setGridImage(const wchar_t *elementname, int what);
    virtual void getGridRect(RECT *r) { getClientRect(r); }

    virtual api_region *getRegion() { return &reg; }
    virtual int onInit();
    virtual int onResize();
protected:
	/*static */void CreateXMLParameters(int master_handle);
  private:
    void doPaint(Canvas *canvas, int dorgn=0);

    enum {
      GRID_SETTOPLEFT= 0,
      GRID_SETTOP,
      GRID_SETTOPRIGHT,
      GRID_SETLEFT,
      GRID_SETMIDDLE,
      GRID_SETRIGHT,
      GRID_SETBOTTOMLEFT,
      GRID_SETBOTTOM,
      GRID_SETBOTTOMRIGHT,
    };
		static XMLParamPair params[];

    int myxuihandle;

    AutoSkinBitmap topleft,    top,    topright;
    AutoSkinBitmap left,       middle, right;
    AutoSkinBitmap bottomleft, bottom, bottomright;
    RegionI reg;
};


// -----------------------------------------------------------------------
extern const wchar_t GridXuiObjectStr[];
extern char GridXuiSvcName[];
class GridXuiSvc : public XuiObjectSvc<Grid, GridXuiObjectStr, GridXuiSvcName> {};

#endif
