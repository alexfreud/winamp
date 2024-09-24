#include <precomp.h>

#include "options.h"

//#include <api/wac/main.h>//CUT!!
#include <api/wnd/wndtrack.h>

#include <bfc/util/inifile.h>
#include <api/config/items/attribs.h>
#include <api/config/items/attrcb.h>
#include <bfc/wasabi_std_wnd.h>

#include <api/locales/localesmgr.h>
#include <api/font/font.h>

#define ININAME "wasabi.ini"

// {280876CF-48C0-40bc-8E86-73CE6BB462E5}
const GUID options_guid = 
{ 0x280876cf, 0x48c0, 0x40bc, { 0x8e, 0x86, 0x73, 0xce, 0x6b, 0xb4, 0x62, 0xe5 } };

#ifndef GEN_FF

static void setMultipleInstance(int val) 
{
  StringPrintf fn("%s%s%s", WASABI_API_APP->path_getAppPath(), DIRCHARSTR, ININAME);
  IniFile(fn).setBool("Wasabi", "MultipleInstance", val);
}

_int forward_skip_time("Forward skip time", 5000);
_int reverse_skip_time("Reverse skip time", 5000);
#endif

_bool cfg_options_altfonts(L"Alternate Fonts", FALSE);
_bool cfg_options_allowbitmapfonts(L"Use bitmap fonts (no international support)", FALSE);
_string cfg_options_defaultfont(L"Default font", WASABI_DEFAULT_FONTNAMEW L".ttf");
_int cfg_options_defaultfontscale(L"Default font scale", 100);
_string cfg_options_ttfoverridefont(L"TTF font override", WASABI_DEFAULT_FONTNAMEW L".ttf");
_int cfg_options_ttfoverridescale(L"TTF Override Scale", 100);
_bool cfg_options_no7bitsttfoverride(L"No 7-bit TTF override", TRUE);
_bool cfg_options_noalt7bitsttfoverride(L"No 7-bit TTF AltFonts", TRUE);
_bool cfg_options_usefontmapper(L"Enable Font Mapper", FALSE);

#ifdef USEAPPBAR
_bool cfg_options_appbarondrag(L"Dock Appbars on Window Drag", FALSE);
_int cfg_options_appbardockingdistance(L"Appbars Docking Distance", 5);
#endif

void invalidateAll(int b) {
#ifdef WASABI_COMPILE_WNDMGR
  WASABI_API_WNDMGR->wndTrackInvalidateAll();
#endif
}

void onDefaultFontChanged(const wchar_t *attribute) {
  Font::uninstallAll(1);
  Wasabi::Std::setDefaultFont(cfg_options_defaultfont.getValue());
  invalidateAll(1);
}

void onOverrideFontChanged(const wchar_t *attribute) {
  Font::uninstallAll(1);
  invalidateAll(1);
}

void onDefaultFontScaleChanged(int scale) {
  Wasabi::Std::setDefaultFontScale(scale);
  invalidateAll(1);
}

void onOverrideFontScaleChanged(int scale) {
  invalidateAll(1);
}

void onFontRendererChanged(const wchar_t *s) {
  Font::uninstallAll(1);
  invalidateAll(1);
}

void onFontMapperChanged(int b) {
  Font::uninstallAll(1);
  invalidateAll(1);
}

_bool cfg_audiooptions_crossfader(L"Enable crossfading", DEFAULT_CROSSFADE_ENABLED);
_bool cfg_options_alwaysontop(L"Always on top", FALSE);
_bool cfg_options_docking(L"Enable docking", TRUE);
_int cfg_options_dockingdistance(L"Docking distance", DEFAULT_DOCK_DIST);
_string cfg_options_fontrenderer(L"Font Renderer", WASABI_FONT_RENDERER);
_int cfg_options_freetypecharmap(L"Character mapping", -1);

Options::Options() : CfgItemI(L"Options", options_guid) 
{
#ifdef _WASABIRUNTIME
  registerAttribute(&cfg_options_alwaysontop, new int_attrCB(Main::setOnTop));
#else
  extern void setOnTop(int ontop);
  registerAttribute(&cfg_options_alwaysontop, new int_attrCB(setOnTop));
#endif
	/* TODO: benski> move to wndmgr.w5s (or wherever it's final home is */
  registerAttribute(&cfg_options_dockingdistance, new int_attrCB(WindowTracker::setDockDistance));
  registerAttribute(&cfg_options_docking, new int_attrCB(WindowTracker::setEnableDocking));
	/* --- END TO MOVE --- */
  registerAttribute(new _bool(L"Find open rect", FALSE));
  registerAttribute(new _bool(L"Animated rects", TRUE));
  registerAttribute(&cfg_options_fontrenderer, new string_attrCB(onFontRendererChanged));
  registerAttribute(&cfg_options_allowbitmapfonts, new int_attrCB(invalidateAll));
  registerAttribute(&cfg_options_altfonts, new int_attrCB(invalidateAll));
  registerAttribute(&cfg_options_defaultfont, new string_attrCB(onDefaultFontChanged));
  registerAttribute(&cfg_options_ttfoverridefont, new string_attrCB(onOverrideFontChanged));
  registerAttribute(&cfg_options_ttfoverridescale, new int_attrCB(onOverrideFontScaleChanged));
  registerAttribute(&cfg_options_defaultfontscale, new int_attrCB(onDefaultFontScaleChanged));
  registerAttribute(&cfg_options_freetypecharmap, new int_attrCB(invalidateAll));
  registerAttribute(&cfg_options_no7bitsttfoverride, new int_attrCB(invalidateAll));
  registerAttribute(&cfg_options_noalt7bitsttfoverride, new int_attrCB(invalidateAll));
  registerAttribute(&cfg_options_usefontmapper, new int_attrCB(onFontMapperChanged));
#ifdef USEAPPBAR
  registerAttribute(&cfg_options_appbarondrag);
  registerAttribute(&cfg_options_appbardockingdistance);
#endif
#ifdef _WASABIRUNTIME
  registerAttribute(new _bool(L"Allow multiple instances", FALSE), new int_attrCB(setMultipleInstance));
  registerAttribute(new _int(L"Icon mode", 1), new int_attrCB(Main::setIconMode));
  registerAttribute(new _bool(L"Auto-play at startup", FALSE));
  registerAttribute(new _string(L"Language", "English"), new LanguageCB(this));
#ifdef WIN32
  registerAttribute(new _bool("Associate with audio CDs", TRUE), new int_attrCB(Filetypes::registerCdPlayer));
#endif
  registerAttribute(new _string("Monitor aspect ratio", "4:3"));
  registerAttribute(new _int("Internet connection",3)); //3==autodetect
  registerAttribute(&forward_skip_time);
  registerAttribute(&reverse_skip_time);
#endif

  //registerAttribute(new _bool("Use Mozilla instead of IE for minibrowser", FALSE)); // TODO:move into minibrowser component
/*  registerAttribute(new _bool("Force antialias on all TTF", FALSE));*/
  addChildItem(&audio_options);
  addChildItem(&ui_options);
}

void Options::checkCd() {
#ifdef _WASABIRUNTIME
#ifdef WIN32
  if(getDataAsInt("Associate with audio CDs") && !Filetypes::isCdPlayer()) setDataAsInt("Associate with audio CDs",false);
#endif
#endif
}

// {FC3EAF78-C66E-4ed2-A0AA-1494DFCC13FF}
static const GUID audio_options_guid = 
{ 0xfc3eaf78, 0xc66e, 0x4ed2, { 0xa0, 0xaa, 0x14, 0x94, 0xdf, 0xcc, 0x13, 0xff } };

AudioOptions::AudioOptions() : CfgItemI(L"Audio options", audio_options_guid) 
{
#ifdef GEN_FF
#ifdef WASABI_COMPILE_MEDIACORE
  extern void setCrossfader(int crossfade);
  registerAttribute(&cfg_audiooptions_crossfader, new int_attrCB(setCrossfader));
#endif
#endif
#ifdef _WASABIRUNTIME
  registerAttribute(&cfg_audiooptions_crossfader);
  int use_dsound=0;
#ifdef WIN32
  //check for Windows version for whether we make DSOUND default
  DWORD dwVersion = GetVersion();
  DWORD dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
  if (dwVersion < 0x80000000) {
    if(dwWindowsMajorVersion<=4) use_dsound=0; // no dsound on NT4/3.51
    else use_dsound=1; // dsound yes on 2000/XP
  } else
    use_dsound=0; // no dsound by default on win9x (per PP)
#endif
  registerAttribute(new _bool("DirectSound output", use_dsound));
#endif
}

#ifdef _WASABIRUNTIME

// {C1BD5354-5EC4-406c-B5C0-549718D3AF45}
static const GUID setup_guid = 
{ 0xc1bd5354, 0x5ec4, 0x406c, { 0xb5, 0xc0, 0x54, 0x97, 0x18, 0xd3, 0xaf, 0x45 } };

SetupOptions::SetupOptions() : CfgItemI("Setup", setup_guid) {
#ifdef WIN32
  addChildItem(&filetypes);
#endif
}

// {99CFD75C-1CA7-49e5-B8C0-7D78AA443C10}
static const GUID installed_guid = 
{ 0x99cfd75c, 0x1ca7, 0x49e5, { 0xb8, 0xc0, 0x7d, 0x78, 0xaa, 0x44, 0x3c, 0x10 } };

InstalledComponents::InstalledComponents() : CfgItemI("Installed components", installed_guid) {
//CUT  setCfgXml("config.components");
}

void LanguageCB::onValueChange(Attribute *attr) {
  char bufero[WA_MAX_PATH]="";
  attr->getData(bufero, WA_MAX_PATH-1);
  const char *locname;
  for(int i=0;locname=LocalesManager::enumLoadableLocales(i);i++) {
    if (STRCASEEQLSAFE(bufero, locname)) {
      LocalesManager::setNewLocaleNum(i);
      return;
    }
  }
  WASABI_API_WNDMGR->messageBox(StringPrintf("Internal problem switching to language %s, the language name couldn't be found in the list of loaded resources", bufero), "Error", 0, NULL, NULL);
}

#endif
