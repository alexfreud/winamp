#pragma once
#ifndef streamData_H_
#define streamData_H_

#include <map>
#include <stdexcept>
#include "threadedRunner.h"
#include "unicode/uniString.h"
#include "threading/thread.h"
#include "yp2.h"
#include <deque>
#include "uvox2Common.h"
#include "auth.h"
#include "MP3Header.h"
#include "ADTSHeader.h"

/* 
	This class encapsulates all the data for a stream. It also includes
	static methods to manage the collection of streams
*/
using namespace std;

#define BUF_SIZE 16384
#define SEND_SIZE 8192

class adGroupAccess;

class streamData
{
public:
	friend class protocol_shoutcastClient;
	friend class protocol_shoutcast1Client;
	friend class protocol_shoutcast2Client;
	friend class protocol_HTTPClient;
	friend class protocol_flvClient;
	friend class protocol_m4aClient;

#if defined(_DEBUG) || defined(DEBUG)
	friend class protocol_relay_uvox;
	friend class protocol_relay_shoutcast;
#endif

#define ADVERT_MAP_FIXED        0
#define ADVERT_MAP_FLEX         1
#define ADVERT_MAP_PAUSE        2
#define ADVERT_MAP_MAGIC        3

	class specialFileData;
	class adEntry;
	class adGroup;

	typedef enum
	{
		UNKNOWN = 0x0,
		SHOUTCAST1 = 0x1,
		SHOUTCAST2 = 0x2,
		HTML5 = 0x4,
		RELAY = 0x8,
		WINAMP = 0x10,
		ICECAST = 0x20,
		VLC = 0x40,
		FB2K = 0x80,
		WMP = 0x100,
		ITUNES = 0x200,
		IE = 0x400,
		CHROME = 0x800,
		SAFARI = 0x1000,
		FIREFOX = 0x2000,
		SC_CDN_MASTER = 0x4000,
		SC_CDN_SLAVE = 0x8000,
		ROKU = 0x10000,
		APPLE = 0x20000,
		MPLAYER = 0x40000,
		WARNING = 0x80000,
		PS = 0x100000,
		RADIO_TOOLBOX = 0x200000,
		SC_IRADIO = 0x400000,
		FLV = 0x800000,
		M4A = 0x1000000,
		HTTP = 0x2000000,
		RADIONOMY = 0x4000000,
		CURL_TOOL = 0x8000000,
		SYNOLOGY = 0x10000000,
		WIIMC = 0x20000000
	} source_t;

	typedef size_t streamID_t;
	typedef std::set<streamData::streamID_t> streamIDs_t;
	typedef uintptr_t ringBufferAccess_t;	// we use power of 2 and mask trick to access ring buffer
											// because of this, all related variables must have same type
	struct ringBuffer_t
	{
		std::vector<__uint8>	m_data;
		ringBufferAccess_t m_writePtr;
		ringBufferAccess_t m_ptrMask;		// must be same type as m_sc1_write_ptr

		ringBuffer_t() : m_writePtr(0), m_ptrMask(0) {}
	};

	struct songHistoryInfo
	{
		uniString::utf8	m_title;	// general title (or is empty)
		uniString::utf8	m_metadata;	// full metadata (if provided)
		time_t			m_when;		// when it was played

		explicit songHistoryInfo(const uniString::utf8 &title, const uniString::utf8 &metadata) throw() :
								 m_title(title), m_metadata(metadata), m_when(::time(NULL)) {}
	};
	typedef std::deque<songHistoryInfo> streamHistory_t;

	struct extraInfo
	{
		int ypConnected;
		int ypErrorCode;
		uniString::utf8 ypErrorMsg;
		uniString::utf8 ypErrorMsgExtra;
		bool isConnected;
		bool isRelay;
		bool isBackup;

		extraInfo() : ypConnected(0), ypErrorCode(200), isConnected(false), isRelay(false), isBackup(false) {}
	};

	struct streamInfo
	{
		uniString::utf8 m_streamUser;
		uniString::utf8	m_streamName;
		uniString::utf8 m_streamGenre[5];
		uniString::utf8 m_streamLogo;
		uniString::utf8 m_streamURL;
		uniString::utf8 m_radionomyID;
		uniString::utf8 m_contentType;
		int				m_streamBitrate; // in kilobits
		bool			m_streamPublic;
		bool			m_vbr;			// if it's vbr content and we need to bitrate match
		bool			m_backup;		// signals if this a backup source being used
		bool			m_allowPublicRelay;
		int				m_streamSampleRate; // in Hz
		int				m_avgBitrate;	// in bits
		int				m_maxBitrate;	// in bits
		int				m_minBitrate;	// in bits
		source_t		m_sourceType;
		//size_t			m_streamPeakUser;
		int				m_streamMaxUser;
		int				m_streamMaxBitrate;
		int				m_streamMinBitrate;
		int				m_uvoxDataType;	// sc2 attributes
		uniString::utf8	m_relayURL;		// set if we are a relay (used in reporting)
		uniString::utf8	m_backupURL;	// set if we have a backup url for the source
		uniString::utf8	m_srcAddr;		// where is the source coming from
		uniString::utf8	m_backupServer;

		// for yp2
		uniString::utf8	m_authHash;
		uniString::utf8	m_publicIP;			// the public IP address of the DNAS if returned by the YP
        uniString::utf8 m_stationID;
        uniString::utf8 m_serverID;
		uniString::utf8 m_authUrl;			// overriden url
		uniString::utf8 m_advertsUrl;		// overriden url
		uniString::utf8 m_audienceUrl;		// overriden url
		uniString::utf8	m_tuneinAirApiUrl;	// overriden url
		uniString::utf8 m_targetSpotUrl;	// overriden url
		std::vector<uniString::utf8> m_backupServersList;

		uniString::utf8 m_currentURL;	// used by v1 sources to set StreamUrl='%s'; in v1 metadata
		uniString::utf8 m_currentSong;
		uniString::utf8 m_comingSoon;
		uniString::utf8 m_sourceIdent;
		std::vector<uniString::utf8> m_nextSongs;

        static int      m_allowSSL_global;
        static int      m_allowAllFormats_global;
        static int      m_allowMaxBitrate_global;
        static int      m_allowBackupURL_global;

        int             m_allowSSL;
        int             m_allowAllFormats;
        int             m_allowMaxBitrate;
        int             m_allowBackupURL;

        int             m_ypResponseCode;
		int				m_advertMode;
		u_short			m_srcPort;

		streamInfo() : m_streamBitrate(0), m_streamPublic(false), m_vbr(false), m_backup(false),
					   m_allowPublicRelay(true), m_streamSampleRate(0), m_avgBitrate(0),
					   m_maxBitrate(0), m_minBitrate(0), m_sourceType(streamData::SHOUTCAST1),
					   m_streamMaxUser(0), m_streamMaxBitrate(0),
					   m_streamMinBitrate(0), m_uvoxDataType(MP3_DATA), m_authUrl(DNAS_AUTH_URL),
					   m_advertsUrl(METRICS_ADVERTS_URL), m_audienceUrl(METRICS_AUDIENCE_URL),
					   m_targetSpotUrl(TARGETSPOT_URL),
                       #ifdef LICENCE_FREE
                       m_allowSSL(1), m_allowAllFormats(1), m_allowMaxBitrate(0), m_allowBackupURL(1),
                       #else
					   m_allowSSL(1), m_allowAllFormats(1), m_allowMaxBitrate(0), m_allowBackupURL(1),
                       #endif
                       m_advertMode(0), m_srcPort(0) {}
	};

	struct uvoxConfigData_t //(uvox2 and uvox21)
	{
		uniString::utf8	m_mimeType;
		int				m_avgBitrate; // in bits
		int				m_maxBitrate; // in bits
		// buffer size stuff is not actually used in streamData, but is here for completeness.
		// note that protocol_relay_uvox does not set these (and pretty much can't anyhow)
		int				m_desiredBufferSize;	// in kilobytes, as requested by broadcaster
		int				m_minimumBufferSize;	// in kilobytes, as requested by broadcaster
		/////////////
		int				m_icyPub;
		uniString::utf8	m_icyName;
		uniString::utf8	m_icyGenre;
		uniString::utf8 m_icyURL;

		uniString::utf8 toLogString() const throw()
		{
			return "mimeType=" + m_mimeType + stringUtil::eol() +
				   "avgBitrate=" + stringUtil::tos(m_avgBitrate) + stringUtil::eol() +
				   "maxBitrate=" + stringUtil::tos(m_maxBitrate) + stringUtil::eol() +
				   "desiredBufferSize=" + stringUtil::tos(m_desiredBufferSize) + stringUtil::eol() +
				   "minimumBufferSize=" + stringUtil::tos(m_minimumBufferSize) + stringUtil::eol() +
				   "icyName=" + m_icyName + stringUtil::eol() +
				   "icyGenre=" + m_icyGenre + stringUtil::eol() +
				   "icyURL=" + m_icyURL + stringUtil::eol() +
				   "icyPub=" + stringUtil::tos(m_icyPub);
		}

		uvoxConfigData_t() : m_avgBitrate(0), m_maxBitrate(0), m_desiredBufferSize(0), m_minimumBufferSize(0), m_icyPub(0) {}
	};

	struct streamSetup
	{
		const uniString::utf8& m_logString;
		const uniString::utf8& m_srcAddr;
		const uniString::utf8& m_authHash;
		const uniString::utf8& m_streamUser;
		const uniString::utf8& m_relayURL;
		const uniString::utf8& m_backupURL;

		const source_t m_sourceType;
		const streamID_t m_sid;

		const int m_srcPort;
		const int m_maxStreamUser;
		const int m_maxStreamBitrate;
		const int m_minStreamBitrate;
		const bool m_allowPublicRelay;
		const bool m_backup;
		const unsigned int m_sampleRate;
		const bool m_vbr;

		httpHeaderMap_t m_headers;
		uvoxConfigData_t m_config;

		streamSetup(const uniString::utf8 &logString, const uniString::utf8 &srcAddr,
					const uniString::utf8 &authHash, const uniString::utf8 &streamUser,
					const uniString::utf8 &relayURL, const uniString::utf8 &backupURL,
					const source_t sourceType, const streamID_t sid, const int srcPort,
					const int maxStreamUser, const int maxStreamBitrate,
					const int minStreamBitrate, const bool allowPublicRelay,
					const bool backup, const unsigned int sampleRate,
					const bool vbr, httpHeaderMap_t headers) throw()
			: m_logString(logString), m_srcAddr(srcAddr), m_authHash(authHash),
			  m_streamUser(streamUser), m_relayURL(relayURL), m_backupURL(backupURL),
			  m_sourceType(sourceType), m_sid(sid), m_srcPort(srcPort),
			  m_maxStreamUser(maxStreamUser), m_maxStreamBitrate(maxStreamBitrate),
			  m_minStreamBitrate(minStreamBitrate), m_allowPublicRelay(allowPublicRelay),
			  m_backup(backup), m_sampleRate(sampleRate), m_vbr(vbr), m_headers(headers) {}

		streamSetup(const uniString::utf8 &logString, const uniString::utf8 &srcAddr,
					const uniString::utf8 &authHash, const uniString::utf8 &streamUser,
					const uniString::utf8 &relayURL, const uniString::utf8 &backupURL,
					const source_t sourceType, const streamID_t sid, const int srcPort,
					const int maxStreamUser, const int maxStreamBitrate,
					const int minStreamBitrate, const bool allowPublicRelay,
					const bool backup, const unsigned int sampleRate,
					const bool vbr, uvoxConfigData_t& config) throw()
			: m_logString(logString), m_srcAddr(srcAddr), m_authHash(authHash),
			  m_streamUser(streamUser), m_relayURL(relayURL), m_backupURL(backupURL),
			  m_sourceType(sourceType), m_sid(sid), m_srcPort(srcPort),
			  m_maxStreamUser(maxStreamUser), m_maxStreamBitrate(maxStreamBitrate),
			  m_minStreamBitrate(minStreamBitrate), m_allowPublicRelay(allowPublicRelay),
			  m_backup(backup), m_sampleRate(sampleRate), m_vbr(vbr), m_config(config) {}
	};

    typedef pair<string, streamData::specialFileData*> adGroupEntry;  // is the strings needed here?

    class adGroups;
	class specialFileData
	{
        friend class adEntry;
        friend class adGroups;

	private:
		unsigned int m_refcount;
		mutable AOL_namespace::mutex m_lock;
		std::vector<__uint8> m_sc1Buffer;
		std::vector<__uint8> m_sc2Buffer;
		void _replaceData(const std::vector<__uint8> &data, const int uvoxDataType,
						  const int bitrate, const unsigned int samplerate) throw();

	public:
        const std::string       m_description;
        const std::string       m_url;          // these could be offloaded into separate object, only apply initially
        int                     m_samplerate;
        int                     m_bitrate;
        float                   m_duration;
        bool                    m_failed;
        bool                    m_missing;
        unsigned                m_retried;
        time_t                  m_lastUsed;

        explicit specialFileData(const std::string &description, const std::string &url = "") throw() : m_description(description), m_url(url)
        {
            m_failed = false;
            m_missing = (url == "" ? false : true);
            m_refcount = 1;
            m_retried = 0;
            m_samplerate = 0;
            m_duration = 0;
            m_lastUsed = (time_t)0;
        }
        ~specialFileData() throw() { m_lock.lock(); m_sc1Buffer.clear(); m_sc2Buffer.clear(); m_lock.unlock(); /*if (m_refcount > 1) abort();*/ }

		int loadFromFile(const uniFile::filenameType &name, const int bitrate, const int uvoxDataType,
						 const unsigned int samplerate, const uniString::utf8& logString) throw();
        int verifyData (const uniString::utf8& logString);

		void getSc1Data(std::vector<__uint8> &v) const throw() { stackLock sml(m_lock); v = m_sc1Buffer; }
		void getSc2Data(std::vector<__uint8> &v) const throw() { stackLock sml(m_lock); v = m_sc2Buffer; }

		void updateUvoxData(const int uvoxDataType, const int bitrate, const unsigned int samplerate) throw();
		void replaceData(const std::vector<__uint8> &data, const int uvoxDataType,
						 const int bitrate, const unsigned int samplerate) throw(); 
		bool gotData(void) const throw() { stackLock sml(m_lock); return (!m_sc2Buffer.empty() && !m_sc1Buffer.empty()); }
        unsigned int getRefcount () { return m_refcount; }
        void  increaseRefcount () { stackLock sml(m_lock); ++m_refcount; }

        static void release (specialFileData *f);

		friend class adGroup;
		friend class protocol_shoutcastClient;
		friend class protocol_shoutcast1Client;
		friend class protocol_shoutcast2Client;
		friend class protocol_HTTPClient;
		friend class protocol_flvClient;
		friend class protocol_m4aClient;
	};

	static int cleanFileData(uniFile::filenameType fn, vector<__uint8> &buffer, size_t siz,
							 const int bitrate, const unsigned int samplerate,
							 const int uvoxDataType, const uniString::utf8& logString,
							 const uniString::utf8& description, unsigned int &read_samplerate);

	class adTrigger;
	class adGroupQueue;

	class adEntry
	{
	public:
		size_t upper;
		specialFileData *ad;

		~adEntry() { if (ad) ad->release(ad); }

		explicit adEntry(specialFileData *d);
        adEntry (const adEntry &e);
		void flush(metrics::adSummary &summary);
	};

	class adGroup
	{
	    friend class streamData::adGroups;
	    friend class streamData::adGroupQueue;
	    friend class adGroupAccess;

	    adTrigger *m_trigger;
		size_t m_totalSizeSC1;
		size_t m_totalSizeSC2;
	public:
		std::deque <adEntry> ads;
		adGroup() { m_totalSizeSC1 = m_totalSizeSC2 = 0; m_trigger = NULL; }
        adGroup(adGroup &, adTrigger &);
		~adGroup() {;}

        void appendAdvert (specialFileData &d);
        void flushCounts (metrics::adSummary &summary);
        int recalculateTotalSize ();
	};

	class adGroupQueue
	{
	    friend class adGroups;
	    void flushStats (metrics::adSummary &summary, adTrigger *);
	public:
		unsigned int 		listeners;
        list <adGroup*>		queue;

		adGroupQueue()		{ listeners = 0; }
		~adGroupQueue();

	};

	class adTrigger
	{
        friend class adGroupQueue;
    public:
        unsigned long                   m_inUse;
        long                            m_type;             // fixed/flex etc
        streamData::ringBufferAccess_t  m_startPosSC1;
        streamData::ringBufferAccess_t  m_startPosSC2;
        streamData::ringBufferAccess_t  m_returnPtrSC1;
        streamData::ringBufferAccess_t  m_returnPtrSC2;
        time_t                          m_playedAt;
        streamID_t                      m_sid;
        string                          m_id;
        size_t                          m_maxSizeSC1;
        size_t                          m_maxSizeSC2;
        unsigned                        m_duration;

        adTrigger(const char *id, streamID_t sid);
        adTrigger(const string &id, streamID_t sid);
        ~adTrigger();
		ringBufferAccess_t getStartPos (bool sc2 = true);
		friend bool operator==(const streamData::adTrigger &lhs, const streamData::adTrigger &rhs);
	};

	typedef std::map<size_t, adGroupQueue*>	adGroupMap_t;

	class adGroups
	{
        friend class adGroupAccess;

        static AOL_namespace::mutex                 adContentLock;
        // a global map with a custom destructor
        static struct gpool : map<string, specialFileData*>
        { ~gpool(); }                               adData;                 // global pool
        static list<specialFileData*>               adContentPending;       // downloading to global pool

        static time_t                               m_nextDownloadRun;

        streamData                                  *m_sd;
        string                                      lastUUID;     // last update marker
        unsigned                                    m_recheck;      // serial count

		adGroup *findGroup (adGroupAccess &ac, ringBufferAccess_t pos);
		void flushStats (adTrigger *trigger);
	public:
        class adGroupsRetriever;
        adGroupsRetriever *retriever;
		mutable AOL_namespace::mutex m_lock;
		streamData::ringBufferAccess_t m_returnPtrSC1;
		streamData::ringBufferAccess_t m_returnPtrSC2;

		adGroupMap_t			mapping;
		std::list<adTrigger*> 		triggers;

		unsigned refcount; // prevent update to groups if in use
		int skippedAds;	// used to help track stream mis-matches
		time_t nextUpdate;
        long                            m_type;             // fixed/flex etc

		adGroups (streamData *sd);
		~adGroups();

		bool attachGroupQueue (adGroupAccess &ac);
		void detachGroupQueue (adGroupAccess &ac);

        void createNewTrigger (const string &reqID);
        void purge (streamData::ringBuffer_t &buffer);
        void purgeGroups();
        void duplicateTrigger (const string &id);
        void setType (const uniString::utf8 &s);
        size_t overlaySize (adTrigger *t, bool sc2 = false);

		adGroup *addToGroup(const string &id, const int grp, specialFileData *f);

		void loadGroups(const uniString::utf8& url, const streamData::streamID_t sid);
        adGroupQueue *findQueue (int grp);
		void checkForNewAdverts(const uniString::utf8& url, const streamData::streamID_t sid);
		uniString::utf8 getAdvertGroup();
		void queueNewSet(const streamData::streamID_t id, adGroups *newgroups);
		void flushGroupCounts(const streamData::streamID_t sid);
		void releaseTrigger (adTrigger *trigger);

        static THREAD_FUNC process(void* arg);
        static THREAD_FUNC runAdDownload(void* arg);
	};

private:
	// streams fall into two groups. Those which are active and usable, and those which are "dead".
	// dead streams are those which are going away due to a media type change or a uvox terminate stream message.
	// since the clients cannot go away instantaneously, they are considered "dying" until all references counts go
	// to zero. Streams that are not dying (including those without a source) are cross referenced in the g_streamMap
	// table
	static AOL_namespace::mutex						g_streamMapLock;
	static std::set<streamData*>					g_streams;		// all the streams, including those that are "dying"
	static std::map<streamID_t, streamData*>		g_streamMap;	// maps stream IDs to stream objects (active only)
	static std::map<streamID_t, time_t>				g_streamUptime;	// tracks per-stream uptime (active only)
	static std::map<streamData*, int>				g_streamReferenceCounts;
	static std::map<streamData*, bool>				g_streamSourceIsConnected;	// is the source connected
	static AOL_namespace::mutex						g_sourceRelayMapLock;
	static std::map<streamData::streamID_t, int>	g_streamSourceRelayIsActive;	// is the source relay active e.g. connected or trying to connect or needs to die?
	static AOL_namespace::mutex						g_streamSongHistoryMapLock;
	static std::map<streamID_t, streamHistory_t>	g_streamSongHistoryMap;
#if defined(_DEBUG) || defined(DEBUG)
	static map<streamData::streamID_t, FILE*>		g_streamSaving;
#endif
	// note: the "source is connected" concept was originally in the streamData object itself.To avoid races, that
	// flag must be manipulated under the g_streamMapLock. This was confusing, I've moved the flag to the set of tables
	// above that clearly require that lock to be in place

	// create a new stream object with the given config data and install it in the static global tables above
	static streamData* _createNewStream(const streamSetup& setup) throw(std::exception);

	static void _reduceReferenceCount(const uniString::utf8& logString, streamData* sd, const streamID_t id);
	// return null if stream does not exist
	static streamData* _increaseReferenceCount(const streamID_t ID);

	static void _moveStreamToDeadPool(const streamID_t ID) throw();
	static void _moveStreamToDeadPool(streamData *sd) throw();
	static void _moveStreamToDeadPool(std::map<streamID_t,streamData*>::iterator i) throw();

public:
    static void removeRelayStatus(streamID_t ID);

	static source_t getClientType(source_t clientType, const uniString::utf8& userAgent);
	static source_t getClientTypeCompat(source_t clientType, const uniString::utf8& userAgent);

	static bool isAllowedType(const int type);
	static bool isAllowedType(const int type, bool& mp3);

    static int convertRawToUvox (vector<__uint8> &sc2buffer, const vector<__uint8> &buf,
        const int uvoxDataType, const int bitrate, const unsigned int samplerate) throw();


	// permanently stop all sources in preparation for shutdown
	static void killAllSources() throw();
	static void killStreamSource(const streamID_t id) throw();
	static void killSource(const streamID_t id, streamData *sd = 0) throw();
	static streamIDs_t getStreamIds(const int mode = 0) throw();
	static size_t totalStreams() throw(); // returns total number of streamData objects including those that are dying
	static size_t totalActiveStreams(size_t &lastSID) throw(); // returns total number of active streamData objects
	static streamID_t enumStreams(const size_t index) throw(); // will enumerate the available streams
	static streamID_t getStreamIdFromPath(const uniString::utf8 &url, bool& htmlPage) throw();

	// Called by sources. create a stream from a type 1 (original shoutcast/http/icecast)
	// source. If stream exists and is compatible then returns existing stream
	// otherwise it creates a new one. Returns null if the stream already has a source
	// this will fetch an existing stream for a source if it exists, or create it if it does not or the source is incompatible
	static streamData* createStream(const streamSetup& setup) throw(std::exception);
	//////////////////////////////////

	// when the source goes away
	static void streamSourceLost(const uniString::utf8& logString, streamData *sd, const streamID_t id);
	// when a client goes away
	static void streamClientLost(const uniString::utf8& logString, streamData *sd, const streamID_t id);

	// called by clients to get access to a stream. Null if stream does not exist
	// isSourceActive flag is false if the source went away or if we are using yp2 and
	// the yp2::add() function hasn't returned yet.
	static streamData* accessStream(const streamID_t ID) throw();
	static streamData* accessStream(const streamID_t ID, bool &isSourceActive) throw();
	// ensure this is called after calling accessStream(..) otherwise we end up
	// with references to the objects kept which isn't good for larger streams
	void releaseStream();

	static bool isSourceConnected(const streamID_t id) throw();

	static int isRelayActive(const streamID_t id, bool &noEntry) throw();
	static void setRelayActive(const streamID_t id, int state) throw();
    static int setRelayActiveFlags (streamID_t id, bool &noEntry, int flags, int mask = 0);

	// get a streams content type, return empty string if not found or not set
	static uniString::utf8 getStreamContentType(const streamID_t ID) throw();

	// get all relevant info about the stream
	static bool getStreamInfo(const streamID_t id, streamInfo &info, extraInfo &extra) throw();

	static void getStreamSongHistory(const streamID_t id, streamHistory_t& songHistory) throw();

	static bool getStreamNextSongs(const streamID_t id, uniString::utf8& currentSong,
								   uniString::utf8& comingSoon,
								   std::vector<uniString::utf8>& nextSongs) throw();

	static time_t getStreamUptime(const streamID_t ID) throw();

	static bool validateTitle(uniString::utf8 &m_updinfoSong) throw();

	
	static uniString::utf8 getContentType(const streamData::streamInfo &info) throw();

	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// END OF STATICS //////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
private:
	/////////////////////////////////////////////////////////////////////////////////////////
	mutable AOL_namespace::mutex	m_stateLock; 
	streamInfo	m_streamInfo;

	const streamID_t	m_ID;
	time_t			m_startTime;		// when the stream started;
	//// yp reporting state
	time_t			m_nextYPPush;		// time we should send another push if not earlier
	time_t			m_maxYPInterval;	// max seconds between reports
	uniString::utf8	m_lastTouchTitle;	// last title sent via tchsrv
	yp2::yp2SessionKey	m_yp2SessionKey;
	////
	int				m_creating;	// 1 if in auto-creation state and 2 if completed or was an error
	int				m_kill;		// 1 or 2 if we need to kill ourselves i.e. when using backup sources
	short			m_dead;
	short			m_adTest;		// 1 if the advert trigger is for testing only

	bool			m_insertAdvert;
    unsigned        m_duration; // for flexbreak;
	// hole

	unsigned int	m_lastStreamSampleRate; // in Hz (used for tracking samplerates
											// to ensure we've got a 'good' match
	int				m_lastStreamBitrate;	// used to detect vbr streams vs cbr/cbr
    unsigned        m_syncFrameCount;		// used to see if we're failing to sync

	//////////////////// end variables covered by state lock ///////////////////////////////

	specialFileData	m_introFile;
	specialFileData	m_backupFile;
	specialFileData	m_adTestFile;
	specialFileData	m_adTestFile2;
	specialFileData	m_adTestFile3;
	specialFileData	m_adTestFile4;

	//////////////////////////////////////////////////////
	// shoutcast1 ring buffer
	AOL_namespace::rwLock	m_sc1StreamLock;
	AOL_namespace::rwLock	m_sc21StreamLock;

	ringBuffer_t	m_sc1_ring_buffer;
	ringBuffer_t	m_sc21_ring_buffer; // data is uvox encoded

public:
	adGroups advertGroups;

	void checkForAdverts();
	uniString::utf8 getAdvertGroup();
	static bool knownAdvertGroup(const streamData::streamID_t &sid, const int group);

private:
	// where are the start of packets
	std::deque<ringBufferAccess_t> m_sc1_packet_starts;// not really important for sc1
	std::deque<ringBufferAccess_t> m_sc21_packet_starts;
	/////////////////////////////////////////////////////////////

	// the metadata table for sc1 stores metadata changes and the point in the
	// buffer where they occur. The metadata is stored in the outgoing format
	// a.k.a StreamTitle=.....;
	//
	// we also store nextTrack and comingSoon info in this table. It's a bit of a hack, but
	// the code that updates yp2 uses this table to get the track title since it is quicker
	// to search than the potential binary gunk that makes up uvox data.
public:
	struct sc1MetadataAndExtensionInfo
	{
		uniString::utf8 m_songTitle; // this is in native shoutcast 1 stream format
		uniString::utf8 m_songMetadataForYP2; // xml to be inserted directly into yp2 request
	};

private:
	AOL_namespace::mutex m_sc1MetadataLock;
	typedef std::deque<std::pair<ringBufferAccess_t,sc1MetadataAndExtensionInfo> > sc1MetadataTable_t;
	sc1MetadataTable_t	m_sc1MetadataTable;

	// note that metadata for uvox style streams cannot be the utf8 type directly, because the utf8 type
	// does sanity checks for valid utf8 codes. The metadata could just be binary junk and we must
	// accept that. Instead, we use a vector of utf8::value_type which at least makes conversion to utf8
	// classes easier
public:
	typedef std::vector<uniString::utf8::value_type> uvoxMetadata_t;

private:
	AOL_namespace::mutex	m_sc21MetadataLock;
	typedef std::deque<std::pair<ringBufferAccess_t,uvoxMetadata_t> > sc21MetadataTable_t;
	sc21MetadataTable_t		m_sc21MetadataTable;
	uvoxMetadata_t			m_sc21MetadataToPutInline;	// metadata that must be put inline asap

	AOL_namespace::mutex	m_sc21StreamAlbumArtLock;
	AOL_namespace::mutex	m_sc21PlayingAlbumArtLock;
	typedef std::deque<std::pair<ringBufferAccess_t,uvoxMetadata_t> > sc21AlbumArtTable_t;
	sc21AlbumArtTable_t		m_sc21StreamAlbumArtTable;
	uvoxMetadata_t			m_sc21StreamAlbumArtToPutInline;	// albumart that must be put inline asap
	sc21AlbumArtTable_t		m_sc21PlayingAlbumArtTable;
	uvoxMetadata_t			m_sc21PlayingAlbumArtToPutInline;	// albumart that must be put inline asap
	std::vector<__uint8>	m_sc21AlbumArtData[2];
	size_t					m_sc21AlbumArtMime[2];
#if 0
	// limit triggers
	AOL_namespace::mutex	m_sc1LimitTriggerLock;
	std::set<pipeDrivenSignal<AOL_namespace::mutex>*> m_sc1LimitTriggers;

	AOL_namespace::mutex	m_sc21LimitTriggerLock;
	std::set<pipeDrivenSignal<AOL_namespace::mutex>*> m_sc21LimitTriggers;

	static void _scheduleLimitTrigger(pipeDrivenSignal<AOL_namespace::mutex> *t, const ringBufferAccess_t readPtr,
									  AOL_namespace::mutex &streamLock, const ringBuffer_t &ringBuffer,
									  AOL_namespace::mutex &triggerSetLock,
									  std::set<pipeDrivenSignal<AOL_namespace::mutex>*> &triggerSet) throw();
#endif
	void _setupBuffers(const uniString::utf8& logString, const bool re_init = false) throw();

	const bool updateSourceSampleRate(unsigned int& samplerate, const unsigned int read_samplerate) throw();

	void YP2_add() throw();
	void _YP2_add() throw();
	void YP2_remove() throw();
	void _YP2_remove() throw();
	void YP2_update() throw();

	// you must remove this streamData object from the g_streamMap immediately before or after calling
	// this, otherwise you'll corrupt the static structures.
	// Reason: die will cause the streamData to be removed from g_streams, but since die() causes ID() to return zero,
	// it will not get removed from g_streamMap
	void die() throw();

    parserInfo  *m_parser;

public:
	explicit streamData(const streamSetup& setup) throw(); // create shoutcast 1

	void streamUpdate(const streamID_t ID, const uniString::utf8 &authHash, const int streamMaxUser,
					  const int streamMaxBitrate, const int streamMinBitrate) throw();

	~streamData() throw();

	void sourceReconnect(const streamSetup& setup) throw(); // reconnect as shoutcast 1

    static MP3_FrameInfo *detectMP3 (unsigned int &failureThresh,const unsigned char *buf, unsigned int len, unsigned chk);
    int detectMP3 (MP3_FrameInfo *&parser, unsigned int &failureThresh,const unsigned char *buf, unsigned int len, unsigned chk);

    static AAC_FrameInfo *detectAAC (unsigned int &failureThresh,const unsigned char *buf, unsigned int len, unsigned chk);
    int detectAAC (parserInfo *&parser, unsigned int &failureThresh,const unsigned char *buf, unsigned int len, unsigned chk);

    static int getFrameInfo (parserInfo *&parser, unsigned int &failureThresh,const unsigned char *buf, unsigned int len, unsigned chk = 6);

	const bool syncToStream(short unsigned int& remainderSize, __uint8 *remainder,
							int amt, int& bitrate, const int type, const char *buf,
							const uniString::utf8& logString);

	const bool isSourceCompatible(const streamSetup& setup) const throw(); // is this configuration compatible with the existing stream

	static uniString::utf8 getHTML5Player(const size_t sid) throw();
	static uniString::utf8 getStreamMessage(const size_t sid) throw();
	static void updateStreamMessage(const size_t sid, const uniString::utf8& message) throw();

	const streamID_t ID() const { return (m_dead ? 0 : m_ID); }
	const time_t getStartTime() const { return m_startTime; }
    const streamInfo &getInfo() const { return m_streamInfo; }
	const uniString::utf8& streamName() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_streamName; }
	const uniString::utf8& streamGenre() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_streamGenre[0]; }
	const uniString::utf8& streamGenre2() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_streamGenre[1]; }
	const uniString::utf8& streamGenre3() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_streamGenre[2]; }
	const uniString::utf8& streamGenre4() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_streamGenre[3]; }
	const uniString::utf8& streamGenre5() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_streamGenre[4]; }
	const uniString::utf8& streamURL() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_streamURL; }
	const uniString::utf8& radionomyID() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_radionomyID; }
	const uniString::utf8& streamContentType() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_contentType; }
	const int streamBitrate() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_streamBitrate; }
	const bool streamPublic() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_streamPublic; }
	const bool streamIsVBR() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_vbr; }
	const bool allowPublicRelay() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_allowPublicRelay; }
	const int streamSampleRate() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_streamSampleRate; }
	const int streamAvgBitrate() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_avgBitrate; }
	const int streamActualMaxBitrate() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_maxBitrate; }
	const int streamUvoxDataType() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_uvoxDataType; }
	const uniString::utf8& streamBackupServer() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_backupServer; }
	const std::vector<uniString::utf8>& streamBackupServers() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_backupServersList; }
	const uniString::utf8& streamPublicIP() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_publicIP; }
	const uniString::utf8& streamAuthhash() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_authHash; }
	const int streamMaxUser() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_streamMaxUser; }
	const int streamMaxBitrate() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_streamMaxBitrate; }
	const int streamMinBitrate() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_streamMinBitrate; }

	const int streamAdvertMode() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_advertMode; }
	const uniString::utf8& authUrl() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_authUrl; }
	const uniString::utf8& advertsUrl() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_advertsUrl; }
	const uniString::utf8& audienceUrl() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_audienceUrl; }
	const uniString::utf8& tuneinAirApiUrl() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_tuneinAirApiUrl; }
	const uniString::utf8& targetSpotUrl() const throw() { stackLock sml(m_stateLock); return m_streamInfo.m_targetSpotUrl; }

	const std::vector<__uint8>& streamAlbumArt() const throw() { stackLock sml(m_stateLock); return m_sc21AlbumArtData[0]; }
	const std::vector<__uint8>& streamPlayingAlbumArt() const throw() { stackLock sml(m_stateLock); return m_sc21AlbumArtData[1]; }

	const size_t streamAlbumArtMime() const throw() { stackLock sml(m_stateLock); return m_sc21AlbumArtMime[0]; }
	const size_t streamPlayingAlbumArtMime() const throw() { stackLock sml(m_stateLock); return m_sc21AlbumArtMime[1]; }

	// streamSetMimeType is only used by uvox 2 since the determination must be delayed until the first data packet
	void streamSetMimeType(const uniString::utf8 &mt) throw() { stackLock sml(m_stateLock); m_streamInfo.m_contentType = mt; }

	static bool isRelayStream(const streamID_t id) throw();
	static bool isBackupStream(const streamID_t id) throw();
	const int isDead() const throw() { stackLock sml(m_stateLock); return m_dead; }
	void setKill(int kill) throw() { stackLock sml(m_stateLock); m_kill = kill; }
	const int isKill() const throw() { stackLock sml(m_stateLock); return m_kill; }

	void updateSongHistorySize() throw();
    void pushMetricsYP (bool force = true);

	void updateSourceIdent(uniString::utf8& sourceIdent, const bool relay = false) throw();

	// write data into ring buffers (sc1 and sc2)
	#if defined(_DEBUG) || defined(DEBUG)
	void writeSc1(const __uint8 *data, const int amt, const streamID_t id);
	#else
	void writeSc1(const __uint8 *data, const int amt);
	#endif
	void writeSc21(const std::vector<__uint8> &data) throw();	// data is exactly one uvox data packet or packet of uvox passthru metadata.
																// passthru metadata is treated just like regular data, but we obviously
																// can't pass it to the sc1 buffers since there is no analog for that in sc1

	const int getStreamData(streamData::ringBufferAccess_t& amt, const streamData::ringBufferAccess_t& readPtr,
							std::vector<__uint8>& data, const size_t remSize, const bool sc2 = false) throw(); /* for readers only */

	// make sure streamData is not holding a reference to this.
	void abandonLimitTrigger(pipeDrivenSignal<AOL_namespace::mutex> *t, const bool sc2 = false) throw();
	void scheduleLimitTrigger(pipeDrivenSignal<AOL_namespace::mutex> *t, const ringBufferAccess_t readPtr, const bool sc2 = false) throw();

	// add shoutcast1 metadata at current position. This is just the string
	int addSc1MetadataAtCurrentPosition(const uniString::utf8& logString, const uniString::utf8& sc1_metadata_raw,
										const uniString::utf8& sc1_url_raw, const uniString::utf8& sc1_next_raw) throw();
	int addUvoxMetadataAtCurrentPosition(__uint16 voxMsgType,const std::vector<__uint8> &data) throw();

	// get metadata that is appropriate for the indicated position
	sc1MetadataAndExtensionInfo getSc1Metadata(const ringBufferAccess_t) throw();
	uvoxMetadata_t getSc21Metadata(const ringBufferAccess_t) throw();
	uvoxMetadata_t getSc21StreamAlbumArt(const ringBufferAccess_t) throw();
	uvoxMetadata_t getSc21PlayingAlbumArt(const ringBufferAccess_t) throw();
	// 
	void clearCachedMetadata() throw(); // clear all metadata that is in caches
	void clearCachedArtwork(const size_t index) throw(); // clears specific artwork
	//
	void resetAdvertTriggers(const uniString::utf8& m_updinfoSong);

	// get good places for a client to start
	const ringBufferAccess_t getClientStartPosition(const bool sc2 = false) throw();
	ringBufferAccess_t getClientStartPosition(ringBufferAccess_t ptr, bool sc2) throw();

	specialFileData& getIntroFile() throw() { return m_introFile; }
	specialFileData& getBackupFile() throw() { return m_backupFile; }
	specialFileData& getAdTestFile(const int num) throw()
	{
		switch (num)
		{
			case 1: return m_adTestFile2;
			case 2: return m_adTestFile3;
			case 3: return m_adTestFile4;
			default: return m_adTestFile;
		}
	}

	static int YP_SrvID(const streamID_t id) throw();
	static int YP_StnID(const streamID_t id) throw();

	void updateStreamUser(const uniString::utf8& streamUser);
	void resetStreamAuthhash();

	// get the current title (if there is one) for touches/updates/tunein air api/etc
	uniString::utf8 getYPStreamTitle() throw();

	void YP2_updateInfo(const yp2::stationInfo &info) throw();
	bool YP2_addSuccessful(int &addFailIgnore, int &errorCode) throw();
};

// advert related

class adGroupAccess
{
    friend class streamData::adGroups;
    size_t              group;
    int                 idx;
    bool                m_sc2;  // use until the data buffer duplication is fixed
    bool                m_grpChanged;
	streamData::adGroup     *cached_ref;
	streamData::adGroupQueue *m_groupQueue;

    bool foundNextAdvert (const streamData::streamID_t sid, streamData::adGroups &groups, streamData::adGroup &g);
	streamData::adGroup *getGroup(streamData::streamID_t sid, streamData::adGroups &groups, const bool testing);

public:
	size_t      offset;
	size_t      total_processed;
	time_t	    start_time;
    unsigned            m_duration;

	explicit adGroupAccess(bool sc2 = false, int id = 0) { group = id; m_sc2 = sc2; idx = 0; cached_ref = NULL; offset = 0; m_grpChanged = false; m_groupQueue = NULL; }

    void setGroup (size_t id);
    size_t getGroup() const   { return group; }
	const bool inAdvertMode() const;

	void changedGroup (streamData::adGroups &groups);
	bool haveAdverts(const streamData::streamID_t sid, streamData::adGroups &groups, streamData::ringBufferAccess_t pos);
	streamData::specialFileData *getAd(const streamData::streamID_t sid, streamData::adGroups &groups, const bool testing);
	bool anotherAd(const streamData::streamID_t sid, streamData::adGroups &groups, const bool testing);
	void stopAdRun(const streamData::streamID_t sid, streamData::adGroups &groups, const bool testing);
    const streamData::adTrigger *getCurrentTrigger () const { return cached_ref ? cached_ref->m_trigger : NULL; }
};

#endif
