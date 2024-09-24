#pragma once
#ifndef protocol_HTTPClient_H_
#define protocol_HTTPClient_H_

#include "protocol_shoutcastClient.h"
#include <vector>

class streamData;

class protocol_HTTPClient: public protocol_shoutcastClient
{
private:

	typedef void (protocol_HTTPClient::*state_t)();
	state_t m_state;
	state_t m_nextState;

	void state_AttachToStream() throw(std::exception);
	void state_Close() throw(std::exception);
	void state_SendText() throw(std::exception);
	void state_InitiateStream() throw(std::exception);
	void state_SendIntro() throw(std::exception);

	virtual void timeSlice() throw(std::exception);
	virtual uniString::utf8 name() const throw() { return "protocol_HTTPClient"; }

public:
    protocol_HTTPClient (protocol_HTTPStyle &hs, const streamData::streamID_t streamID, const uniString::utf8 &hostName,
            const uniString::utf8 &addr, const uniString::utf8 &XFF) throw(std::exception);

	virtual ~protocol_HTTPClient() throw();

	virtual void setCallback (protocol_shoutcastClient::state_t callback = NULL, protocol_shoutcastClient::state_t next = NULL);

	void return_403(void);
};

#endif
