#ifndef NULLSOFT_WINAMP_OMBROWSER_INTERNAL_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_INTERNAL_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#ifndef __IProtectFocus_INTERFACE_DEFINED__
#define __IProtectFocus_INTERFACE_DEFINED__

/* interface IProtectFocus */
/* [unique][uuid][object] */ 

EXTERN_C const IID IID_IProtectFocus;
    
MIDL_INTERFACE("d81f90a3-8156-44f7-ad28-5abb87003274")
IProtectFocus : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE AllowFocusChange(/* [out] */ BOOL *pfAllow) = 0;
    
};

#endif 	/* __IProtectFocus_INTERFACE_DEFINED__ */

#define SID_SProtectFocus  IID_IProtectFocus

typedef void (CALLBACK *DISPATCHAPC)(IDispatch *pDisp, ULONG_PTR /*param*/);

HRESULT FormatEncryptionString(UINT encryptionId, LPWSTR pszBuffer, INT cchBufferMax);

#endif //NULLSOFT_WINAMP_OMBROWSER_INTERNAL_HEADER