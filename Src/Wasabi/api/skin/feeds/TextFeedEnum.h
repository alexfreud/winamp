#ifndef NULLSOFT_WASABI_TEXTFEEDENUM_H
#define NULLSOFT_WASABI_TEXTFEEDENUM_H


#include <bfc/string/StringW.h>
#include <api/skin/feeds/api_textfeed.h>
// see helper class TextFeed

#include <api/service/servicei.h>
template <class T>
class TextFeedCreatorSingle : public waServiceFactoryTSingle<svc_textFeed, T>
{
public:
	svc_textFeed *getFeed()
	{
		return getSingleService();
	}
};

#include <api/service/svc_enum.h>

class TextFeedEnum : public SvcEnumT<svc_textFeed>
{
public:
	TextFeedEnum(const wchar_t *_feedid) : feedid(_feedid) {}
protected:
	virtual int testService(svc_textFeed *svc)
	{
		return (svc->hasFeed(feedid));
	}
private:
	StringW feedid;
};

#endif