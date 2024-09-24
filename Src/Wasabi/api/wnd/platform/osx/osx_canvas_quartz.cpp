#include <bfc/platform/types.h>
#include <Carbon/Carbon.h>
#include <tataki/canvas/canvas.h>
#include <api/wnd/basewnd.h>
#include <tataki/region/api_region.h>



/* various functions that might help out

for drawSysObject:
HIThemeDrawButton
HIThemeDrawTitleBarWidget for minimize, maximize, exit
*/

inline float QuartzBlue(RGB32 color)
{
  unsigned char *pixel = (unsigned char *)&color;
  return pixel[0] / 255.f;
}

inline float QuartzGreen(RGB32 color)
{
  unsigned char *pixel = (unsigned char *)&color;
  return pixel[1] / 255.f;

}

inline float QuartzRed(RGB32 color)
{
  unsigned char *pixel = (unsigned char *)&color;
  return pixel[2] / 255.f;

}

inline float QuartzAlpha(RGB32 color)
{
  unsigned char *pixel = (unsigned char *)&color;
  return pixel[3] / 255.f;

}

Canvas::Canvas(CGrafPtr _context)
{
  
}

void Canvas::fillRect(const RECT *r, ARGB32 color)
{
  CGContextSetRGBFillColor(context, 
                           QuartzRed(color), // red
                           QuartzGreen(color), // green
                           QuartzBlue(color), // blue
                           QuartzAlpha(color) // alpha
                           );

  HIRect rect = HIRectFromRECT(r);
  CGContextFillRect(context, rect);
}

void Canvas::fillRgn(api_region *r, ARGB32 color)
{
  CGContextSetRGBFillColor(context, 
                           QuartzRed(color), // red
                           QuartzGreen(color), // green
                           QuartzBlue(color), // blue
                           QuartzAlpha(color) // alpha
                           );
  
  HIShapeRef shape = r->getOSHandle();
  HIShapeReplacePathInCGContext(shape, context);
  CGContextFillPath(context);
}

void Canvas::blit(int srcx, int srcy, Canvas *dest, int dstx, int dsty, int dstw, int dsth)
{
  // clip dest
  // Create CGImage from context
  // CGContextDrawImage
}

HDC Canvas::getHDC()
{
  return context;
}

void Canvas::selectClipRgn(api_region *r)
{
  if (r)
  {
    HIShapeRef shape = r->getOSHandle();
    HIShapeReplacePathInCGContext(shape, context);
    CGContextClip(context);
  }
  else
  {
    CGContextClipToRect(context, CGRectInfinite);
  }
}

void Canvas::stretchblit(int srcx, int srcy, int srcw, int srch, Canvas *dest, int dstx, int dsty, int dstw, int dsth)
{
  // Create CGImage from context
  // CGContextDrawImage
}

void Canvas::textOut(int x, int y, const wchar_t *txt, const Wasabi::FontInfo *fontInfo)
{
  // TODO: turn this code into a svc_fontI, and use api_font here instead
  size_t len = wcslen(txt);
  UniChar *unistr = (UniChar *)malloc((len + 1) * sizeof(UniChar));
  UniChar *copy = unistr;
  while (*txt)
    *copy++=*txt++;
  *copy=0;
  
  ATSUStyle style;
  ATSUCreateStyle(&style);
  
  CGContextSaveGState(context);
  CGContextSetRGBFillColor(context, 
                           QuartzRed(fontInfo->color), // red
                           QuartzGreen(fontInfo->color), // green
                           QuartzBlue(fontInfo->color), // blue
                           QuartzAlpha(fontInfo->color) // alpha
                           );
  
  ATSUTextLayout layout;
  ATSUCreateTextLayout(&layout);
  
  ATSUSetTextPointerLocation(layout, unistr, kATSUFromTextBeginning, kATSUToTextEnd, len);
  
  ATSUSetRunStyle(layout, style, kATSUFromTextBeginning, kATSUToTextEnd);
  
  Rect imageRect;
	ATSUMeasureTextImage(layout, kATSUFromTextBeginning, kATSUToTextEnd, 0, 0, &imageRect);
  y-=(imageRect.bottom - imageRect.top);
  CGContextScaleCTM(context, 1.0, -1.0);

  ATSUAttributeTag tags[] = {kATSUCGContextTag};
  ATSUAttributeValuePtr values[] = {&context};
  ByteCount sizes[] = {sizeof(CGContextRef)};
  ATSUSetLayoutControls(layout, 1, tags, sizes, values);
  ATSUDrawText(layout, kATSUFromTextBeginning, kATSUToTextEnd, FloatToFixed(x), FloatToFixed(y));
  ATSUDisposeTextLayout(layout);
  ATSUDisposeStyle(style);
  CGContextRestoreGState(context);
  free(unistr);
}

void Canvas::drawSysObject(const RECT *r, int sysobj, int alpha)
{
#warning TODO
  using namespace DrawSysObj;
  switch(sysobj)
  {
    case OSBUTTON:
    {
      HIRect buttonRect = HIRectFromRECT(r);
      HIThemeButtonDrawInfo buttonDrawInfo;
      buttonDrawInfo.version=0;
      buttonDrawInfo.state = kThemeStateActive;
      buttonDrawInfo.kind = kThemePushButton;
      buttonDrawInfo.value = kThemeButtonOn;
      buttonDrawInfo.adornment = kThemeAdornmentNone;
        buttonDrawInfo.animation.time.start = 0;
        buttonDrawInfo.animation.time.current=0;
        HIThemeDrawButton(&buttonRect, &buttonDrawInfo, context, /*kHIThemeOrientationNormal*/kHIThemeOrientationInverted, 0);
                  }
      break;
    case OSBUTTON_PUSHED:
    {
      HIRect buttonRect = HIRectFromRECT(r);
      HIThemeButtonDrawInfo buttonDrawInfo;
      buttonDrawInfo.version=0;
      buttonDrawInfo.state = kThemeStatePressed;
      buttonDrawInfo.kind = kThemePushButton;
      buttonDrawInfo.value = kThemeButtonOn;
      buttonDrawInfo.adornment = kThemeAdornmentNone;
      buttonDrawInfo.animation.time.start = 0;
      buttonDrawInfo.animation.time.current=0;
      HIThemeDrawButton(&buttonRect, &buttonDrawInfo, context, /*kHIThemeOrientationNormal*/kHIThemeOrientationInverted, 0);
    }
      break;
    case OSBUTTON_DISABLED:
    {
      HIRect buttonRect = HIRectFromRECT(r);
      HIThemeButtonDrawInfo buttonDrawInfo;
      buttonDrawInfo.version=0;
      buttonDrawInfo.state = kThemeStateInactive;
      buttonDrawInfo.kind = kThemePushButton;
      buttonDrawInfo.value = kThemeButtonOn;
      buttonDrawInfo.adornment = kThemeAdornmentNone;
      buttonDrawInfo.animation.time.start = 0;
      buttonDrawInfo.animation.time.current=0;
      HIThemeDrawButton(&buttonRect, &buttonDrawInfo, context, /*kHIThemeOrientationNormal*/kHIThemeOrientationInverted, 0);
    }
      break;
  }
}

void Canvas::getTextExtent(const wchar_t *text, int *w, int *h, const Wasabi::FontInfo *fontInfo)
{
  // TODO: turn this code into a svc_fontI, and use api_font here instead
  size_t len = wcslen(text);
  UniChar *unistr = (UniChar *)malloc((len + 1) * sizeof(UniChar));
  UniChar *copy = unistr;
  while (*text)
    *copy++=*text++;
  *copy=0;
  
  ATSUStyle style;
  ATSUCreateStyle(&style);
    
  ATSUTextLayout layout;
  ATSUCreateTextLayout(&layout);
  
  ATSUSetTextPointerLocation(layout, unistr, kATSUFromTextBeginning, kATSUToTextEnd, len);
  
  ATSUSetRunStyle(layout, style, kATSUFromTextBeginning, kATSUToTextEnd);
  
  Rect imageRect;
	ATSUMeasureTextImage(layout, kATSUFromTextBeginning, kATSUToTextEnd, 0, 0, &imageRect);
  *h=(imageRect.bottom - imageRect.top);
  *w = (imageRect.right - imageRect.left);

  ATSUDisposeTextLayout(layout);
  ATSUDisposeStyle(style);
  free(unistr); 
}

void Canvas::textOutCentered(RECT *r, const wchar_t *txt, const Wasabi::FontInfo *fontInfo)
{
  textOut(r->left, r->top, txt, fontInfo);
}


#define CBCLASS Canvas
START_DISPATCH;
CB(GETHDC, getHDC);
END_DISPATCH;
#undef CBCLASS

BaseCloneCanvas::BaseCloneCanvas(ifc_canvas *cloner)
{
	if (cloner != NULL) clone(cloner);
}

int BaseCloneCanvas::clone(ifc_canvas *cloner)
{
	ASSERTPR(context == NULL, "can't clone twice");
  context = cloner->getHDC();
  CGContextRetain(context);
//	bits = cloner->getBits();
//	cloner->getDim(&width, &height, &pitch);
	//  srcwnd = cloner->getBaseWnd();
//	cloner->getOffsets(&xoffset, &yoffset);
//	setTextFont(cloner->getTextFont());
//	setTextSize(cloner->getTextSize());
//	setTextBold(cloner->getTextBold());
//	setTextOpaque(cloner->getTextOpaque());
//	setTextUnderline(cloner->getTextUnderline());
//	setTextItalic(cloner->getTextItalic());
//	setTextAlign(cloner->getTextAlign());
//	setTextColor(cloner->getTextColor());
//	setTextBkColor(cloner->getTextBkColor());
	return (context != NULL);
}

BaseCloneCanvas::~BaseCloneCanvas()
{
  CGContextRelease(context);
	context = NULL;
}

