#include "http/api.h"
#include "HTTPPlayback.h"
#include "http/svc_http_demuxer.h"
#include "service/ifc_servicefactory.h"
#include <time.h>
#ifdef _WIN32
#include "nu/AutoChar.h"
#endif
#include "nu/strsafe.h"
#include "nx/nxsleep.h"
#ifdef __ANDROID__
#include <android/log.h> // TODO: replace with generic logging API
#else
#define ANDROID_LOG_INFO 0
#define ANDROID_LOG_ERROR 1
void __android_log_print(int, const char *, const char *, ...)
{
}
#endif
#include <time.h>

HTTPPlayback::HTTPPlayback()
{
	http=0;
	demuxer=0;
}

int HTTPPlayback::Initialize(nx_uri_t url, ifc_player *player)
{
	int ret = PlaybackBase::Initialize(url, player);
	if (ret != NErr_Success)
		return ret;
	http=0;
	demuxer=0;
	ifc_playback::Retain(); /* the thread needs to hold a reference to this object so that it doesn't disappear out from under us */
	NXThreadCreate(&playback_thread, HTTPPlayerThreadFunction, this);
	return NErr_Success;
}

HTTPPlayback::~HTTPPlayback()
{
	if (demuxer)
		demuxer->Release();
	if (http)
		jnl_http_release(http);
}

nx_thread_return_t HTTPPlayback::HTTPPlayerThreadFunction(nx_thread_parameter_t param)
{
	HTTPPlayback *playback = (HTTPPlayback *)param;
	NXThreadCurrentSetPriority(NX_THREAD_PRIORITY_PLAYBACK);
	nx_thread_return_t ret = playback->DecodeLoop();
	playback->ifc_playback::Release();
	return ret;
}

int HTTPPlayback::Init()
{
	http = jnl_http_create(2*1024*1024, 0);
	if (!http)
		return NErr_OutOfMemory;
	
	return NErr_Success;
}

static void SetupHTTP(jnl_http_t http)
{
	char accept[1024], user_agent[256];
	accept[0]=0;
	user_agent[0]=0;
	size_t accept_length=sizeof(accept)/sizeof(*accept);
	size_t user_agent_length=sizeof(user_agent)/sizeof(*user_agent);
	char *p_accept = accept, *p_user_agent=user_agent;

	const char *application_user_agent = WASABI2_API_APP->GetUserAgent();
	StringCchCopyExA(p_user_agent, user_agent_length, application_user_agent, &p_user_agent, &user_agent_length, 0);

	GUID http_demuxer_guid = svc_http_demuxer::GetServiceType();
	ifc_serviceFactory *sf;
	size_t n = 0;
	while (sf = WASABI2_API_SVC->EnumService(http_demuxer_guid, n++))
	{
		svc_http_demuxer *l = (svc_http_demuxer*)sf->GetInterface();
		if (l)
		{
			const char *this_accept;
			size_t i=0;
			while (this_accept=l->EnumerateAcceptedTypes(i++))
			{
				if (accept == p_accept) // first one added
					StringCchCopyExA(p_accept, accept_length, this_accept, &p_accept, &accept_length, 0);
				else
					StringCchPrintfExA(p_accept, accept_length, &p_accept, &accept_length, 0, ", %s", this_accept);
			}

			const char *this_user_agent = l->GetUserAgent();
			if (this_user_agent)
			{
				StringCchPrintfExA(p_user_agent, user_agent_length, &p_user_agent, &user_agent_length, 0, " %s", this_user_agent);
			}

			l->CustomizeHTTP(http);
			l->Release();
		}
	}
	if (accept != p_accept)
		jnl_http_addheadervalue(http, "Accept", accept);
	jnl_http_addheadervalue(http, "User-Agent", user_agent);
	jnl_http_addheadervalue(http, "Connection", "close");

}

static NError FindDemuxer(nx_uri_t uri, jnl_http_t http, ifc_http_demuxer **demuxer)
{
	GUID http_demuxer_guid = svc_http_demuxer::GetServiceType();
	ifc_serviceFactory *sf;

	bool again;
	int pass=0;
	do
	{
		size_t n = 0;
		again=false;
		while (sf = WASABI2_API_SVC->EnumService(http_demuxer_guid, n++))
		{
			svc_http_demuxer *l = (svc_http_demuxer*)sf->GetInterface();
			if (l)
			{
				NError err = l->CreateDemuxer(uri, http, demuxer, pass);
				if (err == NErr_Success)
					return NErr_Success;

				if (err == NErr_TryAgain)
					again=true;
			}
		}
		pass++;
	} while (again);
	return NErr_NoMatchingImplementation;
}

int HTTPPlayback::Internal_Connect(uint64_t byte_position)
{
	int http_ver = byte_position?1:0;

	if (byte_position != 0)
	{
		char str[512];
		StringCchPrintfA(str, 512, "Range: bytes=%llu-", byte_position);

		jnl_http_addheader(http, str);
	}
	//jnl_http_allow_accept_all_reply_codes(http);
#ifdef _WIN32
	jnl_http_connect(http, AutoChar(filename->string), http_ver, "GET");
#else
	jnl_http_connect(http, filename->string, http_ver, "GET");
#endif

	/* wait for connection */
	time_t start_time = time(0);

	int http_status;
	do
	{
		int ret =  PlaybackBase::Sleep(10, PlaybackBase::WAKE_STOP); 
		if (ret == PlaybackBase::WAKE_STOP)
			return NErr_Interrupted;

		ret = jnl_http_run(http);
		if (ret == HTTPGET_RUN_ERROR)
			return NErr_ConnectionFailed;
		if (start_time + 15 < time(0))
			return NErr_TimedOut;

		http_status = jnl_http_get_status(http);
	} while (http_status == HTTPGET_STATUS_CONNECTING || http_status == HTTPGET_STATUS_READING_HEADERS);

	if (http_status == HTTPGET_STATUS_ERROR)
	{
		switch(jnl_http_getreplycode(http))
		{
		case 400:
			return NErr_BadRequest;
		case 401:
			// TODO: deal with this specially
			return NErr_Unauthorized;
		case 403:
			// TODO: deal with this specially?
			return NErr_Forbidden;
		case 404:
			return NErr_NotFound;
		case 405:
			return NErr_BadMethod;
		case 406:
			return NErr_NotAcceptable;
		case 407:
			// TODO: deal with this specially
			return NErr_ProxyAuthenticationRequired;
		case 408:
			return NErr_RequestTimeout;
		case 409:
			return NErr_Conflict;
		case 410:
			return NErr_Gone;
		case 500:
			return NErr_InternalServerError;
		case 503:
			return NErr_ServiceUnavailable;

		default:
					return NErr_ConnectionFailed;

			}
	}
	return NErr_Success;
}

nx_thread_return_t HTTPPlayback::DecodeLoop()
{
	player->OnLoaded(filename);

	int ret = Init();
	if (ret != NErr_Success)
	{
		player->OnError(ret);
		return 0;
	}

	SetupHTTP(http);

	/* connect, then find an ifc_http_demuxer */
	ret = Internal_Connect(0);

	if (ret == NErr_Success && FindDemuxer(filename, http, &demuxer) == NErr_Success && demuxer)
	{
		/* turn control over to the demuxer */
		ret = demuxer->Run(this, player, secondary_parameters);
		if (ret == NErr_EndOfFile)
		{
			/* TODO: re-implement the individual demuxers so they keep calling set position for a while */
			player->OnClosed();
			return 0;
		}
	}
	else if (ret == NErr_Interrupted)
	{
		player->OnStopped();
		return 0;
	}
	else if (ret == NErr_TimedOut)
	{
		player->OnError(ret);
		return 0;
	}
	else if (ret == NErr_Success)
	{
		player->OnError(NErr_NoMatchingImplementation);
		return 0;
	}
	else
	{
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[http] error: %d, reply code: %d", ret, jnl_http_getreplycode(http));
		player->OnError(ret);
		return 0;
	}

	return 0;
}

int HTTPPlayback::HTTP_Wake(int mask)
{
	return PlaybackBase::Wake(mask);
}

int HTTPPlayback::HTTP_Check(int mask)
{
	return PlaybackBase::Check(mask);
}

int HTTPPlayback::HTTP_Wait(unsigned int milliseconds, int mask)
{
	return PlaybackBase::Wait(milliseconds, mask);
}

int HTTPPlayback::HTTP_Sleep(int milliseconds, int mask)
{
	return PlaybackBase::Sleep(milliseconds, mask);
}

Agave_Seek *HTTPPlayback::HTTP_GetSeek()
{
	return PlaybackBase::GetSeek();
}

void HTTPPlayback::HTTP_FreeSeek(Agave_Seek *seek)
{
	PlaybackBase::FreeSeek(seek);
}

int HTTPPlayback::HTTP_Seek(uint64_t byte_position)
{
	jnl_http_reset_headers(http);
	SetupHTTP(http);
	return Internal_Connect(byte_position);
}
#if defined(_WIN32) && !defined(strcasecmp)
#define strcasecmp _stricmp
#endif

int HTTPPlayback::HTTP_Seekable()
{
	const char *accept_ranges = jnl_http_getheader(http, "accept-ranges");
	if (accept_ranges && !strcasecmp(accept_ranges, "none"))
			return NErr_False; /* server says it doesn't accept ranges */
	
	/* note that not having an accept-ranges header doesn't necessary mean it's not seekable.  see RFC2616 14.5 */
	return NErr_True;
}

int HTTPPlayback::HTTP_AudioOpen(const ifc_audioout::Parameters *format, ifc_audioout **out_output)
{
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[http] output_service=%x", output_service);
	return output_service->AudioOpen(format, player, secondary_parameters, out_output);
}
