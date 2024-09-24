#include "./fileview.h"
#include "./fileview_internal.h"
#include "./resource.h"
#include <strsafe.h>

INT FileView_FormatFileTime(FILETIME *pft, LPWSTR pszDest, INT cchDest)
{
	SYSTEMTIME st;
	
	if (!pszDest) return 0;
	
	
	pszDest[0] = 0x00;
	if (!pft || (0 == pft->dwHighDateTime && 0 == pft->dwLowDateTime)) return 0;
	if (FileTimeToSystemTime(pft, &st))
	{	
		INT len;
		len = GetDateFormatW(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, pszDest, cchDest);
		if (0 == len) return 0;
		cchDest -= len;
		if (cchDest > 0)
		{
 			pszDest += len;
			*(pszDest - 1) = L' ';
				
			INT len2;
			len2 = GetTimeFormatW(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, pszDest, cchDest);
			if (0 == len2) return 0;
			len += len2;
			return len - 1;
		}
		return len;
	}
	return 0;
}

INT FileView_FormatType(UINT fileType, LPWSTR pszDest, INT cchDest)
{
	if (!pszDest) return 0;
	pszDest[0] = 0x00;

	INT strId;
	switch(fileType)
	{
		case FVFT_AUDIO:			strId = IDS_FILETYPE_AUDIO; break;
		case FVFT_VIDEO:			strId = IDS_FILETYPE_VIDEO; break;
		case FVFT_PLAYLIST:			strId = IDS_FILETYPE_PLAYLIST; break;
		default:					strId = IDS_FILETYPE_UNKNOWN; break;
	}

	WASABI_API_LNGSTRINGW_BUF(strId, pszDest, cchDest);
	return lstrlenW(pszDest);
}

INT FileView_FormatAttributes(UINT uAttributes, LPWSTR pszDest, INT cchDest)
{
	if (!pszDest) return 0;
	pszDest[0] = 0x00;

	wchar_t szAttrib[32] = {0};
	if(!WASABI_API_LNGSTRINGW_BUF(IDS_FILE_ATTRIBUTES, szAttrib, sizeof(szAttrib)/sizeof(szAttrib[0])))
		szAttrib[0] = L'\0';

	INT len = 0;
	if (len < cchDest && (FILE_ATTRIBUTE_READONLY & uAttributes)) { pszDest[len] = szAttrib[0]; len++; }
	if (len < cchDest && (FILE_ATTRIBUTE_HIDDEN & uAttributes)) { pszDest[len] = szAttrib[1]; len++; }
	if (len < cchDest && (FILE_ATTRIBUTE_SYSTEM & uAttributes)) { pszDest[len] = szAttrib[2]; len++; }
	if (len < cchDest && (FILE_ATTRIBUTE_ARCHIVE & uAttributes)) { pszDest[len] = szAttrib[3]; len++; }
	if (len < cchDest && (FILE_ATTRIBUTE_COMPRESSED & uAttributes)) { pszDest[len] = szAttrib[4]; len++; }
	if (len < cchDest && (FILE_ATTRIBUTE_ENCRYPTED & uAttributes)) { pszDest[len] = szAttrib[5]; len++; }
	if (len < cchDest) pszDest[len] = 0x00;
	return len;
}


INT FileView_FormatYesNo(BOOL bValue, LPWSTR pszDest, INT cchDest)
{
	if (!pszDest) return 0;
	pszDest[0] = 0x00;

	WASABI_API_LNGSTRINGW_BUF(((bValue) ? IDS_YES : IDS_NO), pszDest, cchDest);
	return lstrlenW(pszDest);
}

INT FileView_FormatYear(INT nYear, LPWSTR pszDest, INT cchDest)
{	
	if (nYear < 1 || S_OK != StringCchPrintfW(pszDest, cchDest, L"%d", nYear)) *pszDest = L'\0';
	return lstrlenW(pszDest);
}
INT FileView_FormatBitrate(INT nBitrate, LPWSTR pszDest, INT cchDest)
{	
	if (nBitrate < 1 || S_OK != StringCchPrintfW(pszDest, cchDest, L"%d%s", nBitrate, WASABI_API_LNGSTRINGW(IDS_KBPS))) *pszDest = L'\0';
	return lstrlenW(pszDest);
}

INT FileView_FormatLength(INT nLength, LPWSTR pszDest, INT cchDest)
{	
	if (nLength < 1 || S_OK != StringCchPrintfW(pszDest, cchDest, L"%d:%02d:%02d", 
			nLength / (60 * 60), (nLength % (60 * 60)) / 60, nLength % 60)) *pszDest = L'\0';
	return lstrlenW(pszDest);
}

INT FileView_FormatIntSlashInt(INT part1, INT part2, LPWSTR pszDest, INT cchDest)
{	
	if (part1 > 0)
	{
		size_t remaining = cchDest;
		HRESULT hr = (part2 > 0) ? StringCchPrintfExW(pszDest, cchDest, NULL, &remaining, STRSAFE_IGNORE_NULLS,  L"%d/%d", part1, part2) :
									StringCchPrintfExW(pszDest, cchDest, NULL, &remaining, STRSAFE_IGNORE_NULLS,  L"%d", part1);
		if (S_OK != hr) *pszDest = L'\0';
		if (remaining != (size_t)cchDest) remaining;
		return (S_OK == hr) ? (cchDest - (INT)remaining) : 0;
	}
	else 
	{
		*pszDest = L'\0';
		return 0;
	}
}

static void FileView_FormatPlaylistTip(FILERECORD *pfr, LPWSTR pszText, size_t cchTextMax, LPCWSTR pszSeparator)
{
	INT nLines = 0;
	if (!pfr->pMeta || METATYPE_PLAYLIST != pfr->pMeta->type) return;

	PLAYLISTMETA *plm = &pfr->pMeta->playlist;

	if (plm->pszTitle && *plm->pszTitle && cchTextMax > 2)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
							L"%s:  %s", WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_TITLE), plm->pszTitle)) return;
		nLines++;
	}

	LPCWSTR pszExt = FileView_GetTypeFamily(pfr->Info.cFileName + pfr->extOffset);
	if (pszExt && *pszExt)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
					L"%s%s:  %s", ((!nLines) ? L"" : pszSeparator), WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_TYPE), pszExt)) return;
		nLines++;
	}

	
	if (plm->nLength > 0 && cchTextMax > 2)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
							L"%s%s:  ", ((!nLines) ? L"" : pszSeparator), WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_TOTALLENGTH))) return;
		size_t len = FileView_FormatLength(plm->nLength, pszText, (INT)cchTextMax);
		pszText += len;
		cchTextMax -= (len + 1);
		nLines++;
	}
	
	if (cchTextMax > 2)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
			L"%s%s:  %d", ((!nLines) ? L"" : pszSeparator), WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_ENTRYCOUNT), plm->nCount)) return;
		nLines++;
	}


	for (UINT i = 0; i < sizeof(plm->szEntries)/sizeof(plm->szEntries[0]) && i < plm->nCount; i++) 
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
							L"%s%02d. %s", ((!nLines) ? L"" : pszSeparator), (i + 1), plm->szEntries[i].pszTitle)) return;
		nLines++;
	}
	if (plm->nCount > sizeof(plm->szEntries)/sizeof(plm->szEntries[0]))
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
							L"%s>>>", ((!nLines) ? L"" : pszSeparator))) return;
		nLines++;
	}
}

static void FileView_FormatAudioTip(FILERECORD *pfr, LPWSTR pszText, size_t cchTextMax, LPCWSTR pszSeparator)
{
	size_t len;
	INT nLines = 0;
	if (!pfr->pMeta || METATYPE_AUDIO != pfr->pMeta->type) return;

	AUDIOMETA *pam = &pfr->pMeta->audio;

	if (pam->pszArtist && *pam->pszArtist && cchTextMax > 2)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
							L"%s: %s", WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_ARTIST), pam->pszArtist)) return;
		nLines++;
	}

	if (pam->pszAlbum && *pam->pszAlbum && cchTextMax > 2)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
			L"%s%s: %s", ((!nLines) ? L"" : pszSeparator), WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_ALBUM), pam->pszAlbum)) return;
		nLines++;
	}

	if (pam->pszTitle && *pam->pszTitle && cchTextMax > 2)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
						L"%s%s: %s", ((!nLines) ? L"" : pszSeparator), WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_TITLE), pam->pszTitle)) return;
		nLines++;
	}

	if (pam->nYear > 0 && cchTextMax > 2)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
						L"%s%s: %d", ((!nLines) ? L"" : pszSeparator), WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_YEAR), pam->nYear)) return;
		nLines++;
	}

	if (pam->pszGenre && *pam->pszGenre && cchTextMax > 2)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
							L"%s%s: %s", ((!nLines) ? L"" : pszSeparator), WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_GENRE), pam->pszGenre)) return;
		nLines++;
	}

	if (pam->nLength > 0 && cchTextMax > 2)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
							L"%s%s: ", ((!nLines) ? L"" : pszSeparator), WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_LENGTH))) return;
		len = FileView_FormatLength(pam->nLength, pszText, (INT)cchTextMax);
		pszText += len;
		cchTextMax -= (len + 1);
		nLines++;
	}

	if (pam->nTrackNum > 0 && cchTextMax > 2)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
							L"%s%s: ", ((!nLines) ? L"" : pszSeparator), WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_TRACK))) return;
		len = FileView_FormatIntSlashInt(pam->nTrackNum, pam->nTrackCount, pszText, (INT)cchTextMax);
		pszText += len;
		cchTextMax -= (len + 1);
		nLines++;
	}

	if (pam->nDiscNum > 0 && cchTextMax > 2)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
							L"%s%s: ", ((!nLines) ? L"" : pszSeparator), WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_DISC))) return;
		len = FileView_FormatIntSlashInt(pam->nDiscNum, pam->nDiscCount, pszText, (INT)cchTextMax);
		pszText += len;
		cchTextMax -= (len + 1);
		nLines++;
	}

	if (pam->nBitrate > 0 && cchTextMax > 2)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
							L"%s%s: ", ((!nLines) ? L"" : pszSeparator), WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_BITRATE))) return;
		len = FileView_FormatBitrate(pam->nBitrate, pszText, (INT)cchTextMax);
		pszText += len;
		cchTextMax -= (len + 1);
		nLines++;
	}

	LPCWSTR pszExt = FileView_GetTypeFamily(pfr->Info.cFileName + pfr->extOffset);
	if (pszExt && *pszExt)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
							L"%s%s: %s", ((!nLines) ? L"" : pszSeparator), WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_TYPE), pszExt)) return;
		nLines++;
	}

	if (pam->nSource != METADATA_SOURCE_UNKNOWN && cchTextMax > 2)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
							L"%s%s: ", ((!nLines) ? L"" : pszSeparator), WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_INMLDB))) return;

		len = FileView_FormatYesNo(METADATA_SOURCE_MLDB == pam->nSource, pszText, (INT)cchTextMax); 
		pszText += len;
		cchTextMax -= (len + 1);
		nLines++;
	}

	if (cchTextMax > 2)
	{
		if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
							L"%s%s: ", ((!nLines) ? L"" : pszSeparator), WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_SIZE))) return;

		WASABI_API_LNG->FormattedSizeString(pszText, (INT)cchTextMax, (((__int64)pfr->Info.nFileSizeHigh << 32) | pfr->Info.nFileSizeLow));
		len = lstrlenW(pszText);
		pszText += len;
		cchTextMax -= (len + 1);
		nLines++;
	}
}

void FileView_FormatDefaultTip(FILERECORD *pfr, LPWSTR pszText, size_t cchTextMax, LPCWSTR pszSeparator)
{
	size_t len;
	if ((len = lstrlenW(pszText)) > 0) { cchTextMax -= len; pszText += len; }
	if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
									L"%s: ", WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_TYPE))) return;
	len = FileView_FormatType(pfr->fileType, pszText, (INT)cchTextMax);
	pszText += len;
	cchTextMax -= (len + 1);

	if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
									L"%s%s: ", pszSeparator, WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_MODIFIED))) return;
	len = FileView_FormatFileTime(&pfr->Info.ftLastWriteTime, pszText, (INT)cchTextMax);
	pszText += len;
	cchTextMax -= (len + 1);

	if (S_OK != StringCchPrintfExW(pszText, cchTextMax, &pszText, &cchTextMax, STRSAFE_IGNORE_NULLS,
									L"%s%s: ", pszSeparator, WASABI_API_LNGSTRINGW(IDS_FILEVIEW_COL_SIZE))) return;
	WASABI_API_LNG->FormattedSizeString(pszText, (INT)cchTextMax, (((__int64)pfr->Info.nFileSizeHigh << 32) | pfr->Info.nFileSizeLow));
	len = lstrlenW(pszText);
	pszText += len;
	cchTextMax -= (len + 1);
}

void FileView_FormatFileInfo(FILERECORD *pfr, LPWSTR pszText, size_t cchTextMax, UINT uMode)
{
	LPCWSTR pszSeparator;
	switch(uMode)
	{
		case FIF_STATUS:	pszSeparator = L"  "; break;
		case FIF_TOOLTIP:
		default:			pszSeparator = L"\r\n"; break;
	}

	switch(pfr->fileType)
	{
		case FVFT_AUDIO:
			if (pfr->pMeta) { FileView_FormatAudioTip(pfr, pszText, cchTextMax, pszSeparator); return; }
			break;
		case FVFT_PLAYLIST:
			if (pfr->pMeta) { FileView_FormatPlaylistTip(pfr, pszText, cchTextMax, pszSeparator); return; }
			break;
	}
	FileView_FormatDefaultTip(pfr, pszText, cchTextMax, pszSeparator);
}