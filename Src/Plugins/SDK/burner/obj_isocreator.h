#pragma once
#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
class ifc_burner_writecallback;

class obj_isocreator : public Dispatchable
{
protected:
	obj_isocreator(){}
	~obj_isocreator(){}
public:
	int Open(const wchar_t *volumeName, int format, int media);
	int AddFile(const wchar_t *source, const wchar_t *destination);
	int AddFolder(const wchar_t *folder); // AddFile auto-creates the folders.  Use AddFolder if you need to create an empty directory
	int Write(const wchar_t *destination, ifc_burner_writecallback *callback);
	void ForceCallback(); // call this (from another thread) to force the Write() function to call your callback ASAP

	enum
	{
		FORMAT_ISOLEVEL1 = 0x00000100,
		FORMAT_JOLIET = 0x00000200,
		FORMAT_UDF = 0x00000400,
		FORMAT_ISOLEVEL2 = 0x00200000,
		FORMAT_ISOLEVEL3 = 0x00400000,
		FORMAT_UDF201 = 0x00100000,

		MEDIA_CD = 0x0,
		MEDIA_DVD = 0x08000000,
	};

	enum
	{
		ISOCREATOR_OPEN = 0,
		ISOCREATOR_ADDFILE = 1,
		ISOCREATOR_ADDFOLDER = 2,
		ISOCREATOR_WRITE = 3,
		ISOCREATOR_FORCECALLBACK = 4,
	};

};
	
inline int obj_isocreator::Open(const wchar_t *volumeName, int format, int media)
{
	return _call(ISOCREATOR_OPEN, (int)1, volumeName, format, media);
}

inline int obj_isocreator::AddFile(const wchar_t *source, const wchar_t *destination)
{
	return _call(ISOCREATOR_ADDFILE, (int)1, source, destination);
}

inline int obj_isocreator::AddFolder(const wchar_t *folder)
{
	return _call(ISOCREATOR_ADDFOLDER, (int)1, folder);
}

inline int obj_isocreator::Write(const wchar_t *destination, ifc_burner_writecallback *callback)
{
	return _call(ISOCREATOR_WRITE, (int)1, destination, callback);
}

inline void obj_isocreator::ForceCallback()
{
	_voidcall(ISOCREATOR_FORCECALLBACK);
}

// {4F0ED42C-A6C1-4c60-97D1-16D9BD02E461}
static const GUID obj_isocreatorGUID = 
{ 0x4f0ed42c, 0xa6c1, 0x4c60, { 0x97, 0xd1, 0x16, 0xd9, 0xbd, 0x2, 0xe4, 0x61 } };

