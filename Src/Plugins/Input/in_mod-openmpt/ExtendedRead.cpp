#include "api__in_mod.h"
#include <libopenmpt/libopenmpt.h>
#include "../nsutil/pcm.h"

static const size_t kModBufferSize = 512;
static const unsigned int kModSampleRate = 44100; // TODO(benski) configurable!

openmpt_module *OpenMod(const wchar_t *filename);

class PlayParams
{
public:
	PlayParams();
	~PlayParams();
	openmpt_module *mod;
	float *buffer;
	int bps;
	int channels;
	int sample_rate;
	bool use_float;
	size_t (*openmpt_read)(openmpt_module * mod, int32_t samplerate, size_t count, float *interleaved_stereo);
};

PlayParams::PlayParams()
{
	mod = 0;
	buffer = 0;
}

PlayParams::~PlayParams()
{
	openmpt_module_destroy(mod);
	free(buffer);

}

static PlayParams *ExtendedOpen(const wchar_t *fn, int *size, int *bps, int *nch, int *srate, bool use_float) 
{
	float *float_buffer = 0;
	size_t (*openmpt_read)(openmpt_module * mod, int32_t samplerate, size_t count, float *interleaved_stereo)=openmpt_module_read_interleaved_float_stereo;

	openmpt_module *mod = OpenMod(fn);
	if (!mod) {
		return 0;
	}

	int requested_channels = *nch;
	int requested_bits = *bps;
	int requested_srate = *srate;

	if (!requested_channels) {
		requested_channels=2;
	}

	if (!requested_bits) {
		if (use_float) {
			requested_bits=32;
		} else {
			requested_bits=16;
		}
	}

	if (!requested_srate) {
		requested_srate = kModSampleRate;
	}

	if (requested_channels == 1) {
		openmpt_read = openmpt_module_read_float_mono;
	} else if (requested_channels < 4) {
		requested_channels = 2;
		openmpt_read = openmpt_module_read_interleaved_float_stereo;
	} else if (requested_channels) {
		requested_channels = 4;
		openmpt_read = openmpt_module_read_interleaved_float_quad;
	}

	if (!use_float) {
		float_buffer = (float *)malloc(sizeof(float) * kModBufferSize * requested_channels);
		if (!float_buffer) {
			openmpt_module_destroy(mod);
			return 0;
		}
	}

	PlayParams *play_params = new PlayParams;
	if (!play_params) {
		openmpt_module_destroy(mod);
		free(float_buffer);
		return 0;
	}

	play_params->mod = mod;
	play_params->buffer = float_buffer;
	play_params->bps = requested_bits;
	play_params->channels = requested_channels;
	play_params->use_float = use_float;
	play_params->openmpt_read = openmpt_read;
	play_params->sample_rate = requested_srate;

	*nch = requested_channels;
	*srate = requested_srate;
	*bps = requested_bits;

	*size = (int)(openmpt_module_get_duration_seconds(mod) * (double)requested_bits * (double)requested_srate * (double)requested_channels / 8.0);

	return play_params;
}
extern "C" __declspec(dllexport) intptr_t winampGetExtendedRead_openW_float(const wchar_t *fn, int *size, int *bps, int *nch, int *srate)
{
	return (intptr_t)ExtendedOpen(fn, size, bps, nch, srate, true);
}

extern "C" __declspec(dllexport) intptr_t winampGetExtendedRead_openW(const wchar_t *fn, int *size, int *bps, int *nch, int *srate)
{
	return (intptr_t)ExtendedOpen(fn, size, bps, nch, srate, false);
}

extern "C" __declspec(dllexport) size_t winampGetExtendedRead_getData(intptr_t handle, char *dest, size_t len, int *killswitch)
{
	PlayParams *play_params = (PlayParams *)handle;
	size_t requested_samples = len / (play_params->channels * play_params->bps/8);

	if (play_params->use_float) {
		return play_params->openmpt_read(play_params->mod, play_params->sample_rate, requested_samples, (float *)dest) * sizeof(float) * play_params->channels;
	} else {
		if (requested_samples > kModBufferSize) {
			requested_samples = kModBufferSize;
		}
		size_t count = play_params->openmpt_read(play_params->mod, play_params->sample_rate, requested_samples, play_params->buffer);
		nsutil_pcm_FloatToInt_Interleaved(dest, play_params->buffer, play_params->bps, play_params->channels*count);
		return count * play_params->bps * play_params->channels / 8;
	}
}

extern "C" __declspec(dllexport) void winampGetExtendedRead_close(intptr_t handle)
{
	PlayParams *play_params = (PlayParams *)handle;
	delete play_params;
}