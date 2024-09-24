#pragma once
#include "../nsv/enc_if.h"
class EncoderCommon : public AudioCoder
{
public:
	virtual void PrepareToFinish()=0;
	virtual void Finish(const wchar_t *filename){}
	char tool[256];
};