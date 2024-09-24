#include "../Plugins/Input/in_mp4/mpeg4video.h"
#include "loader_jpg.h"

// {CFF4B746-8D98-48c1-BDDF-9AA750F51517}
static const GUID mp4_jpeg_guid = 
{ 0xcff4b746, 0x8d98, 0x48c1, { 0xbd, 0xdf, 0x9a, 0xa7, 0x50, 0xf5, 0x15, 0x17 } };


class MP4JPEGDecoder : public MP4VideoDecoder
{
public:
	static const char *getServiceName() { return "JPEG MP4 Decoder"; }
	static GUID getServiceGuid() { return mp4_jpeg_guid; } 
	MP4JPEGDecoder();
	int Open(MP4FileHandle mp4_file, MP4TrackId mp4_track);
	int GetOutputFormat(int *x, int *y, int *color_format, double *aspect_ratio);
	int DecodeSample(const void *inputBuffer, size_t inputBufferBytes, MP4Timestamp timestamp);
	void Close();
	int CanHandleCodec(const char *codecName);
	int GetPicture(void **data, void **decoder_data, MP4Timestamp *timestamp);
	void FreePicture(void *data, void *decoder_data);
protected:
	RECVS_DISPATCH;
private:
	JpgLoad *jpegLoader; // new during Open(), since we might have been created just for CanHandleCodec
	int width, height;
	void *decoded_image; // ARGB32

};