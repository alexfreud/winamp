#ifndef NULLSOFT_CONFIG_H_
#define NULLSOFT_CONFIG_H_

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#define BUFF_SIZE 8192

class ConfigW
{
public:
	ConfigW();
    ConfigW(const wchar_t *ini, const wchar_t *section);
    ~ConfigW();

public:
    void Flush(void);

	BOOL Write(const wchar_t *name, double value);
	BOOL Write(const wchar_t *section, const wchar_t *name, double value);
	BOOL Write(const wchar_t *name, long long value);
	BOOL Write(const wchar_t *section, const wchar_t *name, long long value);
	BOOL Write(const wchar_t *name, int value);
	BOOL Write(const wchar_t *section, const wchar_t *name, int value);
	BOOL Write(const wchar_t *name, const wchar_t *value);
	BOOL Write(const wchar_t *section, const wchar_t *name, const wchar_t *value);
	BOOL Write(const wchar_t *name, const char value);
	BOOL Write(const wchar_t *section, const wchar_t *name, const char *value);

    int ReadInt(const wchar_t *name, int defvalue);
	long long ReadInt64(const wchar_t *name, long long defvalue);
	double ReadDouble(const wchar_t *name, double defvalue);
	const char* ReadStringA(const wchar_t *name, const char *defvalue);
	const wchar_t* ReadStringW(const wchar_t *name, const wchar_t *defvalue);
    int ReadInt(const wchar_t *section, const wchar_t *name, int defvalue);
	long long ReadInt64(const wchar_t *section, const wchar_t *name, long long defvalue);
	double ReadDouble(const wchar_t *section, const wchar_t *name, double defvalue);
	const char* ReadStringA(const wchar_t *section, const wchar_t *name, const char *defvalue);
	const wchar_t* ReadStringW(const wchar_t *section, const wchar_t *name, const wchar_t *defvalue);

	BOOL SetSection(const wchar_t *section);
	BOOL SetIniFile(const wchar_t *file);
	BOOL IsFileExist(void);

	const wchar_t* GetSection(void);
	const wchar_t* GetFile(void);
private:
	HANDLE CreateFileHandle();
	void CreateFileWithBOM(void);
	void RemoveEmptyFile(void);
private:
	BOOL emptyBOM;
    wchar_t	buff[BUFF_SIZE];
	char	*buffA;
    wchar_t *fileName;
	wchar_t *defSection;
};

#endif	//NULLSOFT_CONFIG_H_