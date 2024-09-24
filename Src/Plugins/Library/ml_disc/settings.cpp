#include "main.h"
#include "./settings.h"
#include "./resource.h"
#include <shlwapi.h>
#include <strsafe.h>

#define SECTION_DEFAULT		TEXT("gen_ml_config")
#define SECTION_DATAVIEW		TEXT("data_view")
// Keys
#define KEY_EF_PATH				TEXT("extractpath")
#define KEY_EF_TITLEFMT			TEXT("extractfmt2")
#define KEY_EF_PLAYLISTFMT		TEXT("extractplfmt2")
#define KEY_EF_COMMENTTEXT		TEXT("tagcomment")
#define KEY_EF_UPPEREXTENSION	TEXT("extractucext")
#define KEY_EF_ADDMETADATA		TEXT("extracttag")
#define KEY_EF_CALCULATERG		TEXT("auto_rg")
#define KEY_EF_USETOTALTRACKS	TEXT("total_tracks")
#define KEY_EF_ADDTOMLDB			TEXT("extractaddml")
#define KEY_EF_TRACKOFFSET		TEXT("trackoffs")
#define KEY_EF_CREATEM3U			TEXT("extractm3u")
#define KEY_EF_CREATEPLS			TEXT("extractpls")
#define KEY_EF_CREATEMLPL		TEXT("extractplml")
#define KEY_EF_USEM3UEXT			TEXT("extractm3uext")
#define KEY_EF_FOURCC			TEXT("extract4cc")

// Defeault values
#define DEF_EF_PATH				GetDefaultExtractPath()
#define DEF_EF_TITLEFMT			TEXT("<Artist> - <Album>\\## - <Trackartist> - <Title>")
#define DEF_EF_PLAYLISTFMT		TEXT("<Artist> - <Album>\\<Artist> - <Album>")
#define DEF_EF_COMMENTTEXT		TEXT("Ripped by Winamp")
#define DEF_EF_UPPEREXTENSION	FALSE
#define DEF_EF_ADDMETADATA		TRUE
#define DEF_EF_CALCULATERG		FALSE
#define DEF_EF_USETOTALTRACKS	FALSE
#define DEF_EF_ADDTOMLDB			TRUE
#define DEF_EF_TRACKOFFSET		1
#define DEF_EF_CREATEM3U			TRUE
#define DEF_EF_CREATEPLS		FALSE
#define DEF_EF_CREATEMLPL		TRUE
#define DEF_EF_USEM3UEXT			TRUE
#define DEF_EF_FOURCC			mmioFOURCC('A','A','C','f')


#define KEY_CF_PATH				KEY_EF_PATH
#define KEY_CF_USETITLEFMT		TEXT("copy_use_title_fmt")
#define KEY_CF_TITLEFMT			TEXT("copy_title_fmt")
#define KEY_CF_ADDTOMLDB			TEXT("copy_add_to_mldb")
#define KEY_CF_CALCULATERG		TEXT("copy_calc_gain")
#define KEY_CF_GAINMODE			TEXT("copy_gain_mode")

#define DEF_CF_PATH				DEF_EF_PATH
#define DEF_CF_USETITLEFMT		TRUE
#define DEF_CF_TITLEFMT			TEXT("<Artist>\\<Album>\\<Filename><extension>")
#define DEF_CF_ADDTOMLDB			DEF_EF_ADDTOMLDB
#define DEF_CF_CALCULATERG		DEF_EF_CALCULATERG
#define DEF_CF_GAINMODE			0


#define KEY_GF_SHOWINFO			TEXT("showinfo")
#define KEY_GF_SHOWPARENT		TEXT("showparent")
#define KEY_GF_ENQUEUEBYDEFAULT	TEXT("enqueuedef")

#define DEF_GF_SHOWINFO			FALSE
#define DEF_GF_SHOWPARENT		FALSE
#define DEF_GF_ENQUEUEBYDEFAULT	FALSE

#define KEY_DVF_COLUMNLIST		TEXT("column_list")
#define KEY_DVF_LASTFOLDER		TEXT("last_folder")
#define KEY_DVF_ORDERBY			TEXT("column_order_by")
#define KEY_DVF_ORDERASC			TEXT("column_order_asc")
#define KEY_DVF_VIEWMODE			TEXT("view_mode")
#define KEY_DVF_SHOWAUDIO		TEXT("show_audio")
#define KEY_DVF_SHOWVIDEO		TEXT("show_video")
#define KEY_DVF_SHOWPLAYLIST		TEXT("show_playlist")
#define KEY_DVF_SHOWUNKNOWN		TEXT("show_unknown")
#define KEY_DVF_HIDEEXTENSION	TEXT("hide_extension")
#define KEY_DVF_IGNOREHIDDEN		TEXT("ignore_hidden")
#define KEY_DVF_DIVIDERPOS		TEXT("horz_divider_pos")


#define DEF_DVF_COLUMNLIST		NULL
#define DEF_DVF_LASTFOLDER		NULL
#define DEF_DVF_ORDERBY			0
#define DEF_DVF_ORDERASC		TRUE
#define DEF_DVF_VIEWMODE			-1
#define DEF_DVF_SHOWAUDIO		TRUE
#define DEF_DVF_SHOWVIDEO		TRUE
#define DEF_DVF_SHOWPLAYLIST		TRUE
#define DEF_DVF_SHOWUNKNOWN		FALSE
#define DEF_DVF_HIDEEXTENSION	TRUE
#define DEF_DVF_IGNOREHIDDEN	TRUE
#define DEF_DVF_DIVIDERPOS		240


// configs
#define GLOBAL						g_config
#define VIEW							g_view_metaconf

// readers
#define READ_LPTSTR(config)			(config)->ReadCbStringEx
#define READ_QUOTED_LPTSTR(config)	(config)->ReadCbQuotedString
#define GET_INT(config)				(config)->ReadIntEx
#define GET_BOOL(config)			(config)->ReadBoolEx

// writers
#define WRITE_LPCTSTR(config)			(config)->WriteStringEx
#define WRITE_QUOTED_LPCTSTR(config)		(config)->WriteQuotedString
#define WRITE_INT(config)				(config)->WriteIntEx
#define WRITE_BOOL(config)				(config)->WriteBoolEx


#define CHECK_EXREAD(type, config, section, field, buffer, cb) case field##: READ_##type##(config)(((##type##)buffer), cb, section, KEY_##field, DEF_##field); return S_OK;
#define CHECK_EXREAD_QUOTED(type, config, section, field, buffer, cb) case field##: READ_QUOTED_##type##(config)(((##type##)buffer), cb, section, KEY_##field, DEF_##field); return S_OK;
#define CHECK_EXGET(type, config, section, field, result) case field##: *((##type##*)result) = GET_##type##(config)(section, KEY_##field, DEF_##field); return S_OK;
#define CHECK_EXWRITE(type, config, section, field, value) case field##: WRITE_##type##(config)(section, KEY_##field, value); return S_OK;
#define CHECK_EXWRITE_QUOTED(type, config, section, field, value) case field##: WRITE_QUOTED_##type##(config)(section, KEY_##field, value); return S_OK;

#define CHECK_DEFAULT(type, field, result) case field##: *((##type##*)result) = DEF_##field; return S_OK;
#define CHECK_READ(type, config, field, buffer, cb) CHECK_EXREAD(type, config, SECTION_DEFAULT, field, buffer, cb)
#define CHECK_READ_QUOTED(type, config, field, buffer, cb) CHECK_EXREAD_QUOTED(type, config, SECTION_DEFAULT, field, buffer, cb)
#define CHECK_GET(type, config, field, result) CHECK_EXGET(type, config, SECTION_DEFAULT, field, result)
#define CHECK_WRITE(type, config, field, value) CHECK_EXWRITE(type, config, SECTION_DEFAULT, field, value)
#define CHECK_WRITE_QUOTED(type, config, field, value) CHECK_EXWRITE_QUOTED(type, config, SECTION_DEFAULT, field, value)

#define CHECK_EXREAD_VIEW(type, section, field, buffer, cb) CHECK_EXREAD(type, VIEW, section, field, buffer, cb)
#define CHECK_EXGET_VIEW(type, section, field, result) CHECK_EXGET(type, VIEW, section, field, result)
#define CHECK_EXWRITE_VIEW(type, section, field, value) CHECK_EXWRITE(type, VIEW, section, field, value)

#define CHECK_READ_GLOBAL(type, field, buffer, cb) CHECK_READ(type, GLOBAL, field, buffer, cb)
#define CHECK_READ_QUOTED_GLOBAL(type, field, buffer, cb) CHECK_READ_QUOTED(type, GLOBAL, field, buffer, cb)
#define CHECK_GET_GLOBAL(type, field, result) CHECK_GET(type, GLOBAL, field, result)
#define CHECK_READ_VIEW(type, field, buffer, cb) CHECK_READ(type, VIEW, field, buffer, cb)
#define CHECK_GET_VIEW(type, field, result) CHECK_GET(type, VIEW, field, result)
#define CHECK_WRITE_GLOBAL(type, field, value) CHECK_WRITE(type, GLOBAL, field, value)
#define CHECK_WRITE_QUOTED_GLOBAL(type, field, value) CHECK_WRITE_QUOTED(type, GLOBAL, field, value)
#define CHECK_WRITE_VIEW(type, field, value) CHECK_WRITE(type, VIEW, field, value)

#define STR_CHECK_DEFAULT(field, result) CHECK_DEFAULT(LPCTSTR, field, result)
#define INT_CHECK_DEFAULT(field, result) CHECK_DEFAULT(INT, field, result)
#define BOOL_CHECK_DEFAULT(field, result) CHECK_DEFAULT(BOOL, field, result)

#define STR_CHECK_READ_GLOBAL(field, buffer, cb) CHECK_READ_GLOBAL(LPTSTR, field, buffer, cb)
#define STR_CHECK_READ_QUOTED_GLOBAL(field, buffer, cb) CHECK_READ_QUOTED_GLOBAL(LPTSTR, field, buffer, cb)
#define INT_CHECK_GET_GLOBAL(field, result) CHECK_GET_GLOBAL(INT, field, result)
#define BOOL_CHECK_GET_GLOBAL(field, result) CHECK_GET_GLOBAL(BOOL, field, result)

#define STR_CHECK_WRITE_GLOBAL(field, value) CHECK_WRITE_GLOBAL(LPCTSTR, field, value)
#define STR_CHECK_WRITE_QUOTED_GLOBAL(field, value) CHECK_WRITE_QUOTED_GLOBAL(LPCTSTR, field, value)
#define INT_CHECK_WRITE_GLOBAL(field, value) CHECK_WRITE_GLOBAL(INT, field, value)
#define BOOL_CHECK_WRITE_GLOBAL(field, value) CHECK_WRITE_GLOBAL(BOOL, field, value)

#define STR_CHECK_EXREAD_VIEW(section, field, buffer, cb) CHECK_EXREAD_VIEW(LPTSTR, section, field, buffer, cb)
#define INT_CHECK_EXGET_VIEW(section, field, result) CHECK_EXGET_VIEW(INT, section, field, result)
#define BOOL_CHECK_EXGET_VIEW(section, field, result) CHECK_EXGET_VIEW(BOOL, section, field, result)

#define STR_CHECK_EXWRITE_VIEW(section, field, value) CHECK_EXWRITE_VIEW(LPCTSTR, section, field, value)
#define INT_CHECK_EXWRITE_VIEW(section, field, value) CHECK_EXWRITE_VIEW(INT, section, field, value)
#define BOOL_CHECK_EXWRITE_VIEW(section, field, value) CHECK_EXWRITE_VIEW(BOOL, section, field, value)

#define STR_CHECK_READ_VIEW(field, buffer, cb) CHECK_READ_VIEW(LPTSTR, field, buffer, cb)
#define INT_CHECK_GET_VIEW(field, result) CHECK_GET_VIEW(INT, field, result)
#define BOOL_CHECK_GET_VIEW(field, result) CHECK_GET_VIEW(BOOL, field, result)

#define STR_CHECK_WRITE_VIEW(field, value) CHECK_WRITE_VIEW(LPCTSTR, field, value)
#define INT_CHECK_WRITE_VIEW(field, value) CHECK_WRITE_VIEW(INT, field, value)
#define BOOL_CHECK_WRITE_VIEW(field, value) CHECK_WRITE_VIEW(BOOL, field, value)

static LPCWSTR GetDefaultExtractPath()
{
	static TCHAR m_def_extract_path[MAX_PATH]	= { TEXT('\0'), };

	if (L'\0' == m_def_extract_path[0])
	{
		if(FAILED(SHGetFolderPath(NULL, CSIDL_MYMUSIC, NULL, SHGFP_TYPE_CURRENT, m_def_extract_path)))
		{
			if(FAILED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, m_def_extract_path)))
			{
				// and if that all fails then do a reasonable default
				StringCchCopyW(m_def_extract_path, ARRAYSIZE(m_def_extract_path), L"C:\\My Music");
			}
			// if there's no valid My Music folder (typically win2k) then default to %my_documents%\my music
			else PathCombineW(m_def_extract_path, m_def_extract_path, L"My Music");
		}
	}
	return m_def_extract_path;
}

HRESULT Settings_GetDefault(INT categoryId, INT fieldId, VOID *pValue)
{
	switch(categoryId)
	{
		case C_EXTRACT:
			switch(fieldId)
			{
				STR_CHECK_DEFAULT(EF_PATH, pValue);
				STR_CHECK_DEFAULT(EF_TITLEFMT, pValue);
				STR_CHECK_DEFAULT(EF_PLAYLISTFMT, pValue);
				STR_CHECK_DEFAULT(EF_COMMENTTEXT, pValue);
				BOOL_CHECK_DEFAULT(EF_UPPEREXTENSION, pValue);
				BOOL_CHECK_DEFAULT(EF_ADDMETADATA, pValue);
				BOOL_CHECK_DEFAULT(EF_CALCULATERG, pValue);
				BOOL_CHECK_DEFAULT(EF_USETOTALTRACKS, pValue);
				BOOL_CHECK_DEFAULT(EF_ADDTOMLDB, pValue);
				INT_CHECK_DEFAULT(EF_TRACKOFFSET, pValue);
				BOOL_CHECK_DEFAULT(EF_CREATEM3U, pValue);
				BOOL_CHECK_DEFAULT(EF_CREATEPLS, pValue);
				BOOL_CHECK_DEFAULT(EF_CREATEMLPL, pValue);
				BOOL_CHECK_DEFAULT(EF_USEM3UEXT, pValue);
				INT_CHECK_DEFAULT(EF_FOURCC, pValue);
			}
			break;
		case C_COPY:
			switch(fieldId)
			{
				STR_CHECK_DEFAULT(CF_PATH, pValue);
				STR_CHECK_DEFAULT(CF_TITLEFMT, pValue);
				BOOL_CHECK_DEFAULT(CF_USETITLEFMT, pValue);
				BOOL_CHECK_DEFAULT(CF_ADDTOMLDB, pValue);
				BOOL_CHECK_DEFAULT(CF_CALCULATERG, pValue);
				INT_CHECK_DEFAULT(CF_GAINMODE, pValue);
			}
			break;
		case C_GLOBAL:
			switch(fieldId)
			{
				BOOL_CHECK_DEFAULT(GF_SHOWINFO, pValue);
				BOOL_CHECK_DEFAULT(GF_SHOWPARENT, pValue);
				BOOL_CHECK_DEFAULT(GF_ENQUEUEBYDEFAULT, pValue);
			}
			break;
		case C_DATAVIEW:
			switch(fieldId)
			{
				STR_CHECK_DEFAULT(DVF_COLUMNLIST, pValue);
				STR_CHECK_DEFAULT(DVF_LASTFOLDER, pValue);
				INT_CHECK_DEFAULT(DVF_ORDERBY, pValue);
				BOOL_CHECK_DEFAULT(DVF_ORDERASC, pValue);
				INT_CHECK_DEFAULT(DVF_VIEWMODE, pValue);
				BOOL_CHECK_DEFAULT(DVF_SHOWAUDIO, pValue);
				BOOL_CHECK_DEFAULT(DVF_SHOWVIDEO, pValue);
				BOOL_CHECK_DEFAULT(DVF_SHOWPLAYLIST, pValue);
				BOOL_CHECK_DEFAULT(DVF_SHOWUNKNOWN, pValue);
				BOOL_CHECK_DEFAULT(DVF_HIDEEXTENSION, pValue);
				BOOL_CHECK_DEFAULT(DVF_IGNOREHIDDEN, pValue);
				INT_CHECK_DEFAULT(DVF_DIVIDERPOS, pValue);
			}
			break;
	}
	return E_INVALIDARG;
}


HRESULT Settings_ReadValue(INT categoryId, INT fieldId, VOID *pValue, INT cbSize)
{
	LPCTSTR pszSection;
	switch(categoryId)
	{
		case C_EXTRACT:
			switch(fieldId)
			{
				STR_CHECK_READ_GLOBAL(EF_PATH, pValue, cbSize);
				STR_CHECK_READ_QUOTED_GLOBAL(EF_TITLEFMT, pValue, cbSize);
				STR_CHECK_READ_QUOTED_GLOBAL(EF_PLAYLISTFMT, pValue, cbSize);
				STR_CHECK_READ_QUOTED_GLOBAL(EF_COMMENTTEXT, pValue, cbSize);
				BOOL_CHECK_GET_GLOBAL(EF_UPPEREXTENSION, pValue);
				BOOL_CHECK_GET_GLOBAL(EF_ADDMETADATA, pValue);
				BOOL_CHECK_GET_GLOBAL(EF_CALCULATERG, pValue);
				BOOL_CHECK_GET_GLOBAL(EF_USETOTALTRACKS, pValue);
				BOOL_CHECK_GET_GLOBAL(EF_ADDTOMLDB, pValue);
				INT_CHECK_GET_GLOBAL(EF_TRACKOFFSET, pValue);
				BOOL_CHECK_GET_GLOBAL(EF_CREATEM3U, pValue);
				BOOL_CHECK_GET_GLOBAL(EF_CREATEPLS, pValue);
				BOOL_CHECK_GET_GLOBAL(EF_CREATEMLPL, pValue);
				BOOL_CHECK_GET_GLOBAL(EF_USEM3UEXT, pValue);
				INT_CHECK_GET_GLOBAL(EF_FOURCC, pValue);
			}
			break;
		case C_COPY:
			switch(fieldId)
			{
				STR_CHECK_READ_GLOBAL(CF_PATH, pValue, cbSize);
				STR_CHECK_READ_QUOTED_GLOBAL(CF_TITLEFMT, pValue, cbSize);
				BOOL_CHECK_GET_GLOBAL(CF_USETITLEFMT, pValue);
				BOOL_CHECK_GET_GLOBAL(CF_ADDTOMLDB, pValue);
				BOOL_CHECK_GET_GLOBAL(CF_CALCULATERG, pValue);
				INT_CHECK_GET_GLOBAL(CF_GAINMODE, pValue);
			}
			break;
		case C_GLOBAL:
			switch(fieldId)
			{
				BOOL_CHECK_GET_VIEW(GF_SHOWINFO, pValue);
				BOOL_CHECK_GET_VIEW(GF_SHOWPARENT, pValue);
				BOOL_CHECK_GET_GLOBAL(GF_ENQUEUEBYDEFAULT, pValue);
			}
			break;
		case C_DATAVIEW:
			pszSection = SECTION_DATAVIEW;
			switch(fieldId)
			{
				STR_CHECK_EXREAD_VIEW(pszSection, DVF_COLUMNLIST, pValue, cbSize);
				STR_CHECK_EXREAD_VIEW(pszSection, DVF_LASTFOLDER, pValue, cbSize);
				INT_CHECK_EXGET_VIEW(pszSection, DVF_ORDERBY, pValue);
				BOOL_CHECK_EXGET_VIEW(pszSection, DVF_ORDERASC, pValue);
				INT_CHECK_EXGET_VIEW(pszSection, DVF_VIEWMODE, pValue);
				BOOL_CHECK_EXGET_VIEW(pszSection, DVF_SHOWAUDIO, pValue);
				BOOL_CHECK_EXGET_VIEW(pszSection, DVF_SHOWVIDEO, pValue);
				BOOL_CHECK_EXGET_VIEW(pszSection, DVF_SHOWPLAYLIST, pValue);
				BOOL_CHECK_EXGET_VIEW(pszSection, DVF_SHOWUNKNOWN, pValue);
				BOOL_CHECK_EXGET_VIEW(pszSection, DVF_HIDEEXTENSION, pValue);
				BOOL_CHECK_EXGET_VIEW(pszSection, DVF_IGNOREHIDDEN, pValue);
				INT_CHECK_EXGET_VIEW(pszSection, DVF_DIVIDERPOS, pValue);
			}
			break;
	}
	return E_INVALIDARG;
}

HRESULT Settings_SetString(INT categoryId, INT fieldId, LPCWSTR pszBuffer)
{
	LPCTSTR pszSection;
	switch(categoryId)
	{
		case C_EXTRACT:
			switch(fieldId)
			{
				STR_CHECK_WRITE_QUOTED_GLOBAL(EF_PATH, pszBuffer);
				STR_CHECK_WRITE_QUOTED_GLOBAL(EF_TITLEFMT, pszBuffer);
				STR_CHECK_WRITE_QUOTED_GLOBAL(EF_PLAYLISTFMT, pszBuffer);
				STR_CHECK_WRITE_QUOTED_GLOBAL(EF_COMMENTTEXT, pszBuffer);
				STR_CHECK_WRITE_GLOBAL(EF_UPPEREXTENSION, pszBuffer);
				STR_CHECK_WRITE_GLOBAL(EF_ADDMETADATA, pszBuffer);
				STR_CHECK_WRITE_GLOBAL(EF_CALCULATERG, pszBuffer);
				STR_CHECK_WRITE_GLOBAL(EF_USETOTALTRACKS, pszBuffer);
				STR_CHECK_WRITE_GLOBAL(EF_ADDTOMLDB, pszBuffer);
				STR_CHECK_WRITE_GLOBAL(EF_TRACKOFFSET, pszBuffer);
				STR_CHECK_WRITE_GLOBAL(EF_CREATEM3U, pszBuffer);
				STR_CHECK_WRITE_GLOBAL(EF_CREATEPLS, pszBuffer);
				STR_CHECK_WRITE_GLOBAL(EF_CREATEMLPL, pszBuffer);
				STR_CHECK_WRITE_GLOBAL(EF_USEM3UEXT, pszBuffer);
				STR_CHECK_WRITE_GLOBAL(EF_FOURCC, pszBuffer);
			}
			break;
		case C_COPY:
			switch(fieldId)
			{
				STR_CHECK_WRITE_QUOTED_GLOBAL(CF_PATH, pszBuffer);
				STR_CHECK_WRITE_QUOTED_GLOBAL(CF_TITLEFMT, pszBuffer);
				STR_CHECK_WRITE_GLOBAL(CF_USETITLEFMT, pszBuffer);
				STR_CHECK_WRITE_GLOBAL(CF_ADDTOMLDB, pszBuffer);
				STR_CHECK_WRITE_GLOBAL(CF_CALCULATERG, pszBuffer);
				STR_CHECK_WRITE_GLOBAL(CF_GAINMODE, pszBuffer);
			}
			break;
		case C_GLOBAL:
			switch(fieldId)
			{
				STR_CHECK_WRITE_VIEW(GF_SHOWINFO, pszBuffer);
				STR_CHECK_WRITE_VIEW(GF_SHOWPARENT, pszBuffer);
				STR_CHECK_WRITE_VIEW(GF_ENQUEUEBYDEFAULT, pszBuffer);
			}
			break;
		case C_DATAVIEW:
			pszSection = SECTION_DATAVIEW;
			switch(fieldId)
			{
				STR_CHECK_EXWRITE_VIEW(pszSection, DVF_COLUMNLIST, pszBuffer);
				STR_CHECK_EXWRITE_VIEW(pszSection, DVF_LASTFOLDER, pszBuffer);
				STR_CHECK_EXWRITE_VIEW(pszSection, DVF_ORDERBY, pszBuffer);
				STR_CHECK_EXWRITE_VIEW(pszSection, DVF_ORDERASC, pszBuffer);
				STR_CHECK_EXWRITE_VIEW(pszSection, DVF_VIEWMODE, pszBuffer);
				STR_CHECK_EXWRITE_VIEW(pszSection, DVF_SHOWAUDIO, pszBuffer);
				STR_CHECK_EXWRITE_VIEW(pszSection, DVF_SHOWVIDEO, pszBuffer);
				STR_CHECK_EXWRITE_VIEW(pszSection, DVF_SHOWPLAYLIST, pszBuffer);
				STR_CHECK_EXWRITE_VIEW(pszSection, DVF_SHOWUNKNOWN, pszBuffer);
				STR_CHECK_EXWRITE_VIEW(pszSection, DVF_HIDEEXTENSION, pszBuffer);
				STR_CHECK_EXWRITE_VIEW(pszSection, DVF_IGNOREHIDDEN, pszBuffer);
				STR_CHECK_EXWRITE_VIEW(pszSection, DVF_DIVIDERPOS, pszBuffer);
			}
			break;
	}
	return E_INVALIDARG;
}


HRESULT Settings_GetInt(INT categoryId, INT fieldId, INT *pnVal)
{
	return Settings_ReadValue(categoryId, fieldId, pnVal, sizeof(INT));
}
HRESULT Settings_GetBool(INT categoryId, INT fieldId, BOOL *pbVal)
{
	return Settings_ReadValue(categoryId, fieldId, pbVal, sizeof(BOOL));
}

HRESULT Settings_ReadString(INT categoryId, INT fieldId, LPTSTR pszBuffer, INT cchBufferMax)
{
	return Settings_ReadValue(categoryId, fieldId, pszBuffer, cchBufferMax * sizeof(TCHAR));
}

HRESULT Settings_SetWindowText(INT categoryId, INT fieldId, HWND hwnd)
{
	TCHAR szBuffer[8192] = {0};
	HRESULT hr = Settings_ReadValue(categoryId, fieldId, szBuffer, sizeof(szBuffer));
	if (S_OK == hr) SetWindowText(hwnd, szBuffer);
	return hr;
}

HRESULT Settings_SetWindowInt(INT categoryId, INT fieldId, HWND hwnd)
{
	INT nVal;
	HRESULT hr = Settings_ReadValue(categoryId, fieldId, &nVal, sizeof(nVal));
	
	if (S_OK == hr)
	{
		TCHAR szBuffer[256] = {0};
		hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("%d"), nVal);
		if (S_OK == hr) SetWindowText(hwnd, szBuffer);
	}
	return hr;
}
HRESULT Settings_SetDlgItemText(INT categoryId, INT fieldId, HWND hdlg, INT nItemId)
{
	HWND hctrl = GetDlgItem(hdlg, nItemId);
	return (hctrl) ? Settings_SetWindowText(categoryId, fieldId, hctrl) : E_HANDLE;
}

HRESULT Settings_SetDlgItemInt(INT categoryId, INT fieldId, HWND hdlg, INT nItemId)
{
	HWND hctrl = GetDlgItem(hdlg, nItemId);
	return (hctrl) ? Settings_SetWindowInt(categoryId, fieldId, hctrl) : E_HANDLE;
}

HRESULT Settings_SetCheckBox(INT categoryId, INT fieldId, HWND hdlg, INT nItemId)
{
	INT nVal;
	HRESULT hr = Settings_GetInt(categoryId, fieldId, &nVal);
	if (S_OK == hr) hr = (CheckDlgButton(hdlg, nItemId, (0 != nVal) ? BST_CHECKED : BST_UNCHECKED)) ? S_OK : E_HANDLE;
	return hr;
}

HRESULT Settings_SetInt(INT categoryId, INT fieldId, INT nValue)
{
	TCHAR szBuffer[64] = {0};
	HRESULT hr = StringCchPrintfW(szBuffer, ARRAYSIZE(szBuffer), TEXT("%d"), nValue);
	if (S_OK == hr) hr = Settings_SetString(categoryId, fieldId, szBuffer);
	return hr;	
}
HRESULT Settings_SetBool(INT categoryId, INT fieldId, BOOL bValue)
{
	return Settings_SetInt(categoryId, fieldId, (0 != bValue));
}
HRESULT Settings_FromWindowText(INT categoryId, INT fieldId, HWND hwnd)
{
	if (!hwnd) return E_HANDLE;
	INT l = GetWindowTextLength(hwnd);
	if (l < 1024)
	{
		TCHAR szBuffer[1024] = {0};
		GetWindowText(hwnd, szBuffer, ARRAYSIZE(szBuffer));
		return Settings_SetString(categoryId, fieldId, szBuffer);
	}
	else
	{
		LPTSTR pszBuffer = (LPTSTR)calloc((l + 1), sizeof(TCHAR));
		if (!pszBuffer) return E_OUTOFMEMORY;
		GetWindowText(hwnd, pszBuffer, sizeof(TCHAR)*(l + 1));
		HRESULT hr = Settings_SetString(categoryId, fieldId, pszBuffer);
		free(pszBuffer);
		return hr;
	}
}
HRESULT Settings_FromDlgItemText(INT categoryId, INT fieldId, HWND hdlg, INT nItemId)
{
	HWND hctrl = GetDlgItem(hdlg, nItemId);
	return (hctrl) ? Settings_FromWindowText(categoryId, fieldId, hctrl) : E_HANDLE;
}

HRESULT Settings_FromCheckBox(INT categoryId, INT fieldId, HWND hdlg, INT nItemId)
{
	UINT v = IsDlgButtonChecked(hdlg, nItemId);
	return Settings_SetInt(categoryId, fieldId, v);
}

typedef struct _FBROWSER
{
	LPCWSTR pszSelection;
} FBROWSER;

BOOL CALLBACK browseEnumProc(HWND hwnd, LPARAM lParam)
{
	wchar_t cl[32] = {0};
	GetClassNameW(hwnd, cl, ARRAYSIZE(cl));
	if (!lstrcmpiW(cl, WC_TREEVIEW))
	{
		PostMessage(hwnd, TVM_ENSUREVISIBLE, 0, (LPARAM)TreeView_GetSelection(hwnd));
		return FALSE;
	}

	return TRUE;
}

int CALLBACK WINAPI BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM pData)
{
	FBROWSER *pfb = (FBROWSER*)pData;
	switch(uMsg)
	{
		case BFFM_INITIALIZED:
			if (pfb && pfb->pszSelection) SendMessageW(hwnd, BFFM_SETSELECTIONW, TRUE, (LPARAM)pfb->pszSelection);

			// this is not nice but it fixes the selection not working correctly on all OSes
			EnumChildWindows(hwnd, browseEnumProc, 0);
			break;
	}
	return 0;
}

HRESULT Settings_BrowseForFolder(INT categoryId, INT fieldId, HWND hDlg, INT nEditId)
{
	BROWSEINFO bi = {0};
	FBROWSER fb;
	wchar_t szPath[MAX_PATH] = {0};

	HRESULT hr = Settings_ReadString(categoryId,fieldId, szPath, ARRAYSIZE(szPath));
	if (S_OK != hr) return hr;

	fb.pszSelection = szPath;

	bi.hwndOwner = hDlg;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = szPath;
	bi.lpszTitle = WASABI_API_LNGSTRINGW(IDS_CHOOSE_A_FOLDER);
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)&fb;
	HRESULT hrCom = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(hrCom))
	{
		ITEMIDLIST *idlist = SHBrowseForFolder(&bi);
		if (idlist)
		{
			if (SHGetPathFromIDList(idlist, szPath)) 
			{
				hr = Settings_SetString(categoryId,fieldId, szPath);
				if (S_OK == hr) SetDlgItemText(hDlg, nEditId, szPath);
			}
			else hr = S_FALSE;
			CoTaskMemFree(idlist);
		}
	}
	else hr = hrCom;
	if (S_OK == hrCom) CoUninitialize();
	return hr;
}

HRESULT Settings_SetDirectoryCtrl(INT categoryId, INT fieldId, HWND hdlg, INT nItemId)
{
	TCHAR szBuffer[MAX_PATH] = {0};
	HRESULT hr;
	hr = Settings_ReadString(categoryId, fieldId, szBuffer, ARRAYSIZE(szBuffer));
	if (S_OK != hr) *szBuffer = TEXT('\0');
	else CleanupDirectoryString(szBuffer);
	SetDlgItemText(hdlg, nItemId, szBuffer);
	return hr;
}
HRESULT Settings_FromDirectoryCtrl(INT categoryId, INT fieldId, HWND hdlg, INT nItemId)
{
	TCHAR szBuffer[MAX_PATH] = {0};
	GetDlgItemText(hdlg, nItemId, szBuffer, ARRAYSIZE(szBuffer));
	CleanupDirectoryString(szBuffer);
	return Settings_SetString(categoryId, fieldId, szBuffer);
}


