#ifndef NULLSOFT_GRAB_WINDOW_HEADER
#define NULLSOFT_GRAB_WINDOW_HEADER


#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

typedef void (CALLBACK *GRABCB)(HWND /*hwnd*/, CREATESTRUCT* /*lpcs*/, HWND* /*phwndInsertAfter*/, ULONG_PTR /*user*/);

BOOL BeginGrabCreateWindow(LPCWSTR pszClassName, LPCWSTR pszTitle, HWND hwndParent, GRABCB callback, ULONG_PTR user); // you can skip fields that you don't need
void EndGrabCreateWindow(void); //always call it when you done to gurantee proper shutdown



#endif //NULLSOFT_GRAB_WINDOW_HEADER