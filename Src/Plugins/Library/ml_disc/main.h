#ifndef NULLSOFT_MAINH
#define NULLSOFT_MAINH

#include "..\..\General\gen_ml/ml.h"
#include "..\..\General\gen_ml/ml_ipc_0313.h"

#include "./config.h"
#include "./drivemngr.h"
#include "./drive.h"
#include "./medium.h"
#include "api__ml_disc.h"
#include "..\..\General\gen_ml/menu.h"

#include "./copyfiles.h"
#include "./copyinternal.h"

#include <shlobj.h>
#include <windows.h>
#include <commctrl.h>

#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

#define OLD_AAC_CODEC mmioFOURCC('A','A','C',' ')

#define BN_EX_GETTEXT	0x0FFF 

#define WM_EXTRACTDISC		(WM_APP + 0x010)
#define WM_SHOWFILEINFO		(WM_APP + 0x11) // (wParam)(UINT)WISF_, (lParam)(LPCWSTR)file(track)name (can be NULL to reset)
#define WM_QUERYFILEINFO	(WM_APP + 0x12) 
#define WM_TAGUPDATED		(WM_APP + 0x13) // wParam = 0, lParam = (LPCWSTR)pszFileName

#define VCM_CREATECOMMANDBAR	(WM_APP + 0x20)
#define VCM_DESTROYCOMMANDBAR	(WM_APP + 0x21)
#define VCM_GETCOMMANDBAR		(WM_APP + 0x22)
#define VCM_GETMININFOENABLED	(WM_APP + 0x23)
#define VCM_GETMININFOVISIBLE	(WM_APP + 0x24)


typedef struct __CMDBARCREATESTRUCT
{
	HWND		hwndOwner;
	UINT		resourceId;
	DLGPROC		fnDialogProc;
	ULONG_PTR	uData;
} CMDBARCREATESTRUCT;

#define MSGRESULT(__hwnd, __result) { SetWindowLongPtrW((__hwnd), DWLP_MSGRESULT, ((LONGX86)(LONG_PTR)(__result))); return TRUE; }

#define ViewContainer_CreateCmdBar(/*HWND*/ __hwndViewContainer, /*HWND*/ __hwndOwner, /*INT_PTR*/ __resourceId, /*DLGPROC*/ __fnDialogProc, /*ULONG_PTR*/ __uData)\
		{CMDBARCREATESTRUCT cs; cs.hwndOwner = (__hwndOwner); cs.resourceId = (__resourceId); cs.fnDialogProc = (__fnDialogProc); cs.uData = (__uData);\
		((HWND)SNDMSG((__hwndViewContainer), VCM_CREATECOMMANDBAR, 0, (LPARAM)(&cs)));}

#define ViewContainer_DestroyCmdBar(/*HWND*/ __hwndViewContainer)\
		((BOOL)SNDMSG((__hwndViewContainer), VCM_DESTROYCOMMANDBAR, 0, 0L))

#define ViewContainer_GetCmdBar(/*HWND*/ __hwndViewContainer)\
		((HWND)SNDMSG((__hwndViewContainer), VCM_GETCOMMANDBAR, 0, 0L))

#define ViewContainer_GetMiniInfoEnabled(/*HWND*/ __hwndViewContainer)\
		((HWND)SNDMSG((__hwndViewContainer), VCM_GETMININFOENABLED, 0, 0L))

#define ViewContainer_GetMiniInfoVisible(/*HWND*/ __hwndViewContainer)\
		((HWND)SNDMSG((__hwndViewContainer), VCM_GETMININFOVISIBLE, 0, 0L))


extern winampMediaLibraryPlugin plugin;
extern LARGE_INTEGER freq;

void CleanupDirectoryString(LPTSTR pszDirectory);
LPWSTR GetExtensionString(LPWSTR pszBuffer, INT cchBufferMax, DWORD fourcc);
HRESULT FormatFileName(LPTSTR pszTextOut, INT cchTextMax, LPCTSTR pszFormat,
					   INT nTrackNo, LPCTSTR pszArtist,
					   LPCTSTR pszAlbum, LPCTSTR pszTitle,
					   LPCTSTR pszGenre, LPCTSTR pszYear,
					   LPCTSTR pszTrackArtist,
					   LPCTSTR pszFileName, LPCTSTR pszDisc);


bool RegisteredEncoder(DWORD fourcc);

extern C_Config *g_config;
extern HMENU g_context_menus;
extern C_Config *g_view_metaconf;

#define DSF_CANRECORD	0x00010000

#define DSF_PLAYING		0x00000001
#define DSF_RIPPING		0x00000002
#define DSF_BURNING		0x00000004
#define DSF_GETTINGINFO	0x00000008

typedef struct _DRIVE
{
	CHAR	cLetter;
	CHAR	cMode;
	WCHAR	szTitle[64];
	DWORD	textSize;
	BOOL	textOrigWidth;
	WORD		itemWidth;
	BYTE		nBtnState;
	BOOL	bEjectVisible;
	UINT_PTR	timerId;
} DRIVE;


typedef BOOL (CALLBACK *NAVITEMENUMPROC)(HNAVITEM hItem, DRIVE *pDrive, LPARAM param);

DRIVE *Plugin_GetDriveFromNavItem(HNAVITEM hItem);
HNAVITEM Plugin_GetNavItemFromLetter(CHAR cLetter);
BOOL Plugin_EnumerateNavItems(NAVITEMENUMPROC callback, LPARAM param);
void Plugin_RegisterListener(HWND hwnd, UINT uMsg, CHAR cLetter); // active view can register itself to be notified about drive/medium changes if cLetter = 0 you will be notifed for all drives
void Plugin_UnregisterListener(HWND hwnd);
void Plugin_ShowRippingPreferences(void);
BOOL Plugin_IsExtractScheduled(CHAR cLetter);

int getFileInfo(const char *filename, const char *metadata, char *dest, int len);
int getFileInfoW(const wchar_t *filename, const wchar_t *metadata, wchar_t *dest, int len);

#define HF_DOMODAL			0x0001
#define HF_ALLOWRESIZE		0x0010

HWND MLDisc_ShowHelp(HWND hParent, LPCWSTR pszWindowTitle, LPCWSTR pszCaption, LPCWSTR pszText, UINT uFlags); // returns hwnd only if not HF_DOMODAL


#define QBF_SHOW_CHECKBOX		0x00000001L
#define QBF_SHOW_EXTRA_BUTTON	0x00000002L
#define QBF_TOPMOST				0x00000100L
#define QBF_SETFOREGROUND		0x00000200L
#define QBF_BEEP				0x00000400L
#define QBF_FLASH				0x00000800L
#define QBF_DEFAULT_OK			0x00000000L
#define QBF_DEFAULT_CANCEL		0x00001000L
#define QBF_DEFAULT_EXTRA1		0x00002000L


typedef struct _QUESTIONBOX
{
	HWND			hParent;			// [in]
	LPCTSTR		pszIcon;			// [in]
	UINT		uBeepType;			// [in]
	LPCTSTR		pszTitle;			// [in] accepts MAKEINTRESOURCE() as parameters. 
	LPCTSTR		pszMessage;			// [in] accepts MAKEINTRESOURCE() as parameters. 
	UINT		uFlags;				// [in]
	LPCTSTR		pszBtnOkText;		// [in] accepts MAKEINTRESOURCE() as parameters.
	LPCTSTR		pszBtnCancelText;	// [in] accepts MAKEINTRESOURCE() as parameters.
	LPCTSTR		pszCheckboxText;	// [in] accepts MAKEINTRESOURCE() as parameters.
	LPCTSTR		pszBtnExtraText;	// [in] accepts MAKEINTRESOURCE() as parameters.
	BOOL		checkboxChecked;	// [in][out] 
} QUESTIONBOX;

INT_PTR MLDisc_ShowQuestionBox(QUESTIONBOX *pQuestionBox);  // returns pressed button id;

// cdrip.cpp
BOOL CALLBACK CDRipPrefsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

typedef struct
{
	char drive_letter;

	wchar_t *album;
	wchar_t *artist;
	wchar_t *genre;
	wchar_t *year;
	wchar_t *publisher; // record label
	wchar_t *disc; // disc ##/##
	wchar_t *comment; // notes from CDDB
	wchar_t **composers; 
	wchar_t **conductors;
	wchar_t **gracenoteFileIDs;
	wchar_t **gracenoteExtData;
	int  total_length_bytes;

	int ntracks; // total number of tracks
	wchar_t **tracks; // set these to NULL to not rip em
	wchar_t **trackArtists;

	int  *lengths; // lengths, in seconds

	wchar_t **filenames;	// can be used internally to override output filenames 
							// (should always allocate, but leave NULL ptrs in the array)
	wchar_t **tempFilenames;	//where we are ripping to, we'll move at the end
} cdrip_params;

void cdrip_extractFiles(cdrip_params *parms);

int cdrip_isextracting(char drive);
void cdrip_stop_all_extracts();

//gracenote.cpp
void gracenoteInit();
int gracenoteQueryFile(const char *filename);
void gracenoteCancelRequest();
int gracenoteDoTimerStuff();
void gracenoteSetValues(char *artist, char *album, char *title);
char *gracenoteGetTuid();
int gracenoteIsWorking();

//view_ripburn.cpp
INT_PTR CALLBACK view_ripburnDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

// view_cdrom.cpp
void saveCDToItemRecordList(CHAR cLetter, itemRecordList *obj, char *album);
int cdrom_contextMenu(HWND parent, CHAR cLetter, HNAVITEM hItem);
void cdburn_appendItemRecord(itemRecordList *obj, char driveletter);

HWND CreateContainerWindow(HWND hwndParent, CHAR cLetter, BOOL bQueryInfo);
HWND CreateWaitWindow(HWND hwndParent, CHAR cLetter);
HWND CreateInfoWindow(HWND hwndParent, CHAR cLetter);
HWND CreateCDViewWindow(HWND hwndParent, DM_NOTIFY_PARAM *phdr);
HWND CreateCDBurnWindow(HWND hwndParent, CHAR cLetter);
HWND CreateCDRipWindow(HWND hwndParent, CHAR cLetter);
HWND CreateCdDataViewWindow(HWND hwndParent, CHAR cLetter);

BOOL CALLBACK browseEnumProc(HWND hwnd, LPARAM lParam);

#endif