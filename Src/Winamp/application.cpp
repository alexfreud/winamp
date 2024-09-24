/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: application.cpp
 ** Project: Winamp 5
 ** Description: Winamp's implementation of Wasabi's Application API.
 **              Also includes the main message loop.
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "application.h"
#include "../nu/AutoWide.h"
#include "api.h"
#include "../nu/ns_wc.h"

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)

static UINT WINAMP_WM_DIRECT_MOUSE_WHEEL = WM_NULL;
static ATOM DIRECT_MOUSE_WHEEL_CONVERT_TO_MOUSE_WHEEL = 0;

static BOOL DirectMouseWheel_RegisterMessage()
{
	WINAMP_WM_DIRECT_MOUSE_WHEEL = RegisterWindowMessageW(L"WINAMP_WM_DIRECT_MOUSE_WHEEL");
	if (WM_NULL == WINAMP_WM_DIRECT_MOUSE_WHEEL)
		return FALSE;

	if (NULL != application)
		application->DirectMouseWheel_InitBlackList();
	
	return TRUE;
}

BOOL 
IsDirectMouseWheelMessage(const UINT uMsg)
{
	if (WM_NULL == WINAMP_WM_DIRECT_MOUSE_WHEEL)
	{ 
		if (FALSE == DirectMouseWheel_RegisterMessage())
			return FALSE;
	}

	return (WINAMP_WM_DIRECT_MOUSE_WHEEL == uMsg);
}

BOOL 
DirectMouseWheel_EnableConvertToMouseWheel(HWND hwnd, BOOL enable)
{
	if (FALSE != enable)
	{
		if (0 == DIRECT_MOUSE_WHEEL_CONVERT_TO_MOUSE_WHEEL)
		{
			DIRECT_MOUSE_WHEEL_CONVERT_TO_MOUSE_WHEEL = GlobalAddAtomW(L"DIRECT_MOUSE_WHEEL_CONVERT_TO_MOUSE_WHEEL");
			if (0 == DIRECT_MOUSE_WHEEL_CONVERT_TO_MOUSE_WHEEL)
				return FALSE;
		}

		if (0 == SetPropW(hwnd, (const wchar_t*)MAKEINTATOM(DIRECT_MOUSE_WHEEL_CONVERT_TO_MOUSE_WHEEL), (HANDLE)1))
			return FALSE;
	}
	else
	{
		if (0 != DIRECT_MOUSE_WHEEL_CONVERT_TO_MOUSE_WHEEL)
			RemovePropW(hwnd, (const wchar_t*)MAKEINTATOM(DIRECT_MOUSE_WHEEL_CONVERT_TO_MOUSE_WHEEL));
	}
	
	return TRUE;
}

BOOL 
DirectMouseWheel_IsConvertToMouseWheelEnabled(HWND hwnd)
{
	return(0 != DIRECT_MOUSE_WHEEL_CONVERT_TO_MOUSE_WHEEL &&
			NULL != GetPropW(hwnd, (const wchar_t*)MAKEINTATOM(DIRECT_MOUSE_WHEEL_CONVERT_TO_MOUSE_WHEEL)));
}

Application::Application() 
	: shuttingdown(0), activeDialog(NULL), machineID(GUID_NULL), userID(GUID_NULL), sessionID(GUID_NULL),
	  threadStorageIndex(TLS_OUT_OF_INDEXES), messageHook(NULL), disableMessageHook(false)
{
	tlsIndex = TlsAlloc();
}

Application::~Application()
{
	if (NULL != messageHook)
	{
		UnhookWindowsHookEx(messageHook);
		messageHook = NULL;
	}
}

const wchar_t *Application::main_getAppName()
{
	return WIDEN(APP_NAME);
}

const wchar_t *Application::main_getVersionString()
{
	return WIDEN(APP_NAME) L" " WIDEN(APP_VERSION_STRING);
}

const wchar_t *Application::main_getVersionNumString()
{
	return WIDEN(APP_VERSION);
}

unsigned int Application::main_getBuildNumber()
{
	return BUILD_NUMBER;
}

// a guid for our app : {4BE592C7-6937-426a-A388-ACF0EBC88E93}
static const GUID WinampGUID =
  {
    0x4be592c7, 0x6937, 0x426a, { 0xa3, 0x88, 0xac, 0xf0, 0xeb, 0xc8, 0x8e, 0x93 }
  };

GUID Application::main_getGUID()
{
	return WinampGUID;
}

HANDLE Application::main_getMainThreadHandle()
{
	if (hMainThread == 0)
		return (HANDLE)0;
	HANDLE h = (HANDLE)0;
	DuplicateHandle(GetCurrentProcess(), hMainThread, GetCurrentProcess(), &h, 0, FALSE, DUPLICATE_SAME_ACCESS);
	return h;
}

HINSTANCE Application::main_gethInstance()
{
	return hMainInstance;
}

const wchar_t *Application::main_getCommandLine()
{
	return GetCommandLineW();
}

void Application::main_shutdown(int deferred)
{

	int x = static_cast<int>(SendMessageW(hMainWindow, WM_WA_IPC, 0, IPC_IS_EXIT_ENABLED));

	if (!x)
		return;

	shuttingdown = 1;

	SendMessageW(hMainWindow, WM_CLOSE, 0, 0);
}

void Application::main_cancelShutdown()
{
	shuttingdown = 0;
}

int Application::main_isShuttingDown()
{
	return shuttingdown;
}

const wchar_t *Application::path_getAppPath()
{
	return PROGDIR;
}

const wchar_t *Application::path_getUserSettingsPath()
{
	return CONFIGDIR;
}

const wchar_t *Application::path_getSkinSettingsPath()
{
	return SKINDIR;
}

int Application::app_getInitCount()
{
	return 1;
}

void Application::app_addMessageProcessor(api_messageprocessor *processor)
{
	messageProcessors.push_back(processor);
}

void Application::app_removeMessageProcessor(api_messageprocessor *processor)
{
	//messageProcessors.eraseAll(processor);
	auto it = messageProcessors.begin();
	while (it != messageProcessors.end())
	{
		if (*it != processor)
		{
			it++;
			continue;
		}

		it = messageProcessors.erase(it);
	}
}

void Application::app_addModelessDialog(HWND hwnd)
{
	OutputDebugStringA( "[Error] 'app_addModelessDialog' removed! Use 'ActiveDialog_Register' instead!\r\n" );
}

void Application::app_removeModelessDialog(HWND hwnd)
{
	OutputDebugStringA( "[Error] 'app_removeModelessDialog' removed!Use 'ActiveDialog_Unregister' instead!\r\n" );
}

void Application::app_addAccelerators(HWND hwnd, HACCEL *phAccel, INT cAccel, UINT translateMode)
{
	AccelMap::iterator accelIterator;
	ACCELNODE *pList;

	if (accelerators.size() > 0)
	{
		accelIterator = accelerators.end();
		do
		{
			accelIterator--;
			if (!IsWindow(accelIterator->first))
			{
				//app_removeAccelerators(accelIterator->first);
				ACCELNODE* pList, * pNode;
				pList = accelIterator->second;
				while (pList)
				{
					pNode = pList;
					pList = (pList->pNext) ? pList->pNext : NULL;
					free(pNode);
				}
				accelIterator = accelerators.erase(accelIterator);
			}
		} while (accelIterator != accelerators.begin() && accelIterator != accelerators.end());
	}

	if (!IsWindow(hwnd) || !phAccel || cAccel <= 0) return;

	accelIterator = accelerators.find(hwnd);
	if(accelIterator != accelerators.end()) 
		pList = accelIterator->second;
	else
	{
		pList = (ACCELNODE*)calloc(1, sizeof(ACCELNODE));
		if(pList)
		{
			accelerators.insert({hwnd, pList});
		}
	}
	if (!pList) return;
	while (pList->pNext) pList = pList->pNext;
	
	while(cAccel--)
	{
		if (*phAccel)
		{
			ACCELNODE *pNode;
			if (pList->hAccel)
			{
				pNode = (ACCELNODE*)calloc(1, sizeof(ACCELNODE));
				pNode->pNext = NULL;
				pList->pNext = pNode;
				pList = pNode;
			}
			else pNode = pList;

			pNode->hAccel = *phAccel;
			pNode->translateMode = translateMode;
			
		}
		phAccel++;
	}
}

void Application::app_removeAccelerators(HWND hwnd)
{
	AccelMap::iterator iter = accelerators.find(hwnd);
	if(iter == accelerators.end()) 
		return;

	ACCELNODE *pList, *pNode;
	pList = iter->second;
	while(pList)
	{
		pNode = pList;
		pList = (pList->pNext) ? pList->pNext : NULL;
		free(pNode);
	}
	accelerators.erase(hwnd);
}

int Application::app_getAccelerators(HWND hwnd, HACCEL *phAccel, INT cchAccelMax, BOOL bGlobal)
{

	if (!hwnd || 0 == accelerators.size()) return 0;
	if ((!phAccel && cchAccelMax) || (phAccel && !cchAccelMax)) return 0;

	AccelMap::iterator accelIterator = accelerators.end();
	INT count = 0;
	
	do
	{
		accelIterator--;
		for (ACCELNODE *pNode = accelIterator->second;  NULL != pNode; pNode = pNode->pNext)
		{
			if (accelIterator->first == hwnd  || 
				(bGlobal && (TRANSLATE_MODE_GLOBAL == pNode->translateMode || 
							(TRANSLATE_MODE_CHILD == pNode->translateMode && IsChild(accelIterator->first, hwnd)))))
			{
				if (phAccel && cchAccelMax) 
				{
					*phAccel = pNode->hAccel;
					phAccel++;
					cchAccelMax--;
				}
				count++;
			}
		}
	}
	while (accelIterator != accelerators.begin() && 	(!phAccel || cchAccelMax));
	
	return count;
}

void Application::app_registerGlobalWindow(HWND hwnd)
{
	for (std::vector<HWND>::const_iterator e = globalWindows.begin(); e != globalWindows.end(); e++)
	{
		if (*e == hwnd) return;
	}
	globalWindows.push_back(hwnd);
}

void Application::app_unregisterGlobalWindow(HWND hwnd)
{
	for (std::vector<HWND>::iterator e = globalWindows.begin(); e != globalWindows.end(); e++)
	{
		if (*e == hwnd) 
		{
			globalWindows.erase(e);
			return;
		}
	}
	
}

bool Application::FilterMessage(MSG *msg) // returns 1 if you should Dispatch the message, will handle TranslateMessage for you
{
	if (msg->hwnd != NULL && msg->hwnd != hMainWindow && msg->hwnd != hEQWindow // && msg.hwnd != hMBWindow
	    && msg->hwnd != hPLWindow && msg->hwnd != hVideoWindow)
	{
		HWND hWndParent = NULL;
		HWND temphwnd = msg->hwnd;
		if (GetClassLong(temphwnd, GCW_ATOM) == (INT)32770)
			hWndParent = temphwnd;

		/*while (*/temphwnd =      GetParent(temphwnd)/*)*/ ;
		{
			if (GetClassLong(temphwnd, GCW_ATOM) == (INT)32770)
				hWndParent = temphwnd;
		}

		if (NULL != hWndParent)
		{
			BOOL processed;
		
			disableMessageHook = true;
			processed = IsDialogMessageW(hWndParent, msg);
			disableMessageHook = false;

			if (FALSE != processed)
				return true;
		}
		
		//if (msg->message == WM_KEYDOWN || msg->message == WM_SYSKEYDOWN ||
		//    msg->message == WM_KEYUP || msg->message == WM_SYSKEYUP)
		//{
			//if (IsChild(hMBWindow,msg.hwnd) && TranslateAccelerator(hMBWindow,hAccel[3],&msg)) continue;
			TranslateMessage(msg);
		//}
	}
	return false;
}
bool Application::isGlobalWindow(HWND hwnd)
{
	for (std::vector<HWND>::const_iterator e = globalWindows.begin(); e != globalWindows.end(); e++)
	{
		if (*e == hwnd || IsChild(*e, hwnd)) return true;
	}
	return false;
}

bool Application::app_translateAccelerators(MSG *msg)
{
	if (accelerators.size() > 0)
	{
		AccelMap::iterator accelIterator = accelerators.end();
		do
		{
			accelIterator--;
			for (ACCELNODE *pNode = accelIterator->second;  NULL != pNode; pNode = pNode->pNext)
			{
				if (((TRANSLATE_MODE_GLOBAL == pNode->translateMode && isGlobalWindow(msg->hwnd)) || 
					accelIterator->first == msg->hwnd || 
					(TRANSLATE_MODE_CHILD == pNode->translateMode && IsChild(accelIterator->first, msg->hwnd))) && 
					TranslateAcceleratorW(accelIterator->first, pNode->hAccel, msg)) 
				{
					return true;
				}
			}
		}
		while (accelIterator != accelerators.begin());
	}
	return false;
}

bool 
Application::DirectMouseWheel_RegisterSkipClass(ATOM klass)
{
	size_t index;

	if (klass < 0xC000)
		return false;
	
	index = directMouseWheelBlackList.size();
	while(index--)
	{
		if (directMouseWheelBlackList[index] == klass)
			return false;
	}

	directMouseWheelBlackList.push_back(klass);

	return true;
}

bool
Application::DirectMouseWheel_UnregisterSkipClass(ATOM klass)
{
	size_t index;

	if (klass < 0xC000)
		return false;
	
	index = directMouseWheelBlackList.size();
	while(index--)
	{
		if (directMouseWheelBlackList[index] == klass)
		{
			directMouseWheelBlackList.erase(directMouseWheelBlackList.begin() + index);
			return true;
		}
	}
	return false;
}

bool 
Application::DirectMouseWheel_EnableConvertToMouseWheel(HWND hwnd, BOOL enable)
{
	return (0 != ::DirectMouseWheel_EnableConvertToMouseWheel(hwnd, enable));
}

void
Application::DirectMouseWheel_InitBlackList()
{
	size_t index;
	WNDCLASSW klassInfo;
	ATOM klassAtom;

	const static LPCWSTR defaultBlackListNames[] = 
	{
		L"msctls_trackbar32",
		L"msctls_updown32",
		L"SysHeader32",
	};

	const static ATOM defaultBlackListAtoms[] =
	{		
		0xC017, // button
		0xC018, // edit
		0xC019, // static
		0xC01C, // combobox
	};

	for (index = 0; index < ARRAYSIZE(defaultBlackListAtoms); index++)
	{
		directMouseWheelBlackList.push_back(defaultBlackListAtoms[index]);
	}

	for (index = 0; index < ARRAYSIZE(defaultBlackListNames); index++)
	{
		klassAtom = (ATOM)GetClassInfoW(NULL, defaultBlackListNames[index], &klassInfo);
		if (0 != klassAtom)
			directMouseWheelBlackList.push_back(klassAtom);
	}
}

bool
Application::DirectMouseWheel_ProccessMessage(MSG *msg)
{
	HWND targetWindow;
	ATOM targetAtom;
	DWORD targetProcessId;
	POINT mousePoint;
	size_t index, listSize;
	
	if (msg->message != WM_MOUSEWHEEL)
		return false;
	
	if (WM_NULL == WINAMP_WM_DIRECT_MOUSE_WHEEL)
	{
		if (FALSE == DirectMouseWheel_RegisterMessage())
			return false;
	}
		
	POINTSTOPOINT(mousePoint, msg->lParam);
	targetWindow = WindowFromPoint(mousePoint);
		
	if (NULL == targetWindow || targetWindow == msg->hwnd)
		return false;

	GetWindowThreadProcessId(targetWindow, &targetProcessId);
	if (targetProcessId != GetCurrentProcessId())
		return false;

	if (FALSE == IsWindowEnabled(targetWindow))
		return false;

	listSize = directMouseWheelBlackList.size();
	index = 0;

	while(index != listSize)
	{
		targetAtom = (ATOM)GetClassLongPtrW(targetWindow, GCW_ATOM);
		for(index = 0; index < listSize; index++)
		{
			if (targetAtom == directMouseWheelBlackList[index])
			{
				targetWindow = GetAncestor(targetWindow, GA_PARENT);
				if (NULL == targetWindow || targetWindow == msg->hwnd)
					return false;
				break;
			}
		}
	} 

	if (FALSE != DirectMouseWheel_IsConvertToMouseWheelEnabled(targetWindow))
	{
		SendMessageW(targetWindow, WM_MOUSEWHEEL, msg->wParam, msg->lParam);
		return true;
	}
	else if (0 != SendMessageW(targetWindow, WINAMP_WM_DIRECT_MOUSE_WHEEL, msg->wParam, msg->lParam))
		return true;
	
	return false;
}

HWND ActiveChildWindowFromPoint( HWND hwnd, POINTS cursor_s, const int *controls, size_t controlsCount )
{
	POINT pt;
	RECT controlRect;
	HWND controlWindow;

	POINTSTOPOINT( pt, cursor_s );

	while ( controlsCount-- )
	{
		controlWindow = GetDlgItem( hwnd, controls[ controlsCount ] );

		if ( controlWindow != NULL && GetClientRect( controlWindow, &controlRect ) != FALSE )
		{
			MapWindowPoints( controlWindow, HWND_DESKTOP, (POINT *) &controlRect, 2 );

			if ( PtInRect( &controlRect, pt ) != FALSE )
			{
				unsigned long windowStyle = (unsigned long) GetWindowLongPtrW( controlWindow, GWL_STYLE );

				if ( ( ( WS_VISIBLE | WS_DISABLED ) & windowStyle ) == WS_VISIBLE )
					return controlWindow;

				break;
			}
		}
	}

	return NULL;
}

BOOL DirectMouseWheel_ProcessDialogMessage(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam, const int controls[], int controlslen)
{
	if (FALSE != ::IsDirectMouseWheelMessage(uMsg))
	{
		HWND targetWindow = ::ActiveChildWindowFromPoint(hwnd, MAKEPOINTS(lParam), controls, controlslen);
		if (NULL != targetWindow)
		{
			SendMessageW(targetWindow, WM_MOUSEWHEEL, wParam, lParam);
			SetWindowLongPtrW(hwnd, DWLP_MSGRESULT, (long)TRUE);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL Application::DirectMouseWheel_ProcessDialogMessage(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam, const int controls[], int controlslen)
{
	return ::DirectMouseWheel_ProcessDialogMessage(hwnd, uMsg, wParam, lParam, controls, controlslen);
}

void Application::ActiveDialog_Register(HWND hwnd)
{
	activeDialog = hwnd;
}

void Application::ActiveDialog_Unregister(HWND hwnd)
{
	if (hwnd == activeDialog)
		activeDialog = NULL;
}

HWND Application::ActiveDialog_Get()
{
	return activeDialog;
}

/* Plugins can register a 'Message Processor' to tap into the main message loop
  for some purposes (Dialog message handling, ActiveX/ATL/COM, wxWidgets)
	this is the only way to make sure all messages are processed correctly
	@see api_application::app_addMessageProcessor() and api_messageprocessor.h
	*/

bool Application::ProcessMessageLight( MSG *msg )
{
	/* messageProcessors is the list of registered Message Processors */
	for ( api_messageprocessor *processor : messageProcessors )
	{
		disableMessageHook = true;

		if ( processor->ProcessMessage( msg ) ) // it'll return true if it wants to eat the message
		{
			disableMessageHook = false;

			return true;
		}
	}

	disableMessageHook = false;

	if ( false != DirectMouseWheel_ProccessMessage( msg ) )
		return true;

	return false;
}

bool Application::ProcessMessage( MSG *msg )
{
	if ( ProcessMessageLight( msg ) != false )
		return true;

	if ( ( msg->message == WM_KEYDOWN || msg->message == WM_SYSKEYDOWN ) && app_translateAccelerators( msg ) )
		return true;


	if ( activeDialog != NULL )
	{
		disableMessageHook = true;

		if ( IsDialogMessageW( activeDialog, msg ) != FALSE )
		{
			disableMessageHook = false;

			return true;
		}
	}

	disableMessageHook = false;

	return false;
}

intptr_t Application::app_messageLoopStep()
{
	MSG msg;

	if (PeekMessageW(&msg, NULL, 0, 0, TRUE))
	{
		if (msg.message == WM_QUIT)
		{
			PostQuitMessage((int)msg.wParam);
			return msg.wParam ; // abandoned due to WM_QUIT
		}

		if (!ProcessMessage(&msg) && !FilterMessage(&msg))
			DispatchMessageW(&msg);

		if (WM_NCDESTROY == msg.message)
			DirectMouseWheel_EnableConvertToMouseWheel(msg.hwnd, FALSE);
		
		return msg.wParam;
	}

	return 0;
}

LRESULT CALLBACK Application::MessageHookProc( INT code, WPARAM wParam, LPARAM lParam )
{
	if ( application == NULL || application->messageHook == NULL )
	{
		return FALSE;
	}

	if ( application->disableMessageHook == false )
	{
		switch ( code )
		{
			case MSGF_DIALOGBOX:
			case MSGF_MESSAGEBOX:
				if ( false != application->ProcessMessageLight( (MSG *) lParam ) )
					return 1;
				break;
		}
	}
	else
		application->disableMessageHook = false;

	return CallNextHookEx( application->messageHook, code, wParam, lParam );
}

WPARAM Application::MessageLoop()
{
	if ( messageHook == NULL )
		messageHook = SetWindowsHookEx( WH_MSGFILTER, MessageHookProc, NULL, GetCurrentThreadId() );

	for ( ;;)
	{
		DWORD dwStatus = MsgWaitForMultipleObjectsEx( 0, NULL, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE | MWMO_INPUTAVAILABLE );

		if ( dwStatus == WAIT_OBJECT_0 )
		{
			MSG msg;
			while ( PeekMessageW( &msg, NULL, 0, 0, PM_REMOVE ) )
			{
				if ( msg.message == WM_QUIT )
					return msg.wParam;

				if ( !ProcessMessage( &msg ) && !FilterMessage( &msg ) )
					DispatchMessageW( &msg );
			}
		}
	}
}

WPARAM WinampMessageLoop()
{
	return WASABI_API_APP->MessageLoop();
}

const wchar_t *Application::path_getWorkingPath()
{
	return config_cwd;
}

void Application::path_setWorkingPath(const wchar_t *newPath)
{
	StringCchCopyW(config_cwd, MAX_PATH, newPath);
}

/*
int Application::GetMachineID(GUID *id)
{
	// TODO: stuff this in the registry
	if (machineID == GUID_NULL)
	{
		if (!GetPrivateProfileStruct(app_name, "mid", (LPVOID)&machineID, sizeof(machineID), INI_FILE) || machineID == GUID_NULL)
		{
			UuidCreate(&machineID);
			WritePrivateProfileStruct(app_name, "mid", (LPVOID)&machineID, sizeof(machineID), INI_FILE);
		}
	}

	*id = machineID;
	return API_APPLICATION_SUCCESS;
}
*/
int Application::GetUserID(GUID *id)
{
	if (userID == GUID_NULL)
	{
		config_uid_ft = (char)GetPrivateProfileIntW(L"Winamp", L"uid_ft", 0, INI_FILE);

		wchar_t uid_str[512] = {0};
		if (!GetPrivateProfileStructA(app_name, "uid", (LPVOID)&userID, sizeof(userID), INI_FILEA) || userID == GUID_NULL)
		{
			// attempt to restore the client uid after an uninstall, lost config file, etc
			readwrite_client_uid(0, uid_str);
			if (uid_str[0])
			{
				WritePrivateProfileStringW(AutoWide(app_name), L"uid", uid_str, INI_FILE);
			}

			if (!GetPrivateProfileStructA(app_name, "uid", (LPVOID)&userID, sizeof(userID), INI_FILEA) || userID == GUID_NULL)
			{
				UuidCreate(&userID);
				WritePrivateProfileStructA(app_name, "uid", (LPVOID)&userID, sizeof(userID), INI_FILEA);
				GetPrivateProfileStringW(AutoWide(app_name), L"uid", L"", uid_str, ARRAYSIZE(uid_str), INI_FILE);
				readwrite_client_uid(1, uid_str);
				// if done then no need to re-do it at a later point
				config_uid_ft = 1;
			}
		}

		// and just to make sure, if this is like a new run then we'll need to force into the registry
		if (!config_uid_ft)
		{
			config_uid_ft = 1;
			GetPrivateProfileStringW(AutoWide(app_name), L"uid", L"", uid_str, ARRAYSIZE(uid_str), INI_FILE);
			readwrite_client_uid(1, uid_str);
		}
	}

	*id = userID;

	return API_APPLICATION_SUCCESS;
}

int Application::GetSessionID(GUID *id)
{
	if (sessionID == GUID_NULL)
		UuidCreate(&sessionID);

	*id = sessionID;
	return API_APPLICATION_SUCCESS;
}

size_t Application::AllocateThreadStorage()
{
	return (size_t)InterlockedIncrement(&threadStorageIndex);
}

void *Application::GetThreadStorage(size_t index)
{
	std::vector<void*> *ptrlist = (std::vector<void*> *)TlsGetValue(tlsIndex);
	if (!ptrlist)
		return 0;
	
	if ((index+1) > ptrlist->size())
		return 0;
	
	return ptrlist->at(index);
}

void Application::SetThreadStorage(size_t index, void *value)
{
	std::vector<void*> *ptrlist = (std::vector<void*> *)TlsGetValue(tlsIndex);
	if (!ptrlist)
	{
		ptrlist = new std::vector<void*>;
		TlsSetValue(tlsIndex, ptrlist);
	}
	size_t ptrlist_size = ptrlist->size();
	if ((index+1) > ptrlist_size)
	{
		ptrlist->reserve(index+1);
		for (size_t i=ptrlist_size;i<=index;i++)
		{
			ptrlist->push_back(0);
		}
	}
	ptrlist->at(index) = value;
}

const wchar_t *Application::getATFString()
{
	return config_titlefmt;
}

int Application::getScaleX(int x)
{
	return ScaleX(x);
}

int Application::getScaleY(int y)
{
	return ScaleY(y);
}

#define CBCLASS Application
START_DISPATCH;
CB(API_APPLICATION_MAIN_GETAPPNAME, main_getAppName)
CB(API_APPLICATION_MAIN_GETVERSIONSTRING, main_getVersionString)
CB(API_APPLICATION_MAIN_GETVERSIONSTRING2, main_getVersionNumString)
CB(API_APPLICATION_MAIN_GETBUILDNUMBER, main_getBuildNumber)
CB(API_APPLICATION_MAIN_GETGUID, main_getGUID)
CB(API_APPLICATION_MAIN_GETMAINTHREADHANDLE, main_getMainThreadHandle)
CB(API_APPLICATION_MAIN_GETHINSTANCE, main_gethInstance)
CB(API_APPLICATION_MAIN_GETCOMMANDLINE, main_getCommandLine)
VCB(API_APPLICATION_MAIN_SHUTDOWN, main_shutdown)
VCB(API_APPLICATION_MAIN_CANCELSHUTDOWN, main_cancelShutdown)
CB(API_APPLICATION_MAIN_ISSHUTTINGDOWN, main_isShuttingDown)
CB(API_APPLICATION_PATH_GETAPPPATH, path_getAppPath)
CB(API_APPLICATION_PATH_GETUSERSETTINGSPATH, path_getUserSettingsPath)
CB(API_APPLICATION_PATH_GETSKINSETTINGSPATH, path_getSkinSettingsPath)
CB(API_APPLICATION_APP_GETINITCOUNT, app_getInitCount)
CB(API_APPLICATION_APP_MESSAGELOOPSTEP, app_messageLoopStep)
VCB(API_APPLICATION_APP_ADDMESSAGEPROCESSOR, app_addMessageProcessor)
VCB(API_APPLICATION_APP_REMOVEMESSAGEPROCESSOR, app_removeMessageProcessor)
VCB(API_APPLICATION_APP_ADDMODELESSDIALOG, app_addModelessDialog)
VCB(API_APPLICATION_APP_REMOVEMODELESSDIALOG, app_removeModelessDialog)
VCB(API_APPLICATION_APP_ADDACCELERATORS, app_addAccelerators)
VCB(API_APPLICATION_APP_REMOVEACCELERATORS, app_removeAccelerators)
CB(API_APPLICATION_APP_TRANSLATEACCELERATORS, app_translateAccelerators);
CB(API_APPLICATION_APP_GETACCELERATORS, app_getAccelerators);
VCB(API_APPLICATION_APP_REGISTERGLOBALWINDOW, app_registerGlobalWindow);
VCB(API_APPLICATION_APP_UNREGISTERGLOBALWINDOW, app_unregisterGlobalWindow);
CB(API_APPLICATION_PATH_GETWORKINGPATH, path_getWorkingPath);
VCB(API_APPLICATION_PATH_SETWORKINGPATH, path_setWorkingPath);
CB(API_APPLICATION_DIRECTMOUSEWHEEL_REGISTERSKIPCLASS, DirectMouseWheel_RegisterSkipClass);
CB(API_APPLICATION_DIRECTMOUSEWHEEL_UNREGISTERSKIPCLASS, DirectMouseWheel_UnregisterSkipClass);
CB(API_APPLICATION_DIRECTMOUSEWHEEL_ENABLECONVERTTOMOUSEWHEEL, DirectMouseWheel_EnableConvertToMouseWheel);
CB(API_APPLICATION_DIRECTMOUSEWHEEL_PROCESSDIALOGMESSAGE, DirectMouseWheel_ProcessDialogMessage);
VCB(API_APPLICATION_ACTIVEDIALOG_REGISTER, ActiveDialog_Register);
VCB(API_APPLICATION_ACTIVEDIALOG_UNREGISTER, ActiveDialog_Unregister);
CB(API_APPLICATION_ACTIVEDIALOG_GET, ActiveDialog_Get);
/*
CB(API_APPLICATION_GETMACHINEID, GetMachineID);
*/
CB(API_APPLICATION_GETUSERID, GetUserID);
CB(API_APPLICATION_GETSESSIONID, GetSessionID);
CB(API_APPLICATION_ALLOCATETHREADSTORAGE, AllocateThreadStorage);
CB(API_APPLICATION_GETTHREADSTORAGE, GetThreadStorage);
VCB(API_APPLICATION_SETTHREADSTORAGE, SetThreadStorage);
CB(API_APPLICATION_GETATFSTRING, getATFString);
CB(API_APPLICATION_GETSCALEX, getScaleX);
CB(API_APPLICATION_GETSCALEY, getScaleY);
END_DISPATCH;
#undef CBCLASS