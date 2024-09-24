#ifndef _HEADER_MPG_H
#define _HEADER_MPG_H

#include <stdio.h>
#include "Header.h"

class HeaderMpg : public Header
{
public:
	HeaderMpg();

	int getInfos(const wchar_t *filename, bool checkMetadata=false);

	int video_bitrate,audio_bitrate;

private:
	HANDLE fh;

	int decodeHeader();

	unsigned char buf[8192*8];
	unsigned char *pbuf;
	unsigned char *end;
	int sync_packet() {
		// sync packet:
		while(1){
			if(pbuf>=end) return 0;
			if(pbuf[0]==0 && pbuf[1]==0 && pbuf[2]==1) break; // synced
			pbuf++;
		}
		return 0x100|pbuf[3];
	}

	int mp3headerFromInt(unsigned long dwHdrBits);
	int m_BitrateNdx;
	int m_SampleRateNdx;
	int m_Layer;
	int m_Id;
	int m_Mode;
	int m_Idex;
};

#endif