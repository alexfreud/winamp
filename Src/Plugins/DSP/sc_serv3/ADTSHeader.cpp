#include "ADTSHeader.h"
#include "nmrCommon/services/stdServiceImpl.h"


enum
{
	ADTS_NOT_PROTECTED = 1,
	ADTS_PROTECTED = 0,
	ADTS_SYNC = 0xFFF,
	ADTS_MAIN = 0x00,
	ADTS_LC = 0x01,
	ADTS_SSR = 0x10,
	ADTS_LTP = 0x11
};

const int aac_adts_parse(const aac_adts_header_t header, const __uint8 *buffer)
{
	header->syncword = ((buffer[0] << 4) | (buffer[1] >> 4));

	if (header->syncword != ADTS_SYNC)
	{
		return NErr_LostSynchronization;
	}

	header->id = ((buffer[1] >> 3) & 1);
	header->layer = ((buffer[1] >> 1) & 3);
	if (header->layer != 0)
	{
		return NErr_WrongFormat;
	}

	header->protection = ((buffer[1]) & 1);
	header->profile = ((buffer[2] >> 6) & 3);
	header->sample_rate_index = ((buffer[2] >> 2) & 0xF);
	if (header->sample_rate_index == 15)
	{
		return NErr_UnsupportedFormat; // might actually be OK if we can separately signal the sample rate
	}

	if (header->sample_rate_index > 13)
	{
		return NErr_Reserved;
	}

	header->private_bit = ((buffer[2] >> 1) & 1);
	header->channel_configuration = ((buffer[2] & 1) << 2) | ((buffer[3] >> 6) & 3);
	header->original = ((buffer[3] >> 5) & 1);
	header->home = ((buffer[3] >> 4) & 1);

	header->frame_length = ((buffer[3] & 3) << 11) | (buffer[4]<<3) | ((buffer[5] >> 5) & 7);
	header->buffer_fullness = ((buffer[5] & 0x1F) << 6) | (buffer[6] >> 2);
	header->num_data_blocks = (buffer[6] & 3);
	return NErr_Success;
}

static const unsigned int aac_sratetab[] =
{
	96000,
	88200,
	64000,
	48000,
	44100,
	32000,
	24000,
	22050,
	16000,
	12000,
	11025,
	8000,
	7350,
};
#if 0
const int aac_adts_match(const aac_adts_header_t header1, const aac_adts_header_t header2)
{
	if (header1->id != header2->id)
	{
		return NErr_False;
	}

	if (header1->profile != header2->profile)
	{
		return NErr_False;
	}

	if (header1->sample_rate_index != header2->sample_rate_index)
	{
		return NErr_False;
	}

	if (header1->channel_configuration != header2->channel_configuration)
	{
		return NErr_False;
	}

	return NErr_True;
}
#endif
const int aac_adts_get_channel_count(const aac_adts_header_t header)
{
	switch (header->channel_configuration)
	{
		case 7:
		{
			return 8;
		}
		default:
		{
			return header->channel_configuration;
		}
	}
}

const __uint16 getADTSFrameInfo(const char *hdr, unsigned int *samplerate, __uint8 *asc_header)
{
	ADTSHeader header = {0};
	if (aac_adts_parse(&header, (const __uint8 *)hdr) == NErr_Success)
	{
		*samplerate = aac_sratetab[header.sample_rate_index];

		// we need this when generating flv frames
		// as it creates the AudioSpecificConfig
		// from the existing ADTS header details
		// (is like a mini-ADTS header to create)
		if (asc_header)
		{
			asc_header[0] |= (((header.profile + 1) & 31) << 3) + (header.sample_rate_index >> 1);
			asc_header[1] |= ((header.sample_rate_index & 0x1) << 7) + (header.channel_configuration << 3);
		}

		//*bitrate = (int)(((header.frame_length / 1/*frames*/) * (aac_sratetab[header.sample_rate_index] / 1024.0)) + 0.5) * 8;
		return (__uint16)header.frame_length;
	}
	return 0;
}

const char *AAC_FrameInfo::getVersionName() const
{
    if (m_version)
        return "v2";
    return "v4";
}

const char *AAC_FrameInfo::getAOT() const
{
    switch (m_aot)
    {
        case 2:     return "LC";
        case 5:     return "SBR";
        case 29:    return "PS";
        default:    return "unknown profile";
    }
}


int getAACFrameLength (const unsigned char *p, unsigned int len)
{
    if (len < 6)
        return -1;
    return ((p[3] & 0x3) << 11) + (p[4] << 3) + ((p[5] & 0xE0) >> 5);
}

int getAACFrameInfo (const unsigned char *p, unsigned int len, AAC_FrameInfo &info)
{
    if (len < 6)
        return -1;
    if ((((long)p[0])<<4 | (p[1]>>4)) != 0xfff)
        return -1;

    int layer = ((p[1] >> 1) & 3);
    if (layer != 0)
        return -1;
    int sample_rate_index = ((p[2] >> 2) & 0xF);

    if (sample_rate_index > 13)
        return -1;
    int samplerate = aac_sratetab [sample_rate_index];
    if (info.m_samplerate)
    {
        if (info.m_samplerate != samplerate)
            return -1;
    }
    else
        info.m_samplerate = samplerate;
    info.m_blocks = (p[6] & 0x3) + 1;
    info.m_pattern = (((unsigned long)(p[0])<<24) | (p[1]<<16) | (p[2]<<8) | p[0]) & info.m_mask;

    return getAACFrameLength (p, len);
}


int AAC_FrameInfo::verifyFrame (const unsigned char *buf, unsigned int len)
{
    if (len > 6)
    {
        unsigned long v = (unsigned long)(buf[0])<<24 | (buf[1]<<16) | (buf[2]<<8) | buf[0];

        if ((v & m_mask) == m_pattern)
        {
            m_blocks = (buf[6] & 0x3) + 1;
            m_version = (buf[1] >> 3) & 1; // 1 mpeg2, 0 mpeg4
            m_aot = ((buf[2] >> 6) & 3) + 1;
            return getAACFrameLength (buf, len);
        }
        // DLOG ("AAC failed sync, retry..");
        return -1;
    }
    return 0;
}


AAC_FrameInfo::AAC_FrameInfo (unsigned long value) : parserInfo (0xFFFEFDC0, value)
{
    m_description = "AAC";
    m_blocks = 0;
    m_aot = 0;
}

AAC_FrameInfo::AAC_FrameInfo(const unsigned char *p, unsigned int len) : parserInfo()
{
    m_mask = 0xFFFEFDC0;
    m_description = "AAC";
    m_blocks = 0;
    getAACFrameInfo (p, len, *this);
}

