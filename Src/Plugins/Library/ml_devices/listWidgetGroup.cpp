#include "main.h"
#include "./listWidgetInternal.h"
#include <algorithm>

ListWidgetGroup *
ListWidget_CreateGroup(const char *name)
{
	ifc_devicetype *deviceType;
	wchar_t buffer[1024] = {0};

	if (NULL == name)
		return NULL;

	if (NULL != WASABI_API_DEVICES &&
		S_OK == WASABI_API_DEVICES->TypeFind(name, &deviceType))
	{
		if (FAILED(deviceType->GetDisplayName(buffer, ARRAYSIZE(buffer))))
			buffer[0] = L'\0';

		deviceType->Release();
	}
	else
		buffer[0] = L'\0';

	return ListWidget_CreateGroupEx(name, buffer);
}

ListWidgetGroup *
ListWidget_CreateGroupEx(const char *name, const wchar_t *title)
{
	ListWidgetGroup *group;

	if (NULL == name)
		return NULL;

	group = new ListWidgetGroup();
	if (NULL == group)
		return NULL;

	group->name = AnsiString_Duplicate(name);
	group->title = String_Duplicate(title);
			
	return group;
}


void 
ListWidget_DestroyGroup(ListWidgetGroup *group)
{
	size_t index;
	if (NULL == group)
		return;

	index = group->items.size();
	while(index--)
	{
		ListWidget_DestroyItem(group->items[index]);
	}

	AnsiString_Free(group->name);
	String_Free(group->title);

	delete group;
}

BOOL
ListWidget_AddGroup( ListWidgetCategory *category, ListWidgetGroup *group)
{
	if (NULL == category || NULL == group)
		return FALSE;

	category->groups.push_back(group);
	return TRUE;
}

ListWidgetGroup *
ListWidget_FindGroupEx(ListWidgetCategory *category, const char *name, size_t max)
{
	size_t index, count;

	if (NULL == category || NULL == name)
		return NULL;

	count = category->groups.size();
	if (max < count) 
		count = max;

	for(index = 0; index < count; index++)
	{
		ListWidgetGroup *group = category->groups[index];
		if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, 0, name, -1, group->name, -1))
			return group;
	}
	
	return NULL;
}

ListWidgetGroup *
ListWidget_FindGroup(ListWidgetCategory *category, const char *name)
{
	return ListWidget_FindGroupEx(category, name, -1);
}

static int
ListWidget_ItemSortCb(const void *element1, const void *element2)
{
	ListWidgetItem *item1;
	ListWidgetItem *item2;
	int result;

	item1 = (ListWidgetItem*)element1;
	item2 = (ListWidgetItem*)element2;

	result = CompareString(LOCALE_USER_DEFAULT, 0, item1->title, -1, item2->title, -1);
	if (CSTR_EQUAL == result || 0 == result)
		result = CompareStringA(CSTR_INVARIANT, 0, item1->name, -1, item2->name, -1);
		
	return (result - 2);

}
static bool
ListWidget_ItemSortCb_V2(const void* element1, const void* element2)
{
	return ListWidget_ItemSortCb(element1, element2) < 0;
}

void 
ListWidget_SortGroup(ListWidgetGroup *group)
{
	if (group->items.size())
	{
		//qsort(group->items.first(), group->items.size(), sizeof(ListWidgetItem**), ListWidget_ItemSortCb);
		std::sort(group->items.begin(), group->items.end(), ListWidget_ItemSortCb_V2);
	}
}
