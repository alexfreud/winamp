#ifndef NULLSOFT_PODCAST_PLUGIN_SUBSCRIPTION_VIEW_HEADER
#define NULLSOFT_PODCAST_PLUGIN_SUBSCRIPTION_VIEW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#include <windows.h>
#include "../nu/listview.h"
#include "../nu/AutoLock.h"

class OmService;

HWND CALLBACK SubscriptionView_Create(HWND hParent, OmService *service);
HWND CALLBACK SubscriptionView_FindWindow(void);

#define SVM_FIRST			(WM_APP + 13)

#define SVM_REFRESHCHANNELS	(SVM_FIRST + 0)
#define SubscriptionView_RefreshChannels(/*HWND*/ __hwnd, /*BOOL*/ __sort)\
	((BOOL)PostMessage((__hwnd), SVM_REFRESHCHANNELS, (WPARAM)(__sort), 0L))

#define SVM_SETSTATUS		(SVM_FIRST + 1)
#define SubscriptionView_SetStatus(/*HWND*/ __hwnd, /*LPCWSTR*/ __status)\
	((BOOL)SNDMSG((__hwnd), SVM_SETSTATUS, 0, (LPARAM)(__status)))

#endif //NULLSOFT_PODCAST_PLUGIN_SUBSCRIPTION_VIEW_HEADER