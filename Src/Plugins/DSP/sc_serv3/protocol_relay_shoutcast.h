#pragma once
#ifndef protocol_relay_shoutcast_H_
#define protocol_relay_shoutcast_H_

#include "threadedRunner.h"

class streamData;

class protocol_relay_shoutcast: public runnable
{
	int m_originalBitrate;	// original bitrate for checking
	uniString::utf8 m_originalMimeType;		// original mimetype for checking
	const config::streamConfig m_originalRelayInfo;

	short unsigned int m_metadataInterval;	// interval on source
	const bool m_backup;	// used to change log output depending on relay or backup usage
	bool m_denied;			// used to prevent source disconnected messages e.g. for failed passwords

	short unsigned int m_remainderSize;
	__uint8 *m_remainder;

	const uniString::utf8	m_srcAddrName;
	const uniString::utf8	m_srcAddrNumeric;
	const uniString::utf8	m_srcURLpart;

	uniString::utf8	m_metadataBuffer;

	streamData *m_streamData;
	const uniString::utf8	m_srcLogString;

	typedef void (protocol_relay_shoutcast::*state_t)();
	state_t	m_state;

	int m_bytesSinceMetadata;
	int m_metadataSizeByte;	// metadata length indicator

	void state_GetStreamData() throw(std::exception);
	void state_GetMetadata() throw(std::exception);
	void state_Fail() throw(std::exception);
	void state_CloseConnection() throw(std::exception);

	void cleanup();

protected:
	virtual void timeSlice() throw(std::exception);
	virtual uniString::utf8 name() const throw() { return "protocol_relay_shoutcast"; }

public:
	protocol_relay_shoutcast(const socketOps::tSOCKET s, const config::streamConfig &originalRelayInfo,
							 const uniString::utf8 &srcAddrName, const uniString::utf8 &srcAddrNumeric,
							 const int srcPort, const uniString::utf8 &srcURLpart,
							 httpHeaderMap_t &httpHeaders, const int originalbitrate = 0,
							 const uniString::utf8& originalMimeType = "", const bool backup = false);
	~protocol_relay_shoutcast() throw();
};

#endif
