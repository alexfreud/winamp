#ifndef NULLSOFT_AUTH_LOGINPOPUP_HEADER
#define NULLSOFT_AUTH_LOGINPOPUP_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

// messages
#define NLPOPUP_FIRST				(WM_APP + 100)
#define NLPOPUP_UPDATEWNDPOS		(NLPOPUP_FIRST + 0) // wParam - (WPARAM)(const RECT*)clientRect, lParam = (LPARAM)(RECT*)popupRectOut; Return TRUE if you set controlRect;
#define LoginPopup_UpdateWindowPos(/*HWND*/ __hwnd, /*const RECT* */__clientRect, /*RECT* */__popupRectOut)\
	((BOOL)SNDMSG((__hwnd), NLPOPUP_UPDATEWNDPOS, (WPARAM)(__clientRect), (LPARAM)(__popupRectOut)))

#define NLPOPUP_PLAYBEEP			(NLPOPUP_FIRST + 1) // wParam - not used, lParam - not used; Return ignored
#define LoginPopup_PlayBeep(/*HWND*/ __hwnd)\
		(SNDMSG((__hwnd), NLPOPUP_PLAYBEEP, 0, 0L))

// notifications
#define NLPOPUPN_FIRST		(100)

typedef struct __NLPNRESULT
{
	NMHDR hdr;
	INT_PTR exitCode;
} NLPNRESULT;

#define NLPN_RESULT		(NLPOPUPN_FIRST + 0)

class __declspec(novtable) LoginPopup 
{
public:
	typedef HRESULT (CALLBACK *Creator)(HWND /*hwnd*/, LPARAM /*param*/, LoginPopup** /*instance*/);
	typedef BOOL (CALLBACK *Enumerator)(HWND /*hwnd*/, LPARAM /*param*/);

protected:
	LoginPopup(HWND hwnd, UINT popupType, LPCWSTR pszTitle);
	virtual ~LoginPopup();

protected:
	static HWND CreatePopup(LPCWSTR pszTemplate, HWND hParent, LPARAM param, Creator fnCreator);

public:
	static BOOL RegisterPopup(HWND hwnd, BOOL fRegister);
	static BOOL EnumeratePopups(HWND hHost, Enumerator callback, LPARAM param);
	static BOOL AnyPopup(HWND hHost);

protected:
	virtual void UpdateLayout(BOOL fRedraw);
	virtual void Paint(HDC hdc, const RECT *prcPaint, BOOL fErase);
	virtual void EndDialog(INT_PTR code);
	virtual void UpdateMargins();
	virtual void SetTitle(UINT type, LPCWSTR title);
	virtual void SetAlert(UINT type, LPCWSTR message);
	virtual void RemoveAlert();
	virtual void UpdateTitle(BOOL playBeep);

	BOOL GetInfoRect(RECT *rect);
	BOOL CalculateWindowRect(LONG infoWidth, LONG infoHeight, const INT *buttonList, UINT buttonCount, BOOL includeTitle, RECT *rect);
	
	HDWP LayoutButtons(HDWP hdwp, const INT *buttonList, UINT buttonCount, BOOL redraw, RECT *rectOut);
	LRESULT SendNotification(UINT code, NMHDR *pnmh);
	BOOL GetTextSize(HWND hText, LONG width, SIZE *size);
	
protected:
	virtual BOOL OnInitDialog(HWND hFocus, LPARAM param);
	virtual void OnDestroy();
	virtual void OnCommand(UINT commandId, UINT eventType, HWND hControl);
	virtual LRESULT OnNotify(UINT controlId, const NMHDR *pnmh);
	virtual HBRUSH OnGetStaticColor(HDC hdc, HWND hControl);
	virtual void OnParentNotify(UINT eventId, UINT wParam, LPARAM lParam);

	void OnPaint();
	void OnWindowPosChanged(const WINDOWPOS *pwp);
	void OnPrintClient(HDC hdc, UINT options);
	void OnSetFont(HFONT font, BOOL redraw);
				
	virtual BOOL OnUpdateWindowPos(const RECT* clientRect, RECT *rectOut);
	virtual void OnPlayBeep();
	
	virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	friend static INT_PTR CALLBACK LoginPopup_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	friend static LRESULT CALLBACK LoginPopup_MessageFilter(INT code, WPARAM wParam, LPARAM lParam);

protected:
	HWND hwnd;
	SIZE idealSize;
	RECT clientMargins;
	RECT infoMargins;
	LONG buttonHeight;
	LONG buttonSpace;
	UINT popupType;
	UINT alertType;
	LPWSTR alertMessage;
};

#endif //NULLSOFT_AUTH_LOGINPOPUP_HEADER