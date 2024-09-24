/* ---------------------------------------------------------------------------
 Nullsoft Database Engine
 --------------------
 codename: Near Death Experience
 --------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 
 Database Class Prototypes
 
 --------------------------------------------------------------------------- */

#ifndef __DATABASE_H
#define __DATABASE_H

#include "nde.h"

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>

class Database
{
public:
	Database();
#ifdef WIN32
	Database(HINSTANCE hinst);
	HINSTANCE GetInstance();
	void SetInstance(HINSTANCE hinst);
#endif
	~Database();
#ifdef _WIN32
	Table *OpenTable(const wchar_t *table, const wchar_t *index, BOOL create, BOOL cached);
#else
		Table *OpenTable(const char *table, const char *index, BOOL create, BOOL cached);
#endif
	void CloseTable(Table *table);
private:
#ifdef WIN32
	HINSTANCE hInstance;
#endif
};

#endif