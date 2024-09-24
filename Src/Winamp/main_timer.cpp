#include "main.h"
#include "stats.h"

// Display update, and menu popping up (vs killing)
LRESULT Main_OnTimer(HWND hwnd, UINT id)
{
#ifdef BENSKI_TEST_WM_PRINTCLIENT
	if (id == 9999)
	{
		PrintWindow(hPLWindow);
	}
#endif
	if (id == 99)
	{
		// 250 ms after a fs application has settled on
		KillTimer(hMainWindow, 99);
		g_dropaot_timer_set = 0;
		dropAOT();
		return 0;
	}
	if (id == 100)
	{
		// 250 ms after a fs application has settled off
		KillTimer(hMainWindow, 100);
		g_restoreaot_timer_set = 0;
		restoreAOT();
		return 0;
	}
	if (id == 101)
	{
		KillTimer(hMainWindow, 101);
		// 250ms after wm_displaychange, enforce config_keeponscreen, but not if the display change was because of a
		// fullscreen app, like a game
		if (!g_fsapp) set_aot(-1);
	}
	if (id == 200)
	{
		KillTimer(hMainWindow, 200);
		if (!deferring_show)
			return 0;
		deferring_show = 0;
		if (g_showcode == SW_SHOWMINIMIZED && config_taskbar)
		{
			ShowWindow(hMainWindow, SW_SHOWNORMAL);
		}
		ShowWindow(hMainWindow, g_showcode);
		return 0;
	}
	if (id == STATS_TIMER)
	{
		stats.IncrementStat(Stats::TIME_RUNNING);
		if (playing) stats.IncrementStat(Stats::TIME_PLAYING);
		if (!config_minimized) stats.IncrementStat(Stats::TIME_VISIBLE);
		if (!config_minimized && playing) stats.IncrementStat(Stats::TIME_VISIBLE_PLAYING);
		//if (config_mb_open && !config_minimized) stats_timemb++;
		//if (config_mb_open && !config_minimized && playing) stats_timembplaying++;
	}
	// removing the playing only check on this so rating menu updates will appear even when stopped
	if ((id == UPDATE_DISPLAY_TIMER) || (id == UPDATE_DISPLAY_TIMER + 2) || (id == UPDATE_DISPLAY_TIMER + 4))
	{
		if (playing)
		{
			int a = (in_getouttime()) / 1000;
			int l = in_getlength();
			if (!config_minimized && (config_mw_open || config_pe_open))
			{
				static int t = -1;
				static int la = -123;
				static int ll = -15055;
				if (paused)
				{
					if (t == -1) t = 10;
					else t--;
				}
				else t = -1;
				if (a != la || l != ll || (paused && t == 10) || id == UPDATE_DISPLAY_TIMER + 4) ui_drawtime(a, 0);
				la = a;
				ll = l;
			}
			if (g_brate != last_brate)
				draw_bitmixrate(last_brate = g_brate, g_srate);
		}
		if (g_need_titleupd || (id == UPDATE_DISPLAY_TIMER + 2))
		{
			if (id == UPDATE_DISPLAY_TIMER + 2)
				KillTimer(hMainWindow, id);

			g_need_titleupd = 0;
			PlayList_getcurrent_tupdate(FileName, FileTitle);

			{
				if (!do_volbar_active && !do_posbar_active && !do_panbar_active)
					draw_songname(FileTitle, &ui_songposition, playing ? in_getlength() : PlayList_getcurrentlength());
			}
			set_caption(!playing, L"%s - %S", (config_dotasknum?FileTitleNum:FileTitle), app_name);
			plEditSelect(PlayList_getPosition() | (1 << 30));
		}
		if (playing && g_need_infoupd)
		{
			int r = g_need_infoupd;
			g_need_infoupd = 0;

			if (r & 8)
			{
				last_brate = g_brate;
				draw_bitmixrate(g_brate, g_srate);
				draw_monostereo(g_nch);
			}
			if (r & 2)
				draw_playicon(4);
			else if (r & 1)
				draw_playicon(1);
			else
				draw_playicon(8);
		}
	}
	if (id == UPDATE_DISPLAY_TIMER + 1 && !config_minimized && !config_windowshade && config_mw_open)
		if (config_autoscrollname&1)
		{
			ui_doscrolling();
		}
	if (id == 666)
	{
		KillTimer(hwnd, 666);
		SendMessageW(hwnd, WM_COMMAND, WINAMP_MAINMENU, 0);
	}
	if (id == UPDATE_DISPLAY_TIMER + 1)
	{
		if (config_autoscrollname&2)
			do_caption_autoscroll();
	}
	return 1;
}
