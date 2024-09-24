#include "main.h"
#include "MP3Info.h"

StreamInfo::StreamInfo(void *buffer) : ID3Info()
{
	unsigned __int8 *header = (unsigned __int8 *)buffer;
	da_tag.Parse(header, &header[ID3_TAGHEADERSIZE]);
	GetID3V2Values();
}
