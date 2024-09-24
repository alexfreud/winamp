#ifndef NULLOSFT_MEDIALIBRARY_MLDISC_SETTINGS_HEADER
#define NULLOSFT_MEDIALIBRARY_MLDISC_SETTINGS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

	// Groups
enum
{
	C_EXTRACT = 0,
	C_COPY,
	C_GLOBAL,
	C_DATAVIEW,
};

// Extract Group Fields
enum
{
	EF_PATH = 0,
	EF_TITLEFMT,
	EF_PLAYLISTFMT,
	EF_UPPEREXTENSION,
	EF_ADDMETADATA,
	EF_CALCULATERG,
	EF_USETOTALTRACKS,
	EF_ADDTOMLDB,
	EF_TRACKOFFSET,
	EF_COMMENTTEXT,
	EF_CREATEM3U,
	EF_CREATEPLS,
	EF_CREATEMLPL,
	EF_USEM3UEXT,
	EF_FOURCC,
};

// Copy Gorup fields
enum
{
	CF_PATH = 0,
	CF_USETITLEFMT,
	CF_TITLEFMT,
	CF_ADDTOMLDB,
	CF_CALCULATERG,
	CF_GAINMODE,
};

// Global group
enum
{
	GF_SHOWINFO = 0,
	GF_SHOWPARENT,
	GF_ENQUEUEBYDEFAULT,
};
// Data View flags
enum
{
	DVF_COLUMNLIST = 0,
	DVF_ORDERBY,
	DVF_ORDERASC,
	DVF_VIEWMODE,
	DVF_SHOWAUDIO,
	DVF_SHOWVIDEO,
	DVF_SHOWPLAYLIST,
	DVF_SHOWUNKNOWN,
	DVF_HIDEEXTENSION,
	DVF_IGNOREHIDDEN,
	DVF_LASTFOLDER,
	DVF_DIVIDERPOS,
};
			
HRESULT Settings_ReadValue(INT categoryId, INT fieldId, VOID *pValue, INT cbSize);
HRESULT Settings_GetDefault(INT categoryId, INT fieldId, VOID *pValue);
HRESULT Settings_ReadString(INT categoryId, INT fieldId, LPTSTR pszBuffer, INT cchBufferMax);
HRESULT Settings_GetInt(INT categoryId, INT fieldId, INT *pnVal);
HRESULT Settings_GetBool(INT categoryId, INT fieldId, BOOL *pbVal);

HRESULT Settings_SetWindowText(INT categoryId, INT fieldId, HWND hwnd);
HRESULT Settings_SetWindowInt(INT categoryId, INT fieldId, HWND hwnd);
HRESULT Settings_SetDlgItemText(INT categoryId, INT fieldId, HWND hdlg, INT nItemId);
HRESULT Settings_SetDlgItemInt(INT categoryId, INT fieldId, HWND hdlg, INT nItemId);
HRESULT Settings_SetCheckBox(INT categoryId, INT fieldId, HWND hdlg, INT nItemId);

HRESULT Settings_SetString(INT categoryId, INT fieldId, LPCWSTR pszBuffer);
HRESULT Settings_SetInt(INT categoryId, INT fieldId, INT nValue);
HRESULT Settings_SetBool(INT categoryId, INT fieldId, BOOL bValue);
HRESULT Settings_FromWindowText(INT categoryId, INT fieldId, HWND hwnd);
HRESULT Settings_FromDlgItemText(INT categoryId, INT fieldId, HWND hdlg, INT nItemId);
HRESULT Settings_FromCheckBox(INT categoryId, INT fieldId, HWND hdlg, INT nItemId);
HRESULT Settings_BrowseForFolder(INT categoryId, INT fieldId, HWND hDlg, INT nEditId);

HRESULT Settings_SetDirectoryCtrl(INT categoryId, INT fieldId, HWND hdlg, INT nItemId);
HRESULT Settings_FromDirectoryCtrl(INT categoryId, INT fieldId, HWND hdlg, INT nItemId);


#ifdef __cplusplus
}
#endif



#endif // NULLOSFT_MEDIALIBRARY_MLDISC_SETTINGS_HEADER