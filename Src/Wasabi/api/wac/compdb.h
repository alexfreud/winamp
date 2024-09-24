#ifndef __COMP_DB_H
#define __COMP_DB_H

#define DB_DENY  0
#define DB_ALLOW 1

#define DB_READ             0
#define DB_WRITE            1
#define DB_DELETE           2
#define DB_GETSCANNER       3
#define DB_DROPINDEX        4

#define DB_ERROR            0
#define DB_SUCCESS          1
#define DB_NOSUCHFIELD      2
#define DB_RECORDNOTFOUND   3
#define DB_UNKNOWNDATATYPE  4
#define DB_DATATYPEMISMATCH 5
#define DB_OPERATIONDENIED  6
#define DB_INVALIDGUID      7
#define DB_EMPTYFIELD       8
#define DB_NOTAVAILABLE     9

#define DB_ENDOFENUM        0
#define DB_FOUND            TRUE
#define DB_NOTFOUND         FALSE

#endif
