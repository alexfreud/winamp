#include <precomp.h>

#include "svc_textfeed.h"
#include <bfc/depend.h>

#define CBCLASS svc_textFeedI
START_DISPATCH;
  CB(SVCTEXTFEED_HASFEED, hasFeed);
  CB(SVCTEXTFEED_GETFEEDTEXT, getFeedText);
  CB(SVCTEXTFEED_GETFEEDDESC, getFeedDescription);
  CB(SVCTEXTFEED_GETDEPENDENCYPTR, getDependencyPtr);
END_DISPATCH;
#undef CBCLASS

void *svc_textFeedI::dependent_getInterface(const GUID *classguid) {
  HANDLEGETINTERFACE(svc_textFeed);
  return NULL;
}
