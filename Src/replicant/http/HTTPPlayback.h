#pragma once
#include "nx/nxonce.h"
#include "jnetlib/jnetlib.h"
#include "http/ifc_http_demuxer.h"
#include "http/ifc_http.h"
#include "nswasabi/PlaybackBase.h"

class HTTPPlayback : public PlaybackBase, public ifc_http
{
public:
	HTTPPlayback();
	~HTTPPlayback();

	int Initialize(nx_uri_t url, ifc_player *player);
	/* ifc_http implementation */
	int WASABICALL HTTP_Wake(int mask);
	int WASABICALL HTTP_Check(int mask);
	int WASABICALL HTTP_Wait(unsigned int milliseconds, int mask);
	int WASABICALL HTTP_Sleep(int milliseconds, int mask);
	Agave_Seek *WASABICALL HTTP_GetSeek();
	void WASABICALL HTTP_FreeSeek(Agave_Seek *seek);
	int WASABICALL HTTP_Seek(uint64_t byte_position);
	int WASABICALL HTTP_Seekable();
	int WASABICALL HTTP_AudioOpen(const ifc_audioout::Parameters *format, ifc_audioout **out_output);
private:
	int Internal_Connect(uint64_t byte_position);
	ifc_http_demuxer *demuxer;
		
	volatile int paused;
	volatile int stopped;
	jnl_http_t http;

	int Init();

	nx_thread_return_t NXTHREADCALL DecodeLoop();
	static nx_thread_return_t NXTHREADCALL HTTPPlayerThreadFunction(nx_thread_parameter_t param);
};
