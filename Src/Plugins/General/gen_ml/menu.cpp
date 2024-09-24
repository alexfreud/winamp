#include "menu.h"

INT Menu_TrackPopupParam(HWND library, HMENU hMenu, UINT fuFlags, INT x, INT y,  HWND hwnd, LPTPMPARAMS lptpm, ULONG_PTR param)
{
	if ( hMenu == NULL )
		return NULL;
	
	MLSKINNEDPOPUP popup = {0};
	popup.cbSize         = sizeof(MLSKINNEDPOPUP);
	popup.hmenu          = hMenu;
    popup.fuFlags        = fuFlags;
    popup.x              = x;
    popup.y              = y;
    popup.hwnd           = hwnd;
	popup.lptpm          = lptpm;
    popup.skinStyle      = SMS_USESKINFONT;
	popup.customParam    = param;

	return (INT)SENDMLIPC( library, ML_IPC_TRACKSKINNEDPOPUPEX, &popup );
}