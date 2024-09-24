/* ---------------------------------------------------------------------------

                 Nullsoft Database Engine - Codename: Neutrino

--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

 Global Include File

--------------------------------------------------------------------------- */

#ifndef __NDE_H
#define __NDE_H

// TODO: find better Mac preproc symbol
#include "nde_defines.h"

extern const char *tSign;
extern const char *iSign;

// Magic headers
#define __TABLE_SIGNATURE__ tSign
#define __INDEX_SIGNATURE__ iSign

// Linked list entry types
#define UNKNOWN           0
#define FIELD             1
#define FILTER           2
#define SCANNER           3



// Records constants
#define NEW_RECORD    -128
#define FIELD_NOT_FOUND -1
#define INVALID_RECORD  -1
#define NUM_SPECIAL_RECORDS 2 // 
#define FIELDS_RECORD_NUM 0 
#define INDEX_RECORD_NUM 1

// Index constants
#define PRIMARY_INDEX   255
#define QFIND_ENTIRE_SCOPE -1




#define throwException(x) {}

#define FILTER_NOT       0
#define FILTER_AND       1
#define FILTER_OR         2

#define FILTERS_INVALID    -1
#define FILTERS_INCOMPLETE  1
#define FILTERS_COMPLETE    1
#define ADDFILTER_FAILED    2

// Permissions
#define PERM_DENYALL     0
#define PERM_READ         1
#define PERM_READWRITE   3



// All our classes so we can foreward reference
class NDE_API LinkedListEntry;
class LinkedList;
class Field;
class ColumnField;
class IndexField;
class StringField;
class IntegerField;
class Int64Field;
class Int128Field;
class GUIDField;
class FloatField;
class BooleanField;
class BinaryField;
class Binary32Field;
class BitmapField;
class PrivateField;
class FilenameField;
class Record;
class NDE_API Scanner;
class Table;
class Index;
class Filter;
class Database;

#if !defined (WIN32) && !defined(WIN64)  
#define NDE_NOWIN32FILEIO
#define NO_TABLE_WIN32_LOCKING
typedef int BOOL;
typedef void* HANDLE;
#define TRUE 1
#define FALSE 0
#ifdef __APPLE__
#include <bfc/platform/types.h>
#else
#include <foundation/types.h> // TODO 
#endif
#define HINSTANCE int
//#define HWND int
#define DWORD int
#define wsprintf sprintf
#define OutputDebugString(x) ;
#define GetCurrentProcessId() getpid()

#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <wchar.h>
#include <sys/stat.h>
void clear_stdin(void);

void CopyFile(const char *filename, const char *destfilename, BOOL b);
void DeleteFile(const char *filename);
BOOL MoveFile(const char *filename, const char *destfilename);
void Sleep(int ms);

#define _stricmp strcasecmp
#define strcmpi strcasecmp
#define _strnicmp strncasecmp
#define strncmpi strncasecmp

#define _wtoi(x) wcstol(x,0,10)
// TODO: find case insensitive compare on Mac OS X
#define _wcsicmp wcscmp 
#define _wcsnicmp wcsncmp

#define _MAX_PATH 8192
#define _MAX_DRIVE 256
#define _MAX_DIR 7424
#define _MAX_FNAME 256
#define _MAX_EXT 256

#define min(x,y) ((x > y) ? y : x)
#define max(x,y) ((x < y) ? y : x)

#endif

// All our includes+

#include "Vfs.h"
#include "LinkedList.h"
#include "Field.h"
#include "ColumnField.h"
#include "IndexField.h"
#include "StringField.h"
#include "IntegerField.h"
#include "Int64Field.h"
#include "Int128Field.h"
#include "BinaryField.h"
#include "Binary32Field.h"
#include "FilenameField.h"
#include "Record.h"
#include "Scanner.h"
#include "Table.h"
#include "Database.h"
#include "Index.h"
#include "Filter.h"
#include "DBUtils.h"


#endif
