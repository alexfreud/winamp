#ifndef NULLSOFT_ML_IMAGE_HEADER
#define NULLSOFT_ML_IMAGE_HEADER

#include <windows.h>

#define RGBA(r,g,b,a)  ((COLORREF)(((BYTE)(r)|((WORD)(g)<<8))|(((DWORD)(BYTE)(b))<<16)|(((DWORD)(BYTE)(a))<<24)))
#define FIXCOLORREF(clr) RGBA(GetBValue(clr),GetGValue(clr), GetRValue(clr),((DWORD)(clr)) >> 24)

// loader function will be called every time MLImage need to 
// reload picture. Input parameter - handle to the calling object
// Output - loaded bitmap
typedef HBITMAP (*IMGLOADFUNC)(INT_PTR handle);

class MLImage
{
public:
	MLImage(void);
	MLImage(IMGLOADFUNC loader, BOOL deleteDone);
	MLImage(int width, int height);
	~MLImage(void);

public: 
	// sets the loader function and returns handle to the class or NULL if error
	// loader	  - pointer to the loader function
	// deleteDone - if TRUE MLImage will delete HBITMAP object from loader every time it is done loading
	// forceLoad  - forcing to load bitamp immedialty by calling Load()
	INT_PTR SetLoader(IMGLOADFUNC loader, BOOL deleteDone, BOOL forceLoad); 
	BOOL Load(void); // load image 

	MLImage* Init(int width, int height); // init image (allocates memory)
	MLImage* Init(int width, int height, COLORREF color); // init image (allocates memory) and set 

	BOOL Draw(HDC hdcDest, int destX, int destY, int destWidth, int destHeight, int sourceX, int sourceY); // draw image
	BOOL Draw(HDC hdcDest, int destX, int destY); // draw image
	
public:	
	int GetWidth(void) const;
	int GetHeight(void) const;
	void* GetData(void) const;
	

private:
	void ResetData(void);

public:
	static MLImage* Copy(MLImage* destination, const MLImage* original);// copy  all data from the original object (including image data) to the destination
	
private:
	static HBITMAP ConvertTo32BppDIB(HBITMAP bmpHandle, int bmpWidth, int bmpHeight, LPBITMAPINFO bmpInfo, LPVOID *bmpData);

private:
	IMGLOADFUNC loader; // pointer to the loader function
	BOOL loaderDelete;	// TRUE - delete HBITMAP from loader after load 
	
	HBITMAP		hbmp;       // my bitmap
	BITMAPINFO	info;
	void		*data;
};

#endif // NULLSOFT_ML_IMAGE_HEADER

