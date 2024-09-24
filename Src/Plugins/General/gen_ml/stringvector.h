#ifndef NULLOSFT_MEDIALIBRARY_STRINGVECTOR_HEADER
#define NULLOSFT_MEDIALIBRARY_STRINGVECTOR_HEADER


#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Manages strings as array. Strings a copied to the continues buffer which make it fast to access data

#include <windows.h>

class StringVector
{

public:
	StringVector(size_t cchAllocate = 0, int cchAllocateStep = 32);
	virtual ~StringVector(void);

public:
	
	size_t Add(const wchar_t *entry, int cchLen); // returns index of a new created string object;
	size_t Add(const wchar_t *entry) { return Add(entry, -1); }

	BOOL Remove(size_t index);
	size_t Count(void); // returns count;
	
	void TrimCount(size_t newCount);
	void Clear(void);	// makes it empty;
	// void Vacuum(void);	// compact space;
	LPCWSTR GetString(size_t index); // returns string from index;
	int GetStringLength(size_t index); // returns string length;
	size_t FindString(LCID lcid, LPCWSTR string, int cchLen = -1, BOOL igonreCase = FALSE); // returns index of the string if found or -1 if no string. (uses CompareString() function) 
	int SetAllocateStep(int cchNewStep); // Sets new allocate step and returns previous one (if newStep < 1 then just return current one).
	size_t GetCbAllocated(void); // returns amount of allocated memory
	size_t GetCchAllocated(void); // returns amount of allocated memory

private:
	typedef struct _RECORD { size_t offset; int length; } RECORD, *HRECORD;

protected:
	HRECORD pointers;
	size_t	ptCount;
	size_t  ptAllocated;
	LPWSTR	buffer;
	size_t  cchBuffer;
	LPWSTR	tail;
	int		allocateStep;
};

#endif //NULLOSFT_MEDIALIBRARY_STRINGVECTOR_HEADER