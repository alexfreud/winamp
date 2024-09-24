// SendEmail.h  Version 1.0
//
// Author:  Hans Dietrich
//          hdietrich2@hotmail.com
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

#ifndef SENDEMAIL_H
#define SENDEMAIL_H

#include <tchar.h>
#include <windows.h>

const wchar_t* GetFilePart(LPCWSTR lpszFile);

BOOL SendEmail(HWND    hWnd,			// parent window, must not be NULL
			   LPCTSTR lpszTo,			// must NOT be NULL or empty
			   LPCTSTR lpszToName,		// may be NULL
			   LPCTSTR lpszSubject,		// may be NULL
			   LPCTSTR lpszMessage,		// may be NULL
			   LPCTSTR lpszAttachment);	// may be NULL


#define _wcstombsz xwcstombsz
#define _mbstowcsz xmbstowcsz

int xwcstombsz(char* mbstr, const wchar_t* wcstr, size_t count);
int xmbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count);

#endif //SENDEMAIL_H