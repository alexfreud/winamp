#include "main.h"
#include "LAMEInfo.h"
#include "AACFrame.h"
#include "config.h"
#include "../winamp/wa_ipc.h"
#include <Richedit.h>
#include "api__in_mp3.h"
#include "FactoryHelper.h"
#include <shlwapi.h>
#include <strsafe.h>
#include <foundation/error.h>

int fixAACCBRbitrate(int br);

#if 0
/*

*/
/*
int MP3Info::remove_id3v1()
{
char temp[3] = {0, 0, 0};
DWORD x;
int err = 0;
HANDLE hFile = CreateFile(file, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0,
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
if (hFile == INVALID_HANDLE_VALUE)
{
	return 0; //1;
}
SetFilePointer(hFile, -128, NULL, FILE_END);
ReadFile(hFile, temp, 3, &x, NULL);
if (!memcmp(temp, "TAG", 3))
{
	SetFilePointer(hFile, -128, NULL, FILE_END);
	if (!SetEndOfFile(hFile))
	{
		err = 1;
	}
}
CloseHandle(hFile);
if (!err) fbuf[0] = 0;
return err;

return 0;
}
*/




/*
da_tag.SetUnsync(false);
da_tag.SetExtendedHeader(true);
da_tag.SetCompression(false);
da_tag.SetPadding(true);
*/


#endif


int FindAverageAACBitrate(unsigned __int8 *data, int sizeBytes)
{
	AACFrame aacFrame;
	aacFrame.ReadBuffer(data);

	if (aacFrame.OK())
	{
		int aac_frame_length = aacFrame.frameLength;
		int no_rawdb = aacFrame.numDataBlocks;

		int fc_tot = aac_frame_length;
		int fc_cnt = no_rawdb + 1;

		unsigned char *aa = data + aac_frame_length;
		int tt = sizeBytes - aac_frame_length;
		while (tt >= 8)
		{
			AACFrame nextFrame;
			nextFrame.ReadBuffer(aa);
			if (!nextFrame.OK()) break; // error
			int fcaac_frame_length = nextFrame.frameLength;
			int fcno_rawdb = nextFrame.numDataBlocks;

			fc_cnt += fcno_rawdb + 1;
			fc_tot += fcaac_frame_length;

			aa += fcaac_frame_length;
			tt -= fcaac_frame_length;
		}


		int avg_framesize = fc_tot / (fc_cnt ? fc_cnt : 1);
		return fixAACCBRbitrate(MulDiv(avg_framesize * 8, aacFrame.GetSampleRate(), 1024 * 1000));
	}
	return 0;
}

static bool ScanForFrame(CGioFile *file, int *bytesRead)
{
	unsigned char buffer[512] = {0}; /* don't want to read too much, since most MP3's start at 0 */
	int buflen = 0;

	int checked=0;
	if (file->Peek(buffer, sizeof(buffer), &buflen) != NErr_Success)
		return false;
	unsigned char *b = buffer;
	while (buflen >= 4)
	{
		MPEGFrame frame1;
		frame1.ReadBuffer(b);
		if (frame1.IsSync())
		{
			if (checked)
				file->Read(buffer, checked, &buflen);
			*bytesRead=checked;
			return true;
		}

		checked++;
		buflen--;
		b++;
	}
	if (checked)
		file->Read(buffer, checked, &buflen);
	*bytesRead=checked;
	return false;
}

static bool mp3sync(CGioFile *file)
{
	unsigned char buffer[1448 + 4] = {0}; /* large enough for one max-size frame and the header of the second */
	int buflen = 0;

	static const unsigned long gdwHeaderSyncMask = 0xfffe0c00L;
	unsigned long ulHdr1=0;
	unsigned long ulHdr2=0;

	int bytesChecked=0;
	while (bytesChecked<32768)
	{
		int bytesRead=0;
		if (ScanForFrame(file, &bytesRead))
		{
			if (file->Peek(buffer, sizeof(buffer), &buflen) != NErr_Success)
				return false;

			if (buflen >= 4)
			{
				MPEGFrame frame1;
				frame1.ReadBuffer(buffer);
				if (frame1.IsSync())
				{
					ulHdr1 = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
					int framelength= frame1.FrameSize();
					if (buflen >= (framelength+4))
					{
						unsigned char *b = buffer + framelength;
						buflen -= frame1.FrameSize();
						MPEGFrame frame2;
						frame2.ReadBuffer(b);
						ulHdr2 = (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
						if (!((ulHdr1 ^ ulHdr2) & gdwHeaderSyncMask) && frame2.IsSync())
							return true;
						else
						{

						}
					}
				}
				file->Read(buffer, 1, &buflen);
				bytesChecked++;
			}
		}
		else if (file->EndOf())
				return 0;

		bytesChecked+=bytesRead;
	}
	return false;
}

static const wchar_t *GetMPEGVersionString(int mpegVersion)
{
	switch (mpegVersion)
	{
	case MPEGFrame::MPEG1:
		return L"MPEG-1";
	case MPEGFrame::MPEG2:
		return L"MPEG-2";
	case MPEGFrame::MPEG2_5:
		return L"MPEG-2.5";
	default:
		static wchar_t temp[64];
		return WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,temp,64);
	}
}

static const wchar_t *GetEmphasisString(int emphasis)
{
	static wchar_t tempE[32];
	switch (emphasis)
	{
		case 0:
			return WASABI_API_LNGSTRINGW_BUF(IDS_NONE,tempE,32);
		case 1:
			return WASABI_API_LNGSTRINGW_BUF(IDS_50_15_MICROSEC,tempE,32);
		case 2:
			return WASABI_API_LNGSTRINGW_BUF(IDS_INVALID,tempE,32);
		case 3:
			return L"CITT j.17";
		default:
			return WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,tempE,32);
	}
}

static const wchar_t *GetChannelModeString(int channelMode)
{
	static wchar_t tempM[32];
	switch (channelMode)
	{
		case 0:
			return WASABI_API_LNGSTRINGW_BUF(IDS_STEREO,tempM,32);
		case 1:
			return WASABI_API_LNGSTRINGW_BUF(IDS_JOINT_STEREO,tempM,32);
		case 2:
			return WASABI_API_LNGSTRINGW_BUF(IDS_2_CHANNEL,tempM,32);
		case 3:
			return WASABI_API_LNGSTRINGW_BUF(IDS_MONO,tempM,32);
		default:
			return WASABI_API_LNGSTRINGW_BUF(IDS_ERROR,tempM,32);
	}
}
#define INFO_READ_SIZE 32768
void GetFileDescription(const wchar_t *file, CGioFile &_file, wchar_t *data, size_t datalen)
{
	int hdroffs = 0;
	size_t size = datalen;
	wchar_t *mt = data;
	wchar_t *ext = PathFindExtension(file);

	wchar_t langbuf[256] = {0};
	int flen = _file.GetContentLength();
	StringCchPrintfExW(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW_BUF(IDS_PAYLOAD_SIZE, langbuf, 256), _file.GetContentLength());
	if (!_wcsicmp(ext, L".aac") || !_wcsicmp(ext, L".vlb"))
	{
		#if 0 // TODO!
		if (_wcsicmp(ext, L".vlb")) // aacplus can't do VLB
		{
			if (aacPlus)
			{
				aacPlus->EasyOpen(AACPLUSDEC_OUTPUTFORMAT_INT16_HOSTENDIAN, 6);
				AACPLUSDEC_EXPERTSETTINGS *pConf = aacPlus->GetDecoderSettingsHandle();
				pConf->bEnableOutputLimiter = 1;
				pConf->bDoUpsampling = 1;
				aacPlus->SetDecoderSettings();

				StringCchCatEx(mt, size, WASABI_API_LNGSTRINGW_BUF(IDS_FORMAT_AAC, langbuf, 256), &mt, &size, 0);

				char buffer[INFO_READ_SIZE] = {0};
				int inputRead;
				_file.Read(buffer, INFO_READ_SIZE, &inputRead);
				AACPLUSDEC_BITSTREAMBUFFERINFO bitbufInfo = { inputRead, 0, 0};
				aacPlus->StreamFeed((unsigned char *)buffer, &bitbufInfo);

				unsigned char tempBuf[65536] = {0}; // grr, can't we find a better way to do this?
				AACPLUSDEC_AUDIOBUFFERINFO audioBufInfo = {65536, 0, 0};
				aacPlus->StreamDecode(tempBuf, &audioBufInfo, 0, 0);
				audioBufInfo.nBytesBufferSizeIn -= audioBufInfo.nBytesWrittenOut;
				aacPlus->StreamDecode(tempBuf + audioBufInfo.nBytesWrittenOut, &audioBufInfo, 0, 0);
				AACPLUSDEC_STREAMPROPERTIES *streamProperties = aacPlus->GetStreamPropertiesHandle();
				if (streamProperties->nDecodingState == AACPLUSDEC_DECODINGSTATE_STREAMVERIFIED)
				{
					AACPLUSDEC_PROGRAMPROPERTIES *currentProgram = &(streamProperties->programProperties[streamProperties->nCurrentProgram]);

					switch (currentProgram->nStreamType)
					{
					case AACPLUSDEC_MPEG2_PROFILE_AACMAIN:
						StringCchCatEx(mt, size, L"\r\nMPEG-2 AAC", &mt, &size, 0);
						break;
					case AACPLUSDEC_MPEG2_PROFILE_AACLC:
						if (currentProgram->bProgramSbrEnabled)
							StringCchCatEx(mt, size, WASABI_API_LNGSTRINGW_BUF(IDS_MPEG2_HE_AAC_IS, langbuf, 256), &mt, &size, 0);
						else
							StringCchCatEx(mt, size, L"\r\nMPEG-2 AAC LC", &mt, &size, 0);
						break;
					case AACPLUSDEC_MPEG4_AOT_AACMAIN:
						StringCchCatEx(mt, size, L"\r\nMPEG-4 AAC", &mt, &size, 0);
						break;
					case AACPLUSDEC_MPEG4_AOT_AACLC:
						if (currentProgram->bProgramSbrEnabled)
							StringCchCatEx(mt, size, WASABI_API_LNGSTRINGW_BUF(IDS_MPEG4_HE_AAC_IS, langbuf, 256), &mt, &size, 0);
						else
							StringCchCatEx(mt, size, L"\r\nMPEG-4 AAC LC", &mt, &size, 0);
						break;
					case AACPLUSDEC_MPEG4_AOT_SBR:
						StringCchCatEx(mt, size, L"\r\nMPEG-4 HE-AAC", &mt, &size, 0);
						break;
					}

					if (currentProgram->nAacSamplingRate != currentProgram->nOutputSamplingRate)
						StringCchPrintfEx(mt, size, &mt, &size, 0,
						                  WASABI_API_LNGSTRINGW(IDS_SAMPLE_RATE_OUTPUT),
						                  currentProgram->nAacSamplingRate, currentProgram->nOutputSamplingRate);
					else
						StringCchPrintfEx(mt, size, &mt, &size, 0,
						                  WASABI_API_LNGSTRINGW(IDS_SAMPLE_RATE),
						                  currentProgram->nAacSamplingRate);
					int srate = currentProgram->nOutputSamplingRate;

					if (currentProgram->bProgramSbrEnabled)
						StringCchCatEx(mt, size, WASABI_API_LNGSTRINGW(IDS_SBR_PRESENT), &mt, &size, 0);
					else
						StringCchCatEx(mt, size, WASABI_API_LNGSTRINGW(IDS_SBR_NOT_PRESENT), &mt, &size, 0);

					if (currentProgram->nAacChannels != currentProgram->nOutputChannels)
						StringCchPrintfEx(mt, size, &mt, &size, 0,
						                  WASABI_API_LNGSTRINGW(IDS_CHANNELS_OUTPUT),
						                  currentProgram->nAacChannels, currentProgram->nOutputChannels);
					else
						StringCchPrintfEx(mt, size, &mt, &size, 0,
						                  WASABI_API_LNGSTRINGW(IDS_CHANNELS),
						                  currentProgram->nAacChannels);
					switch (currentProgram->nChannelMode)
					{
					case AACPLUSDEC_CHANNELMODE_MONO:
						StringCchCatEx(mt, size, WASABI_API_LNGSTRINGW(IDS_MODE_MONO), &mt, &size, 0);
						break;
					case AACPLUSDEC_CHANNELMODE_STEREO:
						StringCchCatEx(mt, size, WASABI_API_LNGSTRINGW(IDS_MODE_STEREO), &mt, &size, 0);
						break;
					case AACPLUSDEC_CHANNELMODE_PARAMETRIC_STEREO:
						StringCchCatEx(mt, size, WASABI_API_LNGSTRINGW(IDS_MODE_PARAMETRIC_STEREO), &mt, &size, 0);
						break;
					case AACPLUSDEC_CHANNELMODE_DUAL_CHANNEL:
						StringCchCatEx(mt, size, WASABI_API_LNGSTRINGW(IDS_MODE_DUAL_CHANNEL), &mt, &size, 0);
						break;
					case AACPLUSDEC_CHANNELMODE_4_CHANNEL_2CPE:
						StringCchCatEx(mt, size, WASABI_API_LNGSTRINGW(IDS_MODE_4_CHANNEL_2_CPE), &mt, &size, 0);
						break;
					case AACPLUSDEC_CHANNELMODE_4_CHANNEL_MPEG:
						StringCchCatEx(mt, size, WASABI_API_LNGSTRINGW(IDS_MODE_4_CHANNEL_MPEG), &mt, &size, 0);
						break;
					case AACPLUSDEC_CHANNELMODE_5_CHANNEL:
						StringCchCatEx(mt, size, WASABI_API_LNGSTRINGW(IDS_MODE_5_CHANNEL), &mt, &size, 0);
						break;
					case AACPLUSDEC_CHANNELMODE_5_1_CHANNEL:
						StringCchCatEx(mt, size, WASABI_API_LNGSTRINGW(IDS_MODE_5_1), &mt, &size, 0);
						break;
					case AACPLUSDEC_CHANNELMODE_6_1_CHANNEL:
						StringCchCatEx(mt, size, WASABI_API_LNGSTRINGW(IDS_MODE_6_1), &mt, &size, 0);
						break;
					case AACPLUSDEC_CHANNELMODE_7_1_CHANNEL:
						StringCchCatEx(mt, size, WASABI_API_LNGSTRINGW(IDS_MODE_7_1), &mt, &size, 0);
						break;
					}

					if (streamProperties->nBitrate)
					{
						StringCchPrintfEx(mt, size, &mt, &size, 0,
						                  WASABI_API_LNGSTRINGW(IDS_BITRATE),
						                  streamProperties->nBitrate);
					}
					else
					{
						int avg_bitrate = FindAverageAACBitrate((unsigned char *)buffer, inputRead);
						StringCchPrintfEx(mt, size, &mt, &size, 0,
						                  WASABI_API_LNGSTRINGW(IDS_AVERAGE_BITRATE),
						                  avg_bitrate);
					}
				}
			}
		}

		if (!aacPlus)
			#endif
		{
			char buffer[INFO_READ_SIZE] = {0};
			int inputRead = 0;
			_file.Read(buffer, INFO_READ_SIZE, &inputRead);

			StringCchCopyEx(mt, size, WASABI_API_LNGSTRINGW(IDS_FORMAT_AAC), &mt, &size, 0);
			unsigned char *a = (unsigned char *)buffer;
			while (inputRead-- >= 8)
			{
				AACFrame aacFrame;
				aacFrame.ReadBuffer(a);

				if (aacFrame.OK())
				{
					int aac_frame_length = aacFrame.frameLength;
					int no_rawdb = aacFrame.numDataBlocks;

					/*size_t size = 1024;
					char *mt = mpeg_description;*/
					StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW(IDS_HEADER_FOUND_AT_X_BYTES), hdroffs);
					StringCchPrintfEx(mt, size, &mt, &size, 0, L"\r\nMPEG-%d AAC", aacFrame.GetMPEGVersion());

					int fc_tot = aac_frame_length;
					int fc_cnt = no_rawdb + 1;

					unsigned char *aa = a + aac_frame_length;
					int tt = inputRead - aac_frame_length;
					while (tt >= 8)
					{
						AACFrame nextFrame;
						nextFrame.ReadBuffer(aa);
						if (!nextFrame.OK()) break; // error
						int fcaac_frame_length = nextFrame.frameLength;
						int fcno_rawdb = nextFrame.numDataBlocks;

						fc_cnt += fcno_rawdb + 1;
						fc_tot += fcaac_frame_length;

						aa += fcaac_frame_length;
						tt -= fcaac_frame_length;
					}

					{
						int avg_framesize = fc_tot / (fc_cnt ? fc_cnt : 1);
						int srate = aacFrame.GetSampleRate();
						int avg_bitrate = fixAACCBRbitrate(MulDiv(avg_framesize * 8, srate, 1024 * 1000));

						int len_s = MulDiv(flen, 1024, avg_framesize * srate);

						StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW(IDS_LENGTH_X_SECONDS), len_s);
						StringCchPrintfEx(mt, size, &mt, &size, 0, L"\r\nCBR %d kbps", avg_bitrate);
					}

					StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW(IDS_PROFILE), aacFrame.GetProfileName());
					StringCchPrintfEx(mt, size, &mt, &size, 0, L"\r\n%dHz %s", aacFrame.GetSampleRate(), aacFrame.GetChannelConfigurationName());
					StringCchPrintfEx(mt, size, &mt, &size, 0, L"\r\nCRC: %s", WASABI_API_LNGSTRINGW((aacFrame.protection == 0 ? IDS_YES : IDS_NO)));
					break;
				}

				a++;
				hdroffs++;
			}
		}
	}
	else
	{
		unsigned char mp3syncbuf[INFO_READ_SIZE] = {0};
		int inputRead = 0;

		DWORD start = _file.GetCurrentPosition();
		// find position of first sync
		if (!mp3sync(&_file))
			return;
		int syncposition = _file.GetCurrentPosition()-start;

		// advance to first sync
		_file.Peek(mp3syncbuf, INFO_READ_SIZE, &inputRead);

		unsigned int padding = 0;
		unsigned int encoderDelay = 0;

		MPEGFrame frame;
		frame.ReadBuffer(mp3syncbuf);


		int framelen = frame.FrameSize();

		//const CMp3StreamInfo *info = decoder.GetStreamInfo();
		//const CMpegHeader *header = decoder.m_Mbs.GetHdr();

		StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW_BUF(IDS_HEADER_FOUND_AT_X_BYTES, langbuf, 256), _file.GetHeaderOffset() + syncposition);
		if (!padding) padding = _file.postpad;
		if (!encoderDelay) encoderDelay = _file.prepad;

		if (padding || encoderDelay)
			StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW_BUF(IDS_ENC_DELAY_ZERO_PADDING, langbuf, 256), encoderDelay, padding);

		int is_vbr_lens = _file.m_vbr_ms;
		int iLen = (is_vbr_lens ? is_vbr_lens/1000 : ((flen * 8) / frame.GetBitrate()));
		if(iLen > 0)
			StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW_BUF(IDS_LENGTH_X_SECONDS, langbuf, 256), iLen);
		else
		{
			float fLen = (is_vbr_lens ? is_vbr_lens/1000.0f : ((flen * 8.0f) / frame.GetBitrate()));
			StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW_BUF(IDS_LENGTH_X_PART_SECONDS, langbuf, 256), fLen);
		}

		StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW_BUF(IDS_S_LAYER_X, langbuf, 256), GetMPEGVersionString(frame.mpegVersion), frame.GetLayer());

		int frames = _file.m_vbr_frames;

		int is_vbr = _file.m_vbr_flag || _file.m_vbr_hdr;
		if (!is_vbr || _file.encodingMethod == ENCODING_METHOD_CBR)
		{
			if (frames)
			{
				StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW_BUF(IDS_X_KBIT, langbuf, 256), frame.GetBitrate() / 1000, frames);
			}
			else
			{
				StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW_BUF(IDS_X_KBIT_APPROX, langbuf, 256), frame.GetBitrate() / 1000, MulDiv(flen, 8, framelen));
			}
		}
		else if (is_vbr && _file.encodingMethod == ENCODING_METHOD_ABR)
			StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW_BUF(IDS_X_KBIT_ABR, langbuf, 256), _file.GetAvgVBRBitrate(), frames);
		else
		{
			StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW_BUF(IDS_X_KBIT_VBR, langbuf, 256), _file.GetAvgVBRBitrate(), _file.m_vbr_hdr?L"I":L"", frames);
		}

		StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW_BUF(IDS_X_HZ_S, langbuf, 256), frame.GetSampleRate(), GetChannelModeString(frame.channelMode));
		StringCchPrintfEx(mt, size, &mt, &size, 0, L"\r\nCRC: %s", WASABI_API_LNGSTRINGW_BUF((frame.CRC ? IDS_YES : IDS_NO), langbuf, 256));
		wchar_t tmp[16] = {0};
		StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW_BUF(IDS_COPYRIGHTED, langbuf, 256), WASABI_API_LNGSTRINGW_BUF((frame.copyright ? IDS_YES : IDS_NO),tmp,16));
		StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW_BUF(IDS_ORIGINAL, langbuf, 256), WASABI_API_LNGSTRINGW_BUF((frame.original ? IDS_YES : IDS_NO),tmp,16));
		StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW_BUF(IDS_EMPHASIS, langbuf, 256), GetEmphasisString(frame.emphasis));

		if (_file.m_vbr_frame_len && !_file.lengthVerified)
			StringCchPrintfEx(mt, size, &mt, &size, 0, WASABI_API_LNGSTRINGW_BUF(IDS_MP3_HAS_BEEN_MODIFIED_NOT_ALL_MAY_BE_CORRECT, langbuf, 256));
	}
}

void GetAudioInfo(const wchar_t *filename, CGioFile *file, int *len, int *channels, int *bitrate, int *vbr, int *sr)
{
	*bitrate=0;
	*len=0;
	*vbr=0;
	*channels=0;
	if (file)
	{
		wchar_t *ext = PathFindExtension(filename);
		if (!_wcsicmp(ext, L".aac") || !_wcsicmp(ext, L".vlb"))
		{
			unsigned char t[INFO_READ_SIZE*2] = {0};
			unsigned char *a = t;
			int n = 0;

			if (file->Read(t, sizeof(t), &n) != NErr_Success)
				return;

			while (n-- >= 8)
			{
				AACFrame aacFrame;
				aacFrame.ReadBuffer(a);

				if (aacFrame.OK())
				{
					int aac_frame_length = aacFrame.frameLength;

					int fc_tot = aac_frame_length;
					int fc_cnt = aacFrame.numDataBlocks + 1;

					unsigned char *aa = a + aac_frame_length;
					int tt = n - aac_frame_length;
					while (tt >= 8 && aac_frame_length)
					{
						AACFrame nextFrame;
						nextFrame.ReadBuffer(aa);
						if (!nextFrame.OK()) break; // error
						int fcaac_frame_length = nextFrame.frameLength;
						int fcno_rawdb = nextFrame.numDataBlocks;

						fc_cnt += fcno_rawdb + 1;
						fc_tot += fcaac_frame_length;

						aa += fcaac_frame_length;
						tt -= fcaac_frame_length;
					}

					int avg_framesize = fc_tot / (fc_cnt ? fc_cnt : 1);

					int br = MulDiv(avg_framesize * 8, aacFrame.GetSampleRate(), 1024);
					*len = MulDiv(file->GetContentLength(), 1024*8, br);
					*bitrate = fixAACCBRbitrate(br/1000)*1000;
					
					*sr = aacFrame.GetSampleRate();
					*channels = aacFrame.GetNumChannels();
					
					break;
				}
				a++;
			}
		}
		else
		{
			if (*bitrate = file->GetAvgVBRBitrate()*1000)
			{
				*len = file->m_vbr_ms;
				*vbr = file->m_vbr_flag || file->m_vbr_hdr;
			}

			if (!mp3sync(file))
				return;

			unsigned char t[4] = {0};
			int n = 0;

			if (file->Peek(t, sizeof(t), &n) != NErr_Success)
				return;

			MPEGFrame frame;
			frame.ReadBuffer(t);
			if (frame.IsSync())
			{
				if (!*bitrate)
				{
					*bitrate = frame.GetBitrate();
					*len = MulDiv(file->GetContentLength(), 1000*8, *bitrate);
				}
				*channels = frame.GetNumChannels();
				*sr = frame.GetSampleRate();
			}
		}
	}
}