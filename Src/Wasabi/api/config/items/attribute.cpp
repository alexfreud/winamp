#include <precomp.h>

#include "attribute.h"
#include <api/config/items/cfgitemi.h>


Attribute::Attribute(const wchar_t *newname, const wchar_t *_desc) :
  NamedW(newname), desc(_desc), cfgitemi(NULL), private_storage(NULL) { }

Attribute::~Attribute() {
  delete private_storage;
}

const wchar_t *Attribute::getAttributeName() {
  return NamedW::getName();
}

const wchar_t *Attribute::getAttributeDesc() {
  return desc;
}

int Attribute::getValueAsInt() 
{
  wchar_t buf[1024]=L"";
  getData(buf, 1024);
  return WTOI(buf);
}

int Attribute::setValueAsInt(int newval, bool def) 
{
  return setData(StringPrintfW(newval), def);
}

double Attribute::getValueAsDouble() 
{
	wchar_t buf[1024] = {0};
  getData(buf, 1024);
  return WTOF(buf);
}

double Attribute::setValueAsDouble(double newval, bool def) 
{
  return setData(StringPrintfW(newval), def);
}

int Attribute::getDataLen() {
  if (private_storage != NULL)
    return (int)private_storage->len()+1;

  ASSERTPR(WASABI_API_CONFIG != NULL, "getDataLen() before API");
  int r = WASABI_API_CONFIG->getStringPrivateLen(mkTag());
  if (r < 0) {
    r = (int)default_val.len()+1;
  }
  return r;
}

int Attribute::getData(wchar_t *data, int data_len) 
{
  if (data == NULL || data_len < 0)
		return 0;
  if (private_storage != NULL) 
	{
    if (private_storage->isempty()) 
		{
      if (data_len >= 1) {
        *data = 0;
        return 1;
      }
      return 0;
    }
    WCSCPYN(data, private_storage->getValue(), data_len);
    return MIN((int)private_storage->len(), data_len);
  }
  ASSERTPR(WASABI_API_CONFIG != NULL, "can't get without api");
  if (WASABI_API_CONFIG == NULL) return 0;
  int r = WASABI_API_CONFIG->getStringPrivate(mkTag(), data, data_len, default_val.isempty() ? L"" : default_val.getValue());
  return r;
}

int Attribute::setData(const wchar_t *data, bool def) 
{
  if (def) {	// setting default value
    default_val = data;
    return 1;
  }
  ASSERTPR(WASABI_API_CONFIG != NULL, "can't set data before api");
  if (WASABI_API_CONFIG == NULL) return 0;

  int r = setDataNoCB(data);
  if (r && cfgitemi != NULL) cfgitemi->cfgitem_onAttribSetValue(this);
  return r;
}

int Attribute::setDataNoCB(const wchar_t *data)
{
  if (private_storage != NULL) {
    private_storage->setValue(data);
  } else {
    WASABI_API_CONFIG->setStringPrivate(mkTag(), data);
  }
  dependent_sendEvent(Attribute::depend_getClassGuid(), Event_DATACHANGE);
  return 1;
}

void Attribute::setCfgItem(CfgItemI *item) 
{
  delete private_storage; private_storage = NULL;
  ASSERT(cfgitemi == NULL || item == NULL);
  cfgitemi = item;
  if (cfgitemi != NULL) 
	{
    if (cfgitemi->cfgitem_usePrivateStorage()) 
			private_storage = new StringW;
  }
}

StringW Attribute::mkTag() 
{
  StringW ret;
	if (cfgitemi)
	{
		ret = cfgitemi->cfgitem_getPrefix();
	}
  ret.cat(getName());
  return ret;
}

void Attribute::disconnect() {
  setCfgItem(NULL);
}
