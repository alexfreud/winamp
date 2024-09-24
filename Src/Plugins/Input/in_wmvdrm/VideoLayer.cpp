#include "Main.h"
#include "VideoLayer.h"
#include <initguid.h>
#include <wmsdkidl.h>
#include <cassert>
#include "util.h"
#include "resource.h"
#include <strsafe.h>

#include "config.h"
#define VIDEO_ACCEPTABLE_DROP (config_video_drop_threshold*10000)

VideoLayer::VideoLayer(IWMReader *_reader)
		: reader(_reader), videoOutputNum(-1),
		reader2(0), offset(0), nextRest(0),
		converter(NULL), videoOpened(false),
		video_output_opened(false),
		killSwitch(0), aspect(0),
		earlyDelivery(0), fourcc(0),
		drmProtected(false),
		videoStream(0), flip(false),
		videoWidth(0), videoHeight(0)
{
	reader->AddRef();
	if (FAILED(reader->QueryInterface(&reader2)))
		reader2 = 0;

	if (FAILED(reader->QueryInterface(&header)))
		header = 0;

		killSwitch = CreateEvent(NULL, TRUE, FALSE, NULL);
}

VideoLayer::~VideoLayer()
{
	videoThread.Kill();
	if (reader2)
		reader2->Release();
	if (header)
		header->Release();
	reader->Release();
	CloseHandle(killSwitch);
}

bool AcceptableFormat(GUID &subtype)
{
	if (subtype == WMMEDIASUBTYPE_YV12 
	    || subtype == WMMEDIASUBTYPE_YUY2 
	    || subtype == WMMEDIASUBTYPE_UYVY 
	    //|| subtype == WMMEDIASUBTYPE_YVYU 
			|| subtype == WMMEDIASUBTYPE_RGB24
	    || subtype == WMMEDIASUBTYPE_RGB32
	    || subtype == WMMEDIASUBTYPE_I420 
			|| subtype == WMMEDIASUBTYPE_IYUV 
	    || subtype == WMMEDIASUBTYPE_RGB1 
	    || subtype == WMMEDIASUBTYPE_RGB4 
	    || subtype == WMMEDIASUBTYPE_RGB8 
	    || subtype == WMMEDIASUBTYPE_RGB565 
	  ||  subtype == WMMEDIASUBTYPE_RGB555
			)
		return true;
	else
		return false;
}

bool VideoLayer::AttemptOpenVideo(VideoOutputStream *attempt)
{
	videoWidth = attempt->DestinationWidth();
	videoHeight = attempt->DestinationHeight();
	flip = attempt->Flipped();
	fourcc = attempt->FourCC();
	if (!fourcc)
		return false;

	aspect = 1.0;
	return true;

}

bool VideoLayer::OpenVideo()
{
	videoOutputNum = -1; 
	DWORD numOutputs, numFormats;
	IWMOutputMediaProps *formatProperties;
	VideoOutputStream *stream;
	GUID mediaType;

	reader->GetOutputCount(&numOutputs);

	for (DWORD output = 0;output < numOutputs;output++)
	{
		// test the default format first, and if that fails, iterate through the rest
		const int defaultFormat = -1;
		HRESULT hr;
		if (FAILED(hr = reader->GetOutputFormatCount(output, &numFormats)))
			continue;

		for (int format = 0/*defaultFormat*/;format != numFormats;format++)
		{
			if (format == defaultFormat)
				reader->GetOutputProps(output, &formatProperties);
			else
				reader->GetOutputFormat(output, format, &formatProperties);

			formatProperties->GetType(&mediaType);

			if (mediaType == WMMEDIATYPE_Video)
			{
				stream = new VideoOutputStream(formatProperties);

				if (stream->IsVideo() // if it's video
				    && AcceptableFormat(stream->GetSubType())	 // and a video format we like
				    && AttemptOpenVideo(stream)) // and winamp was able to open it
				{
					videoOpened = true;
					int fourcc = stream->FourCC();
					if (fourcc == '8BGR')
					{
						RGBQUAD *palette = stream->CreatePalette();
						winamp.SetVideoPalette(palette);
						
						// TODO: don't leak the palette
					}
					char *cc = (char *) & fourcc;
					char status[512] = {0};
					StringCchPrintfA(status, 512, WASABI_API_LNGSTRING(IDS_WINDOWS_MEDIA_XXX),
									 stream->DestinationWidth(), stream->DestinationHeight(), cc[0], cc[1], cc[2], cc[3]);
					winamp.SetVideoStatusText(status);
					converter = MakeConverter(stream);
					videoOutputNum = output;
					videoStream = stream;
					reader->SetOutputProps(output, formatProperties);
					formatProperties->Release();
					return true;
				}

				delete stream;
				stream = 0;

			}
			formatProperties->Release();
		}
	}
	return false;
}

void VideoLayer::SampleReceived(QWORD &timeStamp, QWORD &duration, unsigned long &outputNum, unsigned long &flags, INSSBuffer *&sample)
{
	if (outputNum == videoOutputNum)
	{
		if (WaitForSingleObject(killSwitch, 0) == WAIT_OBJECT_0)
			return ;

		INSSBuffer3 *buff3;
		if (SUCCEEDED(sample->QueryInterface(&buff3)))
		{
			short aspectHex = 0;
			DWORD size = 2;
			buff3->GetProperty(WM_SampleExtensionGUID_PixelAspectRatio, &aspectHex, &size);
			if (aspectHex)
			{
				double newAspect = (double)((aspectHex & 0xFF00) >> 8) / (double)(aspectHex & 0xFF) ;
	
				if (newAspect != aspect)
				{
					aspect = newAspect;
					videoThread.OpenVideo(drmProtected, videoWidth, videoHeight, flip, aspect, fourcc);
					video_output_opened=true;
				}
			}
			buff3->Release();
		}
				
		if (!video_output_opened)
		{
			videoThread.OpenVideo(drmProtected, videoWidth, videoHeight, flip, aspect, fourcc);
			video_output_opened=true;
		}

		__int64 timeDiff;
		First().TimeToSync(timeStamp, timeDiff);

		if (timeDiff < -VIDEO_ACCEPTABLE_DROP) // late
		{
			timeDiff = -timeDiff;
			if (config_video_catchup) First().VideoCatchup(timeDiff);
			if (config_video_framedropoffset) this->VideoFrameDrop((DWORD)(timeDiff / 10000));
			if (config_video_notifylate) reader2->NotifyLateDelivery(timeDiff);

			// drop the frame
		}
		else	// early
		{
			while (!videoThread.AddBuffer(sample, timeStamp, flags, drmProtected))
			{
				if (WaitForSingleObject(killSwitch, VIDEO_ACCEPTABLE_JITTER_MS) == WAIT_OBJECT_0)
					return ;
      }
		}	
	}
	else
		WMHandler::SampleReceived(timeStamp, duration, outputNum, flags, sample);
}

void VideoLayer::Opened()
{
	WORD stream = 0;
	WMT_ATTR_DATATYPE type = WMT_TYPE_BOOL;
	BOOL value;
	WORD valueLen = sizeof(value);
	header->GetAttributeByName(&stream, g_wszWMProtected, &type, (BYTE *)&value, &valueLen);
	drmProtected = !!value;

	ResetEvent(killSwitch);
	if (OpenVideo())
	{
		ResetEvent(killSwitch);
		HRESULT hr;
		
		BOOL dedicatedThread = config_video_dedicated_thread ? TRUE : FALSE;
		hr = reader2->SetOutputSetting(videoOutputNum, g_wszDedicatedDeliveryThread, WMT_TYPE_BOOL, (BYTE *) & dedicatedThread, sizeof(dedicatedThread));
		assert(hr == S_OK);

		earlyDelivery = config_video_early ? config_video_early_pad : 0;
		hr = reader2->SetOutputSetting(videoOutputNum, g_wszEarlyDataDelivery, WMT_TYPE_DWORD, (BYTE *) & earlyDelivery , sizeof(earlyDelivery));
		assert(hr == S_OK);

		BOOL outOfOrder = config_video_outoforder ? TRUE : FALSE;
		hr = reader2->SetOutputSetting(videoOutputNum, g_wszDeliverOnReceive, WMT_TYPE_BOOL, (BYTE *) & outOfOrder, sizeof(outOfOrder));
		assert(hr == S_OK);

		BOOL justInTime = config_lowmemory ? TRUE : FALSE;
		hr = reader2->SetOutputSetting(videoOutputNum, g_wszJustInTimeDecode, WMT_TYPE_BOOL, (BYTE *) & justInTime, sizeof(justInTime));
		assert(hr == S_OK);
	}
	else
	{
		videoOpened = false;
	}

	WMHandler::Opened();
}

void VideoLayer::VideoFrameDrop(DWORD lateness)
{
	//earlyDelivery+=lateness;
	lateness += earlyDelivery;
	HRESULT hr = reader2->SetOutputSetting(videoOutputNum, g_wszEarlyDataDelivery, WMT_TYPE_DWORD, (BYTE *) & lateness, sizeof(lateness));
	assert(hr == S_OK);
}

void VideoLayer::Closed()
{
	if (video_output_opened)
	{
		videoThread.CloseVideo(drmProtected);
		video_output_opened = false;
	}
	videoOpened = false;
	delete videoStream;
	videoStream=0;
	WMHandler::Closed();
}


bool VideoLayer::IsOpen()
{
	return videoOpened;
}

void VideoLayer::HasVideo(bool &video)
{
	video=videoOpened;
}

void VideoLayer::Kill()
{
	SetEvent(killSwitch);
	if (videoOpened)
		videoThread.SignalStop();//SignalStop();
	
	WMHandler::Kill();
	if (videoOpened)
		videoThread.WaitForStop();
}

void VideoLayer::Started()
{		
	ResetEvent(killSwitch);
	if (videoOpened)
		videoThread.Start(converter, &First());
	WMHandler::Started();
}

void VideoLayer::Stopped()
{
	if (videoOpened)
		videoThread.Stop();
	WMHandler::Stopped();
}
