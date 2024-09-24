#ifndef NULLOSFT_MEDIALIBRARY_IPC_EXTENSION_HEADER_0x0313
#define NULLOSFT_MEDIALIBRARY_IPC_EXTENSION_HEADER_0x0313

#include <wtypes.h>

////////////////////////////////////////////////////////////////////
// Notifications 
//
//

#define ML_MSG_NAVIGATION_FIRST				0x106							// base

#define ML_MSG_NAVIGATION_ONMOVE			(ML_MSG_NAVIGATION_FIRST + 1)	// Notifies that item was moved. 
																			// param1 = item handle. param2 - item old sort order, param3 - item new sort order
																			// Return value ignored. 
#define ML_MSG_NAVIGATION_ONDELETE			(ML_MSG_NAVIGATION_FIRST + 2)	// Notifies that item is being deleted. 
																			// param1 = item handle.
																			// Return value ignored. 
#define ML_MSG_NAVIGATION_ONBEGINTITLEEDIT	(ML_MSG_NAVIGATION_FIRST + 3)	// Notifies about the start of title editing for an item. 
																			// param1 = item handle. 
																			// Return TRUE to cancel title editing. 
#define ML_MSG_NAVIGATION_ONENDTITLEEDIT	(ML_MSG_NAVIGATION_FIRST + 4)	// Notifies about the end of title editing for an item. 
																			// param1 = item handle, param2 = (LPCWSTR)pszNewTitle. 
																			// Return TRUE to accept new title. if (NULL == pszNewText) editing was canceled and return value ignored.

#define ML_MSG_NAVIGATION_ONCUSTOMDRAW		(ML_MSG_NAVIGATION_FIRST + 5)	// Perform custom draw here. 
																			// param1 = item handle, param2 = (NAVITEMDRAW*)pnicd, param3 = (LPARAM)lParam.
																			// Return combination of the NICDRF_XXX flags.

#define ML_MSG_NAVIGATION_ONHITTEST			(ML_MSG_NAVIGATION_FIRST + 6)	// Perform custom hit test 
																			// param1 = item handle, param2 = (NAVHITTEST*)pnavHitTest, param3 = (LPARAM)lParam.
																			// Return non zero if you handle it.

#define ML_MSG_NAVIGATION_ONSETCURSOR		(ML_MSG_NAVIGATION_FIRST + 7)	// Set cursor. 
																			// param1 = item handle, param2 = 0, param3 = (LPARAM)lParam.
																			// Return 1 if you proccessed this messeage and set cursor, -1 if you want default processing, 0 - if not yours.

#define ML_MSG_NAVIGATION_ONDESTROY			(ML_MSG_NAVIGATION_FIRST + 8)	// Notifies that navigation control about to be destroyed.
																			// param1 = not used, param2 = not used, param3 = not used.
																			// Return value ignored. 

#define ML_MSG_NAVIGATION_CONTEXTMENU		(ML_MSG_NAVIGATION_FIRST + 9)	// Notifies that context menu was requested for the item
																			// param1 = item handle, param2 = (HWND)hHost, param3 = (POINTS)cursor
																			// Return non zero if you handle it.

#define ML_MSG_NAVIGATION_HELP				(ML_MSG_NAVIGATION_FIRST + 10)	// Notifies that help was requested for the item
																			// param1 = item handle, param2 = (HWND)hHost, param3 = (POINTS)cursor
																			// Return non zero if you handle it.

#define ML_MSG_MLVISIBLE					0x410							// Notifies that Media Library Window changed visible state.
																			// param1 = (BOOL)fVisible, param2 = Not used, param3 = Not used.
																			// Return value ignored. (Return zero). 


/////////////////////////////////////////////////////////////////////
// Drag&Drop for outsiders <<< will be discontinued at some point >>>
//

#define WAMLM_DRAGDROP			L"WaMediaLibraryDragDrop"	// use RegisterWindowMessage

// values for wParam
#define DRAGDROP_DRAGENTER		0	// lParam = (LPARAM)(mlDropItemStruct*)pdis. return TRUE if you accepted this message. 
#define DRAGDROP_DRAGLEAVE		1	// lParam = not used. Return: nothing.
#define DRAGDROP_DRAGOVER		2	// lParam = (LPARAM)(mlDropItemStruct*)pdis. Return: nothing.
#define DRAGDROP_DROP			3	// (LPARAM)(mlDropItemStruct*)pdis. Return: nothing.

////////////////////////////////////////////////////////////////////
// IPC
//


#ifdef __cplusplus
#define SENDMSG(__hwnd, __msgId, __wParam, __lParam) ::SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#else
#define SENDMSG(__hwnd, __msgId, __wParam, __lParam) SendMessageW((__hwnd), (__msgId), (__wParam), (__lParam))
#endif // __cplusplus

#define SENDMLIPC(__hwndML, __ipcMsgId, __param) SENDMSG((__hwndML), WM_ML_IPC, (WPARAM)(__param), (LPARAM)(__ipcMsgId))
#define SENDWAIPC(__hwndWA, __ipcMsgId, __param) SENDMSG((__hwndWA), WM_WA_IPC, (WPARAM)(__param), (LPARAM)(__ipcMsgId))


/////////////////////////////////////////////////////////////////
// Getting MediaLibrary (gen_ml) version
// if version is less then 3.13 return value will be 0.

// Build types
#define ML_BUILDTYPE_FINAL		0x0000
#define ML_BUILDTYPE_BETA		0x0001
#define ML_BUILDTYPE_NIGHTLY	0x0002

// Message
#define ML_IPC_GETVERSION		0x0000L		// Returns DWORD where LOWORD contains version number and HIWORD - build type . param is ignored.
#define GetMLVersion(/*HWND*/ hwndML) (DWORD)SENDMLIPC(hwndML, ML_IPC_GETVERSION, 0)

#define ML_IPC_SHOW_HELP		(0x1200L - 1)	// Shows Help. param = (const wchar_t*)helpUrl, Returns TRUE on SUCCESS.
#define MediaLibrary_ShowHelp(/*HWND*/ _hwndML, /* const wchar_t* */ _helpUrl)\
							  ((BOOL)SENDMLIPC((_hwndML), ML_IPC_SHOW_HELP, (WPARAM)(_helpUrl)))

//////////////////////////////////////////////////////////////////
// MLImageLoader
// 
//
//

/// Structs
typedef struct _MLIMAGESOURCE
{
	INT				cbSize;		// sizeof(MLIMAGESOURCE)
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
} MLIMAGESOURCE;

// Image Source types:
#define SRC_TYPE_BMP			0x01	// hInst = depends on ISF_LOADFROMFILE, lpszName = resource (file) name. To make resource name from resource id use MAKEINTERESOURCEW().
#define SRC_TYPE_PNG			0x02	// hInst = depends on ISF_LOADFROMFILE, lpszName = resource (file) name. To make resource name from resource id use MAKEINTERESOURCEW().
#define SRC_TYPE_HBITMAP		0x03	// hInst = NULL, lpszName =(HBITMAP)hbmp.
#define SRC_TYPE_HIMAGELIST		0x04	// hInst = (HIMAGELIST)himl, lpszName = MAKEINTERESOURCEW(index). Make Sure that common controls initialized before using this.

// Image Source flags:
#define ISF_LOADFROMFILE	0x0001	// Load image from file. hInst ignored.
#define ISF_USE_OFFSET		0x0002	// xSrc and ySrc valid.
#define ISF_USE_SIZE		0x0004	// cxSrc and cySrc valid.

#define ISF_FORCE_BPP		0x0010	//
#define ISF_FORCE_SIZE		0x0020	//
#define ISF_SCALE			0x0040	//

#define ISF_PREMULTIPLY	0x0100			// supported only by png
#define ISF_NOLOCALIZED_LOAD 0x0200		// when loading res:// protocol do not try to load resource from localized resource first


// use with ML_IPC_IMAGESOURCE_COPYSTRUCT
typedef struct _MLIMAGESOURCECOPY
{
	MLIMAGESOURCE *src;		// pointer to the source struct
	MLIMAGESOURCE *dest;	// pointer to the destination struct
} MLIMAGESOURCECOPY;

// Messages

#define ML_IPC_IMAGELOADER_FIRST				0x1200L 

#define ML_IPC_IMAGELOADER_LOADDIB			(ML_IPC_IMAGELOADER_FIRST + 1) // Loads Dib from source. param = (MLIMAGESOURCE*)pmlis;
#define ML_IPC_IMAGELOADER_COPYSTRUCT		(ML_IPC_IMAGELOADER_FIRST + 2) // Creates copy of image source struct. Use ML_IPC_IMAGESOURCE_FREESTRUCT to free copy data. Returns TRUE if success. param = (MLIMAGESOURCECOPY*)pmlis;
#define ML_IPC_IMAGELOADER_FREESTRUCT		(ML_IPC_IMAGELOADER_FIRST + 3) // Use it to free  MLIMAGESOURCE that was filled using ML_IPC_IMAGESOURCE_COPYSTRUCT. Returns TRUE if sucess. param = (MLIMAGESOURCE*)pmlis;
#define ML_IPC_IMAGELOADER_CHECKEXIST		(ML_IPC_IMAGELOADER_FIRST + 4) // Performs simple checks to validate source. Returns TRUE if sucess. param = (MLIMAGESOURCE*)pmlis;


// Macros
#define MLImageLoader_LoadDib(/*HWND*/ hwndML, /*MLIMAGESOURCE**/ pmlisStruct) \
							  (HBITMAP)SENDMLIPC((hwndML), ML_IPC_IMAGELOADER_LOADDIB, (WPARAM)(pmlisStruct))

#define MLImageLoader_CopyStruct(/*HWND*/ hwndML, /*MLIMAGESOURCECOPY**/ pmlisCopyStrust) \
								 ((BOOL)SENDMLIPC((hwndML), ML_IPC_IMAGELOADER_COPYSTRUCT, (WPARAM)(pmlisCopyStrust)))

#define MLImageLoader_FreeStruct(/*HWND*/ hwndML, /*MLIMAGESOURCE*/ pmlisStruct) \
								 ((BOOL)SENDMLIPC((hwndML), ML_IPC_IMAGELOADER_FREESTRUCT, (WPARAM)(pmlisStruct)))

#define MLImageLoader_CheckExist(/*HWND*/ hwndML, /*MLIMAGESOURCE**/ pmlisStruct) \
								 ((BOOL)SENDMLIPC((hwndML), ML_IPC_IMAGELOADER_CHECKEXIST, (WPARAM)(pmlisStruct)))


//////////////////////////////////////////////////////////////////////////////
// MLImageFilter
//
//
//
// Filter callback
typedef BOOL (CALLBACK *MLIMAGEFILTERPROC)(LPBYTE /*pData*/, LONG /*cx*/, LONG /*cy*/, INT /*bpp*/, COLORREF /*rgbBk*/,
										   COLORREF /*rgbFg*/, INT_PTR /*imageTag*/, LPARAM /*lParam*/);

typedef struct _MLIMAGEFILTERINFO
{
	INT					cbSize;			// sizeof(MLIMAGEFILTERINFO)
	UINT				mask;			// mask 
	GUID				uid;			// filterUID
	MLIMAGEFILTERPROC	fnProc;			// procedure to call 
	LPARAM				lParam;			// user parameter ( will be passed to every call of filter proc)
	UINT				fFlags;			// filter flags 
	LPWSTR				pszTitle;		// filter title
	INT					cchTitleMax;	// max title size (only for get info)
} MLIMAGEFILTERINFO;

typedef struct _MLIMAGEFILTERAPPLYEX
{ 
	INT			cbSize;
	GUID		filterUID;		// Filter to use.
	LPBYTE		pData;			// Pointer to dib data.
	LONG		cx;				// Image width in pixels.
	LONG		cy;				// Image height in pixels.
	INT			bpp;			// Bits per pixel
	COLORREF	rgbBk;			// Back color to use
	COLORREF	rgbFg;			// Front color to use
	INT_PTR		imageTag;		// Image tag. will be passed to the filter
} MLIMAGEFILTERAPPLYEX;

// filter flags
#define MLIFF_IGNORE_BKCOLOR		0x0001	// Filter result doesn't depend on rgbBk.
#define MLIFF_IGNORE_FGCOLOR		0x0002	// Filter result doesn't depend on rgbFg.

// mask used to set/get filter info
#define MLIFF_TITLE		0x0001	// 
#define MLIFF_PARAM		0x0002  // 
#define MLIFF_FLAGS		0x0004	//
#define MLIFF_PROC		0x0008  //


// Messages
#define ML_IPC_IMAGEFILTER_FIRST			0x1240L

#define ML_IPC_IMAGEFILTER_REGISTER			(ML_IPC_IMAGEFILTER_FIRST + 1)	// Registers filter in ml. param = (MLIMAGEFILTERINFO*)pmlif. uid and proc must be always set. Returns TRUE on success.
#define ML_IPC_IMAGEFILTER_UNREGISTER		(ML_IPC_IMAGEFILTER_FIRST + 2)	// Unregisters filter. param = (const GUID*)filterUID. Returns TRUE on success.
#define ML_IPC_IMAGEFILTER_GETINFO			(ML_IPC_IMAGEFILTER_FIRST + 3)	// Get fitler info. param = (MLIMAGEFILTERINFO*)pmlif. set mask to whatever you wnat to get. set uid  to filterUID. Returnd TRUE on success
#define ML_IPC_IMAGEFILTER_APPLYEX			(ML_IPC_IMAGEFILTER_FIRST + 4)	// Apply Filter. param = (MLIMAGEFILTERAPPLYEX)pmlif. Runs Filter over image. Returns TRUE if ok.

// Macros
#define MLImageFilter_Register(/*HWND*/ hwndML, /*MLIMAGEFILTERINFO**/ pmlifInfo) \
							   (BOOL)SENDMLIPC((hwndML), ML_IPC_IMAGEFILTER_REGISTER, (WPARAM)(pmlifInfo))

#define MLImageFilter_Unregister(/*HWND*/ hwndML, /*const GUID**/ filterUID) \
								 (BOOL)SENDMLIPC((hwndML), ML_IPC_IMAGEFILTER_UNREGISTER, (WPARAM)(filterUID)) 

#define MLImageFilter_GetInfo(/*HWND*/ hwndML, /*MLIMAGEFILTERINFO**/ pmlifInfo) \
							  (BOOL)SENDMLIPC((hwndML), ML_IPC_IMAGEFILTER_GETINFO, (WPARAM)(pmlifInfo))

#define MLImageFilter_ApplyEx(/*HWND*/ hwndML, /*MLIMAGEFILTERAPPLY**/ pmlifApply) \
							  (BOOL)SENDMLIPC((hwndML), ML_IPC_IMAGEFILTER_APPLYEX, (WPARAM)(pmlifApply))

/// Default Filters

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern const GUID MLIF_FILTER1_UID;			// {8A054D1F-E38E-4cc0-A78A-F216F059F57E}
extern const GUID MLIF_FILTER2_UID;			// {BE1A6A40-39D1-4cfb-8C33-D8988E8DD2F8}
extern const GUID MLIF_FILTER3_UID;			// {721E9E62-CC6D-4fd7-A6ED-DD4CD2B2612E}
extern const GUID MLIF_GRAYSCALE_UID;		// {B6310C20-E731-44dd-83BD-FBC3349798F2}
extern const GUID MLIF_BLENDONBK_UID;		// {526C6F4A-C979-4d6a-B8ED-1A90F5A26F7B}
extern const GUID MLIF_BUTTONBLEND_UID;		// {E61C5E67-B2CC-4f89-9C95-40D18FCAF1F8}
extern const GUID MLIF_BUTTONBLENDPLUSCOLOR_UID;	// {3619BA52-5088-4f21-9AF1-C5FCFE5AAA99}
extern const GUID MLIF_FILTER1_PRESERVE_ALPHA_UID;	// {F1DD3228-7DA8-4524-B7D3-46F651BFB680}

#ifdef __cplusplus
}
#endif // __cplusplus


//////////////////////////////////////////////////////////////////////////////
// MLImageList
// 
//
//

typedef LPVOID HMLIMGLST;

// Used with ML_IPC_IMAGELIST_CREATE
typedef struct _MLIMAGELISTCREATE
{
	INT		cx;				// image width
	INT		cy;				// image height
	UINT	flags;			// see MLIMAGELISTCREATE flags 
	INT		cInitial;		// initial size
	INT		cGrow;			// realloc step
	INT		cCacheSize;		// how many colors can be cached for each entry
}MLIMAGELISTCREATE;

// MLIMAGELISTCREATE flags
#define MLILC_COLOR24		0x00000018	// Use a 24-bit DIB section.
#define MLILC_COLOR32		0x00000020	// Use a 32-bit DIB section.
#define MLILC_MASK			0x00000001	// Creates underlying HIMAGELIST with mask parameter.

// Used with ML_IPC_IMAGELIST_GETREALINDEX
typedef struct _MLIMAGELISTREALINDEX
{
	INT			cbSize;			// sizeof(MLIMAGELISTREALINDEX)
	HMLIMGLST	hmlil;			// MLImageList handle returned from ML_IPC_IMAGELIST_CREATE.
	INT			mlilIndex;		// Image index in MLImageList
	COLORREF	rgbBk;			// Requested background color.
	COLORREF	rgbFg;			// Requested foreground color.
} MLIMAGELISTREALINDEX;

// Used with ML_IPC_IMAGELIST_ADD,  ML_IPC_IMAGELIST_REPLACE,  ML_IPC_IMAGELIST_REMOVE
typedef struct _MLIMAGELISTITEM
{
	INT				cbSize;				// sizeof(MLIMAGELISTITEM)
	HMLIMGLST		hmlil;				// MLImageList handle returned from ML_IPC_IMAGELIST_CREATE.
	MLIMAGESOURCE	*pmlImgSource;		// handle to the MLIMAGESOURCE struct.
	GUID			filterUID;			// MLImageFilter to use. // Set to GUID_NULL if you don't wnat any filtering.
	INT_PTR			nTag;				// Tag assign to image. Note: MLImageList dosn't check if tag is unique!
	INT				mlilIndex;			// MLImageList index. This field ignored in ML_IPC_IMAGELIST_ADD. 
} MLIMAGELISTITEM;

// Used with ML_IPC_IMAGELIST_GETIMAGESIZE
typedef struct _MLIMAGELISTIMAGESIZE
{
	HMLIMGLST	hmlil;			// MLImageList handle returned from ML_IPC_IMAGELIST_CREATE.
	INT			cx;				// Image width
	INT			cy;				// Image height
} MLIMAGELISTIMAGESIZE;

// Used with ML_IPC_IMAGELIST_GETINDEXFROMTAG and ML_IPC_IMAGELIST_GETTAGFROMINDEX
typedef struct _MLIMAGELISTTAG
{
	HMLIMGLST	hmlil;			// MLImageList handle returned from ML_IPC_IMAGELIST_CREATE.
	INT			mlilIndex;		// MLImageList image index. ML_IPC_IMAGELIST_GETINDEXFROMTAG will contain index if any.
	INT_PTR		nTag;			// MLImageList image tag. ML_IPC_IMAGELIST_GETTAGFROMINDEX will contain tag if any.
} MLIMAGELISTTAG;

#define ML_IPC_IMAGELIST_FIRST				0x1260L

#define ML_IPC_IMAGELIST_CREATE				(ML_IPC_IMAGELIST_FIRST + 1)	// Creates new MLImageList
#define ML_IPC_IMAGELIST_DESTROY			(ML_IPC_IMAGELIST_FIRST + 2)	// Destroys MLImageList
#define ML_IPC_IMAGELIST_GETREALLIST		(ML_IPC_IMAGELIST_FIRST + 3)	// Returns handle to the system HIMAGELIST
#define ML_IPC_IMAGELIST_GETREALINDEX		(ML_IPC_IMAGELIST_FIRST + 4)	// Returns image index in the HIMAGELIST if ok or -1 if error. If image with this colors is not cached yet it will be loaded and if Filter function speicified it will be Filtered
																			//	( user param in Filter function will be set to the tag assigned to the image)
#define ML_IPC_IMAGELIST_ADD				(ML_IPC_IMAGELIST_FIRST + 5)	// Add new item. Returns item index or -1 if failed. 
																			// When adding images IMAGESOURCE can have different bpp from imagelist. After image loaded it first filtered and than added to to the system imagelist
#define ML_IPC_IMAGELIST_REPLACE			(ML_IPC_IMAGELIST_FIRST + 6)	// Replace existing item. Returns TRUE if ok.
#define ML_IPC_IMAGELIST_REMOVE				(ML_IPC_IMAGELIST_FIRST + 7)	// Removes existing item. Returns TRUE if ok.
#define ML_IPC_IMAGELIST_GETIMAGESIZE		(ML_IPC_IMAGELIST_FIRST + 8)	// Retrives size of stored images. Returns TRUE if ok.
#define ML_IPC_IMAGELIST_GETIMAGECOUNT		(ML_IPC_IMAGELIST_FIRST + 9)	// Gets images count in MLImageList. 
#define ML_IPC_IMAGELIST_GETINDEXFROMTAG	(ML_IPC_IMAGELIST_FIRST + 10)	// Finds item with specified tag. Returns TRUE if found.
#define ML_IPC_IMAGELIST_GETTAGFROMINDEX	(ML_IPC_IMAGELIST_FIRST + 11)	// Finds item tag by index. Returns TRUE if found.
#define ML_IPC_IMAGELIST_CHECKEXIST			(ML_IPC_IMAGELIST_FIRST + 12)	// Performs simple checks to validate source. Returns TRUE ok.

// Macros
#define MLImageList_Create(/*HWND*/ hwndML, /*MLIMAGELISTCREATE**/ pmlilCreate) \
						   (HMLIMGLST)SENDMLIPC((hwndML), ML_IPC_IMAGELIST_CREATE, (WPARAM)(pmlilCreate))

#define MLImageList_Destroy(/*HWND*/ hwndML, /*HMLIMGLST*/ hmlil) \
							(BOOL)SENDMLIPC((hwndML), ML_IPC_IMAGELIST_DESTROY, (WPARAM)(hmlil))

#define MLImageList_GetRealList(/*HWND*/ hwndML, /*HMLIMGLST*/ hmlil) \
								(HIMAGELIST)SENDMLIPC((hwndML), ML_IPC_IMAGELIST_GETREALLIST, (WPARAM)(hmlil))

#define MLImageList_GetRealIndex(/*HWND*/ hwndML, /*MLIMAGELISTREALINDEX**/ pmlilRealIndex) \
								 (INT)SENDMLIPC((hwndML), ML_IPC_IMAGELIST_GETREALINDEX, (WPARAM)(pmlilRealIndex))

#define MLImageList_Add(/*HWND*/ __hwndML, /*MLIMAGELISTITEM**/ pmlilItem) \
						(INT)SENDMLIPC((__hwndML), ML_IPC_IMAGELIST_ADD, (WPARAM)(pmlilItem))

#define MLImageList_Add2(/*HWND*/ __hwndML, /*HMLIMGLST*/ __hmlil, /*GUID*/ __filterUID, /*MLIMAGESOURCE* */__pmlis, /*INT_PTR*/ __nTag) \
						{ MLIMAGELISTITEM mli;\
						mli.cbSize = sizeof(MLIMAGELISTITEM);\
						mli.hmlil = (__hmlil);\
						mli.filterUID = (__filterUID);\
						mli.pmlImgSource = (__pmlis);\
						mli.nTag = (__nTag);\
						((INT)SENDMLIPC((__hwndML), ML_IPC_IMAGELIST_ADD, (WPARAM)(&mli)));}

#define MLImageList_Replace(/*HWND*/ hwndML, /*MLIMAGELISTITEM**/ pmlilItem) \
							(BOOL)SENDMLIPC((hwndML), ML_IPC_IMAGELIST_REPLACE, (WPARAM)(pmlilItem))

#define MLImageList_Remove(/*HWND*/ hwndML, /*MLIMAGELISTITEM**/ pmlilItem) \
						   (BOOL)SENDMLIPC((hwndML), ML_IPC_IMAGELIST_REMOVE, (WPARAM)(pmlilItem))

#define MLImageList_GetImageSize(/*HWND*/ hwndML, /*MLIMAGELISTIMAGESIZE**/ pmlilImageSize) \
								 (BOOL)SENDMLIPC((hwndML), ML_IPC_IMAGELIST_GETIMAGESIZE, (WPARAM)(pmlilImageSize))

#define MLImageList_GetImageCount(/*HWND*/ hwndML, /*HMLIMGLST*/ hmlil) \
								  (INT)SENDMLIPC((hwndML), ML_IPC_IMAGELIST_GETIMAGECOUNT, (WPARAM)(hmlil))

#define MLImageList_GetIndexFromTag(/*HWND*/ hwndML, /*MLIMAGELISTTAG**/ pmlilTag)\
									(BOOL)SENDMLIPC((hwndML), ML_IPC_IMAGELIST_GETINDEXFROMTAG, (WPARAM)(pmlilTag))

#define MLImageList_GetTagFromIndex(/*HWND*/ hwndML, /*MLIMAGELISTTAG**/ pmlilTag)\
									(BOOL)SENDMLIPC((hwndML), ML_IPC_IMAGELIST_GETTAGFROMINDEX, (WPARAM)(pmlilTag))

#define MLImageList_CheckItemExist(/*HWND*/ hwndML, /*MLIMAGELISTITEM**/ pmlilItem)\
								   (BOOL)SENDMLIPC((hwndML), ML_IPC_IMAGELIST_CHECKEXIST, (WPARAM)(pmlilItem))


//////////////////////////////////////////////////////////////////////////////
// MLNavigation
// Navigation Control & Items
//
//

typedef LPVOID HNAVCTRL;
typedef LPVOID HNAVITEM;

// Structs
// Navigation Item Info
typedef struct _NAVITEM
{
	INT			cbSize;				// Always set to sizeof(NAVITEM).
	HNAVITEM	hItem;				// handle to the item (must be set for ML_IPC_NAVITEM_GETINFO and ML_IPC_NAVITEM_SETINFO)
	UINT		mask;				// Mask valid values.
	INT			id;					// Item id.
	LPWSTR		pszText;			// Text to display.
	INT			cchTextMax;			// Text buffer length (used to GET item info).
	LPWSTR		pszInvariant;		// Invariant text name. Any unique string you can assign to the item that will not change. 
									// For expample if item can be renamed by user invariat text will stay the same and can be used to save/load item state and position.
	INT			cchInvariantMax;	// Invariant text buffer size ( used to GET item info).
	INT			iImage;				// Image index. This is image index in the MLImageList assigned to the Navigation control. If you want to use default images set index to -1
	INT			iSelectedImage;		// Selected image index. This is image index in the MLImageList assigned to the Navigation control. 
									// You need/want to assign selected image index only if you want to display different image on selection.
	UINT		state;				// Item state.
	UINT		stateMask;			// Item state mask.
	UINT		style;				// Item style.
	UINT		styleMask;			// Item style mask.
	HFONT		hFont;				// Item font. You can set this if you want item to use special font. You are responsible about free font object. If you want item to use default font set hFont to NULL.
	LPARAM		lParam;				// User parameter. You can use it as you want.
} NAVITEM;

typedef struct _NAVINSERTSTRUCT
{
	HNAVITEM	hParent;			// Parent Item (NULL to insert in the root).
	HNAVITEM	hInsertAfter;		// insert after item. You can use NCI_FIRST or NCI_LAST or you can use MAKE_NAVITEMSORTORDER to specify order index instead.
	NAVITEM		item;
} NAVINSERTSTRUCT;

typedef struct _NAVITEMDRAW
{
	HDC			hdc;			// Handle to the DC.
	COLORREF	clrText;		// Text Fore color
	COLORREF	clrTextBk;		// Text Back color.
	HFONT		hFont;			// Handle to the selected font.
	RECT		*prc;			// pointer to the item rect.
	UINT		itemState;		// NIIS_XXX
	UINT		drawStage;		// NIDS_XXX
	INT			iLevel;			// Zero-based level of the item being drawn. 
} NAVITEMDRAW;

// ML_IPC_NAVCTRL_ENUMITEMS callback
typedef BOOL (CALLBACK *NAVENUMPROC)(HNAVITEM /*hItem*/, LPARAM /*lParam*/); // Return FALSE to stop enumeration

// Use it to enumerate navigation items with ML_IPC_NAVCTRL_ENUMITEMS
typedef struct _NAVCTRLENUMPARAMS
{
	NAVENUMPROC		enumProc;		// Enum proc..
	HNAVITEM		hItemStart;		// Item to start enumeration from. If hItemStar == NULL enumeration will start from the root.
	LPARAM			lParam;			// User value
} NAVCTRLENUMPARAMS;

/// Find by name prarams (ML_IPC_NAVCTRL_FINDBYNAME)
typedef struct _NAVCTRLFINDPARAMS
{
	LCID		Locale;				// Locale to use. Note: when searching using invariant name locale forced to invariant and case ignored.
	UINT		compFlags;			// Name compare  flags (NCCF_XXX).
	LPWSTR		pszName;			// Name to look for.
	INT			cchLength;			// Name length (can be -1).
	BOOL		fFullNameSearch;	// Perform full name search.
	BOOL		fAncestorOk;		// When performing full name search if fAncestorOk is set and item with this name not exist - will fall back to closest ancestor
} NAVCTRLFINDPARAMS;

// Perform hit test
typedef struct _NAVHITTEST
{
	POINT		pt;			// [in]	 Point in control coordinates.
	UINT		flags;		// [out] Test result flags
	HNAVITEM	hItem;		// [out] Navigation Item at this point if any.
} NAVHITTEST;

// used to send item epxnd command ML_IPC_NAVITEM_EXPAND
typedef struct _NAVITEMEXPAND
{
	HNAVITEM	hItem;			// handle to the item 
	UINT		fCmdExpand;		// command to execute (NAITEM_TOGGLE/ NAITEM_EXOPAND/NAITEM_COLLAPSE).
} NAVITEMEXPAND;

// used to get fullitem name via ML_IPC_NAVITEM_GETFULLNAME
typedef struct _NAVITEMFULLNAME
{
	HNAVITEM	hItem;			// Handle to the item.
	LPWSTR		pszText;		// Buffer that will recive full name.
	INT			cchTextMax;		// Maximum amount of characters that can be wirtten to buffer.
} NAVITEMFULLNAME;

// ML_IPC_NAVITEM_GETRECT
typedef struct _NAVITEMGETRECT
{
	HNAVITEM	hItem;		// [in]  Handle to the item.
	RECT		rc;			// [out] Bounding rectangle.
	BOOL		fItem;		// [in]  If this parameter is TRUE, the bounding rectangle includes only the text of the item. Otherwise, it includes the entire line that the item occupies in the navigation control. 
} NAVITEMGETRECT;

typedef struct _NAVITEMINAVLIDATE
{
	HNAVITEM	hItem;		// [in] Handle to the item.
	RECT		*prc;		// [in] If this parameter is set specifies portion of item to invalidate in control coordinates. if it is NULL whole item will be invalidated
	BOOL		fErase;		// [in] If TRUE background will be erased.
} NAVITEMINAVLIDATE;

//ML_IPC_NAVITEM_MOVE
typedef struct _NAVITEMMOVE
{
	HNAVITEM	hItem;		// [in] Handle to the item that need to be moved.
	HNAVITEM	hItemDest;	// [in] Handle to the item where to move.
	BOOL		fAfter;		// [in] If set to TRUE hItem will be moved after hItemDest.
} NAVITEMMOVE;

typedef struct _NAVITEMORDER
{
	HNAVITEM	hItem;		// Handle to the item that need to be moved.
	WORD		order;		// Minimal order value 1. 
	UINT		flags;		// Flags one of the NOF_XXX
} NAVITEMORDER;

// Nav control styles
#define NCS_NORMAL				0x0000
#define NCS_FULLROWSELECT		0x0001
#define NCS_SHOWICONS			0x0002

// NAVITEM mask flags. if mask set filed considered to be valid 
#define NIMF_ITEMID				0x0001
#define NIMF_TEXT				0x0002
#define NIMF_TEXTINVARIANT		0x0004
#define NIMF_IMAGE				0x0008
#define NIMF_IMAGESEL			0x0010
#define NIMF_STATE				0x0020
#define NIMF_STYLE				0x0040
#define NIMF_FONT				0x0080
#define NIMF_PARAM				0x0100

// NAVITEM states.
#define NIS_NORMAL				0x0000
#define NIS_SELECTED			0x0001
#define NIS_EXPANDED			0x0002
#define NIS_DROPHILITED			0x0004
#define NIS_FOCUSED				0x0008	// used with draw item

// NAVITEM styles
#define NIS_HASCHILDREN			0x0001	// item has children
#define NIS_ALLOWCHILDMOVE		0x0002	// allow children to be moved (re-ordered)
#define NIS_ALLOWEDIT			0x0004	// allow title edit
#define NIS_ITALIC				0x0100	// when displaying item text draw it with italic style 
#define NIS_BOLD				0x0200	// when displaying item text draw it with bold style
#define NIS_UNDERLINE			0x0400	// when displaying item text draw it with underline style
#define NIS_CUSTOMDRAW			0x0010	// use ML_MSG_NAVIGATION_ONCUSTOMDRAW
#define NIS_WANTSETCURSOR		0x0020	// item want to recive set cursor notification
#define NIS_WANTHITTEST			0x0040	// item want to monitor/modify hittest results
#define NIS_DEFAULTIMAGE		0x0080  // navigation control will ignore iImage and iSelectedImage

// NAVITEMINSERT hInsertAfter flags
#define NCI_FIRST				((HNAVITEM)(ULONG_PTR)-0x0FFFF)	// insert first item 
#define NCI_LAST				((HNAVITEM)(ULONG_PTR)-0x00000) // insert as last item

// BeginUpdate Lock flags
#define NUF_LOCK_NONE			0x00 // Do not lock...
#define NUF_LOCK_SELECTED		0x01 // Lock selected item position.
#define NUF_LOCK_TOP			0x02 // Lock top item position

// Name compare flags
#define NICF_DISPLAY			0x0001 // Compare display name (if specified in combination with othe flags will be used last).
#define NICF_INVARIANT			0x0002 // Compare invariant name (if specified in combination with DISPLAY will be used first).
#define NICF_IGNORECASE			0x0004 // Ignore case (always used when comparing invariant names).

// Navigation control hit test result flags.
#define NAVHT_NOWHERE			0x0001	// 
#define NAVHT_ONITEM			0x0002	//
#define NAVHT_ONITEMBUTTON		0x0004	// only if item currently has children
#define NAVHT_ONITEMINDENT		0x0010	//
#define NAVHT_ONITEMRIGHT		0x0020	//
#define NAVHT_ABOVE				0x0100	//
#define NAVHT_BELOW				0x0200	//
#define NAVHT_TORIGHT			0x0400	//
#define NAVHT_TOLEFT			0x0800	//

// item expand command flags
#define NAVITEM_TOGGLE			0x0000	
#define NAVITEM_EXPAND			0x0001
#define NAVITEM_COLLAPSE		0x0002

// item drawing stages
#define NIDS_PREPAINT			0x0001
#define NIDS_POSTPAINT			0x0002

// item custom draw return flags
#define NICDRF_NOTMINE			0x0000	// It is not your item.
#define NICDRF_DODEFAULT		0x0001	// Perform default drawing.
#define NICDRF_SKIPDEFAULT		0x0002	// Do not perform default drawing.
#define NICDRF_NOTIFYPOSTPAINT	0x0004	// You want to be called after paint.
#define NICDRF_NEWFONT			0x0008	// Font changed.


// Messages
#define ML_IPC_NAVIGATION_FIRST				0x1280L

#define ML_IPC_NAVCTRL_BEGINUPDATE			(ML_IPC_NAVIGATION_FIRST + 1)	// Use it to lock control updates while you managing items. param = NUF_LOCK_XXX. Do not forget to call ML_IPC_NAVCTRL_ENDUPDATE when done. Returns lock counter or -1 if error.
#define ML_IPC_NAVCTRL_DELETEITEM			(ML_IPC_NAVIGATION_FIRST + 2)	// Delete Item. param = (WPARAM)(HNAVITEM)hItem. Return TRUE if success.
#define ML_IPC_NAVCTRL_ENDEDITTITLE			(ML_IPC_NAVIGATION_FIRST + 13)	// Ends Title edit,	 param = (WPARAM)(BOOL)fCancel, if fCancel == TRUE item editing will be terminated and changes lost otherwise new title will be saved.
#define ML_IPC_NAVCTRL_ENDUPDATE			(ML_IPC_NAVIGATION_FIRST + 3)	// Call this when you finished updating items. Returns lock counter or -1 if error. if loock reached 0 - window will be updated.
#define ML_IPC_NAVCTRL_ENUMITEMS			(ML_IPC_NAVIGATION_FIRST + 4)	// Enumerates items in navigation control. param= (WPARAM)(NAVCTRLENUMPARAMS*)pnavEnumParams. Returns TRUE if all items was enumerated successfully.
#define ML_IPC_NAVCTRL_FINDITEMBYID			(ML_IPC_NAVIGATION_FIRST + 5)	// Search for an item in navigation control by it's id. param = (WPARAM)(INT)id. Returns HNAVITEM on success or NULL.
#define ML_IPC_NAVCTRL_FINDITEMBYNAME		(ML_IPC_NAVIGATION_FIRST + 6)	// Search for an item in navigation control by it's name. param = (WPARAM)(NAVCTRLFINDPARAMS)pnavFindParams. Returns HNAVITEM on success or NULL.
#define ML_IPC_NAVCTRL_GETIMAGELIST			(ML_IPC_NAVIGATION_FIRST + 7)	// Returnrs handle to MLImageList associated with navigation control. param = (WPARAM)0. Return HMLIMGLST if success or NULL.
#define ML_IPC_NAVCTRL_GETFIRST				(ML_IPC_NAVIGATION_FIRST + 8)	// Returns handle to first item in the navigation control. param = (WPARAM)0. Return HNAVITEM or NULL.
#define ML_IPC_NAVCTRL_GETINDENT			(ML_IPC_NAVIGATION_FIRST + 14)	// Retrieves the amount, in pixels, that child items are indented relative to their parent items. param = (WPARAM)0. 
#define ML_IPC_NAVCTRL_GETSELECTION			(ML_IPC_NAVIGATION_FIRST + 9)	// Returns handle to selected item in the navigation control. param = (WPARAM)0. Return HNAVITEM or NULL.
#define ML_IPC_NAVCTRL_GETHWND				(ML_IPC_NAVIGATION_FIRST + 10)	// Returns host window HWND. param = (WPARAM)0. Return HWND.
#define ML_IPC_NAVCTRL_HITTEST				(ML_IPC_NAVIGATION_FIRST + 11)	// Performs hit-test. param = (WPARAM)(NAVHITTEST*)pnavHitTest. Return HNAVITEM if any item under this point or NULL.
#define ML_IPC_NAVCTRL_INSERTITEM			(ML_IPC_NAVIGATION_FIRST + 12)	// Insert Item. param = (WPARAM)(NAVINSERTSTRUCT*)pnavInsert. Returns handle to created item if you not speicified item id - you wil be assigned auto generated one pnavInsert->id
#define ML_IPC_NAVCTRL_GETSTYLE				(ML_IPC_NAVIGATION_FIRST + 15)	// Returns NCS_XXX. param = (WPARAM)0.

#define ML_IPC_NAVITEM_EDITTITLE			(ML_IPC_NAVIGATION_FIRST + 58)	// Begins Title edit. param = (WPARAM)(HNAVITEM)hItem. Returns TRUE if item edit begins	
#define ML_IPC_NAVITEM_ENSUREVISIBLE		(ML_IPC_NAVIGATION_FIRST + 40)	// Enusres that item visible. param =(WPARAM)(HNAVITEM)hItem. Return TRUE if ok.
#define ML_IPC_NAVITEM_EXPAND				(ML_IPC_NAVIGATION_FIRST + 41)  // Command to change item expand state. param = (WPARAM)(NAVITEMEXPAND*)hnavItemExpand. Returns TRUE if ok.
#define ML_IPC_NAVITEM_GETCHILD				(ML_IPC_NAVIGATION_FIRST + 42)	// Returns handle to the child item of current one. param =(WPARAM)(HNAVITEM)hItem. Return HNAVITEM or NULL.
#define ML_IPC_NAVITEM_GETCHILDRENCOUNT		(ML_IPC_NAVIGATION_FIRST + 43)	// Returns children count. param = (WPARAM)(HNAVITEM)hItem.
#define ML_IPC_NAVITEM_GETFULLNAME			(ML_IPC_NAVIGATION_FIRST + 44)	// Returns item full name. param = (WPARAM)(NAVITEMFULLNAME*)pnavFullName. Returns INT number of characters copied or 0. Full name includes names of all item parents separated by '/' if character '/' was used in name it is doubled
#define ML_IPC_NAVITEM_GETINFO				(ML_IPC_NAVIGATION_FIRST + 45)	// Retrives item info. param = (WPARAM)(NAVITEM*)pnavItem. Returns TRUE if success. You must set hItem.
#define ML_IPC_NAVITEM_GETNEXT				(ML_IPC_NAVIGATION_FIRST + 46)	// Returns handle to the next item of current one. param =(WPARAM)(HNAVITEM)hItem. Return HNAVITEM or NULL.
#define ML_IPC_NAVITEM_GETORDER				(ML_IPC_NAVIGATION_FIRST + 47)	// Returns item sort order. param = (WPARAM)(HNAVITEM)hItem. Return item sort order or -1 if error.
#define ML_IPC_NAVITEM_GETPARENT			(ML_IPC_NAVIGATION_FIRST + 48)	// Returns handle to the parent item of current one. param =(WPARAM)(HNAVITEM)hItem. Return HNAVITEM or NULL.
#define ML_IPC_NAVITEM_GETPREVIOUS			(ML_IPC_NAVIGATION_FIRST + 49)	// Returns handle to the previous item of current one. param =(WPARAM)(HNAVITEM)hItem. Return HNAVITEM or NULL.
#define ML_IPC_NAVITEM_GETRECT				(ML_IPC_NAVIGATION_FIRST + 50)	// Retrive item rect. param =(WPARAM)(NAVITEMGETRECT*)hnavRect. If the item is visible and the bounding rectangle was successfully retrieved, the return value is TRUE. 
#define ML_IPC_NAVITEM_GETSTATE				(ML_IPC_NAVIGATION_FIRST + 51)	// Returns item state flags. param = (WPARAM)(HNAVITEM)hItem. Return item state flags.
#define ML_IPC_NAVITEM_HASCHILDRENREAL		(ML_IPC_NAVIGATION_FIRST + 52)	// Returns TRUE if item has at least one child right now.  param =(WPARAM)(HNAVITEM)hItem. Return BOOL.
#define ML_IPC_NAVITEM_INVALIDATE			(ML_IPC_NAVIGATION_FIRST + 53)	// Invalidate Item. param = (WPARAM)(NAVITEMINAVLIDATE*)hnavItemInvalidate. Return TRUE if success.
#define ML_IPC_NAVITEM_MOVE					(ML_IPC_NAVIGATION_FIRST + 54)	// Move Item inside parent group. param =(WPARAM)(NAVITEMMOVE*)pnavMove. Return TRUE on success.
#define ML_IPC_NAVITEM_SELECT				(ML_IPC_NAVIGATION_FIRST + 55)	// Select current item. param =(WPARAM)(HNAVITEM)hItem. Return TRUE if success.
#define ML_IPC_NAVITEM_SETINFO				(ML_IPC_NAVIGATION_FIRST + 56)	// Modifies item info. param = (WPARAM)(NAVITEM*)pnavItem. Returns TRUE if success. You must set hItem.
#define ML_IPC_NAVITEM_SETORDER				(ML_IPC_NAVIGATION_FIRST + 57)	// Sets item order and modifies all items oreder after it if required. if oder == 0xFFFF order will be set to max + 1 for this group. param = (WPARAM)(NAVITEMORDER*)pnavOrder. Returns new item order or 0xFFFF if error


// Macros

// Use this in ML_IPC_NAVCTRL_INSERTITEM to create hInsertAfter
#define MAKE_NAVITEMSORTORDER(o) ((HNAVITEM)((ULONG_PTR)((WORD)(o))))
#define IS_NAVITEMSORTORDER(_i) ((((ULONG_PTR)(_i)) >> 16) == 0)

#define MLNavCtrl_BeginUpdate(/*HWND*/ hwndML, /*UINT*/ fLockFlags) \
							  (INT)SENDMLIPC((hwndML), ML_IPC_NAVCTRL_BEGINUPDATE, (WPARAM)(fLockFlags))

#define MLNavCtrl_DeleteItem(/*HWND*/ hwndML, /*HNAVITEM*/hItem) \
							 (INT)SENDMLIPC((hwndML), ML_IPC_NAVCTRL_DELETEITEM, (WPARAM)(hItem))

#define MLNavCtrl_EndEditTitle(/*HWND*/ hwndML, /*BOOL*/fCancel) \
							   (BOOL)SENDMLIPC((hwndML), ML_IPC_NAVCTRL_ENDEDITTITLE, (WPARAM)(fCancel))

#define MLNavCtrl_EndUpdate(/*HWND*/ hwndML) \
							(INT)SENDMLIPC((hwndML), ML_IPC_NAVCTRL_ENDUPDATE, (WPARAM)0)

#define MLNavCtrl_EnumItems(/*HWND*/ hwndML, /*NAVCTRLENUMPARAMS**/pnavEnumParams) \
							(BOOL)SENDMLIPC((hwndML), ML_IPC_NAVCTRL_ENUMITEMS, (WPARAM)(pnavEnumParams))

#define MLNavCtrl_FindItemById(/*HWND*/ hwndML, /*INT*/itemId) \
							   (HNAVITEM)SENDMLIPC((hwndML), ML_IPC_NAVCTRL_FINDITEMBYID, (WPARAM)(itemId))

#define MLNavCtrl_FindItemByName(/*HWND*/ hwndML, /*NAVCTRLFINDPARAMS**/pnavFindParams) \
								 (HNAVITEM)SENDMLIPC((hwndML), ML_IPC_NAVCTRL_FINDITEMBYNAME, (WPARAM)(pnavFindParams))

#define MLNavCtrl_GetImageList(/*HWND*/ hwndML) \
							   (HNAVITEM)SENDMLIPC((hwndML), ML_IPC_NAVCTRL_GETIMAGELIST, (WPARAM)0)

#define MLNavCtrl_GetFirst(/*HWND*/ hwndML) \
						   (HNAVITEM)SENDMLIPC((hwndML), ML_IPC_NAVCTRL_GETFIRST, (WPARAM)0)

#define MLNavCtrl_GetIndent(/*HWND*/ hwndML) \
							(INT)SENDMLIPC((hwndML), ML_IPC_NAVCTRL_GETINDENT, (WPARAM)0)

#define MLNavCtrl_GetSelection(/*HWND*/ hwndML) \
							   (HNAVITEM)SENDMLIPC((hwndML), ML_IPC_NAVCTRL_GETSELECTION, (WPARAM)0)

#define MLNavCtrl_GetHWND(/*HWND*/ hwndML) \
						  (HWND)SENDMLIPC((hwndML), ML_IPC_NAVCTRL_GETHWND, (WPARAM)0)

#define MLNavCtrl_HitTest(/*HWND*/ hwndML, /*NAVHITTEST**/pnavHitTest) \
						  (HNAVITEM)SENDMLIPC((hwndML), ML_IPC_NAVCTRL_HITTEST, (WPARAM)(pnavHitTest))

#define MLNavCtrl_InsertItem(/*HWND*/ hwndML, /*NAVINSERTSTRUCT**/pnavInsert) \
							 (HNAVITEM)SENDMLIPC((hwndML), ML_IPC_NAVCTRL_INSERTITEM, (WPARAM)(pnavInsert))

#define MLNavCtrl_GetStyle(/*HWND*/ hwndML) \
						   (DWORD)SENDMLIPC((hwndML), ML_IPC_NAVCTRL_GETSTYLE, (WPARAM)0)

#define MLNavItem_EditTitle(/*HWND*/ hwndML, /*HNAVITEM*/hItem) \
							(BOOL)SENDMLIPC((hwndML), ML_IPC_NAVITEM_EDITTITLE, (WPARAM)(hItem))

#define MLNavItem_EnsureVisible(/*HWND*/ hwndML, /*HNAVITEM*/hItem) \
								(BOOL)SENDMLIPC((hwndML), ML_IPC_NAVITEM_ENSUREVISIBLE, (WPARAM)(hItem))

#define MLNavItem_Expand(/*HWND*/ hwndML, /*NAVITEMEXPAND**/pnavItemExpand) \
						 (BOOL)SENDMLIPC((hwndML), ML_IPC_NAVITEM_EXPAND, (WPARAM)(pnavItemExpand))

#define MLNavItem_GetChild(/*HWND*/ hwndML, /*HNAVITEM*/hItem) \
						   (HNAVITEM)SENDMLIPC((hwndML), ML_IPC_NAVITEM_GETCHILD, (WPARAM)(hItem))

#define MLNavItem_GetChildrenCount(/*HWND*/ hwndML, /*HNAVITEM*/hItem) \
								   (INT)SENDMLIPC((hwndML), ML_IPC_NAVITEM_GETCHILDRENCOUNT, (WPARAM)(hItem))

#define MLNavItem_GetFullName(/*HWND*/ hwndML, /*NAVITEMFULLNAME**/pnavFullName) \
							  (INT)SENDMLIPC((hwndML), ML_IPC_NAVITEM_GETFULLNAME, (WPARAM)(pnavFullName))

#define MLNavItem_GetInfo(/*HWND*/ hwndML, /*NAVITEM**/pnavItem) \
						  (INT)SENDMLIPC((hwndML), ML_IPC_NAVITEM_GETINFO, (WPARAM)(pnavItem))

#define MLNavItem_GetNext(/*HWND*/ hwndML, /*HNAVITEM*/hItem) \
						  (HNAVITEM)SENDMLIPC((hwndML), ML_IPC_NAVITEM_GETNEXT, (WPARAM)(hItem))

#define MLNavItem_GetOrder(/*HWND*/ hwndML, /*HNAVITEM*/hItem) \
						   (WORD)SENDMLIPC((hwndML), ML_IPC_NAVITEM_GETORDER, (WPARAM)(hItem))

#define MLNavItem_GetParent(/*HWND*/ hwndML, /*HNAVITEM*/hItem) \
							(HNAVITEM)SENDMLIPC((hwndML), ML_IPC_NAVITEM_GETPARENT, (WPARAM)(hItem))

#define MLNavItem_GetPrevious(/*HWND*/ hwndML, /*HNAVITEM*/hItem) \
							  (HNAVITEM)SENDMLIPC((hwndML), ML_IPC_NAVITEM_GETPREVIOUS, (WPARAM)(hItem))

#define MLNavItem_GetRect(/*HWND*/ hwndML, /*NAVITEMGETRECT**/pnavRect) \
						  (BOOL)SENDMLIPC((hwndML), ML_IPC_NAVITEM_GETRECT, (WPARAM)(pnavRect))

#define MLNavItem_GetState(/*HWND*/ hwndML, /*HNAVITEM*/hItem) \
						   (BOOL)SENDMLIPC((hwndML), ML_IPC_NAVITEM_GETSTATE, (WPARAM)(hItem))

#define MLNavItem_HasChildrenReal(/*HWND*/ hwndML, /*HNAVITEM*/hItem) \
								  (BOOL)SENDMLIPC((hwndML), ML_IPC_NAVITEM_HASCHILDRENREAL, (WPARAM)(hItem))

#define MLNavItem_Invalidate(/*HWND*/ hwndML, /*NAVITEMINAVLIDATE**/pnavItemInvalidate) \
							 (BOOL)SENDMLIPC((hwndML), ML_IPC_NAVITEM_INVALIDATE, (WPARAM)(pnavItemInvalidate))

#define MLNavItem_Move(/*HWND*/ hwndML, /*NAVITEMMOVE**/pnavMove) \
					   (BOOL)SENDMLIPC((hwndML), ML_IPC_NAVITEM_MOVE, (WPARAM)(pnavMove))

#define MLNavItem_Select(/*HWND*/ hwndML, /*HNAVITEM*/hItem) \
						 (BOOL)SENDMLIPC((hwndML), ML_IPC_NAVITEM_SELECT, (WPARAM)(hItem))

#define MLNavItem_SetInfo(/*HWND*/ hwndML, /*NAVITEM**/pnavItem) \
						  (BOOL)SENDMLIPC((hwndML), ML_IPC_NAVITEM_SETINFO, (WPARAM)(pnavItem))

#define MLNavItem_SetOrder(/*HWND*/ hwndML, /*NAVITEMORDER**/pNavOrder) \
						   (WORD)SENDMLIPC((hwndML), ML_IPC_NAVITEM_SETORDER, (WPARAM)(pNavOrder))


//////////////////////////////////////////////////////////////////////////////
// MLRating
// Rating control
//
//

typedef struct _RATINGDRAWPARAMS
{
	INT			cbSize;			// The size of this structure, in bytes.  You must specify size !!!
	HDC			hdcDst;			// A handle to the destination device context. 
	RECT		rc;				// Rectangle that specifies where the image is drawn. 
	INT			value;			// Rating value.
	INT			maxValue;		// Maximum rating value.
	INT			trackingValue;	// Current tracking value.
	UINT		fStyle;			// combination of RDS_XXX
	HMLIMGLST	hMLIL;			// Media Libary ImageList. Set to NULL if you want to use default media library list.
	INT			index;			// index of image inside Media Libary ImageList to use. Ignored if used with default media library list.
} RATINGDRAWPARAMS;

typedef struct _RATINGHITTESTPARAMS
{
	INT			cbSize;			// The size of this structure, in bytes.  You must specify size !!!
	POINT		pt;				// Client coordinates of the point to test. 
	RECT		rc;				// Rectangle that specifies where the image is drawn. 
	INT			maxValue;		// Maximum rating value.
	UINT		fStyle;			// combination of RDS_XXX
	HMLIMGLST	hMLIL;			// Media Libary ImageList. Set to NULL if you want to use default media library list.
	INT			value;			// Rating value at this point.
	UINT		hitFlags;		// Variable that receives information about the results of a hit test.
} RATINGHITTESTPARAMS;

/// Rating_Draw Styles
#define RDS_SHOWEMPTY	0x00000001		// Draw elements that not set.
#define RDS_OPAQUE		0x00000002		// Fill rest of the rectangle  with rgbBk.
#define RDS_HOT			0x00000004		// Draw elements as "hot".
#define RDS_LEFT		0x00000000		// Aligns elements to the left.
#define RDS_TOP			0x00000000		// Justifies elements to the top of the rectangle.
#define RDS_RIGHT		0x00000010		// Aligns elements to the right.
#define RDS_BOTTOM		0x00000020		// Justifies elements to the bottom of the rectangle.
#define RDS_HCENTER		0x00000040		// Centers elements horizontally in the rectangle.
#define RDS_VCENTER		0x00000080		// Centers elements horizontally in the rectangle.
#define RDS_NORMAL		(RDS_SHOWEMPTY | RDS_OPAQUE | RDS_LEFT | RDS_TOP)


// Rating_HitTest hitFlags

#define RHT_NOWHERE			0x0001
#define RHT_ONVALUE			0x0002
#define RHT_ONVALUEABOVE	0x0004
#define RHT_ONVALUEBELOW	0x0008
#define RHT_TOLEFT			0x0100
#define RHT_TORIGHT			0x0200


// Messages
#define ML_IPC_RATING_FIRST			0x1380L

#define ML_IPC_RATING_DRAW			(ML_IPC_RATING_FIRST + 1)	// Draws rating. param = (WPARAM)(RATINGDRAWPARAMS*)prdp. return TRUE on success.
#define ML_IPC_RATING_HITTEST		(ML_IPC_RATING_FIRST + 2)	// Performs hit test. param = (WPARAM)(RATINGDRAWPARAMS*)prdp. returns value.
#define ML_IPC_RATING_CALCRECT		(ML_IPC_RATING_FIRST + 3)	// calculates minimal rect required to show rating. (param) = (WPARAM)(RECT*)prc. Set prc->left to HMLIMGLST (or NULL to use default) and prc->top to maxValue. Returns TRUE if ok.

#define MLRating_Draw(/*HWND*/ hwndML, /*RATINGDRAWPARAMS**/prdp) \
					  (BOOL)SENDMLIPC((hwndML), ML_IPC_RATING_DRAW, (WPARAM)(RATINGDRAWPARAMS*)(prdp))

#define MLRating_HitTest(/*HWND*/ hwndML, /*RATINGHITTESTPARAMS**/prhtp) \
						 (INT)SENDMLIPC((hwndML), ML_IPC_RATING_HITTEST, (WPARAM)(RATINGHITTESTPARAMS*)(prhtp))

  
#define MLRating_CalcRect(/*HWND*/ hwndML, /*HMLIMGLST*/ hmlil, /*INT*/ maxVal, /*RECT**/prc) \
						  (BOOL)SENDMLIPC((hwndML), ML_IPC_RATING_CALCRECT, ((prc) ? ((((RECT*)(prc))->left = hmlil), \
						  (((RECT*)(prc))->top = maxVal), (WPARAM)(prc)) : (WPARAM)(RECT*)NULL))


//////////////////////////////////////////////////////////////////////////////
// MLRatingColumn
// Helpers to display rating in the listview
//
//

typedef struct _RATINGCOLUMNPAINT 	
{
	HWND		hwndList;	// hwnd of the listview
	HDC			hdc;		// hdc
	UINT		iItem;		// item index
	UINT		iSubItem;	// subitem index
	INT			value;		// database rating value
	RECT		*prcItem;	// whole item rect (plvcd->nmcd.rc)
	RECT		*prcView;	// client area size (you can get it at CDDS_PREPAINT in plvcd->nmcd.rc)
	COLORREF	rgbBk;		// color to use as background (plvcd->clrTextBk)
	COLORREF	rgbFg;		// color to use as foreground (plvcd->clrText)
	UINT		fStyle;		// style to use RCS_XXX
} RATINGCOLUMNPAINT;

typedef struct _RATINGCOLUMN
{
	HWND	hwndList;
	UINT	iItem;
	UINT	iSubItem;
	INT		value;
	POINT	ptAction;		// 
	BOOL	bRedrawNow;		// You want list to be redrawn immediatly
	BOOL	bCanceled;		// Used with EndDrag - i
	UINT	fStyle;			// RCS_XXX
} RATINGCOLUMN;

typedef struct _RATINGBACKTEXT
{
	LPWSTR	pszText;
	INT		cchTextMax;
	INT		nColumnWidth; // used if style is RCS_ALLIGN_CENTER or RCS_ALLIGN_RIGHT
	UINT	fStyle;	
} RATINGBACKTEXT;

typedef struct _RATINGWIDTH
{
	INT	width;		// desired width
	UINT fStyle;	// RCS_XXX
} RATINGWIDTH;

// styles
#define RCS_DEFAULT					0xFFFFFFFF	// set this if you want to use gen_ml provided style
// layout
#define RCS_ALLIGN_LEFT				0x00000000  // allign column left
#define RCS_ALLIGN_CENTER			0x00000001	// allign column center
#define RCS_ALLIGN_RIGHT			0x00000002  // allign column right
// showepmty
#define RCS_SHOWEMPTY_NEVER			0x00000000
#define RCS_SHOWEMPTY_NORMAL		0x00000010
#define RCS_SHOWEMPTY_HOT			0x00000020
#define RCS_SHOWEMPTY_ANIMATION		0x00000040
#define RCS_SHOWEMPTY_ALWAYS		0x00000070
// traking (when)
#define RCS_TRACK_NEVER				0x00000000
#define RCS_TRACK_WNDFOCUSED		0x00000100
#define RCS_TRACK_ANCESTORACITVE	0x00000200
#define RCS_TRACK_PROCESSACTIVE		0x00000400
#define RCS_TRACK_ALWAYS			0x00000800
// traking (what)
#define RCS_TRACKITEM_ALL			0x00000000
#define RCS_TRACKITEM_SELECTED		0x00100000
#define RCS_TRACKITEM_FOCUSED		0x00200000

#define RCS_BLOCKCLICK				0x01000000
#define RCS_BLOCKUNRATECLICK		0x02000000
#define RCS_BLOCKDRAG				0x04000000

#define RCS_SIZE_ALLOWDECREASE		0x10000000
#define RCS_SIZE_ALLOWINCREASE		0x20000000


#define ML_IPC_RATINGCOLUMN_FIRST			0x1420L

#define ML_IPC_RATINGCOLUMN_PAINT			(ML_IPC_RATINGCOLUMN_FIRST + 1)		// Paints rating column (call it in  (CDDS_SUBITEM | CDDS_ITEMPREPAINT) handler).
																				// param = (WPARAM)(RATINGCOLUMNPAINT*)prcp. if Return TRUE - you need to return SDRF_SKIPDEFAULT
#define ML_IPC_RATINGCOLUMN_CLICK			(ML_IPC_RATINGCOLUMN_FIRST + 2)		// param = (WPARAM)(RATINGCOLUMN*)prc. You need to set: hwndList, ptAction, bRedraw. iItem - will be set to iItem where click happened. Returns TRUE if click is on the column. 
#define ML_IPC_RATINGCOLUMN_TRACK			(ML_IPC_RATINGCOLUMN_FIRST + 3)		// param = (WPARAM)(RATINGCOLUMN*)prc. You need to set: hwndList, ptAction, iItem, iSubItem, vale, bRedraw. iItem - will be set to iItem where click happened. No return value.
#define ML_IPC_RATINGCOLUMN_CANCELTRACKING	(ML_IPC_RATINGCOLUMN_FIRST + 4)		// param = (WPARAM)(BOOL)bRedraw. No return value.
#define ML_IPC_RATINGCOLUMN_BEGINDRAG		(ML_IPC_RATINGCOLUMN_FIRST + 5)		// param = (WPARAM)(RATINGCOLUMN*)prc. Set: hwndList, iItem, iSubItem, value. No return value. 
#define ML_IPC_RATINGCOLUMN_DRAG				(ML_IPC_RATINGCOLUMN_FIRST + 6)	// param = (WPARAM)(POINT*)ppt. Returns TRUE if drag is porcessed.
#define ML_IPC_RATINGCOLUMN_ENDDRAG			(ML_IPC_RATINGCOLUMN_FIRST + 7)		// param = (WPARAM)(RATINGCOLUMN*)prc. Set: ptAction, bCanceled, bRedrawNow. Returns TRUE if value modified. Sets: iItem, iSubItemVal.
#define ML_IPC_RATINGCOLUMN_ANIMATE			(ML_IPC_RATINGCOLUMN_FIRST + 8)		// param = (WPARAM)(RATINGCOLUMN*)prc. Set: hwndList, iItem, iSubItem. iSubItem - contains duration in ms. No return.
#define ML_IPC_RATINGCOLUMN_GETMINWIDTH		(ML_IPC_RATINGCOLUMN_FIRST + 9)		// param = (WPARAM)0. Returns minimum width required to display rating column.
#define ML_IPC_RATINGCOLUMN_FILLBACKSTRING	(ML_IPC_RATINGCOLUMN_FIRST + 10)	// Generates string thats when displayed equal to the right border of maximum rating value. If rating column has 0 index you can use this string as column text to prevent mouse group selection over rating. If width is less than allowed pszText[0] = 0x00
																				// param = (WPARAM)(RATINGBACKTEXT*)prct. Returns TRUE if ok
#define ML_IPC_RATINGCOLUMN_GETWIDTH		(ML_IPC_RATINGCOLUMN_FIRST + 11)	// Set width according to the policies. param = (WPARAM)(RATINGWIDTH*)prw. Return TRUE if ok. prw->width  will contain allowed width


#define MLRatingColumn_Paint(/*HWND*/ hwndML, /*RATINGCOLUMNPAINT**/prcp) \
						(BOOL)SENDMLIPC((hwndML), ML_IPC_RATINGCOLUMN_PAINT, (WPARAM)(RATINGCOLUMNPAINT*)(prcp))

#define MLRatingColumn_Click(/*HWND*/ hwndML, /*RATINGCOLUMN**/pRating) \
						(BOOL)SENDMLIPC((hwndML), ML_IPC_RATINGCOLUMN_CLICK, (WPARAM)(RATINGCOLUMN*)(pRating))

#define MLRatingColumn_Track(/*HWND*/ hwndML, /*RATINGCOLUMN**/pRating) \
						(BOOL)SENDMLIPC((hwndML), ML_IPC_RATINGCOLUMN_TRACK, (WPARAM)(RATINGCOLUMN*)(pRating))

#define MLRatingColumn_CancelTracking(/*HWND*/ hwndML, /*BOOL*/bRedrawNow) \
						(BOOL)SENDMLIPC((hwndML), ML_IPC_RATINGCOLUMN_CANCELTRACKING, (WPARAM)(BOOL)(bRedrawNow))

#define MLRatingColumn_BeginDrag(/*HWND*/ hwndML, /*RATINGCOLUMN**/pRating) \
						(BOOL)SENDMLIPC((hwndML), ML_IPC_RATINGCOLUMN_BEGINDRAG, (WPARAM)(RATINGCOLUMN*)(pRating))

#define MLRatingColumn_Drag(/*HWND*/ hwndML, /*POINT**/ppt) \
						(BOOL)SENDMLIPC((hwndML), ML_IPC_RATINGCOLUMN_DRAG, (WPARAM)(POINT*)(ppt))

#define MLRatingColumn_EndDrag(/*HWND*/ hwndML, /*RATINGCOLUMN**/pRating) \
						(BOOL)SENDMLIPC((hwndML), ML_IPC_RATINGCOLUMN_ENDDRAG, (WPARAM)(RATINGCOLUMN*)(pRating))

#define MLRatingColumn_Animate(/*HWND*/ hwndML, /*RATINGCOLUMN**/pRating) \
						(BOOL)SENDMLIPC((hwndML), ML_IPC_RATINGCOLUMN_ANIMATE, (WPARAM)(RATINGCOLUMN*)(pRating))

#define MLRatingColumn_GetMinWidth(/*HWND*/ hwndML) \
						(INT)SENDMLIPC((hwndML), ML_IPC_RATINGCOLUMN_GETMINWIDTH, (WPARAM)0)

#define MLRatingColumn_FillBackString(/*HWND*/ hwndML, /*RATINGBACKTEXT**/prbt) \
						(BOOL)SENDMLIPC((hwndML), ML_IPC_RATINGCOLUMN_FILLBACKSTRING, (WPARAM)(RATINGBACKTEXT*)(prbt))

#define MLRatingColumn_GetWidth(/*HWND*/ hwndML, /*RATINGWIDTH**/prw) \
						(BOOL)SENDMLIPC((hwndML), ML_IPC_RATINGCOLUMN_GETWIDTH, (WPARAM)(RATINGWIDTH*)(prw))


//////////////////////////////////////////////////////////////////////////////
// MLCloud
// Cloud control
//
//

typedef struct _CLOUDDRAWPARAMS
{
	INT			cbSize;			// The size of this structure, in bytes.  You must specify size !!!
	HDC			hdcDst;			// A handle to the destination device context. 
	RECT		rc;				// Rectangle that specifies where the image is drawn. 
	INT			value;			// Database cloud status (1=full,2=partial,3=unavail)
	HMLIMGLST	hMLIL;			// Media Libary ImageList. Set to NULL if you want to use default media library list.
	INT			index;			// index of image inside Media Libary ImageList to use. Ignored if used with default media library list.
} CLOUDDRAWPARAMS;

#define ML_IPC_CLOUD_FIRST			0x1500L

#define ML_IPC_CLOUD_DRAW			(ML_IPC_CLOUD_FIRST + 1)	// Draws cloud status. param = (WPARAM)(CLOUDDRAWPARAMS*)prdp. return TRUE on success.
#define ML_IPC_CLOUD_CALCRECT		(ML_IPC_CLOUD_FIRST + 2)	// calculates minimal rect required to show cloud status. (param) = (WPARAM)(RECT*)prc. Set prc->left to HMLIMGLST (or NULL to use default) and prc->top to maxValue. Returns TRUE if ok.

#define MLCloud_Draw(/*HWND*/ hwndML, /*CLOUDDRAWPARAMS**/prdp) \
					 (BOOL)SENDMLIPC((hwndML), ML_IPC_CLOUD_DRAW, (WPARAM)(CLOUDDRAWPARAMS*)(prdp))
 
#define MLCloud_CalcRect(/*HWND*/ hwndML, /*HMLIMGLST*/ hmlil, /*RECT**/prc) \
						 (BOOL)SENDMLIPC((hwndML), ML_IPC_CLOUD_CALCRECT, \
						 ((prc) ? ((((RECT*)(prc))->left = hmlil), \
						 (WPARAM)(prc)) : (WPARAM)(RECT*)NULL))


//////////////////////////////////////////////////////////////////////////////
// MLCloudColumn
// Helpers to display cloud in the listview
//
//

typedef struct _CLOUDCOLUMNPAINT 	
{
	HWND		hwndList;	// hwnd of the listview
	HDC			hdc;		// hdc
	UINT		iItem;		// item index
	UINT		iSubItem;	// subitem index
	INT			value;		// database cloud status (1=full,2=partial,3=unavail)
	RECT		*prcItem;	// whole item rect (plvcd->nmcd.rc)
	RECT		*prcView;	// client area size (you can get it at CDDS_PREPAINT in plvcd->nmcd.rc)
	COLORREF	rgbBk;		// color to use as background (plvcd->clrTextBk)
	COLORREF	rgbFg;		// color to use as foreground (plvcd->clrText)
} CLOUDCOLUMNPAINT;

typedef struct _CLOUDBACKTEXT
{
	LPWSTR	pszText;
	INT		cchTextMax;
	INT		nColumnWidth; // used if style is RCS_ALLIGN_CENTER or RCS_ALLIGN_RIGHT
} CLOUDBACKTEXT;

#define ML_IPC_CLOUDCOLUMN_PAINT			(ML_IPC_CLOUD_FIRST + 3)	// Paints cloud column (call it in (CDDS_SUBITEM | CDDS_ITEMPREPAINT) handler).
																		// param = (WPARAM)(CLOUDCOLUMNPAINT*)prcp. if Return TRUE - you need to return SDRF_SKIPDEFAULT
#define ML_IPC_CLOUDCOLUMN_GETMINWIDTH		(ML_IPC_CLOUD_FIRST + 4)	// param = (WPARAM)0. Returns minimum width required to display cloud status column.
#define ML_IPC_CLOUDCOLUMN_GETWIDTH			(ML_IPC_CLOUD_FIRST + 5)	// Set width according to the policies. param = (WPARAM)(INT*)width. Return TRUE if ok. 'width' will contain allowed width


#define MLCloudColumn_Paint(/*HWND*/ hwndML, /*CLOUDCOLUMNPAINT**/prcp) \
							(BOOL)SENDMLIPC((hwndML), ML_IPC_CLOUDCOLUMN_PAINT, (WPARAM)(CLOUDCOLUMNPAINT*)(prcp))

#define MLCloudColumn_GetMinWidth(/*HWND*/ hwndML) \
								  (INT)SENDMLIPC((hwndML), ML_IPC_CLOUDCOLUMN_GETMINWIDTH, (WPARAM)0)

#define MLCloudColumn_GetWidth(/*HWND*/ hwndML, /*INT**/width) \
							   (BOOL)SENDMLIPC((hwndML), ML_IPC_CLOUDCOLUMN_GETWIDTH, (WPARAM)(INT*)(width))


//////////////////////////////////////////////////////////////////////////////////
// Skinning
///

// supported window types
#define SKINNEDWND_TYPE_AUTO			0x0000	// type will be determine based on class name
#define SKINNEDWND_TYPE_ERROR			0x0000	// return code for ML_IPC_SKINNEDWND_GETTYPE in case of error
#define SKINNEDWND_TYPE_WINDOW			0x0001	// base skinned window type.
#define SKINNEDWND_TYPE_SCROLLWND		0x0002	// base skinned window with skinned scrollbar.
#define SKINNEDWND_TYPE_DIALOG			0x0003
#define SKINNEDWND_TYPE_HEADER			0x0004
#define SKINNEDWND_TYPE_LISTVIEW		0x0005
#define SKINNEDWND_TYPE_BUTTON 			0x0006
#define SKINNEDWND_TYPE_DIVIDER 		0x0007
#define SKINNEDWND_TYPE_EDIT 			0x0008
#define SKINNEDWND_TYPE_STATIC			0x0009
#define SKINNEDWND_TYPE_LISTBOX			0x000A // you need to set owner draw style to allow selection skinning
#define SKINNEDWND_TYPE_COMBOBOX		0x000B
#define SKINNEDWND_TYPE_FOLDERBROWSER	0x000C
#define SKINNEDWND_TYPE_POPUPMENU		0x000D
#define SKINNEDWND_TYPE_TOOLTIP			0x000E
#define SKINNEDWND_TYPE_PROGRESSBAR		0x000F

// skin styles
#define SWS_NORMAL					0x00000000
#define SWS_USESKINFONT				0x00000001
#define	SWS_USESKINCOLORS			0x00000002
#define SWS_USESKINCURSORS			0x00000004
#define SWS_NO_DIRECT_MOUSE_WHEEL	0x00000008

// styles listview
#define SWLVS_FULLROWSELECT		0x00010000
#define SWLVS_DOUBLEBUFFER		0x00020000
#define SWLVS_ALTERNATEITEMS	0x00040000 // Since Winamp 5.55
#define SWLVS_SELALWAYS			0x00080000 // Since Winamp 5.56

// styles divider
#define SWDIV_HORZ				0x00000000
#define SWDIV_VERT				0x00010000
#define SWDIV_NOHILITE			0x00020000 // do not draw hilite line

// styles edit (single line)
#define SWES_TOP				0x00000000
#define SWES_BOTTOM				0x00010000
#define SWES_VCENTER			0x00020000
#define SWES_SELECTONCLICK		0x00040000

// styles button
#define SWBS_SPLITBUTTON		0x00010000
#define SWBS_DROPDOWNBUTTON		0x00020000
#define SWBS_SPLITPRESSED		0x00100000
#define SWBS_TOOLBAR			0x00080000 // draw control as a part of the toolbar

// styles combobox
#define SWCBS_TOOLBAR			0x00080000 // draw control as a part of the toolbar

// reserved range 0x10000000 - 0xF0000000  

// Use to skin window (ML_IPC_SKINWINDOW)
typedef struct _MLSKINWINDOW
{
	HWND	hwndToSkin;			// hwnd to skin
	UINT	skinType;			// skin as... SKINNEDWND_TYPE_XXX
	UINT	style;				// skin style supported by this window
} MLSKINWINDOW;

#define ML_IPC_SKINWINDOW		0x1400L		// param = (WPARAM)(MLSKINWINDOW)pmlSkinWnd. Return TRUE if ok. 
											// Note: If window already skinned you will get error. 
											//		 Some controls require they parent to support reflection which provided by all skinned windows. 
											//	 	 In case parent not skined it will be skinned automaticly as SKINNEDWND_TYPE_WINDOW.
											//		 You can always reskin parent window later with desired type.
											//		 Skinned window will unskin itself on WM_NCDESTROY.
#define ML_IPC_UNSKINWINDOW		0x1401L		// param = (WPARAM)(HWND)hwndToUnskin.

// Macros
#define MLSkinWindow(/*HWND*/ hwndML, /*MLSKINWINDOW**/pmlSkinWnd) \
					 (BOOL)SENDMLIPC((HWND)(hwndML), ML_IPC_SKINWINDOW, (WPARAM)(MLSKINWINDOW*)(pmlSkinWnd))

#define MLSkinWindow2(/*HWND*/ __hwndML, /*HWND*/__hwndToSkin, /*UINT*/__skinType, /*UINT*/__skinStyle) \
					 { MLSKINWINDOW __mlsw;\
					   __mlsw.hwndToSkin = __hwndToSkin;\
					   __mlsw.skinType = __skinType;\
					   __mlsw.style = __skinStyle;\
					   (BOOL)SENDMLIPC((HWND)(__hwndML), ML_IPC_SKINWINDOW, (WPARAM)(MLSKINWINDOW*)(&__mlsw));}

#define MLUnskinWindow(/*HWND*/ hwndML, /*HWND*/hwndToUnskin) \
					   (BOOL)SENDMLIPC((HWND)(hwndML), ML_IPC_UNSKINWINDOW, (WPARAM)(HWND)(hwndToUnskin))

#define SMS_NORMAL				0x00000000
#define SMS_USESKINFONT			0x00000001
#define SMS_FORCEWIDTH			0x00000002
#define SMS_DISABLESHADOW		0x00000004
#define SMS_SYSCOLORS			0x00000008

#define MLMENU_WANT_DRAWPART	((-1))

#define MLMENU_ACTION_MEASUREITEM	0	// param = (LPARAM)(MEASUREITEMSTRUCT*)pmis;
#define MLMENU_ACTION_DRAWITEM		1	// param = (LPARAM)(DRAWITEMSTRUCT*)pdis;  Return: 0 - default processing, 1 - if you draw it all, MLMENU_WANT_DRAWPART - want to receive parts
#define MLMENU_ACTION_DRAWBACK		2	// param = (LPARAM)(DRAWITEMSTRUCT*)pdis;  Return: 0 - default processing, 1 - if you draw it all
#define MLMENU_ACTION_DRAWICON		3	// param = (LPARAM)(DRAWITEMSTRUCT*)pdis;  Return: 0 - default processing, 1 - if you draw it all
#define MLMENU_ACTION_DRAWTEXT		4	// param = (LPARAM)(DRAWITEMSTRUCT*)pdis;  Return: 0 - default processing, 1 - if you draw it all

typedef INT (CALLBACK *MENUCUSTOMIZEPROC)(INT /*action*/, HMENU /*hMenu*/, HDC /*hdc*/, LPARAM /*param*/, ULONG_PTR /*user*/);

typedef struct _MLSKINNEDPOPUP
{
	INT			cbSize;
	HMENU		hmenu;
    UINT		fuFlags;
    int			x;
    int			y;
    HWND		hwnd;
    LPTPMPARAMS lptpm;
	HMLIMGLST	hmlil;
	INT			width;	
	UINT		skinStyle;
	MENUCUSTOMIZEPROC customProc;
	ULONG_PTR	customParam;
} MLSKINNEDPOPUP;

#define ML_IPC_TRACKSKINNEDPOPUPEX			0x1402L		// param = (WPARAM)(MLSKINNEDPOPUP)pmlSkinnedPopup
#define MLTrackSkinnedPopupMenuEx(/*HWND*/ __hwndML, /*HMENU*/__hmenu, /*UINT*/__uFlags, /*INT*/__x, /*INT*/__y,\
								  /*HWND*/ __hwndOwner, /*LPTPMPARAMS*/__lptpm, /*HMLIMGLST*/ __hmlil, /*INT*/__width,\
								  /*UINT*/__skinStyle, /*MENUCUSTOMIZEPROC*/ __customProc, /*ULONG_PTR*/ __customParam) \
								  { MLSKINNEDPOPUP __mlspm;\
								    __mlspm.cbSize      = sizeof(MLSKINNEDPOPUP);\
									__mlspm.hmenu       = __hmenu;\
									__mlspm.fuFlags     = __uFlags;\
									__mlspm.x           = __x;\
									__mlspm.y           = __y;\
									__mlspm.hwnd        = __hwndOwner;\
									__mlspm.lptpm       = __lptpm;\
									__mlspm.hmlil       = __hmlil;\
									__mlspm.width       = __width;\
									__mlspm.skinStyle   = __skinStyle;\
									__mlspm.customProc  = __customProc;\
									__mlspm.customParam = __customParam;\
									(BOOL)SENDMLIPC((HWND)(__hwndML), ML_IPC_TRACKSKINNEDPOPUPEX, (WPARAM)(MLSKINNEDPOPUP*)(&__mlspm));}

#define ML_IPC_INITSKINNEDPOPUPHOOK			0x1403L		// param = (WPARAM)(MLSKINNEDPOPUP)pmlSkinnedPopup (you need to fill only hwnd, hmlil, width, skinStyle, customProc, customParam). Return handle
#define ML_IPC_REMOVESKINNEDPOPUPHOOK		0x1404L		// param = (WPARAM)(HANDLE)hPopupHook
#define MLRemoveSkinnedPopupHook(/*HWND*/ __hwndML, /*HANDLE*/__hPopupHook) \
								 (SENDMLIPC((HWND)(__hwndML), ML_IPC_REMOVESKINNEDPOPUPHOOK, (WPARAM)(__hPopupHook)))

typedef struct MLSKINNEDMINMAXINFO
{
	SIZE min;
	SIZE max;
} MLSKINNEDMINMAXINFO;

// Send this messages directly to the skinned window
#define ML_IPC_SKINNEDWND_ISSKINNED			0x0001		// param ignored
#define ML_IPC_SKINNEDWND_GETTYPE			0x0002		// param ignored. return SKINNEDWND_TYPE_XXX or SKINNEDWND_TYPE_ERROR if error.
#define ML_IPC_SKINNEDWND_SKINCHANGED		0x0003		// param = (WPARAM)MAKEWPARAM(NotifyChldren, bRedraw). Return TRUE if ok.
#define ML_IPC_SKINNEDWND_ENABLEREFLECTION	0x0004		// param = (WPARAM)(BOOL)bEnable. Return TRUE if ok. Note: as a lot of skinned controls relay on reflection be carefull using this command
#define ML_IPC_SKINNEDWND_GETPREVWNDPROC	0x0005		// param = (WPARAM)(BOOL*)pbUnicode. Return WNDPROC if ok.
#define ML_IPC_SKINNEDWND_SETSTYLE			0x0006		// param = (WPARAM)(UINT)newStyle. Returns previous style.
#define ML_IPC_SKINNEDWND_GETSTYLE			0x0007		// param igonred. Return current styles
#define ML_IPC_SKINNEDWND_SETMINMAXINFO		0x0008		// param = (WPARAM)(MLSKINNEDMINMAXINFO*)minMax. Returns TRUE if ok.
#define ML_IPC_SKINNEDWND_SKINUPDATED		0x0009		// param = (WPARAM)MAKEWPARAM(NotifyChldren, bRedraw). Return ignored. Send after ML_IPC_SKINNEDWND_SKINCHANGED processed right before redraw.

// Macros
#define MLSkinnedWnd_IsSkinned(/*HWND*/ hwndSkinned) \
							   (BOOL)SENDMLIPC((HWND)(hwndSkinned), ML_IPC_SKINNEDWND_ISSKINNED, (WPARAM)0)

#define MLSkinnedWnd_GetType(/*HWND*/ hwndSkinned) \
							 (UINT)SENDMLIPC((HWND)(hwndSkinned), ML_IPC_SKINNEDWND_GETTYPE, (WPARAM)0)

#define MLSkinnedWnd_SkinChanged(/*HWND*/ hwndSkinned, /*BOOL*/ bNotifyChildren, /*BOOL*/ bRedraw) \
								 (BOOL)SENDMLIPC((HWND)(hwndSkinned), ML_IPC_SKINNEDWND_SKINCHANGED, MAKEWPARAM( (0 != (bNotifyChildren)), (0 != (bRedraw))))

#define MLSkinnedWnd_EnableReflection(/*HWND*/ hwndSkinned, /*BOOL*/ bEnable) \
									  (BOOL)SENDMLIPC((HWND)(hwndSkinned), ML_IPC_SKINNEDWND_ENABLEREFLECTION, (WPARAM)(BOOL)(bEnable))

#define MLSkinnedWnd_GetPrevWndProc(/*HWND*/ hwndSkinned, /*BOOL* */ pbUnicode) \
									(WNDPROC)SENDMLIPC((HWND)(hwndSkinned), ML_IPC_SKINNEDWND_GETPREVWNDPROC, (WPARAM)(BOOL*)(pbUnicode))

#define MLSkinnedWnd_SetStyle(/*HWND*/ __hwndSkinned, /*UINT*/ __newStyles) \
							  (UINT)SENDMLIPC((HWND)(__hwndSkinned), ML_IPC_SKINNEDWND_SETSTYLE, (WPARAM)(__newStyles))

#define MLSkinnedWnd_GetStyle(/*HWND*/ __hwndSkinned) \
							  (UINT)SENDMLIPC((HWND)(__hwndSkinned), ML_IPC_SKINNEDWND_GETSTYLE, (WPARAM)0)

#define MLSkinnedWnd_SetMinMaxInfo(/*HWND*/ __hwndSkinned, /*MLSKINNEDMINMAXINFO* */ _minMax) \
								   (UINT)SENDMLIPC((HWND)(__hwndSkinned), ML_IPC_SKINNEDWND_SETMINMAXINFO, (WPARAM)(_minMax))


#define SCROLLMODE_STANDARD		0x0000
#define SCROLLMODE_LISTVIEW		0x0001
#define SCROLLMODE_TREEVIEW		0x0002
#define SCROLLMODE_COMBOLBOX	0x0003


// Send this messages directly to the skinned window
#define IPC_ML_SKINNEDSCROLLWND_UPDATEBARS		0x0010		// param = (WPARAM)(BOOL)fInvalidate
#define IPC_ML_SKINNEDSCROLLWND_SHOWHORZBAR		0x0011		// param = (WPARAM)(BOOL)bEnable
#define IPC_ML_SKINNEDSCROLLWND_SHOWVERTBAR		0x0013		// param = (WPARAM)(BOOL)bEnable
#define IPC_ML_SKINNEDSCROLLWND_SETMODE			0x0012		// param = (WPARAM)(UINT)nScrollMode
#define IPC_ML_SKINNEDSCROLLWND_DISABLENOSCROLL	0x0014		// param = (WPARAM)(BOOL)bDisableNoScroll

typedef struct _SBADJUSTHOVER
{
	UINT	hitTest;
	POINTS	ptMouse;
	INT		nResult;
} SBADJUSTHOVER;

#define IPC_ML_SKINNEDSCROLLWND_ADJUSTHOVER			0x0015		// param = (WPARAM)(SBADJUSTHOVER*)pAdjustData
#define IPC_ML_SKINNEDSCROLLWND_GETHORZBARHIDDEN	0x0016		// param - not used, returns TRUE if horizontal scrollbar always hidden
#define IPC_ML_SKINNEDSCROLLWND_GETVERTBARHIDDEN	0x0017		// param - not used, returns TRUE if vertical scrollbar always hidden
#define IPC_ML_SKINNEDSCROLLWND_GETMODE				0x0018		// param - not used, returns current mode
#define IPC_ML_SKINNEDSCROLLWND_GETNOSCROLLDISABLED	0x0019		// param - not used, returns TRUE if no scroll disabled.

#define MLSkinnedScrollWnd_UpdateBars(/*HWND*/ hwndSkinned, /*BOOL*/ fInvalidate) \
									  (BOOL)SENDMLIPC((HWND)(hwndSkinned), IPC_ML_SKINNEDSCROLLWND_UPDATEBARS, (WPARAM)(BOOL)(fInvalidate))

#define MLSkinnedScrollWnd_ShowHorzBar(/*HWND*/ hwndSkinned, /*BOOL*/ bEnable) \
									   (BOOL)SENDMLIPC((HWND)(hwndSkinned), IPC_ML_SKINNEDSCROLLWND_SHOWHORZBAR, (WPARAM)(BOOL)(bEnable))

#define MLSkinnedScrollWnd_ShowVertBar(/*HWND*/ hwndSkinned, /*BOOL*/ bEnable) \
									   (BOOL)SENDMLIPC((HWND)(hwndSkinned), IPC_ML_SKINNEDSCROLLWND_SHOWVERTBAR, (WPARAM)(BOOL)(bEnable))

#define MLSkinnedScrollWnd_SetMode(/*HWND*/ hwndSkinned, /*UINT*/ nScrollMode) \
								   (BOOL)SENDMLIPC((HWND)(hwndSkinned), IPC_ML_SKINNEDSCROLLWND_SETMODE, (WPARAM)(UINT)(nScrollMode))

#define MLSkinnedScrollWnd_DisableNoScroll(/*HWND*/ __hwndSkinned, /*BOOL*/ __bDisable) \
										   (BOOL)SENDMLIPC((__hwndSkinned), IPC_ML_SKINNEDSCROLLWND_DISABLENOSCROLL, (WPARAM)(__bDisable))

#define MLSkinnedScrollWnd_GetHorzBarHidden(/*HWND*/ __hwndSkinned) \
											(BOOL)SENDMLIPC((__hwndSkinned), IPC_ML_SKINNEDSCROLLWND_GETHORZBARHIDDEN, 0)

#define MLSkinnedScrollWnd_GetVertBarHidden(/*HWND*/ __hwndSkinned) \
											(BOOL)SENDMLIPC((__hwndSkinned), IPC_ML_SKINNEDSCROLLWND_GETVERTBARHIDDEN, 0)

#define MLSkinnedScrollWnd_GetMode(/*HWND*/ __hwndSkinned) \
								   (UINT)SENDMLIPC((__hwndSkinned), IPC_ML_SKINNEDSCROLLWND_GETMODE, 0)

#define MLSkinnedScrollWnd_GetNoScrollDisabled(/*HWND*/ __hwndSkinned) \
											   (BOOL)SENDMLIPC((__hwndSkinned), IPC_ML_SKINNEDSCROLLWND_GETNOSCROLLDISABLED, 0)

// Send this messages directly to the skinned window
#define ML_IPC_SKINNEDLISTVIEW_DISPLAYSORT	0x0020		// param = MAKEWPARAM(sortIndex, ascending), To hide set sortIndex to -1.
#define ML_IPC_SKINNEDLISTVIEW_GETSORT		0x0021		// param not used. Return LOWORD() - index, HIWORD() - ascending 
#define ML_IPC_SKINNEDLISTVIEW_SETCURRENT	0x0022		// param = (WPARAM)(INT)currentItem. if currentItem == -1 - the current item text will be removed,
														// otherwise it will select the item specified (used for showing the currently playing item - 5.7+)
#define ML_IPC_SKINNEDHEADER_DISPLAYSORT	0x0030		// param = MAKEWPARAM(sortIndex, ascending)
#define ML_IPC_SKINNEDHEADER_GETSORT		0x0031		// param not used. Return LOWORD() - index, HIWORD() - ascending 
#define ML_IPC_SKINNEDHEADER_SETHEIGHT		0x0032		// param = (WPARAM)(INT)newHeight. if newHeight == -1 - height will be calculated based on the font size.
#define ML_IPC_SKINNEDHEADER_SETCLOUDCOLUMN	0x0033		// param = (WPARAM)(INT)cloudColumn. sets the column which will show a cloud icon header image

#define MLSkinnedListView_DisplaySort(/*HWND*/hwndSkinned, /*WORD*/ index, /*WORD*/ascending) \
									  (BOOL)SENDMLIPC((HWND)(hwndSkinned), ML_IPC_SKINNEDLISTVIEW_DISPLAYSORT, MAKEWPARAM((index), (ascending)))
#define MLSkinnedListView_GetSort(/*HWND*/hwndSkinned) \
								  (UINT)SENDMLIPC((HWND)(hwndSkinned), ML_IPC_SKINNEDLISTVIEW_GETSORT, 0)

#define MLSkinnedHeader_GetSort(/*HWND*/hwndSkinned) \
								(UINT)SENDMLIPC((HWND)(hwndSkinned), ML_IPC_SKINNEDHEADER_GETSORT, 0)
#define MLSkinnedHeader_SetHeight(/*HWND*/hwndSkinned, /*INT*/ newHeight) \
								  (UINT)SENDMLIPC((HWND)(hwndSkinned), ML_IPC_SKINNEDHEADER_SETHEIGHT, (WPARAM)newHeight)

#define MLSkinnedHeader_SetCloudColumn(/*HWND*/hwndSkinned, /*INT*/ cloudColumn) \
								  (UINT)SENDMLIPC((HWND)(hwndSkinned), ML_IPC_SKINNEDHEADER_SETCLOUDCOLUMN, (WPARAM)cloudColumn)

// Skinned divider messages

typedef void (CALLBACK *MLDIVIDERMOVED)(HWND /*hwndDivider*/, INT /*nPos*/, LPARAM /*userParam*/);

typedef struct _MLDIVIDERCALLBACK
{
	MLDIVIDERMOVED fnCallback;
	LPARAM userParam;
} MLDIVIDERCALLBACK;

#define ML_IPC_SKINNEDDIVIDER_SETCALLBACK	0x0010		// param = (WPARAM)(MLDIVIDERCALLBACK*)dividerCallback.

#define MLSkinnedDivider_SetCallback(/*HWND*/_hwndSkinned, /*MLDIVIDERMOVED*/ _fnCallback, /*LPARAM*/_userParam) \
									 { MLDIVIDERCALLBACK _cbData;\
									   _cbData.fnCallback = (_fnCallback);\
									   _cbData.userParam = (_userParam);\
									   (BOOL)SENDMLIPC((HWND)(_hwndSkinned), ML_IPC_SKINNEDDIVIDER_SETCALLBACK, ((WPARAM)(MLDIVIDERCALLBACK*)(&_cbData)));}

// Skinned button
#define MLBN_DROPDOWN           20

typedef struct _MLBUTTONIMAGELIST
{
	HMLIMGLST	hmlil;
	INT			normalIndex;  // set index to -1 if you don't have special image for this type
	INT			pressedIndex;
	INT			hoverIndex;
	INT			disabledIndex;
} MLBUTTONIMAGELIST;

#define ML_IPC_SKINNEDBUTTON_SETIMAGELIST	0x0010		// param = (WPARAM)(MLBUTTONIMAGELIST*)buttonImagelist.
#define ML_IPC_SKINNEDBUTTON_GETIMAGELIST	0x0011		// param = (WPARAM)(MLBUTTONIMAGELIST*)buttonImagelist. 
#define ML_IPC_SKINNEDBUTTON_SETDROPDOWNSTATE	0x0012	// param = (WPARAM)bPressed. 
#define ML_IPC_SKINNEDBUTTON_GETIDEALSIZE		0x0013	// param = (WPARAN)(LPCWSTR)pszText - if pszText == NULL will use button text. Returns: HIWORD - height, LOWORD width

#define MLSkinnedButton_SetImageList(/*HWND*/__hwndSkinned, /*HMLIMGLST*/ __hmlil, /*INT*/__normalIndex, \
									 /*INT*/__pressedIndex, /*INT*/ __hoverIndex, /*INT*/__disabledIndex) \
									 { MLBUTTONIMAGELIST __buttonImagelist;\
									   __buttonImagelist.hmlil = (__hmlil);\
									   __buttonImagelist.normalIndex = (__normalIndex);\
									   __buttonImagelist.pressedIndex = (__pressedIndex);\
									   __buttonImagelist.hoverIndex = (__hoverIndex);\
									   __buttonImagelist.disabledIndex = (__disabledIndex);\
									   (BOOL)SENDMLIPC((HWND)(__hwndSkinned), ML_IPC_SKINNEDBUTTON_SETIMAGELIST, ((WPARAM)(MLBUTTONIMAGELIST*)(&__buttonImagelist)));}

#define MLSkinnedButton_GetImageList(/*HWND*/__hwndSkinned, /*MLBUTTONIMAGELIST* */ __pButtonImagelist)\
									 (BOOL)SENDMLIPC((HWND)(__hwndSkinned), ML_IPC_SKINNEDBUTTON_GETIMAGELIST, ((WPARAM)(MLBUTTONIMAGELIST*)(__pButtonImagelist)))

#define MLSkinnedButton_SetDropDownState(/*HWND*/__hwndSkinned, /*BOOL*/ __bPressed)\
										 SENDMLIPC((HWND)(__hwndSkinned), ML_IPC_SKINNEDBUTTON_SETDROPDOWNSTATE, ((WPARAM)(__bPressed)))

#define MLSkinnedButton_GetIdealSize(/*HWND*/__hwndSkinned, /*LPCWSTR*/__pszText)\
									 ((DWORD)SENDMLIPC((HWND)(__hwndSkinned), ML_IPC_SKINNEDBUTTON_GETIDEALSIZE, (WPARAM)(__pszText)))

// Skinned static
#define ML_IPC_SKINNEDSTATIC_GETIDEALSIZE		0x0014	// param = (WPARAN)(LPCWSTR)pszText - if pszText == NULL will use button text. Returns: HIWORD - height, LOWORD width
#define MLSkinnedStatic_GetIdealSize(/*HWND*/__hwndSkinned, /*LPCWSTR*/__pszText)\
									 ((DWORD)SENDMLIPC((HWND)(__hwndSkinned), ML_IPC_SKINNEDSTATIC_GETIDEALSIZE, (WPARAM)(__pszText)))

//////////////////////////////////////////////////////////////////////////////////
// WebInfo
///

typedef struct _WEBINFOCREATE
{
	HWND hwndParent;		// HWND of the parent
	UINT uMsgQuery;			// message that WebInfo can use to query filename
	INT	x;					// position x
	INT	y;					// position y
	INT cx;					// size cx;
	INT cy;					// size cy ( if size cy == -1 height will be set to preffer height and cy will be modified
	INT	ctrlId;				// ctrlId you want to use
} WEBINFOCREATE;

typedef struct _WEBINFOSHOW
{
	LPCWSTR pszFileName;	// File name.
	UINT	fFlags;			// flags WIDF_XXX
} WEBINFOSHOW;

#define WISF_NORMAL			0x0000	
#define WISF_FORCE			0x0001	// even if file name same as was in previous call command will be executed
#define WISF_MESSAGE		0x0100	// pszFileName contains pointer to the message that need to be displayed

// Messages
#define ML_IPC_CREATEWEBINFO			0x1410L			// param = (WPARAM)(WEBINFOCREATE*)pWebInfoParam. Return HWND of the WebInfo dialog or NULL if error

/// Send this messages directly to the window
#define ML_IPC_WEBINFO_RELEASE			0x0100			// Call this in your WM_DESTROY.
#define ML_IPC_WEBINFO_SHOWINFO			0x0101			// Call this to show file info. param = (WPARAM)(WEBINFOSHOW)pWebInfoShow. Returns TRUE on ok.


////////// FolderBrowser

#define FOLDERBROWSER_NAME		L"MLFolderBrowser"

// styles
#define FBS_IGNOREHIDDEN		0x0001
#define FBS_IGNORESYSTEM		0x0002


#define FBM_FIRST				0x2100      

#define FBM_GETBKCOLOR          (FBM_FIRST + 0)
#define FolderBrowser_GetBkColor(hwnd)  \
								 ((COLORREF)SNDMSG((hwnd), FBM_GETBKCOLOR, 0, 0L))

#define FBM_SETBKCOLOR          (FBM_FIRST + 1)
#define FolderBrowser_SetBkColor(hwnd, clrBk) \
								 ((BOOL)SNDMSG((hwnd), FBM_SETBKCOLOR, 0, (LPARAM)(COLORREF)(clrBk)))

#define FBM_GETTEXTCOLOR        (FBM_FIRST + 2)
#define FolderBrowser_GetTextColor(hwnd)  \
								   ((COLORREF)SNDMSG((hwnd), FBM_GETTEXTCOLOR, 0, 0L))

#define FBM_SETTEXTCOLOR        (FBM_FIRST + 3)
#define FolderBrowser_SetTextColor(hwnd, clrText) \
								   ((BOOL)SNDMSG((hwnd), FBM_SETTEXTCOLOR, 0, (LPARAM)(COLORREF)(clrText)))

typedef struct _FOLDERBROWSERINFO
{
	DWORD cbSize;
	HWND hwndDraw;
	HWND hwndActive;
	size_t activeColumn;
} FOLDERBROWSERINFO;

#define FBM_GETFOLDERBROWSERINFO      (FBM_FIRST + 4)
#define FolderBrowser_GetInfo(__hwnd, /*FOLDERBROWSERINFO* */__pControlInfo) \
							  ((BOOL)SNDMSG((__hwnd), FBM_GETFOLDERBROWSERINFO, 0, (LPARAM)(__pControlInfo)))

#define FBM_SETROOT      (FBM_FIRST + 5)
#define FolderBrowser_SetRoot(__hwnd, /*LPCWSTR*/__pszRoot) \
							  ((BOOL)SNDMSG((__hwnd), FBM_SETROOT, 0, (LPARAM)(__pszRoot)))

#define FBM_GETROOT      (FBM_FIRST + 6)
#define FolderBrowser_GetRoot(__hwnd, /*LPWSTR*/__pszBuffer, /*INT*/ __cchMax) \
							  ((INT)SNDMSG((__hwnd), FBM_GETROOT, (WPARAM)(__cchMax), (LPARAM)(__pszBuffer)))

typedef HANDLE (CALLBACK *FBC_FINDFIRSTFILE)(LPCWSTR /*lpFileName*/, WIN32_FIND_DATAW* /*lpFindFileData*/); 
typedef BOOL (CALLBACK *FBC_FINDNEXTFILE)(HANDLE /*hFindFile*/, WIN32_FIND_DATAW* /*lpFindFileData*/); 
typedef BOOL (CALLBACK *FBC_FINDCLOSE)(HANDLE /*hFindFile*/); 

typedef struct _FILESYSTEMINFO
{
	UINT				cbSize;
	FBC_FINDFIRSTFILE	fnFindFirstFile;
	FBC_FINDNEXTFILE	fnFindNextFile;
	FBC_FINDCLOSE		fnFindClose;
} FILESYSTEMINFO;

#define FBM_SETFILESYSTEMINFO      (FBM_FIRST + 7)  // lParam = (FILESYSTEMINFO*)pFSInfo ; Set pFSInfo == NULL to use default.
#define FolderBrowser_SetFileSystemInfo(__hwnd, /*FILESYSTEMINFO* */__pFSInfo) \
										((BOOL)SNDMSG((__hwnd), FBM_SETFILESYSTEMINFO, 0, (LPARAM)(__pFSInfo)))

#define FBM_GETCURRENTPATH      (FBM_FIRST + 8)  // wParam - (INT) cchTextMax, lParam = (LPWSTR*)pszText.
#define FolderBrowser_GetCurrentPath(__hwnd, /*LPWSTR */ __pszText, /*INT*/ __cchTextMax) \
									 ((BOOL)SNDMSG((__hwnd), FBM_GETCURRENTPATH, (WPARAM)(__cchTextMax), (LPARAM)(__pszText)))

#define FBM_SETCURRENTPATH      (FBM_FIRST + 9)  //  lParam = (LPCWSTR*)pszPath.
#define FolderBrowser_SetCurrentPath(__hwnd, /*LPCWSTR */ __pszPath, /*BOOL*/__bRedraw) \
									 ((BOOL)SNDMSG((__hwnd), FBM_SETCURRENTPATH, (WPARAM)(__bRedraw), (LPARAM)(__pszPath)))

#define FBM_GETFILESYSTEMINFO      (FBM_FIRST + 10)  // lParam = (FILESYSTEMINFO*)pFSInfo;
#define FolderBrowser_GetFileSystemInfo(__hwnd, /*FILESYSTEMINFO* */__pFSInfo) \
										((BOOL)SNDMSG((__hwnd), FBM_GETFILESYSTEMINFO, 0, (LPARAM)(__pFSInfo)))

#define EVF_NOEXTRARSPACE			0x001
#define EVF_NOEXTRALSPACE			0x002
#define EVF_NOREDRAW				0x004

#define FBM_ENSUREVISIBLE        (FBM_FIRST + 11)
#define FolderBrowser_EnsureVisible(__hwnd, /*INT*/ __columnIndex, /*UINT*/ __uFlags) \
									((BOOL)SNDMSG((__hwnd), FBM_ENSUREVISIBLE, MAKEWPARAM((__columnIndex), (__uFlags)), 0L))

// Folder browser notifications
#define FBN_FIRST	(0U-3000U)
#define FBN_SELCHANGED	(FBN_FIRST + 1)		//phdr = (NMHDR*)lParam. No return value.

////////// FileView

typedef struct __MLFILEVIEWCREATESTRUCT
{
	HWND hwndParent;
	UINT fStyle;
	HWND hwndInsertAfter;
	INT x;
	INT y; 
	INT cx;
	INT cy;
} MLFILEVIEWCREATESTRUCT;

#define ML_IPC_CREATEFILEVIEW		0x1412L	 //param = (WPARAM)(MLFILEVIEWCREATESTRUCT*)pmlFileViewCreateStruct

// styles
#define FVS_VIEWMASK		0x000F
#define FVS_LISTVIEW		0x0000
#define FVS_ICONVIEW		0x0001
#define FVS_DETAILVIEW		0x0002

#define FVS_IGNOREHIDDEN	0x0010
#define FVS_HIDEEXTENSION	0x0020

#define FVS_SHOWAUDIO		0x0100
#define FVS_SHOWVIDEO		0x0200
#define FVS_SHOWPLAYLIST	0x0400
#define FVS_SHOWUNKNOWN		0x0800
#define FVS_ENQUEUE			0x1000		// will use enqueue as default command

// messages
#define MLFVM_FIRST			(WM_APP + 0x0001)

#define FVM_SETROOT			(MLFVM_FIRST + 0)

#define FileView_SetRoot(/*HWND*/ __hwndFV, /*LPCWSTR*/  __pszRoot)\
						 ((BOOL)SENDMSG((__hwndFV), FVM_SETROOT, 0, (LPARAM)(__pszRoot)))

#define FVM_GETROOT			(MLFVM_FIRST + 1)

#define FileView_GetRoot(/*HWND*/ __hwndFV, /*LPCWSTR*/  __pszBuffer, /*INT*/ __cchMax)\
						 ((INT)SENDMSG((__hwndFV), FVM_SETROOT, (WPARAM)(__cchMax), (LPARAM)(__pszBuffer)))

#define FVM_REFRESH			(MLFVM_FIRST + 2)

#define FileView_Refresh(/*HWND*/ __hwndFV, /*BOOL*/ __bIncludeFolder)\
						 ((BOOL)SENDMSG((__hwndFV), FVM_REFRESH, (WPARAM)(__bIncludeFolder), 0L))

#define FVM_SETSTYLE		(MLFVM_FIRST + 3)

#define FileView_SetStyle(/*HWND*/ __hwndFV, /*UINT*/ _uStyle, /*UINT*/ _uStyleMask)\
						  ((BOOL)SENDMSG((__hwndFV), FVM_SETSTYLE, (WPARAM)(_uStyleMask), (LPARAM)(_uStyle)))

#define FVM_GETSTYLE		(MLFVM_FIRST + 4)

#define FileView_GetStyle(/*HWND*/ __hwndFV)\
						  ((UINT)SENDMSG((__hwndFV), FVM_GETSTYLE, 0, 0L))

#define FVM_SETSORT			(MLFVM_FIRST + 5)

#define FileView_SetSort(/*HWND*/ __hwndFV, /*UINT*/ __uColumn, /*BOOL*/ __bAscending)\
						 ((BOOL)SENDMSG((__hwndFV), FVM_SETSORT, MAKEWPARAM((__uColumn), (__bAscending)), 0L))

#define FVM_GETSORT			(MLFVM_FIRST + 6) // returns MAKELONG(sortColumn, bAscending) or -1  if error.

#define FileView_GetSort(/*HWND*/ __hwndFV)\
						 ((DWORD)SENDMSG((__hwndFV), FVM_GETSORT, 0, 0L))

#define FVM_GETCOLUMNCOUNT	(MLFVM_FIRST + 7)		// wParam - not used, lParam - not used, returns column count in current view

#define FileView_GetColumnCount(/*HWND*/ __hwndFV)\
								((INT)SENDMSG((__hwndFV), FVM_GETCOLUMNCOUNT, 0, 0L))

// columns
#define FVCOLUMN_NAME			0
#define FVCOLUMN_SIZE			1
#define FVCOLUMN_TYPE			2
#define FVCOLUMN_MODIFIED		3
#define FVCOLUMN_CREATED		4
#define FVCOLUMN_EXTENSION		5
#define FVCOLUMN_ATTRIBUTES		6
#define FVCOLUMN_ARTIST			7
#define FVCOLUMN_ALBUM			8
#define FVCOLUMN_TITLE			9
#define FVCOLUMN_INMLDB			10
#define FVCOLUMN_GENRE			11
#define FVCOLUMN_COMMENT		12
#define FVCOLUMN_LENGTH			13
#define FVCOLUMN_BITRATE		14
#define FVCOLUMN_TRACK			15
#define FVCOLUMN_DISC			16
#define FVCOLUMN_YEAR			17
#define FVCOLUMN_PUBLISHER		18
#define FVCOLUMN_COMPOSER		19
#define FVCOLUMN_ALBUMARTIST	20

#define FVM_GETCOLUMNARRAY		(MLFVM_FIRST + 8)		// wParam - (WPARAM)iCount, lParam - (LPARAM)(UINT*)puArray, returns actually copied count.

#define FileView_GetColumnArray(/*HWND*/ __hwndFV, /*INT*/__iCount, /*UINT* */ __puArray)\
								((INT)SENDMSG((__hwndFV), FVM_GETCOLUMNARRAY, (WPARAM)(__iCount), (LPARAM)(__puArray)))

#define FVM_DELETECOLUMN		(MLFVM_FIRST + 9)		// wParam - (WPARAM)ufvColumn, lParam - not used, returns TRUE on success

#define FileView_DeleteColumn(/*HWND*/ __hwndFV, /*UINT*/__ufvColumn)\
							  ((BOOL)SENDMSG((__hwndFV), FVM_DELETECOLUMN, (WPARAM)(__ufvColumn), 0L))

#define FVCF_WIDTH		1
#define FVCF_ORDER		2
#define FVCF_WIDTHMIN	4
#define FVCF_WIDTHMAX	8

#define FVCO_DEFAULT_ORDER		((INT)0x80000000)		// can be used only with FVM_INSERTCOLUMN

typedef struct _FVCOLUMN
{
	UINT mask;
	UINT id;
	INT	width;
	INT order;
	INT widthMin;
	INT widthMax;
} FVCOLUMN;

#define FVM_INSERTCOLUMN		(MLFVM_FIRST + 10)		// wParam - not used, lParam - (LPARAM)(FVCOLUMN*)pfvColumn, returns column index or -1

#define FileView_InsertColumn(/*HWND*/ __hwndFV, /*FVCOLUMN* */__pfvColumn)\
							  ((INT)SENDMSG((__hwndFV), FVM_INSERTCOLUMN, (WPARAM)0, (LPARAM)(__pfvColumn)))

#define FVM_GETCOLUMNAME		(MLFVM_FIRST + 11)		// wParam - MAKEWPARAM(ufvColumn, cchTextMax), lParam - (LPARAM)(LPWSTR)pszText, returns TRUE if ok

#define FileView_GetColumnName(/*HWND*/ __hwndFV, /*UINT*/__ufvColumn, /*INT*/__cchTextMax, /*LPWSTR*/__pszText)\
							   ((BOOL)SENDMSG((__hwndFV), FVM_GETCOLUMNAME, MAKEWPARAM((__ufvColumn), (__cchTextMax)), (LPARAM)(__pszText)))

#define FVM_SETFILESYSTEMINFO	(MLFVM_FIRST + 12)		// wParam - not used,lParam = (FILESYSTEMINFO*)pFSInfo, returns TRUE on success.

#define FileView_SetFileSystemInfo(/*HWND*/ __hwndFV, /*FILESYSTEMINFO* */__pFSInfo) \
								   ((BOOL)SNDMSG((__hwndFV), FVM_SETFILESYSTEMINFO, 0, (LPARAM)(__pFSInfo)))

#define FVM_GETFILESYSTEMINFO	(MLFVM_FIRST + 13)		// wParam - not used,lParam = (FILESYSTEMINFO*)pFSInfo, returns TRUE on success.

#define FileView_GetFileSystemInfo(/*HWND*/ __hwndFV, /*FILESYSTEMINFO* */__pFSInfo) \
								   ((BOOL)SNDMSG((__hwndFV), FVM_GETFILESYSTEMINFO, 0, (LPARAM)(__pFSInfo)))

#define FVM_GETFILECOUNT		(MLFVM_FIRST + 14)		// wParam - not used,lParam = not used, returns the number of items.

#define FileView_GetFileCount(/*HWND*/ __hwndFV) \
							  ((INT)SNDMSG((__hwndFV), FVM_GETFILECOUNT, 0, 0L))

#define FVM_GETSELECTEDCOUNT	(MLFVM_FIRST + 15)		// wParam - not used,lParam = not used, Returns the number of selected items. 

#define FileView_GetSelectedCount(/*HWND*/ __hwndFV) \
								  ((INT)SNDMSG((__hwndFV), FVM_GETSELECTEDCOUNT, 0, 0L))

#define FVNF_ALL                0x0000
#define FVNF_FOCUSED            0x0001
#define FVNF_SELECTED           0x0002
#define FVNF_CUT                0x0004
#define FVNF_DROPHILITED        0x0008
#define FVNF_PLAYABLE			0x0080

#define FVNF_ABOVE              0x0100
#define FVNF_BELOW              0x0200
#define FVNF_TOLEFT             0x0400
#define FVNF_TORIGHT            0x0800

#define FVM_GETNEXTFILE			(MLFVM_FIRST + 16)		// wParam - (WPARAM)(INT)iStart, lParam = (LPARAM)(UINT)uFlags, Returns the index of the next file if successful, or -1 otherwise.

#define FileView_GetNextFile(/*HWND*/ __hwndFV, /*INT*/ __iStart, /*UINT*/  __uFlags) \
							 ((INT)SNDMSG((__hwndFV), FVM_GETNEXTFILE, (WPARAM)(__iStart), (LPARAM)(__uFlags)))

// file states
#define FVFS_FOCUSED		0x0001	// The file has the focus, so it is surrounded by a standard focus rectangle. Although more than one file may be selected, only one file can have the focus.
#define FVFS_SELECTED		0x0002	// The file is selected. 
#define FVFS_CUT			0x0004	// The file is marked for a cut-and-paste operation.
#define FVFS_DROPHILITED	0x0008	// The file is highlighted as a drag-and-drop target.

// FileView file types
#define FVFT_UNKNOWN	0
#define FVFT_AUDIO		1
#define FVFT_VIDEO		2
#define FVFT_PLAYLIST	3
#define FVFT_LAST		FVFT_PLAYLIST

#define FVIF_TEXT				0x0001
#define FVIF_STATE				0x0002
#define FVIF_ATTRIBUTES			0x0004
#define FVIF_SIZE				0x0008
#define FVIF_CREATETIME			0x0010
#define FVIF_ACCESSTIME			0x0020
#define FVIF_WRITETIME			0x0040
#define FVIF_TYPE				0x0080

typedef struct __FVITEM
{
	UINT		mask;
    UINT		state;
    UINT		stateMask;
    LPWSTR		pszText;			// in some case returned pszText can be pointed on internal buffer and not one that was provided
    INT			cchTextMax;
    UINT		uAttributes;
	DWORD		dwSizeLow;
	DWORD		dwSizeHigh;
	FILETIME	ftCreationTime;
	FILETIME	ftLastAccessTime;
	FILETIME	ftLastWriteTime;
	WORD		wType;
} FVITEM, *PFVITEM;

#define FVM_GETFILE				(MLFVM_FIRST + 17)		// wParam - (WPARAM)(INT)iFile, lParam = (LPARAM)(FVITEM*)pFile, Returns TRUE if successful, or FALSE otherwise.

#define FileView_GetFile(/*HWND*/ __hwndFV, /*INT*/ __iFile, /*PFVITEM*/  __pFile) \
						 ((BOOL)SNDMSG((__hwndFV), FVM_GETFILE, (WPARAM)(__iFile), (LPARAM)(__pFile)))

#define FVM_GETCURRENTPATH		(MLFVM_FIRST + 18)		// wParam - (WPARAM)(INT)cchPathMax, lParam = (LPARAM)(LPWSTR)pszPath,  Returns TRUE if successful, or FALSE otherwise.

#define FileView_GetCurrentPath(/*HWND*/ __hwndFV, /*LPWSTR*/  __pszBuffer, /*INT*/ __cchBufferMax) \
								((BOOL)SNDMSG((__hwndFV), FVM_GETCURRENTPATH, (WPARAM)(__cchBufferMax), (LPARAM)(__pszBuffer)))

#define FVM_GETFOLDERBROWSER	(MLFVM_FIRST + 19)		// wParam - not used, lParam - not used,  Returns HWND of the folderbrowser.

#define FileView_GetFoderBrowser(/*HWND*/ __hwndFV) \
								 ((HWND)SNDMSG((__hwndFV), FVM_GETFOLDERBROWSER, 0, 0L))

#define FVM_GETFOLDERSIZE		(MLFVM_FIRST + 20)		// wParam - (WPARAM)(BOOL)bSelectionOnly, lParam - (LPARAM)(DWORD*)pdwFoldeSizeHigh (optional),  Returns dwSizeLow.

#define FileView_GetFoderSize(/*HWND*/ __hwndFV, /*BOOL*/ __bSelectionOnly, /*LPDWROD*/ __pdwFoldeSizeHigh) \
							  ((DWORD)SNDMSG((__hwndFV), FVM_GETFOLDERSIZE, (WPARAM)(__bSelectionOnly), (LPARAM)(__pdwFoldeSizeHigh)))

#define FVM_GETSTATUSTEXT		(MLFVM_FIRST + 21)		// wParam - (WPARAM)(INT)cchPathMax, lParam = (LPARAM)(LPWSTR)pszPath,  Returns TRUE if successful, or FALSE otherwise.

#define FileView_GetStatusText(/*HWND*/ __hwndFV, /*LPWSTR*/  __pszText, /*INT*/ __cchTextMax) \
							   ((BOOL)SNDMSG((__hwndFV), FVM_GETSTATUSTEXT, (WPARAM)(__cchTextMax), (LPARAM)(__pszText)))

#define FVEF_AUDIO		0x0001
#define FVEF_VIDEO		0x0002
#define FVEF_PLAYLIST	0x0004
#define FVEF_UNKNOWN	0x0008

#define FVEF_ALLKNOWN	(FVEF_AUDIO | FVEF_VIDEO| FVEF_PLAYLIST)
#define FVEF_ALL		(FVEF_ALLKNOWN | FVEF_UNKNOWN)

#define FVM_ENQUEUESELECTION	(MLFVM_FIRST + 22)		// wParam - (WPARAM)(UINT)uEnqueueFilter, lParam - (LPARAM)(INT*)pnFocused, Returns number of files sent to playlist. pnFocused - optional (if not null will contain playlist index of focused item)

#define FileView_EnqueueSelection(/*HWND*/ __hwndFV, /*UINT*/ __uEnqueueFilter, /*PINT*/__pnFocused) \
								  ((BOOL)SNDMSG((__hwndFV), FVM_ENQUEUESELECTION, (WPARAM)(__uEnqueueFilter), (LPARAM)(__pnFocused)))

#define FVM_ISFILEPLAYABLE		(MLFVM_FIRST + 23)	//  wParam - (WPARAM)(UINT)uEnqueueFilter, lParam - (LPARAM)(INT)iFile - file index,  Returns TRUE if successful, or FALSE otherwise.

#define FileView_IsFilePlayable(/*HWND*/ __hwndFV, /*INT*/  __iFile, /*UINT*/ __uEnqueueFilter) \
								((BOOL)SNDMSG((__hwndFV), FVM_ISFILEPLAYABLE, (WPARAM)(__uEnqueueFilter), (LPARAM)(__iFile)))

#define FVM_SETFILESTATE		(MLFVM_FIRST + 24)	//  wParam - (WPARAM)(INT)iFile ( if -1 applyed to all files), (LPARAM)(LPFVITEM)pFile - only stateMask and state are used,  Returns TRUE if successful, or FALSE otherwise.

#define FileView_SetFileState(/*HWND*/ __hwndFV, /*INT*/ __iFile, /*UINT*/ __uMask, /*UINT*/ __uData) \
							  { FVITEM __fvi;\
							    __fvi.stateMask = __uMask;\
								__fvi.state = __uData;\
								SNDMSG((__hwndFV), FVM_SETFILESTATE, (WPARAM)(__iFile), (LPARAM)(LV_ITEM *)&__fvi);}

#define FVMENU_OPTIONS			0
#define FVMENU_COLUMNS			1
#define FVMENU_ARRANGEBY		2
#define FVMENU_PLAY				3
#define FVMENU_FILEVIEWCONTEXT	4
#define FVMENU_FILEOPCONTEXT	5
#define FVMENU_FILECONTEXT		6		// in getMenu will be resolved to FVMENU_FILEVIEWCONTEXT or FVMENU_FILEOPCONTEXT
#define FVMENU_VIEWMODE			7

#define IDM_COLUMN_SHOW_MIN		60000
#define IDM_COLUMN_SHOW_MAX		60099
#define IDM_COLUMN_ARRANGE_MIN	60100
#define IDM_COLUMN_ARRANGE_MAX	60199

#define FVM_GETMENU				(MLFVM_FIRST + 25)	//  wParam - (WPARAM)(UINT)uMenuType, lParam - not used.

#define FileView_GetMenu(/*HWND*/ __hwndFV, /*UINT*/  __uMenuType) \
						 ((HMENU)SNDMSG((__hwndFV), FVM_GETMENU, (WPARAM)(__uMenuType), 0L))

#define FVA_PLAY			0
#define FVA_ENQUEUE			1

#define FVM_GETACTIONCMD		(MLFVM_FIRST + 26)	//  wParam - (WPARAM)(UINT)uAction, lParam - not used. returns 0 - if no cmd, otherwise cmd id.

#define FileView_GetActionCommand(/*HWND*/ __hwndFV, /*UINT*/  __uAction) \
								  ((UINT)SNDMSG((__hwndFV), FVM_GETACTIONCMD, (WPARAM)(__uAction), 0L))

#define FVHT_NOWHERE            0x0001
#define FVHT_ONFILEICON         0x0002
#define FVHT_ONFILELABEL        0x0004
#define FVHT_ONFILE             (FVHT_ONFILEICON | FVHT_ONFILELABEL)
#define FVHT_ABOVE              0x0008
#define FVHT_BELOW              0x0010
#define FVHT_TORIGHT            0x0020
#define FVHT_TOLEFT             0x0040

typedef struct _FVHITTEST
{
	POINT	pt;			// The position to hit test, in fileview client coordinates . 
	UINT	uFlags;		// 
	INT		iItem;		// file index;
} FVHITTEST;

#define FVM_HITTEST				(MLFVM_FIRST + 27)	//  wParam - not used, lParam - (LPARAM)(FVHITTEST*)pHitTest. Returns the index of the item at the specified position, if any, or -1 otherwise.

#define FileView_HitTest(/*HWND*/ __hwndFV, /*FVHITTEST* */  __pHitTest) \
						 ((INT)SNDMSG((__hwndFV), FVM_HITTEST, 0, (LPARAM)__pHitTest))

#define FVM_GETCOLUMNWIDTH		(MLFVM_FIRST + 28)		// wParam - (WPARAM)ufvColumn - column id, lParam - not used. returns column width

#define FileView_GetColumnWidth(/*HWND*/ __hwndFV, /*UINT*/__ufvColumn)\
								((BOOL)SENDMSG((__hwndFV), FVM_GETCOLUMNWIDTH, (WPARAM)(__ufvColumn), 0L))

#define FVM_SETCURRENTPATH		(MLFVM_FIRST + 29)

#define FileView_SetCurrentPath(__hwnd, /*LPCWSTR */ __pszPath, /*BOOL*/__bRedraw) \
								((BOOL)SNDMSG((__hwnd), FVM_SETCURRENTPATH, (WPARAM)(__bRedraw), (LPARAM)(__pszPath)))

#define FVM_GETDIVIDERPOS		 (MLFVM_FIRST + 30)
#define FileView_GetDividerPos(__hwnd) \
							   ((INT)SNDMSG((__hwnd), FVM_GETDIVIDERPOS, 0,  0L))

#define FVRF_VALIDATE		0x0001
#define FVRF_NOREDRAW		0x0002

#define FVM_SETDIVIDERPOS		 (MLFVM_FIRST + 31)
#define FileView_SetDividerPos(__hwnd, /*INT*/ __nPos, /*UINT*/ __uFlags) \
							   ((INT)SNDMSG((__hwnd), FVM_SETDIVIDERPOS, (__nPos),  (__uFlags)))

// Folder browser notifications
#define FVN_FIRST	(0U-3200U)

#define FVN_FOLDERCHANGED		(FVN_FIRST + 1)		//phdr = (NMHDR*)lParam. No return value.

typedef struct __NMFVSTATECHANGED
{
	NMHDR	hdr;
	INT		iFrom;
	INT		iTo;
	UINT	uNewState;
	UINT	uOldState;
} NMFVSTATECHANGED;

#define FVN_STATECHANGED		(FVN_FIRST + 2)		//phdr = (NMFVSTATECHANGED*)lParam. No return value.

#define FVN_STATUSCHANGED		(FVN_FIRST + 3)		//phdr = (NMHDR*)lParam. No return value.

typedef struct __NMFVFILEACTIVATE
{
	NMHDR	hdr;
	INT		iFile;
	UINT	uNewState;
    UINT	uOldState;
	POINT	ptAction;
	UINT	uKeyFlags;
} NMFVFILEACTIVATE;

typedef struct __NMFVMENU
{
	NMHDR	hdr;
	UINT	uMenuType;  // FVMENU_XXX
	HMENU	hMenu;		// handle to thr menu
	UINT	uCommand;	// FVN_MENUCOMMNAD - user selected command
	POINT	ptAction;	// point where menu will be dispalayed
} NMFVMENU;

#define FVN_INITMENU		(FVN_FIRST + 4)		// phdr = (NMFVMENU*)lParam. Return TRUE to prevent menu from appearing, otherwise FALSE.
#define FVN_UNINITMENU		(FVN_FIRST + 5)		// phdr = (NMFVMENU*)lParam. no return value.
#define FVN_MENUCOMMAND		(FVN_FIRST + 6)		// phdr = (NMFVMENU*)lParam. Return TRUE to prevent command processing, otherwise FALSE.

// Fileview also implements this notifications: NM_CLICK, NM_DBLCLK, NM_RCLICK, NM_RDBLCLK. they all have phdr = (NMFVFILEACTIVATE*)lParam.

#endif // NULLOSFT_MEDIALIBRARY_IPC_EXTENSION_HEADER_0x0313

#if (defined _ML_HEADER_IMPMLEMENT && !defined _ML_HEADER_IMPMLEMENT_DONE)
#define _ML_HEADER_IMPMLEMENT_DONE

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// {8A054D1F-E38E-4cc0-A78A-F216F059F57E}
const GUID MLIF_FILTER1_UID = { 0x8a054d1f, 0xe38e, 0x4cc0, { 0xa7, 0x8a, 0xf2, 0x16, 0xf0, 0x59, 0xf5, 0x7e } };
// {BE1A6A40-39D1-4cfb-8C33-D8988E8DD2F8}
const GUID MLIF_FILTER2_UID = { 0xbe1a6a40, 0x39d1, 0x4cfb, { 0x8c, 0x33, 0xd8, 0x98, 0x8e, 0x8d, 0xd2, 0xf8 } };
// {721E9E62-CC6D-4fd7-A6ED-DD4CD2B2612E}
const GUID MLIF_FILTER3_UID = { 0x721e9e62, 0xcc6d, 0x4fd7, { 0xa6, 0xed, 0xdd, 0x4c, 0xd2, 0xb2, 0x61, 0x2e } };
// {B6310C20-E731-44dd-83BD-FBC3349798F2}
const GUID MLIF_GRAYSCALE_UID = { 0xb6310c20, 0xe731, 0x44dd, { 0x83, 0xbd, 0xfb, 0xc3, 0x34, 0x97, 0x98, 0xf2 } };
// {526C6F4A-C979-4d6a-B8ED-1A90F5A26F7B}
const GUID MLIF_BLENDONBK_UID = { 0x526c6f4a, 0xc979, 0x4d6a, { 0xb8, 0xed, 0x1a, 0x90, 0xf5, 0xa2, 0x6f, 0x7b } };
// {E61C5E67-B2CC-4f89-9C95-40D18FCAF1F8}
const GUID MLIF_BUTTONBLEND_UID =  { 0xe61c5e67, 0xb2cc, 0x4f89, { 0x9c, 0x95, 0x40, 0xd1, 0x8f, 0xca, 0xf1, 0xf8 } };
// {3619BA52-5088-4f21-9AF1-C5FCFE5AAA99}
const GUID MLIF_BUTTONBLENDPLUSCOLOR_UID = { 0x3619ba52, 0x5088, 0x4f21, { 0x9a, 0xf1, 0xc5, 0xfc, 0xfe, 0x5a, 0xaa, 0x99 } };
// {F1DD3228-7DA8-4524-B7D3-46F651BFB680}
const GUID MLIF_FILTER1_PRESERVE_ALPHA_UID = { 0xf1dd3228, 0x7da8, 0x4524, { 0xb7, 0xd3, 0x46, 0xf6, 0x51, 0xbf, 0xb6, 0x80 } };

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ML_HEADER_IMPMLEMENT