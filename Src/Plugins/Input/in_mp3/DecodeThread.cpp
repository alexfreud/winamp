#include "DecodeThread.h"
#include "giofile.h"
#include "main.h"
#include "pdtimer.h"
#include "mpegutil.h"
#include "../Winamp/wa_ipc.h"
#include "config.h"
#include <shlwapi.h>
#include "adts.h"
#include "adts_vlb.h"
#include <foundation/error.h>

// {19450308-90D7-4E45-8A9D-DC71E67123E2}
static const GUID adts_aac_guid = 
{ 0x19450308, 0x90d7, 0x4e45, { 0x8a, 0x9d, 0xdc, 0x71, 0xe6, 0x71, 0x23, 0xe2 } };

// {4192FE3F-E843-445c-8D62-51BE5EE5E68C}
static const GUID adts_mp2_guid = 
{ 0x4192fe3f, 0xe843, 0x445c, { 0x8d, 0x62, 0x51, 0xbe, 0x5e, 0xe5, 0xe6, 0x8c } };

extern int m_is_stream;
extern bool m_is_stream_seekable;

// post this to the main window at end of file (after playback as stopped)
#define WM_WA_MPEG_EOF WM_USER+2

/* public data */
int last_decode_pos_ms;
int decode_pos_ms; // current decoding position, in milliseconds.
volatile int seek_needed; // if != -1, it is the point that the decode
// thread should seek to, in ms.
int g_ds;

size_t g_bits;
int g_sndopened;
int g_bufferstat;
int g_length = -1000;
int g_vis_enabled;
volatile int g_closeaudio = 0;

CGioFile *g_playing_file=0;
/* private data */
static size_t g_samplebuf_used;
static int need_prebuffer;
static int g_srate, g_nch, g_br_add, g_br_div, g_avg_vbr_br;
int g_br;

class EndCutter
{
public:
	EndCutter() : buffer(0), cutSize(0), filledSize(0), preCutSize(0), preCut(0), decoderDelay(0)
	{}
	~EndCutter()
	{
		free(buffer);
	}
	void SetEndSize(int postSize)
	{
		postSize -= decoderDelay;
		if (postSize < 0)
			postSize = 0;
		else if (postSize)
		{
			free(buffer);
			buffer = (char *)calloc(postSize, sizeof(char));
			cutSize = postSize;
		}
	}

	void SetSize(int decoderDelaySize, int preSize, int postSize)
	{
		decoderDelay = decoderDelaySize;
		SetEndSize(postSize);

		preCutSize = preSize;
		preCut = preCutSize + decoderDelay;
	}

	void Flush(int time_in_ms)
	{
		if (time_in_ms == 0) // TODO: calculate actual delay if we seek within the encoder delay area
			preCut = preCutSize; // reset precut size if we seek to the start

		filledSize = 0;
		mod.outMod->Flush(time_in_ms);
	}

	void Write(char *out, int outSize)
	{
		if (!out && (!outSize))
		{
			mod.outMod->Write(0, 0);
			return ;
		}

		// cut pre samples, if necessary
		int pre = min(preCut, outSize);
		out += pre;
		outSize -= pre;
		preCut -= pre;

		if (!outSize)
			return ;

		int remainingFill = cutSize - filledSize;
		int fillWrite = min(outSize - remainingFill, filledSize); // only write fill buffer if we've got enough left to fill it up

		if (fillWrite > 0)
		{
			mod.outMod->Write((char *)buffer, fillWrite);
			if (cutSize - fillWrite)
				memmove(buffer, buffer + fillWrite, cutSize - fillWrite);
			filledSize -= fillWrite;

		}
		remainingFill = cutSize - filledSize;
		int outWrite = max(0, outSize - remainingFill);
		if (outWrite)
			mod.outMod->Write((char *)out, outWrite);
		out += outWrite;
		outSize -= outWrite;

		if (outSize)
		{
			memcpy(buffer + filledSize, out, outSize);
			filledSize += outSize;
		}


	}
	char *buffer;
	int cutSize;
	int filledSize;
	int preCut, preCutSize, decoderDelay;
};

class DecodeLoop
{
public:
	DecodeLoop() : decoder(0)
	{
		isAac = 0;
		isEAAC = 0;

		last_bpos = -1;
		need_synclight = true;
		done = 0;
		br = 0;

		g_framesize = 0;
		maxlatency = 0;
		sampleFrameSize = 0;
		memset(&g_samplebuf, 0, sizeof(g_samplebuf));
	}

	~DecodeLoop()
	{
		if (decoder)
		{
			decoder->Close();
			decoder->Release();
		}
		decoder=0;

	}

	DWORD Loop();
	DWORD OpenDecoder();
	void Seek(int seekPosition);
	void PreBuffer();
	void Decode();
	void Viz();
	void CalculateCodecDelay();
	DWORD OpenOutput(int numChannels, int sampleRate, int bitsPerSample);
	void SetupStream();

	BYTE g_samplebuf[6*3*2*2*1152];

	int g_framesize;
	int isAac;
	int isEAAC;

	CGioFile file;

	int maxlatency;
	int last_bpos;
	bool need_synclight;
	int done; // set to TRUE if decoding has finished, 2 if all has been written
	size_t br;

	EndCutter endCutter;
	int sampleFrameSize;
	adts *decoder;
};

static int CalcPreBuffer(int buffer_setting, int bitrate)
{
	if (bitrate < 8) 
		bitrate = 8;
	else if (bitrate > 320) 
		bitrate = 320;
	int prebuffer = (buffer_setting * bitrate) / 128;
	if (prebuffer > 100)
		prebuffer=100;
	return prebuffer;
}

void DecodeLoop::SetupStream()
{
	char buf[1024] = {0};
	int len;

	m_is_stream = file.IsStream();

	//Wait until we have data...
	while (!killDecodeThread && file.Peek(buf, 1024, &len) == NErr_Success && !len)
		Sleep(50);

	m_is_stream_seekable = file.IsStreamSeekable();
	char *content_type = file.m_content_type;
	if (content_type)
	{
		if (!_strnicmp(content_type, "misc/ultravox", 13))
		{
			switch (file.uvox_last_message)
			{
			case 0x8001:
			case 0x8003:
				isEAAC = 1;
				isAac = 1;
				break;

			case 0x8000:
				isAac = 1;
				break;
			}
		}
		else if (!_strnicmp(content_type, "audio/aac", 9))
		{
			isEAAC = 1;
			isAac = 1;
		}
		else if (!_strnicmp(content_type, "audio/aacp", 10))
		{
			isEAAC = 1;
			isAac = 1;
		}
		else if (!_strnicmp(content_type, "audio/apl", 10))
		{
			isEAAC = 1;
			isAac = 1;
		}
	}

	// todo: poll until connected to see if we get aac uvox frames or a content-type:aac header
}

DWORD DecodeLoop::OpenOutput(int numChannels, int sampleRate, int bitsPerSample)
{
	maxlatency = mod.outMod->Open(sampleRate, numChannels, bitsPerSample, -1, -1);

	// maxlatency is the maxium latency between a outMod->Write() call and
	// when you hear those samples. In ms. Used primarily by the visualization
	// system.

	if (maxlatency < 0) // error opening device
	{
		PostMessage(mod.hMainWindow, WM_COMMAND, 40047, 0);
		return 0;
	}
	g_sndopened = 1;
	if (maxlatency == 0 && file.IsStream() == 2) // can't use with disk writer
	{
		if (!killDecodeThread)
		{
			EnterCriticalSection(&g_lfnscs);
			WASABI_API_LNGSTRING_BUF(IDS_CANNOT_WRITE_STREAMS_TO_DISK,lastfn_status,256);
			LeaveCriticalSection(&g_lfnscs);
			PostMessage(mod.hMainWindow, WM_USER, 0, IPC_UPDTITLE);
		}
		if (!killDecodeThread) Sleep(200);
		if (!killDecodeThread) PostMessage(mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
		g_bufferstat = 0;
		g_closeaudio = 1;

		return 0;
	}

	if (paused) mod.outMod->Pause(1);

	// set the output plug-ins default volume.
	// volume is 0-255, -666 is a token for
	// current volume.

	mod.outMod->SetVolume(-666);
	return 1;
}

void DecodeLoop::CalculateCodecDelay()
{
	int decoderDelaySamples = (int)decoder->GetDecoderDelay();

	endCutter.SetSize(decoderDelaySamples*sampleFrameSize,
	                  file.prepad*sampleFrameSize,
	                  file.postpad*sampleFrameSize);
}

void DecodeLoop::Viz()
{
	if (!config_fastvis || (decoder->GetLayer() != 3 || g_ds))
	{
		int vis_waveNch;
		int vis_specNch;
		int csa = mod.SAGetMode();
		int is_vis_running = mod.VSAGetMode(&vis_specNch, &vis_waveNch);
		if (csa || is_vis_running)
		{
			int l = 576 * sampleFrameSize;
			int ti = decode_pos_ms;
			{
				if (g_ds == 2)
				{
					memcpy(g_samplebuf + g_samplebuf_used, g_samplebuf, g_samplebuf_used);
				}
				size_t pos = 0;
				while (pos < g_samplebuf_used)
				{
					int a, b;
					if (mod.SAGetMode()) mod.SAAddPCMData((char *)g_samplebuf + pos, g_nch, (int)g_bits, ti);
					if (mod.VSAGetMode(&a, &b)) mod.VSAAddPCMData((char *)g_samplebuf + pos, g_nch, (int)g_bits, ti);
					ti += ((l / sampleFrameSize * 1000) / g_srate);
					pos += l >> g_ds;
				}
			}
		}
	}
	else
	{
		int l = (576 * (int)g_bits * g_nch);
		int ti = decode_pos_ms;
		size_t pos = 0;
		int x = 0;
		while (pos < g_samplebuf_used)
		{
			do_layer3_vis((short*)(g_samplebuf + pos), &g_vis_table[x++][0][0][0], g_nch, ti);
			ti += (l / g_nch / 2 * 1000) / g_srate;
			pos += l;
		}
	}
}

void DecodeLoop::Decode()
{
	while (g_samplebuf_used < (size_t)g_framesize && !killDecodeThread && seek_needed == -1)
	{
		size_t newl = 0;
		size_t br=0;
		size_t endCut=0;
		int res = decoder->Decode(&file, g_samplebuf + g_samplebuf_used, sizeof(g_samplebuf) / 2 - g_samplebuf_used, &newl, &br, &endCut);

		if (config_gapless && endCut)
			endCutter.SetEndSize((int)endCut* sampleFrameSize);

		// we're not using switch here because we sometimes need to break out of the while loop
		if (res == adts::SUCCESS)
		{
			if (!file.m_vbr_frames)
			{
				if (br) {
					bool do_real_br=false;
					if (!(config_miscopts&2) && br != decoder->GetCurrentBitrate())
					{
						do_real_br=true;
					}

					int r = (int)br;
					g_br_add += r;
					g_br_div++;
					r = (g_br_add + g_br_div / 2) / g_br_div;
					if (g_br != r)
					{
						need_synclight = false;
						g_br = r;
						if (!file.m_vbr_frames && file.IsSeekable()) g_length = MulDiv(file.GetContentLength(), 8, g_br);
						if (!do_real_br)
							mod.SetInfo(g_br, -1, -1, 1);
					}
					if (do_real_br)
						mod.SetInfo((int)br, -1, -1, 1);
				}
			}
			else
			{
				if (br) {
					int r;
					if (!(config_miscopts&2) || !g_avg_vbr_br)
						r = (int)br;
					else r = g_avg_vbr_br;
					if (g_br != r)
					{
						need_synclight = false;
						g_br = r;
						mod.SetInfo(g_br, -1, -1, 1);
					}
				}
			}
			if (need_synclight)
			{
				need_synclight = false;
				mod.SetInfo(-1, -1, -1, 1);
			}
			g_samplebuf_used += newl;
		}
		else if (res == adts::ENDOFFILE)
		{
			done = 1;
			break;
		}
		else if (res == adts::NEEDMOREDATA)
		{
			if (file.IsStream() && !need_synclight)
			{
				need_synclight = true; mod.SetInfo(-1, -1, -1, 0);
			}
			if (file.IsStream() && !mod.outMod->IsPlaying())
			{
				need_prebuffer = CalcPreBuffer(config_http_prebuffer_underrun, (int)br);
			}
			break;
		}
		else
		{
			if (!need_synclight) mod.SetInfo(-1, -1, -1, 0);
			need_synclight = true;
			break;
		}
	}
}

void DecodeLoop::PreBuffer()
{
	int p = file.RunStream();
	int pa = file.PercentAvailable();
	if (pa >= need_prebuffer || p == 2)
	{
		EnterCriticalSection(&g_lfnscs);
		lastfn_status[0] = 0;
		LeaveCriticalSection(&g_lfnscs);
		PostMessage(mod.hMainWindow, WM_USER, 0, IPC_UPDTITLE);
		need_prebuffer = 0;
		g_bufferstat = 0;
		last_bpos = -1;
	}
	else
	{
		int bpos = pa * 100 / need_prebuffer;
		if (!g_bufferstat) g_bufferstat = decode_pos_ms;
		if (bpos != last_bpos)
		{
			last_bpos = bpos;
			EnterCriticalSection(&g_lfnscs);
			if (stricmp(lastfn_status, "stream temporarily interrupted"))
			{
				char langbuf[512] = {0};
				wsprintfA(lastfn_status, WASABI_API_LNGSTRING_BUF(IDS_BUFFER_X,langbuf,512), bpos);
			}
			LeaveCriticalSection(&g_lfnscs);

			int csa = mod.SAGetMode();
			char tempdata[75*2] = {0, };
			int x;
			if (csa&1)
			{
				for (x = 0; x < bpos*75 / 100; x ++)
				{
					tempdata[x] = x * 16 / 75;
				}
			}
			if (csa&2)
			{
				int offs = (csa & 1) ? 75 : 0;
				x = 0;
				while (x < bpos*75 / 100)
				{
					tempdata[offs + x++] = -6 + x * 14 / 75;
				}
				while (x < 75)
				{
					tempdata[offs + x++] = 0;
				}
			}
			if (csa == 4)
			{
				tempdata[0] = tempdata[1] = (bpos * 127 / 100);
			}

			if (csa) mod.SAAdd(tempdata, ++g_bufferstat, (csa == 3) ? 0x80000003 : csa);
			PostMessage(mod.hMainWindow, WM_USER, 0, IPC_UPDTITLE);
		}
	}
}

void DecodeLoop::Seek(int seekPosition)
{
	if (done == 3)
		return;
	done=0;
	int br = (int)decoder->GetCurrentBitrate();

	need_prebuffer = CalcPreBuffer(config_http_prebuffer_underrun, br);
	if (need_prebuffer < 1) need_prebuffer = 5;

	last_decode_pos_ms = decode_pos_ms = seekPosition;

	seek_needed = -1;
	endCutter.Flush(decode_pos_ms);
	decoder->Flush(&file);
	done = 0;
	g_samplebuf_used = 0;

	int r = g_br;
	if (g_br_div) r = (g_br_add + g_br_div / 2) / g_br_div;
	file.Seek(decode_pos_ms, r);
	//      need_prebuffer=config_http_prebuffer/8;
	//			g_br_add=g_br_div=0;

}

DWORD DecodeLoop::OpenDecoder()
{
	mod.UsesOutputPlug &= ~8;
	if (isAac)
	{
		if (isEAAC)
		{
				waServiceFactory *factory = mod.service->service_getServiceByGuid(adts_aac_guid);
				if (factory)
					decoder = (adts *)factory->getInterface();

				mod.UsesOutputPlug|=8;
		}
		if (!decoder)
		{
			decoder = new ADTS_VLB;
			mod.UsesOutputPlug &= ~8;
		}
	}
	else
	{
		waServiceFactory *factory = mod.service->service_getServiceByGuid(adts_mp2_guid);
		if (factory)
			decoder = (adts *)factory->getInterface();

		mod.UsesOutputPlug|=8;
	}

	if (decoder) {
		decoder->SetDecoderHooks(mp3GiveVisData, mp2Equalize, mp3Equalize);
	}

	if (decoder 
		&& decoder->Initialize(AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"mono", false), 
													 config_downmix == 2, 
													 AGAVE_API_CONFIG->GetBool(playbackConfigGroupGUID, L"surround", true),
													 (int)AGAVE_API_CONFIG->GetUnsigned(playbackConfigGroupGUID, L"bits", 16), true, false,
													 (config_miscopts&1)/*crc*/) == adts::SUCCESS
		&& decoder->Open(&file))
	{
		// sync to stream
		while (1)
		{
			switch (decoder->Sync(&file, g_samplebuf, sizeof(g_samplebuf), &g_samplebuf_used, &br))
			{
			case adts::SUCCESS:
				return 1;
			case adts::FAILURE:
			case adts::ENDOFFILE:
				if (!killDecodeThread)
				{
					if (!lastfn_status_err)
					{
						EnterCriticalSection(&g_lfnscs);
						WASABI_API_LNGSTRING_BUF(IDS_ERROR_SYNCING_TO_STREAM,lastfn_status,256);
						LeaveCriticalSection(&g_lfnscs);
						PostMessage(mod.hMainWindow, WM_USER, 0, IPC_UPDTITLE);
					}
				}
				if (!killDecodeThread) Sleep(200);
				if (!killDecodeThread) 			PostMessage(mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
				return 0;
			case adts::NEEDMOREDATA:
				if (!killDecodeThread && file.IsStream()) Sleep(25);
				if (killDecodeThread) return 0;
			}
		}
	}

	return 0;
}

DWORD DecodeLoop::Loop()
{
	last_decode_pos_ms = 0;

	if (file.Open(lastfn, config_max_bufsize_k) != NErr_Success)
	{
		if (!killDecodeThread) Sleep(200);
		if (!killDecodeThread) PostMessage(mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
		return 0;
	}

	if (file.IsSeekable()) mod.is_seekable = 1;

	wchar_t *ext = PathFindExtension(lastfn);
	if (!_wcsicmp(ext, L".aac")
	    || !_wcsicmp(ext, L".vlb")
	    || !_wcsicmp(ext, L".apl"))
	{
		if (file.IsStream())
			SetupStream();
		else
		{
			isAac = 1;
			if (!_wcsicmp(ext, L".aac") || !_wcsicmp(ext, L".apl")) isEAAC = 1;
		}
	}
	else if (file.IsStream())
		SetupStream();

	if (OpenDecoder() == 0)
		return 0;

	EnterCriticalSection(&streamInfoLock);
	g_playing_file = &file;
	if (file.uvox_3901)
	{
		PostMessage(mod.hMainWindow, WM_WA_IPC, (WPARAM) "0x3901", IPC_METADATA_CHANGED);
			PostMessage(mod.hMainWindow, WM_WA_IPC, 0, IPC_UPDTITLE);
	}
	LeaveCriticalSection(&streamInfoLock);
	

	EnterCriticalSection(&g_lfnscs);
	lastfn_status[0] = 0;
	LeaveCriticalSection(&g_lfnscs);

	lastfn_data_ready = 1;

// TODO?	if (decoder != &aacp) // hack because aac+ bitrate isn't accurate at this point
		br = decoder->GetCurrentBitrate();

	need_prebuffer = CalcPreBuffer(config_http_prebuffer, (int)br);

	if (((!(config_eqmode&4) && decoder->GetLayer() == 3) ||
	                    ((config_eqmode&8) && decoder->GetLayer() < 3)))
	{
		mod.UsesOutputPlug |= 2;
	}
	else
		mod.UsesOutputPlug &= ~2;

	decoder->CalculateFrameSize(&g_framesize);
	decoder->GetOutputParameters(&g_bits, &g_nch, &g_srate);

	if (!killDecodeThread && file.IsStream() == 1)
	{
		DWORD_PTR dw;
		if (!killDecodeThread) SendMessageTimeout(mod.hMainWindow, WM_USER, 0, IPC_UPDTITLE, SMTO_BLOCK, 100, &dw);
		if (!killDecodeThread) SendMessageTimeout(mod.hMainWindow, WM_TIMER, 38, 0, SMTO_BLOCK, 100, &dw);
	}

	sampleFrameSize = g_nch * ((int)g_bits/8);

	if (config_gapless)
		CalculateCodecDelay();

	if (OpenOutput(g_nch, g_srate, (int)g_bits) == 0)
		return 0;

	/* ----- send info to winamp and vis: bitrate, etc ----- */
	g_br = (int)decoder->GetCurrentBitrate();

	g_br_add = g_br;
	g_br_div = 1;
	g_avg_vbr_br = file.GetAvgVBRBitrate();
	mod.SetInfo(g_br, g_srate / 1000, g_nch, 0);

	// initialize visualization stuff
	mod.SAVSAInit((maxlatency << g_ds), g_srate);
	mod.VSASetInfo(g_srate, g_nch);
	/* ----- end send info to winamp and vis ----- */

	if (file.IsSeekable() && g_br)
	{
		mod.is_seekable = 1;
		if (!file.m_vbr_frames) g_length = MulDiv(file.GetContentLength(), 8, g_br);
		else g_length = file.m_vbr_ms;
	}

	if (file.IsStream())
	{
		if (need_prebuffer < config_http_prebuffer / 2)
			need_prebuffer = config_http_prebuffer / 2;
	}

	while (!killDecodeThread)
	{
		if (seek_needed != -1)
			Seek(seek_needed);

		if (need_prebuffer && file.IsStream() && maxlatency && !file.EndOf())
			PreBuffer();

		int needsleep = 1;

		if (done == 2) // done was set to TRUE during decoding, signaling eof
		{
			mod.outMod->CanWrite();		// some output drivers need CanWrite
			// to be called on a regular basis.

			if (!mod.outMod->IsPlaying())
			{
				// we're done playing, so tell Winamp and quit the thread.
				if (!killDecodeThread) PostMessage(mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
				done=3;
				break;
			}
		}
		else
		{
			int fs = (g_framesize * ((mod.dsp_isactive() == 1) ? 2 : 1));
			// TODO: we should really support partial writes, there's no gaurantee that CanWrite() will EVER get big enough
			if (mod.outMod->CanWrite() >= fs && (!need_prebuffer || !file.IsStream() || !maxlatency))
				// CanWrite() returns the number of bytes you can write, so we check that
				// to the block size. the reason we multiply the block size by two if
				// mod.dsp_isactive() is that DSP plug-ins can change it by up to a
				// factor of two (for tempo adjustment).
			{
				int p = mod.SAGetMode();
				g_vis_enabled = ((p & 1) || p == 4);
				if (!g_vis_enabled)
				{
					int s, a;
					mod.VSAGetMode(&s, &a);
					if (s) g_vis_enabled = 1;
				}

				Decode();

				if ((g_samplebuf_used >= (size_t)g_framesize || (done && g_samplebuf_used > 0)) && seek_needed == -1)
				{
					// adjust decode position variable
					if (file.isSeekReset())
						last_decode_pos_ms = decode_pos_ms = 0;
					else
						decode_pos_ms += ((int)g_samplebuf_used / sampleFrameSize  * 1000) / g_srate;

					// if we have a DSP plug-in, then call it on our samples
					if (mod.dsp_isactive())
					{
						g_samplebuf_used = mod.dsp_dosamples((short *)g_samplebuf, (int)g_samplebuf_used / sampleFrameSize, (int)g_bits, g_nch, g_srate) * sampleFrameSize;
					}
					Viz();
					endCutter.Write((char *)g_samplebuf, (int)g_samplebuf_used);
					g_samplebuf_used = 0;
					needsleep = 0;
					//memcpy(g_samplebuf,g_samplebuf+r,g_samplebuf_used);
				}
				if (done)
				{
					endCutter.Write(0, 0);
					done = 2;
				}
			}
		}
		if (decode_pos_ms > last_decode_pos_ms + 1000)
		{
			last_decode_pos_ms = decode_pos_ms;
		}

		if (needsleep) Sleep(10);
		// if we can't write data, wait a little bit. Otherwise, continue
		// through the loop writing more data (without sleeping)
	}

	/* ---- change some globals to let everyone know we're done */
	EnterCriticalSection(&g_lfnscs);
	lastfn_status[0] = 0;
	LeaveCriticalSection(&g_lfnscs);
	g_bufferstat = 0;
	g_closeaudio = 1;
	/* ----  */

	return 0;
}

DWORD WINAPI DecodeThread(LPVOID b)
{
	DecodeLoop loop;

	

	DWORD ret = loop.Loop();

	EnterCriticalSection(&streamInfoLock);
	g_playing_file = 0;
	LeaveCriticalSection(&streamInfoLock);
	return ret;
}

