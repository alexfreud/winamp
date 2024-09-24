#ifndef __FEEDWATCH_H
#define __FEEDWATCH_H

#include <bfc/wasabi_std.h>
#include <api/syscb/callbacks/svccbi.h>
#include "feedwatcherso.h"

class svc_textFeed;
class ifc_dependent;



class FeedWatcher : private SvcCallbackI, public FeedWatcherScriptObject {
public:
  FeedWatcher();
  virtual ~FeedWatcher();

  int setFeed(const wchar_t *feedid);
  void releaseFeed();

  const wchar_t *getFeedId();

  virtual void feedwatcher_onSetFeed(svc_textFeed *svc) { }
  virtual void feedwatcher_onFeedChange(const wchar_t *data) { }

protected:
  class FWDV : public DependentViewerI 
{
public:
  friend class FeedWatcher;

  FWDV(FeedWatcher *parent);

  virtual int viewer_onItemDeleted(ifc_dependent *item);
  virtual int viewer_onEvent(ifc_dependent *item, const GUID *classguid, int event, intptr_t param, void *ptr, size_t ptrlen);

  FeedWatcher *parent;
};
  
  // catches text feed change events
  virtual int fwdv_onItemDeleted(ifc_dependent *item);
  virtual int fwdv_onEvent(ifc_dependent *item, int event, intptr_t param2, void *ptr, size_t ptrlen);

private:
  // catches new feeds being registered
  virtual void svccb_onSvcRegister(FOURCC type, waServiceFactory *svc);

  int registered_syscb;
  StringW feed_id;
  svc_textFeed *textfeed;
  FWDV fwdv;
};

#endif
