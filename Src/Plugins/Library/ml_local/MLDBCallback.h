#pragma once
#include <api/syscb/callbacks/svccb.h>
#include <api/syscb/api_syscb.h>
#include "../ml_local/api_mldb.h"

class MLDBCallback : public SysCallback
{
private:
	FOURCC GetEventType()
	{
		return api_mldb::SYSCALLBACK;
	}

	int notify(int msg, intptr_t param1, intptr_t param2)
	{
		const wchar_t *filename = (const wchar_t *)param1;

		switch (msg)
		{
			case api_mldb::MLDB_FILE_ADDED:
				OnFileAdded(filename);
			return 1;

			case api_mldb::MLDB_FILE_REMOVED_PRE:
				OnFileRemove_Pre(filename);
			return 1;

			case api_mldb::MLDB_FILE_REMOVED_POST:
				OnFileRemove_Post(filename);
			return 1;

			case api_mldb::MLDB_FILE_UPDATED:
			case api_mldb::MLDB_FILE_UPDATED_EXTERNAL:
				OnFileUpdated(filename, (msg == api_mldb::MLDB_FILE_UPDATED));
			return 1;

			case api_mldb::MLDB_FILE_PLAYED:
				OnFilePlayed(filename,
							 ((api_mldb::played_info *)param2)->played,
							 ((api_mldb::played_info *)param2)->count);
			return 1;

			case api_mldb::MLDB_CLEARED:
				OnCleared((const wchar_t **)param1, param2);
			return 1;

			case api_mldb::MLDB_FILE_GET_CLOUD_STATUS:
			{
				OnGetCloudStatus((const wchar_t *)param1, (HMENU *)param2);
			}
			return 1;

			case api_mldb::MLDB_FILE_PROCESS_CLOUD_STATUS:
				OnProcessCloudStatus(param1, (int *)param2);
			return 1;

			default:
			return 0;
		}
	}
	virtual void OnFileAdded(const wchar_t *filename) {}
	virtual void OnFileRemove_Pre(const wchar_t *filename) {}
	virtual void OnFileRemove_Post(const wchar_t *filename) {}
	virtual void OnFileUpdated(const wchar_t *filename, bool from_library) {}
	virtual void OnFilePlayed(const wchar_t *filename, time_t played, int count) {}
	virtual void OnCleared(const wchar_t **filenames, int count) {}
	virtual void OnGetCloudStatus(const wchar_t *filename, HMENU *menu) {}
	virtual void OnProcessCloudStatus(int menu_item, int *result) {}

#define CBCLASS MLDBCallback
	START_DISPATCH_INLINE;
	CB(SYSCALLBACK_GETEVENTTYPE, GetEventType);
	CB(SYSCALLBACK_NOTIFY, notify);
	END_DISPATCH;
#undef CBCLASS
};