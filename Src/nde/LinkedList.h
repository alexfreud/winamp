/* ---------------------------------------------------------------------------
 Nullsoft Database Engine
 --------------------
 codename: Near Death Experience
 --------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 
 Double-Linked List Class Prototypes
 
 --------------------------------------------------------------------------- */

#ifndef __LINKEDLIST_H
#define __LINKEDLIST_H

class LinkedListEntry
{
public:
	LinkedListEntry *Next;
	LinkedListEntry *Previous;
public:
	LinkedListEntry *GetNext() const;
	LinkedListEntry *GetPrevious() const;
	LinkedListEntry();
	virtual ~LinkedListEntry();
};

template <class T> class VListEntry : public LinkedListEntry 
{
public:
	void SetVal(T val) 
	{
		Val = val;
	}
	T GetVal(void) 
	{
		return Val;
	}
	
private:
	T Val;
};

template <class T> class PListEntry : public LinkedListEntry 
{
public:
	void SetVal(T *val) 
	{
		Val = val;
	}
	T *GetVal(void) 
	{
		return Val;
	}
	
private:
	T *Val;
};


typedef bool (*WalkListProc)(LinkedListEntry *Entry, int, void*, void*);

class LinkedList
{
protected:
	int NElements;
	LinkedListEntry *Head;
	LinkedListEntry *Foot;
	
public:
	LinkedList();
	~LinkedList();
	void AddEntry(LinkedListEntry *Entry, bool Cat);
	void RemoveEntry(LinkedListEntry *Entry);
	
	void WalkList(WalkListProc WalkProc, int ID, void *Data1, void *Data2);
	int GetNElements(void) { return NElements; }
	LinkedListEntry *GetHead(void) { return Head; }
	LinkedListEntry *GetFoot(void) { return Foot; }
};

#endif