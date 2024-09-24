#include ".\configdlg.h"
#include ".\smtpdlg.h"
#include ".\resource.h"
#include <shlobj.h>
#include ".\miniVersion.h"
#include ".\getwinver.h"
#include ".\settings.h"
#include ".\minidump.h"
#include ".\main.h"
#include <strsafe.h>
#include "api__gen_crasher.h"

extern Settings settings;

BOOL CALLBACK ConfigDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static wchar_t *path;
	
	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
			HWND hwCombo = GetDlgItem(hwndDlg, IDC_CMB_DMPTYPE);
   
			// detect windows version
			wchar_t strBuff[2048] = {0}, winVer[32] = {0}, build[32] = {0};
			int nWinVer = 0;
			GetWinVer(winVer, &nWinVer, build);
 			StringCchPrintf(strBuff, 2048, L"%s (%s)", winVer, build);
			SetWindowText(GetDlgItem(hwndDlg, IDC_LBL_OSVERSION), strBuff);

			// discover dbghlp.dll
			HMODULE hm = NULL;
			// first in app folder
			if (GetModuleFileName( NULL, strBuff, _MAX_PATH ))
			{
				wchar_t *pSlash = wcsrchr( strBuff, L'\\' );
				if (pSlash)
				{
					StringCchCopy(pSlash+1, 2048 - (pSlash + 1 - strBuff), L"dbghelp.dll" );
					hm = LoadLibraryW( strBuff );
				}
			}
			if (!hm)
			{
				// load any version we can
				hm = LoadLibraryW(L"dbghelp.dll");
			}
			
			if (hm)
			{
				GetModuleFileName(hm, strBuff, 2048);
				SetWindowText(GetDlgItem(hwndDlg, IDC_LBL_DLLPATH), strBuff);
				// try to get dll version
				CMiniVersion ver(strBuff);
				if(ver.Init())
				{
					WORD dwBuf[4] = {0};
					ver.GetProductVersion(dwBuf);
					StringCchPrintf(strBuff, 2048, L"%d.%d.%d.%d", dwBuf[0], dwBuf[1], dwBuf[2], dwBuf[3]);
				}
				else
				{
					WASABI_API_LNGSTRINGW_BUF(IDS_UNABLE_TO_LOAD, strBuff, 128);
				}
				ver.Release();
				
				BOOL (WINAPI* MiniDumpWriteDump)(
												HANDLE hProcess,
												DWORD ProcessId,
												HANDLE hFile,
												MINIDUMP_TYPE DumpType,
												PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
												PMINIDUMP_EXCEPTION_INFORMATION UserStreamParam,
												PMINIDUMP_EXCEPTION_INFORMATION CallbackParam
												) = NULL;
				*(FARPROC*)&MiniDumpWriteDump = GetProcAddress(hm, "MiniDumpWriteDump");

				wchar_t temp[256] = {0};
				StringCchPrintfW(temp, 256, L"%s [%s]", strBuff, WASABI_API_LNGSTRINGW(MiniDumpWriteDump ? IDS_LOADED_OK : IDS_UNABLE_TO_LOAD));
				SetDlgItemText(hwndDlg, IDC_LBL_DLLVERSION, temp);

				FreeLibrary(hm);
			}
			else
			{
				SetWindowText(GetDlgItem(hwndDlg, IDC_LBL_DLLPATH), WASABI_API_LNGSTRINGW(IDS_NOT_FOUND));
				wchar_t temp[256] = {0}, temp2[128] = {0};
				StringCchPrintfW(temp, 256, L"%s [%s]", WASABI_API_LNGSTRINGW(IDS_UNKNOWN), WASABI_API_LNGSTRINGW_BUF(IDS_UNABLE_TO_LOAD, temp2, 128));
				SetWindowText(GetDlgItem(hwndDlg, IDC_LBL_DLLVERSION), temp);
			}
			 // set combobox with values
			WPARAM pos;
			pos = SendMessage(hwCombo, CB_ADDSTRING, 0, (LPARAM) L"MiniDumpNormal");
			SendMessage(hwCombo, CB_SETITEMDATA, pos, 0x00000000);
			pos = SendMessage(hwCombo, CB_ADDSTRING, 0, (LPARAM) L"MiniDumpWithDataSegs");
			SendMessage(hwCombo, CB_SETITEMDATA, pos, 0x00000001);
			pos = SendMessage(hwCombo, CB_ADDSTRING, 0, (LPARAM) L"MiniDumpWithFullMemory");
			SendMessage(hwCombo, CB_SETITEMDATA, pos, 0x00000002);
			pos = SendMessage(hwCombo, CB_ADDSTRING, 0, (LPARAM) L"MiniDumpWithHandleData (Not Supported: Windows Me/98/95)");
			SendMessage(hwCombo, CB_SETITEMDATA, pos, 0x00000004);
			pos = SendMessage(hwCombo, CB_ADDSTRING, 0, (LPARAM) L"MiniDumpFilterMemory");
			SendMessage(hwCombo, CB_SETITEMDATA, pos, 0x00000008);
			pos = SendMessage(hwCombo, CB_ADDSTRING, 0, (LPARAM) L"MiniDumpScanMemory");
			SendMessage(hwCombo, CB_SETITEMDATA, pos, 0x00000010);
			pos = SendMessage(hwCombo, CB_ADDSTRING, 0, (LPARAM) L"MiniDumpWithUnloadedModules (Not Supported: DbgHelp 5.1 and earlier)");
			SendMessage(hwCombo, CB_SETITEMDATA, pos, 0x00000020);
			pos = SendMessage(hwCombo, CB_ADDSTRING, 0, (LPARAM) L"MiniDumpWithIndirectlyReferencedMemory (Not Supported: DbgHelp 5.1 and earlier)");
			SendMessage(hwCombo, CB_SETITEMDATA, pos, 0x00000040);
			pos = SendMessage(hwCombo, CB_ADDSTRING, 0, (LPARAM) L"MiniDumpFilterModulePaths (Not Supported: DbgHelp 5.1 and earlier)");
			SendMessage(hwCombo, CB_SETITEMDATA, pos, 0x00000080);
			pos = SendMessage(hwCombo, CB_ADDSTRING, 0, (LPARAM) L"MiniDumpWithProcessThreadData (Not Supported: DbgHelp 5.1 and earlier)");
			SendMessage(hwCombo, CB_SETITEMDATA, pos, 0x00000100);
			pos = SendMessage(hwCombo, CB_ADDSTRING, 0, (LPARAM) L"MiniDumpWithPrivateReadWriteMemory (Not Supported: DbgHelp 5.1 and earlier)");
			SendMessage(hwCombo, CB_SETITEMDATA, pos, 0x00000200);
			pos = SendMessage(hwCombo, CB_ADDSTRING, 0, (LPARAM) L"MiniDumpWithoutOptionalData (Not Supported: DbgHelp 6.1 and earlier)");
			SendMessage(hwCombo, CB_SETITEMDATA, pos, 0x00000400);
			pos = SendMessage(hwCombo, CB_ADDSTRING, 0, (LPARAM) L"MiniDumpWithFullMemoryInfo (Not Supported: DbgHelp 6.1 and earlier)");
			SendMessage(hwCombo, CB_SETITEMDATA, pos, 0x00000800);
			pos = SendMessage(hwCombo, CB_ADDSTRING, 0, (LPARAM) L"MiniDumpWithThreadInfo (Not Supported: DbgHelp 6.1 and earlier)");
			SendMessage(hwCombo, CB_SETITEMDATA, pos, 0x00001000);
			pos = SendMessage(hwCombo, CB_ADDSTRING, 0, (LPARAM) L"MiniDumpWithCodeSegs (Not Supported: DbgHelp 6.1 and earlier)");
			SendMessage(hwCombo, CB_SETITEMDATA, pos, 0x00002000);

			// read settings
			settings.Load();
			CheckDlgButton(hwndDlg, IDC_CHK_CREATELOG, settings.createLOG);
			CheckDlgButton(hwndDlg, IDC_CHK_CREATEDMP, settings.createDMP);
			CheckDlgButton(hwndDlg, IDC_CHK_RESTART, settings.autoRestart);
			CheckDlgButton(hwndDlg, IDC_CHK_SILENT, settings.silentMode);
			CheckDlgButton(hwndDlg, IDC_CHK_SEND, settings.sendData);
			CheckDlgButton(hwndDlg, IDC_CHK_COMPRESS, settings.zipData);
			CheckDlgButton(hwndDlg, IDC_RB_USECLIENT, settings.sendByClient);
			CheckDlgButton(hwndDlg, IDC_RB_USESMTP, settings.sendBySMTP);

			CreatePathFromFullName(&path, settings.zipPath);
			SetWindowText(GetDlgItem(hwndDlg, IDC_EDT_PATH), path);

			SetWindowText(GetDlgItem(hwndDlg, IDC_EDT_ZIPNAME), GetFileName(settings.zipPath));

			SelectComboBoxItem(hwCombo, settings.dumpType);
			SetWindowText(GetDlgItem(hwndDlg, IDC_EDT_DMPPATH), GetFileName(settings.dumpPath));

			CheckDlgButton(hwndDlg, IDC_CHK_LOGSYSTEM, settings.logSystem);
			CheckDlgButton(hwndDlg, IDC_CHK_LOGREGISTRY, settings.logRegistry);
			CheckDlgButton(hwndDlg, IDC_CHK_LOGSTACK, settings.logStack);
			CheckDlgButton(hwndDlg, IDC_CHK_LOGMODULE, settings.logModule);
			SetWindowText(GetDlgItem(hwndDlg, IDC_EDT_LOGPATH), GetFileName(settings.logPath));

			UpdateSend(hwndDlg, settings.sendData);
			UpdateZip(hwndDlg, settings.zipData);
			UpdateCreateDmp(hwndDlg, settings.createDMP);
			UpdateCreateLog(hwndDlg, settings.createLOG);
			}
			break;
		case WM_DESTROY:
		{
			wchar_t buf[1024] = {0};
			int len, pathlen;
			
			HWND hwCombo;
			settings.createLOG  = (SendMessage(GetDlgItem(hwndDlg, IDC_CHK_CREATELOG), BM_GETCHECK, 0,0) == BST_CHECKED); 
			settings.createDMP = (SendMessage(GetDlgItem(hwndDlg, IDC_CHK_CREATEDMP), BM_GETCHECK, 0,0) == BST_CHECKED); 
			settings.autoRestart  = (SendMessage(GetDlgItem(hwndDlg, IDC_CHK_RESTART), BM_GETCHECK, 0,0) == BST_CHECKED); 
			settings.silentMode  = (SendMessage(GetDlgItem(hwndDlg, IDC_CHK_SILENT), BM_GETCHECK, 0,0) == BST_CHECKED); 
			settings.sendData  = (SendMessage(GetDlgItem(hwndDlg, IDC_CHK_SEND), BM_GETCHECK, 0,0) == BST_CHECKED); 
			settings.sendByClient  = (SendMessage(GetDlgItem(hwndDlg, IDC_RB_USECLIENT), BM_GETCHECK, 0,0) == BST_CHECKED); 
			settings.sendBySMTP = !settings.sendByClient;
			settings.zipData  = (SendMessage(GetDlgItem(hwndDlg, IDC_CHK_COMPRESS), BM_GETCHECK, 0,0) == BST_CHECKED); 
			
			len = GetWindowText(GetDlgItem(hwndDlg, IDC_EDT_PATH), buf, 1024);
			if (path) free(path);
			path = NULL;
			if (len) 
			{
				int cpyLen;
				path = (wchar_t*)malloc((len +1)*2);
				cpyLen = (buf[len-1] == L'\\')  ? len -1: len;
				StringCchCopyN(path, len + 1, buf, cpyLen);
			}
			pathlen = (path) ? (int)lstrlen(path) + 1 : 0;

			if (settings.zipPath) free(settings.zipPath);
			settings.zipPath = NULL;
			len = GetWindowText(GetDlgItem(hwndDlg, IDC_EDT_ZIPNAME), buf, 1024);
			if (len)
			{
				if (pathlen)
				{
					len += pathlen;
					settings.zipPath = (wchar_t*)malloc((len + 1)*2);
					StringCchPrintf(settings.zipPath, len+1, L"%s\\%s", path, buf);
				}
				else
				{
					settings.zipPath = (wchar_t*)malloc((len + 1)*2);
					StringCchCopy(settings.zipPath, len+1, buf);
				}
			}
			else // because path is based on the zipPath just write path 
			{
				if (pathlen)
				{
					len = pathlen;
					settings.zipPath = (wchar_t*)malloc((len + 1)*2);
					StringCchPrintf(settings.zipPath, len+1, L"%s\\", path);
				}
			}

			hwCombo = GetDlgItem(hwndDlg, IDC_CMB_DMPTYPE);
			settings.dumpType = (int) SendMessage(hwCombo, CB_GETITEMDATA, SendMessage(hwCombo, CB_GETCURSEL, 0, 0), 0);

			if (settings.dumpPath) free(settings.dumpPath);
			settings.dumpPath = NULL;
			len = GetWindowText(GetDlgItem(hwndDlg, IDC_EDT_DMPNAME), buf, 1024);
			if (len)
			{
				if (pathlen)
				{
					len += pathlen;
					settings.dumpPath = (wchar_t*)malloc((len + 1)*2);
					StringCchPrintf(settings.dumpPath, len+1, L"%s\\%s", path, buf);
				}
				else
				{
					settings.dumpPath = (wchar_t*)malloc((len + 1)*2);
					StringCchCopy(settings.dumpPath, len+1, buf);
				}
			}

			settings.logSystem  = (SendMessage(GetDlgItem(hwndDlg, IDC_CHK_LOGSYSTEM), BM_GETCHECK, 0,0) == BST_CHECKED); 
			settings.logRegistry  = (SendMessage(GetDlgItem(hwndDlg, IDC_CHK_LOGREGISTRY), BM_GETCHECK, 0,0) == BST_CHECKED); 
			settings.logStack  = (SendMessage(GetDlgItem(hwndDlg, IDC_CHK_LOGSTACK), BM_GETCHECK, 0,0) == BST_CHECKED); 
			settings.logModule  = (SendMessage(GetDlgItem(hwndDlg, IDC_CHK_LOGMODULE), BM_GETCHECK, 0,0) == BST_CHECKED); 

			if (settings.logPath) free(settings.logPath);
			settings.logPath = NULL;
			len = GetWindowText(GetDlgItem(hwndDlg, IDC_EDT_LOGNAME), buf, 1024);
			if (len)
			{
				if (pathlen)
				{
					len += pathlen;
					settings.logPath = (wchar_t*)malloc((len + 1)*2);
					StringCchPrintf(settings.logPath, len+1, L"%s\\%s", path, buf);
				}
				else
				{
					settings.logPath = (wchar_t*)malloc((len + 1)*2);
					StringCchCopy(settings.logPath, len+1, buf);
				}
			}

			if (!settings.Save())
			{
				wchar_t title[32] = {0};
				MessageBox(NULL, WASABI_API_LNGSTRINGW(IDS_UNABLE_TO_SAVE_SETTINGS),
						   WASABI_API_LNGSTRINGW_BUF(IDS_SAVE_ERROR,title,32), MB_OK);
			}

			if(path) free(path);
			path = NULL;
			break;
		}
		case WM_COMMAND:
			int ctrl = LOWORD(wParam);
			switch (ctrl)
			{
				case IDC_CHK_CREATELOG:
				case IDC_CHK_CREATEDMP:
				case IDC_CHK_SEND:
				case IDC_CHK_COMPRESS:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						BOOL enabled = (SendMessage((HWND) lParam, BM_GETCHECK, 0,0) == BST_CHECKED);
						if (ctrl == IDC_CHK_CREATELOG) UpdateCreateLog(hwndDlg, enabled);
						else if (ctrl == IDC_CHK_CREATEDMP) UpdateCreateDmp(hwndDlg, enabled);
						else if (ctrl == IDC_CHK_SEND) UpdateSend(hwndDlg, enabled);
						else if (ctrl == IDC_CHK_COMPRESS) UpdateZip(hwndDlg, enabled);
					}
					break;
				case IDC_RB_USESMTP:
				case IDC_RB_USECLIENT:
					if (HIWORD(wParam) == BN_CLICKED) UpdateSendType(hwndDlg, (ctrl == IDC_RB_USESMTP));
					break;
				case IDC_BTN_PATH:
					{	
						wchar_t szFile[2048] = {0};       // buffer for file name
						GetWindowText(GetDlgItem(hwndDlg, IDC_EDT_PATH), szFile, 2048);
						if (OpenFolderDialog(NULL, szFile))SetWindowText(GetDlgItem(hwndDlg, IDC_EDT_PATH), szFile);
					}
					break;
				case IDC_BTN_SMTP:
					WASABI_API_DIALOGBOXW(IDD_DLG_SMTP, hwndDlg, (DLGPROC)smtpDlgProc);
					break;
			}
			break;
	}
	//settings.smtpUser;
	return FALSE;
}

int SelectComboBoxItem(const HWND hwCombo, int data)
{
	int count = (int) SendMessage(hwCombo, CB_GETCOUNT , 0, 0);
	for (int i = 0; i < count; i++)
	{
		if (data == (int) SendMessage(hwCombo, CB_GETITEMDATA, i, 0))
			return (int) SendMessage(hwCombo, CB_SETCURSEL, i, 0);
	}
	return -1;
}

void UpdateZip(HWND hwndDlg, BOOL enabled)
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_GRP_ZIP), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_ZIPNAME), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_ZIPNAME), enabled);
}

void UpdateSendType(HWND hwndDlg, BOOL enabled)
{	
	EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SMTP), enabled);
}

void UpdateSend(HWND hwndDlg, BOOL enabled)
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_RB_USECLIENT), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RB_USESMTP), enabled);
	BOOL stmpPressed = (SendMessage(GetDlgItem(hwndDlg,IDC_RB_USESMTP), BM_GETCHECK, 0,0) == BST_CHECKED);
	EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SMTP), enabled && stmpPressed);
}

void UpdateCreateDmp(HWND hwndDlg, BOOL enabled)
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_OSVERSION_CAPTION), enabled);
    EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_DLLPATH_CAPTION), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_DLLVERSION_CAPTION), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_OSVERSION), enabled);
    EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_DLLPATH), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_DLLVERSION), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_DMPTYPE), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CMB_DMPTYPE), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_DMPNAME), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_DMPNAME), enabled);
}

void UpdateCreateLog(HWND hwndDlg, BOOL enabled)
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_GRP_LOG), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_LOGSYSTEM), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_LOGREGISTRY), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_LOGSTACK), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_LOGMODULE), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LBL_LOGNAME), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_EDT_LOGNAME), enabled);
}

BOOL CALLBACK browseEnumProc(HWND hwnd, LPARAM lParam)
{
	wchar_t cl[32] = {0};
	GetClassNameW(hwnd, cl, ARRAYSIZE(cl));
	if (!lstrcmpiW(cl, WC_TREEVIEW))
	{
		PostMessage(hwnd, TVM_ENSUREVISIBLE, 0, (LPARAM)TreeView_GetSelection(hwnd));
		return FALSE;
	}

	return TRUE;
}

static int CALLBACK WINAPI BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
		{
			//SetWindowText(hwnd, getString(IDS_P_SELECT_LANGDIR,NULL,0));
			SendMessage(hwnd, BFFM_SETSELECTION, 1, (LPARAM)lpData);

			// this is not nice but it fixes the selection not working correctly on all OSes
			EnumChildWindows(hwnd, browseEnumProc, 0);
		}
		return 0;
	}
	return 0;
}

BOOL OpenFolderDialog(HWND parent, LPWSTR pathBuffer)
{
	if (!pathBuffer) return FALSE;

	// Return value for the function
	BOOL ret = TRUE;

	// Set up the params
	BROWSEINFO browseInfo = {0};
	browseInfo.hwndOwner = parent;
	browseInfo.lpfn = BrowseCallbackProc;
	browseInfo.lParam = (LPARAM)pathBuffer;
	browseInfo.lpszTitle = WASABI_API_LNGSTRINGW(IDS_SELECT_FOLDER_FOR_ERROR_INFO);
	browseInfo.ulFlags = BIF_NEWDIALOGSTYLE;

	// Show the dialog
	LPITEMIDLIST itemIDList = SHBrowseForFolder(&browseInfo);

	// Did user press cancel?
	if (!itemIDList)
		ret = FALSE;

	// Is everything so far?
	if (ret != FALSE) 
	{
		// Get the path from the returned ITEMIDLIST
		if (!SHGetPathFromIDList(itemIDList, pathBuffer))
			ret = FALSE;

		// Now we need to free the ITEMIDLIST the shell allocated
		LPMALLOC	shellMalloc;
		HRESULT		hr;

		// Get pointer to the shell's malloc interface
		hr = SHGetMalloc(&shellMalloc);

		// Did it work?
		if (SUCCEEDED(hr)) 
		{
			shellMalloc->Free(itemIDList);
			shellMalloc->Release();
			ret = TRUE;
		}
	}
	return ret;
}

void CreatePathFromFullName(wchar_t **path, const wchar_t *fullname)
{
	if (*path) free(*path);
	*path = NULL;
	if (!fullname) return;
	const wchar_t *end = wcsrchr(fullname, L'\\');
	if (!end || end == fullname)  return;
	int len = (int)(end - fullname + 1);
	if (len == 3) len++;
	
	*path = (wchar_t*)malloc(len*2);
	StringCchCopyN(*path, len, fullname, len -1);
}

const wchar_t* GetFileName(const wchar_t *fullname)
{
	if (!fullname) return NULL;
	const wchar_t *start = wcsrchr(fullname, L'\\');
	if (start && start != fullname) start = CharNext(start);
	
	return start; 
}