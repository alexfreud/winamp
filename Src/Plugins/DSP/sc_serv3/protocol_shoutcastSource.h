#pragma once
#ifndef protocol_shoutcastSource_H_
#define protocol_shoutcastSource_H_

#include "threadedRunner.h"
#include <map>

class streamData;

/*
	Runnable object that handles the shoutcast source (broadcaster)
	protocol
*/

class protocol_shoutcastSource: public runnable
{
private:
	const u_short			m_srcPort;
	short unsigned int		m_remainderSize;
	bool					m_denied;	// used to prevent source disconnected messages e.g. for failed passwords
	// is a padding hole around here...
	__uint8 *				m_remainder;
	const uniString::utf8	m_srcAddr;
	uniString::utf8			m_srcLogString;
	uniString::utf8			m_srcUserID;
	int						m_srcStreamID;
	int						m_outBufferSize;
	const uniString::utf8::value_type	*m_outBuffer; // for outgoing text lines
	uniString::utf8			m_lineBuffer;	// received header line
	httpHeaderMap_t			m_headers;		// the received source headers
	streamData				*m_streamData;	// associated stream object

	typedef void (protocol_shoutcastSource::*state_t)();

	state_t	m_state;
	state_t m_nextState;

	void state_ConfirmPassword() throw(std::exception);
	void state_SendBuffer() throw(std::exception);
	void state_GetLine() throw(std::exception);
	void state_GetHeaders() throw(std::exception);
	void state_AnalyzeHeaders() throw(std::exception);
	void state_CloseConnection() throw(std::exception);
	void state_GetStreamData() throw(std::exception);

protected:
	virtual void timeSlice() throw(std::exception);
	virtual uniString::utf8 name() const throw() { return "protocol_shoutcastSource"; }

public:
	protocol_shoutcastSource (microConnection &mc, const uniString::utf8 &password) throw(std::exception);

	protocol_shoutcastSource(const socketOps::tSOCKET s, const uniString::utf8 &addr,
							 const u_short port, const uniString::utf8 &password) throw(std::exception);
	virtual ~protocol_shoutcastSource() throw();
};

#endif
