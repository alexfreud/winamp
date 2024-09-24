#ifndef NULLSOFT_IN_DSHOW_HEADER_H
#define NULLSOFT_IN_DSHOW_HEADER_H
#include <wchar.h>
class Header
{
public:
	Header();
	virtual ~Header();

	virtual int getInfos(const wchar_t *filename, bool checkMetadata)=0;

	// benski> this is a shitty way to do it, but there's too much code I'd have to change
	wchar_t *title, *artist, *comment, *genre, *album, *composer, *publisher;
	bool has_audio, has_video;	
	int length;
	int audio_nch, audio_bps, audio_srate;
	int video_w, video_h;
};

Header *MakeHeader(const wchar_t *filename, bool metadata);

#endif