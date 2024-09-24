/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "wa_dlg.h"
#include "./api.h" 

// Destroy handler
 int Main_OnDestroy(HWND hwnd)
{
	if (NULL != WASABI_API_APP) WASABI_API_APP->app_unregisterGlobalWindow(hwnd);
	if (g_main_created)
	{
		SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
		Main_OnEndSession(NULL,TRUE);
		PostQuitMessage(0);
	}
	return 1;
}

int Main_OnClose(HWND hwnd)
{
	if (g_exit_disabled)
	{
		if (NULL != WASABI_API_APP) 
			WASABI_API_APP->main_cancelShutdown();
		return 0;
	}
	ReleaseCapture();

	if (!SendMessageW(hwnd,WM_WA_IPC,0,IPC_HOOK_OKTOQUIT))
	{
		if (NULL != WASABI_API_APP) 
			WASABI_API_APP->main_cancelShutdown();
		return 0;
	}

	if (playing) StopPlaying(1);
	sa_setthread(-1);
	vis_stop();
	dsp_quit();

	if (pTaskbar3 != NULL) 
		pTaskbar3->Release();
	pTaskbar3=0;
	
	if (toolbarIcons)
		ImageList_Destroy(toolbarIcons);
	toolbarIcons = 0;

	DestroyWindow(hwnd);
	return 0;
}

void Main_OnEndSession(HWND hwnd, BOOL fEnding)
{
	// TODO try to make this match normal shutdown...
	if (fEnding)
	{
		EndFullscreenAppMonitor();
		if (playing) StopPlaying(1);
		sa_setthread(-1);
		vis_stop();
		dsp_quit();

		if (!hwnd && systray_intray) systray_restore();

		hEQWindow=hPLWindow=/*hMBWindow=*/hVideoWindow=0;	
		hVisWindow=0;
		hPLVisWindow=0;	

		draw_kill();

		draw_finalquit();
		WADlg_close();
		Skin_CleanupZip();

		// is possible the prefs window was open but has already been destroyed
		// so don't use IsWindow(..) here and instead just look if its non-null
		if (prefs_hwnd)
		{
			prefs_hwnd = 0;
			config_write(2);
		}
		else config_write(1);

		stats_write();
  		PlayList_destroy();
  		PlayList_randpos(-666);
		DestroyMenu(top_menu);

		if (hwnd) unload_genplugins();
		out_deinit();
		in_deinit();

		Lang_EndLangSupport();
		Lang_CleanupZip();

		// if we're working on a delayed saving of language pack change then now
		if(config_langpack2[0]){
			// if < is the buffer contents then we're setting it back to the base support
			if(config_langpack2[0] == '<') config_langpack2[0] = 0;
			lstrcpynW(config_langpack,config_langpack2,MAX_PATH);
			config_save_langpack_var();
		}
	}
}