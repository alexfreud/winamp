#ifndef NULLSOFT_AUTH_LOGINPAGE_HEADER
#define NULLSOFT_AUTH_LOGINPAGE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define NLPM_FIRST			(WM_APP + 10)
#define NLPM_PAGEFIRST		(NLPM_FIRST + 40)

#define NLPM_GETLOGINDATA	(NLPM_FIRST + 0) // wParam - not used, lParam = (LPARAM)(LoginData**)__ppLoginData; Return:  TRUE on success.
#define LoginPage_GetData(/*HWND*/ __hwnd, /*LoginData** */ __ppLoginData)\
	((BOOL)(SNDMSG((__hwnd), NLPM_GETLOGINDATA, 0, (LPARAM)(__ppLoginData))))

#define NLPM_UPDATESTATECHANGE	(NLPM_FIRST + 1) // wParam - not used, lParam = (BOOL)__updateActive; Return - ignored
#define LoginPage_UpdateStateChange(/*HWND*/ __hwnd, /*BOOL*/ __updateActive)\
	((BOOL)(SNDMSG((__hwnd), NLPM_UPDATESTATECHANGE, 0, (LPARAM)(__updateActive))))

#define NLPM_SETUSERNAME		(NLPM_FIRST + 2) // wParam - not used, lParam = (LPARAM)(LPCWSTR)__pszUsername; Return TRUE on success
#define LoginPage_SetUsername(/*HWND*/ __hwnd, /*LPCWSTR*/ __pszUsername)\
	((BOOL)(SNDMSG((__hwnd), NLPM_SETUSERNAME, 0, (LPARAM)(__pszUsername))))

#define NLPM_SETPASSWORD		(NLPM_FIRST + 3) // wParam - not used, lParam = (LPARAM)(LPCWSTR)__pszPassword; Return TRUE on success
#define LoginPage_SetPassword(/*HWND*/ __hwnd, /*LPCWSTR*/ __pszPassword)\
	((BOOL)(SNDMSG((__hwnd), NLPM_SETPASSWORD, 0, (LPARAM)(__pszPassword))))

#define NLPM_GETFIRSTITEM		(NLPM_FIRST + 4) // wParam - not used, lParam - not used; Return HWND where you want focus to be on page create or NULL.
#define LoginPage_GetFirstItem(/*HWND*/ __hwnd)\
	((HWND)(SNDMSG((__hwnd), NLPM_GETFIRSTITEM, 0, 0L)))

#define NLPM_SETTITLE			(NLPM_FIRST + 5) // wParam - not used, lParam = (LPARAM)(LPCWSTR)__pszTitle; Return TRUE on success
#define LoginPage_SetTitle(/*HWND*/ __hwnd, /*LPCWSTR*/ __pszTitle)\
	((BOOL)(SNDMSG((__hwnd), NLPM_SETTITLE, 0, (LPARAM)(__pszTitle))))

class LoginPage;
class LoginData;

typedef HRESULT (CALLBACK *LOGINPAGECREATOR)(HWND /*hwnd*/, HWND /*hLoginbox*/, LoginPage** /*instance*/);

class __declspec(novtable) LoginPage
{

protected:
	LoginPage(HWND hwnd, HWND hLoginbox);
	virtual ~LoginPage();

public:
	static HWND CreatePage(HWND hLoginbox, LPCWSTR pszTemplate, HWND hParent, LPARAM param, LOGINPAGECREATOR fnCreator);

protected:
	virtual void UpdateMargins();
	virtual void UpdateColors();
	virtual void UpdateLayout(BOOL fRedraw);

	virtual BOOL GetPageRect(RECT *prc);
	BOOL ShowHelp();
	BOOL IsHelpAvailable();
	INT GetTitleSpacing();
	BOOL SetLabelText(INT controlId, LPCWSTR pszText);


protected:
	virtual BOOL OnInitDialog(HWND hFocus, LPARAM param);
	virtual void OnDestroy();
	virtual void OnWindowPosChanged(const WINDOWPOS *pwp);
	virtual void OnCommand(UINT commandId, UINT eventType, HWND hControl);
	virtual BOOL OnNotify(UINT controlId, const NMHDR *pnmh);
	virtual BOOL OnSetCursor(HWND hTarget, INT hitCode, INT uMsg);
	virtual HBRUSH OnGetStaticColor(HDC hdc, HWND hControl);
	virtual HBRUSH OnGetDialogColor(HDC hdc, HWND hControl);
	virtual BOOL OnHelp(HELPINFO *phi);
	virtual void OnThemeChanged();
	virtual void OnSysColorChanged();
	
	virtual BOOL OnGetLoginData(LoginData **ppLoginData);
	virtual void OnUpdateStateChange(BOOL updateActive);
	virtual BOOL OnSetUsername(LPCWSTR pszUsername);
	virtual BOOL OnSetPassword(LPCWSTR pszPassword);
	virtual HWND OnGetFirstItem();
	virtual BOOL OnSetTitle(LPCWSTR pszTitle);

	virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	friend static INT_PTR CALLBACK LoginPage_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	HWND	hwnd;
	HWND	hLoginbox;
	RECT	margins;
	COLORREF rgbTitle;
	COLORREF rgbSecondaryText;
	COLORREF rgbText;
	COLORREF rgbBack;
	HBRUSH	hbrBack;
	
};

#endif //NULLSOFT_AUTH_LOGINPAGE_HEADER