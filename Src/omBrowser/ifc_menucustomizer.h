#ifndef NULLSOFT_WINAMP_MENU_CUSTOMIZER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_MENU_CUSTOMIZER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {780C1929-76FD-4a06-934B-C85CC285A4DE}
static const GUID IFC_MenuCustomizer = 
{ 0x780c1929, 0x76fd, 0x4a06, { 0x93, 0x4b, 0xc8, 0x5c, 0xc2, 0x85, 0xa4, 0xde } };


#include "main.h"
#include <bfc/dispatch.h>

typedef LPVOID HMLIMGLST;

class __declspec(novtable) ifc_menucustomizer : public Dispatchable
{
protected:
	ifc_menucustomizer() {}
	~ifc_menucustomizer() {}

public:
	HMLIMGLST GetImageList(void);
	UINT GetSkinStyle(void);
	HRESULT GetWidth(INT *widthOut);
	INT CustomDraw(HMENU menuInstance, INT action, HDC hdc, LPARAM param);

public:
	DISPATCH_CODES
	{
		API_GETIMAGELIST		= 10,
		API_GETSKINSTYLE		= 20,
		API_GETWIDTH			= 30, 
		API_CUSTOMDRAW		= 40,
	};
};

inline HMLIMGLST ifc_menucustomizer::GetImageList(void)
{
	return _call(API_GETIMAGELIST, (HMLIMGLST)NULL);
}

inline UINT ifc_menucustomizer::GetSkinStyle(void)
{
	return (UINT)_call(API_GETSKINSTYLE, (UINT)/*SMS_USESKINFONT*/0x00000001);
}

inline HRESULT ifc_menucustomizer::GetWidth(INT *widthOut)
{
	return _call(API_GETWIDTH, (HRESULT)E_NOTIMPL, widthOut);
}

inline INT ifc_menucustomizer::CustomDraw(HMENU menuInstance, INT action, HDC hdc, LPARAM param)
{
	return _call(API_CUSTOMDRAW, (INT)FALSE, menuInstance, action, hdc, param);
}

#endif // NULLSOFT_WINAMP_MENU_CUSTOMIZER_INTERFACE_HEADER