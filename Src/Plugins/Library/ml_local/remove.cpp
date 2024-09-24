#include "main.h"
#include "api_mldb.h"

// returns 0 on success
// returns 1 on failure of either bad filename or invalid table
int RemoveFileFromDB(const wchar_t *filename)
{
	// From mldbApi
	int ret = 1;
	if (!g_table) openDb();
	if (filename && g_table)
	{
		// Issue wasabi callback for pre removal
		WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_REMOVED_PRE, (size_t)filename, 0);
		EnterCriticalSection(&g_db_cs);

		nde_scanner_t s = NDE_Table_CreateScanner(g_table);
		if (NDE_Scanner_LocateFilename(s, MAINTABLE_ID_FILENAME, FIRST_RECORD, filename))
		{
			NDE_Scanner_Delete(s);
			NDE_Scanner_Post(s);
			g_table_dirty++;
			ret = 0;
		}
		NDE_Table_DestroyScanner(g_table, s);
		LeaveCriticalSection(&g_db_cs);
		// Issue wasabi callback for post removal
		WASABI_API_SYSCB->syscb_issueCallback(api_mldb::SYSCALLBACK, api_mldb::MLDB_FILE_REMOVED_POST, (size_t)filename, 0);
	}
	return ret;
}
