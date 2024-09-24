#include "./dataAddress.h"
#include "./common.h"

#include <strsafe.h>

LoginDataAddress::LoginDataAddress(const GUID *pRealm, HWND hPage, HWND hLoginbox, LPCWSTR pszAddress)
	: LoginData(pRealm, hPage, hLoginbox), address(NULL)
{
	address = LoginBox_CopyString(pszAddress);
}

LoginDataAddress::~LoginDataAddress()
{	
	LoginBox_FreeString(address);
}

HRESULT LoginDataAddress::CreateInstance(const GUID *pRealm, HWND hPage, HWND hLoginbox, LPCWSTR pszAddress, LoginDataAddress **instance)
{
	if (NULL == instance) return E_POINTER;
	if (NULL == hPage || NULL == hLoginbox) return E_INVALIDARG;

	*instance = new LoginDataAddress(pRealm, hPage, hLoginbox, pszAddress);
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

HRESULT LoginDataAddress::QueryInterface(REFIID riid, void** ppObject)
{
	if (NULL == ppObject) 
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_LoginDataAddress))
	{
		*ppObject = static_cast<LoginDataAddress*>(this);
		if (NULL == *ppObject) return E_UNEXPECTED;
		AddRef();
		return S_OK;
	}
	
	return __super::QueryInterface(riid, ppObject);
}

LPCWSTR LoginDataAddress::GetAddress()
{
	return address;
}