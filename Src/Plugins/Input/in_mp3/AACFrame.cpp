#include "AACFrame.h"
#include "api__in_mp3.h"
#include "resource.h"
#include "in2.h"
extern In_Module mod;

void AACFrame::ReadBuffer( unsigned __int8 *buffer )
{
	syncword             = ( buffer[ 0 ] << 4 ) | ( buffer[ 1 ] >> 4 );
	id                   = ( buffer[ 1 ] >> 3 ) & 1;
	layer                = ( buffer[ 1 ] >> 1 ) & 3;
	protection           = ( buffer[ 1 ] ) & 1;
	profile              = ( buffer[ 2 ] >> 6 ) & 3;
	sampleRateIndex      = ( buffer[ 2 ] >> 2 ) & 0xF;
	privateBit           = ( buffer[ 2 ] >> 1 ) & 1;
	channelConfiguration = ( ( buffer[ 2 ] & 1 ) << 2 ) | ( ( buffer[ 3 ] >> 6 ) & 3 );
	original             = ( buffer[ 3 ] >> 5 ) & 1;
	home                 = ( buffer[ 3 ] >> 4 ) & 1;

	//copyright_identification_bit = (buffer[3] >> 3) & 1;
	//copyright_identification_start = (buffer[3] >> 2) & 1;
	frameLength          = ( ( buffer[ 3 ] & 3 ) << 11 ) | ( buffer[ 4 ] << 3 ) | ( ( buffer[ 5 ] >> 5 ) & 7 );
	bufferFullness       = ( ( buffer[ 5 ] & 0xF8 ) << 5 ) | ( ( buffer[ 6 ] >> 2 ) & 0x3F );
	numDataBlocks        = buffer[ 6 ] & 3;
}

bool AACFrame::OK()
{
	if (syncword == SYNC
		&& layer == 0
		&& sampleRateIndex < 13
		//&& profile != LTP // TODO: can coding technologies decoder do LTP?
		)
		return true;
	else
		return false;
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

int AACFrame::GetSampleRate()
{
	return aac_sratetab[sampleRateIndex];
}

static const wchar_t *aac_profiletab[] = {L"Main", L"LC", L"SSR", L"LTP"};

const wchar_t *AACFrame::GetProfileName()
{
	return aac_profiletab[profile];
}

//static const char *aac_channels[] = {"Custom", "Mono", "Stereo", "3 channel", "4 channel", "surround", "5.1", "7.1"};
static wchar_t aac_channels_str[64];
static int aac_channels_id[] = {IDS_CUSTOM, IDS_MONO, IDS_STEREO, IDS_3_CHANNEL, IDS_4_CHANNEL, IDS_SURROUND, IDS_5_1, IDS_7_1};
const wchar_t *AACFrame::GetChannelConfigurationName()
{
	return WASABI_API_LNGSTRINGW_BUF(aac_channels_id[channelConfiguration],aac_channels_str,64);
}

int AACFrame::GetNumChannels()
{
	switch(channelConfiguration)
	{
	case 7:
		return 8;
	default:
		return channelConfiguration;
	}
}

int AACFrame::GetMPEGVersion()
{
	if (id == 0)
		return 2;
	else
		return 4;
}