#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include "protocol_FlashPolicyServer.h"
#include "webNet/urlUtils.h"
#include "services/stdServiceImpl.h"
#include "global.h"
#include "bandwidth.h"

using namespace std;
using namespace uniString;
using namespace stringUtil;

protocol_FlashPolicyServer::protocol_FlashPolicyServer(const socketOps::tSOCKET s,
													   const uniString::utf8 &clientLogString) throw(std::exception)
	: runnable(s), m_outBufferSize(0), m_clientLogString(clientLogString),
	  m_outBuffer(0), m_state(&protocol_FlashPolicyServer::state_Send)
{
	// flash x-domain file
	m_outMsg = gOptions.getCrossDomainFile(false);
	if (m_outMsg.empty()) m_outMsg = MSG_HTTP404;
	m_outBuffer = m_outMsg.c_str();
	bandWidth::updateAmount(bandWidth::FLASH_POLICY, (m_outBufferSize = (int)m_outMsg.size()));
}

protocol_FlashPolicyServer::~protocol_FlashPolicyServer() throw()
{
	socketOps::forgetTCPSocket(m_socket);
}

void protocol_FlashPolicyServer::timeSlice() throw(std::exception)
{
	(this->*m_state)();
}

// send buffer text
void protocol_FlashPolicyServer::state_Send() throw(std::exception)
{
	if (sendDataBuffer(DEFAULT_CLIENT_STREAM_ID, m_outBuffer, m_outBufferSize, m_clientLogString))
	{
		m_result.done();
	}
}
