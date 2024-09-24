#ifndef NULLSOFT_WINAMP_ML_NAVIGATION_CALLBACK_INTERFACE_HEADER
#define NULLSOFT_WINAMP_ML_NAVIGATION_CALLBACK_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {9CBB5A7C-CEC9-4f12-9615-49BC7BEFE8CB}
static const GUID IFC_MlNavigationCallback = 
{ 0x9cbb5a7c, 0xcec9, 0x4f12, { 0x96, 0x15, 0x49, 0xbc, 0x7b, 0xef, 0xe8, 0xcb } };

#include <bfc/dispatch.h>

class __declspec(novtable) ifc_mlnavigationcallback : public Dispatchable
{
protected:
	ifc_mlnavigationcallback() {}
	~ifc_mlnavigationcallback() {}

public:
	void ImageChanged(const wchar_t *name, int index);
    
public:
	DISPATCH_CODES
	{
		API_IMAGECHANGED	 = 10,	
	};
};

inline void ifc_mlnavigationcallback::ImageChanged(const wchar_t *name, int index)
{
	_voidcall(API_IMAGECHANGED, name, index); 
}

#endif // NULLSOFT_WINAMP_ML_NAVIGATION_CALLBACK_INTERFACE_HEADER