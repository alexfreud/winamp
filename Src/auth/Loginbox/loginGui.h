#ifndef NULLSOFT_AUTH_LOGIN_GUI_OBJECT_HEADER
#define NULLSOFT_AUTH_LOGIN_GUI_OBJECT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class LoginGuiObject
{
public: 
	typedef enum
	{
		iconNone = -1,
		iconInfo = 0,
		iconWarning = 1,
		iconError = 2, 
		iconQuestion = 3,
	} IconType;

protected:
	LoginGuiObject();
	~LoginGuiObject();

public:
	static HRESULT InitializeThread();
	static HRESULT UninitializeThread();

	static HRESULT QueryInstance(LoginGuiObject **instance);

public:
	ULONG AddRef();
	ULONG Release();

	
	HRESULT Reset();

	HRESULT GetIconDimensions(INT *pWidth, INT *pHeight);
	HBITMAP GetIcon(INT iconId, RECT *prcIcon);
	HFONT GetTitleFont();
	HFONT GetEditorFont();
	HFONT GetTextFont();

private:
	ULONG	ref;
	HBITMAP bitmapIcons;
	HFONT	fontTitle;
	HFONT	fontEditor;
	HFONT	fontText;
};

#endif //NULLSOFT_AUTH_LOGIN_GUI_OBJECT_HEADER