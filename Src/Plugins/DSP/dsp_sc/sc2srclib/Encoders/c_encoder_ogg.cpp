#include "c_encoder_ogg.h"
#include "../../utils.h"

HINSTANCE C_ENCODER_OGG::hEncoderInstance = NULL;

C_ENCODER_OGG::C_ENCODER_OGG(HWND winamp) : C_ENCODER_NSV(sizeof(T_ENCODER_OGG_INFO)) {
	SetName("OGG Vorbis Encoder");
	winampWnd = winamp;
	ConfigAudio3 = NULL;
	if(hEncoderInstance == NULL) {
		wchar_t dir[MAX_PATH] = {0};
		snwprintf(dir, MAX_PATH, L"%s\\enc_vorbis.dll", GetPluginDirectoryW(winamp));
		hEncoderInstance = LoadLibraryW(dir);
	}

	if(hEncoderInstance) {
		void * CreateAudio3=(void *)GetProcAddress(hEncoderInstance, "CreateAudio3");
		void * GetAudioTypes3=(void *)GetProcAddress(hEncoderInstance, "GetAudioTypes3");
		void * ConfigAudio3=(void *)GetProcAddress(hEncoderInstance, "ConfigAudio3");
		void * SetWinampHWND=(void *)GetProcAddress(hEncoderInstance, "SetWinampHWND");
		SetEncoder(CreateAudio3,GetAudioTypes3,ConfigAudio3,SetWinampHWND);
	}

	T_ENCODER_OGG_INFO * EncInfo = (T_ENCODER_OGG_INFO *)ExtendedInfoPtr;
	EncInfo->output_bitRate = OGG_DEFAULT_OUTPUTBITRATE;
	EncInfo->output_channelmode = OGG_DEFAULT_OUTPUTMODE;
	EncInfo->output_samplerate = OGG_DEFAULT_OUTPUTSAMPLERATE;
}

C_ENCODER_OGG::~C_ENCODER_OGG() {
	C_ENCODER_NSV::~C_ENCODER_NSV();
}

static int cacheVal=0;
bool C_ENCODER_OGG::isPresent(HWND winamp) {
	if(cacheVal!=0 && hEncoderInstance!=0) return cacheVal==2;
	bool ret=false;
	wchar_t dir[MAX_PATH] = {0};
	snwprintf(dir, MAX_PATH, L"%s\\enc_vorbis.dll", GetPluginDirectoryW(winamp));
	FILE * f = _wfopen(dir, L"rb");
	if (f) {
		fseek(f,0,2);
		if(ftell(f) > 0) ret=true;
		fclose(f);
	}
	cacheVal=ret?2:1;
	return ret;
}

void C_ENCODER_OGG::FillAttribs() {
	T_ENCODER_OGG_INFO &EncInfo = *(T_ENCODER_OGG_INFO *)ExtendedInfoPtr;
	T_ENCODER_OGG_INFO *attribs = new T_ENCODER_OGG_INFO;
	*attribs = EncInfo;
	AddAttrib("",attribs);
}

void C_ENCODER_OGG::FillConfFile(char * conf_file, char * section) {
	if(!section) section="audio_ogg";

	T_ENCODER_OGG_INFO &EncInfo = *(T_ENCODER_OGG_INFO *)ExtendedInfoPtr;
	configtype * cfg = new configtype;
	cfg->cfg_abr_use_max=0;
	cfg->cfg_abr_use_min=0;
	cfg->cfg_mode=0; //VBR
	cfg->cfg_vbrquality=EncInfo.output_quality;
	cfg->cfg_abr_nominal=EncInfo.output_bitRate;
	cfg->cfg_abr_max=EncInfo.output_bitRate;
	cfg->cfg_abr_min=EncInfo.output_bitRate;

	if (conf_file) WritePrivateProfileStruct(section,"conf",cfg,sizeof(configtype),conf_file);
}
int setBitrate(float ql)
{
	int br = 64;
	//ql = ql*10;
	// jkey: this is a pain in the ass,but the only
	// way i can figure out how to get the bitrate
	// outside of enc_vorbis.
	// Also quality enforcement is needed to prevent the
	// yp filling up with non standard bitrate streams.
	// although this is vbr and will be variable bitrate anyway.
	if(ql == 10 || (ql < 10 && ql > 9.5)){br=500;ql = 10.0f; return br;}
	if(ql == 9.0f || (ql < 10.0f && ql > 9.0f)){br=320;ql = 9.0f;return br;}
	if(ql == 8.0f || (ql < 9.0f && ql > 8.0f)){br=256;ql = 8.0f;return br;}
	if(ql == 7.0f || (ql < 8.0f && ql > 7.0f)){br=224;ql = 7.0f;return br;}
	if(ql == 6.0f || (ql < 7.0f && ql > 6.0f)){br=192;ql = 6.0f;return br;}
	if(ql == 5.0f || (ql < 6.0f && ql > 5.0f)){br=160;ql = 5.0f;return br;}
	if(ql == 4.0f || (ql < 5.0f && ql > 4.0f)){br=128;ql = 4.0f;return br;}
	if(ql == 3.0f || (ql < 4.0f && ql > 3.0f)){br=112;ql = 3.0f;return br;}
	if(ql == 2.0f || (ql < 3.0f && ql > 2.0f)){br=96;ql = 2.0f;return br;}
	if(ql == 1.0f || (ql < 2.0f && ql > 1.0f)){br=80;ql = 1.0f;return br;}
	if(ql == 0.0f || (ql < 1.0f && ql > 0.0f)){ br=64;ql = 0.0f;return br;}
	if(ql == -0.5f || (ql < 0.0f && ql > -0.5f)){br=56;ql = -0.5f;return br;}
	if(ql == -1.0f || ql < -0.5f){br=48;ql = -1.0f;return br;}
	return br;
}
void C_ENCODER_OGG::ReadConfFile(char * conf_file, char * section) {
	if(!section) section="audio_ogg";
	T_ENCODER_OGG_INFO &EncInfo = *(T_ENCODER_OGG_INFO *)ExtendedInfoPtr;
	T_ENCODER_OGG_INFO *attribs = new T_ENCODER_OGG_INFO;
	*attribs = EncInfo;
	configtype * cfg = new configtype;
	cfg->cfg_abr_use_max=0;
	cfg->cfg_abr_use_min=0;
	cfg->cfg_mode=0; //VBR
	cfg->cfg_vbrquality=0.0f;
	cfg->cfg_abr_nominal=64;
	cfg->cfg_abr_max=352;
	cfg->cfg_abr_min=32;

	if (conf_file) GetPrivateProfileStruct(section,"conf",cfg,sizeof(configtype),conf_file);
	attribs->output_samplerate = OGG_DEFAULT_OUTPUTSAMPLERATE;
	attribs->output_channelmode = cfg->cfg_mode;
	attribs->output_quality = cfg->cfg_vbrquality;
	attribs->output_bitRate = setBitrate(attribs->output_quality*10);
	ChangeSettings(attribs);
}