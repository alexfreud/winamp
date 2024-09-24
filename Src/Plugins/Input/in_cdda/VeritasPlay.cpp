#include "Main.h"
#include <windows.h>
#include "VeritasPlay.h"
#include "CDDB.h"
#include "workorder.h"
#include "api.h"

VeritasPlay::VeritasPlay(bool _ripping)
: opened(false), ripping(_ripping),
primo(0), padStart(false), padEnd(false)
{
	submitHandle=0;
	hThread = NULL;
	overflowBuffer = NULL;
	overflow = 0;
	buffers = NULL;
	currentBuffer = 0;
}

VeritasPlay::~VeritasPlay()
{
	if (opened)
		Abort();

	Close();

	DestroyBuffers();


	if (primo)
	{
		waServiceFactory *sf = (line.service ? line.service->service_getServiceByGuid(obj_primo::getServiceGuid()) : NULL);
		if (sf) sf->releaseInterface(primo);
	}
	
	primo = 0;

	if (submitHandle)
	{
		int x = workorder->CloseSig(submitHandle);
		submitHandle=0;
	}
}

void VeritasPlay::CreateBuffers()
{
	if (buffers)
		return;

	if (ripping)
	{
		buf_size = config_rip_buffersize;
		nb_veritas_buf = config_rip_buffers; 
	}
	else
	{
			buf_size = config_play_buffersize; 
		nb_veritas_buf = config_play_buffers; 
	}

	overflowBuffer = new BYTE[2352 * buf_size];
	overflow = 0;

	buffers = new VeritasBuffer[nb_veritas_buf];
	for (int i = 0;i < nb_veritas_buf;i++)
	{
		buffers[i].Create(buf_size);
	}
}

void VeritasPlay::DestroyBuffers()
{
	if (buffers)
	{
		for (int i = 0;i < nb_veritas_buf;i++)
		{
			buffers[i].Destroy();
		}
		delete [] buffers;
	}

	buffers = 0;

	delete overflowBuffer;
	overflowBuffer = 0;
}

void VeritasPlay::setvolume(int a_v, int a_p)
{
	line.outMod->SetVolume(a_v);
	line.outMod->SetPan(a_p);
}

void VeritasPlay::setoutputtime(int time_in_ms)
{
	need_seek = time_in_ms;
}

void VeritasPlay::stop()
{
	killswitch = 1;

	Close();

	if (hThread)
	{
		WaitForEvent(hThread, INFINITE);
		hThread=0;
	}

	line.outMod->Close();
}

void VeritasPlay::SeekAndFlush()
{
	int destsector = start_sector + ((need_seek * 75) / 1000);
	Abort();
	openVeritasTrack(destsector, end_sector - destsector);
	decode_pos_ms = need_seek;
	// wait for a buffer before we flush
	DWORD cursec = 0, totsec = 0;
	while (true)
	{
		if (primo->RunningStatus(PRIMOSDK_GETSTATUS, &cursec, &totsec) != PRIMOSDK_RUNNING
			&& primo->UnitStatus(&unit, NULL, NULL, NULL, NULL) == PRIMOSDK_UNITERROR)
			return;

		// check how far we've gone
		if (cursec < buffers[currentBuffer].sector)
			Sleep(1);
		else
			break;
	}

	overflow = 0;
	lastseek = destsector;
	line.outMod->Flush(need_seek);
	need_seek = -1;	
}


void VeritasPlay::Seek()
{
	int destsector = start_sector + ((need_seek * 75) / 1000);
	Abort();
	openVeritasTrack(destsector, end_sector - destsector);

	overflow = 0;
	decode_pos_ms = need_seek;
	need_seek = -1;

	lastseek = destsector;
}

void VeritasPlay::Abort()
{
	AbortAsync();
	WaitForAbort(66);
}

void VeritasPlay::AbortAsync()
{
	primo->RunningStatus(PRIMOSDK_ABORT, NULL, NULL);
}

void VeritasPlay::WaitForAbort(int time)
{
	while (primo->RunningStatus(PRIMOSDK_GETSTATUS, NULL, NULL) == PRIMOSDK_RUNNING)
		Sleep(time);
	opened=false;
}

int VeritasPlay::openVeritasTrack(DWORD start, DWORD length)
{
	int speedChoice = config_maxextractspeed;
	DWORD speed;
	if (ripping)
	{
		switch (speedChoice)
		{
		case 0:     // 0.5x
		case 1:     // 1x
			speed = 1; // can't do 0.5x in the SDK
			break;
		case 2:     // 2x
			speed = 2;
			break;
		case 3:     // 4x
			speed = 4;
			break;
		case 4:     // 8x
			speed = 8;
			break;
		case 5:     // 16x
			if (getRegVer() <= 0)
				speed = 8;
			else
				speed = 16;
			break;
		default:
			if (speedChoice < 0)
				speed = PRIMOSDK_MIN;
			else
			{
				if (getRegVer() <= 0)
					speed = 8;
				else
					speed = PRIMOSDK_MAX;
			}
			break;
		}
	}
	else
		speed = 4;//PRIMOSDK_MAX;

	if (primo->ExtractAudioToBuffer(&unit, start, length, speed, 0, 0, 0) != PRIMOSDK_OK)
		return 0;

	for (int i = 0;i < nb_veritas_buf;i++)
	{
		buffers[i].internal = buffers[i].buffer;
		if (i==0 && padStart)
		{
			buffers[i].offset = buf_size*2352 + config_offset*4;
			buffers[i].readSize = buf_size*2352;
			memset((char *)(buffers[i].internal)+buffers[i].offset, 0, buffers[i].readSize);
			buffers[i].sector=0;
			continue;
		}
		if (i==0 && ripping)
			buffers[i].offset = ((2352 + config_offset*4) % 2352);
		if (primo->NextExtractAudioBuffer(buffers[i].buffer, buf_size*2352, &buffers[i].readSize, &buffers[i].sector) != PRIMOSDK_OK)
			return 0;
	}

	currentBuffer = 0;
	opened = true;
	return 1;
}

int VeritasPlay::CopyOverflow(char *sample_buffer, int len)
{
	if (overflow)
	{
		len = min(len, overflow);
		memset(sample_buffer, 0, len);
		memcpy(sample_buffer, overflowBuffer, len);
		overflow -= len;
		return len;
	}
	return 0;
}

void VeritasPlay::OutputOverflow()
{
	while (overflow)
	{
		char sample_buffer[576*4*2] = {0};
		int bytes = 576 * 4;
		int len = min(bytes, overflow);
		memset(sample_buffer, 0, bytes);
		memcpy(sample_buffer, overflowBuffer, len);
		Output(sample_buffer, bytes);
		overflow -= len;
	}
}
#include <assert.h>
void VeritasPlay::OutputBuffer(VeritasBuffer &buffer)
{
	char sample_buffer[576*4*2] = {0};
	size_t bytes = 576 * 4;
	char *bufferPosition = sample_buffer;
	if (overflow)
	{
		assert(overflow < (long)bytes);
		memcpy(bufferPosition, overflowBuffer, overflow);
		bytes -= overflow;
		bufferPosition += overflow;
		overflow = 0;
	}
	BYTE *bufferInput = buffer.buffer;
	while (buffer.readSize)
	{
		if (buffer.readSize < bytes) // if we don't have enough left, save it to overflow
		{
			// if there was overflow last time, and the passed buffer didn't fill us up
			// then we'll have to save both
			BYTE *temp = overflowBuffer;
			int samplesLeft = 576 * 4 - bytes;
			if (samplesLeft)
			{
				memcpy(temp, sample_buffer, samplesLeft);
				temp += samplesLeft;
				overflow += samplesLeft;
			}

			// copy any leftovers of the passed buffer
			memcpy(temp, bufferInput, buffer.readSize);
			bufferInput += buffer.readSize;
			overflow += buffer.readSize;
			buffer.readSize = 0;
			return ;
		}
		else
		{
			memcpy(bufferPosition, bufferInput, bytes);
			bufferPosition = sample_buffer;
			bufferInput += bytes;
			buffer.readSize -= bytes;
			bytes = 576 * 4;
			Output(sample_buffer, 576*4);
		}
	}
}

size_t VeritasPlay::CopyBuffer(VeritasBuffer &buffer, char *&sample_buffer, int &bytes)
{
	// since a buffer is only copied once, this is safe
	buffer.readSize -= buffer.offset;
	buffer.internal += buffer.offset;
	buffer.offset=0;
	size_t len = min((size_t)bytes, buffer.readSize);

	memcpy(sample_buffer, buffer.internal, len);
	buffer.internal += len;
	buffer.readSize -= len;
	sample_buffer += len;
	bytes -= len;
	return len;
}

void VeritasPlay::Output(char *buffer, int len)
{
	line.VSAAddPCMData(buffer, g_nch, 16, line.outMod->GetWrittenTime() /*decode_pos_ms*/);
	line.SAAddPCMData(buffer, g_nch, 16, line.outMod->GetWrittenTime() /*decode_pos_ms*/);

	int bytes = len;
	if (line.dsp_isactive())
		bytes = line.dsp_dosamples((short *)buffer, len / g_nch / 2, 16, g_nch, 44100) * (g_nch * 2);

	while (line.outMod->CanWrite() < bytes && !killswitch) Sleep(10);

	line.outMod->Write(buffer, bytes);

	decode_pos_ms += ((len / g_nch / 2) * 1000) / 44100;
}

int VeritasPlay::read(char *dest, int len, int *killswitch) //called by winampGetExtendedRead_getData
{
	bool noAbort=false;
	int bytesCopied = 0;

	speedLimiter.Limit(killswitch);
	while (!*killswitch)
	{
		DWORD cursec = 0, totsec = 0;
		int r = primo->RunningStatus(PRIMOSDK_GETSTATUS, &cursec, &totsec);

		switch(r)
		{
		case PRIMOSDK_RUNNING:
			break;
		case PRIMOSDK_OK:
			noAbort=true;
			break;
		default:
			goto readabort; // benski> Mr. Faulkner's high school BASIC class prepared me for the real world!
		}

		int a = primo->UnitStatus(&unit, NULL, NULL, NULL, NULL);
		switch(a)
		{
		case PRIMOSDK_OK:
			break;
		default: // todo is everything else really an error? maybe message box for testing purposes
			goto readabort;
		}

		// check how far we've gone
		if (cursec >= buffers[currentBuffer].sector)
		{
			char *olddest=dest;
			int	bytes	= CopyBuffer(buffers[currentBuffer], dest, len);
			if (submitHandle && bytes)
			{
				int res = workorder->WriteSigData(submitHandle, olddest, bytes);
				switch(res)
				{
				case S_OK:
					break;
				case SG_SignatureAcquired:
				default:
					workorder->CloseSig(submitHandle);
					submitHandle=0;
					break;
				}
			}
			speedLimiter.MoreBytesRead(bytes);
			bytesCopied += bytes;

			if (!len || end == 1)
			{
				return bytesCopied;
			}

			if ((buffers[currentBuffer].sector + lastseek) == end_sector) // are we done?
			{
				bytesCopied -= ((2352 - config_offset*4) % 2352);
				end = 1;
				return bytesCopied;
			}

			buffers[currentBuffer].internal = buffers[currentBuffer].buffer;
			buffers[currentBuffer].offset = 0;
			if (!noAbort)
				if (primo) primo->NextExtractAudioBuffer(buffers[currentBuffer].buffer, buf_size*2352, &buffers[currentBuffer].readSize, &buffers[currentBuffer].sector);
			currentBuffer = (currentBuffer + 1) % nb_veritas_buf;
		
		}
		else if (bytesCopied != 0)
			return bytesCopied;
		else
			Sleep(13*buf_size);
	}
	// TODO: we can only get here if killswitch got set or if there was an error
readabort:
	if (submitHandle)
	{
		workorder->AbortSig(submitHandle);
		submitHandle=0;
	}
	AbortAsync();
	return -1;
}

int VeritasPlay::threadProc2()
{
	bool noAbort=false;
	while (!killswitch)
	{
		if (need_seek != -1)
			SeekAndFlush();

		DWORD cursec = 0, totsec = 0;
		int r = primo->RunningStatus(PRIMOSDK_GETSTATUS, &cursec, &totsec);

		switch(r)
		{
		case PRIMOSDK_RUNNING:
			break;
		case PRIMOSDK_OK:
			noAbort=true;
			break;
		default:
			Abort();
			if (!killswitch) Sleep(200);
			if (!killswitch) PostMessage(line.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
			return 0;
		}

		int a = primo->UnitStatus(&unit, NULL, NULL, NULL, NULL);
		switch(a)
		{
		case PRIMOSDK_OK:
			break;
		default: // todo is everything else really an error? maybe message box for testing purposes
			Abort();
			if (!killswitch) Sleep(200);
			if (!killswitch) PostMessage(line.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
			return 0;
		}

		// check how far we've gone
		if (cursec >= buffers[currentBuffer].sector)
		{
			OutputBuffer(buffers[currentBuffer]);

			if ((buffers[currentBuffer].sector+lastseek)==end_sector) // are we done?
				break;

			buffers[currentBuffer].internal = buffers[currentBuffer].buffer;
			if (!noAbort)
				primo->NextExtractAudioBuffer(buffers[currentBuffer].buffer, buf_size*2352, &buffers[currentBuffer].readSize, &buffers[currentBuffer].sector);

			currentBuffer = (currentBuffer + 1) % nb_veritas_buf;
		}
		else
			Sleep(13*buf_size);
	}

	if (killswitch)
	{
		Abort();
		return 0;
	}

	if (!noAbort)
		AbortAsync();

	OutputOverflow();
	//wait for output to be finished
	line.outMod->Write(NULL, 0);
	if (!noAbort)
		WaitForAbort(10);
	while (!killswitch && line.outMod->IsPlaying()) Sleep(10);
	if (!killswitch)
	PostMessage(line.hMainWindow, WM_WA_MPEG_EOF, 0, 0);


	return 0;
}

#define VERITASPLAY_OPEN_FAIL 1
#define VERITASPLAY_OPEN_NOPRIMO 2

int VeritasPlay::open(char drive, int track) //called by winampGetExtendedRead
{
	if ((ripping && !config_rip_veritas))
		return VERITASPLAY_OPEN_FAIL;

	need_seek = -1;

	driveLetter = drive;
	unit = drive;

	hThread = NULL;

	if (!primo)
	{
		waServiceFactory *sf = line.service->service_getServiceByGuid(obj_primo::getServiceGuid());
		if (sf) primo = reinterpret_cast<obj_primo *>(sf->getInterface());
	}

	if (!primo)
		return VERITASPLAY_OPEN_NOPRIMO;

	end = 0;

	speedLimiter.Reset();
	if (getRegVer() <= 0)
		speedLimiter.SetLimit(5);
	else
		speedLimiter.NoLimit();

	if (primo->DiscInfoEx(&unit,0, NULL, NULL, NULL, NULL, NULL, NULL) == PRIMOSDK_OK)
	{
		DWORD sesnum, tracktype, pregap;
		if (primo->TrackInfo(track, &sesnum, &tracktype, &pregap, &start_sector, &sec_length) == PRIMOSDK_OK)
		{
			if (ripping)
			{			
				if (config_offset != 0)
					sec_length++; // TODO: some sort of logic for the last track 
				if (config_offset<0)
				{
					if (track != 1 || config_read_leadin)
						start_sector--; 
					else
					{
						sec_length--;
						padStart=true;
					}
				}
				
			}
			end_sector = start_sector + sec_length;
#if 0 // TODO:  add a config option to skip pregap (maybe separate config for burning and playback)
			start_sector+=pregap; 
			sec_length-=pregap;
#endif
			CreateBuffers();

			if (openVeritasTrack(start_sector, sec_length)) 
			{
				g_nch = 2; // TODO: maybe we should handle red book 4 channel audio?
				g_playlength = (sec_length / 75) * 1000;

				decode_pos_ms = 0;
				lastseek = start_sector;
				need_seek = -1;
				return 0;
			}
		}
	}
	Close();
	return VERITASPLAY_OPEN_FAIL;
}

int VeritasPlay::play(char drive, int track) //called by winamp2
{
	if (!config_use_dae2 || !config_use_veritas)
		return 1;

	hThread=NULL;
	{
		g_playtrack = track;
	}

	switch(open(drive, track))
	{
	case VERITASPLAY_OPEN_FAIL:
		Sleep(200);
		// fall through
	case VERITASPLAY_OPEN_NOPRIMO:
		Close();
		return 1;
	}

	DWORD thread_id;
	hThread = CreateThread(NULL, NULL, &threadProc, (LPVOID)this, CREATE_SUSPENDED, &thread_id);
	SetThreadPriority(hThread, AGAVE_API_CONFIG->GetInt(playbackConfigGroupGUID, L"priority", THREAD_PRIORITY_HIGHEST));

	int maxlat = line.outMod->Open(44100, g_nch, 16, -1, -1);
	if (maxlat < 0) 
	{
		Sleep(200);
		Close();
		CloseHandle(hThread);
		return 1;
	}

	killswitch = 0;

	line.SetInfo(1411, 44, g_nch, 1);
	line.SAVSAInit(maxlat, 44100);
	line.VSASetInfo(g_nch, 44100);
	line.is_seekable = 1;
	ResumeThread(hThread);

	return 0;
}

void VeritasPlay::Close()
{
	driveLetter=0;
}