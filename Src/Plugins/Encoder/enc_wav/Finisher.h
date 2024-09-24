#ifndef NULLSOFT_ENC_WAV_FINISHER_H
#define NULLSOFT_ENC_WAV_FINISHER_H

class AudioCommon : public AudioCoder
{
public:
	virtual void FinishAudio(const wchar_t *filename)=0;
};

#endif
