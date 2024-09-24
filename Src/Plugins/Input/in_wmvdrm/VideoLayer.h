#ifndef NULLSOFT_VIDEOLAYERH
#define NULLSOFT_VIDEOLAYERH

#include "WMHandler.h"
#include "OutputStream.h"
#include <wmsdk.h>
#include "VideoDataConverter.h"
#include "Config.h"
#include "VideoThread.h"

#define VIDEO_ACCEPTABLE_JITTER (config_video_jitter*10000)
#define VIDEO_ACCEPTABLE_JITTER_MS (config_video_jitter)

class VideoLayer : public WMHandler
{
public:
	VideoLayer(IWMReader *_reader);
	~VideoLayer();
		bool IsOpen();
		void Kill(); 
private:
	// WMHandler
	void VideoFrameDrop(DWORD lateness);
	void SampleReceived(QWORD &timeStamp, QWORD &duration, unsigned long &outputNum, unsigned long &flags, INSSBuffer *&sample);
	void Opened();
	void AudioBufferMilliseconds(long ms);
	void Closed();

	void Started();
	void Stopped();
	void HasVideo(bool &video);
	// utility methods
	bool AttemptOpenVideo(VideoOutputStream *attempt);
	bool OpenVideo();
	
	// other people's data
	IWMReader *reader;

	// our data
	IWMReaderAdvanced2 *reader2;
	IWMHeaderInfo *header;
	int videoOutputNum;
	DWORD offset;
	long nextRest;
	VideoDataConverter *converter;
	VideoOutputStream *videoStream;
	
	bool videoOpened;
	QWORD catchupTime;
	double aspect;
	int fourcc;
	bool flip;
	int videoWidth, videoHeight;
	HANDLE killSwitch;
	DWORD earlyDelivery;
	bool drmProtected;
	bool video_output_opened;
	VideoThread videoThread;

};

#endif
