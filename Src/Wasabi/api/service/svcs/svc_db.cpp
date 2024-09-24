#include <precomp.h>

#include "svc_db.h"

#define CBCLASS svc_dbI
START_DISPATCH;
  CB(OPENTABLE, openTable);
  VCB(CLOSETABLE, closeTable);
  CB(TESTQUERYFORMAT, testQueryFormat);
END_DISPATCH;
#undef CBCLASS


#define CBCLASS dbSvcTableI
START_DISPATCH;
  CB(GETSCANNER, getScanner);
  CB(NEWSCANNER, newScanner);
  VCB(DELETESCANNER, deleteScanner);
  VCB(CBNEW, _new);
  VCB(CBINSERT, insert);
  VCB(CBCANCEL, cancel);
  VCB(CBEDIT, edit);
  VCB(CBPOST, post);
  VCB(CBDELETE, _delete);
  CB(EDITING, editing);
  VCB(SETFIELDBYNAME, setFieldByName);
  VCB(SETFIELDBYID, setFieldById);
  VCB(DELETEFIELDBYNAME, deleteFieldByName);
  VCB(DELETEFIELDBYID, deleteFieldById);
  VCB(ADDCOLUMN, addColumn);
  VCB(ADDINDEXBYNAME, addIndexByName);
  VCB(ADDINDEXBYID, addIndexById);
  VCB(DROPINDEXBYNAME, dropIndexByName);
  VCB(DROPINDEXBYID, dropIndexById);
  VCB(SYNC, sync);                                             
END_DISPATCH;
#undef CBCLASS

#define CBCLASS dbSvcScannerI
START_DISPATCH;
  VCB(CBFIRST, first);
  VCB(CBLAST, last);
  VCB(CBNEXT, block_next);
  VCB(CBPREVIOUS, block_previous);
  CB(CBNEXT2, next);
  CB(CBPREVIOUS2, previous);
  VCB(CBPUSH, push);
  VCB(CBPOP, pop);
  CB(CBEOF, eof);
  CB(CBBOF, bof);
  CB(GETNUMROWS, getNumRows);
  VCB(MOVETOROW, moveToRow);
  CB(GETCURROW, getCurRow);
  CB(LOCATEBYNAME, locateByName);
  CB(LOCATEBYID, locateById);
  CB(GETNUMCOLS, getNumCols);
  CB(ENUMCOL, enumCol);
  CB(GETCOLBYNAME, getColByName);
  CB(GETCOLBYID, getColByName);
  CB(GETFIELDBYNAME, getFieldByName);
  CB(GETFIELDBYID, getFieldById);
  VCB(SETINDEXBYNAME, setIndexByName);
  VCB(SETINDEXBYID, setIndexById);
  CB(UNIQUEBYNAME, newUniqueScannerByName);
  CB(UNIQUEBYID, newUniqueScannerById);
  VCB(DELETEUNIQUE, deleteUniqueScanner);
  CB(QUERY, query);
  VCB(CANCELQUERY, cancelQuery);
  CB(INDEXCHANGED, hasIndexChanged);
  VCB(CLEARDIRTYBIT, clearDirtyBit);
  VCB(JOINSCANNER, joinScanner);
  VCB(UNJOINSCANNER, unjoinScanner);
  CB(GETLASTQUERY, getLastQuery);
  VCB(SETBLOCKING, setBlocking);
END_DISPATCH;
#undef CBCLASS


