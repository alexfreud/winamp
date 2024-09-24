#include "main.h"
#include "./menu.h"
#include "./resource.h"
#include "../gen_ml/ml_ipc_0313.h"

INT Menu_TrackPopup(HMENU hMenu, UINT fuFlags, INT x, INT y,  HWND hwnd, LPTPMPARAMS lptpm)
{
	if (NULL == hMenu)
		return NULL;
	
	MLSKINNEDPOPUP popup;
	ZeroMemory(&popup, sizeof(MLSKINNEDPOPUP));
	popup.cbSize = sizeof(MLSKINNEDPOPUP);
	popup.hmenu = hMenu;
    popup.fuFlags = fuFlags;
    popup.x = x;
    popup.y = y;
    popup.hwnd = hwnd;
	popup.lptpm = lptpm;
    popup.skinStyle = SMS_USESKINFONT/*SMS_SYSCOLORS*/;
	popup.customProc = NULL;
	popup.customParam = 0;

	return (INT)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_TRACKSKINNEDPOPUPEX, &popup);
}