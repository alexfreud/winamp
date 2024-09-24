#include <precomp.h>

#include "svc_itemmgr.h"

#define CBCLASS svc_itemMgrI
START_DISPATCH;
  CB(ISMINE, isMine);
  CB(OPTIMIZEPLAYSTRING, optimizePlaystring);
  CB(CREATEINITIALNAME, createInitialName);
  CB(OPTIMIZEFILEDATA, optimizeFileData);
  CB(ONDATABASEADD, onDatabaseAdd);
  CB(ONDATABASEDEL, onDatabaseDel);
  CB(ONTITLECHANGE, onTitleChange);
  CB(ONTITLE2CHANGE, onTitle2Change);
  VCB(ONNEXTFILE, onNextFile);
  VCB(ONFILECOMPLETE, onFileComplete);
  CB(WANTSCANDATA, wantScanData);
  CB(GETSORTORDER, getSortOrder);
END_DISPATCH;
#undef CBCLASS
