#include "main.h"
#include "MoreItems.h"

static const wchar_t g_noentry[] = L"No Entry";

	wchar_t *strFile;
	size_t cbFile;
	wchar_t *strTitle;
	size_t cbTitle;
	char *strCurtain;
	size_t cbCurtain;
	int length;
	int index;
	unsigned long starttime;  // Start time in MS  (0, begin of file)
	unsigned long endtime; // End time in MS  (0, end of file)
	moreitems *Next; // Next Item in linked list

moreitems::moreitems()
: strFile(0), cbFile(0), strTitle(0), cbTitle(0),
  strCurtain(0), cbCurtain(0), length(0), index(0),
  starttime(0), endtime(0), Next(0)
{
}

moreitems::~moreitems()
{
	// recursive, find the _tail and remove it, work back to _head
	delete Next;  Next = NULL;
	delete[] strFile; strFile=NULL;
	delete[] strTitle;
	delete[] strCurtain;
}

const wchar_t *moreitems::GetHiddenFilename(int index)
{
	if (this->index == index)
		return strFile;
	if (Next == NULL)
		return g_noentry;
	return Next->GetHiddenFilename(index);
}

int moreitems::SetRange(int index, unsigned long start, unsigned long end)
{
	if (this->index == index)
  {
     this->starttime = start;
     this->endtime = end;
     return 1;
  }
	if (Next == NULL)
		return 0;
	return Next->SetRange(index,start,end);
}

unsigned long moreitems::GetStart(int index)
{
	if (this->index == index)
  {
     return this->starttime;
  }
	if (Next == NULL)
		return 0;
	return Next->GetStart(index);
}

unsigned long moreitems::GetEnd(int index)
{
	if (this->index == index)
  {
    return this->endtime;
  }
	if (Next == NULL)
		return 0;
	return Next->GetEnd(index);
}

int moreitems::AddHiddenItem(const wchar_t *filename, const wchar_t *title, int length, int index, char *curtain)
{
	// Linked list _head
	moreitems *additem = this;
	if (additem && index == 1)
	{
		// List empty
		// Use placeholder
	}
	else
	{
		// Found items, walk to the end
		while (additem && additem->Next) additem = additem->Next;
		if (additem)
		{
			additem->Next = new moreitems;
			additem = additem->Next;
		}
	}
	if (additem)
	{
		additem->cbFile = lstrlenW(filename) + 1;
		additem->strFile = new wchar_t[additem->cbFile];
		StringCchCopyW(additem->strFile , additem->cbFile, filename);
		additem->cbTitle = (int)lstrlenW(title) + 1;
		additem->strTitle = new wchar_t[additem->cbFile];
		StringCchCopyW(additem->strTitle, additem->cbTitle, title);
		if (curtain && *curtain) 
		{
			additem->cbCurtain = (int)strlen(curtain) + 1;
			additem->strCurtain = new char[additem->cbCurtain];
			StringCchCopyA(additem->strCurtain, additem->cbCurtain, curtain);
		}
		else
		{
			additem->cbCurtain = 0;
			additem->strCurtain = NULL;
		}

		additem->length = length;
		additem->index = index;

		return 1;
	}
	return 0;
}

const char *moreitems::GetHiddenCurtain(int index)
{
	moreitems *where = this;
	while ( where )
	{
		if ( where->index == index && where->cbCurtain ) return where->strCurtain;
		where = where->Next;
	}
	return NULL;
}
