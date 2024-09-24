#include "c_encoder_mp3dll.h"
#include "../../utils.h"

T_ENCODER_MP3_INFO formatlist[] = {
	{8, 22050, 1, 44100, 2, 8}, {16, 22050, 1, 44100, 2, 8}, {24, 22050, 1, 44100, 2, 8},
	{32, 22050, 1, 44100, 2, 8}, {40, 22050, 1, 44100, 2, 8}, {48, 22050, 1, 44100, 2, 8},
	{48, 44100, 1, 44100, 2, 8}, {56, 22050, 1, 44100, 2, 8}, {56, 44100, 1, 44100, 2, 8},
	{64, 44100, 1, 44100, 2, 8}, {80, 44100, 1, 44100, 2, 8}, {96, 44100, 1, 44100, 2, 8},
	{112, 44100, 1, 44100, 2, 8}, {128, 44100, 1, 44100, 2, 8}, {40, 22050, 2, 44100, 2, 8},
	{48, 22050, 2, 44100, 2, 8}, {56, 22050, 2, 44100, 2, 8}, {64, 22050, 2, 44100, 2, 8},
	{80, 22050, 2, 44100, 2, 8}, {56, 44100, 2, 44100, 2, 8}, {64, 44100, 2, 44100, 2, 8},
	{80, 44100, 2, 44100, 2, 8}, {96, 44100, 2, 44100, 2, 8}, {112, 44100, 2, 44100, 2, 8},
	{128, 44100, 2, 44100, 2, 8}, {160, 44100, 2, 44100, 2, 8}, {192, 44100, 2, 44100, 2, 8},
	{224, 44100, 2, 44100, 2, 8}, {256, 44100, 2, 44100, 2, 8}, {320, 44100, 2, 44100, 2, 8}
};

static unsigned int formatlist_numEntries = sizeof(formatlist) / sizeof(T_ENCODER_MP3_INFO);

C_ENCODER_MP3::C_ENCODER_MP3(void *init, void *params, void *encode, void *finish) : C_ENCODER(sizeof(T_ENCODER_MP3_INFO)) { //sizeof(T_ENCODER_LAMEMP3_INFO)
	SetName("MP3 Encoder");
	T_ENCODER_MP3_INFO &EncInfo = *((T_ENCODER_MP3_INFO *)ExtendedInfoPtr);
	Handle = NULL;
	has_encoded = 0;
	EncInfo = formatlist[MP3_DEFAULT_ATTRIBNUM];
	hMutex = CreateMutex(NULL,TRUE,NULL);
	ReleaseMutex(hMutex);

	lame_init = (lame_t (__cdecl *)(void))init;
	lame_init_params = (int (__cdecl *)(lame_global_flags *))params;
	lame_encode_buffer_interleaved = (int (__cdecl *)(lame_global_flags *, short int pcm[], int num_samples, char *mp3buffer, int mp3buffer_size))encode;
	lame_encode_flush = (int (__cdecl *)(lame_global_flags *, char *mp3buffer, int size))finish;
}

C_ENCODER_MP3::~C_ENCODER_MP3() {
	WaitForSingleObject(hMutex,INFINITE);
	CloseHandle(hMutex);
	hMutex = NULL;
	C_ENCODER::~C_ENCODER();
}

void C_ENCODER_MP3::Close() {
	C_ENCODER::Close();
	if(lame_init != NULL) {
		if(has_encoded && lame_encode_flush) {
			char buf[1024] = {0};
			lame_encode_flush(Handle,(char *)buf,sizeof(buf));
		}
		//delete Handle; caused crash !! needs looking at
		Handle = NULL;
		has_encoded = 0;
	}
}

void C_ENCODER_MP3::Reset() {
	T_ENCODER_MP3_INFO &EncInfo = *(T_ENCODER_MP3_INFO *)ExtendedInfoPtr;
	if(WaitForSingleObject(hMutex,INFINITE) != WAIT_OBJECT_0) return;
	Close();
	if(lame_init != NULL && EncInfo.input_sampleRate != 0 && EncInfo.input_numChannels != 0) {
		if(EncInfo.output_sampleRate != 0 && EncInfo.output_bitRate != 0 && Handle == NULL) {
			has_encoded = 0;
			Handle = lame_init();
			Handle->samplerate_in = EncInfo.input_sampleRate;
			Handle->num_channels = 2;	// always process as 2 channels as it resolves issues with soundcard input in mono mode (which is padded to stereo)
			Handle->samplerate_out = EncInfo.output_sampleRate;
			Handle->mode = EncInfo.output_numChannels == 2 ? (MPEG_mode)0 : (MPEG_mode)3;
			Handle->brate = EncInfo.output_bitRate;
			Handle->VBR = vbr_off;
			Handle->write_lame_tag = 0;
			Handle->write_id3tag_automatic = 0;
			Handle->quality = EncInfo.QualityMode;
			if(Handle->quality < 0 || Handle->quality > 9) Handle->quality = 8;
			lame_init_params(Handle);
		} else {
			Handle = NULL;
		}
		for(unsigned int i = 0; i < formatlist_numEntries; i++) {
			char textbuf[256];
			formatlist[i].QualityMode = EncInfo.QualityMode;
			snprintf(textbuf,sizeof(textbuf),"%dkbps, %dHz, %s",formatlist[i].output_bitRate,formatlist[i].output_sampleRate,(formatlist[i].output_numChannels == 1 ? "Mono" : "Stereo"));
			T_ENCODER_MP3_INFO *attribs = new T_ENCODER_MP3_INFO;
			*attribs = formatlist[i];
			AddAttrib((char *)&textbuf,attribs);
		}
	}
	ReleaseMutex(hMutex);
}

int C_ENCODER_MP3::Encode(const void *inputbuf, const unsigned int inputbufsize, void *outputbuf, const unsigned int outputbufsize, int *inputamtused) {
	if((inputbuf != NULL) && (outputbuf != NULL) && (inputbufsize != 0) && (outputbufsize != 0) && (inputamtused != NULL) && (Handle != NULL)) {
		if(WaitForSingleObject(hMutex,INFINITE) != WAIT_OBJECT_0) return 0;
		int outputamtused = 0;
		if(lame_encode_buffer_interleaved) {
			outputamtused = lame_encode_buffer_interleaved(Handle, (short *)inputbuf, inputbufsize / (2 * sizeof(short)), (char *)outputbuf, outputbufsize);
			if(outputamtused < 0) {
				ReleaseMutex(hMutex);
				return 0;
			}
			has_encoded = 1;
		}
		*inputamtused = inputbufsize;
		ReleaseMutex(hMutex);
		return outputamtused;
	}
	return 0;
}