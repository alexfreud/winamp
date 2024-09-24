#ifndef __WASABI_AUTHCB_H
#define __WASABI_AUTHCB_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <api/syscb/callbacks/syscb.h>

namespace AuthCallback 
{
  enum 
  {
    CREDENTIALS_CHANGED=10,			//param1 = (api_auth*)auth; param2 = (const GUID*)realm; no return value.
	CREDENTIALS_ABOUTTOCHANGE=20,	//param1 = (api_auth*)auth; param2 = (const GUID*)realm; no return value.

  };
};

#endif //__WASABI_AUTHCB_H
