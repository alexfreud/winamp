#ifndef _SVC_TEXTFEED_H
#define _SVC_TEXTFEED_H

//#include <api/service/services.h>
#include <api/skin/feeds/api_textfeed.h>

class NOVTABLE svc_textFeedI : public svc_textFeed
{
public:
	virtual int hasFeed(const wchar_t *name) = 0;
	virtual const wchar_t *getFeedText(const wchar_t *name) = 0;
	virtual const wchar_t *getFeedDescription(const wchar_t *name) = 0;
	virtual api_dependent *getDependencyPtr() = 0;
	virtual void *dependent_getInterface(const GUID *classguid); //implemented for you

protected:
	RECVS_DISPATCH;
};


#endif
