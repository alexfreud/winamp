#pragma once
#ifndef yp2_H_
#define yp2_H_

#include "webClient.h"

/*
	The operational model for yp2 is somewhat different from that of yp1.
	In yp1, there is no reason for the core of sc_serv to wait for result. The
	only bit of information that is truly required for correct operation is the serverID
	returned by addserv in yp1. This, however, is handled internally by the code dealing with
	yp1 since any subsequent yp requests cannot be cleared until the addsrv succeeds and the
	serverID is obtained.
	
	In yp2, the actual streaming to the client cannot proceed until the addition of the server
	is complete, since some information like stream title and genre must be obtained from the
	database. Therefore, we need to build some sort of waiting mechanism, but one that obviously
	does not block processing since the responses from yp2 may not come as fast as required for
	code that blocks.

*/

#define YP2_LOGNAME "[YP] "
#define YP_MAINTENANCE_CODE 503
#define YP_AUTH_ISSUE_CODE 123
#define YP_NOT_VISIBLE 480
#define YP_COMMS_FAILURE -1

class yp2: public webClient
{
public:
	// stationInfo data gets returned from YP in response to "add" request
	struct stationInfo
	{
		int				m_updateFrequency;
		int				m_advertMode;
        int             m_allowSSL;
        int             m_allowAllFormats;
        int             m_allowMaxBitrate;
        int             m_allowBackupURL;
        int             m_responseCode;
        uniString::utf8 m_stationID;
        uniString::utf8 m_serverID;
		uniString::utf8	m_streamTitle;
		uniString::utf8 m_streamGenre[5];
		uniString::utf8 m_streamLogo;
		uniString::utf8	m_radionomyID;
		uniString::utf8	m_advertType;
		uniString::utf8	m_advertTrigger;
		uniString::utf8	m_metrics_audience_url;
		uniString::utf8	m_metrics_adverts_url;
		uniString::utf8	m_metrics_reset_url;
		uniString::utf8	m_metrics_auth_url;
		uniString::utf8	m_tunein_air_api_url;
		uniString::utf8	m_targetspot_url;
		uniString::utf8	m_broadcasterURL;
		uniString::utf8	m_backupServer;
		std::vector<uniString::utf8> m_backupServersList;
		uniString::utf8	m_publicIP;

		uniString::utf8 logString() const throw()
		{
			return "title=" + m_streamTitle + " genre=" + m_streamGenre[0] +
				   " genre2=" + m_streamGenre[1] + " genre3=" + m_streamGenre[2] +
				   " genre4=" + m_streamGenre[3] + " genre5=" + m_streamGenre[4] +
				   " logo=" + m_streamLogo;
		}
		stationInfo() throw();
	};

	typedef unsigned yp2SessionKey;
	static const yp2SessionKey INVALID_SESSION_KEY=0;

	static uniString::utf8 logString(const yp2SessionKey &key) throw();

	enum addState_t
	{ 
		ADD_NONE = 0,
		ADD_PENDING = 1,
		ADD_SUCCEEDED = 2,
		ADD_FAILED = 3,
		ADD_REMOVEISSUED = 4, // we issued a remove
		UPDATE_FAILED = 5
	};

	struct ypInfo
	{
		yp2SessionKey key;

		bool streamPublic;
		bool streamArtwork;
		bool playingArtwork;
		bool vbr;

		size_t sid;
		uniString::utf8 authhash;

		int bitrate;
		int samplerate;
		uniString::utf8 mimeType;
		uniString::utf8 relayURL;

		size_t peakClientConnections;
		size_t maxClientConnections;

		size_t numListeners;									// update only
		size_t numUniqueListeners;								// update only
		time_t avgUserListenTime;								// update only
		time_t streamStartTime;
		size_t numberOfClientsConnectedMoreThanFiveMinutes;		// update only
		size_t numberOfClientConnectsSinceLastUpdate;			// update only

		uniString::utf8 songMetadataForYP2;
		uniString::utf8 sourceIdent;
		uniString::utf8 sourceUser;

		ypInfo(const yp2SessionKey _key, const size_t _sid, const uniString::utf8& _authhash,
			   const bool _vbr = false, const bool _streamPublic = true,
			   const bool _streamArtwork = false, const bool _playingArtwork = false) :
				key(_key), streamPublic(_streamPublic), streamArtwork(_streamArtwork),
				playingArtwork(_playingArtwork), vbr(_vbr), sid(_sid),
				authhash(_authhash), bitrate(0), samplerate(0),
				peakClientConnections(0), maxClientConnections(0),
				numListeners(0), numUniqueListeners(0), avgUserListenTime(0),
				numberOfClientsConnectedMoreThanFiveMinutes(0),
				numberOfClientConnectsSinceLastUpdate(0) { streamStartTime = 0; }
	};

	typedef enum
	{ 
		CREATE_AUTH = 3,
		CHECK_AUTH = 4,
		UPDATE_AUTH = 5,
		//REMOVE_AUTH = 6,
		ALLOW_AUTH = 7,
		VER_CHECK = 8
	} authMethodState_t;

	// function result from call to update	
	struct updateResult
	{
		int	m_maxYPInterval;	// touch interval if no title change
		int	m_requestQueued;	// if false, then "add" has not finished so this request was ignored

		updateResult() throw() : m_maxYPInterval(0), m_requestQueued(0) {}
	};

	// structures for handling the authhash management requests
	struct authhashResult
	{
		int				m_action;	// action - create, check, update, remove
		int				m_restart;	// should we restart the stream
		size_t			m_status;	// status code from response
		uniString::utf8	m_response;	// response body back from the YP request

		authhashResult() throw() : m_action(0), m_restart(0), m_status(0), m_response("") {}
	};

	typedef std::map<size_t, authhashResult> authhashMap_t;
	authhashMap_t m_authhashMap;

private:
	struct yp2_server_state
	{
		yp2::stationInfo	m_stationInfo;

		yp2SessionKey		m_sessionKey;

		addState_t			m_addState;

		uniString::utf8		m_serverID;	// as assigne by YP
		uniString::utf8		m_stationID; // as assigne by YP

		uniString::utf8		m_authHash; // needed for reposting removes

		size_t				m_streamID;

		time_t				m_lastOperationTime; // for error delays

		webClient::request	m_lastRequest;	// keep a copy of the last request incase it needs to be resent

		size_t				m_addRetries;

		yp2_server_state() : m_sessionKey(INVALID_SESSION_KEY), m_addState(ADD_NONE), m_streamID(1), m_lastOperationTime(0), m_addRetries(0) {}
		explicit yp2_server_state(const yp2SessionKey &key) : m_sessionKey(key), m_addState(ADD_NONE), m_streamID(1), m_lastOperationTime(0), m_addRetries(0) {}

		uniString::utf8 logString() const throw()
		{
			return yp2::logString(m_sessionKey) + " state=" + stringUtil::tos((int)m_addState) + " serverId=" + m_serverID + " " + m_stationInfo.logString();
		}
	};

	yp2SessionKey m_sessionKeyCounter;
	int m_errorCode; // keeps a copy of the last YP response code (good or bad)

	typedef std::map<yp2SessionKey,yp2_server_state> serverMap_t;
	serverMap_t	m_serverMap;
	uniString::utf8 m_errorMsg;
	uniString::utf8 m_errorMsgExtra;

	AOL_namespace::mutex	m_serverMapLock;

	bool m_addFailed;
	bool m_ignoreAdd;

	virtual void gotResponse(const request &q, const response &r) throw(std::exception);
	virtual	void gotFailure(const request &q) throw(std::exception);

	void response_add(const request &q, const response &r) throw(std::exception);
	void response_remove(const request &q, const response &r) throw(std::exception);
	void response_update(const request &q, const response &r) throw(std::exception);

	void getAddUpdateBody(webClient::request& r, const ypInfo& info, const bool update) throw();

	yp2SessionKey pvt_add(ypInfo& info) throw(std::exception);

	void pvt_queue_remove(const ypInfo& info) throw();

	void pvt_remove(ypInfo& info) throw(std::exception);

	updateResult pvt_update(const ypInfo& info) throw(std::exception);

	addState_t pvt_addStatus(yp2SessionKey key, int &addFailIgnore, int &errorCode) throw();

	virtual uniString::utf8 name() const throw() { return "yp2"; }

	void pvt_runAuthHashAction(const uniString::utf8 &tempId, const int action,
							   const uniString::utf8 &urlPath,
							   const httpHeaderMap_t &queryParameters,
							   const uniString::utf8 &content) throw();

	bool pvt_authHashActionStatus(const uniString::utf8 &tempId, authhashResult &info, const bool remove) throw();

	int pvt_getSrvID(yp2SessionKey key) throw();
	int pvt_getStnID(yp2SessionKey key) throw();

	static void updateYPBandWidthSent(webClient::request r) throw();

public:
	yp2() throw();
	~yp2() throw();

	// return new key if yp2_server_state_key == YP2_UNDEFINED_SERVER_STATE_KEY
	static yp2SessionKey add(ypInfo& info, const int creating) throw();

	// return true if add has succeeded
	static addState_t addStatus(yp2SessionKey key, int &addFailIgnore, int &errorCode) throw();

	static void remove(ypInfo& info) throw();

	static updateResult update(ypInfo& info);

	// used in main during shutdown to wait for request queue to clear out
	static size_t requestsInQueue() throw();

	// used for managing authhashes within the dnas
	static void runAuthHashAction(const uniString::utf8 &tempId, const int action,
								  const uniString::utf8 &urlPath,
								  const httpHeaderMap_t &queryParameters,
								  const uniString::utf8 &content = "") throw();

	static bool authHashActionStatus(const uniString::utf8& tempId, authhashResult &info, bool remove) throw();

	static bool isValidAuthhash(uniString::utf8 authhash);

	static int getSrvID(yp2SessionKey key) throw();
	static int getStnID(yp2SessionKey key) throw();
};
	
#endif
