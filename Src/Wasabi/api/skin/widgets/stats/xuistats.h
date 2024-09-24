#ifndef __XUISTATS_H
#define __XUISTATS_H

#include <api/wnd/wndclass/guiobjwnd.h>

#define XUISTATS_PARENT GuiObjectWnd

// {12D9C377-A981-4b77-95E0-242AF7226960}
static const GUID COLOREDIT_PREVIEWRECT_GUID = 
{ 0x12d9c377, 0xa981, 0x4b77, { 0x95, 0xe0, 0x24, 0x2a, 0xf7, 0x22, 0x69, 0x60 } };

class ColorEditorInstance;  

// -----------------------------------------------------------------------
class XuiStats : public XUISTATS_PARENT {
  
  public:

    XuiStats();
    virtual ~XuiStats();

    virtual int onInit();
    virtual int onPaint(Canvas *c);
    virtual void onSetVisible(int show);
    virtual void timerCallback(int p1);

		virtual void addLine(const wchar_t *txt, const Wasabi::FontInfo *fontInfo);

  private:
		void doTextOut(Canvas *canvas, const wchar_t *text, int line, int col, const Wasabi::FontInfo *fontInfo);
    int hastimer;
   
    int line ;
    int col;
    Canvas *curcanvas;
};

// -----------------------------------------------------------------------
extern const wchar_t XuiStatsXuiObjectStr[];
extern char XuiStatsXuiSvcName[];
class XuiStatsXuiSvc : public XuiObjectSvc<XuiStats, XuiStatsXuiObjectStr, XuiStatsXuiSvcName> {};

#endif
