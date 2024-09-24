#include "config.h"
#include <windows.h>
#include <strsafe.h>
#include <assert.h>
#include "mp4FastAAClib.h"
#include "preferences.h"

AACConfigurationFile *AACConfig_Create(unsigned int type, const char *filename)
{
	AACConfigurationFile *cfg = (AACConfigurationFile*)calloc(1, sizeof(AACConfigurationFile));
	if (cfg)
	{
		cfg->type = type;

		if (filename) 
			lstrcpynA(cfg->config_file, filename, MAX_PATH);
		else 
			cfg->config_file[0] = 0;

		AACConfig_Load(cfg);
	}
	return cfg;
}

void AACConfig_Load(AACConfigurationFile *cfg)
{
	if (cfg->type == ENCODER_TYPE_MPEG4)
	{
		cfg->config.mode     = GetPrivateProfileIntA("audio_fhgaac", "mode",     AAC_DEFAULT_MODE,     cfg->config_file);
		cfg->config.profile  = GetPrivateProfileIntA("audio_fhgaac", "profile",  AAC_DEFAULT_PROFILE,  cfg->config_file);
		cfg->config.bitrate  = GetPrivateProfileIntA("audio_fhgaac", "bitrate",  AAC_DEFAULT_BITRATE,  cfg->config_file);
		cfg->config.preset   = GetPrivateProfileIntA("audio_fhgaac", "preset",   AAC_DEFAULT_PRESET,   cfg->config_file);
		cfg->config.surround = GetPrivateProfileIntA("audio_fhgaac", "surround", AAC_DEFAULT_SURROUND, cfg->config_file);
		cfg->shoutcast = 0;
	}
	else
	{
		cfg->config.mode      = AAC_MODE_CBR;
		cfg->config.profile   = GetPrivateProfileIntA("audio_adtsaac", "profile",  AAC_PROFILE_HE,  cfg->config_file);
		cfg->config.bitrate   = GetPrivateProfileIntA("audio_adtsaac", "bitrate",  64,  cfg->config_file);
		cfg->config.surround  = GetPrivateProfileIntA("audio_adtsaac", "surround", AAC_DEFAULT_SURROUND, cfg->config_file);
		cfg->shoutcast = GetPrivateProfileIntA("audio_adtsaac", "shoutcast", 0, cfg->config_file);
	}
}

void AACConfig_Save(const AACConfigurationFile *cfg)
{
	char temp[128] = {0};

	if (cfg->type == ENCODER_TYPE_MPEG4)
	{
		StringCbPrintfA(temp, sizeof(temp), "%u", cfg->config.mode);
		WritePrivateProfileStringA("audio_fhgaac", "mode", temp, cfg->config_file);

		StringCbPrintfA(temp, sizeof(temp), "%u", cfg->config.profile);
		WritePrivateProfileStringA("audio_fhgaac", "profile", temp, cfg->config_file);

		StringCbPrintfA(temp, sizeof(temp), "%u", cfg->config.bitrate);
		WritePrivateProfileStringA("audio_fhgaac", "bitrate", temp, cfg->config_file);

		StringCbPrintfA(temp, sizeof(temp), "%u", cfg->config.preset);
		WritePrivateProfileStringA("audio_fhgaac", "preset", temp, cfg->config_file);

		StringCbPrintfA(temp, sizeof(temp), "%u", cfg->config.surround);
		WritePrivateProfileStringA("audio_fhgaac", "surround", temp, cfg->config_file);
	}
	else
	{
		StringCbPrintfA(temp, sizeof(temp), "%u", cfg->config.profile);
		WritePrivateProfileStringA("audio_adtsaac", "profile", temp, cfg->config_file);

		StringCbPrintfA(temp, sizeof(temp), "%u", cfg->config.bitrate);
		WritePrivateProfileStringA("audio_adtsaac", "bitrate", temp, cfg->config_file);

		StringCbPrintfA(temp, sizeof(temp), "%u", cfg->config.surround);
		WritePrivateProfileStringA("audio_adtsaac", "surround", temp, cfg->config_file);
	}
}

void AACConfig_GetBitrateRange(const AACConfiguration *cfg, int *low, int *high)
{
	switch(cfg->profile)
	{
	case AAC_PROFILE_AUTOMATIC:
		*low = 12000;
		*high = 448000;
		break;

	case AAC_PROFILE_LC:
		*low = 16000;
		*high = 448000;
		break;

	case AAC_PROFILE_HE:
		*low = 16000;
		*high = 128000;
		break;

	case AAC_PROFILE_HE_V2:
		*low = 12000;
		*high = 56000;
		break;
	}
}

AUD_OBJ_TYP AACConfig_GetAOT(const AACConfiguration *cfg)
{
	if (cfg->mode == AAC_MODE_VBR)
	{
		switch(cfg->preset)
		{
		case 1:
			return AUD_OBJ_TYP_PS;
		case 2:
			return AUD_OBJ_TYP_HEAAC;
		default:
			return AUD_OBJ_TYP_LC; 
		}
	}
	else switch (cfg->profile) /* CBR */
	{
		case AAC_PROFILE_AUTOMATIC:
			if (cfg->bitrate <= 40)
				return AUD_OBJ_TYP_PS;
			else if (cfg->bitrate <= 80)
				return AUD_OBJ_TYP_HEAAC;
			else
				return AUD_OBJ_TYP_LC;
		case AAC_PROFILE_LC:
			return AUD_OBJ_TYP_LC;
		case AAC_PROFILE_HE:
			return AUD_OBJ_TYP_HEAAC;
		case AAC_PROFILE_HE_V2:

			return AUD_OBJ_TYP_PS;

	}
	return AUD_OBJ_TYP_LC;
}

int AACConfig_GetBitrate(const AACConfiguration *cfg, unsigned int channels)
{
	if (cfg->mode == AAC_MODE_VBR)
	{
		switch(cfg->preset)
		{
		case 1:
			return 16000*channels;
		case 2:
			return 32000*channels;
		case 3:
			return 48000*channels;
		case 4:
			return 64000*channels;		
		case 5:
			return 96000*channels;
		case 6:
			return 128000*channels;
		default:
			return 0;
		}
	}
	else
		return cfg->bitrate * 1000;
}

MPEG4ENC_BITRATE_MODE AACConfig_GetBitrateMode(const AACConfiguration *cfg)
{
	if (cfg->mode == AAC_MODE_VBR)
	{
		/* by coincidence, these match 
		to help future maintainers, let's assert this fact */
		assert(MP4_BR_MODE_VBR_1 == (MPEG4ENC_BITRATE_MODE)1);
		assert(MP4_BR_MODE_VBR_2 == (MPEG4ENC_BITRATE_MODE)2);
		assert(MP4_BR_MODE_VBR_3 == (MPEG4ENC_BITRATE_MODE)3);
		assert(MP4_BR_MODE_VBR_4 == (MPEG4ENC_BITRATE_MODE)4);
		assert(MP4_BR_MODE_VBR_5 == (MPEG4ENC_BITRATE_MODE)5);
		assert(MP4_BR_MODE_VBR_6 == (MPEG4ENC_BITRATE_MODE)6);
		return (MPEG4ENC_BITRATE_MODE)cfg->preset;
	}
	else /* CBR */
	{
		return MP4_BR_MODE_CBR;
	}
}

MPEG4ENC_CH_MODE AACConfig_GetChannelMode(const AACConfiguration *cfg, unsigned int channels)
{
	switch(channels)
	{
	case 1:
		return MP4_CH_MODE_MONO;
	case 2:
		if (cfg->mode == AAC_MODE_VBR)
		{
			if (cfg->preset == 1)
				return MP4_CH_MODE_PARAMETRIC_STEREO;
			else
				return MP4_CH_MODE_STEREO;
		}
		else /* CBR */
		{
			if (AACConfig_GetAOT(cfg) == AUD_OBJ_TYP_PS)
				return MP4_CH_MODE_PARAMETRIC_STEREO; 
			else
				return MP4_CH_MODE_STEREO;
		}
	case 3: return MP4_CH_MODE_3;
	case 4: return MP4_CH_MODE_4;
	case 5: return MP4_CH_MODE_5;
	case 6: return MP4_CH_MODE_5_1;
	case 8: return MP4_CH_MODE_7_1;
	default:
		return MP4_CH_MODE_INVALID;
	}
}

void AACConfig_GetToolString(const MPEG4ENC_SETUP *setup, char tool[], size_t cch)
{
	char version[128] = {0};
	MPEG4ENC_GetVersionInfo(version, sizeof(version)/sizeof(*version));
	char *p = version;
	while (p && *p)
	{
		if (*p != '.' && (*p < '0' || *p > '9'))
		{
			*p = 0;
			break;
		}
		p++;
	}


	switch(setup->bitrateMode)
	{
	case MP4_BR_MODE_CBR:
		StringCchPrintfA(tool, cch, "fhgaac v%s;CBR=%d", version, setup->nBitRate);
		break;
	case MP4_BR_MODE_VBR_1:
		StringCchPrintfA(tool, cch, "fhgaac v%s;VBR=1", version);
		break;
	case MP4_BR_MODE_VBR_2:
		StringCchPrintfA(tool, cch, "fhgaac v%s;VBR=2", version);
		break;
	case MP4_BR_MODE_VBR_3:
		StringCchPrintfA(tool, cch, "fhgaac v%s;VBR=3", version);
		break;
	case MP4_BR_MODE_VBR_4:
		StringCchPrintfA(tool, cch, "fhgaac v%s;VBR=4", version);
		break;
	case MP4_BR_MODE_VBR_5:
		StringCchPrintfA(tool, cch, "fhgaac v%s;VBR=5", version);
		break;
	case MP4_BR_MODE_VBR_6:
		StringCchPrintfA(tool, cch, "fhgaac v%s;VBR=6", version);
		break;
	}
}