#ifndef NULLOSFT_MEDIALIBRARY_FILEVIEW_CONTROL_INTERNAL_HEADER
#define NULLOSFT_MEDIALIBRARY_FILEVIEW_CONTROL_INTERNAL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
#include "../winamp/gen.h"
#include "api__gen_ml.h"
#include "./ml_ipc_0313.h"
#include "../nu/trace.h"
#include "./ml_imagelist.h"
#include "./ml_imagefilter.h"
#include "./skinning.h"

#ifndef LONGX86
#ifdef _WIN64
  #define LONGX86	LONG_PTR
#else /*_WIN64*/
  #define LONGX86	 LONG	
#endif  /*_WIN64*/
#endif // LONGX86

#define METADATA_SOURCE_UNKNOWN				0
#define METADATA_SOURCE_FILEINFO			1
#define METADATA_SOURCE_MLDB				2


#define FVM_GETIDEALHEIGHT		(MLFVM_FIRST + 101)	// internal use

#define CSTR_INVARIANT		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

#define METATYPE_AUDIO		0
#define METATYPE_VIDEO		1
#define METATYPE_PLAYLIST	2

#define MF_NAME				0
#define MF_SIZE				1
#define MF_TYPE				2
#define MF_MODIFIED			3
#define MF_CREATED			4
#define MF_EXTENSION			5
#define MF_ATTRIBUTES		6
#define MF_ARTIST			7
#define MF_ALBUM				8
#define MF_TITLE				9
#define MF_INMLDB			10
#define MF_GENRE				11
#define MF_COMMENT			12
#define MF_LENGTH			13
#define MF_BITRATE			14
#define MF_TRACKNUM			15
#define MF_TRACKCOUNT		16
#define MF_DISCNUM			17
#define MF_DISCCOUNT			18
#define MF_YEAR				19
#define MF_PUBLISHER			20
#define MF_COMPOSER			21
#define MF_ALBUMARTIST		22
#define MF_SOURCE			23

typedef struct __AUDIOMETA
{
	UINT	nSource;
	LPWSTR	pszArtist;
	LPWSTR	pszTitle;
	LPWSTR	pszAlbum;
	LPWSTR  pszGenre;
	LPWSTR  pszComment;
	INT		nLength;
	INT		nBitrate;
	INT		nTrackNum;
	INT		nTrackCount;
	INT		nDiscNum;
	INT		nDiscCount;
	INT		nYear;
	LPWSTR	pszAlbumArtist;
	LPWSTR pszPublisher;
	LPWSTR pszComposer;
} AUDIOMETA;

typedef __AUDIOMETA VIDEOMETA;

#define MAX_PLAYLIST_ENTRIES		20
typedef struct __PLENTRY
{
	LPWSTR	pszTitle;
	INT		nLength;
} PLENTRY;
typedef struct __PLAYLISTMETA
{
	UINT	nCount;
	LPWSTR	pszTitle;
	INT		nLength;
	PLENTRY	szEntries[MAX_PLAYLIST_ENTRIES];
} PLAYLISTMETA;


typedef struct _FILEMETARECORD
{ 
	DWORD type; 
	union 
	{
		AUDIOMETA		audio; 
		VIDEOMETA		video;
		PLAYLISTMETA		playlist;
	};
}FILEMETARECORD;

typedef struct _FILERECORD
{	
	WIN32_FIND_DATAW		Info;
	INT					fileType;
	size_t				extOffset;
	FILEMETARECORD		*pMeta;
} FILERECORD;

typedef struct _FILEDATA
{	
	size_t			count;
	size_t			allocated;
	FILERECORD		*pRec;
	size_t			*pSort;
	ULONGLONG		folderSize;
	WCHAR			szPath[MAX_PATH*2];
} FILEDATA;

#ifdef __cplusplus
extern "C" {
#endif

extern winampGeneralPurposePlugin plugin;

#ifdef __cplusplus
}
#endif
extern HWND g_hwnd;
extern HMLIMGFLTRMNGR hmlifMngr;	// default gen_ml fitler manager




// toolbar
HWND FileViewToolbar_Create(HWND hwndParent);

// filesystem 
size_t FileView_ReadFileData(FILEDATA *pfd, LPCWSTR pszPath, UINT fStyle, FILESYSTEMINFO *pfsi);
LPCWSTR FileView_GetTypeFamily(LPCWSTR pszExtension);

// metadata
typedef void (CALLBACK *DISCOVERCALLBACK)(LPCWSTR /*pszFileName*/, ULONG_PTR /*param*/);

void FileViewMeta_InitializeStorage(HWND hView);
void FileViewMeta_ReleaseStorage(HWND hView);
FILEMETARECORD *FileViewMeta_GetFromCache(LPCWSTR pszPath, FILERECORD *pfr);
BOOL FileViewMeta_Discover(LPCWSTR pszPath, FILERECORD *pfr, DISCOVERCALLBACK fnCallback, ULONG_PTR param, INT queueMax);
void FileViewMeta_TruncateQueue(size_t max);
BOOL FileViewMeta_GetString(FILEMETARECORD *pMeta, UINT uMetaField, LPCWSTR *ppszOut);
BOOL FileViewMeta_GetInt(FILEMETARECORD *pMeta, UINT uMetaField, INT *pOut);

// formatting
INT FileView_FormatFileTime(FILETIME *pft, LPWSTR pszDest, INT cchDest);
INT FileView_FormatType(UINT fileType, LPWSTR pszDest, INT cchDest);
INT FileView_FormatAttributes(UINT uAttributes, LPWSTR pszDest, INT cchDest);
INT FileView_FormatYesNo(BOOL bValue, LPWSTR pszDest, INT cchDest);
INT FileView_FormatYear(INT nYear, LPWSTR pszDest, INT cchDest);
INT FileView_FormatBitrate(INT nBitrate, LPWSTR pszDest, INT cchDest);
INT FileView_FormatLength(INT nLength, LPWSTR pszDest, INT cchDest);
INT FileView_FormatIntSlashInt(INT part1, INT part2, LPWSTR pszDest, INT cchDest);

#define FIF_TOOLTIP		0
#define FIF_STATUS		1
void FileView_FormatFileInfo(FILERECORD *pfr, LPWSTR pszText, size_t cchTextMax, UINT mode);
void FileView_FormatAudioTip(FILERECORD *pfr, LPWSTR pszText, size_t cchTextMax, LPCWSTR pszSeparator);
void FileView_FormatDefaultTip(FILERECORD *pfr, LPWSTR pszText, size_t cchTextMax, LPCWSTR pszSeparator);

// sorting
void FileView_SortByColumn(FILEDATA *pFileData, UINT uColumn);
void FileView_SortByColumnEx(FILEDATA *pFileData, UINT uColumn, size_t *pOrder, size_t count);

// menu
HMENU FileViewMenu_Initialize();
HMENU FileViewMenu_GetSubMenu(HWND hView, HMENU hViewMenu, UINT uMenuType);
UINT FileViewMenu_GetMenuType(HWND hView, HMENU hViewMenu, HMENU hMenu);
// view
void FileView_DisplayPopupMenu(HWND hdlg, UINT uMenu, UINT uFlags, POINT pt);

// registered columns

typedef struct _FILEVIEWCOLUMN
{
	UINT	id;
	LPWSTR	pszText;
	INT		width;
	UINT	format;
	INT		order;
	INT		widthMin;
	INT		widthMax;
} FILEVIEWCOLUMN;

extern const FILEVIEWCOLUMN szRegisteredColumns[];
extern const INT RegisteredColumnsCount;



#endif // NULLOSFT_MEDIALIBRARY_FILEVIEW_CONTROL_INTERNAL_HEADER