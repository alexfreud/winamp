#ifndef NULLSOFT_AUTH_LOGINBOX_IMAGELOADER_HEADER
#define NULLSOFT_AUTH_LOGINBOX_IMAGELOADER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

void *ImageLoader_LoadData(HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply, INT *widthOut, INT *heightOut);
void ImageLoader_FreeData(void *data);
HBITMAP ImageLoader_LoadBitmapEx(HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply, BITMAPINFOHEADER *headerInfo, void **dataOut);
HBITMAP ImageLoader_LoadBitmap(HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply, INT *widthOut, INT *heightOut);
BOOL ImageLoader_GetDimensions(HINSTANCE hInstance, LPCWSTR pszName, INT *widthOut, INT *heightOut);

#endif //NULLSOFT_AUTH_LOGINBOX_IMAGELOADER_HEADER