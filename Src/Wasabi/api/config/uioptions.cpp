#include <precomp.h>

#include "uioptions.h"

#include <api/wndmgr/layout.h>

#include <api/config/items/attribs.h>
#include <api/config/items/attrcb.h>

#include <api/skin/skinparse.h>
#include <api/wndmgr/alphamgr.h>

// {9149C445-3C30-4e04-8433-5A518ED0FDDE}
const GUID uioptions_guid = 
{ 0x9149c445, 0x3c30, 0x4e04, { 0x84, 0x33, 0x5a, 0x51, 0x8e, 0xd0, 0xfd, 0xde } };

_bool cfg_uioptions_desktopalpha(L"Enable desktop alpha", DEFAULT_DESKTOPALPHA);
_bool cfg_uioptions_linkratio(L"Link layouts scale", DEFAULT_LINKLAYOUTSCALE);
_bool cfg_uioptions_linkalpha(L"Link layouts alpha", DEFAULT_LINKLAYOUTSALPHA);
_bool cfg_uioptions_linkallalpha(L"Link All layouts alpha", DEFAULT_LINKALLALPHA);
_bool cfg_uioptions_linkallratio(L"Link All layouts scale", DEFAULT_LINKALLRATIO);
_int cfg_uioptions_linkedalpha(L"Linked layouts alpha", DEFAULT_LINKEDALPHA);
_int cfg_uioptions_autoopacitytime(L"Auto Opacity hold time", DEFAULT_AUTOOPACITYTIME);
_int cfg_uioptions_autoopacityfadein(L"Auto Opacity fade in time", DEFAULT_AUTOOPACITYFADEIN);
_int cfg_uioptions_autoopacityfadeout(L"Auto Opacity fade out time", DEFAULT_AUTOOPACITYFADEOUT);
_int cfg_uioptions_autoopacitylinked(L"Use Auto Opacity", DEFAULT_AUTOOPACITYTYPE);
_int cfg_uioptions_extendautoopacity(L"Auto Opacity extend by", DEFAULT_EXTENDAUTOOPACITY);
_bool cfg_uioptions_uselocks(L"Use layout scale locks", DEFAULT_USERATIOLOCKS);
_int cfg_uioptions_timerresolution(L"Multiplexed timers resolution", DEFAULT_TIMERRESOLUTION);
_bool cfg_uioptions_tooltips(L"Enable tooltips", DEFAULT_TOOLTIPS);
_float cfg_uioptions_textspeed(L"Text Ticker Speed", DEFAULT_TEXTSPEED);
_int cfg_uioptions_textincrement(L"Text Ticker Increment", DEFAULT_TEXTINCREMENT);
_int cfg_uioptions_appbarshidetime(L"Appbar Hide Time", DEFAULT_APPBARHIDETIME);
_int cfg_uioptions_appbarsshowtime(L"Appbar Show Time", DEFAULT_APPBARSHOWTIME);

void onSetOpacityTime(int n) {
	Layout *l = SkinParser::getMainLayout();
	if (l) l->getAlphaMgr()->setHoldTime(n);
}

void onSetOpacityFadeIn(int n) {
	Layout *l = SkinParser::getMainLayout();
	if (l) l->getAlphaMgr()->setFadeInTime(n);
}

void onSetOpacityFadeOut(int n) {
	Layout *l = SkinParser::getMainLayout();
	if (l) l->getAlphaMgr()->setFadeOutTime(n);
}

void onSetAllRatio(int on) {
	if (on) {
		Layout *l = SkinParser::getMainLayout();
		if (l) l->setRenderRatio(l->getRenderRatio());
	}
}

void onSetAllAlpha(int on) {
	Layout *l = SkinParser::getMainLayout();
	if (l) l->getAlphaMgr()->setAllLinked(on);
}

void onSetLinkedAlpha(int a) {
	Layout *l = SkinParser::getMainLayout();
	if (l) l->getAlphaMgr()->setGlobalAlpha(a);
}

void onSetLinkedAuto100(int v) {
	Layout *l = SkinParser::getMainLayout();
	if (l) l->getAlphaMgr()->setAutoOpacify(v);
}

void onSetExtendAutoOpacity(int v) {
	Layout *l = SkinParser::getMainLayout();
	if (l) l->getAlphaMgr()->setExtendAutoOpacity(v);
}

UIOptions::UIOptions(const wchar_t *name) : CfgItemI(name ? name : L"Skins and UI Tweaks", uioptions_guid) 
{
	registerAttribute(&cfg_uioptions_linkratio);
	registerAttribute(&cfg_uioptions_linkalpha);
	registerAttribute(&cfg_uioptions_uselocks);
	registerAttribute(&cfg_uioptions_autoopacitylinked, new int_attrCB(onSetLinkedAuto100));
	registerAttribute(&cfg_uioptions_linkallratio, new int_attrCB(onSetAllRatio)); 
	registerAttribute(&cfg_uioptions_linkallalpha, new int_attrCB(onSetAllAlpha));
	registerAttribute(&cfg_uioptions_desktopalpha, new int_attrCB(Layout::onGlobalEnableDesktopAlpha));
	registerAttribute(&cfg_uioptions_linkedalpha, new int_attrCB(onSetLinkedAlpha));
	registerAttribute(&cfg_uioptions_tooltips);
	registerAttribute(&cfg_uioptions_timerresolution);
	registerAttribute(&cfg_uioptions_textspeed);
	registerAttribute(&cfg_uioptions_textincrement);
	registerAttribute(&cfg_uioptions_appbarshidetime);
	registerAttribute(&cfg_uioptions_appbarsshowtime);
	registerAttribute(&cfg_uioptions_autoopacitytime, new int_attrCB(onSetOpacityTime));
	registerAttribute(&cfg_uioptions_autoopacityfadein, new int_attrCB(onSetOpacityFadeIn));
	registerAttribute(&cfg_uioptions_autoopacityfadeout, new int_attrCB(onSetOpacityFadeOut));
	registerAttribute(&cfg_uioptions_extendautoopacity, new int_attrCB(onSetExtendAutoOpacity));
	registerAttribute(new _int(L"Timer refresh rate", 30), new int_attrCB(onTimerRefreshRate));
	registerAttribute(new _int(L"Popup menu alpha", 240));
	registerAttribute(new _int(L"Spectrum analyzer mode",1));
}

void UIOptions::onTimerRefreshRate(int rate) {
	if(rate==0 || rate<9 || rate>70) return;
	int res=1000/rate;
	CfgItem *ci=WASABI_API_CONFIG->config_getCfgItemByGuid(uioptions_guid);
	if(!ci) return;
	ci->setDataAsInt(L"Multiplexed timers resolution",res);
}
