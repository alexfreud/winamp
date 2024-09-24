/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "Main.h"
#include "resource.h"
extern "C" extern int g_stopaftercur;

// button 1 (prev) functions
int Main_OnButton1(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if (id == WINAMP_BUTTON1_SHIFT)	SendMessageW(hwnd, WM_COMMAND, WINAMP_REW5S, 0);
	else if (id == WINAMP_BUTTON1_CTRL)
	{
		PlayList_setposition(0);
		if (config_shuffle) PlayList_randpos( -BIGINT);
		PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
		if (playing) StartPlaying();
		else StopPlaying(0);
	}
	else
	{
		int nitem = SendMessageW(hMainWindow, WM_WA_IPC, 0, IPC_GET_PREVIOUS_PLITEM);
		if (nitem != -1)
		{
			PlayList_setposition(nitem);
			PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
			StartPlaying();
		}
		else
		{
			int s = 1;
			if (!config_shuffle && PlayList_advance( -1) < 0)
			{
				s = 0;
				if (config_repeat)
				{
					s = 1;
					PlayList_advance(BIGINT);
				}
			}
			if (s)
			{
				if (PlayList_getlength())
				{
					if (config_shuffle)
					{
						if (PlayList_randpos( -1)) return 1;
					}
					PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
					if (playing)
					{
						StopPlaying(0);
						StartPlaying();
					}
					else StopPlaying(0);
				}
			}
		}
	}
	return 1;
} // Main_OnButton1

// button 2 (play) functions
int Main_OnButton2(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	PlayList_resetcurrent();
	if (!playing)
	{
		if (!no_notify_play)
		{
			FileName[0] = 0;
			PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
		}
		if (id == WINAMP_BUTTON2_CTRL)
			SendMessageW(hwnd, WM_COMMAND, WINAMP_FILE_LOC, 0);
		else if (!FileName[0] || id == WINAMP_BUTTON2_SHIFT)
		{
			SendMessageW(hwnd, WM_COMMAND, WINAMP_FILE_PLAY, 0);
		}
		else
		{
			StartPlaying();
		}
	}
	else
	{
		if (id == WINAMP_BUTTON2_CTRL)
			SendMessageW(hwnd, WM_COMMAND, WINAMP_FILE_LOC, 0);
		else if (id == WINAMP_BUTTON2_SHIFT)
			SendMessageW(hwnd, WM_COMMAND, WINAMP_FILE_PLAY, 0);
		else if (paused) UnPausePlaying();
		else
		{
			PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
			StartPlaying();
		}
	}
	return 1;
} // Main_OnButton2

// button 3 (pause) functions
int Main_OnButton3(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if (playing)
	{
		if (paused) UnPausePlaying();
		else PausePlaying();
	}
	return 1;
}

// button 4 (stop) functions
int Main_OnButton4(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if (playing)
	{
		if (id == WINAMP_BUTTON4_CTRL)
		{
			HMENU m;
			int r;
			g_stopaftercur = !g_stopaftercur;
			r = g_stopaftercur ? MF_CHECKED : MF_UNCHECKED;
			CheckMenuItem(main_menu, WINAMP_BUTTON4_CTRL, r);
			m = GetSubMenu(top_menu, 3);
			CheckMenuItem(m, WINAMP_BUTTON4_CTRL, r);
			return 1;
		}
		if (id == WINAMP_BUTTON4_SHIFT && !paused)
		{
//			double v=0;
			int x = 256;

			int v = config_volume;
//			double v = (double)(int)config_volume;
			//double dv;// = -v / 256.;//(double)x;
			int delay = 8;//2000 / x;
//			v = (double)config_volume;
			//dv = -v / 256.;
			while (x--)
			{
				if (v < 0) v = 0;
				in_setvol((int)v);
			//	v += dv;
				v--;
				Sleep(delay);
			}
			Sleep(100);
			PausePlaying();
			in_flush(in_getouttime());
			in_setvol(config_volume);

		}

		StopPlaying(0);
	}
	return 1;
}

// button 5 (next) fucntions
int Main_OnButton5(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if (id == WINAMP_BUTTON5_SHIFT)
		SendMessageW(hwnd, WM_COMMAND, WINAMP_FFWD5S, 0);
	else if (id == WINAMP_BUTTON5_CTRL)
	{
		PlayList_setposition(BIGINT);
		if (config_shuffle)
		{
			PlayList_randpos( -BIGINT);
			PlayList_randpos(PlayList_getlength() - 1);
		}
		PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
		if (playing) StartPlaying();
		else StopPlaying(0);
	}
	else
	{
		int nitem = SendMessageW(hMainWindow, WM_WA_IPC, 0, IPC_GET_NEXT_PLITEM);
		if (nitem != -1)
		{
			PlayList_setposition(nitem);
			PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
			StartPlaying();
		}
		else if (!config_shuffle && PlayList_advance(1) < 0)
		{
			if (config_repeat)
		  	{
				PlayList_setposition(0);
				PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
				if (playing)
				{
  					StopPlaying(0);
	  				StartPlaying();
		  		}
			  	else StopPlaying(0);
			}
		}
		else
		{
			if (!PlayList_getlength()) return 1;
			if (config_shuffle)
			{
				int lp = PlayList_getPosition();
				int q = PlayList_randpos(1);
				if (q || PlayList_getlength() == 1)
				{
					if (!config_repeat) return 1;
					PlayList_randpos( -BIGINT);
				}
				if (PlayList_getPosition() == lp && PlayList_getlength() > 1)
				{
					PlayList_randpos(1);
				}
			}
			else
			{
				// 5.64 - if pledit is cleared, shuffle is off & we're playing
				// then we set playing to go back to the start of the playlist
				// as we get complaints it'll go to #2 instead of #1 as shown.
				if (plcleared)
				{
					plcleared = 0;
					PlayList_setposition(0);
				}
			}
			PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
			if (playing)
			{
				StopPlaying(0);
				StartPlaying();
			}
			else StopPlaying(0);
		}
	}
	return 1;
} // Main_OnButton5()