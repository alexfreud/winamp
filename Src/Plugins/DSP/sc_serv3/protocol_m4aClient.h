#pragma once
#if 0
#ifndef protocol_m4aClient_H_
#define protocol_m4aClient_H_

#include "protocol_shoutcastClient.h"
#include <vector>

class streamData;

class protocol_m4aClient: public protocol_shoutcastClient
{
private:
	typedef void (protocol_m4aClient::*state_t)();
	state_t m_state;
	state_t m_nextState;

	void state_AttachToStream() throw(std::exception);
	void state_Close() throw(std::exception);
	void state_SendText() throw(std::exception);
	void state_InitiateStream() throw(std::exception);
	void state_Stream() throw(std::exception);
	void state_SendIntroFile() throw(std::exception);
	void state_SendIntro() throw(std::exception);
	void state_SendBackupFile() throw(std::exception);
	void state_SendAdverts() throw(std::exception);

	virtual void timeSlice() throw(std::exception);
	virtual uniString::utf8 name() const throw() { return "protocol_m4aClient"; }

public:
	protocol_m4aClient(const socketOps::tSOCKET s, const streamData::streamID_t streamID,
					   const uniString::utf8 &hostName, const uniString::utf8 &addr,
					   const u_short port, const uniString::utf8 &userAgent,
					   const uniString::utf8 &XFF, const uniString::utf8 &referer) throw(std::exception);
	virtual ~protocol_m4aClient() throw();

	void return_403(void);
};

#endif
#endif
