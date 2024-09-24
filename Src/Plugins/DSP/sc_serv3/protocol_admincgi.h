#pragma once
#ifndef protocol_admincgi_H_
#define protocol_admincgi_H_

#include "threadedRunner.h"
#include "protocol_HTTPStyle.h"
#include "streamData.h"

// this class takes any necessary actions indicated by an HTTP
// call with the /admin.cgi url
class protocol_admincgi: public runnable
{
private:
	const bool	m_noSID;
	const bool	m_zeroSID;
	short		m_saveLogFile;

	const uniString::utf8			m_clientLogString;
	const protocol_HTTPStyle::HTTPRequestInfo	m_httpRequestInfo;
	const uniString::utf8			m_password;
	uniString::utf8					m_referer;
	const uniString::utf8			m_hostIP;
	const uniString::utf8			m_userAgent;
	const streamData::streamID_t	m_sid;

	virtual uniString::utf8 name() const throw() { return "protocol_admincgi"; }
	void timeSlice() throw(std::exception);
	typedef void (protocol_admincgi::*state_t)();

	state_t	m_state;
	state_t m_nextState;

	const uniString::utf8::value_type	*m_outBuffer;	// for outgoing text lines
	int									m_outBufferSize;

	bool m_tailLogFile;
	// for log file output colouring
	char lastChar;
	bool inMsg;
	bool first;

	uniString::utf8	m_outMsg;
	uniString::utf8	m_updinfoSong;	// for the updinfo so we're not re-processing after checking
	uniString::utf8	m_updinfoURL;	// for the updinfo so we're not re-processing after checking
	uniString::utf8	m_updinfoDJ;	// for the updinfo so we're not re-processing after checking
	uniString::utf8	m_updinfoNext;	// for the updinfo so we're not re-processing after checking

	// for log file transmission
	FILE*			m_logFile;
	uniString::utf8	m_logFileName;
	uniString::utf8	m_logFileHeader;
	uniString::utf8	m_logFileBodyPrefix;
	uniString::utf8	m_logFileBodyFooter;
	std::vector<uniString::utf8::value_type> m_logFileBuffer;
	z_stream m_stream;

	void state_ConfirmPassword() throw(std::exception);
	void state_Send() throw(std::exception);
	void state_Close() throw(std::exception);
	void state_UpdateMetadata() throw(std::exception);
	void state_UpdateXMLMetadata() throw(std::exception);
	void state_SendFileHeader() throw(std::exception);
	void state_SendFileFooter() throw(std::exception);
	void state_SendFileContents() throw(std::exception);
	void state_SendFileEnd() throw(std::exception);

	void sendMessageAndClose(const uniString::utf8 &msg) throw();

	void mode_none(const streamData::streamID_t sid, const int refreshRequired) throw();
	void mode_kickdst(const streamData::streamID_t sid, const uniString::utf8 &kickAddrs) throw();
	void mode_ban(const streamData::streamID_t sid, const uniString::utf8 &banAddrs, const int banMask) throw();
	void mode_unban(const streamData::streamID_t sid, const uniString::utf8 &banAddr, const int banMask) throw();
	void mode_viewban(const streamData::streamID_t sid) throw();
	void mode_viewrip(const streamData::streamID_t sid) throw();
	void mode_rip(const streamData::streamID_t sid, const uniString::utf8 &ripAddr, const uniString::utf8 &rawIpAddr) throw();
	void mode_unrip(const streamData::streamID_t sid, const uniString::utf8 &ripAddr, const uniString::utf8 &rawIpAddr) throw();
	void mode_viewagent(const streamData::streamID_t sid) throw();
	void mode_agent(const streamData::streamID_t sid, const uniString::utf8 &agent) throw();
	void mode_unagent(const streamData::streamID_t sid, const uniString::utf8 &agent) throw();
	void mode_viewxml(const streamData::streamID_t sid, int page, const bool iponly, const bool ipcount) throw();
	void mode_viewjson(const streamData::streamID_t sid, int page, const bool iponly, const bool ipcount, const uniString::utf8& callback) throw();
	void mode_viewlog(const streamData::streamID_t sid, const bool tail, const bool save, const bool server) throw();
	void mode_art(const streamData::streamID_t sid, const int mode) throw();
	void mode_register(const streamData::streamID_t sid, const streamData::streamInfo &info) throw();
	void mode_listeners(const streamData::streamID_t sid) throw();
	void mode_summary(const int refreshRequired) throw();
	void mode_bandwidth_html(const int refreshRequired) throw();
	void mode_bandwidth_xml() throw();
	void mode_bandwidth_json(const uniString::utf8& callback) throw();
	void mode_ypstatus_xml() throw();
	void mode_ypstatus_json(const uniString::utf8& callback) throw();
	void mode_sources(const uniString::utf8& host) throw();
	void mode_adgroups() throw();
	void mode_debug(const uniString::utf8& option, const int on_off, const bool adminRefer) throw();
	void mode_logs() throw();
	void mode_history(const streamData::streamID_t sid) throw();
	void mode_help() throw();
	void mode_config() throw();

	const uniString::utf8 getClientIP(const bool streamPublic, const uniString::utf8 &publicIP) throw();
	uniString::utf8 escapeText(const std::vector<uniString::utf8::value_type> &s) throw();

public:
	protocol_admincgi(const socketOps::tSOCKET s, const streamData::streamID_t sid, const bool no_sid,
					  const bool zero_sid, const uniString::utf8 &clientLogString,
					  const uniString::utf8 &password, const uniString::utf8 &referer,
					  const uniString::utf8 &hostIP, const uniString::utf8 &m_userAgent,
					  const protocol_HTTPStyle::HTTPRequestInfo &httpRequestInfo) throw(std::exception);
	~protocol_admincgi() throw();
};

extern uniString::utf8 logId;
const uniString::utf8 randomId(uniString::utf8 &temp);

#endif
