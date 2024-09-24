#ifndef WINAMP_SETUP_API_HEADER
#define WINAMP_SETUP_API_HEADER

#include <windows.h>


class __declspec(novtable) WASetupAPI
{
protected:
	WASetupAPI(void){};
	virtual ~WASetupAPI(void) = 0;
public:
	virtual INT GetInterfaceVersion(void) = 0;
	virtual LPCWSTR GetName(BOOL bShort) = 0;
	virtual HICON GetIcon(BOOL bSmall) = 0;
	virtual BOOL Initialize(void) = 0;
	virtual BOOL Finish(BOOL bCancelled) = 0;
	virtual HWND CreateView(HWND hwndParent) = 0;
	virtual HWND GetHWND(void) = 0;
};



#endif //WINAMP_SETUP_API_HEADER