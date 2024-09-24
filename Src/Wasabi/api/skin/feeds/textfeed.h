#ifndef _TEXTFEED_H
#define _TEXTFEED_H

#include <bfc/common.h>
#include <map>
#include <string>
#include <utility>
#include <bfc/depend.h>
#include <api/service/svcs/svc_textfeed.h>
#include <api/syscb/callbacks/corecbi.h>

/**
  This is the standard helper class for implementing a textfeed.
  Be sure to check out class EzFeed in ezfeed.h, which combines this
  class with a service factory.
*/
class TextFeed : public svc_textFeedI, public DependentI 
{
public:
/**
  Call this to register your feeds by id. Make the ids unique!
  @see sendFeed()
  @ret TRUE if succeeded, FALSE on error (i.e. nonunique id)
*/
  int registerFeed(const wchar_t *feedid, const wchar_t *initial_text=L"", const wchar_t *description=L"");

/**
  Call this to send text into a feed.
  @see registerFeed()
  @ret TRUE if succeeded, FALSE on error (i.e. feedid not registered)
*/
  int sendFeed(const wchar_t *feedid, const wchar_t *text);

//  Gives the most recently sent text on a feed.
  virtual const wchar_t *getFeedText(const wchar_t *feedid);

//  Gives a description for the feed (used by accessibility).
  virtual const wchar_t *getFeedDescription(const wchar_t *feedid);

protected:
  virtual api_dependent *getDependencyPtr() { return this; }
  virtual void dependent_onRegViewer(api_dependentviewer *viewer, int add);
  virtual void *dependent_getInterface(const GUID *classguid);

/**
  Called when someone subscribes to this feed.
*/
  virtual void onRegClient() { }
  virtual void onDeregClient() { }

  virtual int hasFeed(const wchar_t *name);

private:
  std::map<std::wstring, std::pair<std::wstring, std::wstring> > feeds;
};

#endif
