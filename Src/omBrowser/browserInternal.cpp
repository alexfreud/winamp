#include "main.h"
#include "./browserInternal.h"
#include "./resource.h"
#include <exdisp.h>
#include <strsafe.h>

#if (_MSC_VER < 1500)
// {D81F90A3-8156-44F7-AD28-5ABB87003274}
EXTERN_C const IID IID_IProtectFocus = 
{ 0xd81f90a3, 0x8156, 0x44f7, { 0xad, 0x28, 0x5a, 0xbb, 0x87, 0x00, 0x32, 0x74 } };
#endif

HRESULT FormatEncryptionString(UINT encryptionId, LPWSTR pszBuffer, INT cchBufferMax)
{
	HRESULT hr = S_OK;
	if (secureLockIconUnsecure == encryptionId)
	{
		Plugin_LoadString(IDS_CONNECTION_UNSECURE, pszBuffer, cchBufferMax);
		return S_OK;
	}

	Plugin_LoadString(IDS_CONNECTION_ENCRYPTED, pszBuffer, cchBufferMax);

	INT resId = 0;
	switch(encryptionId)
	{				
		case secureLockIconMixed:				resId = IDS_ENCRYPTION_MIXED; break;
		case secureLockIconSecure40Bit:			resId = IDS_ENCRYPTION_40BIT; break;
		case secureLockIconSecure56Bit:			resId = IDS_ENCRYPTION_56BIT; break;
		case secureLockIconSecureFortezza:		resId = IDS_ENCRYPTION_FORTEZZA; break;
		case secureLockIconSecure128Bit:			resId = IDS_ENCRYPTION_128BIT; break;
	}

	if (0 != resId)
	{
		WCHAR szEncryption[96] = {0};
		Plugin_LoadString(resId, szEncryption, ARRAYSIZE(szEncryption));
		if (L'\0' != szEncryption[0])
		{
			INT cchLen = lstrlen(pszBuffer);
			hr = StringCchPrintf(pszBuffer + cchLen, cchBufferMax - cchLen, L": %s", szEncryption);
		}
	}
	return hr;
}