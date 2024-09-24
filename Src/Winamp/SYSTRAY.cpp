/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "Main.h"
#include <stdarg.h>
#include "strutil.h"

#define SYSTRAY_ICON 502
#define SYSTRAY_WINDOW hMainWindow

int systray_intray = 0;

static BOOL systray_add(HWND hwnd, UINT uID, LPWSTR lpszTip);
static BOOL systray_del(HWND hwnd, UINT uID);
static BOOL systray_mod(HWND hwnd, UINT uID, LPWSTR lpszTip);
static HICON hhIcon;
static int hicon_index=-1;

void systray_minimize(wchar_t *tip) 
{
	extern int geticonid(int x);
	int newindex=geticonid(config_sticon);
	if (!SYSTRAY_WINDOW) return;
	if (!hhIcon || hicon_index != newindex)	{
		if (hhIcon) {
			DestroyIcon(hhIcon);
		}
		if(newindex == -666) {
			wchar_t tmp[MAX_PATH] = {0};
			StringCchPrintfW(tmp,MAX_PATH,L"%s\\winamp.ico",CONFIGDIR);
			if(!PathFileExistsW(tmp)) {
				hhIcon = (HICON)LoadImageW(hMainInstance,MAKEINTRESOURCEW(ICON_XP),IMAGE_ICON,16,16,LR_SHARED);
			}
			else {
				hhIcon = (HICON)LoadImageW(0,tmp,IMAGE_ICON,16,16,LR_LOADFROMFILE);
			}
		}
		else {
			hhIcon = (HICON)LoadImageW(hMainInstance,MAKEINTRESOURCEW(newindex),IMAGE_ICON,16,16,LR_SHARED);
		}
	}
	hicon_index=newindex;
	if (!systray_intray) {
		systray_add(SYSTRAY_WINDOW,SYSTRAY_ICON,tip);
		systray_intray = 1;
	}
	else {
		systray_mod(SYSTRAY_WINDOW,SYSTRAY_ICON,tip);
	}
}

void systray_restore(void) 
{
	if (systray_intray) {
		systray_del(SYSTRAY_WINDOW,SYSTRAY_ICON);
		systray_intray = 0;
		if (hhIcon) {
			DestroyIcon(hhIcon);
			hhIcon=NULL;
			hicon_index=-1;
		}
	}
}

static void mktipstr(wchar_t *out, wchar_t *in, size_t outlen)
{
	wchar_t *nextOut;
	size_t outpos=0;
	while (outpos < outlen-1 && *in)
	{
		if (*in == L'&')
		{
			if ((outpos+=2) >= outlen-1) break;
			*out++=L'&';
			*out++=L'&';
		}

		CopyCharW(out, in);
		nextOut = CharNextW(out);
		in = CharNextW(in);
		outpos+=(nextOut-out);
		out=nextOut;
	}
	*out=0;
}

static BOOL systray_add(HWND hwnd, UINT uID, LPWSTR lpszTip) 
{
	NOTIFYICONDATAW tnid = {0};
	tnid.cbSize = sizeof(NOTIFYICONDATAW);
	tnid.hWnd = hwnd;
	tnid.uID = uID;
	tnid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	tnid.uCallbackMessage = WM_WA_SYSTRAY;
	tnid.hIcon = hhIcon;
	mktipstr(tnid.szTip,lpszTip,sizeof(tnid.szTip)/sizeof(wchar_t));
	return Shell_NotifyIconW(NIM_ADD, &tnid);
}

static BOOL systray_del(HWND hwnd, UINT uID) 
{
	NOTIFYICONDATAW tnid = {0};
	tnid.cbSize = sizeof(NOTIFYICONDATAW);
	tnid.hWnd = hwnd;
	tnid.uID = uID;
	return(Shell_NotifyIconW(NIM_DELETE, &tnid));
}

static BOOL systray_mod(HWND hwnd, UINT uID, LPWSTR lpszTip) 
{
	NOTIFYICONDATAW tnid = {0};
	tnid.cbSize = sizeof(NOTIFYICONDATAW);
	tnid.hWnd = hwnd;
	tnid.uID = uID;
	tnid.uFlags = NIF_TIP|NIF_ICON;
	tnid.hIcon = hhIcon;
	mktipstr(tnid.szTip,lpszTip,sizeof(tnid.szTip)/sizeof(wchar_t));
	return (Shell_NotifyIconW(NIM_MODIFY, &tnid));
}