/* ---------------------------------------------------------------------------
 Nullsoft Database Engine
 --------------------
 codename: Near Death Experience
 --------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 
 Scanner Class Prototypes
 
 --------------------------------------------------------------------------- */

#ifndef __SCANNER_H
#define __SCANNER_H

#include <vector>
#include "record.h"
#include "Table.h"
#include "index.h"
#include <vector>
#include <set>
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif
class Table;
class Index;
#pragma warning( disable: 4251 )
class Scanner;
class Record;
class Scanner : public LinkedListEntry 
{
public:
	Record *GetRecord(int Idx);
	Scanner(Table *parentTable);
	void IndexModified(void);
	Index *GetIndex() { return index; }
	Index *index; // TODO: make protected

protected:
	~Scanner();

	Table *pTable;
	BOOL iModified;
	typedef std::vector<StringField *> SearchStrings;
	SearchStrings search_strings;
	typedef std::set<unsigned char> SearchFields;
	SearchFields search_fields;
	bool search_any;

	void GetCurrentRecord(void);
	bool MatchFilters(void);
	bool MatchSearches();
	bool MatchSearch(const SearchFields &fields, StringField *search_field);
	//BOOL MatchJoins(void);
	int CheckFilters(void);
	void CacheLastLocate(int Id, int From, Field *field, Index *i, int j);

	static int Query_LookupToken(const wchar_t *token);
	void Query_CleanUp(void);
	void Query_SyntaxError(int c);
public:
	static int Query_GetNextToken(const wchar_t *p, int *size, wchar_t **token, int tokentable=0);
	static const wchar_t *Query_EatSpace(const wchar_t *p);
	static wchar_t *Query_ProbeSpace(wchar_t *p);
	static const wchar_t *Query_ProbeNonAlphaNum(const wchar_t *p);
	static wchar_t *Query_ProbeAlphaNum(wchar_t *p);
	static int Query_isControlChar(wchar_t p);

	BOOL Query(const wchar_t *query);
	BOOL Query_Parse(const wchar_t *query);

	const wchar_t *GetLastQuery();

public://fucko: protected
	LinkedList pstack;
	wchar_t *token;
	wchar_t *last_query;
	int last_query_failed;

protected:
	Record *CurrentRecord;
	int CurrentRecordIdx;
	LinkedList FilterList;
	Index *lastLocateIndex;
	int lastLocateIdx;
	Field *lastLocateFieldClone;
	int lastLocateFrom;
	int lastLocateId;
	BOOL Edition;
	int ResultPtr;
	BOOL FiltersOK;

public:
	bool MatchFilter(Filter *filter);
	typedef bool (*FilterWalker)(Scanner *scanner, Filter *filter, void *context);
	void WalkFilters(FilterWalker walker, void *context);

	ColumnField *GetColumnByName(const wchar_t *FieldName);
	ColumnField *GetColumnById(unsigned char id);

	Field *NewFieldByName(const wchar_t *fieldName, unsigned char Perm);
	Field *NewFieldByType(unsigned char Type, unsigned char Id, unsigned char Perm);
	Field *NewFieldById(unsigned char Id, unsigned char Perm);
	void DeleteField(Field *field);
	void DeleteFieldByName(const wchar_t *name); 
	void DeleteFieldById(unsigned char Id);

	void Cancel(void);
	void Insert(void);
	void Edit(void);
	void Post(void);
	void Delete(void); 

	Field *GetFieldByName(const wchar_t *FieldName);
	Field *GetFieldById(unsigned char Id);

	void First(int *killswitch=0);
	void Last(int *killswitch=0);
	int Next(int *killswitch=0);
	int Previous(int *killswitch=0);
	BOOL Eof(void);
	BOOL Bof(void);
	void New(void);
	int GetRecordsCount(void);
	void GetRecordById(int Id, BOOL checkFilters=TRUE);
	int GetRecordId(void);
	void Sync(void);
	BOOL LocateByName(const wchar_t *column, int From, Field *field, int *nskip=NULL);
	BOOL LocateById(int Id, int From, Field *field, int *nskip=NULL);
	BOOL LocateByIdEx(int Id, int From, Field *field, int *nskip, int comp_mode);

	// Filters
	int AddFilterByName(const wchar_t *name, Field *Data, unsigned char Op);
	int AddFilterById(unsigned char Id, Field *Data, unsigned char Op);
	int AddFilterOp(unsigned char Op);
	void RemoveFilters(void);
	Filter *GetLastFilter(void);

	BOOL SetWorkingIndexByName(const wchar_t *desc);
	BOOL SetWorkingIndexById(unsigned char Id);

	void Search(const wchar_t *search_string);
	BOOL HasIndexChanged(void) { return iModified; }
	void ClearDirtyBit(void);
	float FragmentationLevel(void);

	void WalkFields(Record::FieldsWalker callback, void *context);

	Table *GetTable();
	int in_query_parser;
	int disable_date_resolution;
};

#endif
