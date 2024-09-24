#ifndef NULLSOFT_WINAMP_OMBROWSER_IEVERSION_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_IEVERSION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

HRESULT MSIE_GetVersionString(LPWSTR pszBuffer, INT cchBufferMax);
HRESULT MSIE_GetVersion(INT *majorOut, INT *minorOut, INT *buildOut, INT *subBuildOut);

#endif //NULLSOFT_WINAMP_OMBROWSER_IEVERSION_HEADER