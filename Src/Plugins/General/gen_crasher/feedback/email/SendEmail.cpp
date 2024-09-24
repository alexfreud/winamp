// SendEmail.cpp  Version 1.0
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

// Notes on compiling:  this does not use MFC.  Set precompiled header
//                      option to "Not using precompiled headers".
//
// This code assumes that a default mail client has been set up.

#include ".\sendemail.h"
#include <crtdbg.h>
#include <io.h>
#pragma warning(disable: 4228)
#include <mapi.h>
#pragma warning(default: 4228)

#pragma warning(disable:4127)	// for _ASSERTE

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif


BOOL SendEmail(HWND    hWnd,			// parent window, must not be NULL
			   LPCTSTR lpszTo,			// must NOT be NULL or empty
			   LPCTSTR lpszToName,		// may be NULL
			   LPCTSTR lpszSubject,		// may be NULL
			   LPCTSTR lpszMessage,		// may be NULL
			   LPCTSTR lpszAttachment)	// may be NULL
{
	
	_ASSERTE(lpszTo && lpszTo[0] != _T('\0'));
	if (lpszTo == NULL || lpszTo[0] == _T('\0'))
		return FALSE;

	// ===== LOAD MAPI DLL =====

	HMODULE hMapi = ::LoadLibraryA("MAPI32.DLL");

	_ASSERTE(hMapi);
	if (hMapi == NULL)
	{
		::MessageBox(NULL,
					 _T("Failed to load MAPI32.DLL."), 
					 _T("CrashRep"),
					 MB_OK|MB_ICONSTOP);
		return FALSE;
	}

	// get proc address for MAPISendMail
	ULONG (PASCAL *lpfnSendMail)(ULONG, ULONG, MapiMessage*, FLAGS, ULONG);
	(FARPROC&)lpfnSendMail = GetProcAddress(hMapi, "MAPISendMail");
	_ASSERTE(lpfnSendMail);
	if (lpfnSendMail == NULL)
	{
		::MessageBox(NULL,
					 _T("Invalid MAPI32.DLL, cannot find MAPISendMail."),
					 _T("CrashRep"),
					 MB_OK|MB_ICONSTOP);
		::FreeLibrary(hMapi);
		return FALSE;
	}

	// ===== SET UP MAPI STRUCTS =====

	// ===== file description (for the attachment) =====

	MapiFileDesc fileDesc;
	memset(&fileDesc, 0, sizeof(fileDesc));

	// ----- attachment path
	TCHAR szTempName[_MAX_PATH*2];
	memset(szTempName, 0, sizeof(szTempName));
	if (lpszAttachment && lpszAttachment[0] != _T('\0'))
		lstrcpyn(szTempName, lpszAttachment, _countof(szTempName)-2);

#ifdef _UNICODE
	char szTempNameA[_MAX_PATH*2];
	memset(szTempNameA, 0, sizeof(szTempNameA));
	_wcstombsz(szTempNameA, szTempName, _countof(szTempNameA)-2);
#endif

	// ----- attachment title
	TCHAR szTitle[_MAX_PATH*2];
	memset(szTitle, 0, sizeof(szTitle));
	if (lpszAttachment && lpszAttachment[0] != _T('\0'))
		lstrcpyn(szTitle, GetFilePart(lpszAttachment), _countof(szTitle)-2);

#ifdef _UNICODE
	char szTitleA[_MAX_PATH*2];
	memset(szTitleA, 0, sizeof(szTitleA));
	_wcstombsz(szTitleA, szTitle, _countof(szTitleA)-2);
#endif

	fileDesc.nPosition = (ULONG)-1;
#ifdef _UNICODE
	fileDesc.lpszPathName = szTempNameA;
	fileDesc.lpszFileName = szTitleA;
#else
	fileDesc.lpszPathName = szTempName;
	fileDesc.lpszFileName = szTitle;
#endif

	// ===== recipient =====

	MapiRecipDesc recip;
	memset(&recip, 0, sizeof(recip));

	// ----- name
	TCHAR szRecipName[_MAX_PATH*2];
	memset(szRecipName, 0, sizeof(szRecipName));
	if (lpszToName && lpszToName[0] != _T('\0'))
		lstrcpyn(szRecipName, lpszToName, _countof(szRecipName)-2);
#ifdef _UNICODE
	char szRecipNameA[_MAX_PATH*2];
	memset(szRecipNameA, 0, sizeof(szRecipNameA));
	_wcstombsz(szRecipNameA, szRecipName, _countof(szRecipNameA)-2);
#endif

	if (lpszToName && lpszToName[0] != _T('\0'))
	{
#ifdef _UNICODE
		recip.lpszName = szRecipNameA;
#else
		recip.lpszName = szRecipName;
#endif
	}

	// ----- address
	TCHAR szAddress[_MAX_PATH*2];
	memset(szAddress, 0, sizeof(szAddress));
	lstrcpyn(szAddress, lpszTo, _countof(szAddress)-2);
#ifdef _UNICODE
	char szAddressA[_MAX_PATH*2];
	memset(szAddressA, 0, sizeof(szAddressA));
	_wcstombsz(szAddressA, szAddress, _countof(szAddressA)-2);
#endif

#ifdef _UNICODE
	recip.lpszAddress = szAddressA;
#else
	recip.lpszAddress = szAddress;
#endif

	recip.ulRecipClass = MAPI_TO;

	// ===== message =====

	MapiMessage message;
	memset(&message, 0, sizeof(message));

	// ----- recipient
	message.nRecipCount = 1;
	message.lpRecips = &recip;

	// ----- attachment
	if (lpszAttachment && lpszAttachment[0] != _T('\0'))
	{
		message.nFileCount = 1;
		message.lpFiles = &fileDesc;
	}

	// ----- subject
	TCHAR szSubject[_MAX_PATH*2];
	memset(szSubject, 0, sizeof(szSubject));
	if (lpszSubject && lpszSubject[0] != _T('\0'))
		lstrcpyn(szSubject, lpszSubject, _countof(szSubject)-2);
#ifdef _UNICODE
	char szSubjectA[_MAX_PATH*2];
	memset(szSubjectA, 0, sizeof(szSubjectA));
	_wcstombsz(szSubjectA, szSubject, _countof(szSubjectA)-2);
#endif

	if (lpszSubject && lpszSubject[0] != _T('\0'))
	{
#ifdef _UNICODE
		message.lpszSubject = szSubjectA;
#else
		message.lpszSubject = szSubject;
#endif
	}
	
	// ----- message
	// message may be large, so allocate buffer
	TCHAR *pszMessage = NULL;
	int nMessageSize = 0;
	if (lpszMessage)
	{
		nMessageSize = lstrlen(lpszMessage);
		if (nMessageSize > 0)
		{
			pszMessage = new TCHAR [nMessageSize + 10];
			_ASSERTE(pszMessage);
			memset(pszMessage, 0, nMessageSize + 10);
			lstrcpy(pszMessage, lpszMessage);
		}
	}

	char *pszMessageA = NULL;
#ifdef _UNICODE
	if (nMessageSize > 0)
	{
		pszMessageA = new char [nMessageSize + 10];
		_ASSERTE(pszMessageA);
		memset(pszMessageA, 0, nMessageSize + 10);
	}
	_wcstombsz(pszMessageA, pszMessage, nMessageSize+2);
#endif

	if (nMessageSize > 0)
	{
#ifdef _UNICODE
		message.lpszNoteText = pszMessageA;
#else
		message.lpszNoteText = pszMessage;
#endif
	}


	// ===== SETUP FINISHED, READY TO SEND =====


	// some extra precautions are required to use MAPISendMail as it
	// tends to enable the parent window in between dialogs (after
	// the login dialog, but before the send note dialog).

	::SetCapture(hWnd);
	::SetFocus(NULL);
	::EnableWindow(hWnd, FALSE);

	ULONG nError = lpfnSendMail(0, 
								(LPARAM)hWnd,
								&message, 
								MAPI_LOGON_UI | MAPI_DIALOG, 
								0);

#ifdef _DEBUG
	TCHAR *cp = NULL;
	switch (nError)
	{
		case SUCCESS_SUCCESS:                 cp = _T("SUCCESS_SUCCESS"); break;
		case MAPI_E_USER_ABORT:               cp = _T("MAPI_E_USER_ABORT"); break;
		case MAPI_E_FAILURE:                  cp = _T("MAPI_E_FAILURE"); break;
		case MAPI_E_LOGON_FAILURE:            cp = _T("MAPI_E_LOGON_FAILURE"); break;
		case MAPI_E_DISK_FULL:                cp = _T("MAPI_E_DISK_FULL"); break;
		case MAPI_E_INSUFFICIENT_MEMORY:      cp = _T("MAPI_E_INSUFFICIENT_MEMORY"); break;
		case MAPI_E_ACCESS_DENIED:            cp = _T("MAPI_E_ACCESS_DENIED"); break;
		case MAPI_E_TOO_MANY_SESSIONS:        cp = _T("MAPI_E_TOO_MANY_SESSIONS"); break;
		case MAPI_E_TOO_MANY_FILES:           cp = _T("MAPI_E_TOO_MANY_FILES"); break;
		case MAPI_E_TOO_MANY_RECIPIENTS:      cp = _T("MAPI_E_TOO_MANY_RECIPIENTS"); break;
		case MAPI_E_ATTACHMENT_NOT_FOUND:     cp = _T("MAPI_E_ATTACHMENT_NOT_FOUND"); break;
		case MAPI_E_ATTACHMENT_OPEN_FAILURE:  cp = _T("MAPI_E_ATTACHMENT_OPEN_FAILURE"); break;
		case MAPI_E_ATTACHMENT_WRITE_FAILURE: cp = _T("MAPI_E_ATTACHMENT_WRITE_FAILURE"); break;
		case MAPI_E_UNKNOWN_RECIPIENT:        cp = _T("MAPI_E_UNKNOWN_RECIPIENT"); break;
		case MAPI_E_BAD_RECIPTYPE:            cp = _T("MAPI_E_BAD_RECIPTYPE"); break;
		case MAPI_E_NO_MESSAGES:              cp = _T("MAPI_E_NO_MESSAGES"); break;
		case MAPI_E_INVALID_MESSAGE:          cp = _T("MAPI_E_INVALID_MESSAGE"); break;
		case MAPI_E_TEXT_TOO_LARGE:           cp = _T("MAPI_E_TEXT_TOO_LARGE"); break;
		case MAPI_E_INVALID_SESSION:          cp = _T("MAPI_E_INVALID_SESSION"); break;
		case MAPI_E_TYPE_NOT_SUPPORTED:       cp = _T("MAPI_E_TYPE_NOT_SUPPORTED"); break;
		case MAPI_E_AMBIGUOUS_RECIPIENT:      cp = _T("MAPI_E_AMBIGUOUS_RECIPIENT"); break;
		case MAPI_E_MESSAGE_IN_USE:           cp = _T("MAPI_E_MESSAGE_IN_USE"); break;
		case MAPI_E_NETWORK_FAILURE:          cp = _T("MAPI_E_NETWORK_FAILURE"); break;
		case MAPI_E_INVALID_EDITFIELDS:       cp = _T("MAPI_E_INVALID_EDITFIELDS"); break;
		case MAPI_E_INVALID_RECIPS:           cp = _T("MAPI_E_INVALID_RECIPS"); break;
		case MAPI_E_NOT_SUPPORTED:            cp = _T("MAPI_E_NOT_SUPPORTED"); break;
		default:                              cp = _T("unknown error"); break;
	}

	if (nError == SUCCESS_SUCCESS)
	{
		OutputDebugString(_T("MAPISendMail ok\r\n"));
	}
	else
	{
		OutputDebugString(L"ERROR - MAPISendMail failed: ");
		OutputDebugString(cp);
		OutputDebugString(L"\r\n");
		
	}
#endif // _DEBUG


	// ===== SEND COMPLETE, CLEAN UP =====

	// after returning from the MAPISendMail call, the window must be
	// re-enabled and focus returned to the frame to undo the workaround
	// done before the MAPI call.

	::ReleaseCapture();
	::EnableWindow(hWnd, TRUE);
	::SetActiveWindow(NULL);
	::SetActiveWindow(hWnd);
	::SetFocus(hWnd);

	if (pszMessage)
		delete [] pszMessage;
	pszMessage = NULL;

	if (pszMessageA)
		delete [] pszMessageA;
	pszMessageA = NULL;

	if (hMapi)
		::FreeLibrary(hMapi);
	hMapi = NULL;

	BOOL bRet = TRUE;
	if (nError != SUCCESS_SUCCESS) // &&
		//nError != MAPI_USER_ABORT && 
		//nError != MAPI_E_LOGIN_FAILURE)
	{
		bRet = FALSE;
	}

	return bRet;
}

const wchar_t* GetFilePart(LPCWSTR lpszFile)
{
	const wchar_t *result = wcsrchr(lpszFile, _T('\\'));
	if (result)
		result++;
	else
		result = (wchar_t *) lpszFile;
	return result;
}

int xwcstombsz(char* mbstr, const wchar_t* wcstr, size_t count)
{
	if (count == 0 && mbstr != NULL)
		return 0;

	int result = ::WideCharToMultiByte(CP_ACP, 0, wcstr, -1,
		mbstr, (int)count, NULL, NULL);
	_ASSERTE(mbstr == NULL || result <= (int)count);
	if (result > 0)
		mbstr[result-1] = 0;
	return result;
}

int xmbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count)
{
	if (count == 0 && wcstr != NULL)
		return 0;

	int result = ::MultiByteToWideChar(CP_ACP, 0, mbstr, -1,
		wcstr, (int)count);
	_ASSERTE(wcstr == NULL || result <= (int)count);
	if (result > 0)
		wcstr[result-1] = 0;
	return result;
}