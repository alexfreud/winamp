#ifndef __SHOUTCAST_OUTPUT_H__
#define __SHOUTCAST_OUTPUT_H__

#include <time.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <fstream>
#include "c_serial_jobmanager.h"
#include "../Components/wac_network/wac_network_connection_api.h"
#include <WinSock2.h>

#include "../Encoders/c_encoder.h"
#include "../Encoders/c_encoder_mp3dll.h"

#include "../lame/include/lame.h"
#include "../lame/libmp3lame/lame_global_flags.h"
#include "../uvAuth21/uvAuth21.h"

#define UV_SYNC_BYTE 0x5A
#define UV_RESERVED 0x00
#define UV_END 0x00
#define UV_END_LEN 1
#define UV_HEADER_LEN 6
#define UV_META_LEN 6
#define UV_FRAME_LEN 16384
#define UV_MAX_DATA_LEN (UV_FRAME_LEN - UV_HEADER_LEN - UV_END_LEN)
#define UV_MAX_META_LEN (UV_FRAME_LEN - UV_HEADER_LEN - UV_META_LEN - UV_END_LEN)
#define UV_MAX_META_FRAGMENTS 32
#define UV_MAX_TOTAL_META_LEN (UV_MAX_META_LEN * UV_MAX_META_FRAGMENTS)

typedef struct {
	char* name;
	bool parent;
	bool children;
} SCgenres;
static SCgenres genres[] = {{"Alternative", true, true},
							{"Adult Alternative", false},
							{"Britpop", false},
							{"Classic Alternative", false},
							{"College", false},
							{"Dancepunk", false},
							{"Dream Pop", false},
							{"Emo", false},
							{"Goth", false},
							{"Grunge", false},
							{"Hardcore", false},
							{"Indie Pop", false},
							{"Indie Rock", false},
							{"Industrial", false},
							{"LoFi", false},
							{"Modern Rock", false},
							{"New Wave", false},
							{"Noise Pop", false},
							{"Post Punk", false},
							{"Power Pop", false},
							{"Punk", false},
							{"Ska", false},
							{"Xtreme", false},

							{"Blues", true, true},
							{"Acoustic Blues", false},
							{"Cajun and Zydeco", false},
							{"Chicago Blues", false},
							{"Contemporary Blues", false},
							{"Country Blues", false},
							{"Delta Blues", false},
							{"Electric Blues", false},

							{"Classical", true, true},
							{"Baroque", false},
							{"Chamber", false},
							{"Choral", false},
							{"Classical Period", false},
							{"Early Classical", false},
							{"Impressionist", false},
							{"Modern", false},
							{"Opera", false},
							{"Piano", false},
							{"Romantic", false},
							{"Symphony", false},

							{"Country", true, true},
							{"Alt Country", false},
							{"Americana", false},
							{"Bluegrass", false},
							{"Classic Country", false},
							{"Contemporary Bluegrass", false},
							{"Contemporary Country", false},
							{"Honky Tonk", false},
							{"Hot Country Hits", false},
							{"Western", false},

							{"Decades", true, true},
							{"30s", false},
							{"40s", false},
							{"50s", false},
							{"60s", false},
							{"70s", false},
							{"80s", false},
							{"90s", false},
							{"00s", false},

							{"Easy Listening", true, true},
							{"Exotica", false},
							{"Light Rock", false},
							{"Lounge", false},
							{"Orchestral Pop", false},
							{"Polka", false},
							{"Space Age Pop", false},

							{"Electronic", true, true},
							{"Acid House", false},
							{"Ambient", false},
							{"Big Beat", false},
							{"Breakbeat", false},
							{"Dance", false},
							{"Demo", false},
							{"Disco", false},
							{"Downtempo", false},
							{"Drum and Bass", false},
							{"Dubstep", false},
							{"Electro", false},
							{"Garage", false},
							{"Hard House", false},
							{"House", false},
							{"IDM", false},
							{"Jungle", false},
							{"Progressive", false},
							{"Techno", false},
							{"Trance", false},
							{"Tribal", false},
							{"Trip Hop", false},

							{"Folk", true, true},
							{"Alternative Folk", false},
							{"Contemporary Folk", false},
							{"Folk Rock", false},
							{"New Acoustic", false},
							{"Old Time", false},
							{"Traditional Folk", false},
							{"World Folk", false},

							{"Inspirational", true, true},
							{"Christian", false},
							{"Christian Metal", false},
							{"Christian Rap", false},
							{"Christian Rock", false},
							{"Classic Christian", false},
							{"Contemporary Gospel", false},
							{"Gospel", false},
							{"Praise and Worship", false},
							{"Sermons and Services", false},
							{"Southern Gospel", false},
							{"Traditional Gospel", false},

							{"International", true, true},
							{"African", false},
							{"Afrikaans", false},
							{"Arabic", false},
							{"Asian", false},
							{"Bollywood", false},
							{"Brazilian", false},
							{"Caribbean", false},
							{"Celtic", false},
							{"Creole", false},
							{"European", false},
							{"Filipino", false},
							{"French", false},
							{"German", false},
							{"Greek", false},
							{"Hawaiian and Pacific", false},
							{"Hebrew", false},
							{"Hindi", false},
							{"Indian", false},
							{"Islamic", false},
							{"Japanese", false},
							{"Klezmer", false},
							{"Korean", false},
							{"Mediterranean", false},
							{"Middle Eastern", false},
							{"North American", false},
							{"Russian", false},
							{"Soca", false},
							{"South American", false},
							{"Tamil", false},
							{"Turkish", false},
							{"Worldbeat", false},
							{"Zouk", false},

							{"Jazz", true, true},
							{"Acid Jazz", false},
							{"Avant Garde", false},
							{"Big Band", false},
							{"Bop", false},
							{"Classic Jazz", false},
							{"Cool Jazz", false},
							{"Fusion", false},
							{"Hard Bop", false},
							{"Latin Jazz", false},
							{"Smooth Jazz", false},
							{"Swing", false},
							{"Vocal Jazz", false},
							{"World Fusion", false},

							{"Latin", true, true},
							{"Bachata", false},
							{"Banda", false},
							{"Bossa Nova", false},
							{"Cumbia", false},
							{"Flamenco", false},
							{"Latin Dance", false},
							{"Latin Pop", false},
							{"Latin Rap and Hip Hop", false},
							{"Latin Rock", false},
							{"Mariachi", false},
							{"Merengue", false},
							{"Ranchera", false},
							{"Reggaeton", false},
							{"Regional Mexican", false},
							{"Salsa", false},
							{"Samba", false},
							{"Tango", false},
							{"Tejano", false},
							{"Tropicalia", false},

							{"Metal", true, true},
							{"Black Metal", false},
							{"Classic Metal", false},
							{"Death Metal", false},
							{"Extreme Metal", false},
							{"Grindcore", false},
							{"Hair Metal", false},
							{"Heavy Metal", false},
							{"Metalcore", false},
							{"Power Metal", false},
							{"Progressive Metal", false},
							{"Rap Metal", false},
							{"Thrash Metal", false},

							{"Misc", true, false},

							{"New Age", true, true},
							{"Environmental", false},
							{"Ethnic Fusion", false},
							{"Healing", false},
							{"Meditation", false},
							{"Spiritual", false},

							{"Pop", true, true},
							{"Adult Contemporary", false},
							{"Barbershop", false},
							{"Bubblegum Pop", false},
							{"Dance Pop", false},
							{"Idols", false},
							{"JPOP", false},
							{"KPOP", false},
							{"Oldies", false},
							{"Soft Rock", false},
							{"Teen Pop", false},
							{"Top 40", false},
							{"World Pop", false},

							{"Public Radio", true, true},
							{"College", false},
							{"News", false},
							{"Sports", false},
							{"Talk", false},
							{"Weather", false},

							{"R&B and Urban", true, false},
							{"Classic R&B", false},
							{"Contemporary R&B", false},
							{"Doo Wop", false},
							{"Funk", false},
							{"Motown", false},
							{"Neo Soul", false},
							{"Quiet Storm", false},
							{"Soul", false},
							{"Urban Contemporary", false},

							{"Rap", true, true},
							{"Alternative Rap", false},
							{"Dirty South", false},
							{"East Coast Rap", false},
							{"Freestyle", false},
							{"Gangsta Rap", false},
							{"Hip Hop", false},
							{"Mixtapes", false},
							{"Old School", false},
							{"Turntablism", false},
							{"Underground Hip Hop", false},
							{"West Coast Rap", false},

							{"Reggae", true, true},
							{"Contemporary Reggae", false},
							{"Dancehall", false},
							{"Dub", false},
							{"Pop Reggae", false},
							{"Ragga", false},
							{"Reggae Roots", false},
							{"Rock Steady", false},

							{"Rock", true, true},
							{"Adult Album Alternative", false},
							{"British Invasion", false},
							{"Celtic Rock", false},
							{"Classic Rock", false},
							{"Garage Rock", false},
							{"Glam", false},
							{"Hard Rock", false},
							{"Jam Bands", false},
							{"JROCK", false},
							{"Piano Rock", false},
							{"Prog Rock", false},
							{"Psychedelic", false},
							{"Rock & Roll", false},
							{"Rockabilly", false},
							{"Singer and Songwriter", false},
							{"Surf", false},

							{"Seasonal and Holiday", true, true},
							{"Anniversary", false},
							{"Birthday", false},
							{"Christmas", false},
							{"Halloween", false},
							{"Hanukkah", false},
							{"Honeymoon", false},
							{"Kwanzaa", false},
							{"Valentine", false},
							{"Wedding", false},
							{"Winter", false},

							{"Soundtracks", true, true},
							{"Anime", false},
							{"Kids", false},
							{"Original Score", false},
							{"Showtunes", false},
							{"Video Game Music", false},

							{"Talk", true, true},
							{"BlogTalk", false},
							{"Comedy", false},
							{"Community", false},
							{"Educational", false},
							{"Government", false},
							{"News", false},
							{"Old Time Radio", false},
							{"Other Talk", false},
							{"Political", false},
							{"Scanner", false},
							{"Spoken Word", false},
							{"Sports", false},
							{"Technology", false},

							{"Themes", true, true},
							{"Adult", false},
							{"Best Of", false},
							{"Chill", false},
							{"Eclectic", false},
							{"Experimental", false},
							{"Female", false},
							{"Heartache", false},
							{"Instrumental", false},
							{"LGBT", false},
							{"Love and Romance", false},
							{"Party Mix", false},
							{"Patriotic", false},
							{"Rainy Day Mix", false},
							{"Reality", false},
							{"Sexy", false},
							{"Shuffle", false},
							{"Travel Mix", false},
							{"Tribute", false},
							{"Trippy", false},
							{"Work Mix", false}
};

// pulled from nmrCommon\intTypes.h
typedef unsigned char 	__uint8;
typedef unsigned short 	__uint16;
typedef unsigned int 	__uint32;
typedef unsigned long long __uint64;

#pragma pack(push,1)

// this structure should be 16384 bytes in total size
// and is defined in full size so that we know its ok
struct uv2xHdr
{ // uvox2 message
	__uint8		sync;
	__uint8		qos;
	__uint16	msgType;
	__uint16	msgLen;
	__uint8		m_data[UV_MAX_DATA_LEN];
	__uint8		end;
};

struct uv2xMetadataHdr 
{   /* uvox 2 metadata header */
	__uint8		sync;
	__uint8		qos;
	__uint16	msgType;
	__uint16	msgLen;

	__uint16	id;		/* ID (cookie) identifying a metadata package */
	__uint16	span;	/* Span of messages in the metadata package being assembled */
	__uint16	index;	/* Index of the message in the metadata package being assembled */

	__uint8		m_data[UV_MAX_META_LEN];
	__uint8		end;
};

#pragma pack(pop)

#define MSG_AUTH					0x1001
#define MSG_BROADCAST_SETUP			0x1002
#define MSG_NEGOTIATE_BUFFER_SIZE	0x1003
#define MSG_STANDBY					0x1004
#define MSG_TERMINATE				0x1005
#define MSG_FLUSH_CACHED_METADATA	0x1006
#define MSG_LISTENER_AUTHENTICATION	0x1007
#define MSG_MAX_PAYLOAD_SIZE		0x1008
#define MSG_CIPHER					0x1009
#define MSG_MIME_TYPE				0x1040
#define MSG_FILE_TRANSFER_BEGIN		0x1050
#define MSG_FILE_TRANSFER_DATA		0x1051

#define MSG_BROADCAST_INTERRUPTION	0x2001
#define MSG_BROADCAST_TERMINATE		0x2002

#define MSG_ICYNAME					0x1100
#define MSG_ICYGENRE				0x1101
#define MSG_ICYURL					0x1102
#define MSG_ICYPUB					0x1103

#define MSG_METADATA_CONTENTINFO	0x3000
#define MSG_METADATA_URL			0x3001
#define MSG_METADATA_XML			0x3901
#define MSG_METADATA_XML_NEW		0x3902

// only id the start of the album art type as it's variable
#define MSG_METADATA_ALBUMART		0x4000
#define MSG_METADATA_STATION_ART	0x0000
#define MSG_METADATA_PLAYING_ART	0x0100
/*
	0x4    0x0xx    Station logo
	0x4    0x1xx    Album art

	00 = image/jpeg
	01 = image/png
	02 = image/bmp
	03 = image/gif
*/

#define MSG_METADATA_TIMEREMAINING	0x5001

#define MP3_DATA					0x7000
#define VLB_DATA					0x8000
#define AAC_LC_DATA					0x8001
#define AACP_DATA					0x8003
#define OGG_DATA					0x8004

struct T_OUTPUT_CONFIG {
	char Name[32];
	wchar_t DisplayName[32];
	char UserID[256];
	char Address[1024];
	u_short Port;
	char StationID[12];
	char Password[256];		// 4 - 8 for 1.8.2
	char cipherkey[64];		// sc2 cipherkey
	int AutoRecon;
	int ReconTime;
	char Description[1024];
	char ServerURL[2048];
	char Genre[256];
	char ICQ[128];
	char AIM[1024];
	char IRC[1024];
	int Public;
	int doTitleUpdate;
	int protocol;
	int protocol_retry;
	char Now[1024];
	char Next[1024];
};

#define DEFAULT_ENCODER (C_ENCODER *)(-1)

#define OM_ENCODE 1
#define OM_OUTPUT 2
#define OM_OTHER 4
#define OM_ALL (OM_ENCODE | OM_OUTPUT | OM_OTHER)

enum OUTPUTTYPE {
	OUTTYPE_SOURCE,
	OUTTYPE_TITLE,
};

typedef unsigned long ARGB32;

struct T_OUTPUT_TITLE {
	wchar_t *Title;
	wchar_t *Song;
	wchar_t *Album;
	wchar_t *Artist;
	wchar_t *Genre;
	wchar_t *Comment;
	wchar_t *Year;
	std::vector<std::wstring> NextList;
	void *APIC[2];
	int APICLength[2];
	int APICType[2];

	T_OUTPUT_TITLE() : Title(0), Song(0), Album(0), Artist(0), Genre(0), Comment(0), Year(0)
	{
		memset(APIC, 0, sizeof(void *) * 2);
		memset(APICLength, 0, sizeof(int) * 2);
		memset(APICType, 0, sizeof(int) * 2);
	}


	~T_OUTPUT_TITLE()
	{
		if (Title)
		{
			free(Title);
			Title = 0;
		}

		if (Song)
		{
			free(Song);
			Song = 0;
		}

		if (Album)
		{
			free(Album);
			Album = 0;
		}

		if (Artist)
		{
			free(Artist);
			Artist = 0;
		}

		if (Genre)
		{
			free(Genre);
			Genre = 0;
		}

		if (Comment)
		{
			free(Comment);
			Comment = 0;
		}

		if (Year)
		{
			free(Year);
			Year = 0;
		}
	}
};

struct T_OUTPUT_INFO {
	unsigned int BytesSent;	// how many bytes of content we've sent
	clock_t ConnectionTime;	// time a socket connection occurred
	int Version;			// server version
	int Caps;				// server capabilities
	int Reconnect;			// flag for the reconnection algorithm
	int ReconnectTime;		// value used in conjunction with the reconnection algorithm
	int Succeeded;			// had at least one successful connection (for reconnection alg.)  -1 = password failure
	int last_state;			// using this as a means to allow for closing on errors but able to show a better message
	int Switching;			// if we're doing an automatic protocol version change (from v2 to v1)
	char *ErrorMsg;
	time_t ConnectedAt;
	int meta_cached;
	int art_cached[2];
	unsigned short art_index[2];
	unsigned short art_cached_span[2];
	int art_cached_length[2];

	// metadata information about the stream, etc
	wchar_t *Title;
	std::vector<std::wstring> NextList;
	wchar_t *Song;
	wchar_t *Album;
	wchar_t *Artist;
	wchar_t *Genre;
	wchar_t *Comment;
	wchar_t *Year;
	void *APIC[2];
	int APICLength[2];
	int APICType[2];

	T_OUTPUT_INFO() : BytesSent(0), ConnectionTime(0), Version(0),
					  Caps(0), Reconnect(0), ReconnectTime(0),
					  Succeeded(0), last_state(0), Switching(0),
					  ErrorMsg(0), ConnectedAt(0), meta_cached(0),
					  Title(0), Song(0), Album(0), Artist(0),
					  Genre(0), Comment(0), Year(0)
	{
		memset(art_cached, 0, sizeof(int) * 2);
		memset(art_index, 0, sizeof(unsigned short) * 2);
		memset(art_cached_span, 0, sizeof(unsigned short) * 2);
		memset(art_cached_length, 0, sizeof(int) * 2);

		memset(APIC, 0, sizeof(void *) * 2);
		memset(APICLength, 0, sizeof(int) * 2);
		memset(APICType, 0, sizeof(int) * 2);
	}

	~T_OUTPUT_INFO()
	{
		if (Title)
		{
			free(Title);
			Title = 0;
		}

		if (Song)
		{
			free(Song);
			Song = 0;
		}

		if (Album)
		{
			free(Album);
			Album = 0;
		}

		if (Artist)
		{
			free(Artist);
			Artist = 0;
		}

		if (Genre)
		{
			free(Genre);
			Genre = 0;
		}

		if (Comment)
		{
			free(Comment);
			Comment = 0;
		}

		if (Year)
		{
			free(Year);
			Year = 0;
		}

		if (Succeeded == -2 && ErrorMsg) {
			free(ErrorMsg);
			ErrorMsg = 0;
		}
	}
};

struct T_OUTPUT {
	int Connection;			// using this for the title update callback so the correct instance is updated
	void (*TitleCallback)(const int Connection, const int Mode);
	api_connection *Output;
	enum OUTPUTTYPE Type;
	int Bitrate;			// this shouldn't be here, but it's the only way we can tell the shoutcast server the bitrate
	char *ContentType;		// neither should this
	int SlowClose;			// set to 1 to wait until all data is sent before closing the connection
	T_OUTPUT_CONFIG *Config;
	T_OUTPUT_INFO Info;

	T_OUTPUT() : Connection(0), TitleCallback(0), Output(0), Type(OUTTYPE_SOURCE), Bitrate(0), ContentType(0), SlowClose(0), Config(0) {}
};

enum OUTPUTSTATE {
	OUT_ERROR,				// not a true state, but is returned when GetState() is called with an invalid connection handle
	OUT_DISCONNECTED,
	OUT_CONNECT,
	OUT_REQUEST_CIPHER,
	OUT_RECV_CIPHER,
	OUT_SENDAUTH,
	OUT_RECVAUTHRESPONSE,
	OUT_SEND_MIME,
	OUT_RECV_MIME,
	OUT_SEND_BITRATE,
	OUT_RECV_BITRATE,
	OUT_SEND_BUFSIZE,
	OUT_RECV_BUFSIZE,
	OUT_SEND_MAX,
	OUT_RECV_MAX,
	OUT_SENDYP,
	OUT_SEND_INITFLUSH,
	OUT_RECV_INITFLUSH,
	OUT_SEND_INITSTANDBY,
	OUT_RECV_INITSTANDBY,
	/*OUT_SEND_INTRO,
	OUT_RECV_INTRO,
	OUT_SEND_BACKUP,
	OUT_RECV_BACKUP,*/
	OUT_SENDCONTENT,
	OUT_DISCONNECT,
	OUT_RECONNECT,
	OUT_TITLESENDUPDATE,
	OUT_FAIL_CIPHER,
	OUT_SEND_METADATA,
	OUT_SEND_ARTWORK,
};
static void *mutex;

class SHOUTCAST_OUTPUT {
private:
	C_ENCODER *Encoder;
	int IOwnEncoder;
	C_SERIAL_JOBMANAGER<T_OUTPUT> OutputManager;
	T_OUTPUT_TITLE metadata;

protected:
	static int Output_Disconnected(int state, int last_state, T_OUTPUT *userData);
	static int Output_Connect(int state, int last_state, T_OUTPUT *userData);
	static int Output_Request_Cipher(int state, int last_state, T_OUTPUT *userData);
	static int Output_Receive_Cipher(int state, int last_state, T_OUTPUT *userData);
	static int Output_SendAuth(int state, int last_state, T_OUTPUT *userData);
	static int Output_RecvAuthResponse(int state, int last_state, T_OUTPUT *userData);
	static int Output_Send_Mime(int state, int last_state, T_OUTPUT *userData);
	static int Output_Recv_Mime(int state, int last_state, T_OUTPUT *userData);
	static int Output_Send_Bitrate(int state, int last_state, T_OUTPUT *userData);
	static int Output_Recv_Bitrate(int state, int last_state, T_OUTPUT *userData);
	static int Output_Send_Buf_Size(int state, int last_state, T_OUTPUT *userData);
	static int Output_Recv_Buf_Size(int state, int last_state, T_OUTPUT *userData);
	static int Output_Send_Max_Size(int state, int last_state, T_OUTPUT *userData);
	static int Output_Recv_Max_Size(int state, int last_state, T_OUTPUT *userData);
	static int Output_DUMMY(int state, int last_state, T_OUTPUT *userData);
	static int Output_SendYP(int state, int last_state, T_OUTPUT *userData);
	static int Output_Send_InitFlush(int state, int last_state, T_OUTPUT *userData);
	static int Output_Recv_InitFlush(int state, int last_state, T_OUTPUT *userData);
	static int Output_Send_InitStandby(int state, int last_state, T_OUTPUT *userData);
	static int Output_Recv_InitStandby(int state, int last_state, T_OUTPUT *userData);
	/*static int Output_Send_Intro(int state, int last_state, T_OUTPUT *userData);
	static int Output_Recv_Intro(int state, int last_state, T_OUTPUT *userData);
	static int Output_Send_Backup(int state, int last_state, T_OUTPUT *userData);
	static int Output_Recv_Backup(int state, int last_state, T_OUTPUT *userData);*/
	static int Output_SendContent(int state, int last_state, T_OUTPUT *userData);
	static int Output_Disconnect(int state, int last_state, T_OUTPUT *userData);
	static int Output_Reconnect(int state, int last_state, T_OUTPUT *userData);

	static int Output_Title_SendUpdate(int state, int last_state, T_OUTPUT *userData);
	static int Output_Send_Metadata(int state, int last_state, T_OUTPUT *userData);
	static int Output_Send_Artwork(int state, int last_state, T_OUTPUT *userData);

	// uvox21
	static void createUvoxFrame(int length, char *payload_in, int type, T_OUTPUT *userData);
    static int createUvoxMetaFrame(int length, char *payload_in, int type,
								   T_OUTPUT *userData, unsigned short id, unsigned short span = 1);
	static int parseUvoxFrame(char *payload_in, char *payload_out);
	static int checkUvoxFrameForError(char *pload_out, int state, T_OUTPUT *userData);

	void (*lame_init)(void);
	void (*lame_init_params)(lame_global_flags *);
	int (*lame_encode_buffer_interleaved)(lame_global_flags *, short int pcm[], int num_samples, char *mp3buffer, int mp3buffer_size);
	int (*lame_encode_flush)(lame_global_flags *, char *mp3buffer, int size);

	HINSTANCE libinst;

public:
	SHOUTCAST_OUTPUT();
	void SetLame(void *init, void *params, void *encode, void *finish);
	~SHOUTCAST_OUTPUT();
	int Run(int mode = 0, void *Input = NULL, int InputSize = 0, int SaveEncoder = -1);
	int AddOutput(int Connection, T_OUTPUT_CONFIG *Config, void (*TitleCallback)(const int Connection, const int Mode)=0);
	void UpdateOutput(int Connection);
	void RemoveOutput(int Connection);
	int ConnectOutput(int Connection);
	int DisconnectOutput(int Connection, int withReconnect = 0, int reconnectTime = -1); // withReconnect of -1 will use the output config's setting
	void SetEncoder(C_ENCODER *encoder, int takeOwnership = 0);

	// we will attempt to cache the title information to save on duplication and
	// also to make it easier for the title to be re-sent on server disconnect
	void UpdateTitleCache(wchar_t *Title, std::vector<std::wstring> NextList, wchar_t *Song,
						  wchar_t *Album, wchar_t *Artist, wchar_t *Genre, wchar_t *Comment,
						  wchar_t* Year, int Connection, bool sendNext);
	void UpdateTitle(wchar_t *Title, std::vector<std::wstring> NextList,
					 int Connection, bool sendNext, bool UseCache = true);

	void UpdateArtwork(int Connection);
	void UpdateAlbumArtCache(void* APIC, int APIClength, int APICType, int Connection);
	int UpdateAlbumArt(int Connection);

	enum OUTPUTSTATE GetState(int Connection);
	T_OUTPUT_CONFIG *operator[](int Connection);
	T_OUTPUT_CONFIG *GetOutput(int Connection);
	C_ENCODER *GetEncoder();
	T_OUTPUT_INFO *GetOutputInfo(int Connection);
};
static unsigned short mid = 1;

#ifdef _DEBUG
#define DEBUG_STATE OutputDebugString(__FUNCTION__); OutputDebugString("\r\n");
#else
#define DEBUG_STATE
#endif

#define STATE userData->Info.last_state = last_state
#define LOCK if(WaitForSingleObject(mutex,INFINITE) == WAIT_OBJECT_0)
#define UNLOCK ReleaseMutex(mutex);

extern char sourceVersion[64];
extern HWND hMainDLG;
#endif // !__SHOUTCAST_OUTPUT_H__