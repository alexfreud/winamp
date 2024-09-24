#include <precomp.h>

#include "svc_redir.h"

#define CBCLASS svc_redirectI
START_DISPATCH;
  CB(ISMINE, isMine);
  CB(REDIRECT, redirect);
END_DISPATCH;
#undef CBCLASS
