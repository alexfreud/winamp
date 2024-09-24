/* ---------------------------------------------------------------------------
 Nullsoft Database Engine
 --------------------
 codename: Near Death Experience
 --------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 
 Scanner Class Prototypes
 
 Android (linux) implementation

 --------------------------------------------------------------------------- */

#ifndef __SCANNER_H
#define __SCANNER_H

#include "../nde.h"
#include "record.h"
#include "../index.h"
#include <nu/Vector.h>
#include <nu/ValueSet.h>
#include "../LinkedList.h"
class Table;
class Index;

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
	bool iModified;
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
	bool CheckFilters(void);
	void CacheLastLocate(int Id, int From, Field *field, Index *i, int j);

	static int Query_LookupToken(const char *token);
	void Query_CleanUp(void);
	void Query_SyntaxError(int c);
public:
	static int Query_GetNextToken(const char *p, int *size, char **token, int tokentable=0);
	static const char *Query_EatSpace(const char *p);
	static char *Query_ProbeSpace(char *p);
	static const char *Query_ProbeNonAlphaNum(const char *p);
	static char *Query_ProbeAlphaNum(char *p);
	static int Query_isControlChar(char p);

	bool Query(const char *query);
	bool Query_Parse(const char *query);

	const char *GetLastQuery();

public://fucko: protected
	LinkedList pstack;
	char *token;
	char *last_query;
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
	bool Edition;
	int ResultPtr;
	bool FiltersOK;

public:
	bool MatchFilter(Filter *filter);
	typedef bool (*FilterWalker)(Scanner *scanner, Filter *filter, void *context);
	void WalkFilters(FilterWalker walker, void *context);

	ColumnField *GetColumnByName(const char *FieldName);
	ColumnField *GetColumnById(unsigned char id);

	Field *NewFieldByName(const char *fieldName, unsigned char Perm);
	Field *NewFieldById(unsigned char Id, unsigned char Perm);
	void DeleteField(Field *field);
	void DeleteFieldByName(const char *name); 
	void DeleteFieldById(unsigned char Id);

	void Cancel(void);
	void Insert(void);
	void Edit(void);
	void Post(void);
	void Delete(void); 

	Field *GetFieldByName(const char *FieldName);
	Field *GetFieldById(unsigned char Id);

	void First(int *killswitch=0);
	void Last(int *killswitch=0);
	int Next(int *killswitch=0);
	int Previous(int *killswitch=0);
	bool Eof(void);
	bool Bof(void);
	void New(void);
	int GetRecordsCount(void);
	void GetRecordById(int Id, bool checkFilters=true);
	int GetRecordId(void);
	void Sync(void);
	bool LocateByName(const char *column, int From, Field *field, int *nskip=NULL);
	bool LocateById(int Id, int From, Field *field, int *nskip=NULL);
	bool LocateByIdEx(int Id, int From, Field *field, int *nskip, int comp_mode);

	// Filters
	int AddFilterByName(const char *name, Field *Data, unsigned char Op);
	int AddFilterById(unsigned char Id, Field *Data, unsigned char Op);
	int AddFilterOp(unsigned char Op);
	void RemoveFilters(void);
	Filter *GetLastFilter(void);

	bool SetWorkingIndexByName(const char *desc);
	bool SetWorkingIndexById(unsigned char Id);

	void Search(const char *search_string);
	bool HasIndexChanged(void) { return iModified; }
	void ClearDirtyBit(void);
	float FragmentationLevel(void);

	Table *GetTable();
	int in_query_parser;
	int disable_date_resolution;
};

#endif
