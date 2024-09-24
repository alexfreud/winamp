#pragma once
#ifndef protocol_backup_H_
#define protocol_backup_H_

#include "threadedRunner.h"

/*
	Runnable object that handles the initial part of a backup connection
	which is basically the same as a relay connection bar a different name.
	Makes the connection to the source and determines what type of protocol
	should be used, then hands off to the particular relay protocol
	(shoutcast or uvox relay)
*/

class protocol_backup: public runnable
{			
private:
	uniString::utf8 m_srcAddrName;		// server DNS name or value as specified in backupInfo
	std::string m_srcAddrNumeric;	// resolved numeric addr
	uniString::utf8 m_srcURLpart;		// server

	uniString::utf8 m_originalMimeType;	// original mimetype for checking

	uniString::utf8 m_srcLogString;
	httpHeaderMap_t m_HTTPHeaders;
	uniString::utf8 m_HTTPGreetingResponse; // first line of response

	const uniString::utf8::value_type *m_outBuffer; // for outgoing text lines
	int m_outBufferSize;
	const int m_originalBitrate;	// original bitrate for checking

	uniString::utf8 m_lineBuffer;		// in/out lines

	int		m_retryCount;
	bool	m_backupWaitingToReconnect;
	bool	m_backupSentConnectWait;
	bool	m_skip;
	bool	m_tryRelaySource;

	time_t m_backupReconnectStartTime;

	typedef void (protocol_backup::*state_t)();

	state_t	m_state;
	state_t m_nextState;

	config::streamConfig		m_backupInfo;
	const config::streamConfig	m_originalbackupInfo; // used for reconnects later on

	u_short	m_srcPort;			// server port
	short	m_redirectCount;

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

protected:
	virtual void timeSlice() throw(std::exception);
	virtual uniString::utf8 name() const throw() { return "protocol_backup"; }

public:
	protocol_backup(const config::streamConfig &info, const int originalBitrate, const uniString::utf8& originalMimeType) throw();
	virtual ~protocol_backup() throw();
};

#endif
