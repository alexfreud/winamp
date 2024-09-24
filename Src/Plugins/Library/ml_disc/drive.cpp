#include "./main.h"
#include "./drive.h"
#include "./resource.h"
//#include <primosdk.h>
#include <strsafe.h>

static LPCWSTR pszBusType[] = 
{
	L"ATAPI",
	L"SCSI",
	L"1394",
	L"USB",
	L"USB2"
};

static LPCWSTR pszType[] = 
{
	L"CD-ROM",
	L"CD-R",
	L"CD-RW",
	L"DVD-ROM",
	L"DVD-R",
	L"DVD-RW",
	L"DVD+R",
	L"DVD+RW",
	L"DVD-RAM",
	L"DDCD-ROM",
	L"DDCD-R",
	L"DDCD-RW",
	L"DL DVD+R",
	L"DL DVD-R",
	L"BD-RW",
	L"BD-R",
	L"BD-ROM",
	L"HDDVD-RW",
	L"HDDVD-R",
	L"HDDVD-ROM",
};

static wchar_t buffer[64];

LPCWSTR Drive_GetBusTypeString(DWORD nBusType)
{
	int index = -1;
#if 0
	switch (nBusType)
	{
		case PRIMOSDK_ATAPI:		index = 0; break;
		case PRIMOSDK_SCSI:		index = 1; break; 
		case PRIMOSDK_1394:		index = 2; break;
		case PRIMOSDK_USB:		index = 3; break;
		case PRIMOSDK_USB2:		index = 4; break;
	}
#endif
	return (-1 != index) ? pszBusType[index] :
							WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN, buffer, sizeof(buffer)/sizeof(wchar_t));
}

LPCWSTR Drive_GetTypeString(DWORD nType)
{
	int index = -1;
#if 0
	switch (nType)
	{
		case PRIMOSDK_CDROM:		index = 0; break;
		case PRIMOSDK_CDR:		index = 1; break;
		case PRIMOSDK_CDRW:		index = 2; break;
		case PRIMOSDK_DVDROM:	index = 3; break;
		case PRIMOSDK_DVDR:		index = 4; break;
		case PRIMOSDK_DVDRW:		index = 5; break;
		case PRIMOSDK_DVDPR:		index = 6; break;
		case PRIMOSDK_DVDPRW:	index = 7; break;
		case PRIMOSDK_DVDRAM:	index = 8; break;
		case PRIMOSDK_DDCDROM:	index = 9; break;
		case PRIMOSDK_DDCDR:		index = 10; break;
		case PRIMOSDK_DDCDRW:	index = 11; break;
		case PRIMOSDK_DVDPR9:	index = 12; break;
		case PRIMOSDK_DVDR9:		index = 13; break;
		case PRIMOSDK_BDRE:		index = 14; break;
		case PRIMOSDK_BDR:		index = 15; break;
		case PRIMOSDK_BDROM:		index = 16; break;
		case PRIMOSDK_HDDVDRW:	index = 17; break;
		case PRIMOSDK_HDDVDR:	index = 18; break;
		case PRIMOSDK_HDDVDROM:	index = 19; break;
	}
#endif
	return (-1 != index) ? pszType[index] :
							WASABI_API_LNGSTRINGW_BUF(IDS_UNKNOWN, buffer, sizeof(buffer)/sizeof(wchar_t));
}

BOOL Drive_IsRecorderType(DWORD nType)
{
#if 0
	switch(nType)
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
		case PRIMOSDK_BDRE:
		case PRIMOSDK_BDR:	
		case PRIMOSDK_HDDVDRW:	
		case PRIMOSDK_HDDVDR:
			return TRUE;
	}
#endif
	return FALSE;
}

BOOL Drive_IsRecorder(CHAR cLetter)
{
#if 0
	wchar_t info[128] = {0};
	wchar_t name[] = L"cda://X.cda";
	DWORD result;
	BOOL reloaded = FALSE;

	name[6] = cLetter;

	for(;;)
	{
		result = getFileInfoW(name, L"cdinfo", info, sizeof(info)/sizeof(wchar_t));
		if (result || reloaded || !getFileInfoW(name, L"reloadsonic", NULL, 0)) break;
		reloaded = TRUE;
	}

	return (result) ? Drive_IsRecorderType(_wtoi(info)) : FALSE;
#else
	return FALSE;
#endif
}