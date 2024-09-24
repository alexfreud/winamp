#ifndef _SVC_DB_H
#define _SVC_DB_H

#include <bfc/dispatch.h>
#include <api/service/services.h>

typedef enum {
  DBSVC_DATATYPE_UNKNOWN = 0,
  DBSVC_DATATYPE_INT     = 1,
  DBSVC_DATATYPE_STRING  = 2,
  DBSVC_DATATYPE_BINARY  = 3,
  DBSVC_DATATYPE_GUID    = 4,
  DBSVC_DATATYPE_FLOAT   = 5,
} dbSvcDatatypeEnum;

typedef struct {
  void *data;
  dbSvcDatatypeEnum datatype;
  int datalen;
} dbSvcDatachunk;

typedef struct {
  const char *name;
  dbSvcDatatypeEnum datatype;
  int id;
  int indexed;
  int uniques_indexed;
} dbSvcColInfo;

class dbSvcScanner;

class NOVTABLE dbSvcScanner : public Dispatchable {

  public:

    void first();
    void last();
    void block_next(); // blocking call for baclkward compat
    void block_previous(); // blocking call for baclkward compat
    int next(); // if non blocking, returns 0 for "HOLD ON!" and 1 for "GOT A ROW!"
    int previous(); // if non blocking, returns 0 for "HOLD ON!" and 1 for "GOT A ROW!"
    void push();
    void pop();
    int eof();
    int bof();

    int getNumRows();
    void moveToRow(int row);
    int getCurRow();

    int locateByName(const char *col, int from_row, dbSvcDatachunk *data);
    int locateById(int id, int from_row, dbSvcDatachunk *data);

    int getNumCols();
    dbSvcColInfo *enumCol(int n);
    dbSvcColInfo *getColById(int n);
    dbSvcColInfo *getColByName(const char *col);

    dbSvcDatachunk *getFieldByName(const char *col);
    dbSvcDatachunk *getFieldById(int id);

    void setIndexByName(const char *col);
    void setIndexById(int id);

    dbSvcScanner *newUniqueScannerByName(const char *col);
    dbSvcScanner *newUniqueScannerById(int colid);
    void deleteUniqueScanner(dbSvcScanner *);

    int query(const char *query);
    const char *getLastQuery();
    void cancelQuery();
    int hasIndexChanged();
    void clearDirtyBit();

    void joinScanner(dbSvcScanner *scanner, const char *field);
    void unjoinScanner(dbSvcScanner *scanner);

    void setBlocking(int b); // blocking is the default behavior

    enum {
      CBFIRST             = 100,
      CBLAST              = 110,
      CBNEXT              = 120, // retired
      CBPREVIOUS          = 130, // retired
      CBNEXT2             = 121,
      CBPREVIOUS2         = 131,
      CBPUSH              = 140,
      CBPOP               = 150,
      CBEOF               = 160,
      CBBOF               = 170,
      GETNUMROWS          = 180,
      MOVETOROW           = 190,
      GETCURROW           = 200,
      LOCATEBYNAME        = 210,
      LOCATEBYID          = 220,
      GETNUMCOLS          = 230,
      ENUMCOL             = 240,
      GETCOLBYID          = 250,
      GETCOLBYNAME        = 260,
      GETFIELDBYNAME      = 270,
      GETFIELDBYID        = 280,
      SETINDEXBYNAME      = 290,
      SETINDEXBYID        = 300,
      QUERY               = 310,
      CANCELQUERY         = 320,
      INDEXCHANGED        = 330,
      CLEARDIRTYBIT       = 340,
      UNIQUEBYNAME        = 350,
      UNIQUEBYID          = 360,
      DELETEUNIQUE        = 370,
      GETLASTQUERY        = 380,
      JOINSCANNER         = 390,
      UNJOINSCANNER       = 400,
      SETBLOCKING         = 410,
    };
};


inline void dbSvcScanner::first() {
  _voidcall(CBFIRST);
}

inline void dbSvcScanner::last() {
  _voidcall(CBLAST);
}

inline void dbSvcScanner::block_next() {
  _voidcall(CBNEXT);
}

inline int dbSvcScanner::next() {
  return _call(CBNEXT2, 0);
}

inline void dbSvcScanner::block_previous() {
  _voidcall(CBPREVIOUS);
}

inline int dbSvcScanner::previous() {
  return _call(CBPREVIOUS2, 0);
}

inline void dbSvcScanner::push() {
  _voidcall(CBPUSH);
}

inline void dbSvcScanner::pop() {
  _voidcall(CBPOP);
}

inline int dbSvcScanner::eof() {
  return _call(CBEOF, 0);
}

inline int dbSvcScanner::bof() {
  return _call(CBBOF, 0);
}

inline int dbSvcScanner::getNumRows() {
  return _call(GETNUMROWS, 0);
}

inline void dbSvcScanner::moveToRow(int row) {
  _voidcall(MOVETOROW, row);
}

inline int dbSvcScanner::getCurRow() {
  return _call(GETCURROW, 0);
}

inline int dbSvcScanner::locateByName(const char *col, int from_row, dbSvcDatachunk *data) {
  return _call(LOCATEBYNAME, 0, col, from_row, data);
}

inline int dbSvcScanner::locateById(int colid, int from_row, dbSvcDatachunk *data) {
  return _call(LOCATEBYNAME, 0, colid, from_row, data);
}

inline int dbSvcScanner::getNumCols() {
  return _call(GETNUMCOLS, 0);
}

inline dbSvcColInfo *dbSvcScanner::enumCol(int n) {
  return _call(ENUMCOL, ((dbSvcColInfo *)NULL), n);
}

inline dbSvcColInfo *dbSvcScanner::getColByName(const char *col) {
  return _call(GETCOLBYNAME, ((dbSvcColInfo *)NULL), col);
}

inline dbSvcColInfo *dbSvcScanner::getColById(int colid) {
  return _call(GETCOLBYID, ((dbSvcColInfo *)NULL), colid);
}

inline dbSvcDatachunk *dbSvcScanner::getFieldByName(const char *col) {
  return _call(GETFIELDBYNAME, ((dbSvcDatachunk *)NULL), col);
}

inline dbSvcDatachunk *dbSvcScanner::getFieldById(int colid) {
  return _call(GETFIELDBYNAME, ((dbSvcDatachunk *)NULL), colid);
}

inline void dbSvcScanner::setIndexByName(const char *col) {
  _voidcall(SETINDEXBYNAME, col);
}

inline void dbSvcScanner::setIndexById(int colid) {
  _voidcall(SETINDEXBYID, colid);
}

inline dbSvcScanner *dbSvcScanner::newUniqueScannerByName(const char *col) {
  return _call(UNIQUEBYNAME, (dbSvcScanner *)NULL, col);
}

inline dbSvcScanner *dbSvcScanner::newUniqueScannerById(int colid) {
  return _call(UNIQUEBYID, (dbSvcScanner *)NULL, colid);
}

inline void dbSvcScanner::deleteUniqueScanner(dbSvcScanner *s) {
  _voidcall(DELETEUNIQUE, s);
}

inline int dbSvcScanner::query(const char *q) {
  return _call(QUERY, 0, q);
}

inline void dbSvcScanner::cancelQuery() {
  _voidcall(CANCELQUERY);
}

inline int dbSvcScanner::hasIndexChanged() {
  return _call(INDEXCHANGED, 0);
}

inline void dbSvcScanner::clearDirtyBit() {
  _voidcall(CLEARDIRTYBIT);
}

inline const char *dbSvcScanner::getLastQuery() {
  return _call(GETLASTQUERY, (const char *)NULL);
}

inline void dbSvcScanner::joinScanner(dbSvcScanner *scanner, const char *field) {
  _voidcall(JOINSCANNER, scanner, field);
}

inline void dbSvcScanner::unjoinScanner(dbSvcScanner *scanner) {
  _voidcall(UNJOINSCANNER, scanner);
}

inline void dbSvcScanner::setBlocking(int b) {
  _voidcall(SETBLOCKING, b);
}

class NOVTABLE dbSvcScannerI : public dbSvcScanner {
public:
  virtual void first()=0;
  virtual void last()=0;
  virtual void block_next()=0;
  virtual void block_previous()=0;
  virtual int next()=0;
  virtual int previous()=0;
  virtual void push()=0;
  virtual void pop()=0;
  virtual int eof()=0;
  virtual int bof()=0;
  virtual int getNumRows()=0;
  virtual void moveToRow(int row)=0;
  virtual int getCurRow()=0;
  virtual int locateByName(const char *col, int from_row, dbSvcDatachunk *data)=0;
  virtual int locateById(int id, int from_row, dbSvcDatachunk *data)=0;
  virtual int getNumCols()=0;
  virtual dbSvcColInfo *enumCol(int n)=0;
  virtual dbSvcColInfo *getColById(int n)=0;
  virtual dbSvcColInfo *getColByName(const char *col)=0;
  virtual dbSvcDatachunk *getFieldByName(const char *col)=0;
  virtual dbSvcDatachunk *getFieldById(int id)=0;
  virtual void setIndexByName(const char *col)=0;
  virtual void setIndexById(int id)=0;
  virtual dbSvcScanner *newUniqueScannerByName(const char *col)=0;
  virtual dbSvcScanner *newUniqueScannerById(int colid)=0;
  virtual void deleteUniqueScanner(dbSvcScanner *)=0;
  virtual int query(const char *query)=0;
  virtual void cancelQuery()=0;
  virtual int hasIndexChanged()=0;
  virtual void clearDirtyBit()=0;
  virtual const char *getLastQuery()=0;
  virtual void joinScanner(dbSvcScanner *scanner, const char *field)=0;
  virtual void unjoinScanner(dbSvcScanner *scanner)=0;
  virtual void setBlocking(int block)=0;

protected:
  RECVS_DISPATCH;
};

class NOVTABLE dbSvcTable : public Dispatchable {
public:
  dbSvcScanner *getScanner();
  dbSvcScanner *newScanner();
  void deleteScanner(dbSvcScanner *scanner);
  void _new();
  void insert();
  void cancel();
  void edit();
  void post();
  void _delete();
  int editing();
  void setFieldByName(const char *col, dbSvcDatachunk *data);
  void setFieldById(int colid, dbSvcDatachunk *data);
  void deleteFieldByName(const char *col);
  void deleteFieldById(int colid);
  void addColumn(const char *colname, int colid, int datatype, int uniques_indexed);
  void addIndexByName(const char *col);
  void addIndexById(int colid);
  void dropIndexByName(const char *col);
  void dropIndexById(int colid);
  void sync();

  enum {
    GETSCANNER        = 100,
    NEWSCANNER        = 110,
    DELETESCANNER     = 111,
    CBNEW             = 120,
    CBINSERT          = 130,
    CBCANCEL          = 140,
    CBEDIT            = 150,
    CBPOST            = 160,
    CBDELETE          = 170,
    EDITING           = 180,
    SETFIELDBYNAME    = 190,
    SETFIELDBYID      = 200,
    DELETEFIELDBYNAME = 210,
    DELETEFIELDBYID   = 220,
    ADDCOLUMN         = 230,
    ADDINDEXBYNAME    = 240,
    ADDINDEXBYID      = 250,
    SYNC              = 260,
    DROPINDEXBYNAME   = 270,
    DROPINDEXBYID     = 280,
  };
};

inline dbSvcScanner *dbSvcTable::getScanner() {
  return _call(GETSCANNER, static_cast<dbSvcScanner *>(NULL));
}

inline dbSvcScanner *dbSvcTable::newScanner() {
  return _call(NEWSCANNER, static_cast<dbSvcScanner *>(NULL));
}

inline void dbSvcTable::deleteScanner(dbSvcScanner *scanner) {
  _voidcall(DELETESCANNER, scanner);
}

inline void dbSvcTable::_new() {
  _voidcall(CBNEW);
}

inline void dbSvcTable::insert() {
  _voidcall(CBINSERT);
}

inline void dbSvcTable::cancel() {
  _voidcall(CBCANCEL);
}

inline void dbSvcTable::edit() {
  _voidcall(CBEDIT);
}

inline void dbSvcTable::post() {
  _voidcall(CBPOST);
}

inline void dbSvcTable::_delete() {
  _voidcall(CBDELETE);
}

inline int dbSvcTable::editing() {
  return _call(EDITING, 0);
}

inline void dbSvcTable::setFieldByName(const char *col, dbSvcDatachunk *data) {
  _voidcall(SETFIELDBYNAME, col, data);
}

inline void dbSvcTable::setFieldById(int colid, dbSvcDatachunk *data) {
  _voidcall(SETFIELDBYID, colid, data);
}

inline void dbSvcTable::deleteFieldByName(const char *col) {
  _voidcall(DELETEFIELDBYNAME, col);
}

inline void dbSvcTable::deleteFieldById(int colid) {
  _voidcall(DELETEFIELDBYID, colid);
}

inline void dbSvcTable::addColumn(const char *colname, int colid, int datatype, int index_uniques) {
  _voidcall(ADDCOLUMN, colname, colid, datatype, index_uniques);
}

inline void dbSvcTable::addIndexByName(const char *col) {
  _voidcall(ADDINDEXBYNAME, col);
}

inline void dbSvcTable::addIndexById(int colid) {
  _voidcall(ADDINDEXBYID, colid);
}

inline void dbSvcTable::dropIndexByName(const char *col) {
  _voidcall(DROPINDEXBYNAME, col);
}

inline void dbSvcTable::dropIndexById(int colid) {
  _voidcall(DROPINDEXBYID, colid);
}

inline void dbSvcTable::sync() {
  _voidcall(SYNC);
}

class NOVTABLE dbSvcTableI : public dbSvcTable {
public:
  virtual dbSvcScanner *getScanner()=0;
  virtual dbSvcScanner *newScanner()=0;
  virtual void deleteScanner(dbSvcScanner *scanner)=0;
  virtual void _new()=0;
  virtual void insert()=0;
  virtual void cancel()=0;
  virtual void edit()=0;
  virtual void post()=0;
  virtual void _delete()=0;
  virtual int editing()=0;
  virtual void setFieldByName(const char *col, dbSvcDatachunk *data)=0;
  virtual void setFieldById(int colid, dbSvcDatachunk *data)=0;
  virtual void deleteFieldByName(const char *col)=0;
  virtual void deleteFieldById(int colid)=0;
  virtual void addColumn(const char *colname, int colid, int datatype, int index_uniques)=0;
  virtual void addIndexByName(const char *col)=0;
  virtual void addIndexById(int colid)=0;
  virtual void dropIndexByName(const char *col)=0;
  virtual void dropIndexById(int colid)=0;
  virtual void sync()=0;

protected:
  RECVS_DISPATCH;
};

class NOVTABLE svc_db : public Dispatchable {
protected:
  svc_db() {}
  ~svc_db() {}
public:
  static FOURCC getServiceType() { return WaSvc::DB; }

  int testQueryFormat(int queryformat);

  dbSvcTable *openTable(const char *tablefilename, int create_if_not_exist, int cache_in_memory);
  void closeTable(dbSvcTable *table);

  enum {
    TESTQUERYFORMAT = 10,
    OPENTABLE       = 20,
    CLOSETABLE      = 30,
  };
};

inline int svc_db::testQueryFormat(int queryformat) {
  return _call(TESTQUERYFORMAT, 0, queryformat);
}

inline dbSvcTable *svc_db::openTable(const char *tablename, int create_if_not_exist, int cache_in_memory) {
  return _call(OPENTABLE, static_cast<dbSvcTable *>(NULL), tablename, create_if_not_exist, cache_in_memory);
}

inline void svc_db::closeTable(dbSvcTable *table) {
  _voidcall(CLOSETABLE, table);
}

// derive from this one
class NOVTABLE svc_dbI : public svc_db{
public:
  virtual int testQueryFormat(int queryformat)=0;
  virtual dbSvcTable *openTable(const char *filename, int create_if_not_exist, int cache_in_memory)=0;
  virtual void closeTable(dbSvcTable *table)=0;

protected:
  RECVS_DISPATCH;
};

#include <api/service/svc_enum.h>

enum {
  DBQFORMAT_MINIQUERY =1,
  DBQFORMAT_SQL       =2,
};

class DatabaseEnum : public SvcEnumT<svc_db> {
public:
  DatabaseEnum(int queryformat=DBQFORMAT_MINIQUERY) :
    query_format(queryformat) {}
protected:
  virtual int testService(svc_db *svc) {
    return svc->testQueryFormat(query_format);
  }

private:
  int query_format;
};

#endif
