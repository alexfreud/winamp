#ifdef _WIN32
#include <winsock2.h>
#endif

#include "protocol_shoutcastClient.h"
#include "stats.h"
#include "agentList.h"
#include "services/stdServiceImpl.h"
#include <map>

using namespace std;
using namespace uniString;
using namespace stringUtil;

static AOL_namespace::mutex	g_statLock;

#define DEBUG_LOG(...) do { if (gOptions.statsDebug()) DLOG(__VA_ARGS__); } while (0)

#define LOGNAME "[STATS] "

const utf8 EMPTY_AGENT("**EMPTY**");

struct statTableEntry_t
{
	size_t m_peakListeners;
	size_t m_totalStreamHits;
	typedef map<size_t, stats::clientData_t*> clientEntry_t;
	clientEntry_t m_clientData;

	statTableEntry_t() : m_peakListeners(0), m_totalStreamHits(0) {}
};

typedef map<streamData::streamID_t,statTableEntry_t> streamStatTable_t;

static streamStatTable_t g_streamStatTable;
static size_t g_totalClients;
static size_t g_uniqueClientId;

const size_t stats::getNewClientId()
{
	return ++g_uniqueClientId;
}

static bool _kickNonRipClientFromStream(const streamData::streamID_t id) throw()
{
	bool kicked = false;

	// first try our stream
	streamStatTable_t::iterator ti = g_streamStatTable.find(id);
	if (ti != g_streamStatTable.end())
	{
		statTableEntry_t &ste = (*ti).second;
		if (!ste.m_clientData.empty())
		{
			statTableEntry_t::clientEntry_t &ce = ste.m_clientData;

			for (statTableEntry_t::clientEntry_t::iterator ci = ce.begin(); (ci != ce.end()) && (!kicked); ++ci)
			{
				if ((!(*ci).second->m_kicked) && (!(*ci).second->m_ripClient))
				{
					(*ci).second->m_kicked = true;
					if ((*ci).second->m_client)
					{
						(*ci).second->m_client->kickNextRound();
					}
					kicked = true;
				}
			}
		}
	}

	return kicked;
}

static bool _kickRandomNonRipClient(const streamData::streamID_t id, const bool anyStream) throw()
{
	bool kicked = _kickNonRipClientFromStream(id);
	if ((!kicked) && anyStream)
	{
		for (streamStatTable_t::const_iterator i = g_streamStatTable.begin(); (i != g_streamStatTable.end()) && (!kicked); ++i)
		{
			kicked = _kickNonRipClientFromStream((*i).first);
		}
	}

	return kicked;
}

const int stats::addClient(const streamData::streamID_t id, const utf8 &hostName,
						   const utf8 &ipAddr, const utf8 &userAgent, const bool ripClient,
						   const size_t clientUnique, protocol_shoutcastClient *client)
{
	if (id > 0)
	{
		stackLock sml(g_statLock);

		// to prevent some of the stats / logs getting skewed
		// then we filter out the SHOUTcast site 'test' users
		if (isUserAgentOfficial(toLower(userAgent)))
		{
			return 1;
		}

		statTableEntry_t &ste = g_streamStatTable[id];
		++ste.m_totalStreamHits;

		if (ste.m_clientData.find(clientUnique) == ste.m_clientData.end())
		{
			// seen this crop up a bit and seem dodgy so rejecting (may change based on usage feedback)
			if (userAgent.empty() && gOptions.blockEmptyUserAgent())
			{
				return -1;
			}

			// this will check against a blocked 'user agent' list
			// so we can give stations a means to block bad clients
			// e.g. Winamp/5.0 or Bass/2.x or something like that
			if (!userAgent.empty() && g_agentList.find(userAgent, ((gOptions.read_stream_agentFile(id) && !gOptions.stream_agentFile(id).empty()) ? id : 0)))
			{
				return -2;
			}

			streamData::streamInfo info;
			streamData::extraInfo extra;
			streamData::getStreamInfo(id, info, extra);
			const int _maxUser = gOptions.maxUser(),
					  maxUsers = ((info.m_streamMaxUser > 0) && (info.m_streamMaxUser < _maxUser) ? info.m_streamMaxUser : _maxUser);

			const size_t num_clients = ste.m_clientData.size();
			const bool over = ((maxUsers && ((int)num_clients >= maxUsers)) ||
							   (_maxUser && ((int)g_totalClients >= _maxUser)));
			if (over && !ripClient)
			{
				return 0; // too many, and we are not allowed to boot anyone
			}

			if (over && ripClient)	// too many, and we are allowed to try to boot someone...
			{
				// if total reserved is already at the listener limit then nothing else to do
				if (getTotalRipClients(id) < num_clients)
				{
					// we only allow the new listener to join as long as we can kick someone
					if (!_kickRandomNonRipClient(id, true))
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}

			ste.m_clientData[clientUnique] = new stats::clientData_t(hostName, ipAddr, ripClient, client);
			ste.m_peakListeners = max(ste.m_clientData.size(), ste.m_peakListeners);
			++g_totalClients;

			DEBUG_LOG(LOGNAME "System wide listener total now " + tos(g_totalClients));

			return 1;
		}
	}
	return 0;
}


// hacky, to maintain a count in here, should be one in sd
long stats::getUserCount (const streamData::streamID_t id)
{
    long c = 0;
    if (id > 0)
    {
        stackLock sml(g_statLock);
        statTableEntry_t &ste = g_streamStatTable[id];
        c = ste.m_clientData.size();
    }
    return c;
}


void stats::removeClient(const streamData::streamID_t id, const size_t clientUnique)
{
	if ((id > 0) && (g_totalClients > 0))
	{
		stackLock sml(g_statLock);

		streamStatTable_t::iterator ti = g_streamStatTable.find(id);
		if (ti != g_streamStatTable.end())
		{
			statTableEntry_t &ste = (*ti).second;
			if (!ste.m_clientData.empty())
			{
				statTableEntry_t::clientEntry_t::iterator i = ste.m_clientData.find(clientUnique);
				if (i != ste.m_clientData.end())
				{
					delete i->second;
					ste.m_clientData.erase(clientUnique);

					g_totalClients -= 1;
					DEBUG_LOG(LOGNAME "System wide listener total now " + tos(g_totalClients));
				}
			}
		}
	}
}

// get stats. Some values, like newSessions and newConnects set a flag so that clients are only counted once.
// This is fine for touch reporting, but sometimes we want to fetch this information and, since we are not touching YP,
// do not want to set the flags indicating that things have been counted. The "resetAccumulators" flag controls this.
// Set to true for touch, false otherwise
void stats::getStats(streamData::streamID_t id, statsData_t &data, bool resetAccumulators) throw()
{
	if (id > 0)
	{
		stackLock sml(g_statLock);

		streamStatTable_t::iterator ti = g_streamStatTable.find(id);
		if (ti != g_streamStatTable.end())
		{
			statTableEntry_t &ste = (*ti).second;

			data.peakListeners = ste.m_peakListeners;
			data.totalStreamHits = ste.m_totalStreamHits;

			if (!ste.m_clientData.empty())
			{
				set<utf8> ipTable;

				time_t t = ::time(NULL);
				data.connectedListeners = ste.m_clientData.size();
				__uint64 total_listen_time = 0;
				for (statTableEntry_t::clientEntry_t::iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
				{
					if (!(*i).second->m_kicked)
					{
						if (ipTable.find((*i).second->m_ipAddr) == ipTable.end())
						{
							++data.uniqueListeners;
						}
						ipTable.insert((*i).second->m_ipAddr);
					}

					total_listen_time += (t - (*i).second->m_client->getStartTime());
					if (!(*i).second->m_connectCounted)
					{
						if (resetAccumulators)
						{
							(*i).second->m_connectCounted = true;
						}
						++data.newConnects;
					}

					if ((!(*i).second->m_fiveMinuteCumeCounted) && ((t - (*i).second->m_client->getStartTime()) > (5 * 60)))
					{
						if (resetAccumulators)
						{
							(*i).second->m_fiveMinuteCumeCounted = true;
						}
						++data.newSessions;
					}
				}

				data.avgUserListenTime = (data.connectedListeners ? (int)(total_listen_time / data.connectedListeners) : 0);
			}
		}
	}
}

void stats::getFinalStats() throw()
{
	stackLock sml(g_statLock);

	size_t totalPeak = 0;
	utf8 msg;

	if (!g_streamStatTable.empty())
	{
		for (streamStatTable_t::const_iterator ti = g_streamStatTable.begin(); ti != g_streamStatTable.end(); ++ti)
		{
			size_t peakListeners = (*ti).second.m_peakListeners;
			totalPeak += peakListeners;
			msg += (msg.size() > 0 ? ",#" : "#") + tos((*ti).first) + ":" + tos(peakListeners);
		}
	}

	if (totalPeak > 0)
	{
		msg += (!g_streamStatTable.empty() ? "," : (utf8)"") + "Total: " + tos(totalPeak);
		ILOG(LOGNAME "Peak numbers: " + msg);
	}
}

const streamData::streamIDs_t stats::getActiveStreamIds() throw()
{
	stackLock sml(g_statLock);

	streamData::streamIDs_t activeIds;

	for (streamStatTable_t::const_iterator ti = g_streamStatTable.begin(); ti != g_streamStatTable.end(); ++ti)
	{
		// we check this to make sure that we're only
		// adding 'active' details and skip inactive
		const statTableEntry_t &ste = (*ti).second;
		if (!ste.m_clientData.empty())
		{
			if (activeIds.find((*ti).first) == activeIds.end())
			{
				activeIds.insert((*ti).first);
			}
		}
	}

	return activeIds;
}

const size_t stats::getTotalUniqueListeners() throw()
{
	stackLock sml(g_statLock);

	size_t uniqueListeners = 0;

	set<utf8> ipTable;

	for (streamStatTable_t::const_iterator ti = g_streamStatTable.begin(); ti != g_streamStatTable.end(); ++ti)
	{
		const statTableEntry_t &ste = (*ti).second;
		if (!ste.m_clientData.empty())
		{
			for (statTableEntry_t::clientEntry_t::const_iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
			{
				if (!(*i).second->m_kicked)
				{
					if (ipTable.find((*i).second->m_ipAddr) == ipTable.end())
					{
						++uniqueListeners;
					}
					ipTable.insert((*i).second->m_ipAddr);
				}
			}
		}
	}

	return uniqueListeners;
}

const size_t stats::getTotalRipClients(streamData::streamID_t id) throw()
{
	size_t uniqueListeners = 0;

	if (id > 0)
	{
		streamStatTable_t::const_iterator ti = g_streamStatTable.find(id);
		if (ti != g_streamStatTable.end())
		{
			const statTableEntry_t &ste = (*ti).second;
			if (!ste.m_clientData.empty())
			{
				for (statTableEntry_t::clientEntry_t::const_iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
				{
					if (!(*i).second->m_kicked && (*i).second->m_ripClient)
					{
						++uniqueListeners;
					}
				}
			}
		}
	}

	return uniqueListeners;
}

void stats::updateRipClients(const streamData::streamID_t id, const uniString::utf8& ripAddr, const bool mode)
{
	stackLock sml(g_statLock);

	if (id)
	{
		streamStatTable_t::iterator ti = g_streamStatTable.find(id);
		if (ti != g_streamStatTable.end())
		{
			statTableEntry_t &ste = (*ti).second;
			if (!ste.m_clientData.empty())
			{
				for (statTableEntry_t::clientEntry_t::iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
				{
					if (!(*i).second->m_kicked && ((*i).second->m_ipAddr == ripAddr))
					{
						(*i).second->m_ripClient = mode;
					}
				}
			}
		}
	}
	else
	{
		for (streamStatTable_t::iterator i = g_streamStatTable.begin(); i != g_streamStatTable.end(); ++i)
		{
			statTableEntry_t &ste = (*i).second;
			if (!ste.m_clientData.empty())
			{
				for (statTableEntry_t::clientEntry_t::iterator c = ste.m_clientData.begin(); c != ste.m_clientData.end(); ++c)
				{
					if (!(*c).second->m_kicked && ((*c).second->m_ipAddr == ripAddr))
					{
						(*c).second->m_ripClient = mode;
					}
				}
			}
		}
	}
}

void stats::resetStats(const streamData::streamID_t id) throw()
{
	if (id > 0)
	{
		stackLock sml(g_statLock);

		streamStatTable_t::iterator ti = g_streamStatTable.find(id);
		if (ti != g_streamStatTable.end())
		{
			statTableEntry_t &ste = (*ti).second;
			// reset peak and stream hits to current client connection level
			ste.m_totalStreamHits = ste.m_peakListeners = ste.m_clientData.size();
		}
	}
}

void stats::updatePeak(const streamData::streamID_t id, const size_t peakListeners) throw()
{
	if (id > 0)
	{
		stackLock sml(g_statLock);

		streamStatTable_t::iterator ti = g_streamStatTable.find(id);
		statTableEntry_t &ste = ((ti != g_streamStatTable.end()) ? (*ti).second : g_streamStatTable[id]);
		ste.m_peakListeners = max(peakListeners, ste.m_peakListeners);
	}
}

void stats::updateTriggers(const streamData::streamID_t id, const size_t unique) throw()
{
	if (id > 0)
	{
		stackLock sml(g_statLock);

		streamStatTable_t::iterator ti = g_streamStatTable.find(id);
		if (ti != g_streamStatTable.end())
		{
			statTableEntry_t &ste = (*ti).second;
			if (!ste.m_clientData.empty())
			{
				for (statTableEntry_t::clientEntry_t::iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
				{
					if (!(*i).second->m_kicked && ((*i).first == unique))
					{
						++(*i).second->m_triggers;
						break;
					}
				}
			}
		}
	}
}

static bool sortCurrentClientDataByTime(const stats::currentClientData_t* a, const stats::currentClientData_t* b)
{
	return (a->m_startTime < b->m_startTime);
}

// get all client data blocks for stream
void stats::getClientDataForStream(const streamData::streamID_t id, currentClientList_t &client_data) throw()
{
	stackLock sml(g_statLock);

	if (id > 0)
	{
		streamStatTable_t::const_iterator ti = g_streamStatTable.find(id);
		if (ti != g_streamStatTable.end())
		{
			const statTableEntry_t &ste = (*ti).second;
			if (!ste.m_clientData.empty())
			{
				for (statTableEntry_t::clientEntry_t::const_iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
				{
					if (!(*i).second->m_kicked)
					{
						currentClientData_t* client = new currentClientData_t((*i).second->m_hostName, (*i).second->m_ipAddr,
																			  (*i).second->m_client->getXFF(), (*i).second->m_client->getUserAgent(),
																			  (*i).second->m_client->getReferer(), (*i).second->m_triggers,
																			  (*i).second->m_client->getUnique(), (*i).second->m_client->getStartTime(),
																			  (*i).second->m_client->getGroup(), (*i).second->m_client->getClientType(),
																			  (*i).second->m_fiveMinuteCumeCounted, (*i).second->m_connectCounted,
																			  (*i).second->m_ripClient, (*i).second->m_kicked);
						client_data.push_back(client);
					}
				}
			}
		}
	}
	else
	{
		for (streamStatTable_t::const_iterator i = g_streamStatTable.begin(); i != g_streamStatTable.end(); ++i)
		{
			const statTableEntry_t &ste = (*i).second;
			if (!ste.m_clientData.empty())
			{
				for (statTableEntry_t::clientEntry_t::const_iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
				{
					if (!(*i).second->m_kicked)
					{
						currentClientData_t* client = new currentClientData_t((*i).second->m_hostName, (*i).second->m_ipAddr,
																			  (*i).second->m_client->getXFF(), (*i).second->m_client->getUserAgent(),
																			  (*i).second->m_client->getReferer(), (*i).second->m_triggers,
																			  (*i).second->m_client->getUnique(), (*i).second->m_client->getStartTime(),
																			  (*i).second->m_client->getGroup(), (*i).second->m_client->getClientType(),
																			  (*i).second->m_fiveMinuteCumeCounted, (*i).second->m_connectCounted,
																			  (*i).second->m_ripClient, (*i).second->m_kicked);
						client_data.push_back(client);
					}
				}
			}
		}
	}

	std::sort(client_data.begin(), client_data.end(), sortCurrentClientDataByTime);
}

void stats::getClientDataForKicking(const streamData::streamID_t id, kickClientList_t &kick_data) throw()
{
	stackLock sml(g_statLock);

	if (id > 0)
	{
		streamStatTable_t::const_iterator ti = g_streamStatTable.find(id);
		if (ti != g_streamStatTable.end())
		{
			const statTableEntry_t &ste = (*ti).second;
			if (!ste.m_clientData.empty())
			{
				for (statTableEntry_t::clientEntry_t::const_iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
				{
					if (!(*i).second->m_kicked)
					{
						kickClientData_t* client = new kickClientData_t((*i).second->m_client->getUserAgent(),
																		(*i).second->m_client->getUnique(),
																		(*i).second->m_kicked);
						kick_data.push_back(client);
					}
				}
			}
		}
	}
	else
	{
		for (streamStatTable_t::const_iterator i = g_streamStatTable.begin(); i != g_streamStatTable.end(); ++i)
		{
			const statTableEntry_t &ste = (*i).second;
			if (!ste.m_clientData.empty())
			{
				for (statTableEntry_t::clientEntry_t::const_iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
				{
					if (!(*i).second->m_kicked)
					{
						kickClientData_t* client = new kickClientData_t((*i).second->m_client->getUserAgent(),
																		(*i).second->m_client->getUnique(),
																		(*i).second->m_kicked);
						kick_data.push_back(client);
					}
				}
			}
		}
	}
}

void stats::catchPreAddClients(const streamData::streamID_t id)
{
	if (id > 0)
	{
		stackLock sml(g_statLock);

		// make sure the client still exists before calling it
		streamStatTable_t::iterator ti = g_streamStatTable.find(id);
		if (ti != g_streamStatTable.end())
		{
			statTableEntry_t &ste = (*ti).second;
			if (!ste.m_clientData.empty())
			{
				for (statTableEntry_t::clientEntry_t::iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
				{
					// if we find an active listener which has m_group = -1 then
					// we need to get the listener to do a listener_add request.
					if (!(*i).second->m_kicked &&
						(*i).second->m_client->m_removeClientFromStats &&
						((*i).second->m_client->getGroup() == -1))
					{
						// we only want to do this once
						// so we'll set it to group = 0
						// and it'll not be done again.
						(*i).second->m_client->setGroup(0);

						DEBUG_LOG((*i).second->m_client->m_clientLogString + "Re-authenticating listener for adverts / metrics.");

						// using this to force an attept to check but only for non
						// 'local' listener connections which won't get a group id
						if ((isRemoteAddress((*i).second->m_client->m_clientAddr) ||
							 isRemoteAddress((*i).second->m_client->m_XFF)))
						{
							(*i).second->m_client->authForStream((*i).second->m_client->m_streamData);
						}
						(*i).second->m_client->reportNewListener();
					}
				}
			}
		}
	}
}

// set flag in client so it will bail on next round. This is safe since the client object
// cannot delete itself until calling stats::removeClient() which is protected by the g_statLock	
void stats::kickClient(const streamData::streamID_t id, const size_t unique) throw()
{
	if (id > 0)
	{
		stackLock sml(g_statLock);

		// make sure the client still exists before calling it
		streamStatTable_t::iterator ti = g_streamStatTable.find(id);
		if (ti != g_streamStatTable.end())
		{
			statTableEntry_t &ste = (*ti).second;
			if (!ste.m_clientData.empty())
			{
				for (statTableEntry_t::clientEntry_t::iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
				{
					if (!(*i).second->m_kicked && ((*i).first == unique))
					{
						(*i).second->m_kicked = true;
						if ((*i).second->m_client)
						{
							(*i).second->m_client->kickNextRound();
						}
						break;
					}
				}
			}
		}
	}
}

void stats::kickClient(const streamData::streamID_t id, const uniString::utf8& ipAddr) throw()
{
	if (id > 0)
	{
		stackLock sml(g_statLock);

		// make sure the client still exists before calling it
		streamStatTable_t::iterator ti = g_streamStatTable.find(id);
		if (ti != g_streamStatTable.end())
		{
			statTableEntry_t &ste = (*ti).second;
			if (!ste.m_clientData.empty())
			{
				for (statTableEntry_t::clientEntry_t::iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
				{
					if (!(*i).second->m_kicked && ((*i).second->m_ipAddr == ipAddr))
					{
						(*i).second->m_kicked = true;
						if ((*i).second->m_client)
						{
							(*i).second->m_client->kickNextRound();
						}
					}
				}
			}
		}
	}
}

const bool stats::kickAllClients(const streamData::streamID_t id, const bool allStreams) throw()
{
	bool kicked = false;
	if ((id > 0) && !allStreams)
	{
		stackLock sml(g_statLock);

		// make sure the client still exists before calling it
		streamStatTable_t::iterator ti = g_streamStatTable.find(id);
		if (ti != g_streamStatTable.end())
		{
			statTableEntry_t &ste = (*ti).second;
			if (!ste.m_clientData.empty())
			{
				for (statTableEntry_t::clientEntry_t::iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
				{
					if (!(*i).second->m_kicked)
					{
						(*i).second->m_kicked = true;
						if ((*i).second->m_client)
						{
							(*i).second->m_client->kickNextRound();
							kicked = true;
						}
					}
				}
			}
		}
	}
	else if (!id && allStreams)
	{
		stackLock sml(g_statLock);

		for (streamStatTable_t::iterator ti = g_streamStatTable.begin(); ti != g_streamStatTable.end(); ++ti)
		{
			statTableEntry_t &ste = (*ti).second;
			if (!ste.m_clientData.empty())
			{
				for (statTableEntry_t::clientEntry_t::iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
				{
					if (!(*i).second->m_kicked)
					{
						(*i).second->m_kicked = true;
						if ((*i).second->m_client)
						{
							(*i).second->m_client->kickNextRound();
							kicked = true;
						}
					}
				}
			}
		}
	}

	return kicked;
}

static bool sortClientDataByTime(const stats::clientData_t &a, const stats::clientData_t &b)
{
	return (a.m_client->getStartTime() < b.m_client->getStartTime());
}

const bool stats::kickDuplicateClients(const streamData::streamID_t id) throw()
{
	bool kicked = false;
	if (id > 0)
	{
		stackLock sml(g_statLock);

		// we first spin through all of the connected listeners and work out
		// which listener addresses have more than one connection against it
		streamStatTable_t::const_iterator ti = g_streamStatTable.find(id);
		if (ti != g_streamStatTable.end())
		{
			map<utf8, size_t> ipTable;

			const statTableEntry_t &ste = (*ti).second;
			if (!ste.m_clientData.empty())
			{
				for (statTableEntry_t::clientEntry_t::const_iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
				{
					if (!(*i).second->m_kicked)
					{
						if (ipTable.find((*i).second->m_ipAddr) == ipTable.end())
						{
							ipTable[(*i).second->m_ipAddr] = 1;
						}
						else
						{
							++ipTable[(*i).second->m_ipAddr];
						}
					}
				}
			}

			if (!ipTable.empty())
			{
				for (map<utf8, size_t>::const_iterator ip = ipTable.begin(); ip != ipTable.end(); ++ip)
				{
					// we now only look at addresses with multiple clients
					// being reported for the address which has been noted
					if (ip->second > 1)
					{
						map<utf8, size_t> agentTable;

						for (statTableEntry_t::clientEntry_t::const_iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
						{
							if (!(*i).second->m_kicked && (ip->first == (*i).second->m_ipAddr))
							{
								const utf8 userAgent = (*i).second->m_client->getUserAgent();
								if (agentTable.find(userAgent) == agentTable.end())
								{
									agentTable[userAgent] = 1;
								}
								else
								{
									++agentTable[userAgent];
								}
							}
						}

						if (!agentTable.empty())
						{
							std::vector<clientData_t> data;
							for (map<utf8, size_t>::const_iterator ai = agentTable.begin(); ai != agentTable.end(); ++ai)
							{
								// this should now just leave us with duplicate
								// user-agents connected from the same address
								if (ai->second > 1)
								{
									// now we need to process through and get the
									// details needed so we can finally kick them
									for (statTableEntry_t::clientEntry_t::const_iterator i = ste.m_clientData.begin(); i != ste.m_clientData.end(); ++i)
									{
										if (!(*i).second->m_kicked &&
											(ip->first == (*i).second->m_ipAddr) &&
											(ai->first == (*i).second->m_client->getUserAgent()))
										{
											data.push_back(*(*i).second);
										}
									}
								}
							}

							// now we've got data, we sort by data and then
							// process through the final set of data & kick
							if (!data.empty())
							{
								std::sort(data.begin(), data.end(), sortClientDataByTime);
								// remove the newest and kick the remainder
								data.pop_back();

								for (vector<stats::clientData_t>::iterator i = data.begin(); i != data.end(); ++i)
								{
									if (!(*i).m_kicked)
									{
										(*i).m_kicked = true;
										if ((*i).m_client)
										{
											(*i).m_client->kickNextRound();
											kicked = true;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return kicked;
}

const bool stats::kickRandomNonRipClient(const streamData::streamID_t id, const bool anyStream) throw()
{
	if (id > 0)
	{
		stackLock sml(g_statLock);
		return _kickRandomNonRipClient(id, anyStream);
	}
	return false;
}
