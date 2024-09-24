/* ---------------------------------------------------------------------------
                           Nullsoft Database Engine
                             --------------------
                        codename: Near Death Experience
--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 Raw Index Class Prototypes

--------------------------------------------------------------------------- */

#ifndef __RAWINDEX_H
#define __RAWINDEX_H

#include <stdio.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include "vfs.h"
#include "Table.h"

class IndexField;
#define BLOCK_SIZE 2048 // 8192 entries blocks
class RecordBase;
class Index
{
public:
	Index(VFILE *Handle, unsigned char id, int pos, int type, bool newindex, int nentries, Table *parentTable);
	~Index();
	int Get(int Idx);
	void Set(int Idx, int P);

	void LoadIndex(bool newindex);
	void WriteIndex(void);
	int Insert(Index *parindex, int N, bool localonly);
	int Insert(int N);
	int Update(int Idx, int Pos, RecordBase *record, bool localonly);
	int Update(Index *parindex, int paridx, int Idx, int Pos, RecordBase *record, bool forceLast, bool localonly);
	unsigned char GetId();

	int FindSortedPlace(Field *field, int idx, int *laststate, int start);
	int FindSortedPlaceEx(Field *field, int idx, int *laststate, int start, int comp_mode);
	int MoveIndex(int idx, int newidx);
	void Colaborate(IndexField *secindex);
	void SetCooperative(int Idx, int secpos);
	int GetCooperative(int Idx);
	void UpdateMe(Index *Me, int newidx, int oldidx);
	Field *QuickFindField(unsigned char Id, int Pos);
	int QuickFind(int Id, Field *field, int start);
	int QuickFindEx(int Id, Field *field, int start, int comp_mode);
	int TranslateIndex(int Pos, Index *index);
	void Delete(int Idx, int Pos, Record *record);
	void Shrink(void);

	void Propagate(void);
	void SetGlobalLocateUpToDate(bool isUptodate);
	int NeedFix();

public:
	// TODO: make these protected
	int NEntries;
	IndexField *SecIndex;
	bool locateUpToDate;

protected:
	VFILE *Handle;
	VFILE *TableHandle;
	Table *pTable;

	int *IndexTable;
	size_t MaxSize;
	unsigned char Id;

	bool InChain;
	int Position;
	bool InInsert;
	int InChainIdx;
};

#endif