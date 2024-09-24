#include "ISOCreator.h"
#include <malloc.h>
#include "../nu/AutoChar.h"
#include "ifc_burner_writecallback.h"
#include <strsafe.h>

ISOCreator::ISOCreator(obj_primo *_primo) : BurnerCommon(_primo)
{
}

ISOCreator::~ISOCreator()
{
	if (primo)
		primo->CloseImage();
}

int ISOCreator::Open(const wchar_t *volumeName, int format, int media)
{
	if (!primo)
		return 1;

	DWORD units=0xFFFFFFFF;
	DWORD ret = primo->NewImageWCS(
		&units,
		(PWORD)volumeName,
		0, // no track session # needed, I don't think
		PRIMOSDK_ORIGDATE | format | media,
		0,  // don't think we need a swap file for ISO
		0 // probably don't need a swap location either, we'll see ...			
		); // TODO: validate format and media

	if (ret != PRIMOSDK_OK)
		return 1;

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

int ISOCreator::AddFile(const wchar_t *source, const wchar_t *destination)
{
	createDirForFileW(primo, destination);
	DWORD ret = primo->AddFileWCS((PWORD)destination, (PWORD)source);

	if (ret != PRIMOSDK_OK)
		return 1;

	return 0;
}

int ISOCreator::AddFolder(const wchar_t *folder)
{
	createDirForFileW(primo, folder);
	DWORD ret = primo->AddFolderWCS(folder); // add again in case they didn't terminate with a slash
	if (ret != PRIMOSDK_OK)
		return 1;

	return 0;
}

int ISOCreator::Write(const wchar_t *destination, ifc_burner_writecallback *callback)
{
	DWORD size=0;
	DWORD ret = primo->SaveImage((PBYTE)(char *)AutoChar(destination), &size);
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
			if (callback)
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

#define CBCLASS ISOCreator
START_DISPATCH;
CB(ISOCREATOR_OPEN, Open)
CB(ISOCREATOR_ADDFILE, AddFile)
CB(ISOCREATOR_ADDFOLDER, AddFolder)
CB(ISOCREATOR_WRITE, Write)
VCB(ISOCREATOR_FORCECALLBACK, ForceCallback)
END_DISPATCH;
#undef CBCLASS
