#pragma once
#include "obj_databurner.h"
#include "BurnerCommon.h"

class DataBurner : public obj_databurner, protected BurnerCommon
{
public:
	DataBurner(obj_primo *primo);
	~DataBurner();
	int Open(const wchar_t *volumeName, wchar_t driveLetter, int format);
	int AddFile(const wchar_t *source, const wchar_t *destination);
	int AddFolder(const wchar_t *folder);
	int Write(int flags, unsigned int speed, ifc_burner_writecallback *callback);
	inline void ForceCallback() { BurnerCommon::TriggerCallback(); }
protected:
	RECVS_DISPATCH;

private:
	wchar_t driveLetter;
};
