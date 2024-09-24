#include "api__in_mod.h"
#include "../Winamp/wa_ipc.h"
#include "MODPlayer.h"
#include <libopenmpt/libopenmpt_stream_callbacks_file.h>
#include <nx/nxuri.h>
#include <nx/nxstring.h>
#include <nx/nxfile.h>
#include "../nsutil/pcm.h"

openmpt_module *OpenMod(const wchar_t *filename);

extern int g_duration;
extern In_Module plugin;
// {B6CB4A7C-A8D0-4c55-8E60-9F7A7A23DA0F}
static const GUID playbackConfigGroupGUID =
{
	0xb6cb4a7c, 0xa8d0, 0x4c55, { 0x8e, 0x60, 0x9f, 0x7a, 0x7a, 0x23, 0xda, 0xf }
};

static const size_t kModBufferSize = 512;
static const unsigned int kModSampleRate = 44100; // TODO(benski) configurable!

void MODPlayer::MODWait::Wait_SetEvents(HANDLE killswitch, HANDLE seek_event)
{
	handles[0]=killswitch;
	handles[1]=seek_event;
}

int MODPlayer::MODWait::WaitOrAbort(int time_in_ms)
{
	switch(WaitForMultipleObjects(2, handles, FALSE, time_in_ms))
	{
	case WAIT_TIMEOUT: // all good, wait successful
		return 0; 
	case WAIT_OBJECT_0: // killswitch
		return MODPlayer::MOD_STOP;
	case WAIT_OBJECT_0+1: // seek event
		return MODPlayer::MOD_ABORT;
	default: // some OS error?
		return MODPlayer::MOD_ERROR;
	}
}

MODPlayer::MODPlayer(const wchar_t *_filename) : audio_output(&plugin)
{
	filename = _wcsdup(_filename);
	m_needseek = -1;

	killswitch = CreateEvent(NULL, TRUE, FALSE, NULL);
	seek_event = CreateEvent(NULL, TRUE, FALSE, NULL);

	audio_output.Wait_SetEvents(killswitch, seek_event);
}

MODPlayer::~MODPlayer()
{
	CloseHandle(killswitch);
	CloseHandle(seek_event);
	free(filename);
}

void MODPlayer::Kill()
{
	SetEvent(killswitch);
}

void MODPlayer::Seek(int seek_pos)
{
	m_needseek = seek_pos;
	SetEvent(seek_event);
}

int MODPlayer::GetOutputTime() const
{
	if (m_needseek != -1)
		return m_needseek;
	else
		return plugin.outMod->GetOutputTime();
}

DWORD CALLBACK MODThread(LPVOID param)
{
	MODPlayer *player = (MODPlayer *)param;
	DWORD ret = player->ThreadFunction();
	return ret;	
}

DWORD CALLBACK MODPlayer::ThreadFunction()
{
	float *float_buffer = 0;
	void *int_buffer = 0;

	HANDLE handles[] = {killswitch, seek_event};
	size_t count = 0;
	size_t (*openmpt_read)(openmpt_module * mod, int32_t samplerate, size_t count, float *interleaved_stereo)=openmpt_module_read_interleaved_float_stereo;

	int channels = 2;
	bool force_mono = AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"mono", false);
	bool surround = AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"surround", true);
	int bits = (int)AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"bits", 16);
	if (force_mono) {
		channels = 1;
		openmpt_read = openmpt_module_read_float_mono;
	} else if (surround) {
		channels = 4;
		openmpt_read = openmpt_module_read_interleaved_float_quad;
	}

	// ===== tell audio output helper object about the output plugin =====
	audio_output.Init(plugin.outMod);

	openmpt_module * mod = OpenMod(filename);
	if (!mod) {
		goto btfo;
	}
	openmpt_module_ctl_set(mod, "seek.sync_sample", "1");
	g_duration = (int)(openmpt_module_get_duration_seconds(mod) * 1000);
	audio_output.Open(0, channels, kModSampleRate, bits);

	float_buffer = (float *)malloc(sizeof(float) * kModBufferSize * channels); 
	int_buffer = malloc(kModBufferSize * channels * bits/8);

	while (WaitForMultipleObjects(2, handles, FALSE, 0) != WAIT_OBJECT_0) {
		count = openmpt_read(mod, kModSampleRate, kModBufferSize, float_buffer);
		if (count == 0) {
			break;
		}
		nsutil_pcm_FloatToInt_Interleaved(int_buffer, float_buffer, bits, channels*count);
		int ret = audio_output.Write((char *)int_buffer, channels*count*bits/8);		
		
		if (ret == MOD_STOP) {
			break;
		} else if (ret == MOD_ABORT) {
			ResetEvent(seek_event);
			openmpt_module_set_position_seconds(mod, m_needseek/1000.0);
			audio_output.Flush(m_needseek);
			m_needseek = -1;
		} else if (ret != MOD_CONTINUE) {
			ret = ret;
		}
	}

	if (WaitForSingleObject(killswitch, 0) != WAIT_OBJECT_0) {
		audio_output.Write(0,0);
		audio_output.WaitWhilePlaying();

		if (WaitForSingleObject(killswitch, 0) != WAIT_OBJECT_0) {
			PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
		}
	}
	audio_output.Close();
	openmpt_module_destroy(mod);
	free(float_buffer);
	free(int_buffer);
	return 0;

btfo: // bail the fuck out
	if (WaitForSingleObject(killswitch, 0) != WAIT_OBJECT_0) {
		PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
	}
	audio_output.Close();
	openmpt_module_destroy(mod);
	free(float_buffer);
	free(int_buffer);
	return 1;
}