#include "c_encoder_fhgaac.h"
#include "../../utils.h"

HINSTANCE C_ENCODER_FHGAAC::hEncoderInstance = NULL;

C_ENCODER_FHGAAC::C_ENCODER_FHGAAC(HWND winamp) : C_ENCODER_NSV(sizeof(T_ENCODER_FHGAAC_INFO)) {
	SetName("Fraunhofer Encoder");
	winampWnd = winamp;
	ConfigAudio3 = NULL;
	if(hEncoderInstance == NULL) {
		wchar_t dir[MAX_PATH] = {0};
		snwprintf(dir, MAX_PATH, L"%s\\enc_fhgaac.dll", GetPluginDirectoryW(winamp));
		hEncoderInstance = LoadLibraryW(dir);
	}

	if(hEncoderInstance) {
		void * CreateAudio3=(void *)GetProcAddress(hEncoderInstance, "CreateAudio3");
		void * GetAudioTypes3=(void *)GetProcAddress(hEncoderInstance, "GetAudioTypes3");
		void * ConfigAudio3=(void *)GetProcAddress(hEncoderInstance, "ConfigAudio3");
		void * SetWinampHWND=(void *)GetProcAddress(hEncoderInstance, "SetWinampHWND");
		SetEncoder(CreateAudio3,GetAudioTypes3,ConfigAudio3,SetWinampHWND,1);
	}

	T_ENCODER_FHGAAC_INFO * EncInfo = (T_ENCODER_FHGAAC_INFO *)ExtendedInfoPtr;
	EncInfo->output_bitRate = FHGAAC_DEFAULT_OUTPUTBITRATE;
	EncInfo->output_profile = FHGAAC_DEFAULT_OUTPUTPROFILE;
	EncInfo->output_surround = FHGAAC_DEFAULT_OUTPUTSURROUND;
}

C_ENCODER_FHGAAC::~C_ENCODER_FHGAAC() {
	C_ENCODER_NSV::~C_ENCODER_NSV();
}

static int cacheVal=0;
bool C_ENCODER_FHGAAC::isPresent(HWND winamp) {
	if(cacheVal!=0 && hEncoderInstance!=0) return cacheVal==2;
	bool ret=false;
	wchar_t dir[MAX_PATH] = {0};
	snwprintf(dir, MAX_PATH, L"%s\\enc_fhgaac.dll", GetPluginDirectoryW(winamp));
	FILE * f = _wfopen(dir, L"rb");
	if (f) {
		fseek(f,0,2);
		if(ftell(f) > 0) ret=true;
		fclose(f);
	}
	cacheVal=ret?2:1;
	return ret;
}

void C_ENCODER_FHGAAC::FillAttribs() {
	T_ENCODER_FHGAAC_INFO &EncInfo = *(T_ENCODER_FHGAAC_INFO *)ExtendedInfoPtr;
	T_ENCODER_FHGAAC_INFO *attribs = new T_ENCODER_FHGAAC_INFO;
	*attribs = EncInfo;
	AddAttrib("",attribs);
}

void C_ENCODER_FHGAAC::FillConfFile(char * conf_file, char * section) {
	if(!section) section="audio_adtsaac";

	T_ENCODER_FHGAAC_INFO &EncInfo = *(T_ENCODER_FHGAAC_INFO *)ExtendedInfoPtr;

	WritePrivateProfileInt("profile", EncInfo.output_profile, section, conf_file);
	WritePrivateProfileInt("bitrate", EncInfo.output_bitRate, section, conf_file);
	WritePrivateProfileInt("surround", EncInfo.output_surround, section, conf_file);
	WritePrivateProfileInt("shoutcast", 1, section, conf_file);
}

void C_ENCODER_FHGAAC::ReadConfFile(char * conf_file, char * section) {
	if(!section) section="audio_adtsaac";

	T_ENCODER_FHGAAC_INFO &EncInfo = *(T_ENCODER_FHGAAC_INFO *)ExtendedInfoPtr;
	T_ENCODER_FHGAAC_INFO *attribs = new T_ENCODER_FHGAAC_INFO;
	*attribs = EncInfo;

	attribs->output_profile = GetPrivateProfileInt(section,"profile",FHGAAC_DEFAULT_OUTPUTPROFILE,conf_file);
	attribs->output_bitRate = GetPrivateProfileInt(section,"bitrate",FHGAAC_DEFAULT_OUTPUTBITRATE,conf_file);
	attribs->output_surround = GetPrivateProfileInt(section,"surround",FHGAAC_DEFAULT_OUTPUTSURROUND,conf_file);

	ChangeSettings(attribs);
}