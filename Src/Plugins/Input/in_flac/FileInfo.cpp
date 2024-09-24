/*
** Copyright (C) 2007-2011 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
** Author: Ben Allison benski@winamp.com
** Created: March 1, 2007
**
*/

#include <FLAC/all.h>
#include "main.h"
#include "../nu/ns_wc.h"
#include <windows.h>
#include "resource.h"
#include "Metadata.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoChar.h"
#include "Stopper.h"
#include <strsafe.h>
#include <commctrl.h>
#include "../Agave/Language/api_language.h"

bool FlacTagToWinampTag(wchar_t * tag, int len) 
{
#define TAG_ALIAS(b,a) if(!_wcsicmp(L ## a, tag)) { lstrcpynW(tag, L ## b, len); return true; }
	TAG_ALIAS("title", "TITLE");
	TAG_ALIAS("artist", "ARTIST");
	TAG_ALIAS("album", "ALBUM");
	TAG_ALIAS("genre", "GENRE");
	TAG_ALIAS("comment", "COMMENT");
	TAG_ALIAS("year", "DATE");
	TAG_ALIAS("track", "TRACKNUMBER");
	TAG_ALIAS("albumartist", "ALBUM ARTIST");
	TAG_ALIAS("composer", "COMPOSER");
	TAG_ALIAS("disc", "DISCNUMBER");
	TAG_ALIAS("publisher", "PUBLISHER");
	TAG_ALIAS("conductor", "CONDUCTOR");
	TAG_ALIAS("bpm", "BPM");
	return false;
#undef TAG_ALIAS
}

bool WinampTagToFlacTag(wchar_t * tag, int len) 
{
#define TAG_ALIAS(a,b) if(!_wcsicmp(L ## a, tag)) { lstrcpynW(tag, L ## b, len); return true; }
	TAG_ALIAS("title", "TITLE");
	TAG_ALIAS("artist", "ARTIST");
	TAG_ALIAS("album", "ALBUM");
	TAG_ALIAS("genre", "GENRE");
	TAG_ALIAS("comment", "COMMENT");
	TAG_ALIAS("year", "DATE");
	TAG_ALIAS("track", "TRACKNUMBER");
	TAG_ALIAS("albumartist", "ALBUM ARTIST");
	TAG_ALIAS("composer", "COMPOSER");
	TAG_ALIAS("disc", "DISCNUMBER");
	TAG_ALIAS("publisher", "PUBLISHER");
	TAG_ALIAS("conductor", "CONDUCTOR");
	TAG_ALIAS("bpm", "BPM");
	return false;
#undef TAG_ALIAS
}

static INT_PTR CALLBACK ChildProc_Advanced(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	static int sel=-1;
	static int ismychange=0;
	wchar_t key[512]={0};
	wchar_t value[32768]={0};

	switch(msg)
	{
		case WM_NOTIFYFORMAT:
			return NFR_UNICODE;
		case WM_INITDIALOG:
			{
				#define ListView_InsertColumnW(hwnd, iCol, pcol) \
						(int)SNDMSG((hwnd), LVM_INSERTCOLUMNW, (WPARAM)(int)(iCol), (LPARAM)(const LV_COLUMNW *)(pcol))
				SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);
				sel=-1;
				HWND hwndlist = GetDlgItem(hwndDlg,IDC_LIST);
				ListView_SetExtendedListViewStyle(hwndlist, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
				LVCOLUMNW lvc = {0, };
				lvc.mask = LVCF_TEXT|LVCF_WIDTH;
				lvc.pszText = WASABI_API_LNGSTRINGW(IDS_NAME);
				lvc.cx = 82;
				ListView_InsertColumnW(hwndlist, 0, &lvc);
				lvc.pszText = WASABI_API_LNGSTRINGW(IDS_VALUE);
				lvc.cx = 160;
				ListView_InsertColumnW(hwndlist, 1, &lvc);

				Info *info = (Info *)lParam;
				int n = info->metadata.GetNumMetadataItems();
				for(int i=0; i<n; i++) {
					char key_[512] = {0};
					const char* value_ = info->metadata.EnumMetadata(i,key_,512);
					if(value_ && key_[0]) {
						AutoWide k(key_, CP_UTF8);
						AutoWide v(value_, CP_UTF8);
						LVITEMW lvi={LVIF_TEXT,i,0};
						lvi.pszText = k;
						SendMessage(hwndlist,LVM_INSERTITEMW,0,(LPARAM)&lvi);
						lvi.iSubItem=1;
						lvi.pszText = v;
						SendMessage(hwndlist,LVM_SETITEMW,0,(LPARAM)&lvi);
					}
				}
				ListView_SetColumnWidth(hwndlist,0,LVSCW_AUTOSIZE);
				ListView_SetColumnWidth(hwndlist,1,LVSCW_AUTOSIZE);

				SetDlgItemTextW(hwndDlg,IDC_NAME,L"");
				SetDlgItemTextW(hwndDlg,IDC_VALUE,L"");
				EnableWindow(GetDlgItem(hwndDlg,IDC_NAME),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_VALUE),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DEL),FALSE);
			}
			break;
		case WM_DESTROY:
			{
				HWND hwndlist = GetDlgItem(hwndDlg,IDC_LIST);
				ListView_DeleteAllItems(hwndlist);
				while(ListView_DeleteColumn(hwndlist,0));
				Info * info = (Info*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
				delete info;
				info = 0;
			}
			break;
		case WM_USER:
			if(wParam && lParam && !ismychange)
			{
				wchar_t * value = (wchar_t*)lParam;
				wchar_t tag[100] = {0};
				lstrcpynW(tag,(wchar_t*)wParam,100);
				WinampTagToFlacTag(tag,100);
				Info *info = (Info *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				if(!*value) 
				{
					info->metadata.RemoveMetadata(AutoChar(tag,CP_UTF8));
					if(!_wcsicmp(L"ALBUM ARTIST",tag)) 
					{
						// need to remove these two, also, or else it's gonna look like delete doesn't work
						// if the file was tagged using these alternate fields
						info->metadata.RemoveMetadata("ALBUMARTIST");
						info->metadata.RemoveMetadata("ENSEMBLE");
					}
					if(!_wcsicmp(L"PUBLISHER",tag)) 
					{
						// need to remove this also, or else it's gonna look like delete doesn't work
						// if the file was tagged using this alternate field
						info->metadata.RemoveMetadata("ORGANIZATION");
					}
					if(!_wcsicmp(L"DATE",tag)) 
					{
						// need to remove this also, or else it's gonna look like delete doesn't work
						// if the file was tagged using this alternate field
						info->metadata.RemoveMetadata("YEAR");
					}
					if(!_wcsicmp(L"TRACKNUMBER",tag)) 
					{
						// need to remove this also, or else it's gonna look like delete doesn't work
						// if the file was tagged using this alternate field
						info->metadata.RemoveMetadata("TRACK");
					}
				}
				else 
				{
					info->metadata.SetMetadata(AutoChar(tag,CP_UTF8),AutoChar(value,CP_UTF8));
				}
				HWND hlist = GetDlgItem(hwndDlg,IDC_LIST);
				int n = ListView_GetItemCount(hlist);
				for(int i=0; i<n; i++)
				{
					key[0]=0;
					LVITEMW lvi={LVIF_TEXT,i,0};
					lvi.pszText=key;
					lvi.cchTextMax=sizeof(key)/sizeof(*key);
					SendMessage(hlist,LVM_GETITEMW,0,(LPARAM)&lvi);
					if(!_wcsicmp(key,tag))
					{
						lvi.iSubItem = 1;
						lvi.pszText = value;
						SendMessage(hlist,LVM_SETITEMW,0,(LPARAM)&lvi);
						if(!*value)
							ListView_DeleteItem(hlist,i);
						else if(ListView_GetItemState(hlist,i,LVIS_SELECTED))
							SetDlgItemTextW(hwndDlg,IDC_VALUE,value);
						return 0;
					}
				}
				// bew hew, not found
				LVITEMW lvi={0,0x7FFFFFF0,0};
				n = SendMessage(hlist,LVM_INSERTITEMW,0,(LPARAM)&lvi);
				lvi.mask = LVIF_TEXT;
				lvi.iItem = n;
				lvi.iSubItem = 0;
				lvi.pszText = tag;
				SendMessage(hlist,LVM_SETITEMW,0,(LPARAM)&lvi);
				lvi.iSubItem = 1;
				lvi.pszText = value;
				SendMessage(hlist,LVM_SETITEMW,0,(LPARAM)&lvi);
			}
			break;
		case WM_NOTIFY:
		{
				LPNMHDR l=(LPNMHDR)lParam;
				if(l->idFrom==IDC_LIST && l->code == LVN_KEYDOWN) {
					if((((LPNMLVKEYDOWN)l)->wVKey) == VK_DELETE){
					int selitem = ListView_GetNextItem(l->hwndFrom,-1,LVNI_SELECTED|LVNI_FOCUSED);
						if(selitem != -1)
							SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDC_BUTTON_DEL,BN_CLICKED),(LPARAM)GetDlgItem(hwndDlg,IDC_BUTTON_DEL));
					}
				}
				else if(l->idFrom==IDC_LIST && l->code == LVN_ITEMCHANGED) {
					LPNMLISTVIEW lv=(LPNMLISTVIEW)lParam;
					if(lv->uNewState & LVIS_SELECTED) {
						int n = lv->iItem;
						LVITEMW lvi={LVIF_TEXT,lv->iItem,0};
						lvi.pszText=key;
						lvi.cchTextMax=sizeof(key)/sizeof(*key);
						SendMessage(l->hwndFrom,LVM_GETITEMW,0,(LPARAM)&lvi);
						lvi.pszText=value;
						lvi.cchTextMax=sizeof(value)/sizeof(*value);
						lvi.iSubItem=1;
						SendMessage(l->hwndFrom,LVM_GETITEMW,0,(LPARAM)&lvi);
						SetDlgItemTextW(hwndDlg,IDC_NAME,key);
						SetDlgItemTextW(hwndDlg,IDC_VALUE,value);
						sel = n;
						EnableWindow(GetDlgItem(hwndDlg,IDC_NAME),TRUE);
						EnableWindow(GetDlgItem(hwndDlg,IDC_VALUE),TRUE);
						EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DEL),TRUE);
					}
					if(lv->uOldState & LVIS_SELECTED) {
						sel = -1;
						SetDlgItemTextW(hwndDlg,IDC_NAME,L"");
						SetDlgItemTextW(hwndDlg,IDC_VALUE,L"");
						EnableWindow(GetDlgItem(hwndDlg,IDC_NAME),FALSE);
						EnableWindow(GetDlgItem(hwndDlg,IDC_VALUE),FALSE);
						EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DEL),FALSE);
					}
				}
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					{
						Info * info = (Info*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
						Stopper stopper;
						if (lastfn && !_wcsicmp(lastfn, info->filename))
							stopper.Stop();
						bool success = info->metadata.Save(info->filename);
						stopper.Play();
						if (success)
						{
							ResetMetadataCache();
						}
						else
						{
							wchar_t title[128] = {0};
							MessageBoxW(hwndDlg,WASABI_API_LNGSTRINGW(IDS_CANNOT_SAVE_METADATA),
										WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_SAVING_METADATA,title,128),
										MB_OK | MB_ICONWARNING);
						}
					}
					break;
				case IDC_NAME:
				case IDC_VALUE:
					if(HIWORD(wParam) == EN_CHANGE && sel>=0) {
						LVITEMW lvi={LVIF_TEXT,sel,0};
						GetDlgItemTextW(hwndDlg,IDC_NAME,key,sizeof(key)/sizeof(*key));
						GetDlgItemTextW(hwndDlg,IDC_VALUE,value,sizeof(value)/sizeof(*value));
						lvi.pszText=key;
						lvi.cchTextMax=sizeof(key)/sizeof(*key);
						SendMessage(GetDlgItem(hwndDlg,IDC_LIST),LVM_SETITEMW,0,(LPARAM)&lvi);
						lvi.pszText=value;
						lvi.cchTextMax=sizeof(value)/sizeof(*value);
						lvi.iSubItem=1;
						SendMessage(GetDlgItem(hwndDlg,IDC_LIST),LVM_SETITEMW,0,(LPARAM)&lvi);
						FlacTagToWinampTag(key,sizeof(key)/sizeof(*key));
						ismychange=1;
						SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)key,(WPARAM)value);
						ismychange=0;
					}
					else if(HIWORD(wParam) == EN_KILLFOCUS && sel>=0) {
						GetDlgItemTextW(hwndDlg,IDC_NAME,key,sizeof(key)/sizeof(*key));
						GetDlgItemTextW(hwndDlg,IDC_VALUE,value,sizeof(value)/sizeof(*value));
						Info *info = (Info *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
						char oldkeyA[100]="";
						bool newitem=true;
						if(sel < info->metadata.GetNumMetadataItems()) {
							info->metadata.EnumMetadata(sel,oldkeyA,100);
							newitem=false;
						}
						AutoWide oldkey(oldkeyA);
						if(!newitem && _wcsicmp(oldkey,key)) { // key changed
							info->metadata.SetTag(sel,AutoChar(key,CP_UTF8));
						} else {
							info->metadata.SetMetadata(AutoChar(key,CP_UTF8),AutoChar(value,CP_UTF8));
						}
						FlacTagToWinampTag(key,sizeof(key)/sizeof(*key));
						ismychange=1;
						SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)key,(WPARAM)value);
						ismychange=0;
					}
					break;
				case IDC_BUTTON_DEL:
					if(sel >= 0){
						GetDlgItemTextW(hwndDlg,IDC_NAME,key,sizeof(key)/sizeof(*key));
						SetDlgItemTextW(hwndDlg,IDC_NAME,L"");
						SetDlgItemTextW(hwndDlg,IDC_VALUE,L"");
						EnableWindow(GetDlgItem(hwndDlg,IDC_NAME),FALSE);
						EnableWindow(GetDlgItem(hwndDlg,IDC_VALUE),FALSE);
						EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DEL),FALSE);
						Info *info = (Info *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
						if(sel < info->metadata.GetNumMetadataItems())
							info->metadata.RemoveMetadata(sel);
						ListView_DeleteItem(GetDlgItem(hwndDlg,IDC_LIST),sel);
						sel=-1;
						FlacTagToWinampTag(key,sizeof(key)/sizeof(*key));
						ismychange=1;
						SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)key,(WPARAM)L"");
						ismychange=0;
					}
					break;
				case IDC_BUTTON_DELALL:
					ListView_DeleteAllItems(GetDlgItem(hwndDlg,IDC_LIST));
					SetDlgItemTextW(hwndDlg,IDC_NAME,L"");
					SetDlgItemTextW(hwndDlg,IDC_VALUE,L"");
					EnableWindow(GetDlgItem(hwndDlg,IDC_NAME),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_VALUE),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_DEL),FALSE);
					sel=-1;
					{
						Info *info = (Info *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
						int n = info->metadata.GetNumMetadataItems();
						while(n>0) {
							--n;
							char tag[100] = {0};
							info->metadata.EnumMetadata(n,tag,100);
							MultiByteToWideCharSZ(CP_UTF8, 0, tag, -1, key, sizeof(key)/sizeof(*key));
							FlacTagToWinampTag(key,sizeof(key)/sizeof(*key));
							ismychange=1;
							SendMessage(GetParent(hwndDlg),WM_USER,(WPARAM)key,(WPARAM)L"");
							ismychange=0;
							info->metadata.RemoveMetadata(n);
						}
					}
					break;
				case IDC_BUTTON_ADD:
					{
						HWND hwndlist = GetDlgItem(hwndDlg,IDC_LIST);
						LVITEMW lvi={0,0x7FFFFFF0,0};
						int n = SendMessage(hwndlist,LVM_INSERTITEMW,0,(LPARAM)&lvi);
						ListView_SetItemState(hwndlist,n,LVIS_SELECTED,LVIS_SELECTED);
					}
					break;
			}
			break;
	}
	return 0;
}

extern "C"
{
	// return 1 if you want winamp to show it's own file info dialogue, 0 if you want to show your own (via In_Module.InfoBox)
	// if returning 1, remember to implement winampGetExtendedFileInfo("formatinformation")!
	__declspec(dllexport) int winampUseUnifiedFileInfoDlg(const wchar_t * fn)
	{
		return 1;
	}

	// should return a child window of 513x271 pixels (341x164 in msvc dlg units), or return NULL for no tab.
	// Fill in name (a buffer of namelen characters), this is the title of the tab (defaults to "Advanced").
	// filename will be valid for the life of your window. n is the tab number. This function will first be 
	// called with n == 0, then n == 1 and so on until you return NULL (so you can add as many tabs as you like).
	// The window you return will recieve WM_COMMAND, IDOK/IDCANCEL messages when the user clicks OK or Cancel.
	// when the user edits a field which is duplicated in another pane, do a SendMessage(GetParent(hwnd),WM_USER,(WPARAM)L"fieldname",(LPARAM)L"newvalue");
	// this will be broadcast to all panes (including yours) as a WM_USER.
	__declspec(dllexport) HWND winampAddUnifiedFileInfoPane(int n, const wchar_t * filename, HWND parent, wchar_t *name, size_t namelen)
	{
		if(n == 0) { // add first pane
			SetPropW(parent,L"INBUILT_NOWRITEINFO", (HANDLE)1);
			info = new Info;
			info->filename = filename;
			info->metadata.Open(filename, true);
			return WASABI_API_CREATEDIALOGPARAMW(IDD_INFOCHILD_ADVANCED,parent,ChildProc_Advanced,(LPARAM)info);
		}
		return NULL;
	}
};