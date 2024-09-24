#ifndef NULLSOFT_DRIVE_COMBOBOX_HEADER
#define NULLSOFT_DRIVE_COMBOBOX_HEADER

#include "windows.h"

class DriveListBox
{
public:
	DriveListBox(int controlId);
	~DriveListBox(void);

public:
	void SetColors(COLORREF clrNormalBG, COLORREF clrSelected1, COLORREF clrSelected2, COLORREF clrTextSel, COLORREF clrTextNorm);
	void SetImages(HINSTANCE hInstance, int bgndResId, int driveResId);
	void Init(HWND hwnd);
	int HandleMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ReloadImages(void);
	HWND GetHWND(void);

private:
	void DestroyImages(void);
	void CreateBitmaps(HBITMAP bmpBck, HBITMAP bmpDrive);

	void DrawItem(LPDRAWITEMSTRUCT di);
	int MeasureItem(LPMEASUREITEMSTRUCT mi);
	
private:

	HWND m_hwnd, m_parentHwnd;

	HINSTANCE hInstance;

	HBITMAP bmpNormal;
	HBITMAP bmpSelected;

	int driveResId; 
	int bgndResId;

	RECT rcItem;
	
	int controlId;

	COLORREF clrNormalBG;
	COLORREF clrSelected1;
	COLORREF clrSelected2;
	COLORREF clrTextSel;
	COLORREF clrTextNorm;
};

#endif // NULLSOFT_DRIVE_COMBOBOX_HEADER