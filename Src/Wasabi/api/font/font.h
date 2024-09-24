#ifndef __FONT_H
#define __FONT_H

#include <bfc/ptrlist.h>
#include <bfc/string/StringW.h>

class ifc_canvas;
class svc_font;
class svc_fontMaker;

class FontDef {
  public:
  StringW filename;
  StringW path;
  StringW id;
  int allowmapping;
  int isbitmap;
  int scriptid;
};

class Font {
 public: 
  static void init();

  static svc_font *installTrueTypeFont(const wchar_t *filename, const wchar_t *path, const wchar_t *id, int scriptid, int allowmapping, int isreload);                     // call this to install a new font
  static void installBitmapFont(const wchar_t *filename, const wchar_t *path, const wchar_t *id, int charwidth, int charheight, int hspacing, int vspacing, int scriptid, int allowmapping);                       
  static void uninstallAll(int isttfreload=0);
  static void uninstallByScriptId(int scriptid);
  
  static svc_font *requestSkinFont(const wchar_t *id, int *size=NULL, int *gotdefault=NULL);  // call this to get a Font pointer to a font id, pass your size if you have one, so that the mapper can do its job. 
  static void dispatchTextOut(ifc_canvas *c, int style, int x, int y, int w, int h, const wchar_t *txt);
  static int dispatchGetInfo(ifc_canvas *c, const wchar_t *font, int infoid, const wchar_t *txt, int *w, int *h);

  static int useTrueTypeOverride(const wchar_t *txt);
  static const wchar_t *getTrueTypeOverride();
  static int getTrueTypeOverrideScale();
  static int getNumFonts() { return fontlist.getNumItems(); }
  
  static FontDef *enumFontDef(int n) { return fontdefs.enumItem(n); }
  static int getNumFontDefs() { return fontdefs.getNumItems(); }

  static const wchar_t *getFontMapping(const wchar_t *id, int *size);

 private:
  static void deleteFont(svc_font *font);
  static svc_font *newTrueTypeFont();

  static PtrList<svc_font> fontlist;
  static PtrList<FontDef> fontdefs;
  StringW font_id;
  int scriptid;
  static StringW mapping;
};

#endif
