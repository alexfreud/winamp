#include <precomp.h>

#include "svc_metadata.h"

#define CBCLASS svc_metaDataI
START_DISPATCH;
  CB(ENUMMETADATA, enumMetaData);
  CB(HASMETADATA, hasMetaData);
  CB(ALLOWOPERATION, allowOperation);
  CB(GETGUID, getGUID);
  CB(GETMETANAME, getMetaTableName);
END_DISPATCH;
#undef CBCLASS

