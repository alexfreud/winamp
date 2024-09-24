#include "./templateInfo.h"
#include "./pageInfo.h"
#include "./common.h"

LoginTemplateInfo::LoginTemplateInfo()
	: ref(1), title(NULL), message(NULL)
{
}

LoginTemplateInfo::~LoginTemplateInfo()
{
	LoginBox_FreeString(title);
	LoginBox_FreeString(message);
}
	
HRESULT LoginTemplateInfo::CreateInstance(LoginTemplateInfo **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = new LoginTemplateInfo();
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

ULONG LoginTemplateInfo::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG LoginTemplateInfo::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT LoginTemplateInfo::GetType(GUID *templateUid)
{
	if (NULL == templateUid) return E_INVALIDARG;
	*templateUid = LTUID_INFO;
	return S_OK;
}

HRESULT LoginTemplateInfo::SetParameter(LPCWSTR pszKey, LPCWSTR pszValue)
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
	return S_OK;
}

HRESULT LoginTemplateInfo::IsValid()
{
	return S_OK;
}

HRESULT LoginTemplateInfo::IsIdentical(LoginTemplate *test)
{
	if (NULL == test)
		return E_INVALIDARG;

	GUID typeId;
	if (FAILED(test->GetType(&typeId)) || FALSE == IsEqualGUID(LTUID_INFO, typeId))
		return S_FALSE;
	
	LoginTemplateInfo *testInfo = (LoginTemplateInfo*)test;

	if(S_OK != LoginBox_IsStrEq(title, testInfo->title) ||
		S_OK != LoginBox_IsStrEq(message, testInfo->message))
	{
		return S_FALSE;
	}

	return S_OK;
}

HWND LoginTemplateInfo::CreatePage(HWND hLoginbox, HWND hParent)
{
	HWND hPage = LoginPageInfo::CreatePage(hLoginbox, hParent);
	if (NULL == hPage) return NULL;
	
	if (NULL != title) 
		LoginPage_SetTitle(hPage, title);

	if (NULL != message) 
		LoginPageInfo_SetMessage(hPage, message);
			
	return hPage;
}