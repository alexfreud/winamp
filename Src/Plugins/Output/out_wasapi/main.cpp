#include "../Winamp/OUT.H"
#include "api.h"
#include "resource.h"
#include <Mmdeviceapi.h>
#include <Audioclient.h>
#include <Audiosessiontypes.h>
#include "../winamp/wa_ipc.h"
#include <api/service/waServiceFactory.h>
#include <strsafe.h>

#define WASAPI_PLUGIN_VERSION L"0.3"

constexpr auto VolumeLevelMultiplier = 255;
static wchar_t plugin_name[256];

// wasabi based services for localisation support
api_service *WASABI_API_SVC = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

static const UINT32 REFTIMES_PER_SEC = 10000000;
static const UINT32 REFTIMES_PER_MILLISEC = 10000;

// TODO(benski) is there some library that has this
static const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
static const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
static const IID IID_IAudioClient = __uuidof(IAudioClient);
static const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
static const IID IID_IAudioClock = __uuidof(IAudioClock);

static bool InitializedCOM;
extern Out_Module plugin;

static IAudioClient *client=0;
static IAudioRenderClient *render_client=0;
static IAudioClock *clock=0;
static ISimpleAudioVolume *audio_volume = 0;
static IChannelAudioVolume *channel_volume = 0;

static UINT32 bufferFrameCount;
static WORD bytes_per_frame;
static UINT64 frequency=0;
static UINT32 sample_rate;
static double start_time_ms = 0;
static bool paused=false;
static float start_volume = 1.0;
static float start_pan = 0;
WAVEFORMATEXTENSIBLE WaveFormatForParameters(int samplerate, int numchannels, int bitspersamp);
static void SetVolume(int volume);
static void SetPan(int pan);
static CRITICAL_SECTION ThreadSync;


static void Config(HWND hwndParent)
{
}

static int DoAboutMessageBox(HWND parent, wchar_t* title, wchar_t* message)
{
	MSGBOXPARAMSW msgbx = {sizeof(MSGBOXPARAMSW),0};
	msgbx.lpszText = message;
	msgbx.lpszCaption = title;
	msgbx.lpszIcon = MAKEINTRESOURCEW(102);
	msgbx.hInstance = GetModuleHandle(0);
	msgbx.dwStyle = MB_USERICON;
	msgbx.hwndOwner = parent;
	return MessageBoxIndirectW(&msgbx);
}

static void About(HWND hwndParent)
{
	wchar_t message[1024], text[1024] =L"";
	WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_WASAPI_OLD,text,1024);
	StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),
					 plugin.description, TEXT(__DATE__));
	DoAboutMessageBox(hwndParent,text,message);
}

static void Init()
{

	/*
	HRESULT hr;
	hr=CoInitializeEx(0, COINIT_MULTITHREADED);
	if (SUCCEEDED(hr)) {
	InitializedCOM = true;
	} else {
	InitializedCOM = false;
	}
	*/
	// loader so that we can get the localisation service api for use
	WASABI_API_SVC = (api_service*)SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (WASABI_API_SVC == (api_service*)1) WASABI_API_SVC = NULL;

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,OutWasapiLangGUID);

	StringCbPrintfW(plugin_name,sizeof(plugin_name),WASABI_API_LNGSTRINGW(IDS_NULLSOFT_WASAPI), WASAPI_PLUGIN_VERSION);
	plugin.description = (char *)plugin_name;
}

static void Quit()
{
	/*
	if (InitializedCOM) {
	CoUninitialize();
	}*/
}

static int Open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms)
{
	
	CoInitialize(0);
	IMMDeviceEnumerator *enumerator=0;
	IMMDevice *device=0;
	sample_rate = samplerate;
	HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&enumerator);
	hr = enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &device);

	hr = device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void **)&client);
	if (FAILED(hr)) {
		wchar_t temp[1234];
		wsprintf(temp, L"device->Activate: %x", hr);
		::MessageBox(NULL, temp, L"error", MB_OK);
		return -1;
	}

	WAVEFORMATEXTENSIBLE wave_format = WaveFormatForParameters(samplerate, numchannels, bitspersamp);
	bytes_per_frame = wave_format.Format.nBlockAlign;

	hr = client->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		0x80000000,
		1 * REFTIMES_PER_SEC,
		0,
		(WAVEFORMATEX *)&wave_format,
		NULL);
	if (FAILED(hr)) {
		wchar_t temp[1234];
		wsprintf(temp, L"client->Initialize: %x", hr);
		::MessageBox(NULL, temp, L"error", MB_OK);
		return -1;
	}

	// Get the actual size of the allocated buffer.
	hr = client->GetBufferSize(&bufferFrameCount);
	if (FAILED(hr)) {
		wchar_t temp[1234];
		wsprintf(temp, L"client->GetBufferSize: %x", hr);
		::MessageBox(NULL, temp, L"error", MB_OK);
		return -1;
	}
	hr = client->GetService(
		IID_IAudioRenderClient,
		(void**)&render_client);
	if (FAILED(hr)) {
		wchar_t temp[1234];
		wsprintf(temp, L"client->GetService(IID_IAudioRenderClient): %x", hr);
	::MessageBox(NULL, temp, L"error", MB_OK);
	return -1;
	}
	hr = client->GetService(
		IID_IAudioClock,
		(void**)&clock);
	if (FAILED(hr)) {
		wchar_t temp[1234];
		wsprintf(temp, L"client->GetService(IID_IAudioClock): %x", hr);
		::MessageBox(NULL, temp, L"error", MB_OK);
		return -1;
	}
	hr = clock->GetFrequency(&frequency);

	hr = client->GetService(__uuidof(ISimpleAudioVolume), reinterpret_cast<void **>( & audio_volume));

	hr = client->GetService(__uuidof(IChannelAudioVolume), (void **)&channel_volume);

	start_time_ms = 0;
	paused=false;
	client->Start();

	// Start volume is in range 0.0 to 1.0, should be converted
	SetVolume((int)(start_volume * VolumeLevelMultiplier));
	
	SetPan((int)start_pan);
	
	return 1000;
}

static void Close()
{
	
	if (client) {
		client->Stop();
		client->Release();
		client=0;
	}

	if (render_client) {
		render_client->Release();
		render_client=0;
	}

	if (clock) {
		clock->Release();
		clock=0;
	}

	if (audio_volume) {
		audio_volume->Release();
		audio_volume=0;
	}

	if (channel_volume) {
		channel_volume->Release();
		channel_volume=0;
	}
	
}

static int CanWrite()
{

	if (client) {
		UINT32 numFramesPadding;
		HRESULT hr = client->GetCurrentPadding(&numFramesPadding);
		return (bufferFrameCount - numFramesPadding) * bytes_per_frame;
	}
	else {
		return 0;
	}
}

static int Write(char* buf, int len)
{
	
	if (!render_client) 
	{
		return -1;
	}
	else 
	{
		int LenghtToWrite = CanWrite();
		if (LenghtToWrite > 0 && LenghtToWrite >= len)
		{
			BYTE* data;
			render_client->GetBuffer(len / bytes_per_frame, &data);
			memcpy(data, buf, len);
			render_client->ReleaseBuffer(len / bytes_per_frame, 0);
		}
	}

	return 0;
	
}



static int IsPlaying()
{
	return CanWrite() == 0;
}

static int Pause(int pause)
{
	
	
	int old_paused = paused?1:0;
	if (client) {
		if (pause) {
			client->Stop();
			paused=true;
		} else {
			client->Start();
			paused=false;
		}
	}
	
	return old_paused;
}

static void SetVolume(int volume)
{
	
	
	float fVolume = (float)volume / (float)VolumeLevelMultiplier;
	if (volume >= 0) {
		start_volume = fVolume;
		if (audio_volume) {
			audio_volume->SetMasterVolume(fVolume, 0);
		} 
	}
	
}

static void SetPan(int pan)
{
	
	
	float fPan = (float)pan/128.0f;
	if (channel_volume) {
		start_pan = fPan;
		if (fPan < 0) { 
			channel_volume->SetChannelVolume(0, 1.0f, NULL);
			channel_volume->SetChannelVolume(1, 1.0f-fPan, NULL);
		} else if (fPan > 0) {
			channel_volume->SetChannelVolume(1, 1.0f, NULL);
			channel_volume->SetChannelVolume(0, 1.0f-fPan, NULL);
		}
	}
	
}

static void Flush(int t)
{
	
	
	if (client) {
		client->Stop();
		client->Reset();
		start_time_ms = t;
		client->Start();
	}
	
}

static double GetOutputTimeAsDouble()
{
	
	
	if (clock) {
		UINT64 position;
		HRESULT hr = clock->GetPosition(&position, NULL);
		double output_time = (double)position * 1000.0 / (double)frequency;
		
		return output_time + start_time_ms;
	} else {
		
		return 0;
	}
}

static int GetOutputTime()
{
	return (int)GetOutputTimeAsDouble();
}

static int GetWrittenTime()
{
	double time_in_buffer = (1000.0 * (double)CanWrite()) / ((double)bytes_per_frame * (double)sample_rate);
	return (int)(GetOutputTimeAsDouble() + time_in_buffer);
}

Out_Module plugin = {
	OUT_VER_U,
	0,
	70,
	NULL,
	NULL,
	Config,
	About,
	Init,
	Quit,
	Open,
	Close,
	Write,
	CanWrite,
	IsPlaying,
	Pause,
	SetVolume,
	SetPan,
	Flush,
	GetOutputTime,
	GetWrittenTime,
};


extern "C" {

	__declspec(dllexport) int __cdecl winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param)
	{
		return OUT_PLUGIN_UNINSTALL_REBOOT;
	}

	__declspec(dllexport) Out_Module * __cdecl winampGetOutModule(){ return &plugin; }

	__declspec(dllexport) void __cdecl winampGetOutModeChange(int mode)
	{ 
	}

}
