#include "precomp.h"

#include "shadowwnd.h"

#include "../bfc/canvas.h"
#include "../bfc/region.h"

enum { TARGET };

char XuiShadowWndParams[][]=
{
	"TARGET",
};
XuiShadowWnd::XuiShadowWnd() {
  myxuihandle = newXuiHandle();
  addParam(myxuihandle, XuiShadowWndParams[0], TARGET, XUI_ATTRIBUTE_REQUIRED);
  group = NULL;
  bltcanvas = NULL;
  c_w = c_h = 0;
  in_paint = 0;
}

XuiShadowWnd::~XuiShadowWnd() {
  delete bltcanvas;
}

int XuiShadowWnd::onInit() {
  XUISHADOWWND_PARENT::onInit();

DebugString("on iniiit");

  attachToGroup();
setTimer(10, 50);

  return 1;
}

void XuiShadowWnd::timerclient_timerCallback(int id) {
  if (id == 10) {
    if (group == NULL) attachToGroup();
    delete bltcanvas;
    RECT r; group->getClientRect(&r);
    bltcanvas = new BltCanvas(r.right - r.left, r.bottom - r.top);
    in_paint++;
    group->paint(bltcanvas);
MEMFILL<ARGB32>((unsigned long *)bltcanvas->getBits(), 0xffffffff, (r.right - r.left) * 20);
    in_paint--;
    invalidate();
  } else
    XUISHADOWWND_PARENT::timerclient_timerCallback(id);
}

int XuiShadowWnd::onPaint(Canvas *canvas) {
#if 0
if (group == NULL) attachToGroup();
if (group == NULL) { DebugString("groupNull"); }
if (group == NULL) return 0;

#endif
DebugString("begin painting");

if (in_paint++) {
//    RECT cr = clientRect();
//    canvas->fillRect(&cr, RGB(255,0,255));
//MEMFILL<ARGB32>((unsigned long *)canvas->getBits(), 0xffffffff, (cr.right - cr.left) * 20);
DebugString("filla!");
} else {
#if 0
    RECT cr;
    group->getClientRect(&cr);
    SkinBitmap *bm
bltcanvas->blit(0, 0, 
BltCanvas c(cr.right - cr.left, cr.bottom - cr.top);
group->paint(&c);
#if 0
c.pushPen(0,255,0);
c.lineDraw(0, 0, cr.right, cr.bottom);
/c.popPen();
#endif
MEMFILL<ARGB32>((unsigned long *)c.getBits(), 0xffffffff, (cr.right - cr.left) * 20);
c.blit(0, 0, canvas, 0, 0, cr.right - cr.left, cr.bottom - cr.top);

DebugString("get from group!");
#endif
  if (bltcanvas != NULL) {
    SkinBitmap *bm = bltcanvas->getSkinBitmap();
    bm->stretchToRectAlpha(canvas, &clientRect(), getPaintingAlpha());
DebugString("bleet!");
  }
}
in_paint--;
  return 1;
}

int XuiShadowWnd::setXuiParam(int xuihandle, int xmlattributeid, const wchar_t *xmlattributename, const wchar_t *value) {
  if (xuihandle != myxuihandle)
    return XUISHADOWWND_PARENT::setXuiParam(xuihandle, xmlattributeid, xmlattributename, value);
  switch (xmlattributeid) {
    case TARGET:
      targetname = value;
DebugString("set target %s", value);
      if (isPostOnInit()) attachToGroup();
    break;
    default: return 0;
  }
  return 1;
}

void XuiShadowWnd::attachToGroup() {
  if (targetname.isempty()) return;
  group = findWindow(targetname);
  if (group == NULL) return;
  monitorWindow(group);
DebugString("attached to group rw %d", group);

  delete bltcanvas; bltcanvas = NULL;
}

void XuiShadowWnd::onAfterPaint(PaintCallbackInfo *info) {
DebugString("after paint");
#if 0
  RECT ncr;
  group->getNonClientRect(&ncr);
  c_w = ncr.right - ncr.left;
  c_h = ncr.bottom - ncr.top;

DebugString("w %d h %d", c_w, c_h);

  delete bltcanvas; bltcanvas = NULL;
  if (c_w != 0 && c_h != 0) bltcanvas = new BltCanvas(c_w, c_h);

  Canvas *c = info->getCanvas();
  api_region *r = info->getRegion();
  // blit what changed
  RegionI saved;
  c->getClipRgn(&saved);
  bltcanvas->selectClipRgn(r);
  c->blit(0, 0, bltcanvas, 0, 0, c_w, c_h);
  c->selectClipRgn(&saved);

  invalidate();
#endif
}

void XuiShadowWnd::onInvalidation(PaintCallbackInfo *info) {
//  invalidate();
DebugString("got invalidate");
}

void XuiShadowWnd::onWindowDeleted(api_window *w) {
  if (w == group) group = NULL;
}
