/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: 
 ** Created:
 **/


#include "main.h"
#include "resource.h"

extern void ConvertEOF();
int PlayQueue_OnEOF()
{
	if (m_converting)
	{
		ConvertEOF();
		return 0;
	}
	LRESULT nextItem;

	g_fullstop = 1;

	// for gen_jumpex mainly so it doesn't have to use ugly hacks
	nextItem = SendMessageW(hMainWindow, WM_WA_IPC, 0, IPC_GET_NEXT_PLITEM);

	if (nextItem != -1) // check to see if anyone has overridden what track we play next
	{
		PlayList_setposition(nextItem);

		PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
		StartPlaying();
	}
	else // nope?  just advanced to the next track
	{
		if (PlayList_getlength() && !g_stopaftercur)
		{
			if (!config_pladv) // if manual playlist advance is on
			{
				if (!config_repeat
				    && !PlayList_current_hidden()) // if this is a "horizontal" playlist, let the tracks play out
					goto fullstop;
				StartPlaying();
			}
			else if (!config_shuffle && PlayList_advance(HIDDEN_TRAP) < 0) // -33 is so i can trap playnext
			{
				if (config_repeat)
				{
					PlayList_setposition(0);
					PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
					StartPlaying();
				}
				else
				{
					g_fullstop = 2;
					StopPlaying(0);
				}
			}
			else
			{
				if (config_shuffle && !PlayList_current_hidden())
				{
					int lp = PlayList_getPosition();
					if ((PlayList_randpos(1) || PlayList_getlength() == 1)  // this isn't really the place for this check
					    && !config_repeat)
					{
						g_fullstop = 2;
						StopPlaying(0);
						g_fullstop = 0;
						return 0;
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
				StartPlaying();
			}
		}
		else
		{
		HMENU m;
		fullstop:
			g_stopaftercur = 0;
			CheckMenuItem(main_menu, WINAMP_BUTTON4_CTRL, MF_UNCHECKED);
			m = GetSubMenu(top_menu, 3);
			CheckMenuItem(m, WINAMP_BUTTON4_CTRL, MF_UNCHECKED);
			g_fullstop = 2;
			StopPlaying(0);
		}
	}

	g_fullstop = 0;
	return 1;
}