#include <precomp.h>
//<?#include "<class data="implementationheader"/>"
#include "api_configi.h"
//?>

#include <bfc/parse/pathparse.h>
#include <api/config/cfglist.h>


static CfgList cfglist;

api_config *configApi = NULL;

api_configI::api_configI() 
: public_config(StringPrintfW(INVALID_GUID), L"public data")
{
  public_config.initialize();
}

api_configI::~api_configI() 
{
}

void api_configI::config_registerCfgItem(CfgItem *cfgitem)
{
  cfglist.addItem(cfgitem);
}

void api_configI::config_deregisterCfgItem(CfgItem *cfgitem) 
{
  cfglist.delItem(cfgitem);
}

int api_configI::config_getNumCfgItems() 
{
  return cfglist.getNumItems();
}

CfgItem *api_configI::config_enumCfgItem(int n) 
{
  return cfglist.enumItem(n);
}

CfgItem *api_configI::config_getCfgItemByGuid(GUID g) 
{
  return cfglist.getByGuid(g);
}

// The private config functions are currently pointing at the public config item, this is because
// only the monolithic api gets instantiated once per component and thus can know its GUID, this
// version of the config api should eventually be instantiated once per component as well when
// we start making them use the modular apis
void api_configI::setIntPrivate(const wchar_t *name, int val)
{
  public_config.setInt(name, val);
}

int api_configI::getIntPrivate(const wchar_t *name, int def_val) 
{
  int ret = public_config.getInt(name, def_val);
  return ret;
}

void api_configI::setIntArrayPrivate(const wchar_t *name, const int *val, int nval) 
{
  if (nval > 256) return;
  wchar_t buf[12*256]=L"";
  for (int i = 0; i < nval; i++)
  {
    wcscat(buf, StringPrintfW(L"%d", val[i]));
    if (i != nval-1) 
      wcscat(buf, L",");
  }
  public_config.setString(name, buf);
}

int api_configI::getIntArrayPrivate(const wchar_t *name, int *val, int nval) 
{
  wchar_t buf[12*256]=L"";
  public_config.getString(name, buf, sizeof(buf)/sizeof(*buf), L"");
  PathParserW pp(buf, L",");
  if (pp.getNumStrings() != nval) return 0;
  for (int i = 0; i < nval; i++) {
    *val = WTOI(pp.enumString(i));
    val ++;
  }
  return 1;
}

void api_configI::setStringPrivate(const wchar_t *name, const wchar_t *str) 
{
  public_config.setString(name, str);
}

int api_configI::getStringPrivate(const wchar_t *name, wchar_t *buf, int buf_len, const wchar_t *default_str) 
{
  int ret = public_config.getString(name, buf, buf_len, default_str);
  return ret;
}

int api_configI::getStringPrivateLen(const wchar_t *name) 
{
  return public_config.getStringLen(name);
}

void api_configI::setIntPublic(const wchar_t *name, int val) 
{
  public_config.setInt(name, val);
}

int api_configI::getIntPublic(const wchar_t *name, int def_val) 
{
  return public_config.getInt(name, def_val);
}

void api_configI::setStringPublic(const wchar_t *name, const wchar_t *str)
{
  public_config.setString(name, str);
}

int api_configI::getStringPublic(const wchar_t *name, wchar_t *buf, int buf_len, const wchar_t *default_str) 
{
  return public_config.getString(name, buf, buf_len, default_str);
}
