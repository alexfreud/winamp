#include "../../../Winamp/in2.h"      // Winamp Input Plugin SDK
#include "../../../Winamp/wa_ipc.h"   // Winamp Inter-Process Communication
#include "main.h"                     // Our own main header
#include "resource.h"                 // For string resources etc.

// For Wasabi/Agave services
#include "../../../Agave/Language/api_language.h"
#include <api/service/waservicefactory.h>
#include "../../../Agave/Config/api_config.h" // For AGAVE_API_CONFIG
#include "../../../Wasabi/api/memmgr/api_memmgr.h" // For WASABI_API_MEMMGR
#include "../../../nu/Singleton.h"      // For SingletonServiceFactory
#include "../../../Agave/DecodeFile/svc_raw_media_reader.h" // For RawMediaReaderService (if we implement it)


// Opus specific includes
#include <opusfile.h> // From openmpt-trunk includes
#include <shlwapi.h> // For PathFindExtensionW
#include <strsafe.h> // For StringCchPrintfW etc.
#include "../../../nu/AutoCharFn.h" // For AutoCharFn_downcast
#include "../../../nu/ns_wc.h" // For lstrcpynW etc.

// Service includes
#include "OpusRawMediaReader.h" // Include our new classes


// --- Global Service Instances and Factories ---
static OpusRawMediaReader opus_raw_reader_service;
static SingletonServiceFactory<svc_raw_media_reader, OpusRawMediaReader> opus_raw_reader_factory;

static OpusFileTypeEditor opus_file_type_editor_service;
static SingletonServiceFactory<svc_filetypeeditor, OpusFileTypeEditor> opus_file_type_editor_factory;


// --- Global Variables ---
api_config *AGAVE_API_CONFIG = NULL;
api_language *WASABI_API_LNG = NULL;
api_memmgr *WASABI_API_MEMMGR = NULL; // Example if needed
HINSTANCE WASABI_API_LNG_HINST = NULL;
HINSTANCE WASABI_API_ORIG_HINST = NULL;

wchar_t currentFilePath[MAX_PATH * 4] = {L""}; // Store the currently playing file path
volatile int currentSongLengthMs = 0; // Length of the current song in milliseconds
char g_plugin_description_char[256] = "Opus Decoder (Uninitialized)"; // Static buffer for plugin.description

// --- Service Management (Helper Templates from in_wave) ---
template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = reinterpret_cast<api_T *>(factory->getInterface());
	}
}

template <class api_T>
void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC && api_t)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
	api_t = NULL;
}


// --- Module Functions ---

void Config(HWND hwndParent)
{
	// TODO: Implement configuration dialog if needed
	MessageBoxW(hwndParent, L"Opus Plugin Configuration (Not Implemented)", L"Config", MB_OK);
}

void About(HWND hwndParent)
{
	wchar_t message[1024];
	wchar_t about_title[256];
	StringCchPrintfW(message, 1024, WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT_OPUS), PLUGIN_VERSION_OPUS, __DATE__);
	StringCchPrintfW(about_title, 256, WASABI_API_LNGSTRINGW(IDS_NULLSOFT_OPUS_DECODER_OLD));
	MessageBoxW(hwndParent, message, about_title, MB_OK);
}

int Init()
{
	if (!IsWindow(plugin.hMainWindow)) // Check if Winamp main window is valid
		return IN_INIT_FAILURE;

	// Get standard Winamp services
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid); // Example

	// Initialize localization
	WASABI_API_START_LANG(plugin.hDllInstance, InOpusLangGUID);

	// Set plugin description dynamically using localized string
	static wchar_t szDescriptionUnicode[256];
	StringCchPrintfW(szDescriptionUnicode, 256, WASABI_API_LNGSTRINGW(IDS_NULLSOFT_OPUS_DECODER), PLUGIN_VERSION_OPUS);

	// Convert wchar_t description to char for the In_Module struct
	WideCharToMultiByte(CP_UTF8, 0, szDescriptionUnicode, -1, g_plugin_description_char, sizeof(g_plugin_description_char), NULL, NULL);
	plugin.description = g_plugin_description_char;

	// Register Opus-specific services
	if (WASABI_API_SVC)
	{
		opus_raw_reader_factory.Register(WASABI_API_SVC, &opus_raw_reader_service);
		opus_file_type_editor_factory.Register(WASABI_API_SVC, &opus_file_type_editor_service);
	}

	plugin.FileExtensions = "opus\0Opus Audio Files (*.opus)\0"; // Define supported extensions, more descriptive
	plugin.is_seekable = 1; // Opus files are generally seekable with opusfile
	plugin.UsesOutputPlug = 1; // Standard for input plugins using Winamp's output system

	return IN_INIT_SUCCESS;
}

void Quit()
{
	// Deregister Opus-specific services
	if (WASABI_API_SVC)
	{
		opus_raw_reader_factory.Deregister(WASABI_API_SVC);
		opus_file_type_editor_factory.Deregister(WASABI_API_SVC);
	}

	// Release standard services
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
}

void GetFileInfo(const wchar_t *file, wchar_t *title, int *length_in_ms)
{
	const wchar_t *fileToOpen = file ? file : currentFilePath;
	if (!fileToOpen || !*fileToOpen)
	{
		if (title) title[0] = L'\0';
		if (length_in_ms) *length_in_ms = -1000;
		return;
	}

	OggOpusFile *of = op_open_file(AutoCharFn_downcast(fileToOpen), NULL); // AutoCharFn_downcast for const wchar_t*
	if (of)
	{
		if (length_in_ms)
		{
			ogg_int64_t pcm_total = op_pcm_total(of, -1);
			const OpusHead *head = op_head(of, 0); // Assuming single link opus file for simplicity here
			if (head && head->input_sample_rate > 0) // input_sample_rate is the original rate before resampler
				*length_in_ms = (int)(pcm_total * 1000 / head->input_sample_rate);
			else if (head) // Fallback if input_sample_rate is not available, use fixed 48kHz as opus internal rate
				*length_in_ms = (int)(pcm_total * 1000 / 48000);
			else
				*length_in_ms = -1000;

			if (!file) // If we're querying the current file, update currentSongLengthMs
				currentSongLengthMs = *length_in_ms;
		}

		if (title)
		{
			const OpusTags *tags = op_tags(of, -1);
			if (tags)
			{
				const char *tag_title = opus_tags_query(tags, "TITLE", 0);
				if (tag_title && *tag_title)
				{
					MultiByteToWideChar(CP_UTF8, 0, tag_title, -1, title, GETFILEINFO_TITLE_LENGTH);
				}
				else
				{
					// No title tag, use filename
					lstrcpynW(title, PathFindFileNameW(fileToOpen), GETFILEINFO_TITLE_LENGTH);
				}
			}
			else
			{
				lstrcpynW(title, PathFindFileNameW(fileToOpen), GETFILEINFO_TITLE_LENGTH);
			}
		}
		op_free(of);
	}
	else
	{
		if (title) lstrcpynW(title, PathFindFileNameW(fileToOpen), GETFILEINFO_TITLE_LENGTH);
		if (length_in_ms) *length_in_ms = -1000;
	}
}

int InfoBox(const wchar_t *file, HWND hwndParent)
{
    if (!file || !*file) return INFOBOX_UNCHANGED;

    wchar_t message[2048] = {0};
    wchar_t fileInfo[1024] = {0};
    int length_ms = -1;

    OggOpusFile *of = op_open_file(AutoCharFn_downcast(file), NULL);
    if (of)
    {
        const OpusHead *head = op_head(of, 0);
        const OpusTags *tags = op_tags(of, -1);
        ogg_int64_t pcm_total = op_pcm_total(of, -1);
        int bitrate = op_bitrate(of, -1);

        if (head)
			length_ms = (int)(pcm_total * 1000 / (head->input_sample_rate ? head->input_sample_rate : 48000));

        StringCchPrintfW(fileInfo, 1024, L"File: %s\n\n", PathFindFileNameW(file));

        wchar_t temp[256];
        if (length_ms != -1000 && length_ms != -1)
            StringCchPrintfW(temp, 256, L"Length: %d:%02d\n", length_ms / 60000, (length_ms / 1000) % 60);
        else
            StringCchCopyW(temp, 256, L"Length: Unknown\n");
        StringCchCatW(fileInfo, 1024, temp);

        if (head)
        {
            StringCchPrintfW(temp, 256, L"Channels: %d\n", head->channel_count);
            StringCchCatW(fileInfo, 1024, temp);
            StringCchPrintfW(temp, 256, L"Sample Rate: %ld Hz (output)\n", op_current_link(of)->rate); // op_current_link for output rate
            StringCchCatW(fileInfo, 1024, temp);
             if (head->input_sample_rate > 0) {
                StringCchPrintfW(temp, 256, L"Original Sample Rate: %ld Hz\n", head->input_sample_rate);
                StringCchCatW(fileInfo, 1024, temp);
            }
        }
        if (bitrate > 0)
        {
            StringCchPrintfW(temp, 256, L"Bitrate: %d kbps\n", bitrate / 1000);
            StringCchCatW(fileInfo, 1024, temp);
        }
        else
        {
            StringCchCatW(fileInfo, 1024, L"Bitrate: VBR\n");
        }

        if (tags)
        {
            StringCchCatW(fileInfo, 1024, L"\nTags:\n");
            for (int i = 0; i < tags->comments; i++)
            {
                MultiByteToWideChar(CP_UTF8, 0, tags->user_comments[i], tags->comment_lengths[i], temp, 255);
                temp[tags->comment_lengths[i]] = 0; // Ensure null termination if not already
                StringCchCatW(fileInfo, 1024, temp);
                StringCchCatW(fileInfo, 1024, L"\n");
            }
        }
        op_free(of);
    }
    else
    {
        StringCchPrintfW(fileInfo, 1024, L"Could not open or parse Opus file: %s", file);
    }

    StringCchCopyW(message, 2048, fileInfo);
    MessageBoxW(hwndParent, message, L"Opus File Information", MB_OK | MB_USERICON); // MB_USERICON might need an icon in RC
	return INFOBOX_UNCHANGED;
}

int IsOurFile(const wchar_t *fn)
{
	// More robust check could involve op_test_callbacks if we were using custom I/O,
	// or trying op_open_file and checking for specific errors.
	// For now, extension check is standard for In_Module::IsOurFile.
	if (!fn || !*fn) return 0;
	const wchar_t *ext = PathFindExtensionW(fn);
	if (ext && (_wcsicmp(ext, L".opus") == 0)) // Use _wcsicmp for case-insensitive comparison
		return 1;
	return 0;
}

// --- Playback Management ---
static OpusAudioStream *g_current_stream = NULL; // Our currently active audio stream
static volatile int g_kill_playback_flag = 0;  // 0: play, 1: stop requested, 2: EOF reached
static HANDLE g_playback_thread = INVALID_HANDLE_VALUE;
static CRITICAL_SECTION g_stream_lock; // To protect access to g_current_stream

// Forward declaration for the playback thread
DWORD WINAPI OpusPlaybackThread(LPVOID param);


// Helper to safely delete the stream
void CleanupCurrentStream()
{
    EnterCriticalSection(&g_stream_lock);
    if (g_current_stream)
    {
        delete g_current_stream;
        g_current_stream = NULL;
    }
    LeaveCriticalSection(&g_stream_lock);
}

// --- In_Module Callbacks ---

int Init() // Modified to include InitializeCriticalSection
{
	if (!IsWindow(plugin.hMainWindow))
		return IN_INIT_FAILURE;

    InitializeCriticalSection(&g_stream_lock);

	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);

	WASABI_API_START_LANG(plugin.hDllInstance, InOpusLangGUID);

	static wchar_t szDescriptionUnicode[256];
	StringCchPrintfW(szDescriptionUnicode, 256, WASABI_API_LNGSTRINGW(IDS_NULLSOFT_OPUS_DECODER), PLUGIN_VERSION_OPUS);

	WideCharToMultiByte(CP_UTF8, 0, szDescriptionUnicode, -1, g_plugin_description_char, sizeof(g_plugin_description_char), NULL, NULL);
	plugin.description = g_plugin_description_char;

	if (WASABI_API_SVC)
	{
		opus_raw_reader_factory.Register(WASABI_API_SVC, &opus_raw_reader_service);
		opus_file_type_editor_factory.Register(WASABI_API_SVC, &opus_file_type_editor_service);
	}

	plugin.FileExtensions = "opus\0Opus Audio Files (*.opus)\0";
	plugin.is_seekable = 1;
	plugin.UsesOutputPlug = 1;

	return IN_INIT_SUCCESS;
}

void Quit() // Modified to include DeleteCriticalSection
{
    DeleteCriticalSection(&g_stream_lock);

	if (WASABI_API_SVC)
	{
		opus_raw_reader_factory.Deregister(WASABI_API_SVC);
		opus_file_type_editor_factory.Deregister(WASABI_API_SVC);
	}

	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
}


DWORD WINAPI OpusPlaybackThread(LPVOID param)
{
    const int DECODE_BUFFER_SIZE = 5760 * 2 * sizeof(short); // Max 120ms at 48kHz stereo 16-bit
    unsigned char *decodeBuffer = (unsigned char *)WASABI_API_MEMMGR->sysMalloc(DECODE_BUFFER_SIZE);
    if (!decodeBuffer) {
        g_kill_playback_flag = 2; // Indicate error/EOF
        return 1;
    }

    int last_output_time = 0;

    while (!g_kill_playback_flag)
    {
        EnterCriticalSection(&g_stream_lock);
        if (!g_current_stream) {
            LeaveCriticalSection(&g_stream_lock);
            g_kill_playback_flag = 2; // Stream disappeared
            break;
        }

        int kill_flag_for_read = g_kill_playback_flag; // Pass a copy
        int stream_err = 0;
        size_t bytes_read = g_current_stream->ReadAudio(decodeBuffer, DECODE_BUFFER_SIZE, &kill_flag_for_read, &stream_err);
        LeaveCriticalSection(&g_stream_lock);

        if (kill_flag_for_read || stream_err != 0) { // Check if ReadAudio itself was told to stop or errored
            g_kill_playback_flag = (stream_err !=0 && stream_err != OP_EOF) ? 2 : 1; // if error other than EOF, treat as EOF/Stop
            break;
        }

        if (bytes_read == 0) // Normal EOF from ReadAudio
        {
            g_kill_playback_flag = 2; // EOF
            break;
        }

        // Wait for output plugin if buffer is full
        while (plugin.outMod->CanWrite() < (int)bytes_read && !g_kill_playback_flag) {
            Sleep(10);
        }
        if (g_kill_playback_flag) break;

        plugin.outMod->Write(decodeBuffer, bytes_read);

        // Update VSA, this is typically done by the input plugin
        // However, the exact data for VSA (spectrum analyzer) might need raw PCM before any DSP.
        // For now, just indicate data flow.
        // plugin.SAAddPCMData(decodeBuffer, g_current_stream->GetChannels(), g_current_stream->GetBitsPerSample(), plugin.outMod->GetOutputTime());
        // plugin.VSAAddPCMData also exists for video spectrum.
    }

    WASABI_API_MEMMGR->sysFree(decodeBuffer);

    // If we reached EOF, tell Winamp's output to flush remaining data and not cut off early
    if (g_kill_playback_flag == 2) { // EOF
        plugin.outMod->Write(NULL, 0); // Signal EOF to output
    }

    return 0;
}


int Play(const wchar_t *fn)
{
	if (!fn || !*fn) return 1;

    Stop(); // Stop any previous playback first

	lstrcpynW(currentFilePath, fn, MAX_PATH * 4);
	currentSongLengthMs = -1000;
    g_kill_playback_flag = 0;

    int stream_open_error = 0;
    EnterCriticalSection(&g_stream_lock);
    g_current_stream = new (std::nothrow) OpusAudioStream(currentFilePath, &stream_open_error);
    if (!g_current_stream || stream_open_error != 0 || g_current_stream->GetError() != 0) {
        if (g_current_stream) delete g_current_stream;
        g_current_stream = NULL;
        LeaveCriticalSection(&g_stream_lock);
        currentFilePath[0] = L'\0';
        return 1; // Failed to open stream
    }

    long sampleRate = g_current_stream->GetSampleRate();
    int numChannels = g_current_stream->GetChannels();
    int bps = g_current_stream->GetBitsPerSample();
    currentSongLengthMs = (int)(g_current_stream->GetTotalPCMSamples() * 1000 / sampleRate);
    LeaveCriticalSection(&g_stream_lock);


	plugin.is_seekable = g_current_stream->CanSeek();
    // The SetInfo call is deprecated, but some old vis plugins might use it.
    // plugin.SetInfo((sampleRate * bps * numChannels) / 8000 /*kbps*/, sampleRate / 1000, numChannels, 1);

	plugin.SAVSADeInit(); // Deinit any previous vis
	plugin.SAVSAInit(plugin.dsp_isactive()?plugin.dsp_getlatent()+plugin.outMod->GetLatency():plugin.outMod->GetLatency(), sampleRate); // latency, srate

    int open_flags = plugin.UsesOutputPlug;
    // Forcing use of specific bit depth or channel count can be done here if needed
    // e.g. open_flags |= OUT_BITDEPTH_16;
	int ret = plugin.outMod->Open(sampleRate, numChannels, bps, -1, -1); // srate,nch,bps,bufferlen_ms,prebuffer_ms
    if (ret < 0) { // Error opening output device
        CleanupCurrentStream();
        currentFilePath[0] = L'\0';
        return 1;
    }

    // If already paused from a previous track, honor it.
    if (plugin.SAVSADeInit) // Check if plugin struct is fully populated (hackish way to see if Winamp is paused globally)
    {
        int winampIsPaused = SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_ISPLAYING);
        if (winampIsPaused == 3) { // 3 means paused
             plugin.outMod->Pause(1);
        }
    }


    g_playback_thread = CreateThread(NULL, 0, OpusPlaybackThread, NULL, 0, NULL);
    if (g_playback_thread == INVALID_HANDLE_VALUE) {
        CleanupCurrentStream();
        plugin.outMod->Close();
        currentFilePath[0] = L'\0';
        return 1;
    }
    SetThreadPriority(g_playback_thread, AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));

	return 0;
}

void Pause()
{
	if (plugin.outMod) plugin.outMod->Pause(1);
}

void UnPause()
{
	if (plugin.outMod) plugin.outMod->Pause(0);
}

int IsPaused()
{
	return plugin.outMod ? plugin.outMod->IsPaused() : 0;
}

void Stop()
{
    g_kill_playback_flag = 1;
    if (g_playback_thread != INVALID_HANDLE_VALUE) {
        WaitForSingleObject(g_playback_thread, INFINITE); // Wait for thread to finish
        CloseHandle(g_playback_thread);
        g_playback_thread = INVALID_HANDLE_VALUE;
    }

    CleanupCurrentStream(); // Safely delete the stream object

	if (plugin.outMod) plugin.outMod->Close();
	plugin.SAVSADeInit();
	currentFilePath[0] = L'\0';
	currentSongLengthMs = 0;
    g_kill_playback_flag = 0; // Reset for next play
}

int GetLength()
{
	return currentSongLengthMs > 0 ? currentSongLengthMs : -1000;
}

int GetOutputTime()
{
    // If output is active, get its time. Otherwise, if seeking while stopped, could return a target seek time.
    // For now, simple approach.
	return plugin.outMod ? plugin.outMod->GetOutputTime() : 0;
}

void SetOutputTime(int time_in_ms)
{
    EnterCriticalSection(&g_stream_lock);
	if (g_current_stream && g_current_stream->CanSeek()) {
		g_current_stream->SeekToTimeMs(time_in_ms);
        // After seeking the source, tell the output plugin to flush its buffers and reset its timeline
		plugin.outMod->Flush(time_in_ms);
	}
    LeaveCriticalSection(&g_stream_lock);
}

int pan = 0;
int volume = -666; // Winamp's way of saying "not set by plugin"

void SetVolume(int _volume)
{
	volume = _volume;
	if (plugin.outMod) plugin.outMod->SetVolume(volume);
}

void SetPan(int _pan)
{
	pan = _pan;
	if (plugin.outMod) plugin.outMod->SetPan(pan);
}

void EQSet(int on, char data[10], int preamp)
{
	// Usually handled by output plugin, or Winamp core
	if (plugin.outMod && plugin.outMod->CanSetEQ()) {
    plugin.outMod->SetEQ(on, data, preamp);
  }
}

// --- In_Module Structure ---
In_Module plugin = {
	IN_VER_RET,                       // Interface version (defined in in2.h)
	g_plugin_description_char,        // Description (UTF-8 char*, will be set by Init)
	0,                                // Winamp main window handle (filled by Winamp)
	0,                                // DLL instance handle (filled by Winamp)
	"opus\0Opus Audio Files (*.opus)\0", // Supported extensions (UTF-8 char*)
	1,                                // is_seekable
	1,                                // UsesOutputPlug (we'll use Winamp's output system)
	Config,                           // Config dialog function
	About,
	Init,
	Quit,
	GetFileInfo,
	InfoBox,
	IsOurFile,
	Play,
	Pause,
	UnPause,
	IsPaused,
	Stop,
	GetLength,
	GetOutputTime,
	SetOutputTime,
	SetVolume,
	SetPan,
	0, // Vis stuff (filled by Winamp)
	0, // Vis stuff (filled by Winamp)
	0, // DSP stuff (filled by Winamp)
	0, // DSP stuff (filled by Winamp)
	0, // EQ values (filled by Winamp)
	0, // Playlist editor (filled by Winamp)
	0, // Output module (filled by Winamp)
	0, // Video output (filled by Winamp)
	0, // DMA
	0, // CURINFO
	0, // Paused
	EQSet,
	NULL, // SetInfo (deprecated)
	NULL  // SAVSAInit / VSAAdd / etc. are now part of outMod
};

// --- Plugin Export ---
extern "C" __declspec(dllexport) In_Module *winampGetInModule2()
{
	return &plugin;
}

// --- Extended File Info (Optional, for metadata) ---
extern "C" __declspec(dllexport) int winampGetExtendedFileInfoW(const wchar_t *fn, const char *data, wchar_t *dest, int destlen)
{
	if (!fn || !*fn || !data || !*data || !dest || destlen <= 0)
		return 0;

	dest[0] = L'\0'; // Initialize output buffer

	if (lstrcmpiA(data, "type") == 0)
	{
		// Type: 0 for audio, 1 for video
		StringCchCopyW(dest, destlen, L"0");
		return 1;
	}
	else if (lstrcmpiA(data, "family") == 0)
	{
		// This is used by Winamp's "Jump to File" feature's "Format Family" filter.
		// Provide a description for the .opus extension.
		LPCWSTR ext = PathFindExtensionW(fn);
		if (ext && _wcsicmp(ext, L".opus") == 0)
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_FAMILY_STRING_OPUS, dest, destlen);
			return 1;
		}
		return 0;
	}

	OggOpusFile *of = op_open_file(AutoCharFn_downcast(fn), NULL);
	if (!of)
		return 0;

	int result = 0;
	const OpusTags *tags = op_tags(of, -1);
	const OpusHead *head = op_head(of, 0);

	if (tags)
	{
		const char *tag_value = NULL;
		if (lstrcmpiA(data, "artist") == 0) tag_value = opus_tags_query(tags, "ARTIST", 0);
		else if (lstrcmpiA(data, "title") == 0) tag_value = opus_tags_query(tags, "TITLE", 0);
		else if (lstrcmpiA(data, "album") == 0) tag_value = opus_tags_query(tags, "ALBUM", 0);
		else if (lstrcmpiA(data, "albumartist") == 0) tag_value = opus_tags_query(tags, "ALBUMARTIST", 0);
		else if (lstrcmpiA(data, "composer") == 0) tag_value = opus_tags_query(tags, "COMPOSER", 0);
		else if (lstrcmpiA(data, "genre") == 0) tag_value = opus_tags_query(tags, "GENRE", 0);
		else if (lstrcmpiA(data, "year") == 0) tag_value = opus_tags_query(tags, "DATE", 0); // DATE often used for year
		else if (lstrcmpiA(data, "track") == 0 || lstrcmpiA(data, "tracknumber") == 0) tag_value = opus_tags_query(tags, "TRACKNUMBER", 0);
		else if (lstrcmpiA(data, "disc") == 0 || lstrcmpiA(data, "discnumber") == 0) tag_value = opus_tags_query(tags, "DISCNUMBER", 0);
		else if (lstrcmpiA(data, "comment") == 0) tag_value = opus_tags_query(tags, "COMMENT", 0);
		else if (lstrcmpiA(data, "replaygain_track_gain") == 0) tag_value = opus_tags_query(tags, "REPLAYGAIN_TRACK_GAIN", 0);
		else if (lstrcmpiA(data, "replaygain_track_peak") == 0) tag_value = opus_tags_query(tags, "REPLAYGAIN_TRACK_PEAK", 0);
		else if (lstrcmpiA(data, "replaygain_album_gain") == 0) tag_value = opus_tags_query(tags, "REPLAYGAIN_ALBUM_GAIN", 0);
		else if (lstrcmpiA(data, "replaygain_album_peak") == 0) tag_value = opus_tags_query(tags, "REPLAYGAIN_ALBUM_PEAK", 0);
		else if (lstrcmpiA(data, "encoder") == 0) tag_value = opus_tags_query(tags, "ENCODER", 0);
		// Add more tags as needed: COPYRIGHT, etc.

		if (tag_value && *tag_value)
		{
			MultiByteToWideChar(CP_UTF8, 0, tag_value, -1, dest, destlen);
			result = 1;
		}
	}

	if (!result) // If not found in common string tags, check for other info
	{
		if (head && lstrcmpiA(data, "bitrate") == 0)
		{
			int bitrate_bps = op_bitrate(of, -1);
			if (bitrate_bps > 0)
			{
				StringCchPrintfW(dest, destlen, L"%d", bitrate_bps / 1000);
				result = 1;
			}
		}
		else if (head && lstrcmpiA(data, "samplerate") == 0)
		{
			StringCchPrintfW(dest, destlen, L"%ld", op_current_link(of)->rate); // Output sample rate
			result = 1;
		}
		else if (head && lstrcmpiA(data, "channels") == 0)
		{
			StringCchPrintfW(dest, destlen, L"%d", head->channel_count);
			result = 1;
		}
		else if (head && lstrcmpiA(data, "length") == 0) // Length in milliseconds as string
		{
			ogg_int64_t pcm_total = op_pcm_total(of, -1);
			long sr = head->input_sample_rate ? head->input_sample_rate : 48000;
			if (sr > 0)
			{
				StringCchPrintfW(dest, destlen, L"%lld", pcm_total * 1000 / sr);
				result = 1;
			}
		}
		// TODO: Add "codec", "vbr", "format" etc. if useful
	}

	op_free(of);
	return result;
}
