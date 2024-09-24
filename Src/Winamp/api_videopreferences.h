#ifndef __WASABI_API_VIDEOPREFERENCES_H
#define __WASABI_API_VIDEOPREFERENCES_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
class NOVTABLE api_videopreferences : public Dispatchable
{
public:
	DISPATCH_CODES
	{
		API_VIDEOPREFERENCES_IS_OUTPUT_YV12=10,
	};

	int IsOutputYV12();
};

inline int api_videopreferences::IsOutputYV12()
{
	return _call(API_VIDEOPREFERENCES_IS_OUTPUT_YV12, (int)0);
}


// {9DE9DBEE-1466-4da4-939D-2EFDDEA14DA7}
static const GUID videoPreferencesGUID = 
{ 0x9de9dbee, 0x1466, 0x4da4, { 0x93, 0x9d, 0x2e, 0xfd, 0xde, 0xa1, 0x4d, 0xa7 } };


#endif