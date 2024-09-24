#pragma once
#ifndef bandWidth_H_
#define bandWidth_H_

#include "unicode/uniString.h"

namespace bandWidth
{
	typedef enum
	{
		ALL = 0,
		PUBLIC_WEB,			// public facing pages
		PRIVATE_WEB,		// private admin pages

		SOURCE_V1_SENT,		// v1 source connections sent
		SOURCE_V1_RECV,		// v1 source connections received
		SOURCE_V2_SENT,		// v2 source connections sent
		SOURCE_V2_RECV,		// v2 source connections received

		CLIENT_V1_SENT,		// v1 client connections
		CLIENT_V2_SENT,		// v2 client connections
		CLIENT_HTTP_SENT,	// HTTP client connections
		CLIENT_FLV_SENT,	// FLV client connections
		CLIENT_M4A_SENT,	// M4A client connections

		FLASH_POLICY,		// flash policy server

		RELAY_MISC_RECV,	// relay connnections handshaking
		RELAY_V1_RECV,		// v1 relay connnections received
		RELAY_V2_SENT,		// v2 relay connnections sent
		RELAY_V2_RECV,		// v2 relay connnections received

		YP_SENT,			// YP connections sent
		YP_RECV,			// YP connections received

		AUTH_AND_METRICS,	// metrics based responses
		ADVERTS,			// advert data requests / pulls

		ALL_SENT,			// consolidated sent total
		ALL_RECV,			// consolidated received total
		ALL_WEB,			// consolidated web page total
		ALL_SOURCE_SENT,	// consolidated source sent total
		ALL_SOURCE_RECV,	// consolidated source received total
		ALL_CLIENT_SENT,	// consolidated client sent total
		ALL_RELAY_RECV,		// consolidated relay received total
		ALL_OTHER			// consolidated remainder total
	} usageType_t;

	const __uint64 getAmount(const bandWidth::usageType_t);
	void updateAmount(const bandWidth::usageType_t, const __uint64 amount);
	void getFinalAmounts();
}

#endif
