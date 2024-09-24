#ifndef NULLSOFT_WINAMP_OMBROWSER_TOOLBAREDITBOX_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_TOOLBAREDITBOX_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class ToolbarEditboxHost;

BOOL ToolbarEditbox_AttachWindow(HWND hEditbox, ToolbarEditboxHost *host);

#define NTEBM_FIRST		(WM_APP + 2)
#define NTEBM_SELECTALL		(NTEBM_FIRST + 0)
#define NTEBM_UPDATECURSOR	(NTEBM_FIRST + 1) // internal


class __declspec(novtable) ToolbarEditboxHost
{
public:
	virtual void EditboxDestroyed(HWND hwnd) = 0;
	virtual BOOL EditboxKillFocus(HWND hwnd, HWND hFocus) = 0; // return TRUE if handled
	virtual void EditboxResetText(HWND hwnd) = 0;
	virtual void EditboxNavigateNextCtrl(HWND hwnd, BOOL fForward) = 0;
	virtual void EditboxAcceptText(HWND hwnd) = 0;
	virtual BOOL EditboxKeyDown(HWND hwnd, UINT vKey, UINT state) = 0; // return TRUE if handled;
	virtual BOOL EditboxKeyUp(HWND hwnd, UINT vKey, UINT state) = 0; // return TRUE if handled;
	virtual BOOL EditboxPreviewChar(HWND hwnd, UINT vKey, UINT state) = 0; // return TRUE if handled;
};

#endif // NULLSOFT_WINAMP_OMBROWSER_TOOLBAREDITBOX_HEADER