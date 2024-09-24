#pragma once
#ifndef protocol_relay_H_
#define protocol_relay_H_

#include "threadedRunner.h"

/*
	Runnable object that handles the initial part of a relay connection.
	Makes the connection to the source and determines what type of protocol
	should be used, then hands off to that particular protocol
	(shoutcast or uvox relay)
*/

class protocol_relay: public runnable
{			
private:
	u_short							m_srcPort;			// server port
	short							m_redirectCount;

	uniString::utf8	m_srcAddrName;		// server DNS name or value as specified in relayInfo
	std::string		m_srcAddrNumeric;	// resolved numeric addr
	uniString::utf8	m_srcURLpart;		// server 

	uniString::utf8	m_srcLogString;
	httpHeaderMap_t	m_HTTPHeaders;
	uniString::utf8	m_HTTPGreetingResponse; // first line of response

	const uniString::utf8::value_type	*m_outBuffer; // for outgoing text lines
	int				m_outBufferSize;

	bool			m_relayWaitingToReconnect;
	bool			m_relaySentConnectWait;
	bool			m_retryRelay;
	bool			m_skip;
    bool            m_backupStarted;
    bool            m_registered;       // true if runnable is ok to start. new relay have to wait for exisitng one to drop

	uniString::utf8	m_lineBuffer;		// in/out lines

	config::streamConfig		m_relayInfo;
	const config::streamConfig	m_originalRelayInfo; // used for reconnects later on

	/// reconnects
	int				m_retryCount;
	time_t			m_relayReconnectStartTime;

	typedef void (protocol_relay::*state_t)();

	state_t	m_state;
	state_t m_nextState;

	void state_Initial() throw(std::exception);
	void state_ResolveServer() throw(std::exception);
	void state_Connect() throw(std::exception);
	void state_ConnectWait() throw(std::exception);
	void state_SendGreeting() throw(std::exception);
	void state_GetGreetingResponse() throw(std::exception);
	void state_AnalyzeGreetingResponse() throw(std::exception);
	void state_DetermineProtocol() throw(std::exception);
	void state_Send() throw(std::exception);
	void state_GetLine() throw(std::exception);
	void startBackupConnection(uniString::utf8 errorMessage) throw(std::exception);

protected:
	virtual void timeSlice() throw(std::exception);
	virtual uniString::utf8 name() const throw() { return "protocol_relay"; }

public:
	protocol_relay(const config::streamConfig &info, const bool retry = false) throw();
	virtual ~protocol_relay() throw();
};

#endif
