#include "bandwidth.h"
#include "global.h"
#include "services/stdServiceImpl.h"

using namespace std;
using namespace uniString;
using namespace stringUtil;

#define DEBUG_LOG(...) do { if (gOptions.statsDebug()) DLOG(__VA_ARGS__); } while (0)

#define LOGNAME "[BANDWIDTH] "

static __uint64 g_bandWidthTable[bandWidth::ALL_OTHER];
static AOL_namespace::rwLock g_bandwidthLock;

const __uint64 bandWidth::getAmount(const bandWidth::usageType_t type)
{
    stackRWLock sl (g_bandwidthLock);

	if (type < ALL_SENT)
	{
		return g_bandWidthTable[type];
	}

	switch (type)
	{
		case bandWidth::ALL_SENT:
		{
			return g_bandWidthTable[CLIENT_V1_SENT] +
				   g_bandWidthTable[CLIENT_V2_SENT] +
				   g_bandWidthTable[CLIENT_HTTP_SENT] +
				   g_bandWidthTable[CLIENT_FLV_SENT] +
				   g_bandWidthTable[CLIENT_M4A_SENT] +
				   g_bandWidthTable[SOURCE_V1_SENT] +
				   g_bandWidthTable[SOURCE_V2_SENT] +
				   g_bandWidthTable[PUBLIC_WEB] +
				   g_bandWidthTable[PRIVATE_WEB] +
				   g_bandWidthTable[FLASH_POLICY] +
				   g_bandWidthTable[RELAY_V2_SENT]+
				   g_bandWidthTable[YP_SENT] +
				   g_bandWidthTable[AUTH_AND_METRICS];
		}
		case bandWidth::ALL_RECV:
		{
			return g_bandWidthTable[SOURCE_V1_RECV] +
				   g_bandWidthTable[SOURCE_V2_RECV] +
				   g_bandWidthTable[RELAY_MISC_RECV] +
				   g_bandWidthTable[RELAY_V1_RECV] +
				   g_bandWidthTable[RELAY_V2_RECV] +
				   g_bandWidthTable[YP_RECV] +
				   g_bandWidthTable[ADVERTS];
		}
		case bandWidth::ALL_WEB:
		{
			return g_bandWidthTable[PUBLIC_WEB] +
				   g_bandWidthTable[PRIVATE_WEB];
		}
		case bandWidth::ALL_SOURCE_SENT:
		{
			return g_bandWidthTable[SOURCE_V1_SENT] +
				   g_bandWidthTable[SOURCE_V2_SENT];
		}
		case bandWidth::ALL_SOURCE_RECV:
		{
			return g_bandWidthTable[SOURCE_V1_RECV] +
				   g_bandWidthTable[SOURCE_V2_RECV];
		}
		case bandWidth::ALL_CLIENT_SENT:
		{
			return g_bandWidthTable[CLIENT_V1_SENT] +
				   g_bandWidthTable[CLIENT_V2_SENT] +
				   g_bandWidthTable[CLIENT_HTTP_SENT] +
				   g_bandWidthTable[CLIENT_FLV_SENT] +
				   g_bandWidthTable[CLIENT_M4A_SENT];
		}
		case bandWidth::ALL_RELAY_RECV:
		{
			return g_bandWidthTable[RELAY_MISC_RECV] +
				   g_bandWidthTable[RELAY_V1_RECV] +
				   g_bandWidthTable[RELAY_V2_RECV];
		}
		case bandWidth::ALL_OTHER:
		{
			return g_bandWidthTable[FLASH_POLICY] +
				   g_bandWidthTable[RELAY_V2_SENT] +
				   g_bandWidthTable[YP_SENT] +
				   g_bandWidthTable[YP_RECV] +
				   g_bandWidthTable[AUTH_AND_METRICS] +
				   g_bandWidthTable[ADVERTS];
		}
		default:
		{
			return 0;
		}
	}
}

void bandWidth::updateAmount(const bandWidth::usageType_t type, const __uint64 amount)
{
    if (amount > 0)
    {
	stackRWLock sl (g_bandwidthLock, false);
	g_bandWidthTable[type] += amount;
	g_bandWidthTable[bandWidth::ALL] += amount;
    }
}

void bandWidth::getFinalAmounts()
{
	__uint64 all = getAmount(bandWidth::ALL);
	if (all > 0)
	{
		ILOG(LOGNAME "Total: " + tos(all) +
					 ", Sent: " + tos(getAmount(bandWidth::ALL_SENT)) +
					 ", Recv: " + tos(getAmount(bandWidth::ALL_RECV)));
	}
}
