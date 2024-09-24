#include "ISOBurner.h"
#include "../nu/AutoChar.h"
#include "ifc_burner_writecallback.h"

ISOBurner::ISOBurner(obj_primo *_primo) : BurnerCommon(_primo)
{
}

int ISOBurner::Open()
{
	if (!primo)
		return 1;

	return 0;
}

int ISOBurner::Write(wchar_t driveLetter, const wchar_t *isoFile, int flags, unsigned int speed, ifc_burner_writecallback *callback)
{
	DWORD unit[] = {driveLetter, 0xFFFFFFFF};
	DWORD ret = primo->WriteOtherCDImage(unit, (PBYTE)(char *)AutoChar(isoFile), flags, speed);
	if (ret != PRIMOSDK_OK)
		return 1;

	while (1)
	{
		DWORD cursec = 0, totsec = 0;
		ret = primo->RunningStatus(PRIMOSDK_GETSTATUS, &cursec, &totsec);

		if (ret == PRIMOSDK_RUNNING)
		{
			if (callback)
			{
				int abort = callback->OnStatus(cursec, totsec);
				if (abort)
				{
					ret = primo->RunningStatus(PRIMOSDK_ABORT, 0, 0);
					callback = 0; // cheap way to prevent making further callbacks
				}
			}
			WaitForSingleObject(triggerEvent, 500);
		}
		else if (ret == PRIMOSDK_OK)
		{
			DWORD unit = driveLetter;
			ret = primo->UnitStatus(&unit, NULL, NULL, NULL, NULL);

			if (ret == PRIMOSDK_OK && callback)
				callback->Finished();
			break;
		}
		else
			break;
	}

	if (ret != PRIMOSDK_OK)
		return 1;

	return 0;
}

#define CBCLASS ISOBurner
START_DISPATCH;
CB(ISOBURNER_OPEN, Open)
CB(ISOBURNER_WRITE, Write)
VCB(ISOBURNER_FORCECALLBACK, ForceCallback)
END_DISPATCH;
#undef CBCLASS
