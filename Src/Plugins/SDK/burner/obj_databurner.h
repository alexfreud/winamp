#ifndef NULLSOFT_BURNER_OBJ_DATABURNER_H
#define NULLSOFT_BURNER_OBJ_DATABURNER_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
class ifc_burner_writecallback;

class obj_databurner : public Dispatchable
{
protected:
	obj_databurner() {}
	~obj_databurner() {}
public:
	int Open(const wchar_t *volumeName, wchar_t driveLetter, int format);
	int AddFile(const wchar_t *source, const wchar_t *destination);
	int AddFolder(const wchar_t *folder);
	int Write(int flags, unsigned int speed, ifc_burner_writecallback *callback);
	void ForceCallback(); // call this (from another thread) to force the Write() function to call your callback ASAP

	DISPATCH_CODES
	{
		DATABURNER_OPEN = 0,
		DATABURNER_ADDFILE = 1,
		DATABURNER_ADDFOLDER = 2,
		DATABURNER_WRITE = 3,
		DATABURNER_FORCECALLBACK = 4,
	};
};

inline int obj_databurner::Open(const wchar_t *volumeName, wchar_t driveLetter, int format)
{
	return _call(DATABURNER_OPEN, (int)1, volumeName, driveLetter, format);
}

inline int obj_databurner::AddFile(const wchar_t *source, const wchar_t *destination)
{
	return _call(DATABURNER_ADDFILE, (int)1, source, destination);
}

inline int obj_databurner::AddFolder(const wchar_t *folder)
{
	return _call(DATABURNER_ADDFOLDER, (int)1, folder);
}

inline int obj_databurner::Write(int flags, unsigned int speed, ifc_burner_writecallback *callback)
{
	return _call(DATABURNER_WRITE, (int)1, flags, speed, callback);
}

inline void obj_databurner::ForceCallback()
{
	_voidcall(DATABURNER_FORCECALLBACK);
}

// {0AF177FF-EC9E-4004-8886-B092879895BC}
static const GUID obj_databurnerGUID = 
{ 0xaf177ff, 0xec9e, 0x4004, { 0x88, 0x86, 0xb0, 0x92, 0x87, 0x98, 0x95, 0xbc } };


#endif