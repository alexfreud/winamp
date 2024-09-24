#pragma once
#ifndef protocol_shoutcastClient_H_
#define protocol_shoutcastClient_H_

#include "threadedRunner.h"
#include "streamData.h"
#include "stats.h"
#include "bandwidth.h"
#include "protocol_HTTPStyle.h"


#pragma pack(push, 1)
class protocol_shoutcastClient: public runnable, public clientProtocol
{
public:
    friend class protocol_HTTPStyle;

	typedef void (protocol_shoutcastClient::*state_t)();
    protocol_shoutcastClient (protocol_HTTPStyle &hs, const streamData::streamID_t streamID, const uniString::utf8 &hostName, const uniString::utf8 &clientAddr, const uniString::utf8 &XFF, const streamData::source_t clientType);

	protocol_shoutcastClient(const socketOps::tSOCKET socket, const u_short port,
							 const streamData::source_t clientType, const uniString::utf8 &hostName,
							 const streamData::streamID_t streamID, const size_t unique,
							 const uniString::utf8 &userAgent, const uniString::utf8 &referer,
							 const uniString::utf8 &clientAddr, const uniString::utf8 &XFF,
							 const bool headRequest);

	virtual ~protocol_shoutcastClient() throw()
	{
		m_streamData = 0;
		socketOps::forgetTCPSocket(m_socket);
	}

	virtual void setIntro(vector<__uint8> &v, int uvoxDataType = MP3_DATA) { m_introFile = v; m_introFileOffset = 0; (void)uvoxDataType; }

	virtual void acquireIntroFile(const bool sc2 = false) throw();
	virtual int acquireBackupFile(int *dataType = 0, const bool sc2 = false) throw();

	void kickNextRound() throw() { m_kickNextRound = true; }

	void setGroup(size_t group) throw() { m_adAccess.setGroup (group); }
	const size_t getGroup() const throw() { return m_adAccess.getGroup(); }

	const size_t getUnique() const throw() { return m_unique; }
	const streamData::source_t getClientType() const throw() { return m_clientType; }
	const time_t getStartTime() const throw() { return m_startTime; }

	const uniString::utf8 getXFF() const throw() { return m_XFF; }
	const uniString::utf8 getUserAgent() const throw() { return (!m_userAgent.empty() ? m_userAgent : EMPTY_AGENT); }
	const uniString::utf8 getReferer() const throw() { return m_referer; }

	void return_403();

	const streamData::ringBufferAccess_t resetReadPtr(std::vector<__uint8>& data, const bool sc2 = false) throw();
	void resetReadPtr(const bool sc2 = false) throw();
	void resetReadPtr (streamData::ringBufferAccess_t ptr, bool sc2) throw();

	const int doTimeSlice(const bool sc2 = false) throw(std::exception);
	void resetCommon() throw();

	const bool sendText() throw(std::exception);

	void setupWorkingBuffer(const std::vector<__uint8>& data, std::vector<__uint8>& tempBuf, int& len) throw();

	virtual void processFrame (int type, const unsigned char *buf, unsigned int len);

	const int doFLVFrameSync(const int type, const bool debug, const int offset,
							 const std::vector<__uint8>& buf, const time_t cur_time,
							 const int bitrate, const unsigned int samplerate,
							 int &len, int &frames, bool &advert, int &timestamp,
							 const bool fill_remainder = false) throw();

	virtual const int doFrameSync(const int type, const bool debug, const int len,
							  const int offset, const std::vector<__uint8>& buf,
							  const time_t cur_time, const int bitrate,
							  const unsigned int samplerate, int &frames,
							  bool &advert, const bool fill_remainder = false) throw();

	void createFrameRate(const bool mp3, const int samplerate);
	const __uint64 calculateFrameLimit(const time_t cur_time = 0);
	void updateFrameCount(const int frames, const time_t cur_time = 0);
	const bool calculateDelay(const int autoDumpTime = -1);
	virtual int doSend(const bool debug, const time_t cur_time,
					 const int autoDumpTime, int adjust = 0) throw(std::runtime_error);
	void checkListenerIsValid(const bool debug) throw(std::runtime_error);

	void setW3CState() throw();
	const uniString::utf8 getContainer() const;
	void authForStream(streamData *_sd = 0);

	const bool handleNoData(const uniString::utf8 &logString, const int remainder,
							const streamData::ringBufferAccess_t amt,
							const int autoDumpTime, const time_t cur_time,
							const bool sc2 = false);

	void streamMovedOrRejected(const uniString::utf8 &logString, const bandWidth::usageType_t type,
							   const uniString::utf8 &serverUrl, const int mode) throw();

	const bool processAdvertTrigger(const bool advert);

	const uniString::utf8 fixICYMetadata(uniString::utf8 metadata);
	void logW3C() throw();
	void doLogW3C(const uniString::utf8 &title = "") throw();
	void processTitleW3C() throw();

	const adGroupAccess &getAdAccess () const { return m_adAccess; }
	void releaseAdvert();

	const int processAdd(const uniString::utf8 &logString, const bandWidth::usageType_t type,
						 const uniString::utf8 &msg, const int msgLen,
						 const uniString::utf8 &movedUrl, const uniString::utf8 &serverUrl) throw();
	const bool processReject(const uniString::utf8 &logString, const bandWidth::usageType_t type,
							 const uniString::utf8 &msg, const int msgLen, int *read_bitrate,
							 int *dataType = 0, const bool sc2 = false) throw();

	const int addClient();
	void reportNewListener(const uniString::utf8 &logString = "");
	void reportStopListener();

	void cleanup(const uniString::utf8 &logString, const bool debug,
				 const bool sc2 = false, const bool altLog = false) throw(exception);

    virtual int detectAutoDumpTimeout (time_t &cur_time, const size_t streamID, const uniString::utf8 &msg) throw(runtime_error);

	virtual void state_AttachToStream() throw(exception) = 0;
	virtual void state_Close() throw(std::exception) = 0;
	virtual void state_InitiateStream() throw(std::exception) = 0;
	virtual void state_SendText() throw(std::exception);
	virtual void state_SendAdverts() throw(std::exception);
	virtual void state_Stream() throw(std::exception);
	virtual void state_SendBackupFile() throw(std::exception);
	virtual void state_SendIntroFile() throw(std::exception);
	virtual void state_SendIntro() throw(std::exception);

	virtual void setCallback (state_t callback = NULL, state_t next = NULL) = 0;

	friend class auth::authService;
	friend void metrics::metrics_listener_new(const protocol_shoutcastClient &client);
	friend void metrics::metrics_listener_drop(const protocol_shoutcastClient &client);
	friend void stats::catchPreAddClients(const streamData::streamID_t id);

protected:
	uniString::utf8	m_clientHostName;
	const streamData::streamID_t	m_streamID;		// stream ID for this connection

	const size_t			m_unique;		// clients' unique id
	uniString::utf8	m_userAgent;	// client's user agent string
	uniString::utf8	m_referer;		// client's referer string
	uniString::utf8	        m_queryParams;		// client's query parameters

	streamData::source_t	m_clientType;	// stream client type

	bool					m_removeClientFromStats; // do we have to do a stats::removeClient
	bool					m_kickNextRound;
	bool					m_headRequest;
	bool					m_ignoreDisconnect;	// used to prevent the 'client connection closed (0 seconds) [Bytes: 0] Agent:'
												// message if redirecting or if max limit was reached which keeps logs cleaner

	time_t					m_startTime;	// when connection started, used to implement listenerTime
	time_t					m_newListener;  // when the new listener metric was sent
	time_t					m_timerStart; // for time regulated sending
	__uint64				m_timerFrames;  // amount sent during timer;

	streamData				*m_streamData;

	uniString::utf8	m_clientLogString;
	uniString::utf8	m_clientAddr;	// client's address
	uniString::utf8	m_XFF;
	uniString::utf8			m_OKResponse;

	uniString::utf8			m_lastSentMetadata; // used for w3C tracking

	std::vector<__uint8>	m_introFile;
	int						m_introFileOffset;
	int						m_backupFileOffset;
	std::vector<__uint8>	m_backupFile;
	int						m_backupLoopTries;
    unsigned int            m_lagOffset;

    adGroupAccess	m_adAccess;

	//// stats for w3c logs
	time_t		m_lastTitleTime;	// time of last title update used for w3c logs
	__uint64	m_bytesSentForCurrentTitle; // bytes sent since last title change. used for w3c logs
	__uint64	m_totalBytesSent;	// total bytes sent whilst the connection is open
	///////////////////////////

	streamData::ringBufferAccess_t	m_readPtr;		// pointer into ring buffer (main playback)

	const uniString::utf8::value_type	*m_outBuffer; // for outgoing text lines
	int									m_outBufferSize;

	unsigned short	m_metaInterval;
	unsigned short			m_metaIntervalCounter; // counter for metadata updates

	double		m_fps;
	__uint64	m_frameLimit;		// how many frames we're allowed
	__uint64	m_frameCount;		// how many frames we've provided

	std::vector<__uint8> m_shortSend;	// used to keep a copy of the data from short sends
										// which will be re-inserted into the output buffer
										// before processing more audio data to the output.
	std::vector<__uint8> m_remainder;
	std::vector<__uint8> m_output;

};
#pragma pack(pop)

#endif
