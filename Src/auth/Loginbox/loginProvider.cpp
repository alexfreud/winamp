#include "./loginProvider.h"
#include "./loginTemplate.h"
#include "./loginCommand.h"
#include "./common.h"

#include <strsafe.h>


static HRESULT LoginProvider_SetString(LPWSTR *ppszTarget, LPCWSTR pszSource)
{
	if (NULL == ppszTarget) 
		return E_POINTER;
	if (NULL != *ppszTarget) 
		free(*ppszTarget);

	
	if (NULL == pszSource) 
		*ppszTarget = NULL;
	else
	{
		*ppszTarget = LoginBox_CopyString(pszSource);
		if (NULL == *ppszTarget) return E_OUTOFMEMORY;
	}
	return S_OK;
}

LoginProvider::LoginProvider(const GUID *providerUid)
	: ref(1), name(NULL), description(NULL), imagePath(NULL), tosUrl(NULL), 
	privacyUrl(NULL), helpUrl(NULL), pageTemplate(NULL), command(NULL)
{
	id = (NULL != providerUid) ? *providerUid : GUID_NULL;
}

LoginProvider::~LoginProvider()
{
	LoginBox_FreeString(name);
	LoginBox_FreeString(description);
	LoginBox_FreeString(imagePath);
	LoginBox_FreeString(tosUrl);
	LoginBox_FreeString(privacyUrl);
	LoginBox_FreeString(helpUrl);
	if (NULL != pageTemplate) pageTemplate->Release();
	if (NULL != command) command->Release();
}

HRESULT LoginProvider::CreateInstance(const GUID *providerUid, LoginProvider **instance)
{
	if (NULL == instance) return E_POINTER;
	if (NULL == providerUid) return E_INVALIDARG;
	*instance = new LoginProvider(providerUid);
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

ULONG LoginProvider::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG LoginProvider::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT LoginProvider::IsIdentical(LoginProvider *test)
{
	if (NULL == test)
		return E_INVALIDARG;
	
	if (FALSE == IsEqualGUID(id, test->id))
		return S_FALSE;
	
	
	if (S_OK != LoginBox_IsStrEq(name, test->name) || 
		S_OK != LoginBox_IsStrEq(description, test->description) ||
		S_OK != LoginBox_IsStrEqInvI(imagePath, test->imagePath) ||
		S_OK != LoginBox_IsStrEqInvI(tosUrl, test->tosUrl) ||
		S_OK != LoginBox_IsStrEqInvI(privacyUrl, test->privacyUrl) ||
		S_OK != LoginBox_IsStrEqInvI(helpUrl, test->helpUrl))
	{
		return S_FALSE;
	}

	if ((NULL == pageTemplate) != (NULL == test->pageTemplate))
		return S_FALSE;
	
	if (NULL != pageTemplate)
	{
		HRESULT hr = pageTemplate->IsIdentical(test->pageTemplate);
		if (S_OK != hr) return hr;
	}
	
	if ((NULL == command) != (NULL == test->command))
		return S_FALSE;
	
	if (NULL != command)
	{
		HRESULT hr = command->IsIdentical(test->command);
		if (S_OK != hr) return hr;
	}

	return S_OK;
}

HRESULT LoginProvider::IsValid()
{
	return (NULL != pageTemplate && NULL != command) ? S_OK :S_FALSE;
}

HRESULT LoginProvider::GetId(GUID *pId)
{
	if (NULL == pId) return E_POINTER;
	*pId = id;
	return S_OK;
}

HRESULT LoginProvider::GetName(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return E_INVALIDARG;
	return StringCchCopyEx(pszBuffer, cchBufferMax, name, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT LoginProvider::GetDescription(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return E_INVALIDARG;
	return StringCchCopyEx(pszBuffer, cchBufferMax, description, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT LoginProvider::GetImagePath(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return E_INVALIDARG;
	return StringCchCopyEx(pszBuffer, cchBufferMax, imagePath, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT LoginProvider::GetTosLink(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return E_INVALIDARG;
	return StringCchCopyEx(pszBuffer, cchBufferMax, tosUrl, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT LoginProvider::GetPrivacyLink(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return E_INVALIDARG;
	return StringCchCopyEx(pszBuffer, cchBufferMax, privacyUrl, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT LoginProvider::GetHelpLink(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return E_INVALIDARG;
	return StringCchCopyEx(pszBuffer, cchBufferMax, helpUrl, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT LoginProvider::GetTemplate(LoginTemplate **ppTemplate)
{
	if (NULL == ppTemplate) 
		return E_POINTER;

	*ppTemplate = pageTemplate;
	if (NULL != pageTemplate)
		pageTemplate->AddRef();
	
	return S_OK;
}

HRESULT LoginProvider::GetCommand(LoginCommand **ppCommand)
{
	if (NULL == ppCommand) 
		return E_POINTER;

	*ppCommand = command;
	if (NULL != command)
		command->AddRef();
	
	return S_OK;
}

HRESULT LoginProvider::SetName(LPCWSTR pszName)
{
	return LoginProvider_SetString(&name, pszName);
}

HRESULT LoginProvider::SetDescription(LPCWSTR pszDescription)
{
	return LoginProvider_SetString(&description, pszDescription);
}

HRESULT LoginProvider::SetImagePath(LPCWSTR pszImagePath)
{
	return LoginProvider_SetString(&imagePath, pszImagePath);
}

HRESULT LoginProvider::SetTosLink(LPCWSTR pszUrl)
{
	return LoginProvider_SetString(&tosUrl, pszUrl);
}

HRESULT LoginProvider::SetPrivacyLink(LPCWSTR pszUrl)
{
	return LoginProvider_SetString(&privacyUrl, pszUrl);
}

HRESULT LoginProvider::SetHelpLink(LPCWSTR pszUrl)
{
	return LoginProvider_SetString(&helpUrl, pszUrl);
}

HRESULT LoginProvider::SetTemplate(LoginTemplate *pTemplate)
{
	if (NULL != pageTemplate) pageTemplate->Release();
	pageTemplate = pTemplate;
	if (NULL != pageTemplate) pageTemplate->AddRef();
	return S_OK;
}

HRESULT LoginProvider::SetCommand(LoginCommand *pCommand)
{
	if (NULL != command) command->Release();
	command = pCommand;
	if (NULL != command) command->AddRef();
	return S_OK;
}



