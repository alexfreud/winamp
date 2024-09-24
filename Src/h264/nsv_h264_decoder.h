#pragma once
#include "../nsv/dec_if.h"
#include "annexb.h"
#include "MFTDecoder.h"
#include <vector>

class H264_Decoder : public IVideoDecoder 
{
public:
	H264_Decoder();
	~H264_Decoder();
	int decode(int need_kf, 
		void *in, int in_len,
		void **out, // out is set to a pointer to data
		unsigned int *out_type, // 'Y','V','1','2' is currently defined
		int *is_kf);
	void flush();

private:
	MFTDecoder decoder;
	YV12_PLANES *vidbufdec;
	void *last_pic;
	std::vector<nullsoft_h264_frame_data> buffered_frames;
};
