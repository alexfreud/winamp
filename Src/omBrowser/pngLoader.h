#ifndef NULLSOFT_WINAMP_OMIMAGELOADER_PNG_HEADER
#define NULLSOFT_WINAMP_OMIMAGELOADER_PNG_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_imageloader.h"

class ifc_wasabihelper;

class PngLoader : public ifc_omimageloader
{
protected:
	PngLoader(HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply);
	~PngLoader();

public:
	static HRESULT CreateInstance(HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply, ifc_omimageloader **imageLoader);

public:
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	HRESULT (LoadData)(int *widthOut, int *heightOut, void **dataOut);
	HRESULT (FreeData)(void *data);
	HRESULT (LoadBitmapEx)(HBITMAP *bitmapOut, BITMAPINFOHEADER *headerInfo, void **dataOut);
	HRESULT (LoadBitmap)(HBITMAP *bitmapOut, int *widthOut, int *heightOut);

protected:
	RECVS_DISPATCH;

	typedef enum 
	{
		flagPremultiply = 0x0001,
	} Flags;

protected:
	ULONG ref;
	HINSTANCE instance;
	LPWSTR name;
	UINT flags;
};

#endif //NULLSOFT_WINAMP_OMIMAGELOADER_PNG_HEADER