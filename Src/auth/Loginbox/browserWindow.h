#ifndef NULLSOFT_AUTH_LOGINBOX_BROWSER_WINDOW_HEADER
#define NULLSOFT_AUTH_LOGINBOX_BROWSER_WINDOW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class BrowserEvent;

BOOL BrowserWindow_Attach(HWND hBrowser, BrowserEvent *eventHandler);
BOOL BrowserWindow_Detach(HWND hBrowser);
BOOL BrowserWindow_QueueApc(HWND hBrowser, LPARAM param);

#endif // NULLSOFT_AUTH_LOGINBOX_BROWSER_WINDOW_HEADER