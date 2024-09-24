#ifndef NULLSOFT_WINAMP_LANGUAGE_H
#define NULLSOFT_WINAMP_LANGUAGE_H

#include "../Agave/Language/api_language.h"

class Language : public api_language
{
public:
	static const char *getServiceName() { return "Language API"; }
	static const GUID getServiceGuid() { return languageApiGUID; }	
public:
	char *GetString(HINSTANCE hinst, HINSTANCE owner, UINT uID, char *str=NULL, size_t maxlen=0);
	wchar_t *GetStringW(HINSTANCE hinst, HINSTANCE owner, UINT uID, wchar_t *str=NULL, size_t maxlen=0);
	char *GetStringFromGUID(const GUID guid, HINSTANCE owner, UINT uID, char *str=NULL, size_t maxlen=0);
	wchar_t *GetStringFromGUIDW(const GUID guid, HINSTANCE owner, UINT uID, wchar_t *str=NULL, size_t maxlen=0);

	HINSTANCE FindDllHandleByGUID(const GUID guid);
	HINSTANCE FindDllHandleByString(const char* str);
	HINSTANCE FindDllHandleByStringW(const wchar_t* str);
	HINSTANCE StartLanguageSupport(HINSTANCE hinstance, const GUID guid);

	const wchar_t *GetLanguageFolder();

	const wchar_t *GetLanguageIdentifier(int mode);

	HWND CreateLDialogParam(HINSTANCE localised, HINSTANCE original, UINT id, HWND parent, DLGPROC proc, LPARAM param);
	HWND CreateLDialogParamW(HINSTANCE localised, HINSTANCE original, UINT id, HWND parent, DLGPROC proc, LPARAM param);

	INT_PTR LDialogBoxParam(HINSTANCE localised, HINSTANCE original, UINT id, HWND parent, DLGPROC proc, LPARAM param);
	INT_PTR LDialogBoxParamW(HINSTANCE localised, HINSTANCE original, UINT id, HWND parent, DLGPROC proc, LPARAM param);
	
	HMENU LoadLMenu(HINSTANCE localised, HINSTANCE original, UINT id);
	HMENU LoadLMenuW(HINSTANCE localised, HINSTANCE original, UINT id);

	HACCEL LoadAcceleratorsA(HINSTANCE hinst, HINSTANCE owner, LPCSTR lpTableName);
	HACCEL LoadAcceleratorsW(HINSTANCE hinst, HINSTANCE owner, LPCWSTR lpTableName);

	void* LoadResourceFromFile(HINSTANCE hinst, HINSTANCE owner, LPCTSTR lpType, LPCTSTR lpName, DWORD* size);
	void* LoadResourceFromFileW(HINSTANCE hinst, HINSTANCE owner, LPCWSTR lpType, LPCWSTR lpName, DWORD* size);

	BOOL UseUserNumericLocale();
	_locale_t Get_C_NumericLocale();

	wchar_t* FormattedSizeString(wchar_t *out, int cchLen, __int64 size);

protected:
	RECVS_DISPATCH;
};

extern Language *langManager;

#define LANG_STATIC_BUFFER_SIZE 1024

#endif