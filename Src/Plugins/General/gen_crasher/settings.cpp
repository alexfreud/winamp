#include ".\settings.h"
#include <shlwapi.h>
#include <strsafe.h>

Settings::Settings(void)
{
	dumpPath = NULL;
	logPath = NULL;
	smtpServer = NULL;
	smtpUser = NULL;
	smtpPwd = NULL;
	path = NULL;
	smtpAddress = NULL;
	updatePath = TRUE;
	createDMP = TRUE;
	createLOG = TRUE;
	autoRestart = FALSE;
	silentMode = TRUE;
	sendData = TRUE;
	zipData = TRUE;
	zipPath = NULL;
	sendByClient = TRUE;
	sendBySMTP = FALSE;
	smtpPort = 25;
	smtpAuth = TRUE;
	dumpType = 0;
	logSystem = TRUE;
	logRegistry = TRUE;
	logStack = TRUE;
	logModule = TRUE;
}

Settings::~Settings(void)
{
	if (dumpPath) free(dumpPath);
	if (logPath) free(logPath);
	if (smtpServer) free(smtpServer);
	if (smtpUser) free(smtpUser);
	if (smtpPwd) free(smtpPwd);
	if (path) free(path);
	if (smtpAddress) free(smtpAddress);
}

void Settings::SetPath(wchar_t *iniPath)
{
	size_t size = lstrlen(iniPath);
	if (path) free(path);
	path = NULL;
	path = (wchar_t*)malloc((size + 1) * sizeof(wchar_t));
	StringCchCopy(path, size+1, iniPath);
	wchar_t iniFile[MAX_PATH*2] = {0};
	size += 14 * sizeof(wchar_t);
	CreateDirectory(iniPath, NULL);
	StringCchPrintf(iniFile, size, L"%s\\feedback.ini", iniPath);
	cfg.SetIniFile(iniFile);
}

const wchar_t* Settings::GetPath(void)
{
	return path;
}

BOOL Settings::Load(void)
{
	if (!cfg.IsFileExist()) return FALSE;
    cfg.SetSection(L"General");
	updatePath = cfg.ReadInt(L"UpdatePath", TRUE);
	if (updatePath) return FALSE;
	createDMP = cfg.ReadInt(L"CreateDmp", TRUE);
	createLOG = cfg.ReadInt(L"CreateLog", TRUE);
	autoRestart = cfg.ReadInt(L"AutoRestart", FALSE);
	silentMode = cfg.ReadInt(L"SilentMode", TRUE);
	sendData = cfg.ReadInt(L"SendData", TRUE);
	
	cfg.SetSection(L"Send");
	sendByClient =  cfg.ReadInt(L"UseClient", TRUE);
    sendBySMTP = cfg.ReadInt(L"UseSMTP", FALSE);
	smtpPort = cfg.ReadInt(L"Port", 25);
	smtpAuth = cfg.ReadInt(L"ReqAuth", TRUE);
	CreateStrCopy(&smtpAddress, cfg.ReadStringW(L"Address", L"bug@winamp.com"));
	CreateStrCopy(&smtpServer, cfg.ReadStringW(L"Server", NULL));
	CreateStrCopy(&smtpUser, cfg.ReadStringW(L"User", NULL));
	CreateStrCopy(&smtpPwd, cfg.ReadStringW(L"Pwd", NULL));

	cfg.SetSection(L"Zip");
	zipData =  cfg.ReadInt(L"ZipData", TRUE);
	CreateStrCopy(&zipPath, cfg.ReadStringW(L"Path", NULL));

	cfg.SetSection(L"Dump");
	dumpType = cfg.ReadInt(L"Type", 0);
	CreateStrCopy(&dumpPath, cfg.ReadStringW(L"Path", NULL));

	cfg.SetSection(L"Log");
	logSystem = cfg.ReadInt(L"System", TRUE);
	logRegistry = cfg.ReadInt(L"Registry", TRUE);
	logStack = cfg.ReadInt(L"Stack", TRUE);
	logModule = cfg.ReadInt(L"Module", TRUE);
	CreateStrCopy(&logPath, cfg.ReadStringW(L"Path", NULL));
	return TRUE;
}

void Settings::CreateStrCopy(wchar_t **dest, const wchar_t* source)
{
	if (*dest) free(*dest);
	*dest = NULL;
	if (source)
	{
		size_t len = lstrlen(source) + 1;
		*dest = (wchar_t*) malloc(len*sizeof(wchar_t));
		StringCchCopy(*dest, len, source);
	}
}

BOOL Settings::Save(void)
{
	BOOL error = FALSE;
	if (FALSE == cfg.SetSection(L"General")) error = TRUE;
	if (FALSE == cfg.Write(L"UpdatePath", FALSE)) error = TRUE;
	if (FALSE == cfg.Write(L"CreateDmp", createDMP)) error = TRUE;
	if (FALSE == cfg.Write(L"CreateLog", createLOG)) error = TRUE;
	if (FALSE == cfg.Write(L"AutoRestart", autoRestart)) error = TRUE;
	if (FALSE == cfg.Write(L"SilentMode", silentMode)) error = TRUE;
	if (FALSE == cfg.Write(L"SendData", sendData)) error = TRUE;
	if (FALSE == cfg.SetSection(L"Send")) error = TRUE;
	if (FALSE == cfg.Write(L"UseClient", sendByClient)) error = TRUE;
	if (FALSE == cfg.Write(L"UseSMTP", sendBySMTP)) error = TRUE;
    if (FALSE == cfg.Write(L"Port", smtpPort)) error = TRUE;
	if (FALSE == cfg.Write(L"Server", smtpServer)) error = TRUE;
	if (FALSE == cfg.Write(L"Address", smtpAddress)) error = TRUE;
	if (FALSE == cfg.Write(L"ReqAuth", smtpAuth)) error = TRUE;
	if (FALSE == cfg.Write(L"User", smtpUser)) error = TRUE;
	if (FALSE == cfg.Write(L"Pwd", smtpPwd)) error = TRUE;
	if (FALSE == cfg.SetSection(L"Zip")) error = TRUE;
	if (FALSE == cfg.Write(L"ZipData", zipData)) error = TRUE;
	if (FALSE == cfg.Write(L"Path", zipPath)) error = TRUE;
	if (FALSE == cfg.SetSection(L"Dump")) error = TRUE;
	if (FALSE == cfg.Write(L"Type", dumpType)) error = TRUE;
	if (FALSE == cfg.Write(L"Path", dumpPath)) error = TRUE;
	if (FALSE == cfg.SetSection(L"Log")) error = TRUE;
	if (FALSE == cfg.Write(L"System", logSystem)) error = TRUE;
	if (FALSE == cfg.Write(L"Registry", logRegistry)) error = TRUE;
	if (FALSE == cfg.Write(L"Stack", logStack)) error = TRUE;
	if (FALSE == cfg.Write(L"Module", logModule)) error = TRUE;
	if (FALSE == cfg.Write(L"Path", logPath)) error = TRUE;
	return !error;
}

BOOL Settings::CreateDefault(wchar_t* iniPath)
{
	wchar_t temp[MAX_PATH] = {0};
	int len;

	createDMP = TRUE;
	createLOG = TRUE;
	autoRestart = FALSE;
	silentMode = TRUE;
	sendData = TRUE;
// zip 
	PathCombine(temp, iniPath, L"report.zip");
	len = (int)wcslen(temp) + 1;
	zipData = TRUE;
	zipPath = (wchar_t*) malloc(len*2);
	StringCchCopy(zipPath, len, temp);
// send	
	sendByClient = TRUE;
	sendBySMTP = FALSE;
	smtpPort = 25;
	smtpAddress = (wchar_t*) malloc(32*2);
	StringCchCopy(smtpAddress, 32, L"bug@winamp.com");
	smtpAuth = TRUE;
	smtpServer = NULL;
	smtpUser = NULL;
	smtpPwd = NULL;
// dump
	PathCombine(temp, iniPath, L"_crash.dmp");
	len = (int)wcslen(temp) + 1;
	dumpType = NULL;
	dumpPath = (wchar_t*) malloc(len*2);
	StringCchCopy(dumpPath, len, temp);
// log
	logSystem = TRUE;
	logRegistry = TRUE;
	logStack = TRUE;
	logModule = TRUE;
	PathCombine(temp, iniPath, L"_crash.log");
	len = (int)wcslen(temp) + 1;
	logPath = (wchar_t*) malloc(len*2);
	StringCchCopy(logPath, len, temp);
	return TRUE;
}

BOOL Settings::IsOk(void)
{
	return (logPath != NULL && dumpPath != NULL);
}

void Settings::ClearTempData(void)
{
	cfg.Write(L"Temp", L"TS", L"");
	cfg.Write(L"Temp", L"LOG", L"0");
	cfg.Write(L"Temp", L"DMP", L"0");
}

void Settings::WriteErrorTS(const wchar_t *time)
{
	cfg.Write(L"Temp", L"TS", time);
}

void Settings::WriteLogCollectResult(BOOL result)
{
	cfg.Write(L"Temp", L"LOG", result);
}

void Settings::WriteDmpCollectResult(BOOL result)
{
	cfg.Write(L"Temp", L"DMP", result);
}

void Settings::WriteWinamp(const wchar_t *winamp)
{
	cfg.Write(L"Temp", L"WA", winamp);
}

const wchar_t* Settings::ReadErrorTS(void)
{
	return cfg.ReadStringW(L"Temp", L"TS", L"");
}

BOOL Settings::ReadLogCollectResult(void)
{
	return cfg.ReadInt(L"Temp", L"LOG", 0);
}
BOOL Settings::ReadDmpCollectResult(void)
{
	return cfg.ReadInt(L"Temp", L"DMP", 0);
}

const wchar_t* Settings::ReadWinamp(void)
{
	return cfg.ReadStringW(L"Temp", L"WA", L"");
}

void Settings::WriteBody(const wchar_t *body)
{
	cfg.Write(L"Temp", L"Body", body);
}

const wchar_t* Settings::ReadBody(void)
{
	return cfg.ReadStringW(L"Temp", L"Body", L"");
}