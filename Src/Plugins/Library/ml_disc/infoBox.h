#ifndef NULLSOFT_ML_INFOBOX_HEADER
#define NULLSOFT_ML_INFOBOX_HEADER

#include <windows.h>

#define CAPTION_LENGTH 64
class MLInfoBox
{
public:
	MLInfoBox(void);
	~MLInfoBox(void);

public:

	void SetColors(COLORREF bodyBG, COLORREF headerFG, COLORREF headerBG);
	void Init(HWND hwnd);

protected:
	static LRESULT CALLBACK newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
	void SetSize(int cx, int cy);

private:

	HWND m_hwnd;
	WNDPROC oldWndProc;

	wchar_t headerText[CAPTION_LENGTH];
	BOOL drawHeader;

	COLORREF headerBG;
	COLORREF headerFG;
	COLORREF bodyBG;

	HFONT headerFont;
	
	HBRUSH headerBrush;
	HBRUSH bodyBrush;
	RECT rcBody;
	RECT rcHeader;
};

#endif // NULLSOFT_ML_INFOBOX_HEADER