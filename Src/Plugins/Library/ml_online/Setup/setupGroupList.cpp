#include "./setupGroupList.h"
#include "../api__ml_online.h"
#include <strsafe.h>

SetupGroupList::SetupGroupList() 
	: ref(1)
{
}

SetupGroupList::~SetupGroupList()
{
	size_t index = list.size();
	while(index--)
	{
		list[index]->Release();
	}
}

SetupGroupList *SetupGroupList::CreateInstance()
{
	return new SetupGroupList();
}

ULONG SetupGroupList::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG SetupGroupList::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	return r;
}

BOOL SetupGroupList::AddGroup(SetupGroup *group)
{
	if (NULL == group) return FALSE;
	list.push_back(group);
	group->AddRef();
	return TRUE;
}
size_t SetupGroupList::GetGroupCount()
{
	return list.size();
}

BOOL SetupGroupList::IsModified()
{
	size_t index = list.size();
	while(index--)
	{
		if (list[index]->IsModified()) 
			return TRUE;
	}

	return FALSE;
}

BOOL SetupGroupList::FindGroupIndex(SetupGroup *group, size_t *groupIndex)
{
	if (NULL == group) return FALSE;

	size_t index = list.size();
	while(index--)
	{
		if (list[index] == group)
		{
			if (NULL != groupIndex)
				*groupIndex = index;
			return TRUE;
		}
	}
	 
	return FALSE;

}

HRESULT SetupGroupList::FindGroupById(UINT groupId, SetupGroup **group)
{
	if (NULL == group) return E_POINTER;

	size_t index = list.size();
	while(index--)
	{
		if (list[index]->GetId() == groupId)
		{
			*group = list[index];
			(*group)->AddRef();
			return S_OK;
		}
	}
	return S_FALSE;
}

size_t SetupGroupList::GetListboxCount()
{
	size_t recordCount = list.size();
	size_t index = recordCount;
	while(index--)
	{
		recordCount += list[index]->GetListboxCount();
	}
	return recordCount;
}

HRESULT SetupGroupList::Save(SetupLog *log)
{
	HRESULT hr(S_OK);
	size_t index = list.size();
	while(index--)
	{
		if (FAILED(list[index]->Save(log)))
			hr = E_FAIL;
	}
	return hr;
}

HRESULT SetupGroupList::FindListboxItem(size_t listboxId, SetupListboxItem **listboxItem)
{
	if (NULL == listboxItem) return E_POINTER;

	size_t index = 0;
	size_t groupCount = list.size();

	SetupGroup *group;
	for (size_t i = 0; i < groupCount; i++)
	{
		group = list[i];
		if (index == listboxId)
		{
			*listboxItem = (SetupListboxItem*)group;
			return S_OK;
		}
		index++;

		size_t itemCount;
		if (0 != (itemCount = group->GetListboxCount()))
		{
			if (listboxId < (index + itemCount))
			{
				size_t itemIndex = (listboxId - index);
				*listboxItem = group->GetListboxItem(itemIndex);
				return S_OK;
			}
			index += itemCount;
		}
	}
	return E_NOTIMPL;
}

INT SetupGroupList::GetListboxItem(SetupListboxItem *item)
{
	if (NULL == item) return LB_ERR;
	size_t index = 0;
	size_t groupCount = list.size();
	SetupGroup *group;
	SetupListboxItem *groupItem;

	for (size_t i = 0; i < groupCount; i++)
	{
		group = list[i];
		if (item == group)
			return (INT)index;

		index++;
		size_t itemCount = group->GetListboxCount();
		for (size_t j = 0; j < itemCount; j++)
		{
			groupItem = group->GetListboxItem(j);
			if (groupItem == item)
				return (INT)index;
			index++;
		}
	}
	return LB_ERR;
}

void SetupGroupList::SetPageWnd(HWND hPage)
{
	size_t index = list.size();
	while(index--)
	{
		list[index]->SetPageWnd(hPage);
	}
}