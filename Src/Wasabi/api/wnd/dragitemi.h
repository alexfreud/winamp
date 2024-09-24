#ifndef _DRAGITEMI_H
#define _DRAGITEMI_H

#include "dragitem.h"
#include <bfc/common.h>
#include <bfc/string/stringW.h>
#include <bfc/ptrlist.h>

class DragItemI : public DragItem
{
public:
	DragItemI(const wchar_t *datatype, void *datum = NULL);
	virtual ~DragItemI() {}

	void addVoidDatum(void *newdatum);	// up to you to cast it right

	const wchar_t *getDatatype();
	int getNumData();
	void *getDatum(int pos = 0);

private:
	RECVS_DISPATCH;

	StringW datatype;
	PtrList<char> datalist;
};

template <class T>
class DragItemT : public DragItemI
{
public:
	DragItemT(T *item = NULL) : DragItemI(T::dragitem_getDatatype(), item) {}
	static inline DragItemI *create(T *item) { return new DragItemT<T>(item); }

	void addDatum(T *newdatum)
	{
		addVoidDatum(static_cast<void *>(newdatum));
	}
};

template <class T>
class DragItemCast
{
public:
	DragItemCast(DragItem *_item, int _pos = 0) : item(_item), pos(_pos) {}
	operator T *()
	{
		if (item == NULL || !STREQL(T::dragitem_getDatatype(), item->getDatatype()))
			return NULL;
		else
			return static_cast<T*>(item->getDatum(pos));
	}
private:
	DragItem *item;
	int pos;
};

#endif
