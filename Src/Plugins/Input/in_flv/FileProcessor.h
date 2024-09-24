#pragma once
#include "FLVProcessor.h"
#include <windows.h>
#include <vector>
#include "../nu/AutoLock.h"

class FileProcessor : public FLVProcessor
{
public:
	FileProcessor(const wchar_t *filename);
	~FileProcessor();
private:
	/* FLVProcessor virtual method overrides */
	int Write(void *data, size_t datalen, size_t *written) { return 1;	}
	uint64_t GetProcessedPosition();
	uint32_t GetMaxTimestamp();
	bool GetPosition(int time_in_ms, size_t *frameIndex, bool needVideoKeyFrame);
	bool IsStreaming() { return false; }
	FLVHeader *GetHeader();
public:
	/* FLVProcessor virtual method overrides, continued
	(these are public so FileProcessor can be used standalone */
	int Process();
	uint64_t Seek(uint64_t position);
	size_t Read(void *data, size_t bytes);
	bool GetFrame(size_t frameIndex, FrameData &frameData);

protected:
	FileProcessor();
	uint64_t processedPosition;
	uint64_t writePosition;
	HANDLE processedCursor, readCursor;
	// file positions of samples
	std::vector<FrameData> frames;
	Nullsoft::Utility::LockGuard frameGuard;
	uint64_t flen;

private:
	void Init();
	uint32_t maxTimeStamp;
	FLVHeader header;
	bool headerOK;
};