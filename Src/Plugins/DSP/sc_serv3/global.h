#pragma once
#ifndef global_H_
#define global_H_

#include "config.h"
#include "versions.h"
#include <limits.h>
#include <stdexcept>

// #define INCLUDE_BACKUP_STREAMS 1
// #define INCLUDE_SSL_CERTS 1

#define EL "\n"

#ifdef _WIN32
#define getpid _getpid
#define MAXHOSTNAMELEN 256
#endif
#ifndef MAXHOSTNAMELEN
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
#endif
#define MAXHOSTNAMELEN		HOST_NAME_MAX
#endif

extern config gOptions;
extern size_t gFF_fix;

// manage killed state
const int iskilled() throw();
void setkill(int v) throw();

// manage post-setup handling
const int isPostSetup() throw();
void setPostSetup(int v) throw();

// helper template to throw an exception with a utf8 stream
template<typename T>	inline void throwEx(const char *msg) throw(T) { throw T(msg); }
template<typename T>	inline void throwEx(const uniString::utf8 &msg) throw(T) { throw T(msg.hideAsString()); }
template<typename T>	inline void throwEx(const std::string &msg) throw(T) { throw T(msg); }

// subclass used to distinguish between error messages that are already labeled
// with [<subsystem>] and those that aren't
class tagged_error: public std::runtime_error
{
public:
	explicit tagged_error(const std::string &s):runtime_error(s){}
};


struct parserInfo
{
    unsigned long   m_mask;
    unsigned long   m_pattern;
    unsigned int    m_samplerate;
    int             m_version;
    int             m_bitrate;
    unsigned        m_frameCount;
    unsigned        m_reduce;
    float           m_duration;
    bool            m_inUse;
    std::string     m_description;

    virtual int verifyFrame (const unsigned char *buf, unsigned int len) = 0;
    virtual const std::string &getDescription() const  { return m_description; }
    virtual const char *getVersionName() const = 0;

    parserInfo(unsigned long mask = 0, unsigned long v = 0) { m_mask = mask; m_pattern = v; m_samplerate = m_bitrate = 0; m_inUse = false; m_frameCount = m_reduce = 0; m_duration = 0.0; m_version = 0; }
    virtual int getUvoxType() = 0;
    virtual ~parserInfo() {}
};

/////////////// various strings /////////////////////////////

void constructMessageResponses();

extern const uniString::utf8::value_type* MSG_INVALIDPASSWORD;
extern const int MSG_INVALIDPASSWORD_LEN;

extern const uniString::utf8::value_type* MSG_VALIDPASSWORD;
extern const int MSG_VALIDPASSWORD_LEN;

extern const uniString::utf8::value_type* MSG_HTTP_VALIDPASSWORD;
extern const int MSG_HTTP_VALIDPASSWORD_LEN;

extern const uniString::utf8::value_type* MSG_BADSTREAMID;
extern const int MSG_BADSTREAMID_LEN;

extern const uniString::utf8::value_type* MSG_STREAMINUSE;
extern const int MSG_STREAMINUSE_LEN;

extern const uniString::utf8::value_type* MSG_STREAMMOVED;
extern const int MSG_STREAMMOVED_LEN;

extern uniString::utf8 MSG_ICY_HTTP401;
extern uniString::utf8 MSG_ICY200;
extern uniString::utf8 MSG_ICY_HTTP200;
extern uniString::utf8 MSG_UVOX_HTTP200;

extern int MSG_ICY_HTTP401_LEN;

extern const uniString::utf8::value_type* MSG_AUTHFAILURE401;

extern const uniString::utf8::value_type* MSG_200;
extern const uniString::utf8::value_type* MSG_NO_CLOSE_200;
extern const uniString::utf8::value_type* MSG_STD200;

extern const uniString::utf8::value_type* MSG_HTTP400;

extern const uniString::utf8::value_type* MSG_HTTP403;

extern const uniString::utf8::value_type* MSG_HTTP404;
extern const int MSG_HTTP404_LEN;

extern const uniString::utf8::value_type* MSG_HTTP405;

extern const uniString::utf8::value_type* MSG_HTTP503;
extern const int MSG_HTTP503_LEN;

extern uniString::utf8 g_userAgentBase; // user agent for sc_serv2
extern uniString::utf8 g_userAgent;		// user agent for sc_serv2

extern time_t g_upTime;
const uniString::utf8 timeString(const time_t t, const bool slim = false) throw();

const bool isUserAgentRelay(const uniString::utf8 &user_agent) throw();
const bool isUserAgentOfficial(const uniString::utf8 &user_agent) throw();

const uniString::utf8 redirect(const uniString::utf8 &url, bool compress) throw();
const uniString::utf8 urlLink(const uniString::utf8 &url, const uniString::utf8 &text = "", const bool embed = false) throw();

const uniString::utf8 http302(const uniString::utf8 &url) throw();

const uniString::utf8 addWBR(const uniString::utf8 &str) throw();

uniString::utf8 getfooterStr();
uniString::utf8 getHTML5Remover();
uniString::utf8 getIEFlexFix();
uniString::utf8 getStreamListeners(const size_t sid, const bool nowrap, const int fh);

extern uniString::utf8 g_IPAddressForClients;	// address clients will connect to
extern u_short g_portForClients;				// port clients will connect to, generally portBase
extern int g_legacyPort;						// port legacy v1 sources will connect to, or not

const bool reloadConfig(int force);
void reloadBanLists();
void reloadRipLists();
void reloadAdminAccessList();
void reloadAgentLists();
void rotatew3cFiles(uniString::utf8 files);

const uniString::utf8 loadLocalFile(uniFile::filenameType fn, const uniString::utf8& logPrefix = "", const size_t sizeLimit = 0);

const uniString::utf8 escapeJSON(const uniString::utf8 &s) throw();

// creates GZIP (RFC 1952) encoded output as
// either one go or in blocks for logs, etc
#include <zlib.h>
const bool compressData(uniString::utf8 &body);
const bool compressDataStart(uniString::utf8 &body, z_stream *stream, Bytef* name = (Bytef*)"sc_serv.log\0", bool local = true);
const int compressDataCont(uniString::utf8 &body, z_stream *stream);
void compressDataFinish(uniString::utf8 &body, z_stream *stream);
const bool compressDataEnd(z_stream *stream);


// used for getting and setting modified dates on relevant http responses
// set use = 0 for current time, otherwise pass in a time_t as needed
const uniString::utf8 getRFCDate(const time_t use);
const time_t readRFCDate(const uniString::utf8& str);
#ifdef _WIN32
char *strptime(const char *buf, const char *format, struct tm *timeptr);
#endif
const uniString::utf8 getStreamPath(const size_t sid, const bool for_public = false);

const bool isCDNMaster(size_t sid);
const bool isCDNSlave(size_t sid);

#ifndef _WIN32
#define localtime_s(x,y) localtime_r(y,x)
#else
#define snprintf _snprintf
#endif

// return system error string
const uniString::utf8 errMessage() throw();

const uniString::utf8 bob();

void printUpdateMessage();

extern uniString::utf8 listenerId;
const uniString::utf8 randomId(uniString::utf8 &temp);

const uniString::utf8 getCurrentSong(uniString::utf8 currentSong);
const uniString::utf8 stripHTTPprefix(const uniString::utf8& addr);

const bool isAddress(const uniString::utf8& addr);

const bool extractPassword(uniString::utf8 &dj_password, uniString::utf8 &dj_name, int &streamID);

typedef std::map<uniString::utf8,uniString::utf8> httpHeaderMap_t;
const int getStreamBitrate(const httpHeaderMap_t &headers);
const int getStreamSamplerate(const httpHeaderMap_t &headers);
const int getHTTPRequestDetails(const std::string &firstLine, std::string &request,
								std::string &url, std::string &protocolAndVersion);
const uniString::utf8 fixMimeType(const uniString::utf8& mimeType);

const uniString::utf8 sampleRateStr(const int sr);

const int detectAutoDumpTimeout(time_t &cur_time, const time_t lastActivityTime,
								const uniString::utf8& msg, const bool debug,
								const size_t streamID = DEFAULT_SOURCE_STREAM) throw(std::runtime_error);

const __uint64 time_now_ms();

const bool isRemoteAddress(const uniString::utf8 &addr);

#endif
