#ifndef _svc_fONTRENDER_H
#define _svc_fONTRENDER_H

#include <bfc/dispatch.h>
#include <bfc/string/string.h>
#include <api/service/svc_enum.h>
#include <api/service/services.h>
#include <api/service/servicei.h>

class svc_font;
class ifc_canvas;

class NOVTABLE svc_fontRender : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::FONTRENDER; }

  // test the type.
  int isNamed(const char *renderer_name);

  // static methods from Font::
  void installTrueTypeFont(const char *filename, const char *path, const char *id, int scriptid);                     // call this to install a new font
  void installBitmapFont(const char *filename, const char *path, const char *id, int charwidth, int charheight, int hspacing, int vspacing, int scriptid);                       
  void uninstallAll();
  void uninstallByScriptId(int scriptid);
  svc_font *requestSkinFont(const char *id);                                                      // call this to get a Font pointer to a font id
  void dispatchTextOut(ifc_canvas *c, int style, int x, int y, int w, int h, const char *txt);
  int dispatchGetInfo(ifc_canvas *c, const char *font, int infoid, const char *txt, int *w, int *h);
  int useTrueTypeOverride();
  const char *getTrueTypeOverride();


protected:
  enum {
    ISNAMED,
    INSTALLTRUETYPEFONT,
    INSTALLBITMAPFONT,
    UNINSTALLALL,
    UNINSTALLBYSCRIPTID,
    REQUESTSKINFONT,
    DISPATCHTEXTOUT,
    DISPATCHGETINFO,
    USETRUETYPEOVERRIDE,
    GETTRUETYPEOVERRIDE,
  };
};


inline int svc_fontRender::isNamed(const char *renderer_name) {
  return _call(ISNAMED, (int)0, renderer_name);
}

inline void svc_fontRender::installTrueTypeFont(const char *filename, const char *path, const char *id, int scriptid) {
  _voidcall(INSTALLTRUETYPEFONT, filename, path, id, scriptid);
}

inline void svc_fontRender::installBitmapFont(const char *filename, const char *path, const char *id, int charwidth, int charheight, int hspacing, int vspacing, int scriptid) {
  _voidcall(INSTALLBITMAPFONT, filename, path, id, charwidth, charheight, hspacing, vspacing, scriptid);
}

inline void svc_fontRender::uninstallAll() {
  _voidcall(UNINSTALLALL);
}

inline void svc_fontRender::uninstallByScriptId(int scriptid) {
  _voidcall(UNINSTALLBYSCRIPTID, scriptid);
}

inline svc_font *svc_fontRender::requestSkinFont(const char *id) {
  return _call(REQUESTSKINFONT, (svc_font *)0, id);
}

inline void svc_fontRender::dispatchTextOut(ifc_canvas *c, int style, int x, int y, int w, int h, const char *txt) {
  _voidcall(DISPATCHTEXTOUT, c, style, x, y, w, h, txt);
}

inline int svc_fontRender::dispatchGetInfo(ifc_canvas *c, const char *font, int infoid, const char *txt, int *w, int *h) {
  return _call(DISPATCHGETINFO, (int)0, c, font, infoid, txt, w, h  );
}

inline int svc_fontRender::useTrueTypeOverride() {
  return _call(USETRUETYPEOVERRIDE, (int)0);
}

inline const char *svc_fontRender::getTrueTypeOverride() {
  return _call(GETTRUETYPEOVERRIDE, (const char *)0);
}

// implementor derives from this one
class NOVTABLE svc_fontRenderI : public svc_fontRender {
public:

  // test the type
  virtual int isNamed(const char *renderer_name) = 0;

  // static methods from Font::
  virtual void installTrueTypeFont(const char *filename, const char *path, const char *id, int scriptid) = 0;
  virtual void installBitmapFont(const char *filename, const char *path, const char *id, int charwidth, int charheight, int hspacing, int vspacing, int scriptid) = 0;
  virtual void uninstallAll() = 0;
  virtual void uninstallByScriptId(int scriptid) = 0;
  virtual svc_font *requestSkinFont(const char *id) = 0;                                                      // call this to get a Font pointer to a font id
  virtual void dispatchTextOut(ifc_canvas *c, int style, int x, int y, int w, int h, const char *txt) = 0;
  virtual int dispatchGetInfo(ifc_canvas *c, const char *font, int infoid, const char *txt, int *w, int *h) = 0;
  virtual int useTrueTypeOverride() = 0;
  virtual const char *getTrueTypeOverride() = 0;


protected:
  RECVS_DISPATCH;
};

class FontRenderEnum : public SvcEnumT<svc_fontRender> {
public:
  FontRenderEnum(const char *_renderer_name = NULL) : renderer_name(_renderer_name) {}
protected:
  virtual int testService(svc_fontRender *svc) {
    return (svc->isNamed(renderer_name));
  }
private:
  String renderer_name;
};

template <class T>
class FontRenderCreator : public waServiceFactoryTSingle<svc_fontRender, T> {};


#endif // _svc_fONTRENDER_H
