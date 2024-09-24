#ifndef _EZFEED_H
#define _EZFEED_H

//#include <bfc/wasabi_std.h>
#include <api/service/waservicefactorybase.h>
#include <api/skin/feeds/textfeed.h>

class EzFeed : public waServiceFactoryBase<svc_textFeed, EzFeed>, public TextFeed
{
public:
	EzFeed() : registered(0) { }
	static const char *getServiceName() { return "EzTextFeed"; }
	virtual svc_textFeed *newService() { return this; }
	virtual int delService(svc_textFeed *service) { return TRUE; }

	virtual int svc_notify(int msg, int param1 = 0, int param2 = 0)
	{
		if (msg == SvcNotify::ONREGISTERED) registered = 1;
		else if (msg == SvcNotify::ONDEREGISTERED) registered = 0;
		return 1;
	}

private:
	int registered;
};

#endif
