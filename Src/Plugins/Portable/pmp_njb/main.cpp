//#define PLUGIN_NAME "Nullsoft Creative NJB Plug-in"
#define PLUGIN_VERSION L"0.57"

#include "NJBDevice.h"
#include "../Winamp/wa_ipc.h"

#define WM_PMP_NJB_DEVICE_CONNECTED (WM_USER+23)
int init();
void quit();
intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3);

PMPDevicePlugin plugin = {PMPHDR_VER, 0, init, quit, MessageProc};

LRESULT CALLBACK CallbackWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

C_ItemList devices;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

ICTJukebox2 * m_pCTJukebox2 = NULL;
HWND mainMessageWindow = 0;
static bool classRegistered = 0;
HWND CreateDummyWindow()
{
	if (!classRegistered)
	{
		WNDCLASSW wc = {0, };

		wc.style = 0;
		wc.lpfnWndProc = CallbackWndProc;
		wc.hInstance = plugin.hDllInstance;
		wc.hIcon = 0;
		wc.hCursor = NULL;
		wc.lpszClassName = L"pmp_njb_window";

		if (!RegisterClassW(&wc))
			return 0;

		classRegistered = true;
	}
	HWND dummy = CreateWindowW(L"pmp_njb_window", L"pmp_njb_window", 0, 0, 0, 0, 0, NULL, NULL, plugin.hDllInstance, NULL);

	return dummy;
}

int init()
{
	waServiceFactory *sf = plugin.service->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,PmpNJBLangGUID);

	static wchar_t szDescription[256];
	swprintf(szDescription, ARRAYSIZE(szDescription),
							WASABI_API_LNGSTRINGW(IDS_NULLSOFT_CREATIVE_NJB), PLUGIN_VERSION);
	plugin.description = szDescription;

	OleInitialize(NULL);
	HRESULT hr = CoCreateInstance(CLSID_CTJukeBox2, NULL, CLSCTX_ALL, IID_ICTJukebox2, (void**) & m_pCTJukebox2);
	if (hr != S_OK) return 0;

	hr = m_pCTJukebox2->Initialize2();
	if (hr != S_OK)
	{
		m_pCTJukebox2->Release();
		m_pCTJukebox2=0;
		return 0;
	}

	mainMessageWindow = CreateDummyWindow();
	m_pCTJukebox2->SetCallbackWindow2(0, (long)mainMessageWindow);
	
	long devCount = 0;
	m_pCTJukebox2->GetDeviceCount2(&devCount);
	for (long i = 1; i <= devCount; i++)
		PostMessage(mainMessageWindow, WM_PMP_NJB_DEVICE_CONNECTED, i, 0);

	return 0;
}

void quit()
{

	if (m_pCTJukebox2)
	{
		m_pCTJukebox2->ShutDown2();
		m_pCTJukebox2->Release();
		m_pCTJukebox2 = 0;
	}
	DestroyWindow(mainMessageWindow);
	OleUninitialize();
}

LRESULT CALLBACK CallbackWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PMP_NJB_DEVICE_CONNECTED:
		{
      new NJBDevice((int)wParam);
		}
		break;
	case WM_DAPSDK_JUKEBOX_REMOVAL:
		{
			long devCount = 0;
			if (m_pCTJukebox2->GetDeviceCount2(&devCount) == S_OK)
			{
				if (devCount == 0) // TODO benski> shouldn't we always register for messages?
					m_pCTJukebox2->SetCallbackWindow2(0, (long)mainMessageWindow);
				for (long i = 0; i < devices.GetSize(); /*i++*/)
				{
					NJBDevice * dev = (NJBDevice *)devices.Get(i);
					bool attached = false;
					for (long j = 1; j <= devCount; j++)
					{
						BYTE * ptr = NULL;
						if (m_pCTJukebox2->GetDeviceProperties(j, kDeviceSerialNumberValue, (IUnknown*)ptr) == S_OK)
						{
							if (memcmp(ptr, dev->serial, 16) == 0)
							{
								attached = true;
								dev->id = j;
								break;
							}
							//free(ptr);
						}
					}
					if (!attached)
					{
            SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)dev,PMP_IPC_DEVICEDISCONNECTED);
						delete dev;
					}
					else
						i++;
				}
			}
		}
		break;
	case WM_DAPSDK_JUKEBOX_ARRIVAL:
		{
			long devCount = 0;
      if (m_pCTJukebox2->GetDeviceCount2(&devCount) == S_OK)
        new NJBDevice(devCount);
		}
		break;
	default:
		{
			for (int i = 0; i < devices.GetSize(); i++)
			{
				NJBDevice * dev = (NJBDevice *)devices.Get(i);
				if (dev->messageWindow == hwnd)
					return dev->WindowMessage(hwnd, uMsg, wParam, lParam);
			}
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);;
}

intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3) {
	switch(msg) {
		case PMP_DEVICECHANGE:
			return 0;
		case PMP_NO_CONFIG:
			return TRUE;
		case PMP_CONFIG:
			return 0;
	}
	return 0;
}

extern "C" {
	__declspec( dllexport ) PMPDevicePlugin * winampGetPMPDevicePlugin(){return &plugin;}
	__declspec( dllexport ) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) {
		int i = devices.GetSize();
		while(i-- > 0) ((Device*)devices.Get(i))->Close();
		return PMP_PLUGIN_UNINSTALL_NOW;
	}
};