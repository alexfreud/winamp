#include "precomp.h"
// ============================================================================================================================================================
// Font abstract class + statics to install TT fonts and Bitmap fonts
// ============================================================================================================================================================

#include <api/font/font.h>
#include <api/font/bitmapfont.h>
#include <bfc/parse/pathparse.h>

#ifdef WASABI_COMPILE_SKIN
  #include <api/skin/skin.h>
  #include <api/skin/skinparse.h>
#endif

#include <tataki/canvas/ifc_canvas.h>
#include <api/wnd/fontdef.h>

#ifdef WASABI_COMPILE_FONT
#include <api/service/svcs/svc_font.h>
//#include "services/svc_fontmaker.h"
#endif

#ifdef WASABI_API_CONFIG
#include <api/config/options.h>
#include <api/config/items/attrint.h>
#include <api/config/items/attrstr.h>
#include <api/config/items/attrbool.h>
#endif
#include <api/memmgr/api_memmgr.h>
#include <api/font/FontSvcEnum.h>

extern _bool cfg_options_usefontmapper;
extern _string cfg_options_ttfoverridefont;
extern _int cfg_options_defaultfontscale;

PtrList<svc_font> Font::fontlist;
PtrList<FontDef> Font::fontdefs;

void Font::init() 
{
#ifdef WASABI_API_CONFIG
  Wasabi::Std::setDefaultFont(cfg_options_defaultfont.getValue());
  Wasabi::Std::setDefaultFontScale(cfg_options_defaultfontscale.getValueAsInt());
#endif
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void Font::dispatchTextOut(ifc_canvas *c, int style, int x, int y, int w, int h, const wchar_t *txt) 
{
  int isoverride = 0;
  if (WASABI_API_APP->main_isShuttingDown()) return;

  int size = c->getTextSize();

  svc_font *f = requestSkinFont(c->getTextFont(), &size);

  ASSERT(f != NULL);

  // After we get the font we want, check to see if it is bitmap.
  // If bitmap fonts are disallowed, use the truetype override font.
  if (f->isBitmap() && useTrueTypeOverride(txt)) 
	{
    int gotdefault=0;
    svc_font *ttFont = requestSkinFont(getTrueTypeOverride(), &size, &gotdefault);
    if (ttFont != NULL) 
		{
      if (!gotdefault) 
        isoverride = 1;
      f = ttFont;
    }
  }

  if (isoverride) 
	{
    double f = (double)getTrueTypeOverrideScale() / 100.0f;
    size = (int)(size*f);
  }
  int bold = c->getTextBold();
  int opaque = c->getTextOpaque();
  int underline = c->getTextUnderline();
  int italic = c->getTextItalic();
  int align = c->getTextAlign();
  int antialiased = c->getTextAntialias();
  ARGB32 color = c->getTextColor();
  ARGB32 bkcolor = c->getTextBkColor();
  int xoffset=0, yoffset=0;
  c->getOffsets(&xoffset, &yoffset);
/*  if (!f->isBitmap() && _intVal(Main::enumRootCfgItem(0), "Force antialias on all TTF"))
    antialiased = 1;*/
  switch (style) 
	{
    case WA_FONT_TEXTOUT_NORMAL:
      f->textOut(c, x, y, txt, size, bold, opaque, underline, italic, color, bkcolor, xoffset, yoffset, antialiased);
      break;
    case WA_FONT_TEXTOUT_RECT:
      f->textOut(c, x, y, w, h, txt, size, bold, opaque, underline, italic, align, color, bkcolor, xoffset, yoffset, antialiased);
      break;
    case WA_FONT_TEXTOUT_ELLIPSED:
      f->textOutEllipsed(c, x, y, w, h, txt, size, bold, opaque, underline, italic, align, color, bkcolor, xoffset, yoffset, antialiased);
      break;
    case WA_FONT_TEXTOUT_WRAPPED:
      f->textOutWrapped(c, x, y, w, h, txt, size, bold, opaque, underline, italic, align, color, bkcolor, xoffset, yoffset, antialiased);
      break;
    case WA_FONT_TEXTOUT_WRAPPEDPATHED:
      f->textOutWrappedPathed(c, x, y, w, txt, size, bold, opaque, underline, italic, align, color, bkcolor, xoffset, yoffset, antialiased);
      break;
    case WA_FONT_TEXTOUT_CENTERED:
      RECT r;
      r.left = x;
      r.top = y;
      r.right = w;
      r.bottom = h;
      f->textOutCentered(c, &r, txt, size, bold, opaque, underline, italic, align, color, bkcolor, xoffset, yoffset, antialiased);
      break;
  }
}


// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int Font::dispatchGetInfo(ifc_canvas *c, const wchar_t *font, int infoid, const wchar_t *txt, int *w, int *h) 
{
  int isoverride = 0;
  if (WASABI_API_APP->main_isShuttingDown()) return 0;
  // mig: Let's not crash if we want to see how big a NULL pointer is.
  if (txt == NULL) {
    if ( infoid == WA_FONT_GETINFO_WIDTHHEIGHT ) {
      if (w != NULL) {
        *w = 0;
      }
      if (h != NULL) {
        *h = 0;
      }
    }
    return 0;
  }

  int size = c->getTextSize();

  svc_font *f = requestSkinFont(font, &size);
  ASSERT(f != NULL);

  // After we get the font we want, check to see if it is bitmap.
  // If bitmap fonts are disallowed, use the truetype override font.
  if (f->isBitmap() && useTrueTypeOverride(txt)) 
	{
    int gotdefault = 0;
    svc_font *ttFont = requestSkinFont(getTrueTypeOverride(), &size, &gotdefault);
    if (ttFont != NULL) 
		{
      if (!gotdefault) 
        isoverride = 1;
      f = ttFont;
    }
  }

  if (isoverride) {
    double f = (double)getTrueTypeOverrideScale() / 100.0f;
    size = (int)(size*f);
  }
  int bold = c->getTextBold();
  int underline = c->getTextUnderline();
  int italic = c->getTextItalic();
  int antialiased = c->getTextAntialias();
  switch (infoid) {
    case WA_FONT_GETINFO_WIDTH:
      return f->getTextWidth(c, txt, size, bold, underline, italic, antialiased);
    case WA_FONT_GETINFO_HEIGHT:
      return f->getTextHeight(c, txt, size, bold, underline, italic, antialiased);
    case WA_FONT_GETINFO_WIDTHHEIGHT:
      f->getTextExtent(c, txt, w, h, size, bold, underline, italic, antialiased);
      return 0;
  }
  return 0;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// Install a truetype font from its filename and associate a script_id to it
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
svc_font *Font::installTrueTypeFont(const wchar_t *filename, const wchar_t *path, const wchar_t *id, int scriptid, int allowmapping, int isttfreload) {
 
  if (!isttfreload) 
	{
    FontDef *fd = new FontDef;
    fd->filename = filename;
    fd->path = path;
    fd->id = id;
    fd->scriptid = scriptid;
    fd->isbitmap = 0;
    fd->allowmapping = allowmapping;
    fontdefs.addItem(fd);
  }

	StringW file;
  
  OSFILETYPE ff=OPEN_FAILED;
  if (wcschr(filename, ':'))
    ff = WFOPEN(filename, WF_READONLY_BINARY);

  if (ff == OPEN_FAILED) 
	{
		file = StringPathCombine(path, filename);
    ff = WFOPEN(file, WF_READONLY_BINARY);
  }
#ifdef WASABI_COMPILE_SKIN
  if (ff == OPEN_FAILED) 
	{
		file = StringPathCombine(SkinParser::getXmlRootPath(), filename);
    ff = WFOPEN(file, WF_READONLY_BINARY);
    if (ff == OPEN_FAILED) 
		{
			file = StringPathCombine(Skin::getDefaultSkinPath(), filename);
      ff = WFOPEN(file, WF_READONLY_BINARY); 
      if (ff == OPEN_FAILED) 
			{
        DebugString("Font not found %s\n", filename);
      // todo: do something if still not found
      }
    }
  }
#endif
  if (ff == OPEN_FAILED) {
    DebugString("Could not install font %s\n", filename);
    return 0;
  }

  StringW fs = filename;
  wchar_t *p = wcschr(fs.getNonConstVal(), '.');
  if (p)
		*p = 0;
  PathParserW pp(fs); 
	fs = pp.getLastString();

  svc_font *f = newTrueTypeFont();
  if (f && f->addFontResource( ff, fs) ) 
	{
    f->setFontId(id);
    f->setScriptId(scriptid);
    fontlist.addItem(f);
  } else {
    DebugString("font.cpp ====== CAN'T LOAD FONT FILE.\n");
  }

  FCLOSE(ff);
  return f;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// Uninstall all installed fonts
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void Font::uninstallAll(int ttfreload) {
  int i;
  // delete all by hand
  for (i = 0; i < fontlist.getNumItems(); i++) {
    svc_font *f = fontlist.enumItem(i);
    if (ttfreload && f->isBitmap()) continue;
    deleteFont(f);
    fontlist.removeByPos(i);
    i--;
  }
  if (!ttfreload) fontdefs.deleteAll();
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// Uninstall by scriptid
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void Font::uninstallByScriptId(int scriptid) {
  for (int i=0;i<fontlist.getNumItems();i++) {
    svc_font *f = fontlist.enumItem(i);
    if (f->getScriptId() == scriptid) {
      fontlist.removeByPos(i);
      deleteFont(f);
      i--;
    }
  }
  for (int i=0;i<fontdefs.getNumItems();i++) {
    FontDef *fd = fontdefs.enumItem(i);
    if (fd->scriptid == scriptid) {
      fontdefs.removeByPos(i);
      delete fd;
      i--;
    }
  }
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// Install a bitmap font and associates a script_id to it
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void Font::installBitmapFont(const wchar_t *filename, const wchar_t *path, const wchar_t *id, int cw, int ch, int hs, int vs, int scriptid, int allowmapping) 
{
  FontDef *fd = new FontDef;
  fd->filename = filename;
  fd->path = path;
  fd->id = id;
  fd->scriptid = scriptid;
  fd->isbitmap = 1;
  fd->allowmapping = allowmapping;
  fontdefs.addItem(fd);

  BitmapFont *f = new BitmapFont;
  f->setFontBitmap(filename, path); 
  f->setFontId(id);
  f->setFontMetrics(cw, ch, hs, vs);
  f->setScriptId(scriptid);
  fontlist.addItem(f);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// Requests a Font* from its id
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
svc_font *Font::requestSkinFont(const wchar_t *id, int *size, int *gotdefault) 
{
  if (gotdefault) *gotdefault = 0;
  int oldsize = size ? *size : -1;
  const wchar_t *mapped_id = getFontMapping(id, size);
  if (mapped_id != NULL) 
		id = mapped_id;

  // First try to get a font by that id
  foreach_reverse(fontlist)
    const wchar_t *thisid = fontlist.getfor()->getFontId();
    if (thisid && !WCSICMP(thisid, id))
      return fontlist.getfor();
  endfor
  // if it wasnt found, try to load a wa-installed ttfont with this face name
  foreach_reverse(fontlist)
    const wchar_t *facename=fontlist.getfor()->getFaceName();
    if (facename && !WCSICMP(facename, id)) return fontlist.getfor();
  endfor

  // not found, try to reload it front the list of fonts defined by the skin
  foreach(fontdefs)
    FontDef *fd = fontdefs.getfor();
    if (!WCSICMP(fd->id, id))
		{
      if (!fd->isbitmap) 
			{
        svc_font *f = installTrueTypeFont(fd->filename, fd->path, fd->id, fd->scriptid, fd->allowmapping, 1);
        if (f) return f;
      }
    }
  endfor;

  /*
  for (i=fontlist.getNumItems()-1;i>=0;i--) {
    const char *thisid = fontlist.enumItem(i)->getFontId();
    if (thisid && STRCASEEQL(thisid, "wasabi.font.ttf.default" ))
      return fontlist.enumItem(i);
  }
  */

  // not found ? try to find it in the windows fonts directory
  {
    wchar_t *fp = WMALLOC(WA_MAX_PATH);
    Wasabi::Std::getFontPath(WA_MAX_PATH, fp);
    StringW file;
		file.own(fp);
//  FREE(fp); // benski> no need because we now own it
		file.AppendPath(StringPrintfW(L"%s%s", id, WCSCASESTR(id, L".ttf") == NULL ? L".ttf":L""));

    if (!WACCESS(file, 0)) 
		{
      svc_font *f = newTrueTypeFont();
      f->setFontFace(id);
      f->setFontId(id);
      OSFILETYPE ff = WFOPEN(file, WF_READONLY_BINARY);
      if (ff != OPEN_FAILED) 
			{
        if (f->addFontResource(ff, id)) 
				{
          DebugStringW(L"font.cpp ====== FONT FOR ID=%s NOT FOUND.  USING WIN FONT FILE:\n%s\n", id, file.getValue());
          fontlist.addItem(f);
        }
      } else {
        DebugStringW(L"font.cpp ====== FONT FOR ID=%s NOT FOUND.  CANNOT OPEN WIN FONT FILE:\n%s\n", id, file.getValue());
        delete f;
        f = NULL;
      }
      return f;
    }
  }

  // not found ? ask the Std:: interface for the folder and the 
  // default fontname (ie: one you know will always be in the OS)
  svc_font *f = newTrueTypeFont();
  if (f) {
    if (gotdefault) *gotdefault = 1;
    if (oldsize != -1 && size) {
      *size = oldsize;
      double f = (double)Wasabi::Std::getDefaultFontScale() / 100.0;
      *size = (int)(*size*f);
    }
    // Query Std:: and build the path to the default font file.
    wchar_t *fontPath = WMALLOC(WA_MAX_PATH);
    Wasabi::Std::getFontPath(WA_MAX_PATH, fontPath);
    wchar_t fontFile[WA_MAX_PATH] = {0};
    Wasabi::Std::getDefaultFont(WA_MAX_PATH, fontFile);
		StringW defaultFont;
		defaultFont.own(fontPath);
		defaultFont.AppendPath(fontFile);
//    FREE(fontFile);
    StringW fs = defaultFont;
    wchar_t *p = wcschr(fs.getNonConstVal(), '.');
    if (p) *p = 0;
    PathParserW pp(fs); 
		fs = pp.getLastString();
    f->setFontFace(fs);
    f->setFontId(id);
    // Open it and load it as the font resource.
    OSFILETYPE ff = WFOPEN(defaultFont, WF_READONLY_BINARY);
    if (ff != OPEN_FAILED) {
      if (f->addFontResource(ff, fs)) 
			{
        DebugStringW(L"font.cpp ====== FONT FOR ID=%s NOT FOUND.  USING DEFAULT FONT FILE:\n%s\n", id, defaultFont);
        fontlist.addItem(f);
      }
    } else {
      DebugStringW(L"font.cpp ====== FONT FOR ID=%s NOT FOUND.  CANNOT OPEN FONT FILE:\n%s\n", id, defaultFont);
      delete f;
      f = NULL;
    }
  } else {
    DebugString("font.cpp ====== CAN'T GET NEW FONT FILE.\n");
    delete f;
    f = NULL;
  }


#ifdef _WIN32
  if (f == NULL) {
    // not found :((((( grab the default font data and use this, whatever it is
    f = newTrueTypeFont();
    if (f) 
    {
      HDC dc = GetDC(GetDesktopWindow());
      HDC dc2 = CreateCompatibleDC(dc);
      SelectObject(dc2, GetStockObject(DEFAULT_GUI_FONT));
      
      int datalen = GetFontData(dc2, 0, 0, NULL, 0);
      if (datalen > 0) {
        void *mem = WASABI_API_MEMMGR->sysMalloc(datalen+1); // freed by the service !!
        ASSERT(mem != NULL);
        GetFontData(dc2, 0, 0, mem, datalen);
        
        f->setFontFace(id);
        f->setFontId(id);
        f->addFontResource2(mem, datalen, id); 

        ReleaseDC(GetDesktopWindow(), dc);
        DeleteDC(dc2);
        fontlist.addItem(f);
        return f;
      }
      delete f;
      f = NULL;
    }
  }
#else
#warning port me
#endif

  if (f == NULL) {
    // ok, NOW I'm getting pissed
    wchar_t fp[WA_MAX_PATH] = {0};
    Wasabi::Std::getFontPath(WA_MAX_PATH, fp);
#ifdef _WIN32
    Wasabi::Std::messageBox(StringPrintfW(L"Fatal error trying to load truetype fonts.\n\nYou need arial.ttf at the very least, but it does not appear to be in %s", fp), L"Fatal Error", MB_ICONERROR);
#else
#warning port me
#endif
  }

  //if (f == NULL) DebugString("font.cpp ====== FALLBACK FOR FONT %s CANNOT BE FOUND IN OUR LISTS.\n",f->getFontId());
  
  return f;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// Intelligently delete the font
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void Font::deleteFont(svc_font *f) 
{
	if (f)
	{
		if (f->isBitmap()) 
		{
			delete static_cast<BitmapFont *>(f); // we delete our own bitmap fonts.
		}
		else 
		{
			SvcEnum::release(f);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// Intelligently make a new truetype font from the service interfaces
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
svc_font *Font::newTrueTypeFont() 
{
/*#ifdef WASABI_COMPILE_CONFIG
  const GUID options_guid = 
  { 0x280876cf, 0x48c0, 0x40bc, { 0x8e, 0x86, 0x73, 0xce, 0x6b, 0xb4, 0x62, 0xe5 } };
  CfgItem *options = WASABI_API_CONFIG->config_getCfgItemByGuid(options_guid);
#endif*/

  svc_font *retval = NULL;
  const wchar_t *name = NULL;

#ifdef WASABI_COMPILE_CONFIG
  //const wchar_t *attr = L"Font Renderer";
  // First, try to find a font service that matches the attribute.
//  if (options) {
//    char buf[256]; // WHEEE for stack arrays
//    if (options->getData(attr, buf, sizeof buf)) {
	if (WASABI_API_SKIN->skin_getVersion() >= 1.3) // hardcode win32 renderer for v1.3+ skins
		retval = FontSvcEnum(L"Win32 TextOut").getFirst();
	else
     retval = FontSvcEnum(cfg_options_fontrenderer.getValue()).getFirst();

#else
#ifndef WASABI_FONT_RENDERER
#error You need to define WASABI_FONT_RENDERER (ie: #define WASABI_FONT_RENDERER "Freetype")
#endif
      retval = FontSvcEnum(WASABI_FONT_RENDERER).getFirst();
#endif
#ifdef WASABI_COMPILE_CONFIG
//    }
//  }
  
  // If we can't find one, fallback and just take the first.
  if (!retval) 
	{
    retval = FontSvcEnum().getFirst();
    if (retval != NULL) 
			name = retval->getFontSvcName();
  }

  // If we had to fallback, remember the fallback service in the attribute.
  if (name/* && options*/) 
	{
    //options->setData(attr, name);
    cfg_options_fontrenderer.setValue(name);
  }
#endif

  return retval;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// Test whether to forbid bitmap fonts.
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int Font::useTrueTypeOverride(const wchar_t *txt)

{
	if (cfg_options_no7bitsttfoverride.getValueAsInt()) 
	{
		const wchar_t *p = (const wchar_t *)txt;
		while (p && *p) 
		{
			// TODO: benski> some characters above 127 can be handled by the bitmap fonts - it might be worth checking those explicitly
			if (*p & 0xFF80)
				break;
			p++;
		}
		if (!*p) return 0;
	}
#ifdef WASABI_COMPILE_CONFIG
/*  // {280876CF-48C0-40bc-8E86-73CE6BB462E5}
	const GUID options_guid = 
	{ 0x280876cf, 0x48c0, 0x40bc, { 0x8e, 0x86, 0x73, 0xce, 0x6b, 0xb4, 0x62, 0xe5 } };
	return !_intVal(WASABI_API_CONFIG->config_getCfgItemByGuid(options_guid), "Use bitmap fonts (no international support)", 1);*/
	return !cfg_options_allowbitmapfonts.getValueAsInt();
#else
	return WASABI_FONT_TTFOVERRIDE;
#endif
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// Get the font to be used to override bitmap fonts.
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
const wchar_t *Font::getTrueTypeOverride() 
{
#ifdef WASABI_COMPILE_CONFIG
	return cfg_options_ttfoverridefont.getValue();
#else
  return L"Arial"; 
#warning TODO
#endif
}

int Font::getTrueTypeOverrideScale() 
{
#ifdef WASABI_COMPILE_CONFIG
  return cfg_options_ttfoverridescale.getValueAsInt();
#else
  return 1;
#warning TODO
#endif
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// Returns the font mapping for this font & skin, if font mapper is on and if there is a mapping, otherwise returns null
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
const wchar_t *Font::getFontMapping(const wchar_t *id, int *size) 
{
  if (cfg_options_usefontmapper.getValueAsInt()) 
	{
    wchar_t t[256]=L"";
    StringW tmp;
    tmp.printf(L"Skin:%s/Font Mapping/%s",WASABI_API_SKIN->getSkinName(), id);
    WASABI_API_CONFIG->getStringPrivate(tmp, t, 256, L""); 

    tmp.printf(L"Skin:%s/Font Mapping/%s_scale",WASABI_API_SKIN->getSkinName(), id);
    int v = WASABI_API_CONFIG->getIntPrivate(tmp, -1);
    if (!*t) 
		{
      tmp.printf(L"Font Mapping/%s", id);
			WASABI_API_CONFIG->getStringPrivate(tmp, t, 256, L"");
      tmp.printf(L"Font Mapping/%s_scale", id);
      v = WASABI_API_CONFIG->getIntPrivate(tmp, -1);
    }
    mapping = t;
    if (mapping.isempty()) return NULL;
    if (size != NULL) 
		{
      if (v != -1) 
			{
        double f = (double)v / 100.0;
        *size = (int)((double)*size * f);
      }
    }
    return mapping;
  }
  return NULL;
}

StringW Font::mapping;