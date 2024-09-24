#ifndef NULLSOFT_MEIDALIBRARY_IMAGE_SOURCE_HEADER
#define NULLSOFT_MEIDALIBRARY_IMAGE_SOURCE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>


typedef struct _MLIMAGESOURCE_I
{
	HINSTANCE		hInst;		//
	LPCWSTR			lpszName;	//
	UINT			bpp;		//
	INT				xSrc;		//
	INT				ySrc;		//
	INT				cxSrc;		//
	INT				cySrc;		//
	INT				cxDst;		//
	INT				cyDst;		//
	UINT			type;		//
	UINT			flags;		// 
} MLIMAGESOURCE_I;

// Image Source types:
#define SRC_TYPE_BMP_I			0x01	// hInst = depends on ISF_LOADFROMFILE, lpszName = resource (file) name. To make resource name from resource id use MAKEINTERESOURCEW().
#define SRC_TYPE_PNG_I			0x02		// hInst = depends on ISF_LOADFROMFILE, lpszName = resource (file) name. To make resource name from resource id use MAKEINTERESOURCEW().
#define SRC_TYPE_HBITMAP_I		0x03	// hInst = NULL, lpszName =(HBITMAP)hbmp.
#define SRC_TYPE_HIMAGELIST_I	0x04	// hInst = (HIMAGELIST)himl, lpszName = MAKEINTERESOURCEW(index). Make Sure that common controls initialized before using this.

// Image Source flags:
#define ISF_LOADFROMFILE_I		0x0001	// Load image from file. hInst ignored.
#define ISF_USE_OFFSET_I		0x0002	// xSrc and ySrc valid.
#define ISF_USE_SIZE_I			0x0004	// cxSrc and cySrc valid.

#define ISF_FORCE_BPP_I		0x0010	//  
#define ISF_FORCE_SIZE_I	0x0020	//
#define ISF_SCALE_I			0x0040	//  

#define ISF_PREMULTIPLY_I	0x0100 // supported only by png
#define ISF_NOLOCALIZED_LOAD_I 0x0200 // when loading res:// protocol do not try to load resource from localized resource first


HBITMAP MLImageLoaderI_LoadDib(const MLIMAGESOURCE_I *pImgSource);
BOOL MLImageLoaderI_CopyData(MLIMAGESOURCE_I *pisDst, const MLIMAGESOURCE_I *pisSrc); /// Creates copy of MLIMAGESOURCE. 
BOOL MLImageLoaderI_FreeData(MLIMAGESOURCE_I *pisDst); /// use it to properly free MLIMAGESOURCE that was set using MLImageLoader_CopyData
BOOL MLImageLoaderI_CheckExist(const MLIMAGESOURCE_I *pis);



#endif //NULLSOFT_MEIDALIBRARY_IMAGE_SOURCE_HEADER