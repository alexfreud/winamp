#pragma once
#ifndef protocol_FlashPolicyServer_H_
#define protocol_FlashPolicyServer_H_

#include "threadedRunner.h"
#include <map>

// this class takes any necessary actions indicated by an HTTP
// call with the <policy-file-request/> url (typically on :843)
class protocol_FlashPolicyServer: public runnable
{
private:
	int						m_outBufferSize;
	const uniString::utf8&	m_clientLogString;
	const uniString::utf8::value_type	*m_outBuffer; // for outgoing text lines
	uniString::utf8			m_outMsg;

	typedef void (protocol_FlashPolicyServer::*state_t)();

	state_t	m_state;

	void state_Send() throw(std::exception);

protected:
	virtual void timeSlice() throw(std::exception);
	virtual uniString::utf8 name() const throw() { return "protocol_FlashPolicyServer"; }

public:
	protocol_FlashPolicyServer(const socketOps::tSOCKET s, const uniString::utf8 &clientLogString) throw(std::exception);
	virtual ~protocol_FlashPolicyServer() throw();
};

#endif
