#ifndef _C_CONFIG_H_
#define _C_CONFIG_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#define C_CONFIG_WIN32NATIVE
#include <windows.h>

class C_Config
{
  public:
    C_Config(LPCTSTR pszIniFile);
    ~C_Config();

	void WriteIntEx(LPCTSTR pszSection, LPCTSTR pszKey, int nValue);
	void WriteStringEx(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszValue);
    int ReadIntEx(LPCTSTR pszSection, LPCTSTR pszKey, int nDefault);
	LPTSTR ReadCchStringEx(LPTSTR pszBuffer, INT cchBuffer, LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault);
	LPTSTR ReadCbStringEx(LPTSTR pszBuffer, INT cbBuffer, LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault);
	DWORD GetStringEx(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault, LPTSTR pszReturnedValue, DWORD nSize);
	
	void WriteBoolEx(LPCTSTR pszSection, LPCTSTR pszKey, int nValue) { WriteIntEx(pszSection, pszKey, ( 0 != nValue)); }
	BOOL ReadBoolEx(LPCTSTR pszSection, LPCTSTR pszKey, int nDefault) { return (0 != ReadIntEx(pszSection, pszKey, nDefault)); }

	LPTSTR ReadCchQuotedString(LPTSTR pszBuffer, INT cchBuffer, LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault);
	LPTSTR ReadCbQuotedString(LPTSTR pszBuffer, INT cbBuffer, LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszDefault);
	void WriteQuotedString(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszValue);

    void WriteInt(LPCTSTR pszKey, int nValue);
	void WriteString(LPCTSTR pszKey, LPCTSTR pszValue);
    int ReadInt(LPCTSTR pszKey, int nDefault);
	LPCTSTR ReadString(LPCTSTR pszKey, LPCTSTR pszDefault);
	DWORD GetString(LPCTSTR pszKey, LPCTSTR pszDefault, LPTSTR pszReturnedValue, DWORD nSize);

	void WriteBool(LPCTSTR pszKey, int nValue) { WriteInt(pszKey, ( 0 != nValue)); }
	BOOL ReadBool(LPCTSTR pszKey, int nDefault) { return (0 != ReadInt(pszKey, nDefault)); }

  private:
    TCHAR *m_inifile;
};

#endif//_C_CONFIG_H_
