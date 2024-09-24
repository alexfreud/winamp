/*
** Copyright (C) 2007-2011 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
** Author: Ben Allison benski@winamp.com
** Created: March 1, 2007
**
*/
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "main.h"
#include <windows.h>
#include <math.h>
#include <assert.h>
#include <locale.h>
#include <FLAC/all.h>
#include "StreamFileWin32.h"
#include "../Winamp/wa_ipc.h"
#include "QuickBuf.h"
#include "api__in_flv.h"
#include "../nu/AudioOutput.h"
#include "../Agave/Language/api_language.h"
#include "FLACFileCallbacks.h"
#include "nx/nxpath.h"

int m_force_seek = -1; // el hacko grande
const DWORD PAUSE_TIMEOUT = 100; // number of milliseconds to sleep for when paused

static bool paused = false;

// could probably move this inside client_data
int realbits;
int bps;
int samplerate;
int channels;
volatile int currentSongLength=-1000;
int averageBitrate;

// if input plugins weren't single-instance only, we could put this in thread-local-storage
static FLAC__StreamDecoder *decoder = 0;
static uint64_t fileSize = 0;
static FLAC__uint64 decodePosition = 0;

double gain = 1.0;
static double buffer_scale=1.0;

class FLACWait
{
public:
	int WaitOrAbort(int time_in_ms)
	{
			if (WaitForSingleObject(killswitch, time_in_ms) == WAIT_OBJECT_0)
				return 1;
				return 0;
	}
};


volatile int bufferCount=0;
static void Buffering(int bufStatus, const wchar_t *displayString)
{
	if (bufStatus < 0 || bufStatus > 100)
		return;

	char tempdata[75*2] = {0, };

	int csa = plugin.SAGetMode();
	if (csa & 1)
	{
		for (int x = 0; x < bufStatus*75 / 100; x ++)
			tempdata[x] = x * 16 / 75;
	}
	else if (csa&2)
	{
		int offs = (csa & 1) ? 75 : 0;
		int x = 0;
		while (x < bufStatus*75 / 100)
		{
			tempdata[offs + x++] = -6 + x * 14 / 75;
		}
		while (x < 75)
		{
			tempdata[offs + x++] = 0;
		}
	}
	else if (csa == 4)
	{
		tempdata[0] = tempdata[1] = (bufStatus * 127 / 100);
	}
	if (csa)	plugin.SAAdd(tempdata, ++bufferCount, (csa == 3) ? 0x80000003 : csa);	

	/*
	TODO
	wchar_t temp[64] = {0};
	StringCchPrintf(temp, 64, L"%s: %d%%",displayString, bufStatus);
	SetStatus(temp);
	*/
	//SetVideoStatusText(temp); // TODO: find a way to set the old status back
	//	videoOutput->notifyBufferState(static_cast<int>(bufStatus*2.55f));
}

static nu::AudioOutput<FLACWait> audio_output(&plugin);

#pragma region Truncaters
inline static void clip(double &x, double a, double b)
{
	double x1 = fabs(x - a);
	double x2 = fabs(x - b);
	x = x1 + (a + b);
	x -= x2;
	x *= 0.5;
}

static void InterleaveAndTruncate32(const FLAC__int32 *const buffer[], void *_output, int bps, int channels, int blocksize, double gain)
{
	// TODO: this can be sped up significantly
	FLAC__int32 *output = (FLAC__int32 *)_output;

	for (int b = 0;b < blocksize;b++)
	{
		for (int c = 0;c < channels;c++)
		{
			double appliedGain = gain * (double)buffer[c][b];
			// TODO: add dither
			clip(appliedGain, -2147483648., 2147483647.);
			*output = (FLAC__int32)appliedGain;
			output++;
		}
	}
}

static void InterleaveAndTruncate24(const FLAC__int32 *const buffer[], void *_output, int bps, int channels, int blocksize, double gain)
{
	char *output = (char *)_output;

	for (int b = 0;b < blocksize;b++)
	{
		for (int c = 0;c < channels;c++)
		{
			double appliedGain = gain * (double)buffer[c][b];
			// TODO: add dither
			clip(appliedGain, -8388608., 8388607.);
			FLAC__int32 sample = (FLAC__int32)appliedGain;

			// little endian specific code
			output[0] = (unsigned char)(sample);
			output[1] = (unsigned char)(sample >> 8);
			output[2] = (unsigned char)(sample >> 16);
			output += 3;
		}
	}
}

static void InterleaveAndTruncate16(const FLAC__int32 *const buffer[], void *_output, int bps, int channels, int blocksize, double gain)
{
	short *output = (short *)_output;

	for (int b = 0;b < blocksize;b++)
	{
		for (int c = 0;c < channels;c++)
		{
			double appliedGain = gain * (double)buffer[c][b];
			// TODO: add dither
			clip(appliedGain, -32768., 32767.);
			FLAC__int32 sample = (FLAC__int32)appliedGain;

			*output = (short) sample ;
			output ++;
		}
	}
}

static void InterleaveAndTruncate8(const FLAC__int32 *const buffer[], void *_output, int bps, int channels, int blocksize, double gain)
{
	unsigned char *output = (unsigned char *)_output;

	for (int b = 0;b < blocksize;b++)
	{
		for (int c = 0;c < channels;c++)
		{
			double appliedGain = gain * (double)buffer[c][b];
			// TODO: add dither
			clip(appliedGain, -128., 127.);
			FLAC__int32 sample = (FLAC__int32)appliedGain;

			*output = (unsigned char)(sample + 128); 
			output++;
		}
	}
}

/* --- Versions without gain adjustment --- */

static void InterleaveAndTruncate32(const FLAC__int32 *const buffer[], void *_output, int bps, int channels, int blocksize)
{
	FLAC__int32 *output = (FLAC__int32 *)_output;

	for (int b = 0;b < blocksize;b++)
	{
		for (int c = 0;c < channels;c++)
		{
			*output = buffer[c][b];
			output++;
		}
	}
}

static void InterleaveAndTruncate24(const FLAC__int32 *const buffer[], void *_output, int bps, int channels, int blocksize)
{
	char *output = (char *)_output;

	for (int b = 0;b < blocksize;b++)
	{
		for (int c = 0;c < channels;c++)
		{
			FLAC__int32 sample = buffer[c][b];

			// little endian specific code
			output[0] = (unsigned char)(sample);
			output[1] = (unsigned char)(sample >> 8);
			output[2] = (unsigned char)(sample >> 16);
			output += 3;
		}
	}
}

static void InterleaveAndTruncate16(const FLAC__int32 *const buffer[], void *_output, int bps, int channels, int blocksize)
{
	short *output = (short *)_output;

	for (int b = 0;b < blocksize;b++)
	{
		for (int c = 0;c < channels;c++)
		{
			*output = (short) buffer[c][b];
			output ++;
		}
	}
}

static void InterleaveAndTruncate8(const FLAC__int32 *const buffer[], void *_output, int bps, int channels, int blocksize)
{
	unsigned char *output = (unsigned char *)_output;

	for (int b = 0;b < blocksize;b++)
	{
		for (int c = 0;c < channels;c++)
		{
			*output = (unsigned char)(buffer[c][b] + 128); 
			output++;
		}
	}
}

void InterleaveAndTruncate(const FLAC__int32 *const buffer[], void *_output, int bps, int channels, int blocksize, double gain)
{
	if (gain == 1.0) // if it's EXACTLY 1.0, i.e. from a hardcoded value. not meant to be a "RG happens to be 1.0" check
	{
		switch(bps)
		{
		case 8:
			InterleaveAndTruncate8(buffer, _output, bps, channels, blocksize);
			break;
		case 16:
			InterleaveAndTruncate16(buffer, _output, bps, channels, blocksize);
			break;
		case 24:
			InterleaveAndTruncate24(buffer, _output, bps, channels, blocksize);
			break;
		case 32:
			InterleaveAndTruncate32(buffer, _output, bps, channels, blocksize);
			break;
		}
	}
	else // apply replay gain
	{
		switch(bps)
		{
		case 8:
			InterleaveAndTruncate8(buffer, _output, bps, channels, blocksize, gain);
			break;
		case 16:
			InterleaveAndTruncate16(buffer, _output, bps, channels, blocksize, gain);
			break;
		case 24:
			InterleaveAndTruncate24(buffer, _output, bps, channels, blocksize, gain);
			break;
		case 32:
			InterleaveAndTruncate32(buffer, _output, bps, channels, blocksize, gain);
			break;
		}
	}
}
#pragma endregion

QuickBuf output;
FLAC__uint64 lastoutputtime;
static FLAC__StreamDecoderWriteStatus OnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	// TODO: if frame bps/samplerate/channels doesn't equal our own, we'll probably have to close & re-open the audio output
	// mux buffer into interleaved samples
	FLAC__uint64 newPosition;
	FLAC__stream_decoder_get_decode_position(decoder, &newPosition);
	FLAC__uint64 delta = newPosition - decodePosition;
	decodePosition = newPosition;
	if (!config_average_bitrate)
		plugin.SetInfo((int)(delta / (125.*frame->header.blocksize / samplerate)), -1, -1, -1);
	else if (fixBitrate)
	{
		fixBitrate = false;
		plugin.SetInfo(averageBitrate, -1, -1, -1);
	}

	if (frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER)
		lastoutputtime = 1000ULL*frame->header.number.sample_number / (FLAC__uint64)samplerate;
	else
		lastoutputtime = 0;

	int byteLength = (bps / 8) * channels * frame->header.blocksize;
	output.Reserve(byteLength*2);
	InterleaveAndTruncate(buffer, output, bps, channels, frame->header.blocksize, gain);

		audio_output.Write(output, byteLength);
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static int GetBits(int incoming)
{
	int bits = AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"bits", 16);
	if (bits > 16 && AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain", false))
		return bits;

	return min(bits, incoming);
}

static double GetGain(const FLAC__StreamMetadata *metadata)
{
	if (AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain", false))
	{
		if (metadata)
		{
			int gainPos = -1, peakPos = -1;
			switch (AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"replaygain_source", 0))
			{
			case 0:  // track
				gainPos = FLAC__metadata_object_vorbiscomment_find_entry_from(metadata, 0, "REPLAYGAIN_TRACK_GAIN");
				if (gainPos < 0	 && !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
					gainPos = FLAC__metadata_object_vorbiscomment_find_entry_from(metadata, 0, "REPLAYGAIN_ALBUM_GAIN");

				peakPos = FLAC__metadata_object_vorbiscomment_find_entry_from(metadata, 0, "REPLAYGAIN_TRACK_PEAK");
				if (peakPos < 0	 && !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
					peakPos  = FLAC__metadata_object_vorbiscomment_find_entry_from(metadata, 0, "REPLAYGAIN_ALBUM_PEAK");

				break;
			case 1:
				gainPos = FLAC__metadata_object_vorbiscomment_find_entry_from(metadata, 0, "REPLAYGAIN_ALBUM_GAIN");
				if (gainPos < 0	 && !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
					gainPos = FLAC__metadata_object_vorbiscomment_find_entry_from(metadata, 0, "REPLAYGAIN_TRACK_GAIN");

				peakPos = FLAC__metadata_object_vorbiscomment_find_entry_from(metadata, 0, "REPLAYGAIN_ALBUM_PEAK");
				if (peakPos < 0 && !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"replaygain_preferred_only", false))
					peakPos = FLAC__metadata_object_vorbiscomment_find_entry_from(metadata, 0, "REPLAYGAIN_TRACK_PEAK");
				break;
			}

			double dB = 0, peak = 1.0;
			_locale_t C_locale = WASABI_API_LNG->Get_C_NumericLocale();
			if (gainPos >= 0)
			{
				const char *entry = (const char *)metadata->data.vorbis_comment.comments[gainPos].entry;
				const char *value = strchr(entry, '='); // find the first equal
				if (value++)
				{
					if (value[0] == '+')
						dB = _atof_l(&value[1], C_locale);
					else
						dB = _atof_l(value, C_locale);
				}
			}
			else
			{
				dB = AGAVE_API_CONFIG->GetFloat(playbackConfigGroupGUID, L"non_replaygain", -6.0);
				return pow(10.0, dB / 20.0);
			}

			if (peakPos >= 0)
			{
				const char *entry = (const char *)metadata->data.vorbis_comment.comments[peakPos].entry;
				const char *value = strchr(entry, '='); // find the first equal
				if (value++)
				{
					peak = _atof_l(value, C_locale);
				}
			}

			switch (AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"replaygain_mode", 1))
			{
			case 0:  // apply gain
				return pow(10.0, dB / 20.0);
				break;
			case 1:  // apply gain, but don't clip
				return min(pow(10.0, dB / 20.0), 1.0 / peak);
				break;
			case 2:  // normalize
				return 1.0 / peak;
				break;
			case 3:  // prevent clipping
				if (peak > 1.0)
					return 1.0 / peak;
				else
					return 1.0;
			}
		}
		else
		{
			double dB = AGAVE_API_CONFIG->GetFloat(playbackConfigGroupGUID, L"non_replaygain", -6.0);
			return pow(10.0, dB / 20.0);
		}
	}

	return 1.0;
}

static void OnMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	switch (metadata->type)
	{
	case FLAC__METADATA_TYPE_STREAMINFO:
		{
			realbits = metadata->data.stream_info.bits_per_sample;
			channels = metadata->data.stream_info.channels;
			samplerate = metadata->data.stream_info.sample_rate;
			bps = GetBits(metadata->data.stream_info.bits_per_sample);
			gain = GetGain(0) * pow(2., (double)(bps - realbits));
			if (metadata->data.stream_info.total_samples)
			{
				currentSongLength = (int)((metadata->data.stream_info.total_samples*1000ULL)/(uint64_t)samplerate);
				averageBitrate = (int)(fileSize / (125 * metadata->data.stream_info.total_samples / (__int64)samplerate));
			}
			else
			{
				currentSongLength=-1000;
				averageBitrate=0;
			}
		}
		break;
	case FLAC__METADATA_TYPE_VORBIS_COMMENT:
		gain = GetGain(metadata) * pow(2., (double)(bps - realbits));
		break;
	}
}

static void OnError(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	//client_data = client_data; // dummy line so i can set a breakpoint
}

void CALLBACK APCPause(ULONG_PTR data)
{
	paused = !!data;
	plugin.outMod->Pause(!!paused);
}

void CALLBACK APCSeek(ULONG_PTR data)
{
	// TODO: break out of end-of-file handling if necessary
	buffer_scale=1.0;
	int time_in_ms = (int)data;
	lastoutputtime=time_in_ms; // cheating a bit here :)
	audio_output.Flush(time_in_ms);
	__int64 frames = Int32x32To64(time_in_ms, samplerate) / 1000;
	FLAC__StreamDecoderState state = FLAC__stream_decoder_get_state(decoder);
	plugin.outMod->Pause(0);
	FLAC__stream_decoder_seek_absolute(decoder, frames);
	plugin.outMod->Pause(paused);
	if (FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_SEEK_ERROR)
	{
		FLAC__stream_decoder_flush(decoder);
	}
}

static const double prebuffer_seconds = 0.5;
static bool DoBuffering(nx_file_t file)
{
	// TODO: check for disconnect, etc.

		// anything less than half-a-second and we'll rebuffer.  but when we rebuffer, we'll get 2 seconds worth of audio
	// also, every time we're forced to re-buffer, we'll double the buffer amount

	uint64_t required;
	if (averageBitrate > 0)
	{
		required = (uint64_t)((double)averageBitrate * 1000.0 * buffer_scale  * prebuffer_seconds  / 8.0);  
	}
	else /* no bitrate specified, let's assume 1000kbps */
	{
		required = (uint64_t)(1000.0 * 1000.0 * buffer_scale * prebuffer_seconds / 8.0);
	}

	uint64_t available;	
	if (NXFileProgressiveDownloaderAvailable(file, required, &available) == NErr_True)
		return true;
																													
	Buffering(0, 0);
	bufferCount=lastoutputtime;
	required *= 4;
	buffer_scale *= 2.0;

	while (NXFileProgressiveDownloaderAvailable(file, required, &available) == NErr_False)
	{
		int percent = (int)((double)available * 100.0 / (double)required);
		if (percent <= 0)
			percent=1;
		if (percent > 99)
			percent=99;
		
		Buffering(percent, 0);
		if (WaitForSingleObject(killswitch, 10) == WAIT_OBJECT_0)
		{
			return false;
		}
	}
		Buffering(100, 0);
		bufferCount=0;
		return true;	
}

extern HANDLE threadStarted;
DWORD CALLBACK FLACThread(LPVOID param)
{
	buffer_scale=1.0;
	bool streaming=false;
	nx_file_t file;
	FLACClientData state;
	audio_output.Init(plugin.outMod);
	paused=false;
	SetEvent(threadStarted);
	nx_uri_t filename = (nx_uri_t)param;
	decodePosition = 0;
	gain = 1.0;
	bufferCount = 0;
	decoder = FLAC__stream_decoder_new();
	if (decoder == 0)
	{
		if (WaitForSingleObject(killswitch, 200) != WAIT_OBJECT_0)
			PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);

		free(filename);
		return 0;
	}

	FLAC__stream_decoder_set_md5_checking(decoder, false);
	FLAC__stream_decoder_set_metadata_respond(decoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);

	plugin.is_seekable = 1; 


	int ret;
		
	if (NXPathIsURL(filename) == NErr_True)
	{
		// Display a window to request to download
		// 
		MessageBox(NULL, L"Cannot stream flac file, please download it", NULL, 0);
		/*
			char agent[256] = { 0 };
			snprintf(agent, 256, "User-Agent: %S/%S", "Winamp", "5.9.1");
			ret = NXFileOpenProgressiveDownloader(&file, filename, nx_file_FILE_read_binary, 0, agent); // TODO: calculate real user agent
		*/
		return NErr_Disabled;
	}
	else
	{
		ret = NXFileOpenFile(&file, filename, nx_file_FILE_read_binary);
	}

	if (ret != NErr_Success)
	{
		FLAC__stream_decoder_delete(decoder);
		if (WaitForSingleObject(killswitch, 200) != WAIT_OBJECT_0)
			PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);

		NXURIRelease(filename);
		return 0;
	}

	state.SetFile(file);

	NXFileLength(file, &fileSize);

	if (FLAC__stream_decoder_init_stream(
		decoder,
		FLAC_NXFile_Read,
		FLAC_NXFile_Seek,
		FLAC_NXFile_Tell,
		FLAC_NXFile_Length,
		FLAC_NXFile_EOF,
		OnAudio,
		OnMetadata,  // or NULL
		OnError,
		&state
		) != FLAC__STREAM_DECODER_INIT_STATUS_OK)
	{
		FLAC__stream_decoder_delete(decoder);
		NXFileRelease(file);
		if (WaitForSingleObject(killswitch, 200) != WAIT_OBJECT_0)
			PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);

		NXURIRelease(filename);
		return 0;
	}

	FLAC__stream_decoder_process_until_end_of_metadata(decoder);



	if (!audio_output.Open(0, channels, samplerate, bps))
	{
		FLAC__stream_decoder_finish(decoder);
		FLAC__stream_decoder_delete(decoder);
		NXFileRelease(file);
		if (WaitForSingleObject(killswitch, 200) != WAIT_OBJECT_0)
			PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);

		NXURIRelease(filename);
		return 0;
	}

	plugin.SetInfo(averageBitrate, -1, -1, -1);
	plugin.outMod->SetVolume(volume);
	plugin.outMod->SetPan(pan);

	if (streaming && !DoBuffering(file))
	{
		FLAC__stream_decoder_finish(decoder);
		FLAC__stream_decoder_delete(decoder);
		NXFileRelease(file);
		NXURIRelease(filename);
		
		return 0;
	}

	if (m_force_seek != -1)
		APCSeek((ULONG_PTR)m_force_seek);
	m_force_seek = -1;

	HANDLE events[] = {killswitch};
	DWORD timeout = 0;
	while (true)
	{
		DWORD result = WaitForMultipleObjectsEx(sizeof(events) / sizeof(*events), events, FALSE, timeout, TRUE);
		switch (result)
		{
		case WAIT_OBJECT_0:// kill thread
			FLAC__stream_decoder_finish(decoder);
			FLAC__stream_decoder_delete(decoder);
			NXFileRelease(file);
			NXURIRelease(filename);
			return 0;

		case WAIT_TIMEOUT:
			timeout = paused ? PAUSE_TIMEOUT : 0;
				if (streaming && !DoBuffering(file))
	{
		FLAC__stream_decoder_finish(decoder);
		FLAC__stream_decoder_delete(decoder);
		NXFileRelease(file);
		NXURIRelease(filename);
		
		return 0;
	}

			if (!paused)
			{
				FLAC__bool decode_successful = FLAC__stream_decoder_process_single(decoder);

				FLAC__StreamDecoderState FLACstate = FLAC__stream_decoder_get_state(decoder);

				if (FLACstate == FLAC__STREAM_DECODER_END_OF_STREAM)
				{
					audio_output.Write(0, 0);
					if (audio_output.WaitWhilePlaying())
					{
							FLAC__stream_decoder_finish(decoder);
							FLAC__stream_decoder_delete(decoder);
							NXFileRelease(file);
							NXURIRelease(filename);
							return 0;
					}
					PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
					timeout = INFINITE; // sit and wait for killswitch
				} 
				else if (!decode_successful)
				{
					// some other error - abort playback
					// if we can find FLAC files with errors, we might be able to gracefully handle some errors
					PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
					timeout = INFINITE; // sit and wait for killswitch
				}
			}
			break;
		}
	}

	return 0;
}
