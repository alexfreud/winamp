#ifndef NULLSOFT_AUTH_LOGINPOPUP_MESSAGE_HEADER
#define NULLSOFT_AUTH_LOGINPOPUP_MESSAGE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginPopup.h"

class LoginPopupMessage : public LoginPopup
{
public:
	typedef enum
	{
		typeMask		= 0x0000FFFF,
		typeContinue	= 0x00000000,
		typeYesNo		= 0x00000001,
		iconMask		= 0xFFFF0000,
		iconInfo		= 0x00000000,
		iconWarning		= 0x00010000,
		iconError		= 0x00020000,
	} Type;

	typedef void (CALLBACK *ResultCallback)(HWND /*hPopup*/, INT_PTR /*resultCode*/, LPARAM param);

protected:
	LoginPopupMessage(HWND hwnd);
	~LoginPopupMessage();

public:
	static HWND CreatePopup(HWND hParent, LPCWSTR pszTitle, LPCWSTR pszMessage, UINT uType, ResultCallback callback, LPARAM param);

protected:
	void UpdateLayout(BOOL fRedraw);
	void EndDialog(INT_PTR code);
	
	BOOL OnInitDialog(HWND hFocus, LPARAM param);
	BOOL OnUpdateWindowPos(const RECT* clientRect, RECT *rectOut);

protected:
	ResultCallback callback;
	LPARAM param;
	INT szButtons[8];
	UINT buttonsCount;

private:
	friend static HRESULT CALLBACK LoginPopupMessage_CreateInstance(HWND hwnd, LPARAM param, LoginPopup **instance);
};



#endif //NULLSOFT_AUTH_LOGINPOPUP_MESSAGE_HEADER