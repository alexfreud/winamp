#include "./templateAddress.h"
#include "./pageAddress.h"
#include "./common.h"

#include <shlwapi.h>
#include <strsafe.h>

LoginTemplateAddress::LoginTemplateAddress()
	: ref(1), title(NULL), message(NULL), address(NULL), addressTitle(NULL), 
	replaceUsername(FALSE)
{
}

LoginTemplateAddress::~LoginTemplateAddress()
{
	LoginBox_FreeString(title);
	LoginBox_FreeString(message);
	LoginBox_FreeString(address);
	LoginBox_FreeString(addressTitle);
}
	
HRESULT LoginTemplateAddress::CreateInstance(LoginTemplateAddress **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = new LoginTemplateAddress();
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

ULONG LoginTemplateAddress::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG LoginTemplateAddress::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT LoginTemplateAddress::GetType(GUID *templateUid)
{
	if (NULL == templateUid) return E_INVALIDARG;
	*templateUid = LTUID_ADDRESS;
	return S_OK;
}

HRESULT LoginTemplateAddress::SetParameter(LPCWSTR pszKey, LPCWSTR pszValue)
{
	
	if (S_OK == LoginBox_IsStrEqInvI(pszKey, L"title"))
	{
		LoginBox_FreeString(title);
		title = LoginBox_CopyString(pszValue);
	}
	else if (S_OK == LoginBox_IsStrEqInvI(pszKey, L"message"))
	{
		LoginBox_FreeString(message);
		message = LoginBox_CopyString(pszValue);
	}
	else if (S_OK == LoginBox_IsStrEqInvI(pszKey, L"address"))
	{
		LoginBox_FreeString(address);
		address = LoginBox_CopyString(pszValue);
	}
	else if (S_OK == LoginBox_IsStrEqInvI(pszKey, L"addressTitle"))
	{
		LoginBox_FreeString(addressTitle);
		addressTitle = LoginBox_CopyString(pszValue);
	}
	else if (S_OK == LoginBox_IsStrEqInvI(pszKey, L"replaceUsername"))
	{
		if (NULL != pszValue)
		{
			WORD charType;
			LPCWSTR p = pszValue;
			while(L'\0' != *p)
			{
				if (FALSE == GetStringTypeW(CT_CTYPE1, p, 1, &charType) || 
					0 != ((C1_DIGIT | C1_XDIGIT) & charType))
				{
					break;
				}
				p = CharNext(p);
			}
			INT r;
			if (FALSE != StrToIntEx(p, STIF_SUPPORT_HEX, &r) && 0 != r)
				replaceUsername = TRUE;

		}
	}
	return S_OK;
}

HRESULT LoginTemplateAddress::IsValid()
{
	return S_OK;
}

HRESULT LoginTemplateAddress::IsIdentical(LoginTemplate *test)
{
	if (NULL == test)
		return E_INVALIDARG;

	GUID typeId;
	if (FAILED(test->GetType(&typeId)) || FALSE == IsEqualGUID(LTUID_ADDRESS, typeId))
		return S_FALSE;
	
	LoginTemplateAddress *testAddr = (LoginTemplateAddress*)test;

	if(S_OK != LoginBox_IsStrEq(title, testAddr->title) ||
		S_OK != LoginBox_IsStrEq(message, testAddr->message) ||
		S_OK != LoginBox_IsStrEqInvI(address, testAddr->address) ||
		S_OK != LoginBox_IsStrEq(addressTitle, testAddr->addressTitle) ||
		replaceUsername != testAddr->replaceUsername)
	{
		return S_FALSE;
	}
		
	return S_OK;
}


HWND LoginTemplateAddress::CreatePage(HWND hLoginbox, HWND hParent)
{
	HWND hPage = LoginPageAddress::CreatePage(hLoginbox, hParent);
	if (NULL == hPage) return NULL;
	
	if (NULL != title) 
		LoginPage_SetTitle(hPage, title);

	if (NULL != message) 
		LoginPageAddress_SetMessage(hPage, message);

	if (NULL != address) 
		LoginPageAddress_SetAddress(hPage, address, replaceUsername);

	if (NULL != addressTitle) 
		LoginPageAddress_SetAddressTitle(hPage, addressTitle);
			
	return hPage;
}