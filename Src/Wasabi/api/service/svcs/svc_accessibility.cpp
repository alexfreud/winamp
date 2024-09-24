#include <precomp.h>
#include "svc_accessibility.h"

#define CBCLASS svc_accessibilityI
START_DISPATCH;
  CB(SVC_ACCESSIBILITY_CREATEACCESSIBLEOBJECT,   createAccessibleObject);
END_DISPATCH;
#undef CBCLASS


