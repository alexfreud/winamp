#include <precomp.h>
#include "wa2cfgitems.h"
#include "wa2wndembed.h"
#include "wa2core.h"
#include <api/config/options.h>
#include "wa2frontend.h"
#include "../../Plugins/Output/out_ds/ds_ipc.h"
#include <bfc/parse/pathparse.h>
#include "gen.h"
#include "resource.h"
#include "../Agave/Language/api_language.h"


// cfg item callbacks
void setCrossfader(int crossfade);
void onWa2CrossfadeChange(int crossfade);
void setOnTop(int ontop);
void onWa2OnTopChanged(int ontop);

int my_set = 0;
int disable_set_wa2_repeat = 0;

//---------------------------------------------------------
// playlist editor cfg items
//---------------------------------------------------------
_bool shuffle(L"Shuffle", FALSE);
_int repeat(L"Repeat", 0);
_bool visrandom(L"Random", FALSE);

Wa2PlEditCfgItems::Wa2PlEditCfgItems() : CfgItemI(L"Playlist Editor", pleditWndGuid) {
  registerAttribute(&shuffle, new int_attrCB(Wa2PlEditCfgItems::onSetShuffle));
  registerAttribute(&repeat, new int_attrCB(Wa2PlEditCfgItems::onSetRepeat));
  WASABI_API_CONFIG->config_registerCfgItem(this);
  shuffle.setValueAsInt(g_Core->getShuffle());
  disable_set_wa2_repeat = 1;
  repeat.setValueAsInt(g_Core->getRepeat());
  disable_set_wa2_repeat = 0;
}

Wa2PlEditCfgItems::~Wa2PlEditCfgItems() {
  WASABI_API_CONFIG->config_deregisterCfgItem(this);
}

void Wa2PlEditCfgItems::onSetRepeat(int set) {
  my_set = 1;
  ASSERT(g_Core != NULL);
  if (!disable_set_wa2_repeat) {
    // if (!!set != !!g_Core->getRepeat()) // FG> do NOT uncomment this
      g_Core->setRepeat(set);
  }
  my_set = 0;
}

void Wa2PlEditCfgItems::onSetShuffle(BOOL set) {
  my_set = 1;
  ASSERT(g_Core != NULL);
  if (!!set != !!g_Core->getShuffle())
    g_Core->setShuffle(set);
  my_set = 0;
}

//---------------------------------------------------------
static _bool prevent_videostoponclose(L"Prevent video playback Stop on video window Close", FALSE);
static _bool prevent_videoresize(L"Prevent video resize", FALSE); // 5.32+
static _bool show_videownd_on_play(L"Show video wnd on play"); // 5.36+
static _bool hide_videownd_on_stop(L"Hide video wnd on stop"); // 5.36+
int preventvc_savedvalued = -1;
int preventvresize_savedvalued = -1;

SkinTweaks::SkinTweaks() : CfgItemI(L"SkinTweaks", skinTweaksGuid) 
{
  registerAttribute(&prevent_videostoponclose, new int_attrCB(SkinTweaks::onPreventVideoStopChanged));
	registerAttribute(&prevent_videoresize, new int_attrCB(SkinTweaks::onPreventVideoResize));
}

//---------------------------------------------------------
AvsCfg::AvsCfg() : CfgItemI(L"Avs", avs_guid) {
  registerAttribute(&visrandom, new int_attrCB(AvsCfg::onVisRandomChanged));
}

//---------------------------------------------------------
void SkinTweaks::onPreventVideoStopChanged(BOOL set) {
  if (set) {
    if (preventvc_savedvalued == -1) {
      preventvc_savedvalued = wa2.getStopOnVideoClose();
      wa2.setStopOnVideoClose(0);
    } 
  } else {
    if (preventvc_savedvalued != -1) {
      wa2.setStopOnVideoClose(preventvc_savedvalued);
      preventvc_savedvalued = -1;
    } 
  }
}

void SkinTweaks::onPreventVideoResize(BOOL set) {
  if (set) {
    if (preventvresize_savedvalued == -1) {
      preventvresize_savedvalued = wa2.GetVideoResize();
      wa2.SetVideoResize(0);
    } 
  } else {
    if (preventvresize_savedvalued != -1) {
      wa2.SetVideoResize(preventvresize_savedvalued);
      preventvresize_savedvalued = -1;
    } 
  }
}


//---------------------------------------------------------
void AvsCfg::onVisRandomChanged(BOOL set) {
  extern int disable_send_visrandom;
  if (!disable_send_visrandom) {
    disable_send_visrandom = 1;
    wa2.visRandom(set);
    disable_send_visrandom = 0;
  }
}

//---------------------------------------------------------
Options *options;
CustomOptionsMenuItems *optionsmenuitems;
CustomWindowsMenuItems *windowsmenuitems;
SkinTweaks *skintweaks;
Crossfader *crossfader;
AvsCfg *avscfg;
//---------------------------------------------------------

Wa2CfgItems::Wa2CfgItems() {
  // set up main options
  WASABI_API_CONFIG->config_registerCfgItem((options = new Options()));
  
  // set up custom options menu items
  WASABI_API_CONFIG->config_registerCfgItem((optionsmenuitems = new CustomOptionsMenuItems()));

  // set up custom windows menu items
  WASABI_API_CONFIG->config_registerCfgItem((windowsmenuitems = new CustomWindowsMenuItems()));

  // set up skintweaks options menu items
  WASABI_API_CONFIG->config_registerCfgItem((skintweaks = new SkinTweaks()));

  // set up crossfader options menu item
  WASABI_API_CONFIG->config_registerCfgItem((crossfader = new Crossfader()));

  // set up avs cfg item 
  WASABI_API_CONFIG->config_registerCfgItem((avscfg = new AvsCfg()));
}

Wa2CfgItems::~Wa2CfgItems() {
  WASABI_API_CONFIG->config_deregisterCfgItem(options);
  WASABI_API_CONFIG->config_deregisterCfgItem(optionsmenuitems);
  WASABI_API_CONFIG->config_deregisterCfgItem(windowsmenuitems);
  WASABI_API_CONFIG->config_deregisterCfgItem(skintweaks);
  WASABI_API_CONFIG->config_deregisterCfgItem(crossfader);
  WASABI_API_CONFIG->config_deregisterCfgItem(avscfg);
  delete options;
  options = NULL;
  delete optionsmenuitems;
  optionsmenuitems = NULL;
  delete windowsmenuitems;
  windowsmenuitems = NULL;
  delete skintweaks;
  skintweaks = NULL;
  delete crossfader;
  crossfader = NULL;
  delete avscfg;
  avscfg = NULL;
}

//-----------------------------------------------------------------------------------------------
// always on top was toggled in freeform skin
//-----------------------------------------------------------------------------------------------
void setOnTop(int ontop) {
  if (!!ontop != !!wa2.isOnTop()) wa2.setOnTop(!!ontop);
}

//-----------------------------------------------------------------------------------------------
// setontop was toggled in wa2 
//-----------------------------------------------------------------------------------------------
void onWa2OnTopChanged(int ontop) {
  cfg_options_alwaysontop.setValueAsInt(!!ontop);
}

//-----------------------------------------------------------------------------------------------
// Crossfader cfgitem - monitors out_ds
//-----------------------------------------------------------------------------------------------
_int crossfade_time(L"Crossfade time", 2);	// in seconds

HWND output_ipc = NULL;
static WNDPROC oldOutputIPCProc;
int syncing_crossfade = 0;
int using_dsound = 0;
extern HWND last_dlg_parent;

// sync out_ds with gen_ff
void syncOutputCrossfade() {
  if (output_ipc == NULL) return;
  SendMessageW(output_ipc, WM_DS_IPC, crossfade_time.getValueAsInt() * 1000, DS_IPC_SET_CROSSFADE_TIME);
  if (using_dsound) SendMessageW(output_ipc, WM_DS_IPC, cfg_audiooptions_crossfader.getValueAsInt(), DS_IPC_SET_CROSSFADE);
}

// sync gen_ff with out_ds, can't happen if out_ds isn't the selected plugin since this occurs when the user changes his config, and he needs to select out_ds to access its config
void syncGenFFCrossfade() {
  syncing_crossfade = 1;
  if (output_ipc == NULL) return;
  crossfade_time.setValueAsInt(SendMessageW(output_ipc, WM_DS_IPC, 0, DS_IPC_GET_CROSSFADE_TIME) / 1000);
  cfg_audiooptions_crossfader.setValueAsInt(SendMessageW(output_ipc, WM_DS_IPC, 0, DS_IPC_GET_CROSSFADE));
  syncing_crossfade = 0;
}

// called when gen_ff changes the crossfade time setting
void onSetCrossfadeTime(int sectime) {
  if (syncing_crossfade) return;
  syncOutputCrossfade();
}

// called when gen_ff toggles crossfading
void setCrossfader(int crossfade) {
  if (syncing_crossfade) return;
  if (crossfade && (!output_ipc || !using_dsound)) {
    cfg_audiooptions_crossfader.setValueAsInt(0);
	char title[256] = {0};
	int r = MessageBoxA(last_dlg_parent ? last_dlg_parent : wa2.getMainWindow(),
					   WASABI_API_LNGSTRING(IDS_CROSSFADER_ONLY_UNDER_OUT_DS),
					   WASABI_API_LNGSTRING_BUF(IDS_NOT_SUPPORTED,title,256), MB_YESNO);
    if (r == IDYES) {
      SendMessageW(wa2.getMainWindow(),WM_WA_IPC,32,IPC_OPENPREFSTOPAGE);
    }
  }
  syncOutputCrossfade();
}

// ----------------------- ipc hook 
extern int m_are_we_loaded;

static DWORD WINAPI outputIPCProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
  if (m_are_we_loaded) {
    switch (uMsg) {
      case WM_DS_IPC:
        switch (lParam) {
          case DS_IPC_CB_CFGREFRESH:
            syncGenFFCrossfade();
            break;
          case DS_IPC_CB_ONSHUTDOWN:
            output_ipc = NULL;
            break;
        }
        break;
    }
  }
  return CallWindowProc(oldOutputIPCProc, hwndDlg, uMsg, wParam, lParam);
}

void hookOutputIPC() {
  oldOutputIPCProc = (WNDPROC)SetWindowLongPtrW(output_ipc, GWLP_WNDPROC, (LONG_PTR)outputIPCProc);
}

// not called in fact, since we never need to let go of it
void unhookOutputIPC() {
  SetWindowLongPtrW(output_ipc, GWLP_WNDPROC, (LONG_PTR)oldOutputIPCProc);
}

BOOL CALLBACK findOutputIPCProc( HWND hwnd, LPARAM lParam )
{
    char cn[ 256 ] = { 0 };
    GetClassNameA( hwnd, cn, 255 ); cn[ 255 ] = 0;   // Must stay in ANSI mode
    if ( STRCASEEQL( cn, DS_IPC_CLASSNAME ) )
    {
        output_ipc = hwnd;
        return FALSE;
    }
    return TRUE;
}

// ------------------------- output plugin changed !

void Crossfader::onOutputChanged() {
  if (!(output_ipc && IsWindow(output_ipc))) {
    output_ipc = NULL;
    EnumChildWindows(wa2.getMainWindow(), findOutputIPCProc, 0);

    if (output_ipc) {
      hookOutputIPC();
    }
  }

  const char* outputPluginPath = wa2.getOutputPlugin();
  PathParser pp(outputPluginPath);
  using_dsound = STRCASEEQLSAFE(pp.getLastString(), "out_ds.dll");
  if (!using_dsound) {
    cfg_audiooptions_crossfader.setValueAsInt(0);
  } else
    syncGenFFCrossfade();
}

// ------------------------------------------------------------------------

Crossfader::Crossfader() : CfgItemI(L"Crossfader", crossfaderGuid) {
  registerAttribute(&crossfade_time, new int_attrCB(onSetCrossfadeTime));
}

Crossfader::~Crossfader() {
  unhookOutputIPC();
}

