#include "FLVAudioHeader.h"

/*
(c) 2006 Nullsoft, Inc.
Author: Ben Allison benski@nullsoft.com 
*/

/*
soundType 	 1 bit (byte & 0x01) >> 0 	 0: mono, 1: stereo
soundSize 	1 bit (byte & 0x02)>> 1 	0: 8-bit, 2: 16-bit
soundRate 	2 bits (byte & 0x0C) >> 2 	0: 5.5 kHz, 1: 11 kHz, 2: 22 kHz, 3: 44 kHz
soundFormat 	4 bits (byte & 0xf0) >> 4 	0: Uncompressed, 1: ADPCM, 2: MP3, 5: Nellymoser 8kHz mono, 6: Nellymoser
*/

bool FLVAudioHeader::Read(unsigned __int8 *data, size_t size)
{
	if (size < 1)
		return false; // header size too small


	unsigned __int8 byte = data[0];

	stereo = !!((byte & 0x01) >> 0);
	bits = ((byte & 0x02) >> 1) ? 16 : 8;
	switch ((byte & 0x0C) >> 2)
	{
		case 0: sampleRate = 5512; break;
		case 1: sampleRate = 11025; break;
		case 2: sampleRate = 22050; break;
		case 3: sampleRate = 44100; break;
	}
	format = (byte & 0xf0) >> 4;
	return true;
}