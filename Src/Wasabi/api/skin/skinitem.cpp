#include <precomp.h>
#include "skinitem.h"

#ifdef CBCLASS
#undef CBCLASS
#endif
#define CBCLASS SkinItemI
START_DISPATCH;
  CB(SKINITEM_GETXMLROOTPATH, getXmlRootPath);
  CB(SKINITEM_GETNAME, getName);
  CB(SKINITEM_GETPARAMS, getParams);
  CB(SKINITEM_GETSKINPARTID, getSkinPartId);
  CB(SKINITEM_GETANCESTOR, getAncestor);
END_DISPATCH;
