/* metrics.h  external interface to metric processing */

#ifndef _METRICS_H
#define _METRICS_H

#include "unicode/uniString.h"

class protocol_shoutcastClient;
class streamData;
class config;

#if 1
#define METRICS_AUDIENCE_URL "https://metrics.shoutcast.com/metrics"
#define METRICS_ADVERTS_URL "https://ads.shoutcast.com/dnas"
#define TARGETSPOT_URL "https://ads.shoutcast.com/dnas"
#define METRICS_YP_URL "https://dnas-services.shoutcast.com/yp/"
#else
#define METRICS_AUDIENCE_URL "http://localhost:9001"
#define METRICS_ADVERTS_URL "http://localhost:9001/dnas"
#define TARGETSPOT_URL "http://localhost/~karl/mapping1.php"
#endif
#define METRICS_RESET_URL "https://metrics.radionomy.com/connections/closednas"


namespace metrics
{
	typedef size_t streamID_t;
    typedef enum {
        METRIC_AUDIENCE = 1,
        METRIC_ADVERT = 2,
        METRIC_NOTIFICATION = 4,
        METRIC_LICENCE = 8,
        METRIC_YP = 16
    } service_t;

    struct adSummary
    {
        streamID_t         sid;
        bool               sendRest;
        bool               failed;
        bool               missing;
        uniString::utf8    path;
        time_t             tstamp;
        const char         *name;
        int                count;
        std::string        id;
        size_t             group;
        streamData         *sd;

        adSummary() : sid(1), tstamp(0), name(0), count(0), group(0) { sd = NULL; sendRest = failed = missing = false; }
    };

    struct metaInfo
    {
        streamID_t          m_sid;
        bool                m_private;
        unsigned int        m_audience;
        unsigned int        m_maxListeners;
        unsigned int        m_bitrate;
        unsigned int        m_samplerate;
        uniString::utf8     m_version;
        uniString::utf8     m_song;
        uniString::utf8     m_format;
        uniString::utf8     m_agent;
        uniString::utf8     m_publicIP;
        uniString::utf8     m_sourceIP;
    };

	void metrics_init();
	void metrics_stop();
	void metrics_apply(config &conf);

	//  new listener triggers
	void metrics_listener_new(const protocol_shoutcastClient &client);

	// exiting listener triggers
	void metrics_listener_drop(const protocol_shoutcastClient &client);

	void metrics_adListener (const protocol_shoutcastClient &client, const adSummary &summary);

	void metrics_advert_started (const adSummary &summary);

	void metrics_advert_stats(const adSummary &summary);

	typedef unsigned yp2SessionKey;

	void metrics_stream_down (const streamID_t sid, const uniString::utf8& radionomyID,
		const uniString::utf8& serverID, const uniString::utf8& publicip, time_t tm);

	void metrics_stream_up (const streamID_t sid, const uniString::utf8& radionomyID,
		const uniString::utf8& serverID, const uniString::utf8& publicip, time_t tm);

	// periodic wakeup for any stalled metrics
	void metrics_wakeup(bool force = false);

    void updateMeta (const metaInfo &meta);

	uniString::utf8 metrics_verifyDestIP(config &conf, bool full = true, uniString::utf8 url = "");
}

#endif  /* _METRICS_H */
