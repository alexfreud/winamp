#ifndef __CONFIG_API_H
#define __CONFIG_API_H

#include "../../bfc/api/api_configi.h"

class ConfigApi : public config_apiI {
  public:
    ConfigApi() {}
    virtual ~ConfigApi() {}

    virtual void config_registerCfgItem(CfgItem *cfgitem);
    virtual void config_deregisterCfgItem(CfgItem *cfgitem);
    virtual int config_getNumCfgItems();
    virtual CfgItem *config_enumCfgItem(int n);
    virtual CfgItem *config_getCfgItemByGuid(GUID g);
    virtual void setIntPrivate(const wchar_t *name, int val);
    virtual int getIntPrivate(const wchar_t *name, int def_val);
    virtual void setIntArrayPrivate(const wchar_t *name, const int *val, int nval);
    virtual int getIntArrayPrivate(const wchar_t *name, int *val, int nval);
    virtual void setStringPrivate(const wchar_t *name, const wchar_t *str);
    virtual int getStringPrivate(const wchar_t *name, wchar_t *buf, int buf_len, const wchar_t *default_str);
    virtual int getStringPrivateLen(const wchar_t *name);
    virtual void setIntPublic(const wchar_t *name, int val);
    virtual int getIntPublic(const wchar_t *name, int def_val);
    virtual void setStringPublic(const wchar_t *name, const wchar_t *str);
    virtual int getStringPublic(const wchar_t *name, wchar_t *buf, int buf_len, const wchar_t *default_str);

  protected:
};


#endif
