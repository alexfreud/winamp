#ifndef _WA2CFGITEMS_H
#define _WA2CFGITEMS_H

#include <api/config/items/cfgitemi.h>
#include <api/config/items/attribs.h>
#include <api/config/items/attrcb.h>

//---------------------------------------------------------
// playlist editor cfg items
//---------------------------------------------------------
class Wa2PlEditCfgItems : public CfgItemI {
public:  
  Wa2PlEditCfgItems();
  virtual ~Wa2PlEditCfgItems();

  static void onSetShuffle(BOOL set);
  static void onSetRepeat(BOOL set);
};

//---------------------------------------------------------
// {1828D28F-78DD-4647-8532-EBA504B8FC04}
static const GUID customOptionsMenuitemGuid = 
{ 0x1828d28f, 0x78dd, 0x4647, { 0x85, 0x32, 0xeb, 0xa5, 0x4, 0xb8, 0xfc, 0x4 } };

//	{0542AFA4-48D9-4c9f-8900-5739D52C114F}
static const GUID skinTweaksGuid = 
{ 0x542afa4, 0x48d9, 0x4c9f, { 0x89, 0x0, 0x57, 0x39, 0xd5, 0x2c, 0x11, 0x4f } };

// {6559CA61-7EB2-4415-A8A9-A2AEEF762B7F}
static const GUID customWindowsMenuitemGuid = 
{ 0x6559ca61, 0x7eb2, 0x4415, { 0xa8, 0xa9, 0xa2, 0xae, 0xef, 0x76, 0x2b, 0x7f } };

// {F1239F09-8CC6-4081-8519-C2AE99FCB14C}
static const GUID crossfaderGuid = 
{ 0xf1239f09, 0x8cc6, 0x4081, { 0x85, 0x19, 0xc2, 0xae, 0x99, 0xfc, 0xb1, 0x4c } };

//---------------------------------------------------------
class CustomOptionsMenuItems : public CfgItemI {
public:
  CustomOptionsMenuItems() : CfgItemI(L"Custom OptionsMenu Items", customOptionsMenuitemGuid) { };
  virtual ~CustomOptionsMenuItems() {}
};

//---------------------------------------------------------
class Crossfader : public CfgItemI {
public:
  Crossfader();
  virtual ~Crossfader();

  static void onOutputChanged();
};

//---------------------------------------------------------
class AvsCfg : public CfgItemI {
public:
  AvsCfg();
  virtual ~AvsCfg() {}

  static void onVisRandomChanged(BOOL set);
};

//---------------------------------------------------------
class CustomWindowsMenuItems : public CfgItemI {
public:
  CustomWindowsMenuItems() : CfgItemI(L"Custom WindowsMenu Items", customWindowsMenuitemGuid) { };
  virtual ~CustomWindowsMenuItems() {}
};

//---------------------------------------------------------
class SkinTweaks : public CfgItemI {
public:
  SkinTweaks();
  virtual ~SkinTweaks() {}
  static void onPreventVideoStopChanged(BOOL set);
	static void onPreventVideoResize(BOOL set);
	//static void onDisplayVideoWndOnPlay(BOOL set);
	//static void onCloseVideoWndOnStop(BOOL set);
};

//---------------------------------------------------------
class Wa2CfgItems {
public:
  Wa2CfgItems();
  virtual ~Wa2CfgItems();

private:  
  Wa2PlEditCfgItems pledit;
};

extern _bool shuffle;
extern _int repeat;
extern _bool cfg_audiooptions_crossfader;
extern _bool cfg_options_alwaysontop;
extern _bool cfg_uioptions_desktopalpha;
extern _bool cfg_uioptions_linkratio;
extern _bool cfg_uioptions_linkalpha;
extern _bool cfg_uioptions_linkallratio;
extern _bool cfg_uioptions_linkallalpha;
extern _bool cfg_uioptions_tooltips;
extern _float cfg_uioptions_textspeed;
extern _int cfg_uioptions_textincrement;
extern _int cfg_uioptions_appbarshidetime;
extern _int cfg_uioptions_appbarsshowtime;
extern _int cfg_uioptions_timerresolution;
extern _bool cfg_audiooptions_crossfader;
extern _bool cfg_options_altfonts;
extern _bool cfg_options_allowbitmapfonts;
extern _string cfg_options_defaultfont;
extern _string cfg_options_ttfoverridefont;
extern _int cfg_options_ttfoverridescale;
extern _int cfg_options_defaultfontscale;
extern _string cfg_options_fontrenderer;
extern _bool cfg_options_docking;
extern _int cfg_options_dockingdistance;
extern _bool cfg_options_appbarondrag;
extern _int cfg_options_appbardockingdistance;
extern _int cfg_options_freetypecharmap;
extern _bool cfg_options_no7bitsttfoverride;
extern _bool cfg_options_noalt7bitsttfoverride;
extern _bool cfg_uioptions_uselocks;
extern _int cfg_uioptions_autoopacitytime;
extern _int cfg_uioptions_autoopacitylinked;
extern _int cfg_uioptions_linkedalpha;
extern _int cfg_uioptions_autoopacityfadein;
extern _int cfg_uioptions_autoopacityfadeout;
extern _int cfg_uioptions_extendautoopacity;
extern _bool cfg_options_usefontmapper;

extern CustomOptionsMenuItems *optionsmenuitems;
extern CustomWindowsMenuItems *windowsmenuitems;
extern SkinTweaks *skintweaks;
extern int disable_set_wa2_repeat;
extern StringW eqmenustring;

extern int my_set;

#endif