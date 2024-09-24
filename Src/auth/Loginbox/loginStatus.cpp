#include "./loginStatus.h"
#include "./common.h"


LoginStatus::LoginStatus(HWND hTarget) 
	: ref(1), hwnd(hTarget)
{
	InitializeCriticalSection(&lock);
}
LoginStatus::~LoginStatus()
{
	DeleteCriticalSection(&lock);
}

HRESULT LoginStatus::CreateInstance(HWND hTarget, LoginStatus **instance)
{
	if (NULL == instance)
		return E_POINTER;
	*instance = new LoginStatus(hTarget);
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}


ULONG LoginStatus::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG LoginStatus::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}


UINT LoginStatus::Add(BSTR status)
{
	EnterCriticalSection(&lock);

	Record r;
	r.cookie = GetNextCookie();
	r.text = status;
	list.push_back(r);
	
	LeaveCriticalSection(&lock);
	
	UpdateWindowText();
	return r.cookie;
}

BOOL LoginStatus::Set(UINT cookie, BSTR status)
{
	BOOL foundOk = FALSE;
	EnterCriticalSection(&lock);

	size_t index = list.size();
	while(index--)
	{
		if (cookie == list[index].cookie)
		{
			SysFreeString(list[index].text);
			list[index].text = status;
			foundOk = TRUE;
			break;
		}
	}
	
	LeaveCriticalSection(&lock);
	
	UpdateWindowText();
	return foundOk;
}

void LoginStatus::Remove(UINT cookie)
{	
	EnterCriticalSection(&lock);

	size_t index = list.size();
	while(index--)
	{
		if (cookie == list[index].cookie)
		{
			SysFreeString(list[index].text);
			list.eraseAt(index);
			break;
		}
	}
	
	LeaveCriticalSection(&lock);
	UpdateWindowText();
}

BOOL LoginStatus::AttachWindow(HWND hTarget)
{
	DetachWindow();

	hwnd = hTarget;
	UpdateWindowText();
	return TRUE;
}

BOOL LoginStatus::DetachWindow()
{
	hwnd = NULL;
	return TRUE;
}

UINT LoginStatus::GetNextCookie()
{	
	size_t i, count;
	
	count = list.size();
	UINT cookie = (UINT)count;
	
	do
	{
		for (i = 0; i < count; i++)
		{
			if (list[i].cookie == cookie)
			{
				cookie++;
				break;
			}
		}
	} while(i != count);
	return cookie;
}

BOOL LoginStatus::UpdateWindowText()
{
	
	EnterCriticalSection(&lock);
	
	BOOL resultOk = FALSE;
	if (NULL != hwnd)
	{
		BSTR text = NULL;
		size_t index = list.size();
		while(index--)
		{
			if (NULL != list[index].text && L'\0' != list[index].text)
			{
				text = list[index].text;
				break;
			}
		}
		resultOk = (BOOL)SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)text);
	}
	LeaveCriticalSection(&lock);
	return resultOk;
}

