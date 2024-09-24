#pragma once
#include "api_history.h"

class HistoryAPI : public api_history
{
public:
	historyRecordList *Query(const wchar_t *query);
	void FreeHistoryList(historyRecordList *historyList);

protected:
	RECVS_DISPATCH;
};