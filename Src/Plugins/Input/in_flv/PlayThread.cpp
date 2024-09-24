#include "main.h"
#include <windows.h>
#include "api__in_flv.h"
#include "../Winamp/wa_ipc.h"
#include "FLVHeader.h"
#include "FLVStreamHeader.h"
#include "FLVAudioHeader.h"
#include "FLVVideoHeader.h"
#include "FLVMetadata.h"
#include <malloc.h>
#include <stdio.h>
#include <shlwapi.h>
#include "VideoThread.h"
#include "FLVReader.h"
#include "resource.h"
#include "FLVCOM.h"
#include <api/service/waservicefactory.h>
#include "ifc_flvaudiodecoder.h"
#include "svc_flvdecoder.h"
#include "../nu/AudioOutput.h"

#define PRE_BUFFER_MS 5000.0
#define PRE_BUFFER_MAX (1024*1024)

uint32_t last_timestamp=0;
static bool audioOpened;
int bufferCount;
static int bits, channels, sampleRate;
static double dataRate_audio, dataRate_video;
static double dataRate;
static uint64_t prebuffer;
bool mute=false;
uint32_t first_timestamp;
static size_t audio_buffered=0;
void VideoStop();
static ifc_flvaudiodecoder *audioDecoder=0;
static bool checked_in_swf=false;
extern bool video_only;

class FLVWait
{
public:
	int WaitOrAbort(int len)
	{
		if (WaitForSingleObject(killswitch, len) == WAIT_OBJECT_0)
			return 1;

		return 0;
	}
};

static nu::AudioOutput<FLVWait> outputter(&plugin);

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
	videoOutput->notifyBufferState(static_cast<int>(bufStatus*2.55f));
}

static bool Audio_IsSupported(int type)
{
	size_t n = 0;
	waServiceFactory *factory = NULL;
	while (factory = plugin.service->service_enumService(WaSvc::FLVDECODER, n++))
	{
		svc_flvdecoder *creator = (svc_flvdecoder *)factory->getInterface();
		if (creator)
		{
			int supported = creator->HandlesAudio(type);
			factory->releaseInterface(creator);
			if (supported == svc_flvdecoder::CREATEDECODER_SUCCESS)
				return true;
		}		
	}
	return false;
}

static ifc_flvaudiodecoder *CreateAudioDecoder(const FLVAudioHeader &header)
{
	ifc_flvaudiodecoder *audio_decoder=0;
	size_t n=0;
	waServiceFactory *factory = NULL;
	while (factory = plugin.service->service_enumService(WaSvc::FLVDECODER, n++))
	{
		svc_flvdecoder *creator = (svc_flvdecoder *)factory->getInterface();
		if (creator)
		{
			if (creator->CreateAudioDecoder(header.stereo, header.bits, header.sampleRate, header.format, &audio_decoder) == FLV_AUDIO_SUCCESS)
				return audio_decoder;

			factory->releaseInterface(creator);
		}		
	}
	return 0;
}

void OnStart()
{
	Video_Init();

	audioOpened = false;
	audioDecoder = 0;

	bufferCount=0;
	mute = false;
	audio_buffered=0;

	if (!videoOutput)
		videoOutput = (IVideoOutput *)SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GET_IVIDEOOUTPUT);
}

void Audio_Close()
{
	if (audioDecoder)
		audioDecoder->Close();
	audioDecoder=0;
}

static bool OpenAudio(unsigned int sampleRate, unsigned int channels, unsigned int bits)
{
	if (!outputter.Open(0, channels, sampleRate, bits, -666))
		return false;
	audioOpened = true;
	return true;
}

static void DoBuffer(FLVReader &reader)
{
	// TODO: we should pre-buffer after getting the first audio + video frames
	// so we can estimate bitrate
	for (;;)
	{
		uint64_t processedPosition = reader.GetProcessedPosition();
		if (processedPosition < prebuffer && !reader.IsEOF())
		{
			Buffering((int)((100ULL * processedPosition) / prebuffer), WASABI_API_LNGSTRINGW(IDS_BUFFERING));
			if (WaitForSingleObject(killswitch, 100) == WAIT_OBJECT_0)
				break;
			else
				continue;
		}
		else
			break;
	}
}


char pcmdata[65536] = {0};
size_t outlen = 32768;

static uint32_t audio_type;
static void OnAudio(FLVReader &reader, void *data, size_t length, const FLVAudioHeader &header)
{
	if (!audioDecoder)
	{
		audioDecoder = CreateAudioDecoder(header);
		audio_type = header.format;
		video_only=false;
	}

	if (audioDecoder)
	{

		if (!audioDecoder->Ready())
		{
			//first_timestamp = -1;
		}

		outlen = sizeof(pcmdata)/2 - audio_buffered;
		double bitrate = 0;
		int ret = audioDecoder->DecodeSample(data, length, pcmdata+audio_buffered, &outlen, &bitrate);
		if (ret == FLV_AUDIO_SUCCESS)
		{
			outlen+=audio_buffered;
			audio_buffered=0;
			if (bitrate && dataRate_audio != bitrate)
			{
				dataRate_audio = bitrate;
				dataRate = dataRate_audio + dataRate_video;
				plugin.SetInfo((int)dataRate, -1, -1, 1);
			}

			if (!audioOpened)
			{
				// pre-populate values for decoders that use the header info (e.g. ADPCM)
				sampleRate=header.sampleRate;
				channels = header.stereo?2:1;
				bits = header.bits;

				if (audioDecoder->GetOutputFormat((unsigned int *)&sampleRate, (unsigned int *)&channels, (unsigned int *)&bits) == FLV_AUDIO_SUCCESS)
				{

				// buffer (if needed)
				prebuffer = (uint64_t)(dataRate * PRE_BUFFER_MS / 8.0);
				if (prebuffer > PRE_BUFFER_MAX)
					prebuffer=PRE_BUFFER_MAX;
				DoBuffer(reader); // benski> admittedly a crappy place to call this
				if (WaitForSingleObject(killswitch, 0) == WAIT_OBJECT_0)
					return;

				OpenAudio(sampleRate, channels, bits);
				}
			}

			if (mute)
			{
				if (bits == 8) // 8 bit is signed so 128 is zero voltage
					memset(pcmdata, 0x80, outlen);
				else
					memset(pcmdata, 0, outlen);
			}

			if (audioOpened && outlen)
			{
				outputter.Write(pcmdata, outlen);
			}
			else
			{
				audio_buffered=outlen;
			}

		}
		else if (ret == FLV_AUDIO_NEEDS_MORE_INPUT)
		{
			plugin.SetInfo(-1, -1, -1, 0);
		}

	}
}


#define PREBUFFER_BYTES 2048ULL


enum
{
	CODEC_CHECK_NONE=0,
	CODEC_CHECK_AUDIO=1,
	CODEC_CHECK_VIDEO=2,
	CODEC_CHECK_UNSURE = -1,
};

static bool CheckSWF()
{
	if (!checked_in_swf)
	{
		const wchar_t *pluginsDir = (const wchar_t *)SendMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_GETPLUGINDIRECTORYW);
		wchar_t in_swf_path[MAX_PATH] = {0};
		PathCombineW(in_swf_path, pluginsDir, L"in_swf.dll");
		in_swf = LoadLibraryW(in_swf_path);
		checked_in_swf = true;
	}
	return !!in_swf;
}

static void CALLBACK SWFAPC(ULONG_PTR param)
{
	if (in_swf)
	{
		typedef In_Module *(*MODULEGETTER)();
		MODULEGETTER moduleGetter=0;
		moduleGetter = (MODULEGETTER)GetProcAddress(in_swf, "winampGetInModule2");
		if (moduleGetter)
			swf_mod = moduleGetter();
	}

	if (swf_mod)
	{
		if (swf_mod->Play(playFile))
			swf_mod=0;
	}

	if (!swf_mod)
	{
		if (WaitForSingleObject(killswitch, 0) != WAIT_OBJECT_0)
			PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
	}
}

static bool Audio_DecoderReady()
{
	if (!audioDecoder)
		return true;

	return !!audioDecoder->Ready();
}

static bool DecodersReady()
{
	return Audio_DecoderReady() && Video_DecoderReady();
}

DWORD CALLBACK PlayProcedure(LPVOID param)
{
	int needCodecCheck=CODEC_CHECK_UNSURE;
	int missingCodecs=CODEC_CHECK_NONE;
	size_t codecCheckFrame=0;

	dataRate_audio=0;
	dataRate_video=0;
	dataRate=0;
	first_timestamp=-1;

	outputter.Init(plugin.outMod);

	FLVReader reader(playFile);
	size_t i=0;
	bool hasDuration=false;

	OnStart();
	plugin.is_seekable=0;

	prebuffer = PREBUFFER_BYTES;
	bool first_frame_parsed=false;

	for (;;)
	{
		DoBuffer(reader);

		if (WaitForSingleObject(killswitch, paused?100:0) == WAIT_OBJECT_0)
			break;

		if (paused)
			continue;

		if (m_need_seek != -1 && DecodersReady() && first_frame_parsed)
		{
			if (reader.GetPosition(m_need_seek, &i, video_opened))
			{
				VideoFlush();
				if (audioDecoder)
					audioDecoder->Flush();
				FrameData frameData;
				reader.GetFrame(i, frameData);
				outputter.Flush(frameData.header.timestamp);

				if (video_only)
				{
					video_clock.Seek(frameData.header.timestamp);
				}
			}
			uint32_t first_timestamp = 0;
			m_need_seek=-1;
		}

		// update the movie length
		if (!hasDuration)
		{
			if (reader.IsStreaming())
			{
				hasDuration=true;
				g_length = -1000;
				plugin.is_seekable=0;
			}
			else
			{
				g_length=reader.GetMaxTimestamp();
				if (g_length != -1000)
					plugin.is_seekable=1;
			}
			PostMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_UPDTITLE);
		}

		/** if it's a local file or an HTTP asset with content-length
		** we verify that the FLV has codecs we can decode
		** depending on settings, we will do one of the following with unsupport assets
		** 1) Play anyway (e.g. an H.264 video might play audio only)
		** 2) Use in_swf to play back
		** 3) skip
		**/

		if (needCodecCheck == CODEC_CHECK_UNSURE)
		{
			FLVHeader *header = reader.GetHeader();
			if (header)
			{
				needCodecCheck=CODEC_CHECK_NONE;
				if (!reader.IsStreaming())
				{
					if (header->hasVideo)
						needCodecCheck |= CODEC_CHECK_VIDEO;
					if (header->hasAudio)
						needCodecCheck |= CODEC_CHECK_AUDIO;

				}
				if (!header->hasAudio)
				{
					video_only=true;
					video_clock.Start();
				}
			}
		}

		if (needCodecCheck)
		{
			FrameData frameData;
			if (reader.GetFrame(codecCheckFrame, frameData))
			{
				FLVStreamHeader &frameHeader = frameData.header;
				if ((needCodecCheck & CODEC_CHECK_AUDIO) && frameHeader.type == FLV::FRAME_TYPE_AUDIO)
				{
					reader.Seek(frameData.location+15); // TODO: check for -1 return value

					uint8_t data[1] = {0};
					FLVAudioHeader audioHeader;
					size_t bytesRead = reader.Read(data, 1);
					if (audioHeader.Read(data, bytesRead))
					{
						if (Audio_IsSupported(audioHeader.format))
						{
							needCodecCheck &= ~CODEC_CHECK_AUDIO;
						}
						else
						{
							needCodecCheck &= ~CODEC_CHECK_AUDIO;
							missingCodecs|=CODEC_CHECK_AUDIO;
							video_only=true;
						}
					}
				}
				if ((needCodecCheck & CODEC_CHECK_VIDEO) && frameHeader.type == FLV::FRAME_TYPE_VIDEO)
				{
					reader.Seek(frameData.location+15); // TODO: check for -1 return value

					uint8_t data[1] = {0};
					FLVVideoHeader videoHeader;
					size_t bytesRead = reader.Read(data, 1);
					if (videoHeader.Read(data, bytesRead))
					{
						if (Video_IsSupported(videoHeader.format))
						{
							needCodecCheck &= ~CODEC_CHECK_VIDEO;
						}
						else
						{
							needCodecCheck &= ~CODEC_CHECK_VIDEO;
							missingCodecs|=CODEC_CHECK_VIDEO;
						}
					}
				}

				codecCheckFrame++;
			}
			else if (reader.IsEOF())
				break;
			else if (WaitForSingleObject(killswitch, 55) == WAIT_OBJECT_0)
				break;

		}

		if (needCodecCheck)
			continue; // don't start decoding until we've done our codec check

		if (missingCodecs)
		{
			// use in_swf to play this one
			if (CheckSWF())
			{
				HANDLE mainThread = WASABI_API_APP->main_getMainThreadHandle();
				if (mainThread)
				{
					reader.Kill();
					Audio_Close();
					Video_Stop();
					Video_Close();
					QueueUserAPC(SWFAPC, mainThread,0);
					CloseHandle(mainThread);
					return 0;
				}
			}
			else
			{
				FLVHeader *header = reader.GetHeader();
				if (header)
				{
					bool can_play_something = false;

					if (header->hasVideo && !(missingCodecs & CODEC_CHECK_VIDEO))
						can_play_something = true; // we can play video
					else if (header->hasAudio && !(missingCodecs & CODEC_CHECK_AUDIO))
						can_play_something = true; // we can play audio

					if (can_play_something)
					{
						missingCodecs=false;
						continue;
					}
				}
				break; // no header or no codecs at all, bail out
			}
		}

		/* --- End Codec Check --- */
		FrameData frameData;
		if (reader.GetFrame(i, frameData))
		{
			i++;
			uint8_t data[2] = {0};
			FLVStreamHeader &frameHeader = frameData.header;
			reader.Seek(frameData.location+15); // TODO: check for -1 return value
			switch (frameHeader.type)
			{
			default:
#ifdef _DEBUG
				DebugBreak();
#endif
				break;
			case FLV::FRAME_TYPE_AUDIO: // audio
				first_frame_parsed=true;
				if (m_need_seek == -1 || !Audio_DecoderReady())
				{
					FLVAudioHeader audioHeader;
					size_t bytesRead = reader.Read(data, 1);
					if (audioHeader.Read(data, bytesRead))
					{
						size_t dataSize = frameHeader.dataSize - 1;

						uint8_t *audiodata = (uint8_t *)calloc(dataSize, sizeof(uint8_t));
						if (audiodata)
						{
							bytesRead = reader.Read(audiodata, dataSize);
							if (bytesRead != dataSize)
								break;
							if (!reader.IsStreaming())
							{
								if (first_timestamp == -1)
									first_timestamp = frameHeader.timestamp;
								last_timestamp = frameHeader.timestamp;
								last_timestamp = plugin.outMod->GetWrittenTime();
							}

							OnAudio(reader, audiodata, dataSize, audioHeader);
							free(audiodata);
						}
					}
				}
				break;
			case FLV::FRAME_TYPE_VIDEO: // video
				first_frame_parsed=true;
				if (m_need_seek == -1 || !Video_DecoderReady())
				{

					FLVVideoHeader videoHeader;
					size_t bytesRead = reader.Read(data, 1);
					if (videoHeader.Read(data, bytesRead))
					{
						size_t dataSize = frameHeader.dataSize - 1;

						uint8_t *videodata = (uint8_t *)calloc(dataSize, sizeof(uint8_t));
						if (videodata)
						{
							bytesRead = reader.Read(videodata, dataSize);
							if (bytesRead != dataSize)
							{
								free(videodata);
								break;
							}
							if (!OnVideo(videodata, dataSize, videoHeader.format, frameHeader.timestamp))
								free(videodata);
						}
					}
				}
				break;
			case FLV::FRAME_TYPE_METADATA: // metadata
				{
					first_frame_parsed=true;
					size_t dataSize = frameHeader.dataSize;
					uint8_t *metadatadata= (uint8_t *)calloc(dataSize, sizeof(uint8_t));
					if (metadatadata)
					{
						size_t bytesRead = reader.Read(metadatadata, dataSize);
						if (bytesRead != dataSize)
						{
							free(metadatadata);
							break;
						}
						FLVMetadata metadata;
						metadata.Read(metadatadata, dataSize);
						for ( FLVMetadata::Tag *tag : metadata.tags )
						{
							if (!_wcsicmp(tag->name.str, L"onMetaData"))
							{
								AMFType *amf_stream_title;

								amf_stream_title  = tag->parameters->array[L"streamTitle"];
								if (amf_stream_title && amf_stream_title->type == AMFType::TYPE_STRING)
								{
									AMFString *stream_title_string = (AMFString *)amf_stream_title;
									Nullsoft::Utility::AutoLock stream_lock(stream_title_guard);
									free(stream_title);
									stream_title = _wcsdup(stream_title_string->str);
									PostMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_UPDTITLE);
								}

								AMFType *w, *h;
								w = tag->parameters->array[L"width"];
								h = tag->parameters->array[L"height"];
								if (w && h)
								{
									width = (int)AMFGetDouble(w);
									height = (int)AMFGetDouble(h);
								}

								AMFType *duration;
								duration=tag->parameters->array[L"duration"];
								if (duration)
								{
									hasDuration=true;
									plugin.is_seekable=1;
									g_length = (int)(AMFGetDouble(duration)*1000.0);
								}

								// grab the data rate. we'll need this to determine a good pre-buffer.
								AMFType *videoDataRate, *audioDataRate;
								videoDataRate=tag->parameters->array[L"videodatarate"];
								audioDataRate=tag->parameters->array[L"audiodatarate"];
								if (videoDataRate || audioDataRate)
								{
									dataRate_audio = audioDataRate?AMFGetDouble(audioDataRate):0.0;
									dataRate_video = videoDataRate?AMFGetDouble(videoDataRate):0.0;

									dataRate = dataRate_audio + dataRate_video;

									if (dataRate < 1.0f)
										dataRate = 720.0f;
									plugin.SetInfo((int)dataRate, -1, -1, 1);
									prebuffer = (uint64_t)(dataRate * PRE_BUFFER_MS / 8.0);
									if (prebuffer > PRE_BUFFER_MAX)
										prebuffer=PRE_BUFFER_MAX;

									PostMessage(plugin.hMainWindow, WM_WA_IPC, 0, IPC_UPDTITLE);
								}
							}
							flvCOM.MetadataCallback(tag);
						}
						free(metadatadata);
					}
				}
				break;
			}
		}
		else if (reader.IsEOF())
			break;
		else if (WaitForSingleObject(killswitch, 55) == WAIT_OBJECT_0)
			break;
	}

	reader.SignalKill();

	if (WaitForSingleObject(killswitch, 0) != WAIT_OBJECT_0)
	{
		outputter.Write(0,0);
		outputter.WaitWhilePlaying();
		
		if (WaitForSingleObject(killswitch, 0) != WAIT_OBJECT_0)
			PostMessage(plugin.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
	}

	SetEvent(killswitch);

	Video_Stop();
	Video_Close();

	reader.Kill();
	Audio_Close();
	return 0;
}