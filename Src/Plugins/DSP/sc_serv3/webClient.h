#pragma once
#ifndef webClient_H_
#define webClient_H_

#include "threadedRunner.h"
#include "curl/curl.h"
#include <stdio.h>

// runnable that provides generic web client functionality to a single host
// you can subclass this to provide specific behaviour
#pragma pack(push, 1)
class webClient: public runnable
{
public:
	struct request
	{
		typedef enum { GET,POST } method_t;

		uniString::utf8 m_addr;
		uniString::utf8 m_path;

		// content
		std::vector<__uint8> m_content;
		uniString::utf8 m_contentType;

		// used for the setting the 'X-Forwarded-For' header field which
		// is a bit of a fiddle to allow destip to be different so the YP
		// can test an alternative external IP from that it auto detects
		uniString::utf8 m_XFF;

		httpHeaderMap_t	m_queryVariables;
		method_t m_method;

		int m_port;
		size_t m_sid;
		int m_retryCounter;

		int m_userData_i;

		size_t m_received;	// size of data got
		int m_nonBlocking;	// do on an additional thread e.g. the YP add needs
							// this else it blocks the YP test exists connetion

		// two random pieces of user data
		unsigned m_userData_u;
		void* m_userData_p;

		request() : m_path(uniString::utf8("/")), m_method(GET), m_port(80),
					m_sid(0), m_retryCounter(0), m_userData_i(0), m_received(0),
					m_nonBlocking(0), m_userData_u(0), m_userData_p(0) {}
	};

	struct response
	{
		uniString::utf8	m_resultText;
		uniString::utf8	m_body;		// body of response
		httpHeaderMap_t	m_headers;	// response headers
		size_t m_resultCode;		// HTTP result code
		size_t m_received;			// size of data got

		response() : m_resultCode(0), m_received(0) {}

		void clear() throw()
		{
			m_resultCode = 0;
			m_received = 0;
			m_headers.clear();
			m_body.clear();
			m_resultText.clear();
		}
	};

	static size_t ParseReplyHeaders(void *buffer, size_t size, size_t count, FILE *stream) throw(std::runtime_error);
	static size_t GetBody(void *buffer, size_t size, size_t count, FILE *stream);

	CURL* getCurlHandle(const size_t SID = 0) { return m_curl[(!SID ? m_nonBlockingID : SID)]; }
	void clearCurlHeader(const size_t SID = 0)
	{
		size_t id = (!SID ? m_nonBlockingID : SID);
		struct curl_slist *header = m_curl_headers[id];
		if (header)
		{
			curl_slist_free_all(header);
			m_curl_headers[id] = 0;
		}
	}
	void gotCurlRespose();
	void getCurlError(CURL* curl, const CURLcode ret) throw(std::exception);

	static CURL* setupCurlDefaults(CURL* oldCurl, const char *logPrefix, const uniString::utf8& requestUrl,
								   const int timeout = -1, const int connnectTimeout = -1, size_t SID = 0);

	static THREAD_FUNC process(void* arg);

protected:
	class retry_exception : public std::runtime_error
	{
	public:
		explicit retry_exception(const uniString::utf8 &msg) : runtime_error(msg.hideAsString()){}
	};

private:
	typedef void (webClient::*state_t)();
	const uniString::utf8	m_logPrefix;

	state_t	m_state;
	state_t m_nextState;

	// curl specific parts
	typedef std::map<size_t, CURL*> curlMap_t;
	curlMap_t m_curl;

	size_t m_nonBlockingID;
	uniString::utf8 m_curl_path;

	typedef std::map<size_t, struct curl_slist*> curlHeadersMap_t;
	curlHeadersMap_t m_curl_headers;

	response	m_response;
	char*		m_curl_error;

	AOL_namespace::mutex		m_requestQueueLock;
	std::queue<request>			m_requestQueue;
	pipeDrivenSignal<nullLock>	m_requestSignal;

	void state_Idle() throw(std::exception);
	void state_Wait() throw(std::exception);
	void state_ResolveServer() throw(std::exception);
	void state_Connect() throw(std::exception);
	void state_ConnectWait() throw(std::exception);
	void state_Send() throw(std::exception);
	void state_Get() throw(std::exception);
	void state_SendRequest() throw(std::exception);
	void state_RequestComplete() throw(std::exception);

	virtual void timeSlice() throw(std::exception);

protected:
	explicit webClient(const uniString::utf8 &logPrefix) throw();
	virtual ~webClient() throw();

	void queueRequest(const request &req) throw();
	// throw to treat as an error and retry entire request
	virtual void gotResponse(const request &q, const response &r) throw(std::exception); // called when response is received
	virtual	void gotFailure(const request &q) throw(std::exception); // called when retries are exhausted

	const size_t queueEntries() throw();
};
#pragma pack(pop)

uniString::utf8 encodeVariables(const httpHeaderMap_t &m) throw();

#endif
