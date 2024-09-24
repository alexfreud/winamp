#ifndef NULLSOFT_AUTH_LOGINBOX_EDITBOX_EXTENDER_HEADER
#define NULLSOFT_AUTH_LOGINBOX_EDITBOX_EXTENDER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

BOOL EditboxExtender_AttachWindow(HWND hEditbox);

// notification
typedef  struct __EENMPASTE
{
	NMHDR hdr;
	LPCWSTR text;
}EENMPASTE;

#define EENM_FIRST		10
#define EENM_PASTE		(EENM_FIRST + 0L)

#endif // NULLSOFT_AUTH_LOGINBOX_EDITBOX_EXTENDER_HEADER