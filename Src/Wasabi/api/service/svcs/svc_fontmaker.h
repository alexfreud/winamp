#ifndef _SVC_FONTMAKER_H
#define _SVC_FONTMAKER_H

#include <bfc/dispatch.h>
#include <bfc/string/string.h>
#include <api/service/svc_enum.h>
#include <api/service/services.h>
#include <api/service/servicei.h>

class svc_font;

//
// This class doesn't do anything fantastic.  It's just the way
// you make your OS-Specific font class available to the system. 

class NOVTABLE svc_fontMaker : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::FONTRENDER; }

  // You implement these:
  const char *getFontMakerName();
  svc_font *newTrueTypeFont();
  int deleteTrueTypeFont(svc_font *font);

protected:
  enum {
    GETFONTMAKERNAME,
    NEWTRUETYPEFONT,
    DELETETRUETYPEFONT,
  };
};


inline const char *svc_fontMaker::getFontMakerName() {
  return _call(GETFONTMAKERNAME, (const char *)0);
}

inline svc_font *svc_fontMaker::newTrueTypeFont() {
  return _call(NEWTRUETYPEFONT, (svc_font *)0);
}

inline int svc_fontMaker::deleteTrueTypeFont(svc_font *font) {
  return _call(DELETETRUETYPEFONT, (int)0, font);
}

// implementor derives from this one
class NOVTABLE svc_fontMakerI : public svc_fontMaker {
public:
  virtual const char *getFontMakerName() = 0;
  virtual svc_font *newTrueTypeFont() = 0;
  virtual int deleteTrueTypeFont(svc_font *font) = 0;

protected:
  RECVS_DISPATCH;
};

class FontMakerEnum : public SvcEnumT<svc_fontMaker> {
public:
  FontMakerEnum(const char *_maker_name = NULL) : maker_name(_maker_name) {}
protected:
  virtual int testService(svc_fontMaker *svc) {
    if (!maker_name.len()) return 1; // blank name returns all services.
    return (STRCASEEQL(svc->getFontMakerName(),maker_name));
  }
private:
  String maker_name;
};

template <class T>
class FontMakerCreator : public waServiceFactoryTSingle<svc_fontMaker, T> {};


#endif // _SVC_FONTMAKER_H
