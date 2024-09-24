#ifndef _PTRLIST_H
#define _PTRLIST_H

#include <bfc/std_math.h>
//#include <bfc/memblock.h>
#include <bfc/bfc_assert.h>

#include <bfc/platform/platform.h>

//#include <wasabicfg.h>

#ifdef _DEBUG
extern int ptrlist_totalnitems;
#endif

// Disable the "identifier was truncated to '255' characters..." warning.
#pragma warning( disable : 4786 )

/*
 
a generic pointer list template. only takes up 12 bytes when empty. auto-grows
the array as necessary, NEVER shrinks it unless you removeAll() or equivalent
 
Use, PtrList<typename>, never PtrListRoot
 
*/

// yes, this really should be an enum or something
#define PTRLIST_POS_LAST -1

// 4k each, leaving 16 bytes for MALLOC overhead
//#define DEF_PTRLIST_INCREMENT (4096/sizeof(void*)-16)

// in average, 4k doubles the VM working set of a skin, 128bytes (32 elements) seems most optimal
#define DEF_PTRLIST_INCREMENT (128)

class __foreach;

// base class, to share as much code as possible
class NOVTABLE PtrListRoot
{
	friend class __foreach;
protected:
	PtrListRoot(int initial_size = 0);
	PtrListRoot(const PtrListRoot *from);
	virtual ~PtrListRoot();

	void copyFrom(const PtrListRoot *from);
	void appendFrom(const PtrListRoot *from);

	void setMinimumSize(int nslots);	// expand to at least this many slots
	int getNumItems() const;
	int size() const                                 { return nitems; }
	bool empty()                                     { return nitems == 0; }

	void *enumItem(int n) const;

	void moveItem(int from, int to);

	int removeItem(void *item);
	void removeEveryItem(void *item);
	void removeByPos(int pos);
	void removeLastItem();
	void removeAll();
	void freeAll();
	void purge();

	// gross-ass linear search to find index of item
	// note that PtrListSorted provides a binary search version
	int searchItem(void *item) const;

#if 0//fuct
	// speedy binary search. although it'll be fuct if it's not sorted right
	int bsearchItem(void *item) const;
#endif

	void *addItem(void *item, int pos, int inc);
	void *setItem(void *item, int pos); // replace what's in the slot with the new value

	void reverse();

	void **getItemList() const                       { return items; } // try to avoid! this is inline to make q() fast

private:
#undef verify // for Mac
	void verify();

	int nitems, nslots;
	void **items;
};

// now we add the methods that refer specifically to the pointer type
template <class T>
class PtrList : public PtrListRoot
{
	friend class __foreach;
public:
	PtrList(int initial_size = 0)                    {}
	PtrList(const PtrList<T> &r)                     { copyFrom(&r); }
	PtrList(const PtrList<T> *r)                     { copyFrom(r); }
	~PtrList() {}

	// copy another PtrList
	// deletes previous contents
	void copyFrom(const PtrList<T> *from)            { PtrListRoot::copyFrom(from); }

	// append contents of another PtrList to the end of this one
	// preserves previous contents
	void appendFrom(const PtrList<T> *from)          { PtrListRoot::appendFrom(from); }

	// adding

	// expand freelist to at least this many slots, even if 0 items in list
	void setMinimumSize(int nslots)                  { PtrListRoot::setMinimumSize(nslots); }

	// provide a public addItem for the pointer type
	T *addItem(const T *item, int pos = PTRLIST_POS_LAST, int inc = DEF_PTRLIST_INCREMENT)
	{
		return static_cast<T *>(PtrListRoot::addItem(const_cast<T*>(item), pos, inc));
	}

	void push_back(const T* item)                    { return addItem(item); }
	
	// replace what's in the slot with the new value
	T *setItem(const T *item, int pos)               { return static_cast<T *>(PtrListRoot::setItem(const_cast<T*>(item), pos)); }

	// reverse the order of the list in place
	void reverse()                                   { PtrListRoot::reverse(); }

	// enumerating

	// returns # of items in list
	int getNumItems() const                          { return PtrListRoot::getNumItems(); }

	// basic list enumerator. returns NULL for out of bounds
	T *enumItem(int n) const                         { return static_cast<T *>(PtrListRoot::enumItem(n)); }
	T *operator[](int n) const                       { return enumItem(n); }

	// this will safely return NULL if 0 items due to enumItems's boundscheck
	T *getFirst() const                              { return enumItem(0); }
	T *getLast() const                               { return enumItem(getNumItems() - 1); }

	// this is a NON-BOUNDS-CHECKING lookup
	T *q(int n)                                      { return static_cast<T*>(getItemList()[n]); }

	// gross-ass linear search to find index of item
	// note that PtrListSorted provides a binary search version
	int searchItem(T *item) const                    { return PtrListRoot::searchItem(item); }
	int haveItem(T *item) const                      { return searchItem(item) >= 0; }

	// deleteing

	// removes first instance of a pointer in list, returns how many are left
	int removeItem(T *item)                          { return PtrListRoot::removeItem(item); }
	
	//DEPRECATED
	int delItem(T *item)                             { return removeItem(item); }
	
	// removes all instances of this pointer
	void removeEveryItem(const T *item)              { PtrListRoot::removeEveryItem(const_cast<T*>(item)); }	
	// removes pointer at specified position regardless of value
	void removeByPos(int pos)                        { PtrListRoot::removeByPos(pos); }	
	// DEPRECATED
	void delByPos(int pos)                           { removeByPos(pos); }
	// removes last item
	void removeLastItem()                            { PtrListRoot::removeLastItem(); }
	// removes all entries, also deletes memory space
	void removeAll()                                 { PtrListRoot::removeAll(); }
	// removes all entries, calling FREE on the pointers
	void freeAll()                                   { PtrListRoot::freeAll(); }
	// removes all entries, calling delete on the pointers
	void deleteAll()
	{
		int i, nitems = getNumItems();
		for (i = 0; i < nitems; i++)
			delete enumItem(i);
		removeAll();
	}
	// removes all entries, calling delete on the pointers
	// FG>this version removes each entry as it deletes it so if
	// one of the object uses this list in its destructor, it will
	// still work. It is MUCH slower than deleteAll tho.
	void deleteAllSafe()
	{
		//CUT    ASSERT(!(nitems != 0 && items == NULL));
		while (getNumItems())
		{
			T *i = enumItem(0);
			delete i;
			removeItem(i);
		}
	}
	void deleteItem(int item)
	{
		if (item < getNumItems())
		{
			deleteItem(enumItem(item));
		}
	}
	void deleteItem(T *item)
	{
		delete item;
		removeItem(item);
	}

	void moveItem(int from, int to) { PtrListRoot::moveItem(from, to); }

	static T *castFor(void *ptr) { return static_cast<T*>(ptr); }

	using PtrListRoot::purge;

protected:
	T **getItemList()
	{
		return reinterpret_cast<T **>(PtrListRoot::getItemList());
	}
};

class NotSorted
{
public:
	// comparator for searching -- override
	static int compareAttrib(const wchar_t *attrib, void *item) { return 0; }
	// comparator for sorting -- override	, -1 p1 < p2, 0 eq, 1 p1 > p2
	static int compareItem(void *p1, void* p2) { return CMP3(p1, p2); }
};

//template <class T, class C> class NoSort {
//  static void _sort(T **, int) {}
//};

// a base class to sort the pointers
// you must implement the comparisons (C) and the sort algorithm (S)
template < class T, class C, class S >
class PtrListSorted : public PtrList<T>
{
public:
	PtrListSorted(int initial_size = 0) : PtrList<T>(initial_size)
	{
		need_sorting = 0;
		auto_sort = true;
		dups_low = dups_hi = dups_pos = 0;
	}

	void copyFrom(const PtrList<T> *from)
	{
		PtrList<T>::copyFrom(from);
		need_sorting = 1;
		if (auto_sort) sort();
	}

	T *addItem(T *item, int pos = PTRLIST_POS_LAST, int inc = DEF_PTRLIST_INCREMENT)
	{
#if 1
		// check for appending in sorted order
		if (pos == PTRLIST_POS_LAST && !need_sorting && auto_sort)
		{
			int n = PtrList<T>::getNumItems();
			if (n > 0 && C::compareItem(item, q(n - 1)) < 0) need_sorting = 1;
		}
		else
#endif
			need_sorting = 1;
		return PtrList<T>::addItem(item, pos, inc);
	}

	void sort(bool force_sort = false)
	{
		if (need_sorting || force_sort)
			S::_sort(PtrList<T>::getItemList(), PtrList<T>::getNumItems());
		need_sorting = 0;
	}

	T *enumItem(int n)
	{	// NOT const since we might call sort()
		if (auto_sort) sort();
		return PtrList<T>::enumItem(n);
	}
T *operator[](int n) { return PtrListSorted<T, C, S>::enumItem(n); }

	T *q(int n)
	{
		if (auto_sort) sort();
		return static_cast<T*>(PtrList<T>::getItemList()[n]);
	}

	T *findItem(const wchar_t *attrib, int *pos = NULL)
	{
		ASSERTPR(!(!auto_sort && need_sorting), "must call sort() first if auto-sorting is disabled");
		sort();
#if 1	// do binary search
		if (PtrList<T>::getNumItems() == 0) return NULL;

		int bot = 0, top = PtrList<T>::getNumItems() - 1, mid;

		for (int c = 0; c < PtrList<T>::getNumItems() + 1; c++)
		{
			if (bot > top) return NULL;
			mid = (bot + top) / 2;
			int r = C::compareAttrib(attrib, PtrList<T>::getItemList()[mid]);
			if (r == 0)
			{
				if (pos != NULL) *pos = mid;
				return PtrList<T>::getItemList()[mid];
			}
			if (r < 0)
			{
				top = mid - 1;
			}
			else
			{
				bot = mid + 1;
			}
		}
		ASSERTPR(0, "binary search fucked up");
#else
		// re-enable this in case of fuckup
		for (int i = 0; i < nitems; i++)
		{
			if (C::compareAttrib(attrib, static_cast<T *>(items[i])) == 0)
				return static_cast<T *>items[i];
		}
#endif
		return NULL;
	}

	T *findItem(T *attrib, int *pos = NULL)
	{
		return findItem((const wchar_t *)attrib, pos);
	}

	int beginEnumDups(const char *attrib)
	{
		int pos;
		findItem(attrib, &pos);
		
		if (pos < 0)
			return -1;
		
		dups_hi = pos;
		dups_low = pos;

		int i;
		for (i = pos - 1;i >= 0;i--)
		{
			if (C::compareAttrib(attrib, static_cast<T *>(enumItem(i))) == 0)
				break;
			
			dups_low = i;
		}
		
		for (i = pos + 1;i < PtrList<T>::getNumItems();i++)
		{
			if (C::compareAttrib(attrib, static_cast<T *>(enumItem(i))) == 0)
				break;
			
			dups_hi = i;
		}
		
		dups_pos = dups_low;
		
		return dups_pos;
	}
	
	int getNextDup()
	{ // returns -1 when done
		if (dups_pos >= dups_hi)
			return -1;
		
		return ++dups_pos;
	}

#if 0
	// replace search with binary search
	int searchItem(T *item) const
	{
		ASSERTPR(!(!auto_sort && need_sorting), "must call sort() first if auto-sorting is disabled");
		sort();
		return bsearchItem(item);
	}
#endif

	void setAutoSort(bool as)                        { auto_sort = as; }
	bool getAutoSort() const                         { return auto_sort; }

	void removeDups()
	{
		ASSERTPR(!(!auto_sort && need_sorting), "must call sort() first if auto-sorting is disabled");
		sort();
		for (int i = 1; i < PtrList<T>::getNumItems(); i++)
		{
			if (C::compareItem(enumItem(i - 1), enumItem(i)) == 0)
			{
				PtrList<T>::delByPos(i);
				i--;
			}
		}
	}

private:
	int need_sorting;
	bool auto_sort;
	int dups_low, dups_hi, dups_pos;
};

// quicksort -- you still need to override the compare fns
template <class T, class C>
class QuickSorted
{
public:
	static void _sort(T **items, int nitems)
	{
		if (items == NULL || nitems <= 1)
			return ;
		
		Qsort(items, 0, nitems - 1);
	}

private:
	static void swapItem(T **items, int a, int b)
	{ // no bounds checking!
		T *tmp = items[a];
		items[a] = items[b];
		items[b] = tmp;
	}

	static void Qsort(T **items, int lo0, int hi0)
	{
		int lo = lo0, hi = hi0;
		if (hi0 > lo0)
		{
			T *mid = items[(lo0 + hi0) / 2];
			while (lo <= hi)
			{
				while ((lo < hi0) && (C::compareItem(items[lo], mid) < 0))
					lo++;
				
				while ((hi > lo0) && (C::compareItem(items[hi], mid) > 0))
					hi--;

				if (lo <= hi)
				{
					swapItem(items, lo, hi);
					lo++;
					hi--;
				}
			}

			if (lo0 < hi)
				Qsort(items, lo0, hi);
			
			if (lo < hi0)
				Qsort(items, lo, hi0);
		}
	}
};

// easy way to specify quicksorting, just data type and comparison class
template <class T, class C> class PtrListQuickSorted : public PtrListSorted<T, C, QuickSorted<T, C> >
{
public:
	PtrListQuickSorted(int initial_size = 0) : PtrListSorted<T, C, QuickSorted<T, C> >(initial_size) {}
};


// easy way to get a list sorted by pointer val
class SortByPtrVal
{
public:
	static int compareItem(void *p1, void *p2)                     { return CMP3(p1, p2); }
	static int compareAttrib(const wchar_t *attrib, void *item)    { return CMP3((void *)attrib, item); }
};

template <class T> class PtrListQuickSortedByPtrVal : public PtrListQuickSorted<T, SortByPtrVal > {};


// this class automatically inserts at the correct position, so
// the binary searches are very fast if you need to insert and search often (no need to sort)
template < class T, class C >
class PtrListInsertSorted : public PtrList<T>
{
public:
	PtrListInsertSorted() : last_insert_pos(0) { disable_sort = 0; }
	
	T *addItem(T *item)
	{
		int numItems = PtrList<T>::getNumItems();
		if (numItems == 0)
		{
			last_insert_pos = 0;
			
			return PtrList<T>::addItem(item);
		}

		int insertpoint = -1;
		if (!disable_sort)
		{
			int bot = 0, top = numItems - 1, mid;
			// benski>
			// optimization based on profiler info.  Too many string compares!
			// Most of the use of this comes from GuiObjectWnd's constructor (and derived classes)
			// so I've changed GuiObjectWnd to add things in alphabetical order.
			// Before we start the binary search, we'll check the new item against the LAST item inserted
			// Most of the time, we'll finish the insert in O(1)
			// Even if we fail, we mitigate the loss somewhat by limiting the binary search

			if (last_insert_pos >= numItems) // the list may have shrunk since last time
				last_insert_pos = numItems - 1;
				
			int quickTest = C::compareItem(item, PtrList<T>::getItemList()[last_insert_pos]);

			if (quickTest == 0) // right on the money.. we'll go ahead and insert ourselves next
				return PtrList<T>::addItem(item, last_insert_pos);

			if (quickTest > 0) // ok we go after the last inserted item (good), but we need to make sure we go before the next one
			{
				last_insert_pos++;
				
				if (last_insert_pos == numItems) // we're at the end? cool...
					return PtrList<T>::addItem(item, PTRLIST_POS_LAST);

				quickTest = C::compareItem(item, PtrList<T>::getItemList()[last_insert_pos]); // test against the next item
				
				if (quickTest <= 0) // and we're not bigger than the next one... perfect!
					return PtrList<T>::addItem(item, last_insert_pos);
				else // too bad
					bot = last_insert_pos; // help out the binary search ... We're at least bigger than everything before last_insert_pos
			}
			else // ok our optimization failed, but we can still help out the binary search
				top = last_insert_pos - 1; // we're at least smaller than everything before last_insert_pos

			// end optimization code


			for (int c = 0; c < numItems + 1; c++)
			{
				if (bot > top)
				{
					// insert here
					insertpoint = bot;
					break;
				}
				mid = (bot + top) / 2;
				int r = C::compareItem(item, PtrList<T>::getItemList()[mid]);
				
				if (r == 0)
				{
					// insert here
					insertpoint = mid;
					break;
				}
				
				if (r < 0)
				{
					top = mid - 1;
				}
				else
				{
					bot = mid + 1;
				}
			}
			last_insert_pos = insertpoint;
			ASSERTPR(insertpoint != -1, "insertsort/binary search fucked up");
		}
		else // no sorting
		{
			last_insert_pos = numItems;
			insertpoint = PTRLIST_POS_LAST;
		}

		return PtrList<T>::addItem(item, insertpoint);
	}
	T *getInsertionPoint(T *item, int *pos)
	{
		if (PtrList<T>::getNumItems() == 0)
		{ 
			if (pos)
				*pos = 0;

			return NULL; 
		}

		int bot = 0, top = PtrList<T>::getNumItems() - 1, mid;

		int insertpoint = -1;
		if (!disable_sort )
		{
			for (int c = 0; c < PtrList<T>::getNumItems() + 1; c++)
			{
				if (bot > top)
				{
					// insert here
					insertpoint = bot;
					break;
				}
				mid = (bot + top) / 2;
				int r = C::compareItem(item, PtrList<T>::getItemList()[mid]);
				
				if (r == 0)
				{
					// insert here
					insertpoint = mid;
					break;
				}
				
				if (r < 0)
				{
					top = mid - 1;
				}
				else
				{
					bot = mid + 1;
				}
			}
			
			ASSERTPR(insertpoint != -1, "insertsort/binary search fucked up");
		}
		else
			insertpoint = PTRLIST_POS_LAST;
		
		if (pos)
			*pos = insertpoint;
		
		return PtrList<T>::enumItem(insertpoint);
	}
	T *findItem(const wchar_t *attrib, int *pos = NULL)
	{
		if (isSorted())
		{
			// binary search
			if (PtrList<T>::getNumItems() == 0)
				return NULL;

			int bot = 0, top = PtrList<T>::getNumItems() - 1, mid;

			for (int c = 0; c < PtrList<T>::getNumItems() + 1; c++)
			{
				if (bot > top)
					return NULL;
				
				mid = (bot + top) / 2;
				int r = C::compareAttrib(attrib, PtrList<T>::getItemList()[mid]);
				
				if (r == 0)
				{
					if (pos != NULL)
						*pos = mid;
					
					return PtrList<T>::getItemList()[mid];
				}
				
				if (r < 0)
				{
					top = mid - 1;
				}
				else
				{
					bot = mid + 1;
				}
			}
			ASSERTPR(0, "binary search fucked up");
		}
		else
		{
			// linear search
			for (int i = 0; i < PtrList<T>::getNumItems(); i++)
			{
				int r = C::compareAttrib(attrib, PtrList<T>::getItemList()[i]);
				if (r == 0)
				{
					if (pos != NULL)
						*pos = i;
					
					return PtrList<T>::getItemList()[i];
				}
			}
		}
		
		return NULL;
	}
	T *findItem(T *attrib, int *pos = NULL)          { return findItem((const wchar_t *)attrib, pos); }
	void setSorted(int dosort)                       { disable_sort = !dosort; }
	int isSorted()                                   { return !disable_sort; }
	int disable_sort;

	int last_insert_pos;
};

// this list allows you to have multiple items with same attrib and adds findLastItem so you can
// sort on more than just one item. this can be used to make autosorting lists of overriding items
// which you can add and remove at will.
template <class T, class C> class PtrListQuickMultiSorted : public PtrListQuickSorted<T, C>
{
public:
	PtrListQuickMultiSorted(int initial_size = 0) : PtrListQuickSorted<T, C>(initial_size) {}
	T *findLastItem(const wchar_t *attrib, int *pos = NULL)
	{
		PtrListQuickSorted<T, C>::sort();
		int p = 0;
		int fp = 0;
		T *item = PtrListQuickSorted<T, C>::findItem(attrib, &fp);
		
		if (!item)
			return NULL;

		p = fp;

		for(;;)
		{
			p++;
			
			if (p >= PtrListQuickSorted<T, C>::getNumItems())
				break;
			
			T* i = PtrListQuickSorted<T, C>::enumItem(p);
			
			if (!C::compareAttrib(attrib, i))
			{
				fp = p;
				item = i;
			}
			else
				break;
		}

		if (pos != NULL)
			*pos = fp;
		
		return item;
	}
};
//same thing but Insert sorted. use this one if you insert and search items often (no need to sort on findItem)
template <class T, class C> class PtrListInsertMultiSorted : public PtrListInsertSorted<T, C>
{
public:
	PtrListInsertMultiSorted() : PtrListInsertSorted<T, C>() {}
	T *findLastItem(const wchar_t *attrib, int *pos = NULL)
	{
		//sort();
		int p = 0;
		int fp = 0;
		T *item = PtrListInsertSorted<T, C>::findItem(attrib, &fp);
		
		if (!item)
			return NULL;

		p = fp;

		for (;;)
		{
			p++;
			
			if (p >= PtrListInsertSorted<T, C>::getNumItems())
				break;
			
			T* i = PtrListInsertSorted<T, C>::enumItem(p);
			
			if (!C::compareAttrib(attrib, i))
			{
				fp = p;
				item = i;
			}
			else
				break;
		}

		if (pos != NULL)
			*pos = fp;
		
		return item;
	}
};


#include <bfc/foreach.h>

#endif
