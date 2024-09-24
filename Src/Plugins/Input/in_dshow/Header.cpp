#include "Header.h"
#include <windows.h>
#include "header_avi.h"
#include "header_mpg.h"
#include "header_asf.h"
#include "header_wav.h"

extern const wchar_t *extension(const wchar_t *fn);

Header *MakeHeader(const wchar_t *filename, bool metadata)
{
	const wchar_t *ext=extension(filename);

	Header *header=0;
	if(!_wcsicmp(ext, L"asf") || !_wcsicmp(ext, L"wmv") || !_wcsicmp(ext, L"wma")) 
		header = new HeaderAsf();
	else if(!_wcsicmp(ext, L"avi") || !_wcsicmp(ext, L"divx")) 
		header = new HeaderAvi();
	else if(!_wcsicmp(ext, L"mpg") || !_wcsicmp(ext, L"mpeg")) 
		header = new HeaderMpg();
	else if (!_wcsicmp(ext, L"wav"))
		header = new HeaderWav();

	if (header && !header->getInfos(filename, metadata))
	{
		delete header;
		return 0;
	}
	return header;
}

Header::Header()
{
	length = -1;
	has_audio = has_video = false;
	title = artist = comment = genre = album = composer = publisher = 0;
	audio_nch = audio_bps = audio_srate = video_w = video_h = 0;
}

Header::~Header()
{
	free(title);
	free(artist);
	free(comment);
	free(genre);
	free(album);
	free(composer);
	free(publisher);
}
