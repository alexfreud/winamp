#pragma once
#ifndef protocol_flvClient_H_
#define protocol_flvClient_H_

#include "protocol_shoutcastClient.h"
#include <vector>

class streamData;

class protocol_flvClient: public protocol_shoutcastClient
{
private:

	typedef void (protocol_flvClient::*state_t)();
	state_t m_state;
	state_t m_nextState;

	int		m_timestamp;
	int		m_fileOffset;

	void state_AttachToStream() throw(std::exception);
	void state_Close() throw(std::exception);
	void state_SendText() throw(std::exception);
	void state_InitiateStream() throw(std::exception);
	void state_SendIntro() throw(std::exception);

	virtual void timeSlice() throw(std::exception);
	virtual uniString::utf8 name() const throw() { return "protocol_flvClient"; }

public:
    protocol_flvClient (protocol_HTTPStyle &hs, const streamData::streamID_t streamID, const uniString::utf8 &hostName, const uniString::utf8 &addr, const uniString::utf8 &XFF) throw(std::exception);

	protocol_flvClient(const socketOps::tSOCKET s, const streamData::streamID_t streamID,
					   const uniString::utf8 &hostName, const uniString::utf8 &addr,
					   const u_short port, const uniString::utf8 &userAgent,
					   const uniString::utf8 &XFF, const uniString::utf8 &referer,
					   const bool headRequest) throw(std::exception);
	virtual ~protocol_flvClient() throw();

	virtual void setCallback (protocol_shoutcastClient::state_t callback = NULL, protocol_shoutcastClient::state_t next = NULL);
	void processFrame (int type, const unsigned char *buf, unsigned int len);

	void return_403(void);
};

#endif
