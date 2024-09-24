#include "c_encoder_nsv.h"
#include "../../utils.h"
#include <mmsystem.h>
#include <stdio.h>

static char * configfile;
static unsigned int configfourcc;
static HWND (*ConfigAudio3)(HWND intParent, HINSTANCE hinst, unsigned int outt, char *configfile);
static HINSTANCE encoderDllInstance;
static HWND cfgwnd=NULL;
C_ENCODER_NSV::C_ENCODER_NSV(int ExtInfoSize) : C_ENCODER(ExtInfoSize) {
	hMutex = CreateMutex(NULL,TRUE,NULL);
	ReleaseMutex(hMutex);
	CreateAudio3 = NULL;
	ConfigAudio3 = NULL;
	GetAudioTypes3 = NULL;
	SetWinampHWND = NULL;
	SetConfigItem = NULL;
	GetConfigItem = NULL;
	winampWnd = NULL;
	fourcc = 0;
	encoder = NULL;
}

void C_ENCODER_NSV::SetEncoder(void * CreateAudio3, void * GetAudioTypes3, void * ConfigAudio3, void * SetWinampHWND, int encoderNum) {
	*(void **)&(this->CreateAudio3) = CreateAudio3;
	*(void **)&(this->GetAudioTypes3) = GetAudioTypes3;
	*(void **)&(this->ConfigAudio3) = ConfigAudio3;
	*(void **)&(this->SetWinampHWND) = SetWinampHWND;

	if(this->SetWinampHWND) {
		this->SetWinampHWND(winampWnd);
	}

	if(this->GetAudioTypes3) {
		char name[C_ENCODER_NameLen];
		fourcc = this->GetAudioTypes3(encoderNum,name);
	}
}

C_ENCODER_NSV::~C_ENCODER_NSV() {
	WaitForSingleObject(hMutex,INFINITE);

	CloseHandle(hMutex);
	hMutex = NULL;
	C_ENCODER::~C_ENCODER();
}

void C_ENCODER_NSV::Close() {
	C_ENCODER::Close();

	if(encoder)
		delete encoder;
	encoder = NULL;
}

void C_ENCODER_NSV::Reset() {
	if(WaitForSingleObject(hMutex,INFINITE) != WAIT_OBJECT_0) return;

	Close();

	if(!configfile) {
		configfile = GetSCIniFile(winampWnd);
	}

	FillConfFile(configfile);

	int nch = ((T_ENCODER_NSV_INFO*)ExtendedInfoPtr)->input_numChannels;
	int srate = ((T_ENCODER_NSV_INFO*)ExtendedInfoPtr)->input_sampleRate;
	if (CreateAudio3) {
		const int bps = 16; /* I think this is always the case. */
		encoder = CreateAudio3(nch,srate,bps,mmioFOURCC('P','C','M',' '),&fourcc,configfile);
	}
	else encoder = NULL;

	/* I think (in that I havn't found anything to the contrary) that in the CreateAudio3 call, the encoder
	* reads all its settings from the conf_file and never touches them again. Hence, this is safe. Ahem.
	*/
	FillAttribs();
	ReleaseMutex(hMutex);
}

int C_ENCODER_NSV::Encode(const void *inputbuf, const unsigned int inputbufsize, void *outputbuf, const unsigned int outputbufsize, int *inputamtused) {

	if(WaitForSingleObject(hMutex,INFINITE) != WAIT_OBJECT_0) return 0;
	int ret=0;
	if(encoder && (inputbuf != NULL) && (outputbuf != NULL) && (inputbufsize != 0) && (outputbufsize != 0) && (inputamtused != NULL)) {
		ret = encoder->Encode(0,(void *)inputbuf,inputbufsize,inputamtused,outputbuf,outputbufsize);
	} else
		*inputamtused = inputbufsize; /* we havn't got the dll, so just say everything is ok? */

	ReleaseMutex(hMutex);
	return ret;
}

static BOOL CALLBACK configure_dlgproc(HWND intDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	
	switch(uMsg) {
		case WM_INITDIALOG:
			if(configfourcc == mmioFOURCC('A','D','T','S')) {
				SetWindowTextW(intDlg, LocalisedString(IDS_FHGAAC_ENCODER, NULL, 0));
			}
			#ifdef USE_OGG
			else if(configfourcc == mmioFOURCC('O','G','G',' ')) {
				SetWindowTextW(intDlg, LocalisedString(IDS_OGG_CONFIG_TITLE, NULL, 0));
			}
			#endif

			cfgwnd=ConfigAudio3(intDlg, encoderDllInstance, configfourcc, configfile);

			if(cfgwnd) {
			RECT r;
				GetWindowRect(GetDlgItem(intDlg,IDC_GO_HERE),&r);
				ScreenToClient(intDlg,(LPPOINT)&r);
				SetWindowPos(cfgwnd,NULL,r.left,r.top,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
				ShowWindow(cfgwnd,SW_SHOWNA);
				InvalidateRect(intDlg,NULL,FALSE);
			}
		break;
		case WM_COMMAND:
			if(LOWORD(wParam) == IDCANCEL) EndDialog(intDlg,0);
		break;
		case WM_DESTROY:
			DestroyWindow(cfgwnd);
		break;
	}
	return 0;
}

void C_ENCODER_NSV::Configure(HWND parent, HINSTANCE hDllInstance) {
	if(ConfigAudio3) {
		configfourcc = fourcc;
		if(!configfile) {
			configfile = GetSCIniFile(winampWnd);
		}
        
		::ConfigAudio3 = this->ConfigAudio3;
		::encoderDllInstance = GetEncoderInstance();

		if(WaitForSingleObject(hMutex,INFINITE) != WAIT_OBJECT_0) return;
		FillConfFile(configfile);
		ReleaseMutex(hMutex);

		LocalisedDialogBox(hDllInstance,IDD_NSVCONFIG,parent,::configure_dlgproc);

		if(WaitForSingleObject(hMutex,INFINITE) != WAIT_OBJECT_0) return;
		ReadConfFile(configfile);
		Reset();
		ReleaseMutex(hMutex);
	}
}