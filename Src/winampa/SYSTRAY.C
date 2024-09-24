#include "main.h"

#define SYSTRAY_ICON_BASE 1024

int ist = 0;

BOOL systray_isintray(void)
{
	return ist;
}

void CopyCharW(wchar_t *dest, const wchar_t *src)
{
	wchar_t *end = CharNextW(src);
	int count = (int)(end-src);
	while (count--)
	{
		*dest++=*src++;
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

BOOL systray_add(HWND hwnd, HICON hIcon, LPWSTR lpszTip)
{
	NOTIFYICONDATAW tnid = {0};
	tnid.cbSize = sizeof(NOTIFYICONDATAW);
	tnid.hWnd = hwnd;
	tnid.uID = SYSTRAY_ICON_BASE;
	tnid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	tnid.uCallbackMessage = WM_USER+8;
	tnid.hIcon = hIcon;
	mktipstr(tnid.szTip, lpszTip, sizeof(tnid.szTip)/sizeof(wchar_t));
	ist = 1;
	return Shell_NotifyIconW(NIM_ADD, &tnid);
}

BOOL systray_mod(HWND hwnd, HICON hIcon, LPWSTR lpszTip) {
	NOTIFYICONDATAW tnid = {0};
	tnid.cbSize = sizeof(NOTIFYICONDATAW);
	tnid.hWnd = hwnd;
	tnid.uID = SYSTRAY_ICON_BASE;
	tnid.hIcon = hIcon;
	tnid.uFlags = (lpszTip ? NIF_TIP : 0) | (hIcon ? NIF_ICON : 0);
	if (lpszTip) mktipstr(tnid.szTip, lpszTip, sizeof(tnid.szTip)/sizeof(wchar_t));
	return (Shell_NotifyIconW(NIM_MODIFY, &tnid));
}

BOOL systray_del(HWND hwnd) {
	NOTIFYICONDATAW tnid = {0};
	tnid.cbSize = sizeof(NOTIFYICONDATAW);
	tnid.hWnd = hwnd;
	tnid.uID = SYSTRAY_ICON_BASE;
	ist = 0;
	return(Shell_NotifyIconW(NIM_DELETE, &tnid));
}