#include <precomp.h>
#pragma warning(disable:4355)

//<?#include "<class data="implementationheader"/>"
#include "feedwatch.h"
//?>

#include <api/skin/feeds/TextFeedEnum.h>


FeedWatcher::FWDV::FWDV(FeedWatcher *_parent) : parent(_parent) { }

int FeedWatcher::FWDV::viewer_onItemDeleted(ifc_dependent *item) {
  return parent->fwdv_onItemDeleted(item);
}

int FeedWatcher::FWDV::viewer_onEvent(ifc_dependent *item, const GUID *classguid, int event, intptr_t param, void *ptr, size_t ptrlen) {
  return parent->fwdv_onEvent(item, event, param, ptr, ptrlen);
}

FeedWatcher::FeedWatcher() :
  registered_syscb(0), textfeed(NULL), fwdv(this)
{
}


FeedWatcher::~FeedWatcher() {
  releaseFeed();
  if (registered_syscb) {
    WASABI_API_SYSCB->syscb_deregisterCallback(static_cast<SvcCallbackI*>(this));
    DebugStringW(L"unreg syscb");
  }
}

int FeedWatcher::setFeed(const wchar_t *feedid)
{
  if (!registered_syscb++) WASABI_API_SYSCB->syscb_registerCallback(static_cast<SvcCallbackI*>(this));

  releaseFeed();

  feed_id = feedid;
  if (!feed_id.isempty()) textfeed = TextFeedEnum(feed_id).getFirst();

  if (textfeed != NULL) {
    fwdv.viewer_addViewItem(textfeed->getDependencyPtr());
    feedwatcher_onSetFeed(textfeed);
//CUT    if (first_cb) feedwatcher_onFeedChange(textfeed->getFeedText(feed_id));
  }

  return 1;
}

void FeedWatcher::releaseFeed() {
  if (textfeed) { 
    fwdv.viewer_delViewItem(textfeed->getDependencyPtr());
    feed_id = L"";
    SvcEnum::release(textfeed);
    textfeed = NULL;
  }                   
}

const wchar_t *FeedWatcher::getFeedId() {
  return feed_id;
}

int FeedWatcher::fwdv_onItemDeleted(ifc_dependent *item) {
  textfeed = NULL;
  // send text change msg? dunno for sure
  return 1;
}

int FeedWatcher::fwdv_onEvent(ifc_dependent *item, int event, intptr_t param2, void *ptr, size_t ptrlen) {
  if (textfeed && item == textfeed->getDependencyPtr()
		&& event == svc_textFeed::Event_TEXTCHANGE 
		&& WCSCASEEQLSAFE((const wchar_t *)param2, feed_id)) {
    feedwatcher_onFeedChange((const wchar_t *)ptr);
    return 1;
  }
  return 0;
}

void FeedWatcher::svccb_onSvcRegister(FOURCC type, waServiceFactory *svc) {
  // brand new feed and we don't have one, try to register it
//CUT __asm int 3;
  if (type == WaSvc::TEXTFEED && textfeed == NULL) {
    if (!feed_id.isempty()) {
      setFeed(feed_id);
    }
  }
}