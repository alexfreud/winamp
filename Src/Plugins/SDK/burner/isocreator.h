#pragma once
#include "obj_isocreator.h"
#include "BurnerCommon.h"


class ISOCreator : public obj_isocreator, protected BurnerCommon
{
public:
	ISOCreator(obj_primo *_primo);
	~ISOCreator();
	int Open(const wchar_t *volumeName, int format, int media);
	int AddFile(const wchar_t *source, const wchar_t *destination);
	int AddFolder(const wchar_t *folder);
	int Write(const wchar_t *destination, ifc_burner_writecallback *callback);
	inline void ForceCallback() { BurnerCommon::TriggerCallback(); }
	
protected:
	RECVS_DISPATCH;

};