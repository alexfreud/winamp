#pragma once

#include <bfc/dispatch.h>
#include "history.h"

class api_history : public Dispatchable
{
protected:
	api_history() {}
	~api_history() {}
public:
	historyRecordList *Query(const wchar_t *query);
	void FreeHistoryList(historyRecordList *historyList);

	enum
	{
		API_HISTORY_QUERY = 0,
		API_HISTORY_FREEHISTORYLIST = 1,
	};
};

inline historyRecordList *api_history::Query(const wchar_t *query)
{
	return _call(API_HISTORY_QUERY, (historyRecordList *)0, query);
}

inline void api_history::FreeHistoryList(historyRecordList *historyList)
{
	_voidcall(API_HISTORY_FREEHISTORYLIST, historyList);
}

// {F9BF9119-D163-4118-BEA7-5980869DBB2E}
static const GUID HistoryApiGuid = 
{ 0xf9bf9119, 0xd163, 0x4118, { 0xbe, 0xa7, 0x59, 0x80, 0x86, 0x9d, 0xbb, 0x2e } };
