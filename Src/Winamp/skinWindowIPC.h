#ifndef NULLOSFT_WINAMP_SKINNEDWINDOW_IPC_HEADER
#define NULLOSFT_WINAMP_SKINNEDWINDOW_IPC_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define SWS_EX_NOSNAP		0x00000001		// never snap

// send next messages as WM_WA_IPC  but directly to the skinnedWindow
#define IPC_SKINWINDOW_SETEXSTYLE		1		// param - new flags
#define IPC_SKINWINDOW_GETEXSTYLE		2		// param - not used
#define IPC_SKINWINDOW_SETEMBEDFLAGS	3		// param - new flags
#define IPC_SKINWINDOW_GETEMBEDFLAGS	4		// param - not used
#define IPC_SKINWINDOW_GETWASABIWND		5		// param - not used; returns ifc_window if any
#define IPC_SKINWINDOW_GETGUID			6		// param = (WPARAM)(GUID*)pguid; return TRUE on success
#define IPC_SKINWINDOW_GETEMBEDNUMS		7		// param - not used

#define IPC_SKINWINDOW_UNSKIN			10		// param - not used

#endif // NULLOSFT_WINAMP_SKINNEDWINDOW_IPC_HEADER