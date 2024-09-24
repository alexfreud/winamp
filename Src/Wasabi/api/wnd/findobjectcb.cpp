#include "precomp.h"
#include "findobjectcb.h"

#define CBCLASS _FindObjectCallback
START_DISPATCH;
  CB(FINDOBJECTCB_MATCHOBJECT, findobjectcb_matchObject);
END_DISPATCH;

