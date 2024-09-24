#pragma once

#include "mp4FastAAClib.h"
#include <windows.h> // or MAX_PATH


#define ENCODER_TYPE_MPEG4 (mmioFOURCC('A','A','C','f'))
#define ENCODER_TYPE_ADTS (mmioFOURCC('A','D','T','S'))

enum
{
	AAC_MODE_VBR=0,
	AAC_MODE_CBR=1,

	/* these are the profile options when CBR is selected */
	AAC_PROFILE_AUTOMATIC=0,
	AAC_PROFILE_LC=1,
	AAC_PROFILE_HE=2,
	AAC_PROFILE_HE_V2=3,

	/* Surround options */
	AAC_SURROUND_BCC = 0, /* Binaural Cue Coding, stereo + surround layer, aka MPEG-Surround */
	AAC_SURROUND_DISCRETE = 1, /* Discrete surround (traditional AAC surround sound with separate SCE per channel pair) */

	/* defaults */
	AAC_DEFAULT_MODE = AAC_MODE_VBR,
	AAC_DEFAULT_PROFILE = AAC_PROFILE_AUTOMATIC,
	AAC_DEFAULT_BITRATE = 128,
	AAC_DEFAULT_PRESET = 4,
	AAC_DEFAULT_SURROUND = AAC_SURROUND_BCC,
};

struct AACConfiguration
{
	unsigned int mode; /* CBR or VBR */
	unsigned int profile; /* what flavor of AAC, e.g. LC, or automatic */
	unsigned int bitrate; /* bitrate for CBR */
	unsigned int preset; /* preset for VBR */
	unsigned int surround; /* 0 = discrete, 1 = MPEG Surround */
};

struct AACConfigurationFile
{
	AACConfiguration config;
	unsigned int channels;
	unsigned int sample_rate;
	unsigned int type; /* either ENCODER_TYPE_MPEG4 or ENCODER_TYPE_ADTS */
	unsigned int shoutcast; /* 0 by default, 1 if we're being invoked from dsp_sc */
	bool changing; /* used internally by preferences */
	char config_file[MAX_PATH];
};

AACConfigurationFile *AACConfig_Create(unsigned int type, const char *filename); /* de-allocate with free() */
void AACConfig_Load(AACConfigurationFile *cfg);
void AACConfig_Save(const AACConfigurationFile *cfg);
/* bitrates are in bits/sec (not kbps), divide by 1000 if you need to 
TODO: ASSUMES 44.1kHz Stereo.  We need to make the encoder API accept samplerate/channels input better!
*/
void AACConfig_GetBitrateRange(const AACConfiguration *cfg, int *low, int *high);
AUD_OBJ_TYP AACConfig_GetAOT(const AACConfiguration *cfg);
int AACConfig_GetBitrate(const AACConfiguration *cfg, unsigned int channels);
MPEG4ENC_BITRATE_MODE AACConfig_GetBitrateMode(const AACConfiguration *cfg);
MPEG4ENC_CH_MODE AACConfig_GetChannelMode(const AACConfiguration *cfg, unsigned int channels);
void AACConfig_GetToolString(const MPEG4ENC_SETUP *setup, char tool[], size_t cch);