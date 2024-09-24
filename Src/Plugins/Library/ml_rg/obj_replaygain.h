#ifndef NULLSOFT_ML_RG_OBJ_REPLAYGAIN_H
#define NULLSOFT_ML_RG_OBJ_REPLAYGAIN_H

#include <bfc/dispatch.h>

enum
{
	RG_SUCCESS = 0,
	RG_FAILURE = 1,
	RG_MODE_NOT_SUPPORTED=2,

	RG_INDIVIDUAL_TRACKS = 0,  // use this mode to calculate each track sent individually
	RG_ALBUM = 1,  // use this mode to treat all tracks sent as belonging to the same album
	RG_AUTO = 2,  // retrieve tags from the files to determine album info
};

class obj_replaygain : public Dispatchable
{
protected:
	obj_replaygain() {}
	~obj_replaygain() {}
public:
	int Open(int mode);
	int ProcessTrack(const wchar_t *filename);
	int Write();
	void Close();

	DISPATCH_CODES 
	{
		OBJ_REPLAYGAIN_OPEN = 10,
		OBJ_REPLAYGAIN_PROCESSTRACK = 20,
		OBJ_REPLAYGAIN_WRITE = 30,
		OBJ_REPLAYGAIN_CLOSE = 40,
	};
};

inline int obj_replaygain::Open(int mode)
{
	return _call(OBJ_REPLAYGAIN_OPEN, (int)RG_FAILURE, mode);
}

inline int obj_replaygain::ProcessTrack(const wchar_t *filename)
{
	return _call(OBJ_REPLAYGAIN_PROCESSTRACK, (int)RG_FAILURE, filename);
}

inline int obj_replaygain::Write()
{
	return _call(OBJ_REPLAYGAIN_WRITE, (int)RG_FAILURE);
}

inline void obj_replaygain::Close()
{
	_voidcall(OBJ_REPLAYGAIN_CLOSE);
}

// {3A398A1B-D316-4094-993E-27EAEA553D19}
static const GUID RGGUID = 
{ 0x3a398a1b, 0xd316, 0x4094, { 0x99, 0x3e, 0x27, 0xea, 0xea, 0x55, 0x3d, 0x19 } };


#endif
