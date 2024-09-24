#ifndef NULLSOFT_AUTH_LOGINBOX_ADDRESS_ENCODER_HEADER
#define NULLSOFT_AUTH_LOGINBOX_ADDRESS_ENCODER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define ENC_E_INSUFFICIENT_BUFFER	(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))

HRESULT AddressEncoder_DecodeString(LPCWSTR pszAddress, LPWSTR *ppResult);
HRESULT AddressEncoder_EncodeString(LPCWSTR pszAddress, LPWSTR pszBuffer, size_t *pcchBufferMax, UINT flags);

#endif //NULLSOFT_AUTH_LOGINBOX_ADDRESS_ENCODER_HEADER