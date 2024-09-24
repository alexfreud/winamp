#pragma once
#ifndef protocol_shoutcast1Client_H_
#define protocol_shoutcast1Client_H_

#include "protocol_shoutcastClient.h"
#include <vector>

class streamData;

class protocol_shoutcast1Client: public protocol_shoutcastClient
{
private:

	typedef void (protocol_shoutcast1Client::*state_t)();
	state_t m_state;
	state_t m_nextState;

	std::vector<__uint8>	m_ICYMetadata; // metadata buffer

	void state_AttachToStream() throw(std::exception);
	void state_Close() throw(std::exception);
	void state_InitiateStream() throw(std::exception);
	void state_SendIntro() throw(std::exception);

	void sendICYMetadata(const uniString::utf8 &md) throw();

	virtual void timeSlice() throw(std::exception);
	virtual uniString::utf8 name() const throw() { return "protocol_shoutcast1Client"; }

public:
    protocol_shoutcast1Client (protocol_HTTPStyle &hs, const streamData::streamID_t streamID,
            const uniString::utf8 &hostName, const uniString::utf8 &addr, const uniString::utf8 &XFF) throw(std::exception);

	virtual ~protocol_shoutcast1Client() throw();

	virtual void setCallback (protocol_shoutcastClient::state_t callback = NULL, protocol_shoutcastClient::state_t next = NULL);
	virtual int doSend(const bool debug, const time_t cur_time, const int autoDumpTime, int adjust = 0) throw(std::runtime_error);

	void return_403(void);
};

#endif
