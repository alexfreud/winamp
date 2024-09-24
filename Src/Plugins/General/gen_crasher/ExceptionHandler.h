// ExceptionHandler.h  Version 1.1
//
// Copyright © 1998 Bruce Dawson
//
// Author:       Bruce Dawson
//               brucedawson@cygnus-software.com
//
// Modified by:  Hans Dietrich
//               hdietrich2@hotmail.com
//
// A paper by the original author can be found at:
//     http://www.cygnus-software.com/papers/release_debugging.html
//
///////////////////////////////////////////////////////////////////////////////

#ifndef	EXCEPTIONHANDLER_H
#define	EXCEPTIONHANDLER_H

BOOL CreateLog(PEXCEPTION_POINTERS pExceptPtrs, LPCWSTR lpszMessage);
BOOL CreateDump(PEXCEPTION_POINTERS pExceptPtrs);

// We forward declare PEXCEPTION_POINTERS so that the function
// prototype doesn't needlessly require windows.h.
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#endif