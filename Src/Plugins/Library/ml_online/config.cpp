#include "main.h"
#include "./config.h"
#include "./api__ml_online.h"

#include <shlwapi.h>
#include <strsafe.h>

#define CONFIG_SUFFIX		L"Plugins\\ml"
#define CONFIG_SECTION		"ml_online_config"

void C_Config::Flush(void)
{
#ifndef C_CONFIG_WIN32NATIVE
  if (!m_dirty) return;
  FILE *fp=fopen(m_inifile,"wt");
  if (!fp) return;

  fprintf(fp,"[ml_online_config]\n");
  int x;
  if (m_strs) 
  {
    for (x = 0; x < m_num_strs; x ++)
    {
      char name[17] = {0};
      memcpy(name,m_strs[x].name,16);
      name[16]=0;
      if (m_strs[x].value) fprintf(fp,"%s=%s\n",name,m_strs[x].value);
    }
  }
  fclose(fp);
  m_dirty=0;
#endif
}

C_Config::~C_Config()
{
#ifndef C_CONFIG_WIN32NATIVE
  int x;
  Flush();
  if (m_strs) for (x = 0; x < m_num_strs; x ++) free(m_strs[x].value);
  free(m_strs);
#endif

  Plugin_FreeAnsiString(m_inifile);
}

C_Config::C_Config(char *ini)
{
  memset(m_strbuf,0,sizeof(m_strbuf));
  m_inifile= Plugin_CopyAnsiString(ini);

#ifndef C_CONFIG_WIN32NATIVE
  m_dirty=0;
  m_strs=NULL;
  m_num_strs=m_num_strs_alloc=0;

  // read config
  FILE *fp=fopen(m_inifile,"rt");
  if (!fp) return;

  for (;;)
  {
    char buf[4096] = {0};
    fgets(buf,sizeof(buf),fp);
    if (!buf[0] || feof(fp)) break;
    for (;;)
    {
      int l=strlen(buf);
      if (l > 0 && (buf[l-1] == '\n' || buf[l-1] == '\r')) buf[l-1]=0;
      else break;
    }
    if (buf[0] != '[') 
    {
      char *p=strstr(buf,"=");
      if (p)
      {
        *p++=0;
        WriteString(buf,p);
      }
    }
  }
  m_dirty=0;
  fclose(fp);
#endif
}

void C_Config::WriteInt(char *name, int value)
{
  char buf[32] = {0};
  StringCchPrintfA(buf, ARRAYSIZE(buf), "%d",value);
  WriteString(name,buf);
}

int C_Config::ReadInt(char *name, int defvalue)
{
#ifndef C_CONFIG_WIN32NATIVE
  char *t=ReadString(name,"");
  if (*t) return atoi(t);
  return defvalue;
#else
  return GetPrivateProfileIntA("ml_online_config",name,defvalue,m_inifile);
#endif
}

char *C_Config::WriteString(char *name, char *string)
{
#ifndef C_CONFIG_WIN32NATIVE
  	m_dirty=1;
  	for (int x = 0; x < m_num_strs; x ++)
  	{
    	if (m_strs[x].value && !strncmp(name,m_strs[x].name,16))
    	{
      		unsigned int l=(strlen(m_strs[x].value)+16)&~15;
      		if (strlen(string)<l)
      		{
        		strcpy(m_strs[x].value,string);
      		}
      		else
      		{
        		free(m_strs[x].value);
        		m_strs[x].value = (char *)malloc((strlen(string)+16)&~15);
        		strcpy(m_strs[x].value,string);
      		}
      		return m_strs[x].value;
    	}
  	}

  	// not already in there
  	if (m_num_strs >= m_num_strs_alloc || !m_strs)
  	{
		m_old_num_strs_alloc = m_num_strs_alloc;
    	m_num_strs_alloc=m_num_strs*3/2+8;
		strType *data = (strType*)::realloc(m_strs, sizeof(strType) * m_num_strs_alloc);
		if (data)
		{
			m_strs = data;
		}
		else
		{
    		data = (strType*)::malloc(sizeof(strType) * m_num_strs_alloc);
			if (data)
			{
				memcpy(data, m_strs, sizeof(strType) * m_old_num_strs_alloc);
				free(m_strs);
				m_strs = data;
			}
			else m_num_strs_alloc = m_old_num_strs_alloc;
		}
  	}
  	strncpy(m_strs[m_num_strs].name,name,16);
  	m_strs[m_num_strs].value = (char *)malloc((strlen(string)+16)&~15);
	if (m_strs[m_num_strs].value)
	{
  		strcpy(m_strs[m_num_strs].value,string);
  		return m_strs[m_num_strs++].value;
	}
	return "";
#else
  	WritePrivateProfileStringA("ml_online_config",name,string,m_inifile);
  	return name;
#endif
}

char *C_Config::ReadString( char *name, char *defstr )
{
#ifndef C_CONFIG_WIN32NATIVE
	int x;
	for ( x = 0; x < m_num_strs; x++ )
	{
		if ( m_strs[ x ].value && !::strncmp( name, m_strs[ x ].name, 16 ) )
		{
			return m_strs[ x ].value;
		}
	}
	return defstr;
#else
	static char foobuf[] = "___________ml_online_lameness___________";
	m_strbuf[ 0 ] = 0;
	GetPrivateProfileStringA( "ml_online_config", name, foobuf, m_strbuf, sizeof( m_strbuf ), m_inifile );
	if ( !strcmp( foobuf, m_strbuf ) || !strcmp( m_strbuf, "" ) )
	{
		return defstr;
	}

	m_strbuf[ sizeof( m_strbuf ) - 1 ] = 0;
	return m_strbuf;
#endif
}

static LPCSTR Config_GetPath()
{
	static LPSTR configPath = NULL;
	if (NULL == configPath)
	{
		LPCWSTR p = (NULL != WASABI_API_APP) ? WASABI_API_APP->path_getUserSettingsPath() : NULL;
		if (NULL != p)
		{
			WCHAR szBuffer[MAX_PATH * 2] = {0};
			if (0 != PathCombine(szBuffer, p, CONFIG_SUFFIX))
			{
				OMUTILITY->EnsurePathExist(szBuffer);
				PathAppend(szBuffer, L"ml_online.ini");
				configPath = Plugin_WideCharToMultiByte(CP_UTF8, 0, szBuffer, -1, NULL, NULL);
			}
		}
	}

	return configPath;
}

DWORD Config_ReadStr(LPCSTR lpSectionName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize)
{
	if (NULL == lpSectionName) lpSectionName = CONFIG_SECTION;
	return GetPrivateProfileStringA(lpSectionName, lpKeyName, lpDefault, lpReturnedString, nSize, Config_GetPath());
}

UINT Config_ReadInt(LPCSTR lpSectionName, LPCSTR lpKeyName, INT nDefault)
{
	if (NULL == lpSectionName) lpSectionName = CONFIG_SECTION;
	return GetPrivateProfileIntA(lpSectionName, lpKeyName, nDefault, Config_GetPath());
}

HRESULT Config_WriteStr(LPCSTR lpSectionName, LPCSTR lpKeyName, LPCSTR lpString)
{
	LPCSTR configPath = Config_GetPath();
	if (NULL == configPath || '\0' == *configPath) 
		return E_UNEXPECTED;
	
	if (NULL == lpSectionName) lpSectionName = CONFIG_SECTION;
	if (0 != WritePrivateProfileStringA(lpSectionName, lpKeyName, lpString, configPath))
		return S_OK;

	DWORD errorCode = GetLastError();
	return HRESULT_FROM_WIN32(errorCode);
}

HRESULT Config_WriteInt(LPCSTR lpSectionName, LPCSTR lpKeyName, INT nValue)
{
	char szBuffer[32] = {0};
	HRESULT hr = StringCchPrintfA(szBuffer, ARRAYSIZE(szBuffer), "%d", nValue);
	if (FAILED(hr)) return hr;

	if (NULL == lpSectionName) lpSectionName = CONFIG_SECTION;
	return Config_WriteStr(lpSectionName, lpKeyName, szBuffer);
}

HRESULT Config_WriteSection(LPCSTR lpSectionName, LPCSTR lpData)
{
	LPCSTR configPath = Config_GetPath();
	if (NULL == configPath || '\0' == *configPath) 
		return E_UNEXPECTED;
	
	if (NULL == lpSectionName) lpSectionName = CONFIG_SECTION;
	if (0 == WritePrivateProfileSectionA(lpSectionName, lpData, configPath))
		return S_OK;

	DWORD errorCode = GetLastError();
	return HRESULT_FROM_WIN32(errorCode);
}

HRESULT Config_ReadServiceIdList(LPCSTR lpSectionName, LPCSTR lpKeyName, CHAR separator, ReadServiceIdCallback callback, void *data)
{
	if (NULL == callback)
		return E_INVALIDARG;

	DWORD bufferSize = 16384;
	LPSTR buffer = Plugin_MallocAnsiString(bufferSize);
	if (NULL  == buffer) return E_OUTOFMEMORY;

	DWORD bufferLen = Config_ReadStr(lpSectionName, lpKeyName, NULL, buffer, bufferSize);
	if (0 != bufferLen) 
	{
		LPSTR cursor = buffer;
		LPSTR block = cursor;
		UINT serviceId;
		for(;;)
		{
			if (separator == *cursor || '\0' == *cursor)
			{
				while (' ' == *block && block < cursor) block++;
				
				if (block < cursor &&
					FALSE != StrToIntExA(block, STIF_SUPPORT_HEX, (INT*)&serviceId) &&
					0 != serviceId)
				{
	
					if (FALSE == callback(serviceId, data))
						break;
				}

				if ('\0' == *cursor)
					break;

				cursor = CharNextA(cursor);
				block = cursor;
			}
			else
				cursor = CharNextA(cursor);
		}
	}

	Plugin_FreeAnsiString(buffer);
	return S_OK;
}