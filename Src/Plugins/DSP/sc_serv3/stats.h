#pragma once
#ifndef stats_H_
#define stats_H_

#include "streamData.h"

extern uniString::utf8 niceURL(uniString::utf8 srcAddr);

extern const uniString::utf8 EMPTY_AGENT;

namespace stats
{
	struct clientData_t
	{
		uniString::utf8				m_hostName;			// holds the hostname (usually only available with namelookups=1)
		uniString::utf8				m_ipAddr;
		size_t						m_triggers;			// number of valid advert triggers done for the client
		protocol_shoutcastClient*	m_client;			// reference to the client (mainly for kicking)
		bool						m_fiveMinuteCumeCounted;	// already reported against 5 minute session (cm)
		bool						m_connectCounted;	// already reported against new connects (ht)
		bool						m_ripClient;		// client cannot be autobooted
		bool						m_kicked;			// was ordered to be kicked and will go away soon

		clientData_t(const uniString::utf8 &hostName, const uniString::utf8 &ipAddr,
					 const bool ripClient, protocol_shoutcastClient* client) throw()
					: m_hostName(niceURL(hostName)), m_ipAddr(niceURL(ipAddr)),
					  m_triggers(0), m_client(client), m_fiveMinuteCumeCounted(false),
					  m_connectCounted(false), m_ripClient(ripClient), m_kicked(false) {}
	};

	const size_t getNewClientId();

	// return false if reach maxUsers for that stream or all streams. If ripClient == true, then this client MUST be added, even
	// if it means we have to boot someone
	const int addClient(const streamData::streamID_t id, const uniString::utf8& hostName,
						const uniString::utf8 &ipAddr, const uniString::utf8 &userAgent,
						const bool ripClient, const size_t clientUnique,
						protocol_shoutcastClient *client);
	void removeClient(const streamData::streamID_t id, const size_t clientUnique);

	struct statsData_t
	{
		size_t connectedListeners;	// li
		time_t avgUserListenTime;	// alt
		size_t newSessions;			// cm
		size_t newConnects;			// ht
		size_t peakListeners;
		size_t uniqueListeners;
		size_t totalStreamHits;		// total hits against stream since start

		statsData_t() throw() : connectedListeners(0), avgUserListenTime(0),
								newSessions(0), newConnects(0), peakListeners(0),
								uniqueListeners(0), totalStreamHits(0) {}
	};

	// get stats. Some values, like newSessions and newConnects set a flag so that clients are only counted once.
	// This is fine for touch reporting, but sometimes we want to fetch this information and, since we are not touching YP,
	// do not want to set the flags indicating that things have been counted. The "resetAccumulators" flag controls this.
	// Set to true for touch, false otherwise	
	void getStats(const streamData::streamID_t id, statsData_t &data, bool resetAccumulators = false) throw();

	// provides log message when the DNAS is stopped of stream 'peak' values
	void getFinalStats() throw();

	// works out the total number of unique listeners even if connected to different streams
	const size_t getTotalUniqueListeners() throw();

	// works out the active stream ids so we can also catch listeners
	// on non-configured streams but are being provided the backup
	const streamData::streamIDs_t getActiveStreamIds() throw();

    // get the current number of listeners on the specified stream
    long getUserCount (const streamData::streamID_t id);

	// wsorks out the total number of reserved listeners on the specified stream
	const size_t getTotalRipClients(const streamData::streamID_t id) throw();

	// updates the appropriate clients (by stream) against any changes to the reserved list
	void updateRipClients(const streamData::streamID_t id, const uniString::utf8& ripAddr, const bool mode);

	// only resets peakListeners and totalStreamHits currently with peakListeners set to current listeners if there are any
	void resetStats(const streamData::streamID_t id) throw();

	// used only during a YP 'add' to help restore the previous peak level (if applicable)
	void updatePeak(const streamData::streamID_t id, const size_t peakListeners) throw();

	void updateTriggers(const streamData::streamID_t id, const size_t unique) throw();

	struct currentClientData_t
	{
		const uniString::utf8		m_hostName;		// holds the hostname (usually only available with namelookups=1)
		const uniString::utf8		m_ipAddr;
		const uniString::utf8		m_XFF;
		const uniString::utf8		m_userAgent;
		const uniString::utf8		m_referer;

		const size_t				m_triggers;		// number of valid advert triggers done for the client
		const size_t				m_unique;		// clients' unique id

		const time_t				m_startTime;	// when connection started, used to implement listenerTime

		const int					m_group;		// advert group id
		const streamData::source_t	m_clientType;	// stream client type

		const bool					m_fiveMinuteCumeCounted;	// already reported against 5 minute session (cm)
		const bool					m_connectCounted;			// already reported against new connects (ht)
		const bool					m_ripClient;				// client cannot be autobooted
		const bool					m_kicked;					// was ordered to be kicked and will go away soon

		currentClientData_t(const uniString::utf8 &hostName, const uniString::utf8 &ipAddr,
							const uniString::utf8 &XFF, const uniString::utf8 &userAgent,
							const uniString::utf8 &referer, const size_t triggers,
							const size_t unique, const time_t startTime,
							const int group, const streamData::source_t	clientType,
							const bool fiveMinuteCumeCounted, const bool connectCounted,
							const bool ripClient, const bool kicked) throw()
						: m_hostName(niceURL(hostName)), m_ipAddr(niceURL(ipAddr)), m_XFF(XFF),
						  m_userAgent(userAgent), m_referer(referer), m_triggers(triggers),
						  m_unique(unique), m_startTime(startTime), m_group(group),
						  m_clientType(clientType), m_fiveMinuteCumeCounted(fiveMinuteCumeCounted),
						  m_connectCounted(connectCounted), m_ripClient(ripClient), m_kicked(kicked) {}
	};

	typedef std::vector<currentClientData_t*> currentClientList_t;
	void getClientDataForStream(const streamData::streamID_t id, currentClientList_t &client_data) throw();


	struct kickClientData_t
	{
		const uniString::utf8		m_userAgent;
		const size_t				m_unique;		// clients' unique id
		const bool					m_kicked;					// was ordered to be kicked and will go away soon

		kickClientData_t(const uniString::utf8 &userAgent, const size_t unique, const bool kicked) throw()
						: m_userAgent(userAgent), m_unique(unique), m_kicked(kicked) {}
	};

	typedef std::vector<kickClientData_t*> kickClientList_t;
	void getClientDataForKicking(const streamData::streamID_t id, kickClientList_t &kick_data) throw();

	void catchPreAddClients(const streamData::streamID_t id);

	// kick client if it's still around
	void kickClient(const streamData::streamID_t id, const size_t unique) throw();
	void kickClient(const streamData::streamID_t id, const uniString::utf8& ipAddr) throw();

	// kick all clients currently connected
	const bool kickAllClients(const streamData::streamID_t id, const bool allStreams = false) throw();

	// kick all duplicate clients currently connected
	// based on the older first and by the user-agent
	const bool kickDuplicateClients(const streamData::streamID_t id) throw();

	// kick a random nonRip client. return true if successful. If anyStream, then we can boot
	// someone from another stream if we have to
	const bool kickRandomNonRipClient(const streamData::streamID_t id, const bool anyStream = true) throw();

	struct uniqueClientData_t
	{
		time_t m_connectTime;			// longest connection time of group
		uniString::utf8	m_hostName;		// holds the hostname (usually only available with namelookups=1)
		uniString::utf8 m_ipAddr;
		uniString::utf8	m_XFF;
		uniString::utf8 m_userAgent;	// either holds a raw or processed list of user agents
		uniString::utf8 m_unique;		// unique id of the client
		size_t m_total;					// total number of clients on this
		bool m_ripAddr;					// if the address is reserved or not

		uniqueClientData_t() throw() : m_connectTime(0), m_total(0), m_ripAddr(false) {}
	};
}

#endif
