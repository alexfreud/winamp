#include ".\image.h"

MLImage::MLImage(void)
{
	loader	= NULL;
	loaderDelete = TRUE;
	ResetData();
}

MLImage::MLImage(IMGLOADFUNC loader, BOOL deleteDone)
{
	ResetData();
	SetLoader(loader, deleteDone, FALSE);
}

MLImage::MLImage(int width, int height)
{
	loader	= NULL;
	ResetData();
	Init(width,height);
}

MLImage::~MLImage(void)
{
	ResetData();
}

INT_PTR MLImage::SetLoader(IMGLOADFUNC loader, BOOL deleteDone, BOOL forceLoad)
{
	this->loader = loader;
	this->loaderDelete = deleteDone;
	if (loader && forceLoad) Load();
	return (loader != NULL) ? (INT_PTR) this : FALSE;
}

BOOL MLImage::Load(void)
{
	ResetData();

	if (!loader) return FALSE;
	HBITMAP hbmpLoaded = loader((INT_PTR)this);
	if(hbmpLoaded == NULL) return FALSE;

	BITMAP bi;
	if (GetObject(hbmpLoaded, sizeof(bi), &bi))
	{
		hbmp = ConvertTo32BppDIB(hbmpLoaded, bi.bmWidth, bi.bmHeight, &info, &data);
	}
	
	if (loaderDelete) DeleteObject(hbmpLoaded);
	return (hbmp != NULL);
}

void MLImage::ResetData(void)
{
	if (hbmp) DeleteObject(hbmp);
	hbmp = NULL;
	SecureZeroMemory(&info, sizeof(BITMAPINFO));
	data = NULL;
}

BOOL MLImage::Draw(HDC hdcDest, int destX, int destY, int destWidth, int destHeight, int sourceX, int sourceY)
{
	if (!hbmp) return FALSE;
	
	int realheight = abs(info.bmiHeader.biHeight);
	int rsX = min(sourceX, info.bmiHeader.biWidth);
	int rsY = min(sourceY, info.bmiHeader.biWidth);
	int height = min(destHeight, realheight - rsY);

	BOOL bResult = SetDIBitsToDevice(	hdcDest, destX, destY, 
										min(destWidth, info.bmiHeader.biWidth - rsX), height,
										rsX, realheight - height - rsY,
										0, height, 
										data, &info, DIB_RGB_COLORS); 
	return bResult;
}

BOOL MLImage::Draw(HDC hdcDest, int destX, int destY)
{
	return (!hbmp) ? FALSE : SetDIBitsToDevice(	hdcDest, destX, destY, 
										info.bmiHeader.biWidth,  abs(info.bmiHeader.biHeight),
										0, 0,
										0, abs(info.bmiHeader.biHeight), 
										data, &info, DIB_RGB_COLORS); 
}

int MLImage::GetWidth(void) const
{
	return (hbmp) ? info.bmiHeader.biWidth : 0;
}

int MLImage::GetHeight(void) const
{
	return (hbmp) ? abs(info.bmiHeader.biHeight) : 0;
}

void* MLImage::GetData(void) const
{
	return data;
}

HBITMAP MLImage::ConvertTo32BppDIB(HBITMAP bmpHandle, int bmpWidth, int bmpHeight, LPBITMAPINFO bmpInfo, LPVOID *bmpData)
{
	HBITMAP hbmpNew = NULL;

	HDC hdc = GetWindowDC(NULL); 
	HDC hdcTmp = CreateCompatibleDC(hdc);
	HBITMAP hbmpTmp = CreateCompatibleBitmap(hdc, bmpWidth, bmpHeight);
	HBITMAP hbmpOld = (HBITMAP) SelectObject(hdcTmp, hbmpTmp);

	// render original bitmap to the temp dc
	HDC hdcBmp = CreateCompatibleDC(hdc);
	SelectObject(hdcBmp, bmpHandle);
	BitBlt(hdcTmp, 0, 0, bmpWidth, bmpHeight, hdcBmp, 0,0, SRCCOPY);
	SelectObject(hdcBmp, NULL);
	DeleteDC(hdcBmp);

	// Create a 32 bit bitmap
	BITMAPINFO bih;
    // create DIB Section
    bih.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    bih.bmiHeader.biWidth         = bmpWidth; 
    bih.bmiHeader.biHeight        = 0 - bmpHeight; 
    bih.bmiHeader.biPlanes        = 1; 
    bih.bmiHeader.biBitCount      = 32; 
    bih.bmiHeader.biCompression   = BI_RGB; 
    bih.bmiHeader.biSizeImage     = 0; 
    bih.bmiHeader.biXPelsPerMeter = 0; 
    bih.bmiHeader.biYPelsPerMeter = 0; 
    bih.bmiHeader.biClrUsed       = 0; 
    bih.bmiHeader.biClrImportant  = 0; 

	// Create a DC which will be used to get DIB, then create DIBsection
	 hbmpNew = CreateDIBSection(hdc, (const BITMAPINFO*) &bih, DIB_RGB_COLORS, bmpData, NULL, 0);


	DWORD* line = (DWORD*)(*bmpData);
	// Copy the bits into our 32 bit dib..
	for(int i=0; i<bmpHeight; i++)
	{
		for(int j=0; j<bmpWidth; j++)
		{
			line[(i*bmpWidth) + j] = FIXCOLORREF(GetPixel(hdcTmp, j, i));
		}
	}

	SelectObject(hdcTmp, hbmpOld);
	ReleaseDC(NULL, hdc);
	DeleteDC(hdcTmp);

	memcpy(bmpInfo, &bih, sizeof(BITMAPINFO));

	return hbmpNew;
}

MLImage* MLImage::Copy(MLImage* destination, const MLImage* original)
{
	if (!destination) return NULL;

	destination->ResetData();

	destination->loader = original->loader;
	destination->loaderDelete = original->loaderDelete;
	destination->info = original->info;
	HDC hdc = GetWindowDC(NULL); 
	destination->hbmp = CreateDIBSection(hdc, (const BITMAPINFO*) &destination->info, DIB_RGB_COLORS, &destination->data, NULL, 0);

	CopyMemory(destination->data, original->data, 4*destination->GetHeight() * destination->GetWidth());
	ReleaseDC(NULL, hdc);
	return destination;
}

MLImage* MLImage::Init(int width, int height)
{
	ResetData();

	loader = NULL;
	loaderDelete = TRUE;

    // create DIB Section
    info.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth         = width; 
    info.bmiHeader.biHeight        = 0 - height; 
    info.bmiHeader.biPlanes        = 1; 
    info.bmiHeader.biBitCount      = 32; 
    info.bmiHeader.biCompression   = BI_RGB; 
    info.bmiHeader.biSizeImage     = 0; 
    info.bmiHeader.biXPelsPerMeter = 0; 
    info.bmiHeader.biYPelsPerMeter = 0; 
    info.bmiHeader.biClrUsed       = 0; 
    info.bmiHeader.biClrImportant  = 0; 

	HDC hdc = GetWindowDC(NULL); 
	hbmp = CreateDIBSection(hdc, (const BITMAPINFO*) &info, DIB_RGB_COLORS, &data, NULL, 0);
	ReleaseDC(NULL, hdc);
	return this;
}

MLImage* MLImage::Init(int width, int height, COLORREF color)
{
	Init(width, height);

	int rColor = FIXCOLORREF(color);
	DWORD *line = (DWORD*)(data);
	DWORD *end = line + GetHeight() * GetWidth();
	for(;line != end; line++)	*line = rColor;
	return this;
}