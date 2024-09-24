#include "FLVReader.h"
#include "FLVHeader.h"
#include "FLVStreamheader.h"
#include "BackgroundDownloader.h"
#include "../nu/AutoChar.h"
#include "FileProcessor.h"
#include "ProgressiveProcessor.h"
#include "StreamProcessor.h"
#include "../../..\Components\wac_network\wac_network_http_receiver_api.h"
#include <shlwapi.h>

FLVReader::FLVReader(const wchar_t *_url)
{
	processor = 0;
	url = _wcsdup(_url);
	end_of_stream=false;

	killswitch=false;
	DWORD threadId;
	flvThread=CreateThread(NULL, NULL, ParserThreadStub, this, 0, &threadId);
	SetThreadPriority(flvThread, THREAD_PRIORITY_BELOW_NORMAL);
}

FLVReader::~FLVReader()
{
	free(url);
	delete processor;
}

uint64_t FLVReader::Seek(uint64_t position)
{
	if (processor)
		return processor->Seek(position);
	else
		return -1;
}

size_t FLVReader::Read(void *data, size_t bytes)
{
	if (processor)
		return processor->Read(data, bytes);
	else
		return 0;
}

uint64_t FLVReader::GetProcessedPosition()
{
	if (processor)
		return processor->GetProcessedPosition();
	else
		return 0;
}

void FLVReader::ProcessFile()
{
	if (!processor)
		return;

	while (processor->Process() == FLV_OK)
	{
		if (killswitch) 
			return ;
	}
}

int FLVReader::OnConnect(api_httpreceiver *http)
{
	if (http->content_length())
		processor = new ProgressiveProcessor;
	else
		processor = new StreamProcessor;
	return 0;
}

int FLVReader::OnData(void *data, size_t datalen)
{
	if (!processor)
		return 1;

	bool needSleep=false;
	while (datalen)
	{
		if (killswitch)	
			return 1;

		if (needSleep)
		{
			Sleep(10);
			needSleep=false;
		}

		size_t written=0;
		switch (processor->Write(data, datalen, &written))
		{
		case FLVPROCESSOR_WRITE_ERROR:
			return 1;
		case FLVPROCESSOR_WRITE_WAIT:
			needSleep=true;
			break;
		}
			
		datalen -= written;

		if (written)
		{
			while (1)
			{
				if (killswitch) 
					return 1;
	
				int ret = processor->Process();
				if (ret == FLV_OK)
					continue;
				if (ret == FLV_NEED_MORE_DATA)
					break;
				if (ret == FLV_ERROR)
					return 1;
			}
		}
	}
	return !!killswitch;
}

bool FLVReader::GetFrame(size_t frameIndex, FrameData &frameData)
{
	if (processor)
		return processor->GetFrame(frameIndex, frameData);
	else
	return false;
}

/*
Test URLs (valid of as oct 23 2007)
http://pdl.stream.aol.com/aol/us/aolmusic/artists/astralwerks/2220s/2220s_shootyourgun_hakjfh_700_dl.flv
http://pdl.stream.aol.com/aol/us/aolmusic/sessions/2007/amberpacific/suc_amberpacific_fallbackintomylife_700_dl.flv
http://pdl.stream.aol.com/aol/us/aolmusic/sessions/2007/amberpacific/suc_amberpacific_gonesoyoung_700_dl.flv
http://pdl.stream.aol.com/aol/us/aolmusic/sessions/2007/amberpacific/suc_amberpacific_soyesterday_700_dl.flv
http://pdl.stream.aol.com/aol/us/aolmusic/sessions/2007/amberpacific/suc_amberpacific_watchingoverme_700_dl.flv

a potential crasher:
http://pdl.stream.aol.com/aol/us/aolcomvideo/TVGuide/johncmcginley_6346/johncmcginley_6346_460_700_dl.flv

FLV streams:
http://208.80.52.96/CBS_R20_452P_F128?ext=.flv
http://208.80.54.37/KROQFM?ext=.flv
http://208.80.52.96/CBS_R20_568P_F128?ext=.flv
*/
DWORD FLVReader::ParserThread()
{
	if (PathIsURL(url))
	{
		Downloader downloader;
		downloader.Download(AutoChar(url), this);
	}
	else
	{		
		processor = new FileProcessor(url);
		ProcessFile();
	}
	end_of_stream=true;
	return 0;
}

void FLVReader::Kill()
{
	killswitch=true;
	WaitForSingleObject(flvThread, INFINITE);
	CloseHandle(flvThread);
}

void FLVReader::SignalKill()
{
	killswitch=true;
}

bool FLVReader::IsEOF()
{
	return end_of_stream;
}

bool FLVReader::GetPosition(int time_in_ms, size_t *frameIndex, bool needVideoKeyFrame)
{
	if (processor)
		return processor->GetPosition(time_in_ms, frameIndex, needVideoKeyFrame);
	else
		return false;
}

uint32_t FLVReader::GetMaxTimestamp()
{
	if (processor)
		return processor->GetMaxTimestamp();
	else
		return 0;
}

bool FLVReader::IsStreaming()
{
		if (processor)
		return processor->IsStreaming();
	else
		return true;
}

FLVHeader *FLVReader::GetHeader()
{
	if (processor)
		return processor->GetHeader();
	else
		return 0;
}