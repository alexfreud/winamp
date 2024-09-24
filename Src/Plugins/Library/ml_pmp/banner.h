#ifndef NULLSOFT_ML_BANNER_HEADER
#define NULLSOFT_ML_BANNER_HEADER

#include <windows.h>

class MLBanner
{
public:
	MLBanner(void);
	~MLBanner(void);

public:

	void SetColors(int color1, int color2);
	void SetImages(HINSTANCE hInstance, int bgndResId, int logoResId);
	void Init(HWND hwnd);
	void ReloadImages(void);

protected:
	void DestroyImages(void);
	void UpdateBunnerBmp(void);
	static INT_PTR CALLBACK newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

private:

	HWND m_hwnd;
	HBITMAP bmpBck;
	HBITMAP bmpLogo;
	HBITMAP bmpLogoMask;
	HBITMAP bmpBanner;

	WNDPROC oldWndProc;

	HINSTANCE hInstance;
	int logoResId; 
	int bgndResId;

	int color1;
	int color2;

	RECT rcBanner;
};

#endif // NULLSOFT_ML_BANNER_HEADER