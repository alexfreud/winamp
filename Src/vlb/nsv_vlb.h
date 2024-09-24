#pragma once
#include "aacdecoderapi.h" 
#include "vlbout.h"
#include "VLBIO.h"
#include "../nsv/dec_if.h"
#include "../nsv/nsvlib.h"
/*
#if (defined WIN32 && !defined __ICL)
// Ignore libraries the aacplus library references (but doesn't need)
#pragma comment(linker, "-nodefaultlib:libmmt.lib")
#pragma comment(linker, "-nodefaultlib:libircmt.lib")
#pragma comment(linker, "-nodefaultlib:svml_disp.lib")
#endif
*/

///////////////////////////////////////////////////////////////////////
//
// Class Declarations
//
///////////////////////////////////////////////////////////////////////


class VLB_Decoder : public IAudioDecoder
{
public:
	VLB_Decoder();
	~VLB_Decoder();
	int decode( void *in, int in_len,
	            void *out, int *out_len,
	            unsigned int out_fmt[8] );
	void flush();
private:
	//FILE* m_f;
	CAacDecoderApi *aacdec;
	VLBIn datain;
	VLBOut dataout;
	AACStreamParameters params;
	int fused;
	int needsync;
	int packets_in_since_flush;
	int packets_decoded_since_flush;
};
