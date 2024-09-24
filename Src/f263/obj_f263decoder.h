#ifndef F263_OBJ_F263DECODER_H
#define F263_OBJ_F263DECODER_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
struct YV12_PLANES;
class obj_f263decoder : public Dispatchable
{
protected:
	obj_f263decoder() {}
	~obj_f263decoder() {}
public:
	int DecodeFrame(void *frameData, size_t frameSize, YV12_PLANES *yv12, int *width, int *height, int *keyframe);

	enum
	{
		DISP_DECODEFRAME = 0,
	};
	enum
	{
		SUCCESS = 0,
		FAILURE = 1,
		FAILURE_TOO_MUCH_DATA = 2,
		FAILURE_NO_DATA = 3,
	};
};

inline int obj_f263decoder::DecodeFrame(void *frameData, size_t frameSize, YV12_PLANES *yv12, int *width, int *height, int *keyframe)
{
	return _call(DISP_DECODEFRAME, (int)FAILURE, frameData, frameSize, yv12, width, height, keyframe);
}

// {496FA082-39F0-424e-9B25-1B234262796D}
static const GUID obj_f263decoderGUID = 
{ 0x496fa082, 0x39f0, 0x424e, { 0x9b, 0x25, 0x1b, 0x23, 0x42, 0x62, 0x79, 0x6d } };

#endif