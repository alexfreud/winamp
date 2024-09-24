#include "precomp.h"

#include "textbar.h"

#include <bfc/ifc_canvas.h>
#include <bfc/string/string.h>
#include <bfc/skinclr.h>
#include <bfc/autobitmap.h>
#include <common/checkwnd.h>

static SkinColor bgcolor("wasabi.textBar.background", "Text backgrounds");
static SkinColor fgcolor("wasabi.textBar.text");

TextBar::TextBar() {
  size = 16;
  usebt = 0;
  alignment = TEXTALIGN_LEFT;  //set default alignment
  checkwndtarget = NULL;

  textshadowed = 1; // display a shadow of the text in bgcolor.  default: on
  textoutlined = 0; // draw an outline of the text in bgcolor.  default: off
  drawbox = 0;      // draw a box of bgcolor the size of the boundsrect.  default: off

//  bgbitmap = "studio.textBar.background";
}

int TextBar::onLeftButtonDown(int x, int y) {
  TEXTBAR_PARENT::onLeftButtonDown(x, y);
  if (checkwndtarget) checkwndtarget->toggle();
  return 1;
}

void TextBar::setUseBaseTexture(int u) {
  usebt = u;
  invalidate();
}

int TextBar::onPaint(Canvas *canvas) {
  RECT r;

  PaintCanvas paintcanvas;
  if (canvas == NULL) {
    if (!paintcanvas.beginPaint(this)) return 0;
    canvas = &paintcanvas;
  }
  TEXTBAR_PARENT::onPaint(canvas);

  getClientRect(&r);

  if (!usebt) {
    if (drawbox) {
      canvas->fillRect(&r, bgcolor);
    }
/*
    if (bgbitmap.getBitmap()->isInvalid())
      canvas->fillRect(&r, bgcolor);
    else {
      RECT br;
      br.left = 0;
      br.top = 0;
      br.right = bgbitmap.getWidth();
      br.bottom = bgbitmap.getHeight();
      bgbitmap.getBitmap()->blitToRect(canvas, &br, &r, 255);
    }
*/
  } else
    renderBaseTexture(canvas, r);

  const char *name = getName();

  if (name != NULL) {
    canvas->setTextOpaque(FALSE);
    canvas->pushTextSize(size);
    int w, h;
    canvas->getTextExtent(name, &w, &h);
    int y = (r.bottom-r.top - h) / 2;
//    int x = centered ? (r.right-r.left - w) / 2 : TEXTBAR_LEFTMARGIN;  //teh old code

    int x = 0;
    switch (alignment) {
      default:
      case TEXTALIGN_LEFT: x = TEXTBAR_LEFTMARGIN; break;
      case TEXTALIGN_CENTER: x = (r.right-r.left - w) / 2; break;
      case TEXTALIGN_RIGHT: x = (r.right-r.left - w); break;
    }

    if (!drawbox && textoutlined) {
      canvas->setTextColor(bgcolor);
      canvas->textOut(r.left+x+1, r.top+y+1, getName());
      canvas->setTextColor(bgcolor);
      canvas->textOut(r.left+x+1, r.top+y-1, getName());
      canvas->setTextColor(bgcolor);
      canvas->textOut(r.left+x-1, r.top+y+1, getName());
      canvas->setTextColor(bgcolor);
      canvas->textOut(r.left+x-1, r.top+y-1, getName());
    } else if (!drawbox && textshadowed) {
      canvas->setTextColor(bgcolor);
      canvas->textOut(r.left+x+1, r.top+y+1, getName());
    }
    canvas->setTextColor(fgcolor);
    canvas->textOut(r.left+x, r.top+y, getName());
    canvas->popTextSize();
  }
  return 1;
}

int TextBar::setTextSize(int newsize) {
  if (newsize < 1 || newsize > 72) return 0;
  size = newsize;
  invalidate();
  return 1;
}

int TextBar::setInt(int i) {
  setName(StringPrintf(i));
  invalidate();
  return 1;
}

void TextBar::onSetName() {
  TEXTBAR_PARENT::onSetName();
  invalidate();
}

int TextBar::getTextWidth() {
  if (!getName()) return 0;
  BltCanvas *c = new BltCanvas(10, 10);
  c->pushTextSize(size);
  int r = c->getTextWidth(getName());
  c->popTextSize();
  delete c;
  return r+4;
}

int TextBar::getTextHeight() {
  return size;
}

void TextBar::setAlign(TextAlign align) {
  if (alignment != align) {
    alignment = align;
    invalidate();
  }
}

TextAlign TextBar::getAlign() {
  return alignment;
}
