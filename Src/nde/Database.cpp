/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 Database Class

--------------------------------------------------------------------------- */

#include "Database.h"
#include "Table.h"

//---------------------------------------------------------------------------
Database::Database()
{
#ifdef WIN32
	hInstance = (HINSTANCE)0;
#endif

}

#ifdef WIN32
//---------------------------------------------------------------------------
Database::Database(HINSTANCE hinst)
{
	hInstance = hinst;
}
#endif

//---------------------------------------------------------------------------
Database::~Database()
{
}

#ifdef WIN32
//---------------------------------------------------------------------------
void Database::SetInstance(HINSTANCE inst) {
	hInstance = inst;
}

HINSTANCE Database::GetInstance() {
	return hInstance;
}
#endif

//--------------------------------------------------------------------------
#ifdef _WIN32
Table *Database::OpenTable(const wchar_t *TableName, const wchar_t *IdxName, BOOL Create, BOOL Cached)
#else
Table *Database::OpenTable(const char *TableName, const char *IdxName, BOOL Create, BOOL Cached)
#endif
//char *tablefn, char*indexfn, BOOL create)
{
	Table *table = new Table(TableName, IdxName, Create, this, Cached);
	if (table)
	{
		if (table->Open())
			return table;
		table->Close();
		delete table;
	}
	return NULL;
}

//---------------------------------------------------------------------------
void Database::CloseTable(Table *table)
{
	if (table)
	{
		table->Close();
		delete table;
	}
}