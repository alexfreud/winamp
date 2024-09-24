/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/


#include "main.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "../nu/ns_wc.h"

void LoadPathsIni()
{
	wchar_t pathsini[MAX_PATH] = {0};
	wchar_t dir[1024] = {0};

	PathCombineW(pathsini, PROGDIR, L"paths.ini");

	GetPrivateProfileStringW(L"Winamp", L"inidir", L"", dir, 1024, pathsini);
	if (dir[0])
	{
		ResolveEnvironmentVariables(dir, 1024);
		config_setinidir(dir);
	}

	GetPrivateProfileStringW(L"Winamp", L"m3udir", L"", dir, 1024, pathsini);
	if (dir[0])
	{
		ResolveEnvironmentVariables(dir, 1024);
		config_setm3udir(dir);
	}

	GetPrivateProfileStringW(L"Winamp", L"m3ubase", L"", dir, 1024, pathsini);
	if (dir[0])
	{
		ResolveEnvironmentVariables(dir, 1024);
		config_setm3ubase(dir);
	}

	GetPrivateProfileStringW(L"Winamp", L"inifile", L"", dir, 1024, pathsini);
	if (dir[0])
	{
		ResolveEnvironmentVariables(dir, 1024);
		config_setinifile(dir);
	}

	GetPrivateProfileStringW(L"Winamp", L"class", L"", dir, 1024, pathsini);
	if (dir[0])
	{
		ResolveEnvironmentVariables(dir, 1024);
		StringCchCopyW(szAppName, 64, dir);
	}

	GetPrivateProfileStringW(L"Winamp", L"cwd", L"", dir, 1024, pathsini);
	if (dir[0])
	{
		ResolveEnvironmentVariables(dir, 1024);
		StringCchCopyW(config_cwd, MAX_PATH, dir);
		//MultiByteToWideCharSZ(CP_ACP, 0, dir, -1, config_cwd, MAX_PATH);
	}
}