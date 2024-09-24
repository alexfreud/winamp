/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: 
 ** Created:
 **/

#include "Main.h"
#include "resource.h"
#include "strutil.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoCharFn.h"
#include "MediaCoreCOM.h"
#include "externalCOM.h"
#include "JSAPI2_CallbackManager.h"

static int in_play;
extern int no_notify_play;


void Play(const wchar_t *playstring)
{
	int i = -31337;
	static int a = -2;
	AutoCharFn charFn(playstring);

	// added in 5.64 - this helps to keep the currently playing file after
	// a tag update to stay playing correctly when it is no longer in the
	// main playlist editor - this helps to keep a consistent ui experience
	bool restart = false;
	if (no_notify_play && playstring && *playstring && !PlayList_getlength())
	{
		PlayList_insert(0, playstring);
		restart = true;
	}

	in_play = 1;

	g_video_numaudiotracks = 1;
	g_video_curaudiotrack = 0;

	g_stopaftercur = 0;
	CheckMenuItem(main_menu, WINAMP_BUTTON4_CTRL, MF_UNCHECKED);
	CheckMenuItem(GetSubMenu(top_menu, 3), WINAMP_BUTTON4_CTRL, MF_UNCHECKED);

	g_has_deleted_current = 0;
	Skin_Random();

	Stats_OnPlay(playstring);

	// has cropped up in a few crash reports where app_name is null which causes the update to bork
	// when receiving a 'stop' command and we try to send an update via JSAPI1_CurrentTitleChanged()
	if (!app_name || app_name && !*app_name || (unsigned int)(ULONG_PTR)app_name < 65536)
		BuildAppName();

	set_caption(0, L"%s - %S", (config_dotasknum?FileTitleNum:FileTitle), app_name); // TODO: benski> get rid of FileTitle here
	eq_autoload(charFn);

	if (a == -2)
		a = PlayList_getlength();

	if (!*playstring || (i = in_open(playstring)))
	{
		if (*playstring && i != -31337 && --a) //-31337 == no_sound_card
		{
			PostMessageW(hMainWindow, WM_WA_MPEG_EOF, 0, 0);
		}
		else
		{
			if (i == -31337) in_mod = 0;
			StopPlaying(0);
			a = -2;
		}
		in_play = 0;
		return ;
	}
	else
	{
		a = -2;
	}

	SendMessageW(hMainWindow, WM_TIMER, UPDATE_DISPLAY_TIMER + 4, 0);
	PlayList_refreshtitle();
	PlayList_GetCurrentTitle(FileTitle, FILETITLE_SIZE); // TODO: benski> don't want to be using global FileTitle here
	// has cropped up in a few crash reports where app_name is null which causes the update to bork
	// when receiving a 'stop' command and we try to send an update via JSAPI1_CurrentTitleChanged()
	if (!app_name || app_name && !*app_name || (unsigned int)(ULONG_PTR)app_name < 65536) BuildAppName();
	set_caption(0, L"%s - %S", (config_dotasknum?FileTitleNum:FileTitle), app_name);
	if (in_mod && in_mod->is_seekable) draw_positionbar(0, 0);
	plEditSelect(PlayList_getPosition());
	SetTimer(hMainWindow, UPDATE_DISPLAY_TIMER, 100, NULL);
	playing = 1;
	draw_playicon(1);
	ui_songposition = 0;
	draw_songname(FileTitle, &ui_songposition, in_getlength());
	if (config_visplugin_autoexec && !vis_running())
		vis_start(hMainWindow, NULL);
	if (!dsp_isactive()) dsp_init();
	paused = 0;

	// added in 5.64
	if(restart) PlayList_deleteitem(0);

	if (!no_notify_play)
	{
		// don't tell anyone we're playing the file is there are hidden items or there's an ad curtain
		if (PlayList_hasanycurtain(PlayList_getPosition()) == 0 && PlayList_ishidden(PlayList_getPosition()) == 0)
		{
			SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)(char *)charFn, IPC_PLAYING_FILE);
			SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)playstring, IPC_PLAYING_FILEW);
			SendNotifyMessage(HWND_BROADCAST, songChangeBroadcastMessage, 0, 0);
		}
	}

	in_play = 0;

	PostMessageW(hMainWindow, WM_WA_IPC, IPC_CB_MISC_STATUS, IPC_CB_MISC);

	MediaCoreCOM *mediaCore;
	if (SUCCEEDED(JSAPI1_GetMediaCoreCOM(&mediaCore)))
	{
		mediaCore->OnPlay();
		mediaCore->Release();
	}

	JSAPI2::callbackManager.OnPlay(FileName);
}

void StartPlaying()
{
	// reset this flag when we start playing otherwise
	// it can cause it to be incorrectly applied later
	plcleared = 0;
	if (playing) StopPlaying(0);
	if (in_play) return ;
	{
		// Check if we have the media library , if we do check to see if we have and ad curtain
		// If we do, and the current item is the first item, display it
		const char *adcurtain = PlayList_getcurtain(PlayList_getPosition());
		if (adcurtain && *adcurtain)
		{
			if (*adcurtain == 'a')
			{
				// 'a' means it's an Ad, so pop out of fullscreen.
				if ( videoIsFullscreen() ) videoForceFullscreenOff();
				adcurtain = CharNextA(adcurtain); // get past the a in ahttp://....
			}
		}
	}

	if (!no_notify_play)
	{
		PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
	}
	Play(FileName);
}

/* -------- Stoppping -------- */

// TODO: avoid passing g_quit
void ClassicSkin_OnStop(int g_quit)
{
	if (playing)
	{
		KillTimer(hMainWindow, UPDATE_DISPLAY_TIMER);
	}

	if (!g_quit)
	{
		sa_setthread(0);
		if (!no_notify_play)
		{
			PlayList_getcurrent_onstop(FileName, FileTitle);
			if (*FileTitle)
			{
				wchar_t titleStr[64] = {0};
				set_caption(0, getStringW(IDS_TITLE_ON_STOP,titleStr,64), (config_dotasknum?FileTitleNum:FileTitle), app_name);
			}
			else
			{
				// has cropped up in a few crash reports where app_name is null which causes the update to bork
				// when receiving a 'stop' command and we try to send an update via JSAPI1_CurrentTitleChanged()
				if (!app_name || app_name && !*app_name || (unsigned int)(ULONG_PTR)app_name < 65536) BuildAppName();
				set_caption(0, L"%S %S", (!app_name || app_name && !*app_name ? "Winamp" : app_name), app_version_string);
			}

			plEditSelect(PlayList_getPosition());
		}

		draw_setnoupdate(1);
		draw_clear();
		draw_monostereo(0);
		draw_clutterbar(0);
		draw_playicon(2);
		draw_shuffle(config_shuffle, 0);
		draw_eject(0);
		draw_eqplbut(config_eq_open, 0, config_pe_open, 0);
		draw_repeat(config_repeat, 0);
		draw_volumebar(config_volume, 0);
		draw_panbar(config_pan, 0);
		draw_buttonbar( -1);
		if (config_pe_height != 14)
			draw_pe_timedisp(NULL, 0, 0, 0, 1);
		draw_songname(FileTitle, &ui_songposition, PlayList_getcurrentlength());
		draw_setnoupdate(0);
		last_brate = -1;
	}
}

void StopPlaying(int g_quit)
{
	if (in_play) return ;
	in_play = 1;
	g_has_deleted_current = 0;

	int last_time = 0;
	if (!g_fullstop) // if this wasn't an EOF situation, grab the current time so that the callback can be told the stop time.
		last_time = in_getouttime();
	
	if (playing)
	{
		paused = 0;
		playing = 0;
		g_video_numaudiotracks = 1;
		g_video_curaudiotrack = 0;
		in_close();
	}

	ClassicSkin_OnStop(g_quit);

	in_play = 0;

	stopPlayingInfoStruct infoStruct;
	infoStruct.g_fullstop = g_fullstop;
	infoStruct.last_time = last_time;
	SendMessageW(hMainWindow, WM_WA_IPC, (WPARAM)&infoStruct, IPC_STOPPLAYING);

	PostMessageW(hMainWindow, WM_WA_IPC, IPC_CB_MISC_STATUS, IPC_CB_MISC);
	
	MediaCoreCOM *mediaCore;
	if (SUCCEEDED(JSAPI1_GetMediaCoreCOM(&mediaCore)))
	{
		mediaCore->OnStop(last_time, g_fullstop);
		mediaCore->Release();
	}

	JSAPI2::callbackManager.OnStop(last_time, g_fullstop);
}

void Skin_OnPause()
{
	wchar_t titleStr[64] = {0};
	set_caption(0, getStringW(IDS_TITLE_ON_PAUSE,titleStr,64), (config_dotasknum?FileTitleNum:FileTitle), app_name);
	draw_playicon(4);
}

void Skin_OnUnpause()
{
	// has cropped up in a few crash reports where app_name is null which causes the update to bork
	// when receiving a 'stop' command and we try to send an update via JSAPI1_CurrentTitleChanged()
	if (!app_name || app_name && !*app_name || (unsigned int)(ULONG_PTR)app_name < 65536) BuildAppName();
	set_caption(0, L"%s - %S", (config_dotasknum?FileTitleNum:FileTitle), app_name);
	draw_playicon(1);
}

/* -------- Pausing -------- */
void PausePlaying()
{
	if (!playing || paused) return ;

	paused = 1;
	in_pause(1);

	Skin_OnPause();
	PostMessageW(hMainWindow, WM_WA_IPC, IPC_CB_MISC_STATUS, IPC_CB_MISC);
	PostMessageW(hMainWindow, WM_WA_IPC, IPC_CB_MISC_PAUSE, IPC_CB_MISC);
	
	MediaCoreCOM *mediaCore;
	if (SUCCEEDED(JSAPI1_GetMediaCoreCOM(&mediaCore)))
	{
		mediaCore->OnPause();
		mediaCore->Release();
	}

	JSAPI2::callbackManager.OnPause(true);
}

void UnPausePlaying()
{
	if (!playing || !paused) return ;

	in_pause(0);
	paused = 0;

	Skin_OnUnpause();
	PostMessageW(hMainWindow, WM_WA_IPC, IPC_CB_MISC_STATUS, IPC_CB_MISC);
	PostMessageW(hMainWindow, WM_WA_IPC, IPC_CB_MISC_UNPAUSE, IPC_CB_MISC);
	
	MediaCoreCOM *mediaCore = 0;
	if (SUCCEEDED(JSAPI1_GetMediaCoreCOM(&mediaCore)) && mediaCore)
	{
		mediaCore->OnResume();
		mediaCore->Release();
	}

	JSAPI2::callbackManager.OnPause(false);
}


void PlayThing(const char *thing, int clearlist)
{
}

void BeginPlayback()
{
	PlayList_setposition(0);
	if (config_shuffle)
		PlayList_randpos( -BIGINT);
	PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
	StartPlaying();
}