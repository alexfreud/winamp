#include "FileProcessor.h"
#include "FLVHeader.h"
#include "FLVVideoHeader.h"

static int64_t Seek64(HANDLE hf, int64_t distance, DWORD MoveMethod)
{
	LARGE_INTEGER li;
	li.QuadPart = distance;
	li.LowPart = SetFilePointer(hf, li.LowPart, &li.HighPart, MoveMethod);
	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
		li.QuadPart = -1;
	}

	return li.QuadPart;
}

int64_t FileSize64(HANDLE file)
{
	LARGE_INTEGER position;
	position.QuadPart=0;
	position.LowPart = GetFileSize(file, (LPDWORD)&position.HighPart);

	if (position.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
		return INVALID_FILE_SIZE;
	else
		return position.QuadPart;
}

FileProcessor::FileProcessor()
{
	Init();
}

FileProcessor::FileProcessor(const wchar_t *filename)
{
	Init();
	processedCursor=CreateFile(filename, GENERIC_READ, FILE_SHARE_WRITE|FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	readCursor=CreateFile(filename, GENERIC_READ, FILE_SHARE_WRITE|FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	writePosition=FileSize64(readCursor);
}

FileProcessor::~FileProcessor()
{
	if (processedCursor != INVALID_HANDLE_VALUE)
		CloseHandle(processedCursor);
	if (readCursor != INVALID_HANDLE_VALUE)
		CloseHandle(readCursor);
}

void FileProcessor::Init()
{
	processedPosition=0;
	processedCursor=INVALID_HANDLE_VALUE;
	writePosition=0;
	readCursor=INVALID_HANDLE_VALUE;
	flen=0;
	maxTimeStamp=0;
	headerOK=false;
}

int FileProcessor::Process()
{
	int64_t oldPosition = Seek64(processedCursor, 0, FILE_CURRENT);

	// read file header if we're at the beginning
	if (processedPosition == 0)
	{
		if (writePosition-processedPosition >= 9) // need at least nine bytes for the FLV file header
		{
			uint8_t data[9] = {0};
			DWORD bytesRead=0;
			ReadFile(processedCursor, data, 9, &bytesRead, NULL);
			if (header.Read(data, bytesRead))
			{
				headerOK=true;
				Nullsoft::Utility::AutoLock lock(frameGuard);
				processedPosition += bytesRead;
				return FLV_OK; // we'll make our logic just a little bit more sane by only processing one frame per pass.
			}
			else
				return FLV_ERROR;
		}

		Seek64(processedCursor, oldPosition, FILE_BEGIN); // rollback
		return FLV_NEED_MORE_DATA;
	}

	if (writePosition-processedPosition >= 15) // need at least fifteen bytes for the FLV frame header
	{
		uint8_t data[15] = {0};
		DWORD bytesRead=0;
		ReadFile(processedCursor, data, 15, &bytesRead, NULL);
		FrameData frameData;
		if (frameData.header.Read(data, bytesRead))
		{
			if (frameData.header.dataSize + processedPosition + 15 <= writePosition)
			{
				frameData.keyFrame = false;
				if (frameData.header.type == FLV::FRAME_TYPE_VIDEO)
				{
					FLVVideoHeader videoHeader;
					DWORD videoHeaderRead=0;
					ReadFile(processedCursor, data, 1, &videoHeaderRead, NULL);
					if (videoHeader.Read(data, bytesRead))
					{
						if (videoHeader.frameType == FLV::VIDEO_FRAMETYPE_KEYFRAME)
						{
							frameData.keyFrame = true;
						}
					}
					Seek64(processedCursor, -(int64_t)videoHeaderRead, FILE_CURRENT);
					// roll back file cursor from ReadFile
				}

				{ 					// critsec enter
					Nullsoft::Utility::AutoLock lock(frameGuard);
					// record the offset where we found it
					frameData.location = processedPosition;
					maxTimeStamp=max(maxTimeStamp, frameData.header.timestamp);
					frames.push_back(frameData);
				} // critsec leave
				Seek64(processedCursor, frameData.header.dataSize, FILE_CURRENT);

				Nullsoft::Utility::AutoLock lock(frameGuard);
				processedPosition+=bytesRead+frameData.header.dataSize;
				return FLV_OK;
			}
			Seek64(processedCursor, oldPosition, FILE_BEGIN); // rollback
			return FLV_NEED_MORE_DATA;
		}
		else
		{
			return FLV_ERROR;
		}
	}
	Seek64(processedCursor, oldPosition, FILE_BEGIN); // rollback
	return FLV_NEED_MORE_DATA;
}

uint32_t FileProcessor::GetMaxTimestamp()
{
	return maxTimeStamp;
}

bool FileProcessor::GetFrame(size_t frameIndex, FrameData &frameData)
{
	Nullsoft::Utility::AutoLock lock(frameGuard);
	if (frameIndex < frames.size())
	{
		frameData = frames[frameIndex];
		return true;
	}
	return false;
}

uint64_t FileProcessor::GetProcessedPosition()
{
	Nullsoft::Utility::AutoLock lock(frameGuard);
	return processedPosition;
}

size_t FileProcessor::Read(void *data, size_t bytes)
{
	DWORD bytesRead = 0;
	ReadFile(readCursor, data, (DWORD)bytes, &bytesRead, NULL);
	return bytesRead;
}

uint64_t FileProcessor::Seek(uint64_t position)
{
	return Seek64(readCursor, position, FILE_BEGIN);
}

bool FileProcessor::GetPosition(int time_in_ms, size_t *frameIndex, bool needVideoKeyFrame)
{
	Nullsoft::Utility::AutoLock lock(frameGuard);
	// TODO: binary search
	for (size_t f=0;f!=frames.size();f++)
	{
		if (frames[f].header.timestamp >= (uint32_t)time_in_ms)
		{
			if (needVideoKeyFrame)
			{
				while (f != 0 && !frames[f].keyFrame)
					f--;
			}
			*frameIndex = f;
			return true;
		}
	}
	return false;	
}

FLVHeader *FileProcessor::GetHeader()
{
	if (headerOK)
		return &header;
	else
		return 0;
}