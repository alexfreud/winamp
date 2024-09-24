#ifndef NULLSOFT_ONLINEMEDIA_PLUGIN_CONFIG_HEADER
#define NULLSOFT_ONLINEMEDIA_PLUGIN_CONFIG_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>


#define C_CONFIG_WIN32NATIVE
class C_Config
{
  public:
    C_Config(char *ini);
    ~C_Config();
    void Flush(void);
    void  WriteInt(char *name, int value);
    char *WriteString(char *name, char *string);
    int   ReadInt(char *name, int defvalue);
    char *ReadString(char *name, char *defvalue);

	const char* GetPath() { return m_inifile; }

  private:
#ifndef C_CONFIG_WIN32NATIVE
    typedef struct 
    {
      char name[16];
      char *value;
    } strType;

    strType *m_strs;
    int m_dirty;
    int m_num_strs, m_num_strs_alloc;
#else
    char m_strbuf[8192];
#endif

    char *m_inifile;
};

// set lpSectionName = NULL to write to default section;
DWORD Config_ReadStr(LPCSTR lpSectionName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize);
UINT Config_ReadInt(LPCSTR lpSectionName, LPCSTR lpKeyName, INT nDefault);
HRESULT Config_WriteStr(LPCSTR lpSectionName, LPCSTR lpKeyName, LPCSTR lpString);
HRESULT Config_WriteInt(LPCSTR lpSectionName, LPCSTR lpKeyName, INT nValue);
HRESULT Config_WriteSection(LPCSTR lpSectionName, LPCSTR lpData);

typedef BOOL (CALLBACK *ReadServiceIdCallback)(UINT /*serviceId*/, void* /*data*/);
HRESULT Config_ReadServiceIdList(LPCSTR lpSectionName, LPCSTR lpKeyName, CHAR separator, ReadServiceIdCallback callback, void *data);


#endif //NULLSOFT_ONLINEMEDIA_PLUGIN_CONFIG_HEADER