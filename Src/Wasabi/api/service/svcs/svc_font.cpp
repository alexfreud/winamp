#include <precomp.h>
#include <api/font/svc_fonti.h>

#define CBCLASS svc_fontI
START_DISPATCH
  VCB(TEXTOUT, textOut);
  VCB(TEXTOUT2, textOut2);
  VCB(TEXTOUTELLIPSED, textOutEllipsed);
  VCB(TEXTOUTWRAPPED, textOutWrapped);
  VCB(TEXTOUTWRAPPEDPATHED, textOutWrappedPathed);
  VCB(TEXTOUTCENTERED, textOutCentered);
  CB(GETTEXTWIDTH, getTextWidth);
  CB(GETTEXTHEIGHT, getTextHeight);
  CB(GETTEXTHEIGHT2, getTextHeight2);
  VCB(GETTEXTEXTENT, getTextExtent);
  VCB(SETFONTID, setFontId);
  CB(GETFONTID, getFontId);
  CB(GETFACENAME_, getFaceName);
  CB(ISBITMAP, isBitmap);
  CB(GETSCRIPTID, getScriptId);
  VCB(SETSCRIPTID, setScriptId);
  VCB(SETFONTFACE, setFontFace);
  CB(ADDFONTRESOURCE, addFontResource);
  CB(ADDFONTRESOURCE2, addFontResource2);
  CB(GETFONTSVCNAME, getFontSvcName);
END_DISPATCH
#undef CBCLASS
