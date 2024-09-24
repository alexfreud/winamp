#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "config.h"

C_Config::~C_Config()
{
  free(m_inifile);
  free(m_section);
}

C_Config::C_Config(wchar_t *ini, wchar_t *section, C_Config * globalWrite) : globalWrite(globalWrite)
{
  m_strbuf[0]=0;
  m_inifile=_wcsdup(ini);
  m_section=_wcsdup(section);
}

void C_Config::WriteInt(wchar_t *name, int value, wchar_t *section)
{
  wchar_t buf[32] = {0};
  wsprintf(buf,L"%d",value);
  WriteString(name,buf,section);
}

int C_Config::ReadInt(wchar_t *name, int defvalue, wchar_t *section)
{
  return GetPrivateProfileInt(section?section:m_section,name,defvalue,m_inifile);
}

wchar_t *C_Config::WriteString(wchar_t *name, wchar_t *string, wchar_t *section)
{
  if(globalWrite && !section) globalWrite->WriteString(name,string,m_section);
  WritePrivateProfileString(section?section:m_section,name,string,m_inifile);
  return name;
}

wchar_t *C_Config::ReadString(wchar_t *name, wchar_t *defstr, wchar_t *section)
{
  static wchar_t foobuf[] = L"___________config_lameness___________";
  m_strbuf[0]=0;
  GetPrivateProfileString(section?section:m_section,name,foobuf,m_strbuf,sizeof(m_strbuf)/sizeof(wchar_t),m_inifile);
  if (!lstrcmp(foobuf,m_strbuf)) return defstr;

  m_strbuf[sizeof(m_strbuf)/sizeof(wchar_t)-1]=0;
  return m_strbuf;
}