#ifndef _CONFIG_H
#define _CONFIG_H

#include <bfc/string/bfcstring.h>
#include <bfc/pair.h>
#include <bfc/critsec.h>
#include <bfc/string/StringW.h>

class StringPair : public Pair<StringW, StringW> 
{
public:
	StringPair(StringW &_a, const wchar_t *_b) 
	{
		b=_b;
		a.swap(_a);	
	}
};

class ConfigFile 
{
public:
  ConfigFile(const wchar_t *section, const wchar_t *name);
  ~ConfigFile();

  static void initialize();

  void setInt(const wchar_t *name, int val);
  int getInt(const wchar_t *name, int default_val);

  void setString(const wchar_t *name, const wchar_t *str);
  int getString(const wchar_t *name, wchar_t *buf, int buf_len, const wchar_t *default_str);

  int getStringLen(const wchar_t *name);

private:
  StringW sectionname;
	StringW prettyname;
  StringPair *getPair(const wchar_t *name);
  StringPair *makePair(const wchar_t *name, const wchar_t *value);
  CriticalSection cs;
};

#endif
