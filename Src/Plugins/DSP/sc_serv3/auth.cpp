/* auth.cpp  routines for authenticating client details with external server */

#include <curl/curl.h>
#include <list>

#include "protocol_shoutcastClient.h"
#include "uvox2Common.h"
#include "webClient.h"
#include "auth.h"
#include "bandwidth.h"
#include "file/fileUtils.h"
#include "services/stdServiceImpl.h"

#define MAX_AUTH_THREADS                20
#define AMNT_ALLOWED_BEFORE_SKIPPING    1000

#define LOGNAME "[AUTH] "
#define DEBUG_LOG(...)    do { if (gOptions.authDebug()) DLOG(__VA_ARGS__); } while(0)

extern int xferinfo(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

namespace auth
{
	using namespace std;
	using namespace uniString;
	using namespace stringUtil;

	class authService
	{
	public:
		time_t				restart_time;
		utf8				main_post;
		list<auth_info*>	clients;
		CURL				*m_curl;
		bool				in_use;

		authService();
		~authService();

		static THREAD_FUNC process(void *arg);

		friend void schedule(auth_info *info);
		friend void init();
	};

    list<auth_info*>        g_queue;
    AOL_namespace::mutex    g_qMutex;
    authService *           g_services = 0;
    utf8                    g_authURL = DNAS_AUTH_URL;

#ifdef CURLOPT_PASSWDFUNCTION
	/* make sure that prompting at the console does not occur */
	static int my_getpass(void *client, char *prompt, char *buffer, int buflen)
	{
		buffer[0] = '\0';
		return 0;
	}
#endif

	static int handle_returned_header(void *ptr, size_t size, size_t nmemb, void *stream)
	{
		auth_info *info = reinterpret_cast<auth_info*>(stream);
		if (info)
		{
			utf8 line((char*)ptr);
			if (!line.empty())
			{
				line = stripWhitespace(line);
				if (!line.empty())
				{
					// skip over the initial HTTP header and just look at key pairs
					utf8::size_type pos = line.find(utf8(":"));
					if (pos != utf8::npos)
					{
						utf8 key = toLower(stripWhitespace(line.substr(0,pos)));
						utf8 value = stripWhitespace(line.substr(pos+1));

						if (!key.empty())
						{
							DEBUG_LOG("[AUTH sid=" + tos(info->sid) + "] Auth Header [" + key + ":" + value + "]");

							if (key == "shoutcast-auth-user")
							{
								if (value == "withintro")
								{
									info->has_intro = true;
								}
								info->authenticated = true;
								info->valid_response = true;
							}
							else if (key == "advert-group")
							{
								info->group = atoi(value.hideAsString().c_str());
								#if defined(_DEBUG) || defined(DEBUG)
								if (info->group == 1729)
								{
									info->group = 501;
								}
								#endif
								info->valid_response = true;
							}
						}
					}
				}
			}
		}

		int amount = (int)(size * nmemb);
		bandWidth::updateAmount(bandWidth::AUTH_AND_METRICS, amount);
		return amount;
	}

	static int handle_returned_data(void *ptr, size_t size, size_t nmemb, void *stream)
	{
		auth_info *info = reinterpret_cast<auth_info*>(stream);
		if (info)
		{
			// cap any intro content to a few meg
			if (info->has_intro && ((int)info->content.size() <= gOptions.maxSpecialFileSize()))
			{
				vector <__uint8> &v = info->content;
				__uint8 *s = (__uint8*)ptr;
				v.insert (v.end(), s, s + (size * nmemb));
			}
		}

		int amount = (int)(size * nmemb);
		bandWidth::updateAmount(bandWidth::AUTH_AND_METRICS, amount);
		return amount;
	}

    authService::authService() : restart_time(0), in_use(false)
    {
        m_curl = webClient::setupCurlDefaults (NULL, LOGNAME, g_authURL, 4L);
        //DEBUG_LOG("[AUTH] Starting auth instance");
    }

    authService::~authService()
    {
        restart_time = 0;
        in_use = false;
        if (m_curl)
        {
            curl_easy_cleanup(m_curl);
            m_curl = NULL;
        }
        //DEBUG_LOG("[AUTH] Stopping auth instance");
    }

    THREAD_FUNC authService::process(void* arg)
    {
        try
        {
            authService* m_auth = reinterpret_cast<authService*>(arg);
            if (m_auth)
            {
                if (!iskilled())
                {
                    g_qMutex.lock();

                    while (!m_auth->clients.empty())
                    {
                        auth_info *info = m_auth->clients.front();
                        m_auth->clients.pop_front();
                        g_qMutex.unlock();

                        if (info)
                        {
                            if (info->url.empty() == false)
                            {
                                g_qMutex.lock();
                                m_auth->main_post = info->post;
                                m_auth->m_curl = webClient::setupCurlDefaults (m_auth->m_curl, LOGNAME, info->url, 4L);
                                DLOG ("updated main post to " + m_auth->main_post + " and URL " + info->url, LOGNAME, info->sid);
                                g_qMutex.unlock();
                            }

                            if (m_auth->restart_time && (::time(NULL) >= m_auth->restart_time))
                            {
                                m_auth->restart_time = 0;
                                DEBUG_LOG("Restarting disabled auth", LOGNAME, info->sid);
                            }

							protocol_shoutcastClient *client = info->client;
							if (client)	// if set to be kicked then no point in processing
							{
								if (client->m_kickNextRound == false)
								{
									if (m_auth->restart_time)
									{
										// auth is disabled for now, but assume client is ok
										info->authenticated = true;
									}
									else
									{
                                        if (info->post.empty() == false)
                                        {
                                            utf8 post = info->post + "&" + m_auth->main_post;

#if defined(_DEBUG) || defined(DEBUG)
                                            DEBUG_LOG("POST body: " + utf8(post.c_str()), LOGNAME, info->sid);
#endif

                                            char errormsg[CURL_ERROR_SIZE] = {0};
                                            if (m_auth->m_curl)
                                            {
                                                curl_easy_setopt(m_auth->m_curl, CURLOPT_HEADERFUNCTION, handle_returned_header);
                                                curl_easy_setopt(m_auth->m_curl, CURLOPT_WRITEFUNCTION, handle_returned_data);

                                                curl_easy_setopt(m_auth->m_curl, CURLOPT_ERRORBUFFER, errormsg);
                                                curl_easy_setopt(m_auth->m_curl, CURLOPT_HEADERDATA, info);
                                                curl_easy_setopt(m_auth->m_curl, CURLOPT_WRITEDATA, info);
                                                curl_easy_setopt(m_auth->m_curl, CURLOPT_POSTFIELDSIZE, post.size());
                                                curl_easy_setopt(m_auth->m_curl, CURLOPT_COPYPOSTFIELDS, post.c_str());

                                                // use progress/xfer functions to trap for the server kill case
                                                curl_easy_setopt(m_auth->m_curl, CURLOPT_NOPROGRESS, 0L);
                                                curl_easy_setopt(m_auth->m_curl, CURLOPT_XFERINFOFUNCTION, xferinfo);
                                                curl_easy_setopt(m_auth->m_curl, CURLOPT_XFERINFODATA, info->sid);
                                            }

                                            CURLcode ret = CURLE_FAILED_INIT;
                                            if (!m_auth->m_curl || ((ret = curl_easy_perform(m_auth->m_curl)) != CURLE_OK))
                                            {
                                                ELOG("[AUTH sid=" + tos(info->sid) + "] Request failed on auth with " +
                                                        (errormsg[0] ? errormsg : curl_easy_strerror(ret)));

                                                g_qMutex.lock();
                                                if (m_auth->restart_time == 0)
                                                {
                                                    m_auth->restart_time = ::time(NULL) + 60;
                                                }
                                                g_qMutex.unlock();

                                                info->authenticated = true;
                                            }
                                        }
                                    }

									if (info->authenticated)
									{
										// we need to double-check that the client still
										// exists as there could have been a delay with
										// the processing of this and it's since dropped
										// or been kicked - if so we don't want to crash
										//stats::

										if (info->group >= 0)
										{
											client->setGroup(info->group);
										}
										if (info->has_intro)
										{
											DEBUG_LOG("[AUTH sid=" + tos(info->sid) + "] Listener auth supplied intro of " + tos((long)info->content.size()));
											client->setIntro(info->content, info->m_dataType);
										}
									}
									else
									{
										// if we didn't get the required header responses
										// but we did get a valid response from the server
										// then we should just allow the client to connect
										// as we don't know what's going on so assume "ok"
										if (!info->valid_response)
										{
											info->authenticated = true;
										}
										else
										{
											DEBUG_LOG("[AUTH sid=" + tos(info->sid) + "] Auth failed, 403 returned");
											client->return_403();
										}
									}
								}

								// if we're re-processing the client then we
								// musn't re-schedule it else it'll go boom!
								if (!info->delayed_auth)
								{
									threadedRunner::scheduleRunnable(client);
								}
							}
						}

						delete info;
						info = NULL;

						g_qMutex.lock();
					}

					m_auth->clients.clear();
					m_auth->in_use = false;
					g_qMutex.unlock();
				}
			}
		}
		catch (...)
		{
			authService* m_auth = reinterpret_cast<authService*>(arg);
			if (m_auth)
			{
				g_qMutex.lock();
				m_auth->clients.clear();
				m_auth->in_use = false;
				g_qMutex.unlock();
			}
		}
		return 0;
	}

	void schedule(auth_info *info)
	{
		if (info)
		{
			g_qMutex.lock();
			do
			{
				if (g_queue.size() > AMNT_ALLOWED_BEFORE_SKIPPING)
				{
					// what to do when under stress/timeout
					DEBUG_LOG(LOGNAME "Heavy backlog of auth requests, skipping");
					threadedRunner::scheduleRunnable(info->client);
					delete info;
					info = NULL;
					break;
				}

				if (g_services)
				{
					int i = 0;
					for (; i < MAX_AUTH_THREADS; ++i)
					{
						if (g_services[i].in_use == false)
						{
							// start auth thread processing
							g_services[i].in_use = true;
							g_services[i].clients.push_back(info);
							if (!g_queue.empty())
							{
								g_services[i].clients.insert(g_services[i].clients.end(), g_queue.begin(), g_queue.end());
								g_queue.clear();
							}

							// do what we can to make sure that if we fail
							// to start the thread then we'll pass it on
							DEBUG_LOG(LOGNAME "Starting auth thread #" + tos(i+1));
							SimpleThread(authService::process, &g_services[i]);
							break;
						}
					}

					if (i >= MAX_AUTH_THREADS)
					{
						g_queue.push_back(info);
						//DEBUG_LOG(LOGNAME "Unable to process auth requests, skipping");
						//threadedRunner::scheduleRunnable(info->client);
						//delete info;
						//info = NULL;
						break;
					}
				}
				else
				{
					DEBUG_LOG(LOGNAME "Unable to process auth requests, skipping");
					info->authenticated = true;
					threadedRunner::scheduleRunnable(info->client);
					delete info;
					info = NULL;
					break;
				}

				// make sure that if things go arwy that we should be able to terminate
				if (iskilled())
				{
					break;
				}
			} while (0);
			g_qMutex.unlock();
		}
	}

    void updateServices (bool initial)
    {
        if (g_services == NULL)
            return;
        httpHeaderMap_t vars;
        vars["server"] = "Shoutcast v" + gOptions.getVersionBuildStrings();
        vars["port"] = tos(g_portForClients);
        utf8 main_post = encodeVariables(vars);

        for (int i = 0; i < MAX_AUTH_THREADS; ++i)
        {
            if (initial)
            {
                g_services[i].main_post = main_post;
            }
            else
            {
                auth_info *info = new auth::auth_info (g_authURL);
                info->post = main_post;
                g_qMutex.lock();
                g_services[i].clients.push_back (info);
                if (g_services[i].in_use == false)
                {
                    g_services[i].in_use = true;
                    SimpleThread(authService::process, &g_services[i]);
                }
                g_qMutex.unlock();
            }
        }
    }

    void init()
    {
        g_services = new authService[MAX_AUTH_THREADS];
        if (g_services)
        {
            updateServices (true);
        }
        else
        {
            WLOG(LOGNAME "Failed to start auth service threads");
        }
    }

	void cleanup()
	{
		if (g_services)
		{
			delete[] g_services;
			g_services = NULL;
		}
	}
}
