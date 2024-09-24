// GetWinVer.h  Version 1.1
//
// Copyright (C) 2001-2003 Hans Dietrich
//
// This software is released into the public domain.  
// You are free to use it in any way you like, except
// that you may not sell this source code.
//
// This software is provided "as is" with no expressed 
// or implied warranty.  I accept no liability for any 
// damage or loss of business that this software may cause. 
//
///////////////////////////////////////////////////////////////////////////////
#include <windows.h>

#ifndef GETWINVER_H
#define GETWINVER_H

#define WUNKNOWNSTR		L"Windows [Unknown version]"

#define W95STR			L"Windows 95"
#define W95SP1STR		L"Windows 95 SP1"
#define W95OSR2STR		L"Windows 95 OSR2"
#define W98STR			L"Windows 98"
#define W98SP1STR		L"Windows 98 SP1"
#define W98SESTR		L"Windows 98 SE"
#define WMESTR			L"Windows ME"

#define WNT351STR		L"Windows NT 3.51"
#define WNT4STR			L"Windows NT 4"
#define W2KSTR			L"Windows 2000"
#define WXPSTR			L"Windows XP"
#define W2003SERVERSTR	L"Windows 2003 Server"
#define WVSTR			L"Windows Vista"
#define W7STR			L"Windows 7"
#define W8STR			L"Windows 8"
#define W81STR			L"Windows 8.1"
#define W10STR			L"Windows 10"
#define W11STR			L"Windows 11"

#define WCESTR			L"Windows CE"


#define WUNKNOWN	0
#define W9XFIRST	1
#define W95			1
#define W95SP1		2
#define W95OSR2		3
#define W98			4
#define W98SP1		5
#define W98SE		6
#define WME			7
#define W9XLAST		99

#define WNTFIRST	101
#define WNT351		101
#define WNT4		102
#define W2K			103
#define WXP			104
#define W2003SERVER	105
#define WV			106
#define W7			107
#define W8			108
#define W81			109
#define W10			110
#define W11			111

#define WNTLAST		199

#define WCEFIRST	201
#define WCE			201
#define WCELAST		299

BOOL GetWinVer(LPWSTR pszVersion, int *nVersion, LPWSTR pszMajorMinorBuild);

#endif //GETWINVER_H
