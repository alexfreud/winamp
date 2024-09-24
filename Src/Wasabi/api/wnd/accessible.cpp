#include "precomp.h"
#include "accessible.h"

#define CBCLASS AccessibleI
START_DISPATCH;
  CB(ACCESSIBLE_GETIACCESSIBLE, getIAccessible);
  CB(ACCESSIBLE_GETOSHANDLE,    getOSHandle);
  VCB(ACCESSIBLE_RELEASE,       release);
  VCB(ACCESSIBLE_ADDREF,        addRef);
  CB(ACCESSIBLE_GETNUMREFS,     getNumRefs);
  VCB(ACCESSIBLE_ONGETFOCUS,    onGetFocus);
  VCB(ACCESSIBLE_ONSETNAME,     onSetName);
  CB(ACCESSIBLE_GETOSWND,       getOSWnd);
  VCB(ACCESSIBLE_ONSTATECHANGE, onStateChange);
  CB(ACCESSIBLE_FLATTENCONTENT, flattenContent);
END_DISPATCH;
