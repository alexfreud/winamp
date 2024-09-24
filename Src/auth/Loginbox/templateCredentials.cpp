#include "./templateCredentials.h"
#include "./pageCredentials.h"
#include "./common.h"

LoginTemplateCredentials::LoginTemplateCredentials()
	: ref(1), title(NULL), accountRecoverUrl(NULL), accountCreateUrl(NULL), usernameLabel(NULL), passwordLabel(NULL)
{
}

LoginTemplateCredentials::~LoginTemplateCredentials()
{
	LoginBox_FreeString(title);
	LoginBox_FreeString(accountRecoverUrl);
	LoginBox_FreeString(accountCreateUrl);
	LoginBox_FreeString(usernameLabel);
	LoginBox_FreeString(passwordLabel);
}
	
HRESULT LoginTemplateCredentials::CreateInstance(LoginTemplateCredentials **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = new LoginTemplateCredentials();
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

ULONG LoginTemplateCredentials::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG LoginTemplateCredentials::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT LoginTemplateCredentials::GetType(GUID *templateUid)
{
	if (NULL == templateUid) return E_INVALIDARG;
	*templateUid = LTUID_CREDENTIALS;
	return S_OK;
}

HRESULT LoginTemplateCredentials::SetParameter(LPCWSTR pszKey, LPCWSTR pszValue)
{
	if (S_OK == LoginBox_IsStrEqInvI(pszKey, L"title"))
	{
		LoginBox_FreeString(title);
		title = LoginBox_CopyString(pszValue);
	}
	else if (S_OK == LoginBox_IsStrEqInvI(pszKey, L"accountRecoverUrl"))
	{
		LoginBox_FreeString(accountRecoverUrl);
		accountRecoverUrl = LoginBox_CopyString(pszValue);
	}
	else if (S_OK == LoginBox_IsStrEqInvI(pszKey, L"accountCreateUrl"))
	{
		LoginBox_FreeString(accountCreateUrl);
		accountCreateUrl = LoginBox_CopyString(pszValue);
	}
	else if (S_OK == LoginBox_IsStrEqInvI(pszKey, L"usernameLabel"))
	{
		LoginBox_FreeString(usernameLabel);
		usernameLabel = LoginBox_CopyString(pszValue);
	}
	else if (S_OK == LoginBox_IsStrEqInvI(pszKey, L"passwordLabel"))
	{
		LoginBox_FreeString(passwordLabel);
		passwordLabel = LoginBox_CopyString(pszValue);
	}

	return S_OK;
}

HRESULT LoginTemplateCredentials::IsValid()
{
	return S_OK;
}


HRESULT LoginTemplateCredentials::IsIdentical(LoginTemplate *test)
{
	if (NULL == test)
		return E_INVALIDARG;

	GUID typeId;
	if (FAILED(test->GetType(&typeId)) || FALSE == IsEqualGUID(LTUID_CREDENTIALS, typeId))
		return S_FALSE;
	
	LoginTemplateCredentials *testCred = (LoginTemplateCredentials*)test;

	if(S_OK != LoginBox_IsStrEq(title, testCred->title) ||
		S_OK != LoginBox_IsStrEqInvI(accountRecoverUrl, testCred->accountRecoverUrl) ||
		S_OK != LoginBox_IsStrEqInvI(accountCreateUrl, testCred->accountCreateUrl) ||
		S_OK != LoginBox_IsStrEq(usernameLabel, testCred->usernameLabel) ||
		S_OK != LoginBox_IsStrEq(passwordLabel, testCred->passwordLabel))
	{
		return S_FALSE;
	}

	return S_OK;
}


HWND LoginTemplateCredentials::CreatePage(HWND hLoginbox, HWND hParent)
{
	HWND hPage = LoginPageCredentials::CreatePage(hLoginbox, hParent);
	if (NULL == hPage) return NULL;
	
	if (NULL != title) 
		LoginPage_SetTitle(hPage, title);
	if (NULL != accountRecoverUrl) 
		LoginPageCredentials_SetAccountRecoverUrl(hPage, accountRecoverUrl);
	if (NULL != accountCreateUrl) 
		LoginPageCredentials_SetAccountCreateUrl(hPage, accountCreateUrl);
	if (NULL != usernameLabel) 
		LoginPageCredentials_SetUsernameLabel(hPage, usernameLabel);
	if (NULL != passwordLabel) 
		LoginPageCredentials_SetPasswordLabel(hPage, passwordLabel);
		
	return hPage;
}