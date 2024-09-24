#include <precomp.h>
#include "cfgitemx.h"

#define CBCLASS CfgItemX
START_DISPATCH
  CB(CFGITEM_GETNAME, cfgitem_getName);
  CB(CFGITEM_GETGUID, cfgitem_getGuid);
  CB(CFGITEM_GETNUMATTRIBUTES, cfgitem_getNumAttributes);
  CB(CFGITEM_ENUMATTRIBUTE, cfgitem_enumAttribute);
  CB(CFGITEM_GETCONFIGXML, cfgitem_getConfigXML);
  VCB(CFGITEM_ONCFGGROUPCREATE, cfgitem_onCfgGroupCreate);
  VCB(CFGITEM_ONCFGGROUPDELETE, cfgitem_onCfgGroupDelete);
  CB(CFGITEM_GETNUMCHILDREN, cfgitem_getNumChildren);
  CB(CFGITEM_ENUMCHILD, cfgitem_enumChild);
  CB(CFGITEM_GETPARENTGUID, cfgitem_getParentGuid);
  VCB(CFGITEM_ONREGISTER, cfgitem_onRegister);
  VCB(CFGITEM_ONDEREGISTER, cfgitem_onDeregister);
  CB(CFGITEM_GETATTRIBUTETYPE, cfgitem_getAttributeType);
  CB(CFGITEM_GETATTRIBUTECONFIGGROUP, cfgitem_getAttributeConfigGroup);
  CB(CFGITEM_GETDATALEN, cfgitem_getDataLen);
  CB(CFGITEM_GETDATA, cfgitem_getData);
  CB(CFGITEM_SETDATA, cfgitem_setData);
  CB(CFGITEM_GETDEPENDENCYPTR, cfgitem_getDependencyPtr);
  CB(CFGITEM_ADDATTRIB, cfgitem_addAttribute);
  CB(CFGITEM_DELATTRIB, cfgitem_delAttribute);
END_DISPATCH
#undef CBCLASS