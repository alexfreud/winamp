#pragma once
#ifndef protocol_HTTPSource_H_
#define protocol_HTTPSource_H_

#include "threadedRunner.h"
#include <map>

class streamData;

/*
	Runnable object that handles the shoutcast source (broadcaster)
	protocol
*/

class protocol_HTTPSource : public runnable
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
	uniString::utf8			m_lineBuffer;	// received ICY header line
	httpHeaderMap_t			m_headers;		// the received source headers
	streamData				*m_streamData;	// associated stream object

	typedef void (protocol_HTTPSource::*state_t)();

	state_t	m_state;
	state_t m_nextState;

	void state_ConfirmPassword() throw(std::exception);
	void state_SendBuffer() throw(std::exception);
	void state_GetLine() throw(std::exception);
	void state_AnalyzeHeaders() throw(std::exception);
	void state_CloseConnection() throw(std::exception);
	void state_GetStreamData() throw(std::exception);

protected:
	virtual void timeSlice() throw(std::exception);
	virtual uniString::utf8 name() const throw() { return "protocol_HTTPSource"; }

public:
	protocol_HTTPSource (microConnection &mc, const string &firstLine) throw(std::exception);

	virtual ~protocol_HTTPSource() throw();
};

#endif
