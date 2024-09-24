#include "precomp__gen_ff.h"
#include "menuactions.h"
#include "wa2frontend.h"
#include <api/skin/skinparse.h>
#include "wa2cfgitems.h"
#include "main.h"
#include "resource.h"
#include <api/locales/xlatstr.h>
#include "../gen_ml/ml_ipc.h"
#include "../winamp/gen.h"
#include "../Agave/Language/api_language.h"
#include "../gen_ml/menufucker.h"
#include "../Winamp/strutil.h"

extern librarySendToMenuStruct mainSendTo;
#define WINAMP_OPTIONS_DSIZE            40165
extern void addWindowOptionsToContextMenu(ifc_window *w);
extern void removeWindowOptionsFromContextMenu();
extern ifc_window *g_controlMenuTarget;
extern HMENU controlmenu;
int lowest_itempos = 0;
int highest_itempos = -1;
int lowest_witempos = 0;
int lowest_witempos2 = 0;
int highest_witempos = -1;
int highest_witempos2 = -1;
int optionsmenu_wa = 1;
TList<HMENU> menulist;
TList<HMENU> wmenulist;
int in_menu = 0;


//-----------------------------------------------------------------------------------------------
MenuActions::MenuActions()
{
	registerAction(L"menu", _ACTION_MENU);
	registerAction(L"sysmenu", _ACTION_SYSMENU);
	registerAction(L"controlmenu", _ACTION_CONTROLMENU);
	registerAction(L"MENU:WA5:File", ACTION_WA5FILEMENU);
	registerAction(L"MENU:WA5:Play", ACTION_WA5PLAYMENU);
	registerAction(L"MENU:WA5:Options", ACTION_WA5OPTIONSMENU);
	registerAction(L"MENU:WA5:Windows", ACTION_WA5WINDOWSMENU);
	registerAction(L"MENU:WA5:Help", ACTION_WA5HELPMENU);
	registerAction(L"MENU:WA5:PE_File", ACTION_WA5PEFILEMENU);
	registerAction(L"MENU:WA5:PE_Playlist", ACTION_WA5PEPLAYLISTMENU);
	registerAction(L"MENU:WA5:PE_Sort", ACTION_WA5PESORTMENU);
	registerAction(L"MENU:WA5:PE_Help", ACTION_WA5PEHELPMENU);
	registerAction(L"MENU:WA5:ML_File", ACTION_WA5MLFILEMENU);
	registerAction(L"MENU:WA5:ML_View", ACTION_WA5MLVIEWMENU);
	registerAction(L"MENU:WA5:ML_Help", ACTION_WA5MLHELPMENU);
	registerAction(L"PE_Add", ACTION_PEADD);
	registerAction(L"PE_Rem", ACTION_PEREM);
	registerAction(L"PE_Sel", ACTION_PESEL);
	registerAction(L"PE_Misc", ACTION_PEMISC);
	registerAction(L"PE_List", ACTION_PELIST);
	registerAction(L"PE_ListOfLists", ACTION_PELISTOFLISTS);
	registerAction(L"VID_FS", ACTION_VIDFS);
	registerAction(L"VID_1X", ACTION_VID1X);
	registerAction(L"VID_2X", ACTION_VID2X);
	registerAction(L"VID_TV", ACTION_VIDTV);
	registerAction(L"VID_Misc", ACTION_VIDMISC);
	registerAction(L"VIS_Next", ACTION_VISNEXT);
	registerAction(L"VIS_Prev", ACTION_VISPREV);
	registerAction(L"VIS_FS", ACTION_VISFS);
	registerAction(L"VIS_CFG", ACTION_VISCFG);
	registerAction(L"VIS_Menu", ACTION_VISMENU);
	registerAction(L"trackinfo", ACTION_TRACKINFO);
	registerAction(L"trackmenu", ACTION_TRACKMENU);
	registerAction(L"ML_SendTo", ACTION_SENDTO);
}

//-----------------------------------------------------------------------------------------------
MenuActions::~MenuActions()
{}

static LRESULT sendMlIpc(int msg, WPARAM param) {
	static HWND mlwnd = NULL;
	if(!IsWindow(mlwnd)) {
		int IPC_GETMLWINDOW = (INT)SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&"LibraryGetWnd", IPC_REGISTER_WINAMP_IPCMESSAGE);
		if(IPC_GETMLWINDOW > 65536) mlwnd = (HWND)SendMessageW(plugin.hwndParent,WM_WA_IPC,0,IPC_GETMLWINDOW);
	}
	if(!IsWindow(mlwnd)) return 0;
	return SendMessageW(mlwnd,WM_ML_IPC,param,msg);
}

//-----------------------------------------------------------------------------------------------
int MenuActions::onActionId(int pvtid, const wchar_t *action, const wchar_t *param /* =NULL */, int p1 /* =0 */, int p2 /* =0 */, void *data /* =NULL */, int datalen /* =0 */, ifc_window *source /* =NULL */)
{
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	RECT r = {0, 0, 0, 0};
	if (source) source->getWindowRect(&r);
	int height = r.bottom - r.top;
	int width = r.right - r.left;
	in_menu = 1;
	switch (pvtid)
	{
	case _ACTION_MENU:
		{
			if (!_wcsicmp(param, L"presets"))
			{
				wa2.triggerEQPresetMenu(p1, p2);
			}
		}
		break;
	case _ACTION_SYSMENU:
		{
			addWindowOptionsToContextMenu(source);
			wa2.triggerPopupMenu(p1, p2);
			break;
		}
	case _ACTION_CONTROLMENU:
		{
			if (g_controlMenuTarget != NULL)
				removeWindowOptionsFromContextMenu();
			addWindowOptionsToContextMenu(g_controlMenuTarget);
			g_controlMenuTarget = source;

			HMENU ctrlmenu = GetSubMenu(controlmenu, 0);
			DoTrackPopup(ctrlmenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, p1, p2, wa2.getMainWindow());
			break;
		}
	case ACTION_WA5FILEMENU:
		{
			wa2.triggerFileMenu(p1, p2, width, height);
			break;
		}
	case ACTION_WA5PLAYMENU:
		{
			wa2.triggerPlayMenu(p1, p2, width, height);
			break;
		}
	case ACTION_WA5OPTIONSMENU:
		{
			wa2.triggerOptionsMenu(p1, p2, width, height);
			break;
		}
	case ACTION_WA5WINDOWSMENU:
		{
			wa2.triggerWindowsMenu(p1, p2, width, height);
			break;
		}
	case ACTION_WA5HELPMENU:
		{
			wa2.triggerHelpMenu(p1, p2, width, height);
			break;
		}
	case ACTION_WA5PEFILEMENU:
		{
			wa2.triggerPEFileMenu(p1, p2, width, height);
			break;
		}
	case ACTION_WA5PEPLAYLISTMENU:
		{
			wa2.triggerPEPlaylistMenu(p1, p2, width, height);
			break;
		}
	case ACTION_WA5PESORTMENU:
		{
			wa2.triggerPESortMenu(p1, p2, width, height);
			break;
		}
	case ACTION_WA5PEHELPMENU:
		{
			wa2.triggerPEHelpMenu(p1, p2, width, height);
			break;
		}
	case ACTION_WA5MLFILEMENU:
		{
			wa2.triggerMLFileMenu(p1, p2, width, height);
			break;
		}
	case ACTION_WA5MLVIEWMENU:
		{
			wa2.triggerMLViewMenu(p1, p2, width, height);
			break;
		}
	case ACTION_WA5MLHELPMENU:
		{
			wa2.triggerMLHelpMenu(p1, p2, width, height);
			break;
		}
	case ACTION_PEADD:
		{
			wa2.sendPlCmd(Winamp2FrontEnd::WA2_PLEDITPOPUP_ADD, r.left, r.top, TPM_BOTTOMALIGN | TPM_LEFTALIGN);
			break;
		}
	case ACTION_PEREM:
		{
			wa2.sendPlCmd(Winamp2FrontEnd::WA2_PLEDITPOPUP_REM, r.left, r.top, TPM_BOTTOMALIGN | TPM_LEFTALIGN);
			break;
		}
	case ACTION_PESEL:
		{
			wa2.sendPlCmd(Winamp2FrontEnd::WA2_PLEDITPOPUP_SEL, r.left, r.top, TPM_BOTTOMALIGN | TPM_LEFTALIGN);
			break;
		}
	case ACTION_PEMISC:
		{
			wa2.sendPlCmd(Winamp2FrontEnd::WA2_PLEDITPOPUP_MISC, r.left, r.top, TPM_BOTTOMALIGN | TPM_LEFTALIGN);
			break;
		}
	case ACTION_PELIST:
		{
			wa2.sendPlCmd(Winamp2FrontEnd::WA2_PLEDITPOPUP_LIST, r.right, r.top, TPM_BOTTOMALIGN | TPM_RIGHTALIGN);
			break;
		}
	case ACTION_PELISTOFLISTS:
		{
			wa2.triggerPEListOfListsMenu(r.left, r.top);
			break;
		}
	case ACTION_VIDFS:
		{
			wa2.sendVidCmd(Winamp2FrontEnd::WA2_VIDCMD_FULLSCREEN);
			break;
		}
	case ACTION_VID1X:
		{
			wa2.sendVidCmd(Winamp2FrontEnd::WA2_VIDCMD_1X);
			break;
		}
	case ACTION_VID2X:
		{
			wa2.sendVidCmd(Winamp2FrontEnd::WA2_VIDCMD_2X);
			break;
		}
	case ACTION_VIDTV:
		{
			wa2.sendVidCmd(Winamp2FrontEnd::WA2_VIDCMD_LIB);
			break;
		}
	case ACTION_VIDMISC:
		{
			wa2.sendVidCmd(Winamp2FrontEnd::WA2_VIDPOPUP_MISC, r.right, r.top, TPM_BOTTOMALIGN | TPM_RIGHTALIGN);
			break;
		}
	case ACTION_VISNEXT:
		{
			wa2.visNext();
			break;
		}
	case ACTION_VISPREV:
		{
			wa2.visPrev();
			break;
		}
	case ACTION_VISFS:
		{
			wa2.visFullscreen();
			break;
		}
	case ACTION_VISCFG:
		{
			wa2.visConfig();
			break;
		}
	case ACTION_VISMENU:
		{
			wa2.visMenu();
			break;
		}
	case ACTION_TRACKMENU:
		{
			extern const wchar_t *GetMenuItemString(HMENU menu, int id, int bypos);
#define WINAMP_TOGGLE_AUTOSCROLL        40189
#define ID_RATING5                      40396
#define ID_RATING4                      40397
#define ID_RATING3                      40398
#define ID_RATING2                      40399
#define ID_RATING1                      40400
#define ID_RATING0                      40401
			HMENU top_menu = wa2.getTopMenu();
			HMENU contextmenus = GetSubMenu(top_menu, 3);
			HMENU songinfomenu = GetSubMenu(contextmenus, 0);
			HMENU ratingmenu = GetSubMenu(songinfomenu, 5);
			int rating = wa2.getCurTrackRating();
			CheckMenuItem(ratingmenu, ID_RATING5, (rating == 5) ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(ratingmenu, ID_RATING4, (rating == 4) ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(ratingmenu, ID_RATING3, (rating == 3) ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(ratingmenu, ID_RATING2, (rating == 2) ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(ratingmenu, ID_RATING1, (rating == 1) ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(ratingmenu, ID_RATING0, (rating == 0) ? MF_CHECKED : MF_UNCHECKED);
			StringW olditemstr = GetMenuItemString(songinfomenu, 3, TRUE);
			RemoveMenu(songinfomenu, 3, MF_BYPOSITION);
			
			LRESULT IPC_LIBRARY_SENDTOMENU = SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&"LibrarySendToMenu", IPC_REGISTER_WINAMP_IPCMESSAGE);
			HMENU menu = 0;
			memset(&mainSendTo, 0, sizeof(mainSendTo));
			if (IPC_LIBRARY_SENDTOMENU > 65536 && SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)0, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
			{
				char stStr[32] = {0};
				MENUITEMINFOA mii = {sizeof(mii), MIIM_SUBMENU | MIIM_TYPE, MFT_STRING, };
				mii.hSubMenu = menu = CreatePopupMenu();
				mii.dwTypeData = WASABI_API_LNGSTRING_BUF(IDS_SEND_TO,stStr,32);
				mii.cch = strlen((char*)mii.dwTypeData);

				InsertMenuItemA(songinfomenu, 3, TRUE, &mii);

				mainSendTo.mode = 1;
				mainSendTo.hwnd = plugin.hwndParent; // TODO???
				mainSendTo.data_type = ML_TYPE_FILENAMESW;
				mainSendTo.build_hMenu = menu;
			}

			menufucker_t mf = {sizeof(mf),MENU_SONGTICKER,songinfomenu,0x3000,0x4000,0};
			pluginMessage message_build = {SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&"menufucker_build", IPC_REGISTER_WINAMP_IPCMESSAGE),(intptr_t)&mf,0};
			sendMlIpc(ML_IPC_SEND_PLUGIN_MESSAGE,(WPARAM)&message_build);
			
			int ret = DoTrackPopup(songinfomenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON|TPM_RETURNCMD, p1, p2, wa2.getMainWindow());
			
			pluginMessage message_result = {SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&"menufucker_result", IPC_REGISTER_WINAMP_IPCMESSAGE),(intptr_t)&mf,ret,0};
			sendMlIpc(ML_IPC_SEND_PLUGIN_MESSAGE,(WPARAM)&message_result);

			if (menu)
			{
				if (mainSendTo.mode == 2)
				{
					mainSendTo.menu_id = ret;
					if (SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&mainSendTo, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
					{
						wchar_t buf[FILENAME_SIZE + 1] = {0};
						wchar_t *end=buf;
						size_t endSize = 0;
						const wchar_t *entry = wa2.getFileW(wa2.getCurPlaylistEntry());
						if (entry && *entry)
						{
							StringCchCopyExW(buf, FILENAME_SIZE, entry, &end, &endSize, 0);

							mainSendTo.mode = 3;
							mainSendTo.data = buf;
							mainSendTo.data_type = ML_TYPE_FILENAMESW;
							SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&mainSendTo, IPC_LIBRARY_SENDTOMENU);
						}
					}
				}
				// remove sendto
				DeleteMenu(songinfomenu, 3, MF_BYPOSITION);
			}
			if (mainSendTo.mode)
			{
				mainSendTo.mode = 4;
				SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&mainSendTo, IPC_LIBRARY_SENDTOMENU); // cleanup
				memset(&mainSendTo, 0, sizeof(mainSendTo));
			}
			InsertMenuW(songinfomenu, 3, MF_BYPOSITION | MF_STRING, WINAMP_TOGGLE_AUTOSCROLL, olditemstr);
			if (ret) SendMessageW(wa2.getMainWindow(), WM_COMMAND, ret, 0); // TODO?
			
			break;
		}
	case ACTION_SENDTO:
		{
			LRESULT IPC_LIBRARY_SENDTOMENU = SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&"LibrarySendToMenu", IPC_REGISTER_WINAMP_IPCMESSAGE);
			HMENU menu = 0;
			memset(&mainSendTo, 0, sizeof(mainSendTo));
			if (IPC_LIBRARY_SENDTOMENU > 65536 && SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)0, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
			{
				menu = CreatePopupMenu();

				mainSendTo.mode = 1;
				mainSendTo.hwnd = plugin.hwndParent; // TODO???
				mainSendTo.data_type = ML_TYPE_FILENAMESW;
				mainSendTo.build_hMenu = menu;
			}
			int ret = DoTrackPopup(menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON|TPM_RETURNCMD, p1, p2, wa2.getMainWindow());
			if (menu)
			{
				if (mainSendTo.mode == 2)
				{
					mainSendTo.menu_id = ret;
					if (SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&mainSendTo, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
					{
						wchar_t buf[FILENAME_SIZE + 1] = {0};
						wchar_t *end=buf;
						size_t endSize = 0;
						const wchar_t *entry = wa2.getFileW(wa2.getCurPlaylistEntry());
						if (entry && *entry)
						{
							StringCchCopyExW(buf, FILENAME_SIZE, entry, &end, &endSize, 0);

							mainSendTo.mode = 3;
							mainSendTo.data = buf;
							mainSendTo.data_type = ML_TYPE_FILENAMESW;
							SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&mainSendTo, IPC_LIBRARY_SENDTOMENU);
						}
					}
				}
			}
			if (mainSendTo.mode)
			{
				mainSendTo.mode = 4;
				SendMessageW(plugin.hwndParent, WM_WA_IPC, (WPARAM)&mainSendTo, IPC_LIBRARY_SENDTOMENU); // cleanup
				memset(&mainSendTo, 0, sizeof(mainSendTo));
			}
			
		}
		break;
	case ACTION_TRACKINFO:
		{
			wa2.openTrackInfo();
			break;
		}
	}
	in_menu = 0;
	return 1;
}

HMENU MenuActions::makeSkinOptionsSubMenu(GUID g, int *cmdoffset)
{
	CfgItem *item = WASABI_API_CONFIG->config_getCfgItemByGuid(g);
	if (item != NULL)
	{
		HMENU menu = CreatePopupMenu();
		int n = MIN(item->getNumAttributes(), 500);
		for (int i = 0;i < n;i++)
		{
			const wchar_t *attr = item->enumAttribute(i);
			if (attr && *attr)
			{
				HMENU submenu = NULL;
				wchar_t txt[256] = {0};
				item->getData(attr, txt, 256);
				GUID g = nsGUID::fromCharW(txt);
				if (g != INVALID_GUID)
				{ // submenu !
					submenu = makeSkinOptionsSubMenu(g, cmdoffset);
				}
				int v = item->getDataAsInt(attr, 0);
				if (WCSCASEEQLSAFE(txt, L"-"))
				{
					InsertMenu(menu, i, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
				}
				else
					InsertMenuW(menu, i, ((v && !submenu) ? MF_CHECKED : MF_UNCHECKED) | MF_STRING | MF_BYPOSITION | (submenu ? MF_POPUP : 0), submenu ? (UINT_PTR)submenu : 44000 + *cmdoffset, _(attr));
			}
			(*cmdoffset)++;
		}
		return menu;
	}
	return NULL;
}

void MenuActions::installSkinOptions(HMENU menu)
{
	optionsmenu_wa = 0;
	HMENU omenu = NULL;
	if (menu == NULL)
	{
		optionsmenu_wa = 1;
		menu = wa2.getMenuBarMenu(Winamp2FrontEnd::WA2_MAINMENUBAR_OPTIONS);
		omenu = GetSubMenu(wa2.getPopupMenu(), 11 + wa2.adjustOptionsPopupMenu(0));
	}
	int pos2 = 12;
	int cmdoffset = 0;
	int pos = optionsmenu_wa ? wa2.adjustFFOptionsMenu(0) + 6 + NUMSTATICWINDOWS /*+ 1*//* + 9*/ : 0;
	lowest_itempos = pos;
	int insertedline = 0;
	if (menu && optionsmenuitems)
	{
		int n = MIN(optionsmenuitems->getNumAttributes(), 500);
		for (int i = 0;i < n;i++)
		{
			const wchar_t *attr = optionsmenuitems->enumAttribute(i);
			if (attr && *attr)
			{
				HMENU submenu = NULL;
				wchar_t txt[256] = {0};
				optionsmenuitems->getData(attr, txt, 256);
				GUID g = nsGUID::fromCharW(txt);
				if (g != INVALID_GUID)
				{ // submenu !
					submenu = makeSkinOptionsSubMenu(g, &cmdoffset);
					if (submenu)
						menulist.addItem(submenu);
				}
				int v = optionsmenuitems->getDataAsInt(attr, 0);
				if (optionsmenu_wa && !insertedline)
				{
					wa2.adjustFFOptionsMenu(1);
					insertedline = 1;
					InsertMenu(menu, pos++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
					if (omenu) InsertMenu(omenu, pos2++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
				}
				if (optionsmenu_wa) wa2.adjustFFOptionsMenu(1);
				if (WCSCASEEQLSAFE(txt, L"-"))
				{
					InsertMenu(menu, pos++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
					if (omenu) InsertMenu(omenu, pos2++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
				}
				else
					InsertMenuW(menu, pos++, (v ? MF_CHECKED : MF_UNCHECKED) | MF_STRING | MF_BYPOSITION | (submenu ? MF_POPUP : 0), submenu ? (UINT_PTR)submenu : 44000 + cmdoffset, _(attr));
				if (omenu) InsertMenuW(omenu, pos2++, (v ? MF_CHECKED : MF_UNCHECKED) | MF_STRING | MF_BYPOSITION | (submenu ? MF_POPUP : 0), submenu ? (UINT_PTR)submenu : 44000 + cmdoffset, _(attr));
			}
			cmdoffset++;
		}
		ffoptionstop = 44000 + cmdoffset;
	}
	// insert colorthemes submenu
	if (omenu)
	{
		PtrListQuickSorted<ColorThemeSlot, ColorThemeSlotSort> sortedthemes;
		int fn = MIN(WASABI_API_SKIN->colortheme_getNumColorSets(), 500);
		for (int t = 0;t < fn;t++)
			sortedthemes.addItem(new ColorThemeSlot(WASABI_API_SKIN->colortheme_enumColorSet(t), t));
		if (!insertedline)
		{
			wa2.adjustFFOptionsMenu(1);
			InsertMenu(menu, pos++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
			if (omenu) InsertMenu(omenu, pos2++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
		}
		HMENU submenu = CreatePopupMenu();
		if (WASABI_API_SKIN->colortheme_getNumColorSets() == 0)
		{
			InsertMenuW(submenu, 0, MF_GRAYED | MF_STRING | MF_BYPOSITION, 0, WASABI_API_LNGSTRINGW(IDS_NO_THEME_AVAILABLE));
		}
		else
		{
			int n = sortedthemes.getNumItems();
			for (int i = 0;i < n;i++)
			{
				const wchar_t *ct = sortedthemes.enumItem(i)->name;
				int entry = sortedthemes.enumItem(i)->entry;
				int iscurrent = WCSCASEEQLSAFE(ct, WASABI_API_SKIN->colortheme_getColorSet());
				InsertMenuW(submenu, i, (iscurrent ? MF_CHECKED : MF_UNCHECKED) | MF_STRING | MF_BYPOSITION, 44500 + entry, ct);
			}
		}
		if (optionsmenu_wa) wa2.adjustFFOptionsMenu(1);
		InsertMenuW(menu, pos++, MF_STRING | MF_BYPOSITION | (submenu ? MF_POPUP : 0), submenu ? (UINT_PTR)submenu : 44000 + cmdoffset, WASABI_API_LNGSTRINGW(IDS_COLOR_THEMES));
		if (omenu) InsertMenuW(omenu, pos2++, MF_STRING | MF_BYPOSITION | (submenu ? MF_POPUP : 0), submenu ? (UINT_PTR)submenu : 44000 + cmdoffset, WASABI_API_LNGSTRINGW(IDS_COLOR_THEMES));
		cmdoffset++;
		sortedthemes.deleteAll();
	}
	highest_itempos = pos;
}

void MenuActions::removeSkinOptions()
{
	if (highest_itempos == -1 || !optionsmenu_wa) return ;
	for (int j = 0;j < menulist.getNumItems();j++)
		DestroyMenu(menulist.enumItem(j));
	menulist.removeAll();

	HMENU menu = wa2.getMenuBarMenu(Winamp2FrontEnd::WA2_MAINMENUBAR_OPTIONS);
	HMENU omenu = GetSubMenu(wa2.getPopupMenu(), 11 + wa2.adjustOptionsPopupMenu(0));
	if (menu && optionsmenuitems)
	{
		for (int i = lowest_itempos;i < highest_itempos;i++)
		{
			RemoveMenu(menu, lowest_itempos, MF_BYPOSITION);
			if (omenu) RemoveMenu(omenu, 12, MF_BYPOSITION);
			wa2.adjustFFOptionsMenu( -1);
		}
	}
	highest_itempos = -1;
}

int MenuActions::toggleOption(int n, GUID guid, int *cmdoffset)
{
	int _cmdoffset = 0;
	if (!cmdoffset) cmdoffset = &_cmdoffset;
	CfgItem *item = NULL;
	if (guid == INVALID_GUID)
		item = optionsmenuitems;
	else
		item = WASABI_API_CONFIG->config_getCfgItemByGuid(guid);

	if (!item) // not sure why this happens, but it happened in a crash report so I'm going to check for it.
		return 1; // TODO: guru

	for (int i = 0; i < item->getNumAttributes();i++)
	{
		const wchar_t *name = item->enumAttribute(i);
		if (name && *name)
		{
			wchar_t txt[256] = {0};
			item->getData(name, txt, 256);
			GUID g = nsGUID::fromCharW(txt);
			if (g != INVALID_GUID)
			{ // submenu !
				if (toggleOption(n, g, cmdoffset)) return 1;
			}
			if (*cmdoffset == n)
			{
				int newv = item->getDataAsInt(name) ? 0 : 1;
				item->setDataAsInt(name, newv);
				return 1;
			}
		}
		(*cmdoffset)++;
	}
	return 0;
}

const wchar_t* MenuActions::localizeSkinWindowName(const wchar_t* attr)
{
	static wchar_t tweaked_attr[96];
	ZERO(tweaked_attr);

	// allows us to map some of the common actions to localised versions (primarily with bundled skins)
	if(!_wcsicmp(attr,L"Equalizer\tAlt+G"))
	{
		// if there's a match then we can force things to use the previous menu
		// string (fixes localisation inconsistancy without altering the scripts!)
		lstrcpynW(tweaked_attr, eqmenustring, 96);
	}
	else if(!_wcsicmp(attr,L"Skin Settings\tAlt+C"))
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_SKIN_SETTINGS,tweaked_attr,96);
	}
	else if(!_wcsicmp(attr,L"Web Browser\tAlt+X"))
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_WEB_BROWSER,tweaked_attr,96);
	}
	else if(!_wcsicmp(attr,L"Album Art\tAlt+A"))
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_ALBUM_ART,tweaked_attr,96);
	}
	else if(!_wcsicmp(attr,L"Color Editor"))
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_COLOR_EDITOR,tweaked_attr,96);
	}
	else
	{
		return attr;
	}
	return tweaked_attr;
}

// FIX ME - menu weirdness going on!!
void MenuActions::installSkinWindowOptions()
{
	HMENU menu = wa2.getMenuBarMenu(Winamp2FrontEnd::WA2_MAINMENUBAR_WINDOWS);
	int pos = lowest_witempos = wa2.adjustFFWindowsMenu(0) + NUMSTATICWINDOWS;
	HMENU omenu = wa2.getPopupMenu();
	int pos2 = lowest_witempos2 = wa2.adjustOptionsPopupMenu(0) + 6 + NUMSTATICWINDOWS + 1;

	MENUITEMINFOW mii = {sizeof(mii), };
	mii.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU;
	mii.fType = MFT_STRING;
	mii.wID = 42000;
	mii.dwItemData = 0xD02;	// use this as a check so we're only removing the correct items!!

	if (menu && windowsmenuitems)
	{
		int n = MIN(windowsmenuitems->getNumAttributes(), 500);
		int cmdoffset = 0;
		for (int i = 0;i < n;i++)
		{
			const wchar_t *attr = windowsmenuitems->enumAttribute(i);

			if (attr && *attr)
			{
				HMENU submenu = NULL;
				wchar_t txt[256] = {0};
				windowsmenuitems->getData(attr, txt, 256);
				GUID g = nsGUID::fromCharW(txt);
				if (g != INVALID_GUID)
				{ // submenu !
					submenu = makeSkinOptionsSubMenu(g, &cmdoffset);
					if (submenu)
						wmenulist.addItem(submenu);
				}
				int v = windowsmenuitems->getDataAsInt(attr, 0);
				wa2.adjustFFWindowsMenu(1);
				wa2.adjustOptionsPopupMenu(1);

				attr = localizeSkinWindowName(attr);
				const wchar_t* t = _(attr);
				mii.dwTypeData = const_cast<wchar_t *>(t);

				if (WCSCASEEQLSAFE(txt, L"-"))
				{
					InsertMenu(menu, pos++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
					InsertMenu(omenu, pos2++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
				}
				else{
					mii.cch = wcslen(mii.dwTypeData);
					mii.fState = (v ? MFS_CHECKED : 0);
					mii.wID = 42000 + cmdoffset;
					mii.hSubMenu = submenu;
					InsertMenuItemW(menu, pos++, TRUE, &mii);
				}
				InsertMenuItemW(omenu, pos2++, TRUE, &mii);
			}
			cmdoffset++;
		}
		ffwoptionstop = 42000 + cmdoffset;
	}
	highest_witempos = pos;
	highest_witempos2 = pos2;
}

void MenuActions::removeSkinWindowOptions()
{
	if (highest_witempos == -1) return ;
	for (int j = 0;j < wmenulist.getNumItems();j++)
		DestroyMenu(wmenulist.enumItem(j));
	wmenulist.removeAll();

	HMENU menu = wa2.getMenuBarMenu(Winamp2FrontEnd::WA2_MAINMENUBAR_WINDOWS);
	HMENU omenu = wa2.getPopupMenu();
	if (menu && windowsmenuitems)
	{
		for(int i = GetMenuItemCount(menu)-1; i != 0; i--)
		{
			MENUITEMINFOW info = {sizeof(info), MIIM_DATA, 0, };
			if(GetMenuItemInfoW(menu,i,TRUE,&info))
			{
				if(info.dwItemData == 0xD02){
					RemoveMenu(menu,i,MF_BYPOSITION);
					wa2.adjustFFWindowsMenu(-1);
				}
			}
		}

		for(int i = GetMenuItemCount(omenu)-1; i != 0; i--)
		{
			MENUITEMINFOW info = {sizeof(info), MIIM_DATA, 0, };
			if(GetMenuItemInfoW(omenu,i,TRUE,&info))
			{
				if(info.dwItemData == 0xD02){
					RemoveMenu(omenu,i,MF_BYPOSITION);
					wa2.adjustOptionsPopupMenu(-1);
				}
			}
		}

		/*for (int i = highest_witempos;i >= lowest_witempos;i--)
		{
			RemoveMenu(menu, i, MF_BYPOSITION);
			wa2.adjustFFWindowsMenu(-1);
		}

		for (int i = highest_witempos2;i >= lowest_witempos2;i--)
		{
			RemoveMenu(menu, i, MF_BYPOSITION);
			wa2.adjustOptionsPopupMenu(-1);
		}
		/*/
		/*for (int i = lowest_witempos;i < highest_witempos;i++)
		{
			RemoveMenu(menu, lowest_witempos, MF_BYPOSITION);
			wa2.adjustFFWindowsMenu(-1);
		}

		for (int i = lowest_witempos2;i < highest_witempos2;i++)
		{
			RemoveMenu(omenu, lowest_witempos2, MF_BYPOSITION);
			wa2.adjustOptionsPopupMenu(-1);
		}/**/
	}
	highest_witempos = -1;
	highest_witempos2 = -1;
}

int MenuActions::toggleWindowOption(int n, GUID guid, int *cmdoffset)
{
	int _cmdoffset = 0;
	if (!cmdoffset) cmdoffset = &_cmdoffset;
	CfgItem *item = NULL;
	if (guid == INVALID_GUID)
		item = windowsmenuitems;
	else
		item = WASABI_API_CONFIG->config_getCfgItemByGuid(guid);

	for (int i = 0; i < item->getNumAttributes();i++)
	{
		const wchar_t *name = item->enumAttribute(i);
		if (name && *name)
		{
			wchar_t txt[256] = {0};
			item->getData(name, txt, 256);
			GUID g = nsGUID::fromCharW(txt);
			if (g != INVALID_GUID)
			{ // submenu !
				if (toggleWindowOption(n, g, cmdoffset)) return 1;
			}
			if (*cmdoffset == n)
			{
				int newv = item->getDataAsInt(name) ? 0 : 1;
				item->setDataAsInt(name, newv);
				return 1;
			}
		}
		(*cmdoffset)++;
	}
	return 0;
}