#include "precomp.h"
#include "configapi.h"
#include "../../studio/config.h"

#include "../../studio/cfglist.h"

config_api *configApi = NULL;

static CfgList cfglist;

void ConfigApi::config_registerCfgItem(CfgItem *cfgitem) {
  cfglist.addItem(cfgitem);
}

void ConfigApi::config_deregisterCfgItem(CfgItem *cfgitem) {
  cfglist.delItem(cfgitem);
}

int ConfigApi::config_getNumCfgItems() {
  return cfglist.getNumItems();
}

CfgItem *ConfigApi::config_enumCfgItem(int n) {
  return cfglist.enumItem(n);
}

CfgItem *ConfigApi::config_getCfgItemByGuid(GUID g) {
  return NULL;//cfglist.getByGuid(g);
}

void ConfigApi::setIntPrivate(const char *name, int val) {
  //config->setInt(name, val);
}

int ConfigApi::getIntPrivate(const char *name, int def_val) {
/*  int ret = config->getInt(name, def_val);
  return ret;*/
  return 0;
}

void ConfigApi::setIntArrayPrivate(const char *name, const int *val, int nval) {
  /*if (nval > 256) return;
  char buf[12*256]="";
  for (int i = 0; i < nval; i++) {
    STRCAT(buf, StringPrintf("%d", val[i]));
    if (i != nval-1) STRCAT(buf, ",");
  }
  config->setString(name, buf);*/
}

int ConfigApi::getIntArrayPrivate(const char *name, int *val, int nval) {
/*  char buf[12*256]="";
  config->getString(name, buf, sizeof(buf), "");
  PathParser pp(buf, ",");
  if (pp.getNumStrings() != nval) return 0;
  for (int i = 0; i < nval; i++) {
    *val = ATOI(pp.enumString(i));
    val ++;
  }
  return 1;*/
  return 1;
}

void ConfigApi::setStringPrivate(const char *name, const char *str) {
  //config->setString(name, str);
}

int ConfigApi::getStringPrivate(const char *name, char *buf, int buf_len, const char *default_str) {
/*  int ret = config->getString(name, buf, buf_len, default_str);
  return ret;*/
  return 0;
}

int ConfigApi::getStringPrivateLen(const char *name) {
  //return config->getStringLen(name);
  return 0;
}

void ConfigApi::setIntPublic(const char *name, int val) {
  //public_config->setInt(name, val);
}

int ConfigApi::getIntPublic(const char *name, int def_val) {
  //return public_config->getInt(name, def_val);
  return 0;
}

void ConfigApi::setStringPublic(const char *name, const char *str) {
//  public_config->setString(name, str);
}
int ConfigApi::getStringPublic(const char *name, char *buf, int buf_len, const char *default_str) {
//  return public_config->getString(name, buf, buf_len, default_str);
  return 0;
}

