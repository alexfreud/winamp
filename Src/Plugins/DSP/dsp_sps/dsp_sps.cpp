#include <windows.h>
#include <commctrl.h>
#include "../winamp/dsp.h"
#include "resource.h"
#include "sps_common.h"
#include "../winamp/wa_ipc.h"
#include "../../General/gen_hotkeys/wa_hotkeys.h"
#define SPS_HOTKEY_ID "dsp_sps sc"
#include "sps_configdlg.h"

//#define PLUGIN_NAME		"Nullsoft Signal Processing Studio DSP v1.0b"
#define PLUGIN_VERSION	"v1.0b"

  // config, winamp specific shit here:
struct
{
	int showeditor; //def 0
	int visible; //def 1
} g_config;

SPSEffectContext g_wacontext;
HWND g_configwindow;
HWND helpWnd;
int helpWndOpenHack = 0;
char *INI_FILE;
static int loaded_once = 0;
char g_path[MAX_PATH];

// wasabi based services for localisation support
api_service  *WASABI_API_SVC       = 0;
api_language *WASABI_API_LNG       = 0;
HINSTANCE     WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

// module getter.
winampDSPModule *getModule(int which);

void config(struct winampDSPModule *this_mod);
int init(struct winampDSPModule *this_mod);
void quit(struct winampDSPModule *this_mod);
int modify_samples(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate);

// Module header, includes version, description, and address of the module retriever function
typedef struct
{
	int version;       // DSP_HDRVER
	char *description; // description of library
	winampDSPModule* (*getModule)(int);	// module retrieval function
	int (*sf)(int);
} winampDSPHeaderEx;

static int sf(int v)
{
	int res;
	res = v * (unsigned long)1103515245;
	res += (unsigned long)13293;
	res &= (unsigned long)0x7FFFFFFF;
	res ^= v;
	return res;
}

winampDSPHeaderEx hdr = { DSP_HDRVER+1, 0, getModule, sf };

// first module
winampDSPModule mod =
{
	0,//"Signal Processing Studio",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	config,
	init,
	modify_samples,
	quit
};

extern "C" 
{
	static HINSTANCE GetMyInstance()
	{
		MEMORY_BASIC_INFORMATION mbi = {0};

		if(VirtualQuery(GetMyInstance, &mbi, sizeof(mbi)))
			return (HINSTANCE)mbi.AllocationBase;

		return NULL;
	}

	__declspec( dllexport ) winampDSPHeaderEx *winampDSPGetHeader2(HWND hwndParent)
	{
		if(IsWindow(hwndParent))
		{
			if(!WASABI_API_LNG_HINST)
			{
				// loader so that we can get the localisation service api for use
				WASABI_API_SVC = (api_service*)SendMessageW(hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);

				if (WASABI_API_SVC == (api_service*)1)
					WASABI_API_SVC = NULL;

				waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);

				if (sf)
					WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

				// need to have this initialised before we try to do anything with localisation features
				WASABI_API_START_LANG(GetMyInstance(),DspSpsLangGUID);
			}

			static char szDescription[256];
			if(!szDescription[0])
			{
				char temp[256];
				wsprintfA(szDescription,"%s %s",WASABI_API_LNGSTRING_BUF(IDS_SPS_TITLE,temp,256), PLUGIN_VERSION );
				hdr.description = szDescription;
			}

			static char szDescription2[256];
			if(!szDescription2[0])
			{
				mod.description = WASABI_API_LNGSTRING_BUF(IDS_SPS_MODULE_TITLE,szDescription2,256);
			}
		}
		return &hdr;
	}
}

// getmodule routine from the main header. Returns NULL if an invalid module was requested,
// otherwise returns either mod1 or mod2 depending on 'which'.
winampDSPModule *getModule(int which)
{
	switch (which)
	{
		case 0: return &mod;
		default:return NULL;
	}
}

void config(struct winampDSPModule *this_mod)
{
	// show config
	if (g_configwindow && IsWindow(g_configwindow))
	{
		g_config.visible=1;
		ShowWindow(g_configwindow,SW_SHOW);
	}
}

static void WriteInt(char *section, char *name,int value, char *fn)
{
	char str[128];
	wsprintf(str,"%d",value);
	WritePrivateProfileString(section,name,str,fn);
}

static char ghkStr[64];
static genHotkeysAddStruct sps_ghas_showconfig = {
	ghkStr,
	0,
	WM_USER+0x80,
	0,
	0,
	SPS_HOTKEY_ID,
	0,
};
static int m_genhotkeys_add_ipc;

int init(struct winampDSPModule *this_mod)
{
	wsprintf(g_path,"%s\\dsp_sps",(char*)SendMessageW(this_mod->hwndParent,WM_WA_IPC,0,IPC_GETPLUGINDIRECTORY));
	CreateDirectory(g_path,NULL);

	// loader so that we can get the localisation service api for use
	WASABI_API_SVC = (api_service*)SendMessageW(this_mod->hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (WASABI_API_SVC == (api_service*)1)
		WASABI_API_SVC = NULL;

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf)
		WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(this_mod->hDllInstance,DspSpsLangGUID);

	SPS_initapp();

	SPS_initcontext(&g_wacontext);

	/* read config */
	INI_FILE = (char*)SendMessageW(this_mod->hwndParent,WM_WA_IPC,0,IPC_GETINIFILE);
	g_config.showeditor=GetPrivateProfileInt("DSP_SPS","showeditor",0,INI_FILE);
	g_config.visible=GetPrivateProfileInt("DSP_SPS","visible",1,INI_FILE);

	g_wacontext.bypass=GetPrivateProfileInt("DSP_SPS","bypass",1,INI_FILE);
	SPS_load_preset(&g_wacontext,INI_FILE,"DSP_SPS");
	GetPrivateProfileString("DSP_SPS","last_preset","",g_wacontext.curpreset_name,sizeof(g_wacontext.curpreset_name),INI_FILE);

	g_configwindow=WASABI_API_CREATEDIALOGPARAM(IDD_DIALOG1,this_mod->hwndParent,SPS_configWindowProc,(LPARAM)&g_wacontext);

	// we are starting minimised so process as needed (keep our window hidden)
	if(!loaded_once)
	{
		loaded_once = 1;
		if (g_config.visible)
			ShowWindow(g_configwindow,!GetPrivateProfileInt("Winamp","minimized",1,INI_FILE)?SW_SHOWNA:SW_SHOWMINNOACTIVE);
	}
	else{
		if (g_config.visible)
			ShowWindow(g_configwindow,SW_SHOWNA);
	}

	m_genhotkeys_add_ipc=SendMessageW(this_mod->hwndParent,WM_WA_IPC,(WPARAM)&"GenHotkeysAdd",IPC_REGISTER_WINAMP_IPCMESSAGE);

	WASABI_API_LNGSTRING_BUF(IDS_SPS_SHOW_CONFIG,ghkStr,64);
	sps_ghas_showconfig.flags &= ~HKF_DISABLED;
	sps_ghas_showconfig.wnd = g_configwindow;

	if (m_genhotkeys_add_ipc > 65536)
		PostMessageW(this_mod->hwndParent,WM_WA_IPC,(WPARAM)&sps_ghas_showconfig,m_genhotkeys_add_ipc); //post so gen_hotkeys will catch it if not inited yet

	return 0;
}

void quit(struct winampDSPModule *this_mod)
{
	if(IsWindow(helpWnd))
	{
		DestroyWindow(helpWnd);
		helpWndOpenHack = 1;
	}

	helpWnd = 0;

	if(IsWindow(g_configwindow))
	{
		DestroyWindow(g_configwindow);
	}

	g_configwindow=0;

	/* write config */
	WriteInt("DSP_SPS","showeditor",g_config.showeditor,INI_FILE);
	WriteInt("DSP_SPS","visible",g_config.visible,INI_FILE);

	WritePrivateProfileString("DSP_SPS","last_preset",g_wacontext.curpreset_name,INI_FILE);
	WriteInt("DSP_SPS","bypass",g_wacontext.bypass,INI_FILE);
	SPS_save_preset(&g_wacontext,INI_FILE,"DSP_SPS");

	SPS_quitcontext(&g_wacontext);
	SPS_quitapp();

	sps_ghas_showconfig.flags |= HKF_DISABLED;
	sps_ghas_showconfig.wnd=0;

	if (m_genhotkeys_add_ipc > 65536) 
	{
		DWORD d;
		SendMessageTimeout(this_mod->hwndParent,WM_WA_IPC,(WPARAM)&sps_ghas_showconfig,m_genhotkeys_add_ipc,SMTO_BLOCK|SMTO_ABORTIFHUNG,500,&d);
	}

	// helps to work around a crash on close issue when the help dialog is open
	if(helpWndOpenHack)
	{
		char buf[MAX_PATH];
		GetModuleFileName(this_mod->hDllInstance, buf, MAX_PATH);
		LoadLibrary(buf);
	}
}

int modify_samples(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate)
{
	return SPS_process_samples(&g_wacontext,samples,numsamples,0,bps,nch,srate,numsamples*2,1);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstDLL);
	}

	return TRUE;
}

extern "C" __declspec( dllexport ) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param)
{
	// force plugin to be uninstalled from a restart so that we can deal with the case of the help dialog being open
	return helpWndOpenHack;
}

//////////// common dialog stuff

#define SPS_CONFIGDLG_IMPL
#define SPS_CONFIGDLG_ON_WM_CLOSE { ShowWindow(hwndDlg,SW_HIDE); g_config.visible=0; }
#define SPS_CONFIGDLG_HIDEABLE_EDITOR g_config.showeditor
#include "sps_configdlg.h"