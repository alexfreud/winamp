#pragma once
#include "../Plugins/Input/in_mp4/mpeg4video.h"
#include "MFTDecoder.h"

// {D3D14DCB-6AA8-4f59-A862-AA81D5AEE550}
static const GUID mp4_mp4v_guid = 
{ 0xd3d14dcb, 0x6aa8, 0x4f59, { 0xa8, 0x62, 0xaa, 0x81, 0xd5, 0xae, 0xe5, 0x50 } };

class MP4VMP4Decoder : public MP4VideoDecoder
{
public:
	static const char *getServiceName() { return "MPEG-4 Part 2 MP4 Decoder"; }
	static GUID getServiceGuid() { return mp4_mp4v_guid; } 
	MP4VMP4Decoder();
	~MP4VMP4Decoder();

private:
	/* mpeg4video interface */
	int Open(MP4FileHandle mp4_file, MP4TrackId mp4_track);
	int GetOutputFormat(int *x, int *y, int *color_format, double *aspect_ratio);
	int DecodeSample(const void *inputBuffer, size_t inputBufferBytes, MP4Timestamp timestamp);
	void Flush();
	void Close();
	int CanHandleCodec(const char *codecName);
	int GetPicture(void **data, void **decoder_data, MP4Timestamp *timestamp);
	void FreePicture(void *data, void *decoder_data);

	MFTDecoder decoder;
protected:
	RECVS_DISPATCH;
};