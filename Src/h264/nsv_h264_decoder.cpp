#include "nsv_h264_decoder.h"
#include "../nsv/nsvlib.h"
#include "../nsv/dec_if.h"
#include <assert.h>
#include <Mferror.h>

H264_Decoder::H264_Decoder()
{
	vidbufdec=0;
	last_pic = 0;
	decoder.Open();
}

H264_Decoder::~H264_Decoder()
{	
	for (size_t i=0;i<buffered_frames.size();i++) {
		nullsoft_h264_frame_data frame_data = buffered_frames[i];
		decoder.FreeFrame((YV12_PLANES *)frame_data.data, frame_data.decoder_data);
	}

	decoder.FreeFrame(vidbufdec, last_pic);
}

int H264_Decoder::decode(int need_kf, 
						 void *_in, int _in_len,
						 void **out, // out is set to a pointer to data
						 unsigned int *out_type, // 'Y','V','1','2' is currently defined
						 int *is_kf)
{
	*out_type=NSV_MAKETYPE('Y','V','1','2');

	if (last_pic)
	{
		decoder.FreeFrame(vidbufdec, last_pic);
		vidbufdec=0;
		last_pic=0;
	}

	if (_in_len) {
		for (;;) {
			HRESULT hr = decoder.FeedRaw(_in, _in_len, 0);
			if (hr == MF_E_NOTACCEPTING) {
				nullsoft_h264_frame_data frame_data;
				if (FAILED(decoder.GetFrame((YV12_PLANES **)&frame_data.data, &frame_data.decoder_data, &frame_data.local_timestamp))) {
					continue;
				}
				buffered_frames.push_back(frame_data);
			} else if (FAILED(hr)) {
				return -1;
			} else {
				break;
			}
		}
	} else {
		decoder.Drain();
	}

	if (SUCCEEDED(decoder.GetFrame(&vidbufdec, &last_pic, 0))) {
		*out = vidbufdec;
		*is_kf = 1;
	} else {
		*out = 0;
	}

	return 0;
}

void H264_Decoder::flush()
{
	for ( size_t i = 0; i < buffered_frames.size(); i++ )
	{
		nullsoft_h264_frame_data frame_data = buffered_frames[ i ];
		decoder.FreeFrame( (YV12_PLANES *)frame_data.data, frame_data.decoder_data );
	}

	decoder.Flush();
}
