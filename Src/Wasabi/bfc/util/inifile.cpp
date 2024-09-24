#include <precomp.h>

#include "inifile.h"
#include <bfc/nsguid.h>

#ifndef WIN32
#include "profile.h"
#endif

IniFile::IniFile(const wchar_t *_filename) : filename(_filename) { }

void IniFile::setString(const wchar_t *section, const wchar_t *tagname, const wchar_t *val) {
  WritePrivateProfileStringW(section, tagname, val, filename);
}

wchar_t *IniFile::getString(const wchar_t *section, const wchar_t *tagname, wchar_t *buf, int buflen, const wchar_t *default_val) {
  GetPrivateProfileStringW(section, tagname, default_val, buf, buflen, filename);
  return buf;
}

StringW IniFile::getString(const wchar_t *section, const wchar_t *tagname, const wchar_t *default_val) {
  wchar_t buf[WA_MAX_PATH]=L"";
  getString(section, tagname, buf, WA_MAX_PATH-1, default_val);
  return StringW(buf);
}

void IniFile::setInt(const wchar_t *section, const wchar_t *tagname, int val) {
  setString(section, tagname, StringPrintfW(val));
}

int IniFile::getInt(const wchar_t *section, const wchar_t *tagname, int default_val) {
  wchar_t buf[MAX_PATH] = {0};
  getString(section, tagname, buf, sizeof(buf), StringPrintfW(default_val));
  return WTOI(buf);
}

int IniFile::getBool(const wchar_t *section, const wchar_t *tagname, int default_val) {
  wchar_t buf[MAX_PATH] = {0};
  getString(section, tagname, buf, sizeof(buf), default_val ? L"true" : L"false");
  if (!_wcsicmp(buf, L"true")) return 1;
  return 0;
}
  
void IniFile::setBool(const wchar_t *section, const wchar_t *tagname, int val) {
  setString(section, tagname, val ? L"true" : L"false");
}

GUID IniFile::getGuid(const wchar_t *section, const wchar_t *tagname, GUID default_val) {
  wchar_t buf[MAX_PATH] = {0};
  getString(section, tagname, buf, sizeof(buf), StringPrintfW(default_val));
  return nsGUID::fromCharW(buf);
}

void IniFile::setGuid(const wchar_t *section, const wchar_t *tagname, const GUID &val) {
  setString(section, tagname, StringPrintfW(val));
}
