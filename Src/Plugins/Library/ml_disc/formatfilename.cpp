#include "main.h"
#include "../nu/ns_wc.h"
#include "../winamp/wa_ipc.h"
#include <shlwapi.h>
#include <strsafe.h>

#define TID_UNKNOWN			0
#define TID_ARTIST			1
#define TID_ALBUM			2
#define TID_TITLE			3
#define TID_GENRE			4
#define TID_YEAR			5
#define TID_TRACKARTIST		6
#define TID_FILENAME		7
#define TID_EXTENSION		8
#define TID_DISC			9
#define TID_DISCS			10

#define TOKEN_ARTIST		TEXT("<artist>")
#define TOKEN_ALBUM			TEXT("<album>")
#define TOKEN_TITLE			TEXT("<title>")
#define TOKEN_GENRE			TEXT("<genre>")
#define TOKEN_YEAR			TEXT("<year>")
#define TOKEN_TRACKARTIST	TEXT("<trackartist>")
#define TOKEN_FILENAME		TEXT("<filename>")
#define TOKEN_EXTENSION		TEXT("<extension>")
#define TOKEN_DISC			TEXT("<disc>")
#define TOKEN_DISCS			TEXT("<discs>")

#define TOKEN_LEN_ARTIST		8
#define TOKEN_LEN_ALBUM			7
#define TOKEN_LEN_TITLE			7
#define TOKEN_LEN_GENRE			7
#define TOKEN_LEN_YEAR			6
#define TOKEN_LEN_TRACKARTIST	13
#define TOKEN_LEN_FILENAME		10
#define TOKEN_LEN_EXTENSION		11
#define TOKEN_LEN_DISC			6
#define TOKEN_LEN_DISCS			7

#define STRCOMP_INVARIANT		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

#define CHECK_TOKEN(token, format)	(CSTR_EQUAL == CompareString(STRCOMP_INVARIANT, NORM_IGNORECASE, TOKEN_##token##, TOKEN_LEN_##token##, format, TOKEN_LEN_##token##))


static TCHAR PathValidateChar(TCHAR cVal, BOOL bAllowBackSlash)
{
	switch(cVal)
	{
		case TEXT('\\'):	if (!bAllowBackSlash) return TEXT('-');
							break;
		case TEXT('/'):		if (!bAllowBackSlash) return TEXT('-'); 
							return TEXT('\\');

		case TEXT(':'):		return TEXT('-'); 
		case TEXT('*'):		return TEXT('_'); 
		case TEXT('?'):		return TEXT('_'); 
		case TEXT('\"'):	return TEXT('\'');
		case TEXT('<'):		return TEXT('('); 
		case TEXT('>'):		return TEXT(')'); 
		case TEXT('|'):		return TEXT('_'); 
	}
	return cVal;
}

void CleanupDirectoryString(LPTSTR pszDirectory)
{
	PathUnquoteSpaces(pszDirectory);
	PathRemoveBlanks(pszDirectory);
	
	LPTSTR pc = pszDirectory;
	while (TEXT('\0') != *pc) { if (TEXT('/') == *pc) *pc = TEXT('\\'); pc++; } 
	
	if (pc > pszDirectory)
	{
		pc--;
		while (pszDirectory != pc && 
				(TEXT('.') == *pc || TEXT(' ') == *pc || TEXT('\\') == *pc)) 
		{ 
			*pc = TEXT('\0'); 
			pc--; 
		}
	}
}

LPWSTR GetExtensionString(LPWSTR pszBuffer, INT cchBufferMax, DWORD fourcc)
{
	char configExt[10] = { '\0', };
	pszBuffer[0] = L'\0'; 
	convertConfigItem cfi;
	cfi.configfile = 0;
	cfi.data = configExt;
	cfi.format = fourcc;
	cfi.item = "extension";
	cfi.len = 10;
	SENDWAIPC(plugin.hwndWinampParent, IPC_CONVERT_CONFIG_GET_ITEM, (WPARAM)&cfi);
	if ('\0' != *configExt)
	{
		if (!MultiByteToWideCharSZ(CP_ACP, 0, configExt, -1, pszBuffer, cchBufferMax))
			return NULL;
	}
	else
	{
		if (cchBufferMax < 5) return NULL;
		pszBuffer[0] = (TCHAR)((fourcc) & 0xff);
		pszBuffer[1] = (TCHAR)((fourcc >> 8) & 0xff);
		pszBuffer[2] = (TCHAR)((fourcc >> 16) & 0xff);
		pszBuffer[3] = TEXT('\0');
		for (LPTSTR p = &pszBuffer[2]; p >= pszBuffer && TEXT(' ') == *p; p--) *p = TEXT('\0');
	}
	return pszBuffer;
}

// if trackno is 0xdeadbeef, or title is 0, they are both ignored
// TODO: use ATF instead
HRESULT FormatFileName(LPTSTR pszTextOut, INT cchTextMax, LPCTSTR pszFormat, 
					INT nTrackNo, LPCTSTR pszArtist,
					LPCTSTR pszAlbum, LPCTSTR pszTitle, 
					LPCTSTR pszGenre, LPCTSTR pszYear,
					LPCTSTR pszTrackArtist,
					LPCTSTR pszFileName, LPCTSTR pszDisc)
{
	HRESULT hr = S_OK;
	TCHAR szBuffer[MAX_PATH] = {0};
	LPTSTR pszStart = pszTextOut + lstrlen(pszTextOut);

	while (pszFormat && TEXT('\0') != *pszFormat)
	{
		int whichstr = TID_UNKNOWN;
		if (*pszFormat == TEXT('#') && nTrackNo != 0xdeadbeef)
		{
			int cnt = 0;
			while (pszFormat && *pszFormat == TEXT('#')) { pszFormat++; cnt++; }
			if (cnt > 8) cnt = 8;
			
			TCHAR szFormat[32] = {0};
			hr = StringCchPrintf(szFormat, ARRAYSIZE(szFormat), TEXT("%%%02dd"), cnt);
			if (S_OK == hr) StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), szFormat, nTrackNo);
			if (S_OK == hr) StringCchCat(pszTextOut, cchTextMax, szBuffer);
			if (S_OK != hr) return hr;
		}
		else if (pszArtist && CHECK_TOKEN(ARTIST, pszFormat)) whichstr = TID_ARTIST;
		else if (pszAlbum && CHECK_TOKEN(ALBUM, pszFormat)) whichstr = TID_ALBUM;
		else if (pszTitle && CHECK_TOKEN(TITLE, pszFormat)) whichstr = TID_TITLE;
		else if (pszGenre && CHECK_TOKEN(GENRE, pszFormat)) whichstr = TID_GENRE;
		else if (pszYear && CHECK_TOKEN(YEAR, pszFormat)) whichstr = TID_YEAR;
		else if (pszTrackArtist && CHECK_TOKEN(TRACKARTIST, pszFormat)) whichstr = TID_TRACKARTIST;
		else if (pszFileName && CHECK_TOKEN(FILENAME, pszFormat)) whichstr = TID_FILENAME;
		else if (pszFileName && CHECK_TOKEN(EXTENSION, pszFormat)) whichstr = TID_EXTENSION;
		else if (pszDisc && CHECK_TOKEN(DISC, pszFormat)) whichstr = TID_DISC;
		else if (pszDisc && CHECK_TOKEN(DISCS, pszFormat)) whichstr = TID_DISCS;
		else
		{
			INT l = lstrlen(pszTextOut);
			pszTextOut += l;
			cchTextMax -= l;
			pszTextOut[0] = *pszFormat++;
			if (cchTextMax < 2) return STRSAFE_E_INSUFFICIENT_BUFFER;
			pszTextOut[0] = PathValidateChar(pszTextOut[0], TRUE);
			if (TEXT('\\') == *pszTextOut)
			{
				// remove end spaces and dots
				while (pszTextOut > pszStart && 
						(TEXT('\\') == *(pszTextOut - 1) || TEXT(' ') == *(pszTextOut - 1) || TEXT('.') == *(pszTextOut - 1)))
				{
					pszTextOut--;
					cchTextMax--;
				}

				if (pszTextOut == pszStart)
				{
					pszTextOut--;
					cchTextMax--;
				}
				else *pszTextOut = TEXT('\\');
			}
			pszTextOut++;
			cchTextMax--;
			*pszTextOut = TEXT('\0');
			if (S_OK != hr) return hr;
		}
		if (whichstr != TID_UNKNOWN)
		{
			LPCTSTR pszSrc = NULL;
			int islow = IsCharLower(pszFormat[1]) && IsCharLower(pszFormat[2]);
			int ishi = IsCharUpper(pszFormat[1]) && IsCharUpper(pszFormat[2]);
			switch(whichstr)
			{
				case TID_ARTIST:		pszSrc = pszArtist; pszFormat += TOKEN_LEN_ARTIST; break;
				case TID_ALBUM:			pszSrc = pszAlbum; pszFormat += TOKEN_LEN_ALBUM; break;
				case TID_TITLE:			pszSrc = pszTitle; pszFormat += TOKEN_LEN_TITLE; break;
				case TID_GENRE:			pszSrc = pszGenre; pszFormat += TOKEN_LEN_GENRE; break;
				case TID_YEAR:			pszSrc = pszYear; pszFormat += TOKEN_LEN_YEAR; break;
				case TID_TRACKARTIST:	pszSrc = pszTrackArtist; pszFormat += TOKEN_LEN_TRACKARTIST; break;
				case TID_DISC:
					if (pszDisc && *pszDisc && TEXT('\0') == *pszDisc) pszSrc = L"1";
					else
					{
						// default to 1 when we've not got a proper value passed to us
						int disc = _wtoi(pszDisc);
						if(disc <= 0) disc = 1;
						StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), L"%d", disc);
						pszSrc = szBuffer;
					}
					pszFormat += TOKEN_LEN_DISC;
					break;
				case TID_DISCS:
					if (pszDisc && *pszDisc && TEXT('\0') == *pszDisc) pszSrc = L"1";
					else
					{
						LPTSTR pszTemp = wcschr((LPTSTR)pszDisc, L'/');
						if(pszTemp == NULL) pszTemp = wcschr((LPTSTR)pszDisc, L'\\');
						if(pszTemp == NULL)
						{
							pszTemp = (LPTSTR)pszDisc;
						}
						else
						{
							pszTemp = CharNext(pszTemp);
						}
						int disc = _wtoi(pszTemp);
						if(disc <= 0) disc = 1;
						StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), L"%d", disc);
						pszSrc = szBuffer;
					}
					pszFormat += TOKEN_LEN_DISCS;
					break;
				case TID_FILENAME:		
					pszSrc = PathFindExtension(pszFileName);
					if (TEXT('\0') == *pszSrc) pszSrc = pszFileName;
					else
					{
						StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), pszFileName);
						PathRemoveExtension(szBuffer);
						pszSrc = szBuffer;
					}
					pszFormat += TOKEN_LEN_FILENAME; 
					break;
				case TID_EXTENSION:		
					pszSrc = PathFindExtension(pszFileName); pszFormat += TOKEN_LEN_EXTENSION; 
					while (pszTextOut > pszStart && TEXT('.') == *(pszTextOut - 1))
					{
						pszTextOut--;
						*pszTextOut = TEXT('\0');
						cchTextMax++;
					}
					break;
			}

			INT l = lstrlen(pszTextOut);
			pszTextOut += l;
			cchTextMax -= l;

			while (pszSrc && TEXT('\0') != *pszSrc && cchTextMax > 0)
			{
				pszTextOut[0] = pszSrc[0];
				if (ishi) CharUpperBuffW(pszTextOut, 1);
				else if (islow) CharLowerBuffW(pszTextOut, 1);
				pszTextOut[0] = PathValidateChar(pszTextOut[0], FALSE);
				cchTextMax--;
				if (cchTextMax) pszTextOut++;
				pszSrc++;
			}
			pszTextOut[0] = TEXT('\0');
			if (0 == cchTextMax) return STRSAFE_E_INSUFFICIENT_BUFFER;
		}
	}
	return hr;
}