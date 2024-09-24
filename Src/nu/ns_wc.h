#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

__inline int MultiByteToWideCharSZ(
	UINT CodePage,         // code page
  DWORD dwFlags,         // character-type options
  LPCSTR lpMultiByteStr, // string to map
  int cbMultiByte,       // number of bytes in string
  LPWSTR lpWideCharStr,  // wide-character buffer
  int cchWideChar        // size of buffer
)
{
	int converted=0;
	if (cchWideChar == 0)
		return MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
	converted = MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar-1);
	if (!converted)
	{
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			lpWideCharStr[cchWideChar-1]=0;
			return cchWideChar;
		}
		else
			return 0;
	} 
	lpWideCharStr[converted]=0;
	return converted+1;
}

__inline int WideCharToMultiByteSZ(
  UINT CodePage,            // code page
  DWORD dwFlags,            // performance and mapping flags
  LPCWSTR lpWideCharStr,    // wide-character string
  int cchWideChar,          // number of chars in string
  LPSTR lpMultiByteStr,     // buffer for new string
  int cbMultiByte,          // size of buffer
  LPCSTR lpDefaultChar,     // default for unmappable chars
  LPBOOL lpUsedDefaultChar  // set when default char used
)
{
	int converted=0;
	if (cbMultiByte == 0)
		return WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
	converted= WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte-1, lpDefaultChar, lpUsedDefaultChar);
	if (!converted)
	{
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			lpMultiByteStr[cbMultiByte-1]=0;
			return cbMultiByte;
		}
		else
			return 0;
	} 
	lpMultiByteStr[converted]=0;
	return converted+1;
}

#ifdef __cplusplus
}
#endif