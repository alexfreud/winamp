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

#include "record.h"
#include "Table.h"
#include "index.h"
#include "../nu/Vector.h"
#include "../nu/ValueSet.h"
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
	typedef Vector<StringField *> SearchStrings;
	SearchStrings search_strings;
	typedef ValueSet<unsigned char> SearchFields;
	SearchFields search_fields;
	bool search_any;

	void GetCurrentRecord(void);
	bool MatchFilters(void);
	bool MatchSearches();
	bool MatchSearch(const SearchFields &fields, StringField *search_field);
	//BOOL MatchJoins(void);
	int CheckFilters(void);
	void CacheLastLocate(int Id, int From, Field *field, Index *i, int j);

#ifdef _WIN32
	static int Query_LookupToken(const wchar_t *token);
#else
	static int Query_LookupToken(CFStringRef token);
#endif
	void Query_CleanUp(void);
	void Query_SyntaxError(int c);
public:
#ifdef _WIN32
	static int Query_GetNextToken(const wchar_t *p, int *size, wchar_t **token, int tokentable=0);
	static const wchar_t *Query_EatSpace(const wchar_t *p);
	static wchar_t *Query_ProbeSpace(wchar_t *p);
	static const wchar_t *Query_ProbeNonAlphaNum(const wchar_t *p);
	static wchar_t *Query_ProbeAlphaNum(wchar_t *p);
	static int Query_isControlChar(wchar_t p);

	BOOL Query(const wchar_t *query);
	BOOL Query_Parse(const wchar_t *query);
#else
	static int Query_GetNextToken(const char *p, size_t *size, CFStringRef *token, int tokentable=0);
	static const char *Query_EatSpace(const char *p);
	static char *Query_ProbeSpace(char *p);
	static const char *Query_ProbeNonAlphaNum(const char *p);
	static char *Query_ProbeAlphaNum(char *p);
	static int Query_isControlChar(char p);

	BOOL Query(const char *query);
	BOOL Query_Parse(const char *query);
#endif

#ifdef _WIN32
	const wchar_t *GetLastQuery();
#elif defined(__APPLE__)
	CFStringRef GetLastQuery();
#endif

public://fucko: protected
	//String token;
	LinkedList pstack;
#ifdef _WIN32
	wchar_t *token;
#else
	CFStringRef token;
#endif
	
#ifdef _WIN32
	wchar_t *last_query;
#elif defined(__APPLE__)
	CFStringRef last_query;
#endif
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

#ifdef _WIN32	
	ColumnField *GetColumnByName(const wchar_t *FieldName);
#else
	ColumnField *GetColumnByName(CFStringRef FieldName);
#endif
	ColumnField *GetColumnById(unsigned char id);

#ifdef _WIN32
	Field *NewFieldByName(const wchar_t *fieldName, unsigned char Perm);
#else
	Field *NewFieldByName(CFStringRef fieldName, unsigned char Perm);
#endif
	Field *NewFieldById(unsigned char Id, unsigned char Perm);
	void DeleteField(Field *field);
#ifdef _WIN32
	void DeleteFieldByName(const wchar_t *name); 
#else
	void DeleteFieldByName(CFStringRef name); 
#endif
	void DeleteFieldById(unsigned char Id);

	void Cancel(void);
	void Insert(void);
	void Edit(void);
	void Post(void);
	void Delete(void); 

#ifdef _WIN32
	Field *GetFieldByName(const wchar_t *FieldName);
#else
	Field *GetFieldByName(CFStringRef FieldName);
#endif
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
#ifdef _WIN32
	BOOL LocateByName(const wchar_t *column, int From, Field *field, int *nskip=NULL);
#else
	BOOL LocateByName(CFStringRef column, int From, Field *field, int *nskip=NULL);
#endif
	BOOL LocateById(int Id, int From, Field *field, int *nskip=NULL);
	BOOL LocateByIdEx(int Id, int From, Field *field, int *nskip, int comp_mode);

	// Filters
#ifdef _WIN32
	int AddFilterByName(const wchar_t *name, Field *Data, unsigned char Op);
#else
	int AddFilterByName(CFStringRef name, Field *Data, unsigned char Op);
#endif
	int AddFilterById(unsigned char Id, Field *Data, unsigned char Op);
	int AddFilterOp(unsigned char Op);
	void RemoveFilters(void);
	Filter *GetLastFilter(void);

#ifdef _WIN32
	BOOL SetWorkingIndexByName(const wchar_t *desc);
#else
	BOOL SetWorkingIndexByName(CFStringRef desc);
#endif
	BOOL SetWorkingIndexById(unsigned char Id);

#ifdef _WIN32
	void Search(const wchar_t *search_string);
#endif
	BOOL HasIndexChanged(void) { return iModified; }
	void ClearDirtyBit(void);
	float FragmentationLevel(void);

	Table *GetTable();
	int in_query_parser;
	int disable_date_resolution;
};

#endif
