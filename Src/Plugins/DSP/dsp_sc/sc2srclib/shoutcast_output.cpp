#include <windows.h>
#include <Ws2tcpip.h>
#include "../api.h"
#include "Include/shoutcast_output.h"
#include "../../sc_serv3/nmrCommon/stl/stringUtils.h"
#include "../utils.h"
#include "../nu/ServiceBuilder.h"
#include <winamp/dsp.h>

#pragma intrinsic(memcpy, memset)

static char buf[1024];
static char out[1024];

extern int iscompatibility;
extern winampDSPModule module;
extern api_service *WASABI_API_SVC;

static api_connection *CreateConnection(const char *url)
{
	api_connection *conn = 0;
	if (WASABI_API_SVC/*module.service*/)
	{
		if (!_strnicmp(url, "https://", 8))
		{
			ServiceBuild(WASABI_API_SVC/*module.service*/, conn, sslConnectionFactoryGUID);
		}
		else
		{
			ServiceBuild(WASABI_API_SVC/*module.service*/, conn, connectionFactoryGUID);
		}
	}

	return conn;
}

static void ReleaseConnection(api_connection *&conn, const char *url)
{
	if (!conn) {
		return;
	}

	waServiceFactory *connectionFactory = 0;
	if (WASABI_API_SVC/*mod.service*/)
	{
		if (!_strnicmp(url, "https://", 8)) {
			connectionFactory = WASABI_API_SVC/*mod.service*/->service_getServiceByGuid(sslConnectionFactoryGUID);
		} else {
			connectionFactory = WASABI_API_SVC/*mod.service*/->service_getServiceByGuid(connectionFactoryGUID);
		}
	}

	if (connectionFactory) {
		connectionFactory->releaseInterface(conn);
	}

	conn = 0;
}

int resolvenow(const char *hostname, unsigned short port, addrinfo **addr, int sockettype, char **saddress)
{
	addrinfo hints;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	if (hostname)
		hints.ai_flags = AI_NUMERICHOST;
	else
		hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
	hints.ai_socktype = sockettype;

	char portString[32] = {0};
	sprintf(portString, "%u", (unsigned int)port);

	if (getaddrinfo(hostname, portString, &hints, addr) == 0)
	{
		return 0;
	}
	else 
	{
		hints.ai_flags = 0;
		if (getaddrinfo(hostname, portString, &hints, addr) == 0)
		{
			// need to make sure we're not using 0.0.0.0
			// as that will break us when using localhost
			bool found = false;
			struct addrinfo *ptr = NULL;
			for (ptr = *addr; ptr != NULL; ptr = ptr->ai_next)
			{
				char* address = ::inet_ntoa(((sockaddr_in*)(ptr->ai_addr))->sin_addr);
				if (strcmp(address, "0.0.0.0"))
				{
					*saddress = address;
					*addr = ptr;
					return 0;
				}
			}
		}
	}
	return -1;
}

void compatible_connect(T_OUTPUT *Output, int port) {

	// due to a bug in re-using the connection
	// with Winamp's jnetlib implementation we
	// need to do a force drop and make a new
	// instance so all is initialised properly
	if (Output->Output && iscompatibility) {
		Output->Output->Close();
		Output->Output->Release();
		Output->Output = NULL;
	}

	// create the connection only when it's needed
	if (!Output->Output) {
		Output->Output = CreateConnection("");
		if (Output->Output) {
			Output->Output->Open(API_DNS_AUTODNS, 16384, 16384);
		}
	}

	if (Output->Output) {
		if (iscompatibility && !strnicmp(Output->Config->Address, "localhost", 9)) {
			addrinfo *saddr = 0;
			char *address = 0;
			if (!resolvenow(Output->Config->Address, (Output->Config->Port) + (LOBYTE(Output->Config->protocol) == 1), &saddr, SOCK_STREAM, &address) && address) {
				Output->Output->connect(address, (Output->Config->Port) + (LOBYTE(Output->Config->protocol) == 1));
			} else {
				Output->Output->connect(Output->Config->Address, (Output->Config->Port) + (LOBYTE(Output->Config->protocol) == 1));
			}
		} else {
			Output->Output->connect(Output->Config->Address, port);
		}
	}
}

SHOUTCAST_OUTPUT::SHOUTCAST_OUTPUT() : lame_init(0), lame_init_params(0), lame_encode_buffer_interleaved(0), lame_encode_flush(0), libinst(NULL) {
	IOwnEncoder = 0;
	SetEncoder(DEFAULT_ENCODER);
	OutputManager.AddHandler(OUT_DISCONNECTED, Output_Disconnected);
	OutputManager.AddHandler(OUT_CONNECT, Output_Connect);
	OutputManager.AddHandler(OUT_REQUEST_CIPHER, Output_Request_Cipher);//cipher request
	OutputManager.AddHandler(OUT_RECV_CIPHER, Output_Receive_Cipher);// and process cipher
	OutputManager.AddHandler(OUT_SENDAUTH, Output_SendAuth);
	OutputManager.AddHandler(OUT_RECVAUTHRESPONSE, Output_RecvAuthResponse);
	OutputManager.AddHandler(OUT_SEND_MIME, Output_Send_Mime);
	OutputManager.AddHandler(OUT_RECV_MIME, Output_Recv_Mime);
	OutputManager.AddHandler(OUT_SEND_BITRATE, Output_Send_Bitrate);
	OutputManager.AddHandler(OUT_RECV_BITRATE, Output_Recv_Bitrate);
	OutputManager.AddHandler(OUT_SEND_BUFSIZE, Output_Send_Buf_Size);
	OutputManager.AddHandler(OUT_RECV_BUFSIZE, Output_Recv_Buf_Size);
	OutputManager.AddHandler(OUT_SEND_MAX, Output_Send_Max_Size);
	OutputManager.AddHandler(OUT_RECV_MAX, Output_Recv_Max_Size);
	OutputManager.AddHandler(OUT_SENDYP, Output_SendYP);
	OutputManager.AddHandler(OUT_SEND_INITFLUSH, Output_Send_InitFlush);
	OutputManager.AddHandler(OUT_RECV_INITFLUSH, Output_Recv_InitFlush);
	OutputManager.AddHandler(OUT_SEND_INITSTANDBY, Output_Send_InitStandby);
	OutputManager.AddHandler(OUT_RECV_INITSTANDBY, Output_Recv_InitStandby);
	//OutputManager.AddHandler(OUT_SEND_INTRO, Output_Send_Intro);
	//OutputManager.AddHandler(OUT_RECV_INTRO, Output_Recv_Intro);
	//OutputManager.AddHandler(OUT_SEND_BACKUP, Output_Send_Backup);
	//OutputManager.AddHandler(OUT_RECV_BACKUP, Output_Recv_Backup);
	OutputManager.AddHandler(OUT_SENDCONTENT, Output_SendContent);
	OutputManager.AddHandler(OUT_DISCONNECT, Output_Disconnect);
	OutputManager.AddHandler(OUT_RECONNECT, Output_Reconnect);
	OutputManager.AddHandler(OUT_TITLESENDUPDATE, Output_Title_SendUpdate);
	OutputManager.AddHandler(OUT_SEND_METADATA, Output_Send_Metadata);
	OutputManager.AddHandler(OUT_SEND_ARTWORK, Output_Send_Artwork);
	ReleaseMutex((mutex = CreateMutex(NULL, TRUE, NULL)));
}

void SHOUTCAST_OUTPUT::SetLame(void *init, void *params, void *encode, void *finish) {
	LOCK{
	lame_init = (void (__cdecl *)(void))init;
	lame_init_params = (void (__cdecl *)(lame_global_flags *))params;
	lame_encode_buffer_interleaved = (int (__cdecl *)(lame_global_flags *, short int pcm[], int num_samples, char *mp3buffer, int mp3buffer_size))encode;
	lame_encode_flush = (int (__cdecl *)(lame_global_flags *, char *mp3buffer, int size))finish;
	}UNLOCK
}

SHOUTCAST_OUTPUT::~SHOUTCAST_OUTPUT() {
	LOCK
	CloseHandle(mutex);
}

/*
Create a Uvox frame
*/

void SHOUTCAST_OUTPUT::createUvoxFrame(int length, char *payload_in, int type, T_OUTPUT *userData) {
	int len = min(length, UV_MAX_DATA_LEN);
	uv2xHdr hdr2 = { UV_SYNC_BYTE, UV_RESERVED, htons(type), htons(len), {UV_END}, UV_END };
	if (payload_in && len > 0 && len <= UV_MAX_DATA_LEN) {
		memcpy(hdr2.m_data, payload_in, len);
	}
	int sent = UV_HEADER_LEN + len + UV_END_LEN;
	userData->Output->send(&hdr2, sent);
	userData->Info.BytesSent += sent;
}

int SHOUTCAST_OUTPUT::createUvoxMetaFrame(int length, char *payload_in, int type,
										  T_OUTPUT *userData, unsigned short id,
										  unsigned short span) {
	// check for the class type and abort if it's not right - will typically be 0x0000 or 0xXXFF
	if ((type >= 0x3000) && (type < 0x5000)) {
		int artType = ((type & 0xFF00) == (MSG_METADATA_ALBUMART|MSG_METADATA_PLAYING_ART));
		int len = min(length, UV_MAX_META_LEN);
		uv2xMetadataHdr metaHdr = { UV_SYNC_BYTE, UV_RESERVED, htons(type), htons(len + UV_META_LEN),
									htons(id), htons(span), htons((span > 1 ? userData->Info.art_index[artType] : 0x1)),
									{UV_END}, UV_END };

		if (payload_in && len > 0 && len <= UV_MAX_META_LEN) {
			memcpy(metaHdr.m_data, payload_in, len);
		}

		int sent = UV_HEADER_LEN + len + UV_META_LEN + UV_END_LEN;
		if(userData->Output->send(&metaHdr, sent) != -1)
		{
			userData->Info.BytesSent += sent;
		}
		else
		{
			// if the send buffer is still full then we need to nudge
			// it all along so that we can send the metadata as not
			// checking for this will cause the loss of meta frames
			while(userData->Output->GetSendBytesAvailable())
			{
				Sleep(10); // sleep a bit and try again
				userData->Output->run();
				if(userData->Output->send(&metaHdr, sent) != -1)
				{
					userData->Info.BytesSent += sent;
					break;
				}
			}
		}
		return id+1;
	}
	return id;
}

/*
Parse a Uvox Frame
*/
int SHOUTCAST_OUTPUT::parseUvoxFrame(char *payload_in, char *payload_out) {
	LOCK{
	uv2xHdr *hdr2 = (uv2xHdr *)payload_in;
	int len = min(ntohs(hdr2->msgLen), UV_MAX_DATA_LEN);
	memmove(payload_out, hdr2->m_data, len);
	// just doing this to ensure its terminated correctly
	hdr2->m_data[len] = UV_END;
	}UNLOCK
	return 1;
}

/*
Check for known Uvox Error responses
*/
int SHOUTCAST_OUTPUT::checkUvoxFrameForError(char *pload_out, int state, T_OUTPUT *userData) {
	LOCK{
	if (userData->Info.Succeeded == -2 &&
		userData->Info.ErrorMsg) {
		free(userData->Info.ErrorMsg);
	}
	userData->Info.Succeeded = 1;
	userData->Info.ErrorMsg = 0;

	if (!strcmp("NAK:2.1:Deny", pload_out) || !strcmp("NAK:Deny", pload_out)) {
		userData->Info.Succeeded = -1;
		userData->Info.ErrorMsg = "NAK:Deny";
		UNLOCK
		return userData->Info.Succeeded;
	}

	if (!strcmp("NAK:2.1:Stream ID Error", pload_out)) {
		userData->Info.Succeeded = -1;
		userData->Info.ErrorMsg = "StreamID";
		UNLOCK
		return userData->Info.Succeeded;
	}

	if (!strcmp("NAK:2.1:Stream Moved", pload_out)) {
		userData->Info.Succeeded = -1;
		userData->Info.ErrorMsg = "StreamMoved";
		UNLOCK
		return userData->Info.Succeeded;
	}

	if (!strcmp("NAK:Bit Rate Error", pload_out)) {
		userData->Info.Succeeded = -1;
		userData->Info.ErrorMsg = "BitrateError";
		UNLOCK
		return userData->Info.Succeeded;
	}

	if (!strcmp("NAK:2.1:Version Error", pload_out)) {
		userData->Info.Succeeded = -1;
		userData->Info.ErrorMsg = "VersionError";
		UNLOCK
		return userData->Info.Succeeded;
	}

	if (!strcmp("NAK:2.1:Parse Error", pload_out) || !strcmp("NAK:Parse Error", pload_out)) {
		userData->Info.Succeeded = -1;
		userData->Info.ErrorMsg = "ParseError";
		UNLOCK
		return userData->Info.Succeeded;
	}

	if (!strcmp("NAK:Stream In Use", pload_out)) {
		userData->Info.Succeeded = -1;
		userData->Info.ErrorMsg = "InUse";
		UNLOCK
		return userData->Info.Succeeded;
	}

	if (!strcmp("NAK", pload_out)) {
		userData->Info.Succeeded = -2;
		// TODO
		userData->Info.ErrorMsg = _strdup(pload_out);
		UNLOCK
		return userData->Info.Succeeded;
	}

	if (!strcmp("ACK", pload_out)) {
		userData->Info.Succeeded = 1;
		UNLOCK
		return userData->Info.Succeeded;
	}

	}UNLOCK
	return userData->Info.Succeeded;
}

int SHOUTCAST_OUTPUT::Run(int mode, void *Input, int InputSize, int SaveEncoder) {
	int in_used = 0;
	LOCK{
	int bad = 1;

	if (mode & OM_OTHER) OutputManager.Run(OutputManager.GetNumJobs());

	for (int i = OutputManager.GetNumJobs()-1; i >= 0; i--) {
		T_OUTPUT *Output = OutputManager[i];
		if (mode & OM_OUTPUT && Output->Output) {
			Output->Output->run();
		}

		int state = OutputManager.GetJobState(i);
		if (state != OUT_DISCONNECTED) {
			int outstate = (Output->Output ? Output->Output->get_state() : CONNECTION_STATE_ERROR);
			if ((outstate == CONNECTION_STATE_ERROR || outstate == CONNECTION_STATE_CLOSED) &&
				state != OUT_DISCONNECT && state != OUT_RECONNECT) {
				if (Output->Type == OUTTYPE_SOURCE) {
					Output->Info.ConnectionTime = clock();
					Output->SlowClose = 0;
					OutputManager.SetJobState(i, state = OUT_DISCONNECT); // kill the connection
				} else if (Output->Type == OUTTYPE_TITLE) {
					delete OutputManager[i];
					OutputManager.DelJob(i); // title update failed, remove it
				}
			}
			if (state == OUT_SENDCONTENT) bad = 0;
		} else {
			if (Output->Type == OUTTYPE_TITLE) {
				#ifndef _DEBUG
				delete OutputManager[i];
				#endif
				OutputManager.DelJob(i);  // title update succeeded, remove it
			}
		}
	}// for num jobs
	if (!bad && Input && (InputSize > 0) && Encoder && (mode & OM_ENCODE)) {
		while (in_used < InputSize && InputSize > 0) {
			int inused = 0;
			char EncodedData[32768] = {0};
            int out_used = Encoder->Encode(((char *)Input+in_used), (InputSize-in_used), EncodedData, sizeof(EncodedData), &inused);
			in_used += inused;

			for (int i = OutputManager.GetNumJobs()-1; i >= 0; i--) {
				T_OUTPUT *Output = OutputManager[i];
				int state = OutputManager.GetJobState(i);
				if (out_used > 0 && state == OUT_SENDCONTENT) {
					int sofar = 0, type = 0;

					// when enabled this will save the encoded output to a local file for DJ backup, etc
					if (SaveEncoder != -1) {
						WriteSaveEncoded(SaveEncoder, EncodedData, out_used);
					}

					if (strcmp("audio/aacp", Output->ContentType)==0) { type = AACP_DATA; }	// aacp
					if (strcmp("audio/aac", Output->ContentType)==0) { type = AAC_LC_DATA; }	// aac lc
					if (strcmp("audio/mpeg", Output->ContentType)==0) { type = MP3_DATA; }	// mp3
					if (strcmp("audio/ogg", Output->ContentType)==0) { type = OGG_DATA; }	// ogg

					// make sure that this doesn't go negative otherwise things will go wrong
					while (out_used > 0) {
						int delta = Output->Output->GetSendBytesAvailable();

						if (LOBYTE(Output->Config->protocol) != 1) {
							// try to clamp things so that it's within the packet size limits
							delta = min(delta, UV_MAX_DATA_LEN);
						}

						delta = min(delta, out_used);
						if (delta > 7) {
							if (LOBYTE(Output->Config->protocol) != 1) {
								// send sc2 data frames
					            createUvoxFrame(delta, EncodedData+sofar, type, Output);
							} else {
								// send sc1 data
								Output->Info.BytesSent += delta;
								Output->Output->send(EncodedData+sofar, delta);
							}
						} else {
							// check for the connection having dropped and we're just spinning
							// and if we are then we need to abort from here else we lock up
							int outstate = Output->Output->get_state();
							if (outstate == CONNECTION_STATE_CLOSED || outstate == CONNECTION_STATE_ERROR) {
								break;
							}
							Sleep(10); // sleep a bit and try again
							Output->Output->run();
						}
						sofar+=delta;
						out_used-=delta;
					}
				}
			}
			if (InputSize <= 0) break;
		}
	}
	}UNLOCK
	return in_used;
}

int SHOUTCAST_OUTPUT::AddOutput(int Connection, T_OUTPUT_CONFIG *Config, void (*TitleCallback)(const int Connection, const int Mode)) {
	if (Connection >= 0 && Connection < 5) {
		T_OUTPUT *job = new T_OUTPUT;
		job->Bitrate = 0;
		job->ContentType = "audio/mpeg";
		if (Encoder) {
			int infosize = sizeof(T_EncoderIOVals);
			T_EncoderIOVals *EncSettings = (T_EncoderIOVals *)Encoder->GetExtInfo(&infosize);
			if (EncSettings && infosize) job->Bitrate = EncSettings->output_bitRate;
			job->ContentType = Encoder->GetContentType();
		}
		job->Connection = Connection;
		job->TitleCallback = TitleCallback;
		job->Config = Config;
		job->Info.Reconnect = Config->AutoRecon;
		job->Info.ReconnectTime = Config->ReconTime;
		job->Type = OUTTYPE_SOURCE;
		job->SlowClose = 0;
		// trigger a title update on start as artwork
		// is done later so does not need to set here
		job->Info.meta_cached = 1;

		int retval = -1;
		LOCK{
		retval = OutputManager.AddJob(OUT_DISCONNECTED, job);
		}UNLOCK
		return retval;
	}
	return -1;
}

void SHOUTCAST_OUTPUT::UpdateOutput(int Connection) {
	if (Connection >= 0 && Connection < 5) {
		LOCK{
		T_OUTPUT *Output = OutputManager[Connection];
		if (Output) {
			Output->Info.Reconnect = Output->Config->AutoRecon;
			Output->Info.ReconnectTime = Output->Config->ReconTime;
		}
		}UNLOCK
	}
}

void SHOUTCAST_OUTPUT::RemoveOutput(int Connection) {
	if (Connection >= 0 && Connection < 5) {
		LOCK{
		T_OUTPUT *Output = OutputManager[Connection];
		if (Output) {
			OutputManager.DelJob(Connection);
			delete Output;
		}
		}UNLOCK
	}
}

int SHOUTCAST_OUTPUT::ConnectOutput(int Connection) {
	if (Connection >= 0 && Connection < 5) {
		LOCK{
		T_OUTPUT *Output = OutputManager[Connection];
		if (Output && Encoder) {
			UpdateOutput(Connection);

			compatible_connect(Output, (Output->Config->Port) + (LOBYTE(Output->Config->protocol) == 1));

			Output->Bitrate = 0;
			Output->ContentType = "audio/mpeg";

			if (Encoder) {
				int infosize = sizeof(T_EncoderIOVals);
				T_ENCODER_MP3_INFO *EncSettings = (T_ENCODER_MP3_INFO *)Encoder->GetExtInfo(&infosize);
				if (EncSettings && infosize) Output->Bitrate = EncSettings->output_bitRate;
				Output->ContentType = Encoder->GetContentType();
			}
			OutputManager.SetJobState(Connection, OUT_CONNECT);
		}
		}UNLOCK
	}
	return 1;
}

int SHOUTCAST_OUTPUT::DisconnectOutput(int Connection, int withReconnect, int reconnectTime) {
	if (Connection >= 0 && Connection < 5) {
		LOCK{
		T_OUTPUT *Output = OutputManager[Connection];
		if (Output) {
			int state = OutputManager.GetJobState(Connection);
			if (state != OUT_DISCONNECTED) {
				Output->Info.Reconnect = withReconnect >= 0 ? withReconnect : Output->Config->AutoRecon;
				Output->Info.ReconnectTime = reconnectTime >= 0 ? reconnectTime : Output->Config->ReconTime;
				Output->SlowClose = 0;
				if (Encoder) {
					int infosize = sizeof(T_EncoderIOVals);
					T_ENCODER_MP3_INFO *EncSettings = (T_ENCODER_MP3_INFO *)Encoder->GetExtInfo(&infosize);
					if (EncSettings && infosize) Output->Bitrate = EncSettings->output_bitRate;
					Output->ContentType = Encoder->GetContentType();
				}
				OutputManager.SetJobState(Connection, OUT_DISCONNECT);
			}
		}
		}UNLOCK
	}
	return 1;
}

void SHOUTCAST_OUTPUT::SetEncoder(C_ENCODER *encoder, int takeOwnership) {
	LOCK{
	if (IOwnEncoder && Encoder && Encoder != DEFAULT_ENCODER)
		delete Encoder;
	if (encoder != DEFAULT_ENCODER) {
		IOwnEncoder = takeOwnership;
		Encoder = encoder;
	} else {
		try {
			Encoder = new C_ENCODER_MP3(lame_init, lame_init_params, lame_encode_buffer_interleaved, lame_encode_flush);
			IOwnEncoder = 1;
		} catch(...) {
			Encoder = NULL;
		}
	}
	}UNLOCK
}

C_ENCODER *SHOUTCAST_OUTPUT::GetEncoder() {
	C_ENCODER *Enc = NULL;
	LOCK{
	Enc = Encoder;
	}UNLOCK
	return Enc;
}

bool validateTitle(wchar_t **dest, const wchar_t* src)
{
	bool allowed = true;
	int src_len = lstrlenW(src);
	char *str = (char*)calloc(src_len, sizeof(char));
	if (str)
	{
		WideCharToMultiByte(CP_ACP, 0, src, -1, str, src_len, 0, 0);

		std::string updinfoSong(str);
		if (!stringUtil::stripAlphaDigit(updinfoSong).empty())
		{
			// work on lowercase comparison as well as doing a check to see if
			// after removing white space + punctuation we have a valid title.
			std::string m_checkUpdinfoSong = stringUtil::toLower(updinfoSong);

			// exclude weird title updates from being accepted
			// as no point in giving junk to the user later on
			if (m_checkUpdinfoSong.find("!doctype") != string::npos ||
				m_checkUpdinfoSong.find("<script") != string::npos ||
				m_checkUpdinfoSong.find("<html") != string::npos ||
				m_checkUpdinfoSong.find("<body") != string::npos ||
				m_checkUpdinfoSong.find("<div") != string::npos ||
				m_checkUpdinfoSong.find("%] ") != string::npos ||
				m_checkUpdinfoSong.find("invalid resource") != string::npos ||
				(m_checkUpdinfoSong.find("nextsong") != string::npos &&
				 m_checkUpdinfoSong.find("sctrans2next") != string::npos) ||

				m_checkUpdinfoSong.find("radio online") != string::npos ||

				m_checkUpdinfoSong.find("track ") == 0 ||
				m_checkUpdinfoSong.find("track0") == 0 ||
				m_checkUpdinfoSong.find("track1") == 0 ||
				m_checkUpdinfoSong.find("stream ") == 0 ||
				m_checkUpdinfoSong.find("no artist ") == 0 ||
				m_checkUpdinfoSong.find("new artist ") == 0 ||
				m_checkUpdinfoSong.find("line-in ") == 0 ||
				m_checkUpdinfoSong.find("inter_") == 0 ||
				m_checkUpdinfoSong.find("jj mckay - ") == 0 ||
				m_checkUpdinfoSong.find("artist - ") == 0 ||

				m_checkUpdinfoSong.find("$") == 0 ||
				m_checkUpdinfoSong.find("%") == 0 ||
				m_checkUpdinfoSong.find("&") == 0 ||
				m_checkUpdinfoSong.find("[") == 0 ||
				m_checkUpdinfoSong.find("?") == 0 ||
				m_checkUpdinfoSong.find("_") == 0 ||
				m_checkUpdinfoSong.find("- ") == 0 ||
				m_checkUpdinfoSong.find(". ") == 0 ||

				m_checkUpdinfoSong == "-" ||
				m_checkUpdinfoSong == "auto dj" ||
				m_checkUpdinfoSong == "ao vivo" ||
				m_checkUpdinfoSong == "unknown" ||
				m_checkUpdinfoSong == "test" ||
				m_checkUpdinfoSong == "dsp" ||
				m_checkUpdinfoSong == "demo" ||
				m_checkUpdinfoSong == "line input" ||
				m_checkUpdinfoSong == "dj mike llama - llama whippin` intro" ||
				m_checkUpdinfoSong == "preview") {
				allowed = false;
			}
		}
		else
		{
			allowed = false;
		}
	}
	else
	{
		allowed = false;
	}

	if (dest) {
		if (*dest)
		{
			free(*dest);
		}
		*dest = _wcsdup(allowed ? src : L"");
	}

	return allowed;
}

void SHOUTCAST_OUTPUT::UpdateTitleCache(wchar_t *Title, std::vector<std::wstring> NextList, wchar_t *Song,
										wchar_t *Album, wchar_t *Artist, wchar_t *Genre, wchar_t *Comment,
										wchar_t* Year, int Connection, bool sendNext) {
	int update = 0;

	if(!metadata.Title || wcscmp(metadata.Title, Title)) {
		if (metadata.Title) free(metadata.Title);
		metadata.Title = _wcsdup(Title);
		update++;
	}

	metadata.NextList.resize(0);
	if (!NextList.empty()) {
		metadata.NextList = NextList;
		update++;
	}

	if(!metadata.Song || wcscmp(metadata.Song, (Song ? Song : L""))) {
		if (validateTitle(&metadata.Song, (Song ? Song : L""))) {
			update++;
		}
	}

	if(!metadata.Album || wcscmp(metadata.Album, (Album ? Album : L""))) {
		if (validateTitle(&metadata.Album, (Album ? Album : L""))) {
			update++;
		}
	}

	if(!metadata.Artist || wcscmp(metadata.Artist, (Artist ? Artist : L""))) {
		if (validateTitle(&metadata.Artist, (Artist ? Artist : L""))) {
			update++;
		}
	}

	if(!metadata.Genre || wcscmp(metadata.Genre, (Genre ? Genre : L""))) {
		if (validateTitle(&metadata.Genre, (Genre ? Genre : L""))) {
			update++;
		}
	}

	if(!metadata.Comment || wcscmp(metadata.Comment, (Comment ? Comment : L""))) {
		if (validateTitle(&metadata.Comment, (Comment ? Comment : L""))) {
			update++;
		}
	}

	if(!metadata.Year || wcscmp(metadata.Year, (Year ? Year : L""))) {
		if (validateTitle(&metadata.Year, (Year ? Year : L""))) {
			update++;
		}
	}

	UpdateTitle(0, NextList, Connection, sendNext);
}

void SHOUTCAST_OUTPUT::UpdateAlbumArtCache(void* APIC, int APICLength, int APICType, int Connection) {
	int artType = ((APICType & 0xFF00) == (MSG_METADATA_ALBUMART|MSG_METADATA_PLAYING_ART));
	// make sure we're within the metadata limits
	// which is 32 * max metadata payload (523872)
	// and if not then we discard the artwork
	int prevAPICLength = metadata.APICLength[artType];
	if (APICLength > UV_MAX_TOTAL_META_LEN) {
		metadata.APICLength[artType] = 0;
		metadata.APIC[artType] = 0;
	} else {
		metadata.APIC[artType] = APIC;
		metadata.APICLength[artType] = APICLength;
	}
	metadata.APICType[artType] = APICType;

	// update the metadata cache usage so we can work
	if (Connection >= 0 && Connection < 5) {
		LOCK{
		T_OUTPUT *Output = OutputManager[Connection];
		if (Output) {
			// only update the flag if it's new stream art or is playing and is different from before
			if (!(!prevAPICLength && artType && (prevAPICLength == metadata.APICLength[artType]))) {
				Output->Info.meta_cached |= (!artType ? 2 : 4);
			}
		}
		}UNLOCK

		UpdateArtwork(Connection);
	}
}

void SHOUTCAST_OUTPUT::UpdateTitle(wchar_t *Title, std::vector<std::wstring> NextList,
								   int Connection, bool sendNext, bool UseCache) {
	//LOCK{
	if (Connection >= 0 && Connection < 5) {
		T_OUTPUT *Output = OutputManager[Connection];
		if (Output && Output->Type == OUTTYPE_SOURCE &&
			(OutputManager.GetJobState(Connection) == OUT_SENDCONTENT ||
			 OutputManager.GetJobState(Connection) == OUT_SEND_METADATA)) {

			// clear the metadata send flag
			if (Output->Info.meta_cached & 1) {
				Output->Info.meta_cached -= 1;
			}

			if (LOBYTE(Output->Config->protocol) == 1 && Output->Config->doTitleUpdate) {
				T_OUTPUT *job = new T_OUTPUT;
				job->Bitrate = Output->Bitrate;
				job->ContentType = Encoder->GetContentType();
				job->Config = Output->Config;
				job->Type = OUTTYPE_TITLE;
				job->SlowClose = 0;
				compatible_connect(job, job->Config->Port);

				wchar_t *title = (UseCache?metadata.Title:Title);
				if (title) {
					validateTitle(&job->Info.Title, title);
					validateTitle(&Output->Info.Title, title);
				} else {
					validateTitle(&job->Info.Title, Output->Info.Title);
				}

				OutputManager.AddJob(OUT_CONNECT, job);
			} else if (LOBYTE(Output->Config->protocol) != 1) {
				validateTitle(&Output->Info.Title, (UseCache?metadata.Title:Title));

				//#ifndef _DEBUG
				if (UseCache) {
					if (!metadata.NextList.empty()) {
						Output->Info.NextList = metadata.NextList;
					}
				} else {
					if (!NextList.empty()) {
						Output->Info.NextList = NextList;
					}
				}
				//#endif

				validateTitle(&Output->Info.Song, (metadata.Song && UseCache ? metadata.Song : L""));
				validateTitle(&Output->Info.Album, (metadata.Album && UseCache ? metadata.Album : L""));
				validateTitle(&Output->Info.Artist, (metadata.Artist && UseCache ? metadata.Artist : L""));
				validateTitle(&Output->Info.Genre, (metadata.Genre ? metadata.Genre : L""));
				validateTitle(&Output->Info.Comment, (metadata.Comment && UseCache ? metadata.Comment : L""));
				validateTitle(&Output->Info.Year, (metadata.Year && UseCache ? metadata.Year : L""));

				if (Output->Config->doTitleUpdate) {
					// only output friendlier line breaks when debugging is needed
					#ifdef _DEBUG
					#define XML_DEBUG
					#endif

					#ifdef XML_DEBUG
					#define EOL "\n"
					#define TAB "\t"
					#else
					#define EOL ""
					#define TAB ""
					#endif
					stringstream s;
					s << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<metadata>" EOL;

					wchar_t * title = (Output->Info.Song && *Output->Info.Song ? Output->Info.Song : Output->Info.Title);
					if (validateTitle(0, title)) {
						 s << "<TIT2>" << ConvertToUTF8Escaped(title) << "</TIT2>" EOL;
					}

					if (Output->Info.Album && Output->Info.Album[0]) s << "<TALB>" << ConvertToUTF8Escaped(Output->Info.Album) << "</TALB>" EOL;
					if (Output->Info.Artist && Output->Info.Artist[0]) s << "<TPE1>" << ConvertToUTF8Escaped(Output->Info.Artist) << "</TPE1>" EOL;
					if (Output->Info.Year && Output->Info.Year[0]) s << "<TYER>" << ConvertToUTF8Escaped(Output->Info.Year) << "</TYER>" EOL;
					if (Output->Info.Comment && Output->Info.Comment[0]) s << "<COMM>" << ConvertToUTF8Escaped(Output->Info.Comment) << "</COMM>" EOL;
					if (Output->Info.Genre && Output->Info.Genre[0]) s << "<TCON>" << ConvertToUTF8Escaped(Output->Info.Genre) << "</TCON>" EOL;

					s << "<TENC>SHOUTcast Source DSP v" << sourceVersion << "</TENC>" EOL;

					if (Output->Config->Description[0]) s << "<TRSN>" << escapeXML(Output->Config->Description) << "</TRSN>" EOL;

					s << "<WORS>" << escapeXML(Output->Config->ServerURL && Output->Config->ServerURL[0] ? Output->Config->ServerURL : "http://www.shoutcast.com") << "</WORS>" EOL;

					if (validateTitle(0, title)) {
						s << "<extension>" EOL TAB "<TITLE seq=\"1\">" << ConvertToUTF8Escaped(title) << "</TITLE>" EOL;

						if (!Output->Info.NextList.empty() && sendNext) {
							s << TAB;
							std::string soon;
							for (size_t idx = 0, seq = 0; idx < Output->Info.NextList.size(); idx++) {
								if (validateTitle(0, Output->Info.NextList[idx].c_str()))
								{
									std::string next = ConvertToUTF8Escaped(Output->Info.NextList[idx].c_str());
									s << "<TITLE seq=\"" << (seq+2) << "\">" << next << "</TITLE>" EOL TAB;
									// store the first item as that is used for the 'soon' item
									if (seq == 0) soon = next;
									seq++;
								}
							}
							if (!soon.empty()) {
								s << "<soon>" << soon << "</soon>" EOL;
							}
						}
						s << "</extension>" EOL;
					}
					s << "</metadata>";

					// TODO make sure this will split up > UV_MAX_META_FRAME_LEN blocks
					mid = createUvoxMetaFrame(s.str().length(),(char*)s.str().data(), MSG_METADATA_XML_NEW, Output, mid);
				}
			}
		}
	}
}

void SHOUTCAST_OUTPUT::UpdateArtwork(int Connection) {
	if (Connection >= 0 && Connection < 5) {
		LOCK{
		T_OUTPUT *Output = OutputManager[Connection];
		if (Output && Output->Type == OUTTYPE_SOURCE) {
	
			if (LOBYTE(Output->Config->protocol) != 1) {
				Output->Info.APIC[0] = metadata.APIC[0];
				Output->Info.APICLength[0] = metadata.APICLength[0];
				Output->Info.APICType[0] = metadata.APICType[0];
	
				Output->Info.APIC[1] = metadata.APIC[1];
				Output->Info.APICLength[1] = metadata.APICLength[1];
				Output->Info.APICType[1] = metadata.APICType[1];
	
				// calculate the album art span inorder to use it once we've sent the title in following callbacks
				for (int i = 0; i < 2; i++) {
					int AlbumArtSpan = (metadata.APICLength[i] / UV_MAX_DATA_LEN) + ((metadata.APICLength[i] % UV_MAX_DATA_LEN) < UV_MAX_DATA_LEN ? 1 : 0);
					// if over the limit then abort trying to send this
					if (AlbumArtSpan > UV_MAX_META_FRAGMENTS) {
						AlbumArtSpan = 1;
						Output->Info.APIC[i] = 0;
					}

					Output->Info.art_cached_span[i] = Output->Info.art_cached[i] = AlbumArtSpan;
					Output->Info.art_index[i] = 1;
					Output->Info.art_cached_length[i] = Output->Info.APICLength[i];
				}
			}
		}
		}UNLOCK
	}
}

int SHOUTCAST_OUTPUT::UpdateAlbumArt(int Connection) {
	if (Connection >= 0 && Connection < 5) {
		LOCK{
		T_OUTPUT *Output = OutputManager[Connection];
		if (Output && Output->Type == OUTTYPE_SOURCE && 
			OutputManager.GetJobState(Connection) == OUT_SEND_ARTWORK) {
			int artType = (Output->Info.meta_cached & 4 ? 1 : (Output->Info.meta_cached & 2 ? 0 : -1));
			if (artType != -1 && LOBYTE(Output->Config->protocol) != 1 && Output->Info.art_cached[artType] > 0) {
				int type = Output->Info.APICType[artType];
				int length = Output->Info.art_cached_length[artType];
				int count = Output->Info.art_cached_span[artType] - Output->Info.art_cached[artType];
	
				// attempt to process as a multi-part message as APIC most likely won't fit in just one uvox packet
				if (length >= UV_MAX_META_LEN) {
					createUvoxMetaFrame(UV_MAX_META_LEN,
										(char*)Output->Info.APIC[artType]+(count * UV_MAX_META_LEN),
										type, Output, mid, Output->Info.art_cached_span[artType]);
					Output->Info.art_cached_length[artType] -= UV_MAX_META_LEN;
				} else {
					createUvoxMetaFrame(length,
										(char*)Output->Info.APIC[artType]+(count * UV_MAX_META_LEN),
										type, Output, mid, Output->Info.art_cached_span[artType]);
					Output->Info.art_cached_length[artType] = 0;
				}
				Output->Info.art_index[artType] += 1;
	
				if (Output->Info.art_cached[artType] == 1) {
					mid+=1;
					Output->Info.meta_cached -= (artType ? 4 : 2);
					// reset the values so we can re-send on disconnect, etc
					Output->Info.art_cached[artType] = Output->Info.art_cached_span[artType];
					Output->Info.art_index[artType] = 1;
					Output->Info.art_cached_length[artType] = Output->Info.APICLength[artType];
				}
			}
		}
		}UNLOCK
	}
	return 1;
}

enum OUTPUTSTATE SHOUTCAST_OUTPUT::GetState(int Connection) {
	int retval = -1;
	if (Connection >= 0 && Connection < 5) {
		LOCK{
		retval = OutputManager.GetJobState(Connection);
		}UNLOCK
	}
	return retval == -1 ? OUT_ERROR : (enum OUTPUTSTATE)retval;
}

T_OUTPUT_CONFIG *SHOUTCAST_OUTPUT::operator[](int Connection) {
	return GetOutput(Connection);
}

T_OUTPUT_CONFIG *SHOUTCAST_OUTPUT::GetOutput(int Connection) {
	T_OUTPUT_CONFIG *Config = NULL;
	if (Connection >= 0 && Connection < 5) {
		LOCK{
		T_OUTPUT *Output = OutputManager[Connection];
		if (Output) Config = Output->Config;
		}UNLOCK
	}
	return Config;
}

T_OUTPUT_INFO *SHOUTCAST_OUTPUT::GetOutputInfo(int Connection) {
	T_OUTPUT_INFO *Info = NULL;
	if (Connection >= 0 && Connection < 5) {
		LOCK{
		T_OUTPUT *Output = OutputManager[Connection];
		if (Output) Info = &Output->Info;
		}UNLOCK
	}
	return Info;
}

// Protected methods

int SHOUTCAST_OUTPUT::Output_Disconnected(int state, int last_state, T_OUTPUT *userData) {
	STATE;
	LOCK{
	userData->Config->protocol_retry = 0;
	}UNLOCK
	return state; // sit and spin
}

int SHOUTCAST_OUTPUT::Output_Connect(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	userData->Config->protocol_retry = MAKEWORD(0, HIBYTE(userData->Config->protocol_retry));
    userData->Info.Succeeded = 1;
	if (userData->Output) {
		userData->Output->FlushSend();
	}

	if ((clock() - userData->Info.ConnectionTime) / CLOCKS_PER_SEC >= 1/*userData->Info.ReconnectTime*/) {
		if (userData->Output && userData->Output->get_state() == CONNECTION_STATE_CONNECTED) { // are we connected yet?
			userData->Info.ConnectionTime = clock();
			if (userData->Type == OUTTYPE_SOURCE) {
				userData->Info.Reconnect = userData->Config->AutoRecon;
				userData->Info.ReconnectTime = userData->Config->ReconTime;
				userData->Info.ConnectedAt = time(NULL);
				UNLOCK
				return OUT_REQUEST_CIPHER; // connected, go authenticate
			} else if (userData->Type == OUTTYPE_TITLE) {
				UNLOCK
				return OUT_TITLESENDUPDATE; // connected, go update the title
			}
		}
	}
	}UNLOCK
	return state; // sit and spin
}

int SHOUTCAST_OUTPUT::Output_SendContent(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	if (userData->Output->get_state() == CONNECTION_STATE_ERROR) { // connection error?
		return OUT_DISCONNECT; // error happened, go shut down
	}

	// check if we have any metadata or artwork to dispatch (depends on protocol)
	if (userData->Info.meta_cached & 1) {
		userData->Info.meta_cached -= 1;
		return OUT_SEND_METADATA;
	} else if (LOBYTE(userData->Config->protocol) != 1 &&
			   (userData->Info.meta_cached & 2 || userData->Info.meta_cached & 4) &&
			   userData->Info.art_cached > 0) {
		return OUT_SEND_ARTWORK;
	}

	return state; // sit and spin
}

int SHOUTCAST_OUTPUT::Output_Disconnect(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	//STATE;
	LOCK{
	// reset the metadata flags so it will trigger on a connection coming back
	// with usage of what was already cached to determine what we will re-send
	userData->Info.meta_cached = 1|(userData->Info.art_cached_length[0] ? 2 : 0)|(userData->Info.art_cached_length[1] ? 4 : 0);

	int outstate = (userData->Output ? userData->Output->get_state() : CONNECTION_STATE_ERROR);
	if (outstate == CONNECTION_STATE_CONNECTING) {	// are we connecting? if so then just abort
		UNLOCK
		return OUT_DISCONNECTED;
	}

	if (outstate == CONNECTION_STATE_CONNECTED) { // are we connected yet?
		userData->Info.ConnectionTime = clock();
		userData->Info.ConnectedAt = 0;
		userData->Info.BytesSent = 0;
		if (LOBYTE(userData->Config->protocol) != 1) {
			createUvoxFrame(1, "0\0", MSG_TERMINATE, userData);
			if (userData->Info.Succeeded != -1) {
				Output_DUMMY(state, last_state, userData);
			}
		}
		userData->Output->Close(!userData->SlowClose);
	}

	if (outstate == CONNECTION_STATE_CLOSING) {
		userData->Info.ConnectionTime = clock(); // update the clock with the current time so that we don't get some weird crap for reconnections.
	}

	if (outstate == CONNECTION_STATE_CLOSED || outstate == CONNECTION_STATE_ERROR) {
		userData->Info.Switching = 0;
		if (last_state == OUT_SENDYP) {
			userData->Info.Succeeded = -1;
			userData->Info.ErrorMsg = "Blocked";
		} else if (userData->Type == OUTTYPE_SOURCE &&
				   (last_state == OUT_CONNECT || last_state == OUT_REQUEST_CIPHER ||
				   last_state == OUT_SENDAUTH || last_state == OUT_RECV_CIPHER ||
				   last_state == OUT_DISCONNECT)) {
			// if using automatic mode then switch between v1 and v2 modes as needed
			if (HIBYTE(userData->Config->protocol) && ((LOBYTE(userData->Config->protocol_retry) > 5) ||
				(last_state == OUT_CONNECT && HIBYTE(userData->Config->protocol_retry) < 1))) {
				int protocol = (2 - (LOBYTE(userData->Config->protocol) != 1));
				userData->Config->protocol_retry = MAKEWORD(0, HIBYTE(userData->Config->protocol_retry)+1);
				userData->Info.Switching = (LOBYTE(userData->Config->protocol) != 1 ? 2 : 1);
				userData->Config->protocol = MAKEWORD(protocol, 1);
				userData->Info.ConnectionTime = clock();
				UNLOCK
				return OUT_RECONNECT;
			}
		}

		if (userData->Info.Reconnect && userData->Type == OUTTYPE_SOURCE) {
			// if using automatic mode then switch between v1 and v2 modes as needed
			if (HIBYTE(userData->Config->protocol)) {
				// TODO need to make this into a function and have it show the switching message
				int protocol = (2 - (LOBYTE(userData->Config->protocol) != 1));
				userData->Config->protocol_retry = 0;
				userData->Info.Switching = (LOBYTE(userData->Config->protocol) != 1 ? 2 : 1);
				userData->Config->protocol = MAKEWORD(protocol, 1);
				userData->Info.ConnectionTime = clock();
			}
			UNLOCK
			return OUT_RECONNECT;
		} else {
			UNLOCK
			return OUT_DISCONNECTED;
		}
	}
	}UNLOCK
	return state; // sit and spin
}

int SHOUTCAST_OUTPUT::Output_Reconnect(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	if ((clock() - userData->Info.ConnectionTime) / CLOCKS_PER_SEC >= userData->Info.ReconnectTime) {
		if (userData->Output) {
			compatible_connect(userData, userData->Config->Port + (LOBYTE(userData->Config->protocol) == 1));
		}
		UNLOCK
		return OUT_CONNECT;
	}
	}UNLOCK
	return state; // sit and spin
}

void FixString(const char *in, char *out) {
	while (in && *in) {
		if ((*in >= 'A' && *in <= 'Z') ||
			(*in >= 'a' && *in <= 'z') ||
			(*in >= '0' && *in <= '9') ||
			(*in == '-') ||
			(*in == '_') ||
			(*in == '.') ||
			(*in == '!') ||
			(*in == '~') ||
			(*in == '*') ||
			(*in == '\'') ||
			(*in == '(') ||
            (*in == '[') ||
			(*in == ']') ||
			(*in == ')')) {
				*out++=*in++;
		} else if (*in >= 0 && *in < 32 && *in != 9 && *in != 10 && *in != 13) {
			// strip out characters which aren't supported by the DNAS
			// (only allow backspace, linefeed and carriage return)
			in++;
		} else {
			unsigned int i=*(unsigned char *)in;
			const char *t=in;
			in++;
			if (in == t + 1) {
				if (i == '\'') i = '`';
				else if (i == '<') i = '(';
				else if (i == '>') i = ')';
				else if (i == '\"') i='`';
                else if (i == '[') i='`';
                else if (i == ']') i='`';
				snprintf(out, 3, "%%%02X", i);
				out += 3;
			} else if (in == t + 2) { // multibyte
			}
		}
	}
	*out=0;
}

/* SHOUTcast 1 title updates */
int SHOUTCAST_OUTPUT::Output_Title_SendUpdate(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	char utf8title[1024] = {0};
	char fixedtitle[3072] = {0}; // if every character is URL-encoded, we would get a maximum of 3 times the space needed
	char buffer[2048] = {0};

	WideCharToMultiByte(CP_ACP, 0, userData->Info.Title, -1, utf8title, sizeof(utf8title), 0, 0);
	FixString(utf8title, fixedtitle);
	fixedtitle[1023] = 0; // DNAS doesn't like titles bigger than 1k

	// send the correct password based on dj name, etc
	std::string t, u, p;
	u = userData->Config->UserID;
	p = userData->Config->Password;
	if (!u.empty()) t = u + ":" + p;
	else t = p;

	snprintf(buffer, sizeof(buffer), "GET /admin.cgi?pass=%s&mode=updinfo&song=%s&dj=%s HTTP/1.0\n", (char *)t.data(), fixedtitle, userData->Config->UserID);
	userData->Output->SendString(buffer);

	stringstream s;
	s << "User-Agent: SHOUTcast Source DSP v" << sourceVersion << " Title Update (Mozilla)\n\n";
	userData->Output->SendString((char*)s.str().data());

	userData->SlowClose = 1;
	userData->Info.meta_cached = 1;
	}UNLOCK
	return OUT_DISCONNECT;
}

/* SHOUTcast 2 in-stream metada */
int SHOUTCAST_OUTPUT::Output_Send_Metadata(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	if (userData->TitleCallback) {
		if (userData->Connection >= 0 && userData->Connection < 5) {
			userData->TitleCallback(userData->Connection, 0);
		}
	}
	}UNLOCK
	return OUT_SENDCONTENT;
}

/* SHOUTcast 2 in-stream metada */
int SHOUTCAST_OUTPUT::Output_Send_Artwork(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	int artType = (userData->Info.meta_cached & 4 ? 1 : (userData->Info.meta_cached & 2 ? 0 : -1));
	if (userData->TitleCallback) {
		if (userData->Connection >= 0 && userData->Connection < 5) {
			if (artType != -1) {
				userData->TitleCallback(userData->Connection, 1);
				// decrement counts to move to next part of image packets if needed
				if (userData->Info.meta_cached & (artType ? 4 : 2)) {
					userData->Info.art_cached[artType]--;
					if (userData->Info.art_cached[artType] < 0) userData->Info.art_cached[artType] = 0;
				}
			}
		}
	} else {
		if (artType != -1) {
			userData->Info.art_cached[artType] = 0;
		}
	}
	}UNLOCK
	return OUT_SENDCONTENT;
}

int SHOUTCAST_OUTPUT::Output_Request_Cipher(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	if (LOBYTE(userData->Config->protocol) != 1) {
		userData->Output->FlushSend();
		createUvoxFrame(3, "2.1\0", MSG_CIPHER, userData);
		UNLOCK
		return OUT_RECV_CIPHER;
	} else {
		// shoutcast 1
		UNLOCK
		return OUT_SENDAUTH;
	}
	}UNLOCK
	return state;
}

int SHOUTCAST_OUTPUT::Output_Receive_Cipher(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	if (userData->Output->GetReceiveBytesAvailable() > 0) {
		memset(buf, '\0', sizeof(buf));
		userData->Output->ReceiveLine(buf, sizeof(buf));
		parseUvoxFrame(buf, out);
		if (checkUvoxFrameForError(out, state, userData) == -1) {
			UNLOCK
			return OUT_DISCONNECT;
		} else {
			// copy the cipher key now we have it.
			strcpy_s(userData->Config->cipherkey, sizeof(userData->Config->cipherkey)-1, out+4);
			UNLOCK
			userData->Config->protocol_retry = 0;
			return OUT_SENDAUTH;
		}
	}

	userData->Config->protocol_retry = MAKEWORD(LOBYTE(userData->Config->protocol_retry)+1, HIBYTE(userData->Config->protocol_retry));

	// this handles automatic mode being enabled
	if (!HIBYTE(userData->Config->protocol) &&
		(LOBYTE(userData->Config->protocol_retry) > 5)) {
		userData->Output->Close(!userData->SlowClose);
		UNLOCK
		return OUT_SENDAUTH;
	}

	}UNLOCK

	return (LOBYTE(userData->Config->protocol_retry) > 5 ? OUT_DISCONNECT : state);
}

int SHOUTCAST_OUTPUT::Output_SendAuth(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	userData->Output->FlushSend();
	userData->Info.Succeeded = 1;
	std::string s,
				u = userData->Config->UserID,
				p = userData->Config->Password;

	if (LOBYTE(userData->Config->protocol) != 1) {
		uvAuth21 * auth = new uvAuth21();
		std::string k;
		k = userData->Config->cipherkey;
		s = "2.1:";
		s += (!userData->Config->StationID[0] ? "1" : userData->Config->StationID);
		s += ":";
		s += auth->encrypt(u, p, k);
		createUvoxFrame(s.length(), (char *)s.data(), MSG_AUTH, userData);
		delete auth;
	} else {
		if (!u.empty()) s = u + ":" + p;
		else s = p;
		userData->Output->SendString((char *)s.data());
		userData->Output->SendString("\n");
	}
	UNLOCK
	return OUT_RECVAUTHRESPONSE;
	}UNLOCK
	return state;
}

int SHOUTCAST_OUTPUT::Output_RecvAuthResponse(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	if (LOBYTE(userData->Config->protocol) != 1) {
		if (userData->Output->GetReceiveBytesAvailable() > 0) {
			memset(buf, '\0', sizeof(buf));
			userData->Info.Succeeded = 1;
			userData->Output->recv_bytes(buf, sizeof(buf));
			parseUvoxFrame(buf, out);
			int err = checkUvoxFrameForError(out, state, userData);
			if (err == -1) {
				UNLOCK
				return OUT_DISCONNECT;
			} else {
				// test for a cipher failure
				if (strncmp(buf, "ACK:", 4) == 0) {
					userData->Info.Succeeded = -1;
					userData->Info.ErrorMsg = "CipherFail";
					UNLOCK
					return OUT_FAIL_CIPHER;
				}
				UNLOCK
				return OUT_SEND_MIME;
			}
		}
	} else {
		// auth shoutcast 1
		if (userData->Output->GetReceiveBytesAvailable() > 0) {
			userData->Output->ReceiveLine(buf, sizeof(buf));
			char *ok = strstr(buf, "OK");
			if (ok) { // we got OK response... shoutcast v1
				userData->Info.Succeeded = 1;
				UNLOCK
				return OUT_SENDYP; // send the sc v1 YP stuff, now
			} else {
				userData->Info.Succeeded = 0;
				if (strcmpi(buf, "invalid password") == 0) {
					userData->Info.Succeeded = -1;
					userData->Info.ErrorMsg = "NAK:Deny";
				} else if (strcmpi(buf, "stream moved") == 0) {
					userData->Info.Succeeded = -1;
					userData->Info.ErrorMsg = "StreamMoved";
				}
				UNLOCK
				return OUT_DISCONNECT;
			}
		}
	}
	}UNLOCK
	return state;
}

int SHOUTCAST_OUTPUT::Output_Send_Mime(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	std::string s = userData->ContentType;
	userData->Output->FlushSend();
	createUvoxFrame(s.length(), (char *)s.data(), MSG_MIME_TYPE, userData);
	}UNLOCK
	return OUT_RECV_MIME;
}

int SHOUTCAST_OUTPUT::Output_Recv_Mime(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	if (userData->Output->GetReceiveBytesAvailable() > 0) {
		memset(buf, '\0', sizeof(buf));
		userData->Output->recv_bytes(buf, sizeof(buf));
		parseUvoxFrame(buf, out);
	    UNLOCK
		return OUT_SEND_BITRATE;
   }
	}UNLOCK
	return state;
}

int SHOUTCAST_OUTPUT::Output_Send_Bitrate(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	char bitrate[128] = {0};
	std::string s;
	snprintf(bitrate, sizeof(bitrate), "%d:%d", userData->Bitrate*1000, userData->Bitrate*1000);
	s = bitrate;
	userData->Output->FlushSend();
	createUvoxFrame(s.length(), (char *)s.data(), MSG_BROADCAST_SETUP, userData);
	}UNLOCK
	return OUT_RECV_BITRATE;
}

int SHOUTCAST_OUTPUT::Output_Recv_Bitrate(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	if (userData->Output->GetReceiveBytesAvailable() > 0) {
		memset(buf, '\0', sizeof(buf));
		userData->Output->recv_bytes(buf, sizeof(buf));
		parseUvoxFrame(buf, out);
		int err = checkUvoxFrameForError(out, state, userData);
		if (err == -1) {
			UNLOCK
			return OUT_DISCONNECT;
		} else {
			UNLOCK
			return OUT_SEND_BUFSIZE;
		}
	}
	}UNLOCK
	return state;
}

int SHOUTCAST_OUTPUT::Output_Send_Buf_Size(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	char bsize[32] = {0};
	int toSend = (userData->Bitrate * 20)/8;
	snprintf(bsize, sizeof(bsize), "%d:0", toSend);
	std::string s = bsize;
	userData->Output->FlushSend();
	createUvoxFrame(s.length(), (char *)s.data(), MSG_NEGOTIATE_BUFFER_SIZE, userData);
	}UNLOCK
	return OUT_RECV_BUFSIZE;
}

int SHOUTCAST_OUTPUT::Output_Recv_Buf_Size(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	if (userData->Output->GetReceiveBytesAvailable() > 0) {
		memset(buf, '\0', sizeof(buf));
		userData->Output->recv_bytes(buf, sizeof(buf));
		UNLOCK
		return OUT_SEND_MAX;
	}
	}UNLOCK
	return state;
}

int SHOUTCAST_OUTPUT::Output_Send_Max_Size(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	userData->Output->FlushSend();
	createUvoxFrame(7, "16377:0\0", MSG_MAX_PAYLOAD_SIZE, userData);
	}UNLOCK
	return OUT_RECV_MAX;
}

int SHOUTCAST_OUTPUT::Output_Recv_Max_Size(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	if (userData->Output->GetReceiveBytesAvailable() > 0) {
		memset(buf, '\0', sizeof(buf));
		userData->Output->recv_bytes(buf, sizeof(buf));
		parseUvoxFrame(buf, out);
	    UNLOCK
	    return OUT_SENDYP;
	}
	}UNLOCK
	return state;
}

int SHOUTCAST_OUTPUT::Output_DUMMY(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	if (userData->Output->GetReceiveBytesAvailable() > 0) {
		memset(buf, '\0', sizeof(buf));
		userData->Output->recv_bytes(buf, sizeof(buf));
		parseUvoxFrame(buf, out);
	}
	}UNLOCK
	return state;
}

int SHOUTCAST_OUTPUT::Output_SendYP(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	if (LOBYTE(userData->Config->protocol) != 1) {
		//icyname
		std::string s;
		s = userData->Config->Description;
		createUvoxFrame(s.length(), (char *)s.data(), MSG_ICYNAME, userData);
		Output_DUMMY(state, last_state, userData);

		//icygenre
		s = userData->Config->Genre;
		createUvoxFrame(s.length(), (char *)s.data(), MSG_ICYGENRE, userData);
		Output_DUMMY(state, last_state, userData);

		//icyurl
		s = userData->Config->ServerURL;
		createUvoxFrame(s.length(), (char *)s.data(), MSG_ICYURL, userData);
		Output_DUMMY(state, last_state, userData);

		//icypub
		char pub[2] = {0};
		snprintf(pub, sizeof(pub), "%d", !!userData->Config->Public);
		createUvoxFrame(1, pub, MSG_ICYPUB, userData);
		Output_DUMMY(state, last_state, userData);
        UNLOCK
		return OUT_SEND_INITFLUSH;
	} else {
		char ypbuf[4096] = {0};
		char ypbuf2[4096] = {0};
		// send Description
		snprintf(ypbuf, sizeof(ypbuf), "icy-name:%s\n", userData->Config->Description);
		userData->Output->SendString(ypbuf);
		// send Genre
		ypbuf2[0] = 0;
		if (strlen(userData->Config->Genre) > 0) {
			snprintf((char *)&ypbuf2+strlen(ypbuf2), sizeof(ypbuf2), "%s", userData->Config->Genre);
		}
		snprintf(ypbuf, sizeof(ypbuf), "icy-genre:%s\n", ypbuf2);
		userData->Output->SendString(ypbuf);
		// send URL
		snprintf(ypbuf, sizeof(ypbuf), "icy-url:%s\n", userData->Config->ServerURL);
		userData->Output->SendString(ypbuf);
		// send IRC
		snprintf(ypbuf, sizeof(ypbuf), "icy-irc:%s\n", userData->Config->IRC);
		userData->Output->SendString(ypbuf);
		// send ICQ
		snprintf(ypbuf, sizeof(ypbuf), "icy-icq:%s\n", userData->Config->ICQ);
		userData->Output->SendString(ypbuf);
		// send AIM
		snprintf(ypbuf, sizeof(ypbuf), "icy-aim:%s\n", userData->Config->AIM);
		userData->Output->SendString(ypbuf);
		// send publicity
		snprintf(ypbuf, sizeof(ypbuf), "icy-pub:%u\n", userData->Config->Public ? 1 : 0);
		userData->Output->SendString(ypbuf);
		// send bitrate (this is a huge bad big ugly hack... needs to be fixed, but this works so far)
		snprintf(ypbuf, sizeof(ypbuf), "icy-br:%d\n", userData->Bitrate);
		userData->Output->SendString(ypbuf);
		// send content type (shouldn't be here as with the bitrate, but it works)
		snprintf(ypbuf, sizeof(ypbuf), "content-type:%s\n", userData->ContentType);
		userData->Output->SendString(ypbuf);
		// end our list of configurations
		userData->Output->SendString("\n");
		UNLOCK
		return OUT_SENDCONTENT;
	}
	}UNLOCK
	return state;
}

int SHOUTCAST_OUTPUT::Output_Send_InitFlush(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	userData->Output->FlushSend();
	createUvoxFrame(1, "0\0", MSG_FLUSH_CACHED_METADATA, userData);
	}UNLOCK
	return OUT_RECV_INITFLUSH;
}

int SHOUTCAST_OUTPUT::Output_Recv_InitFlush(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	Output_DUMMY(state, last_state, userData);
	}UNLOCK
	return OUT_SEND_INITSTANDBY;
}

int SHOUTCAST_OUTPUT::Output_Send_InitStandby(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	userData->Output->FlushSend();
	createUvoxFrame(1, "0\0", MSG_STANDBY, userData);
	}UNLOCK
	return OUT_RECV_INITSTANDBY;//OUT_SENDCONTENT;
}

int SHOUTCAST_OUTPUT::Output_Recv_InitStandby(int state, int last_state, T_OUTPUT *userData) {
	DEBUG_STATE
	STATE;
	LOCK{
	if (userData->Output->GetReceiveBytesAvailable() > 0) {
		memset(buf, '\0', sizeof(buf));
		userData->Output->ReceiveLine(buf, sizeof(buf));
		parseUvoxFrame(buf, out);
		if (checkUvoxFrameForError(out, state, userData) == -1) {
		UNLOCK
		return OUT_DISCONNECT;
		} else {
			UNLOCK
			return OUT_SENDCONTENT;//OUT_SEND_INTRO;
		}
	}
	}UNLOCK
	return OUT_SENDCONTENT;//OUT_SEND_INTRO;
}

/*int SHOUTCAST_OUTPUT::Output_Send_Intro(int state, T_OUTPUT *userData) {
	LOCK{
	}UNLOCK
	return OUT_RECV_INTRO; 
}

int SHOUTCAST_OUTPUT::Output_Recv_Intro(int state, T_OUTPUT *userData) {
	LOCK{
	}UNLOCK
	return OUT_SEND_BACKUP;
}

int SHOUTCAST_OUTPUT::Output_Send_Backup(int state, T_OUTPUT *userData) {
	LOCK{
	}UNLOCK
	return OUT_RECV_BACKUP; 
}

int SHOUTCAST_OUTPUT::Output_Recv_Backup(int state, T_OUTPUT *userData) {
	LOCK{
	}UNLOCK
	return OUT_SENDCONTENT;
}*/