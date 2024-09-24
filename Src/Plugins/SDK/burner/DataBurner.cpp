#include "DataBurner.h"
#include "ifc_burner_writecallback.h"
#include <malloc.h>
#include <strsafe.h>

DataBurner::DataBurner(obj_primo *_primo) : BurnerCommon(_primo)
{
	driveLetter=0;
}

DataBurner::~DataBurner()
{
	if (primo)
		primo->CloseImage();
}

int DataBurner::Open(const wchar_t *volumeName, wchar_t driveLetter, int format)
{
	this->driveLetter = driveLetter;
	if (!primo)
		return 1;
	

		DWORD units[] = {driveLetter, 0xFFFFFFFF};


	// TODO: use PrimoSDK_DiscInfoEx to verify that a disc is available

	// TODO: PrimoSDK_UnitVxBlock after we're done debugging
	// TODO: PrimoSDK_UnitAIN
	// TODO: PrimoSDK_UnitLock

	DWORD ret = primo->NewImageWCS(units,
		(PWORD)volumeName,
		0, // no track session # needed, I don't think
		PRIMOSDK_ORIGDATE | format,
		0,  // TODO
		0 // TODO
		); // TODO: validate format

	return 0;
}

// TODO: check AddFolderWCS for return values
static void createDirForFileW(obj_primo *primo, const wchar_t *str)
{
	const wchar_t *p = str;
	if ((p[0] ==L'\\' || p[0] ==L'/') && (p[1] ==L'\\' || p[1] ==L'/'))
	{
		p += 2;
		while (*p && *p !=L'\\' && *p !=L'/') p++;
		if (!*p) return ;
		p++;
		while (*p && *p !=L'\\' && *p !=L'/') p++;
	}
	else
	{
		while (*p && *p !=L'\\' && *p !=L'/') p++;
	}

	while (*p)
	{
		while (*p !=L'\\' && *p !=L'/' && *p) p = CharNextW(p);
		if (*p)
		{
			size_t copySize = (p-str+1)*sizeof(wchar_t);
			wchar_t *copy = (wchar_t *)alloca(copySize);
			StringCbCopy(copy, copySize, str);
			primo->AddFolderWCS(copy);
			p++;
		}
	}
}

int DataBurner::AddFile(const wchar_t *source, const wchar_t *destination)
{
	createDirForFileW(primo, destination);
	DWORD ret = primo->AddFileWCS((PWORD)destination, (PWORD)source);

	if (ret != PRIMOSDK_OK)
		return 1;

	return 0;
}

int DataBurner::AddFolder(const wchar_t *folder)
{
	createDirForFileW(primo, folder);
	DWORD ret = primo->AddFolderWCS(folder); // add again in case they didn't terminate with a slash
	if (ret != PRIMOSDK_OK)
		return 1;

	return 0;
}

int DataBurner::Write(int flags, unsigned int speed, ifc_burner_writecallback *callback)
{
	DWORD size=0;
	DWORD ret = primo->WriteImage(flags, speed, &size);
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

#define CBCLASS DataBurner
START_DISPATCH;
CB(DATABURNER_OPEN, Open)
CB(DATABURNER_ADDFILE, AddFile)
CB(DATABURNER_ADDFOLDER, AddFolder)
CB(DATABURNER_WRITE, Write)
VCB(DATABURNER_FORCECALLBACK, ForceCallback)
END_DISPATCH;
#undef CBCLASS
