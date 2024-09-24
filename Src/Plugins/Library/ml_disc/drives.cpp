#include "main.h"
#include <Windows.h>
#include "resource.h"
#include "drives.h"
#include <strsafe.h>

//
// UNIT TYPES AND MEDIA TYPES
//
#define  PRIMOSDK_CDROM          0x00000201
#define  PRIMOSDK_CDR            0x00000202
#define  PRIMOSDK_CDRW           0x00000203
#define  PRIMOSDK_DVDR           0x00000204
#define  PRIMOSDK_DVDROM         0x00000205
#define  PRIMOSDK_DVDRAM         0x00000206
#define  PRIMOSDK_DVDRW          0x00000207
#define  PRIMOSDK_ROBOTICS       0x00000208
#define  PRIMOSDK_DVDPRW         0x00000209
#define  PRIMOSDK_DVDPR          0x00000210
#define  PRIMOSDK_DDCDROM        0x00000211
#define  PRIMOSDK_DDCDR          0x00000212
#define  PRIMOSDK_DDCDRW         0x00000213
#define  PRIMOSDK_DVDPR9			0x00000214
#define  PRIMOSDK_DVDR9         0x00000215

//

#define  PRIMOSDK_OTHER          0x00000220

// bus type
#define PRIMOSDK_UNKNOWN	0
#define PRIMOSDK_ATAPI		1
#define PRIMOSDK_SCSI		2 
#define PRIMOSDK_1394		3
#define PRIMOSDK_USB 		4
#define PRIMOSDK_USB2		5

const wchar_t *typeText[] = {L"UNKNOWN", L"CD-ROM", L"CD-R", L"CD-RW", L"DVD-ROM", L"DVD-R", L"DVD-RW", L"DVD+R", L"DVD+RW", L"DVD-RAM", L"DDCD", L"DDCD-R", L"DDCD-RW", L"DL DVD+R", L"DL DVD-R"};
const wchar_t *busText[] = {L"UNKNOWN", L"ATAPI", L"SCSI", L"1394", L"USB", L"USB2"};

Drives::Drives(void)
{
}

Drives::~Drives(void)
{
	Clear();
}

void Drives::AddDrive(wchar_t letter, unsigned int typeCode, wchar_t* description,  const wchar_t *extInfo)
{
	OPTICAL_DRIVE drive;
	drive.letter = (wchar_t)CharUpperW((wchar_t*)letter);
	drive.typeCode = typeCode;
	drive.modelInfo = (NULL == description) ? NULL : _wcsdup(description);
	drive.busType = 0;
	drive.disc = NULL;
	if (extInfo)
	{ // extInfo format: bysType;typeCode1;typeCode2;...;typeCoden
		const wchar_t *desc = extInfo;
		drive.nTypeList = 0;
		
		while(desc[0] != 0x00) {desc = CharNextW(desc); if (desc[0] == ';') drive.nTypeList ++;}
		if (drive.nTypeList) drive.nTypeList;

		drive.pTypeList = (int*) malloc(sizeof(int) * drive.nTypeList);
		int *list = drive.pTypeList;
		const wchar_t *start = extInfo;
		const wchar_t *end  = extInfo;
		BOOL cont;
		do
		{
			while(end[0] != ';' && end[0] != 0x00) end = CharNextW(end);
			
			cont = (end[0] == ';') ;
			
			if (start == extInfo) 
				drive.busType = _wtoi(start);
			else
			{
				*list = _wtoi(start);
				list++;
			}

			if(cont)
			{
				end = CharNextW(end);
				start = end;
			}
		}
		while(cont);
	}
	else
	{
		drive.nTypeList = 1;
		drive.pTypeList = (int*) malloc(sizeof(int) * drive.nTypeList);
		drive.pTypeList[0]  = typeCode;
	}
	
	Map<wchar_t, OPTICAL_DRIVE>::MapPair insert_pair(drive.letter, drive);
	driveList.insert(insert_pair);
}

void Drives::Clear(void)
{
	for (c_iter = driveList.begin(); c_iter != driveList.end(); c_iter++)
	{
		if (c_iter->second.modelInfo) free(c_iter->second.modelInfo);
		if (c_iter->second.pTypeList) free(c_iter->second.pTypeList);
		if (c_iter->second.disc) delete(c_iter->second.disc);
	}
	driveList.clear();
}

unsigned int Drives::GetCount(void)
{
	return (unsigned int)driveList.size();
}

const OPTICAL_DRIVE* Drives::GetFirst(void)
{
	c_iter = driveList.begin();
	return (c_iter == driveList.end()) ? NULL : &c_iter->second;
}

const OPTICAL_DRIVE* Drives::GetNext(void)
{
	return (++c_iter == driveList.end()) ? NULL : &c_iter->second;
}

BOOL Drives::IsRecorder(const OPTICAL_DRIVE *drive)
{
	BOOL recorder = FALSE;
	switch (drive->typeCode)
	{
		case PRIMOSDK_CDR:
		case PRIMOSDK_CDRW:
		case PRIMOSDK_DVDR:
		case PRIMOSDK_DVDRW:
		case PRIMOSDK_DVDPR:
		case PRIMOSDK_DVDPRW:
		case PRIMOSDK_DVDRAM:
		case PRIMOSDK_DDCDR:
		case PRIMOSDK_DDCDRW: 
		case PRIMOSDK_DVDPR9:
		case PRIMOSDK_DVDR9:
			recorder = TRUE;
			break;
	}
	return recorder;	
}

const wchar_t* Drives::GetTypeString(int typeCode)
{
	int index = 0;
	switch (typeCode)
	{
		case PRIMOSDK_CDROM:
			index = 1;
			break;
		case PRIMOSDK_CDR:
			index = 2;
			break;
		case PRIMOSDK_CDRW:
			index = 3;
			break;
		case PRIMOSDK_DVDROM:
			index = 4;
			break;
		case PRIMOSDK_DVDR:
			index = 5;
			break;
		case PRIMOSDK_DVDRW:
			index = 6;
			break;
		case PRIMOSDK_DVDPR:
			index = 7;
			break;
		case PRIMOSDK_DVDPRW:
			index = 8;
			break;
		case PRIMOSDK_DVDRAM:
			index = 9;
			break;
		case PRIMOSDK_DDCDROM:
			index = 10;
			break;
		case PRIMOSDK_DDCDR:
			index = 11;
			break;
		case PRIMOSDK_DDCDRW: 
			index = 12;
			break;
		case PRIMOSDK_DVDPR9:
			index = 13;
			break;
		case PRIMOSDK_DVDR9:
			index = 14;
			break;
		default: 
			static wchar_t tmp2[64];
			return WASABI_API_LNGSTRINGW_BUF(plugin.hDllInstance,IDS_UNKNOWN,tmp2,64);
	}
	return typeText[index];	
}

const wchar_t* Drives::GetBusString(int busCode)
{
	int index = 0;
	switch (busCode)
	{
		case PRIMOSDK_ATAPI:
			index = 1;
			break;
		case PRIMOSDK_SCSI:
			index = 2;
			break;
		case PRIMOSDK_1394:
			index = 3;
			break;
		case PRIMOSDK_USB:
			index = 4;
			break;
		case PRIMOSDK_USB2:
			index = 5;
			break;
		default:
			static wchar_t tmp3[64];
			return WASABI_API_LNGSTRINGW_BUF(plugin.hDllInstance,IDS_UNKNOWN,tmp3,64);
	}
	return busText[index];	
}

const wchar_t* Drives::GetFormatedString(const OPTICAL_DRIVE *drv, wchar_t *buffer, size_t size, BOOL useFullName)
{
	StringCchPrintfW(buffer, size, WASABI_API_LNGSTRINGW(plugin.hDllInstance,IDS_X_DRIVE_BRACKET_X),
					 GetTypeString(drv->typeCode), drv->letter);
	if (useFullName && drv->modelInfo)
	{
		StringCchPrintfW(buffer, size, L"%s  -  %s", buffer, drv->modelInfo);
	}
	return buffer;
}