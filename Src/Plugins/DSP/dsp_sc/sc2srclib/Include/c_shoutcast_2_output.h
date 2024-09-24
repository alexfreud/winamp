#ifndef __C_SHOUTCAST_2_OUTPUT_H__
#define __C_SHOUTCAST_2_OUTPUT_H__

#include <time.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "c_serial_jobmanager.h"
#include "../jnetlib/jnetlib.h"
#include "../Encoders/c_encoder.h"
#include "../Encoders/c_encoder_mp3dll.h"

#include "../lame/include/lame.h"
#include "../lame/libmp3lame/lame_global_flags.h"
#include "../uvAuth21/uvAuth21.h"

#ifdef USEAACP
#include "../Encoders/c_encoder_aacp.h"
#endif
struct T_OUTPUT_CONFIG {
	char Name[32];
	char UserID[256];
	char Address[1024];
	u_short Port;
	char StationID[8];
	char Password[256]; // 4 - 8 for 1.8.2
	char cipherkey[32];// sc2 cipherkey
	int AutoRecon;
	int ReconTime;
	char Description[1024];
	char ServerURL[2048];
	int Genre1;
	int Genre2;
	char Genre3[1024];
	char ICQ[128];
	char AIM[512];
	char IRC[512];
	char content_type[11];
	int Public;
	int doTitleUpdate;
	int protocol;
	int DoUpload;
	char introfilepath[4096];
	char backupfile[4096];
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

struct T_OUTPUT_INFO {
	unsigned int BytesSent;  // how many bytes of content we've sent
	clock_t ConnectionTime;  // time a socket connection occurred
	int Version;  // server version
	int Caps;  // server capabilities
	int Reconnect;  // flag for the reconnection algorithm
	int ReconnectTime;  // value used in conjunction with the reconnection algorithm
	int Succeeded;  // had at least one successful connection (for reconnection alg.)  -1 = password failure
	int sc2Suceeded;//sc2 version
	char ErrorMsg[1024];
	time_t ConnectedAt;
	wchar_t Title[1024];
	wchar_t Next[1024];
	char URL[1024];
	int introuploaded;
	int backupuploaded;
};

struct T_OUTPUT {
	JNL_Connection Output;
	enum OUTPUTTYPE Type;
	int Bitrate;  // this shouldn't be here, but it's the only way we can tell the shoutcast server the bitrate
	char * ContentType; //neither should this
	int SlowClose;  // set to 1 to wait until all data is sent before closing the connection
	T_OUTPUT_CONFIG *Config;
	T_OUTPUT_INFO Info;
	int m_sendmetadata;
	int m_initdone;
};

enum OUTPUTSTATE {
	OUT_ERROR,  // not a true state, but is returned when GetState() is called with an invalid connection handle
	OUT_IDLE,
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
	OUT_RECVYP,
	OUT_SEND_INITFLUSH,
	OUT_RECV_INITFLUSH,
	OUT_SEND_INITSTANDBY,
	OUT_RECV_INITSTANDBY,
	OUT_SEND_INTRO,
	OUT_RECV_INTRO,
	OUT_SEND_BACKUP,
	OUT_RECV_BACKUP,
	OUT_SENDCONTENT,
	OUT_DISCONNECT,
	OUT_RECONNECT,
	OUT_TITLESENDUPDATE,
};
#define OUT_DISCONNECTED OUT_IDLE

class C_SHOUTCAST_2_OUTPUT {
private:
	C_ENCODER *Encoder;
	int IOwnEncoder;
	C_SERIAL_JOBMANAGER<T_OUTPUT> OutputManager;
	HANDLE mutex;

protected:
	static int Output_Idle(int state, T_OUTPUT *userData);
	static int Output_Connect(int state, T_OUTPUT *userData);
	static int Output_Request_Cipher(int state, T_OUTPUT *userData);
	static int Output_Receive_Cipher(int state, T_OUTPUT *userData);
	static int Output_SendAuth(int state, T_OUTPUT *userData);
	static int Output_RecvAuthResponse(int state, T_OUTPUT *userData);
	static int Output_Send_Mime(int state, T_OUTPUT *userData);
	static int Output_Recv_Mime(int state, T_OUTPUT *userData);
	static int Output_Send_Bitrate(int state, T_OUTPUT *userData);
	static int Output_Recv_Bitrate(int state, T_OUTPUT *userData);
	static int Output_Send_Buf_Size(int state, T_OUTPUT *userData);
	static int Output_Recv_Buf_Size(int state, T_OUTPUT *userData);
	static int Output_Send_Max_Size(int state, T_OUTPUT *userData);
	static int Output_Recv_Max_Size(int state, T_OUTPUT *userData);
	static int Output_DUMMY(int state, T_OUTPUT *userData);
	static int Output_SendYP(int state, T_OUTPUT *userData);
	static int Output_RecvYP(int state, T_OUTPUT *userData);
	static int Output_Send_InitFlush(int state, T_OUTPUT *userData);
	static int Output_Recv_InitFlush(int state, T_OUTPUT *userData);
	static int Output_Send_InitStandby(int state, T_OUTPUT *userData);
	static int Output_Recv_InitStandby(int state, T_OUTPUT *userData);
	static int Output_Send_InitMeta(int state, T_OUTPUT *userData);
	static int Output_Recv_InitMeta(int state, T_OUTPUT *userData);
	static int Output_Send_Intro(int state, T_OUTPUT *userData);
	static int Output_Recv_Intro(int state, T_OUTPUT *userData);
	static int Output_Send_Backup(int state, T_OUTPUT *userData);
	static int Output_Recv_Backup(int state, T_OUTPUT *userData);

	static int Output_SendContent(int state, T_OUTPUT *userData);
	static int Output_Disconnect(int state, T_OUTPUT *userData);
	static int Output_Reconnect(int state, T_OUTPUT *userData);
	static int Output_Title_SendUpdate(int state, T_OUTPUT *userData);
	static int Output_Title_SendUpdatev2(int state, T_OUTPUT *userData);
	// uvox21
	static char * createUvoxFrameClasstype(std::string typeString);
	static int createUvoxFrame(int length, char * payload_in,char * payload_out, char * classtype);
	static int parseUvoxFrame(char * payload_in,char * payload_out);
	static int checkUvoxFrameForError(char * pload_out,int state, T_OUTPUT *userData);

	void (*lame_init)(void);
	void (*lame_init_params)(lame_global_flags *);
	int (*lame_encode_buffer_interleaved)(lame_global_flags *,short int pcm[],int num_samples, char *mp3buffer,int  mp3buffer_size);
	int (*lame_encode_flush)(lame_global_flags *,char *mp3buffer, int size);

public:
	C_SHOUTCAST_2_OUTPUT();
	void SetLame(void *init, void *params, void *encode, void *finish);
	~C_SHOUTCAST_2_OUTPUT();
	int Run(int mode = 0, void *Input = NULL, int InputSize = 0);
	int AddOutput(T_OUTPUT_CONFIG *Config);
	void UpdateOutput(int Connection);
	void RemoveOutput(int Connection);
	void ConnectOutput(int Connection);
	void DisconnectOutput(int Connection, int withReconnect = 0, int reconnectTime = -1); // withReconnect of -1 will use the output config's setting
	void SetEncoder(C_ENCODER *encoder, int takeOwnership = 0);
	void UpdateTitle(wchar_t*Title,wchar_t*Next, int Connection,int titleseq);
	enum OUTPUTSTATE GetState(int Connection);
	T_OUTPUT_CONFIG *operator[](int Connection);
	T_OUTPUT_CONFIG *GetOutput(int Connection);
	C_ENCODER *GetEncoder();
	T_OUTPUT_INFO *GetOutputInfo(int Connection);
	int m_titleseq;
};

#endif // !__C_SHOUTCAST_2_OUTPUT_H__