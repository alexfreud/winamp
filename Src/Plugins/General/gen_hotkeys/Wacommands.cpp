#include "WACommands.h"
#include "gen_hotkeys.h"
#include "../nu/AutoWide.h"

typedef struct {
	UINT strId;
	genHotkeysAddStruct ghk;
} genHotkeysAddStructLocalised;

genHotkeysAddStructLocalised WADefCommands[] = {
	{IDS_PLAYBACK__PLAY,{0, 0, WM_COMMAND, WINAMP_BUTTON2, 0, "ghkdc play"}},
	{IDS_PLAYBACK__PAUSE,{0, 0, WM_COMMAND, WINAMP_BUTTON3, 0, "ghkdc pause"}},
	{IDS_PLAYBACK__STOP,{0, 0, WM_COMMAND, WINAMP_BUTTON4, 0, "ghkdc stop"}},
	{IDS_PLAYBACK__PREVIOUS_IN_PLAYLIST,{0, 0, WM_COMMAND, WINAMP_BUTTON1, 0, "ghkdc prev"}},
	{IDS_PLAYBACK__NEXT_IN_PLAYLIST,{0, 0, WM_COMMAND, WINAMP_BUTTON5, 0, "ghkdc next"}},
	{IDS_PLAYBACK__VOLUME_UP,{0, 0, WM_COMMAND, WINAMP_VOLUMEUP, 0, "ghkdc vup"}},
	{IDS_PLAYBACK__VOLUME_DOWN,{0, 0, WM_COMMAND, WINAMP_VOLUMEDOWN, 0, "ghkdc vdown"}},
	{IDS_PLAYBACK__FORWARD,{0, 0, WM_COMMAND, WINAMP_FFWD5S, 0, "ghkdc forward"}},
	{IDS_PLAYBACK__REWIND,{0, 0, WM_COMMAND, WINAMP_REW5S, 0, "ghkdc rewind"}},
	{IDS_PLAYBACK__REPEAT_ON,{0, 0, WM_WA_IPC, 1, IPC_SET_REPEAT, "ghkdc repeat on"}},
	{IDS_PLAYBACK__REPEAT_OFF,{0, 0, WM_WA_IPC, 0, IPC_SET_REPEAT, "ghkdc repeat off"}},
	{IDS_UI__TOGGLE_EQ,{0, 0, WM_COMMAND, WINAMP_OPTIONS_EQ, 0, "ghkdc t eq"}},
	{IDS_UI__TOGGLE_PLAYLIST,{0, 0, WM_COMMAND, WINAMP_OPTIONS_PLEDIT, 0, "ghkdc t pl"}},
	{IDS_UI__TOGGLE_AOT,{0, 0, WM_COMMAND, WINAMP_OPTIONS_AOT, 0, "ghkdc t aot"}},
	{IDS_UI__ABOUT,{0, HKF_BRING_TO_FRONT, WM_COMMAND, WINAMP_HELP_ABOUT, 0, "ghkdc about"}},
	{IDS_PLAYBACK__JUMP_TO_BOX,{0, HKF_BRING_TO_FRONT, WM_COMMAND, 40194, 0, "ghkdc jump"}},
	{IDS_PLAYBACK__OPEN_FILE_DIALOG,{0, HKF_BRING_TO_FRONT|HKF_WPARAM_HWND, WM_WA_IPC, 0, IPC_OPENFILEBOX, "ghkdc file"}},
	{IDS_PLAYBACK__OPEN_LOC_DIALOG,{0, HKF_BRING_TO_FRONT, WM_COMMAND, WINAMP_BUTTON2_CTRL, 0, "ghkdc url"}},
	{IDS_PLAYBACK__OPEN_FOLDER_DIALOG,{0, HKF_BRING_TO_FRONT|HKF_WPARAM_HWND, WM_WA_IPC, 0, IPC_OPENDIRBOX, "ghkdc dir"}},
	{IDS_GENERAL__QUIT,{0, 0, WM_CLOSE, 0, 0, "ghkdc quit"}},
	{IDS_UI__PREFERENCES,{0, HKF_BRING_TO_FRONT, WM_WA_IPC, (WPARAM)(-1), IPC_OPENPREFSTOPAGE, "ghkdc prefs"}},
	{IDS_GENERAL__COPY_TITLE,{0, HKF_COPY_RET|HKF_PLPOS_WPARAM, WM_WA_IPC, 0, IPC_GETPLAYLISTTITLE, "ghkdc copy"}},
	{IDS_GENERAL__COPY_FILE_PATH,{0, HKF_COPY_RET|HKF_WPARAM_PLPOS, WM_WA_IPC, 0, IPC_GETPLAYLISTFILE, "ghkdc copy path"}},
	{IDS_PLAYBACK__PLAY_PAUSE,{0, HKF_ISPLAYING_WL, WM_COMMAND, WINAMP_BUTTON3, WINAMP_BUTTON2, "ghkdc play/pause"}},
	{IDS_PLAYBACK__TOGGLE_REPEAT,{0, 0, WM_COMMAND, 40022, 0, "ghkdc t repeaat"}},
	{IDS_PLAYBACK__TOGGLE_SHUFFLE,{0, 0, WM_COMMAND, 40023, 0, "ghkdc t shuffle"}},
	{IDS_UI__TOGGLE_WINSHADE,{0, 0, WM_COMMAND, 40064, 0, "ghkdc ws"}},
	{IDS_UI__TOGGLE_PLAYLIST_WINSHADE,{0, 0, WM_COMMAND, 40266, 0, "ghkdc pl ws"}},
	{IDS_UI__TOGGLE_DOUBLESIZE,{0, 0, WM_COMMAND, 40165, 0, "ghkdc ds"}},
	{IDS_UI__TOGGLE_MAIN_WINDOW,{0, 0, WM_COMMAND, 40258, 0, "ghkdc t main"}},
	// removed due to it not being in current winamp builds now {IDS_UI__TOGGLE_MINIBROWSER,{0, 0, WM_COMMAND, 40298, 0, "ghkdc t mb"}},
	{IDS_VIS__PREFERENCES,{0, HKF_BRING_TO_FRONT, WM_COMMAND, 40191, 0, "ghkdc vis prefs"}},
	{IDS_VIS_TOGGLE_VIS_ON_OFF,{0, 0, WM_COMMAND, 40192, 0, "ghkdc t vis"}},
	// added in 1.91 based on user feedback (ID_VIS_* options) and also from finding the correct menu id to use
	{IDS_VIS__PLUGIN_PREFERENCES,{0, HKF_BRING_TO_FRONT, WM_COMMAND, 40221, 0, "ghkdc prefs vis"}},
	{IDS_VIS_NEXT_PRESET,{0, 0, WM_COMMAND, ID_VIS_NEXT, 0, "ghkdc vis next"}},
	{IDS_VIS_PREV_PRESET,{0, 0, WM_COMMAND, ID_VIS_PREV, 0, "ghkdc vis prev"}},
	{IDS_VIS_RAND_PRESET,{0, 0, WM_COMMAND, ID_VIS_RANDOM, 0, "ghkdc vis rand"}},
	{IDS_VIS_FULL_SCREEN,{0, 0, WM_COMMAND, ID_VIS_FS, 0, "ghkdc vis fs"}},
	{IDS_UI__SELECT_SKIN,{0, HKF_BRING_TO_FRONT, WM_COMMAND, 40219, 0, "ghkdc skin"}},
	{IDS_PLAYBACK__STOP_WITH_FADEOUT,{0, 0, WM_COMMAND, WINAMP_BUTTON4_SHIFT, 0, "ghkdc stop fade"}},
	{IDS_PLAYBACK__STOP_AFTER_CURRENT,{0, 0, WM_COMMAND, WINAMP_BUTTON4_CTRL, 0, "ghkdc stop ac"}},
	{IDS_PLAYBACK__START_OF_LIST,{0, 0, WM_COMMAND, WINAMP_BUTTON1_CTRL, 0, "ghkdc sol"}},
	{IDS_PLAYBACK__END_OF_LIST,{0, 0, WM_COMMAND, WINAMP_BUTTON5_CTRL, 0, "ghkdc eol"}},
	{IDS_UI__BRING_TO_FRONT_OR_HIDE,{0, HKF_SHOWHIDE, WM_SHOWWINDOW, 0, 0, "ghkdc show/hide"}},
	{IDS_RATING__5_STARS,{0, 0, WM_WA_IPC, 5, IPC_SETRATING, "ghkdc rating 5"}},
	{IDS_RATING__4_STARS,{0, 0, WM_WA_IPC, 4, IPC_SETRATING, "ghkdc rating 4"}},
	{IDS_RATING__3_STARS,{0, 0, WM_WA_IPC, 3, IPC_SETRATING, "ghkdc rating 3"}},
	{IDS_RATING__2_STARS,{0, 0, WM_WA_IPC, 2, IPC_SETRATING, "ghkdc rating 2"}},
	{IDS_RATING__1_STAR,{0, 0, WM_WA_IPC, 1, IPC_SETRATING, "ghkdc rating 1"}},
	{IDS_RATING__NO_RATING,{0, 0, WM_WA_IPC, 0, IPC_SETRATING, "ghkdc rating 0"}},
	// added in 1.4+ so that we can deal with unicode strings on 5.3x+ Winamp versions
	{IDS_GENERAL__COPY_TITLEW,{0, HKF_COPYW_RET|HKF_PLPOS_WPARAM, WM_WA_IPC, 0, IPC_GETPLAYLISTTITLEW, "ghkdc copyw"}},
	{IDS_GENERAL__COPY_FILE_PATHW,{0, HKF_COPYW_RET|HKF_WPARAM_PLPOS, WM_WA_IPC, 0, IPC_GETPLAYLISTFILEW, "ghkdc copy pathw"}},
	// added in 1.6+ for neorth
	{IDS_RATING__INC,{0, 0, WM_WA_IPC, 6, IPC_SETRATING, "ghkdc inc rating"}},
	{IDS_RATING__DEC,{0, 0, WM_WA_IPC, (WPARAM)(-1), IPC_SETRATING, "ghkdc dec rating"}},
	// added in 1.91 for kzuse
	{IDS_PLAYBACK__MANUAL_ADVANCE,{0, 0, WM_COMMAND, 40395, 0, "ghkdc man adv"}},
	// added in 1.92 as killing of separate gen_find_on_disk.dll to be native
	{IDS_GENERAL_FFOD,{0, 0, WM_COMMAND, 40468, 0, "FFOD_Cur"}},
	{IDS_PLAYBACK_EDIT_ID3,{0, 0, WM_COMMAND, 40188, 0, "VCFI"}},
};

static unsigned int WACommandsNum;
static unsigned int WACommandsAllocated;
WACommand *WACommands=0;

inline unsigned int GetCommandsNum()
{
	return WACommandsNum;
}

static bool ReallocateCommands()
{
	if (WACommandsNum < WACommandsAllocated)
    return true;

	WACommand *newCommands = new WACommand[ WACommandsNum * 2 + 16];
	if (newCommands)
		memcpy(newCommands, WACommands, WACommandsAllocated*sizeof(WACommand));

	WACommandsAllocated = WACommandsNum * 2 + 16;
	delete [] WACommands;
	WACommands = newCommands;
	if (!WACommands)
	{
		WACommandsAllocated = 0;
		return false;
	}
	return true;
}

void InitCommands()
{
	int l = sizeof(WADefCommands) / sizeof(WADefCommands[0]);
	while (l--)
	{
		wchar_t rateBuf[1024] = {0};
		wchar_t* rateStr = rateBuf;
		WADefCommands[l].ghk.name = (char*)WASABI_API_LNGSTRINGW_BUF(WADefCommands[l].strId, rateBuf, 1024);
		switch(WADefCommands[l].strId)
		{
			// for the rating entries force any asterisks to a unicode star
			case IDS_RATING__1_STAR:
			case IDS_RATING__2_STARS:
			case IDS_RATING__3_STARS:
			case IDS_RATING__4_STARS:
			case IDS_RATING__5_STARS:
				while(rateStr && *rateStr)
				{
					if(*rateStr == L'*') *rateStr = L'\u2605';
					rateStr=CharNextW(rateStr);
				}
			break;
		}
		WADefCommands[l].ghk.flags |= HKF_UNICODE_NAME;
		AddCommand(&WADefCommands[l].ghk);
	}
}

int AddCommand(genHotkeysAddStruct *ghas)
{
	unsigned int idx = WACommandsNum;
	if (!ghas)
    return -1;

	// legacy support
	if (!ghas->id)
		ghas->id = ghas->name;

	if (WACommands)
	{
		unsigned int l = WACommandsNum;
		while (l--)
		{
			if (!lstrcmpiW(WACommands[l].id, AutoWide(ghas->id)))
			{
				WACommands[l].bEnabled = !(ghas->flags & HKF_DISABLED);
				char *name = 0;
				if(!(ghas->flags & HKF_UNICODE_NAME))
					name = (char *) malloc(lstrlen(ghas->name) + 1);
				else
					name = (char *) malloc((lstrlenW((wchar_t*)ghas->name) + 1) * sizeof(wchar_t));
				if (name)
				{
					free(WACommands[l].name);
					WACommands[l].name = name;
				}
				WACommands[l].dwFlags = (ghas->flags & 0x7FFFFFFF);
				WACommands[l].uMsg = ghas->uMsg;
				WACommands[l].wParam = ghas->wParam;
				WACommands[l].lParam = ghas->lParam;
				WACommands[l].wnd = ghas->wnd;
				return l;
			}
		}
	}
	WACommandsNum++;
	if (!ReallocateCommands())
	{
		WACommandsNum--;
		return -2;
	}

	if(!(ghas->flags & HKF_UNICODE_NAME))
		WACommands[idx].name = _strdup(ghas->name);
	else
		WACommands[idx].name = (char*)_wcsdup((wchar_t*)ghas->name);

	if (!WACommands[idx].name)
	{
		WACommandsNum--;
		return -1;
	}

	WACommands[idx].id = AutoWideDup(ghas->id);
	if (!WACommands[idx].id)
	{
	    free(WACommands[idx].name);
		WACommandsNum--;
		return -1;
	}
	WACommands[idx].dwFlags = (ghas->flags & 0x7FFFFFFF);
	WACommands[idx].uMsg = ghas->uMsg;
	WACommands[idx].wParam = ghas->wParam;
	WACommands[idx].lParam = ghas->lParam;
	WACommands[idx].bEnabled = !(ghas->flags & HKF_DISABLED);
	WACommands[idx].wnd = ghas->wnd;
	return idx;
}

inline char *GetCommandName(unsigned int i, bool *unicode)
{
	if (i < WACommandsNum)
	{
		if(unicode) *unicode = !!(WACommands[i].dwFlags & HKF_UNICODE_NAME);
		return WACommands[i].name;
	}
	return "";
}

inline wchar_t *GetCommandId(unsigned int i)
{
	if (i < WACommandsNum)
		return WACommands[i].id;
	return L"";
}

int GetCommandIdx(wchar_t *id)
{
	if (WACommands)
	{
		unsigned int l = WACommandsNum;
		while (l--)
		{
			if (!lstrcmpiW(WACommands[l].id, id))
			{
				return l;
			}
		}
	}
	return -1;
}

int HandleByScript(unsigned int i)
{
	if (i >= WACommandsNum) return 0;
	if (SendMessage(psPlugin.hwndParent, WM_WA_IPC, (WPARAM)WACommands[i].name, IPC_FF_NOTIFYHOTKEY)) return 0;
	return 1;
}

int DoCommand(unsigned int i)
{
	if (i >= WACommandsNum)
		return 0;

	if (!WACommands[i].bEnabled)
		return 0;

	if (HandleByScript(i)) return 1;

	WPARAM wParam = WACommands[i].wParam;
	LPARAM lParam = WACommands[i].lParam;

  /*if (WACommands[i].dwFlags & HKF_CUSTOM_FUNC)
	{
		if (WACommands[i].uMsg)
		{
		pfnWAC pfn = (pfnWAC) WACommands[i].uMsg;
		pfn();
		}

		return 0;
	}*/

	if (WACommands[i].dwFlags & HKF_SHOWHIDE)
	{
		DWORD dwThisProcId, dwProcId;
		GetWindowThreadProcessId(psPlugin.hwndParent, &dwThisProcId);
		GetWindowThreadProcessId(GetForegroundWindow(), &dwProcId);
		if (IsWindowVisible(psPlugin.hwndParent) && dwProcId == dwThisProcId)
			ShowWindow(psPlugin.hwndParent, SW_MINIMIZE);
		else
		{
			ShowWindow(psPlugin.hwndParent, SW_SHOWNORMAL);
			SetForegroundWindow(psPlugin.hwndParent);
		}
	}

	HWND hwnddest=WACommands[i].wnd ? WACommands[i].wnd : psPlugin.hwndParent;

	if (WACommands[i].dwFlags & HKF_BRING_TO_FRONT)
	{
		SetForegroundWindow(psPlugin.hwndParent);
	}
	if (WACommands[i].dwFlags & HKF_WPARAM_HWND)
	{
		wParam = (WPARAM) psPlugin.hwndParent;
	}
	if (WACommands[i].dwFlags & HKF_WPARAM_PLPOS)
	{
		wParam = (WPARAM) SendMessage(psPlugin.hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS);
	}
	if (WACommands[i].dwFlags & HKF_WPARAM_ISPLAYING_WL)
	{
		if (SendMessage(psPlugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING))
			wParam = WACommands[i].wParam; // Pause
		else
			wParam = WACommands[i].lParam; // Play

		lParam = 0;
	}

	LRESULT lResult = SendMessage(hwnddest, WACommands[i].uMsg, wParam, lParam);

	if (WACommands[i].dwFlags & HKF_COPY && lResult)
	{
		char *szTitle = (char *) lResult;

		if (!OpenClipboard(psPlugin.hwndParent))
		return 0;
		EmptyClipboard();

		int bufLen = lstrlen(szTitle) + 1;
		HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, bufLen);
		if (!hglbCopy)
		{
		CloseClipboard();
		return 0;
		}

		char *szClipboardTitle = (char *) GlobalLock(hglbCopy); 
		if (!szClipboardTitle)
		{
		CloseClipboard();
		return 0;
		}
		lstrcpyn(szClipboardTitle, szTitle, bufLen);
		GlobalUnlock(hglbCopy);

		SetClipboardData(CF_TEXT, hglbCopy);

		CloseClipboard();
	}

	if (WACommands[i].dwFlags & HKF_COPYW && lResult)
	{
		wchar_t *szTitle = (wchar_t *) lResult;

		if (!OpenClipboard(psPlugin.hwndParent))
		return 0;
		EmptyClipboard();

		int bufLen = (lstrlenW(szTitle) + 1) * sizeof(wchar_t);
		HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, bufLen);
		if (!hglbCopy)
		{
			CloseClipboard();
			return 0;
		}

		wchar_t *szClipboardTitle = (wchar_t *) GlobalLock(hglbCopy); 
		if (!szClipboardTitle)
		{
			CloseClipboard();
			return 0;
		}
		lstrcpynW(szClipboardTitle, szTitle, bufLen);
		GlobalUnlock(hglbCopy);

		SetClipboardData(CF_UNICODETEXT, hglbCopy);
		CloseClipboard();
	}
	return (int)lResult;
}