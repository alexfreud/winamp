#ifndef NULLSOFT_AUTH_LOGINBOX_TOS_REMINDER_HEADER
#define NULLSOFT_AUTH_LOGINBOX_TOS_REMINDER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

HWND TosReminder_CreateWindow(HWND hParent);
INT_PTR TosReminder_Show(HWND hParent, INT controlId, BOOL fAnimate);

#endif //NULLSOFT_AUTH_LOGINBOX_TOS_REMINDER_HEADER