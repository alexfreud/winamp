//------------------------------------------------------------------------
//
// Apple plist XML Library Data Structures
// Copyright (C) 2003 Nullsoft, Inc.
//
//------------------------------------------------------------------------

#ifndef NULLSOFT_PLIST_TYPES_H
#define NULLSOFT_PLIST_TYPES_H

#include <bfc/memblock.h>
#include <time.h>
#include <bfc/string/bfcstring.h>
#include <bfc/string/stringw.h>
#include <bfc/ptrlist.h>

//------------------------------------------------------------------------
// Base type class
//------------------------------------------------------------------------

enum {
  PLISTDATA_KEY = 0, 
  PLISTDATA_DICT, 
  PLISTDATA_INTEGER,
  PLISTDATA_STRING,
  PLISTDATA_DATE,
  PLISTDATA_ARRAY,
  PLISTDATA_REAL,
  PLISTDATA_RAW,
	PLISTDATA_BOOLEAN,
};


class plistData 
{
public:
  plistData(int _type) : type(_type) {}
  plistData() : type(-1) {}
  virtual ~plistData() {}
  virtual int getType() const { return type; }
  virtual void setType(int _type) { type = _type; }
	virtual void setData(plistData *d)=0;
  virtual const wchar_t *getString() const =0;
  virtual const wchar_t *getTypeString() const  =0;
  virtual void setString(const wchar_t *str);
private:
  int type;
};

//------------------------------------------------------------------------
// A key is a string attached to another piece of data
//------------------------------------------------------------------------
class plistKey : public plistData
{
public:
  plistKey() : name(L""), data(NULL), plistData(PLISTDATA_KEY) {}
  plistKey(const wchar_t *_name, plistData *_data=NULL) : name(_name), data(_data), plistData(PLISTDATA_KEY) {}
  virtual ~plistKey() { delete data; }
  const wchar_t *getName() const { return name; }
  void setName(const wchar_t *_name) { name = _name; }
  plistData *getData() const { return data; }
  virtual void setData(plistData *d) { data = d; }
  virtual const wchar_t *getString() const { return getName(); }
  virtual const wchar_t *getTypeString() const { return L"key"; }
private:
  StringW name;
  plistData *data;
};


class plistKeyholder
{
public:
	virtual int addKey(plistKey *key) = 0;
};
//------------------------------------------------------------------------
// A dictionary contains a list of keys
//------------------------------------------------------------------------
class plistDict : public plistData, public plistKeyholder
{
public:
  plistDict() : plistData(PLISTDATA_DICT) {}
  virtual ~plistDict() { keys.deleteAll(); }
  int getNumKeys() const { return keys.getNumItems(); }
  plistKey *enumKey(int n) const { return keys.enumItem(n); }
  int addKey(plistKey *key) { if (!keys.haveItem(key)) keys.addItem(key); return keys.getNumItems(); }
  virtual void setData(plistData *d) { if (d->getType() != PLISTDATA_KEY) return; keys.addItem((plistKey*)d); }
  int delKey(const wchar_t *name, int dodelete=1) { 
    foreach(keys)
      if (WCSCASEEQLSAFE(name, keys.getfor()->getName())) {
        if (dodelete) delete keys.getfor();
        keys.removeByPos(foreach_index);
        break;
      }
    endfor;
    return keys.getNumItems();
  }
  plistKey *getKey(const wchar_t *name) const {
    foreach(keys)
      if (WCSCASEEQLSAFE(name, keys.getfor()->getName())) {
        return keys.getfor();
      }
    endfor;
    return NULL;
  }
  virtual const wchar_t *getString() const { return L""; }
	virtual const wchar_t *getTypeString() const { return L"dict"; }
private:
  PtrList<plistKey> keys;
};

//------------------------------------------------------------------------
// An array contains a number of data entries
//------------------------------------------------------------------------
class plistArray : public plistData, public plistKeyholder
{
public:
	plistArray() : plistData(PLISTDATA_ARRAY) {}
	virtual ~plistArray() { items.deleteAll(); }
	int getNumItems() const { return items.getNumItems(); }
	plistData *enumItem(int n) const { return items.enumItem(n); }
	virtual void setData(plistData *d) { items.addItem(d); }
	int addKey(plistKey *key) { items.addItem(key); return items.getNumItems(); }
	virtual const wchar_t *getString() const { return L"(array)"; }
	virtual const wchar_t *getTypeString() const { return L"array"; }
private:
	PtrList<plistData> items;
};

//------------------------------------------------------------------------
// Raw data
//------------------------------------------------------------------------
class plistRaw : public plistData {
public:
  plistRaw() : plistData(PLISTDATA_RAW) {}
  virtual ~plistRaw() {}
  virtual void *getMem(int *size) const { if (size) *size = mem.getSize(); return mem.getMemory(); }
  virtual void setMem(void *m, int size) { mem.setSize(size); mem.setMemory((char *)m, size, 0); }
  virtual void setData(plistData *d);
  virtual const wchar_t *getString() const ;
  virtual const wchar_t *getTypeString() const { return L"data"; }
private:
  mutable MemBlock<char> mem;
  mutable StringW encoded;
};

//------------------------------------------------------------------------
// Int 
//------------------------------------------------------------------------
class plistInteger : public plistData {
public:
  plistInteger(int64_t _value) : value(_value), plistData(PLISTDATA_INTEGER) {}
  plistInteger() : value(0), plistData(PLISTDATA_INTEGER) {}
  virtual ~plistInteger() {}

  virtual int64_t getValue() const { return value; }
  virtual void setValue(int64_t _value) { value = _value; }

  virtual void setData(plistData *d);

  virtual const wchar_t *getString() const
	{ 
		static wchar_t s[64];
#ifdef _MSC_VER
		return _i64tow(getValue(), s, 10);
#elif defined(__GCC__)
		sprintf(s, L"%lld", getValue());
		return s;
#else
#error  define me!
#endif
	}
  virtual const wchar_t *getTypeString() const { return L"integer"; }
private:
  int64_t value;
};

class plistBoolean : public plistData 
{
public:
  plistBoolean(bool _value) : value(_value), plistData(PLISTDATA_BOOLEAN) {}
  plistBoolean() : value(0), plistData(PLISTDATA_BOOLEAN) {}
  virtual ~plistBoolean() {}

  virtual bool getValue() const { return value; }
  virtual void setValue(bool _value) { value = _value; }
  virtual void setData(plistData *d);

  virtual const wchar_t *getString() const
	{ 
		if (value)
			return L"true";
		else
			return L"false";
	}
  virtual const wchar_t *getTypeString() const { return L"boolean"; }
private:
  bool value;
};

//------------------------------------------------------------------------
// String
//------------------------------------------------------------------------
class plistString : public plistData {
public:
	plistString(const wchar_t *_str) : str(_str), plistData(PLISTDATA_STRING) {}
	plistString() : str(L""), plistData(PLISTDATA_STRING) {}
	virtual ~plistString() {}
	virtual const wchar_t *getString() const { return str; }
	virtual void setString(const wchar_t *_str) { str = _str; }
	virtual void setData(plistData *d);
	virtual const wchar_t *getTypeString() const { return L"string"; }
private:
	StringW str;
};

//------------------------------------------------------------------------
// Real 
//------------------------------------------------------------------------
class plistReal : public plistData {
public:
	plistReal(int _value) : value(_value), plistData(PLISTDATA_REAL) {}
	plistReal() : value(0), plistData(PLISTDATA_REAL) {}
	virtual ~plistReal() {}
	virtual double getValue() const { return value; }
	virtual void setValue(double _value) { value = _value; }
	virtual void setData(plistData *d);
	virtual const wchar_t *getString() const { static StringW s; s.printf(L"%f", getValue()); return s; }
	virtual const wchar_t *getTypeString() const { return L"real"; }
private:
	double value;
};

//------------------------------------------------------------------------
// Date/Time
//------------------------------------------------------------------------
class plistDate : public plistData {
public:
	plistDate(time_t _when) : when(_when), plistData(PLISTDATA_DATE) {}
	plistDate() : when(0), plistData(PLISTDATA_DATE) {}
	virtual ~plistDate() {}
	virtual time_t getDate() const { return when; }
	virtual void setDate(time_t _when) { when = _when; }
	virtual void setData(plistData *d);
	virtual const wchar_t *getString() const { 
		struct tm *t = gmtime(&when);
		if (t) {
			static wchar_t time_format[256] = {0};
			wcsftime(time_format, 256, L"%Y-%m-%dT%H:%M:%SZ", t);
			return time_format;
		}
		return L""; 
	}
	virtual void setString(const wchar_t *_str);
	virtual const wchar_t *getTypeString() const { return L"date"; }
private:
	time_t when;
};

#endif

//------------------------------------------------------------------------