#pragma once
#include "http/svc_http_demuxer.h"
#include "http/ifc_http_demuxer.h"
#include "nx/nxstring.h"
#include "nswasabi/ServiceName.h"
#include "MP4FileObject.h"
#include "ifc_mp4audiodecoder.h"

// {C67A19EF-CBDB-4BD9-9B4F-EEC5D6B05093}
static const GUID mp4_demuxer_guid  = 
{ 0xc67a19ef, 0xcbdb, 0x4bd9, { 0x9b, 0x4f, 0xee, 0xc5, 0xd6, 0xb0, 0x50, 0x93 } };


class MP4HTTPService : public svc_http_demuxer
{
public:	
	WASABI_SERVICE_NAME("MP4 HTTP Demuxer");
	static GUID GetServiceGUID() { return mp4_demuxer_guid; }

	const char *WASABICALL HTTPDemuxerService_EnumerateAcceptedTypes(size_t i);
	const char *WASABICALL HTTPDemuxerService_GetUserAgent();
	void WASABICALL HTTPDemuxerService_CustomizeHTTP(jnl_http_t http);
	NError WASABICALL HTTPDemuxerService_CreateDemuxer(nx_uri_t uri, jnl_http_t http, ifc_http_demuxer **demuxer, int pass);
};

class MP4HTTP : public ifc_http_demuxer
{
public:
	MP4HTTP();
	~MP4HTTP();

	int Initialize(nx_uri_t uri, jnl_http_t http);
private:
	/* ifc_http_demuxer implementation */
	int WASABICALL HTTPDemuxer_Run(ifc_http *http_parent, ifc_player *player, ifc_playback_parameters *secondary_parameters);

	/* member data */
	jnl_http_t http;
	nx_uri_t uri;
	nx_file_t file;
	MP4FileObject *mp4_file_object;
	ifc_audioout::Parameters audio_parameters;
	ifc_mp4audiodecoder *audio_decoder;
};
