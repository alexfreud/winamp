
#ifndef __C_ENCODER_MP3DLL_H__
#define __C_ENCODER_MP3DLL_H__

#include "c_encoder.h"
#include "../lame/include/lame.h"
#include "../lame/libmp3lame/lame_global_flags.h"
#include <windows.h>

// Defaults for this encoder
#define MP3_DEFAULT_INPUTSAMPLERATE 44100
#define MP3_DEFAULT_INPUTNUMCHANNELS 2
#define MP3_DEFAULT_OUTPUTSAMPLERATE 44100
#define MP3_DEFAULT_OUTPUTNUMCHANNELS 2
#define MP3_DEFAULT_OUTPUTBITRATE 96

#define MP3_DEFAULT_ATTRIBNUM 22

struct T_ENCODER_MP3_INFO {
	int output_bitRate;
	int output_sampleRate;
	int output_numChannels;
	int input_sampleRate;
	int input_numChannels;
	int QualityMode;
};

#define HBE_STREAM lame_global_flags *

class C_ENCODER_MP3 : public C_ENCODER {
private:
	HANDLE hMutex;
    lame_t Handle;
    int has_encoded;
protected:
	lame_t (*lame_init)(void);
	int (*lame_init_params)(lame_global_flags *);
	int (*lame_encode_buffer_interleaved)(lame_global_flags *, short int pcm[], int num_samples, char *mp3buffer, int mp3buffer_size);
	int (*lame_encode_flush)(lame_global_flags *, char *mp3buffer, int size);

public:
	C_ENCODER_MP3(void *init, void *params, void *encode, void *finish);
    virtual ~C_ENCODER_MP3();

    virtual void Close();
    virtual void Reset();

    virtual int Encode(const void *inputbuf, const unsigned int inputbufsize, void *outputbuf, const unsigned int outputbufsize, int *inputamtused); /* all values are in BYTES! */
};

#endif /* !__C_ENCODER_MP3_H__ */