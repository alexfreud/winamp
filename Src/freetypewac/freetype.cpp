#include <precomp.h>
#include <bfc/wasabi_std.h>
#include "freetype.h"
#include "freetypefont.h"
#include <api/font/FontCreator.h>
#include <tataki/export.h>


class AutoTataki  : public LoadableResource
{
public:
	virtual void onRegisterServices() { Tataki::Init(WASABI_API_SVC); }
	virtual void onDeregisterServices() { Tataki::Quit(); }
};


static WACNAME wac;
WACPARENT *the = &wac;                     

// {EF7E83B0-10ED-4be4-AFAF-D012F05C114E}
static const GUID guid =
{ 0xef7e83b0, 0x10ed, 0x4be4, { 0xaf, 0xaf, 0xd0, 0x12, 0xf0, 0x5c, 0x11, 0x4e } };

// {504060F6-7D8C-4ebe-AE1D-A8BDF5EA1881}
static const GUID freetypeFontRendererGUID = 
{ 0x504060f6, 0x7d8c, 0x4ebe, { 0xae, 0x1d, 0xa8, 0xbd, 0xf5, 0xea, 0x18, 0x81 } };

WACNAME::WACNAME() : WACPARENT(L"Freetype Font Service") 
{
	registerResource(new AutoTataki);
  registerService(new FontCreator<FreeTypeFont>(freetypeFontRendererGUID));
}

WACNAME::~WACNAME() {
}

GUID WACNAME::getGUID() {
  return guid;
}

void WACNAME::onRegisterServices() 
{
	/*
#ifdef WASABI_COMPILE_CONFIG // if not, you need to set the font renderer by hand
  // Here is where we hijack the default renderer and do it all freetype style.
  const GUID options_guid = 
  { 0x280876cf, 0x48c0, 0x40bc, { 0x8e, 0x86, 0x73, 0xce, 0x6b, 0xb4, 0x62, 0xe5 } };
  CfgItem *options = WASABI_API_CONFIG->config_getCfgItemByGuid(options_guid);
  if (options) {
    const char *attr = "Font Renderer";
    const char *name = "Freetype";
    options->setData(attr, name);
  }  
  // the fonts system uses this attrib for what font system to try to use.
#endif
	*/
}

void WACNAME::onCreate() {
}

void WACNAME::onDestroy() {
}

