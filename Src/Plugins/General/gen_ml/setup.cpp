#include "./main.h"
#include "../winamp/setup/svc_setup.h"

#include <shlwapi.h>

EXTERN_C _declspec(dllexport) BOOL RegisterSetup(HINSTANCE hInstance, api_service *waServices)
{
	WIN32_FIND_DATAW findData = {0};
	WCHAR szPath[MAX_PATH] = {0}, szBase[MAX_PATH] = {0};
	
	if (0 == GetModuleFileNameW(hInstance, szBase, ARRAYSIZE(szBase)))
		return 0;
	
	PathRemoveFileSpecW(szBase);
	PathCombineW(szPath, szBase, L"ml_*.dll");

	
	HANDLE hFind = FindFirstFileW(szPath, &findData);
	if (INVALID_HANDLE_VALUE == hFind)
		return FALSE;
	
	do
	{
		PathCombineW(szPath, szBase, findData.cFileName);
		HINSTANCE hLib = LoadLibraryW(szPath);
		if (NULL != hLib)
		{
			Plugin_RegisterSetup fn = (Plugin_RegisterSetup)GetProcAddress(hLib, "RegisterSetup");
			if (NULL == fn || FALSE == fn(hLib, waServices))
			{
				FreeModule(hLib);
			}
		}
	}
	while (FindNextFileW(hFind, &findData));
	FindClose(hFind);
	
	return FALSE;
}