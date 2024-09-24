#ifndef NULLSOFT_WINAMP_OMIMAGELOADER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMIMAGELOADER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

class __declspec(novtable) ifc_omimageloader : public Dispatchable
{
protected:
	ifc_omimageloader() {}
	~ifc_omimageloader() {}

public:
	HRESULT LoadData(int *widthOut, int *heightOut, void **dataOut);
	HRESULT FreeData(void *data);

	HRESULT LoadBitmapEx(HBITMAP *bitmapOut, BITMAPINFOHEADER *headerInfo, void **dataOut);
	HRESULT LoadBitmap(HBITMAP *bitmapOut, int *widthOut, int *heightOut);

public:
	DISPATCH_CODES
	{
		API_LOADDATA		= 10,
		API_FREEDATA		= 20,
		API_LOADBITMAP		= 30,
		API_LOADBITMAPEX	= 40,
	};
};

inline HRESULT ifc_omimageloader::LoadData(int *widthOut, int *heightOut, void **dataOut)
{
	return _call(API_LOADDATA, (HRESULT)E_NOTIMPL, widthOut, heightOut, dataOut);
}

inline HRESULT ifc_omimageloader::FreeData(void *data)
{
	return _call(API_FREEDATA, (HRESULT)E_NOTIMPL, data);
}

inline HRESULT ifc_omimageloader::LoadBitmapEx(HBITMAP *bitmapOut, BITMAPINFOHEADER *headerInfo, void **dataOut)
{
	return _call(API_LOADBITMAPEX, (HRESULT)E_NOTIMPL, bitmapOut, headerInfo, dataOut, bitmapOut);
}

inline HRESULT ifc_omimageloader::LoadBitmap(HBITMAP *bitmapOut, int *widthOut, int *heightOut)
{
	return _call(API_LOADBITMAP, (HRESULT)E_NOTIMPL, bitmapOut, widthOut, heightOut);
}

#endif //NULLSOFT_WINAMP_OMIMAGELOADER_INTERFACE_HEADER