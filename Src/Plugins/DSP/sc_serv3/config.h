#pragma once
#ifndef config_H_
#define config_H_

#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <stdlib.h>
#include "unicode/uniFile.h"
#include "stl/stringUtils.h"
#include "threading/thread.h"
#include "metrics.h"

#define DEFAULT_SOURCE_STREAM 1
#define DEFAULT_CLIENT_STREAM_ID 1
#define DEFAULT_YP_ADDRESS "yp.shoutcast.com"

#ifdef _WIN32
#define DEFAULT_LOG		"%temp%\\sc_serv.log"
#define DEFAULT_LOGW	L"%temp%\\sc_serv.log"
#else
#define DEFAULT_LOG "/tmp/sc_serv.log"
#endif
#define DEFAULT_FLASH_POLICY_FILE "crossdomain.xml"

/*

Each option has a map associated with a member. Let's take an option called foobar

	std::map<size_t,int> m_foobar;

For single options (non-multi options) the map only has a single element. We use a map
so we can treat all options, multi or not, in the same fashion

The value of the option is assigned via the assign_<name> method. subIndex is only used
for multi options (zero based). The value is always passed as a string and is converted 
automatically internally

	inline void assign_foobar(int subIndex,const uniString::utf8 &value)

The value of the option is retrieved as a string via the fetch_<name> method. The value
is fetched as a native type via native_fetch_<name> method. subIndex is used to select a 
particular entry in a multi option. It's ignored for regular options

	uniString::utf8 fetch_foobar(int subIndex)
	int			    native_fetch_foobar(int subIndex)
		
A shorthand is provided via the _<name> method. It returns the same value as native_fetch_<name>

	const int _foobar()
	
The number of elements for the option is returned by the count_<name> method. For single options
this is always one

	size_t count_foobar()
	
The multi_<name> method returns true if the option is a multi option

	bool multi_foobar()
	
The def_<name> method returns the default value for the option as a string

	utf8 def_foobar()

All the proceeding options are private, and not protected by a mutex.
There are two public methods for accessing methods that provide mutex
protection. The value of the options is <name>() and the default value
is provided by <name>_Default()

	const int foobar()
	const int foobar_Default()

All of this is created automatically via the OPT and OPT_MULTI macros below


In the optMap table we associated all these functions with the actual name of the option
as it appears in the config file. In addition there is a change function associated with
each option that is fired when the option is changed. 
*/

/////////////////////////////////////////////////////////////////////////////////////
///////// crazy macros to provide uniform assign/fetch functions for each option

/*
	Create a single option of type "tipe" with the name "name" and a default value of "def"
*/
#define OPT(tipe,name,def)\
private:\
std::map<size_t,tipe> m_##name;\
inline void assign_##name(const uniString::utf8 &value, const size_t subIndex = DEFAULT_CLIENT_STREAM_ID) throw() { assignMulti(m_##name, subIndex, value); }\
uniString::utf8 fetch_##name(const size_t subIndex = DEFAULT_CLIENT_STREAM_ID, size_t *fetchByPos = 0) const throw() { return revert(fetchMulti(m_##name, subIndex, def, fetchByPos)); }\
tipe native_fetch_##name(const size_t subIndex = DEFAULT_CLIENT_STREAM_ID, size_t *fetchByPos = 0) const throw() { return fetchMulti(m_##name, subIndex, def, fetchByPos); }\
const tipe _##name() const throw() { return fetchMulti(m_##name, DEFAULT_CLIENT_STREAM_ID, def, 0); }\
size_t count_##name() const throw() { return 1; }\
bool multi_##name() const throw() { return false; }\
uniString::utf8 def_##name() const throw() { return revert(def); }\
public:\
const tipe name() const throw() { return fetchMulti(m_##name, DEFAULT_CLIENT_STREAM_ID, def, 0); }\
const tipe name##_Default() const throw() { return def; }

// for options that can have multiple instances (like encoders and broadcast points)
/*
	The option has the type "tipe" with the name "name" and a default value of "def"
*/
#define OPT_MULTI(tipe,name,def)\
private:\
std::map<size_t, tipe> m_##name;\
inline void assign_##name(const uniString::utf8 &value, const size_t subIndex) throw() { assignMulti(m_##name, subIndex, value); }\
inline void native_assign_##name(const size_t subIndex, const tipe &value) throw() { native_assignMulti(m_##name, subIndex, value); }\
uniString::utf8 fetch_##name(const size_t subIndex, size_t *fetchByPos = 0) const throw() { return revert(fetchMulti(m_##name, subIndex, def, fetchByPos)); }\
tipe native_fetch_##name(const size_t subIndex) const throw() { return fetchMulti(m_##name, subIndex, def, 0); }\
const tipe _##name(const std::vector<tipe>::size_type i) const throw() { return fetchMulti(m_##name, i, def, 0); }\
bool multi_##name() const throw() { return true; }\
uniString::utf8 def_##name() const throw() { return revert(def); }\
public:\
const bool read_##name(const size_t subIndex) const throw() { return (m_##name.find(subIndex) != m_##name.end()); }\
void unread_##name(const size_t subIndex) { native_assignMulti(m_##name, subIndex, name##_Default()); }\
const tipe name(const size_t subIndex) const throw() { return fetchMulti(m_##name, subIndex, def, 0); }\
size_t count_##name() const throw() { return m_##name.size(); }\
const std::map<size_t, tipe>& name##_map() const { return m_##name; } \
const tipe name##_Default() const throw() { return def; }

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// global configuration
class config
{
public:
	// stream specifications from config file.
#pragma pack(push, 1)
	struct streamConfig
	{
#pragma pack(push, 1)
		class urlObj
		{
		private:
			uniString::utf8	m_url;
			uniString::utf8	m_server;
			uniString::utf8	m_path;
			u_short			m_port;

			static uniString::utf8 parse(const uniString::utf8 &url, uniString::utf8 &server,
										 u_short &port, uniString::utf8 &path) throw(std::exception);

		public:
			explicit urlObj(const uniString::utf8 &url) throw(std::exception)
			{
				if (!url.empty())
				{
					set(url);
				}
				else
				{
					clear();
				}
			}
			urlObj& operator=(const uniString::utf8 &url) throw(std::exception)
			{
				set(url);
				return *this;
			}
			const uniString::utf8 &url() const throw() { return m_url; }
			const uniString::utf8 &server() const throw() { return m_server; }
			const uniString::utf8 &path() const throw() { return m_path; }
			const u_short port() const throw() { return m_port; }
			bool isSet() const throw() { return !m_url.empty(); }
			void set(const uniString::utf8 &url) throw(std::exception)
			{
				m_url = parse(url, m_server, m_port, m_path);
			}
			void clear() throw()
			{
				m_url.clear();
				m_port = 0;
			}
		};
#pragma pack(pop)

		uniString::utf8		m_authHash;
		uniString::utf8		m_urlPath;			// url that clients use to connect
		uniString::utf8		m_adminPassword;	// per stream admin password
		uniString::utf8		m_password;			// per stream source password
		uniString::utf8		m_publicServer;		// per stream source public flag
		size_t				m_streamID;
		int					m_maxStreamUser;	// per stream user limit
		int					m_maxStreamBitrate;	// per stream max bitrate limit
		int					m_minStreamBitrate;	// per stream min bitrate limit
		urlObj				m_relayUrl;			// if this is a relay, then this is set to the source url
		urlObj				m_backupUrl;		// if there is a backup, then this is set to the backup url
		bool				m_allowRelay;		// per stream relay allowed flag
		bool				m_allowPublicRelay;	// per stream relay public flag

		streamConfig() throw() : m_streamID(DEFAULT_CLIENT_STREAM_ID), m_maxStreamUser(0),
								 m_maxStreamBitrate(0), m_minStreamBitrate(0),
								 m_relayUrl((uniString::utf8)""), m_backupUrl((uniString::utf8)""),
								 m_allowRelay(true), m_allowPublicRelay(true) {}

		streamConfig(const size_t id, const uniString::utf8 &authHash, const uniString::utf8 &url,
					 const uniString::utf8 &relayUrl, const uniString::utf8 &backupUrl,
					 const int maxStreamUser, const int maxStreamBitrate, const int minStreamBitrate,
					 const uniString::utf8 &adminPassword, const uniString::utf8 &password,
					 const uniString::utf8 &publicServer, const bool allowRelay,
					 const bool allowPublicRelay) throw(std::exception)
				: m_authHash(authHash), m_urlPath(url), m_adminPassword(adminPassword), m_password(password),
				  m_publicServer(publicServer), m_streamID(id), m_maxStreamUser(maxStreamUser),
				  m_maxStreamBitrate(maxStreamBitrate), m_minStreamBitrate(minStreamBitrate), m_relayUrl(relayUrl),
				  m_backupUrl(backupUrl), m_allowRelay(allowRelay), m_allowPublicRelay(allowPublicRelay) {}
	};
#pragma pack(pop)

	///////////////////////////////////////////////////////////////////////
	///// functions to convert types to and from unicode strings
	template<typename T>	inline static void convert(const uniString::utf8 &v,T &r) throw() { r = v; }
	#ifdef _WIN64
	inline static void convert(const uniString::utf8 &v,size_t &r) throw() { r =  atoi((const char *)v.c_str()); }
	#endif
	inline static void convert(const uniString::utf8 &v, int &r) throw() { r = atoi((const char *)v.c_str()); }
	inline static void convert(const uniString::utf8 &v, unsigned int &r) throw() { r = atoi((const char *)v.c_str()); }
	inline static void convert(const uniString::utf8 &v, unsigned long &r) throw() { r = atol((const char *)v.c_str()); }
	inline static void convert(const uniString::utf8 &v, unsigned short &r) throw() { r = (unsigned short)atoi((const char *)v.c_str()); }
	inline static void convert(const uniString::utf8 &v, short &r) throw() { r = (short)atoi((const char *)v.c_str()); }
	inline static void convert(const uniString::utf8 &v, bool &r) throw() { r = (atoi((const char *)v.c_str()) ? true : false); }
	inline static void convert(const uniString::utf8 &v, double &r) throw() { r = atof((const char *)v.c_str()); }

	template<typename T>	inline static uniString::utf8 revert(const T &r) throw() { return r; }
	#ifdef _WIN64
	inline static uniString::utf8 revert(size_t r) throw()			{ return stringUtil::tos(r); }
	#endif
	inline static uniString::utf8 revert(int r) throw()				{ return stringUtil::tos(r); }
	inline static uniString::utf8 revert(unsigned int r) throw()	{ return stringUtil::tos(r); }
	inline static uniString::utf8 revert(unsigned long r) throw()	{ return stringUtil::tos(r); }
	inline static uniString::utf8 revert(unsigned short r) throw()	{ return stringUtil::tos(r); }
	inline static uniString::utf8 revert(short r) throw()			{ return stringUtil::tos(r); }
	inline static uniString::utf8 revert(bool r) throw()			{ return (r ? "1" : "0"); }
	inline static uniString::utf8 revert(double r) throw()			{ return stringUtil::tos(r); }
	////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////

private:
	mutable AOL_namespace::mutex m_lock; // api may write to config, so we need a lock

	////////////////////////////////////////////////////////////////////////////////////////////
	//// tables and functions so we can read and write options in a generic manner based
	///  on the names that are used in the config file
	typedef void				(config::*assignFunc_t)(const uniString::utf8 &, const size_t subIndex);
	typedef uniString::utf8		(config::*fetchFunc_t)(const size_t subIndex, size_t *fetchByPos) const;
	typedef size_t				(config::*countFunc_t)() const;
	typedef bool				(config::*multiFunc_t)() const;
	typedef uniString::utf8		(config::*defaultFunc_t)() const;

	struct accessor_t
	{
		assignFunc_t		m_assignFunc;
		fetchFunc_t			m_fetchFunc;
		countFunc_t			m_countFunc;
		multiFunc_t			m_multiFunc;
		defaultFunc_t		m_defaultFunc;

		accessor_t(assignFunc_t af, fetchFunc_t ff, countFunc_t cf, multiFunc_t mf, defaultFunc_t df) throw()
				   : m_assignFunc(af), m_fetchFunc(ff), m_countFunc(cf), m_multiFunc(mf), m_defaultFunc(df) {}

		accessor_t() throw() : m_assignFunc(0), m_fetchFunc(0), m_countFunc(0), m_multiFunc(0), m_defaultFunc(0) {}
	};
public:
	typedef std::map<uniString::utf8,accessor_t> optMap_t;
	//////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////

	// takes an option map (container) and returns the value at the index (i) if it exists,
	// otherwise it returns the default value (defaultValue). Value returned as native type
	template<typename T,typename D>
	static T fetchMulti(const std::map<size_t,T> &container, const typename std::vector<T>::size_type &subIndex,
						const D defaultValue, size_t *fetchByPos) throw()
	{
		if (!fetchByPos)
		{
			typename std::map<size_t,T>::const_iterator i = container.find(subIndex);
			if (i != container.end())
			{
				return (*i).second;
			}
		}
		else
		{
			// there's cases where we need to get the value
			// effectively by it's position in the map so
			// for the moment we'll just look through (bad
			// for speed but it's not a commonly used mode).
			typename std::vector<T>::size_type pos = 0;
			for (typename std::map<size_t,T>::const_iterator i = container.begin(); i != container.end(); ++i, pos++)
			{
				if (pos == subIndex)
				{
					*fetchByPos = (*i).first;
					return (*i).second;
				}
			}
		}
		return defaultValue;
	}
private:
	// assign map index. Expand with default value as needed. Value is specified as a string and converted as needed
	template<typename T>
	static void assignMulti(std::map<size_t,T> &container, const typename std::vector<T>::size_type subIndex, const uniString::utf8 &value) throw()
	{
		T vtmp;
		convert(value, vtmp);
		container[subIndex] = vtmp;
	}

	// same as assignMulti, but you can provide the native type instead of a string
	template<typename T>
	static void native_assignMulti(std::map<size_t,T> &container, const typename std::vector<T>::size_type subIndex, const T &value) throw()
	{
		container[subIndex] = value;
	}

	/////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////

	// radionomy metrics
	OPT(bool,adMetricsDebug,false)
	OPT(size_t,metricsMaxQueue,80000)

	OPT(bool,authDebug,false)

public:
	friend void metrics::metrics_apply(config &conf);

private:
	optMap_t m_optMap;

	// we can't log during startup because the loggers don't exist
	std::vector<uniString::utf8> m_deferredWarnLogMessages; // log warning messages from startup
	std::vector<uniString::utf8> m_deferredErrorLogMessages; // log error messages from startup

	// deferred options are those that weren't set because they can't take effect immediately
	// they are used when writing out the new config file.
	std::map<uniString::utf8,uniString::utf8> m_deferredOptions;

	OPT(size_t,configRewrite,0)
	OPT(uniFile::filenameType,confFile,"")

	OPT(uniFile::filenameType,logFile,DEFAULT_LOG)	// file for logging
	OPT(uniFile::filenameType,realLogFile,DEFAULT_LOG)	// file for logging

	OPT(bool,screenLog,true)			// log to screen
	OPT(bool,log,true)					// do I log?
	OPT(int,logRotates,5)				// hwo many backups to keep when doing a log rotate?
	OPT(bool,logArchive,false)			// backup rotated files which would otherwise be deleted
	OPT(int,rotateInterval,86400)		// interval between log file rotations (24 hours)
										// if set to 0 then we won't rotate any of the files

	OPT(int,portBase,8000)				// listen port
	OPT(int,publicPort,-1)				// listen port for firehose - workaround for firehose hosts running on port 8000
										// but need to effectively be seen as bound to port 80 - allowing all to work ok
	OPT(int,portLegacy,-1)				// legacy port override/disable
	OPT(uniString::utf8,alternatePorts,(uniString::utf8)"")	// alternate port(s) for client only connections - comma separated string
	OPT(bool,nameLookups,false)			// do internet reverse lookups

    OPT(uniString::utf8,sslCertificateFile,"");
    OPT(uniString::utf8,sslCertificateKeyFile,"");

	OPT(int,autoDumpTime,30)			// how long before an idle connection is dumped (in seconds). Zero means no timeout
	OPT(int,maxHeaderLineSize,4096)		// maximum size of an HTTP header line. Default is pretty arbitrary right now
										// but should be at least as big as a u-vox packet, since we have to anaylize
										// initial data bytes to determine protocol and type of connectee (source or client)
	OPT(int,maxHeaderLineCount,100)		// max headers lines in HTTP style exchange
	OPT(uniString::utf8,password,(uniString::utf8)""); // password for broadcaster to connect
	OPT(uniString::utf8,adminPassword,(uniString::utf8)""); // administrator password

	// buffer configuration options
	OPT(int,bufferType,1)					// 0 - fixed, 1 - adaptive
	OPT(size_t,fixedBufferSize,524288)		// size of buffer if fixed (gives ~32 seconds ~ 128kbps, 44.1kHz)
	OPT(double,adaptiveBufferSize,16)		// size of adaptive buffer in seconds
	OPT(size_t,bufferHardLimit,16777216)	// no more than this give or take a factor of two

	OPT(unsigned short,metaInterval,16384)	// metadata interval for shoutcast 1
	OPT_MULTI(unsigned short,stream_metaInterval,16384)	// per-stream override

	// special intro and backup files
	OPT(uniFile::filenameType,introFile,"")
	OPT(uniFile::filenameType,backupFile,"")
	OPT(uniFile::filenameType,backupTitle,"")
	OPT(int,backupLoop,0)
	OPT(int,maxSpecialFileSize,30000000)

	OPT(int,adTestFileLoop,1)
	OPT(uniFile::filenameType,adTestFile,"")
	OPT(uniFile::filenameType,adTestFile2,"")
	OPT(uniFile::filenameType,adTestFile3,"")
	OPT(uniFile::filenameType,adTestFile4,"")

	OPT(uniFile::filenameType,artworkFile,"")
	std::map<size_t, uniString::utf8> m_artworkBody;

	OPT(uniString::utf8,uvoxCipherKey,uniString::utf8("foobar"))

	// w3c logs
	OPT(bool,w3cEnable,true)
	OPT(uniString::utf8,w3cLog,uniString::utf8("sc_w3c.log"))

	OPT(uniString::utf8,pidFile,uniString::utf8("sc_serv_$.pid"))

	// relaying
	OPT(bool,allowRelay,true)	// can other servers relay us. Based on Shoutcast user agent, not reliable
	OPT(bool,allowPublicRelay,true)	// relays can list themselves in yp

	OPT(short,maxHTTPRedirects,5)	// max times we can redirect (http 3xx)
	OPT(int,relayReconnectTime,5)	// seconds to reconnect on relay failure
	OPT(int,relayConnectRetries,0)	// number of times we retry a relay request before throwing it away
									// which if set as zero will keep retrying (excluding bitrate blocks)

	////// stream configs
	OPT_MULTI(size_t,stream_ID,DEFAULT_CLIENT_STREAM_ID)
	OPT_MULTI(uniString::utf8,stream_authHash,(uniString::utf8)"")
	OPT_MULTI(uniString::utf8,stream_path,(uniString::utf8)"")
	OPT_MULTI(uniString::utf8,stream_relayURL,(uniString::utf8)"")
	OPT_MULTI(uniString::utf8,stream_backupURL,(uniString::utf8)"")

	OPT_MULTI(uniString::utf8,stream_password,(uniString::utf8)"")
	OPT_MULTI(uniString::utf8,stream_adminPassword,(uniString::utf8)"")
	
	OPT_MULTI(uniString::utf8,stream_publicServer,(uniString::utf8)"")	// if "always" or "never" overrides public flag from source
	OPT_MULTI(bool,stream_allowRelay,true)	// can other servers relay us. Based on Shoutcast user agent, not reliable
	OPT_MULTI(bool,stream_allowPublicRelay,true)	// relays can list themselves in yp

	OPT_MULTI(int,stream_maxUser,0)			// if set to a value greater than zero then we have per stream limits
	OPT_MULTI(int,stream_maxBitrate,0)		// if set to a value greater than zero then we have per stream limits
	OPT_MULTI(int,stream_minBitrate,0)		// if set to a value greater than zero then we have per stream limits
	OPT_MULTI(bool,stream_ripOnly,false)	// only addrs in rip file may connect
	OPT_MULTI(int,stream_autoDumpTime,30)		// how long before an idle connection is dumped (in seconds). Zero means no timeout
	OPT_MULTI(int,stream_autoDumpSourceTime,7)	// how long before an idle source connection is dumped (in seconds). Zero means no timeout
	OPT_MULTI(bool,stream_autoDumpUsers,false)	// if true, then users are dumped if source disconnects
	OPT_MULTI(size_t,stream_listenerTime,0)	// max time in minutes you can listen. 0 means no limit

	OPT_MULTI(int,stream_songHistory,10)	// max song history to preserve
	OPT_MULTI(uniString::utf8,stream_uvoxCipherKey,(uniString::utf8)"")

    OPT_MULTI(uniFile::filenameType,stream_logFile,"");     // file for per mount logging

	OPT_MULTI(int,stream_adTestFileLoop,1)
	OPT_MULTI(uniFile::filenameType,stream_adTestFile,"")
	OPT_MULTI(uniFile::filenameType,stream_adTestFile2,"")
	OPT_MULTI(uniFile::filenameType,stream_adTestFile3,"")
	OPT_MULTI(uniFile::filenameType,stream_adTestFile4,"")
	OPT_MULTI(uniFile::filenameType,stream_introFile,"")
	OPT_MULTI(uniFile::filenameType,stream_backupFile,"")
	OPT_MULTI(uniFile::filenameType,stream_backupTitle,"")
	OPT_MULTI(int,stream_backupLoop,0)

	OPT_MULTI(int,stream_rateLimitWait,0)

	OPT_MULTI(uniFile::filenameType,stream_artworkFile,"")

	OPT_MULTI(uniFile::filenameType,stream_banFile,"")
	OPT_MULTI(uniFile::filenameType,stream_ripFile,"")
	OPT_MULTI(uniFile::filenameType,stream_agentFile,"")
	OPT_MULTI(uniString::utf8,stream_w3cLog,(uniString::utf8)"")

	OPT_MULTI(uniString::utf8,stream_hideStats,(uniString::utf8)"")		// hide /stats & /statistics as well as /index and /played public facing pages
	OPT_MULTI(uniString::utf8,stream_redirectUrl,(uniString::utf8)"")	// used with hideStats=all or if the stream version isn't specified
	OPT_MULTI(uniString::utf8,stream_movedUrl,(uniString::utf8)"")		// used to redirect a deemed dead stream (just in case)

	OPT(bool,requireStreamConfigs,false)	// if true, then sources can only connect if stream configs have been defined
	OPT(uniString::utf8,userId,"")
	OPT(uniString::utf8,licenceId,"")

	// flash policy
	OPT(int,flashPolicyServerPort,-1) // listen on port 843 for flash policy server request
	OPT(uniFile::filenameType,flashPolicyFile,DEFAULT_FLASH_POLICY_FILE)
	uniString::utf8 m_crossdomainStr;		// used to hold a cached copy of the crossdomain.xml file
	uniString::utf8 m_crossdomainStrGZ;		// used to hold a cached copy of the gzipped crossdomain.xml file
	uniString::utf8 m_shoutcastSWFStr;		// used to hold a cached copy of the shoutcast.swf file
	uniString::utf8 m_shoutcastSWFStrGZ;	// used to hold a cached copy of the gzipped shoutcast.swf file
	uniString::utf8 m_usedAlternatePorts;	// used to hold a copy of the valid alternate ports in use

	////// yp
	OPT(int,ypTimeout,30)	// yp timeout interval for requests
	OPT(uniString::utf8,ypAddr,DEFAULT_YP_ADDRESS)
	OPT(int,ypPort,80)
	OPT(uniString::utf8,ypPath,"/yp2")
	OPT(int,ypMaxRetries,10)	// number of times we retry a yp request before throwing it away
	OPT(int,ypReportInterval,5 * 60)	// never touch any slower than this
	OPT(int,ypMinReportInterval,10)		// never touch any faster than this
	OPT(uniString::utf8,publicServer,"default")	// if "always" or "never" overrides public flag from source

	//// cdn behaviour
	OPT(uniString::utf8,cdn,"")		// if 'on' or 'always' then we enable all of the cdn modes (including YP pings for private streams)
									// but use it to determine opt-in (via 'on') or opt-out (via 'always')
	OPT_MULTI(int,cdn_master,-1)	// this and the option below is used to control the behaviour of things
	OPT_MULTI(int,cdn_slave,-1)

	//// stats
	OPT(int,maxUser,512)	// max clients
	OPT(int,minBitrate,0)	// min bitrate of source connections - if zero / not set then there is no limit
	OPT(int,maxBitrate,0)	// max bitrate of source connections - if zero / not set then there is no limit
	OPT(uniString::utf8,hideStats,"") // hide /stats & /statistics as well as /index and /played public facing pages
	OPT(uniString::utf8,redirectUrl,"")	// used with hideStats=all or if the stream version isn't specified

	/// client behaviour
	OPT(size_t,listenerTime,0)	// max time in minutes you can listen. 0 means no limit
	OPT(bool,autoDumpUsers,false) // if true, then users are dumped if source disconnects
	OPT(uniString::utf8,srcIP,"")	// bind addr for sources
	OPT(uniString::utf8,destIP,"")	// bind addr for clients
	OPT(uniString::utf8,publicIP,"")	// public address to use for the YP listing if the bind addr is not appropriate
	OPT(uniString::utf8,titleFormat,"") // modifies icy-name
	OPT(uniString::utf8,urlFormat,"")	// modifies icy-url

	//// banning
	OPT(uniFile::filenameType,banFile,"sc_serv.ban")
	OPT(bool,saveBanListOnExit,true) // save on exiting

	//// rip
	OPT(uniFile::filenameType,ripFile,"sc_serv.rip")
	OPT(bool,saveRipListOnExit,true) // save on exiting
	OPT(bool,ripOnly,false)	// only addrs in rip file may connect
	OPT(uniFile::filenameType,adminFile,"sc_serv.admin")

	/// agent
	OPT(uniFile::filenameType,agentFile,"sc_serv.agent")
	OPT(bool,saveAgentListOnExit,true) // save on exiting
	OPT(bool,blockEmptyUserAgent,false) // if true, block the client connection if there is no user agent specified

	//// debugging
	OPT(bool,webClientDebug,false)
	OPT(bool,yp2Debug,false)
	OPT(bool,shoutcastSourceDebug,false)
	OPT(bool,uvox2SourceDebug,false)
	OPT(bool,HTTPSourceDebug,false)
	OPT(bool,streamDataDebug,false)
	OPT(bool,microServerDebug,false)
	OPT(bool,httpStyleDebug,false)
	OPT(bool,shoutcast1ClientDebug,false)
	OPT(bool,shoutcast2ClientDebug,false)
	OPT(bool,HTTPClientDebug,false)
	OPT(bool,flvClientDebug,false)
	OPT(bool,m4aClientDebug,false)

	OPT(bool,relayDebug,false)
	OPT(bool,relayShoutcastDebug,false)
	OPT(bool,relayUvoxDebug,false)
	OPT(bool,statsDebug,false)
	OPT(bool,threadRunnerDebug,false)

	OPT(bool,logClients,true)

	OPT(int,songHistory,20)	// max song history to preserve

	/// misc nonsense
	OPT(uniString::utf8,unique,"$")	// subsitution string for file names to mimic old sc_serv conf file behaviour
	OPT(uniString::utf8,include,"")	// include file placeholder
	OPT(int,cpuCount,0)				// cpu usage. zero is default
	OPT(bool,clacks,true)			// need i say more...?
	OPT(bool,startInactive,false)	// used to not start the relays on startup
	OPT(bool,rateLimit,true);		// if we do frame rate limiting or not
	OPT(int,rateLimitWait,5);		// if we do frame rate limiting, how many seconds before we enforce it fully
	OPT(bool,useXFF,true);			// if we use XFF (if available) for the listener address (and related actions)
	OPT(bool,forceShortSends,false);// used for debugging streaming issues by introducing forced delays into sends
	OPT(bool,adminNoWrap,false);	// used for defaulting the admin listener page mode for wrapping or not
									// wrapping the listener output list which might be handy for some users

	// used for customising the css of the index.html and admin pages
	OPT(uniFile::filenameType,adminCSSFile,"v2")
	uniString::utf8 m_styleCustomStr;		// used to hold a cached copy of the custom css file
	uniString::utf8 m_styleCustomStrGZ;		// used to hold a cached copy of the gzipped custom css file
	uniString::utf8 m_styleCustomHeader;	// used to hold a cached copy of the gzipped custom css file
	uniString::utf8 m_styleCustomHeaderGZ;	// used to hold a cached copy of the gzipped custom css file
	time_t m_styleCustomHeaderTime;			// used to control the cache handling

	OPT(uniFile::filenameType,faviconFile,"")
	OPT(uniFile::filenameType,faviconFileMimeType,"image/x-icon")
	uniString::utf8 m_faviconBody;
	uniString::utf8 m_faviconHeader;
	uniString::utf8 m_faviconBodyGZ;	// gzipped version
	uniString::utf8 m_faviconHeaderGZ;	// gzipped version
	time_t m_favIconTime;				// used to control the cache handling

	// used for returning robots.txt
	OPT(uniFile::filenameType,robotstxtFile,"")
	uniString::utf8 m_robotsTxtBody;
	uniString::utf8 m_robotsTxtHeader;
	uniString::utf8 m_robotsTxtBodyGZ;		// gzipped version
	uniString::utf8 m_robotsTxtHeaderGZ;	// gzipped version

	uniString::utf8 m_certPath;
	uniString::utf8 m_certFileBody;

	const bool _load(const uniFile::filenameType &file, const uniString::utf8 &uniqueStr, const bool parent) throw();
	int promptConfigFile() throw();

	bool editConfigFileEntry(size_t sid, const uniFile::filenameType &filename,
							 const uniString::utf8 &authhash, const uniString::utf8 &param,
							 bool add, bool &handled, bool &idHandled, bool parent) throw();

	// used for legacy handling of the relayport and relayserver options to sid=1
	uniString::utf8 m_legacyRelayPort;
	uniString::utf8 m_legacyRelayServer;

	streamConfig& getPerStreamConfig(streamConfig& stream, const size_t sid, const bool useParent = true);

public:
	config() throw();
	~config() throw();

	static std::string logSectionName();

	uniString::utf8 getCrossDomainFile(const bool compressed) throw();
	uniString::utf8 getIndexCSS(const bool compressed) throw();
	uniString::utf8 getShoutcastSWF(const bool compressed) throw();

	const uniString::utf8 getStreamRedirectURL(const size_t streamID, const bool isStats, const bool homeSet,
											   const bool compress, const bool force = false) const throw();

	int getRateLimitWait(const size_t streamID) const throw();

	const int isBitrateDisallowed(const size_t streamID, const int bitrate, int &streamMinBitrate, int &streamMaxBitrate) const throw();

	unsigned short getMetaInterval(const size_t streamID) const throw();
	int getBackupLoop(const size_t streamID) const throw();
	size_t getSongHistorySize(const size_t streamID) const throw();
	const int getAutoDumpTime(const size_t streamID = DEFAULT_SOURCE_STREAM) const throw();

	const int getCPUCount() const throw();

	const std::vector<streamConfig> getRelayList();
	const std::vector<streamConfig> getBackupUrl(const size_t streamID) throw(std::exception);
	const uniString::utf8 getStreamHideStats(const size_t streamID) const;

	typedef std::map<size_t, config::streamConfig> streams_t;
	void getStreamConfigs(streams_t& streams, const bool useParent = true);
	const bool getStreamConfig(streamConfig& stream, const size_t streamID);

	// handle updating stream configs on the fly (as applicable)
	#define AUTH_HASH		0x1
	#define URL_PATH		0x2
	#define RELAY_URL		0x4
	#define MAX_USER		0x8
	#define SOURCE_PWD		0x10
	#define ADMIN_PWD		0x20
	#define PUBLIC_SRV		0x40
	#define ALLOW_RELAY		0x80
	#define ALLOW_PUBLIC_RELAY		0x100
	#define RIP_ONLY		0x200
	#define DUMP_TIME		0x400
	#define DUMP_USER		0x800
	#define LIST_TIME		0x1000
	#define SONG_HIST		0x2000
	#define CIPHER_KEY		0x4000
	#define INTRO_FILE		0x8000
	#define BACKUP_FILE		0x10000
	#define BAN_FILE		0x20000
	#define RIP_FILE		0x40000
	#define W3C_FILE		0x80000
	#define MAX_BITRATE		0x100000
	#define BACKUP_URL		0x200000
	#define HIDE_STATS		0x400000
	#define MOVED_URL		0x800000
	#define AGENT_FILE		0x1000000
	#define CDN_MASTER		0x2000000
	#define CDN_SLAVE		0x4000000
	#define ARTWORK_FILE	0x8000000
	#define BACKUP_LOOP		0x10000000
	#define BACKUP_TITLE	0x20000000
	#define MIN_BITRATE		0x40000000
	#define AD_TEST_FILE	0x80000000
	#define AD_TEST_FILE_LOOP	0x100000000ULL
	#define RATE_LIMIT_WAIT	0x200000000ULL
	#define METAINTERVAL	0x400000000ULL
	#define AD_TEST_FILE_2	0x800000000ULL
	#define AD_TEST_FILE_3	0x1000000000ULL
	#define AD_TEST_FILE_4	0x2000000000ULL

	void addStreamConfig(config &readConfig, config::streamConfig) throw(std::exception);
	__uint64 updateStreamConfig(config &readConfig, config::streamConfig update) throw(std::exception);
	void removeStreamConfig(config::streamConfig) throw(std::exception);

	// deals with configuring all of the per-stream passwords, etc
	static bool setupPasswords(const config::streams_t &) throw(std::exception);

	void setOption(uniString::utf8 key, uniString::utf8 value) throw(std::exception);

	bool load(const uniFile::filenameType &file, bool load = true) throw();
	bool rewriteConfigurationFile(bool minimal = true, bool messages = false, bool setup = false) const throw(std::exception); // throw on I/O error
	uniString::utf8 dumpConfigFile() throw();

	//////////////////////////////////////////////////////////////////////////////////////

	const std::vector<uniString::utf8>& deferredWarnLogMessages() const throw() { stackLock sml(m_lock); return m_deferredWarnLogMessages; }
	const std::vector<uniString::utf8>& deferredErrorLogMessages() const throw() { stackLock sml(m_lock); return m_deferredErrorLogMessages; }

	void clearDeferredWarnLogMessages() throw() { stackLock sml(m_lock); m_deferredWarnLogMessages.clear(); }
	void clearDeferredErrorLogMessages() throw() { stackLock sml(m_lock); m_deferredErrorLogMessages.clear(); }

	/////////// interface for service templates
	const std::vector<uniString::utf8> fromArgs(const std::vector<uniString::utf8> &cl) throw();
	bool getConsoleLogging() const throw();
	const uniFile::filenameType getFileLog() const throw();
	static uniString::utf8 getSystemLogConfigString() throw();
	static uniString::utf8 getVersionBuildStrings() throw();
	////////////////////////////////////////////////////////////////////////////
};

#undef OPT
#undef OPT_MULTI

#endif
