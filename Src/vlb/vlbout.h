#pragma once
#define BUFSIZE 16384
#define OBUFSIZE 65536
#include "audio_io.h"
class VLBOut : public AudioIOControl {
public:
	VLBOut() { size = pos = 0; }
	virtual ~VLBOut() { }
	virtual int IO( float **, int );
	virtual int SetFormatInfo( AUDIO_FORMATINFO *info ) { format = *info; return AUDIO_ERROR_NONE; }
	AUDIO_FORMATINFO GetFormatInfo() { return format; }

	int BytesAvail() { return size; }
	virtual void PullBytes( unsigned char *buf, int nbytes );
	void Empty(){ size = 0; }
private:
	unsigned char data[OBUFSIZE];
	int size,pos;
	AUDIO_FORMATINFO format;
};
