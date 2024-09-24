#pragma once
#ifndef protocol_shoutcast2Client_H_
#define protocol_shoutcast2Client_H_

#include "protocol_shoutcastClient.h"
#include <vector>

class streamData;

class protocol_shoutcast2Client: public protocol_shoutcastClient
{
private:
	typedef void (protocol_shoutcast2Client::*state_t)();
	state_t	m_state;
	state_t m_nextState;

	/// w3c logging
	streamData::uvoxMetadata_t		m_lastMetadata; // used for w3C tracking
	/////////////////////

	streamData::uvoxMetadata_t	m_cachedMetadata; // cached metadata that must be sent at connect time

	const bool	m_cdnSlave;		// based on the connection header

	void state_AttachToStream() throw(std::exception);
	void state_Close() throw(std::exception);
	void state_SendText() throw(std::exception);
	void state_InitiateStream() throw(std::exception);
	void state_SendCachedMetadata() throw(std::exception);
	void state_SendCachedStreamAlbumArt() throw(std::exception);
	void state_SendCachedPlayingAlbumArt() throw(std::exception);

	void logW3C() throw();
	const int doFrameSync(const int type, const bool debug, const int len,
			const int offset, const std::vector<__uint8>& buf,
			const time_t cur_time, const int bitrate,
			const unsigned int samplerate, int &frames,
			bool &advert, const bool fill_remainder = false) throw();

	virtual void timeSlice() throw(std::exception);
	virtual void setCallback (protocol_shoutcastClient::state_t callback, protocol_shoutcastClient::state_t next);
	virtual uniString::utf8 name() const throw() { return "protocol_shoutcast2Client"; }

public:
    protocol_shoutcast2Client (protocol_HTTPStyle &hs, const streamData::streamID_t streamID,const uniString::utf8 &hostName, const uniString::utf8 &addr,const uniString::utf8 &XFF, const bool cdnSlave);
	virtual ~protocol_shoutcast2Client() throw();

    virtual void setIntro(vector<__uint8> &v, int uvoxDataType);

	void return_403(void);
};

#endif
