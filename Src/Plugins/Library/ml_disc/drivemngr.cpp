#include "main.h"
#include "./drivemngr.h"
#include "./primosdk_helper.h"
#include "./resource.h"
#include "../nu/trace.h"
#include "./dbt.h"
#include "./spti.h"
#include <setupapi.h>

#include <imapi.h>
#include <strsafe.h>

#define LISTENER_CLASSNAME		L"MLDISCLISTENER"
#define LISTENER_WINDOWNAME		L""

#define POLLMEDIUMCHANGE_INTERVAL		2000
#define POLLMEDIUMVALIDATE_INTERVAL		6000

#define DMS_SUSPENDED	0x0000
#define DMS_ACTIVE		0x0001

#define WM_EX_QUIT		(WM_APP + 1)

typedef struct _MEDIUMINFO_I
{
	UINT	msLastPolled;	// last time medium info was polled
	UINT	serialNumber;	// medium serialnumber
} MEDIUMINFO_I;

typedef struct _DRIVEINFO_I
{
	char		cLetter;			// drive letter
	INT			deviceNumber;		// system assigned device number (unique till next reboot)
	BOOL		bMediumInserted;		// if TRUE mediumInfo contains valid data
	CHAR		cMode;				// drive mode
	DWORD		dwType;				// drive type
	LPWSTR		pszDevName;			// device name
	HANDLE		hThread;			// device info thread 
	DWORD		dwThreadId;
	MEDIUMINFO_I	mediumInfo;
} DRIVEINFO_I;

typedef struct _DRIVEMNGR
{
	HWND					hwndListener;
	DMNPROC				callback;
	UINT				fState;
	CRITICAL_SECTION	csLock;

	DRIVEINFO_I	*pDrives;
	INT			nCount;
	INT			nAlloc;

	HANDLE		hPollingThread;
	DWORD		dwPollingThread;
} DRIVEMNGR;

typedef struct _DEVICEINFO
{
	CHAR	cLetter;
	LPWSTR	pszDevName;
	WCHAR	szTargetPath[128];
	WCHAR	szVolumeName[64];
	DWORD	dwType;
	INT		deviceNumber;
	INT		opCode;
} DEVICEINFO;

static DRIVEMNGR *pMngr = NULL;


static void CALLBACK APC_CheckDrives(ULONG_PTR param);
static void CALLBACK APC_IsMediumChanged(ULONG_PTR param);

static void CALLBACK APC_GetUnitInfo(ULONG_PTR param);
static void CALLBACK APC_GetUnitInfo2(ULONG_PTR param);
static void CALLBACK APC_GetDiscInfoEx(ULONG_PTR param);
static void CALLBACK APC_GetDiscInfo2(ULONG_PTR param);
static void CALLBACK APC_GetTitle(ULONG_PTR param);
static void CALLBACK APC_DriveScan(ULONG_PTR param);
static void CALLBACK APC_GetMCIInfo(ULONG_PTR param);
static void CALLBACK APC_GetIMAPIInfo(ULONG_PTR param);
static void CALLBACK APC_Eject(ULONG_PTR param);

static DWORD CALLBACK InfoThread(LPVOID param);

static DWORD CALLBACK PollingThread(LPVOID param);

static void CALLBACK PollMediumInfo(ULONG_PTR param);

static LRESULT WINAPI ListenerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static CHAR CheckLetter(CHAR cLetter)
{
	if (cLetter < 'A' || cLetter > 'Z')
	{
		if (cLetter >= 'a' && cLetter <= 'z') return (cLetter - 0x20);
		return 0;
	}
	return cLetter;
}

static LPCWSTR GetDeviceName(CHAR cLetter)
{
	LPCWSTR pszDevName;
	if (!pMngr) return NULL;
	pszDevName = NULL;

	EnterCriticalSection(&pMngr->csLock);
	for (int i = 0; i < pMngr->nCount; i++)
	{
		if (pMngr->pDrives[i].cLetter == cLetter) 
		{
			pszDevName = pMngr->pDrives[i].pszDevName;
			break;
		}
	}
	LeaveCriticalSection(&pMngr->csLock);

	return pszDevName;
}

static BOOL IsPollingRequired(void)
{
	HKEY hKey;
	LONG result;
	BOOL bAutoRunEnabled;

	bAutoRunEnabled = FALSE;

	result = RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\Cdrom"), &hKey);
	if (ERROR_SUCCESS == result)
	{
		DWORD value;
		DWORD size;
		size = sizeof(DWORD);
		result = RegQueryValueEx(hKey, TEXT("AutoRun"), NULL, NULL, (LPBYTE)&value, &size);
		if (ERROR_SUCCESS == result) bAutoRunEnabled = (0 != value);

		RegCloseKey(hKey);
	}
	return !bAutoRunEnabled;

}
static CHAR Drive_LetterFromMask(ULONG unitmask)
{
   char i;
   for (i = 0; i < 26; ++i)
   {
      if (unitmask & 0x1)  break;
      unitmask = unitmask >> 1;
   }
   return (i + 'A');
}

static BOOL Drive_Add(DEVICEINFO *pDevInfo)
{
	DRIVEINFO_I *pDrive;
	if (!pMngr) return FALSE;

	EnterCriticalSection(&pMngr->csLock);

	INT index, opCode;

	opCode = 0;
	for (index = 0; index < pMngr->nCount && pMngr->pDrives[index].cLetter != pDevInfo->cLetter; index++);
	if (index != pMngr->nCount) 
	{
		pDrive = &pMngr->pDrives[index];
		if (pDrive->deviceNumber != pDevInfo->deviceNumber || pDrive->dwType != pDevInfo->dwType) opCode = 1;
	}
	else
	{
		if (pMngr->nCount == pMngr->nAlloc)
		{
			LPVOID	data;
			data = realloc(pMngr->pDrives, sizeof(DRIVEINFO_I)*(pMngr->nCount + 2));
			if (!data) 
			{
				LeaveCriticalSection(&pMngr->csLock);
				return FALSE;
			}
			pMngr->pDrives = (DRIVEINFO_I*)data;
			pMngr->nAlloc += 2;
		}
		pDrive = &pMngr->pDrives[pMngr->nCount];
		pMngr->nCount++;

		ZeroMemory(pDrive, sizeof(DRIVEINFO_I));
		pDrive->cLetter = pDevInfo->cLetter;
		opCode = 2;
	}

	if (opCode)
	{
		pDrive->deviceNumber		= pDevInfo->deviceNumber;
		pDrive->dwType			= pDevInfo->dwType;
		if (pDrive->pszDevName) free(pDrive->pszDevName);
		pDrive->pszDevName		= _wcsdup(pDevInfo->pszDevName);
	}

	LeaveCriticalSection(&pMngr->csLock);

	if (opCode && pMngr->callback) pMngr->callback((2 == opCode) ? DMW_DRIVEADDED : DMW_DRIVECHANGED, pDevInfo->cLetter);

	return TRUE;
}

static BOOL Drive_Remove(CHAR cLetter)
{
	INT index;
	BOOL bReportChanges;
	if (!pMngr) return FALSE;

	EnterCriticalSection(&pMngr->csLock);

	bReportChanges = FALSE;
	index = pMngr->nCount;
	while (index-- && pMngr->pDrives[index].cLetter != cLetter);

	if (-1 != index)
	{
		if (pMngr->pDrives[index].pszDevName) free(pMngr->pDrives[index].pszDevName);
		if (index != pMngr->nCount - 1) MoveMemory(&pMngr->pDrives[index], &pMngr->pDrives[index + 1], sizeof(DRIVEINFO_I)*(pMngr->nCount - index));
		pMngr->nCount--;
		bReportChanges = TRUE;
	}
	LeaveCriticalSection(&pMngr->csLock);

	if (bReportChanges && pMngr->callback) pMngr->callback(DMW_DRIVEREMOVED, cLetter);
	return TRUE;
}

static HRESULT QueueInfoAPC(CHAR cLetter, PAPCFUNC pfnAPC, ULONG_PTR param)
{
	DWORD *pdwThreadId(NULL);
	HRESULT hr(S_FALSE);
	HANDLE *phThread(NULL);
	static HANDLE hDrivesInfoThread = NULL;

	if (NULL == pMngr) return E_FAIL;

	EnterCriticalSection(&pMngr->csLock);
	if (cLetter)
	{
		INT index = pMngr->nCount;
		while (index-- && pMngr->pDrives[index].cLetter != cLetter);
		if (-1 != index) 
		{
			phThread = &pMngr->pDrives[index].hThread;
			pdwThreadId = &pMngr->pDrives[index].dwThreadId;
		}
	}
	else 
	{
		phThread = &hDrivesInfoThread;
	}

	if (phThread)
	{
		if (!*phThread)
		{
			DWORD tid;
			*phThread = CreateThread(NULL, 0, InfoThread, NULL, 0, &tid);
			if (pdwThreadId) *pdwThreadId = tid;
			Sleep(100);
		}
		if (*phThread) 
		{
			if (0 == QueueUserAPC(pfnAPC, *phThread, param))
			{
				TRACE_LINE(TEXT("queue user apc failed"));
			}
			else hr = S_OK;
		}
	}

	LeaveCriticalSection(&pMngr->csLock);

	return hr;
}

static BOOL Medium_Add(CHAR cLetter, DWORD serial)
{
	INT index;
	if (!pMngr) return FALSE;

	EnterCriticalSection(&pMngr->csLock);

	index = pMngr->nCount;
	while (index-- && pMngr->pDrives[index].cLetter != cLetter);
	if (-1 != index) 
	{ 
		pMngr->pDrives[index].bMediumInserted			= TRUE; 
		pMngr->pDrives[index].mediumInfo.msLastPolled	= 0;
		pMngr->pDrives[index].mediumInfo.serialNumber	= serial;
	}

	LeaveCriticalSection(&pMngr->csLock);

	if (-1 != index)
	{
		if (-1 == serial) QueueInfoAPC(cLetter, APC_IsMediumChanged, (ULONG_PTR)cLetter); 
		if (pMngr->callback) pMngr->callback(DMW_MEDIUMARRIVED, cLetter);
	}

	return TRUE;
}

static BOOL Medium_Remove(CHAR cLetter)
{
	INT index;
	if (!pMngr) return FALSE;

	EnterCriticalSection(&pMngr->csLock);

	index = pMngr->nCount;
	while (index-- && pMngr->pDrives[index].cLetter != cLetter);
	if (-1 != index) pMngr->pDrives[index].bMediumInserted = FALSE;

	LeaveCriticalSection(&pMngr->csLock);

	if (-1 != index && pMngr->callback) pMngr->callback(DMW_MEDIUMREMOVED, cLetter);

	return TRUE;
}

BOOL DriveManager_Initialize(DMNPROC DMNProc, BOOL bSuspended)
{
	WNDCLASSW wc = {0};
	HINSTANCE hInstance;

	if (pMngr || !DMNProc) return FALSE;

	pMngr = (DRIVEMNGR*)calloc(1, sizeof(DRIVEMNGR));
	if (!pMngr) return FALSE;

	hInstance = GetModuleHandle(NULL);

	if (!GetClassInfoW(hInstance, LISTENER_CLASSNAME, &wc))
	{
		wc.hInstance = hInstance;
		wc.lpfnWndProc = ListenerWndProc;
		wc.lpszClassName = LISTENER_CLASSNAME;
		if (!RegisterClassW(&wc)) 
		{
			DriveManager_Uninitialize(0);
			return FALSE;
		}
	}
	pMngr->hwndListener = CreateWindowW(LISTENER_CLASSNAME, LISTENER_WINDOWNAME, WS_DISABLED, 0,0,0,0, HWND_DESKTOP, NULL, hInstance, 0L);
	if (!pMngr->hwndListener) 
	{
		DriveManager_Uninitialize(0);
		return FALSE;
	}
	InitializeCriticalSection(&pMngr->csLock);
	pMngr->callback = DMNProc;

	return TRUE;
}

BOOL DriveManager_Uninitialize(INT msExitWaitTime)
{
	if (pMngr)
	{
		WNDCLASSW wc;
		HINSTANCE hInstance;

		if (pMngr->hwndListener) DestroyWindow(pMngr->hwndListener);

		hInstance = GetModuleHandle(NULL);
		if (GetClassInfoW(hInstance, LISTENER_CLASSNAME, &wc)) UnregisterClassW(LISTENER_CLASSNAME, hInstance);

		EnterCriticalSection(&pMngr->csLock);

		for (int index =0; index < pMngr->nCount; index++) 
		{
			if (pMngr->pDrives[index].hThread)
			{
				PostThreadMessage(pMngr->pDrives[index].dwThreadId, WM_EX_QUIT, 1, 0); 
				INT result = WaitForSingleObject(pMngr->pDrives[index].hThread, msExitWaitTime);
				if (WAIT_TIMEOUT == result) TerminateThread(pMngr->pDrives[index].hThread, 1);
				CloseHandle(pMngr->pDrives[index].hThread);
				pMngr->pDrives[index].hThread = NULL;
			}
		}
		LeaveCriticalSection(&pMngr->csLock);

		if (pMngr->hPollingThread)
		{
			PostThreadMessage(pMngr->dwPollingThread, WM_EX_QUIT, 1, 0); 
			INT result = WaitForSingleObject(pMngr->hPollingThread, msExitWaitTime);
			if (WAIT_TIMEOUT == result) TerminateThread(pMngr->hPollingThread, 1);
			CloseHandle(pMngr->hPollingThread);
			pMngr->hPollingThread = NULL;
		}

		EnterCriticalSection(&pMngr->csLock);

		if (pMngr->pDrives)
		{
			free(pMngr->pDrives);
		}

		DRIVEMNGR *managerInstance = pMngr;
		pMngr = NULL;

		LeaveCriticalSection(&managerInstance->csLock);
		DeleteCriticalSection(&managerInstance->csLock);

		free(managerInstance);

		PrimoSDKHelper_Uninitialize();

	}
	return TRUE;
}

BOOL DriveManager_Suspend(void)
{
	if (!pMngr) return FALSE;

	pMngr->fState = DMS_SUSPENDED; 
	if (pMngr->hPollingThread) 
	{
		PostThreadMessage(pMngr->dwPollingThread, WM_EX_QUIT, 1, 0);
		pMngr->hPollingThread = NULL;
	}
	return TRUE;
}

BOOL DriveManager_Update(BOOL bAsync)
{
	if (bAsync) return (QueueInfoAPC(0, APC_DriveScan, 0) && QueueInfoAPC(0, PollMediumInfo, 0));
	else
	{
		APC_DriveScan(0);
		QueueInfoAPC(0, PollMediumInfo, 0);
	}
	return TRUE;
}

BOOL DriveManager_Resume(BOOL bUpdate)
{
	if (!pMngr) return FALSE;
	pMngr->fState = DMS_ACTIVE;

	EnterCriticalSection(&pMngr->csLock);
	for (int index =0; index < pMngr->nCount; index++) pMngr->pDrives[index].mediumInfo.msLastPolled = 0;
	LeaveCriticalSection(&pMngr->csLock);

	APC_DriveScan(0);
	QueueInfoAPC(0, PollMediumInfo, 0);

	if (NULL == pMngr->hPollingThread && IsPollingRequired())
	{
		pMngr->hPollingThread = CreateThread(NULL, 0, PollingThread, NULL, 0, &pMngr->dwPollingThread);
	}

	return TRUE;
}

BOOL DriveManager_SetDriveMode(CHAR cLetter, CHAR cMode)
{
	BOOL report;
	INT index;

	index = -1;
	report = FALSE;
	cLetter = CheckLetter(cLetter);

	if (pMngr && cLetter)
	{
		EnterCriticalSection(&pMngr->csLock);
		for (index =0; index < pMngr->nCount; index++) 
		{
			if (pMngr->pDrives[index].cLetter == cLetter)
			{
				if (pMngr->pDrives[index].cMode != cMode) 
				{
					pMngr->pDrives[index].cMode = cMode;
					report = TRUE;
				}
				break;
			}
		}
		if (index == pMngr->nCount) index = -1;
		LeaveCriticalSection(&pMngr->csLock);
		if (report && pMngr->callback) pMngr->callback(DMW_MODECHANGED, MAKEWORD(cLetter, cMode));
	}

	return (-1 != index);
}

CHAR DriveManager_GetDriveMode(CHAR cLetter)
{
	CHAR result;

	result = DM_MODE_ERROR;
	cLetter = CheckLetter(cLetter);

	if (pMngr && cLetter) 
	{
		INT index;
		EnterCriticalSection(&pMngr->csLock);
		for (index =0; index < pMngr->nCount; index++) 
		{
			if (pMngr->pDrives[index].cLetter == cLetter)
			{
				result = pMngr->pDrives[index].cMode;
				break;
			}
		}
		LeaveCriticalSection(&pMngr->csLock);
	}
	return result;
}

DWORD DriveManager_GetDriveType(CHAR cLetter)
{
	DWORD type;

	type = DRIVE_TYPE_UNKNOWN | DRIVE_CAP_UNKNOWN;
	cLetter = CheckLetter(cLetter);

	if (pMngr && cLetter) 
	{
		INT index;
		EnterCriticalSection(&pMngr->csLock);
		for (index =0; index < pMngr->nCount; index++) 
		{
			if (pMngr->pDrives[index].cLetter == cLetter)
			{
				type = pMngr->pDrives[index].dwType;
				break;
			}
		}
		LeaveCriticalSection(&pMngr->csLock);
	}
	return type;
}

BOOL DriveManager_IsMediumInserted(CHAR cLetter)
{
	BOOL result;

	result = FALSE;
	cLetter = CheckLetter(cLetter);

	if (pMngr && cLetter) 
	{
		INT index;
		EnterCriticalSection(&pMngr->csLock);
		for (index =0; index < pMngr->nCount; index++) 
		{
			if (pMngr->pDrives[index].cLetter == cLetter)
			{
				result = pMngr->pDrives[index].bMediumInserted;
				break;
			}
		}
		LeaveCriticalSection(&pMngr->csLock);
	}
	return result;
}

INT DriveManager_GetDriveList(CHAR *pLetters, INT cchSize)
{
	INT r = 0;
	if (!pLetters || !pMngr) return -1;
	EnterCriticalSection(&pMngr->csLock);
	for (int index =0; index < pMngr->nCount; index++) 
	{
		*pLetters = pMngr->pDrives[index].cLetter;
		pLetters++;
		cchSize--;
		r++;
		if (0 == cchSize) break;
	}
	LeaveCriticalSection(&pMngr->csLock);
	return r;
}

static BOOL QueueInfoJob(PAPCFUNC pfnAPC, DM_NOTIFY_PARAM *pHeader)
{
	BOOL result(TRUE);
	if (!pMngr || !pHeader) result = FALSE;

	if (result)
	{
		HANDLE hProc = GetCurrentProcess();
		pHeader->hReserved = 0;		
		result = (BOOL)DuplicateHandle(hProc, GetCurrentThread(), hProc, &pHeader->hReserved,
									0, FALSE, DUPLICATE_SAME_ACCESS);
		if (!result) pHeader->hReserved = 0;
	}

	if (result)
	{
		CHAR cLetter = CheckLetter(pHeader->cLetter);
		result = (cLetter && S_OK == QueueInfoAPC(cLetter, pfnAPC, (ULONG_PTR)pHeader));
	}

	if(!result && pHeader && pHeader->fnFree)
	{
		if (pHeader->hReserved) CloseHandle(pHeader->hReserved);
		pHeader->fnFree(pHeader);
	}
	return result;
}

BOOL DriveManager_GetUnitInfo(DM_UNITINFO_PARAM *puip)
{
	return QueueInfoJob(APC_GetUnitInfo, (DM_NOTIFY_PARAM*)puip);
}

BOOL DriveManager_GetUnitInfo2(DM_UNITINFO2_PARAM *puip)
{
	return QueueInfoJob(APC_GetUnitInfo2, (DM_NOTIFY_PARAM*)puip);
}

BOOL DriveManager_GetDiscInfoEx(DM_DISCINFOEX_PARAM *pdip)
{
	return QueueInfoJob(APC_GetDiscInfoEx, (DM_NOTIFY_PARAM*)pdip);
}
BOOL DriveManager_GetDiscInfo2(DM_DISCINFO2_PARAM *pdip)
{
	return QueueInfoJob(APC_GetDiscInfo2, (DM_NOTIFY_PARAM*)pdip);
}

BOOL DriveManager_QueryTitle(DM_TITLE_PARAM *pdtp)
{
	return QueueInfoJob(APC_GetTitle, (DM_NOTIFY_PARAM*)pdtp);
}

BOOL DriveManager_GetMCIInfo(DM_MCI_PARAM *pmcip)
{
	return QueueInfoJob(APC_GetMCIInfo, (DM_NOTIFY_PARAM*)pmcip);
}

BOOL DriveManager_GetIMAPIInfo(DM_IMAPI_PARAM *pIMAPI)
{
	return QueueInfoJob(APC_GetIMAPIInfo, (DM_NOTIFY_PARAM*)pIMAPI);
}
BOOL DriveManager_Eject(CHAR cLetter, INT nCmd)
{
	if (!pMngr) return FALSE;
	CHAR cLetter1 = CheckLetter(cLetter);

	return (cLetter1 && QueueInfoAPC(cLetter1, APC_Eject, (ULONG_PTR)MAKELONG(cLetter, nCmd)));
}

BOOL DriveManager_IsUnitReady(CHAR cLetter, BOOL *pbReady)
{
	BYTE sc, asc, ascq;

	BOOL bSuccess;
	HANDLE hDevice;

	*pbReady = FALSE;
	hDevice = CreateFileW(GetDeviceName(cLetter), GENERIC_READ | GENERIC_WRITE, 
									FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

	if (INVALID_HANDLE_VALUE == hDevice) return FALSE;

	bSuccess = SPTI_TestUnitReady(hDevice, &sc, &asc, &ascq, 3); 
	if (!bSuccess)
	{
		if (ERROR_SEM_TIMEOUT == GetLastError()) bSuccess = TRUE;
	}
	else if (0x00 == sc || (0x02 == sc && 0x3A == asc)) *pbReady = TRUE;

	CloseHandle(hDevice);

	return bSuccess;
}

static BOOL GetVolumeNameForVolumeMountPoint_DL(LPCWSTR lpszVolumeMountPoint, LPWSTR lpszVolumeName, DWORD cchBufferLength)
{
	static BOOL (WINAPI *func)(LPCWSTR, LPWSTR, DWORD)  = NULL;
	if (!func)
	{
		UINT prevErrorMode;
		HMODULE hModule;
		prevErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
		hModule = LoadLibraryW(L"Kernel32.dll");
		SetErrorMode(prevErrorMode);
		if (hModule) 
		{
			func = (BOOL (WINAPI*)(LPCWSTR, LPWSTR, DWORD))GetProcAddress(hModule, "GetVolumeNameForVolumeMountPointW");
			FreeLibrary(hModule);
		}
	}
	return (func) ? func(lpszVolumeMountPoint, lpszVolumeName, cchBufferLength) : FALSE;
}

static DWORD GetDeviceNames(DEVICEINFO *pDevInfo, INT count)
{
	HANDLE hDevInfo;
	SP_DEVICE_INTERFACE_DATA  spiData;
	SP_DEVICE_INTERFACE_DETAIL_DATA_W  *pspiDetailData;
	DWORD dwErrorCode, dwReqSize, dwDetailSize;
	wchar_t volume[128], szDosName[] = L"X:\\", szDosName1[] = L"X:\\";

	if (!pDevInfo || !count) return ERROR_INVALID_DATA;

	for (int i = 0; i < count; i++)
	{
		szDosName[0] = pDevInfo[i].cLetter;
		GetVolumeNameForVolumeMountPoint_DL(szDosName, pDevInfo[i].szVolumeName, sizeof(pDevInfo[i].szVolumeName)/sizeof(wchar_t));
		szDosName1[0] = pDevInfo[i].cLetter;
		QueryDosDeviceW(szDosName1, pDevInfo[i].szTargetPath, sizeof(pDevInfo[i].szTargetPath)/sizeof(wchar_t));
	}

	hDevInfo = SetupDiGetClassDevs((LPGUID)&CdRomClassGuid, NULL, NULL, (DIGCF_PRESENT | DIGCF_INTERFACEDEVICE));
	if (INVALID_HANDLE_VALUE == hDevInfo) return GetLastError();

	dwDetailSize = 0;
	pspiDetailData = NULL;
	spiData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	dwErrorCode = 0;

	for (int index = 0; !dwErrorCode; index++)
	{
		BOOL bResult = SetupDiEnumDeviceInterfaces(hDevInfo, 0, (LPGUID)&CdRomClassGuid, index, &spiData);
		if (!bResult)
		{
			dwErrorCode = GetLastError();
			break;
		}

		bResult = SetupDiGetDeviceInterfaceDetailW(hDevInfo, &spiData, NULL, 0, &dwReqSize, NULL);
		if (!bResult)
		{
			dwErrorCode = GetLastError();
			if (ERROR_INSUFFICIENT_BUFFER != dwErrorCode) break;
			dwErrorCode = 0;
		}
		dwReqSize += 2*sizeof(wchar_t);
		if (dwReqSize > dwDetailSize)
		{
			LPVOID data;
			data  = realloc(pspiDetailData, dwReqSize);
			if (!data) { dwErrorCode = ERROR_NOT_ENOUGH_MEMORY; break; }
			pspiDetailData = (SP_DEVICE_INTERFACE_DETAIL_DATA*)data;
			dwDetailSize = dwReqSize;
		}

		pspiDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		bResult = SetupDiGetDeviceInterfaceDetailW(hDevInfo, &spiData, pspiDetailData, dwDetailSize, NULL, NULL);
		if (!bResult)
		{
			dwErrorCode = GetLastError();
			break;
		}

		INT cchName;
		cchName = lstrlenW(pspiDetailData->DevicePath);
		pspiDetailData->DevicePath[cchName] = L'\\';
		pspiDetailData->DevicePath[cchName + 1] = 0x00;

		if(GetVolumeNameForVolumeMountPoint_DL(pspiDetailData->DevicePath, volume, sizeof(volume)/sizeof(wchar_t)))
		{
			for (int i = 0; i < count; i++)
			{
				if (!pDevInfo[i].pszDevName && 0 == lstrcmpW(volume, pDevInfo[i].szVolumeName))
				{
					pDevInfo[i].pszDevName = (LPWSTR)calloc((cchName + 1), sizeof(wchar_t));
					if (pDevInfo[i].pszDevName) StringCchCopyNW(pDevInfo[i].pszDevName, cchName + 1, pspiDetailData->DevicePath, cchName);
					break;
				}
			}
		}
	}
	if (pspiDetailData) free(pspiDetailData);
	SetupDiDestroyDeviceInfoList(hDevInfo);

	for (int i = 0; i < count; i++)
	{
		if (!pDevInfo[i].pszDevName)
		{
			wchar_t szDevName[] = L"\\\\.\\x:";
			szDevName[4] = pDevInfo[i].cLetter;
			pDevInfo[i].pszDevName = (LPWSTR)calloc(sizeof(szDevName) + 2, sizeof(wchar_t));
			if (pDevInfo[i].pszDevName) StringCbCopyW(pDevInfo[i].pszDevName, sizeof(szDevName) + 2, szDevName);
		}
	}

	return dwErrorCode;
}

static void GetDeviceCaps(DEVICEINFO *pDevInfo, INT count)
{
	for( int i = 0; i < count; i++) 
	{
		pDevInfo[i].dwType = ((pDevInfo[i].dwType & 0x0000FFFF) | DRIVE_CAP_UNKNOWN);
	}

	// TODO come back to this later on, but for the moment not seeing any noticeable issues
	//		with disabling this and instead and instead it helps prevent random trk****.tmp
	//		files being generated and also seems to fix the crash on start people have here
	/*IDiscMaster				*pdm;
	IDiscRecorder			*pdr;
	IEnumDiscRecorders		*per;
	ULONG nActual;
	HRESULT hr = CoCreateInstance(CLSID_MSDiscMasterObj, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IDiscMaster, (void**)&pdm);
	if (SUCCEEDED(hr))
	{
		// TODO determine why this is causing trk*.tmp files to be created when called
		//		which ends up spamming the %temp% folder everytime Winamp starts :o(
		hr = pdm->Open();
		if (SUCCEEDED(hr))
		{
			IEnumDiscMasterFormats	*pef;
			hr = pdm->EnumDiscMasterFormats(&pef);
			if (SUCCEEDED(hr))
			{
				IID pFormats[2];
				hr = pef->Next(sizeof(pFormats)/sizeof(IID), pFormats, &nActual); 
				if (SUCCEEDED(hr))
				{
					while(nActual--) { if (IID_IRedbookDiscMaster == pFormats[nActual]) break; }
					if (nActual != ((ULONG)-1))
					{
						IRedbookDiscMaster *pdf;
						hr = pdm->SetActiveDiscMasterFormat(IID_IRedbookDiscMaster, (void**)&pdf);
						if (SUCCEEDED(hr))
						{
							pdf->Release();
							hr = pdm->EnumDiscRecorders(&per);
							if (SUCCEEDED(hr))
							{
								while (S_OK== per->Next(1, &pdr, &nActual) && nActual > 0)
								{
									BSTR bstrPath;
									hr = pdr->GetPath(&bstrPath);
									if (SUCCEEDED(hr))
									{
										for (int i = 0; i < count; i++)
										{
											if (0 == lstrcmpW(pDevInfo[i].szTargetPath, bstrPath))
											{
												LONG type;
												if (SUCCEEDED(pdr->GetRecorderType(&type)))
												{
													pDevInfo[i].dwType &= 0x0000FFFF;
													switch(type)
													{
														case RECORDER_CDR:	pDevInfo[i].dwType |= DRIVE_CAP_R; break;
														case RECORDER_CDRW:	pDevInfo[i].dwType |= DRIVE_CAP_RW; break;
													}
												}
												break;
											}
										}
										if (bstrPath) SysFreeString(bstrPath);
									}
									pdr->Release();
								}
								per->Release();
							}
						}
					}
				}
				pef->Release();
			}
			pdm->Close();
		}
		pdm->Release();
	}
	else
	{
	}*/
}

static void Listener_OnDeviceChange(HWND hwnd, UINT nType, DWORD_PTR dwData)
{
	DEV_BROADCAST_HDR *phdr;

	switch(nType)
	{
		case DBT_DEVICEARRIVAL:
			phdr = (DEV_BROADCAST_HDR*)dwData;
			if (DBT_DEVTYP_VOLUME == phdr->dbch_devicetype)
			{
				DEV_BROADCAST_VOLUME *pvol = (DEV_BROADCAST_VOLUME*)phdr;
				if (DBTF_MEDIA == pvol->dbcv_flags) Medium_Add(Drive_LetterFromMask(pvol->dbcv_unitmask), (DWORD)-1);
				else if (0 == pvol->dbcv_flags)
				{
					char root[] = "X:\\";
					root[0] = Drive_LetterFromMask(pvol->dbcv_unitmask);
					if (DRIVE_CDROM == GetDriveTypeA(root)) QueueInfoAPC(0, APC_CheckDrives, (ULONG_PTR)root[0]);
				}
			}
			break;
		case DBT_DEVICEREMOVECOMPLETE:
			phdr = (DEV_BROADCAST_HDR*)dwData;
			if (DBT_DEVTYP_VOLUME == phdr->dbch_devicetype)
			{
				DEV_BROADCAST_VOLUME *pvol = (DEV_BROADCAST_VOLUME*)phdr;
				if (DBTF_MEDIA == pvol->dbcv_flags) 	Medium_Remove(Drive_LetterFromMask(pvol->dbcv_unitmask)); 
				else if (0 == pvol->dbcv_flags)
				{
					char root[] = "X:\\";
					root[0] = Drive_LetterFromMask(pvol->dbcv_unitmask);
					if (DRIVE_CDROM == GetDriveTypeA(root)) Drive_Remove(root[0]);
				}
			}
			break;
	}
}

static DWORD CALLBACK InfoThread(LPVOID param)
{
	MSG msg;
	DWORD start, status, timeout, result(0);
	BOOL bComInit, run(TRUE);
	HANDLE hTemp(NULL);

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
	bComInit = ( S_OK == CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));

	timeout = 20000; // 20 seconds delay
	start = GetTickCount();

	while(run) 
	{
		DWORD elapsed = GetTickCount() - start;
		if (elapsed < timeout) 
			status = MsgWaitForMultipleObjectsEx(0, NULL, timeout - elapsed,
												QS_ALLINPUT, MWMO_WAITALL | MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
		else status = WAIT_TIMEOUT;

		switch(status)
		{
			case WAIT_FAILED:
				if (bComInit) CoUninitialize();
				return (DWORD)-1;
			case WAIT_TIMEOUT:
				if (NULL != pMngr)
				{
					EnterCriticalSection(&pMngr->csLock);
					start = GetCurrentThreadId();
					hTemp = NULL;
					
					for (int i = pMngr->nCount - 1;  i >= 0; i--)
					{
						if (pMngr->pDrives[i].dwThreadId == start)
						{
							pMngr->pDrives[i].dwThreadId = 0;
							hTemp = pMngr->pDrives[i].hThread;
							pMngr->pDrives[i].hThread = NULL;
						}
					}
					LeaveCriticalSection(&pMngr->csLock);

					//while (WAIT_IO_COMPLETION == WaitForMultipleObjectsEx(0, NULL, TRUE, 0, TRUE));
					while (SleepEx(0, TRUE) == WAIT_IO_COMPLETION);
				}
				result = 2;
				run = FALSE;
				break;
			case WAIT_IO_COMPLETION:		start = GetTickCount(); break;
			case WAIT_OBJECT_0:
				while (run && PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
				{
					switch(msg.message)
					{
						case WM_QUIT:
							result =  (DWORD)msg.wParam;
							run = FALSE;
							break;
						case WM_EX_QUIT:
							PostQuitMessage((INT)msg.wParam);
							break;
						default:
							TranslateMessage(&msg);
                            DispatchMessageW(&msg);
							break;
					}
				}
				break;
		}
	}

	if (bComInit) CoUninitialize();
	if (2 == result && hTemp) CloseHandle(hTemp);
	hTemp = NULL;
	return result;
}

static DWORD CALLBACK PollingThread(LPVOID param)
{
	MSG msg;
	DWORD status, timeout;
	BOOL bComInit;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
	bComInit = ( S_OK == CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));

	timeout = POLLMEDIUMCHANGE_INTERVAL;

	for(;;)
	{
		DWORD elapsed, start = GetTickCount();
		while ((elapsed = GetTickCount() - start) < timeout) 
		{
			status = MsgWaitForMultipleObjectsEx(0, NULL, timeout - elapsed,
													QS_ALLINPUT, MWMO_WAITALL | MWMO_ALERTABLE | MWMO_INPUTAVAILABLE);
			switch(status)
			{
				case WAIT_FAILED:	
					if (bComInit) CoUninitialize();
					return (DWORD)-1;
				case WAIT_TIMEOUT:	PollMediumInfo(0);  break;
				case WAIT_OBJECT_0:
					while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
					{
						switch(msg.message)
						{
							case WM_QUIT:
								if (bComInit) CoUninitialize();
								return (DWORD)msg.wParam;
							case WM_EX_QUIT:
								PostQuitMessage((INT)msg.wParam);
								break;
							default:
								TranslateMessage(&msg);
                                DispatchMessageW(&msg);
								break;
						}
					}
					break;
			}
		}
	}
}

static void CALLBACK PollMediumInfo(ULONG_PTR param)
{
	char letters[32] = {0};
	LPCWSTR pszDevName[32] = {0};
	INT index, count;
	if (!pMngr) return;

	count = 0;
	EnterCriticalSection(&pMngr->csLock);
	for (index =0; index < pMngr->nCount; index++) 
	{ 
		if (DM_MODE_BURNING != pMngr->pDrives[index].cMode && DM_MODE_RIPPING != pMngr->pDrives[index].cMode)
		{
			letters[count] = pMngr->pDrives[index].cLetter; 
			pszDevName[count] = pMngr->pDrives[index].pszDevName;
			count++; 
		}
	}
	
	LeaveCriticalSection(&pMngr->csLock);

	while(count--)
	{
		HANDLE hDevice = CreateFileW(pszDevName[count], GENERIC_READ | GENERIC_WRITE, 
									 FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

		if (INVALID_HANDLE_VALUE != hDevice)
		{
			BYTE sc, asc, ascq;
			BOOL bReady, bReportChanges, bNeedRecheck;
			DWORD ticks;

			bReportChanges = FALSE;
			bNeedRecheck = FALSE;

			if(!SPTI_TestUnitReady(hDevice, &sc, &asc, &ascq, 2))
			{
				bReady = FALSE;
			}
			else bReady = (0x00 == sc || (0x02 == sc && 0x3A == asc));

			CloseHandle(hDevice);

			EnterCriticalSection(&pMngr->csLock);
			for (index =0; index < pMngr->nCount; index++) 
			{
				if (pMngr->pDrives[index].cLetter == letters[count])
				{
					ticks = GetTickCount();
					if (pMngr->pDrives[index].bMediumInserted && 
						(ticks - pMngr->pDrives[index].mediumInfo.msLastPolled) > POLLMEDIUMVALIDATE_INTERVAL) bNeedRecheck = TRUE;
					pMngr->pDrives[index].mediumInfo.msLastPolled = ticks;

					if (bReady && ((0x00 == sc) != pMngr->pDrives[index].bMediumInserted)) bReportChanges = TRUE;
					break;
				}
			}
			LeaveCriticalSection(&pMngr->csLock);

			if (bReportChanges)
			{
				if (0 == sc) Medium_Add(letters[count], (DWORD)-1);
				else Medium_Remove(letters[count]);
			}
			else if (bNeedRecheck)
			{
				QueueInfoAPC(letters[count], APC_IsMediumChanged, (DWORD_PTR)letters[count]);
			}

		}
	}
}


static LRESULT WINAPI ListenerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_DEVICECHANGE:
			Listener_OnDeviceChange(hwnd, (UINT)wParam, (DWORD_PTR)lParam);
			break;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static void CALLBACK APC_CheckDrives(ULONG_PTR param)
{
	INT index, result, count;
	DWORD unitmask, dwOutput;
	STORAGE_DEVICE_NUMBER sdn;
	DEVICEINFO *pDevInfo;
	BYTE buffer[4096] = {0};

	if (!pMngr) return;

	unitmask = (DWORD)param;
	count = 0;
	for (int i = 0; i < 26; i++) {if (0x1 & (unitmask >> i)) count++;}
	if (!count) return;

	pDevInfo = (DEVICEINFO*)calloc(count, sizeof(DEVICEINFO));
	if (!pDevInfo) return;

	index = 0;
	for (int i = 0; i < 26; i++) 
	{
		if (0x1 & unitmask) 
		{
			pDevInfo[index].cLetter = (CHAR)(('A' + i));
			index++;
			if (index == count) break;
		}
		unitmask = unitmask >> 1;
	}

	GetDeviceNames(pDevInfo, count);

	for (int i = 0; i < count; i++)
	{
		HANDLE hDevice = CreateFileW(pDevInfo[i].pszDevName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 
									 NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

		if (INVALID_HANDLE_VALUE != hDevice)
		{
			ZeroMemory(&sdn, sizeof(STORAGE_DEVICE_NUMBER));
			result = DeviceIoControl(hDevice, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(STORAGE_DEVICE_NUMBER), &dwOutput, NULL);
			pDevInfo->deviceNumber = (result) ? sdn.DeviceNumber : -1;

			ZeroMemory(&buffer, sizeof(buffer)/sizeof(BYTE));
			result = DeviceIoControl(hDevice, IOCTL_STORAGE_GET_MEDIA_TYPES_EX, NULL, 0, buffer, sizeof(buffer)/sizeof(BYTE), &dwOutput, NULL);
			if (result && buffer && (FILE_DEVICE_DVD & ((GET_MEDIA_TYPES*)buffer)->DeviceType)) pDevInfo[i].dwType = DRIVE_TYPE_DVD;
			else pDevInfo[i].dwType = DRIVE_TYPE_CD;

			CloseHandle(hDevice);
		}
	}

	GetDeviceCaps(pDevInfo, count);

	EnterCriticalSection(&pMngr->csLock);

	for (int i = 0; i < count; i++)
	{
		pDevInfo[i].opCode = 0;
		for (index =0; index < pMngr->nCount; index++) 
		{
			if (pMngr->pDrives[index].cLetter == pDevInfo[i].cLetter)
			{
				if (-1 == pMngr->pDrives[index].deviceNumber || pMngr->pDrives[index].deviceNumber != pDevInfo[i].deviceNumber) 
					pDevInfo[i].opCode = 1;
				break;
			}
		}
		if (pMngr->nCount == index) pDevInfo[i].opCode = 2;
	}
	LeaveCriticalSection(&pMngr->csLock);

	for (int i = 0; i < count; i++)
	{
		if (pDevInfo[i].opCode) Drive_Add(&pDevInfo[i]);
	}

	if (pDevInfo) 
	{
		for (int i = 0; i<count; i++)
		{
			if (pDevInfo[i].pszDevName) free(pDevInfo[i].pszDevName);
		}
		free(pDevInfo);
	}
}

static void CALLBACK APC_IsMediumChanged(ULONG_PTR param)
{
	INT		opCode;
	DWORD	serial;

	wchar_t devname[4] = L"X:\\";

	if (!pMngr) return;

	opCode = 0;
	devname[0] = (char)(0xFF & param);
	if (devname[0]) 
	{
		BOOL result;

		serial = 0;
		result = GetVolumeInformationW(devname, NULL, 0, &serial, NULL, NULL, NULL, 0);
		if (!result) serial = 0; // perhaps this is empty recordable disc

		EnterCriticalSection(&pMngr->csLock);

		for (INT index =0; index < pMngr->nCount; index++) 
		{
			if (pMngr->pDrives[index].cLetter == (char)param )
			{
				pMngr->pDrives[index].mediumInfo.msLastPolled = GetTickCount();
				if (!pMngr->pDrives[index].bMediumInserted && result) opCode = 0x02;
				else if (pMngr->pDrives[index].mediumInfo.serialNumber != serial)
				{
					if (-1 == pMngr->pDrives[index].mediumInfo.serialNumber)  pMngr->pDrives[index].mediumInfo.serialNumber = serial;
					else opCode = 0x03;
				}
				break;
			}
		}
		LeaveCriticalSection(&pMngr->csLock);

		if (0x01 & opCode) Medium_Remove((char)param);
		if (0x02 & opCode) Medium_Add((char)param, serial);
	}
}

static void CALLBACK APC_AsyncOp_Complete(ULONG_PTR param)
{
	DM_NOTIFY_PARAM *phdr = (DM_NOTIFY_PARAM*)param;
	if (phdr->hReserved)
	{
		CloseHandle(phdr->hReserved);
		phdr->hReserved = NULL;
	}

	if (phdr->callback)
	{
		if (phdr->uMsg)
		{
			if (IsWindow((HWND)phdr->callback)) SendMessageW((HWND)phdr->callback, phdr->uMsg, (WPARAM)DMW_OPCOMPLETED, (LPARAM)phdr);
		}
		else ((DMNPROC)phdr->callback)(DMW_OPCOMPLETED, (INT_PTR)param); 
	}

	if (phdr->fnFree)
	{
		phdr->fnFree(phdr);
	}
}

static void AsycOp_Complete(DM_NOTIFY_PARAM *param)
{
	if (param) QueueUserAPC(APC_AsyncOp_Complete, param->hReserved, (ULONG_PTR)param);
}

static void CALLBACK APC_GetUnitInfo(ULONG_PTR param)
{
	DWORD unit;
	DM_UNITINFO_PARAM *puip;
	puip = (DM_UNITINFO_PARAM*)param;
	
	puip->header.opCode = DMOP_UNITINFO;
	
	unit = CheckLetter(puip->header.cLetter);
	
	if (!unit || (!PrimoSDKHelper_IsInitialized() && !PrimoSDKHelper_Initialize())) puip->header.result = PRIMOSDK_CMDSEQUENCE;
	else 
	{
		DWORD ready = 0;
		CHAR  buffer[512] = {0};

		puip->header.result = PrimoSDKHelper_UnitInfo(&unit, &puip->dwType, 
									((DMF_DESCRIPTION & puip->header.fFlags) || (DMF_FIRMWARE & puip->header.fFlags)) ? (BYTE*)buffer: NULL,
									(DMF_READY & puip->header.fFlags) ? &ready : NULL);

		if (PRIMOSDK_OK == puip->header.result)
		{
			if (DMF_READY & puip->header.fFlags) puip->bReady = (0 != ready);

			if (DMF_DESCRIPTION & puip->header.fFlags)
			{
				INT len = lstrlenA(buffer);
				if (len > 5) len -= 5;
				if (!puip->pszDesc || puip->cchDesc < (len + 1)) puip->cchDesc = -(len + 1);
				else 
				{
					StringCchCopyNA(puip->pszDesc, puip->cchDesc, buffer, len); 
					puip->cchDesc = len;
				}
			}
			if (DMF_FIRMWARE & puip->header.fFlags)
			{
				LPSTR p;
				INT len = lstrlenA(buffer);
				p = buffer + (len - ((len > 5) ? 4 : 0));
				if (!puip->pszFirmware || puip->cchFirmware < 4) puip->cchFirmware = -4;
				else
				{
					StringCchCopyA(puip->pszFirmware, puip->cchFirmware, p); 
					puip->cchFirmware = 4;
				}
			}
		}
	}
	AsycOp_Complete(&puip->header);
}

static void CALLBACK APC_GetUnitInfo2(ULONG_PTR param)
{
	DWORD unit;
	DM_UNITINFO2_PARAM *puip;

	puip = (DM_UNITINFO2_PARAM*)param;
	puip->header.opCode = DMOP_UNITINFO2;
	unit = CheckLetter(puip->header.cLetter);

	if (!unit || (!PrimoSDKHelper_IsInitialized() && !PrimoSDKHelper_Initialize())) puip->header.result = PRIMOSDK_CMDSEQUENCE;
	else 
	{
		BOOL bReady;
		DWORD szTypes[32], rfu;

		if (DriveManager_IsUnitReady((char)unit, &bReady) && !bReady)
		{
			SleepEx(1000, TRUE);
			QueueUserAPC(APC_GetUnitInfo2, GetCurrentThread(), param);
			return;
		}

		puip->header.result = PrimoSDKHelper_UnitInfo2(&unit, szTypes, &puip->dwClassId, &puip->dwBusType, &rfu); 
		if (PRIMOSDK_OK == puip->header.result)
		{
			if (DMF_TYPES & puip->header.fFlags)
			{
				INT len;
				for (len = 0; szTypes[len] != 0xFFFFFFFF; len++);
				
				if (!puip->pdwTypes || puip->nTypes < len) puip->nTypes = -len;
				else 
				{
					puip->nTypes = len;
					if (len) CopyMemory(puip->pdwTypes, szTypes, sizeof(DWORD)*len);
				}
			}
		}
	}
	AsycOp_Complete(&puip->header);
}
static void CALLBACK APC_GetDiscInfoEx(ULONG_PTR param)
{
	DWORD unit;
	DM_DISCINFOEX_PARAM *pdip;

	pdip = (DM_DISCINFOEX_PARAM*)param;
	pdip->header.opCode = DMOP_DISCINFO;
	unit = CheckLetter(pdip->header.cLetter);

	if (!unit || (!PrimoSDKHelper_IsInitialized() && !PrimoSDKHelper_Initialize())) pdip->header.result = PRIMOSDK_CMDSEQUENCE;
	else 
	{
		BOOL bReady;
		DWORD dwFlags, dwErasable;

		if (DriveManager_IsUnitReady((char)unit, &bReady) && !bReady)
		{
			SleepEx(1000, TRUE);
			QueueUserAPC(APC_GetDiscInfoEx, GetCurrentThread(), param);
			return;
		}

		dwFlags = (DMF_DRIVEMODE_TAO & pdip->header.fFlags);
		pdip->header.result = PrimoSDKHelper_DiscInfoEx(&unit, dwFlags, 
									(DMF_MEDIUMTYPE & pdip->header.fFlags) ? &pdip->dwMediumType : NULL,
									(DMF_MEDIUMFORMAT & pdip->header.fFlags) ? &pdip->dwMediumFormat : NULL,
									&dwErasable,
									(DMF_TRACKS & pdip->header.fFlags) ? &pdip->dwTracks: NULL,
									(DMF_USED & pdip->header.fFlags) ? &pdip->dwUsed : NULL,
									(DMF_FREE & pdip->header.fFlags) ? &pdip->dwFree : NULL); 

		if (PRIMOSDK_OK == pdip->header.result) pdip->bErasable = (0 != dwErasable);
	}

	AsycOp_Complete(&pdip->header);
}

static void CALLBACK APC_GetDiscInfo2(ULONG_PTR param)
{
	DWORD unit;
	DM_DISCINFO2_PARAM *pdip;

	pdip = (DM_DISCINFO2_PARAM*)param;
	pdip->header.opCode = DMOP_DISCINFO2;
	unit = CheckLetter(pdip->header.cLetter);

	if (!unit || (!PrimoSDKHelper_IsInitialized() && !PrimoSDKHelper_Initialize())) pdip->header.result = PRIMOSDK_CMDSEQUENCE;
	else 
	{
		DWORD rfu, medium, protectedDVD, flags;

		BOOL bReady;
		if (DriveManager_IsUnitReady((char)unit, &bReady) && !bReady)
		{
			SleepEx(1000, TRUE);
			QueueUserAPC(APC_GetDiscInfo2, GetCurrentThread(), param);
			return;
		}

		pdip->header.result = PrimoSDKHelper_DiscInfo2(&unit, 
									(DMF_MEDIUM & pdip->header.fFlags) ? &pdip->dwMedium : (DMF_MEDIUMEX & pdip->header.fFlags) ? &medium : NULL,
									(DMF_PROTECTEDDVD & pdip->header.fFlags) ? &protectedDVD : NULL,
									(DMF_PACKETWRITTEN & pdip->header.fFlags) ? &flags : NULL, 
									(DMF_MEDIUMEX & pdip->header.fFlags) ? &pdip->dwMediumEx : NULL, 
									&rfu); 
		if (PRIMOSDK_OK == pdip->header.result)
		{
			if (DMF_PROTECTEDDVD & pdip->header.fFlags) pdip->bProtectedDVD = (0 != protectedDVD);
			if (DMF_PACKETWRITTEN & pdip->header.fFlags) pdip->bPacketWritten = (0 != (PRIMOSDK_PACKETWRITTEN & protectedDVD));
		}

	}
	AsycOp_Complete(&pdip->header);
}

static void CALLBACK APC_GetTitle(ULONG_PTR param)
{
	CHAR cLetter;
	DM_TITLE_PARAM *pdtp;

	pdtp = (DM_TITLE_PARAM*)param;
	pdtp->header.opCode = DMOP_TITLE;
	cLetter = CheckLetter(pdtp->header.cLetter);

	pdtp->header.result = PRIMOSDK_CMDSEQUENCE;	
	if (cLetter && pdtp->pszTitle) 
	{
		wchar_t name[] = L"X:\\";
		MCIDEVICEID	devId;
		MCI_OPEN_PARMS		op = {0};
		MCI_GENERIC_PARMS	gp = {0};
		MCI_STATUS_PARMS	sp = {0};

		name[0] = cLetter;

		op.lpstrDeviceType		= (LPWSTR)MCI_DEVTYPE_CD_AUDIO;
		op.lpstrElementName		= name;

		if (!mciSendCommandW(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_SHAREABLE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT, (DWORD_PTR)&op))
		{
			HRESULT hr;

			devId = op.wDeviceID;
			sp.dwItem = MCI_STATUS_MEDIA_PRESENT;
			INT present = (!mciSendCommandW(devId, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD_PTR)&sp)) ? (BOOL)sp.dwReturn : 0;

			if (present)
			{
				INT nTracks;
				BOOL bAudio;
				wchar_t szVolume[256] = {0};
				// check if we have at least one audio track
				sp.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
				nTracks = (!mciSendCommandW(devId, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD_PTR)&sp)) ? (INT)sp.dwReturn : -1;
				bAudio = FALSE;
				
				if (nTracks > 0)
				{
					sp.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
					for (sp.dwTrack = 1; sp.dwTrack <= (UINT)nTracks && !bAudio; sp.dwTrack++) 
					{
						mciSendCommandW(devId, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, (DWORD_PTR)&sp);
						bAudio = (MCI_CDA_TRACK_AUDIO == sp.dwReturn);
					}
					if (bAudio) WASABI_API_LNGSTRINGW_BUF(IDS_CD_AUDIO, szVolume, sizeof(szVolume)/sizeof(wchar_t));
					else 
					{
						INT result;
						wchar_t devname[4] = L"X:\\";
						devname[0] = cLetter;
						result = GetVolumeInformationW(devname, szVolume, sizeof(szVolume)/sizeof(wchar_t), NULL, NULL, NULL, NULL, 0);
						if (!result) WASABI_API_LNGSTRINGW_BUF(IDS_DISC_DATA, szVolume, sizeof(szVolume)/sizeof(wchar_t));
					}
				}
				else WASABI_API_LNGSTRINGW_BUF(IDS_DISC_BLANK, szVolume, sizeof(szVolume)/sizeof(wchar_t));

				hr = StringCchPrintfW(pdtp->pszTitle, pdtp->cchTitle, L"%s (%c:)", szVolume, cLetter);
			}
			else
			{
				INT nDriveType, nDriveCap;
				DWORD type;
				wchar_t szDriveType[32] = {0}, szDriveCap[64] = {0};

				type = DriveManager_GetDriveType(cLetter);
				if ((DRIVE_TYPE_UNKNOWN | DRIVE_CAP_UNKNOWN) == type) type = DRIVE_TYPE_CD;

				nDriveCap = ((DRIVE_CAP_R | DRIVE_CAP_RW) & type) ? IDS_RECORDER_CAP : IDS_DRIVE_CAP;
				nDriveType = (IDS_DRIVE_CAP == nDriveCap && (DRIVE_TYPE_DVD & type)) ? IDS_DVD : IDS_CD;

				WASABI_API_LNGSTRINGW_BUF(nDriveType, szDriveType, sizeof(szDriveType)/sizeof(wchar_t));
				WASABI_API_LNGSTRINGW_BUF(nDriveCap, szDriveCap, sizeof(szDriveCap)/sizeof(wchar_t));
				hr = StringCchPrintfW(pdtp->pszTitle, pdtp->cchTitle, L"%s %s (%C:)", szDriveType, szDriveCap, cLetter);
			}
			pdtp->header.result = hr;
			mciSendCommandW(devId, MCI_CLOSE, MCI_WAIT, (DWORD_PTR)&gp);
		}
	}
	AsycOp_Complete(&pdtp->header);
}

static void CALLBACK APC_DriveScan(ULONG_PTR param)
{
	char i;
	char root[] = "A:\\";
	DWORD unitmask;
	DEVICEINFO di = {0};

	/// detect drives
	unitmask = GetLogicalDrives();

	di.deviceNumber = -1;
	di.dwType = DRIVE_TYPE_CD;
	
	for (i = 0; i < 26; ++i)
	{
		if (0x1 & (unitmask >> i))
		{
			root[0] = ('A' + i);
			if(DRIVE_CDROM != GetDriveTypeA(root)) unitmask &= ~(1 << i); 
			else 
			{
				di.cLetter = root[0];
				Drive_Add(&di);
			}
		}
	}
	APC_CheckDrives((ULONG_PTR)unitmask);
}

#define MAX_TEST_ATTEMPT 20

static void CALLBACK APC_Eject(ULONG_PTR param)
{
	INT nCmd;
	CHAR cLetter;
	BYTE sc(0), asc(0), ascq(0);

	nCmd = HIWORD(param);
    cLetter = CheckLetter((CHAR)param);

	if (cLetter && DM_MODE_READY == DriveManager_GetDriveMode(cLetter))
	{
		BOOL bSuccess;
		HANDLE hDevice;

		hDevice = CreateFileW(GetDeviceName(cLetter), GENERIC_READ | GENERIC_WRITE, 
									FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
		if (INVALID_HANDLE_VALUE != hDevice)
		{	
			DWORD dwOutput;
			LARGE_INTEGER start, finish;
			
			bSuccess = SPTI_TestUnitReady(hDevice, &sc, &asc, &ascq, 3);
			if (!bSuccess && ERROR_SEM_TIMEOUT == GetLastError()) 
			{
				bSuccess = TRUE;
				sc = 0xFF;
			}
			
			if (bSuccess && (0 == sc || (0x02 == sc && 0x3A == asc)))
			{
				INT opCode;
				opCode = (DM_EJECT_REMOVE == nCmd || 0x00 == sc || (0x3A == asc && 0x01 == ascq)) ? 
							IOCTL_STORAGE_EJECT_MEDIA : IOCTL_STORAGE_LOAD_MEDIA;
				
				QueryPerformanceCounter(&start);
				bSuccess = DeviceIoControl(hDevice, opCode, NULL, 0, NULL, 0, &dwOutput, NULL);
				QueryPerformanceCounter(&finish);
				
				if (bSuccess && DM_EJECT_CHANGE == nCmd && 0x00 != sc && 0x00 == ascq)
				{
					finish.QuadPart -= start.QuadPart;

					if (finish.QuadPart < freq.QuadPart && (finish.QuadPart*100000 / freq.QuadPart) < 200)
					{
						// test unit redy
						INT i;
						sc = 0x02; asc = 0x04; ascq = 0x01;
						for (i = 0; i < MAX_TEST_ATTEMPT && 0x02 == sc && 0x04 == asc && 0x01 == ascq; i++)
						{
							Sleep(50);
							if (!SPTI_TestUnitReady(hDevice, &sc, &asc, &ascq, 3) && ERROR_SEM_TIMEOUT == GetLastError()) 
								i = MAX_TEST_ATTEMPT;
						}
						if (i < MAX_TEST_ATTEMPT && 0x02 == sc && 0x3A ==asc)
						{
							DeviceIoControl(hDevice, IOCTL_STORAGE_EJECT_MEDIA, NULL, 0, NULL, 0, &dwOutput, NULL);
						}
						sc = 0x00;
					}
				}
			}
			CloseHandle(hDevice);
		}
		else bSuccess = FALSE;

		if (!bSuccess)
		{ // we can try MCI

		}
		else if (0x00 != sc && !(0x02 == sc && 0x3A == asc))
		{
			SleepEx(200, TRUE);
			QueueUserAPC(APC_Eject, GetCurrentThread(), param);
			return;
		}
	}
}

static void CALLBACK APC_GetMCIInfo(ULONG_PTR param)
{
	CHAR cLetter;
	MCI_OPEN_PARMS op = {0};
	DM_MCI_PARAM *pmcip;

	pmcip = (DM_MCI_PARAM*)param;
	pmcip->header.opCode = DMOP_MCIINFO;
	cLetter = CheckLetter(pmcip->header.cLetter);

	pmcip->header.result = PRIMOSDK_CMDSEQUENCE;
	if (cLetter)
	{
		wchar_t name[] = L"X:\\";
		MCIDEVICEID	devId;
		MCI_INFO_PARMS		ip = {0};
		MCI_GENERIC_PARMS	gp = {0};
		MCI_STATUS_PARMS	sp = {0};

		name[0] = cLetter;

		op.lpstrDeviceType		= (LPWSTR)MCI_DEVTYPE_CD_AUDIO;
		op.lpstrElementName		= name;

		if (!mciSendCommandW(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_SHAREABLE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT, (DWORD_PTR)&op))
		{
			WCHAR buffer[512] = {0};
			INT nMaxTracks = pmcip->nTracks;

			devId = op.wDeviceID;

			if ((DMF_TRACKCOUNT | DMF_TRACKSINFO) & pmcip->header.fFlags)
			{
				sp.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
				pmcip->nTracks = (!mciSendCommandW(devId, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD_PTR)&sp)) ? (INT)sp.dwReturn : -1;
			}
			if (DMF_READY & pmcip->header.fFlags)
			{
				sp.dwItem = MCI_STATUS_READY;
				pmcip->bReady = (!mciSendCommandW(devId, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD_PTR)&sp)) ? (BOOL)sp.dwReturn : 0;
			}
			if (DMF_MODE & pmcip->header.fFlags)
			{
				sp.dwItem = MCI_STATUS_MODE;
				pmcip->uMode = (!mciSendCommandW(devId, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD_PTR)&sp)) ? (UINT)sp.dwReturn : 0;
			}
			if (DMF_MEDIUMPRESENT & pmcip->header.fFlags)
			{
				sp.dwItem = MCI_STATUS_MEDIA_PRESENT;
				pmcip->bMediumPresent = (!mciSendCommandW(devId, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD_PTR)&sp)) ? (BOOL)sp.dwReturn : 0;
			}
			if (DMF_MEDIUMUID & pmcip->header.fFlags)
			{
				ip.dwRetSize = sizeof(buffer)/sizeof(wchar_t);
				ip.lpstrReturn= buffer;
				if (!mciSendCommandW(devId, MCI_INFO, MCI_WAIT | MCI_INFO_MEDIA_IDENTITY, (DWORD_PTR)&ip))
				{
					INT len;
					len = lstrlenW(ip.lpstrReturn);
					if (S_OK == StringCchCopyW(pmcip->pszMediumUID, pmcip->cchMediumUID, ip.lpstrReturn))
					{
						pmcip->cchMediumUID = len;
					}
					else pmcip->cchMediumUID = 0 - (len + 1);
				}
				else pmcip->cchMediumUID = -1;
			}
			if (DMF_MEDIUMUPC & pmcip->header.fFlags)
			{
				ip.dwCallback	= NULL;
				ip.dwRetSize		= sizeof(buffer)/sizeof(wchar_t);
				ip.lpstrReturn	= buffer;
				if (!mciSendCommandW(devId, MCI_INFO, MCI_WAIT | MCI_INFO_MEDIA_UPC, (DWORD_PTR)&ip))
				{
					INT len;
					len = lstrlenW(ip.lpstrReturn);
					if (S_OK == StringCchCopyW(pmcip->pszMediumUPC, pmcip->cchMediumUPC, ip.lpstrReturn))
					{
						pmcip->cchMediumUPC = len;
					}
					else pmcip->cchMediumUPC = 0 - (len + 1);
				}
				else pmcip->cchMediumUPC = -1;
			}

			if (DMF_TRACKSINFO & pmcip->header.fFlags)
			{
				MCI_SET_PARMS setp;

				if (nMaxTracks < pmcip->nTracks) pmcip->nTracks = (0 - pmcip->nTracks);
				else
				{
					INT prevPos(0), length(0);
					setp.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
					mciSendCommandW(devId, MCI_SET, MCI_WAIT | MCI_SET_TIME_FORMAT, (DWORD_PTR)&setp);

					for (int i = pmcip->nTracks; i > 0; i--)
					{
						sp.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
						sp.dwTrack = i;
						mciSendCommandW(devId, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, (DWORD_PTR)&sp);
						BOOL bAudio = (MCI_CDA_TRACK_AUDIO == sp.dwReturn);
						
						sp.dwItem = MCI_STATUS_POSITION;
						sp.dwTrack = i;
						mciSendCommandW(devId, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, (DWORD_PTR)&sp);
						
						if (i != pmcip->nTracks) length = prevPos - (INT)sp.dwReturn;
						prevPos = (INT)sp.dwReturn;

						if (i == pmcip->nTracks)
						{
							sp.dwItem = MCI_STATUS_LENGTH;
							sp.dwTrack = i;
							mciSendCommandW(devId, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, (DWORD_PTR)&sp);
							length = (INT)sp.dwReturn;
						}

						pmcip->pTracks[i- 1] = (0x7FFFFFF & length) | ((bAudio) ? 0x80000000 : 0);
					}
					
					setp.dwTimeFormat = MCI_FORMAT_TMSF;
					mciSendCommandW(devId, MCI_SET, MCI_WAIT | MCI_SET_TIME_FORMAT, (DWORD_PTR)&setp);
				}
			}

			mciSendCommandW(devId, MCI_CLOSE, MCI_WAIT, (DWORD_PTR)&gp);
			pmcip->header.result = PRIMOSDK_OK;
		}
	}

	AsycOp_Complete(&pmcip->header);
}

static void CALLBACK APC_GetIMAPIInfo(ULONG_PTR param)
{
	CHAR cLetter;
	BOOL bReady;
	HRESULT hr(S_FALSE);
	IDiscMaster				*pdm;
	IDiscRecorder			*pdr;
	IEnumDiscRecorders		*per;
	ULONG nActual;
	
	wchar_t szDevName[] = L"X:\\";
	wchar_t	szTargetName[128] = {0};
	DM_IMAPI_PARAM *pIMAPI;

	pIMAPI = (DM_IMAPI_PARAM*)param;
	cLetter = CheckLetter(pIMAPI->header.cLetter);

	if (DriveManager_IsUnitReady(cLetter, &bReady) && !bReady)
	{
		SleepEx(1000, TRUE);
		QueueUserAPC(APC_GetIMAPIInfo, GetCurrentThread(), param);
		return;
	}
	
	pIMAPI->header.opCode = DMOP_IMAPIINFO;
		
	pIMAPI->bRecorder = FALSE;
	pIMAPI->header.result = (DWORD)E_INVALIDARG;
	
	szDevName[0] = cLetter;
	if (cLetter && QueryDosDeviceW(szDevName, szTargetName, sizeof(szTargetName)/sizeof(wchar_t)))
	{
		hr = CoCreateInstance(CLSID_MSDiscMasterObj, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IDiscMaster, (void**)&pdm);
		if (SUCCEEDED(hr))
		{
			hr = pdm->Open();
			if (SUCCEEDED(hr))
			{
				IEnumDiscMasterFormats	*pef;
				hr = pdm->EnumDiscMasterFormats(&pef);
				if (SUCCEEDED(hr))
				{
					IID pFormats[2];
					hr = pef->Next(sizeof(pFormats)/sizeof(IID), pFormats, &nActual); 
					if (SUCCEEDED(hr))
					{
						while(nActual--) { if (IID_IRedbookDiscMaster == pFormats[nActual]) break; }
						if (nActual != ((ULONG)-1))
						{
							IRedbookDiscMaster *pdf;
							hr = pdm->SetActiveDiscMasterFormat(IID_IRedbookDiscMaster, (void**)&pdf);
							if (SUCCEEDED(hr))
							{
								pdf->Release();
								hr = pdm->EnumDiscRecorders(&per);
								if (SUCCEEDED(hr))
								{
									while (S_OK== per->Next(1, &pdr, &nActual) && nActual > 0)
									{
										BSTR bstrPath;
										hr = pdr->GetPath(&bstrPath);
										if (SUCCEEDED(hr))
										{
											if (0 == lstrcmp(szTargetName, bstrPath))
											{
												pIMAPI->bRecorder = TRUE;
												if ((DMF_BASEPNPID & pIMAPI->header.fFlags) && FAILED(pdr->GetBasePnPID(&pIMAPI->bstrBasePnPID))) pIMAPI->bstrBasePnPID = NULL;
												if ((DMF_DISPLAYNAMES & pIMAPI->header.fFlags) && FAILED(pdr->GetDisplayNames(&pIMAPI->bstrVendorID, &pIMAPI->bstrProductID, &pIMAPI->bstrRevision)))
												{
													pIMAPI->bstrVendorID = NULL;
													pIMAPI->bstrProductID = NULL;
													pIMAPI->bstrRevision = NULL;
												}
												if (DMF_PATH & pIMAPI->header.fFlags)
												{
													pIMAPI->bstrPath = bstrPath;
													bstrPath = NULL;
												}
												if ((DMF_DRIVESTATE & pIMAPI->header.fFlags) && FAILED(pdr->GetRecorderState(&pIMAPI->ulDriveState))) pIMAPI->ulDriveState = (ULONG)-1;
												if ((DMF_DRIVETYPE & pIMAPI->header.fFlags) && FAILED(pdr->GetRecorderType(&pIMAPI->fDriveType))) pIMAPI->fDriveType = 0;
												if ((DMF_QUERYMEDIATYPE | DMF_QUERYMEDIAINFO) & pIMAPI->header.fFlags)
												{
													BOOL bTypeOk(FALSE), bInfoOk(FALSE);
													if (SUCCEEDED(pdr->OpenExclusive()))
													{
														if (0 == (DMF_QUERYMEDIATYPE & pIMAPI->header.fFlags) || 
																SUCCEEDED(pdr->QueryMediaType(&pIMAPI->fMediaType, &pIMAPI->fMediaFlags))) bTypeOk = TRUE;
														if (0 == (DMF_QUERYMEDIAINFO & pIMAPI->header.fFlags) || 
																SUCCEEDED(pdr->QueryMediaInfo(&pIMAPI->bSessions, &pIMAPI->bLastTrack, &pIMAPI->ulStartAddress, 
																								&pIMAPI->ulNextWritable, &pIMAPI->ulFreeBlocks))) bInfoOk = TRUE;
														pdr->Close();
													}
													
													
													if (!bTypeOk)
													{
														pIMAPI->fMediaType	= -1;
														pIMAPI->fMediaFlags	= -1;
													}
													if (!bInfoOk)
													{
														pIMAPI->bLastTrack = 0;
														pIMAPI->bSessions = 0;
														pIMAPI->ulFreeBlocks = 0;
														pIMAPI->ulNextWritable = 0;
														pIMAPI->ulStartAddress = 0;
													}
														
													
												}
												break;
											}
											if (bstrPath) SysFreeString(bstrPath);
										}
										pdr->Release();
									}
									per->Release();
								}
							}
						}
					}
					pef->Release();
				}
				pdm->Close();
			}
			pdm->Release();
		}
	}
	pIMAPI->header.result = hr;
	AsycOp_Complete(&pIMAPI->header);
}