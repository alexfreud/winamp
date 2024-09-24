#pragma once
#ifndef protocol_HTTPStyle_H_
#define protocol_HTTPStyle_H_

#include "threadedRunner.h"
#include "streamData.h"
#include "cache.h"
#include "stats.h"
#include <map>

/*
	Runnable object that handles protocols which use the HTTP style
	negotiation. Depending on the details this object will hand off
	to others (actually HTTP get or maybe audio streaming)
*/

class protocol_HTTPStyle : public runnable
{
friend class protocol_shoutcastClient;

public:
	enum { 
		ACCEPT_PLAIN = 0,
		ACCEPT_GZIP = 1,
		ACCEPT_DEFLATE = 2
	};

	enum { 
		HTTP_UNKNOWN = -1,
		HTTP_GET = 0,
		HTTP_POST = 1,
		HTTP_HEAD = 2
	};

	struct HTTPRequestInfo
	{
		int					m_request;			// HTTP_GET, HTTP_POST, etc
		int					m_AcceptEncoding;	// the received 'Accept-Encoding' values
		uniString::utf8		m_url;				// url portion of request, unescaped
		//////////////////////////////
		httpHeaderMap_t		m_QueryParameters;	// unescaped
		httpHeaderMap_t		m_HTTPHeaders;	// the received HTTP headers
		uniString::utf8		m_PostLine;		// received POST line

		HTTPRequestInfo() : m_request(HTTP_UNKNOWN), m_AcceptEncoding(ACCEPT_PLAIN) {}
	};

	const u_short			m_clientPort;
	const uniString::utf8	m_clientHostName;
	const uniString::utf8	m_clientAddr;
	const uniString::utf8	m_clientLogString;

private:
	microServer::AllowableProtocols_t	m_protocols;
	HTTPRequestInfo			m_httpRequestInfo;
	uniString::utf8			m_userAgent;
	uniString::utf8			m_userAgentLowered;
	uniString::utf8			m_referer;
	uniString::utf8			m_hostIP;
	uniString::utf8			m_url;

	uniString::utf8			m_outMsg;
	const uniString::utf8::value_type	*m_outBuffer; // for outgoing text lines
	int						m_outBufferSize;
    int                     m_postRequestLength;

	short					m_postRequest;
	short					m_compressed;

	uniString::utf8			m_lastKey;			// received HTTP header line key (used to cope with some weird line splits)
	uniString::utf8			m_lineBuffer;		// received HTTP header line

	typedef void (protocol_HTTPStyle::*state_t)();

	state_t	m_state;
	state_t m_nextState;

	const bool getCachedResponse(cacheItem *item, AOL_namespace::mutex &lock, const int limit = 1);
	void sendCachedResponse(cacheItem *item, CacheMap_t &cache, AOL_namespace::mutex &lock,
							uniString::utf8 &header, uniString::utf8 &body,
							const streamData::streamID_t sid = 0,
							const bool jsonp = false, const bool noCompress = false);

	void sendMessageAndClose(const uniString::utf8 &msg) throw();

	void state_GetLine() throw(std::exception);
	void state_AnalyzeHTTPHeaders() throw(std::exception);
	void state_DetermineAction() throw(std::exception);
	void state_Close() throw(std::exception);
	void state_Send() throw(std::exception);

	void path_redirect_url(const streamData::streamID_t sid, const bool no_sid, const bool isStats) throw();
	void path_root_summary(const uniString::utf8 &XFF, const bool force = false) throw();
	void path_root(const streamData::streamID_t sid, const uniString::utf8 &XFF) throw();
	void path_played_html(const streamData::streamID_t sid, const uniString::utf8 &XFF) throw();
	void path_played_json(const streamData::streamID_t sid, const uniString::utf8 &callback,
						  const bool password = true, const uniString::utf8 &XFF = "") throw();
	void path_played_xml(const streamData::streamID_t sid, const bool password = true,
						 const uniString::utf8 &XFF = "") throw();
	void path_home(const streamData::streamID_t sid) throw();
	void path_track(const streamData::streamID_t sid, int mode) throw();
	void path_tracks_json(const streamData::streamID_t sid, const uniString::utf8 &callback) throw();
	void path_tracks_xml(const streamData::streamID_t sid) throw();
	void path_current_metadata_xml(const streamData::streamID_t sid) throw();
	void path_current_metadata_json(const streamData::streamID_t sid, const uniString::utf8 &callback) throw();
	void path_crossdomain() throw();
	void path_shoutcastswf() throw();
	void path_stats_xml(const streamData::streamID_t sid, const bool proceed) throw();
	void path_statistics_xml(const streamData::streamID_t sid, const bool proceed) throw();
	void path_stats_json(const streamData::streamID_t sid, const bool proceed, const uniString::utf8 &callback) throw();
	void path_statistics_json(const streamData::streamID_t sid, const bool proceed, const uniString::utf8 &callback) throw();
	void path_art(const streamData::streamID_t sid, const int mode) throw();

	const bool isViewingAllowed(const streamData::streamID_t sid, const uniString::utf8 &password, const bool no_stream,
								bool &adminOverride, const bool hide, bool &passworded) throw();
	const bool isAccessAllowed(const streamData::streamID_t sid, const uniString::utf8 &hostAddr, const bool showOutput) throw();
	const bool isAdminAccessAllowed(const uniString::utf8 &hostIP, const uniString::utf8 &hostName) throw();
	const bool findBaseStream(bool& no_sid, streamData::streamID_t& sid);
	const uniString::utf8 getClientIP(const bool streamPublic, const uniString::utf8 &publicIP) throw();
	void getPNGImage(const uniString::utf8 &png) throw();

protected:
	virtual void timeSlice() throw(std::exception);
	virtual uniString::utf8 name() const throw() { return "protocol_HTTPStyle"; }

public:
    protocol_HTTPStyle (microConnection &mc, const string &firstLine) throw(std::exception);

	protocol_HTTPStyle(const socketOps::tSOCKET s, const uniString::utf8 &hostName,
					   const uniString::utf8 &addr, const u_short port, const string &firstLine,
					   const microServer::AllowableProtocols_t protocols) throw(std::exception);
	virtual ~protocol_HTTPStyle() throw();

	static uniString::utf8 getStatsXMLBody(const streamData::streamID_t sid, const bool single,
										   const bool proceed, const socketOps::tSOCKET m_socket,
										   stats::statsData_t& data, const bool no_copy = false);
	static uniString::utf8 getStatsJSONBody(const streamData::streamID_t sid, const bool single,
											const bool proceed, const socketOps::tSOCKET m_socket,
											stats::statsData_t& data, const bool no_copy = false);
	static uniString::utf8 getPlayedBody(const streamData::streamID_t sid);

	static uniString::utf8 getCurrentXMLMetadataBody(const bool mode, const uniString::utf8 &metadata);
	static uniString::utf8 getCurrentJSONMetadataBody(const uniString::utf8 &metadata);

	static uniString::utf8 getPlayedJSON(const streamData::streamID_t sid, uniString::utf8 &header,
										 const uniString::utf8 &callback, const bool allowed) throw();
	static uniString::utf8 getPlayedXML(const streamData::streamID_t sid, uniString::utf8 &header, const bool allowed) throw();
};

const uniString::utf8 getNewVersionMessage(const uniString::utf8& ending = "<br><div align=\"center\"><hr style=\"width:99%\"></div><br>");

#endif
