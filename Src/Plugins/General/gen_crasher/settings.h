#pragma once

#include "config.h"

class Settings
{
public:
	Settings(void);
	~Settings(void);

public:
	void SetPath(wchar_t *iniPath);
	BOOL Load(void);
	BOOL Save(void);
	BOOL CreateDefault(wchar_t* iniPath);
	BOOL IsOk(void);
	const wchar_t* GetPath(void);

protected:
	void CreateStrCopy(wchar_t **dest, const wchar_t* source);
private:
	ConfigW cfg;
	wchar_t* path;

public:
// general 
	BOOL updatePath;
	BOOL createDMP;
	BOOL createLOG;
	BOOL autoRestart;
	BOOL silentMode;
	BOOL sendData;
//zip
	BOOL zipData;
	wchar_t* zipPath;
// send	
	BOOL sendByClient;
	BOOL sendBySMTP;
	int smtpPort;
	wchar_t *smtpServer;
	wchar_t *smtpAddress;
	BOOL smtpAuth;
	wchar_t *smtpUser;
	wchar_t *smtpPwd;
// dump
	int dumpType;
	wchar_t *dumpPath;
// log
	BOOL logSystem;
	BOOL logRegistry;
	BOOL logStack;
	BOOL logModule;
	wchar_t *logPath;
// tmp
	void ClearTempData(void);
	void WriteErrorTS(const wchar_t *time);
	void WriteLogCollectResult(BOOL result);
	void WriteDmpCollectResult(BOOL result);
	void WriteWinamp(const wchar_t *winamp);
	void WriteBody(const wchar_t *body);

	const wchar_t* ReadErrorTS(void);
	BOOL ReadLogCollectResult(void);
	BOOL ReadDmpCollectResult(void);
	const wchar_t* ReadWinamp(void);
	const wchar_t* ReadBody(void);
};