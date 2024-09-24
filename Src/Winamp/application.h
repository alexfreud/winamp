#ifndef NULLSOFT_WINAMP_APPLICATION_H
#define NULLSOFT_WINAMP_APPLICATION_H

#include <api/application/api_application.h>
#include <vector>
#include <map>

class Application : public api_application
{
public:
	static const char *getServiceName() { return "Application API"; }
	static const GUID getServiceGuid() { return applicationApiServiceGuid; }
public:
	Application();
	~Application();

	const wchar_t *main_getAppName();
	const wchar_t *main_getVersionString();
	const wchar_t *main_getVersionNumString();
	unsigned int main_getBuildNumber();
	GUID main_getGUID();
	HANDLE main_getMainThreadHandle();
	HINSTANCE main_gethInstance();
	const wchar_t *main_getCommandLine();
	void main_shutdown(int deferred = TRUE);
	void main_cancelShutdown();
	int main_isShuttingDown();
	const wchar_t *path_getAppPath();
	const wchar_t *path_getUserSettingsPath();
	// added for 5.58+ so gen_ff can fill @SKINSPATH@ in scripts correctly
	const wchar_t *path_getSkinSettingsPath();
	int app_getInitCount();
	intptr_t app_messageLoopStep();
	void app_addMessageProcessor(api_messageprocessor *processor);
	void app_removeMessageProcessor(api_messageprocessor *processor);

	void app_addModelessDialog(HWND hwnd);
	void app_removeModelessDialog(HWND hwnd);
	// added for 5.34
	const wchar_t *path_getWorkingPath();
	void path_setWorkingPath(const wchar_t *newPath);
	// added for 5.35
	/*
	int GetMachineID(GUID *id); 
	*/
	int GetUserID(GUID *id);

	int GetSessionID(GUID *id);

    WPARAM MessageLoop();

	// added for 5.53
	bool app_translateAccelerators(MSG *msg);
	void app_addAccelerators(HWND hwnd, HACCEL *phAccel, INT cAccel, UINT translateMode);
	void app_removeAccelerators(HWND hwnd);
	int app_getAccelerators(HWND hwnd, HACCEL *phAccel, INT cchAccelMax, BOOL bGlobal);

	// added for 5.54
	void app_registerGlobalWindow(HWND hwnd);
	void app_unregisterGlobalWindow(HWND hwnd);
	bool isGlobalWindow(HWND hwnd);

	/* 5.54 + */
	size_t AllocateThreadStorage(); // returns an index, -1 for error
	void *GetThreadStorage(size_t index);
	void SetThreadStorage(size_t index, void *value);

	/* 5.58 + */
	bool DirectMouseWheel_RegisterSkipClass(ATOM klass);
	bool DirectMouseWheel_UnregisterSkipClass(ATOM klass);
	bool DirectMouseWheel_EnableConvertToMouseWheel(HWND hwnd, BOOL enable);
	/* 5.64 + */
	BOOL DirectMouseWheel_ProcessDialogMessage(HWND hwnd, unsigned int uMsg, WPARAM wParam, LPARAM lParam, const int controls[], int controlslen);

	/* 5.61 + */
	void ActiveDialog_Register(HWND hwnd);
	void ActiveDialog_Unregister(HWND hwnd);
	HWND ActiveDialog_Get();

	/* 5.64 + */
	const wchar_t *getATFString();	// returns the current ATF formatting string

	/* 5.66 + */
	// used for dpi scaling so we're consistent in usage throughout the UI, etc
	int getScaleX(int x);
	int getScaleY(int y);
private:
	RECVS_DISPATCH;
	bool ProcessMessageLight(MSG *msg);
	bool ProcessMessage(MSG *msg);
	bool FilterMessage(MSG *msg);
	bool DirectMouseWheel_ProccessMessage(MSG *msg);
	void DirectMouseWheel_InitBlackList();
	friend static BOOL DirectMouseWheel_RegisterMessage();
	static LRESULT CALLBACK MessageHookProc(INT code, WPARAM wParam, LPARAM lParam);

private:
	typedef struct __ACCELNODE
	{
		HACCEL		hAccel;
		UINT		translateMode;	
		__ACCELNODE	*pNext;
	} ACCELNODE;

	typedef std::map<HWND, ACCELNODE*> AccelMap;

private:
	int shuttingdown;
	std::vector<api_messageprocessor*> messageProcessors;
	HWND activeDialog;
			
	AccelMap accelerators;
	std::vector<HWND> globalWindows;
	std::vector<ATOM> directMouseWheelBlackList;

	GUID machineID, userID, sessionID;

	DWORD tlsIndex;
	LONG threadStorageIndex;

	HHOOK messageHook;
	bool disableMessageHook;
};

extern Application *application;

#endif