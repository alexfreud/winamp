#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPGROUPLIST_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPGROUPLIST_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./setupGroup.h"
#include <vector>

class SetupListboxItem;
class SetupLog;

class SetupGroupList
{

protected:
	SetupGroupList();
	~SetupGroupList();

public:
	static SetupGroupList *CreateInstance();

public:
	ULONG AddRef();
	ULONG Release();

	BOOL AddGroup(SetupGroup *group);
	SetupGroup *GetGroup(size_t index) { return list[index]; }
	size_t GetGroupCount();
	BOOL FindGroupIndex(SetupGroup *group, size_t *groupIndex);
	HRESULT FindGroupById(UINT groupId, SetupGroup **group);


	BOOL IsModified();
	
	HRESULT Save(SetupLog *log);

	size_t GetListboxCount();
	INT GetListboxItem(SetupListboxItem *item);
	HRESULT FindListboxItem(size_t listboxId, SetupListboxItem **listboxItem);

	void SetPageWnd(HWND hPage);
	
	
protected:
	ULONG ref;
	std::vector<SetupGroup*> list;
};

#endif //NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPGROUPLIST_HEADER