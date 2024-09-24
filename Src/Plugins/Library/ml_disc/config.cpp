#include "main.h"
#include "./config.h"
#include <shlwapi.h>
#include <strsafe.h>

#define DEFAULT_SECTION		TEXT("gen_ml_config")

#define STRCOMP_INVARIANT		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

C_Config::~C_Config()
{
  if (m_inifile) CoTaskMemFree(m_inifile);
}

C_Config::C_Config(LPCTSTR pszIniFile)
{
	if (S_OK != SHStrDup(pszIniFile, &m_inifile))
		m_inifile = NULL;
}

void C_Config::WriteIntEx(LPCTSTR pszSection, LPCTSTR pszKey, int nValue)
{
  TCHAR buf[32] = {0};
  StringCchPrintf(buf, ARRAYSIZE(buf), TEXT("%d"), nValue);
  WriteStringEx(pszSection, pszKey, buf);
}

int C_Config::ReadIntEx(LPCTSTR pszSection, LPCTSTR pszKey, int nDefault)
{
  return GetPrivateProfileInt(pszSection, pszKey, nDefault, m_inifile);
}

void C_Config::WriteStringEx(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszValue)
{
  WritePrivateProfileString(pszSection, pszKey, pszValue, m_inifile);
 }

LPTSTR C_Config::ReadCbStringEx(LPTSTR pszBuffer, INT cbBuffer, LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault)
{
	return ReadCchStringEx(pszBuffer, cbBuffer/sizeof(TCHAR), pszSection, pszKey, pszDefault);
}

LPTSTR C_Config::ReadCchStringEx(LPTSTR pszBuffer, INT cchBuffer, LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault)
{
	const static TCHAR foobuf[] = TEXT("___________gen_ml_lameness___________");
	pszBuffer[0] = TEXT('\0');
	GetStringEx(pszSection, pszKey, foobuf, pszBuffer, cchBuffer);
	pszBuffer[cchBuffer -1] = TEXT('\0');
	if (CSTR_EQUAL == CompareString(STRCOMP_INVARIANT, 0, foobuf, -1, pszBuffer, -1))
	{
		if (S_OK != StringCchCopyEx(pszBuffer, cchBuffer, pszDefault, NULL, NULL, STRSAFE_IGNORE_NULLS)) 
			return NULL;
	}
	return pszBuffer;
}

LPTSTR C_Config::ReadCchQuotedString(LPTSTR pszBuffer, INT cchBuffer, LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault)
{
	LPTSTR p = ReadCchStringEx(pszBuffer, cchBuffer, pszSection, pszKey, pszDefault);
	if (p) PathUnquoteSpaces(pszBuffer);
	return p;
}

LPTSTR C_Config::ReadCbQuotedString(LPTSTR pszBuffer, INT cbBuffer, LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault)
{
	return ReadCchQuotedString(pszBuffer, cbBuffer/sizeof(TCHAR), pszSection, pszKey, pszDefault);
}

void C_Config::WriteQuotedString(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszValue)
{
	if (!pszValue) return;
	INT cch = lstrlen(pszValue);
	if (cch < MAX_PATH)
	{
		TCHAR szBuffer[MAX_PATH] = {0};
		if (S_OK == StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), pszValue))
		{
			PathQuoteSpaces(szBuffer);
			WriteStringEx(pszSection, pszKey, szBuffer);
			return;
		}
	}
	else
	{
		LPTSTR pszBuffer = (LPTSTR)calloc((cch + 4), sizeof(TCHAR));
		if (pszBuffer)
		{
			if (S_OK == StringCchCopy(pszBuffer, cch + 4, pszValue))
			{
				PathQuoteSpaces(pszBuffer);
				WriteStringEx(pszSection, pszKey, pszBuffer);
				free(pszBuffer);
				return;
			}
			free(pszBuffer);
		}
	}
	WriteStringEx(pszSection, pszKey, pszValue);
}

DWORD C_Config::GetStringEx(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault, LPTSTR pszReturnedValue, DWORD nSize)
{
	return GetPrivateProfileString(pszSection, pszKey, pszDefault, pszReturnedValue, nSize, m_inifile);
}

void C_Config::WriteInt(LPCTSTR pszKey, int nValue)
{
	WriteIntEx(DEFAULT_SECTION, pszKey, nValue);
}

int C_Config::ReadInt(LPCTSTR pszKey, int nDefault)
{
	return ReadIntEx(DEFAULT_SECTION, pszKey, nDefault);
}

void C_Config::WriteString(LPCTSTR pszKey, LPCTSTR pszValue)
{
	WriteStringEx(DEFAULT_SECTION, pszKey, pszValue);
}

LPCTSTR C_Config::ReadString(LPCTSTR pszKey, LPCTSTR pszDefault)
{
	static TCHAR szBuffer[4096];
	return ReadCchStringEx(szBuffer, ARRAYSIZE(szBuffer), DEFAULT_SECTION, pszKey, pszDefault);
}
DWORD C_Config::GetString(LPCTSTR pszKey, LPCTSTR pszDefault, LPTSTR pszReturnedValue, DWORD nSize)
{
	return GetStringEx(DEFAULT_SECTION, pszKey, pszDefault, pszReturnedValue, nSize);
}