// These are disabled for now they have unknown issues.
//#define USE_OGG
//#define CAPTURE_TESTING

// TODO / BE NICE FOR FUTURE VERSIONS
// 1) Fix capture issues on Vista / Windows 7
// 2) Allow metadata to be specified from file
// 3) Move over to using enc_lame (and ui changes for it) [partial]

#define APP_Name "Shoutcast Source"
#define APP_Version "2.4.2"
#define APP_VersionW L"2.4.2"
#define APP_Build "449"

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <commdlg.h>
#include <math.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys.h>
#ifdef CAPTURE_TESTING
#include "Wasapi/WASAPICapture.h"
#endif
#include "resource/resource.h"
#include "sc2srclib/include/shoutcast_output.h"
#include "sc2srclib/encoders/c_encoder_mp3dll.h"
#include "sc2srclib/encoders/c_encoder_nsv.h"
#include "sc2srclib/encoders/c_encoder_fhgaac.h"
#include "sc2srclib/encoders/c_encoder_aacp.h"
#ifdef USE_OGG
#include "sc2srclib/Encoders/c_encoder_ogg.h"
#endif
// allows us to compile with the Wasabi sdk without having to change things
//#define __WASABI_TYPES_H
//#define _GUID_H
//typedef unsigned long ARGB32;
//static const GUID INVALID_GUID = { 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0} };
#include "api.h"
#include "include/c_wavein.h"
#include "crossfader/c_crossfader.h"
#include <winamp/wa_ipc.h>
#include <winamp/dsp.h>
#include "nu/servicebuilder.h"
#include "utils.h"
#ifdef CAPTURE_TESTING
#include "wasapi/player.h"
#endif
#include <strsafe.h>

#define NUM_OUTPUTS		5
#define NUM_ENCODERS	NUM_OUTPUTS
#define NUM_BUFFERS		3
#define MAX_TABWNDS		4
#define MAX_COLS		6
#define MAX_CELLS		12
#define MAX_INWNDS		2
#define MAX_OUTWNDS		6
#define SYSTRAY_BASE_ICON	1024
#define SYSTRAY_ICY_ICON	1
#define SYSTRAY_BASE_MSG	WM_USER
#define SYSTRAY_MAXIMIZE_MSG	27
#define DEFAULT_INPUTDEVICE	0 // winamp

#define DOWNLOAD_URL L"http://www.shoutcast.com/BroadcastNow"
// 404, change to one of these?
// #define DOWNLOAD_URL L"http://wiki.shoutcast.com/wiki/Source_DSP_Plug-in"
// #define DOWNLOAD_URL L"http://www.shoutcast.com"

char sourceVersion[64] = {APP_Version "." APP_Build};
static char szDescription[256];
static char szDescription2[256];
static wchar_t szDescription2W[256];

#ifdef CAPTURE_TESTING
static Player *pPlayer = NULL;

//
// The Player object calls the methods in this class to
// notify the application when certain audio events occur.
//
class CPlayerCallbacks : public PlayerCallbacks
{
    // Notification callback for volume change. Typically, the user
    // adjusts the volume through the SndVol.exe application.
    void VolumeChangeCallback(float volume, BOOL mute)
    {
        /*EnableWindowDlgItem(g_hDlg, IDC_SLIDER_VOLUME, TRUE);
        SetWindowText(GetDlgItem(g_hDlg, IDC_STATIC_MUTE),
                      (mute == TRUE) ? L"Mute" : L"");
        PostMessage(GetDlgItem(g_hDlg, IDC_SLIDER_VOLUME),
                    TBM_SETPOS, TRUE, LPARAM(volume*MAX_VOLUME_LEVEL));*/
    };

    // Notification callback for when stream stops playing unexpectedly
    // (typically, because the player reached the end of a wave file).
    void PlayerStopCallback(void)
    {
        /*SetActiveWindow(g_hDlg);
        PostMessage(GetDlgItem(g_hDlg, IDC_BUTTON_STOP), BM_CLICK, 0, 0);*/
    };

    // Notification callback for when the endpoint capture device is
    // disconnected (for example, the user pulls out the microphone plug).
    void CaptureDisconnectCallback(void)
    {
        /*SetWindowText(GetDlgItem(g_hDlg, IDC_STATIC_LASTACTION),
                      L"Capture device disconnected!");
        SendMessage(GetDlgItem(g_hDlg, IDC_COMBO_CAPTUREDEVICE), CB_RESETCONTENT, 0, 0);*/
    };

    // Notification callback for when the endpoint rendering device is
    // disconnected (for example, the user pulls out the headphones plug).
    void RenderDisconnectCallback(void)
    {
        /*SetWindowText(GetDlgItem(g_hDlg, IDC_STATIC_LASTACTION),
                      L"Playback device disconnected!");
        EnableWindowDlgItem(g_hDlg, IDC_SLIDER_VOLUME, FALSE);
        SetWindowTextW(GetDlgItem(g_hDlg, IDC_STATIC_MUTE), L"Disconnected");
        SendMessage(GetDlgItem(g_hDlg, IDC_COMBO_RENDERDEVICE), CB_RESETCONTENT, 0, 0);
        SendMessage(GetDlgItem(g_hDlg, IDC_SLIDER_VOLUME), TBM_SETPOS, TRUE, 0);*/
    };
};


static CPlayerCallbacks *pCallbacks = NULL;
#endif

// this is used to help determine if we're running on an older
// version of Winamp where jnetlib has issues with the re-use
// of connection handles when the connection previously failed
int iscompatibility = 0;
// used for the about page link so we don't cause win2k issues
int isthemethere = 0;
// Wasabi based services for localisation support
api_service *WASABI_API_SVC = 0;
api_config *AGAVE_API_CONFIG = 0;
api_language *WASABI_API_LNG = 0;
api_queue *WASABI_API_QUEUEMGR = 0;
api_albumart *AGAVE_API_ALBUMART = 0;
api_memmgr *WASABI_API_MEMMGR = 0;
api_explorerfindfile* WASABI_API_EXPLORERFINDFILE = 0;
api_downloadManager *WAC_API_DOWNLOADMANAGER = 0;

// these two must be declared as they're used by the language api's
// when the system is comparing/loading the different resources
HINSTANCE WASABI_API_LNG_HINST = 0,
		  WASABI_API_ORIG_HINST = 0;
HFONT boldFont = 0, normalFont = 0;
HICON icy = 0, wa_icy = 0;
static HHOOK nowPlayingHook,
			 nowPlayingHook2;
static LPARAM nowPlayingID = -1;
// just using these to track the paused and playing states
int was_paused = 0,
	was_playing = 0;
DWORD play_duration = 0,
	  play_diff = 0;
int isplaying = -1, ptt_load = 0;
static wchar_t lastFile[MAX_PATH];
int lastFilterIndex = 4;
static int lastSec[NUM_OUTPUTS],
		   lastMode[NUM_OUTPUTS] = {-1, -1, -1, -1, -1},
		   lastEnable[NUM_OUTPUTS];

static HWND buttonWnd[ NUM_OUTPUTS ];
static HWND tabWnd;
static HWND outTabWnd;
static HWND updateWnd;

static ARGB32 *playingImage;
static ARGB32 *streamImage[NUM_OUTPUTS] = {(ARGB32 *)-1, (ARGB32 *)-1, (ARGB32 *)-1, (ARGB32 *)-1, (ARGB32 *)-1};
static int playingImage_w, playingImage_h, playingLength, playingType;
static int streamLength[NUM_OUTPUTS];
static bool secChanged[NUM_OUTPUTS];
void Config(struct winampDSPModule *this_mod);
int Init(struct winampDSPModule *this_mod);
int ModifySamples(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate);
void Quit(struct winampDSPModule *this_mod);
int secureFunc(int key){
	int res = key * (unsigned long)1103515245;
	res += (unsigned long)13293;
	res &= (unsigned long)0x7FFFFFFF;
	res ^= key;
	return res;
}
winampDSPModule *getModule(int which);
winampDSPModule module = {
	"nullsoft(dsp_sc.dll)",
	NULL,
	NULL,
	Config,
	Init,
	ModifySamples,
	Quit,
	NULL
};

winampDSPHeader header = {
	DSP_HDRVER+1,
	"Nullsoft " APP_Name " DSP " APP_Version,
	getModule,
	secureFunc
};


static ARGB32 * writeImg(const ARGB32 *data, int w, int h, int *length, const wchar_t *ext) {
	if (!ext || ext && !*ext) return NULL;
	if (*ext == L'.') ext++;
	FOURCC imgwrite = svc_imageWriter::getServiceType();
	int n = WASABI_API_SVC->service_getNumServices(imgwrite);
	for (int i=0; i<n; i++) {
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgwrite,i);
		if (sf) {
			svc_imageWriter * l = (svc_imageWriter*)sf->getInterface();
			if (l) {
				if (wcsstr(l->getExtensions(),ext)) {
					void* ret = l->convert(data, 32, w, h, length);
					sf->releaseInterface(l);
					return (ARGB32 *)ret;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return NULL;
}

HICON GetICYIcon(bool winamp = false) {
	if (!winamp) {
		if (!icy) {
			icy = (HICON)LoadImage(WASABI_API_ORIG_HINST?WASABI_API_ORIG_HINST:module.hDllInstance,
								   MAKEINTRESOURCE(IDI_ICY), IMAGE_ICON, 0, 0,
								   LR_SHARED | LR_LOADTRANSPARENT | LR_CREATEDIBSECTION);
		}
		return icy;
	} else {
		if (!wa_icy) {
			wa_icy = (HICON)LoadImage(GetModuleHandle("winamp.exe"),
									  MAKEINTRESOURCE(102), IMAGE_ICON, 0, 0,
									  LR_SHARED | LR_LOADTRANSPARENT | LR_CREATEDIBSECTION);
		}
		return (wa_icy ? wa_icy : icy);
	}
}

BOOL InitLocalisation(HWND winamp) {
	// if this is valid then we should be running on Winamp 5.5+ so try to get the localisation api
	if (IsWindow(winamp)) {
		iscompatibility = 1; ////// SendMessage( winamp, WM_WA_IPC, 0, IPC_IS_COMPATIBILITY_ENABLED );
		isthemethere = !SendMessage(winamp, WM_WA_IPC, IPC_ISWINTHEMEPRESENT, IPC_USE_UXTHEME_FUNC);
		if (!WASABI_API_LNG_HINST) {
			// loader so that we can get the localisation service api for use
			WASABI_API_SVC = (api_service*)SendMessage(winamp, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
			if (WASABI_API_SVC == (api_service*)1) {
				WASABI_API_SVC = NULL;
				return FALSE;
			}

			// initialise all of the wasabi based services
			ServiceBuild(WASABI_API_SVC, AGAVE_API_CONFIG, AgaveConfigGUID);
			ServiceBuild(WASABI_API_SVC, WASABI_API_LNG, languageApiGUID);
			ServiceBuild(WASABI_API_SVC, WASABI_API_MEMMGR, memMgrApiServiceGuid);
			ServiceBuild(WASABI_API_SVC, AGAVE_API_ALBUMART, albumArtGUID);
			ServiceBuild(WASABI_API_SVC, WASABI_API_QUEUEMGR, QueueManagerApiGUID);
			ServiceBuild(WASABI_API_SVC, WASABI_API_EXPLORERFINDFILE, ExplorerFindFileApiGUID);
			ServiceBuild(WASABI_API_SVC, WAC_API_DOWNLOADMANAGER, DownloadManagerGUID);

			// need to have this initialised before we try to do anything with localisation features
			WASABI_API_START_LANG(GetMyInstance(), DspShoutcastLangGUID);

			// do this here so if there is no localisation support then the module names go to defaults
			if (!szDescription[0]) {
				StringCchPrintfA(szDescription, ARRAYSIZE(szDescription), LocalisedStringA(IDS_PLUGIN_NAME, NULL, 0), APP_Version);
				header.description = szDescription;
			}

			if (!szDescription2[0]) {
				module.description = LocalisedStringA(IDS_MODULE_NAME, szDescription2, 256);
				LocalisedString(IDS_MODULE_NAME, szDescription2W, 256);
			}
		}
		return TRUE;
	}
	return FALSE;
}

#ifdef __cplusplus
extern "C" {
#endif
	__declspec(dllexport) winampDSPHeader *winampDSPGetHeader2(HWND hwndParent) {
		if (InitLocalisation(hwndParent)) {
			return &header;
		}
		MessageBoxA(module.hwndParent,
					"You are attempting to use the " APP_Name " plug-in in an\n"
					"unsupported version of Winamp or in a non-Winamp install or via a\n"
					"DSP stacker which does not implement the Winamp 5.5+ DSP api.\n\n"
					"To work this plug-in requires Winamp 5.5 and higher (the most current\n"
					"release is recommended) or for the non-Winamp install or DSP stacker\n"
					"to be updated to support the required Winamp api's the plug-in uses.",
					"Nullsoft " APP_Name, MB_ICONEXCLAMATION|MB_APPLMODAL);
		return 0;
	}

	__declspec(dllexport) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) {
		// this isn't ideal but it ensures that we show a localised version of the message
		// if not it'll make sure that we're using the plug-in dll's internal resources
		// though for ease of code handling we have to fill in some of the dsp structures
		// as the plug-in has effectively been unloaded at this stage for the uninstall.
		HWND winamp = GetWinampHWND(0);
		module.hDllInstance = hDllInst;
		module.hwndParent = winamp;
		InitLocalisation(winamp);

		wchar_t title[256] = {0};
		StringCchPrintfW(title, ARRAYSIZE(title), LocalisedString(IDS_PLUGIN_NAME, NULL, 0), APP_Version);

		// prompt to remove the settings files (defaults to no just incase)
		if (MessageBoxW(hwndDlg, LocalisedString(IDS_PLUGIN_UNINSTALL, NULL, 0), title, MB_YESNO|MB_DEFBUTTON2) == IDYES) {
			DeleteFile(GetSCIniFile(winamp));
		}
		return DSP_PLUGIN_UNINSTALL_NOW;
	}

#ifdef __cplusplus
}
#endif

winampDSPModule *getModule(int which) {
	if (which == 0) return &module;
	return NULL;
}
// Client's proprietary event-context GUID
//extern GUID g_guidMyContext;

// Maximum volume level on trackbar
#define MAX_VOL  100

#define SAFE_RELEASE(what)  \
	if ((what) != NULL)  \
{ (what)->Release(); (what) = NULL; }

//GUID g_guidMyContext = GUID_NULL;

static IAudioStreamVolume *g_pStreamVol = NULL;
static IAudioEndpointVolume *g_pEndptVol = NULL;
static IAudioClient *g_pAudioClient = NULL;
static IAudioCaptureClient *g_pCaptureClient = NULL;

/*#define EXIT_ON_ERROR(hr)  \
	if (FAILED(hr)) { goto Exit; }
#define ERROR_CANCEL(hr)  \
	if (FAILED(hr)) {  \
	MessageBox(hDlg, TEXT("The program will exit."),  \
	TEXT("Fatal error"), MB_OK);  \
	EndDialog(hDlg, TRUE); return TRUE; }*/

HANDLE cf_mutex = NULL;
C_CROSSFADER *Crossfader = NULL;
int CrossfadeLen = 5000;
unsigned int Input_Device_ID=0;
int Restore_PTT = 0;
unsigned int numInputs=0;
char updateStr[16] = {0};

typedef struct	{
	// BladeEnc DLL Version number
	BYTE byDLLMajorVersion;
	BYTE byDLLMinorVersion;
	// BladeEnc Engine Version Number
	BYTE byMajorVersion;
	BYTE byMinorVersion;
	// DLL Release date
	BYTE byDay;
	BYTE byMonth;
	WORD wYear;
	// BladeEnc	Homepage URL
	#define BE_MAX_HOMEPAGE 128
	CHAR zHomepage[BE_MAX_HOMEPAGE + 1];
	BYTE byAlphaLevel;
	BYTE byBetaLevel;
	BYTE byMMXEnabled;
	BYTE btReserved[125];
} BE_VERSION, *PBE_VERSION;
typedef VOID (*BEVERSION)(PBE_VERSION);
BEVERSION beVersion = NULL;
void *init = NULL;
void *params = NULL;
void *encode = NULL;
void *finish = NULL;
void *lameclose = NULL;

int CALLBACK DialogFunc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
HINSTANCE instance = NULL;
HINSTANCE libinst = NULL;
HWND hMainDLG = NULL;
RECT mainrect;
HWND inWin = NULL, inWinWa = NULL;

struct T_TABWND {
	HWND hWnd;
	int id;
	int timer_freq;
	TCITEMW tcitem;
} wnd[MAX_TABWNDS] = {0},
  out_wnd[MAX_OUTWNDS] = {0};
int num_tabwnds = 0,
	num_outwnds = 0;

struct T_COL {
	LVCOLUMNW lvcol;
	LVITEMW lvitem[MAX_CELLS];
	int num_cells;
} col[MAX_COLS] = {0};
int num_cols = 0;

struct T_INPUTWND{
	HWND hWnd;
	int id;
	int timer_freq;
} in_wnd[MAX_INWNDS] = {0};
int num_inwnds = 0;

// shoutcast source
struct T_INPUT_CONFIG {
	int srate;
	int nch;
};
struct MY_T_OUTPUT {
	T_OUTPUT_CONFIG Config;
	int Encoder;	// encoder this config is used by
	int Handle;		// handle that the encoder understands
	int AutoTitle;
	int AutoConnect;
	int Logging;
	int LogCOS;
	int NextTitles;
	int nextTrackLog;
	int nextTrackLogXML;
	wchar_t nextTrackPath[MAX_PATH];
	int useArt;
	int usePlayingArt;
	int useStreamArt;
	wchar_t stationArtPath[MAX_PATH];
	int saveEncoded;
	wchar_t saveEncodedPath[MAX_PATH];
} Output[NUM_OUTPUTS] = {0};
SHOUTCAST_OUTPUT Encoder[NUM_ENCODERS];
HANDLE Enc_mutex[NUM_ENCODERS] = {0};
int Enc_LastType[NUM_ENCODERS] = {0};
C_WAVEIN<NUM_BUFFERS, 5120 > Soundcard;

int last_buffer = 0;
int Connection_CurSelPos = 0;
int Encoder_CurSelPos = 0;
int Input_CurSelPos = 3;
int InputDevice = DEFAULT_INPUTDEVICE;
clock_t audiolag = 0;
clock_t lastaudio = 0;
int curtab = 1;
int curouttab = 0;
int lookAhead = 3;
bool skipMetada = false;
bool doNextLookAhead = false;
HANDLE hthread = NULL;
DWORD threadid = 0;
HANDLE hthreadout = NULL;
DWORD threadoutid = 0;
HWND hWinamp = NULL;
int ini_modified = 0;
HANDLE logFiles[NUM_OUTPUTS] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};

struct T_VU {
	int vu_l;
	int vu_r;
	int update;
	int lastUpdate;
} VU;
int peak_vu_l = -90;
int peak_vu_r = -90;

T_INPUT_CONFIG InputConfig;
int MusVol = 9;
int Mus2Vol = 3;
int MicVol = 9;
int FadeTime = 20;
int MicFadeTime = 10; // mimic old behaviour with a faster fade up/down of the capture device
int devopts = 0;
clock_t FadeStartTime;
clock_t MicFadeStartTime;
int FadeOut = 0;
WNDPROC prevButtonProc = NULL,
		prevListViewProc = NULL,
		prevHeaderProc = NULL,
		prevTabWndProc = NULL,
		prevOutTabWndProc = NULL;
int blockmousemove = 0;
T_INPUT_CONFIG LineInputAttribs[]= {
	{22050, 1},
	{44100, 1},
	{22050, 2},
	{44100, 2},
};

void AddSystrayIcon(HWND hWnd, UINT uIconId, HICON hIcon, UINT uMsg, LPWSTR lpszToolTip) {
	NOTIFYICONDATAW tnid = {0};
	tnid.cbSize = sizeof (NOTIFYICONDATAW);
	tnid.hWnd = hWnd;
	tnid.uID = SYSTRAY_BASE_ICON + uIconId;
	tnid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	tnid.uCallbackMessage = SYSTRAY_BASE_MSG + uMsg;
	tnid.hIcon = hIcon;
	wcsncpy(tnid.szTip, lpszToolTip, ARRAYSIZE(tnid.szTip));
	Shell_NotifyIconW(NIM_ADD, &tnid);
	return;
}

void RemoveSystrayIcon(HWND hWnd, UINT uIconId) {
	NOTIFYICONDATAW tnid = {0};
	tnid.cbSize = sizeof (NOTIFYICONDATAW);
	tnid.hWnd = hWnd;
	tnid.uID = SYSTRAY_BASE_ICON + uIconId;
	Shell_NotifyIconW(NIM_DELETE, &tnid);
	return;
}

void AddTab(int dialog_id, wchar_t *tab_name, HWND hWndParent, DLGPROC DlgProc, int tab_id, int rect_id, int timer_freq) {
	RECT r = {0};
	T_TABWND *twnd = &wnd[num_tabwnds];
	GetWindowRect(GetDlgItem(hWndParent, rect_id), &r);
	ScreenToClient(hWndParent, (POINT *) & r);
	twnd->id = dialog_id;
	twnd->timer_freq = timer_freq;
	twnd->hWnd = LocalisedCreateDialog(instance, dialog_id, hWndParent, DlgProc, dialog_id);
	ShowWindow(twnd->hWnd, SW_HIDE);
	SetWindowPos(twnd->hWnd, NULL, r.left, r.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	twnd->tcitem.mask = TCIF_TEXT;
	twnd->tcitem.pszText = tab_name;
	twnd->tcitem.cchTextMax = wcslen(tab_name);
	SendDlgItemMessage(hWndParent, tab_id, TCM_INSERTITEMW, num_tabwnds++, (LPARAM)&twnd->tcitem);
	if (IsWindow(twnd->hWnd) && isthemethere) {
		SendMessage(module.hwndParent, WM_WA_IPC, (WPARAM)twnd->hWnd, IPC_USE_UXTHEME_FUNC);
	}
}

void SetTab(int tabnum, HWND hWndParent, int tab_id) {
	NMHDR nmh;
	nmh.code = TCN_SELCHANGE;
	nmh.hwndFrom = GetDlgItem(hWndParent, tab_id);
	nmh.idFrom = tab_id;
	curtab = tabnum;
	SendMessage(nmh.hwndFrom, TCM_SETCURSEL, curtab, 0);
	SendMessage(hWndParent, WM_NOTIFY, tab_id, (LPARAM) & nmh);
}

void AddInTab(int dialog_id, wchar_t *tab_name, HWND hWndParent) {
	RECT r = {0};
	T_INPUTWND *twnd = &in_wnd[num_inwnds++];
	GetWindowRect(GetDlgItem(hWndParent, IDC_PANEL_RECT), &r);
	ScreenToClient(hWndParent, (POINT *)&r);
	twnd->id = dialog_id;
	twnd->timer_freq = 0;
	twnd->hWnd = LocalisedCreateDialog(instance, dialog_id, hWndParent, DialogFunc, dialog_id);
	ShowWindow(twnd->hWnd, SW_HIDE);
	SetWindowPos(twnd->hWnd, NULL, r.left, r.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	SendDlgItemMessageW(hWndParent, IDC_INPUTDEVICE, CB_ADDSTRING, 0, (LPARAM)tab_name);
	if (IsWindow(twnd->hWnd) && isthemethere) {
		SendMessage(module.hwndParent, WM_WA_IPC, (WPARAM)twnd->hWnd, IPC_USE_UXTHEME_FUNC);
	}
}

void SetInTab(int tabnum, HWND hWndParent, int combo_id) {
	SendDlgItemMessage(hWndParent, combo_id, CB_SETCURSEL, tabnum, 0);
	SendMessage(hWndParent, WM_COMMAND, MAKEWPARAM(combo_id, CBN_SELCHANGE), (LPARAM) GetDlgItem(hWndParent, combo_id));
}

void AddOutTab(int dialog_id, wchar_t *tab_name, HWND hWndParent, DLGPROC DlgProc, int tab_id, int rect_id, int timer_freq) {
	RECT r = {0};
	T_TABWND *twnd = &out_wnd[num_outwnds];
	GetWindowRect(GetDlgItem(hWndParent, IDC_PANELRECT_C), &r);
	ScreenToClient(hWndParent, (POINT *)&r);
	twnd->id = dialog_id;
	twnd->timer_freq = 0;
	twnd->hWnd = LocalisedCreateDialog(instance, dialog_id, hWndParent, DlgProc, dialog_id);
	ShowWindow(twnd->hWnd, SW_HIDE);
	SetWindowPos(twnd->hWnd, NULL, r.left, r.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	twnd->tcitem.mask = TCIF_TEXT;
	twnd->tcitem.pszText = tab_name;
	twnd->tcitem.cchTextMax = wcslen(tab_name);
	SendDlgItemMessageW(hWndParent, IDC_CONTAB, TCM_INSERTITEMW, num_outwnds++, (LPARAM) & twnd->tcitem);
	if (IsWindow(twnd->hWnd) && isthemethere) {
		SendMessage(module.hwndParent, WM_WA_IPC, (WPARAM)twnd->hWnd, IPC_USE_UXTHEME_FUNC);
	}
}

void SetOutTab(int tabnum, HWND hWndParent, int tab_id) {
	NMHDR nmh;
	nmh.code = TCN_SELCHANGE;
	nmh.hwndFrom = GetDlgItem(hWndParent, tab_id);
	nmh.idFrom = tab_id;
	curouttab = tabnum;
	SendMessage(nmh.hwndFrom, TCM_SETCURSEL, curouttab, 0);
	SendMessage(hWndParent, WM_NOTIFY, tab_id, (LPARAM) & nmh);
}

void AddColumn(wchar_t *column_text, HWND listView) {
	T_COL *tcol = &col[num_cols];
	tcol->lvcol.mask = LVCF_TEXT;
	tcol->lvcol.pszText = column_text;
	tcol->lvcol.cchTextMax = wcslen(column_text);
	SendMessageW(listView, LVM_INSERTCOLUMNW, num_cols++, (LPARAM)&tcol->lvcol);
}

void AddColItem(wchar_t *cell_text, int colnum, HWND hWndParent, int list_id, int pos = -1) {
	LVITEMW *tcell = &col[colnum].lvitem[col[colnum].num_cells];
	tcell->mask = TVIF_TEXT;
	tcell->iItem = pos == -1 ? col[colnum].num_cells++ : pos;
	tcell->iSubItem = colnum;
	tcell->pszText = cell_text;
	tcell->cchTextMax = wcslen(cell_text);
	SendDlgItemMessageW(hWndParent, list_id, pos == -1 && colnum == 0 ? LVM_INSERTITEMW : LVM_SETITEMW, 0, (LPARAM)tcell);
}

void __inline interleave_buffer(const short * inLeft, short *outputbuf, const size_t num_samples) {
    for (size_t i = 0; i < num_samples; ++i) {
        outputbuf[i * 2] = inLeft[i];
        outputbuf[i * 2 + 1] = inLeft[i];
    }
}

DWORD WINAPI ThreadInput(LPVOID lpParameter) {
	do {
		// this is needed when doing soundcard capture
		if (InputDevice == 1) DialogFunc((HWND) lpParameter, WM_TIMER, MAKEWPARAM(1234,0), 0);
		short mybuf[32768] = {0};
		size_t mysamps = 0;

		if (WaitForSingleObject(cf_mutex, INFINITE) == WAIT_OBJECT_0) {
			if (Input_CurSelPos != -1) {
				if (InputDevice == 0) {
					if (isplaying != 1) {
						// when stopped or paused, we need to pump silent output
						// and from testing, sending 2 emtpy samples appears to
						// keep the output bitrate about the same as playback's
						mysamps = sizeof(mybuf)/8;
					} else {
						mysamps = Crossfader->get(mybuf, sizeof (mybuf) / (InputConfig.nch * sizeof (short)), InputConfig.nch) * InputConfig.nch;
					}
				} else {
					int samps = (LineInputAttribs[Input_CurSelPos].nch * sizeof (short));
					if(LineInputAttribs[Input_CurSelPos].nch == 1) {
						mysamps = Crossfader->get(mybuf, (samps ? sizeof (mybuf) / samps : sizeof (mybuf)) / 2, LineInputAttribs[Input_CurSelPos].nch) * LineInputAttribs[Input_CurSelPos].nch;
						short *newbuf = (short*) malloc(mysamps * 2 * sizeof(short));
						interleave_buffer(mybuf, newbuf, mysamps);
						mysamps *= 2;
						memcpy(mybuf, newbuf, mysamps * sizeof (short));
						free(newbuf);
					} else {
						mysamps = Crossfader->get(mybuf, (samps ? sizeof (mybuf) / samps : sizeof (mybuf)), LineInputAttribs[Input_CurSelPos].nch) * LineInputAttribs[Input_CurSelPos].nch;
					}
				}
			}
			ReleaseMutex(cf_mutex);
		}

		if (mysamps > 0) {
			short *tmp = mybuf;
			for (size_t k = 0; k != NUM_ENCODERS; k++) {
				if (WaitForSingleObject(Enc_mutex[k], INFINITE) == WAIT_OBJECT_0) {
					size_t size = mysamps * sizeof (short);
					short * newbuf = (short*) malloc(size);
					if (newbuf) {
						memcpy(newbuf, tmp, size);
						Encoder[k].Run(OM_ENCODE, newbuf, size, k); // this seems to be modifying newbuf
						free(newbuf);
					}
					ReleaseMutex(Enc_mutex[k]);
				}
			}
		}
		Sleep(25);
	} while (hthread != NULL);
	return 0;
}

DWORD WINAPI ThreadOutput(LPVOID lpParameter) {
	do {
		for (int k = 0; k < NUM_ENCODERS; k++) {
			if (WaitForSingleObject(Enc_mutex[k], 25) == WAIT_OBJECT_0) {
				if (Encoder[k].GetEncoder()) {
					Encoder[k].Run(OM_OUTPUT | OM_OTHER);
				}
				ReleaseMutex(Enc_mutex[k]);
			}
		}
		Sleep(25);
	} while (hthreadout != NULL);
	return 0;
}

char olddev[256] = {0};
int DisplayDeviceName(void) {
	if (InputDevice) {
		char deviceBuf[256] = {0};
		char *deviceName = Soundcard.getDeviceName(0);

		if (deviceName && *deviceName) {
			char tmp[128] = {0};
			StringCchPrintfA(deviceBuf, ARRAYSIZE(deviceBuf), WASABI_API_LNGSTRING_BUF(IDS_DEVICE_STRING, tmp, 128), deviceName);
		} else {
			WASABI_API_LNGSTRING_BUF(IDS_NO_DEVICES_FOUND, deviceBuf, ARRAYSIZE(deviceBuf));
			olddev[0] = 0;
		}
		SendDlgItemMessage(wnd[2].hWnd, IDC_CURDEVICE, WM_SETTEXT, 0,(LPARAM)deviceBuf);

		// vista - 7 check for default device and restart soundcard if needed
		if (IsVistaUp()) {
			if (strcmp(deviceBuf, olddev) != 0) {
				lstrcpyn(olddev, deviceBuf, ARRAYSIZE(olddev));
				SuspendThread(hthread);
				Soundcard.Close();
				Soundcard.Create((InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate), (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch));
				ResumeThread(hthread);
				ini_modified = 1;
			}
		}
	} else {
		SendDlgItemMessage(wnd[2].hWnd, IDC_CURDEVICE, WM_SETTEXT, 0,(LPARAM)"");
	}

	return 1;
}

#ifdef CAPTURE_TESTING
//-----------------------------------------------------------
// The input argument to this function is a pointer to the
// IMMDevice interface for a capture endpoint device. The
// function traverses the data path that extends from the
// endpoint device to the system bus (for example, PCI)
// or external bus (USB). If the function discovers a MUX
// (input selector) in the path, it selects the MUX input
// that connects to the stream from the endpoint device.
//-----------------------------------------------------------
#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const IID IID_IDeviceTopology = __uuidof(IDeviceTopology);
const IID IID_IPart = __uuidof(IPart);
const IID IID_IConnector = __uuidof(IConnector);
const IID IID_IAudioInputSelector = __uuidof(IAudioInputSelector);

HRESULT SelectCaptureDevice(IMMDevice *pEndptDev)
{
    HRESULT hr = S_OK;
    DataFlow flow;
    IDeviceTopology *pDeviceTopology = NULL;
    IConnector *pConnFrom = NULL;
    IConnector *pConnTo = NULL;
    IPart *pPartPrev = NULL;
    IPart *pPartNext = NULL;
    IAudioInputSelector *pSelector = NULL;

    if (pEndptDev == NULL)
    {
        EXIT_ON_ERROR(hr = E_POINTER)
    }

    // Get the endpoint device's IDeviceTopology interface.
    hr = pEndptDev->Activate(
                      IID_IDeviceTopology, CLSCTX_ALL, NULL,
                      (void**)&pDeviceTopology);
    EXIT_ON_ERROR(hr)

    // The device topology for an endpoint device always
    // contains just one connector (connector number 0).
    hr = pDeviceTopology->GetConnector(0, &pConnFrom);
    SAFE_RELEASE(pDeviceTopology)
    EXIT_ON_ERROR(hr)

    // Make sure that this is a capture device.
    hr = pConnFrom->GetDataFlow(&flow);
    EXIT_ON_ERROR(hr)

    if (flow != Out)
    {
        // Error -- this is a rendering device.
        EXIT_ON_ERROR(hr = AUDCLNT_E_WRONG_ENDPOINT_TYPE)
    }

    // Outer loop: Each iteration traverses the data path
    // through a device topology starting at the input
    // connector and ending at the output connector.
    while (TRUE)
    {
        BOOL bConnected;
        hr = pConnFrom->IsConnected(&bConnected);
        EXIT_ON_ERROR(hr)

        // Does this connector connect to another device?
        if (bConnected == FALSE)
        {
            // This is the end of the data path that
            // stretches from the endpoint device to the
            // system bus or external bus. Verify that
            // the connection type is Software_IO.
            ConnectorType  connType;
            hr = pConnFrom->GetType(&connType);
            EXIT_ON_ERROR(hr)

            if (connType == Software_IO)
            {
                break;  // finished
            }
            EXIT_ON_ERROR(hr = E_FAIL)
        }

        // Get the connector in the next device topology,
        // which lies on the other side of the connection.
        hr = pConnFrom->GetConnectedTo(&pConnTo);
        EXIT_ON_ERROR(hr)
        SAFE_RELEASE(pConnFrom)

        // Get the connector's IPart interface.
        hr = pConnTo->QueryInterface(
                        IID_IPart, (void**)&pPartPrev);
        EXIT_ON_ERROR(hr)
        SAFE_RELEASE(pConnTo)

        // Inner loop: Each iteration traverses one link in a
        // device topology and looks for input multiplexers.
        while (TRUE)
        {
            PartType parttype;
            UINT localId;
            IPartsList *pParts;

            // Follow downstream link to next part.
            hr = pPartPrev->EnumPartsOutgoing(&pParts);
            EXIT_ON_ERROR(hr)

            hr = pParts->GetPart(0, &pPartNext);
            pParts->Release();
            EXIT_ON_ERROR(hr)

            hr = pPartNext->GetPartType(&parttype);
            EXIT_ON_ERROR(hr)

            if (parttype == Connector)
            {
                // We've reached the output connector that
                // lies at the end of this device topology.
                hr = pPartNext->QueryInterface(
                                  IID_IConnector,
                                  (void**)&pConnFrom);
                EXIT_ON_ERROR(hr)

                SAFE_RELEASE(pPartPrev)
                SAFE_RELEASE(pPartNext)
                break;
            }

            // Failure of the following call means only that
            // the part is not a MUX (input selector).
            hr = pPartNext->Activate(
                              CLSCTX_ALL,
                              IID_IAudioInputSelector,
                              (void**)&pSelector);
            if (hr == S_OK)
            {
                // We found a MUX (input selector), so select
                // the input from our endpoint device.
                hr = pPartPrev->GetLocalId(&localId);
                EXIT_ON_ERROR(hr)

                hr = pSelector->SetSelection(localId, NULL);
                EXIT_ON_ERROR(hr)

                SAFE_RELEASE(pSelector)
            }

            SAFE_RELEASE(pPartPrev)
            pPartPrev = pPartNext;
            pPartNext = NULL;
        }
    }

Exit:
    SAFE_RELEASE(pConnFrom)
    SAFE_RELEASE(pConnTo)
    SAFE_RELEASE(pPartPrev)
    SAFE_RELEASE(pPartNext)
    SAFE_RELEASE(pSelector)
    return hr;
}
#endif

void setlev(int cs, int va) {
	if (IsVistaUp() &&
	   (cs == MIXERLINE_COMPONENTTYPE_SRC_LINE || cs ==  MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)) {
		//HRESULT hr = S_OK;
		IMMDeviceEnumerator *pEnumerator = NULL;
		IMMDevice *pDevice = NULL;
		IMMDeviceCollection *ppDevices = NULL;
		//hr = CoCreateGuid(&g_guidMyContext);
		//EXIT_ON_ERROR(hr)

		// Get enumerator for audio endpoint devices.
		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
							  NULL, CLSCTX_INPROC_SERVER,
							  __uuidof(IMMDeviceEnumerator),
							  (void**)&pEnumerator);
		EXIT_ON_ERROR(hr)

		hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE | DEVICE_STATE_UNPLUGGED, &ppDevices);
		EXIT_ON_ERROR(hr)

		hr = ppDevices->Item(Input_Device_ID, &pDevice);
		EXIT_ON_ERROR(hr)

		//activate
		hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&g_pEndptVol);
		EXIT_ON_ERROR(hr)

		//set mic volume
		float fVolume = (float)(va / 100.0f);
		if (va > 2) {
			hr = g_pEndptVol->SetMasterVolumeLevelScalar(fVolume, NULL/*&g_guidMyContext*/);
			EXIT_ON_ERROR(hr)
		} else {//mute
			hr = g_pEndptVol->SetMasterVolumeLevelScalar((float)0.0f, NULL/*&g_guidMyContext*/);
		}

		/*hr = pDevice->Activate(__uuidof(IAudioStreamVolume ), CLSCTX_ALL, NULL, (void**)&g_pStreamVol);
		EXIT_ON_ERROR(hr)

		g_pStreamVol->SetChannelVolume(0, fVolume);
		g_pStreamVol->SetChannelVolume(1, fVolume);

		IMMDevice *pDefaultDevice = NULL;
		hr = pEnumerator->GetDefaultAudioEndpoint(eCapture,eConsole,&pDefaultDevice);
		EXIT_ON_ERROR(hr)

		hr = pDefaultDevice->Activate(__uuidof(IAudioStreamVolume ), CLSCTX_ALL, NULL, (void**)&g_pStreamVol);
		EXIT_ON_ERROR(hr)

		g_pStreamVol->SetChannelVolume(0, fVolume);
		g_pStreamVol->SetChannelVolume(1, fVolume);*/

Exit:
		SAFE_RELEASE(pEnumerator)
		SAFE_RELEASE(pDevice)
		SAFE_RELEASE(ppDevices)
		SAFE_RELEASE(g_pEndptVol)
		CoUninitialize();
	} // end if mic

	for (UINT i = 0; i < (IsVistaUp() ? mixerGetNumDevs() : 1); i++) {
		HMIXER hmix;
		#ifdef FOLLOW_MIXER
		// TODO use a different handle??
		mixerOpen(&hmix, i, (DWORD_PTR)hMainDLG, 0, MIXER_OBJECTF_MIXER|CALLBACK_WINDOW);
		#endif
		if (mixerOpen(&hmix, i, (DWORD_PTR)hMainDLG, 0, MIXER_OBJECTF_MIXER) == MMSYSERR_NOERROR) {
			MIXERLINE ml = {sizeof (ml), 0};
			ml.dwComponentType = cs;
			if (mixerGetLineInfo((HMIXEROBJ) hmix, &ml, MIXER_GETLINEINFOF_COMPONENTTYPE|MIXER_OBJECTF_MIXER) == MMSYSERR_NOERROR) {
				MIXERLINECONTROLS mlc = {sizeof (mlc), ml.dwLineID,};
				MIXERCONTROL mc = {sizeof (mc),};
				mlc.cControls = 1;
				mlc.cbmxctrl = sizeof (mc);
				mlc.pamxctrl = &mc;
				mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
				if (mixerGetLineControls((HMIXEROBJ) hmix, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE|MIXER_OBJECTF_MIXER) == MMSYSERR_NOERROR) {
					MIXERCONTROLDETAILS mcd = {sizeof (mcd), mc.dwControlID, ml.cChannels,};
					MIXERCONTROLDETAILS_UNSIGNED v[2];
					mcd.cbDetails = sizeof (MIXERCONTROLDETAILS_UNSIGNED);
					mcd.paDetails = v;
					v[0].dwValue = mc.Bounds.dwMinimum + (va * (mc.Bounds.dwMaximum - mc.Bounds.dwMinimum)) / 100;
					v[1].dwValue = mc.Bounds.dwMinimum + (va * (mc.Bounds.dwMaximum - mc.Bounds.dwMinimum)) / 100;
					/*MMRESULT result = */mixerSetControlDetails((HMIXEROBJ) hmix, &mcd, MIXER_OBJECTF_MIXER);
				}
			}
			mixerClose(hmix);
		}
	}

	DisplayDeviceName();
}

int SetDeviceName(void) {
	HRESULT hr = S_OK;
	IMMDeviceEnumerator *pEnumerate = NULL;
	IMMDevice *pDevice = NULL;
	IMMDevice *pDefaultDevice = NULL;
	IMMDeviceCollection *ppDevices = NULL;
	IPropertyStore *pProps = NULL;
	PROPVARIANT varName;

	if (IsVistaUp()) {
		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		PropVariantInit(&varName);
		// Get enumerator for audio endpoint devices.
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
							  NULL, CLSCTX_INPROC_SERVER,
							  __uuidof(IMMDeviceEnumerator),
							  (void**)&pEnumerate);
		EXIT_ON_ERROR(hr)

		hr = pEnumerate->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE | DEVICE_STATE_UNPLUGGED, &ppDevices);
		EXIT_ON_ERROR(hr)

		numInputs = 0;
		hr = ppDevices->GetCount(&numInputs);
		EXIT_ON_ERROR(hr)

		// treat this as a dummy stop
		Exit:;
	}

	int oldCount = SendDlgItemMessage(inWin, IDC_DEVBOX, CB_GETCOUNT, 0, 0);
	SendDlgItemMessage(inWin, IDC_DEVBOX, CB_RESETCONTENT, 0,0);
	EnableWindowDlgItem(inWin, IDC_REFRESH_DEVICES, IsVistaUp());

	if (!IsVistaUp()) {//change back to true when vista enabled !
		SendDlgItemMessageW(inWin, IDC_DEVBOX, CB_ADDSTRING, 0,(LPARAM)LocalisedString(IDS_MIC_LEGACY_MODE, NULL, 0));
		SendDlgItemMessageW(inWin, IDC_DEVBOX, CB_ADDSTRING, 0,(LPARAM)LocalisedString(IDS_LINEIN_LEGACY_MODE, NULL, 0)); 
	} else {
		hr = pEnumerate->GetDefaultAudioEndpoint(eCapture, eConsole, &pDefaultDevice);
		if (SUCCEEDED(hr) && pDefaultDevice != NULL) {
			LPWSTR defaultName = NULL;
			pDefaultDevice->GetId(&defaultName);

			//jkey: This is for vista or 7, so we scan through and add friendly device names
			//		though need to make sure that we don't re-add the current input device
			//      otherwise with the waveout fudge we'll get really bad feedback on output
			for (unsigned int i=0; i < numInputs; i++) {
				LPWSTR itemName = NULL;
				ppDevices->Item(i, &pDevice);
				pDevice->GetId(&itemName);
				// check the id of the endpoints to prevent adding in the default output device
				if (defaultName && wcsicmp(itemName, defaultName)) {
					pDevice->OpenPropertyStore(STGM_READ, &pProps);
					pProps->GetValue(PKEY_Device_FriendlyName, &varName);
					SendDlgItemMessageW(inWin, IDC_DEVBOX, CB_ADDSTRING, 0,(LPARAM)varName.pwszVal);
					SendDlgItemMessage(inWin, IDC_DEVBOX, CB_SETITEMDATA, i,(LPARAM)i);
				}
				CoTaskMemFree(itemName);
			}
			CoTaskMemFree(defaultName);
		}

		int count = SendDlgItemMessage(inWin, IDC_DEVBOX, CB_GETCOUNT, 0, 0);
		if (!count) {
			SendDlgItemMessageW(inWin, IDC_DEVBOX, CB_ADDSTRING, 0, (LPARAM)WASABI_API_LNGSTRINGW(IDS_NO_CAPTURE_DEVICES));
		}
		EnableWindowDlgItem(inWin, IDC_DEVBOX, count);
		// reset to the first item in the list if there's any changes
		if (!oldCount || count != oldCount) {
			SendDlgItemMessage(inWin, IDC_DEVBOX, CB_SETCURSEL, 0, 0);
		}

		PropVariantClear(&varName);
		SAFE_RELEASE(pProps)
		SAFE_RELEASE(pEnumerate)
		SAFE_RELEASE(pDevice)
		SAFE_RELEASE(ppDevices)
		CoUninitialize();
	}

	SendDlgItemMessage(inWin, IDC_DEVBOX, CB_SETCURSEL, Input_Device_ID, 0);
	DisplayDeviceName();
	return 1;
}

/*int getlev(int cs) {
	HMIXER hmix;
	int retval = -1;
#ifdef USE_VISTA_SOUND_FIX
	HRESULT hr = S_OK;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDevice *pDevice = NULL;
	IMMDeviceCollection *ppDevices = NULL;
	hr = CoCreateGuid(&g_guidMyContext);
	EXIT_ON_ERROR(hr)

	// Get enumerator for audio endpoint devices.
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
						  NULL, CLSCTX_INPROC_SERVER,
						  __uuidof(IMMDeviceEnumerator),
						  (void**)&pEnumerator);
	EXIT_ON_ERROR(hr)

	hr = pEnumerator->EnumAudioEndpoints(eCapture,DEVICE_STATE_ACTIVE,&ppDevices);
	EXIT_ON_ERROR(hr)

	hr = ppDevices->Item(Input_Device_ID,&pDevice);
	EXIT_ON_ERROR(hr)

	//activate
	hr = pDevice->Activate(__uuidof(IAudioEndpointVolume),
	CLSCTX_ALL, NULL, (void**)&g_pEndptVol);
	EXIT_ON_ERROR(hr)
	//set mic volume
	float fVolume =0.0;
	hr = g_pEndptVol->GetMasterVolumeLevel(&fVolume);
	EXIT_ON_ERROR(hr)

Exit:
	if (FAILED(hr))	{
		useXpSound = true;
	} else {
		retval = (int)fVolume * 100;
		return retval;
	}

	SAFE_RELEASE(pEnumerator)
	SAFE_RELEASE(pDevice)
	SAFE_RELEASE(ppDevices)
	SAFE_RELEASE(g_pEndptVol)
	CoUninitialize();

#endif //USE_VISTA_SOUND_FIX
	if (mixerOpen(&hmix, 0, 0, 0, 0) == MMSYSERR_NOERROR) {
		MIXERLINE ml = {sizeof (ml), 0};
		ml.dwComponentType = cs;
		if (mixerGetLineInfo((HMIXEROBJ) hmix, &ml, MIXER_GETLINEINFOF_COMPONENTTYPE) == MMSYSERR_NOERROR) {
			MIXERLINECONTROLS mlc = {sizeof (mlc), ml.dwLineID,};
			MIXERCONTROL mc = {sizeof (mc),};
			mlc.cControls = 1;
			mlc.cbmxctrl = sizeof (mc);
			mlc.pamxctrl = &mc;
			mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
			if (mixerGetLineControls((HMIXEROBJ) hmix, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE) == MMSYSERR_NOERROR) {
				MIXERCONTROLDETAILS mcd = {sizeof (mcd), mc.dwControlID, ml.cChannels,};
				MIXERCONTROLDETAILS_UNSIGNED v[2];
				mcd.cbDetails = sizeof (MIXERCONTROLDETAILS_UNSIGNED);
				mcd.paDetails = v;
				if (mixerGetControlDetails((HMIXEROBJ) hmix, &mcd, 0) == MMSYSERR_NOERROR) {
					retval = (v[0].dwValue * 100) / (mc.Bounds.dwMaximum - mc.Bounds.dwMinimum);
					// retval = ((v[0].dwValue + v[1].dwValue) * 5) / (mc.Bounds.dwMaximum-mc.Bounds.dwMinimum);
				}
			}
		}
		mixerClose(hmix);
	}
	return retval;
}*/

void TitleCallback(int Connection, int Mode) {
	MY_T_OUTPUT *Out = &Output[Connection];
	// title update
	if (Mode == 0) {
		if (Out->AutoTitle == 1) {
			// look at the playback queue so we can get the correct 'next song'
			if (!WASABI_API_QUEUEMGR) {
				// due to loading orders its possible the queue won't have been loaded on init so check
				ServiceBuild(WASABI_API_SVC, WASABI_API_QUEUEMGR, QueueManagerApiGUID);
			}

			std::vector<std::wstring> nextList;
			nextList.clear();
			Encoder[Out->Encoder].UpdateTitle(0, nextList, Out->Handle, !!Out->NextTitles);
		}
	} else if (Mode == 1) {
		// album art update
		Encoder[Out->Encoder].UpdateAlbumArt(Out->Handle);
	}
}

void CenterWindow(void) {
    RECT rect, rectP;
    int width, height;
    int screenwidth, screenheight;
    int x, y;

	GetWindowRect(hMainDLG, &rect);
	GetWindowRect(GetDesktopWindow(), &rectP);

	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	x = ((rectP.right-rectP.left) - width) / 2 + rectP.left;
	y = ((rectP.bottom-rectP.top) - height) / 2 + rectP.top;

	screenwidth = GetSystemMetrics(SM_CXSCREEN);
	screenheight = GetSystemMetrics(SM_CYSCREEN);

	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x + width > screenwidth) x = screenwidth - width;
	if (y + height > screenheight) y = screenheight - height;

	mainrect.left = x;
	mainrect.top = y;
}

int LoadConfig(void) {
	lookAhead = GetPrivateProfileInt(APP_Name, "lookAhead", lookAhead, IniName);
	skipMetada = !!GetPrivateProfileInt(APP_Name, "skipMetada", skipMetada, IniName);
	lastFilterIndex = GetPrivateProfileInt(APP_Name, "ofnidx", lastFilterIndex, IniName);

	curtab = GetPrivateProfileInt(APP_Name, "CurTab", curtab, IniName);
	Connection_CurSelPos = GetPrivateProfileInt(APP_Name, "Connection_CurSelPos", Connection_CurSelPos, IniName);
	curouttab = GetPrivateProfileInt(APP_Name, "Connection_CurTab", curouttab, IniName);
	Encoder_CurSelPos = GetPrivateProfileInt(APP_Name, "Encoder_CurSelPos", Encoder_CurSelPos, IniName);
	InputDevice = GetPrivateProfileInt(APP_Name, "InputDevice", InputDevice, IniName);

	Input_CurSelPos = GetPrivateProfileInt(APP_Name, "Input_CurSelPos", Input_CurSelPos, IniName);
	InputConfig.srate = LineInputAttribs[3].srate;
	InputConfig.nch = LineInputAttribs[3].nch;
	cf_mutex = CreateMutex(NULL, TRUE, NULL);
	Crossfader = new C_CROSSFADER(CrossfadeLen,
								  LineInputAttribs[(InputDevice == 0 ? 3 : Input_CurSelPos)].nch,
								  LineInputAttribs[(InputDevice == 0 ? 3 : Input_CurSelPos)].srate);

	MusVol = GetPrivateProfileInt(APP_Name, "MusicVolume", MusVol, IniName);
	Mus2Vol = GetPrivateProfileInt(APP_Name, "BGMusicVolume", Mus2Vol, IniName);
	MicVol = GetPrivateProfileInt(APP_Name, "MicVolume", MicVol, IniName);

	// as we've changed the scaling then we will need to adjust from old to new
	int tempFadeTime = GetPrivateProfileInt(APP_Name, "PTT_FadeTime", -1, IniName);
	if (tempFadeTime == -1) {
		FadeTime = GetPrivateProfileInt(APP_Name, "PTT_FT", FadeTime, IniName);
	} else {
		FadeTime = tempFadeTime * 5;
		// remove the old instance of the settings
		WritePrivateProfileString(APP_Name, "PTT_FadeTime", 0, IniName);
	}

	int tempMicFadeTime = GetPrivateProfileInt(APP_Name, "PTT_MicFadeTime", -1, IniName);
	if (tempMicFadeTime == -1) {
		MicFadeTime = GetPrivateProfileInt(APP_Name, "PTT_MicFT", MicFadeTime, IniName);
	} else {
		MicFadeTime = tempMicFadeTime * 5;
		// remove the old instance of the settings
		WritePrivateProfileString(APP_Name, "PTT_MicFadeTime", 0, IniName);
	}

	Restore_PTT = GetPrivateProfileInt(APP_Name, "PTT_Restore", 0, IniName);
	Input_Device_ID = GetPrivateProfileInt(APP_Name, "PTT_MicInput",0, IniName);

	// align to middle of screen on new installs
	CenterWindow();
	mainrect.left = GetPrivateProfileInt(APP_Name, "WindowLeft", mainrect.left, IniName);
	mainrect.top = GetPrivateProfileInt(APP_Name, "WindowTop", mainrect.top, IniName);

	GetPrivateProfileString(APP_Name, "Update", 0, updateStr, 16, IniName);
	// no point in indicating a new version if we're now showing as ahead
	if (!CompareVersions(updateStr)) {
		WritePrivateProfileString(APP_Name, "Update", 0, IniName);
		updateStr[0] = 0;
	}

	for (int i = 0; i < NUM_OUTPUTS; i++) {
		T_OUTPUT_CONFIG *Out = &Output[i].Config;
		StringCchPrintfA(Out->Name, 32, "Output %u", i + 1);
		StringCchPrintfW(Out->DisplayName, 32, WASABI_API_LNGSTRINGW(IDS_OUTPUT_X), i + 1);
		Output[i].Encoder = -1;
		Output[i].Handle = -1;
		GetPrivateProfileString(Out->Name, "Address", "localhost", Out->Address, ARRAYSIZE(Out->Address), IniName);
		GetPrivateProfileString(Out->Name, "UserID", "", Out->UserID, ARRAYSIZE(Out->UserID), IniName);
		GetPrivateProfileString(Out->Name, "StreamID", "1", Out->StationID, ARRAYSIZE(Out->StationID), IniName);
		Out->Port = GetPrivateProfileInt(Out->Name, "Port", 8000, IniName);
		GetPrivateProfileString(Out->Name, "Password", "", Out->Password, ARRAYSIZE(Out->Password), IniName);
		GetPrivateProfileString(Out->Name, "Cipherkey", "foobar", Out->cipherkey, ARRAYSIZE(Out->cipherkey), IniName);
		GetPrivateProfileString(Out->Name, "Description", "Unnamed Server", Out->Description, ARRAYSIZE(Out->Description), IniName);
		GetPrivateProfileString(Out->Name, "URL", "http://www.shoutcast.com", Out->ServerURL, ARRAYSIZE(Out->ServerURL), IniName);
		GetPrivateProfileString(Out->Name, "Genre3", "Misc", Out->Genre, ARRAYSIZE(Out->Genre), IniName);
		// check that the genre is a support value otherwise reset it to 'misc'
		bool foundGenre = false;
		for (int g = 0; g < ARRAYSIZE(genres); g++) {
			if (!strcmpi(genres[g].name, Out->Genre)) {
				foundGenre = true;
				break;
			}
		}
		if (foundGenre == false) {
			lstrcpyn(Out->Genre, "Misc", ARRAYSIZE(Out->Genre));
		}

		GetPrivateProfileString(Out->Name, "AIM", "N/A", Out->AIM, ARRAYSIZE(Out->AIM), IniName);
		GetPrivateProfileString(Out->Name, "ICQ", "0", Out->ICQ, ARRAYSIZE(Out->ICQ), IniName);
		GetPrivateProfileString(Out->Name, "IRC", "N/A", Out->IRC, ARRAYSIZE(Out->IRC), IniName);
		Out->Public = GetPrivateProfileInt(Out->Name, "Public", 1, IniName);
		Out->AutoRecon = GetPrivateProfileInt(Out->Name, "AutoRecon", 1, IniName);
		Out->ReconTime = GetPrivateProfileInt(Out->Name, "ReconTime", 5, IniName);
		if (Out->ReconTime < 1) {
			Out->ReconTime = 5;
		}
		Out->doTitleUpdate = GetPrivateProfileInt(Out->Name, "doTitleUpdate", 1, IniName);
		GetPrivateProfileString(Out->Name, "now", "", Out->Now, ARRAYSIZE(Out->Now), IniName);
		GetPrivateProfileString(Out->Name, "next", "", Out->Next, ARRAYSIZE(Out->Next), IniName);
		Output[i].AutoTitle = GetPrivateProfileInt(Out->Name, "AutoTitle", 1, IniName);
		Output[i].AutoConnect = GetPrivateProfileInt(Out->Name, "AutoConnect", 0, IniName);
		Output[i].Logging = GetPrivateProfileInt(Out->Name, "Logging", 0, IniName);
		Output[i].LogCOS = GetPrivateProfileInt(Out->Name, "LogCOS", 0, IniName);
		Output[i].NextTitles = GetPrivateProfileInt(Out->Name, "NextTitles", 1, IniName);
		Output[i].Config.protocol = GetPrivateProfileInt(Out->Name, "protocol", -1, IniName);
		if (Output[i].Config.protocol == -1) {
			Output[i].Config.protocol = MAKEWORD(2, 1);
		}
		// check the v1 password for : and split it if the dj/user id is empty (i.e. post 2.2.3 import)
		if (LOBYTE(Output[i].Config.protocol) == 1 && Out->Password[0] && !Out->UserID[0]) {
			char* password = strstr(Out->Password, ":");
			if (password) {
				*password = 0;
				lstrcpyn(Out->UserID, Out->Password, ARRAYSIZE(Out->UserID));
				lstrcpyn(Out->Password, ++password, ARRAYSIZE(Out->Password));
			}
		}
		Output[i].nextTrackLog = GetPrivateProfileInt(Out->Name, "nextTrackLog", 0, IniName);
		Output[i].nextTrackLogXML = GetPrivateProfileInt(Out->Name, "nextTrackLogXML", 0, IniName);
		if (!GetPrivateProfileStringUTF8(Out->Name, "nextTrackPath", 0, Output[i].nextTrackPath, ARRAYSIZE(Output[i].nextTrackPath), IniName)) {
			GetDefaultNextTracksLogFile(module.hwndParent, ARRAYSIZE(Output[i].nextTrackPath), Output[i].nextTrackPath, i);
		}

		Output[i].useArt = GetPrivateProfileInt(Out->Name, "useArt", 0, IniName);
		Output[i].usePlayingArt = GetPrivateProfileInt(Out->Name, "usePlayingArt", 0, IniName);
		Output[i].useStreamArt = GetPrivateProfileInt(Out->Name, "useStreamArt", 1, IniName);
		GetPrivateProfileStringUTF8(Out->Name, "stationArtPath", 0, Output[i].stationArtPath, ARRAYSIZE(Output[i].stationArtPath), IniName);

		Output[i].saveEncoded = GetPrivateProfileInt(Out->Name, "saveEncoded", 0, IniName);
		GetPrivateProfileStringUTF8(Out->Name, "saveEncodedPath", 0, Output[i].saveEncodedPath, ARRAYSIZE(Output[i].saveEncodedPath), IniName);

		Output[i].Encoder = GetPrivateProfileInt(Out->Name, "Encoder", i == 0 ? 0 : -1, IniName);
		if (Output[i].Encoder != -1) {
			if (WaitForSingleObject(Enc_mutex[Output[i].Encoder], INFINITE) == WAIT_OBJECT_0) {
				Output[i].Handle = Encoder[Output[i].Encoder].AddOutput(i, Out, TitleCallback);
				ReleaseMutex(Enc_mutex[Output[i].Encoder]);
			}
		}
	}
	return 1;
}

wchar_t* BuildLameVersion(void) {
	static wchar_t version[128] = {0};
	if (libinst != NULL && !version[0]) {
		BE_VERSION ver;
		beVersion(&ver);

		if (ver.byBetaLevel) {
			StringCchPrintfW(version, ARRAYSIZE(version), L"%u.%ub%u", (unsigned int)ver.byMajorVersion, (unsigned int)ver.byMinorVersion, (unsigned int)ver.byBetaLevel);
		} else if (ver.byAlphaLevel) {
			StringCchPrintfW(version, ARRAYSIZE(version), L"%u.%ua%u", (unsigned int)ver.byMajorVersion, (unsigned int)ver.byMinorVersion, (unsigned int)ver.byAlphaLevel);
		} else {
			StringCchPrintfW(version, ARRAYSIZE(version), L"%u.%u", (unsigned int)ver.byMajorVersion, (unsigned int)ver.byMinorVersion);
		}
	}
	return version;
}

void LoadEncoders() {
	/* load lame_enc.dll */
	wchar_t dllname[MAX_PATH] = {0};
	StringCchPrintfW(dllname, ARRAYSIZE(dllname), L"%s\\lame_enc.dll", GetSharedDirectoryW(module.hwndParent));

	libinst = LoadLibraryW(dllname);
	if (libinst == NULL) {
		wchar_t title[128] = {0}, message[512] = {0};
		StringCchPrintfW(title, ARRAYSIZE(title), LocalisedString(IDS_PLUGIN_NAME, NULL, 0), APP_VersionW);
		StringCchPrintfW(message, ARRAYSIZE(message), LocalisedString(IDS_FAILED_LOAD_LAMEDLL, NULL, 0), GetSharedDirectoryW(module.hwndParent));
		MessageBoxW(module.hwndParent, message, title, MB_ICONWARNING);
	} else {
		beVersion = (BEVERSION) GetProcAddress(libinst, "beVersion");
		init = (void *) GetProcAddress(libinst, "lame_init");
		params = (void *) GetProcAddress(libinst, "lame_init_params");
		encode = (void *) GetProcAddress(libinst, "lame_encode_buffer_interleaved");
		finish = (void *) GetProcAddress(libinst, "lame_encode_flush");

		if (!init || !params || !encode || !finish) {
			wchar_t title[128] = {0};
			StringCchPrintfW(title, ARRAYSIZE(title), LocalisedString(IDS_PLUGIN_NAME, NULL, 0), APP_VersionW);
			MessageBoxW(module.hwndParent, LocalisedString(IDS_LAMEDLL_ISSUE, NULL, 0), title, MB_ICONWARNING);
			FreeLibrary(libinst);
			libinst = 0;
		}
	}

	/* load encoder type */
	for (int i = 0; i < NUM_ENCODERS; i++) {
		char name[32] = {0};
		Enc_mutex[i] = CreateMutex(NULL, TRUE, NULL);
		StringCchPrintfA(name, ARRAYSIZE(name), "Encoder %u", i + 1);
		int EncType = GetPrivateProfileInt(name, "Type", 2, IniName);
		// store our type for later on
		Enc_LastType[i] = EncType;
		switch (EncType) {
			/* mp3 */
			case 1:
				{
				fallback:
					Encoder[i].SetLame(init, params, encode, finish);
					Encoder[i].SetEncoder(DEFAULT_ENCODER);
					C_ENCODER *Enc = Encoder[i].GetEncoder();
					if (Enc) {
						if (strcmp(Enc->GetName(), "MP3 Encoder") == 0) {
							T_ENCODER_MP3_INFO EncSettings;
							int infosize = sizeof (EncSettings);
							T_ENCODER_MP3_INFO *encset = (T_ENCODER_MP3_INFO *) Enc->GetExtInfo(&infosize);
							if (encset && infosize) memcpy(&EncSettings, encset, infosize);
							EncSettings.input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
							EncSettings.input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
							EncSettings.output_bitRate = GetPrivateProfileInt(name, "BitRate", EncSettings.output_bitRate, IniName);
							EncSettings.output_sampleRate = GetPrivateProfileInt(name, "SampleRate", EncSettings.output_sampleRate, IniName);
							EncSettings.output_numChannels = GetPrivateProfileInt(name, "NumChannels", EncSettings.output_numChannels, IniName);
							EncSettings.QualityMode = GetPrivateProfileInt(name, "QualityMode", 8, IniName);
							Enc->ChangeSettings(&EncSettings);
						}
					}
				}
				break;

			case 2:
			// map any AAC LC from the prior versions to FHG AAC with 5.62+
			case 3:
				{ // FHG AAC
					if (C_ENCODER_FHGAAC::isPresent(module.hwndParent)) {
						Encoder[i].SetEncoder(new C_ENCODER_FHGAAC(module.hwndParent), 1);
						C_ENCODER *Enc = Encoder[i].GetEncoder();
						if (Enc) {
							T_ENCODER_FHGAAC_INFO EncSettings;
							int infosize = sizeof (EncSettings);
							T_ENCODER_FHGAAC_INFO *encset = (T_ENCODER_FHGAAC_INFO *) Enc->GetExtInfo(&infosize);
							if (encset && infosize) memcpy(&EncSettings, encset, infosize);
							EncSettings.input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
							EncSettings.input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
							Enc->ChangeSettings(&EncSettings);
							((C_ENCODER_NSV*) Enc)->ReadConfFile(IniName, name);
						}
					} else if (C_ENCODER_AACP::isPresent(module.hwndParent)) { // AAC+
						Encoder[i].SetEncoder(new C_ENCODER_AACP(module.hwndParent), 1);
						C_ENCODER *Enc = Encoder[i].GetEncoder();
						if (Enc) {
							T_ENCODER_AACP_INFO EncSettings;
							int infosize = sizeof (EncSettings);
							T_ENCODER_AACP_INFO *encset = (T_ENCODER_AACP_INFO *) Enc->GetExtInfo(&infosize);
							if (encset && infosize) memcpy(&EncSettings, encset, infosize);
							EncSettings.input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
							EncSettings.input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
							Enc->ChangeSettings(&EncSettings);
							((C_ENCODER_NSV*) Enc)->ReadConfFile(IniName, name);
						}
					} else {
						//Encoder[i].SetEncoder(NULL);
						// attempt to get to a valid encoder if the aac one disappeared
						goto fallback;
					}
				}
				break;
#ifdef USE_OGG
			case 4:
				{ // OGG
					if (C_ENCODER_OGG::isPresent(module.hwndParent)) {
						Encoder[i].SetEncoder(new C_ENCODER_OGG(module.hwndParent), 1);
						C_ENCODER *Enc = Encoder[i].GetEncoder();
						if (Enc) {
							T_ENCODER_OGG_INFO EncSettings;
							int infosize = sizeof (EncSettings);
							T_ENCODER_OGG_INFO *encset = (T_ENCODER_OGG_INFO *) Enc->GetExtInfo(&infosize);
							if (encset && infosize) memcpy(&EncSettings, encset, infosize);
							EncSettings.input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
							EncSettings.input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
							Enc->ChangeSettings(&EncSettings);
							((C_ENCODER_NSV*) Enc)->ReadConfFile(IniName, name);
						}
					} else Encoder[i].SetEncoder(NULL);
				}
				break;
#endif // USE_OGG
			default:
				{
					Encoder[i].SetEncoder(NULL);
				}
				break;
		}
		ReleaseMutex(Enc_mutex[i]);
	}
}

void SetEncoderPanelMode(HWND hDlg, C_ENCODER * Enc) {
	BOOL show = (Enc != NULL);
	if (show) {
		if (Enc->UseNsvConfig() == true) {
			ShowWindowDlgItem(hDlg, IDC_ENCSETTINGS, SW_HIDE);
			ShowWindowDlgItem(hDlg, IDC_ENCSETTINGS_BUTTON, SW_SHOW);
			wchar_t tmp[128] = {0};
			StringCchPrintfW(tmp, ARRAYSIZE(tmp), LocalisedString(IDS_CURRENT_BITRATE, NULL, 0), ((T_EncoderIOVals*) Enc->GetExtInfo())->output_bitRate);
			SetDlgItemTextW(hDlg, IDC_ENCSETTINGS_LABEL, tmp);
			ShowWindowDlgItem(hDlg, IDC_ENCSETTINGS_LAME_VER, SW_HIDE);
		} else {
			wchar_t *lame_version = BuildLameVersion();
			if (lame_version && *lame_version)
			{
				ShowWindowDlgItem(hDlg, IDC_ENCSETTINGS, SW_SHOW);
				ShowWindowDlgItem(hDlg, IDC_ENCSETTINGS_BUTTON, SW_HIDE);
				SetDlgItemTextW(hDlg, IDC_ENCSETTINGS_LABEL, LocalisedString(IDS_ENCODER_SETTINGS, NULL, 0));

				wchar_t tmp[128] = {0};
				StringCchPrintfW(tmp, ARRAYSIZE(tmp), LocalisedString(IDS_LAME_ENCODER_VER, NULL, 0), lame_version);
				SetDlgItemTextW(hDlg, IDC_ENCSETTINGS_LAME_VER, tmp);
				ShowWindowDlgItem(hDlg, IDC_ENCSETTINGS_LAME_VER, SW_SHOW);
			} else {
				ShowWindowDlgItem(hDlg, IDC_ENCSETTINGS, SW_HIDE);
				ShowWindowDlgItem(hDlg, IDC_ENCSETTINGS_BUTTON, SW_HIDE);
				SetDlgItemTextW(hDlg, IDC_ENCSETTINGS_LABEL, LocalisedString(IDS_MP3_ENCODING_NOT_AVAILABLE, NULL, 0));
			}
		}
	} else {
		ShowWindowDlgItem(hDlg, IDC_ENCSETTINGS_LAME_VER, SW_HIDE);
	}

	// show / hide the groupbox around the main encoder options
	// which is setup to make it look like it's only around the
	// options available depending upon the mode that is in use
	ShowWindowDlgItem(hDlg, IDC_INFO_FRAME4, !show);
	ShowWindowDlgItem(hDlg, IDC_INFO_FRAME5, show);

	// show / hide the save encoded audio options as applicable
	ShowWindowDlgItem(hDlg, IDC_INFO_FRAME3, show);
	ShowWindowDlgItem(hDlg, IDC_SAVE_ENCODED_AUDIO, show);
	ShowWindowDlgItem(hDlg, IDC_SAVE_ENCODED_AUDIO_EDIT, show);
	ShowWindowDlgItem(hDlg, IDC_SAVE_ENCODED_AUDIO_BROWSE, show);
}

void FreeStreamAlbumArt(int Index) {
	if (streamImage[Index] > (ARGB32 *)0 &&
		streamImage[Index] != (ARGB32 *)-1 &&
		WASABI_API_MEMMGR) {
		WASABI_API_MEMMGR->sysFree(streamImage[Index]);
		streamImage[Index] = (ARGB32 *)-1;
	}
	streamLength[Index] = 0;
}

// destroy dlg and exit
int doQuit(void) {
	if (nowPlayingHook) {
		UnhookWindowsHookEx(nowPlayingHook);
		nowPlayingHook = 0;
	}
	if (nowPlayingHook2) {
		UnhookWindowsHookEx(nowPlayingHook2);
		nowPlayingHook2 = 0;
	}

	RemoveSystrayIcon(hMainDLG, SYSTRAY_ICY_ICON);

	GetWindowRect(hMainDLG, &mainrect);

	KillTimer(hMainDLG, wnd[curtab].id);
	KillTimer(hMainDLG, IDD_ENCODER);
	KillTimer(hMainDLG, 666);
	KillTimer(hMainDLG, 1234);
	KillTimer(hMainDLG, 1337);
	KillTimer(hMainDLG, 2234);
	KillTimer(hMainDLG, 2235);
	if (curtab == 1) KillTimer(wnd[curtab].hWnd, out_wnd[curouttab].id);
	if (curtab == 2) KillTimer(wnd[curtab].hWnd, in_wnd[InputDevice].id);
	ini_modified = 1;
	/* Disconnect all outputs */
	int done;
	do {
		done = 1;
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			MY_T_OUTPUT *Out = &Output[i];
			if (Out->Encoder != -1 && Out->Handle != -1) {
				SHOUTCAST_OUTPUT *Enc = &Encoder[Out->Encoder];
				int state = Enc->GetState(Out->Handle);
				if (state != OUT_DISCONNECTED && state != OUT_ERROR) {
					done = 0;
					Enc->DisconnectOutput(Out->Handle);
				} else {
					// shutdown the logging options
					if (Out->Logging) {
						StopLogging(i);
					}
					if (Out->nextTrackLog) {
						StopNextTracks(i);
					}
					if (Out->saveEncoded) {
						StopSaveEncoded(i);
					}
				}
				Enc->Run(OM_OUTPUT | OM_OTHER);
			}
		}
		Sleep(200); 
	} while (!done);

	// reset levels if PTT is enabled when we are closing
	if (FadeOut && InputDevice) {
		int micsrc = Input_Device_ID  >= 1 ? MIXERLINE_COMPONENTTYPE_SRC_LINE : MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE;
		setlev(micsrc, 0);
		setlev(MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT, MusVol * 10);
	}

	if (hthread) {
		CloseHandle(hthread);
		hthread = NULL;
	}

	if (hthreadout) {
		CloseHandle(hthreadout);
		hthreadout = NULL;
	}

	ReleaseMutex(cf_mutex);
	Soundcard.Close();

	SendMessage(hMainDLG, WM_TIMER, MAKEWPARAM(666,0), 0); // force an INI save

	if (playingImage && WASABI_API_MEMMGR) {
		WASABI_API_MEMMGR->sysFree(playingImage);
		playingImage = 0;
	}

	for (int i = 0; i < NUM_OUTPUTS; i++) {
		FreeStreamAlbumArt(i);
	}

	if (libinst) {
		FreeLibrary(libinst);
		libinst = 0;
	}
	C_ENCODER_FHGAAC::Unload();
	C_ENCODER_AACP::Unload();
#ifdef USE_OGG
	C_ENCODER_OGG::Unload();
#endif // USE_OGG

	for (int i = 0; i < num_tabwnds; i++) DestroyWindow(wnd[i].hWnd);
	for (int i = 0; i < num_inwnds; i++) DestroyWindow(in_wnd[i].hWnd);
	for (int i = 0; i < num_outwnds; i++) DestroyWindow(out_wnd[i].hWnd);

	for (int ii = 0; ii < NUM_OUTPUTS; ii++) {
		MY_T_OUTPUT *Out = &Output[ii];
		// removed encoder selection
		if (Out->Encoder != -1) {
			if (Out->Handle != -1) {
				if (WaitForSingleObject(Enc_mutex[Out->Encoder], INFINITE) == WAIT_OBJECT_0) {
					Encoder[Out->Encoder].RemoveOutput(Out->Handle);
					ReleaseMutex(Enc_mutex[Out->Encoder]);
				}
			}
		}
	}

	DestroyWindow(hMainDLG);
	hMainDLG = NULL;

	num_cols = num_outwnds = num_inwnds = num_tabwnds = 0;
	memset(&col, 0, sizeof(col));
	memset(&wnd, 0, sizeof(wnd));
	memset(&out_wnd, 0, sizeof(out_wnd));
	memset(&in_wnd, 0, sizeof(in_wnd));
	memset(&Output, 0, sizeof(Output));
	WASABI_API_LNG_HINST = WASABI_API_ORIG_HINST = 0;

	memset(&lastFile, 0, sizeof(lastFile));
	memset(&lastSec, 0, sizeof(lastSec));
	memset(&lastFile, 0, sizeof(lastFile));
	memset(&lastMode, -1, sizeof(lastMode));
	memset(&lastEnable, 0, sizeof(lastEnable));
	memset(&buttonWnd, 0, sizeof(buttonWnd));
	memset(&tabWnd, 0, sizeof(tabWnd));
	memset(&outTabWnd, 0, sizeof(outTabWnd));
	memset(&streamLength, 0, sizeof(streamLength));
	memset(&secChanged, 0, sizeof(secChanged));

	playingImage_w = playingImage_h = playingLength = playingType;

	if (boldFont) {
		DeleteObject(boldFont);
		boldFont = NULL;
	}
	normalFont = NULL;

	ServiceRelease(WASABI_API_SVC, AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_SVC, WASABI_API_LNG, languageApiGUID);
	ServiceRelease(WASABI_API_SVC, WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceRelease(WASABI_API_SVC, AGAVE_API_ALBUMART, albumArtGUID);
	ServiceRelease(WASABI_API_SVC, WASABI_API_QUEUEMGR, QueueManagerApiGUID);
	ServiceRelease(WASABI_API_SVC, WASABI_API_EXPLORERFINDFILE,ExplorerFindFileApiGUID);
	ServiceRelease(WASABI_API_SVC, WAC_API_DOWNLOADMANAGER, DownloadManagerGUID);
	WASABI_API_SVC = NULL;

	return 1;
}

void SetBoldDialogItemFont(HWND hwndControl) {
	if (!boldFont) {
		HFONT hFont = (HFONT)SendMessageW(hMainDLG, WM_GETFONT, 0, 0);
		LOGFONTW lf = {0};
		GetObjectW(hFont, sizeof(LOGFONTW), &lf);
		lf.lfWeight = FW_BOLD;
		boldFont = CreateFontIndirectW(&lf);
	}
	if (boldFont) {
		SendMessageW(hwndControl, WM_SETFONT, (WPARAM)boldFont, MAKELPARAM(1,0));
	}
}

LRESULT WINAPI headerProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_SETCURSOR) {
		return TRUE;
	}
	return CallWindowProcW(prevHeaderProc, hWnd, uMsg, wParam, lParam);
}

LRESULT WINAPI listViewProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_STREAM_1:
				case IDC_STREAM_2:
				case IDC_STREAM_3:
				case IDC_STREAM_4:
				case IDC_STREAM_5:
				{
					int oldCurSelPos = Connection_CurSelPos;
					Connection_CurSelPos = (LOWORD(wParam) - IDC_STREAM_1);
					ListView_SetItemState(hWnd, Connection_CurSelPos, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
					SetFocus(hWnd);
					SendMessage(wnd[1].hWnd, WM_COMMAND, MAKEWPARAM(IDC_CONNECT, BN_CLICKED), (LPARAM)GetDlgItem(wnd[1].hWnd, IDC_CONNECT));
					Connection_CurSelPos = oldCurSelPos;
				}
				return 0;
			}
			break;

		case WM_NOTIFY:
			if(((LPNMHDR)lParam)->code == HDN_BEGINTRACKW ||
			   ((LPNMHDR)lParam)->code == HDN_ITEMCHANGINGW) {
				return TRUE;
			}
		break;
	}
	return CallWindowProcW(prevListViewProc, hWnd, uMsg, wParam, lParam);
}

void SetTabErrorText(HWND hwnd, int index, int current, LPRECT r, bool update = false) {
	HDC hDC = GetDC(hwnd);
	if (!normalFont) {
		normalFont = (HFONT)SendMessageW(hMainDLG, WM_GETFONT, 0, 0);
	}
	HFONT hOldFont = (HFONT)SelectObject(hDC, normalFont);
	int oldTextColor = SetTextColor(hDC, (!update ? RGB(255,0,0) : RGB(0,0,255)));
	int oldBkMode = SetBkMode(hDC, TRANSPARENT);

	wchar_t buf[128] = {0};
	TCITEMW pitem = {0};
	pitem.mask = TCIF_TEXT;
	pitem.pszText = buf;
	pitem.cchTextMax = ARRAYSIZE(buf);
	SendMessageW(hwnd, TCM_GETITEMW, index, (LPARAM)&pitem);

	r->top += (current ? 1 : 3);
	r->left += 6;

	// if themeing is enabled then we need to paint the background to avoid the font going weird / double-bold like
	// and to ensure we're correct, am taking a 1px sliver and stretching it across the area before drawing the text
	if (isthemethere) {
		StretchBlt(hDC, r->left, r->top + 1, r->right - r->left - 3, r->bottom - r->top - 3, hDC, r->left - 4, r->top + 1,  1, r->bottom - r->top - 3, SRCCOPY);
	}

	DrawTextW(hDC, pitem.pszText, wcslen(pitem.pszText), r, DT_SINGLELINE);
	SetTextColor(hDC, oldTextColor);
	SetBkMode(hDC, oldBkMode);
	SelectObject(hDC, hOldFont);
	ReleaseDC(hwnd, hDC);
}

LRESULT WINAPI tabWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT ret = CallWindowProcW(prevTabWndProc, hWnd, uMsg, wParam, lParam);
	if (uMsg == WM_PAINT) {
		RECT r = {0};
		int item = ListView_GetNextItem(GetDlgItem(wnd[0].hWnd, IDC_OUTPUTSTATUS), -1, LVNI_SELECTED);
		if ((curtab == 1 ?
			 (lastMode[Connection_CurSelPos] >= 3 && lastMode[Connection_CurSelPos] <= 6) :
			 (lastMode[item] >= 3 && lastMode[item] <= 6)
			) && TabCtrl_GetItemRect(hWnd, 1, &r)) {
			SetTabErrorText(hWnd, 1, (curtab == 1), &r);
		}

		// show the update flag on things!
		if (updateStr && updateStr[0]) {
			RECT r = {0};
			TabCtrl_GetItemRect(tabWnd, 3, &r);
			SetTabErrorText(tabWnd, 3, (curtab == 3), &r, TRUE);
		}
	}
	return ret;
}

LRESULT WINAPI outTabWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT ret = CallWindowProcW(prevOutTabWndProc, hWnd, uMsg, wParam, lParam);
	if (uMsg == WM_PAINT) {
		for (int i = 0; i < NUM_OUTPUTS; i++) {
			RECT r = {0};
			int index[] = {0, 0, 1, 2};
			if ((lastMode[i] >= 3 && lastMode[i] <= 6) && (i == Connection_CurSelPos) && TabCtrl_GetItemRect(hWnd, index[(lastMode[i] - 3)], &r)) {
				SetTabErrorText(hWnd, index[(lastMode[i] - 3)], (curouttab == index[(lastMode[i] - 3)]), &r);
			}
		}
	}
	return ret;
}

LRESULT WINAPI buttonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int ret;
	switch (uMsg) {
		case WM_LBUTTONDOWN:
			ret = CallWindowProcW(prevButtonProc, hWnd, uMsg, wParam, lParam);
			if (SendDlgItemMessage(GetParent(hWnd), IDC_LOCK, BM_GETSTATE, 0, 0) != BST_CHECKED) {
				blockmousemove = 1;
				KillTimer(hMainDLG, 2234);
				KillTimer(hMainDLG, 2235);
				if ((MicFadeStartTime != 0 || FadeStartTime != 0) && FadeOut != 1) {
					clock_t myTime = clock();
					if (FadeStartTime != 0) FadeStartTime = myTime - ((FadeTime * 100)-(myTime - FadeStartTime));
					if (MicFadeStartTime != 0) MicFadeStartTime = myTime - ((MicFadeTime * 100)-(myTime - MicFadeStartTime));
				}
				FadeOut = 1;
				SetTimer(hMainDLG, 2234, 10, NULL); // fade out
				SetTimer(hMainDLG, 2235, 10, NULL); // fade out
				//if (FadeOut) do_capture();
				#ifdef CAPTURE_TESTING
				if (FadeOut) {
					if (!pPlayer) {
						pPlayer = new Player(hMainDLG);
					}
					if (!pCallbacks) {
						pCallbacks = new CPlayerCallbacks();
					}
					pPlayer->SetPlayerCallbacks(pCallbacks);
					pPlayer->RefreshDeviceList(eRender);
					pPlayer->RefreshDeviceList(eCapture);
					pPlayer->SelectDefaultDevice(eRender, eConsole);
					pPlayer->SelectDefaultDevice(eCapture, eConsole);
					pPlayer->SelectDeviceFromList(eCapture, 0);
					//pPlayer->SelectDeviceFromList(eRender, 2);
			        if (pPlayer->Play(eCaptureEndpoint) == FALSE) {
						return TRUE;
					}
				}
				#endif
			} else {
				SendMessage(hWnd, BM_SETSTATE, TRUE, 0);
				ret = 0;
			}
			return ret;

		case WM_MOUSEMOVE:
			if (blockmousemove) return 0;
			break;

		case WM_LBUTTONUP:
			ret = CallWindowProcW(prevButtonProc, hWnd, uMsg, wParam, lParam);
			if (SendDlgItemMessage(GetParent(hWnd), IDC_LOCK, BM_GETSTATE, 0, 0) != BST_CHECKED) {
				blockmousemove = 0;
				KillTimer(hMainDLG, 2234);
				KillTimer(hMainDLG, 2235);
				if ((MicFadeStartTime != 0 || FadeStartTime != 0) && FadeOut != 0) {
					clock_t myTime = clock();
					if (FadeStartTime != 0) FadeStartTime = myTime - ((FadeTime * 100)-(myTime - FadeStartTime));
					if (MicFadeStartTime != 0) MicFadeStartTime = myTime - ((MicFadeTime * 100)-(myTime - MicFadeStartTime));
				}
				FadeOut = 0;
				SetTimer(hMainDLG, 2234, 10, NULL); // fade in
				SetTimer(hMainDLG, 2235, 10, NULL); // fade in
			} else {
				SendMessage(hWnd, BM_SETSTATE, TRUE, 0);
				ret = 0;
			}
			return ret;
	}
	return CallWindowProcW(prevButtonProc, hWnd, uMsg, wParam, lParam);
}

int CALLBACK EncFunc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_INITDIALOG:
			{
				// doing this to get the button + combobox to appear as the same size when scrolling through them all
				RECT r;
				GetClientRect(GetDlgItem(hDlg, IDC_ENCSETTINGS), &r);
				MapWindowPoints(GetDlgItem(hDlg, IDC_ENCSETTINGS), hDlg, (LPPOINT)&r,2);
				InflateRect(&r, 1, 1);
				SetWindowPos(GetDlgItem(hDlg, IDC_ENCSETTINGS_BUTTON), 0, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_NOZORDER|SWP_NOACTIVATE);
			}
			break;
		case WM_COMMAND:
			{
				switch (LOWORD(wParam)) {
				case IDC_ENCODERLIST:
					{
						if (HIWORD(wParam) == LBN_SELCHANGE) {
							MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
							Encoder_CurSelPos = Out->Encoder;

							if (WaitForSingleObject(Enc_mutex[Encoder_CurSelPos], INFINITE) == WAIT_OBJECT_0) {
								C_ENCODER *Enc = Encoder[Encoder_CurSelPos].GetEncoder();
								HWND e = hDlg;
								SendDlgItemMessage(e, IDC_ENCTYPE, CB_RESETCONTENT, 0, 0);
								int item = SendDlgItemMessageW(e, IDC_ENCTYPE, CB_ADDSTRING, 0, (LPARAM) LocalisedString(IDS_NONE, NULL, 0));
								SendDlgItemMessage(e, IDC_ENCTYPE, CB_SETITEMDATA, item, (LPARAM)"None");

								item = SendDlgItemMessageW(e, IDC_ENCTYPE, CB_ADDSTRING, 0, (LPARAM) LocalisedString(IDS_MP3_ENCODER, NULL, 0));
								SendDlgItemMessage(e, IDC_ENCTYPE, CB_SETITEMDATA, item, (LPARAM)"MP3 Encoder");

								if (C_ENCODER_FHGAAC::isPresent(module.hwndParent)) {
									item = SendDlgItemMessageW(e, IDC_ENCTYPE, CB_ADDSTRING, 0, (LPARAM) LocalisedString(IDS_FHGAAC_ENCODER, NULL, 0));
									SendDlgItemMessage(e, IDC_ENCTYPE, CB_SETITEMDATA, item, (LPARAM)"Fraunhofer Encoder");
								} else if (C_ENCODER_AACP::isPresent(module.hwndParent)) {
									item = SendDlgItemMessageW(e, IDC_ENCTYPE, CB_ADDSTRING, 0, (LPARAM) LocalisedString(IDS_AACP_ENCODER, NULL, 0));
									SendDlgItemMessage(e, IDC_ENCTYPE, CB_SETITEMDATA, item, (LPARAM)"AAC+ Encoder");
								}
#ifdef USE_OGG
								if (C_ENCODER_OGG::isPresent(module.hwndParent)) {
									// TODO
									item = SendDlgItemMessageW(e, IDC_ENCTYPE, CB_ADDSTRING, 0, (LPARAM) L"OGG Vorbis Encoder"/*LocalisedString(IDS_OGG_ENCODER, NULL, 0)*/);
									SendDlgItemMessage(e, IDC_ENCTYPE, CB_SETITEMDATA, item, (LPARAM)"OGG Vorbis Encoder");
								}
#endif // USE_OGG

								SendDlgItemMessage(e, IDC_ENCSETTINGS, CB_RESETCONTENT, 0, 0);
								SetEncoderPanelMode(e, Enc);

								int attribnum = 0, typeval = 0;
								if (Enc) {
									int i;
									for (int i = 0; i < NUM_ENCODERS; i++) {
										char* encoder = (char*)SendDlgItemMessage(e, IDC_ENCTYPE, CB_GETITEMDATA, i, 0);
										if (!strcmp(Enc->GetName(), encoder)) {
											typeval = i;
											break;
										}
									}

									int infosize = sizeof (T_EncoderIOVals);
									T_ENCODER_MP3_INFO *EncInfo = (T_ENCODER_MP3_INFO *) Enc->GetExtInfo(&infosize);
									for (i = Enc->GetNumAttribs() - 1; i >= 0; i--) {
										T_ATTRIB attrib;
										Enc->EnumAttrib(i, &attrib);
										SendDlgItemMessage(e, IDC_ENCSETTINGS, CB_INSERTSTRING, 0, (LPARAM) attrib.Text);
										T_ENCODER_MP3_INFO *OutVal = (T_ENCODER_MP3_INFO *) attrib.OutputVals;
										if (OutVal && EncInfo && infosize) {
											T_ENCODER_MP3_INFO *EncSettings = (T_ENCODER_MP3_INFO *) EncInfo;
											if(OutVal->output_bitRate == EncSettings->output_bitRate &&
											   OutVal->output_sampleRate == EncSettings->output_sampleRate &&
											   OutVal->output_numChannels == EncSettings->output_numChannels) {
												attribnum = i;
											}
										}
									}
								} else {
									SendDlgItemMessageW(e, IDC_ENCSETTINGS, CB_ADDSTRING, 0, (LPARAM) LocalisedString(IDS_NONE, NULL, 0));
									ShowWindowDlgItem(e, IDC_ENCSETTINGS, SW_HIDE);
									ShowWindowDlgItem(e, IDC_ENCSETTINGS_BUTTON, SW_HIDE);
									SetDlgItemTextW(e, IDC_ENCSETTINGS_LABEL, LocalisedString(IDS_NO_ENCODER_SELECTED, NULL, 0));
								}

								SendDlgItemMessage(e, IDC_ENCSETTINGS, CB_SETCURSEL, attribnum, 0);
								SendDlgItemMessage(e, IDC_ENCTYPE, CB_SETCURSEL, typeval, 0);
								ReleaseMutex(Enc_mutex[Encoder_CurSelPos]);
								ini_modified = 1;
								SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_ENCSETTINGS, CBN_SELCHANGE), (LPARAM) GetDlgItem(hDlg, IDC_ENCSETTINGS));
							}
						}
					}
					break;

				case IDC_ENCTYPE:
					{
						if (HIWORD(wParam) == CBN_SELCHANGE) {
							int typenum = SendMessage((HWND) lParam, CB_GETCURSEL, 0, 0);

							// check to see if this is the same encoder as last time as
							// there's no need to update it if there hasn't been a change
							// so selecting the same encoder won't cause a settings reset
							if (typenum && typenum == Enc_LastType[Encoder_CurSelPos]) break;
							else Enc_LastType[Encoder_CurSelPos] = typenum;

							char* typestr = (char*)SendMessage((HWND) lParam, CB_GETITEMDATA, typenum, 0);
							if (WaitForSingleObject(Enc_mutex[Encoder_CurSelPos], INFINITE) == WAIT_OBJECT_0) {
								if (!strcmpi("MP3 Encoder", typestr)) {
									//lame setup
									Encoder[Encoder_CurSelPos].SetLame(init, params, encode, finish);
									Encoder[Encoder_CurSelPos].SetEncoder(new C_ENCODER_MP3(init, params, encode, finish), 1);

									C_ENCODER *Enc = Encoder[Encoder_CurSelPos].GetEncoder();

									if (Enc) {
										if (strcmpi(Enc->GetName(), "MP3 Encoder") == 0) {
											T_ENCODER_MP3_INFO EncSettings;
											int infosize = sizeof (EncSettings);
											T_ENCODER_MP3_INFO *encset = (T_ENCODER_MP3_INFO *) Enc->GetExtInfo(&infosize);
											if (encset && infosize)
												memcpy(&EncSettings, encset, infosize);

											EncSettings.input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
											EncSettings.input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
											EncSettings.output_bitRate = MP3_DEFAULT_OUTPUTBITRATE;
											EncSettings.output_sampleRate = MP3_DEFAULT_OUTPUTSAMPLERATE;
											EncSettings.output_numChannels = MP3_DEFAULT_OUTPUTNUMCHANNELS;
											Enc->ChangeSettings(&EncSettings);
										}
									}
								} else if (!strcmpi("Fraunhofer Encoder", typestr)) { // FHG AAC
									Encoder[Encoder_CurSelPos].SetEncoder(new C_ENCODER_FHGAAC(module.hwndParent), 1);

									C_ENCODER *Enc = Encoder[Encoder_CurSelPos].GetEncoder();
									if (Enc) {
										T_ENCODER_FHGAAC_INFO EncSettings;
										int infosize = sizeof (EncSettings);
										T_ENCODER_FHGAAC_INFO *encset = (T_ENCODER_FHGAAC_INFO *) Enc->GetExtInfo(&infosize);
										if (encset && infosize) memcpy(&EncSettings, encset, infosize);
										EncSettings.input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
										EncSettings.input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
										Enc->ChangeSettings(&EncSettings);
									}
								} else if (!strcmpi("AAC+ Encoder", typestr)) { //AAC+
									Encoder[Encoder_CurSelPos].SetEncoder(new C_ENCODER_AACP(module.hwndParent), 1);

									C_ENCODER *Enc = Encoder[Encoder_CurSelPos].GetEncoder();
									if (Enc) {
										T_ENCODER_AACP_INFO EncSettings;
										int infosize = sizeof (EncSettings);
										T_ENCODER_AACP_INFO *encset = (T_ENCODER_AACP_INFO *) Enc->GetExtInfo(&infosize);
										if (encset && infosize) memcpy(&EncSettings, encset, infosize);
										EncSettings.input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
										EncSettings.input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
										Enc->ChangeSettings(&EncSettings);
									}
								}
#ifdef USE_OGG
								else if (!strcmpi("OGG Vorbis Encoder", typestr)) { //OGG
									Encoder[Encoder_CurSelPos].SetEncoder(new C_ENCODER_OGG(module.hwndParent), 1);

									C_ENCODER *Enc = Encoder[Encoder_CurSelPos].GetEncoder();
									if (Enc) {
										T_ENCODER_OGG_INFO EncSettings;
										int infosize = sizeof (EncSettings);
										T_ENCODER_OGG_INFO *encset = (T_ENCODER_OGG_INFO *) Enc->GetExtInfo(&infosize);
										if (encset && infosize) memcpy(&EncSettings, encset, infosize);
										EncSettings.input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
										EncSettings.input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
										Enc->ChangeSettings(&EncSettings);
									}
								}
#endif // USE_OGG
								else {
									int i;
									for (i = 0; i < NUM_OUTPUTS; i++) {
										MY_T_OUTPUT *Out = &Output[i];
										if (Out->Encoder == Encoder_CurSelPos && Out->Handle != -1) Encoder[Encoder_CurSelPos].DisconnectOutput(Out->Handle);
									}
									Encoder[Encoder_CurSelPos].SetEncoder(NULL);
								}

								// if we can re-map the extension then we do so against the encoder selected
								// which will override what was manually set but this ensures it is correct!
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								if (Out->saveEncodedPath[0] && typenum != 0) {
									PathRemoveExtensionW(Out->saveEncodedPath);
									PathAddExtensionW(Out->saveEncodedPath,
													  (Enc_LastType[Connection_CurSelPos] != 0 ?
													   (!strcmp(Encoder[Out->Encoder].GetEncoder()->GetContentType(),"audio/mpeg") ? L".mp3" :
													    (!strcmp(Encoder[Out->Encoder].GetEncoder()->GetContentType(),"audio/ogg") ? L".ogg" : L".aac")) : L""));

									// update things as the filename was changed
									SetDlgItemTextW(out_wnd[2].hWnd, IDC_SAVE_ENCODED_AUDIO_EDIT, Out->saveEncodedPath);
									if (Out->saveEncoded) {
										StopSaveEncoded(Connection_CurSelPos);
										StartSaveEncoded(Connection_CurSelPos, Out->saveEncodedPath);
									}
								}

								SetEncoderPanelMode(hDlg, Encoder[Encoder_CurSelPos].GetEncoder());
								ReleaseMutex(Enc_mutex[Encoder_CurSelPos]);
								ini_modified = 1;
								SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_ENCODERLIST, LBN_SELCHANGE), (LPARAM) GetDlgItem(hDlg, IDC_ENCODERLIST));
							}
						}
					}
					break;

				case IDC_ENCSETTINGS_BUTTON:
					{
						C_ENCODER *Enc = Encoder[Encoder_CurSelPos].GetEncoder();
						if (Enc) if (Enc->UseNsvConfig()) {
							((C_ENCODER_NSV*) Enc)->Configure(hDlg, module.hDllInstance);
							if (WaitForSingleObject(Enc_mutex[Encoder_CurSelPos], INFINITE) == WAIT_OBJECT_0) {
								ini_modified = 1;
								for (int i = 0; i < NUM_OUTPUTS; i++) {
									MY_T_OUTPUT *Out = &Output[i];
									if (Out->Encoder == Encoder_CurSelPos && Out->Handle != -1) {
										Encoder[Encoder_CurSelPos].DisconnectOutput(Out->Handle, 1, 5);
									}
								}
								ReleaseMutex(Enc_mutex[Encoder_CurSelPos]);
							}
						}
						SetEncoderPanelMode(hDlg, Enc);
					}
					break;

				case IDC_ENCSETTINGS:
					{
						if (HIWORD(wParam) == CBN_SELCHANGE) {
							T_ATTRIB attrib;
							int attribnum = SendMessage((HWND) lParam, CB_GETCURSEL, 0, 0);
							if (WaitForSingleObject(Enc_mutex[Encoder_CurSelPos], INFINITE) == WAIT_OBJECT_0) {
								C_ENCODER *Enc = Encoder[Encoder_CurSelPos].GetEncoder();
								if (Enc) {
									int i;
									if (attribnum < 0 || attribnum >= Enc->GetNumAttribs()) attribnum = 0;
									int oldattrib = -1;
									int infosize = sizeof (T_EncoderIOVals);
									T_EncoderIOVals *EncInfo = (T_EncoderIOVals *) Enc->GetExtInfo(&infosize);
									for (i = Enc->GetNumAttribs() - 1; i >= 0 && oldattrib == -1; i--) {
										Enc->EnumAttrib(i, &attrib);
										if (attrib.OutputVals && EncInfo && infosize) {
											T_ENCODER_MP3_INFO *EncSettings = (T_ENCODER_MP3_INFO *) attrib.OutputVals;
											T_ENCODER_MP3_INFO *EncSettings2 = (T_ENCODER_MP3_INFO *) EncInfo;
											//if (memcmp(attrib.OutputVals, EncInfo, infosize) == 0) oldattrib = i;
											if(EncSettings2->output_bitRate == EncSettings->output_bitRate &&
											   EncSettings2->output_sampleRate == EncSettings->output_sampleRate &&
											   EncSettings2->output_numChannels == EncSettings->output_numChannels) {
												oldattrib = i;
											}
										}
									}
									if (attribnum != oldattrib) {
										if (Enc->EnumAttrib(attribnum, &attrib)) {
											// we make sure that we set the input channels and samplerate to
											// that of the mode being used instead of the default values as
											// this allows us to get things to work correctly (done in 2.3.2)
											T_ENCODER_MP3_INFO *EncSettings = (T_ENCODER_MP3_INFO *) attrib.OutputVals;
											if(InputDevice == 1) {
												EncSettings->input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
												EncSettings->input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
											}
											Enc->ChangeSettings(EncSettings);
											ini_modified = 1;
											for (i = 0; i < NUM_OUTPUTS; i++) {
												MY_T_OUTPUT *Out = &Output[i];
												if (Out->Encoder == Encoder_CurSelPos && Out->Handle != -1) {
													Encoder[Encoder_CurSelPos].DisconnectOutput(Out->Handle, 1, 5);
												}
											}
										}
									}
								}
								ReleaseMutex(Enc_mutex[Encoder_CurSelPos]);
							}
						}
					}
					break;
				}//switch
			}
			break;
	}// umsg
	return CallWindowProcW((WNDPROC)DialogFunc, hDlg, uMsg, wParam, lParam);
}

/* Connection Page callback */
INT_PTR CALLBACK ConnectionFunc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_INITDIALOG:
			{
				switch (lParam) {
					case IDD_CONNECTION:
						{
							int i;
							wchar_t temp[128];
							SendDlgItemMessage(hDlg, IDC_OUTPUTLIST, LB_RESETCONTENT, 0, 0);
							for (i = 0; i < NUM_OUTPUTS; i++) {
								T_OUTPUT_CONFIG *Out = &Output[i].Config;
								SendDlgItemMessageW(hDlg, IDC_OUTPUTLIST, LB_ADDSTRING, 0, (LPARAM) Out->DisplayName);
							}
							SendDlgItemMessage(hDlg, IDC_OUTPUTLIST, LB_SETCURSEL, Connection_CurSelPos, 0);
							num_outwnds = 0;
							SendDlgItemMessage(hDlg, IDC_CONTAB, TCM_DELETEALLITEMS, 0, 0);
							AddOutTab(IDD_PANEL_LOGIN, LocalisedString(IDS_PANEL_LOGIN, temp, 128), hDlg, DialogFunc, IDC_CONTAB, IDC_PANELRECT_C, 0);
							AddOutTab(IDD_PANEL_DIRECTORY, LocalisedString(IDS_PANEL_DIRECTORY, temp, 128), hDlg, DialogFunc, IDC_CONTAB, IDC_PANELRECT_C, 0);
							AddOutTab(IDD_ENCODER, LocalisedString(IDS_PANEL_ENCODERS, temp, 128), hDlg, EncFunc, IDC_CONTAB, IDC_PANELRECT_C, 0);
							AddOutTab(IDD_PANEL_TITLE, LocalisedString(IDS_PANEL_TITLES, temp, 128), hDlg, DialogFunc, IDC_CONTAB, IDC_PANELRECT_C, 0);
							AddOutTab(IDD_ARTWORK, LocalisedString(IDS_PANEL_ART, temp, 128), hDlg, DialogFunc, IDC_CONTAB, IDC_PANELRECT_C, 0);
							AddOutTab(IDD_LOGGING, LocalisedString(IDS_PANEL_LOGGING, temp, 128), hDlg, DialogFunc, IDC_CONTAB, IDC_PANELRECT_C, 0);
							SetOutTab(curouttab, hDlg, IDC_CONTAB);
							SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_OUTPUTLIST, LBN_SELCHANGE), (LPARAM) GetDlgItem(hDlg, IDC_OUTPUTLIST));
							SendDlgItemMessage(wnd[1].hWnd, IDC_AUTOCONNECT, BM_SETCHECK,Output[Connection_CurSelPos].AutoConnect ? BST_CHECKED : BST_UNCHECKED, 0);
						}
						break;
				}//lparam
			}
			return 0;
	}//uMsg
	return CallWindowProcW((WNDPROC)DialogFunc, hDlg, uMsg, wParam, lParam);
}

void ShowUpdateMessage(HWND hDlg)
{
	bool update = (updateStr && updateStr[0]);
	if (IsWindow(hDlg))
	{
		HWND control = GetDlgItem(hDlg, IDC_UPDATE_HEADER);
		if (update)
		{
			wchar_t message[128] = {0};
			StringCchPrintfW(message, ARRAYSIZE(message), WASABI_API_LNGSTRINGW(IDS_UPDATE_HEADER), updateStr);

			SetWindowTextW(control, message);
			SetBoldDialogItemFont(control);
		}
		else 
		{
			SetWindowTextW(control, WASABI_API_LNGSTRINGW(IDS_UPDATE));
			SendMessageW(control, WM_SETFONT, SendMessageW(hMainDLG, WM_GETFONT, 0, 0), 0);
			InvalidateRect(hDlg, 0, TRUE);
		}

		ShowWindowDlgItem(hDlg, IDC_STATIC_UPDATE, update);
		ShowWindowDlgItem(hDlg, IDC_UPDATELINK, update);
	}

	SetWindowTextW(hMainDLG, WASABI_API_LNGSTRINGW((update ? IDS_UPDATE_TITLE : IDS_MODULE_NAME)));

	if (IsWindow(tabWnd)) {
		RECT r = {0};
		TabCtrl_GetItemRect(tabWnd, 3, &r);
		InvalidateRect(tabWnd, &r, 0);
	}
}

class VersionCheckCallback : public ifc_downloadManagerCallback
{
public:
	void OnInit( DownloadToken token )
	{
		api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver( token );
		if ( http )
		{
			http->AllowCompression();
			http->addheader( "Accept: */*" );
		}
	}

	void OnFinish( DownloadToken token )
	{
		api_httpreceiver *http = WAC_API_DOWNLOADMANAGER->GetReceiver( token );
		if ( http && http->getreplycode() == 200 )
		{
			char *buf = 0;
			size_t size = 0;
			if ( WAC_API_DOWNLOADMANAGER->GetBuffer( token, (void **)&buf, &size ) == 0 && size > 0 )
			{
				buf[ size - 1 ] = 0;

				char *p = buf;
				while ( size && ( *p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' ) )
				{
					p++;
					size--;
				}

				// e.g. dsp_sc,2.3.4.210,http://download.nullsoft.com/shoutcast/tools/shoutcast-dsp-2-3-4-windows.exe
				if ( p && *p )
				{
					char *tok = strtok( p, "," );
					if ( tok )
					{
						if ( !strncmp( tok, "dsp_sc", 6 ) )
						{
							tok = strtok( NULL, "," );
							if ( tok )
							{
								bool needsUpdating = CompareVersions( tok );
								lstrcpyn( updateStr, ( needsUpdating ? tok : "" ), ARRAYSIZE( updateStr ) );
								WritePrivateProfileString( APP_Name, "Update", ( needsUpdating ? updateStr : 0 ), IniName );
								ShowUpdateMessage( updateWnd );
								if ( !needsUpdating )
								{
									ShowWindowDlgItem( updateWnd, IDC_STATIC_UPDATE, TRUE );
								}
								SetDlgItemTextW( updateWnd, IDC_STATIC_UPDATE, WASABI_API_LNGSTRINGW( ( needsUpdating ? IDS_HAS_NEW_UPDATE : IDS_NO_NEW_UPDATE ) ) );
							}
						}
					}
				}
			}
		}

		if ( IsWindow( updateWnd ) )
		{
			EnableWindowDlgItem( updateWnd, IDC_GET_UPDATE, TRUE );
			SetDlgItemTextW( updateWnd, IDC_GET_UPDATE, WASABI_API_LNGSTRINGW( IDS_CHECK_FOR_UPDATES ) );
		}
	}

	void OnError( DownloadToken token )
	{
		if ( IsWindow( updateWnd ) )
		{
			ShowWindowDlgItem( updateWnd, IDC_STATIC_UPDATE, TRUE );
			SetDlgItemTextW( updateWnd, IDC_STATIC_UPDATE, WASABI_API_LNGSTRINGW( IDS_CHECK_UPDATE_FAIL ) );
			EnableWindowDlgItem( updateWnd, IDC_GET_UPDATE, TRUE );
			SetDlgItemTextW( updateWnd, IDC_GET_UPDATE, WASABI_API_LNGSTRINGW( IDS_CHECK_FOR_UPDATES ) );
		}
	}

	RECVS_DISPATCH;
};

#define CBCLASS VersionCheckCallback
START_DISPATCH;
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONINIT,   OnInit )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONFINISH, OnFinish )
VCB( IFC_DOWNLOADMANAGERCALLBACK_ONERROR,  OnError )
END_DISPATCH;
#undef CBCLASS

static VersionCheckCallback versionCheckCallback;

static void CheckVersion( HWND hDlg )
{
	ShowWindowDlgItem( hDlg, IDC_STATIC_UPDATE, FALSE );
	ShowWindowDlgItem( hDlg, IDC_UPDATELINK, FALSE );

	if ( WAC_API_DOWNLOADMANAGER )
	{
		char url[ 128 ] = { 0 };
		StringCchPrintf( url, ARRAYSIZE( url ), "http://yp.shoutcast.com/update?c=dsp_sc&v=%s&wa=%x", APP_Version, GetWinampVersion( module.hwndParent ) );
		updateWnd = hDlg;
		WAC_API_DOWNLOADMANAGER->DownloadEx( url, &versionCheckCallback, api_downloadManager::DOWNLOADEX_BUFFER );
	}
	else
	{
		wchar_t title[ 128 ] = { 0 }, message[ 512 ] = { 0 };
		StringCchPrintfW( title, ARRAYSIZE( title ), LocalisedString( IDS_PLUGIN_NAME, NULL, 0 ), APP_VersionW );
		StringCchPrintfW( message, ARRAYSIZE( message ), LocalisedString( IDS_UPDATE_CHECK_ERROR, NULL, 0 ) );
		if ( MessageBoxW( module.hwndParent, message, title, MB_ICONWARNING | MB_YESNO ) == IDYES )
		{
			SendMessage( module.hwndParent, WM_WA_IPC, (WPARAM)DOWNLOAD_URL, IPC_OPEN_URL );
		}
		SetDlgItemTextW( hDlg, IDC_GET_UPDATE, WASABI_API_LNGSTRINGW( IDS_CHECK_FOR_UPDATES ) );
		EnableWindowDlgItem( hDlg, IDC_GET_UPDATE, TRUE );
	}
}

INT_PTR CALLBACK AboutFunc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_COMMAND) {
		switch(LOWORD(wParam))
		{
			case IDC_ABOUTLINK:
			{
				SendMessage(module.hwndParent, WM_WA_IPC, (WPARAM)"http://www.shoutcast.com/", IPC_OPEN_URL);
			}
			break;

			case IDC_HELPLINK:
			{
				// look for a lcoal copy of the documentation otherwise then do as before and use the wiki verison
				wchar_t path[MAX_PATH] = {0};
				StringCchPrintfW(path, ARRAYSIZE(path), L"%s\\SHOUTcast Source DSP\\Source_DSP_Plug-in.html", GetPluginDirectoryW(module.hwndParent));
				if(ShellExecuteW(module.hwndParent, L"open", path, NULL, NULL, SW_SHOWNORMAL) < (HINSTANCE)32)
				{
					SendMessage(module.hwndParent, WM_WA_IPC, (WPARAM)L"http://wiki.shoutcast.com/wiki/Source_DSP_Plug-in", IPC_OPEN_URL);
				}
			}
			break;

			case IDC_FORUMLINK:
			{
				SendMessage(module.hwndParent, WM_WA_IPC, (WPARAM)L"http://forums.shoutcast.com/forumdisplay.php?f=140", IPC_OPEN_URL);
			}
			break;

			case IDC_UPDATELINK:
			{
				SendMessage(module.hwndParent, WM_WA_IPC, (WPARAM)DOWNLOAD_URL, IPC_OPEN_URL);
			}
			break;

			case IDC_ABOUT_ICON:
			{
				if (HIWORD(wParam) == STN_DBLCLK) {
					static bool toggle;
					SendDlgItemMessage(hDlg, IDC_ABOUT_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)GetICYIcon(!toggle));
					SendMessage(hMainDLG, WM_SETICON, ICON_SMALL, (LPARAM)GetICYIcon(!toggle));
					toggle = !toggle;
				}
			}
			break;

			case IDC_GET_UPDATE:
			{
				EnableWindowDlgItem(hDlg, IDC_GET_UPDATE, FALSE);
				SetDlgItemTextW(hDlg, IDC_GET_UPDATE, WASABI_API_LNGSTRINGW(IDS_CHECKING_FOR_UPDATES));
				CheckVersion(hDlg);
			}
			break;
		}
	}
	LRESULT ret = CallWindowProcW((WNDPROC)DialogFunc, hDlg, uMsg, wParam, lParam);
	link_handledraw(hDlg, uMsg, wParam, lParam);
	return ret;
}

void UpdateTitleControls(void) {
	MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
	if (Out->Config.doTitleUpdate) {
		int length = (GetWindowTextLength(GetDlgItem(out_wnd[3].hWnd, IDC_TITLE)) > 0);
		int sc2mode = (LOBYTE(Output[Connection_CurSelPos].Config.protocol) == 1);

		EnableWindowDlgItem(out_wnd[3].hWnd, IDC_SENDNEXTTITLES, Out->AutoTitle && !sc2mode);

		if (Out->AutoTitle) {
			EnableWindowDlgItem(out_wnd[3].hWnd, IDC_SEND, FALSE);
			EnableWindowDlgItem(out_wnd[3].hWnd, IDC_TITLE, FALSE);
			EnableWindowDlgItem(out_wnd[3].hWnd, IDC_NEXT, FALSE);
		} else {
			EnableWindowDlgItem(out_wnd[3].hWnd, IDC_SEND, length);
			EnableWindowDlgItem(out_wnd[3].hWnd, IDC_TITLE, TRUE);
			EnableWindowDlgItem(out_wnd[3].hWnd, IDC_NEXT, !sc2mode && length);
		}
	} else {
		EnableWindowDlgItem(out_wnd[3].hWnd, IDC_SEND, FALSE);
		EnableWindowDlgItem(out_wnd[3].hWnd, IDC_TITLE, FALSE);
		EnableWindowDlgItem(out_wnd[3].hWnd, IDC_NEXT, FALSE);
		EnableWindowDlgItem(out_wnd[3].hWnd, IDC_SENDNEXTTITLES, FALSE);
	}
}

void LockOptionControls(BOOL unlock) {
	int protocol = LOBYTE(Output[Connection_CurSelPos].Config.protocol),
		automatic = (HIBYTE(Output[Connection_CurSelPos].Config.protocol) == 1);
	BOOL sc1 = (protocol == 1);

	// First Connection panel
	EnableWindowDlgItem(out_wnd[0].hWnd, IDC_ADDRESS, unlock);
	EnableWindowDlgItem(out_wnd[0].hWnd, IDC_PORT, unlock);
	// ensure the SC2 items are enabled only if SC1 mode is disabled
	EnableWindowDlgItem(out_wnd[0].hWnd, IDC_STATIONID, (sc1 ? FALSE : unlock));

	EnableWindowDlgItem(out_wnd[0].hWnd, IDC_USERID, unlock);
	EnableWindowDlgItem(out_wnd[0].hWnd, IDC_PASSWORD, unlock);
	EnableWindowDlgItem(out_wnd[0].hWnd, IDC_RECONNECT, unlock);
	EnableWindowDlgItem(out_wnd[0].hWnd, IDC_TIMEOUT, unlock);
	EnableWindowDlgItem(out_wnd[0].hWnd, IDC_PROTOCOL, unlock);

	// controls the information shown on the connection panel
	//ShowWindowDlgItem(out_wnd[0].hWnd, IDC_INFO_FRAME2, !sc1);
	ShowWindowDlgItem(out_wnd[0].hWnd, IDC_INFO_TEXT2, (protocol == 2) && !automatic);
	ShowWindowDlgItem(out_wnd[0].hWnd, IDC_INFO_TEXT3, (protocol == 1) && !automatic);
	ShowWindowDlgItem(out_wnd[0].hWnd, IDC_INFO_TEXT4, automatic);

	// artwork panel controls
	ShowWindowDlgItem(out_wnd[4].hWnd, IDC_USE_ART, !sc1);
	ShowWindowDlgItem(out_wnd[4].hWnd, IDC_USE_ART_PLAYING, !sc1);
	ShowWindowDlgItem(out_wnd[4].hWnd, IDC_USE_ART_STREAM, !sc1);
	ShowWindowDlgItem(out_wnd[4].hWnd, IDC_ART_EDIT, !sc1);
	ShowWindowDlgItem(out_wnd[4].hWnd, IDC_ART_BROWSE, !sc1);
	ShowWindowDlgItem(out_wnd[4].hWnd, IDC_ART_V1_NOTE, sc1);
	ShowWindowDlgItem(out_wnd[4].hWnd, IDC_ARTWORK_V1_FRAME, sc1);
	ShowWindowDlgItem(out_wnd[4].hWnd, IDC_ART_V2_NOTE, !sc1);
	ShowWindowDlgItem(out_wnd[4].hWnd, IDC_ARTWORK_V2_FRAME, !sc1);

	// Second Connection panel
	EnableWindowDlgItem(out_wnd[1].hWnd, IDC_PUBLIC, unlock);
	EnableWindowDlgItem(out_wnd[1].hWnd, IDC_DESCRIPTION, unlock);
	EnableWindowDlgItem(out_wnd[1].hWnd, IDC_SERVERURL, unlock);
	EnableWindowDlgItem(out_wnd[1].hWnd, IDC_GENRE, FALSE);
	EnableWindowDlgItem(out_wnd[1].hWnd, IDC_GENRES, unlock);
	// ensure the SC1 items are enabled only if SC1 mode is enabled
	EnableWindowDlgItem(out_wnd[1].hWnd, IDC_AIM, (!sc1 ? FALSE : unlock));
	EnableWindowDlgItem(out_wnd[1].hWnd, IDC_ICQ, (!sc1 ? FALSE : unlock));
	EnableWindowDlgItem(out_wnd[1].hWnd, IDC_IRC, (!sc1 ? FALSE : unlock));

	// Encoder tab
	EnableWindowDlgItem(out_wnd[2].hWnd, IDC_ENCTYPE, unlock);
	EnableWindowDlgItem(out_wnd[2].hWnd, IDC_ENCSETTINGS_BUTTON, unlock);
	EnableWindowDlgItem(out_wnd[2].hWnd, IDC_ENCSETTINGS, unlock);
}

void WritePrivateProfileInt(LPCSTR lpKeyName, int value, LPCSTR lpAppName, LPCSTR lpFileName) {
	char tmp[64] = {0};
	StringCchPrintfA(tmp, ARRAYSIZE(tmp), "%u", value);
	WritePrivateProfileString((!lpAppName ? APP_Name : lpAppName), lpKeyName, tmp, (!lpFileName ? IniName : lpFileName));
}

wchar_t* GetFileMetaData(wchar_t* file, wchar_t* metadata, wchar_t* buffer, int buffer_len) {
extendedFileInfoStructW efs;
	efs.filename=file;
	efs.metadata=metadata;
	efs.ret=buffer;
	efs.retlen=buffer_len;
	SendMessage(module.hwndParent, WM_WA_IPC, (WPARAM)&efs, IPC_GET_EXTENDED_FILE_INFOW);
	return buffer;
}

void UpdateNextTracks(wchar_t* next, int pos, std::vector<int> &nextListIdx, std::vector<std::wstring> &nextList) {
	nextList.clear();
	nextListIdx.clear();

	int queued = (WASABI_API_QUEUEMGR ? WASABI_API_QUEUEMGR->GetNumberOfQueuedItems() : 0);
	if (WASABI_API_QUEUEMGR && queued) {
		for (int i = 0; i < queued && (lookAhead == -1 ? 1 : i < lookAhead); i++) {
			int idx = WASABI_API_QUEUEMGR->GetQueuedItemFromIndex(i);
			nextList.push_back((wchar_t*)SendMessage(module.hwndParent, WM_WA_IPC, idx, IPC_GETPLAYLISTTITLEW));
			nextListIdx.push_back(idx);
		}
	} else {
		if (next[0]) {
			nextList.push_back(next);
			nextListIdx.push_back(pos);
		}
	}
}

int GetNextTracks(int len, int pos, wchar_t* next) {
	int nextpos = SendMessage(module.hwndParent, WM_WA_IPC, 0, IPC_GETNEXTLISTPOS);

	// attempt to use the new look ahead api in 5.61+ otherwise use existing
	// method or if the new api returns a failure just to cover all bases
	if (doNextLookAhead && nextpos != -1 && len >= 1) {
		wcscpy_s((wchar_t*)next, 1024, (wchar_t*)SendMessage(module.hwndParent, WM_WA_IPC, nextpos, IPC_GETPLAYLISTTITLEW));
	} else {
		// get the next title (as long as shuffle is off, etc)
		// with it mapped back to the first track as appropriate
		if (len >= 2 && !SendMessage(module.hwndParent, WM_WA_IPC, 0, IPC_GET_SHUFFLE)) {
			pos = (pos >= len ? 0 : pos);
			wcscpy_s((wchar_t*)next, 1024, (wchar_t*)SendMessage(module.hwndParent, WM_WA_IPC, pos, IPC_GETPLAYLISTTITLEW));
		}
	}
	return pos;
}

void FillNextTracks(int index, bool xml) {
	// on change then we refresh the file as applicable
	std::vector<std::wstring> nextList;
	std::vector<int> nextListIdx;
	wchar_t next[1024] = {0};
	int curpos = SendMessage(module.hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS);
	int len = SendMessage(module.hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH);
	int pos = curpos + 1;
	pos = GetNextTracks(len, pos, next);
	UpdateNextTracks(next, pos, nextListIdx, nextList);
	WriteNextTracks(index, module.hwndParent, nextListIdx, nextList, xml);
}

static ARGB32 *loadImgFromFile(const wchar_t *file, int *len)
{
	*len = 0;
	HANDLE hf = CreateFileW(file, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (hf != INVALID_HANDLE_VALUE)
	{
		*len = GetFileSize(hf, 0);
		if (WASABI_API_MEMMGR) {
			ARGB32* im = (ARGB32 *)WASABI_API_MEMMGR->sysMalloc(*len);
			if (im) {
				DWORD bytes_read;
				ReadFile(hf, im, *len, &bytes_read, 0);
				CloseHandle(hf);
				return im;
			}
		}
		CloseHandle(hf);
	}
	return (ARGB32 *)-1;
}

static wchar_t bytes[32], kilo[32], mega[32], giga[32], tera[32];
wchar_t* sizeStr(unsigned int size) {
	static wchar_t temp[256];
	if (GetWinampVersion(module.hwndParent) >= 0x5064) {
		// TODO swap over to the native Winamp version on newer clients
		return WASABI_API_LNG->FormattedSizeString(temp, ARRAYSIZE(temp), size);
	} else {
		if (!bytes[0]) {
			LocalisedString(IDS_B, bytes, ARRAYSIZE(bytes));
			LocalisedString(IDS_KIB, kilo, ARRAYSIZE(kilo));
			LocalisedString(IDS_MIB, mega, ARRAYSIZE(mega));
			LocalisedString(IDS_GIB, giga, ARRAYSIZE(giga));
		}

		if(size < 1024) {
			StringCchPrintfW(temp, ARRAYSIZE(temp), L"%u %s", size, bytes);
		} else if(size < 1048576) {
			StringCchPrintfW(temp, ARRAYSIZE(temp), L"%.02f %s", (size / 1024.0f), kilo);
		} else if(size < 1073741824) {
			StringCchPrintfW(temp, ARRAYSIZE(temp), L"%.02f %s", (size / 1048576.0f), mega);
		} else if(size < 1099511627776){
			StringCchPrintfW(temp, ARRAYSIZE(temp), L"%.02f %s", (size / 1073741824.0f), giga);
		} else{
			StringCchPrintfW(temp, ARRAYSIZE(temp), L"%.02f %s", (size / 1073741824.0f), tera);
		}
	}
	return temp;
}

void UpdateArtworkMessage() {
	MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
	SHOUTCAST_OUTPUT *Enc = Out->Encoder != -1 ? &Encoder[Out->Encoder] : NULL;
	wchar_t buf[1024], playing[256], stream[256];
	T_OUTPUT_INFO *Info = Enc->GetOutputInfo(Out->Handle);
	int streamSize = (Info ? Info->art_cached_length[0] : 0),
		playingSize = (Info ? Info->art_cached_length[1] : 0);

	StringCchPrintfW(stream, ARRAYSIZE(stream),
					 WASABI_API_LNGSTRINGW(!Out->useArt || !Out->useStreamArt ? IDS_DISABLED :
					 (streamSize == 0 ? IDS_EMPTY_ART : IDS_ENABLED_SIZE)),
					 sizeStr(streamSize));

	StringCchPrintfW(playing, ARRAYSIZE(playing),
					 WASABI_API_LNGSTRINGW(!Out->useArt || !Out->usePlayingArt ? IDS_DISABLED :
					 (playingSize == 0 ? IDS_EMPTY_ART : IDS_ENABLED_SIZE)),
					 sizeStr(playingSize));

	StringCchPrintfW(buf, ARRAYSIZE(buf), WASABI_API_LNGSTRINGW(IDS_V2_ARTWORK), stream, playing);
	SetDlgItemTextW(out_wnd[4].hWnd, IDC_ART_V2_NOTE, buf);
}

void UpdatePlayingAlbumArt(int Connection, int Index, bool usePlayingArt) {
	// do some quick checks as no need to send if not applicable to do so
	if (!usePlayingArt && !playingImage) {
		return;
	}

	// hard code the playing art to be sent in png format
	Encoder[Index].UpdateAlbumArtCache((usePlayingArt ? playingImage : 0),
									   (usePlayingArt ? playingLength : 0),
									   playingType, Connection);
	UpdateArtworkMessage();
}

bool UpdateStreamAlbumArt(int Connection, int Index, const wchar_t* stationArtPath, bool useStreamArt) {
	int artType = 0x0;
	bool update = false;

	// if not enabled and loaded then unload and refresh
	if (!useStreamArt && streamImage[Index] != (ARGB32 *)-1) {
		FreeStreamAlbumArt(Index);
		// bit of a fiddle to force a dummy update
		artType = 0x4000;
		update = true;
	} else if (useStreamArt && streamImage[Index] != (ARGB32 *)-1) {
		FreeStreamAlbumArt(Index);
	}

	// if enabled and not loaded then attempt to load
	if (!update && streamImage[Index] == (ARGB32 *)-1) {
		if (useStreamArt) {
			streamImage[Index] = loadImgFromFile(stationArtPath, &streamLength[Index]);

			wchar_t* ext = PathFindExtensionW(stationArtPath);
			if (ext) {
				if (*ext) ext++;

				update = true;
				if (!wcsnicmp(ext, L"jpeg", 4) || !wcsnicmp(ext, L"jpg", 3)) {
					artType = 0x4000;
				} else if (!wcsnicmp(ext, L"png", 3)) {
					artType = 0x4001;
				} else if (!wcsnicmp(ext, L"bmp", 3)) {
					artType = 0x4002;
				} else if (!wcsnicmp(ext, L"gif", 3)) {
					artType = 0x4003;
				} else {
					update = false;
				}
			}
		} else {
			// if not enabled and not loaded then do nothing
			UpdateArtworkMessage();
			return false;
		}
	}

	if (update) {
		Encoder[Index].UpdateAlbumArtCache((!useStreamArt || streamImage[Index] == (ARGB32 *)-1 ? 0 : streamImage[Index]),
										   (!useStreamArt ? 0: streamLength[Index]), artType, Connection);
	}
	UpdateArtworkMessage();
	return update;
}

void UpdateVUMeters()
{
	int volume = 0;
	int vu_l = VU.vu_l;
	int vu_r = VU.vu_r;
	VU.vu_l = 0;
	VU.vu_r = 0;
	VU.lastUpdate = VU.update;
	VU.update = 0;
	wchar_t tmp[256], temp[32], temp2[32];

	if (vu_l != 0) {
		volume = (int) (20 * log10((double) vu_l));
		StringCchPrintfW(tmp, ARRAYSIZE(tmp), LocalisedString(IDS_X_DB, temp, 32), volume - 90);
	} else {
		volume = 0;
		LocalisedString(IDS_INF_DB, tmp, 256);
	}
	if (volume - 90 > peak_vu_l) {
		StringCchPrintfW(temp2, ARRAYSIZE(temp2), LocalisedString(IDS_X_DB_PEAK, temp, 32), (peak_vu_l = volume - 90));
		SetDlgItemTextW(wnd[0].hWnd, IDC_VOLUMETEXT_LP, temp2);
		SetDlgItemTextW(wnd[2].hWnd, IDC_VOLUMETEXT_LP, temp2);
	}

	SetDlgItemTextW(wnd[0].hWnd, IDC_VOLUMETEXT_L, tmp);
	SendDlgItemMessage(wnd[0].hWnd, IDC_VOLUMEGRAPH_L, PBM_SETPOS, volume, 0);
	SetDlgItemTextW(wnd[2].hWnd, IDC_VOLUMETEXT_L, tmp);
	SendDlgItemMessage(wnd[2].hWnd, IDC_VOLUMEGRAPH_L, PBM_SETPOS, volume, 0);
	if (vu_r != 0) {
		volume = (int) (20 * log10((double) vu_r));
		StringCchPrintfW(tmp, ARRAYSIZE(tmp), LocalisedString(IDS_X_DB, temp, 32), volume - 90);
	} else {
		volume = 0;
		LocalisedString(IDS_INF_DB, tmp, 256);
	}
	if (volume - 90 > peak_vu_r) {
		StringCchPrintfW(temp2, ARRAYSIZE(temp2), LocalisedString(IDS_X_DB_PEAK, temp, 32), (peak_vu_r = volume - 90));
		SetDlgItemTextW(wnd[0].hWnd, IDC_VOLUMETEXT_RP, temp2);
		SetDlgItemTextW(wnd[2].hWnd, IDC_VOLUMETEXT_RP, temp2);
	}

	SetDlgItemTextW(wnd[0].hWnd, IDC_VOLUMETEXT_R, tmp);
	SendDlgItemMessage(wnd[0].hWnd, IDC_VOLUMEGRAPH_R, PBM_SETPOS, volume, 0);
	SetDlgItemTextW(wnd[2].hWnd, IDC_VOLUMETEXT_R, tmp);
	SendDlgItemMessage(wnd[2].hWnd, IDC_VOLUMEGRAPH_R, PBM_SETPOS, volume, 0);
}

void UpdateSummaryDetails(int item) {
	if (item == -1) {
		return;
	}

	wchar_t message[2048], temp[128], temp2[128], temp3[128],
			temp4[128] = {0}, temp5[128], temp6[128], temp7[128];
	char temp8[128];
	MY_T_OUTPUT *Out = &Output[item];
	C_ENCODER *Enc = Encoder[Out->Encoder].GetEncoder();

	if (Enc) {
		StringCchPrintfW(temp4, ARRAYSIZE(temp4), LocalisedString(IDS_SUMMARY_KBPS, NULL, 0), ((T_EncoderIOVals*) Enc->GetExtInfo())->output_bitRate);
	}

	StringCchPrintfW(message, ARRAYSIZE(message), LocalisedString(IDS_SUMMARY, NULL, 0),
					 // TODO show automatic mode ?
					 (LOBYTE(Out->Config.protocol) != 1 ? 2 : 1),
					 LocalisedString((Out->Config.Public ? IDS_PUBLIC : IDS_PRIVATE), temp, 128),
					 (Out->Config.Address[0] ? Out->Config.Address : LocalisedStringA(IDS_NOT_SET_SUMMARY, temp8, 128)),
					 Out->Config.Port,
					 LocalisedString((Out->Config.doTitleUpdate ? (Out->AutoTitle ? IDS_FOLLOW_WA : IDS_MANUAL) : IDS_DISABLED), temp2, 128),
					 (Enc_LastType[item] != 0 ? (!strcmp(Encoder[Out->Encoder].GetEncoder()->GetContentType(),"audio/mpeg")?L"MP3":L"AAC+") : LocalisedString(IDS_NOT_SET, temp3, 128)),
					 temp4,
					 LocalisedString((Out->Logging ? IDS_YES : IDS_NO), temp5, 128),
					 LocalisedString((Out->AutoConnect ? IDS_YES : IDS_NO), temp6, 128),
					 LocalisedString((Out->saveEncoded ? IDS_YES : IDS_NO), temp7, 128));

	SetDlgItemTextW(wnd[0].hWnd, IDC_SUMMARY, message);

	// see if we're looking at a incomplete setup and flag up the output tab as applicable
	if (IsWindow(tabWnd)) {
		RECT r = {0};
		TabCtrl_GetItemRect(tabWnd, 1, &r);
		if ((lastMode[item] >= 3 && lastMode[item] <= 6)) {
			SetTabErrorText(tabWnd, 1, 0, &r);
		} else {
			InvalidateRect(tabWnd, &r, 0);
		}
	}
}

void ProcessPlayingStatusUpdate(void) {
	if (isplaying == 1 && SendMessage(module.hwndParent, WM_WA_IPC, 0, IPC_GETOUTPUTTIME)) {
		if (was_paused) {
			was_paused = 0;
			play_diff = GetTickCount();
		}
	} else if (isplaying == 3) {
		// pause
		was_paused = 1;
	} else if (isplaying == 0 && was_playing == 1) {
		// stop
		was_playing = was_paused = 0;
	} else if (isplaying == 0 && was_playing == 0) {
		// non-playing track advance
		PostMessage(hMainDLG, WM_USER, 0, nowPlayingID);
	}
}

// check against a list of known "illegal" names as well as not allowing
// the creation of a station with the name the same as a genre string
bool stationNameAllowed(char* name){
	if(name && *name)
	{
		int checked = 0;
		for (int i = 0; i < ARRAYSIZE(genres); i++) {
			checked += !lstrcmpi(name, genres[i].name);
		}

		// check for punctuation only titles with double-processing
		// to strip out alphanum+space and space in second and then
		// we compare a difference in lengths where different is ok
		if (lstrlen(name) > 0) {
			char* stripped = (char*)malloc(lstrlen(name)+1);
			char* stripped2 = (char*)malloc(lstrlen(name)+1);
			stripped[0] = 0;
			for(int i = 0, j = 0; i < lstrlen(name); i++) {
				if(!isalnum(name[i]) && name[i] != ' ') {
					stripped[j] = name[i];
					stripped[++j] = 0;
				}
			}
			for(int i = 0, j = 0; i < lstrlen(name); i++) {
				if(name[i] != ' ') {
					stripped2[j] = name[i];
					stripped2[++j] = 0;
				}
			}
			checked += !(lstrlen(stripped2) > lstrlen(stripped));
			free(stripped);
			free(stripped2);
		}

		char* invalidNames[] = {"127.0.0.1", "admin", "auto dj", "auto jedi", "auto pj",
								"auto-dj", "autodj", "autopj", "demo", "dj", "internet radio",
								"live", "local server", "localhost", "localserver", "music",
								"my radio", "my server", "my station name", "my test server",
								"n/a", "pj", "playlist", "radio", "radio station", "test",
								"test server", "unnamed server", "virtual dj", "virtualdj",
								"web rdio", "web radio", "song", "teste", "default stream",
								"radio stream", "whmsonic autodj", "autopilot",
								"this is my server name"};
		for(int i = 0; i < ARRAYSIZE(invalidNames); i++)
		{
			checked += !lstrcmpi(name, invalidNames[i]);
		}
		return (!checked);
	}
	return false;
}

HBITMAP WAResizeImage(HBITMAP hbmp, INT cx, INT cy) {
	BITMAP bi = {0};
	if (!hbmp || !GetObjectW(hbmp, sizeof(BITMAP), &bi)) return hbmp;

	if (bi.bmWidth != cx || bi.bmHeight != cy) {
		HDC hdc, hdcDst, hdcSrc;
		HBITMAP hbmpOld1, hbmpOld2;

		int ix = cy*(bi.bmWidth*1000/bi.bmHeight)/1000, iy;
		if (ix > cx) { 
			iy = cx*(bi.bmHeight*1000/bi.bmWidth)/1000; 
			ix = cx;
		}
		else iy = cy;

		hdc = GetDC(NULL);
		hdcSrc = CreateCompatibleDC(hdc);
		hdcDst = CreateCompatibleDC(hdc);
		hbmpOld1 = (HBITMAP)SelectObject(hdcSrc, hbmp);
		hbmp = CreateCompatibleBitmap(hdc, cx, cy);
		hbmpOld2 = (HBITMAP)SelectObject(hdcDst, hbmp);
		if (ix != cx || iy != cy) {
			RECT r;
			SetRect(&r, 0, 0, cx, cy);
			FillRect(hdcDst, &r, GetSysColorBrush(COLOR_BTNFACE));
		}
		SetStretchBltMode(hdcDst, HALFTONE);
		StretchBlt(hdcDst, (cx - ix)/2, (cy - iy)/2, ix, iy, hdcSrc, 0, 0, bi.bmWidth, bi.bmHeight, SRCCOPY);

		SelectObject(hdcDst, hbmpOld2);
		hbmpOld2 = (HBITMAP)SelectObject(hdcSrc, hbmpOld1);
		if (hbmpOld2) DeleteObject(hbmpOld2);

		DeleteDC(hdcSrc);
		DeleteDC(hdcDst);
		ReleaseDC(NULL, hdc);
	}
	return hbmp;
}

HBITMAP getBitmap(ARGB32 * data, int w, int h)
{
	BITMAPINFO info={0};
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = w;
	info.bmiHeader.biHeight = -h;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;
	HDC dc = GetDC(NULL);
	HBITMAP bm = CreateCompatibleBitmap(dc, w, h);
	SetDIBits(dc, bm, 0, h, data, &info, DIB_RGB_COLORS);
	ReleaseDC(NULL, dc);
	return WAResizeImage(bm, 155, 88);
}

ARGB32 * decompressImage(const void *data, int datalen, int * dataW, int * dataH) {
	ARGB32* ret=NULL;
	FOURCC imgload = svc_imageLoader::getServiceType();
	int n = WASABI_API_SVC->service_getNumServices(imgload);
	for (int i = 0; i < n; i++) {
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgload,i);
		if (sf) {
			svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
			if (l) {
				if (l->testData(data,datalen)) {
					ret = l->loadImage(data,datalen,dataW,dataH);
					sf->releaseInterface(l);
					break;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return ret;
}

INT_PTR CALLBACK DialogFunc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (SYSTRAY_BASE_MSG + SYSTRAY_MAXIMIZE_MSG == uMsg) {
		int which = LOWORD(wParam) - SYSTRAY_BASE_ICON;
		switch (LOWORD(lParam)) {
			case WM_LBUTTONDOWN:
				{
					switch (which) {
						case 1:
						{
							RemoveSystrayIcon(hDlg, SYSTRAY_ICY_ICON);
							ShowWindow(hDlg, SW_SHOW);
							SendMessage(hDlg, WM_SYSCOMMAND, SC_RESTORE, 0);
						}
						break;
					}
				}
				break;
		}
	}
	switch (uMsg) {
#ifdef FOLLOW_MIXER
		case MM_MIXM_CONTROL_CHANGE:
		{
			HMIXER mix = (HMIXER)wParam;
			DWORD dwControlID = (DWORD)lParam;
			//if (dwControlID == 3)
			{
				MIXERLINE ml = {sizeof (ml), 0};
				ml.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_LINE;
				ml.dwLineID = dwControlID;
				if (mixerGetLineInfo((HMIXEROBJ) mix, &ml, MIXER_GETLINEINFOF_COMPONENTTYPE|MIXER_OBJECTF_MIXER) == MMSYSERR_NOERROR) {
					MIXERLINECONTROLS mlc = {sizeof (mlc), dwControlID,};
					MIXERCONTROL mc = {sizeof (mc),};
					//mlc.cControls = 1;
					mlc.cbmxctrl = sizeof (mc);
					mlc.pamxctrl = &mc;
					mlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
					//mlc.dwControlID = dwControlID;
					mlc.dwLineID = dwControlID;//ml.dwLineID;
			char a[128];
			sprintf(a,"MM_MIXM_CONTROL_CHANGE: %d\n",dwControlID);
			OutputDebugString(a);
					if (mixerGetLineControls((HMIXEROBJ) mix, &mlc, MIXER_GETLINECONTROLSF_ONEBYTYPE|MIXER_OBJECTF_MIXER) == MMSYSERR_NOERROR) {
						//MIXERCONTROLDETAILS mcd = {sizeof (mcd), 0,};
			char a[128];
			sprintf(a,"MM_MIXM_CONTROL_CHANGE 2: %d\n",dwControlID);
			OutputDebugString(a);
						/*MIXERCONTROLDETAILS_UNSIGNED v[2];
						mcd.cbDetails = sizeof (MIXERCONTROLDETAILS_UNSIGNED);
						mcd.paDetails = v;
						/*v[0].dwValue = mc.Bounds.dwMinimum + (va * (mc.Bounds.dwMaximum - mc.Bounds.dwMinimum)) / 100;
						v[1].dwValue = mc.Bounds.dwMinimum + (va * (mc.Bounds.dwMaximum - mc.Bounds.dwMinimum)) / 100;
						MMRESULT result = mixerSetControlDetails((HMIXEROBJ) hmix, &mcd, MIXER_OBJECTF_MIXER);*/
					}
				}
			}
		}
		break;
#endif

		case WM_LBUTTONUP:
			GetWindowRect(hDlg, &mainrect);
			break;

		case WM_INITDIALOG:
			{
				switch (lParam) {
					case IDD_DIALOG:
					{
						int i;
						wchar_t temp[128] = {0};
						hMainDLG = hDlg;
						for (i = 0; i < NUM_ENCODERS; i++) Enc_mutex[i] = NULL;
						INITCOMMONCONTROLSEX ICCE;
						ICCE.dwSize = sizeof (ICCE);
						ICCE.dwICC = ICC_WIN95_CLASSES;
						InitCommonControlsEx(&ICCE);
						GetSCIniFile(module.hwndParent);
						VU.update = VU.lastUpdate = 0;
						memset(&VU, 0, sizeof (VU));
						memset(&Output, 0, sizeof (Output));

						// as only 5.61+ supports the IPC_GETNEXTLISTPOS api we check and cache version now
						doNextLookAhead = (GetWinampVersion(module.hwndParent) >= 0x5061);

						/* Load config */
						LoadConfig();

						/* Load Encoders */
						LoadEncoders();

						/* Start logging and encoded output saving */
						for (i = 0; i < NUM_OUTPUTS; i++) {
							if (Output[i].Logging) {
								StartLogging(i, Output[i].LogCOS);
							}
							if (Output[i].nextTrackLog) {
								StartNextTracks(i, Output[i].nextTrackPath);
							}
							if (Output[i].saveEncoded) {
								StartSaveEncoded(i, Output[i].saveEncodedPath);
							}
						}

						// ensure we've cached the title information, etc as needed before starting
						isplaying = SendMessage(module.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING);
						if (isplaying) ProcessPlayingStatusUpdate();
						PostMessage(hDlg, WM_USER, 0, nowPlayingID);

						/* Setup main tabs */
						num_tabwnds = 0;
						SendDlgItemMessage(hDlg, IDC_TAB, TCM_DELETEALLITEMS, 0, 0);
						AddTab(IDD_MAIN, LocalisedString(IDS_TAB_MAIN, temp, 128), hDlg, DialogFunc, IDC_TAB, IDC_RECT, 100);
						AddTab(IDD_CONNECTION, LocalisedString(IDS_TAB_OUTPUT, temp, 128), hDlg, ConnectionFunc, IDC_TAB, IDC_RECT, 100);
						AddTab(IDD_INPUT, LocalisedString(IDS_TAB_INPUT, temp, 128), hDlg, DialogFunc, IDC_TAB, IDC_RECT, 100);
						AddTab(IDD_ABOUT, LocalisedString(IDS_TAB_ABOUT, temp, 128), hDlg, AboutFunc, IDC_TAB, IDC_RECT, 100);
						SetTab(curtab, hDlg, IDC_TAB);

						/* threads and timer */
						hthread = CreateThread(NULL, 0, ThreadInput, hDlg, CREATE_SUSPENDED, &threadid);
						SetThreadPriority(hthread, THREAD_PRIORITY_HIGHEST);
						hthreadout = CreateThread(NULL, 0, ThreadOutput, hDlg, CREATE_SUSPENDED, &threadoutid);
						SetThreadPriority(hthreadout, THREAD_PRIORITY_HIGHEST);
						Soundcard.Close();
						Soundcard.Create((InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate), (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch));
						SetTimer(hDlg, 666, 300000, NULL); // start up INI save timer
						FadeStartTime = 0;
						MicFadeStartTime = 0;
						FadeOut = 0;
						ini_modified = 1;
						ReleaseMutex(cf_mutex);
						SendDlgItemMessage(wnd[1].hWnd, IDC_AUTOCONNECT, BM_SETCHECK,Output[Connection_CurSelPos].AutoConnect ? BST_CHECKED : BST_UNCHECKED, 0);

						// deal with the PTT restore mode along with setting up the 'down arrow' button
						SendDlgItemMessage(in_wnd[1].hWnd, IDC_LOCK, BM_SETCHECK, Restore_PTT ? BST_CHECKED : BST_UNCHECKED, 0);
						PostMessage(in_wnd[1].hWnd, WM_COMMAND, MAKEWPARAM(IDC_LOCK, BN_CLICKED), (LPARAM)GetDlgItem(in_wnd[1].hWnd, IDC_LOCK));

						SendDlgItemMessage(in_wnd[1].hWnd, IDC_LOCK_MODE, BM_SETIMAGE, IMAGE_ICON,
										   (LPARAM)LoadImage(module.hDllInstance, MAKEINTRESOURCE(IDI_DOWNARROW),
															 IMAGE_ICON, 0, 0, LR_SHARED | LR_LOADTRANSPARENT | LR_CREATEDIBSECTION));

						SendDlgItemMessage(in_wnd[1].hWnd, IDC_REFRESH_DEVICES, BM_SETIMAGE, IMAGE_ICON,
										   (LPARAM)LoadImage(module.hDllInstance, MAKEINTRESOURCE(IDI_REFRESH),
															 IMAGE_ICON, 0, 0, LR_SHARED | LR_LOADTRANSPARENT | LR_CREATEDIBSECTION));

						/* setup information parts to be in bold */
						SetBoldDialogItemFont(GetDlgItem(wnd[0].hWnd, IDC_SUMMARY));
						SetBoldDialogItemFont(GetDlgItem(out_wnd[1].hWnd, IDC_INFO_TEXT));
						SetBoldDialogItemFont(GetDlgItem(out_wnd[0].hWnd, IDC_INFO_TEXT2));
						SetBoldDialogItemFont(GetDlgItem(out_wnd[0].hWnd, IDC_INFO_TEXT3));
						SetBoldDialogItemFont(GetDlgItem(out_wnd[0].hWnd, IDC_INFO_TEXT4));
						SetBoldDialogItemFont(GetDlgItem(wnd[1].hWnd, IDC_CONNECT));
						SetBoldDialogItemFont(GetDlgItem(out_wnd[4].hWnd, IDC_ART_V1_NOTE));
						SetBoldDialogItemFont(GetDlgItem(out_wnd[4].hWnd, IDC_ART_V2_NOTE));

						/* subclass the listview on the summary page for specific handling */
						HWND listView = GetDlgItem(wnd[0].hWnd, IDC_OUTPUTSTATUS);
						ListView_SetItemState(listView, 0, LVIS_SELECTED, LVIS_SELECTED);
						prevListViewProc = (WNDPROC) SetWindowLongPtrW(listView, GWLP_WNDPROC, (LONG_PTR) listViewProc);
						prevHeaderProc = (WNDPROC) SetWindowLongPtrW(ListView_GetHeader(listView), GWLP_WNDPROC, (LONG_PTR) headerProc);

						/* subclass the tab control on the main dialog for error notifications */
						tabWnd = GetDlgItem(hMainDLG, IDC_TAB);
						prevTabWndProc = (WNDPROC) SetWindowLongPtrW(tabWnd, GWLP_WNDPROC, (LONG_PTR) tabWndProc);

						/* subclass the tab control on the connection page for error notifications */
						outTabWnd = GetDlgItem(wnd[1].hWnd, IDC_CONTAB);
						prevOutTabWndProc = (WNDPROC) SetWindowLongPtrW(outTabWnd, GWLP_WNDPROC, (LONG_PTR) outTabWndProc);

						/* only once everything else has been done then we start the threads */
						ResumeThread(hthread);
						ResumeThread(hthreadout);
						break;
					}

					case IDD_MAIN:
					{
						HWND listView = GetDlgItem(hDlg, IDC_OUTPUTSTATUS);
						ListView_DeleteAllItems(listView);
						ListView_SetExtendedListViewStyle(listView, LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
						AddColumn(L"", listView);
						AddColumn(LocalisedString(IDS_COL_OUTPUT_NAME, NULL, 0), listView);
						AddColumn(LocalisedString(IDS_COL_STATUS, NULL, 0), listView);
						for (int i = 0; i < NUM_OUTPUTS; i++) {
							AddColItem(L"", 0, hDlg, IDC_OUTPUTSTATUS);
							AddColItem(Output[i].Config.DisplayName, 1, hDlg, IDC_OUTPUTSTATUS);
							AddColItem(L"", 2, hDlg, IDC_OUTPUTSTATUS);
						}

						// fill the header of the output list with the column headers
						ListView_SetColumnWidth(listView, 0, 24);//LVSCW_AUTOSIZE_USEHEADER);
						ListView_SetColumnWidth(listView, 1, LVSCW_AUTOSIZE_USEHEADER);
						int width = ListView_GetColumnWidth(listView, 0) + ListView_GetColumnWidth(listView, 1);
						RECT listViewRect;
						GetClientRect(listView, &listViewRect);
						ListView_SetColumnWidth(listView, 2, (listViewRect.right - listViewRect.left) - width);

						// add in status / action buttons for the first column...
						for (int i = 0; i < NUM_OUTPUTS; i++) {
							POINT pt;
							ListView_GetItemPosition(listView, i, &pt);
							ListView_GetItemRect(listView, i, &listViewRect, LVIR_BOUNDS);
							buttonWnd[i] = CreateWindowW(L"button", L"", WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_DISABLED | BS_ICON,
													    1, pt.y, ListView_GetColumnWidth(listView, 0) - 3,
													    (listViewRect.bottom - listViewRect.top),
													    listView, (HMENU)(IDC_STREAM_1+i), 0, 0);
							SendMessage(buttonWnd[i], BM_SETIMAGE, IMAGE_ICON,
													  (LPARAM)LoadImage(module.hDllInstance, MAKEINTRESOURCE(IDI_PLAY),
																		IMAGE_ICON, 0, 0, LR_SHARED | LR_LOADTRANSPARENT | LR_CREATEDIBSECTION));
						}

						CheckRadioButton(hDlg, IDC_INPUT_WINAMP, IDC_INPUT_SOUNDCARD, (IDC_INPUT_WINAMP + InputDevice));
						SendDlgItemMessage(hDlg, IDC_VOLUMEGRAPH_L, PBM_SETRANGE, 0, MAKELONG(0, 90));
						SendDlgItemMessage(hDlg, IDC_VOLUMEGRAPH_R, PBM_SETRANGE, 0, MAKELONG(0, 90));
						for (int ii = 0; ii < NUM_OUTPUTS; ii++) {
							int encnum = ii;
							MY_T_OUTPUT *Out = &Output[ii];
							// removed encoder selection
							if (Out->Encoder != -1) {
								if (Out->Handle != -1) {
									if (WaitForSingleObject(Enc_mutex[Out->Encoder], INFINITE) == WAIT_OBJECT_0) {
										Encoder[Out->Encoder].RemoveOutput(Out->Handle);
										ReleaseMutex(Enc_mutex[Out->Encoder]);
									}
								}
							}

							Out->Encoder = encnum;
							if (Out->Encoder != -1) {
								if (WaitForSingleObject(Enc_mutex[Out->Encoder], INFINITE) == WAIT_OBJECT_0) {
									Out->Handle = Encoder[Out->Encoder].AddOutput(Out->Encoder, &Out->Config, TitleCallback);
									ReleaseMutex(Enc_mutex[Out->Encoder]);
								}
								ini_modified = 1;
							}
							// TODO
							//SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_NOTITLES, BN_CLICKED), (LPARAM) GetDlgItem(hDlg, IDC_NOTITLES));
						}
						break;
					}

					case IDD_INPUT:
					{
						SendDlgItemMessage(hDlg, IDC_INPUTDEVICE, CB_RESETCONTENT, 0, 0);
						AddInTab(IDD_PANEL_WINAMP, LocalisedString(IDS_INPUT_WINAMP, NULL, 0), hDlg);
						AddInTab(IDD_PANEL_LINEIN, LocalisedString(IDS_INPUT_SOUNDCARD, NULL, 0), hDlg);
						SetInTab(InputDevice, hDlg, IDC_INPUTDEVICE);
						EnableWindowDlgItem(hDlg, IDC_INPUTDEVICE, 1);
						EnableWindowDlgItem(hDlg, IDC_INPUTDEVICESTATIC, 1);
						SendDlgItemMessage(hDlg, IDC_VOLUMEGRAPH_L, PBM_SETRANGE, 0, MAKELONG(0, 90));
						SendDlgItemMessage(hDlg, IDC_VOLUMEGRAPH_R, PBM_SETRANGE, 0, MAKELONG(0, 90));
						break;
					}

					case IDD_ABOUT:
					{
						// this will make sure that we've got the icon shown even when using a localised version
						// as well as setting the dialog icon to the icy icon irrespective of the dialog class's
						SendDlgItemMessage(hDlg, IDC_ABOUT_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)GetICYIcon());
						SendMessage(hMainDLG, WM_SETICON, ICON_SMALL, (LPARAM)GetICYIcon());

						wchar_t about[1024], tmp[256];
						StringCchPrintfW(about, ARRAYSIZE(about), LocalisedString(IDS_ABOUT_MESSAGE, NULL, 0),
										 LocalisedString(IDS_MODULE_NAME, tmp, 256),
										 APP_Version, APP_Build, __DATE__);
						SetDlgItemTextW(hDlg, IDC_PROGRAMNAME, about);
						link_startsubclass(hDlg, IDC_ABOUTLINK);
						link_startsubclass(hDlg, IDC_HELPLINK);
						link_startsubclass(hDlg, IDC_FORUMLINK);
						link_startsubclass(hDlg, IDC_UPDATELINK);

						ShowUpdateMessage(hDlg);
						break;
					}

					case IDD_PANEL_WINAMP:
					{
						wchar_t buf[128] = {0};
						inWinWa = hDlg;
						HWND metalist = GetDlgItem(hDlg, IDC_METALIST);
						ListView_SetExtendedListViewStyle(metalist, LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
						AddColumn(L"", metalist);
						AddColumn(L"", metalist);
						AddColItem(LocalisedString(IDS_FILEPATH, buf, ARRAYSIZE(buf)), 0, hDlg, IDC_METALIST);
						AddColItem(LocalisedString(IDS_TITLE, buf, ARRAYSIZE(buf)), 0, hDlg, IDC_METALIST);
						AddColItem(LocalisedString(IDS_ARTIST, buf, ARRAYSIZE(buf)), 0, hDlg, IDC_METALIST);
						AddColItem(LocalisedString(IDS_ALBUM, buf, ARRAYSIZE(buf)), 0, hDlg, IDC_METALIST);
						AddColItem(LocalisedString(IDS_GENRE, buf, ARRAYSIZE(buf)), 0, hDlg, IDC_METALIST);
						AddColItem(LocalisedString(IDS_YEAR, buf, ARRAYSIZE(buf)), 0, hDlg, IDC_METALIST);
						AddColItem(LocalisedString(IDS_COMMENT, buf, ARRAYSIZE(buf)), 0, hDlg, IDC_METALIST);
						ListView_SetColumnWidth(metalist, 0, LVSCW_AUTOSIZE);
						RECT metalistRect;
						GetClientRect(metalist, &metalistRect);
						ListView_SetColumnWidth(metalist, 1, (metalistRect.right - metalistRect.left) - ListView_GetColumnWidth(metalist, 0));
						break;
					}

					case IDD_PANEL_LINEIN:
					{
						inWin = NULL;
     					prevButtonProc = (WNDPROC) SetWindowLongPtrW(GetDlgItem(hDlg, IDC_PTT), GWLP_WNDPROC, (LONG_PTR) buttonProc);
						SendDlgItemMessage(hDlg, IDC_MUSSLIDER, TBM_SETRANGE, TRUE, MAKELONG(0, 10));
						SendDlgItemMessage(hDlg, IDC_MUS2SLIDER, TBM_SETRANGE, TRUE, MAKELONG(0, 10));
						SendDlgItemMessage(hDlg, IDC_MICSLIDER, TBM_SETRANGE, TRUE, MAKELONG(0, 10));
						SendDlgItemMessage(hDlg, IDC_FADESLIDER, TBM_SETRANGE, TRUE, MAKELONG(0, 25));
						SendDlgItemMessage(hDlg, IDC_MICFADESLIDER, TBM_SETRANGE, TRUE, MAKELONG(0, 25));
						SendDlgItemMessage(hDlg, IDC_MUSSLIDER, TBM_SETPOS, TRUE, MusVol);
						SendDlgItemMessage(hDlg, IDC_MUS2SLIDER, TBM_SETPOS, TRUE, Mus2Vol);
						SendDlgItemMessage(hDlg, IDC_MICSLIDER, TBM_SETPOS, TRUE, MicVol);
						SendDlgItemMessage(hDlg, IDC_FADESLIDER, TBM_SETPOS, TRUE, FadeTime);
						SendDlgItemMessage(hDlg, IDC_MICFADESLIDER, TBM_SETPOS, TRUE, MicFadeTime);
						SendMessage(hDlg, WM_HSCROLL, 0, (LPARAM) GetDlgItem(hDlg, IDC_MUSSLIDER));
						SendMessage(hDlg, WM_HSCROLL, 0, (LPARAM) GetDlgItem(hDlg, IDC_MUS2SLIDER));
						SendMessage(hDlg, WM_HSCROLL, 0, (LPARAM) GetDlgItem(hDlg, IDC_MICSLIDER));
						SendMessage(hDlg, WM_HSCROLL, 0, (LPARAM) GetDlgItem(hDlg, IDC_FADESLIDER));
						SendMessage(hDlg, WM_HSCROLL, 0, (LPARAM) GetDlgItem(hDlg, IDC_MICFADESLIDER));
						inWin = hDlg;
						SetDeviceName();
						SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_DEVBOX, CBN_SELCHANGE), (LPARAM)GetDlgItem(hDlg,IDC_DEVBOX));
						break;
					}

					case IDD_PANEL_LOGIN:
					{
						SendDlgItemMessageW(hDlg, IDC_PROTOCOL, CB_ADDSTRING, 0,(LPARAM)LocalisedString(IDS_AUTOMATIC, NULL, 0));
						SendDlgItemMessage(hDlg, IDC_PROTOCOL, CB_SETITEMDATA, 0,(LPARAM)0);
						SendDlgItemMessageW(hDlg, IDC_PROTOCOL, CB_ADDSTRING, 0,(LPARAM)LocalisedString(IDS_V2_MODE, NULL, 0));
						SendDlgItemMessage(hDlg, IDC_PROTOCOL, CB_SETITEMDATA, 1,(LPARAM)2);
						SendDlgItemMessageW(hDlg, IDC_PROTOCOL, CB_ADDSTRING, 0,(LPARAM)LocalisedString(IDS_V1_MODE, NULL, 0));
						SendDlgItemMessage(hDlg, IDC_PROTOCOL, CB_SETITEMDATA, 2,(LPARAM)1);
						SendDlgItemMessage(hDlg, IDC_TIMEOUT, EM_SETLIMITTEXT, 5, 0);
						break;
					}

					case IDD_PANEL_DIRECTORY:
					{
						SendDlgItemMessage(hDlg, IDC_GENRES, BM_SETIMAGE, IMAGE_ICON,
										   (LPARAM)LoadImage(module.hDllInstance, MAKEINTRESOURCE(IDI_DOWNARROW),
															 IMAGE_ICON, 0, 0, LR_SHARED | LR_LOADTRANSPARENT | LR_CREATEDIBSECTION));
						break;
					}
				}
			}
			return 1;

		case WM_CLOSE:
			{
				if (hDlg == hMainDLG)
				{
					doQuit();
				}
			}
			break;

		case WM_USER:
			{
				if (lParam == nowPlayingID && wParam == 0) {
					// Winamp Title handling
					wchar_t title[1024] = {0},
							next[1024] = {0},
							song[1024] = {0},
							artist[1024] = {0},
							album[1024] = {0},
							genre[1024] = {0},
							comment[1024] = {0},
							year[32] = {0},
							tmp2[1024] = {0};
					char buffer[1024] = {0},
						 buffer2[1024] = {0};

					// winamp playlist length in tracks
					int len = SendMessage(module.hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH);
					int curpos = SendMessage(module.hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS);
					int pos = curpos + 1;
					int len2 = 0;

					// get the current title
					if (len >= 1) {
						wcscpy_s(lastFile, MAX_PATH, (wchar_t*)SendMessage(module.hwndParent, WM_WA_IPC, curpos, IPC_GETPLAYLISTFILEW));
						wcscpy_s(title, ARRAYSIZE(title), (wchar_t*)SendMessage(module.hwndParent, WM_WA_IPC, curpos, IPC_GETPLAYLISTTITLEW));
					} else {
						title[0] = lastFile[0] = 0;
					}

					// get the position of the next track if possible
					pos = GetNextTracks(len, pos, next);

					if (skipMetada == false) {
						AddColItem(lastFile, 1, inWinWa, IDC_METALIST, 0);
						GetFileMetaData(lastFile, L"title", song, ARRAYSIZE(song));
						AddColItem((song && song[0] ? song : title), 1, inWinWa, IDC_METALIST, 1);
						GetFileMetaData(lastFile, L"artist", artist, ARRAYSIZE(artist));
						AddColItem((artist[0] ? artist : L"<emtpy>"), 1, inWinWa, IDC_METALIST, 2);
						GetFileMetaData(lastFile, L"album", album, ARRAYSIZE(album));
						AddColItem((album[0] ? album : L"<emtpy>"), 1, inWinWa, IDC_METALIST, 3);
						GetFileMetaData(lastFile, L"genre", genre, ARRAYSIZE(genre));
						AddColItem((genre[0] ? genre : L"<emtpy>"), 1, inWinWa, IDC_METALIST, 4);
						GetFileMetaData(lastFile, L"year", year, ARRAYSIZE(year));
						AddColItem((year[0] ? year : L"<emtpy>"), 1, inWinWa, IDC_METALIST, 5);
						GetFileMetaData(lastFile, L"comment", comment, ARRAYSIZE(comment));
						AddColItem((comment[0] ? comment : L"<emtpy>"), 1, inWinWa, IDC_METALIST, 6);

						if (WASABI_API_MEMMGR && playingImage) {
							WASABI_API_MEMMGR->sysFree(playingImage); playingImage = 0;
						}

						playingLength = 0;
						playingType = 0x4100;	// default in-case of issue

						// attempt to get the type of the image, defaulting to jpeg for older versions
						// as only on 5.64+ are we able to query Winamp properly for the artwork type
						wchar_t* mimeType = 0, *uiType = L"jpeg";
						if (GetWinampVersion(module.hwndParent) >= 0x5064)
						{
							LPVOID bits;
							size_t len;
							if (AGAVE_API_ALBUMART && AGAVE_API_ALBUMART->GetAlbumArtData(lastFile, L"cover", &bits, &len, &mimeType) == ALBUMART_SUCCESS) {
								// make sure to free the original image after we've converted
								playingImage = (ARGB32 *)bits;
								playingLength = len;

								wchar_t *ext = &mimeType[0];
								if (ext && *ext) {
									uiType = ext;
									if (!wcsnicmp(ext, L"jpeg", 4) || !wcsnicmp(ext, L"jpg", 3)) {
										playingType = 0x4100;
									} else if (!wcsnicmp(ext, L"png", 3)) {
										playingType = 0x4101;
									} else if (!wcsnicmp(ext, L"bmp", 3)) {
										playingType = 0x4102;
									} else if (!wcsnicmp(ext, L"gif", 3)) {
										playingType = 0x4103;
									}
								}

								// update the current artwork on the winamp panel
								// have to decode as we get the raw data normally
								HWND artwork = GetDlgItem(inWinWa, IDC_ARTWORK);
								HBITMAP bm = getBitmap(decompressImage(playingImage, playingLength, &playingImage_w, &playingImage_h), playingImage_w, playingImage_h);
								HBITMAP bmold = (HBITMAP)SendMessage(artwork, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bm);
								if (bmold) DeleteObject(bmold);
								InvalidateRect(artwork, NULL, TRUE);
							}
						} else {
							playingImage_w = 0, playingImage_h = 0;
							if (AGAVE_API_ALBUMART && AGAVE_API_ALBUMART->GetAlbumArt(lastFile, L"cover", &playingImage_w, &playingImage_h, &playingImage) == ALBUMART_SUCCESS) {
								// make sure to free the original image after we've converted
								ARGB32 *firstPlayingImage = playingImage;

								// update the current artwork on the winamp panel
								// no need to decode as it's already done by this
								HWND artwork = GetDlgItem(inWinWa, IDC_ARTWORK);
								HBITMAP bm = getBitmap(playingImage, playingImage_w, playingImage_h);
								HBITMAP bmold = (HBITMAP)SendMessage(artwork, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bm);
								if (bmold) DeleteObject(bmold);
								InvalidateRect(artwork, NULL, TRUE);

								playingImage = writeImg(playingImage, playingImage_w, playingImage_h, &playingLength, L"jpeg");
								WASABI_API_MEMMGR->sysFree(firstPlayingImage);
							} else {
								playingImage = 0;
							}
						}

						wchar_t tmp3[1024] = {0};
						if(playingImage)
						{
							StringCchPrintfW(tmp3, ARRAYSIZE(tmp3), L"Playing Artwork: Width=%d; Height=%d; Data=%s;",
											 playingImage_w, playingImage_h, sizeStr(playingLength));

							// only use this on compatible Winamp installs where possible
							wchar_t *mime = 0;
							if (GetWinampVersion(module.hwndParent) >= 0x5064 &&
								AGAVE_API_ALBUMART && AGAVE_API_ALBUMART->GetAlbumArtOrigin(lastFile, L"cover", &mime)) {
								if (mime)
								{
									uiType = wcschr(mime, L'/');
									if (uiType && *uiType)
									{
										uiType++;
									}
								}
							} else {
								if (mimeType)
								{
									uiType = wcschr(mimeType, L'/');
									if (uiType && *uiType)
									{
										uiType++;
									}
								}
							}

							wchar_t buf[256] = {0};
							StringCchPrintfW(buf, ARRAYSIZE(buf), LocalisedString(IDS_ARTWORK_SIZES, NULL, 0),
											 playingImage_w, playingImage_h, sizeStr(playingLength),
											 (uiType && *uiType ? uiType : mime));
							SetDlgItemTextW(inWinWa, IDC_ARTWORK3, buf);
							WASABI_API_MEMMGR->sysFree(mime);
						}
						else
						{
							// update the current artwork on the winamp panel
							// by setting a generic image when nothing loaded
							HWND artwork = GetDlgItem(inWinWa, IDC_ARTWORK);
							HBITMAP bmold = (HBITMAP)SendMessage(artwork, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)0);
							if (bmold) DeleteObject(bmold);
							SetDlgItemTextW(inWinWa, IDC_ARTWORK3, L"");
							StringCchPrintfW(tmp3, ARRAYSIZE(tmp3), L"Playing Artwork: Cleared;");
						}
						if (mimeType) WASABI_API_MEMMGR->sysFree(mimeType);
						ShowWindowDlgItem(inWinWa, IDC_ARTWORK, (playingLength > 0));
						ShowWindowDlgItem(inWinWa, IDC_ARTWORK2, (playingLength == 0));
						ShowWindowDlgItem(inWinWa, IDC_ARTWORK3, (playingLength > 0));
						CreateLogFileMessage(buffer2, tmp3, &len2);
					}

					// look at the playback queue so we can get the correct 'next song'
					if (WASABI_API_SVC && !WASABI_API_QUEUEMGR) {
						// due to loading orders its possible the queue won't have been loaded on init so check
						ServiceBuild(WASABI_API_SVC, WASABI_API_QUEUEMGR, QueueManagerApiGUID);
					}

					std::vector<std::wstring> nextList;
					std::vector<int> nextListIdx;
					UpdateNextTracks(next, pos, nextListIdx, nextList);

					StringCchPrintfW(tmp2, ARRAYSIZE(tmp2), L"Metadata: Artist=%s; Album=%s; Genre=%s; Year=%s; Comment=%s; Title=%s;",
									 artist, album, genre, year, comment, (song && song[0] ? song : title));
					len = 0;
					CreateLogFileMessage(buffer, tmp2, &len);

					// update the title cache for all of the encoders otherwise we might fail, etc
					for (int i = 0; i < NUM_OUTPUTS; i++) {
						MY_T_OUTPUT *Out = &Output[i];

						if (Out->nextTrackLog) {
							WriteNextTracks(i, module.hwndParent, nextListIdx, nextList, !!Out->nextTrackLogXML);
						}

						if (WaitForSingleObject(Enc_mutex[Out->Encoder], INFINITE) == WAIT_OBJECT_0) {
							Encoder[i].UpdateTitleCache(title, nextList, song, album, artist, genre, comment, year, Out->Handle, !!Out->NextTitles);
							ReleaseMutex(Enc_mutex[Out->Encoder]);
						}

						// skip this all if the mode is not enabled and only in SC2 mode
						if ((LOBYTE(Out->Config.protocol) != 1) && Out->useArt) {
							// this will only update generally on first connection
							if (Out->useStreamArt && streamImage[i] == (ARGB32 *)-1) {
								UpdateStreamAlbumArt(Out->Handle, i, Out->stationArtPath, !!Out->useStreamArt);
							}

							// this will update against what is read from the playing
							if (Out->usePlayingArt) {
								UpdatePlayingAlbumArt(Out->Handle, i, Out->useArt && Out->usePlayingArt);
							}
						}

						// save the updated metadata to the log file (if enabled)
						if (Out->Logging && logFiles[i] != INVALID_HANDLE_VALUE) {
							DWORD written = 0;
							WriteFile(logFiles[i], buffer, len, &written, 0);
							WriteFile(logFiles[i], buffer2, len2, &written, 0);
						}
					}
				}
			}
			break;

		case WM_TIMER:
			{
				if (wParam == IDD_MAIN || wParam == IDD_INPUT) {
					if (VU.update || VU.update != VU.lastUpdate)
					{
						UpdateVUMeters();
					}
				}

				if (wParam == 1234) { // input timer
					if (InputDevice == 1) {
						for (int i = 0; i < NUM_BUFFERS; i++) {
							if (Soundcard.isFilled(last_buffer)) {
								short *buffer = Soundcard[last_buffer];
								int scardsmps = Soundcard.getNumSamples(last_buffer);
								int numsamps = scardsmps * InputConfig.nch;
								if (!VU.update) {
									for (int j = 0; j < numsamps; j++) {
										if (VU.vu_l < buffer[j]) VU.vu_l = buffer[j];
										if (LineInputAttribs[Input_CurSelPos].nch == 2) {
											if (VU.vu_r < buffer[j + 1]) VU.vu_r = buffer[j + 1];
											j++;
										}
									}
									if (LineInputAttribs[Input_CurSelPos].nch == 1) VU.vu_r = VU.vu_l;
									VU.update = 1;
								}
								if (WaitForSingleObject(cf_mutex, INFINITE) == WAIT_OBJECT_0) {
									Crossfader->put(buffer, scardsmps);
									ReleaseMutex(cf_mutex);
								}
								Soundcard.cycleBuffer(last_buffer++);
								if (last_buffer >= NUM_BUFFERS) last_buffer = 0;
							}
						}
					}
				} else if (wParam == IDD_MAIN || wParam == IDD_CONNECTION ||
						   wParam == IDD_INPUT || wParam == IDD_ABOUT) {
					wchar_t title[1024] = {0};
					wchar_t next[1024] = {0};
					int states[NUM_OUTPUTS] = {OUT_ERROR,OUT_ERROR,OUT_ERROR,OUT_ERROR,OUT_ERROR};
		
					for (int i = 0; i < NUM_OUTPUTS; i++) {
						wchar_t tmp[1024] = {0};
						static wchar_t old_tmp[NUM_ENCODERS][1024] = {0};
						MY_T_OUTPUT *Out = &Output[i];
						SHOUTCAST_OUTPUT *Enc = Out->Encoder != -1 ? &Encoder[Out->Encoder] : NULL;
						bool encoder_ok = false;
						if (Enc) {
							C_ENCODER *Encoder = Enc->GetEncoder();
							if (Encoder) {
								if (strcmp(Encoder->GetName(), "MP3 Encoder") == 0) {
									encoder_ok = (libinst != NULL);
								} else {
									encoder_ok = true;
								}
							}
						}
						lastEnable[i] = (encoder_ok && Out->Config.Address[0] && Out->Config.Password[0] &&
										 stationNameAllowed(Out->Config.Description));

						if (Out->Encoder == -1 || Out->Handle == -1) {
							LocalisedString(IDS_NOT_CONNECTED, tmp, ARRAYSIZE(tmp));
							if (wParam == IDD_CONNECTION && i == Connection_CurSelPos) {
								GetDlgItemTextW(out_wnd[3].hWnd, IDC_TITLE, title, sizeof (title) / sizeof (wchar_t));
								GetDlgItemTextW(out_wnd[3].hWnd, IDC_NEXT, next, sizeof (next) / sizeof (wchar_t));
							}
						} else {
							if (WaitForSingleObject(Enc_mutex[Out->Encoder], INFINITE) == WAIT_OBJECT_0) {
								T_OUTPUT_INFO *Info = Enc->GetOutputInfo(Out->Handle);
								if (title != NULL && next != NULL && Info != NULL) {
									if (Info->Title)
									{
										free(Info->Title);
									}
									Info->Title = _wcsdup(title);
								}

								states[i] = Enc->GetState(Out->Handle);
								switch (states[i]) {
									case OUT_ERROR:
										{
											LocalisedString(IDS_ERROR, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_DISCONNECTED:
										{
											if (Info->Succeeded < 0) {
												if (Info->ErrorMsg && Info->ErrorMsg[0]) {
													// if an error is reported, try to give a user-friendly error message
													if (!strcmp("NAK:Deny", Info->ErrorMsg))
														// localised Password error
														LocalisedString(IDS_PASS_ERROR, tmp, ARRAYSIZE(tmp));
													else if (!strcmp("CipherFail",Info->ErrorMsg))
														// localised cipher error
														LocalisedString(IDS_CIPHER_ERROR, tmp, ARRAYSIZE(tmp));
													else if (!strcmp("BitrateError",Info->ErrorMsg))
														// localised bitrate error (not allowed / not supported)
														LocalisedString(IDS_BITRATE_ERROR, tmp, ARRAYSIZE(tmp));
													else if (!strcmp("StreamID",Info->ErrorMsg))
														// localised stream moved error
														LocalisedString(IDS_STREAMID_ERROR, tmp, ARRAYSIZE(tmp));
													else if (!strcmp("StreamMoved",Info->ErrorMsg))
														LocalisedString(IDS_STREAM_MOVED_ERROR, tmp, ARRAYSIZE(tmp));
													else if (!strcmp("VersionError",Info->ErrorMsg))
														// localised version error
														LocalisedString(IDS_VERSION_ERROR, tmp, ARRAYSIZE(tmp));
													else if (!strcmp("Blocked",Info->ErrorMsg))
														// localised blocked error
														LocalisedString(IDS_BLOCKED_ERROR, tmp, ARRAYSIZE(tmp));
													else if (!strcmp("InUse",Info->ErrorMsg))
														// localised in use error
														LocalisedString(IDS_IN_USE_ERROR, tmp, ARRAYSIZE(tmp));
													else if (!strcmp("ParseError",Info->ErrorMsg))
														// localised parse error
														LocalisedString(IDS_PARSE_ERROR, tmp, ARRAYSIZE(tmp));
													else
														// non localised dynamic nak error
														StringCchPrintfW(tmp, ARRAYSIZE(tmp), L"%hs", Info->ErrorMsg);
												} else {
													// localised Password error
													LocalisedString(IDS_PASS_ERROR, tmp, ARRAYSIZE(tmp));
												}
												Out->Config.AutoRecon = 0;
											} else {
												if (Info->last_state == OUT_RECV_CIPHER ||
													Info->last_state == OUT_REQUEST_CIPHER) {
													StringCchPrintfW(tmp, ARRAYSIZE(tmp), LocalisedString(IDS_ENABLE_OTHER_MODE, NULL, 0), (Info->last_state == OUT_RECV_CIPHER ? 1 : 2));
												} else {
													LocalisedString((lastEnable[i] ? IDS_NOT_CONNECTED : IDS_NOT_CONFIGURED), tmp, ARRAYSIZE(tmp));
												}
												lastSec[i] = 0;
											}
										}
										break;
									case OUT_CONNECT:
										{
											if ((clock() - Info->ConnectionTime) / CLOCKS_PER_SEC < 1) {
												StringCchPrintfW(tmp, ARRAYSIZE(tmp), LocalisedString(IDS_RECONNECTING_X, NULL, 0), 1);
											} else {
												LocalisedString(IDS_CONNECTING, tmp, ARRAYSIZE(tmp));
											}
										}
										break;
									case OUT_REQUEST_CIPHER:
										{
											LocalisedString(IDS_SEND_CIPHER_REQUEST, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_RECV_CIPHER:
										{
											LocalisedString(IDS_CIPHER_RESPONSE_RECEIVED, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_SENDAUTH:
										{
											LocalisedString(IDS_SENDING_AUTH, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_RECVAUTHRESPONSE:
										{
											LocalisedString(IDS_RECEIVING_AUTH_RESPONSE, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_SEND_MIME:
										{
											LocalisedString(IDS_SENDING_CONTENT_TYPE, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_RECV_MIME:
										{
											LocalisedString(IDS_RESPONSE_RECEIVED, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_SEND_BITRATE:
										{
											LocalisedString(IDS_SENDING_BITRATE, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_RECV_BITRATE:
										{
											LocalisedString(IDS_RESPONSE_RECEIVED, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_SEND_BUFSIZE:
										{
											LocalisedString(IDS_SEND_BUF_SIZE, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_RECV_BUFSIZE:
										{
											LocalisedString(IDS_RESPONSE_RECEIVED, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_SEND_MAX:
										{
											LocalisedString(IDS_SEND_MAX_PAYLOAD_SIZE, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_RECV_MAX:
										{
											LocalisedString(IDS_RESPONSE_RECEIVED, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_SENDYP:
										{
											LocalisedString(IDS_SEND_YP_INFO, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_SEND_INITFLUSH:
										{
											LocalisedString(IDS_SEND_FLUSH, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_RECV_INITFLUSH:
										{
											LocalisedString(IDS_RESPONSE_RECEIVED, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_SEND_INITSTANDBY:
										{
											LocalisedString(IDS_SEND_STANDBY, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_RECV_INITSTANDBY:
										{
											LocalisedString(IDS_RESPONSE_RECEIVED, tmp, ARRAYSIZE(tmp));
										}
										break;
									/*case OUT_SEND_INTRO:
										{
											LocalisedString(IDS_SEND_INTRO_FILE, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_RECV_INTRO:
										{
											LocalisedString(IDS_RESPONSE_RECEIVED, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_SEND_BACKUP:
										{
											LocalisedString(IDS_SEND_BACKUP_FILE, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_RECV_BACKUP:
										{
											LocalisedString(IDS_RESPONSE_RECEIVED, tmp, ARRAYSIZE(tmp));
										}
										break;*/
									case OUT_SEND_ARTWORK:
									case OUT_SEND_METADATA:
										break;
									case OUT_SENDCONTENT:
										{
											time_t time_value = time(NULL) - Info->ConnectedAt;
											long hour = (long)(time_value/3600);
											long min = (time_value/60)%60;
											long sec = time_value%60;
											static wchar_t format[256];
											if (!format[0]) LocalisedString(IDS_SENT_X, format, ARRAYSIZE(format));
											StringCchPrintfW(tmp, ARRAYSIZE(tmp), format, hour, min, sec, sizeStr(Info->BytesSent));
											lastSec[i] = sec;

											// do this to filter out some of the log
											// events so it'll only happen every second
											if (lastSec[i] == sec - 1) {
												secChanged[i] = true;
											}
										}
										break;
									case OUT_DISCONNECT:
										{
											LocalisedString(IDS_DISCONNECTING, tmp, ARRAYSIZE(tmp));
										}
										break;
									case OUT_RECONNECT:
										{
											if (Info->Reconnect) {
												int seconds = Info->ReconnectTime - ((clock() - Info->ConnectionTime) / CLOCKS_PER_SEC);
												if (seconds > 0) {
													StringCchPrintfW(tmp, ARRAYSIZE(tmp), LocalisedString(IDS_RECONNECTING_X, NULL, 0), seconds);
												} else {
													if (Info->Switching) {
														StringCchPrintfW(tmp, ARRAYSIZE(tmp), LocalisedString(IDS_SWITCHING_PROTOCOL, NULL, 0),
																		 (Info->Switching == 2 ? 2 : 1), (Info->Switching == 2 ? 1 : 2));
													} else {
														StringCchPrintfW(tmp, ARRAYSIZE(tmp), LocalisedString(IDS_RECONNECTING_X, NULL, 0), seconds);
													}
												}
											} else {
												if (Info->Switching) {
													StringCchPrintfW(tmp, ARRAYSIZE(tmp), LocalisedString(IDS_SWITCHING_PROTOCOL, NULL, 0),
																	 (Info->Switching == 2 ? 2 : 1), (Info->Switching == 2 ? 1 : 2));
												} else {
													LocalisedString(IDS_DISCONNECTING, tmp, ARRAYSIZE(tmp));
												}
											}
										}
										break;
									case OUT_TITLESENDUPDATE:
										{
											LocalisedString(IDS_SEND_TITLE_UPDATE, tmp, ARRAYSIZE(tmp));
										}
										break;
								}

								if (Out->AutoConnect && states[i] == OUT_DISCONNECTED) {
									Enc->ConnectOutput(Out->Handle);
								}

								ReleaseMutex(Enc_mutex[Out->Encoder]);
							}
						}

						// output log messages to the log files if enabled
						// but filter things a bit more with the sent state
						if (tmp[0] && wcsnicmp(old_tmp[i], tmp, ARRAYSIZE(tmp))) {
							if (Out->Logging && logFiles[i] != INVALID_HANDLE_VALUE) {
								if (states[i] != OUT_SENDCONTENT || (states[i] == OUT_SENDCONTENT && secChanged[i] == true)) {
									DWORD written = 0;
									int len = 0;
									char buffer[1024];
									if (states[i] == OUT_CONNECT) {
										wchar_t tmp2[2048] = {0};
										int protocol = (LOBYTE(Out->Config.protocol) != 1 ? 2 : 1);
										StringCchPrintfW(tmp2, ARRAYSIZE(tmp2),
														 L"Connecting to... Server: %hs; Port: %d; Mode: v%d; Stream ID: %hs; DJ / User ID: %hs",
														 Out->Config.Address, Out->Config.Port, protocol,
														 (protocol == 1 ? "n/a" : Out->Config.StationID),
														 (!Out->Config.UserID[0] ? "n/a" : Out->Config.UserID));
										CreateLogFileMessage(buffer, tmp2, &len);
										WriteFile(logFiles[i], buffer, len, &written, 0);
									} else {
										CreateLogFileMessage(buffer, tmp, &len);
										WriteFile(logFiles[i], buffer, len, &written, 0);
									}
								}
							}
							secChanged[i] = false;
						}

						// update summary and output page view and text states as applicable
						if (wParam == IDD_MAIN ||
							(wParam == IDD_CONNECTION && i == Connection_CurSelPos)) {
							// update status text for the output being processed
							if (tmp[0]) {
								if (wParam == IDD_CONNECTION && i == Connection_CurSelPos) {
									SetDlgItemTextW(wnd[1].hWnd, IDC_STATUS, tmp);
								}
								AddColItem(tmp, 2, wnd[0].hWnd, IDC_OUTPUTSTATUS, i);
							}

							bool encoder_ok = false;
							if (Enc) {
								C_ENCODER *Encoder = Enc->GetEncoder();
								if (Encoder) {
									if (strcmp(Encoder->GetName(), "MP3 Encoder") == 0) {
										encoder_ok = (libinst != NULL);
									} else {
										encoder_ok = true;
									}
								}
							}

							// update the playback buttons on the summary page
							int mode = (Out->Config.Address[0] ? (Out->Config.Password[0] ?
										(stationNameAllowed(Out->Config.Description) ?
										 (encoder_ok ? (states[i] == OUT_DISCONNECTED ? 0 :
										 states[i] == OUT_DISCONNECT ? 1 : states[i] == OUT_RECONNECT ? 7 : 2) : 6) : 5) : 4) : 3);

							// used to limit the amount of processing which is done for this to keep updates to just what is needed
							if (lastMode[i] == -1 || lastMode[i] != mode) {
								int image_id[] = {IDI_PLAY, IDI_KILL, IDI_STOP, IDI_PLAY, IDI_PLAY, IDI_PLAY, IDI_PLAY, IDI_STOP};

								// do checks to see if we need to update the error state of the tab items
								int oldMode = lastMode[i];
								lastMode[i] = mode;

								RECT r = {0};
								if (IsWindow(outTabWnd)) {
									int index[] = {0, 0, 1, 2};
									if (lastMode[i] >= 3 && lastMode[i] <= 6) {
										TabCtrl_GetItemRect(outTabWnd, index[(lastMode[i] - 3)], &r);
										InvalidateRect(outTabWnd, &r, 0);
									}

									if (oldMode >= 3 && oldMode <= 6) {
										TabCtrl_GetItemRect(outTabWnd, index[(oldMode - 3)], &r);
										InvalidateRect(outTabWnd, &r, 0);
									}
								}

								TabCtrl_GetItemRect(GetDlgItem(hMainDLG, IDC_TAB), 1, &r);
								InvalidateRect(GetDlgItem(hMainDLG, IDC_TAB), &r, 0);

								// control the button states on the pages
								EnableWindow(buttonWnd[i], lastEnable[i]);

								EnableWindowDlgItem(wnd[1].hWnd, IDC_CONNECT, lastEnable[i]);
								EnableWindowDlgItem(wnd[1].hWnd, IDC_AUTOCONNECT, lastEnable[i]);

								SendMessage(buttonWnd[i], BM_SETIMAGE, IMAGE_ICON,
											(LPARAM)LoadImage(module.hDllInstance, MAKEINTRESOURCE(image_id[mode]),
															  IMAGE_ICON, 0, 0, LR_SHARED | LR_LOADTRANSPARENT | LR_CREATEDIBSECTION));

								// control the 'connect' button to be disabled when no encoder / no password / invalid station name
								if (i == Connection_CurSelPos) {
									int button_id[] = {IDS_CONNECT, IDS_KILL, IDS_DISCONNECT, IDS_SET_SERVER, IDS_SET_PASSWORD, IDS_CHANGE_NAME, IDS_SET_ENCODER, IDS_ABORT};
									SetDlgItemTextW(wnd[1].hWnd, IDC_CONNECT, LocalisedString(button_id[mode], NULL, 0));
									LockOptionControls((states[i] == OUT_DISCONNECTED && Out->Handle != -1));

									InvalidateRect(GetDlgItem(out_wnd[0].hWnd, IDC_ADDRESS_HEADER), 0, 0);
									InvalidateRect(GetDlgItem(out_wnd[0].hWnd, IDC_PASSWORD_HEADER), 0, 0);
									InvalidateRect(GetDlgItem(out_wnd[1].hWnd, IDC_NAME_HEADER), 0, 0);
									InvalidateRect(GetDlgItem(out_wnd[2].hWnd, IDC_ENCODER_HEADER), 0, 0);
								}
							}

							// update the title set
							if (Out->AutoTitle) SetDlgItemTextW(out_wnd[0].hWnd, IDC_TITLE, title);
						}

						// preserve the last status message for filtering of the log output, etc
						if (tmp[0]) wcsncpy(old_tmp[i], tmp, 1024);
					}
				} else if (wParam == 1337) { // stream title update
					KillTimer(hDlg, wParam);
					PostMessage(hMainDLG, WM_USER, 0, nowPlayingID);
				} else if (wParam == 2234) { // fade (music)
					if (InputDevice) {
						clock_t MusCurTime = clock();
						if (FadeStartTime == 0) {
							FadeStartTime = MusCurTime;
						}
						MusCurTime -= FadeStartTime;
						int micsrc = Input_Device_ID  >= 1  ? MIXERLINE_COMPONENTTYPE_SRC_LINE : MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE;
						if (MusCurTime < (FadeTime * 100)) {
							int musvol = FadeOut ? (MusVol * 10)+((((Mus2Vol - MusVol)*10) * MusCurTime) / (FadeTime * 100)) : (Mus2Vol * 10)+((((MusVol - Mus2Vol)*10) * MusCurTime) / (FadeTime * 100));
							setlev(MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT, musvol);
						} else {
							if (FadeOut) {
								setlev(MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT, Mus2Vol * 10);
							} else {
								setlev(MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT, MusVol * 10);
							}
							FadeStartTime = 0;
							KillTimer(hDlg, wParam);
							// kill captured device
							#ifdef CAPTURE_TESTING
							if (!FadeOut) {// end_capture();
								if (pPlayer != NULL) {
									pPlayer->Stop();
									delete pPlayer;
									pPlayer = NULL;
								}
							}
							#endif
						}
					}
				} else if (wParam == 2235) { // fade (capture device)
					if (InputDevice) {
						clock_t MicCurTime = clock();
						if (MicFadeStartTime == 0) {
							MicFadeStartTime = MicCurTime;
						}
						MicCurTime -= MicFadeStartTime;
						int micsrc = Input_Device_ID  >= 1  ? MIXERLINE_COMPONENTTYPE_SRC_LINE : MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE;
						if (MicCurTime < (MicFadeTime * 100)) {
							int micvol = FadeOut ? ((MicVol * 10) * MicCurTime) / (MicFadeTime * 100) : (MicVol * 10)+(((MicVol*-10) * MicCurTime) / (MicFadeTime * 100));
							setlev(micsrc, micvol);
						} else {
							if (FadeOut) {
								setlev(micsrc, MicVol * 10);
							} else {
								setlev(micsrc, 0);
							}
							MicFadeStartTime = 0;
							KillTimer(hDlg, wParam);
							// kill captured device
							#ifdef CAPTURE_TESTING
							if (!FadeOut) {// end_capture();
								if (pPlayer != NULL) {
									pPlayer->Stop();
									delete pPlayer;
									pPlayer = NULL;
								}
							}
							#endif
						}
					}
				} else if (wParam == 666) { // INI save timer
					if (!ini_modified) break;
					ini_modified = 0;
					int i;
					WritePrivateProfileInt("ofnidx", lastFilterIndex, 0, 0);
					WritePrivateProfileInt("CurTab", curtab, 0, 0);
					WritePrivateProfileInt("Connection_CurSelPos", Connection_CurSelPos, 0, 0);
					WritePrivateProfileInt("Connection_CurTab", curouttab, 0, 0);
					WritePrivateProfileInt("Encoder_CurSelPos", Encoder_CurSelPos, 0, 0);
					WritePrivateProfileInt("Input_CurSelPos", Input_CurSelPos, 0, 0);
					WritePrivateProfileInt("InputDevice", InputDevice, 0, 0);
					WritePrivateProfileInt("MusicVolume", MusVol, 0, 0);
					WritePrivateProfileInt("BGMusicVolume", Mus2Vol, 0, 0);
					WritePrivateProfileInt("MicVolume", MicVol, 0, 0);
					WritePrivateProfileInt("PTT_FT", FadeTime, 0, 0);
					WritePrivateProfileInt("PTT_MicFT", MicFadeTime, 0, 0);
					WritePrivateProfileInt("PTT_MicInput", Input_Device_ID, 0, 0);
					WritePrivateProfileInt("PTT_Restore", Restore_PTT, 0, 0);

					if (!IsIconic(hDlg)) {
						WritePrivateProfileInt("WindowLeft", mainrect.left, 0, 0);
						WritePrivateProfileInt("WindowTop", mainrect.top, 0, 0);
					}

					for (i = 0; i < NUM_ENCODERS; i++) {
						int Type = 0;
						char name[32];
						StringCchPrintfA(name, ARRAYSIZE(name), "Encoder %u", i + 1);

						if (WaitForSingleObject(Enc_mutex[i], INFINITE) == WAIT_OBJECT_0) {
							C_ENCODER *Enc = Encoder[i].GetEncoder();
							if (Enc) {
								if (strcmp(Enc->GetName(), "MP3 Encoder") == 0) {
									int infosize = sizeof (T_ENCODER_MP3_INFO);
									T_ENCODER_MP3_INFO *EncInfo = (T_ENCODER_MP3_INFO *) Enc->GetExtInfo(&infosize);
									WritePrivateProfileInt("BitRate", EncInfo->output_bitRate, name, 0);
									WritePrivateProfileInt("SampleRate", EncInfo->output_sampleRate, name, 0);
									WritePrivateProfileInt("NumChannels", EncInfo->output_numChannels, name, 0);
									WritePrivateProfileInt("QualityMode", EncInfo->QualityMode, name, 0);
									Type = 1;
								} else if (strcmp(Enc->GetName(), "Fraunhofer Encoder") == 0) { // FHG AAC
									((C_ENCODER_NSV*) Enc)->FillConfFile(IniName, name);
									Type = 2;
								} else if (strcmp(Enc->GetName(), "AAC+ Encoder") == 0) { // AAC+
									((C_ENCODER_NSV*) Enc)->FillConfFile(IniName, name);
									Type = 2;
								}
#ifdef USE_OGG
								else if (strcmp(Enc->GetName(), "OGG Vorbis Encoder") == 0) { // OGG
									((C_ENCODER_NSV*) Enc)->FillConfFile(IniName, name);
									Type = 4;
								}
#endif
							}
							ReleaseMutex(Enc_mutex[i]);
						}
						WritePrivateProfileInt("Type", Type, name, 0);
					}

					for (i = 0; i < NUM_OUTPUTS; i++) {
						T_OUTPUT_CONFIG *Out = &Output[i].Config;
						WritePrivateProfileString(Out->Name, "Address", Out->Address, IniName);
						WritePrivateProfileInt("Port", Out->Port, Out->Name, 0);
						WritePrivateProfileString(Out->Name, "UserID", Out->UserID, IniName);
						WritePrivateProfileString(Out->Name, "StreamID", Out->StationID, IniName);
						WritePrivateProfileString(Out->Name, "Password", Out->Password, IniName);
						// disabled saving of this as it defeats the point of setting it on load
						// (as it's otherwise over-written with the correct value from the server)
						//WritePrivateProfileString(Out->Name, "Cipherkey", Out->cipherkey, IniName);
						WritePrivateProfileString(Out->Name, "Description", Out->Description, IniName);
						WritePrivateProfileString(Out->Name, "URL", Out->ServerURL, IniName);
						WritePrivateProfileString(Out->Name, "Genre3", Out->Genre, IniName);
						WritePrivateProfileString(Out->Name, "AIM", Out->AIM, IniName);
						WritePrivateProfileString(Out->Name, "ICQ", Out->ICQ, IniName);
						WritePrivateProfileString(Out->Name, "IRC", Out->IRC, IniName);
						WritePrivateProfileInt("Public", Out->Public ? 1 : 0, Out->Name, 0);
						WritePrivateProfileInt("AutoRecon", Out->AutoRecon ? 1 : 0, Out->Name, 0);
						WritePrivateProfileInt("ReconTime", Out->ReconTime ? Out->ReconTime : 1, Out->Name, 0);
						WritePrivateProfileInt("doTitleUpdate", Out->doTitleUpdate ? 1 : 0, Out->Name, 0);
						WritePrivateProfileString(Out->Name, "now", Out->Now, IniName);
						WritePrivateProfileString(Out->Name, "next", Out->Next, IniName);
						WritePrivateProfileInt("AutoTitle", Output[i].AutoTitle ? 1 : 0, Out->Name, 0);
						WritePrivateProfileInt("AutoConnect", Output[i].AutoConnect ? 1 : 0, Out->Name, 0);
						WritePrivateProfileInt("Logging", Output[i].Logging ? 1 : 0, Out->Name, 0);
						WritePrivateProfileInt("LogCOS", Output[i].LogCOS ? 1 : 0, Out->Name, 0);
						WritePrivateProfileInt("NextTitles", Output[i].NextTitles ? 1 : 0, Out->Name, 0);
						WritePrivateProfileInt("Protocol", Output[i].Config.protocol, Out->Name, 0);
						WritePrivateProfileInt("nextTrackLog", Output[i].nextTrackLog ? 1 : 0, Out->Name, 0);
						WritePrivateProfileInt("nextTrackLogXML", Output[i].nextTrackLogXML ? 1 : 0, Out->Name, 0);
						WritePrivateProfileString(Out->Name, "nextTrackPath", ConvertToUTF8(Output[i].nextTrackPath), IniName);
						WritePrivateProfileInt("useArt", Output[i].useArt ? 1 : 0, Out->Name, 0);
						WritePrivateProfileInt("usePlayingArt", Output[i].usePlayingArt ? 1 : 0, Out->Name, 0);
						WritePrivateProfileInt("useStreamArt", Output[i].useStreamArt ? 1 : 0, Out->Name, 0);
						WritePrivateProfileString(Out->Name, "stationArtPath", ConvertToUTF8(Output[i].stationArtPath), IniName);
						WritePrivateProfileInt("saveEncoded", Output[i].saveEncoded ? 1 : 0, Out->Name, 0);
						WritePrivateProfileString(Out->Name, "saveEncodedPath", ConvertToUTF8(Output[i].saveEncodedPath), IniName);
						WritePrivateProfileInt("Encoder", Output[i].Encoder, Out->Name, 0);
					}
				}
			}
			break;

		case WM_SHOWWINDOW:
			{
				if (IsWindow(hDlg) && hDlg == hMainDLG && wnd[curtab].timer_freq != 0) {
					KillTimer(hDlg, wnd[curtab].id);
					SetTimer(hDlg, wnd[curtab].id, wnd[curtab].timer_freq, NULL);
				} else if (IsWindow(hDlg) && hDlg == wnd[2].hWnd && in_wnd[InputDevice].timer_freq != 0) {
					KillTimer(hDlg, in_wnd[InputDevice].id);
					SetTimer(hDlg, in_wnd[InputDevice].id, in_wnd[InputDevice].timer_freq, NULL);
				} else if (IsWindow(hDlg) && hDlg == wnd[1].hWnd && out_wnd[curouttab].timer_freq != 0) {
					KillTimer(hDlg, out_wnd[curouttab].id);
					SetTimer(hDlg, out_wnd[curouttab].id, out_wnd[curouttab].timer_freq, NULL);
				}
			}
			break;

			//Handle Minimize to systray here
		case WM_SIZE:
			{
				switch (wParam) {
					case SIZE_RESTORED:
						{
							RemoveSystrayIcon(hDlg, SYSTRAY_ICY_ICON);
							ShowWindow(hDlg, SW_SHOW);
						}
						break;
					case SIZE_MINIMIZED:
						{
							AddSystrayIcon(hDlg, SYSTRAY_ICY_ICON, GetICYIcon(),
										   SYSTRAY_MAXIMIZE_MSG, szDescription2W);
							ShowWindow(hDlg, SW_HIDE);
						}
						break;
				}
			}
			break;

		case WM_COMMAND:
			{
				switch (LOWORD(wParam)) {
					case IDC_NOTITLES:
					case IDC_AUTOTITLE:
					case IDC_MANUALTITLE:
					//case IDC_EXTERNALTITLE:
						{
							// TODO will need to change around some of the logic to cope with the external option
							if (HIWORD(wParam) == BN_CLICKED) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								int lastAutoTitle = Out->AutoTitle;
								Out->Config.doTitleUpdate = (LOWORD(wParam) != IDC_NOTITLES);
								Out->AutoTitle = (LOWORD(wParam) == IDC_AUTOTITLE);
								ini_modified = 1;
								CheckRadioButton(hDlg, IDC_NOTITLES, IDC_MANUALTITLE, LOWORD(wParam));
								UpdateTitleControls();
								// if enabling then send the title update
								if ((LOWORD(wParam) == IDC_AUTOTITLE) && lastAutoTitle != Out->AutoTitle) {
									if (Out->Encoder != -1 && Out->Handle != -1) {
										SHOUTCAST_OUTPUT *Enc = &Encoder[Out->Encoder];
										if (WaitForSingleObject(Enc_mutex[Out->Encoder], INFINITE) == WAIT_OBJECT_0) {
											std::vector<std::wstring> nextList;
											nextList.clear();
											Enc->UpdateTitle(0, nextList, Out->Handle, !!Out->NextTitles, true);
											ReleaseMutex(Enc_mutex[Out->Encoder]);
										}
									}
								}
							}
						}
						break;

					case IDC_SENDNEXTTITLES:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								Out->NextTitles = SendMessage((HWND) lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
								ini_modified = 1;
							}
						}
						break;

					case IDC_VIEW_LOG:
						{
							wchar_t file[MAX_PATH];
							ShellExecuteW(hMainDLG, L"open", GetSCLogFile(module.hwndParent, ARRAYSIZE(file), file, Connection_CurSelPos), 0, NULL, SW_SHOW);
						}
						break;

					case IDC_CLEAR_LOG:
						{
							MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
							if (Out->Logging && logFiles[Connection_CurSelPos] != INVALID_HANDLE_VALUE) {
								SetFilePointer(logFiles[Connection_CurSelPos], 0, NULL, FILE_BEGIN);
								SetEndOfFile(logFiles[Connection_CurSelPos]);
							} else {
								wchar_t file[MAX_PATH];
								HANDLE handle = CreateFileW(GetSCLogFile(module.hwndParent, ARRAYSIZE(file), file, Connection_CurSelPos),
															GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
								if (handle != INVALID_HANDLE_VALUE) {
									SetFilePointer(handle, 0, NULL, FILE_BEGIN);
									SetEndOfFile(handle);
									CloseHandle(handle);
								}
							}
						}
						break;

					case IDC_LOGGING:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								Out->Logging = SendMessage((HWND) lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
								if (Out->Logging) {
									StartLogging(Connection_CurSelPos,Out->LogCOS);
								} else {
									StopLogging(Connection_CurSelPos);
								}
								ini_modified = 1;
							}
						}
						break;

					case IDC_CLEAR_ON_STARTUP:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								Out->LogCOS = SendMessage((HWND) lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
								ini_modified = 1;
							}
						}
						break;

					case IDC_NEXT_TRACK_LOG:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								Out->nextTrackLog = SendMessage((HWND) lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
								if (Out->nextTrackLog) {
									StartNextTracks(Connection_CurSelPos, Out->nextTrackPath);
									FillNextTracks(Connection_CurSelPos, !!Out->nextTrackLogXML);
								} else {
									StopNextTracks(Connection_CurSelPos);
								}
								EnableWindowDlgItem(out_wnd[5].hWnd, IDC_NEXT_TRACK_XML, Out->nextTrackLog);
								EnableWindowDlgItem(out_wnd[5].hWnd, IDC_NEXT_TRACK_EDIT, Out->nextTrackLog);
								EnableWindowDlgItem(out_wnd[5].hWnd, IDC_NEXT_TRACK_BROWSE, Out->nextTrackLog);

								ini_modified = 1;
							}
						}
						break;

					case IDC_NEXT_TRACK_XML:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								Out->nextTrackLogXML = SendMessage((HWND) lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
								// reset the file if changing the state
								if (Out->nextTrackLog) {
									StopNextTracks(Connection_CurSelPos);
									StartNextTracks(Connection_CurSelPos, Out->nextTrackPath);
									FillNextTracks(Connection_CurSelPos, !!Out->nextTrackLogXML);
								}
								ini_modified = 1;
							}
						}
						break;

					case IDC_NEXT_TRACK_EDIT:
						{
							if (HIWORD(wParam) == EN_SETFOCUS) {
								PostMessage((HWND)lParam, EM_SETSEL, 0, (LPARAM)-1);
							}
						}
						break;

					case IDC_NEXT_TRACK_BROWSE:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								wchar_t filepath[MAX_PATH] = {0},
										file[MAX_PATH] = {0},
										filter[64] = {0};
								// strip path so we can set initial directory, etc
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								wcsncpy(filepath, Out->nextTrackPath, MAX_PATH);
								wcsncpy(file, Out->nextTrackPath, MAX_PATH);
								PathRemoveFileSpecW(filepath);
								PathStripPathW(file);

								OPENFILENAMEW ofn = {0};
								ofn.lStructSize=sizeof(ofn);
								ofn.hwndOwner=hMainDLG;

								WASABI_API_LNGSTRINGW_BUF(IDS_ALL_FILES, filter, 64);
								wchar_t * ptr=filter;
								while(ptr && *ptr) {
									if (*ptr==L'|') *ptr=0;
									ptr++;
								}

								ofn.lpstrFilter=filter;
								ofn.lpstrInitialDir=filepath;
								ofn.lpstrFile=file;
								ofn.nMaxFile=MAX_PATH;
								ofn.Flags=OFN_HIDEREADONLY|OFN_PATHMUSTEXIST;
								ofn.lpstrDefExt=L"log";
								if (GetSaveFileNameW(&ofn))
								{
									// update things if the filename changed, etc
									wcsncpy(Out->nextTrackPath, file, MAX_PATH);
									SetDlgItemTextW(out_wnd[5].hWnd, IDC_NEXT_TRACK_EDIT, Out->nextTrackPath);
									if (Out->nextTrackLog) {
										StopNextTracks(Connection_CurSelPos);
										StartNextTracks(Connection_CurSelPos, Out->nextTrackPath);
									}
								}
							}
						}
						break;

					case IDC_SAVE_ENCODED_AUDIO:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								Out->saveEncoded = SendMessage((HWND) lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;

								// reset the file if changing the state
								if (Out->saveEncoded) {
									StartSaveEncoded(Connection_CurSelPos, Out->saveEncodedPath);
								} else {
									StopSaveEncoded(Connection_CurSelPos);
								}
								ini_modified = 1;
							}
						}
						break;

					case IDC_SAVE_ENCODED_AUDIO_EDIT:
						{
							if (HIWORD(wParam) == EN_SETFOCUS) {
								PostMessage((HWND)lParam, EM_SETSEL, 0, (LPARAM)-1);
							}
						}
						break;

					case IDC_SAVE_ENCODED_AUDIO_BROWSE:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								wchar_t filepath[MAX_PATH] = {0},
										file[MAX_PATH] = {0},
										filter[64] = {0};
								// strip path so we can set initial directory, etc
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								wcsncpy(filepath, Out->saveEncodedPath, MAX_PATH);
								wcsncpy(file, Out->saveEncodedPath, MAX_PATH);
								PathRemoveFileSpecW(filepath);
								PathStripPathW(file);

								OPENFILENAMEW ofn = {0};
								ofn.lStructSize=sizeof(ofn);
								ofn.hwndOwner=hMainDLG;
								// sets the default extension if not specified to the type of the encoder being used
								ofn.lpstrDefExt=(Enc_LastType[Connection_CurSelPos] != 0 ?
												 (!strcmp(Encoder[Out->Encoder].GetEncoder()->GetContentType(),"audio/mpeg") ? L"mp3" :
												  (!strcmp(Encoder[Out->Encoder].GetEncoder()->GetContentType(),"audio/ogg") ? L"ogg" : L"aac")) : L"");

								StringCchPrintfW(filter, ARRAYSIZE(filter), WASABI_API_LNGSTRINGW(IDS_MPEG_AUDIO_FILES), ofn.lpstrDefExt, ofn.lpstrDefExt);
								wchar_t* ptr=filter;
								while(ptr && *ptr) {
									if (*ptr==L'|') *ptr=0;
									ptr++;
								}

								ofn.lpstrFilter=filter;
								ofn.lpstrInitialDir=filepath;
								ofn.lpstrFile=file;
								ofn.nMaxFile=MAX_PATH;
								ofn.Flags=OFN_HIDEREADONLY|OFN_PATHMUSTEXIST;

								if (GetSaveFileNameW(&ofn))
								{
									// update things if the filename changed, etc
									wcsncpy(Out->saveEncodedPath, file, MAX_PATH);
									SetDlgItemTextW(out_wnd[2].hWnd, IDC_SAVE_ENCODED_AUDIO_EDIT, Out->saveEncodedPath);
									if (Out->saveEncoded) {
										StopSaveEncoded(Connection_CurSelPos);
										StartSaveEncoded(Connection_CurSelPos, Out->saveEncodedPath);
									}
								}
							}
						}
						break;

					case IDC_USE_ART:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								Out->useArt = SendMessage((HWND) lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;

								EnableWindowDlgItem(out_wnd[4].hWnd, IDC_USE_ART_PLAYING, Out->useArt);
								EnableWindowDlgItem(out_wnd[4].hWnd, IDC_USE_ART_STREAM, Out->useArt);
								EnableWindowDlgItem(out_wnd[4].hWnd, IDC_ART_EDIT, Out->useArt && Out->useStreamArt);
								EnableWindowDlgItem(out_wnd[4].hWnd, IDC_ART_BROWSE, Out->useArt && Out->useStreamArt);

								// only update if we need to do so
								UpdateStreamAlbumArt(Out->Handle, Connection_CurSelPos, Out->stationArtPath, Out->useArt && Out->useStreamArt);

								// this will update against what is read from the playing
								UpdatePlayingAlbumArt(Out->Handle, Connection_CurSelPos, Out->useArt && Out->usePlayingArt);

								Encoder[Connection_CurSelPos].UpdateArtwork(Out->Handle);

								ini_modified = 1;
							}
						}
						break;

					case IDC_USE_ART_PLAYING:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								Out->usePlayingArt = SendMessage((HWND) lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
								ini_modified = 1;

								if(Out->usePlayingArt){
									playingLength = 0;
									playingType = 0x4101;	// default in-case of issue

									int w = 0, h = 0;
									if (AGAVE_API_ALBUMART && AGAVE_API_ALBUMART->GetAlbumArt(lastFile, L"cover", &w, &h, &playingImage) == ALBUMART_SUCCESS) {
										// make sure to free the original image after we've converted
										ARGB32 *firstPlayingImage = playingImage;
										playingImage = writeImg(playingImage, w, h, &playingLength, L"png");
										WASABI_API_MEMMGR->sysFree(firstPlayingImage);
									} else {
										playingImage = 0;
									}
								}

								UpdatePlayingAlbumArt(Out->Handle, Connection_CurSelPos, Out->useArt && Out->usePlayingArt);
								Encoder[Connection_CurSelPos].UpdateArtwork(Out->Handle);
							}
						}
						break;

					case IDC_USE_ART_STREAM:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								Out->useStreamArt = SendMessage((HWND) lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
								ini_modified = 1;

								// skip this all if the mode is not enabled and only in SC2 mode
								if ((LOBYTE(Out->Config.protocol) != 1) && Out->useArt) {
									// this will only update generally on first connection
									if (Out->useStreamArt && streamImage[Connection_CurSelPos] == (ARGB32 *)-1) {
										UpdateStreamAlbumArt(Out->Handle, Connection_CurSelPos, Out->stationArtPath, !!Out->useStreamArt);
									}
								}

								if (UpdateStreamAlbumArt(Out->Handle, Connection_CurSelPos, Out->stationArtPath, !!Out->useStreamArt)) {
									Encoder[Connection_CurSelPos].UpdateArtwork(Out->Handle);
								}

								EnableWindowDlgItem(out_wnd[4].hWnd, IDC_ART_EDIT, Out->useStreamArt);
								EnableWindowDlgItem(out_wnd[4].hWnd, IDC_ART_BROWSE, Out->useStreamArt);
							}
						}
						break;

					case IDC_ART_EDIT:
						{
							if (HIWORD(wParam) == EN_SETFOCUS) {
								PostMessage((HWND)lParam, EM_SETSEL, 0, (LPARAM)-1);
							}
						}
						break;

					case IDC_ART_BROWSE:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								wchar_t filepath[MAX_PATH] = {0},
										file[MAX_PATH] = {0};
								// strip path so we can set initial directory, etc
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								wcsncpy(filepath, Out->stationArtPath, MAX_PATH);
								wcsncpy(file, Out->stationArtPath, MAX_PATH);
								PathRemoveFileSpecW(filepath);
								PathStripPathW(file);

								OPENFILENAMEW ofn = {0};
								ofn.lStructSize=sizeof(ofn);
								ofn.hwndOwner=hMainDLG;
								ofn.lpstrInitialDir=filepath;
								ofn.lpstrFile=file;
								ofn.nMaxFile=MAX_PATH;
								ofn.Flags=OFN_HIDEREADONLY|OFN_PATHMUSTEXIST;
								ofn.lpstrDefExt=L"png";
								ofn.nFilterIndex=lastFilterIndex;

								static int tests_run = 0;
								static wchar_t filter[1024] = {0}, *sff = filter;

								if (!tests_run) {
									tests_run = 1;
									FOURCC imgload = svc_imageLoader::getServiceType();
									int n = WASABI_API_SVC->service_getNumServices(imgload);
									for (int i=0; i<n; i++) {
										waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgload, i);
										if (sf) {
											svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
											if (l) {
												static int tests_idx[4] = {0, 1, 2, 3};
												size_t size = 1024;
												int j = 0, tests_str[] = {IDS_JPEG_FILE, IDS_PNG_FILE, IDS_BMP_FILE, IDS_GIF_FILE};
												wchar_t *tests[] = {L"*.jpg", L"*.png", L"*.bmp", L"*.gif"};
												for (int i = 0; i < ARRAYSIZE(tests); i++) {
													if (l->isMine(tests[i])) {
														tests_idx[j] = i;
														j++;
														int len = 0;
														LocalisedString(tests_str[i], sff, size);
														size-=(len = wcslen(sff)+1);
														sff+=len;
														wcsncpy(sff, (!i ? L"*.jpg;*.jpeg" : tests[i]), size);
														size-=(len = wcslen(sff)+1);
														sff+=len;
													}
												}
												sf->releaseInterface(l);
											}
										}
									}
								}

								ofn.lpstrFilter = filter;

								if (GetOpenFileNameW(&ofn))
								{
									// update things if the filename changed, etc
									wcsncpy(Out->stationArtPath, file, MAX_PATH);
									SetDlgItemTextW(out_wnd[4].hWnd, IDC_ART_EDIT, Out->stationArtPath);

									if (UpdateStreamAlbumArt(Out->Handle, Connection_CurSelPos, Out->stationArtPath, !!Out->useStreamArt)) {
										Encoder[Connection_CurSelPos].UpdateArtwork(Out->Handle);
									}
								}
								lastFilterIndex = ofn.nFilterIndex;
							}
						}
						break;

					case IDC_AUTOCONNECT:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								Output[Connection_CurSelPos].AutoConnect = SendMessage((HWND) lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
								if (Output[Connection_CurSelPos].Encoder) {
									if (WaitForSingleObject(Enc_mutex[Output[Connection_CurSelPos].Encoder], INFINITE) == WAIT_OBJECT_0) {
										Encoder[Output[Connection_CurSelPos].Encoder].UpdateOutput(Output[Connection_CurSelPos].Handle);
										ReleaseMutex(Enc_mutex[Output[Connection_CurSelPos].Encoder]);
									}
								}
							}
						}
						break;

					case IDC_RECONNECT:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								Output[Connection_CurSelPos].Config.AutoRecon = SendMessage((HWND) lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
								if (Output[Connection_CurSelPos].Encoder) {
									if (WaitForSingleObject(Enc_mutex[Output[Connection_CurSelPos].Encoder], INFINITE) == WAIT_OBJECT_0) {
										Encoder[Output[Connection_CurSelPos].Encoder].UpdateOutput(Output[Connection_CurSelPos].Handle);
										ReleaseMutex(Enc_mutex[Output[Connection_CurSelPos].Encoder]);
									}
								}
							}
						}
						break;

					case IDC_PROTOCOL:
						{
							if (HIWORD(wParam) == CBN_SELCHANGE) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];

								int cur_sel = SendMessage((HWND) lParam, CB_GETCURSEL, 0, 0),
									protocol = SendMessage((HWND) lParam, CB_GETITEMDATA, cur_sel, 0);

								Out->Config.protocol = MAKEWORD(protocol, !protocol);

								// force a refresh on selection change
								lastMode[Connection_CurSelPos] = -1;

								// jkey: disable or enable userid stationid based on protocol.
								if (Output[Connection_CurSelPos].Encoder) {
									if (WaitForSingleObject(Enc_mutex[Output[Connection_CurSelPos].Encoder], INFINITE) == WAIT_OBJECT_0) {
										Encoder[Output[Connection_CurSelPos].Encoder].UpdateOutput(Output[Connection_CurSelPos].Handle);
										ReleaseMutex(Enc_mutex[Output[Connection_CurSelPos].Encoder]);
									}
								}
								//shoutcast 2 else 1 enable aim,icq,irc
								EnableWindowDlgItem(hDlg, IDC_STATIONID, (LOBYTE(Output[Connection_CurSelPos].Config.protocol) == 1));
								UpdateTitleControls();
							}
						}
						break;

					case IDC_CONNECT:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								SHOUTCAST_OUTPUT *Enc = Out->Encoder != -1 ? &Encoder[Out->Encoder] : NULL;
								if (Enc && Out->Handle != -1) {
									int state = Enc->GetState(Out->Handle);
									if (state == OUT_DISCONNECTED) { // disconnected... connect now
										Enc->ConnectOutput(Out->Handle);
									} else { // connected... disconnect now
										Enc->DisconnectOutput(Out->Handle);
									}
								}
							}
						}
						break;

					case IDC_DEVBOX:
						{
							if (HIWORD(wParam) == CBN_SELCHANGE) {
								Input_Device_ID = SendMessage((HWND) lParam, CB_GETCURSEL, 0, 0);
							}
						}
						break;

					case IDC_LOCK:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								int checked = SendMessage((HWND) lParam, BM_GETCHECK, -1, -1) == BST_CHECKED;
								EnableWindowDlgItem(hDlg, IDC_PTT, !checked);
								SendDlgItemMessage(hDlg, IDC_PTT, BM_SETSTATE, checked ? BST_CHECKED : BST_UNCHECKED, 0);
								KillTimer(hMainDLG, 2234);
								KillTimer(hMainDLG, 2235);
								if ((MicFadeStartTime != 0 || FadeStartTime != 0) && FadeOut != checked) {
									clock_t myTime = clock();
									if (FadeStartTime != 0) FadeStartTime = myTime - ((FadeTime * 100)-(myTime - FadeStartTime));
									if (MicFadeStartTime != 0) MicFadeStartTime = myTime - ((MicFadeTime * 100)-(myTime - MicFadeStartTime));
								}
								if(!ptt_load && Restore_PTT || ptt_load) {
									FadeOut = checked;
									SetTimer(hMainDLG, 2234, 10, NULL); // fade out
									SetTimer(hMainDLG, 2235, 10, NULL); // fade out
								} else {
									SetDeviceName();
								}								
								ptt_load = 1;
								//if (FadeOut) do_capture();
								#ifdef CAPTURE_TESTING
								if (FadeOut) {
									if (!pPlayer) {
										pPlayer = new Player(hMainDLG);
									}
									if (!pCallbacks) {
										pCallbacks = new CPlayerCallbacks();
									}
									pPlayer->SetPlayerCallbacks(pCallbacks);
									pPlayer->RefreshDeviceList(eRender);
									pPlayer->RefreshDeviceList(eCapture);
									pPlayer->SelectDefaultDevice(eRender, eConsole);
									pPlayer->SelectDefaultDevice(eCapture, eConsole);
									pPlayer->SelectDeviceFromList(eCapture, 0);
									//pPlayer->SelectDeviceFromList(eRender, 2);
									if (pPlayer->Play(eCaptureEndpoint) == FALSE) {
										return TRUE;
									}
								}
								#endif
							}
						}
						break;

					case IDC_LOCK_MODE:
						{
							HMENU hmenu = CreatePopupMenu();
							RECT r;
							GetWindowRect((HWND)lParam, &r);

							MENUITEMINFOW i = {sizeof(i), MIIM_ID | MIIM_STATE | MIIM_TYPE, MFT_STRING,
							                   Restore_PTT ? MFS_CHECKED : 0, 1337};
							i.dwTypeData = LocalisedString(IDS_PTT_ON_STARTUP, NULL, 0);
							InsertMenuItemW(hmenu, 0, TRUE, &i);
							if (TrackPopupMenu(hmenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, r.left, r.bottom, 0, (HWND)lParam, NULL) == 1337) {
								Restore_PTT = !Restore_PTT;
							}
							DestroyMenu(hmenu);
						}
						break;

					case IDC_REFRESH_DEVICES:
						{
							if (IsVistaUp()) {
								SendDlgItemMessage(inWin,IDC_DEVBOX, CB_RESETCONTENT, 0, 0);
								SetDeviceName();
							}
						}
						break;

					case IDC_MIXER:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								// open vista / win7 or win2k / xp recording panel
								// (more sensible esp. for vista / win7)
								if (IsVistaUp()) {
									ShellExecuteW(hMainDLG, L"open", L"control.exe", L"mmsys.cpl,,1", NULL, SW_SHOW);
								} else {
									ShellExecuteW(hMainDLG, L"open", L"sndvol32.exe", L"", NULL, SW_SHOW);
									ShellExecuteW(hMainDLG, L"open", L"sndvol32.exe", L"/r", NULL, SW_SHOW);
								}
							}
						}
						break;

					case IDC_OUTPUTLIST:
						{
							if (HIWORD(wParam) == LBN_SELCHANGE) {
								Connection_CurSelPos = SendMessage((HWND) lParam, LB_GETCURSEL, 0, 0);

								// force a refresh on selection change
								lastMode[Connection_CurSelPos] = -1;

								// do checks to see if we need to update the error state of the tab items
								if (IsWindow(outTabWnd)) {
									RECT r = {0};
									for (int i = 0; i < MAX_OUTWNDS; i++) {
										TabCtrl_GetItemRect(outTabWnd, i, &r);
										InvalidateRect(outTabWnd, &r, 0);
									}

									TabCtrl_GetItemRect(GetDlgItem(hMainDLG, IDC_TAB), 1, &r);
									InvalidateRect(GetDlgItem(hMainDLG, IDC_TAB), &r, 0);
								}

								T_OUTPUT_CONFIG *Out = &Output[Connection_CurSelPos].Config;
								MY_T_OUTPUT * OutEnc = &Output[Connection_CurSelPos];
								int sc2mode = (LOBYTE(Out->protocol) != 1);
								// Output page 1
								SendDlgItemMessage(out_wnd[0].hWnd, IDC_ADDRESS, EM_SETLIMITTEXT, ARRAYSIZE(Out->Address) - 1, 0);
								SetDlgItemText(out_wnd[0].hWnd, IDC_ADDRESS, Out->Address);
								SendDlgItemMessage(out_wnd[0].hWnd, IDC_STATIONID, EM_SETLIMITTEXT, 10, 0);
								SetDlgItemText(out_wnd[0].hWnd, IDC_STATIONID, Out->StationID);
								SendDlgItemMessage(out_wnd[0].hWnd, IDC_USERID, EM_SETLIMITTEXT, ARRAYSIZE(Out->UserID) - 1, 0);
								SetDlgItemText(out_wnd[0].hWnd, IDC_USERID, Out->UserID);
								SetDlgItemInt(out_wnd[0].hWnd, IDC_PORT, Out->Port, 0);
								SendDlgItemMessage(out_wnd[0].hWnd, IDC_PASSWORD, EM_SETLIMITTEXT, ARRAYSIZE(Out->Password) - 1, 0);
								SetDlgItemText(out_wnd[0].hWnd, IDC_PASSWORD, Out->Password);
								int encval = OutEnc->Encoder;
								SendDlgItemMessage(out_wnd[0].hWnd, IDC_RECONNECT, BM_SETCHECK, Out->AutoRecon ? BST_CHECKED : BST_UNCHECKED, 0);
								SendDlgItemMessage(wnd[1].hWnd, IDC_AUTOCONNECT, BM_SETCHECK, OutEnc->AutoConnect ? BST_CHECKED : BST_UNCHECKED, 0);

								SetDlgItemInt(out_wnd[0].hWnd, IDC_TIMEOUT, Out->ReconTime, 0);

								EnableWindowDlgItem(out_wnd[0].hWnd, IDC_STATIONID, sc2mode);
								SendDlgItemMessage(out_wnd[0].hWnd, IDC_PROTOCOL, CB_SETCURSEL, (HIBYTE(Out->protocol) ? 0 : (sc2mode ? 1 : 2)), 0);


								// Output page 2
								SendDlgItemMessage(out_wnd[1].hWnd, IDC_PUBLIC, BM_SETCHECK, Out->Public ? BST_CHECKED : BST_UNCHECKED, 0);
								SendDlgItemMessage(out_wnd[1].hWnd, IDC_DESCRIPTION, EM_SETLIMITTEXT, ARRAYSIZE(Out->Description) - 1, 0);
								SetDlgItemText(out_wnd[1].hWnd, IDC_DESCRIPTION, Out->Description);
								SendDlgItemMessage(out_wnd[1].hWnd, IDC_SERVERURL, EM_SETLIMITTEXT, ARRAYSIZE(Out->ServerURL) - 1, 0);
								SetDlgItemText(out_wnd[1].hWnd, IDC_SERVERURL, Out->ServerURL);
								SendDlgItemMessage(out_wnd[1].hWnd, IDC_GENRE, EM_SETLIMITTEXT, ARRAYSIZE(Out->Genre) - 1, 0);
								SetDlgItemText(out_wnd[1].hWnd, IDC_GENRE, Out->Genre);
								SendDlgItemMessage(out_wnd[1].hWnd, IDC_AIM, EM_SETLIMITTEXT, ARRAYSIZE(Out->AIM) - 1, 0);
								SetDlgItemText(out_wnd[1].hWnd, IDC_AIM, Out->AIM);
								SendDlgItemMessage(out_wnd[1].hWnd, IDC_ICQ, EM_SETLIMITTEXT, ARRAYSIZE(Out->ICQ) - 1, 0);
								SetDlgItemText(out_wnd[1].hWnd, IDC_ICQ, Out->ICQ);
								SendDlgItemMessage(out_wnd[1].hWnd, IDC_IRC, EM_SETLIMITTEXT, ARRAYSIZE(Out->IRC) - 1, 0);
								SetDlgItemText(out_wnd[1].hWnd, IDC_IRC, Out->IRC);

								SendDlgItemMessage(out_wnd[1].hWnd, IDC_AUTOCONNECT, BM_SETCHECK, OutEnc->AutoConnect ? BST_CHECKED : BST_UNCHECKED, 0);

								// setup the handling of the encoder saving options
								SendDlgItemMessage(out_wnd[2].hWnd, IDC_SAVE_ENCODED_AUDIO_EDIT, EM_SETLIMITTEXT, ARRAYSIZE(OutEnc->stationArtPath) - 1, 0);
								SetDlgItemTextW(out_wnd[2].hWnd, IDC_SAVE_ENCODED_AUDIO_EDIT, OutEnc->saveEncodedPath);
								SendDlgItemMessage(out_wnd[2].hWnd, IDC_SAVE_ENCODED_AUDIO, BM_SETCHECK, OutEnc->saveEncoded ? BST_CHECKED : BST_UNCHECKED, 0);

								// setup the handling of the titles options
								SendDlgItemMessage(out_wnd[3].hWnd, IDC_TITLE, EM_SETLIMITTEXT, ARRAYSIZE(Out->Now) - 1, 0);
								SetDlgItemText(out_wnd[3].hWnd, IDC_TITLE, Out->Now);
								SendDlgItemMessage(out_wnd[3].hWnd, IDC_NEXT, EM_SETLIMITTEXT, ARRAYSIZE(Out->Next) - 1, 0);
								SetDlgItemText(out_wnd[3].hWnd, IDC_NEXT, Out->Next);
								CheckRadioButton(out_wnd[3].hWnd, IDC_NOTITLES, IDC_MANUALTITLE, (Out->doTitleUpdate ? (OutEnc->AutoTitle ? IDC_AUTOTITLE : IDC_MANUALTITLE) : IDC_NOTITLES));
								SendDlgItemMessage(out_wnd[3].hWnd, IDC_SENDNEXTTITLES, BM_SETCHECK, OutEnc->NextTitles ? BST_CHECKED : BST_UNCHECKED, 0);
								UpdateTitleControls();

								// setup the handling of the artwork options
								SendDlgItemMessage(out_wnd[4].hWnd, IDC_ART_EDIT, EM_SETLIMITTEXT, ARRAYSIZE(OutEnc->stationArtPath) - 1, 0);
								SetDlgItemTextW(out_wnd[4].hWnd, IDC_ART_EDIT, OutEnc->stationArtPath);
								SendDlgItemMessage(out_wnd[4].hWnd, IDC_USE_ART, BM_SETCHECK, OutEnc->useArt ? BST_CHECKED : BST_UNCHECKED, 0);
								SendDlgItemMessage(out_wnd[4].hWnd, IDC_USE_ART_PLAYING, BM_SETCHECK, OutEnc->usePlayingArt ? BST_CHECKED : BST_UNCHECKED, 0);
								SendDlgItemMessage(out_wnd[4].hWnd, IDC_USE_ART_STREAM, BM_SETCHECK, OutEnc->useStreamArt ? BST_CHECKED : BST_UNCHECKED, 0);
								EnableWindowDlgItem(out_wnd[4].hWnd, IDC_USE_ART_PLAYING, OutEnc->useArt);
								EnableWindowDlgItem(out_wnd[4].hWnd, IDC_USE_ART_STREAM, OutEnc->useArt);
								EnableWindowDlgItem(out_wnd[4].hWnd, IDC_ART_EDIT, OutEnc->useArt && OutEnc->useStreamArt);
								EnableWindowDlgItem(out_wnd[4].hWnd, IDC_ART_BROWSE, OutEnc->useArt && OutEnc->useStreamArt);
								UpdateArtworkMessage();

								// setup the handling of the next track logging option
								SendDlgItemMessage(out_wnd[5].hWnd, IDC_LOGGING, BM_SETCHECK, OutEnc->Logging ? BST_CHECKED : BST_UNCHECKED, 0);
								SendDlgItemMessage(out_wnd[5].hWnd, IDC_CLEAR_ON_STARTUP, BM_SETCHECK, OutEnc->LogCOS ? BST_CHECKED : BST_UNCHECKED, 0);

								SendDlgItemMessage(out_wnd[5].hWnd, IDC_NEXT_TRACK_LOG, BM_SETCHECK, OutEnc->nextTrackLog ? BST_CHECKED : BST_UNCHECKED, 0);
								SendDlgItemMessage(out_wnd[5].hWnd, IDC_NEXT_TRACK_XML, BM_SETCHECK, OutEnc->nextTrackLogXML ? BST_CHECKED : BST_UNCHECKED, 0);
								SendDlgItemMessage(out_wnd[5].hWnd, IDC_NEXT_TRACK_EDIT, EM_SETLIMITTEXT, ARRAYSIZE(OutEnc->nextTrackPath) - 1, 0);
								SetDlgItemTextW(out_wnd[5].hWnd, IDC_NEXT_TRACK_EDIT, OutEnc->nextTrackPath);
								EnableWindowDlgItem(out_wnd[5].hWnd, IDC_NEXT_TRACK_XML, OutEnc->nextTrackLog);
								EnableWindowDlgItem(out_wnd[5].hWnd, IDC_NEXT_TRACK_EDIT, OutEnc->nextTrackLog);
								EnableWindowDlgItem(out_wnd[5].hWnd, IDC_NEXT_TRACK_BROWSE, OutEnc->nextTrackLog);

								// this is sent to the encoders tab so it will update the selection for the current instance
								// note: this is a change in build 009 to remove the prior listbox and reduce ui inconsistency
								SendMessage(out_wnd[2].hWnd, WM_COMMAND, MAKEWPARAM(IDC_ENCODERLIST, LBN_SELCHANGE), (LPARAM) GetDlgItem(wnd[2].hWnd, IDC_ENCODERLIST));

								ini_modified = 1;
							}
						}
						break;

					case IDC_INPUTSETUP:
						{
							if (HIWORD(wParam) == CBN_SELCHANGE) {
								if (WaitForSingleObject(cf_mutex, INFINITE) == WAIT_OBJECT_0) {
									Crossfader->SetChannels(LineInputAttribs[(InputDevice == 0 ? 3 : Input_CurSelPos)].nch);
									Crossfader->SetSampleRate(LineInputAttribs[(InputDevice == 0 ? 3 : Input_CurSelPos)].srate);
									ReleaseMutex(cf_mutex);
								}

								int attrib = SendMessage((HWND) lParam, CB_GETCURSEL, 0, 0);
								if (attrib != Input_CurSelPos) {
									SuspendThread(hthread);
									Soundcard.Close();
									if (InputDevice == 1) {
										Input_CurSelPos = attrib;
									}
									for (int i = 0; i < NUM_ENCODERS; i++) {
										if (WaitForSingleObject(Enc_mutex[i], INFINITE) == WAIT_OBJECT_0) {
											C_ENCODER *Enc = Encoder[i].GetEncoder();
											if (Enc) {
												int infosize = sizeof (T_EncoderIOVals);
												T_EncoderIOVals *encset = (T_EncoderIOVals *) Enc->GetExtInfo(&infosize);
												if (encset && infosize) {
													T_EncoderIOVals *EncSettings = (T_EncoderIOVals *) malloc(infosize);
													memcpy(EncSettings, encset, infosize);

													if (strcmp(Enc->GetName(), "MP3 Encoder") == 0) {
														((T_ENCODER_MP3_INFO *) EncSettings)->input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
														((T_ENCODER_MP3_INFO *) EncSettings)->input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
													} else if (strcmp(Enc->GetName(), "Fraunhofer Encoder") == 0) {
														((T_ENCODER_FHGAAC_INFO *) EncSettings)->input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
														((T_ENCODER_FHGAAC_INFO *) EncSettings)->input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
													} else if (strcmp(Enc->GetName(), "AAC+ Encoder") == 0) {
														((T_ENCODER_AACP_INFO *) EncSettings)->input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
														((T_ENCODER_AACP_INFO *) EncSettings)->input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
													}
#ifdef USE_OGG
													else if (strcmp(Enc->GetName(), "OGG Vorbis Encoder") == 0) {
														((T_ENCODER_OGG_INFO *) EncSettings)->input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
														((T_ENCODER_OGG_INFO *) EncSettings)->input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
													}
#endif // USE_OGG
													Enc->ChangeSettings(EncSettings);
													free(EncSettings);
												}
											}
											ReleaseMutex(Enc_mutex[i]);
										}
									}
									Soundcard.Create((InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate), (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch));
									ResumeThread(hthread);
									ini_modified = 1;
								}
							}
						}
						break;

					case IDC_INPUT_WINAMP:
					case IDC_INPUT_SOUNDCARD:
						{
							// update the input mode from the summary page options
							HWND inputCtrl = GetDlgItem(wnd[2].hWnd, IDC_INPUTDEVICE);
							SendMessage(inputCtrl, CB_SETCURSEL, (LOWORD(wParam) - IDC_INPUT_WINAMP), 0);
							SendMessage(wnd[2].hWnd, WM_COMMAND, MAKEWPARAM(IDC_INPUTDEVICE,CBN_SELCHANGE), (LPARAM)inputCtrl);
						}
						break;

					case IDC_INPUTDEVICE:
						{
							if (HIWORD(wParam) == CBN_SELCHANGE) {
								int this_device = SendMessage((HWND) lParam, CB_GETCURSEL, 0, 0);
								if (InputDevice != this_device) {
									SuspendThread(hthread);
									Soundcard.Close();
									InputDevice = this_device;
									if (InputDevice == 0) { // winamp
										SendMessage(in_wnd[this_device].hWnd, WM_HSCROLL, 0, (LPARAM) GetDlgItem(in_wnd[this_device].hWnd, IDC_MUSSLIDER));
										SendMessage(in_wnd[this_device].hWnd, WM_HSCROLL, 0, (LPARAM) GetDlgItem(in_wnd[this_device].hWnd, IDC_MUS2SLIDER));
										SendMessage(in_wnd[this_device].hWnd, WM_HSCROLL, 0, (LPARAM) GetDlgItem(in_wnd[this_device].hWnd, IDC_MICSLIDER));
										SendMessage(in_wnd[this_device].hWnd, WM_HSCROLL, 0, (LPARAM) GetDlgItem(in_wnd[this_device].hWnd, IDC_FADESLIDER));
										SendMessage(in_wnd[this_device].hWnd, WM_HSCROLL, 0, (LPARAM) GetDlgItem(in_wnd[this_device].hWnd, IDC_MICFADESLIDER));
									}
									for (int i = 0; i < NUM_ENCODERS; i++) {
										if (WaitForSingleObject(Enc_mutex[i], INFINITE) == WAIT_OBJECT_0) {
											C_ENCODER *Enc = Encoder[i].GetEncoder();
											if (Enc) {
												int infosize = sizeof (T_EncoderIOVals);
												T_EncoderIOVals *encset = (T_EncoderIOVals *) Enc->GetExtInfo(&infosize);
												if (encset && infosize) {
													T_EncoderIOVals *EncSettings = (T_EncoderIOVals *) malloc(infosize);
													memcpy(EncSettings, encset, infosize);

													if (strcmp(Enc->GetName(), "MP3 Encoder") == 0) {
														((T_ENCODER_MP3_INFO *) EncSettings)->input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
														((T_ENCODER_MP3_INFO *) EncSettings)->input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
													} else if (strcmp(Enc->GetName(), "Fraunhofer Encoder") == 0) {
														((T_ENCODER_FHGAAC_INFO *) EncSettings)->input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
														((T_ENCODER_FHGAAC_INFO *) EncSettings)->input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
													} else if (strcmp(Enc->GetName(), "AAC+ Encoder") == 0) {
														((T_ENCODER_AACP_INFO *) EncSettings)->input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
														((T_ENCODER_AACP_INFO *) EncSettings)->input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
													}
#ifdef USE_OGG
													else if (strcmp(Enc->GetName(), "OGG Vorbis Encoder") == 0) {
														((T_ENCODER_OGG_INFO *) EncSettings)->input_numChannels = (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch);
														((T_ENCODER_OGG_INFO *) EncSettings)->input_sampleRate = (InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate);
													}
#endif // USE_OGG
													Enc->ChangeSettings(EncSettings);
													free(EncSettings);
												}
											}
											ReleaseMutex(Enc_mutex[i]);
										}
									}
									Soundcard.Create((InputDevice == 0 ? InputConfig.srate : LineInputAttribs[Input_CurSelPos].srate), (InputDevice == 0 ? InputConfig.nch : LineInputAttribs[Input_CurSelPos].nch));

									DisplayDeviceName();
									CheckRadioButton(wnd[0].hWnd, IDC_INPUT_WINAMP, IDC_INPUT_SOUNDCARD, (IDC_INPUT_WINAMP + InputDevice));

									peak_vu_l = peak_vu_r = -90;
									ResumeThread(hthread);
									ini_modified = 1;
								}
								SendDlgItemMessage(hDlg, IDC_INPUTSETUP, CB_RESETCONTENT, 0, 0);
								if (InputDevice == 1) {
									wchar_t temp[128];
									int num_input_items = ARRAYSIZE(LineInputAttribs);
									for (int i = 0; i < num_input_items; i++) {
										wchar_t tmp[32];
										StringCchPrintfW(temp, ARRAYSIZE(temp), LocalisedString(IDS_X_HZ_X, tmp, 32), LineInputAttribs[i].srate, LocalisedString(LineInputAttribs[i].nch == 1 ? IDS_MONO : IDS_STEREO, NULL, 0));
										SendDlgItemMessageW(hDlg, IDC_INPUTSETUP, CB_ADDSTRING, 0, (LPARAM) temp);
									}
									SendDlgItemMessage(hDlg, IDC_INPUTSETUP, CB_SETCURSEL, Input_CurSelPos, 0);
								}
								for (int i = 0; i < num_inwnds; i++) ShowWindow(in_wnd[i].hWnd, i == InputDevice && curtab == 2 ? SW_SHOW : SW_HIDE);
								SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_INPUTSETUP, CBN_SELCHANGE), (LPARAM) GetDlgItem(hDlg, IDC_INPUTSETUP));
								ShowWindowDlgItem(hDlg, IDC_INPUTSETUPSTATIC, InputDevice == 1);
								ShowWindowDlgItem(hDlg, IDC_INPUTSETUP, InputDevice == 1);
							}
						}
						break;

						// server box
					case IDC_ADDRESS:
						{
							if (HIWORD(wParam) == EN_UPDATE) {
								T_OUTPUT_CONFIG *Out = &Output[Connection_CurSelPos].Config;
								GetWindowText((HWND) lParam, Out->Address, ARRAYSIZE(Out->Address));
								ini_modified = 1;
							} else if (HIWORD(wParam) == EN_SETFOCUS) {
								PostMessage((HWND)lParam, EM_SETSEL, 0, (LPARAM)-1);
							}
						}
						break;

						// stream ID box
					case IDC_STATIONID:
						{
							if (HIWORD(wParam) == EN_UPDATE) {
								BOOL success = FALSE;
								int value = GetDlgItemInt(hDlg, LOWORD(wParam), &success, TRUE);
								T_OUTPUT_CONFIG *Out = &Output[Connection_CurSelPos].Config;
								GetWindowText((HWND) lParam, Out->StationID, ARRAYSIZE(Out->StationID));
								// check and set the default as required
								if (!Out->StationID[0] || (success && value < 1 || !success && value == 0)) {
									SetWindowTextW((HWND) lParam, L"1");
									lstrcpyn(Out->StationID, "1", ARRAYSIZE(Out->StationID));
								} else if (Out->StationID[0] && (success && value > 2147483647)) {
									SetWindowTextW((HWND) lParam, L"2147483647");
									lstrcpyn(Out->StationID, "2147483647", ARRAYSIZE(Out->StationID));
								}
								ini_modified = 1;
							} else if (HIWORD(wParam) == EN_SETFOCUS) {
								PostMessage((HWND)lParam, EM_SETSEL, 0, (LPARAM)-1);
							}
						}
						break;

					case IDC_USERID:
						{
							if (HIWORD(wParam) == EN_UPDATE) {
								T_OUTPUT_CONFIG *Out = &Output[Connection_CurSelPos].Config;
								GetWindowText((HWND) lParam, Out->UserID, ARRAYSIZE(Out->UserID));
								ini_modified = 1;
							} else if (HIWORD(wParam) == EN_SETFOCUS) {
								PostMessage((HWND)lParam, EM_SETSEL, 0, (LPARAM)-1);
							}
						}
						break;

						// server port
					case IDC_PORT:
						{
							if (HIWORD(wParam) == EN_UPDATE) {
								BOOL success = FALSE;
								int value = GetDlgItemInt(hDlg, LOWORD(wParam), &success, TRUE);
								T_OUTPUT_CONFIG *Out = &Output[Connection_CurSelPos].Config;
								// check and set the default as required
								if ((success && value < 1 || !success && value == 0)) {
									SetWindowTextW((HWND) lParam, L"8000");
									Out->Port = 8000;
								} else if ((success && value > 65535)) {
									SetWindowTextW((HWND) lParam, L"65535");
									Out->Port = 65535;
								} else {
									Out->Port = value;
								}

								ini_modified = 1;
							} else if (HIWORD(wParam) == EN_SETFOCUS) {
								PostMessage((HWND)lParam, EM_SETSEL, 0, (LPARAM)-1);
							}
						}
						break;

						// password
					case IDC_PASSWORD:
						{
							if (HIWORD(wParam) == EN_UPDATE) {
								T_OUTPUT_CONFIG *Out = &Output[Connection_CurSelPos].Config;
								GetWindowText((HWND) lParam, Out->Password, ARRAYSIZE(Out->Password));
								ini_modified = 1;
							} else if (HIWORD(wParam) == EN_SETFOCUS) {
								PostMessage((HWND)lParam, EM_SETSEL, 0, (LPARAM)-1);
							}
						}
						break;

					case IDC_SEND:
						{
							EnableWindowDlgItem(hDlg, IDC_SEND, FALSE);
							MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
							if (Out->Encoder != -1 && Out->Handle != -1) {
								wchar_t title[1024] = {0}, next[1024] = {0};
								SHOUTCAST_OUTPUT *Enc = &Encoder[Out->Encoder];
								GetWindowTextW(GetDlgItem(out_wnd[3].hWnd, IDC_TITLE), title, ARRAYSIZE(title));
								GetWindowTextW(GetDlgItem(out_wnd[3].hWnd, IDC_NEXT), next, ARRAYSIZE(next));
								ini_modified = 1;
								if (WaitForSingleObject(Enc_mutex[Out->Encoder], INFINITE) == WAIT_OBJECT_0) {
									std::vector<std::wstring> nextList;
									std::vector<int> nextListIdx;
									nextList.clear();
									nextListIdx.clear();
									if (((Out->AutoTitle == 1 && Out->NextTitles) || Out->AutoTitle == 0) && next[0]) {
										nextList.push_back(next);
										nextListIdx.push_back(-1);
									}

									if (Out->nextTrackLog) {
										WriteNextTracks(Connection_CurSelPos, module.hwndParent, nextListIdx, nextList, !!Out->nextTrackLogXML);
									}

									// check if in v2 mode and have a next title specified so as to change the send flag
									Enc->UpdateTitle(title, nextList, Out->Handle, !!nextList.size(), false);
									ReleaseMutex(Enc_mutex[Out->Encoder]);
								}
							}
						}
						break;

					case IDC_TITLE:
						{
							if (HIWORD(wParam) == EN_UPDATE) {
								int length = GetWindowTextLength((HWND)lParam);
								EnableWindowDlgItem(out_wnd[3].hWnd, IDC_SEND, (length > 0));
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								EnableWindowDlgItem(out_wnd[3].hWnd, IDC_NEXT,
													(length > 0 && (LOBYTE(Out->Config.protocol) != 1)));

								char temp[sizeof(Out->Config.Now)];
								GetWindowText((HWND) lParam, temp, ARRAYSIZE(temp));
								if (strcmp(temp, Out->Config.Now) != 0) {
									lstrcpyn(Out->Config.Now, temp, ARRAYSIZE(Out->Config.Now));
									ini_modified = 1;
								}
							}
						}
						break;

					case IDC_NEXT:
						{
							if (HIWORD(wParam) == EN_UPDATE) {
								EnableWindowDlgItem(out_wnd[3].hWnd, IDC_SEND, TRUE);
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];

								char temp[sizeof(Out->Config.Next)];
								GetWindowText((HWND) lParam, temp, ARRAYSIZE(temp));
								if (strcmp(temp, Out->Config.Next) != 0) {
									lstrcpyn(Out->Config.Next, temp, ARRAYSIZE(Out->Config.Next));
									ini_modified = 1;
								}
							}
						}
						break;

					case IDC_TIMEOUT:
						{
							if (HIWORD(wParam) == EN_UPDATE) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								char temp[128] = {0};
								GetWindowText((HWND) lParam, temp, ARRAYSIZE(temp));

								int rt = atoi(temp);
								if (rt < 1) {
									rt = 5;
									SetDlgItemInt(out_wnd[0].hWnd, IDC_TIMEOUT, 5, 0);
								}

								if (Out->Config.ReconTime != rt) {
									Out->Config.ReconTime = rt;
									if (Out->Encoder) {
										if (WaitForSingleObject(Enc_mutex[Out->Encoder], INFINITE) == WAIT_OBJECT_0) {
											Encoder[Out->Encoder].UpdateOutput(Out->Handle);
											ReleaseMutex(Enc_mutex[Out->Encoder]);
										}
									}
									ini_modified = 1;
								}
							} else if (HIWORD(wParam) == EN_SETFOCUS) {
								PostMessage((HWND)lParam, EM_SETSEL, 0, (LPARAM)-1);
							}
						}
						break;

					// these will reconnect the Output on edit
					case IDC_PUBLIC:
						{
							if (HIWORD(wParam) == BN_CLICKED) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								Out->Config.Public = SendMessage((HWND) lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;

								// force a refresh on selection change
								lastMode[Connection_CurSelPos] = -1;

								ini_modified = 1;
								if (Out->Encoder != -1 && Out->Handle != -1) {
									SHOUTCAST_OUTPUT *Enc = &Encoder[Out->Encoder];
									if (WaitForSingleObject(Enc_mutex[Out->Encoder], INFINITE) == WAIT_OBJECT_0) {
										Enc->DisconnectOutput(Out->Handle, 1, 5);
										ReleaseMutex(Enc_mutex[Out->Encoder]);
									}
								}
							}
						}
						break;

					case IDC_GENRES:
						{
							// build up a menu to allow the user to select only supported genres for use with YP
							if (HIWORD(wParam) == BN_CLICKED) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								HMENU hmenu = CreatePopupMenu(), submenu = 0;
								RECT r;
								GetWindowRect((HWND)lParam, &r);

								for (unsigned int i = 0; i < ARRAYSIZE(genres); i++) {
									MENUITEMINFO mii = {sizeof(mii), MIIM_ID | MIIM_STATE | MIIM_TYPE, MFT_STRING, 0, 1+i, 0, 0, 0, 0, 0, 0};
									bool reAdd = false;

									// fix up genres with & in it to work around menu accelerator display quirks
									std::string str = genres[i].name;
									if (str.find("&") != std::string::npos)
										str.replace(str.find("&"),1,"&&");
									mii.dwTypeData = (LPSTR)str.c_str();

									if (genres[i].parent) {
										if (genres[i].children) {
											reAdd = true;
											mii.fMask |= MIIM_SUBMENU;
											submenu = mii.hSubMenu = CreatePopupMenu();
										}
									}

									if (reAdd == false && !strcmpi(Out->Config.Genre, genres[i].name)) {
										mii.fState = MFS_CHECKED;
									}

									InsertMenuItem((genres[i].parent ? hmenu : submenu), i, TRUE, &mii);

									if (reAdd == true) {
										mii.fMask -= MIIM_SUBMENU;
										if (!strcmpi(Out->Config.Genre, genres[i].name)) {
											mii.fState = MFS_CHECKED;
										}
										InsertMenuItem(submenu, i, TRUE, &mii);
									}
								}

								int ret = TrackPopupMenu(hmenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, r.left, r.bottom, 0, (HWND)hDlg, NULL);
								if (ret > 0) {
									int update = 0;
									ret -= 1;
									if (strcmp(genres[ret].name, Out->Config.Genre) != 0) {
										update = 1;
										SetDlgItemText(hDlg, IDC_GENRE, genres[ret].name);
										lstrcpyn(Out->Config.Genre, genres[ret].name, ARRAYSIZE(Out->Config.Genre));
										ini_modified = 1;
									}
									if (update && Out->Encoder != -1 && Out->Handle != -1) {
										SHOUTCAST_OUTPUT *Enc = &Encoder[Out->Encoder];
										if (WaitForSingleObject(Enc_mutex[Out->Encoder], INFINITE) == WAIT_OBJECT_0) {
											Enc->DisconnectOutput(Out->Handle, 1, 5);
											ReleaseMutex(Enc_mutex[Out->Encoder]);
										}
									}
								}

								DestroyMenu(hmenu);
							}
						}
						break;

					case IDC_DESCRIPTION:
						{
							if (HIWORD(wParam) == EN_UPDATE) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								char temp[sizeof (Out->Config.Description)];
								int update = 0;
								GetWindowText((HWND) lParam, temp, ARRAYSIZE(temp));
								if (strcmp(temp, Out->Config.Description) != 0) {
									update = 1;
									lstrcpyn(Out->Config.Description, temp, ARRAYSIZE(Out->Config.Description));
									ini_modified = 1;
								}
								if (update && Out->Encoder != -1 && Out->Handle != -1) {
									SHOUTCAST_OUTPUT *Enc = &Encoder[Out->Encoder];
									if (WaitForSingleObject(Enc_mutex[Out->Encoder], INFINITE) == WAIT_OBJECT_0) {
										Enc->DisconnectOutput(Out->Handle, 1, 5);
										ReleaseMutex(Enc_mutex[Out->Encoder]);
									}
								}
							} else if (HIWORD(wParam) == EN_SETFOCUS) {
								PostMessage((HWND)lParam, EM_SETSEL, 0, (LPARAM)-1);
							}
						}
						break;

					case IDC_SERVERURL:
						{
							if (HIWORD(wParam) == EN_UPDATE) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								char temp[sizeof (Out->Config.ServerURL)];
								int update = 0;
								GetWindowText((HWND) lParam, temp, ARRAYSIZE(temp));
								if (strcmp(temp, Out->Config.ServerURL) != 0) {
									update = 1;
									lstrcpyn(Out->Config.ServerURL, temp, ARRAYSIZE(Out->Config.ServerURL));
									ini_modified = 1;
								}
								if (update && Out->Encoder != -1 && Out->Handle != -1) {
									SHOUTCAST_OUTPUT *Enc = &Encoder[Out->Encoder];
									if (WaitForSingleObject(Enc_mutex[Out->Encoder], INFINITE) == WAIT_OBJECT_0) {
										Enc->DisconnectOutput(Out->Handle, 1, 5);
										ReleaseMutex(Enc_mutex[Out->Encoder]);
									}
								}
							} else if (HIWORD(wParam) == EN_SETFOCUS) {
								PostMessage((HWND)lParam, EM_SETSEL, 0, (LPARAM)-1);
							}
						}
						break;

					case IDC_AIM:
						{
							if (HIWORD(wParam) == EN_UPDATE) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								char temp[sizeof (Out->Config.AIM)];
								int update = 0;
								GetWindowText((HWND) lParam, temp, ARRAYSIZE(temp));
								if (strcmp(temp, Out->Config.AIM) != 0) {
									update = 1;
									lstrcpyn(Out->Config.AIM, temp, ARRAYSIZE(Out->Config.AIM));
									ini_modified = 1;
								}
								if (update && Out->Encoder != -1 && Out->Handle != -1) {
									SHOUTCAST_OUTPUT *Enc = &Encoder[Out->Encoder];
									if (WaitForSingleObject(Enc_mutex[Out->Encoder], INFINITE) == WAIT_OBJECT_0) {
										Enc->DisconnectOutput(Out->Handle, 1, 5);
										ReleaseMutex(Enc_mutex[Out->Encoder]);
									}
								}
							} else if (HIWORD(wParam) == EN_SETFOCUS) {
								PostMessage((HWND)lParam, EM_SETSEL, 0, (LPARAM)-1);
							}
						}
						break;

					case IDC_ICQ:
						{
							if (HIWORD(wParam) == EN_UPDATE) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								char temp[sizeof (Out->Config.ICQ)];
								int update = 0;
								GetWindowText((HWND) lParam, temp, ARRAYSIZE(temp));
								if (strcmp(temp, Out->Config.ICQ) != 0) {
									update = 1;
									lstrcpyn(Out->Config.ICQ, temp, ARRAYSIZE(Out->Config.ICQ));
									ini_modified = 1;
								}
								if (update && Out->Encoder != -1 && Out->Handle != -1) {
									SHOUTCAST_OUTPUT *Enc = &Encoder[Out->Encoder];
									if (WaitForSingleObject(Enc_mutex[Out->Encoder], INFINITE) == WAIT_OBJECT_0) {
										Enc->DisconnectOutput(Out->Handle, 1, 5);
										ReleaseMutex(Enc_mutex[Out->Encoder]);
									}
								}
							} else if (HIWORD(wParam) == EN_SETFOCUS) {
								PostMessage((HWND)lParam, EM_SETSEL, 0, (LPARAM)-1);
							}
						}
						break;

					case IDC_IRC:
						{
							if (HIWORD(wParam) == EN_UPDATE) {
								MY_T_OUTPUT *Out = &Output[Connection_CurSelPos];
								char temp[sizeof (Out->Config.IRC)];
								int update = 0;
								GetWindowText((HWND) lParam, temp, ARRAYSIZE(temp));
								if (strcmp(temp, Out->Config.IRC) != 0) {
									update = 1;
									lstrcpyn(Out->Config.IRC, temp, ARRAYSIZE(Out->Config.IRC));
									ini_modified = 1;
								}
								if (update && Out->Encoder != -1 && Out->Handle != -1) {
									SHOUTCAST_OUTPUT *Enc = &Encoder[Out->Encoder];
									if (WaitForSingleObject(Enc_mutex[Out->Encoder], INFINITE) == WAIT_OBJECT_0) {
										Enc->DisconnectOutput(Out->Handle, 1, 5);
										ReleaseMutex(Enc_mutex[Out->Encoder]);
									}
								}
							} else if (HIWORD(wParam) == EN_SETFOCUS) {
								PostMessage((HWND)lParam, EM_SETSEL, 0, (LPARAM)-1);
							}
						}
						break;
				}
			}
			break;

		case WM_CTLCOLORSTATIC:
			{
				// this is used to update the header text of the options which need to be checked if there is a config error...
				int id = GetDlgCtrlID((HWND)lParam);
				if (id == IDC_ADDRESS_HEADER || id == IDC_PASSWORD_HEADER || id == IDC_NAME_HEADER || id == IDC_ENCODER_HEADER) {
					int header_id[] = {0, 0, 0, IDC_ADDRESS_HEADER, IDC_PASSWORD_HEADER, IDC_NAME_HEADER, IDC_ENCODER_HEADER, 0};
					if (lastMode[Connection_CurSelPos] >= 3 && lastMode[Connection_CurSelPos] <= 6 &&
						header_id[lastMode[Connection_CurSelPos]] == id) {
						SetTextColor((HDC)wParam, RGB(255,0,0));
					}
				}
			}
			break;

		case WM_NOTIFY:
			{
				LPNMHDR pnmh = (LPNMHDR) lParam;
				switch (LOWORD(wParam)) {
					case IDC_TAB:
						if (pnmh->code == TCN_SELCHANGE) {
							int i;
							KillTimer(hDlg, wnd[curtab].id);
							curtab = SendMessage(pnmh->hwndFrom, TCM_GETCURSEL, 0, 0);

							// send this to update the tab just incase we're showing invalid items
							InvalidateRect(pnmh->hwndFrom, 0, 0);

							ini_modified = 1;
							if (wnd[curtab].timer_freq != 0) SetTimer(hDlg, wnd[curtab].id, wnd[curtab].timer_freq, NULL);
							for (i = 0; i < num_tabwnds; i++) ShowWindow(wnd[i].hWnd, curtab == i ? SW_SHOW : SW_HIDE);
							for (i = 0; i < num_inwnds; i++) ShowWindow(in_wnd[i].hWnd, i == InputDevice && curtab == 2 ? SW_SHOW : SW_HIDE);
							for (i = 0; i < num_outwnds; i++) ShowWindow(out_wnd[i].hWnd, i == curouttab && curtab == 1 ? SW_SHOW : SW_HIDE);
							// update the summary details when going back to it
							if (curtab == 0) {
								UpdateSummaryDetails(ListView_GetNextItem(GetDlgItem(wnd[0].hWnd, IDC_OUTPUTSTATUS), -1, LVNI_SELECTED));
							} else if(curtab == 1) {
								// force a refresh on selection change
								lastMode[Connection_CurSelPos] = -1;
							}
						}
						break;

					case IDC_CONTAB:
						if (pnmh->code == TCN_SELCHANGE) {
							int i;
							KillTimer(hDlg, out_wnd[curouttab].id);
							curouttab = SendMessage(pnmh->hwndFrom, TCM_GETCURSEL, 0, 0);

							// send this to update the tab just incase we're showing invalid items
							InvalidateRect(pnmh->hwndFrom, 0, 0);

							ini_modified = 1;
							if (out_wnd[curouttab].timer_freq != 0) SetTimer(hDlg, out_wnd[curouttab].id, out_wnd[curouttab].timer_freq, NULL);
							for (i = 0; i < num_outwnds; i++) ShowWindow(out_wnd[i].hWnd, i == curouttab ? SW_SHOW : SW_HIDE);
							bool enable = (LOBYTE(Output[Connection_CurSelPos].Config.protocol) == 1);
							EnableWindowDlgItem(out_wnd[1].hWnd, IDC_AIM, enable);
							EnableWindowDlgItem(out_wnd[1].hWnd, IDC_IRC, enable);
							EnableWindowDlgItem(out_wnd[1].hWnd, IDC_ICQ, enable);
						}
						break;

					case IDC_METALIST:
						if (pnmh->code == NM_DBLCLK) {
							LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;
							if (lpnmitem->iItem != -1 && WASABI_API_EXPLORERFINDFILE) {
								wchar_t fn[MAX_PATH]= {0};
								lstrcpynW(fn, lastFile, MAX_PATH);
								if (!PathIsURLW(fn)) {
									// this will attempt to find the path of the parent folder of the file selected
									// as spc files in a rsn archive can display album art (with the compatibility
									// wrapper) and so it should cope with such scenarios...
									wchar_t *filews = fn + lstrlenW(fn) - 1;
									while(filews && *filews && (*filews != L'.') && (filews != fn)){filews = CharPrevW(fn,filews);}
									while(filews && *filews && (*filews != L',' && *filews != L':')){filews = CharNextW(filews);}
									if (filews) *filews = 0;

									filews = wcsstr(fn,L".rsn\\");
									if(filews) {
										*(filews+4) = 0;
									}

									WASABI_API_EXPLORERFINDFILE->AddFile(fn);
									WASABI_API_EXPLORERFINDFILE->ShowFiles();
								}
							}
						}
						break;

					case IDC_OUTPUTSTATUS:
						// on double-click go to the output tab and select the output we used as the currently shown
						if (pnmh->code == NM_DBLCLK) {
							LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;
							if (lpnmitem->iItem != -1 && lpnmitem->iSubItem != 0) {
								// only change the viewed output mode if it is different, otherwise just switch tab
								if (Connection_CurSelPos != lpnmitem->iItem) {
									Connection_CurSelPos = lpnmitem->iItem;

									// force a refresh on selection change
									lastMode[Connection_CurSelPos] = -1;

									SendDlgItemMessage(wnd[1].hWnd, IDC_OUTPUTLIST, LB_SETCURSEL, Connection_CurSelPos, 0);
									SendMessage(wnd[1].hWnd, WM_COMMAND, MAKEWPARAM(IDC_OUTPUTLIST, LBN_SELCHANGE), (LPARAM) GetDlgItem(wnd[1].hWnd, IDC_OUTPUTLIST));
									SetFocus(GetDlgItem(wnd[1].hWnd, IDC_OUTPUTLIST));
								}
								SetTab(1, hMainDLG, IDC_TAB);
							}
						} else if (pnmh->code == LVN_ITEMCHANGED) {
 							// on single-click / keyboard change, show some information about the stream such as most, encoder, etc (makes the page useful)
							LPNMLISTVIEW lpnmitem = (LPNMLISTVIEW) lParam;
							if (lpnmitem->iItem != -1) {
								UpdateSummaryDetails(lpnmitem->iItem);
							}
						} else if(pnmh->code == LVN_KEYDOWN) {
							LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN) lParam;
							// toggle state in the output list via 'space'
							if (pnkd->wVKey == VK_SPACE) {
								int item = ListView_GetNextItem(GetDlgItem(wnd[0].hWnd, IDC_OUTPUTSTATUS), -1, LVNI_SELECTED);
								if (lastEnable[item]) {
									int oldCurSelPos = Connection_CurSelPos;
									Connection_CurSelPos = item;
									SendMessage(wnd[1].hWnd, WM_COMMAND, MAKEWPARAM(IDC_CONNECT, BN_CLICKED), (LPARAM)GetDlgItem(wnd[1].hWnd, IDC_CONNECT));
									Connection_CurSelPos = oldCurSelPos;
								}
							}
						}
					break;
				}
			}
			break;

		case WM_HSCROLL:
			{
				int curpos = SendMessage((HWND) lParam, TBM_GETPOS, 0, 0);
				if (GetDlgItem(hDlg, IDC_MUSSLIDER) == (HWND) lParam) {
					MusVol = curpos;
					if (InputDevice == 1 && !FadeOut) setlev(MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT, MusVol * 10);
					wchar_t tmp[256] = {0};
					if (curpos != 0) {
						wchar_t temp[32] = {0};
						int volume = (int) (20 * log10(curpos * 3276.));
						StringCchPrintfW(tmp, ARRAYSIZE(tmp), LocalisedString(IDS_X_DB, temp, 32), volume - 90);
					} else {
						LocalisedString(IDS_INF_DB, tmp, 256);
					}
					SetDlgItemTextW(hDlg, IDC_MUSLEV1_TEXT, tmp);
				} else if (GetDlgItem(hDlg, IDC_MUS2SLIDER) == (HWND) lParam) {
					Mus2Vol = curpos;
					if (InputDevice == 1 && FadeOut) setlev(MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT, Mus2Vol * 10);
					wchar_t tmp[256] = {0};
					if (curpos != 0) {
						wchar_t temp[32] = {0};
						int volume = (int) (20 * log10(curpos * 3276.));
						StringCchPrintfW(tmp, ARRAYSIZE(tmp), LocalisedString(IDS_X_DB, temp, 32), volume - 90);
					} else {
						LocalisedString(IDS_INF_DB, tmp, 256);
					}
					SetDlgItemTextW(hDlg, IDC_MUSLEV2_TEXT, tmp);
				} else if (GetDlgItem(hDlg, IDC_MICSLIDER) == (HWND) lParam) {
					MicVol = curpos;
					int micsrc = Input_Device_ID  >= 1 ? MIXERLINE_COMPONENTTYPE_SRC_LINE : MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE;
					// changed this so it will only change the capture device level if PTT is pressed
					if (InputDevice == 1 && FadeOut) setlev(micsrc, MicVol *10);
					wchar_t tmp[256] = {0};
					if (curpos != 0) {
						wchar_t temp[32] = {0};
						int volume = (int) (20 * log10(curpos * 3276.));
						StringCchPrintfW(tmp, ARRAYSIZE(tmp), LocalisedString(IDS_X_DB, temp, 32), volume - 90);
					} else {
						LocalisedString(IDS_INF_DB, tmp, 256);
					}
					SetDlgItemTextW(hDlg, IDC_MICLEV_TEXT, tmp);
				} else if (GetDlgItem(hDlg, IDC_FADESLIDER) == (HWND) lParam) {
					FadeTime = curpos;
					wchar_t tmp[256] = {0}, temp[32] = {0};
					StringCchPrintfW(tmp, ARRAYSIZE(tmp), LocalisedString(IDS_X_MS, temp, 32), curpos * 100);
					SetDlgItemTextW(hDlg, IDC_FADETIME_TEXT, tmp);
				} else if (GetDlgItem(hDlg, IDC_MICFADESLIDER) == (HWND) lParam) {
					MicFadeTime = curpos;
					wchar_t tmp[256] = {0}, temp[32] = {0};
					StringCchPrintfW(tmp, ARRAYSIZE(tmp), LocalisedString(IDS_X_MS, temp, 32), curpos * 100);
					SetDlgItemTextW(hDlg, IDC_MICFADETIME_TEXT, tmp);
				}
			}
			break;
	}

	if (FALSE != DirectMouseWheel_ProcessDialogMessage(hDlg, uMsg, wParam, lParam)) {
		return TRUE;
	}
	return 0;
}

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
	LPCWPSTRUCT msg = (LPCWPSTRUCT)lParam;
		// catch the new file playing message and update the cached metadata
		if (msg->message == WM_WA_IPC) {
			if (msg->lParam == IPC_PLAYING_FILEW) {
			DWORD diff = GetTickCount();
				was_paused = 0;
				play_duration = 0;
				play_diff = diff;
				was_playing = 1;
				if (wcsnicmp(lastFile, (wchar_t*)msg->wParam, MAX_PATH)) {
					PostMessage(hMainDLG, WM_USER, 0, nowPlayingID);
				}
			}
		}
	}
	return (nowPlayingHook ? CallNextHookEx(nowPlayingHook, nCode, wParam, lParam) : 0);
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
	LPMSG msg = (LPMSG)lParam;
		// catch the new file playing message and update the cached metadata
		if (msg->message == WM_WA_IPC) {
			if (msg->lParam == IPC_CB_MISC && msg->wParam == IPC_CB_MISC_STATUS) {
				isplaying = SendMessage(module.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING);
				ProcessPlayingStatusUpdate();
			} else if (msg->lParam == IPC_UPDTITLE) {
				// attempt to keep a track of other title updates
				// e.g. the re-streaming an already playing stream
				wchar_t currentFile[MAX_PATH] = {0};
				wchar_t *file=(wchar_t*)SendMessage(module.hwndParent, WM_WA_IPC,
													SendMessage(module.hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS),
													IPC_GETPLAYLISTFILEW);
				wcsncpy(currentFile, (file ? file : L""), MAX_PATH);
				if (!wcsnicmp(currentFile, lastFile, MAX_PATH)) {
					// do a 1 second delay since it's likely Winamp will send
					// this a few times so we reset the timer everytime so we
					// only do a proper title update once everything settles
					KillTimer(hMainDLG, 1337);
					SetTimer(hMainDLG, 1337, 1000, NULL);
				}
			}
		}
	}
	return (nowPlayingHook2 ? CallNextHookEx(nowPlayingHook2, nCode, wParam, lParam) : 0);
}

VOID CALLBACK TimerProc2(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, idEvent);
	SetForegroundWindow(hMainDLG);
}

void doConfig(HWND hwndParent) {
	if (WASABI_API_SVC) {
		if (!IsWindow(hMainDLG)) {
			if (AGAVE_API_CONFIG) {
				if (AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"bits", 16) > 16) {
					wchar_t title[128] = {0}, message[512] = {0};
					StringCchPrintfW(title, ARRAYSIZE(title), LocalisedString(IDS_PLUGIN_NAME, NULL, 0), APP_VersionW);
					StringCchPrintfW(message, ARRAYSIZE(message), LocalisedString(IDS_24BIT_MODE_DETECTED, NULL, 0));
					MessageBoxW(module.hwndParent, message, title, MB_ICONWARNING);
					return;
				}
			}

			// using this to lessen issues with new track events with higher bitrates leading to failures
			if (!nowPlayingHook) {
				nowPlayingHook = SetWindowsHookExW(WH_CALLWNDPROC, CallWndProc, instance, GetCurrentThreadId());
			}
			if (!nowPlayingHook2) {
				nowPlayingHook2 = SetWindowsHookExW(WH_GETMESSAGE, GetMsgProc, instance, GetCurrentThreadId());
			}
			if (nowPlayingID == -1) {
				nowPlayingID = SendMessage(hwndParent, WM_WA_IPC, (WPARAM)&"dsp_sc_np", IPC_REGISTER_WINAMP_IPCMESSAGE);
			}

			HWND hwnd = LocalisedCreateDialog(instance, IDD_DIALOG, hwndParent, DialogFunc, IDD_DIALOG);
			SetWindowPos(hwnd, HWND_TOP, mainrect.left, mainrect.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
			SetTimer(hMainDLG, 999, 1, TimerProc2);
		} else {
			if (IsIconic(hMainDLG)) {
				DialogFunc(hMainDLG, WM_SIZE, SIZE_RESTORED, 0);
				ShowWindow(hMainDLG, SW_RESTORE);
				ShowWindow(hMainDLG, SW_SHOW);
				SetActiveWindow(hMainDLG);
			}
			SetForegroundWindow(hMainDLG);
		}
	}
}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, idEvent);
	doConfig(hwnd);
}

void Config(winampDSPModule *this_mod) {
	// this will hold back opening the config dialog on loading until Winamp is in a ready state
	// this resolves a partial fail to load i've often been seeing (plus from some users afaict)
	SetTimer(this_mod->hwndParent, 999, 1, TimerProc);
}

int Init(winampDSPModule *this_mod) {
	instance = this_mod->hDllInstance;

	// this will hold back opening the config dialog on loading until Winamp is in a ready state
	// this resolves a partial fail to load i've often been seeing (plus from some users afaict)
	SetTimer(this_mod->hwndParent, 999, 1, TimerProc);
	return 1;
}

int ModifySamples(winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate) {
	int numorig = numsamples;

	//connect but only if we're meant to be i.e. there's at least 1 active output
	if (InputDevice == 0) {
		if (WaitForSingleObject(cf_mutex, INFINITE) == WAIT_OBJECT_0) {
			// CT> Resample into the desired srate and nch if needed
			// TODO check out the handling of this for 24-bit output
			short cf_buf[256 * 1024] = {0};
			if (srate != InputConfig.srate || nch != InputConfig.nch) {
				char *s = (char *) samples;
				int ns = numsamples * 2;
				if (InputConfig.nch == 1) {
					if (nch != 1 || bps != 16 || srate != (int) InputConfig.srate) {
						if (nch == 2) {
							int x;
							int nns = MulDiv(numsamples, InputConfig.srate, srate);
							int r = 0;
							int dr = MulDiv(numsamples, 1 << 12, nns);
							if (bps == 16)
							{
								for (x = 0; x < nns; x++) {
									cf_buf[x] = samples[(r >> 12)*2] / 2 + samples[(r >> 12)*2 + 1] / 2;
									r += dr;
								}
							}
							else
							{
								for (x = 0; x < nns; x++) {
									cf_buf[x] = ((((char *) samples)[(r >> 12)*2]^128) << 8) / 2 +
										((((char *) samples)[(r >> 12)*2 + 1]^128) << 8) / 2;
									r += dr;
								}
							}
							ns = nns * 2;
						} else {
							int x;
							int nns = MulDiv(numsamples, InputConfig.srate, srate);
							int r = 0;
							int dr = MulDiv(numsamples, 1 << 12, nns);
							if (bps == 16)
							{
								for (x = 0; x < nns; x++) {
									cf_buf[x] = samples[r >> 12];
									r += dr;
								}
							}
							else
							{
								for (x = 0; x < nns; x++) {
									cf_buf[x] = (((char *) samples)[r >> 12]^128) << 8;
									r += dr;
								}
							}
							ns = nns * 2;
						}
						s = (char *) cf_buf;
					}
				} else {
					if (nch != 2 || bps != 16 || srate != (int) InputConfig.srate) {
						if (nch == 2) { 
							int x;
							int nns = MulDiv(numsamples, InputConfig.srate, srate);
							int r = 0;
							int dr = MulDiv(numsamples, 1 << 12, nns);
							if (bps == 16)
							{
								for (x = 0; x < nns; x++) {
									cf_buf[x * 2] = samples[(r >> 12)*2];
									cf_buf[x * 2 + 1] = samples[(r >> 12)*2 + 1];
									r += dr;
								}
							}
							else
							{
								for (x = 0; x < nns; x++) {
									cf_buf[x * 2] = (((char *) samples)[(r >> 12)*2]^128) << 8;
									cf_buf[x * 2 + 1] = (((char *) samples)[(r >> 12)*2 + 1]^128) << 8;
									r += dr;
								}
								ns = nns * 4;
							}
						} else {
							int x;
							int nns = MulDiv(numsamples, InputConfig.srate, srate);
							int r = 0;
							int dr = MulDiv(numsamples, 1 << 12, nns);
							if (bps == 16)
							{
								for (x = 0; x < nns; x++) {
									cf_buf[x * 2] = cf_buf[x * 2 + 1] = samples[r >> 12];
									r += dr;
								}
							}
							else
							{
								for (x = 0; x < nns; x++)
								{
									cf_buf[x * 2] = cf_buf[x * 2 + 1] = (((char *) samples)[r >> 12]^128) << 8;
									r += dr;
								}
								ns = nns * 4;
							}
						}

						s = (char *) cf_buf;
					}
					else ns *= 2;
				}
				samples = (short *) s;
				numsamples = ns / (InputConfig.nch * 2);
			}

			if (!VU.update) {
				for (int j = 0; j < numsamples; j++) {
					if (VU.vu_l < samples[j]) VU.vu_l = samples[j];
					if (InputConfig.nch == 2) {
						if (VU.vu_r < samples[j + 1]) VU.vu_r = samples[j + 1];
						j++;
					}
				}
				if (InputConfig.nch == 1) VU.vu_r = VU.vu_l;
				VU.update = 1;
			}
			Crossfader->put(samples, numsamples);
			ReleaseMutex(cf_mutex);
		}
	}
	return numorig;
}


void Quit(winampDSPModule *this_mod) {
	doQuit();
}