#ifndef _OPTIONS_H
#define _OPTIONS_H

#include <api/config/items/cfgitemi.h>
#include <api/config/items/attrcb.h>

//#include <api/config/filetypes.h>
#include <api/config/uioptions.h>

class Options;

class _string;
class _int;
class _bool;

extern _string cfg_options_defaultfont;
extern _int cfg_options_ttfoverridescale;
extern _bool cfg_options_no7bitsttfoverride;
extern _bool cfg_options_allowbitmapfonts;
extern _string cfg_options_fontrenderer;

#ifdef _WASABIRUNTIME

class SetupOptions : public CfgItemI {
public:
  SetupOptions();

  Filetypes filetypes;
};

class InstalledComponents : public CfgItemI {
public:
  InstalledComponents();
};
#endif

class AudioOptions : public CfgItemI {
public:
  AudioOptions();
};

#define OPTIONS_PARENT CfgItemI
class Options : public OPTIONS_PARENT {
public:
  Options();
  void checkCd();

  AudioOptions audio_options;
  UIOptions ui_options;
};

#ifdef _WASABIRUNTIME

class LanguageCB : public AttrCallback {
public:
  LanguageCB(CfgItemI *_par) : par(_par) { }
  virtual void onValueChange(Attribute *attr);
private:
  CfgItemI *par;
};

#endif

#endif
