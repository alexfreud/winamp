#ifndef _INIFILE_H
#define _INIFILE_H

#include <bfc/string/StringW.h>

class IniFile 
{
public:
  IniFile(const wchar_t *_filename);

  void setString(const wchar_t *section, const wchar_t *tagname, const wchar_t *val);
  wchar_t *getString(const wchar_t *section, const wchar_t *tagname, wchar_t *buf, int buflen, const wchar_t *default_val = L""); // returns buf
  StringW getString(const wchar_t *section, const wchar_t *tagname, const wchar_t *default_val=L"");

  void setInt(const wchar_t *section, const wchar_t *tagname, int val);
  int getInt(const wchar_t *section, const wchar_t *tagname, int default_val = 0);

  int getBool(const wchar_t *section, const wchar_t *tagname, int default_val = 0);
  void setBool(const wchar_t *section, const wchar_t *tagname, int val);

  GUID getGuid(const wchar_t *section, const wchar_t *tagname, GUID default_val = INVALID_GUID);
  void setGuid(const wchar_t *section, const wchar_t *tagname, const GUID &val);

private:
  StringW filename;
};

#endif
