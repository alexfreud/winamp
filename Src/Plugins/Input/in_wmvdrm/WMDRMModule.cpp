#include "Main.h"
#include "WMDRMModule.h"
#include "AutoWide.h"
#include "WMInformation.h"
#include "AutoChar.h"
#include "FileInfoDialog.h"
#include "ConfigDialog.h"
#include "resource.h"
#include "StatusHook.h"
#include "../nu/Config.h"
#include "util.h"
#include "WMPlaylist.h"
#include "api.h"
#include "output/OutPlugin.h"
#include "output/AudioOut.h"
#include <strsafe.h>

extern Nullsoft::Utility::Config wmConfig;

#define SAMPLES_PER_BLOCK 576
AudioOut *out = 0;

unsigned long endTime = 0;
unsigned long startTime = 0;

void InitOutputs(HWND hMainWindow, HMODULE hDllInstance)
{}

void WMDRM::AssignOutput()
{
	out = &pluginOut;
}

WMDRM::WMDRM()
		: paused(false),
		clock(0), audio(0), video(0), wait(0), info(0), buffer(0), seek(0), reader(NULL), gain(0),
		killswitch(0), opened(false),
		drmProtected(false),
		volume(-666), pan(0),
		reader2(0), network(0), playing(false),
		startAtMilliseconds(0),
		dspBuffer(0),
		vizBuffer(0),
		reader1(0),
		flushed(false)
{
	killswitch = CreateEvent(NULL, TRUE, TRUE, NULL);
}

WMDRM::~WMDRM()
{
	DeleteObject(killswitch);
	delete [] dspBuffer;
	delete [] vizBuffer;
}

static int winampVersion=0;

#define WINAMP_VERSION_MINOR1(winampVersion) ((winampVersion & 0x000000F0)>>4)  // returns, i.e. 0x01 for 5.12 and 0x02 for 5.2...
#define WINAMP_VERSION_MINOR2(winampVersion) ((winampVersion & 0x0000000F))  // returns, i.e. 0x02 for 5.12 and 0x00 for 5.2...
static void MakeVersionString(QWORD *ver)
{
	if (!winampVersion)
		winampVersion = (int)SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GETVERSION);

	LARGE_INTEGER temp;
	temp.HighPart = MAKELONG(WINAMP_VERSION_MINOR1(winampVersion), WINAMP_VERSION_MAJOR(winampVersion));
	temp.LowPart = MAKELONG(WASABI_API_APP->main_getBuildNumber(), WINAMP_VERSION_MINOR2(winampVersion));

	*ver = temp.QuadPart;
}

static void MakeUserAgentString(wchar_t str[256])
{
	if (!winampVersion)
		winampVersion = (int)SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GETVERSION);

	StringCchPrintfW(str, 256, L"WinampASF/%01x.%02x",
	                 WINAMP_VERSION_MAJOR(winampVersion),
	                 WINAMP_VERSION_MINOR(winampVersion));
}


void WMDRM::InitWM()
{
	static int triedInit = 0;
	if (!triedInit)
	{
		if (FAILED(WMCreateReader(0, WMT_RIGHT_PLAYBACK, &reader)) || !reader)
		{
			reader = 0;
			plugin.FileExtensions = "\0";
			return ;
		}
		if (FAILED(reader->QueryInterface(&reader1)))
			reader1 = 0;

		if (FAILED(reader->QueryInterface(&reader2)))
			reader2 = 0;

		if (FAILED(reader->QueryInterface(&network)))
			network = 0;

		if (reader1)
		{
			QWORD verStr;
			wchar_t userAgent[256] = {0};
			MakeVersionString(&verStr);
			MakeUserAgentString(userAgent);
			WM_READER_CLIENTINFO info;
			ZeroMemory(&info, sizeof(WM_READER_CLIENTINFO));
			info.cbSize = sizeof(WM_READER_CLIENTINFO);
			info.wszHostExe = L"winamp.exe";
			info.qwHostVersion = verStr;
			info.wszPlayerUserAgent = userAgent;
			info.wszBrowserWebPage = L"http://www.winamp.com";

			reader1->SetClientInfo(&info);
		}

		clock = new ClockLayer(reader);
		audio = new AudioLayer(reader);
		video = new VideoLayer(reader);
		wait = new WaitLayer(reader);
		info = new WMInformation(reader);
		buffer = new BufferLayer(reader);
		seek = new SeekLayer(reader, clock);
		gain = new GainLayer(audio, info);

		callback >> seek >> buffer >> clock >> video >> audio >> wait >> gain >> this;

		triedInit = 1;
	}
}

void WMDRM::Init()
{
	winampVersion = (int)SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GETVERSION);
	InitOutputs(plugin.hMainWindow, plugin.hDllInstance);

	//Hook(plugin.hMainWindow);
}

void WMDRM::Config(HWND hwndParent)
{
	WASABI_API_DIALOGBOXW(IDD_CONFIG, hwndParent, PreferencesDialogProc);
}

void WMDRM::Quit()
{
	activePlaylist.Clear();
	delete setFileInfo;	setFileInfo = 0;
	delete clock; clock = 0;
	delete audio; audio = 0;
	delete video; video = 0;
	delete wait; wait = 0;
	delete info; info = 0;
	delete buffer; buffer = 0;
	delete seek; seek = 0;

	if (network) network->Release(); network = 0;
	if (reader2)	reader2->Release();	reader2 = 0;
	if (reader1) reader1->Release(); reader1 = 0;
	if (reader) reader->Release();	reader = 0;

//	Unhook(plugin.hMainWindow);
}

static void BuildTitle(WMInformation *info, const wchar_t *file, wchar_t *str, size_t len)
{
	if (info)
	{
		wchar_t artist[256] = L"", title[256] = L"";
		info->GetAttribute(g_wszWMAuthor, artist, 256);
		info->GetAttribute(g_wszWMTitle, title, 256);

		if (!artist[0] && !title[0])
		{
			if (file && *file)
			{
				StringCchCopy(str, len, file);
			}
		}
		else if (artist[0] && title[0])
			StringCchPrintf(str, len, L"%s - %s", artist, title);
		else if (artist[0])
			StringCchCopy(str, len, artist);
		else if (title[0])
			StringCchCopy(str, len, title);
	}
	else if (file)
		StringCchCopy(str, len, file);

}

void WMDRM::GetFileInfo(const wchar_t *file, wchar_t *title, int *length_in_ms)
{
	InitWM();

	if (length_in_ms) *length_in_ms = -1000;
	if (file && file[0])
	{
		bool isURL = !!PathIsURL(file);
		if (config_http_metadata || !isURL)
		{
			WMInformation getFileInfo(file, true);

			if (title)
			{
				BuildTitle(&getFileInfo, file, title, GETFILEINFO_TITLE_LENGTH);
			}
			if (length_in_ms) *length_in_ms = getFileInfo.GetLengthMilliseconds();
		}
		else
		{
			if (title)
				StringCchCopy(title, GETFILEINFO_TITLE_LENGTH, file);
		}
		winamp.GetStatus(title, 256, file);
	}
	else if (activePlaylist.GetFileName())
	{
		//isURL = !!wcsstr(activePlaylist.GetFileName(), L"://");
		if (wait && !wait->IsOpen()) // if it's not open, fill in with some default data... WMDRM::Opened() will refresh the title ...
		{
			StringCchCopy(title, GETFILEINFO_TITLE_LENGTH, activePlaylist.GetOriginalFileName());
			if (length_in_ms) *length_in_ms = -1000;
			//if (isURL)
				winamp.GetStatus(title, 256, activePlaylist.GetOriginalFileName());
			return ;
		}

		if (title)
		{
			BuildTitle(info, activePlaylist.GetOriginalFileName(), title, GETFILEINFO_TITLE_LENGTH);
		}
		if (info)
			if (length_in_ms) *length_in_ms = info->GetLengthMilliseconds();
		else
			if (length_in_ms) *length_in_ms = -1000;

		//if (isURL)
			winamp.GetStatus(title, 256, activePlaylist.GetOriginalFileName());
	}
}

int WMDRM::InfoBox(const wchar_t *fn, HWND hwndParent)
{
	/* CUT> we're now using the unified file info dialogue
	FileInfoDialog dialog(WASABI_API_LNG_HINST, hwndParent, fn);
	if (dialog.WasEdited())
		return 0;
	else
		return 1;
		*/
	return 0;
}

int WMDRM::IsOurFile(const in_char *fn)
{
	//	if (!reader)
	//		return 0;
	if (wcsstr(fn, L".asx")) // TODO: need something WAY better than this
		return 1;
	return fileTypes.IsSupportedURL(fn);
}

int WMDRM::Play(const wchar_t * fn)
{
	InitWM();

	if (!reader)
		return -1;
	if (network)
		network->SetBufferingTime((QWORD)config_buffer_time*10000LL);

	ResetEvent(killswitch);
	wait->ResetForOpen();

	activePlaylist.Clear();
	activePlaylist.playlistFilename = _wcsdup(fn);
	if (playlistManager->Load(fn, &activePlaylist) != PLAYLISTMANAGER_SUCCESS)
		activePlaylist.OnFile(fn, 0, -1, 0); // add it manually (TODO: need a better way to do this)

	winamp.GetVideoOutput();
	playing = true;
	startTime = winamp.GetStart() * 1000;
	if (!startAtMilliseconds)
		startAtMilliseconds = startTime;

	endTime = winamp.GetEnd() * 1000;
	clock->SetLastOutputTime(startAtMilliseconds); // normally 0, but set when metadata editor needs to stop / restart a file
	AssignOutput();
	clock->SetStartTimeMilliseconds(startAtMilliseconds); // normally 0, but set when metadata editor needs to stop / restart a file
	startAtMilliseconds = 0;
	return seek->Open(activePlaylist.GetFileName(), &callback);
}

void WMDRM::ReOpen()
{
	if (opened)
		seek->Stop();
	seek->Open(activePlaylist.GetFileName(), &callback);
}

void WMDRM::Pause()
{
	paused = true;
	if (seek)
		seek->Pause();
}

void WMDRM::UnPause()
{
	paused = false;
	if (seek)
		seek->Unpause();
}

int WMDRM::IsPaused()
{
	return (int)paused;
}

void WMDRM::Stop()
{
	if (!playing)
		return ;

	playing = false;
	SetEvent(killswitch);
	if (paused)
		UnPause();
	if (seek)
		seek->Stop();
}


void WMDRM::Closed()
{
	opened = false;
	WMHandler::Closed();
}

int WMDRM::GetLength()
{
	if (info)
		return info->GetLengthMilliseconds();
	else
		return 0;
}

int WMDRM::GetOutputTime()
{
	if (!opened)
	{
		//if (winamp.bufferCount)
		return winamp.bufferCount;
		//return 0;
	}
	return clock->GetOutputTime();
}

void WMDRM::SetOutputTime(int time_in_ms)
{
	if (startTime || endTime)
	{
		unsigned int seektime = time_in_ms;
		if (endTime && seektime > endTime)
			seektime = endTime;
		if (startTime && seektime < startTime)
			seektime = startTime;
		seek->SeekTo(seektime);
		return ;
	}
	seek->SeekTo(time_in_ms);
}

void WMDRM::SetVolume(int volume)
{
	this->volume = volume;
	if (out)
		out->SetVolume(volume);
}

void WMDRM::SetPan(int pan)
{
	this->pan = pan;
	if (out)
		out->SetPan(pan);
}

void WMDRM::EQSet(int on, char data[10], int preamp)
{}

void WMDRM::BuildBuffers()
{
	remaining.Allocate(audio->AudioSamplesToBytes(SAMPLES_PER_BLOCK));

	// TODO: check against old size
	delete [] dspBuffer;
	delete [] vizBuffer;
	dspBuffer = new unsigned char[audio->AudioSamplesToBytes(SAMPLES_PER_BLOCK) * 2];
	vizBuffer = new unsigned char[audio->AudioSamplesToBytes(SAMPLES_PER_BLOCK) * 2];
}

void WMDRM::AudioDataReceived(void *_data, unsigned long sizeBytes, DWORD timestamp)
{
	// TODO: apply replaygain first
	// but if we change bitdepth, we'll have to be careful about calling audio->AudioSamplesToBytes() and similiar functions

	unsigned char *data = (unsigned char *)_data;
	if (!remaining.Empty())
	{
		if (WaitForSingleObject(killswitch, 0) == WAIT_OBJECT_0)
		{
			remaining.Flush();
			return ;
		}
		remaining.UpdatingWrite(data, sizeBytes);
		if (remaining.Full())
		{
			OutputAudioSamples(remaining.GetData(), SAMPLES_PER_BLOCK, timestamp);
			remaining.Flush();
		}
	}

	long samplesLeft = audio->AudioBytesToSamples(sizeBytes);

	while (samplesLeft)
	{
		if (WaitForSingleObject(killswitch, 0) == WAIT_OBJECT_0)
		{
			remaining.Flush();
			return ;
		}

		if (samplesLeft >= SAMPLES_PER_BLOCK)
		{
			OutputAudioSamples(data, SAMPLES_PER_BLOCK, timestamp);
			data += audio->AudioSamplesToBytes(SAMPLES_PER_BLOCK);
			samplesLeft -= SAMPLES_PER_BLOCK;
		}
		else
		{
			unsigned long bytesLeft = audio->AudioSamplesToBytes(samplesLeft);
			remaining.UpdatingWrite(data, bytesLeft);
			samplesLeft = audio->AudioBytesToSamples(bytesLeft); // should always be 0
			assert(samplesLeft == 0);
		}
	}
}


void WMDRM::QuantizedViz(void *data, long sizeBytes, DWORD timestamp)
{
	if (drmProtected)
	{
		assert(sizeBytes == audio->Channels() *(audio->BitSize() / 8) * SAMPLES_PER_BLOCK);
		memset(vizBuffer, 0, sizeBytes);
		ptrdiff_t stride = audio->BitSize() / 8;
		size_t position = stride - 1;
		unsigned char *origData = (unsigned char *)data;
		for (int i = 0;i < SAMPLES_PER_BLOCK*audio->Channels();i++) // winamp hardcodes this ...
		{
			vizBuffer[position] = (origData[position] & 0xFC); // 6 bits of precision, enough for viz.
			position += stride;
		}

		plugin.SAAddPCMData((char *) vizBuffer, audio->Channels(), audio->ValidBits(), timestamp);
		plugin.VSAAddPCMData((char *) vizBuffer, audio->Channels(), audio->ValidBits(), timestamp);
	}
	else
	{
		plugin.SAAddPCMData((char *) data, audio->Channels(), audio->ValidBits(), timestamp);
		plugin.VSAAddPCMData((char *) data, audio->Channels(), audio->ValidBits(), timestamp);
	}
}

long WMDRM::GetPosition()
{
	if (!opened)
		return 0;
	return out->GetWrittenTime();
}

void WMDRM::OutputAudioSamples(void *data, long samples)
{
	DWORD timestamp = out->GetWrittenTime();
	OutputAudioSamples(data, samples, timestamp);
}

void WMDRM::OutputAudioSamples(void *data, long samples, DWORD &timestamp)
{
	clock->SetLastOutputTime(timestamp);
	timestamp += audio->AudioSamplesToMilliseconds(samples);


	//clock->SetLastOutputTime(winamp.GetWrittenTime());

	//in theory, we could check mod->dsp_isactive(), but that opens up a potential race condition ...
	memcpy(dspBuffer, data, audio->AudioSamplesToBytes(samples));
	int dspSize = samples;
	if (!drmProtected)
		dspSize = plugin.dsp_dosamples((short *)dspBuffer, samples, audio->BitSize(), audio->Channels(), audio->SampleRate());
	dspSize = audio->AudioSamplesToBytes(dspSize);
	if (samples == SAMPLES_PER_BLOCK)
		QuantizedViz(dspBuffer, dspSize, timestamp);
	while (out->CanWrite() <= dspSize)
	{
		if (WaitForSingleObject(killswitch, 10) == WAIT_OBJECT_0)
		{
			remaining.Flush();
			return ;
		}
	}
	out->Write((char *)dspBuffer, dspSize);
	/*long bytesAvail = */out->CanWrite();
}

void WMDRM::Opened()
{
	//winamp.ResetBuffering();
	drmProtected = info->IsAttribute(g_wszWMProtected);
	ResetEvent(killswitch);
	if (!audio->IsOpen())
	{
		if (video->IsOpen())
		{
			winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_REALTIME));
			clock->GoRealTime();
			plugin.is_seekable = info->IsSeekable() ? 1 : 0;
			winamp.SetAudioInfo(info->GetBitrate() / 1000, 0, 0);
			//out->SetVolume( -666); // set default volume
		}
		else
		{
			// no audio or video!!
			seek->Stop();
			First().OpenFailed();
			return ;
		}
	}
	else
	{
		BuildBuffers();
		plugin.is_seekable = info->IsSeekable() ? 1 : 0;
		winamp.SetAudioInfo(info->GetBitrate() / 1000, audio->SampleRate() / 1000, audio->Channels());
		out->SetVolume(volume); // set default volume
		out->SetPan(pan);
		winamp.SetVizInfo(audio->SampleRate(), audio->Channels());
	}
	opened = true;
	winamp.ClearStatus();
	reader->Start(clock->GetStartTime(), 0, 1.0f, NULL);
	WMHandler::Opened();

}

void WMDRM::Started()
{
	ResetEvent(killswitch);
	winamp.ResetBuffering();
	winamp.ClearStatus();
	WMHandler::Started();
}
void WMDRM::EndOfFile()
{
	if (audio->IsOpen())
	{
		if (WaitForSingleObject(killswitch, 0) != WAIT_OBJECT_0)
		{
			if (remaining.used)
			{
				OutputAudioSamples(remaining.GetData(), audio->AudioBytesToSamples(remaining.used));
				remaining.Flush();
			}
			out->Write(0, 0);
			while (out->IsPlaying())
			{
				if (WaitForSingleObject(killswitch, 10) == WAIT_OBJECT_0)
				{
					break;
				}
			}
		}
	}

	// TODO: if we have a playlist, start the next track instead of telling winamp to go to the next track
	if (playing)
		winamp.EndOfFile();
	WMHandler::EndOfFile();
}

void WMDRM::NewMetadata()
{
	winamp.RefreshTitle();
	WMHandler::NewMetadata();
}

void WMDRM::Error()
{
	// wait 200 ms for the killswitch (aka hitting stop)
	// this allows the user to hit "stop" and not have to continue cycling through songs if there are a whole bunch of bad/missing WMAs in the playlist
	if (playing && WaitForSingleObject(killswitch, 200) != WAIT_OBJECT_0)
		winamp.EndOfFile();
}

void WMDRM::OpenFailed()
{
	if (playing && WaitForSingleObject(killswitch, 200) != WAIT_OBJECT_0)  // wait 200 ms for the killswitch (see above notes)
		winamp.EndOfFile();
}

void WMDRM::Stopped()
{
	remaining.Flush();
	WMHandler::Stopped();
}

void WMDRM::Kill()
{
	SetEvent(killswitch);
	WMHandler::Kill();
}

void WMDRM::NewSourceFlags()
{
	plugin.is_seekable = info->IsSeekable() ? 1 : 0;
}

void WMDRM::Connecting()
{
	winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_CONNECTING));
	WMHandler::Connecting();
}

void WMDRM::Locating()
{
	winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_LOCATING));
	WMHandler::Locating();
}

void WMDRM::AccessDenied()
{
	winamp.SetStatus(WASABI_API_LNGSTRINGW(IDS_ACCESS_DENIED));
	if (playing && WaitForSingleObject(killswitch, 200) != WAIT_OBJECT_0)  // wait 200 ms for the killswitch (see above notes)
		winamp.PressStop();
}