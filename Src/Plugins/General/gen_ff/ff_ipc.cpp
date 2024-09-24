#include "precomp__gen_ff.h"
#include "gen_ff_ipc.h"
#include "ff_ipc.h"
#include "../winamp/wa_ipc.h"
#include "wa2frontend.h"
#include <tataki/color/skinclr.h>
#include <api/wnd/wndclass/buttwnd.h>
#include <api/wndmgr/skinwnd.h>
#include <tataki/blending/blending.h>
#include <api/skin/skinparse.h>
#include <api/wnd/wndtrack.h>
#include "wa2wndembed.h"
#include "embedwndguid.h"
#include <tataki/canvas/bltcanvas.h>
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
ColorThemeMonitor *colorThemeMonitor = NULL;
 
HBITMAP CreateBitmapDIB(int w, int h, int planes, int bpp, void *data)
{
#if 0
  return CreateBitmap(w, h, planes, bpp, data);
#else
  void *bits=0;
  BITMAPINFO bmi={0,};
  bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = w;
  bmi.bmiHeader.biHeight = -h;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = bpp;
  bmi.bmiHeader.biSizeImage = w*h*(bpp/8);
  bmi.bmiHeader.biCompression=BI_RGB;
  
  HBITMAP bm=CreateDIBSection(0,&bmi,0,&bits,NULL,0);

  if (bm && bits) memcpy(bits,data,w*h*(bpp/8));
  return bm;
#endif
}

HWND ff_ipc_getContentWnd(HWND w) {
  HWND ret = w;

  ifc_window *wnd = windowTracker->rootWndFromHwnd(w); // TODO: API_WNDMGR->
  if (wnd) {
    ifc_window *dp = wnd->getDesktopParent();
    if (dp) {
      Layout *l = static_cast<Layout *>(dp->getInterface(layoutGuid));
      if (l) {
        Container *c = l->getParentContainer();
        if (c) {
          GUID g = c->getDefaultContent();
          if (g != INVALID_GUID) {
						if (g == playerWndGuid)
							ret = wa2.getMainWindow();
            else if (g == pleditWndGuid) 
              ret = wa2.getWnd(IPC_GETWND_PE);
            else if (g == videoWndGuid) 
              ret = wa2.getWnd(IPC_GETWND_VIDEO);
            else {
              embedWindowState *ews = embedWndGuidMgr.getEmbedWindowState(g);
							if (ews)
              ret = ews->me;
            }
          }
        }
      }
    }
  }
  return ret;
}


void ff_ipc_getSkinColor(ff_skincolor *cs)
{
  if (cs == NULL) return;
  cs->color = RGBTOBGR(SkinColor(AutoWide(cs->colorname)));
}
#define USE_WIN32_ALPHABLEND
static void ButtonSetup(ButtonWnd &_w)
{
	_w.setVirtual(0);
  _w.setStartHidden(1);
  _w.setParent(WASABI_API_WND->main_getRootWnd());
  _w.init(WASABI_API_WND->main_getRootWnd());
	_w.setCloaked(1);
  _w.setVisible(1);
}

static void DoButtonBlit(ButtonWnd &_w, int w, int h, int state, const wchar_t *overlayelement, int xpos, int ypos, BltCanvas *c)
{
  if (state == BUTTONSTATE_PUSHED) _w.setPushed(1);
	else _w.setPushed(0);
  _w.resize(0, 0, w, h);
  _w.deferedInvalidate();
  _w.paint(NULL, NULL);
  Canvas *cv = NULL;
  cv = _w.getFrameBuffer();
  if (cv != NULL) {
    BltCanvas *bltcanvas = static_cast<BltCanvas *>(cv); // hackish
#ifdef USE_WIN32_ALPHABLEND
    bltcanvas->/*getSkinBitmap()->*/blitAlpha(c, xpos, ypos);
#else
	bltcanvas->getSkinBitmap()->blitAlpha(c, xpos, ypos);
#endif
  }
	  if (overlayelement && *overlayelement) {
    AutoSkinBitmap b(overlayelement);
    SkinBitmap *sb = b.getBitmap();
    int shift = (state == BUTTONSTATE_PUSHED) ? 1 : 0;
    sb->blitAlpha(c, xpos+(w-sb->getWidth())/2+shift, ypos+(h-sb->getHeight())/2+shift);
  }
}

void blitButtonToCanvas(int w, int h, int state, const wchar_t *overlayelement, int xpos, int ypos, BltCanvas *c) 
{
  ButtonWnd _w;
	ButtonSetup(_w);
	DoButtonBlit(_w, w, h, state, overlayelement, xpos, ypos, c);

}

static HBITMAP generateButtonBitmap(int w, int h, int state, const wchar_t *overlayelement=NULL) {
  BltCanvas c(w, h);
  blitButtonToCanvas(w,h,state,overlayelement,0,0,&c);
  SysCanvas cc;
  c.blit(0, 0, &cc, 0, 0, w, h);
  return CreateBitmapDIB(w, h, 1, 32, c.getBits());
}

COLORREF getWindowBackground(COLORREF *wb)
{
  static String last_skin, last_theme;
  static COLORREF last_windowbackground = 0x00000000;

  COLORREF windowbackground = 0x00000000;

  // window background (used to set the bg color for the dialog)
  if (!WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.window.background"))
	{
    String curskin = AutoChar(WASABI_API_SKIN->getSkinName());
    String curtheme = AutoChar(WASABI_API_SKIN->colortheme_getColorSet());
    if (!last_skin.iscaseequal(curskin) || !last_theme.iscaseequal(curtheme)) {
      last_skin = curskin;
      last_theme = curtheme;
      // extract the window color from an active piece of skin
      SkinWnd w(L"$$$.some.purposedly.undefined.group.$$$", NULL, 0, NULL, 1, 1);
      ifc_window *wnd = w.getWindow();
      if (wnd != NULL) {
        ifc_window *wh = wnd->findWindowByInterface(windowHolderGuid);
        if (wh != NULL) {
          wnd = wnd->getDesktopParent();
          if (w.isNewContainer()) {
            wnd->setCloaked(1);
            wnd->setVisible(1);
            wnd->resize(0, 0, 320, 200);
            wnd->paint();
          }
          BltCanvas *canvas = static_cast<BltCanvas *>(wnd->getFrameBuffer());
          if (!canvas) goto basetexture; // eek :-D
          int x=0, y=0;
          RECT r;
          wh->getClientRect(&r);
          x = r.left + (r.right-r.left)/2; y = r.top + (r.bottom-r.top)/2;
          COLORREF *bits = (COLORREF *)canvas->getBits();
          int w, h;
          canvas->getDim(&w, &h, NULL);
          if (w == 0 || h == 0) 
            windowbackground = 0; // black by default
          else
            windowbackground = bits[y*w+x];
        }
      } 
      w.destroy();
      if (windowbackground == 0x00000000) {
        basetexture:
        // try for wasabi.basetexture ...
        int _w, _h, _x, _y;
        ARGB32 *b = WASABI_API_SKIN->imgldr_requestSkinBitmap(L"wasabi.basetexture", NULL, &_x, &_y, NULL, NULL, &_w, &_h, 1);
        if (b != NULL) {
          windowbackground = b[_w*_y+_x];
          WASABI_API_SKIN->imgldr_releaseSkinBitmap(b);
        } else {
          // no idea... we'll just set the default windows color
          windowbackground = GetSysColor(COLOR_WINDOWFRAME);
        }
      }
      last_windowbackground = windowbackground;
    } else {
      windowbackground = last_windowbackground;
    }
    if (wb) *wb=windowbackground;
    return windowbackground;
  } else  {
    COLORREF c = RGBTOBGR(SkinColor(L"wasabi.window.background"));
    if (wb) *wb = c;
    return c; 
  }
}

inline int lumidiff(int a, int b) {
  int r1 = (a & 0xFF0000) >> 16;
  int r2 = (b & 0xFF0000) >> 16;
  int g1 = (a & 0xFF00) >> 8;
  int g2 = (b & 0xFF00) >> 8;
  int b1 = a & 0xFF;
  int b2 = b & 0xFF;
  return MIN((ABS(r1-r2), ABS(g1-g2)), ABS(b1-b2));
}

HBITMAP ff_genwa2skinbitmap()
{
  int interpolate = 0;
  if (SkinParser::getSkinVersion()*10 < 10) interpolate = 1;

  BltCanvas c(132, 75);
  COLORREF windowbackground = 0;
  COLORREF wbg=getWindowBackground(&windowbackground);

  COLORREF *ptr=(COLORREF *)c.getBits();
  c.fillBits(wbg);

  // set up bg color for the picky pixel parser heh
  int x;
  for (x = 47; x < 132; x ++) ptr[x]=ptr[x+132]=RGB(0,198,255);

	  ButtonWnd _w;
	ButtonSetup(_w);

  DoButtonBlit(_w, 47,15,0,NULL,0,0,&c);
  DoButtonBlit(_w, 47,15,1,NULL,0,15,&c);

  DoButtonBlit(_w, 14,14,0,L"wasabi.button.label.arrow.up",0,31,&c);
  DoButtonBlit(_w, 14,14,0,L"wasabi.button.label.arrow.down",14,31,&c);
  DoButtonBlit(_w, 14,14,1,L"wasabi.button.label.arrow.up",28,31,&c);
  DoButtonBlit(_w, 14,14,1,L"wasabi.button.label.arrow.down",42,31,&c);

  DoButtonBlit(_w, 14,14,0,L"wasabi.button.label.arrow.left",0,45,&c);
  DoButtonBlit(_w, 14,14,0,L"wasabi.button.label.arrow.right",14,45,&c);
  DoButtonBlit(_w, 14,14,1,L"wasabi.button.label.arrow.left",28,45,&c);
  DoButtonBlit(_w, 14,14,1,L"wasabi.button.label.arrow.right",42,45,&c);

  DoButtonBlit(_w, 14,28,0,L"wasabi.scrollbar.vertical.grip",56,31,&c);
  DoButtonBlit(_w, 14,28,1,L"wasabi.scrollbar.vertical.grip",70,31,&c);

  DoButtonBlit(_w, 28,14,0,L"wasabi.scrollbar.horizontal.grip",84,31,&c);
  DoButtonBlit(_w, 28,14,1,L"wasabi.scrollbar.horizontal.grip",84,45,&c);

  // item background (background to edits, listviews etc)
  if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.background"))
    ptr[48] = RGBTOBGR(SkinColor(L"wasabi.list.background")); 
  else
    ptr[48] = WASABI_API_SKIN->skin_getBitmapColor(L"wasabi.list.background"); // RGBTOBGR(SkinColor(L"wasabi.edit.background")); 

  COLORREF listbkg = ptr[48];

  // item foreground (text color of edit/listview, etc)  
  if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.text"))
    ptr[50] = RGBTOBGR(SkinColor(L"wasabi.list.text"));
  else {
    int c = RGBTOBGR(SkinColor(L"wasabi.edit.text"));
    ptr[50] = c; 
  }

  if (interpolate) {
    int c = ptr[50];
    c = lumidiff(c, listbkg) < 0x1F ? Blenders::BLEND_AVG(ptr[50], 0xFF7F7F7F) : c;
    c = lumidiff(c, listbkg) < 0x1F ? Blenders::BLEND_AVG(ptr[50], 0xFF101010) : c;
    ptr[50] = c | 0xFF000000;
  }

  ptr[52] = wbg;

  ptr[54] = RGBTOBGR(SkinColor(SKINCOLOR_BUTTON_TEXT));
  
  // window text color 
  if (!interpolate && WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.window.text")) 
    ptr[56] = RGBTOBGR(SkinColor(L"wasabi.window.text"));
  else {
    ptr[56] = RGBTOBGR(SkinColor(SKINCOLOR_LIST_ITEMTEXT)); //"wasabi.textbar.text");//
  }
  
  
  // color of dividers and sunken borders
  if (!interpolate && WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.border.sunken")) 
    ptr[58] = RGBTOBGR(SkinColor(L"wasabi.border.sunken"));
  else  {
    int a = MAX((windowbackground & 0xFF0000) >> 16, MAX((windowbackground & 0xFF00) >> 8, windowbackground & 0xFF));
    ptr[58] = Blenders::BLEND_AVG(windowbackground, a > 0xE0 ? 0xFF000000: 0xFFFFFFFF);
  }

  // listview header background color
  COLORREF col = RGBTOBGR(SkinColor(SKINCOLOR_LIST_COLUMNBKG));
  if (interpolate) {
    int a = MAX((col & 0xFF0000) >> 16, MAX((col & 0xFF00) >> 8, col & 0xFF));
    col = a < 0x1F ? Blenders::BLEND_AVG(windowbackground, 0xFF000000) : col;
  }
  ptr[62] = col;

  // listview header text color
  ptr[64] = RGBTOBGR(SkinColor(SKINCOLOR_LIST_COLUMNTEXT));

  // listview header frame top color
  if (!interpolate && WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.column.frame.top"))
    ptr[66] = RGBTOBGR(SkinColor(L"wasabi.list.column.frame.top"));
  else
    ptr[66] = Blenders::BLEND_AVG(col, 0xFFFFFFFF);//listview header frame top color

  // listview header frame middle color
  if (!interpolate && WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.column.frame.middle"))
    ptr[68] = RGBTOBGR(SkinColor(L"wasabi.list.column.frame.middle"));
  else
    ptr[68] = Blenders::BLEND_AVG(col, 0xFF2F2F2F);

  // listview header frame bottom color
  if (!interpolate && WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.column.frame.bottom"))
    ptr[70] = RGBTOBGR(SkinColor(L"wasabi.list.column.frame.bottom"));
  else
    ptr[70] = Blenders::BLEND_AVG(col, 0xFF000000);

  // listview header empty color
  COLORREF empty;
  if (!interpolate && WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.column.empty"))
    empty = RGBTOBGR(SkinColor(L"wasabi.list.column.empty"));
  else 
    empty = Blenders::BLEND_AVG(col, 0xFF000000); 
  ptr[72] = empty;

  // scrollbar foreground color
  if (!interpolate && WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.scrollbar.foreground"))
    ptr[74] = RGBTOBGR(SkinColor(L"wasabi.scrollbar.foreground"));
  else
    ptr[74] = empty;

  // scrollbar background color
  if (!interpolate && WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.scrollbar.background"))
    ptr[76] = RGBTOBGR(SkinColor(L"wasabi.scrollbar.background"));
  else
    ptr[76] = empty;

  // inverse scrollbar foreground color
  if (!interpolate && WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.scrollbar.foreground.inverted"))
    ptr[78] = RGBTOBGR(SkinColor(L"wasabi.scrollbar.foreground.inverted"));
  else
    ptr[78] = col;

  // inverse scrollbar background color
  if (!interpolate && WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.scrollbar.background.inverted"))
    ptr[80] = RGBTOBGR(SkinColor(L"wasabi.scrollbar.background.inverted"));
  else
    ptr[80] = empty;

  // scrollbar dead area color
  ptr[82] = windowbackground;

  // listview/treeview selection bar text color
  COLORREF selfg;
  
  if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.text.selected"))
    selfg = RGBTOBGR(SkinColor(L"wasabi.list.text.selected"));
  else
    if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.text"))
      selfg = RGBTOBGR(SkinColor(L"wasabi.list.text"));
    else
      selfg = SkinColor(L"wasabi.edit.text");

  ptr[84] = selfg;


  // listview/treeview selection bar back color
  COLORREF selbg;
  if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.text.selected.background"))
    selbg = RGBTOBGR(SkinColor(L"wasabi.list.text.selected.background"));
  else {
    if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.item.selected"))
      selbg = RGBTOBGR(SkinColor(L"wasabi.list.item.selected"));
    else 
      selbg = RGBTOBGR(SkinColor(SKINCOLOR_TREE_SELITEMBKG));
    selbg = lumidiff(selbg, listbkg) < 0x2F ? Blenders::BLEND_AVG(windowbackground, 0xFF7F7F7F) : selbg;
    selbg = lumidiff(selbg, listbkg) < 0x2F ? Blenders::BLEND_AVG(listbkg, 0xFF7F7F7F) : selbg;
    selbg = lumidiff(selbg, listbkg) < 0x2F ? Blenders::BLEND_AVG(windowbackground, 0xFFF0F0F0) : selbg;
    selbg = lumidiff(selbg, listbkg) < 0x2F ? Blenders::BLEND_AVG(listbkg, 0xFFF0F0F0) : selbg;
    selbg = lumidiff(selbg, listbkg) < 0x2F ? Blenders::BLEND_AVG(col, 0xFFF0F0F0) : selbg;
    selbg = lumidiff(selbg, listbkg) < 0x2F ? Blenders::BLEND_AVG(col, 0xFF101010) : selbg;
    selbg = lumidiff(selbg, selfg) < 0x1F ? Blenders::BLEND_AVG(selbg, 0xFF101010) : selbg;
    selbg = lumidiff(selbg, selfg) < 0x1F ? Blenders::BLEND_AVG(selbg, 0xFFF0F0F0) : selbg;
  }
  ptr[86] = ptr[60] = (selbg | 0xFF000000);

  // listview/treeview selection bar text color (inactive)
  if (!interpolate && WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.text.selected.inactive"))
    ptr[88] = RGBTOBGR(SkinColor(L"wasabi.list.text.selected.inactive"));
  else
    ptr[88] = selfg;
  
  // listview/treeview selection bar back color (inactive)
  if (!interpolate && WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.text.selected.background.inactive"))
    ptr[90] = RGBTOBGR(SkinColor(L"wasabi.list.text.selected.background.inactive"));
  else
    ptr[90] = Blenders::BLEND_ADJ1(selbg, 0xFF000000, 210);

  // alternate item background
  if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.background.alternate"))
    ptr[92] = RGBTOBGR(SkinColor(L"wasabi.list.background.alternate")); 
  else
    ptr[92] = ptr[48];

  // alternate item foreground
  if (WASABI_API_SKIN->skin_getColorElementRef(L"wasabi.list.text.alternate"))
  {
    ptr[50] = RGBTOBGR(SkinColor(L"wasabi.list.text.alternate"));
	if (interpolate) {
		int c = ptr[94];
		c = lumidiff(c, listbkg) < 0x1F ? Blenders::BLEND_AVG(ptr[94], 0xFF7F7F7F) : c;
		c = lumidiff(c, listbkg) < 0x1F ? Blenders::BLEND_AVG(ptr[94], 0xFF101010) : c;
		ptr[94] = c | 0xFF000000;
	}
  }
  else {
    ptr[94] = ptr[50];
  }

  return CreateBitmapDIB(132, 75, 1, 32, c.getBits());
}

void ff_ipc_genSkinBitmap(ff_skinbitmap *sb) {
  if (sb == NULL) return;
  switch (sb->id) {
    case SKINBITMAP_BUTTON: {
      HBITMAP bmp = generateButtonBitmap(sb->w, sb->h, sb->state);
      sb->bitmap = bmp;
      break;
    }
    case SKINBITMAP_SCROLLBARVBUTTON: {
      HBITMAP bmp = generateButtonBitmap(sb->w, sb->h, sb->state, L"wasabi.scrollbar.vertical.grip");
      sb->bitmap = bmp;
      break;
    }
    case SKINBITMAP_SCROLLBARHBUTTON: {
      HBITMAP bmp = generateButtonBitmap(sb->w, sb->h, sb->state, L"wasabi.scrollbar.horizontal.grip");
      sb->bitmap = bmp;
      break;
    }
    case SKINBITMAP_SCROLLBARUPBUTTON: {
      HBITMAP bmp = generateButtonBitmap(sb->w, sb->h, sb->state, L"wasabi.button.label.arrow.up");
      sb->bitmap = bmp;
      break;
    }
    case SKINBITMAP_SCROLLBARDOWNBUTTON: {
      HBITMAP bmp = generateButtonBitmap(sb->w, sb->h, sb->state, L"wasabi.button.label.arrow.down");
      sb->bitmap = bmp;
      break;
    }
    case SKINBITMAP_SCROLLBARLEFTBUTTON: {
      HBITMAP bmp = generateButtonBitmap(sb->w, sb->h, sb->state, L"wasabi.button.label.arrow.left");
      sb->bitmap = bmp;
      break;
    }
    case SKINBITMAP_SCROLLBAR_FF_UPBUTTON: {
	  HBITMAP bmp = generateButtonBitmap(sb->w, sb->h, SCROLLBARSTATE_NORMAL,
										 ((sb->state==SCROLLBARSTATE_PRESSED)?L"wasabi.scrollbar.vertical.left.pressed":
										  ((sb->state==SCROLLBARSTATE_HOVER)?L"wasabi.scrollbar.vertical.left.hover":
										   L"wasabi.scrollbar.vertical.left")));
      sb->bitmap = bmp;
      break;
    }
    case SKINBITMAP_SCROLLBAR_FF_DOWNBUTTON: {
	  HBITMAP bmp = generateButtonBitmap(sb->w, sb->h, SCROLLBARSTATE_NORMAL,
										 ((sb->state==SCROLLBARSTATE_PRESSED)?L"wasabi.scrollbar.vertical.right.pressed":
										  ((sb->state==SCROLLBARSTATE_HOVER)?L"wasabi.scrollbar.vertical.right.hover":
										   L"wasabi.scrollbar.vertical.right")));
      sb->bitmap = bmp;
      break;
    }
    case SKINBITMAP_SCROLLBAR_FF_LEFTBUTTON: {
	  HBITMAP bmp = generateButtonBitmap(sb->w, sb->h, SCROLLBARSTATE_NORMAL,
										 ((sb->state==SCROLLBARSTATE_PRESSED)?L"wasabi.scrollbar.horizontal.left.pressed":
										  ((sb->state==SCROLLBARSTATE_HOVER)?L"wasabi.scrollbar.horizontal.left.hover":
										   L"wasabi.scrollbar.horizontal.left")));
      sb->bitmap = bmp;
      break;
    }
    case SKINBITMAP_SCROLLBAR_FF_RIGHTBUTTON: {
	  HBITMAP bmp = generateButtonBitmap(sb->w, sb->h, SCROLLBARSTATE_NORMAL,
										 ((sb->state==SCROLLBARSTATE_PRESSED)?L"wasabi.scrollbar.horizontal.right.pressed":
										  ((sb->state==SCROLLBARSTATE_HOVER)?L"wasabi.scrollbar.horizontal.right.hover":
										   L"wasabi.scrollbar.horizontal.right")));
      sb->bitmap = bmp;
      break;
    }
    case SKINBITMAP_SCROLLBAR_FF_BARHBUTTON: {
	  HBITMAP bmp = generateButtonBitmap(sb->w, sb->h, SCROLLBARSTATE_NORMAL,
										 ((sb->state==SCROLLBARSTATE_PRESSED)?L"wasabi.scrollbar.horizontal.button.pressed":
										  ((sb->state==SCROLLBARSTATE_HOVER)?L"wasabi.scrollbar.horizontal.button.hover":
										   L"wasabi.scrollbar.horizontal.button")));
      sb->bitmap = bmp;
      break;
    }
    case SKINBITMAP_SCROLLBAR_FF_BARVBUTTON: {
	  HBITMAP bmp = generateButtonBitmap(sb->w, sb->h, SCROLLBARSTATE_NORMAL,
										 ((sb->state==SCROLLBARSTATE_PRESSED)?L"wasabi.scrollbar.vertical.button.pressed":
										  ((sb->state==SCROLLBARSTATE_HOVER)?L"wasabi.scrollbar.vertical.button.hover":
										   L"wasabi.scrollbar.vertical.button.pressed")));
      sb->bitmap = bmp;
      break;
    }
  }

  #if 0 // debug

  HDC ddc = GetDC(NULL);
  HDC sdc = CreateCompatibleDC(ddc);
  SelectObject(sdc, sb->bitmap);

  BitBlt(ddc, 100, 100, sb->w, sb->h, sdc, 0, 0, SRCCOPY);

  DeleteObject(sdc);
  ReleaseDC(NULL, ddc);

  #endif
}

ColorThemeMonitor::ColorThemeMonitor() {
  WASABI_API_SYSCB->syscb_registerCallback(this);
}

ColorThemeMonitor::~ColorThemeMonitor() {
  WASABI_API_SYSCB->syscb_deregisterCallback(this);
}

int ColorThemeMonitor::skincb_onColorThemeChanged( const wchar_t *colortheme )
{
    SendMessageW( wa2.getMainWindow(), WM_WA_IPC, (WPARAM) colortheme, IPC_FF_ONCOLORTHEMECHANGED );
    return 1;
}
