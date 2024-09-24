#include "c_encoder_aacp.h"
#include "../../utils.h"

HINSTANCE C_ENCODER_AACP::hEncoderInstance = NULL;

C_ENCODER_AACP::C_ENCODER_AACP(HWND winamp) : C_ENCODER_NSV(sizeof(T_ENCODER_AACP_INFO)) {
	SetName("AAC+ Encoder");
	winampWnd = winamp;
	ConfigAudio3 = NULL;
	if(hEncoderInstance == NULL) {
		wchar_t dir[MAX_PATH] = {0};
		snwprintf(dir, MAX_PATH, L"%s\\enc_aacplus.dll", GetPluginDirectoryW(winamp));
		hEncoderInstance = LoadLibraryW(dir);
	}

	if(hEncoderInstance) {
		void * CreateAudio3=(void *)GetProcAddress(hEncoderInstance, "CreateAudio3");
		void * GetAudioTypes3=(void *)GetProcAddress(hEncoderInstance, "GetAudioTypes3");
		void * ConfigAudio3=(void *)GetProcAddress(hEncoderInstance, "ConfigAudio3");
		void * SetWinampHWND=(void *)GetProcAddress(hEncoderInstance, "SetWinampHWND");
		SetEncoder(CreateAudio3,GetAudioTypes3,ConfigAudio3,SetWinampHWND);
	}

	T_ENCODER_AACP_INFO * EncInfo = (T_ENCODER_AACP_INFO *)ExtendedInfoPtr;
	EncInfo->output_bitRate = AACP_DEFAULT_OUTPUTBITRATE;
	EncInfo->output_channelmode = AACP_DEFAULT_OUTPUTCHANNELMODE;
	EncInfo->output_quality = AACP_DEFAULT_OUTPUTQUALITY;
	EncInfo->output_samplerate = AACP_DEFAULT_OUTPUTSAMPLERATE;
	EncInfo->output_v2enable = AACP_DEFAULT_OUTPUTV2ENABLE;
}

C_ENCODER_AACP::~C_ENCODER_AACP() {
	C_ENCODER_NSV::~C_ENCODER_NSV();
}

static int cacheVal=0;
bool C_ENCODER_AACP::isPresent(HWND winamp) {
	if(cacheVal!=0 && hEncoderInstance!=0) return cacheVal==2;
	bool ret=false;
	wchar_t dir[MAX_PATH] = {0};
	snwprintf(dir, MAX_PATH, L"%s\\enc_aacplus.dll", GetPluginDirectoryW(winamp));
	FILE * f = _wfopen(dir, L"rb");
	if (f) {
		fseek(f,0,2);
		if(ftell(f) > 0) ret=true;
		fclose(f);
	}
	cacheVal=ret?2:1;
	return ret;
}

void C_ENCODER_AACP::FillAttribs() {
	T_ENCODER_AACP_INFO &EncInfo = *(T_ENCODER_AACP_INFO *)ExtendedInfoPtr;
	T_ENCODER_AACP_INFO *attribs = new T_ENCODER_AACP_INFO;
	*attribs = EncInfo;
	AddAttrib("",attribs);
}

void C_ENCODER_AACP::FillConfFile(char * conf_file, char * section) {
	if(!section) section="audio_aacplus";

	T_ENCODER_AACP_INFO &EncInfo = *(T_ENCODER_AACP_INFO *)ExtendedInfoPtr;

	WritePrivateProfileInt("samplerate", EncInfo.output_samplerate, section, conf_file);
	WritePrivateProfileInt("channelmode", EncInfo.output_channelmode, section, conf_file);
	WritePrivateProfileInt("bitrate", EncInfo.output_bitRate * 1000, section, conf_file);
	WritePrivateProfileInt("v2enable", EncInfo.output_v2enable, section, conf_file);
}

void C_ENCODER_AACP::ReadConfFile(char * conf_file, char * section) {
	if(!section) section="audio_aacplus";

	T_ENCODER_AACP_INFO &EncInfo = *(T_ENCODER_AACP_INFO *)ExtendedInfoPtr;
	T_ENCODER_AACP_INFO *attribs = new T_ENCODER_AACP_INFO;
	*attribs = EncInfo;

	attribs->output_samplerate = GetPrivateProfileInt(section,"samplerate",AACP_DEFAULT_OUTPUTSAMPLERATE,conf_file);
	attribs->output_channelmode = GetPrivateProfileInt(section,"channelmode",AACP_DEFAULT_OUTPUTCHANNELMODE,conf_file);
	attribs->output_bitRate = GetPrivateProfileInt(section,"bitrate",AACP_DEFAULT_OUTPUTBITRATE,conf_file)/1000;
	attribs->output_quality = GetPrivateProfileInt(section,"quality",AACP_DEFAULT_OUTPUTQUALITY,conf_file);
	attribs->output_v2enable = GetPrivateProfileInt(section,"v2enable",AACP_DEFAULT_OUTPUTV2ENABLE,conf_file);

	ChangeSettings(attribs);
}