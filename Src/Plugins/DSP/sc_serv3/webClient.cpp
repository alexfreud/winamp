#ifdef _WIN32
#include <winsock2.h>
#endif

#include <stdio.h>
#include "webClient.h"
#include "webNet/urlUtils.h"
#include "stl/stringUtils.h"
#include "file/fileUtils.h"
#include "services/stdServiceImpl.h"

using namespace std;
using namespace stringUtil;
using namespace uniString;

#define DEBUG_LOG(...)  do { if (gOptions.webClientDebug()) DLOG(__VA_ARGS__); } while (0)

////////////////// utils

static utf8 toLogString(const webClient::request &r) throw()
{
	// using this to ensure the default YP connection is via HTTPS
	// though if there is a config issue then attempt to handle it
	// by reverting to the pre-HTTPS mode so we can still work-ish
	const bool https = ((r.m_addr == DEFAULT_YP_ADDRESS) && uniFile::fileExists(gOptions.m_certPath));
	if (r.m_port == 80)
	{
		return utf8(https ? "https://" : "http://") + r.m_addr + r.m_path;
	}
	else
	{
		return utf8(https ? "https://" : "http://") + r.m_addr + ":" + tos(r.m_port) + r.m_path;
	}
}

utf8 encodeVariables(const httpHeaderMap_t &m) throw()
{
	utf8 result;

	if (!m.empty())
	{
		for (httpHeaderMap_t::const_iterator i = m.begin(); i != m.end(); ++i)
		{
			if (i != m.begin())
			{
				result += "&";
			}
			result += urlUtils::escapeURI_RFC3986((*i).first) + "=";
			if (!(*i).second.empty())
			{
				result += urlUtils::escapeURI_RFC3986((*i).second);
			}
		}
	}
	return result;
}

static utf8 toLogString(const httpHeaderMap_t &m) throw()
{
	utf8 result = "headers=[" + eol();
	for (httpHeaderMap_t::const_iterator i = m.begin(); i != m.end(); ++i)
	{
		result += "    " + (*i).first + " : " + (*i).second + eol();
	}
	result += "]" + eol();
	return result;
}

static utf8 toLogString(const webClient::response &r) throw()
{
	utf8 result = "code=" + tos(r.m_resultCode) + " reason=" + r.m_resultText + eol();
	result += toLogString(r.m_headers);
	result += "body=[" + eol();
	result += r.m_body + eol() + "]";
	return result;
}

////////////////////////

webClient::webClient(const uniString::utf8 &logPrefix) throw()
	: m_logPrefix(logPrefix), m_state(&webClient::state_Idle),
	  m_nextState(0), m_nonBlockingID(0), m_response()
{
	m_curl_error = new char[CURL_ERROR_SIZE];
	memset(m_curl_error, 0, CURL_ERROR_SIZE);
}

webClient::~webClient() throw()
{
	for (curlMap_t::const_iterator i = m_curl.begin(); i != m_curl.end(); ++i)
	{
		if (i->second != NULL)
		{
			curl_easy_cleanup(i->second);
		}
	}
	for (curlHeadersMap_t::const_iterator i = m_curl_headers.begin(); i != m_curl_headers.end(); ++i)
	{
		if (i->second != NULL)
		{
			curl_slist_free_all(i->second);
		}
	}
	m_curl.clear();
	forgetArray(m_curl_error);
}

///////////////////////////////////////////////////////////////////

void webClient::queueRequest(const request &req) throw()
{
#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(m_logPrefix + __FUNCTION__);
#endif

	stackLock sml(m_requestQueueLock);

	m_requestQueue.push(req);
	m_requestSignal.set();
}

void webClient::timeSlice() throw(exception)
{
	try
	{
		if (this->m_state)
		{
			return (this->*m_state)();
		}
	}
	catch (const webClient::retry_exception &ex)
	{
		// increase failure counter
		m_requestQueueLock.lock();
		if (!m_requestQueue.empty())
		{
			++m_requestQueue.front().m_retryCounter;
		}
		m_requestQueueLock.unlock();

		m_state = &webClient::state_Wait;
		m_nextState = &webClient::state_SendRequest;
		m_lastActivityTime = ::time(NULL);
		m_result.timeout(gOptions.ypTimeout());

		utf8 what = ex.what();
		WLOG(((what.find(m_logPrefix) == utf8::npos) ? m_logPrefix : "") +
			 what + (!what.empty() ? " - " : "") +
			 "Retrying in " + tos(gOptions.ypTimeout()) + " seconds.");
	}
	catch (const exception &ex)
	{
		// increase failure counter
		m_requestQueueLock.lock();
		if (!m_requestQueue.empty())
		{
			++m_requestQueue.front().m_retryCounter;
		}
		m_requestQueueLock.unlock();

		m_state = &webClient::state_Wait;
		m_nextState = &webClient::state_SendRequest;
		m_lastActivityTime = ::time(NULL);
		m_result.timeout(gOptions.ypTimeout());

		utf8 what = ex.what();
		ELOG(((what.find(m_logPrefix) == utf8::npos) ? m_logPrefix : "") +
			 what + (!what.empty() ? " - " : "") +
			 "Restarting in " + tos(gOptions.ypTimeout()) + " seconds.");
	}
	catch(...)
	{
		ELOG(m_logPrefix + "Fatal error. Cannot recover.");
		throw;
	}
}

// waiting for a request to be queued
void webClient::state_Idle() throw(exception)
{
#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(m_logPrefix + __FUNCTION__);
#endif

	stackLock sml(m_requestQueueLock);
	m_requestSignal.clear();
	if (m_requestQueue.empty())
	{
		m_result.schedule(1000);
		m_result.read(m_requestSignal.test());
		m_result.timeout(gOptions.ypTimeout());
	}
	else
	{
		m_result.run();
		m_state = &webClient::state_SendRequest;
	}
}

// TODO can we instead change this to be scheduled?
void webClient::state_Wait() throw(exception)
{
#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(m_logPrefix + __FUNCTION__);
#endif

	time_t t = ::time(NULL);
	stackLock sml(m_requestQueueLock);
	if (t < m_lastActivityTime) // rollover?
	{
		m_lastActivityTime = t;
	}

	if (m_state != m_nextState)
	{
		m_state = m_nextState;
		// this is needed to ensure that we'll not
		// block on some of the YP async requests.
		m_result.timeout(0, 10);
	}
	else
	{
		// we'll add this back to be tested for...
		m_result.schedule (50);
		m_result.timeout((gOptions.ypTimeout() - (int)(t - m_lastActivityTime)));
	}
}

size_t webClient::ParseReplyHeaders(void *buffer, size_t size, size_t count, FILE *stream) throw(runtime_error)
{
	webClient *client = reinterpret_cast<webClient*>(stream);
	if (!client)
	{
		throwEx<runtime_error>("ParseReplyHeaders parameter failure");
	}

	utf8 s = stripWhitespace(utf8((char*)buffer, (size * count)));
	if (!s.empty())
	{
		DEBUG_LOG(client->m_logPrefix + "HTTP header: " + s);

		if (client->m_response.m_resultCode == 0)	// waiting for first line or something other than HTTP/1.1 100 Continue
		{
			utf8::size_type pos = s.find(utf8(" "));
			if (pos == utf8::npos)
			{
				throwEx<runtime_error>(client->m_logPrefix + "Badly formed HTTP response [" + s + "]");
			}
			s = stripWhitespace(s.substr(pos));
			pos = s.find(utf8(" "));
			if (pos == utf8::npos)
			{
				throwEx<runtime_error>(client->m_logPrefix + "Badly formed HTTP response [" + s + "]");
			}
			client->m_response.m_resultCode = utf8(s.substr(0,pos)).toInt();
			if (client->m_response.m_resultCode != 100)
			{
				s = stripWhitespace(s.substr(pos));
				client->m_response.m_resultText = s;
			}
			else
			{
				client->m_response.m_resultCode = 0;
			}
		}
		else
		{
			// header lines
			utf8::size_type pos = s.find(utf8(":"));
			if (pos == utf8::npos)
			{
				throwEx<runtime_error>(client->m_logPrefix + "Badly formed HTTP header line [" + s + "]");
			}
			if ((int)client->m_response.m_headers.size() > gOptions.maxHeaderLineCount())
			{
				throwEx<runtime_error>(client->m_logPrefix + "Badly formed HTTP response. Max header lines exceeded.");
			}

			utf8 key = toLower(stripWhitespace(s.substr(0,pos)));
			utf8 value = stripWhitespace(s.substr(pos+1));
			// allow empty values. (for urls and what-not)
			if (key.empty())
			{
				throwEx<runtime_error>(client->m_logPrefix + "Bad HTTP header string [" + s + "]");
			}
			client->m_response.m_headers[key] = value;
		}
	}

	size_t received = (size * count);
	client->m_response.m_received += received;
	return received;
}

size_t webClient::GetBody(void *buffer, size_t size, size_t count, FILE *stream)
{
	webClient *client = reinterpret_cast<webClient*>(stream);
	size_t received = (size * count);
	unsigned char *arr = (unsigned char *)buffer;

	if (gOptions.webClientDebug())
	{
		// skip outputting the DNAS download updates
		DLOG(client->m_logPrefix + "HTTP body: " + arr);
	}
	client->m_requestQueueLock.lock();
	client->m_response.m_body.insert(client->m_response.m_body.end(), arr, &arr[received]);
	client->m_lastActivityTime = ::time(NULL);

	client->m_response.m_received += received;
	client->m_requestQueueLock.unlock();
	return received;
}

void webClient::gotCurlRespose()
{
	this->m_nextState = &webClient::state_RequestComplete;
	this->m_lastActivityTime = ::time(NULL);
}

// will still need to set certain aspects like the error buffer or any of the header / processing options
CURL* webClient::setupCurlDefaults(CURL* oldCurl, const char *logPrefix, const utf8& requestUrl,
        const int timeout, const int connnectTimeout, size_t SID)
{
	CURL *curl = (oldCurl == NULL ? curl_easy_init() : oldCurl);
	if (curl)
	{
        if (requestUrl != utf8(""))
        {
            if (oldCurl)
            {
                curl_easy_reset(curl);
                DEBUG_LOG ("Recycling curl handle for: " + requestUrl, logPrefix, SID);
            }
            else
            {
                DEBUG_LOG ("Creating new curl handle for: " + requestUrl, logPrefix, SID);
            }
            curl_easy_setopt(curl, CURLOPT_URL, requestUrl.hideAsString().c_str());
        }

		curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, g_userAgent.c_str());
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, (timeout == -1 ? gOptions.ypTimeout() : timeout));
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, (connnectTimeout == -1 ? 3L : connnectTimeout));
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, gOptions.maxHTTPRedirects());
		curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

/*#if defined(_DEBUG) || defined(DEBUG)
		if (gOptions.webClientDebug())
		{
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		}
#endif*/

		// this is needed (mainly for the Windows builds) so things will work with SSL and also
		// self-signed certificates as we appear to be using (as CURLOPT_SSL_VERIFYPEER = FALSE
		// is an absolute no-no due to the man in the middle attack which it allows to succeed.
		//DEBUG_LOG(logPrefix + "Certificate path: `" + gOptions.m_certPath + "'");
		if (!gOptions.m_certPath.empty())
		{
			curl_easy_setopt(curl, CURLOPT_CAINFO, gOptions.m_certPath.hideAsString().c_str());
		}
		else
		{
			WLOG ("Certificate path is invalid - cacert.pem location not known", logPrefix, SID);
		}
	}
	return curl;
}

// build the entire web request, and send it
void webClient::state_SendRequest() throw(exception)
{
#if defined(_DEBUG) || defined(DEBUG)
    DEBUG_LOG(m_logPrefix + __FUNCTION__);
#endif

    do
    {
        m_requestQueueLock.lock();

        if (m_requestQueue.empty())
        {
            // queue is empty, move to idle state
            m_requestSignal.clear();
            m_state = &webClient::state_Idle;
            m_requestQueueLock.unlock();
            break;
        }
        // construct request
        request &r = m_requestQueue.front(); // leave it in the queue until request succeeds
        struct curl_slist *header = m_curl_headers[r.m_sid];

        if (r.m_retryCounter < gOptions.ypMaxRetries())
        {
            m_curl_path.clear();
            m_curl_path = toLogString(r);

            if (r.m_method == webClient::request::GET && !r.m_queryVariables.empty())
            {
                m_curl_path += utf8("?") + encodeVariables(r.m_queryVariables);
            }

            // create or re-use the handle as needed (doing a reset to avoid some quirks with authhash vs general YP usage)
            CURL* curl = setupCurlDefaults(m_curl[r.m_sid], (char *)m_logPrefix.c_str(), m_curl_path);
            if (curl)
            {
                if (r.m_method == webClient::request::POST)
                {
                    utf8 contentType = "";
                    if (r.m_content.empty())
                    {
                        utf8 u8 = encodeVariables(r.m_queryVariables);
                        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, u8.size());
                        curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, &u8[0]);
                    }
                    else
                    {
                        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, r.m_content.size());
                        curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, &r.m_content[0]);
                    }
                    if (r.m_contentType.empty() == false)
                    {
                        string contentType = "Content-Type:" + r.m_contentType.hideAsString();
                        header = curl_slist_append (header, contentType.c_str());
                    }
                }
                else
                    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

                utf8 m_XFF = "";
                if (!r.m_XFF.empty())
                {
                    m_XFF = "X-Forwarded-For:" + r.m_XFF;
                    header = curl_slist_append(header, m_XFF.hideAsString().c_str());
                }

                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

                // send it, then get ready to receive the reply
                m_lastActivityTime = ::time(NULL);
                m_response.clear();
                r.m_content.clear();

                DEBUG_LOG(m_logPrefix + "Request URL: " + m_curl_path);
                curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &this->ParseReplyHeaders);
                curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &this->GetBody);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
                curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &(m_curl_error[0]));

                if (r.m_nonBlocking)
                {
                    m_nonBlockingID = r.m_sid;
                    m_curl_headers[r.m_sid] = header;
                    m_curl[r.m_sid] = curl;

                    m_state = m_nextState = &webClient::state_Wait;
                    m_requestQueueLock.unlock();
                    SimpleThread (webClient::process, this);
                }
                else
                {
                    m_requestQueueLock.unlock();
                    CURLcode ret = curl_easy_perform(curl);
                    m_requestQueueLock.lock();
                    if (ret != CURLE_OK)
                    {
                        getCurlError(curl, ret);
                    }
                    else
                    {
                        m_state = &webClient::state_RequestComplete;
                    }

                    if (header)
                    {
                        curl_slist_free_all(header);
                        m_curl_headers[r.m_sid] = NULL;
                    }
                    m_requestQueueLock.unlock();
                }
            }
            break;
        }
        ELOG(m_logPrefix + "Request [" + toLogString(r) + "] failed. Retries exceeded.");
        gotFailure(r);
        m_requestQueue.pop();
        m_requestQueueLock.unlock();
    } while (0);

    m_result.run();
}

void webClient::getCurlError(CURL* curl, const CURLcode ret) throw(exception)
{
	if (!m_requestQueue.empty())
	{
		webClient::request &r = m_requestQueue.front();
		r.m_received = 0;
		if (curl)
		{
			curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &r.m_received);
			curl_easy_cleanup(curl);
		}
		m_curl[r.m_sid] = NULL;

		ELOG(m_logPrefix + "Request [" + toLogString(r) + "] failed, code: " + tos(ret) +
			 " [" + (m_curl_error[0] ? m_curl_error : curl_easy_strerror(ret)) + "]");
		gotFailure(r);
		m_requestQueue.pop();

		m_lastActivityTime = ::time(NULL);
		m_state = m_nextState = &webClient::state_Idle;
	}
}

THREAD_FUNC webClient::process(void* arg)
{
	try
	{
		webClient* m_client = reinterpret_cast<webClient*>(arg);
		if (m_client)
		{
			if (!iskilled())
			{
				m_client->m_requestQueueLock.lock();

				CURL* curl = m_client->getCurlHandle();
				m_client->m_requestQueueLock.unlock();

				CURLcode ret = (curl ? curl_easy_perform(curl) : CURLE_READ_ERROR);
				if (!iskilled())
				{
					stackLock sml(m_client->m_requestQueueLock);

					if (ret != CURLE_OK)
					{
						m_client->getCurlError(curl, ret);
					}
					else
					{
						m_client->gotCurlRespose();
					}

					m_client->clearCurlHeader();
				}
			}
		}
	}
	catch (...)
	{
	}
	return 0;
} 

// need to let things run in the background so sit and spin and wait...
void webClient::state_Send() throw(std::exception)
{
#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(m_logPrefix + __FUNCTION__);
#endif

	SimpleThread(webClient::process, this);

	m_state = m_nextState = &webClient::state_Wait;
	m_result.run();
}

// request complete. Issue virtual callback, pop request, then move to idle
// state and wait for next request
void webClient::state_RequestComplete() throw(exception)
{
#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(m_logPrefix + __FUNCTION__);
#endif

	m_requestQueueLock.lock();
	if (!m_requestQueue.empty())
	{
			request &r = m_requestQueue.front();
			m_requestQueueLock.unlock();
			gotResponse(r, m_response); // could throw

			m_requestQueueLock.lock();
			m_requestQueue.pop();
	}

	m_lastActivityTime = ::time(NULL);
	m_state = m_nextState = &webClient::state_Idle;
	m_result.run();
	m_requestQueueLock.unlock();
}

// this method should be overridden. The base class here just dumps diagnostics	
// throw an exception if you want to treat the result as a retryable error
void webClient::gotResponse(const request &/*q*/, const response &r) throw(exception)
{
	DEBUG_LOG(m_logPrefix + toLogString(r));
}

void webClient::gotFailure(const request &q) throw(exception)
{
	DEBUG_LOG(m_logPrefix + toLogString(q));
}

const size_t webClient::queueEntries() throw()
{
#if defined(_DEBUG) || defined(DEBUG)
	DEBUG_LOG(m_logPrefix + __FUNCTION__);
#endif

	stackLock sml(m_requestQueueLock);

	return !m_requestQueue.empty();
}
