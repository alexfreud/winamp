#include <precomp.h>
#include "statswnd.h"
#include <bfc/string/StringW.h>

StatsWnd::StatsWnd() 
{
  registerXml();
  WASABI_API_WNDMGR->autopopup_registerGroupId(L"statswnd.group", L"Internal Statistics");
  WASABI_API_SYSCB->syscb_registerCallback(this);
}

StatsWnd::~StatsWnd() 
{
  WASABI_API_SYSCB->syscb_deregisterCallback(this);
}

int StatsWnd::skincb_onBeforeLoadingElements() 
{
  registerXml();
  return 1;
}

void StatsWnd::registerXml() 
{
  StringW xml;
  
  xml = L"buf:";

  xml += L"<?xml version=\"1.0\" encoding=\"UTF-16\" standalone=\"yes\"?>\n";
  xml += L"<WasabiXml version=\"1.0\">\n";
  xml += L"<groupdef id=\"statswnd.group\" name=\"Internal Statistics\">\n";
  xml += L"  <Wasabi:Stats fitparent=\"1\" />\n";
  xml += L"</groupdef>\n";
  xml += L"</WasabiXml>\n";

  WASABI_API_SKIN->loadSkinFile(xml);
}

