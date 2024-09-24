#include <shlwapi.h>
#include "..\..\..\nu\ns_wc.h"
#include "config.h"
#include <strsafe.h>

///////////////////
///
///  UNICODE VERSION
///
/////
ConfigW::ConfigW()
{
	emptyBOM = FALSE;
	buff[0] = 0;
	buffA = NULL;
	fileName = NULL;
	defSection = NULL;
}

ConfigW::ConfigW(const wchar_t *ini, const wchar_t *section)
{
	emptyBOM = FALSE;
	buff[0] = 0;
	buffA = NULL;
	fileName = NULL;
	defSection = NULL;
	if (SetIniFile(ini)) SetSection(section);
}

ConfigW::~ConfigW()
{
  if (fileName) free(fileName);
  if (defSection) free(defSection);
  if (buffA) 	free(buffA);
  if (emptyBOM) RemoveEmptyFile();
}

void ConfigW::Flush(void)
{
	return;
}
	
BOOL ConfigW::Write(const wchar_t *section, const wchar_t *name, const wchar_t *value)
{
	return (fileName) ? WritePrivateProfileString(section, name, value, fileName) : FALSE;
}

BOOL ConfigW::Write(const wchar_t *section, const wchar_t *name, double value)
{
	wchar_t tmp[48] = {0};
	if (S_OK != StringCchPrintf(tmp, 48, L"%g", value)) return FALSE;
	return Write(section, name, tmp);
}

BOOL ConfigW::Write(const wchar_t *section, const wchar_t *name, long long value)
{
	wchar_t tmp[32] = {0};
	if (S_OK != StringCchPrintf(tmp, 32, L"%I64d", value)) return FALSE;
	return Write(section,name, tmp);
}

BOOL ConfigW::Write(const wchar_t *section, const wchar_t *name, int value)
{
	wchar_t tmp[16] = {0};
	if (S_OK != StringCchPrintf(tmp, 16, L"%d", value)) return FALSE;
	return Write(section,name, tmp);
}

BOOL ConfigW::Write(const wchar_t *section, const wchar_t *name, const char *value)
{
	int len = (int)strlen(value) + 1;
	wchar_t *tmp = (wchar_t*)malloc(len*sizeof(wchar_t));
    if (!tmp) return FALSE;
	
	BOOL ret = FALSE;
	if (MultiByteToWideCharSZ(CP_ACP, 0, value, -1, tmp, len)) 
	{
		ret = Write(section,name, tmp);
	}
	free(tmp);
	return ret;
}

BOOL ConfigW::Write(const wchar_t *name, int value)
{
	if(!defSection) return FALSE;
	return Write(defSection, name, value);
}

BOOL ConfigW::Write(const wchar_t *name, long long value)
{
	if(!defSection) return FALSE;
	return Write(defSection, name, value);
}

BOOL ConfigW::Write(const wchar_t *name, double value)
{
	if(!defSection) return FALSE;
	return Write(defSection, name, value);
}

BOOL ConfigW::Write(const wchar_t *name, const wchar_t *value)
{
	if(!defSection) return FALSE;
	return Write(defSection, name, value);
}

BOOL ConfigW::Write(const wchar_t *name, const char value)
{
	if(!defSection) return FALSE;
	return Write(defSection, name, value);
}

int ConfigW::ReadInt(const wchar_t *section, const wchar_t *name, int defvalue)
{
	if (!fileName) return defvalue;
	return GetPrivateProfileInt(section, name, defvalue, fileName);
}

long long ConfigW::ReadInt64(const wchar_t *section, const wchar_t *name, long long defvalue)
{
	if (!fileName) return defvalue;
	wchar_t tmp[32] = {0};
	if (S_OK != StringCchPrintf(tmp, 32, L"%I64d", defvalue)) return defvalue;
	const wchar_t *ret = ReadStringW(section, name, tmp);
	return  _wtoi64(ret);
}

double ConfigW::ReadDouble(const wchar_t *section, const wchar_t *name, double defvalue)
{
	if (!fileName) return defvalue;
	wchar_t tmp[32] = {0};
	if (S_OK != StringCchPrintf(tmp, 32, L"%g", defvalue)) return defvalue;
	const wchar_t *ret = ReadStringW(section, name, tmp);
	return  _wtof(ret);
}

const char* ConfigW::ReadStringA(const wchar_t *section, const wchar_t *name, const char *defvalue)
{
	if (buffA) 	free(buffA);
	if (!fileName) return defvalue;

	int len = (int)strlen(defvalue) + 1;
	wchar_t *tmp = (wchar_t*)malloc(len *sizeof(wchar_t));
    if (!tmp) return defvalue;
		
	if (!MultiByteToWideCharSZ(CP_ACP, 0, defvalue, -1, tmp, len)) 
	{
		free(tmp);
		return defvalue;
	}
	const wchar_t *ret = ReadStringW(section, name, tmp);
	if (!ret || lstrcmp(ret, tmp) == 0)
	{
		free(tmp);
		return defvalue;
	}
	free(tmp);
	
	len = (int)lstrlen(ret) + 1;

	buffA = (char*)malloc(len*sizeof(char));
	if (!buffA) return defvalue;
	if (WideCharToMultiByteSZ(CP_ACP, 0, ret, -1, buffA, len, NULL, NULL))
	{
		free(buffA);
		buffA = NULL;
		return defvalue;
	}	
	return buffA;
}

const wchar_t* ConfigW::ReadStringW(const wchar_t *section, const wchar_t *name, const wchar_t *defvalue)
{
	if (!fileName) return defvalue;
	static wchar_t def[] = L"_$~$_";
	buff[0] = 0;
	int len = GetPrivateProfileString(section, name, def, buff, BUFF_SIZE, fileName);
	if (!len || !lstrcmp(def,buff)) return defvalue;
	buff[BUFF_SIZE-1]=0;
	return buff;
}

int ConfigW::ReadInt(const wchar_t *name, int defvalue)
{
	if(!defSection) return defvalue;
	return ReadInt(defSection, name, defvalue);
}

long long ConfigW::ReadInt64(const wchar_t *name, long long defvalue)
{
	if(!defSection) return defvalue;
	return ReadInt64(defSection, name, defvalue);
}

double ConfigW::ReadDouble(const wchar_t *name, double defvalue)
{
	if(!defSection) return defvalue;
	return ReadDouble(defSection, name, defvalue);
}

const char* ConfigW::ReadStringA(const wchar_t *name, const char *defvalue)
{
	if(!defSection) return defvalue;
	return ReadStringA(defSection, name, defvalue);
}

const wchar_t* ConfigW::ReadStringW(const wchar_t *name, const wchar_t *defvalue)
{
	if(!defSection) return defvalue;
	return ReadStringW(defSection, name, defvalue);
}

BOOL ConfigW::SetSection(const wchar_t *section)
{
	if (defSection) 
	{
		free(defSection); 
		defSection = NULL;
	}
	size_t len;
	len = lstrlen(section) + 1;
	defSection = (wchar_t*)malloc(len*sizeof(wchar_t));
	if (NULL == defSection)
	{
		return FALSE;
	}
	
	if (S_OK != StringCchCopy(defSection, len, section))
	{
		free(defSection); 
		defSection = NULL;
		return FALSE;
	}
	return TRUE;
}

BOOL ConfigW::SetIniFile(const wchar_t *file)
{
	if (fileName) 
	{
		if (emptyBOM) RemoveEmptyFile();
		free(fileName); 
		fileName = NULL;
	}
	size_t len;
	len = lstrlen(file) + 1;
	fileName = (wchar_t*)malloc(len*sizeof(wchar_t));
	if (NULL == fileName) return FALSE;
	
	if (S_OK != StringCchCopy(fileName, len, file))
	{
		free(fileName); 
		fileName = NULL;
		return FALSE;
	}
    if (fileName) CreateFileWithBOM();
	return TRUE;
}

const wchar_t* ConfigW::GetSection(void)
{
	return defSection;
}

const wchar_t* ConfigW::GetFile(void)
{
	return fileName;
}

void ConfigW::CreateFileWithBOM(void)
{
	// benski> this doesn't seem to be working on win9x
	HANDLE hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		WORD wBOM = 0xFEFF;
		DWORD num = 0;
		hFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		WriteFile(hFile, &wBOM, sizeof(WORD), &num, NULL);
		emptyBOM = TRUE;
	}
	CloseHandle(hFile);
}

void ConfigW::RemoveEmptyFile(void)
{	
	emptyBOM = FALSE;
	HANDLE hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile) return;
	DWORD fsize;
	fsize = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	if (fsize == 2) DeleteFile(fileName);
	return;
}

BOOL ConfigW::IsFileExist(void)
{
	HANDLE hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	BOOL exist = (INVALID_HANDLE_VALUE != hFile);
	if (exist)
	{
		exist = (GetFileSize(hFile, NULL) != 2);
		CloseHandle(hFile);
	}
	return exist;	
}