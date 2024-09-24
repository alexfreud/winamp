#pragma once

#include "obj_isoburner.h"
#include "BurnerCommon.h"

class ISOBurner : public obj_isoburner, protected BurnerCommon
{
public:
	ISOBurner(obj_primo *_primo);
	int Open();
	int Write(wchar_t driveLetter, const wchar_t *isoFile, int flags, unsigned int speed, ifc_burner_writecallback *callback);
	inline void ForceCallback() { BurnerCommon::TriggerCallback(); }
protected:
	RECVS_DISPATCH;
};