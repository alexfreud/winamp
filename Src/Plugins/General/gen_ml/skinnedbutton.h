#ifndef NULLOSFT_MEDIALIBRARY_SKINNED_BUTTON_HEADER
#define NULLOSFT_MEDIALIBRARY_SKINNED_BUTTON_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./skinnedwnd.h"

#define BUTTON_TEXT_MAX 512

typedef struct _BUTTONPAINTSTRUCT
{
	HDC	hdc;
	RECT rcClient;
	RECT rcPaint;
    UINT buttonState;
	UINT windowStyle;
	UINT skinnedStyle;
	HBITMAP hImage;
	INT		imageIndex;
	UINT textFormat;
	HFONT hFont;
	COLORREF rgbText;
	COLORREF rgbTextBk;
	INT cchText;
	wchar_t szText[BUTTON_TEXT_MAX];
} BUTTONPAINTSTRUCT;

class SkinnedButton : public SkinnedWnd
{
protected:
	SkinnedButton(void);
	virtual ~SkinnedButton(void);

protected:
	virtual BOOL Attach(HWND hwndButton);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam); // treat this as dialog proc
	virtual void OnPaint(void);
	virtual void OnPrintClient(HDC hdc, UINT options);

	virtual void OnSkinChanged(BOOL bNotifyChildren, BOOL bRedraw);
	virtual BOOL OnMediaLibraryIPC(INT msg, INT_PTR param, LRESULT *pResult);
	virtual void OnLButtonDown(UINT flags, POINTS pts);
	virtual void OnLButtonUp(UINT flags, POINTS pts);
	virtual void OnMouseMove(UINT flags, POINTS pts);
	virtual LRESULT GetIdealSize(LPCWSTR pszText);

protected:
	void Emulate_LeftButtonDown(UINT flags, POINTS pts, BOOL forwardMessage);
	void Emulate_LeftButtonUp(UINT flags, POINTS pts, BOOL forwardMessage);

public:
	static COLORREF GetButtonBkColor(BOOL bPressed, BOOL *pbUnified);
	static void PrePaint(HWND hwnd, BUTTONPAINTSTRUCT *pbps, UINT skinStyle, UINT uiStat);
	static void DrawButton(BUTTONPAINTSTRUCT *pbps);
	static void DrawPushButton(BUTTONPAINTSTRUCT *pbps);
	static void DrawGroupBox(BUTTONPAINTSTRUCT *pbps);
	static void DrawCheckBox(BUTTONPAINTSTRUCT *pbps);
	static void DrawRadioButton(BUTTONPAINTSTRUCT *pbps);
	static BOOL RegisterImageFilter(HANDLE filterMananger);

protected:
	MLBUTTONIMAGELIST imagelist;

private:
	friend BOOL SkinWindowEx(HWND hwndToSkin, INT type, UINT style);

};

#endif // NULLOSFT_MEDIALIBRARY_SKINNED_BUTTON_HEADER