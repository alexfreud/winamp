#pragma once
#include <windows.h>
#include "../primo/obj_primo.h"

class BurnerCommon
{
public:
	BurnerCommon(obj_primo *_primo);
	~BurnerCommon();
	void TriggerCallback();
protected:
	obj_primo *primo;
	HANDLE triggerEvent;
};
