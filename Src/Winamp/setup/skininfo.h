#ifndef WINAMP_SKININFO_HEADER
#define WINAMP_SKININFO_HEADER

#include <windows.h>

#define SIF_COMMENT			0x01
#define SIF_PREVIEW			0x02

#define SKIN_TYPE_UNKNOWN	0
#define SKIN_TYPE_CLASSIC	1
#define SKIN_TYPE_MODERN		2

#define SI_NAMEMAX		32
#define SI_VERMAX		16
#define SI_AUTHORMAX		32
#define SI_EMAILMAX		32
#define SI_HOMEPAGEMAX	64

typedef struct _SKININFO
{
	INT		cbSize;				// sizeof(SKININFO)
	UINT	fMask;				// SIF_DESCRIPTION | SIF_PREVIEW
	int		type;				// classic/modern
	wchar_t szName[SI_NAMEMAX];
	wchar_t szVersion[SI_VERMAX];
	wchar_t szAuthor[SI_AUTHORMAX];
	wchar_t szEmail[SI_EMAILMAX];
	wchar_t szHomePage[SI_HOMEPAGEMAX];
	wchar_t szWasabiVer[SI_VERMAX];
    wchar_t *pszComment;
    int		cchComment;
	HBITMAP	hPreview;			// preiew bitmap;
}SKININFO;


BOOL GetSkinInfo(LPCWSTR pszSkinPath, SKININFO *psi);

#endif //WINAMP_SKININFO_HEADER