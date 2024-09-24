#include <precomp.h>
// bitmap-style buttons

#include "buttwnd.h"
#include <bfc/wasabi_std.h>
#include <tataki/canvas/bltcanvas.h>
#include <tataki/region/region.h>
#include <api/wnd/notifmsg.h>


#include <api/wndmgr/msgbox.h>
#include <api/wnd/PaintCanvas.h>
#define DEFAULT_BUTTON_HEIGHT 20

ButtonWnd::ButtonWnd(const wchar_t *button_text)
{
  if (button_text != NULL) 
		setName(button_text);
  currgn = NULL;
  hirgn = NULL;
  normalrgn = NULL;
  pushedrgn = NULL;
  activatedrgn = NULL;
  base_texture = NULL;
  xShift=0;
  yShift=0;
  textsize = DEFAULT_BUTTON_TEXT_SIZE;
  alignment = TEXTALIGN_CENTER;
  activated = 0;
  userhilite = 0;
  userdown = 0;
  use_base_texture = 0;
  center_bitmap = 0;
  enabled = 1;
  checked=0;
  autodim=0;
  borders = 1;
	borderstyle = 0;
  setBorderStyle(L"button_normal");
  iwantfocus = 1;
  color_text = L"wasabi.button.text";
  color_hilite = L"wasabi.button.hiliteText";
  color_dimmed = L"wasabi.button.dimmedText";         
  checkbmp = L"wasabi.popup.menu.check";  
  inactivealpha = 255;
  activealpha = 255;
  setRectRgn(1);
  retcode = MSGBOX_ABORTED;
  forcedown=0;
}

ButtonWnd::~ButtonWnd() {
  delete normalrgn;
  delete pushedrgn;
  delete hirgn;
  delete activatedrgn;
}

void ButtonWnd::checkState(POINT *pt) {
  POINT pt2;
  if (pt == NULL) {
    pt = &pt2;
    Wasabi::Std::getMousePos(pt);
  }

  api_region *old = currgn;

  if (getDown()) { // button is down
    if (pushedrgn)
      currgn = pushedrgn;
    else
      currgn = normalrgn;
  } else { // button is not down
    if (hirgn && getHilite()) 
      currgn = hirgn;
    else
      currgn = normalrgn;
    }

  if (old != currgn) invalidateWindowRegion();
}

void ButtonWnd::onCancelCapture() {
  BUTTONWND_PARENT::onCancelCapture();
  checkState();
}

int ButtonWnd::onMouseMove(int x, int y) {
  POINT pt;
  checkState(&pt);
  return BUTTONWND_PARENT::onMouseMove(x, y);
}

api_region *ButtonWnd::getRegion() {
  if (borders) return NULL;
  return currgn;
}

void ButtonWnd::setModalRetCode(int r) { retcode = r; }
int ButtonWnd::getModalRetCode() const { return retcode; }

void ButtonWnd::onLeaveArea() {
  BUTTONWND_PARENT::onLeaveArea();
  if (hirgn || getDown()) invalidate();
}

void ButtonWnd::onEnterArea() {
  BUTTONWND_PARENT::onEnterArea();
  if (hirgn) invalidate();
}

/*BOOL ButtonWnd::mouseInRegion(int x, int y) {
  POINT pos={x,y};
  POINT p2=pos;

  RECT r;
  getClientRect(&r);
  pos.x-=r.left;
  pos.y-=r.top;

  return (((!currgn || rectrgn) && PtInRect(&r, p2)) || (currgn && currgn->ptInRegion(&pos)));
}*/

int ButtonWnd::setBitmaps(const wchar_t *_normal, const wchar_t *_pushed, const wchar_t *_hilited, const wchar_t *_activated) {

  if (_normal) { delete normalrgn; normalrgn = NULL; }
  if (_pushed) { delete pushedrgn; pushedrgn = NULL; }
  if (_hilited) { delete hirgn; hirgn = NULL; }
  if (_activated) { delete activatedrgn; activatedrgn = NULL; }

  if (_normal) {
    normalbmp = _normal;
    normalrgn = new RegionI(normalbmp.getBitmap());
    currgn = normalrgn;
  }

  if (_pushed) {
    pushedbmp = _pushed;
    pushedrgn = new RegionI(pushedbmp.getBitmap());
  }

  if (_hilited) {
    hilitebmp = _hilited;
    hirgn = new RegionI(hilitebmp.getBitmap());
  }

  if (_activated) {
    activatedbmp = _activated;
    activatedrgn = new RegionI(activatedbmp.getBitmap());
  }

  if (isPostOnInit())
    invalidate();

  return 1;
}

SkinBitmap *ButtonWnd::getNormalBitmap() {
  return normalbmp.getBitmap();
}

void ButtonWnd::freeResources() {
  BUTTONWND_PARENT::freeResources();
  delete normalrgn;
  delete pushedrgn;
  delete hirgn;
  delete activatedrgn;
  pushedrgn=NULL;
  normalrgn=NULL;
  hirgn=NULL;
  activatedrgn=NULL;
  currgn=NULL;
}

void ButtonWnd::reloadResources() {
  BUTTONWND_PARENT::reloadResources();
  if (normalbmp.getBitmap())
    normalrgn = new RegionI(normalbmp.getBitmap());
  if (pushedbmp.getBitmap())
    pushedrgn = new RegionI(pushedbmp.getBitmap());
  if (hilitebmp.getBitmap())
    hirgn = new RegionI(hilitebmp.getBitmap());
  if (activatedbmp.getBitmap())
    activatedrgn = new RegionI(activatedbmp.getBitmap());
  currgn = normalrgn;
}

int ButtonWnd::setBitmapCenter(int centerit) {
  return center_bitmap = !!centerit;
}

int ButtonWnd::setBitmaps(OSMODULEHANDLE hInst, int _normal, int _pushed, int _hilited, int _activated, const wchar_t *_colorgroup)
{
  if (_normal) { delete normalrgn; normalrgn = NULL; }
  if (_pushed) { delete pushedrgn; pushedrgn = NULL; }
  if (_hilited) { delete hirgn; hirgn = NULL; }
	if (_activated) { delete activatedrgn; activatedrgn = NULL; }

  if (_colorgroup == NULL)
    _colorgroup = colorgroup;

  if (_normal) 
  {
    normalbmp.setHInstanceBitmapColorGroup(_colorgroup);
#ifdef _WIN32
    normalbmp.setHInstance(hInst);
#else
#warning port me?
#endif
    normalbmp = _normal;
    normalrgn = new RegionI(normalbmp.getBitmap());
  }

  if (_pushed) {
    pushedbmp.setHInstanceBitmapColorGroup(_colorgroup);
#ifdef _WIN32
    pushedbmp.setHInstance(hInst);
#else
#warning port me?
#endif
    pushedbmp = _pushed;
    pushedrgn = new RegionI(pushedbmp.getBitmap());
  }

  if (_hilited) {
    hilitebmp.setHInstanceBitmapColorGroup(_colorgroup);
#ifdef _WIN32
    hilitebmp.setHInstance(hInst);
#else
#warning port me?
#endif
    hilitebmp = _hilited;
    hirgn = new RegionI(hilitebmp.getBitmap());
  }

  if (_activated) {
    activatedbmp.setHInstanceBitmapColorGroup(_colorgroup);
    #ifdef _WIN32
    activatedbmp.setHInstance(hInst);
#else
#warning port me?
#endif
    activatedbmp = _activated;
    activatedrgn = new RegionI(activatedbmp.getBitmap());
  }

  return 1;
}

void ButtonWnd::setUseBaseTexture(int useit) 
{
  if (use_base_texture == useit) return;
  use_base_texture = useit;
  invalidate();
}

void ButtonWnd::setBaseTexture(SkinBitmap *bmp, int x, int y, int tile) 
{
  base_texture = bmp;
  use_base_texture = TRUE;
  tile_base_texture=tile;
  xShift=x;
  yShift=y;
  invalidate();
}

int ButtonWnd::setButtonText(const wchar_t *text, int size) 
{
  textsize = size;
  ASSERT(textsize > 0);
  setName(text);
  invalidate();
  return 1;
}

const wchar_t * ButtonWnd::getButtonText() 
{
  return getName();
}

void ButtonWnd::setTextAlign(TextAlign align) 
{
  alignment = align;
  invalidate();
}

int ButtonWnd::getWidth() 
{
  int addl=0;
  if (checked) {
    addl=checkbmp.getWidth()+3;
  }
  if (rightbmp.getBitmap())
    addl+=rightbmp.getWidth()+3;
  if (normalbmp == NULL) 
	{
    TextInfoCanvas blt(this);
    Wasabi::FontInfo fontInfo;
    fontInfo.pointSize = textsize;
    StringPrintfW lstr(L"j%sj", getNameSafe(L""));
    if (wcschr(lstr, '\t')) lstr.cat(L"   ");
    int a=MAX((blt.getTextWidth(lstr, &fontInfo)*11)/10,8)+addl;
    return a;
  }
  return normalbmp.getWidth()+addl;
}

int ButtonWnd::getHeight() 
{
  int minh=0;
  if (checked>0)
    minh=checkbmp.getHeight();
  if (rightbmp.getBitmap())
    minh=MAX(rightbmp.getHeight(),minh);
  if (normalbmp == NULL) 
	{
    TextInfoCanvas blt(this);
    Wasabi::FontInfo fontInfo;
    fontInfo.pointSize = textsize;
    int r = MAX(MAX((blt.getTextHeight(getName(), &fontInfo)*11)/10,minh),4);
    return r;
  }
  return MAX(normalbmp.getHeight(),minh);
}

void ButtonWnd::enableButton(int _enabled) {
  _enabled = !!_enabled;
  if (enabled != _enabled) invalidate();
  enabled = _enabled;
  onEnable(enabled);
}

int ButtonWnd::getEnabled() const {
  return enabled;
}

int ButtonWnd::onPaint(Canvas *canvas)
{
  PaintBltCanvas paintcanvas;
  SkinBitmap *bmp;
  RECT r;
  int labelxoffs=0;
  int offset = (enabled&&(getPushed()||getDown())) ? 1 : 0;

  if (checked) labelxoffs+=3+checkbmp.getWidth();

  if (canvas == NULL) {
    if (!paintcanvas.beginPaint(this)) return 0;
    canvas = &paintcanvas;
  }
  BUTTONWND_PARENT::onPaint(canvas);

  bmp = normalbmp;
  if (pushedbmp && (getDown() || getPushed())) bmp = pushedbmp;
  else if ((getHilite() || userhilite) && hilitebmp) bmp = hilitebmp;
  else if (activatedbmp && getActivatedButton()) bmp = activatedbmp;

  getClientRect(&r);

  RECT nr = r;
//  RECT fcr = r;
  if (use_base_texture) 
  {
    if (!base_texture)
      renderBaseTexture(canvas, r);
    else {
      RECT cr;
      cr.left = xShift;
      cr.top = yShift;
      cr.right = cr.left + (r.right-r.left);
      cr.bottom = cr.top + (r.bottom-r.top);
      if (tile_base_texture) base_texture->blitTile(canvas, &r,xShift,yShift);
      else base_texture->stretchToRectAlpha(canvas, &cr, &r, getPaintingAlpha());
    }
  } 
  else 
  {
    if (borders) 
    {
      int sysobj = -1;
      if (!enabled)
        sysobj = dsoDisabled;
      else
        sysobj = (getPushed() || getDown()) ? dsoPushed : dsoNormal;
      if (sysobj != -1) canvas->drawSysObject(&nr, sysobj, getPaintingAlpha());
    }
  }

  if (checked>0)
  {
    RECT ar;
    int c=(r.top+r.bottom)/2;
    ar.left=r.left;
    ar.top=c-checkbmp.getHeight()/2;
    ar.bottom=ar.top+checkbmp.getHeight();
    ar.right=r.left+checkbmp.getWidth();
    checkbmp.getBitmap()->stretchToRectAlpha(canvas,&ar,getPaintingAlpha());
  }
  if (rightbmp.getBitmap()) {
    RECT ar;
    int c=(r.top+r.bottom)/2;
    ar.top=c-rightbmp.getHeight()/2;
    ar.bottom=ar.top+rightbmp.getHeight();
    ar.right=r.right;
    ar.left=ar.right-rightbmp.getWidth();
    
    int alpha = getPaintingAlpha();
    if (!getEnabled()) alpha /= 2;
    rightbmp.getBitmap()->stretchToRectAlpha(canvas, &ar, alpha);
  }
  
  if (bmp != NULL) {
    RECT br = r;
    if (center_bitmap) {
      int w = (r.right - r.left) - getWidth() - labelxoffs;
      int h = (r.bottom - r.top) - getHeight();
      br.top = r.top + h/2 + offset;
      br.bottom = br.top + getHeight();
      br.left = r.left + w/2 + labelxoffs + offset;
      br.right = br.left + getWidth() - (rightbmp.getBitmap()?rightbmp.getWidth()+3:0);
    } else {
      br.left += labelxoffs;
      br.right -= (rightbmp.getBitmap()?rightbmp.getWidth()+3:0);
    }
    int alpha2;
    if (!hilitebmp && enabled && autodim) {
      alpha2=128;
      if (getHilite() || userhilite) alpha2=255;
    } else alpha2 = enabled ? 255 : 64;
    bmp->stretchToRectAlpha(canvas, &br,autodim ? (getPaintingAlpha()+alpha2)>>1 : getPaintingAlpha());
  }

  if (getName() != NULL) 
  {
    Wasabi::FontInfo fontInfo;
    fontInfo.opaque = false;
    fontInfo.pointSize = textsize;;
		
		int textw, texth;
		canvas->getTextExtent(getName(), &textw, &texth, &fontInfo);

    if (!enabled)
      fontInfo.color = color_dimmed;
    else if (userhilite)
      fontInfo.color = color_hilite;
    else
      fontInfo.color = color_text;
    int h=(r.bottom-r.top-texth)/2;
    if (h<0) h=0;

    r.left += offset + labelxoffs;
    r.right += offset - (rightbmp.getBitmap()?rightbmp.getWidth()+3:0);
    r.top += offset+h;
    r.bottom = r.top+texth;

    switch (alignment) 
    {
      case TEXTALIGN_CENTER:
        canvas->textOutCentered(&r, getName(), &fontInfo);
      break;

      case TEXTALIGN_RIGHT:
        canvas->textOut(r.right-textw, r.top, textw, texth, getName(), &fontInfo);
      break;

			case TEXTALIGN_LEFT:
        if (!wcsstr(getName(), L"\t")) 
        {
          canvas->textOut(r.left, r.top, r.right-r.left, r.bottom-r.top, getName(), &fontInfo);
        }
        else 
        {
          StringW lstr(getName());
          wchar_t *p=wcsstr(lstr.getNonConstVal(),L"\t");
          if (p) *p++=0;
          else p=L"";
          int tw=canvas->getTextWidth(p, &fontInfo);
          canvas->textOut(r.left, r.top, r.right-r.left-tw, r.bottom-r.top, lstr, &fontInfo);
          canvas->textOut(r.right-tw, r.top, tw, r.bottom-r.top, p, &fontInfo);
        }
			break;

      case TEXTALIGN_LEFT_ELLIPSIS:
        if (!wcsstr(getName(),L"\t")) 
        {
          canvas->textOutEllipsed(r.left, r.top, r.right-r.left, r.bottom-r.top, getName(), &fontInfo);
        }
        else 
        {
          StringW lstr(getName());
          wchar_t *p=wcsstr(lstr.getNonConstVal(),L"\t");
          if (p) *p++=0;
          else p=L"";
          int tw=canvas->getTextWidth(p, &fontInfo);
          canvas->textOutEllipsed(r.left, r.top, r.right-r.left-tw, r.bottom-r.top, lstr, &fontInfo);
          canvas->textOutEllipsed(r.right-tw, r.top, tw, r.bottom-r.top, p, &fontInfo);
        }
      break;
    }
/*
    if (textjustify == BUTTONJUSTIFY_CENTER)
      canvas->textOutCentered(&r, getName());
    else if (textjustify == BUTTONJUSTIFY_LEFT)
    {
      if (!STRSTR(getName(),"\t"))
        canvas->textOutEllipsed(r.left, r.top, r.right-r.left, r.bottom-r.top, getName());
      else
      {
        char *lstr=STRDUP(getName());
        char *p=STRSTR(lstr,"\t");
        if (p) *p++=0;
        else p="";
        int tw=canvas->getTextWidth(p);
        canvas->textOutEllipsed(r.left, r.top, r.right-r.left-tw, r.bottom-r.top, lstr);
        canvas->textOutEllipsed(r.right-tw, r.top, tw, r.bottom-r.top, p);
        FREE(lstr);
      }
    }
*/
  }


/*  if (enabled && gotFocus() && wantFocus()) { // SKIN ME
    fcr.left+=3;
    fcr.right-=3;
    fcr.top+=3;
    fcr.bottom-=3;
    DrawFocusRect(canvas->getHDC(), &fcr);
  }*/

  return 1;
}

void ButtonWnd::onLeftPush(int x, int y) 
{
  notifyParent(ChildNotify::BUTTON_LEFTPUSH);
  invalidate();
}
void ButtonWnd::onRightPush(int x, int y) {
  notifyParent(ChildNotify::BUTTON_RIGHTPUSH);
  invalidate();
}
void ButtonWnd::onLeftDoubleClick(int x, int y) {
  notifyParent(ChildNotify::BUTTON_LEFTDOUBLECLICK);
}
void ButtonWnd::onRightDoubleClick(int x, int y) {
  notifyParent(ChildNotify::BUTTON_RIGHTDOUBLECLICK);
}

void ButtonWnd::setHilite(int h) {
  h = !!h;
  if (userhilite != h) {
    userhilite = h;
    invalidate();
  }
}

int ButtonWnd::getHilite() {
  return userhilite || BUTTONWND_PARENT::getHilite();
}

int ButtonWnd::getPushed() const {
  return userdown || forcedown;
}

void ButtonWnd::setPushed(int p) {
  p = !!p;
  if (userdown != p)
  {
    userdown=p;
    invalidate();
  }
}

int ButtonWnd::onResize() {
  BUTTONWND_PARENT::onResize();
//  invalidate();
  return 1;
}

void ButtonWnd::setCheckBitmap(const wchar_t *checkbm) {
  checkbmp = checkbm;
}

int ButtonWnd::setRightBitmap(const wchar_t *bitmap) {
  rightbmp=bitmap;
  return 1;
}

void ButtonWnd::setActivatedButton(int a) {
  if (activated != a) {
    activated = a;
    invalidate();
    onActivateButton(activated);
  }
}

void ButtonWnd::setActivatedNoCallback(int a) {
  // also force invalidate.
  activated = a;
  invalidate();
}

int ButtonWnd::onActivateButton(int active) {
  return 1;
}

int ButtonWnd::getActivatedButton() {
  return activated;
}

void ButtonWnd::setBorders(int b) {
  b = !!b;
  if (borders != b) {
    borders = b;
    setRectRgn(borders);
    invalidate();
  }
}

void ButtonWnd::setBorderStyle(const wchar_t *style) {
  if (style == NULL) style = L"";
  if (borderstyle && !WCSICMP(borderstyle, style)) return;

//  borderstyle = style;

  using namespace DrawSysObj;
  static struct {
    const wchar_t *style;
    int normal, pushed, disabled;
  } chart[] = {
    { L"button_normal", BUTTON, BUTTON_PUSHED, BUTTON_DISABLED },
    { L"osbutton_normal", OSBUTTON, OSBUTTON_PUSHED, OSBUTTON_DISABLED },
    { L"osbutton_close", OSBUTTON_CLOSE, OSBUTTON_CLOSE_PUSHED, OSBUTTON_CLOSE_DISABLED },
    { L"osbutton_minimize", OSBUTTON_MINIMIZE, OSBUTTON_MINIMIZE_PUSHED, OSBUTTON_MINIMIZE_DISABLED },
    { L"osbutton_maximize", OSBUTTON_MAXIMIZE, OSBUTTON_MAXIMIZE_PUSHED, OSBUTTON_MAXIMIZE_DISABLED },
    { NULL, BUTTON, BUTTON_PUSHED, BUTTON_DISABLED },
  };
  dsoNormal = dsoPushed = dsoDisabled = -1;
  for (int i = 0; ; i++) {
    if (chart[i].style == NULL) return;
    if (!WCSICMP(chart[i].style, style)) {
      borderstyle = chart[i].style;
      dsoNormal = chart[i].normal;
      dsoPushed = chart[i].pushed;
      dsoDisabled = chart[i].disabled;
    }
  }

  invalidate();
}

const wchar_t *ButtonWnd::getBorderStyle() {
  return borderstyle;
}

void ButtonWnd::setInactiveAlpha(int a) {
  inactivealpha=a;
}

void ButtonWnd::setActiveAlpha(int a) {
  activealpha=a;
}

int ButtonWnd::onGetFocus() {
  BUTTONWND_PARENT::onGetFocus();
//  invalidate();
  return 1;
}

int ButtonWnd::onKillFocus() {
  BUTTONWND_PARENT::onKillFocus();
//  invalidate();
  return 1;
}

void ButtonWnd::setColors(const wchar_t *text, const wchar_t *hilite, const wchar_t *dimmed) {
  color_text=text;
  color_hilite=hilite;
  color_dimmed=dimmed;
  invalidate();
}

void ButtonWnd::setTextColor(const wchar_t *text) {
  color_text=text;
  invalidate();
}

void ButtonWnd::setTextHoverColor(const wchar_t *hilite) {
  color_hilite=hilite;
  invalidate();
}

void ButtonWnd::setTextDimmedColor(const wchar_t *dimmed) {
  color_dimmed=dimmed;
  invalidate();
}

int ButtonWnd::onEnable(int is) {
  return BUTTONWND_PARENT::onEnable(is);
}

int ButtonWnd::getPreferences(int what) {
  switch (what) {
    case SUGGESTED_W: {
      if (!normalBmpStr.isempty()) return normalbmp.getWidth();
      return getWidth();
    }
    case SUGGESTED_H: {
      if (!normalBmpStr.isempty()) return normalbmp.getHeight();
      return getHeight();
    }
  }
  return BUTTONWND_PARENT::getPreferences(what);
}

int ButtonWnd::onInit() {
  int r = BUTTONWND_PARENT::onInit();
  currgn = normalrgn;
  return r;
}

int ButtonWnd::onChar(unsigned int c) 
{
  switch (c) {
#ifdef _WIN32
    case VK_RETURN:
    case VK_SPACE:
      postDeferredCallback(DEFEREDCB_DOWN, 0, 0);
      postDeferredCallback(DEFEREDCB_UP, 0, 250);
      //return BUTTONWND_PARENT::onChar(c);
      break;
#else
#warning port me
#endif
    default:
      return BUTTONWND_PARENT::onChar(c);
  }  
  return 1;
}


int ButtonWnd::onDeferredCallback(intptr_t p1, intptr_t p2) {
  switch (p1) {
    case DEFEREDCB_DOWN:
      forcedown = 1;
      invalidate();
      break;
     case DEFEREDCB_UP:
      forcedown = 0;      
      invalidate();
      RECT r;
      getClientRect(&r);
      onLeftPush(r.left+(r.right-r.left)/2, r.top+(r.bottom-r.top)/2);
    default:
      return BUTTONWND_PARENT::onDeferredCallback(p1, p2);
  }
  return 1;
}


