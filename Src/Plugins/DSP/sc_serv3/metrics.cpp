/* metrics,c  routines for sending client details to external server */

#include <curl/curl.h>
#include <string>
#include <deque>
#include <list>
#include <fstream>
#include <cstdlib>

#include "bandwidth.h"
#include "metrics.h"
#include "protocol_shoutcastClient.h"
#include "stats.h"
#include "config.h"
#include "services/stdServiceImpl.h"
#include "file/fileUtils.h"
#include "webNet/urlUtils.h"
#include "aolxml/aolxml.h"


using namespace std;
using namespace uniString;
using namespace stringUtil;

#define LOGNAME "METRICS"
#define DEBUG_LOG(...)    do { if (gOptions.adMetricsDebug()) DLOG(__VA_ARGS__); } while (0)

namespace metrics
{
#if 0
#define METRICS_LICENCE_URL "http://www.google.com"
#define LICENCE_RESP "\
<SHOUTCAST>\
  <FUNCTION level='10' />\
  <METRICS url=\"https://metrics.shoutcast.com/metrics\" />\
  <METRICSAD url=\"https://ads.shoutcast.com/dnas\" />\
  <YP url=\"https://dnas-licensing.shoutcast.com/yp\" />\
  <AUTH url=\"//auth.shoutcast.com/AddShout\" />\
  <AD url=\"//ads.shoutcast.com/dnas\" />\
</SHOUTCAST>\
"
#else
#define METRICS_LICENCE_URL "https://dnas-licensing.shoutcast.com/registration/"
#endif
    class service;

    struct metrics_info
    {
        httpHeaderMap_t vars;
        utf8 url;
        size_t id;
        unsigned int match;
        streamData::streamID_t sid;
        int group;
        int mode;
        metrics_info() : id(0), sid(0), group(0), mode(0) { match = 0; }
    };

    struct metrics_data
    {
        utf8 post;
        utf8 url;
        virtual int post_callback()   { return 0; }
        virtual int failed_callback() { return 0; }
        size_t id;
        int group;
        time_t m_schedule;
        int flags;
        streamData::streamID_t sid;

        metrics_data () : id(0), group(0), m_schedule((time_t)0), flags(0), sid(0) {}
        metrics_data (metrics_info &info) : id(0), group(0), m_schedule((time_t)0), flags(0) { sid = info.sid; }
        virtual ~metrics_data() {}

        virtual const char *name()  { return "metrics"; }
    };

    struct parse_response_data : public metrics_data
    {
        stringstream       m_ss;
        size_t             m_length;

        parse_response_data() : metrics_data(), m_length(0) {}
        parse_response_data(metrics_info &info) : metrics_data(info), m_length(0) {}

        virtual int post_callback() = 0;
        virtual int failed_callback() { WLOG ("failed " + utf8(name()) + " attempt with " + m_ss.str(), LOGNAME, sid); m_ss.str(""); return 0; }
    };


    struct licence_data : public parse_response_data
    {
        licence_data() {;}
        licence_data (metrics_info &info) : parse_response_data (info) {;}
        const char *name()  { return "licence"; }

        int post_callback();
        void handleURLs (aolxml::node *root);
        void checkURLNode (aolxml::node *root, const char *ref, service_t s);
    };


    struct YP_data : public parse_response_data
    {
        YP_data (metrics_info &info) : parse_response_data (info) {;}

        const char *name()  { return "YP"; }
        int post_callback();
    };


// libcurl related stuff

#ifdef CURLOPT_PASSWDFUNCTION
/* make sure that prompting at the console does not occur */
static int my_getpass(void *client, char *prompt, char *buffer, int buflen)
{
	buffer[0] = '\0';
	return 0;
}
#endif

static int handle_returned_header(void * ptr, size_t size, size_t nmemb, void *stream)
{
    int amount = (int)(size * nmemb);
#if defined(_DEBUG) || defined(DEBUG)
    metrics_data *entry = (metrics_data *)stream;
    DEBUG_LOG (utf8(entry->name()) + " header [" + utf8 ((const char*)ptr, amount>2 ? amount-2 : 0) + "]", LOGNAME);
#endif
    bandWidth::updateAmount(bandWidth::AUTH_AND_METRICS, amount);
    return amount;
}


static int handle_returned_data(void * ptr, size_t size, size_t nmemb, void * /*stream*/)
{
    int amount = (int)(size * nmemb);
#if defined(_DEBUG) || defined(DEBUG)
    DEBUG_LOG ("Body " + tos (amount) + ":" + utf8 ((const char*)ptr, amount), LOGNAME);
#endif
    bandWidth::updateAmount(bandWidth::AUTH_AND_METRICS, amount);
    return amount;
}


static size_t handle_licence_body (void *ptr, size_t size, size_t nmemb, void *stream)
{
    licence_data *entry = (licence_data *)stream;
    size_t length = size * nmemb;
    if (entry->m_ss)
    {
        if (entry->m_length > 20000)
        {
            WLOG ("response for was too large, ignoring for now");
            return 0;
        }
        //DLOG ("Adding " + tos (length) + ":" + utf8 ((const char*)ptr, length));
        entry->m_ss.write ((const char*)ptr, length);
        entry->m_length += length;
    }

    bandWidth::updateAmount (bandWidth::ADVERTS, length);
    return length;
}



    time_t                  g_metrics_updated = 0, g_recheck_services = 0;
    AOL_namespace::mutex    g_serversLock;
    utf8                    g_licence_DID = "";
    static short int        g_uniqueMetricsId = 0;

const short int getMetricsClientId()
{
	return ++g_uniqueMetricsId;
}


class service
{
protected:
    int                 m_queueCount;
    const short int 	m_id;
    unsigned short      m_flags;
	CURL 			*m_curl;
	string 			desc;
	bool			in_use;
	bool			running;
	utf8 			url;
	const time_t		updated;
	time_t			stop_time;
	utf8			main_post;
	list<metrics_data*>	queue;

	virtual void resetURL (utf8 &new_url)
	{
		if (new_url == "")
			return;
		url = new_url;
		m_curl = webClient::setupCurlDefaults (m_curl, LOGNAME, url, 5L);
		curl_easy_setopt (m_curl, CURLOPT_HEADERFUNCTION, handle_returned_header);
		curl_easy_setopt (m_curl, CURLOPT_WRITEFUNCTION, handle_returned_data);
#ifdef CURLOPT_PASSWDFUNCTION
		curl_easy_setopt (m_curl, CURLOPT_PASSWDFUNCTION, my_getpass);
#endif
	}

public:

	friend void addToServices(metrics_info &info);
	friend void metrics_stop();

	service(short int in_type, const char *in_url, const string in_desc) : m_id(getMetricsClientId()), in_use(false), running(true),
				updated(g_metrics_updated), stop_time(0)
	{
        httpHeaderMap_t vars;
        m_flags = in_type;
        m_queueCount = 0;
		url = in_url;
		desc = in_desc;
		m_curl = NULL;

		resetURL (url);

		vars["server"] = "Shoutcast v" + gOptions.getVersionBuildStrings();
		vars["port"] = tos(g_portForClients);
		main_post = encodeVariables(vars);
		DEBUG_LOG ("using " + url + " : " + main_post, in_desc.c_str());
	}

	virtual ~service(void)
	{
		metrics_cleanup();
		curl_easy_cleanup (m_curl);
	}

    virtual metrics_data *metrics_node(metrics_info &info)  { return new metrics_data (info); }
    virtual int failedEntry (metrics_data *) { return 0; }

	int  addEntry (metrics_info &info);
	void metrics_cleanup(void);

	static THREAD_FUNC process(void* arg);
};


class licenceService : public service
{
    int m_initial;
    time_t m_nextCheck;  // for licence

    void addCheckup(time_t when);
    metrics_data *metrics_node (metrics_info &info)  { return new licence_data (info); }

    void resetURL (utf8 &new_url)
    {
        //DLOG ("in reset url with " + new_url);
        if (new_url == "")
            return;
        url = new_url;
        m_curl = webClient::setupCurlDefaults (m_curl, LOGNAME, url, 5L);
        curl_easy_setopt (m_curl, CURLOPT_HEADERFUNCTION, handle_returned_header);
        curl_easy_setopt (m_curl, CURLOPT_WRITEFUNCTION, handle_licence_body);
#ifdef CURLOPT_PASSWDFUNCTION
        curl_easy_setopt (m_curl, CURLOPT_PASSWDFUNCTION, my_getpass);
#endif
    }

public:

    licenceService() : service (METRIC_LICENCE, "", "licence")
    {
        m_initial = 0;
        m_nextCheck = (time_t)0;
        utf8 s = METRICS_LICENCE_URL;
        resetURL (s);
        addCheckup (::time(NULL));
    }

    int failedEntry (metrics_data *entry)
    {
        if (entry->sid == 0)  // assume licence checker
        {
            if (entry->flags & 1)
                return -1;  // no licence, so no retry
            int retry = -1;
            m_initial++;
            if (m_initial == 1)
                retry = 1;
            else if (m_initial == 2)
            {
                retry = 10;
            }
            else if (m_initial == 3)
                //retry = 60;
                addCheckup (::time(NULL) + 60);
            else
            {
                m_initial = 0;
                addCheckup (::time(NULL) + 3600);
            }
            return retry;
        }
        return 0;
    }
};


class ypService : public service
{
    void resetURL (utf8 &new_url)
    {
        //DLOG ("in reset url with " + new_url);
        if (new_url == "")
            return;
        url = new_url;
        m_curl = webClient::setupCurlDefaults (m_curl, LOGNAME, url, 15L, 4L);
        curl_easy_setopt (m_curl, CURLOPT_HEADERFUNCTION, handle_returned_header);
        curl_easy_setopt (m_curl, CURLOPT_WRITEFUNCTION, handle_licence_body);
#ifdef CURLOPT_PASSWDFUNCTION
        curl_easy_setopt (m_curl, CURLOPT_PASSWDFUNCTION, my_getpass);
#endif
    }

public:

    ypService() : service (METRIC_YP, METRICS_YP_URL, "YP")  { resetURL (url); }

    metrics_data *metrics_node(metrics_info &info)  { return new YP_data (info); }

    int failedEntry (metrics_data *entry)
    {
        if (entry->flags)
            return -1;
        entry->flags |= 1;
        return 15;
    }
};


list <service*> servers;


int service::addEntry(metrics_info &info)
{
	httpHeaderMap_t &vars = info.vars;
	bool start_thread = false, queue_it = false;

	if (vars.empty())
    {
        if (running && (g_metrics_updated > updated))
        {
            DEBUG_LOG("Service " + tos(m_id) + " expired, start clean up", LOGNAME, (size_t)info.sid);
            running = false;
        }

		if ((running == false) && queue.empty())
		{
			DEBUG_LOG ("[METRICS] service to be removed, " + desc);
			return -1;  // trigger a service removal
		}
		start_thread = true;
	}

	if (stop_time == 0 && start_thread == false)
	{
		if (running && queue.size() < gOptions.metricsMaxQueue())
			queue_it = true;
	}
	if (info.mode || queue_it)
	{
		metrics_data *copy = metrics_node(info);
		copy->id = info.id;
		copy->sid = info.sid;
		copy->post = encodeVariables(vars);
		copy->url = info.url;
		start_thread = true;
		if (info.mode == 2)
			queue.push_front(copy);
		else
			queue.push_back(copy);
        m_queueCount++;
		DEBUG_LOG("[METRICS sid=" + tos(copy->sid) + "] Added " + utf8(desc) + " details to queue [count: " + tos(queue.size()) + "]", LOGNAME, copy->sid);
	}

	if ((in_use == false) && start_thread && !queue.empty())
	{
		in_use = true;
		SimpleThread(service::process, this);
	}
	return 0;
}

void service::metrics_cleanup()
{
	if (!queue.empty())
	{
        DEBUG_LOG("Purging " + tos(queue.size()) + " entries from " + tos(m_id), LOGNAME);

		while (!queue.empty())
		{
			list<metrics_data*>::iterator to_go = queue.begin();
			metrics_data *m = *to_go;
			//DLOG ("erasing metric for " + m->post);
			queue.erase(to_go);
			delete m;
			m = NULL;
		}
	}
}

THREAD_FUNC service::process(void* arg)
{
	try
	{
		service* m_service = reinterpret_cast<service*>(arg);
		if (m_service)
		{
            g_serversLock.lock();

			if (m_service->stop_time)
			{
				time_t diff = ::time(NULL) - m_service->stop_time;

				if (diff > (12 * 3600)) // drop after 12 hours of no response
				{
					m_service->metrics_cleanup();
					m_service->in_use = false;
					g_serversLock.unlock();
					return 0;
				}
				DEBUG_LOG ("[METRICS] time since stopping " + tos ((long)diff));
			}
            if (m_service->queue.size() < 5)
            {
                // allow for a build up of metrics over a small time, saves excessive thread creation
                g_serversLock.unlock();
                safe_sleep (0, 80);
                g_serversLock.lock();
			}

            int count = 0;
            int try_later = 0;
            time_t stop_time = (time_t)0;

			while (1)
			{
				utf8 post;
				char errormsg[CURL_ERROR_SIZE] = {0};

				if (m_service->m_queueCount <= try_later)
				{
                    if (count)
                        DEBUG_LOG ("[METRICS] run queue " + m_service->desc + " complete");
					break;
				}

				metrics_data *entry = m_service->queue.front();
				m_service->queue.pop_front();
                m_service->m_queueCount--;
                if (entry->m_schedule > 0 && entry->m_schedule > time(NULL))
                {
                    m_service->queue.push_back (entry);
                    m_service->m_queueCount++;
                    try_later++;
                    continue;
                }
				g_serversLock.unlock();

				if (entry->url.empty() == false)
					m_service->resetURL (entry->url);  // update the URL

                bool failed = false;

                if (entry->post.empty())
                {
                    if (entry->url.empty())
                        DLOG ("empty Post/URL update on " + m_service->desc, LOGNAME);
                }
                else
                {
					CURLcode ret = CURLE_FAILED_INIT;

					if (m_service->m_curl)
					{
						post = entry->post + "&" + m_service->main_post;

#if defined(_DEBUG) || defined(DEBUG)
						DEBUG_LOG(m_service->desc + utf8(" POST body: " + post), LOGNAME, entry->sid);
#endif

						curl_easy_setopt (m_service->m_curl, CURLOPT_ERRORBUFFER, errormsg);
						curl_easy_setopt (m_service->m_curl, CURLOPT_HEADERDATA, entry);
                        curl_easy_setopt (m_service->m_curl, CURLOPT_WRITEDATA, entry);
						curl_easy_setopt (m_service->m_curl, CURLOPT_POSTFIELDSIZE, post.size());
						curl_easy_setopt (m_service->m_curl, CURLOPT_POSTFIELDS, post.c_str());
						ret = curl_easy_perform (m_service->m_curl);
                        ++count;
					}

					if (ret != CURLE_OK)
					{
                        ELOG("Request failed on " + m_service->desc + " with "
                                + (errormsg[0] ? errormsg : curl_easy_strerror(ret)), LOGNAME, entry->sid);
                        failed = true;
                    }
                    else
                    {
                        long response_code = 0;
                        curl_easy_getinfo (m_service->m_curl, CURLINFO_RESPONSE_CODE, &response_code);
                        if (response_code >= 200 && response_code < 300)
                        {
                            entry->post_callback();
                            stop_time = 0;
                        }
                        else
                        {
                            ELOG("Request failed on " + m_service->desc + " with code " + tos (response_code));
                            entry->failed_callback();
                            failed = true;
                        }
                    }
                }
                if (failed && !iskilled())      // a failed metric, and server still running
                {
                    int delay = m_service->failedEntry (entry);

                    if (stop_time == 0)
                        stop_time = ::time(NULL);
                    if (delay >= 0)  // do we drop this one? < 0 something else is done, else put back on queue
                    {
                        g_serversLock.lock();
                        m_service->queue.push_front (entry);
                        m_service->m_queueCount++;

                        if (delay == 0)
                        {
                            m_service->stop_time = stop_time;
                            break;
                        }

                        g_serversLock.unlock();
                        DLOG ("sleeping for " + tos (delay) + "s on " + m_service->desc);
                        safe_sleep (delay, 0);
                        g_serversLock.lock();
                        continue;
                    }
                }

                delete entry;
                entry = NULL;
                g_serversLock.lock();
                m_service->stop_time = stop_time;
            } // while

            m_service->in_use = false;
            g_serversLock.unlock();
		}
	}
    catch(exception &e)
    {
        service* m_service = reinterpret_cast<service*>(arg);
        DLOG ("abort in metric " + m_service->desc + ", " + e.what());
        safe_sleep (0,500);
        if (m_service)
        {
            g_serversLock.lock();
            m_service->in_use = false;
            g_serversLock.unlock();
        }
    }
    return 0;
}


void addToServices(metrics_info &info)
{
    g_serversLock.lock();
    list <service*>::iterator it = servers.begin();

	while (it != servers.end())
	{
		service &s = **it;
		// DLOG ("Applying metric to " + s.desc + "(" + tos((long)s.flags) + ", " + tos((long)info.match) + ")");
		if ((s.m_flags & info.match) && s.addEntry(info) < 0)
		{
			list <service*>::iterator to_go = it;
			service *s = *it;
			++it;
			servers.erase(to_go);
			delete s;
			s = NULL;
		}
		else
		{
			++it;
		}
    }
    g_serversLock.unlock();
}


// assume only called from one thread.
void metrics_wakeup(bool force)
{
	time_t now = ::time(NULL);
    metrics_info info;
    info.match = METRIC_AUDIENCE | METRIC_ADVERT | METRIC_LICENCE | METRIC_YP;
    if ((now > g_recheck_services) || force)
    {
        if (force)
        {
            g_recheck_services = now + 60;
        }
        info.match |= METRIC_AUDIENCE | METRIC_ADVERT;

        g_recheck_services = now + 60; // next recheck in case stalled metrics
    }
    addToServices(info);
}


void metrics_listener_new(const protocol_shoutcastClient &client)
{
	const streamData *sd = client.m_streamData;
	if (sd && !sd->radionomyID().empty())
	{
		metrics_info info;
        const streamData::streamInfo &stream = sd->getInfo();

		info.vars["action"] = "listener_add";
		info.vars["tstamp"] = tos(::time(NULL));
		info.vars["host"] = sd->streamPublicIP();
		info.vars["radionomyid"] = info.vars["ref"] = sd->radionomyID();
		info.vars["client"] = tos(client.m_unique);
		info.vars["group"] = tos(client.getGroup());
		info.vars["ip"] = client.m_clientAddr;
		info.vars["srvid"] = stream.m_serverID;
		info.vars["mount"] = getStreamPath(client.m_streamID);
		info.vars["agent"] = client.m_userAgent;
		info.vars["referer"] = client.m_referer;
		info.vars["bitrate"] = tos(sd->streamBitrate());
		info.vars["codec"] = sd->streamContentType();
		info.vars["contr"] = client.getContainer();

		info.id = client.m_unique;
		info.sid = client.m_streamID;
		info.match = METRIC_AUDIENCE;

		addToServices(info);
	}
}

void metrics_listener_drop(const protocol_shoutcastClient &client)
{
	const streamData *sd = client.m_streamData;
	if (sd && !sd->radionomyID().empty())
	{
		metrics_info info;
        const streamData::streamInfo &stream = sd->getInfo();
		time_t now = ::time(NULL);

		info.vars["action"] = "listener_remove";
		info.vars["tstamp"] = tos(now);
		info.vars["host"] = sd->streamPublicIP();
		info.vars["radionomyid"] = info.vars["ref"] = sd->radionomyID();
		info.vars["client"] = tos(client.m_unique);
		info.vars["ip"] = client.m_clientAddr;
		info.vars["srvid"] = stream.m_serverID;
		info.vars["mount"] = getStreamPath(client.m_streamID);
		info.vars["duration"] = tos(now - client.m_startTime);
		info.vars["agent"] = client.m_userAgent;
		info.vars["referer"] = client.m_referer;
		info.vars["bitrate"] = tos(sd->streamBitrate());
		info.vars["codec"] = sd->streamContentType();
		info.vars["contr"] = client.getContainer();

		info.id = client.m_unique;
		info.sid = client.m_streamID;
		info.match = METRIC_AUDIENCE;

		addToServices(info);
	}
}


void metrics_adListener (const protocol_shoutcastClient &client, const adSummary &summary)
{
	metrics_info info;
    const streamData::streamInfo &stream = summary.sd->getInfo();

	info.vars["action"] = "listener_admetric";
	info.vars["tstamp"] = tos(summary.tstamp);
	info.vars["host"] = summary.sd->streamPublicIP();
	info.vars["radionomyid"] = info.vars["ref"] = summary.sd->radionomyID();
	info.vars["srvid"] = stream.m_serverID;
	info.vars["id"] = summary.id;
	info.vars["mount"] = summary.path.hideAsString();
	info.vars["client"] = tos(client.getUnique());
	info.vars["group"] = tos(client.getGroup());
	info.vars["sent"] = tos (client.getAdAccess().total_processed);
	info.vars["started"] = tos (client.getAdAccess().start_time);

	info.mode = 1;
	info.sid = summary.sid;
	info.match = METRIC_ADVERT;

	addToServices(info);
}


void metrics_advert_started (const adSummary &summary)
{
	metrics_info info;
    const streamData::streamInfo &stream = summary.sd->getInfo();

	info.vars["action"] = "ad_trigger";
	info.vars["tstamp"] = tos(summary.tstamp);
	info.vars["host"] = summary.sd->streamPublicIP();
	info.vars["radionomyid"] = info.vars["ref"] = summary.sd->radionomyID();
	info.vars["srvid"] = stream.m_serverID;
	info.vars["mount"] = summary.path.hideAsString();
	info.vars["id"] = summary.id;
	info.vars["listeners"] = tos(summary.count);
	info.vars["bitrate"] = tos(summary.sd->streamBitrate());
	info.vars["codec"] = summary.sd->streamContentType();

	info.mode = 1;
	info.sid = summary.sid;
	info.match = METRIC_ADVERT;

	addToServices(info);
}


void metrics_advert_stats(const adSummary &summary)
{
    metrics_info info;
    const streamData::streamInfo &stream = summary.sd->getInfo();

    info.vars["action"] = "ad_metric";
    info.vars["tstamp"] = tos(summary.tstamp);
    info.vars["host"] = summary.sd->streamPublicIP();
    info.vars["radionomyid"] = info.vars["ref"] = summary.sd->radionomyID();
    info.vars["srvid"] = stream.m_serverID;
    info.vars["id"] = summary.id;
    info.vars["mount"] = summary.path.hideAsString();
    info.vars["group"] = tos(summary.group);
    info.vars["file"] = summary.name;
    if (summary.missing)
        info.vars["missing"] = (summary.failed ? "failed" : "timeout");
    else
        info.vars["listeners"] = tos(summary.count);
    info.vars["bitrate"] = tos(summary.sd->streamBitrate());
    info.vars["codec"] = summary.sd->streamContentType();

    info.mode = 1;
    info.sid = summary.sid;
    info.match = METRIC_ADVERT;

    addToServices(info);
}


static bool _filled_info_notify (metrics_info &info, const streamID_t sid, const utf8& radionomyID,
	const utf8& serverID, const utf8& publicip, time_t tm = ::time(NULL))
{
	if (radionomyID.empty() || serverID.empty() || publicip.empty())
	    return false;

	info.vars["action"] = iskilled() ? "shutdown" : "reset";
	info.vars["tstamp"] = tos ((long)tm);
	info.vars["host"] = publicip;
	info.vars["radionomyid"] = info.vars["ref"] = radionomyID;
	info.vars["srvid"] = serverID;
	info.vars["mount"] = getStreamPath(sid);

	info.sid = sid;
	info.match = METRIC_NOTIFICATION;
	info.mode = 2;
	return true;
}

void metrics_stream_down (const streamID_t sid, const utf8& radionomyID,
	const utf8& serverID, const utf8& publicip, time_t tm)
{
	metrics_info info;
	if (_filled_info_notify (info, sid, radionomyID, serverID, publicip))
	{
		info.vars["action"] = iskilled() ? "shutdown" : "stopped";
        info.vars["started"] = tos(tm);
		addToServices(info);
	}
}

void metrics_stream_up (const streamID_t sid, const utf8& radionomyID,
	const utf8& serverID, const utf8& publicip, time_t tm)
{
	metrics_info info;
	if (_filled_info_notify (info, sid, radionomyID, serverID, publicip))
	{
		info.vars["action"] = iskilled() ? "shutdown" : "reset";
		addToServices(info);
	}
}


utf8 metrics_verifyDestIP(config &conf, bool full, uniString::utf8 url)
{
	// we'll try to set this where possible but it depends on 'destip' or 'publicip'
	// being set in order for us to have something to be able to be used by the YP
	utf8 destBindAddr = stripWhitespace((url.empty() ? (full && !conf.publicIP().empty() ? conf.publicIP() : conf.destIP()) : url));
	destBindAddr = stripHTTPprefix(destBindAddr);

	// with full then we're wanting to filter out some of the values
	// since this is then used for the public reponses / YP details
	if (full)
	{
		// cleanup things and only provide what should be valid i.e. nothing from a private network
		// and not send this even if provided just means the YP server will use the connection's IP
		if (isRemoteAddress(destBindAddr))
		{
			return destBindAddr;
		}
		return "";
	}
	// otherwise we just return the cleaned version as-is
	// since it'll allow for use in normal bindings, etc
	return destBindAddr;
}

utf8 createGuid()
{
#ifdef _WIN32
#define rand_r(x) rand()
#else
    static unsigned int seed = time(NULL);
#endif
    std::stringstream ss;
    for (int i = 0; i < 30; i++)
         ss << std::hex << (unsigned int)(rand_r(&seed) % 16);
    return ss.str();
}


#ifdef _MSC_VER
utf8 getWindowsRegKey (bool newone = false)
{
    wchar_t *subKey = L"Software\\Microsoft\\Cryptography";
    HKEY hKey;

    LONG nError = RegOpenKeyEx (HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hKey);
    if (nError == ERROR_FILE_NOT_FOUND)
    {
#if 0
        // maybe try to create a local guid to read from.
        subkey = L"Software\\SHOUTcast";
        nError = RegOpenKeyEx (HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hKey);
        if (nError == ERROR_FILE_NOT_FOUND)
        {
            // create one and put id in there
            nError = RegCreateKeyEx (HKEY_LOCAL_MACHINE, subKey, NULL, NULL, REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL, &hKey, NULL);
            if (nError == ERROR_FILE_NOT_FOUND)
                return "";

            string guid = createGuid ();
            DWORD dwSize = lstrlen(&guid[0]) * sizeof(TCHAR);
            nError = RegSetValueEx (hKey, lpValue, NULL, REG_SZ, (unsigned char *)&guid[0], dwSize);
            return guid;
        }
#else
        return "";
#endif
    }
    char buff[100];
    DWORD rdwSize = sizeof (buff);
    DWORD dwType = REG_SZ;
    nError = RegQueryValueExA (hKey, "MachineGuid", NULL, &dwType, (unsigned char*)buff, &rdwSize);
    RegCloseKey (hKey);
    return (nError) ? "" : buff;
}
#endif


void hashDID (utf8 &ident)
{
    // uses openssl for hashing a machine/installation Id
    unsigned char digest[SHA256_DIGEST_LENGTH];

    SHA256((unsigned char*)&ident[0], ident.size(), (unsigned char*)&digest);

    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
         ss << std::hex << (unsigned int)digest[i];
    g_licence_DID = ss.str();
    DLOG ("ident is " + ident + ", DID is " + g_licence_DID);
}


void metricsCheckDID ()
{
    if (g_licence_DID.empty() == false)
        return;

    utf8 s = "DIDC";  // Dnas ID Code
    do
    {
#ifdef _WIN32
        utf8 key = getWindowsRegKey();
        if (key.empty() == false)
        {
            s += key;
            break;
        }
#endif
#ifdef PLATFORM_LINUX
        ifstream myfile ("/etc/machine-id");
        if (myfile.is_open())
        {
            string line;
            getline (myfile, line);
            s += line;
            break;
        }
#endif
        // get random sequence if all else fails
        ILOG ("failed to get a static unique number, falling back to random sequence");
        s += createGuid();
    } while (0);

    // append conf bits to string. XXXX[-publicip]-baseport
    s += "-";
    s += tos(gOptions.portBase());
    utf8 p = gOptions.publicIP();
    if (p.empty() == false)
    {
        s += "-";
        s += p;
    }
    hashDID (s);
}


void metrics_apply(config &conf)
{
    bool same = (metrics_verifyDestIP(conf) == metrics_verifyDestIP(gOptions));
    if (!same || servers.empty())
    {
        if (!same)
        {
            metrics_stop();
        }

        DEBUG_LOG(utf8(same ? "Adding" : "Updating") + " metrics details", LOGNAME);
        metricsCheckDID ();

        service *s = new service (METRIC_AUDIENCE, METRICS_AUDIENCE_URL, "audience");
        servers.push_front(s);

        s = new service (METRIC_ADVERT, METRICS_ADVERTS_URL, "adservice");
        servers.push_front(s);

        s = new service (METRIC_NOTIFICATION, METRICS_RESET_URL, "notification");
        servers.push_front(s);

#ifndef LICENCE_FREE
        s = new licenceService ();
        servers.push_front(s);
#endif

        s = new ypService ();
        servers.push_front(s);
    }
}


void metrics_stop()
{
    g_serversLock.lock();
	if (!servers.empty())
	{
		int loop = 50;
		while (loop > 0)
		{
			list <service*>::iterator it = servers.begin();
			if (it == servers.end())
			{
				break;
			}

			service *s = *it;
			if (s)
			{
				if (s->in_use)
				{
					loop--;
					g_serversLock.unlock();
					safe_sleep(0, 100); // thread active so wait and try again
					g_serversLock.lock();
					continue;
				}
			}

			servers.erase(it);

			if (s)
			{
				delete s;
				s = NULL;
			}
		}
    }
    g_serversLock.unlock();
}


// called near the start of the stream and each metadata update.
//
void updateMeta (const metaInfo &meta)
{
    metrics_info info;
    utf8 uid = gOptions.userId ();

    utf8 ah = gOptions.stream_authHash (meta.m_sid);
    if (ah.empty())
        return;
    info.vars ["uid"] = uid;
    info.vars ["ah"] = ah;
    info.vars ["did"] = g_licence_DID;
    info.vars ["tstamp"] = tos (time(NULL));

    info.vars ["private"] = tos(meta.m_private ? 1 : 0);
    info.vars ["sid"] = tos(meta.m_sid);
    info.vars ["format"] = meta.m_format;
    info.vars ["audience"] = tos (stats::getUserCount (meta.m_sid));
    info.vars ["maxlisteners"] = tos(meta.m_maxListeners);
    info.vars ["currentsong"] = meta.m_song;
    info.vars ["bitrate"] = tos(meta.m_bitrate);
    info.vars ["mount"] = getStreamPath (meta.m_sid);
    info.vars ["samplerate"] = tos (meta.m_samplerate);
    info.vars ["verinfo"] = meta.m_version;
    info.vars ["agent"] = meta.m_agent;
    info.vars ["sourceip"] = meta.m_sourceIP;
    info.vars ["publicip"] = gOptions.publicIP();
    info.vars ["publicport"] = tos (gOptions.publicPort());
    info.vars ["secure"] = tos (threadedRunner::isSSLCapable() ? 1 : 0);

    stats::statsData_t data;
    stats::getStats (meta.m_sid, data);
    info.vars ["peaklisteners"] = tos(data.peakListeners);

    info.match = METRIC_YP;
    info.sid = meta.m_sid;
    info.mode = 1;
    DLOG ("push to YP requested \"" + meta.m_song + "\"", LOGNAME, meta.m_sid);
    addToServices(info);
}


void licence_data::checkURLNode (aolxml::node *root, const char *ref, service_t s)
{
    aolxml::node *n = aolxml::node::findNode (root, ref);
    if (n)
    {
        utf8 url = n->findAttributeString ("url");
        if (url.empty() == false)
        {
            metrics_info info;
            info.url = url;
            info.match = s;
            info.mode = 2;
            addToServices (info);
        }
    }
}


void licence_data::handleURLs (aolxml::node *root)
{
    if (root == NULL)
        return;
    checkURLNode (root, "/SHOUTCAST/METRICS", METRIC_AUDIENCE);
    checkURLNode (root, "/SHOUTCAST/METRICSAD", METRIC_ADVERT);
    checkURLNode (root, "/SHOUTCAST/YP", METRIC_YP);

    aolxml::node *n = aolxml::node::findNode (root, "/SHOUTCAST/AUTH");
    if (n)
    {
        utf8 s = n->findAttributeString ("url");
        if (s.empty() == false)
        {
            auth::g_authURL = s;
            auth::updateServices ();
        }
    }
}


int licence_data::post_callback ()
{
    vector<__uint8> v;
#ifdef LICENCE_RESP
    utf8 s = LICENCE_RESP;
    v.assign (&s[0], &s[s.size()]);
#else
    v.reserve (m_ss.tellp());
    std::copy (std::istreambuf_iterator<char>( m_ss ), std::istreambuf_iterator<char>(), std::back_inserter(v));
#endif

    aolxml::node *n = NULL, *root = NULL;
    do
    {
        if (v.empty())
            break;
#if defined(_DEBUG) || defined(DEBUG)
        //DLOG ("response size is " + tos (v.size()));
        DEBUG_LOG ("Licence body " + tos (v.size()) + ":" + utf8 ((const char*)&v[0], v.size()), LOGNAME);
#endif
        root = aolxml::node::parse (&v[0], v.size());
        if (root)
            n = aolxml::node::findNode (root, "/SHOUTCAST/FUNCTION");
        if (n == NULL)
        {
            ILOG ("license parse failed, skipping", LOGNAME);
            break;
        }

        utf8 s = n->findAttributeString ("level");
        if (s == "10")
        {
            ILOG ("detected paying offer", LOGNAME);
            streamData::streamInfo::m_allowSSL_global = 1;
            streamData::streamInfo::m_allowAllFormats_global = 1;
            streamData::streamInfo::m_allowBackupURL_global = 1;
            streamData::streamInfo::m_allowMaxBitrate_global = 0;
            break;
        }
        ILOG ("free offer only", LOGNAME);
        streamData::streamInfo::m_allowSSL_global = 0;
        streamData::streamInfo::m_allowAllFormats_global = 0;
        streamData::streamInfo::m_allowBackupURL_global = 0;
        streamData::streamInfo::m_allowMaxBitrate_global = 128;
    } while (0);

    handleURLs (root);

    forget (root);
    return 0;
}


// Called at server start and every hour.
void licenceService::addCheckup (time_t when)
{
    // eg http://dnas-services.shoutcast.com:8500/registration/?lid=2&debug=yes
    // eg https://dnas-services.shoutcast.com/registration/?lid=2&debug=yes
    httpHeaderMap_t vars;
    bool            licenceMissing = true;
    utf8            s = gOptions.userId();

    if (s.empty())
        return;
    vars ["uid"] = s;
    s = gOptions.licenceId();
    if (s.empty() == false)
    {
        vars ["lid"] = s;
        licenceMissing = false;
    }

    vars ["did"] = g_licence_DID;
    vars ["tstamp"] = tos (time(NULL));

    licence_data *copy = new licence_data();

    copy->post = encodeVariables(vars);
    copy->m_schedule = when;
    if (licenceMissing)
        copy->flags |= 1;

    g_serversLock.lock();
    queue.push_front (copy);
    m_queueCount++;
    g_serversLock.unlock();
}


int YP_data::post_callback ()
{
    vector<__uint8> v;
    v.reserve (m_ss.tellp());
    std::copy (std::istreambuf_iterator<char>( m_ss ), std::istreambuf_iterator<char>(), std::back_inserter(v));

    aolxml::node *n = NULL, *root = NULL;
    do
    {
        //DLOG ("response size is " + tos (v.size()));
        if (v.empty())
            break;
#if defined(_DEBUG) || defined(DEBUG)
        DEBUG_LOG ("YP Body " + tos (v.size()) + ":" + utf8 ((const char*)&v[0], v.size()), name(), sid);
#endif
        root = aolxml::node::parse (&v[0], v.size());
        if (root)
            n = aolxml::node::findNode (root, "/response");
        if (n == NULL)
        {
            ILOG ("response invalid, skipping", name(), sid);
            break;
        }
        int code = aolxml::subNodeText(n, "/response/statusCode", 400);
        if (code != 200)
        {
            utf8 msg = aolxml::subNodeText(n, "/response/statusText", (utf8)"");
            if (msg != (utf8)"")
                WLOG ("response returned " + tos(code) + ", " + msg, name(), sid);
            break;
        }
        n = aolxml::node::findNode (root, "/response/data");
        if (n == NULL)
        {
            ILOG ("No special settings from YP for stream " + tos(sid), name(), sid);
            break;
        }
        yp2::stationInfo info;
        info.m_advertMode = aolxml::subNodeText(n, "/response/data/admode", 0);
        info.m_streamTitle = aolxml::subNodeText(n, "/response/data/stationname", (utf8)"");
        info.m_stationID = aolxml::subNodeText(n, "/response/data/stationid", (utf8)"");
        info.m_serverID = aolxml::subNodeText(n, "/response/data/serverid", (utf8)"");
        info.m_radionomyID = aolxml::subNodeText(n, "/response/data/callsign", (utf8)"");
        info.m_responseCode = code;
        info.m_advertType = aolxml::subNodeText(n, "/response/data/advert/type", (utf8)"fixed");
        info.m_advertTrigger = aolxml::subNodeText(n, "/response/data/advert/trigger", (utf8)"meta");

        streamData *sd = streamData::accessStream (sid);
        if (sd)
        {
            sd->YP2_updateInfo (info);
            sd->releaseStream();
        }
        ILOG ("Stream #" + tos(sid) + " has been updated on the Shoutcast Directory.", name(), sid);

    } while (0);

    forget (root);
    return 0;
}

} // namespace
