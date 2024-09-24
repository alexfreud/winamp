#ifndef NULLSOFT_BURNER_OBJ_ISOBURNER_H
#define NULLSOFT_BURNER_OBJ_ISOBURNER_H

#include <bfc/dispatch.h>

class ifc_burner_writecallback;
class obj_isoburner : public Dispatchable
{
protected:
	obj_isoburner(){}
		~obj_isoburner(){}
public:
	int Open();
	int Write(wchar_t driveLetter, const wchar_t *isoFile, int flags, unsigned int speed, ifc_burner_writecallback *callback);
	void ForceCallback(); // call this (from another thread) to force the Write() function to call your callback ASAP

	DISPATCH_CODES 
	{
		ISOBURNER_OPEN = 0,
			ISOBURNER_WRITE = 3,
			ISOBURNER_FORCECALLBACK = 4,
	};

};

inline int obj_isoburner::Open()
{
	return _call(ISOBURNER_OPEN, (int)1);
}

inline int obj_isoburner::Write(wchar_t driveLetter, const wchar_t *isoFile, int flags, unsigned int speed, ifc_burner_writecallback *callback)
{
	return _call(ISOBURNER_OPEN, (int)1, driveLetter, isoFile, flags, speed, callback);
}

inline void obj_isoburner::ForceCallback()
{
	_voidcall(ISOBURNER_FORCECALLBACK);
}

// {E3B85A59-E56F-4d2b-8ED2-F02BC4AF8BB5}
static const GUID obj_isoburnerGUID = 
{ 0xe3b85a59, 0xe56f, 0x4d2b, { 0x8e, 0xd2, 0xf0, 0x2b, 0xc4, 0xaf, 0x8b, 0xb5 } };


#endif