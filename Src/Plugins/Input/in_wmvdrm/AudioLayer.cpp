#include "Main.h"
#include "AudioLayer.h"
#include "VideoLayer.h"
#include <Mmreg.h>
#include <cassert>
#include "util.h"
#include "config.h"
#include "AudioThread.h"
#include "api.h"


#pragma warning(disable:4355) // warning C4355: 'this' : used in base member initializer list

AudioLayer::AudioLayer(IWMReader *_reader)
		: reader(_reader), audioOutputNum( -1),
		reader2(0), offset(0), new_offset(0),
		startPosition(0), videoCatchup(0),
		opened(false), killSwitch(0),
		audioThread(this), latency(0)
{
	reader->AddRef();
	if (FAILED(reader->QueryInterface(&reader2)))
		reader2 = 0;

	killSwitch = CreateEvent(NULL, TRUE, FALSE, NULL);
}

void AudioLayer::Opened()
{

	ResetEvent(killSwitch);
	if (AudioLayer::OpenAudio())
	{
		ResetEvent(killSwitch);

		BOOL dedicatedThread = config_audio_dedicated_thread ? TRUE : FALSE;
		reader2->SetOutputSetting(audioOutputNum, g_wszDedicatedDeliveryThread, WMT_TYPE_BOOL, (BYTE *) & dedicatedThread, sizeof(dedicatedThread));

		BOOL outOfOrder = config_audio_outoforder ? TRUE : FALSE;
		reader2->SetOutputSetting(audioOutputNum, g_wszDeliverOnReceive, WMT_TYPE_BOOL, (BYTE *) & outOfOrder, sizeof(outOfOrder));

		BOOL justInTime = config_lowmemory ? TRUE : FALSE;
		reader2->SetOutputSetting(audioOutputNum, g_wszJustInTimeDecode, WMT_TYPE_BOOL, (BYTE *) & justInTime, sizeof(justInTime));

		opened = true;
		offset = ((QWORD)latency) * 10000;
		new_offset = config_audio_early ? (latency + config_audio_early_pad) : 0;
		reader2->SetOutputSetting(audioOutputNum, g_wszEarlyDataDelivery, WMT_TYPE_DWORD, (BYTE *) & new_offset , sizeof(new_offset));

		winamp.OpenViz(latency, SampleRate());
	}

	WMHandler::Opened();
}

void AudioLayer::Started()
{
	ResetEvent(killSwitch);
	if (opened)
	{
		audioThread.Start(&First());
	}

	WMHandler::Started();
}

void AudioLayer::Stopped()
{
	if (opened)
		audioThread.Stop();
	WMHandler::Stopped();

}

WM_MEDIA_TYPE *NewMediaType(IWMOutputMediaProps *props)
{
	DWORD mediaTypeSize;
	props->GetMediaType(0, &mediaTypeSize);
	WM_MEDIA_TYPE *mediaType = (WM_MEDIA_TYPE *)new unsigned char[mediaTypeSize];
	props->GetMediaType(mediaType, &mediaTypeSize);
	return mediaType;
}

bool AudioLayer::OpenAudio()
{
	audioOutputNum = -1;
	DWORD numOutputs, output, format, numFormats;
	IWMOutputMediaProps *formatProperties;
	GUID mediaType;
	if (FAILED((reader->GetOutputCount(&numOutputs))))
		return false;
	for (output = 0;output < numOutputs;output++)
	{
		HRESULT hr;
		DWORD speakerConfig = config_audio_num_channels;

		if (AGAVE_API_CONFIG && AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"mono", false)) // force mono?
			speakerConfig = DSSPEAKER_MONO;
		else if (AGAVE_API_CONFIG && !AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"surround", true)) // is surround disallowed?
			speakerConfig = DSSPEAKER_STEREO;

		hr = reader2->SetOutputSetting(output, g_wszSpeakerConfig, WMT_TYPE_DWORD, (BYTE *) & speakerConfig, sizeof(speakerConfig));
		assert(hr == S_OK);

		BOOL discreteChannels = TRUE;
		hr = reader2->SetOutputSetting(output, g_wszEnableDiscreteOutput, WMT_TYPE_BOOL, (BYTE *) & discreteChannels , sizeof(discreteChannels ));
		assert(hr == S_OK);

		if (FAILED(reader->GetOutputFormatCount(output, &numFormats)))
			continue;
		for (format = 0;format < numFormats;format++)
		{

			reader->GetOutputFormat(output, format, &formatProperties);
			formatProperties->GetType(&mediaType);
			if (mediaType == WMMEDIATYPE_Audio)
			{

				WM_MEDIA_TYPE *mediaType = NewMediaType(formatProperties);
				if (mediaType->subtype == WMMEDIASUBTYPE_PCM)
				{
					if (AGAVE_API_CONFIG)
					{
						unsigned int bits = AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"bits", 16);
					
						WAVEFORMATEXTENSIBLE *waveFormat = (WAVEFORMATEXTENSIBLE *) mediaType->pbFormat;
						if (waveFormat->Format.cbSize >= 22)
							waveFormat->Samples.wValidBitsPerSample=bits;
						waveFormat->Format.wBitsPerSample=bits;
						waveFormat->Format.nBlockAlign = (waveFormat->Format.wBitsPerSample / 8) * waveFormat->Format.nChannels;
						waveFormat->Format.nAvgBytesPerSec=waveFormat->Format.nSamplesPerSec * waveFormat->Format.nBlockAlign;
						if (FAILED(formatProperties->SetMediaType(mediaType)))
						{
							// blah, just use the default settings then
							delete[] mediaType;
							mediaType = NewMediaType(formatProperties);
						}						
					}
					AudioFormat::Open(mediaType);
					delete mediaType;
					bool video = false;
					First().HasVideo(video);

					// this is needed to prevent an audio glitch on first playback
					if (out)
					{
						extern WMDRM mod;
						out->SetVolume(mod.GetVolume());
						out->SetPan(mod.GetPan());
					}

					latency = out->Open(SampleRate(), Channels(), ValidBits(), (video ? -666 : -1), -1);

					if (latency >= 0)
					{
						audioOutputNum = output;
						reader->SetOutputProps(audioOutputNum, formatProperties);
						formatProperties->Release();
						return true;
					}
					else
					{
						formatProperties->Release();
						AudioFormat::Close();
						continue;
					}
				}

				delete mediaType;
				formatProperties->Release();
			}
		}
	}
	return false;
}

void AudioLayer::SampleReceived(QWORD &timeStamp, QWORD &duration, unsigned long &outputNum, unsigned long &flags, INSSBuffer *&sample)
{
	if (outputNum == audioOutputNum)
	{
		if (WaitForSingleObject(killSwitch, 0) == WAIT_OBJECT_0)
			return ;

		if (videoCatchup)
		{
			videoCatchup = videoCatchup / 20000;
			videoCatchup = min(videoCatchup, offset / 40000);
			unsigned int num = (unsigned int) (videoCatchup / VIDEO_ACCEPTABLE_JITTER_MS);
			while (num--)
			{
				if (WaitForSingleObject(killSwitch, VIDEO_ACCEPTABLE_JITTER_MS) == WAIT_OBJECT_0)
					return ;

			}

			videoCatchup = 0;
		}

		while (!audioThread.AddBuffer(sample, timeStamp, flags, false))
		{
			if (WaitForSingleObject(killSwitch, VIDEO_ACCEPTABLE_JITTER_MS) == WAIT_OBJECT_0)
				break;
		}
	}
	else
		WMHandler::SampleReceived(timeStamp, duration, outputNum, flags, sample);
}

void AudioLayer::VideoCatchup(QWORD time)
{
	videoCatchup = time;
	WMHandler::VideoCatchup(time);
}

void AudioLayer::Closed()
{
	if (opened)
	{
		out->Close();
		winamp.CloseViz();
	}
	opened = false;

	//AudioFormat::Close();
	WMHandler::Closed();
}

AudioLayer::~AudioLayer()
{
	audioThread.Kill();
	if (reader2)
		reader2->Release();
	if (reader)
		reader->Release();
	CloseHandle(killSwitch);
}

void AudioLayer::Kill()
{
	SetEvent(killSwitch);
	if (opened)
		audioThread.SignalStop();
	WMHandler::Kill();

	if (opened)
		audioThread.WaitForStop();
}

void AudioLayer::EndOfFile()
{
	if (!opened || audioThread.EndOfFile())
		WMHandler::EndOfFile();
}
