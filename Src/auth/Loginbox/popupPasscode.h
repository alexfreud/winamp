#ifndef NULLSOFT_AUTH_LOGINPOPUP_PASSCODE_HEADER
#define NULLSOFT_AUTH_LOGINPOPUP_PASSCODE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./loginPopup.h"

class LoginData;
class LoginDataCredentials;


// notifications
#define NPPN_FIRST		(120)

typedef struct __NPPNRESULT
{
	NMHDR hdr;
	INT_PTR exitCode;
	LoginData *loginData;
} NPPNRESULT;

#define NPPN_RESULT		(NPPN_FIRST + 0)


class LoginPopupPasscode : public LoginPopup
{
protected:
	LoginPopupPasscode(HWND hwnd);
	~LoginPopupPasscode();

public:
	static HWND CreatePopup(HWND hParent, LoginData *loginData);

protected:
	void UpdateLayout(BOOL fRedraw);
	void EndDialog(INT_PTR code);

	BOOL Validate();

	BOOL OnInitDialog(HWND hFocus, LPARAM param);
	void OnCommand(UINT commandId, UINT eventType, HWND hControl);
	LRESULT OnNotify(UINT controlId, const NMHDR *pnmh);
	HBRUSH OnGetStaticColor(HDC hdc, HWND hControl);

	LRESULT OnEditboxChar(HWND hEdit, UINT ch);
	LRESULT OnEditboxKey(HWND hEdit, UINT vKey, UINT flags);
	LRESULT OnEditboxPaste(HWND hEdit, LPCWSTR pszText);

	INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	LoginDataCredentials *loginData;
	LPWSTR	message;
	UINT	messageType;

private:
	friend static HRESULT CALLBACK LoginPopupPasscode_CreateInstance(HWND hwnd, LPARAM param, LoginPopup **instance);
};




#endif //NULLSOFT_AUTH_LOGINPOPUP_PASSCODE_HEADER