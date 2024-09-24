#include "nde_c.h"
#include "nde.h"
#ifdef _WIN32
#include "../nu/AutoCharFn.h"
#include "../nu/AutoWide.h"
#endif

/* Database */
#ifdef _WIN32
nde_database_t NDE_CreateDatabase(HINSTANCE hInstance)
{
	return (nde_database_t)new Database(hInstance);
}
#else
nde_database_t NDE_CreateDatabase()
{
	return (nde_database_t)new Database();
}
#endif

void NDE_DestroyDatabase(nde_database_t db)
{
	delete (Database *)db;
}

#ifdef _WIN32
nde_table_t NDE_Database_OpenTable(nde_database_t db, const wchar_t *filename, const wchar_t *indexname, int create, int cache)
{
	Database *database = (Database *)db;
	if (database && filename)
		return (nde_table_t)database->OpenTable(filename, indexname, (BOOL)create, (BOOL)cache);
	else
		return 0;
}
#else
nde_table_t NDE_Database_OpenTable(nde_database_t db, const char *filename, const char *indexname, int create, int cache)
{
	Database *database = (Database *)db;
	if (database && filename)
		return (nde_table_t)database->OpenTable(filename, indexname, (BOOL)create, (BOOL)cache);
	else
		return 0;
}
#endif

void NDE_Database_CloseTable(nde_database_t db, nde_table_t t)
{
	Database *database = (Database *)db;
	Table *table = (Table *)t;
	if (database && table)
	{
		database->CloseTable(table);
	}
}
/* Table */

#ifdef _WIN32
nde_field_t NDE_Table_NewColumn(nde_table_t t, unsigned char id, const char *name, unsigned char type)
{
	Table *table = (Table *)t;
	if (table)
		return (nde_field_t)table->NewColumn(id, AutoWide(name), type, FALSE);
	else
		return 0;
}
#else
nde_field_t NDE_Table_NewColumn(nde_table_t t, unsigned char id, CFStringRef name, unsigned char type)
{
	Table *table = (Table *)t;
	if (table)
		return (nde_field_t)table->NewColumn(id, name, type, FALSE);
	else
		return 0;
}
#endif

void NDE_Table_PostColumns(nde_table_t t)
{
	Table *table = (Table *)t;
	if (table)
		table->PostColumns();
}

#ifdef _WIN32
void NDE_Table_AddIndexByID(nde_table_t t, unsigned char id, const char *name)
{
	Table *table = (Table *)t;
	if (table)
		table->AddIndexById(id, AutoWide(name));
}
#else
void NDE_Table_AddIndexByID(nde_table_t t, unsigned char id, CFStringRef name)
{
	Table *table = (Table *)t;
	if (table)
		table->AddIndexById(id, name);
}
#endif

nde_scanner_t NDE_Table_CreateScanner(nde_table_t t)
{
	Table *table = (Table *)t;
	if (table)
		return (nde_scanner_t)table->NewScanner();
	else
		return 0;
}

void NDE_Table_DestroyScanner(nde_table_t t, nde_scanner_t s)
{
	Table *table = (Table *)t;
	Scanner *scanner = (Scanner *)s;
	if (table && scanner)
		table->DeleteScanner(scanner);
}

void NDE_Table_Sync(nde_table_t t)
{
	Table *table = (Table *)t;
	if (table)
		table->Sync();
}

#ifdef _WIN32
void NDE_Table_Compact(nde_table_t t, int *progress)
{
	Table *table = (Table *)t;
	if (table)
		table->Compact(progress);
}
#endif

int NDE_Table_GetRecordsCount(nde_table_t t)
{
	Table *table = (Table *)t;
	if (table)
		return table->GetRecordsCount();
	else
	return 0;
}

nde_field_t NDE_Table_GetColumnByID(nde_table_t t, unsigned char id)
{
	Table *table = (Table *)t;
	if (table)
		return (nde_field_t)table->GetColumnById(id);
	else
		return 0;
}

#ifdef _WIN32
nde_field_t NDE_Table_GetColumnByName(nde_table_t t, const char *name)
{
	Table *table = (Table *)t;
	if (table && name)
		return (nde_field_t)table->GetColumnByName(AutoWide(name));
	else
		return 0;
}
#else
nde_field_t NDE_Table_GetColumnByName(nde_table_t t, CFStringRef name)
{
	Table *table = (Table *)t;
	if (table && name)
		return (nde_field_t)table->GetColumnByName(name);
	else
		return 0;
}
#endif

void NDE_Table_SetColumnSearchableByID(nde_table_t t, unsigned char id, int searchable)
{
	Table *table = (Table *)t;
	if (table)
		table->SetFieldSearchableById(id, !!searchable);
}
/* Scanner */
#ifdef _WIN32
int NDE_Scanner_Query(nde_scanner_t s, const wchar_t *query)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		return scanner->Query(query);
	else
		return 0;
}

void NDE_Scanner_Search(nde_scanner_t s, const wchar_t *search_term)
{
		Scanner *scanner = (Scanner *)s;
	if (scanner)
		scanner->Search(search_term);
}

const wchar_t *NDE_Scanner_GetLastQuery(nde_scanner_t s)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		return scanner->GetLastQuery();
	else
		return 0;
}
#elif defined(__APPLE__)
int NDE_Scanner_Query(nde_scanner_t s, const char *query)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		return scanner->Query(query);
	else
		return 0;
}

CFStringRef NDE_Scanner_GetLastQuery(nde_scanner_t s)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		return scanner->GetLastQuery();
	else
		return 0;
}
#endif

int NDE_Scanner_GetRecordsCount(nde_scanner_t s)
{
		Scanner *scanner = (Scanner *)s;
	if (scanner)
		return scanner->GetRecordsCount();
	else
		return 0;
}

void NDE_Scanner_New(nde_scanner_t s)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		scanner->New();
}

void NDE_Scanner_Post(nde_scanner_t s)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		scanner->Post();
}

void NDE_Scanner_First(nde_scanner_t s, int *killswitch)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		scanner->First(killswitch);
}

void NDE_Scanner_Delete(nde_scanner_t s)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		scanner->Delete();
}

void NDE_Scanner_Edit(nde_scanner_t s)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		scanner->Edit();
}

int NDE_Scanner_EOF(nde_scanner_t s)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		return scanner->Eof();
	else
		return 1;
}

int NDE_Scanner_BOF(nde_scanner_t s)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		return scanner->Bof();
	else
		return 1;
}

nde_field_t NDE_Scanner_NewFieldByID(nde_scanner_t s, unsigned char id)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		return (nde_field_t)scanner->NewFieldById(id, PERM_READWRITE);
	else
		return 0;
}

#ifdef _WIN32
nde_field_t NDE_Scanner_NewFieldByName(nde_scanner_t s, const char *name)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		return (nde_field_t)scanner->NewFieldByName(AutoWide(name), PERM_READWRITE);
	else
		return 0;
}
#else
nde_field_t NDE_Scanner_NewFieldByName(nde_scanner_t s, CFStringRef name)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		return (nde_field_t)scanner->NewFieldByName(name, PERM_READWRITE);
	else
		return 0;
}
#endif

nde_field_t NDE_Scanner_GetFieldByID(nde_scanner_t s, unsigned char id)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		return (nde_field_t)scanner->GetFieldById(id);
	else
		return 0;
}

#ifdef _WIN32
nde_field_t NDE_Scanner_GetFieldByName(nde_scanner_t s, const char *name)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner && name)
		return (nde_field_t)scanner->GetFieldByName(AutoWide(name));
	else
		return 0;
}
#else
nde_field_t NDE_Scanner_GetFieldByName(nde_scanner_t s, CFStringRef name)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner && name)
		return (nde_field_t)scanner->GetFieldByName(name);
	else
		return 0;
}
#endif

void NDE_Scanner_AddFilterByID(nde_scanner_t s, unsigned char id, nde_field_t f, unsigned char filter_operation)
{
	Scanner *scanner = (Scanner *)s;
	Field *field = (Field *)f;
	if (scanner && field)
		scanner->AddFilterById(id, field, filter_operation);
}

int NDE_Scanner_LocateInteger(nde_scanner_t s, unsigned char id, int from, int value, int *nskip, int compare_mode)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
	{
		IntegerField field(value);
		return scanner->LocateByIdEx(id, from, &field, nskip, compare_mode);
	}
	else
		return 0;
}

#ifdef _WIN32
int NDE_Scanner_LocateString(nde_scanner_t s, unsigned char id, int from, const wchar_t *value, int *nskip, int compare_mode)
{
		Scanner *scanner = (Scanner *)s;
	if (scanner)
	{
		StringField field(value);
		return scanner->LocateByIdEx(id, from, &field, nskip, compare_mode);
	}
	else
		return 0;
}


int NDE_Scanner_LocateNDEString(nde_scanner_t s, unsigned char id, int from, wchar_t *value, int *nskip, int compare_mode)
{
		Scanner *scanner = (Scanner *)s;
	if (scanner)
	{
		StringField field(value, STRING_IS_NDESTRING);
		return scanner->LocateByIdEx(id, from, &field, nskip, compare_mode);
	}
	else
		return 0;
}
#endif

#ifdef _WIN32
int NDE_Scanner_LocateFilename(nde_scanner_t s, unsigned char id, int from, const wchar_t *value, int *nskip, int compare_mode)
{
		Scanner *scanner = (Scanner *)s;
	if (scanner)
	{
		FilenameField field(value);
		return scanner->LocateByIdEx(id, from, &field, nskip, compare_mode);
	}
	else
		return 0;
}

int NDE_Scanner_LocateNDEFilename(nde_scanner_t s, unsigned char id, int from, wchar_t *value, int *nskip, int compare_mode)
{
		Scanner *scanner = (Scanner *)s;
	if (scanner)
	{
		FilenameField field(value, STRING_IS_NDESTRING);
		return scanner->LocateByIdEx(id, from, &field, nskip, compare_mode);
	}
	else
		return 0;
}
#endif

int NDE_Scanner_LocateField(nde_scanner_t s, unsigned char id, int from, nde_field_t f, int *nskip, int compare_mode)
{
	Scanner *scanner = (Scanner *)s;
	Field *field = (Field *)f;
	if (scanner && field)
	{
		return scanner->LocateByIdEx(id, from, field, nskip, compare_mode);
	}
	else
		return 0;
}

void NDE_Scanner_DeleteField(nde_scanner_t s, nde_field_t f)
{
	Scanner *scanner = (Scanner *)s;
	Field *field = (Field *)f;
	if (scanner && field)
	{
		scanner->DeleteField(field);
	}
}

void NDE_Scanner_RemoveFilters(nde_scanner_t s)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		scanner->RemoveFilters();
}

int NDE_Scanner_Next(nde_scanner_t s, int *killswitch)
{
	Scanner *scanner = (Scanner *)s;
	if (scanner)
		return scanner->Next(killswitch);
	else
		return 0;
}
/* Filter functions */
unsigned char NDE_Filter_GetID(nde_filter_t f)
{
	Filter *filter = (Filter *)f;
	if (filter)
		return filter->GetId();
	else
		return FILTERS_INVALID; // right value but I'm not sure if it's the best constant name to use
}

NDE_API unsigned char NDE_Filter_GetOp(nde_filter_t f)
{
	Filter *filter = (Filter *)f;
	if (filter)
		return filter->GetOp();
	else
		return FILTERS_INVALID; // right value but I'm not sure if it's the best constant name to use
}

NDE_API nde_field_t NDE_Filter_GetData(nde_filter_t f)
{
	Filter *filter = (Filter *)f;
	if (filter)
		return (nde_field_t)filter->Data();
	else
		return 0;
}

/* Field functions */
unsigned char NDE_Field_GetType(nde_field_t f)
{
	Field *field = (Field *)f;
	if (field)
		return field->GetType();
	else
		return FIELD_UNKNOWN;
}

unsigned char NDE_Field_GetID(nde_field_t f)
{
	Field *field = (Field *)f;
	if (field)
		return field->GetFieldId();
	else
		return FIELD_UNKNOWN;
}

/* StringField functions */
#ifdef _WIN32
void NDE_StringField_SetNDEString(nde_field_t f, wchar_t *nde_string)
{
	StringField *field = (StringField *)(Field *)f;
	if (field)
		field->SetNDEString(nde_string);
}

wchar_t *NDE_StringField_GetString(nde_field_t f)
{
	StringField *field = (StringField *)(Field *)f;
	if (field)
		return field->GetStringW();
	else
		return 0;
}

#ifdef _WIN32
void NDE_StringField_SetString(nde_field_t f, const wchar_t *str)
{
	StringField *field = (StringField *)(Field *)f;
	if (field)
		field->SetStringW(str);
}
#endif

#elif defined(__APPLE__)
void NDE_StringField_SetString(nde_field_t f, CFStringRef nde_string)
{
	StringField *field = (StringField *)(Field *)f;
	if (field)
		field->SetNDEString(nde_string);
}

CFStringRef NDE_StringField_GetString(nde_field_t f)
{
	StringField *field = (StringField *)(Field *)f;
	if (field)
		return field->GetString();
	else
		return 0;
}
#endif


/* IntegerField functions */
void NDE_IntegerField_SetValue(nde_field_t f, int value)
{
	IntegerField *field = (IntegerField *)(Field *)f;
	if (field)
		field->SetValue(value);
}

int NDE_IntegerField_GetValue(nde_field_t f)
{
	IntegerField *field = (IntegerField *)(Field *)f;
	if (field)
		return field->GetValue();
	else
		return 0;
}

nde_field_t NDE_IntegerField_Create(int value)
{
	return (nde_field_t)new IntegerField(value);
}

/* BinaryField */
void *NDE_BinaryField_GetData(nde_field_t f, size_t *length)
{
	BinaryField *field = (BinaryField *)(Field *)f;
	if (field)
		return (void *)field->GetData(length);
	else
		return 0;
}
#ifdef __APPLE__
CFDataRef NDE_BinaryField_GetCFData(nde_field_t f)
{
		BinaryField *field = (BinaryField *)(Field *)f;
	if (field)
		return field->GetCFData();
	else
		return 0;
}
#endif

void NDE_BinaryField_SetData(nde_field_t f, const void *data, size_t length)
{
	BinaryField *field = (BinaryField *)(Field *)f;
	if (field)
		field->SetData((const uint8_t *)data, length);
}

/* Int128Field */
void NDE_Int128Field_SetValue(nde_field_t f, const void *value)
{
	Int128Field *field = (Int128Field *)(Field *)f;
	if (field && value)
		field->SetValue(value);
}

/* ColumnField */
#ifdef _WIN32
const wchar_t *NDE_ColumnField_GetFieldName(nde_field_t f)
{
	ColumnField *field = (ColumnField *)(Field *)f;
	if (field)
		return field->GetFieldName();
	else
		return 0;
}
#else
CFStringRef NDE_ColumnField_GetFieldName(nde_field_t f)
{
	ColumnField *field = (ColumnField *)(Field *)f;
	if (field)
		return field->GetFieldName();
	else
		return 0;
}
#endif

unsigned char NDE_ColumnField_GetDataType(nde_field_t f)
{
	ColumnField *field = (ColumnField *)(Field *)f;
	if (field)
		return field->GetDataType();
	else
		return FIELD_UNKNOWN;
}

#ifdef _WIN32
__time64_t NDE_Time_ApplyConversion(__time64_t value, const wchar_t *str, class TimeParse *tp)
{
	IntegerField f(value);
	f.ApplyConversion(str, tp);
	return f.GetValue();
}
#endif