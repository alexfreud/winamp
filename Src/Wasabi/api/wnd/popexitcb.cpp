#include <precomp.h>
#include "popexitcb.h"

#define CBCLASS PopupExitCallbackI
START_DISPATCH;
  CB(POPUPEXIT_ONEXITPOPUP, popupexitcb_onExitPopup);
  CB(POPUPEXIT_GETDEPENDENCYPTR, popupexit_getDependencyPtr);
END_DISPATCH;
