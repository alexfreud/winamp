#ifndef NULLSOFT_IN_FLV_FLVREADER_H
#define NULLSOFT_IN_FLV_FLVREADER_H

#include <windows.h>
#include <bfc/platform/types.h>
#include "FLVStreamheader.h"
#include "../nu/AutoLock.h"
#include "BackgroundDownloader.h"
#include "FLVProcessor.h"

class FLVReader : private Downloader::DownloadCallback
{
public:
	FLVReader(const wchar_t *_url);
	~FLVReader();

	bool GetFrame(size_t frameIndex, FrameData &frameData);
	bool GetPosition(int time_in_ms, size_t *frameIndex, bool needVideoKeyFrame);
	uint64_t Seek(uint64_t position);
	size_t Read(void *data, size_t bytes);
	void Kill();
	void SignalKill();
	bool IsEOF();
	uint32_t GetMaxTimestamp();
	uint64_t GetProcessedPosition();
	bool IsStreaming();
	FLVHeader *GetHeader();
private:
	void ProcessFile();
	int OnConnect(api_httpreceiver *http);
	int OnData(void *data, size_t datalen);
	int Process();
	DWORD CALLBACK ParserThread();
	static DWORD CALLBACK ParserThreadStub(LPVOID param) { return ((FLVReader *)param)->ParserThread(); }

private:
	bool killswitch;

	HANDLE flvThread;
	bool end_of_stream;
	wchar_t *url;

	/* because we won't know until after opening the stream whether it's progressive download
	or a real-time stream, we need have a pointer to a virtual base class to do the processing
	we'll create a different one depending on what kind of stream */
	FLVProcessor *processor;
};

#endif