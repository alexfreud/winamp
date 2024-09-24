#ifndef __WASABI_API_WEBSERV_ONCONNCB_H
#define __WASABI_API_WEBSERV_ONCONNCB_H

#include "bfc/dispatch.h"
#include "bfc/platform/types.h"
//#include "listen.h"

class api_webserv;
class api_pagegenerator;
class api_httpserv;

class JNL_Listen;

class api_onconncb : public Dispatchable
{
protected:
	api_onconncb()                                                    {}
	~api_onconncb()                                                   {}

	public:
	api_pagegenerator *onConnection( api_httpserv *serv, int port );
	void destroyConnection( api_pagegenerator *conn );

	DISPATCH_CODES
	{
		API_ONCONNCB_ONCONNECTION      = 10,
		API_ONCONNCB_DESTROYCONNECTION = 20,
	};

	api_webserv *caller = NULL;
};

inline api_pagegenerator *api_onconncb::onConnection( api_httpserv *serv, int port )
{
	return _call( API_ONCONNCB_ONCONNECTION, (api_pagegenerator *)0, serv, port );
}

inline void api_onconncb::destroyConnection( api_pagegenerator *connection )
{
	_voidcall( API_ONCONNCB_DESTROYCONNECTION, connection );
}

#endif  // !__WASABI_API_WEBSERV_ONCONNCB_H
