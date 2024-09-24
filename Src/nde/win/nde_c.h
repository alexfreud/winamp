#pragma once
/* C style API.

We'll eventually deprecate the C++ API as it presents a lot of linking challenges
*/
#include "../nde_defines.h" 
#include "../NDEString.h"
#include <bfc/platform/types.h>

#ifdef __cplusplus
extern "C" {
#endif
	typedef struct nde_database_struct_t { } *nde_database_t;
	typedef struct nde_table_struct_t { } *nde_table_t;
	typedef struct nde_scanner_t_struct_t { } *nde_scanner_t;
	typedef struct nde_field_t_struct_t { } *nde_field_t;
	typedef struct nde_filter_struct_t { } *nde_filter_t;
	
	/* Database functions */

	// Windows API
#ifdef __cplusplus
#define NDE_DEFAULT_PARAMETER(x) =x
#else
#define NDE_DEFAULT_PARAMETER(x)
#endif

NDE_API void NDE_Init();
NDE_API void NDE_Quit();
NDE_API nde_database_t NDE_CreateDatabase(HINSTANCE hInstance NDE_DEFAULT_PARAMETER(0)); 
NDE_API void NDE_DestroyDatabase(nde_database_t db);
NDE_API nde_table_t NDE_Database_OpenTable(nde_database_t db, const wchar_t *filename, const wchar_t *filename_index, int create, int cache);

NDE_API void NDE_Database_CloseTable(nde_database_t db, nde_table_t table);

/* Table functions */
NDE_API nde_field_t NDE_Table_NewColumn(nde_table_t table, unsigned char id, const char *name, unsigned char type);
NDE_API nde_field_t NDE_Table_NewColumnW(nde_table_t table, unsigned char id, const wchar_t *name, unsigned char type);
NDE_API void NDE_Table_PostColumns(nde_table_t table);
NDE_API void NDE_Table_AddIndexByID(nde_table_t table, unsigned char id, const char *name);
NDE_API void NDE_Table_AddIndexByIDW(nde_table_t table, unsigned char id, const wchar_t *name);
NDE_API nde_scanner_t NDE_Table_CreateScanner(nde_table_t table);
NDE_API void NDE_Table_DestroyScanner(nde_table_t table, nde_scanner_t scanner);
NDE_API void NDE_Table_Sync(nde_table_t table);
NDE_API void NDE_Table_Compact(nde_table_t table, int *progress NDE_DEFAULT_PARAMETER(0));
NDE_API int NDE_Table_GetRecordsCount(nde_table_t table);
NDE_API nde_field_t NDE_Table_GetColumnByID(nde_table_t table, unsigned char id);

NDE_API nde_field_t NDE_Table_GetColumnByName(nde_table_t table, const char *name);

NDE_API void NDE_Table_SetColumnSearchableByID(nde_table_t table, unsigned char id, int searchable);

/* Scanner functions */
NDE_API int NDE_Scanner_Query(nde_scanner_t scanner, const wchar_t *query);
NDE_API void NDE_Scanner_Search(nde_scanner_t scanner, const wchar_t *search_term);
NDE_API const wchar_t *NDE_Scanner_GetLastQuery(nde_scanner_t scanner);
NDE_API int NDE_Scanner_GetRecordsCount(nde_scanner_t scanner);
NDE_API void NDE_Scanner_New(nde_scanner_t scanner);
NDE_API void NDE_Scanner_Post(nde_scanner_t scanner);
NDE_API void NDE_Scanner_First(nde_scanner_t scanner, int *killswitch NDE_DEFAULT_PARAMETER(0));
NDE_API int NDE_Scanner_Next(nde_scanner_t scanner, int *killswitch NDE_DEFAULT_PARAMETER(0));
NDE_API void NDE_Scanner_Delete(nde_scanner_t scanner);
NDE_API void NDE_Scanner_Edit(nde_scanner_t scanner);
NDE_API int NDE_Scanner_EOF(nde_scanner_t scanner);
NDE_API int NDE_Scanner_BOF(nde_scanner_t scanner);
NDE_API nde_field_t NDE_Scanner_NewFieldByID(nde_scanner_t scanner, unsigned char id);
NDE_API nde_field_t NDE_Scanner_NewFieldByType(nde_scanner_t s, unsigned char type, unsigned char id);
NDE_API nde_field_t NDE_Scanner_NewFieldByName(nde_scanner_t scanner, const char *name);
NDE_API nde_field_t NDE_Scanner_GetFieldByID(nde_scanner_t scanner, unsigned char id);
NDE_API nde_field_t NDE_Scanner_GetFieldByName(nde_scanner_t scanner, const char *name);
NDE_API void NDE_Scanner_AddFilterByID(nde_scanner_t scanner, unsigned char id, nde_field_t field, unsigned char filter_operation);

NDE_API int NDE_Scanner_LocateInteger(nde_scanner_t scanner, unsigned char id, int from, int value,
																			int *nskip NDE_DEFAULT_PARAMETER(0), int compare_mode NDE_DEFAULT_PARAMETER(COMPARE_MODE_EXACT));
NDE_API int NDE_Scanner_LocateString(nde_scanner_t scanner, unsigned char id, int from, const wchar_t *value, 
																		 int *nskip NDE_DEFAULT_PARAMETER(0), int compare_mode NDE_DEFAULT_PARAMETER(COMPARE_MODE_EXACT));
NDE_API int NDE_Scanner_LocateNDEString(nde_scanner_t scanner, unsigned char id, int from, wchar_t *value,
																				int *nskip NDE_DEFAULT_PARAMETER(0), int compare_mode NDE_DEFAULT_PARAMETER(COMPARE_MODE_EXACT));
NDE_API int NDE_Scanner_LocateFilename(nde_scanner_t scanner, unsigned char id, int from, const wchar_t *value, 
																			 int *nskip NDE_DEFAULT_PARAMETER(0), int compare_mode NDE_DEFAULT_PARAMETER(COMPARE_MODE_EXACT));
NDE_API int NDE_Scanner_LocateNDEFilename(nde_scanner_t scanner, unsigned char id, int from, wchar_t *value, 
																					int *nskip NDE_DEFAULT_PARAMETER(0), int compare_mode NDE_DEFAULT_PARAMETER(COMPARE_MODE_EXACT));
NDE_API int NDE_Scanner_LocateField(nde_scanner_t scanner, unsigned char id, int from, nde_field_t field,
																		int *nskip NDE_DEFAULT_PARAMETER(0), int compare_mode NDE_DEFAULT_PARAMETER(COMPARE_MODE_EXACT));

NDE_API void NDE_Scanner_DeleteField(nde_scanner_t scanner, nde_field_t field);
NDE_API void NDE_Scanner_RemoveFilters(nde_scanner_t scanner);
typedef int (__cdecl *FieldEnumerator)(void *record, nde_field_t field, void *context);
NDE_API void NDE_Scanner_WalkFields(nde_scanner_t scanner, FieldEnumerator enumerator, void *context);
/* Filter functions */
NDE_API unsigned char NDE_Filter_GetID(nde_filter_t filter);
NDE_API unsigned char NDE_Filter_GetOp(nde_filter_t filter);
NDE_API nde_field_t NDE_Filter_GetData(nde_filter_t filter);

/* Field functions */
NDE_API unsigned char NDE_Field_GetType(nde_field_t field);
NDE_API unsigned char NDE_Field_GetID(nde_field_t field);

/* String Field functions */
NDE_API wchar_t *NDE_StringField_GetString(nde_field_t field); /* returns non-const because it's an NDE string (reference counted, see ndestring.h) */
NDE_API void NDE_StringField_SetNDEString(nde_field_t field, wchar_t *nde_string);
NDE_API void NDE_StringField_SetString(nde_field_t field, const wchar_t *str);

/* IntegerField functions */
NDE_API void NDE_IntegerField_SetValue(nde_field_t field, int value);
NDE_API int NDE_IntegerField_GetValue(nde_field_t field);
NDE_API nde_field_t NDE_IntegerField_Create(int value); /* mainly used for NDE_Scanner_AddFilterByID */

/* Int64Field functions */
NDE_API void NDE_Int64Field_SetValue(nde_field_t field, __int64 value);
NDE_API __int64 NDE_Int64Field_GetValue(nde_field_t field);
NDE_API nde_field_t NDE_Int64Field_Create(__int64 value);

/* BinaryField functions */
// on windows, the data pointer is optionally reference counted via ndestring (ndestring_retain if you plan on keeping it)
NDE_API void *NDE_BinaryField_GetData(nde_field_t field, size_t *length); 
NDE_API void NDE_BinaryField_SetData(nde_field_t field, const void *data, size_t length);

/* Int128Field functions */
NDE_API void NDE_Int128Field_SetValue(nde_field_t field, const void *value);

/* ColumnField functions */
NDE_API const wchar_t *NDE_ColumnField_GetFieldName(nde_field_t field);
NDE_API unsigned char NDE_ColumnField_GetDataType(nde_field_t field);
NDE_API unsigned char NDE_ColumnField_GetFieldID(nde_field_t);

NDE_API __time64_t NDE_Time_ApplyConversion(__time64_t value, const wchar_t *str, class TimeParse *tp); 

#ifdef __cplusplus
}
#endif