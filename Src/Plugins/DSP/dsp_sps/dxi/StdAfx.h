// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__5BCD2B6C_0D14_4C1E_8269_E522431AE0DD__INCLUDED_)
#define AFX_STDAFX_H__5BCD2B6C_0D14_4C1E_8269_E522431AE0DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <windowsx.h>

#ifdef _DEBUG
#define DEBUG	// the DirectShow headers use this symbol
#endif

#include <objbase.h>
#include <control.h>
#include <streams.h>
#include <pstream.h>

#ifndef WAVE_FORMAT_IEEE_FLOAT
	#define WAVE_FORMAT_IEEE_FLOAT (3)
#endif

#pragma warning( disable: 4786 ) // identifier was trucated to '255' characters in the debug information

#include <string>
#include <vector>
#include <algorithm>
using namespace std;

#pragma warning( disable: 4355 )	// 'this' : used in base member initialization list

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__5BCD2B6C_0D14_4C1E_8269_E522431AE0DD__INCLUDED_)
