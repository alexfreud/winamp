#ifndef NULLSOFT_WEBDEV_PLUGIN_CONFIG_HEADER
#define NULLSOFT_WEBDEV_PLUGIN_CONFIG_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

DWORD Config_ReadStr(LPCSTR lpSectionName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize);
UINT Config_ReadInt(LPCSTR lpSectionName, LPCSTR lpKeyName, INT nDefault);
HRESULT Config_WriteStr(LPCSTR lpSectionName, LPCSTR lpKeyName, LPCSTR lpString);
HRESULT Config_WriteInt(LPCSTR lpSectionName, LPCSTR lpKeyName, INT nValue);
HRESULT Config_WriteSection(LPCSTR lpSectionName, LPCSTR lpData);



#endif //NULLSOFT_WEBDEV_PLUGIN_CONFIG_HEADER