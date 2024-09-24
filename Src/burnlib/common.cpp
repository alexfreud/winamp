#include "./main.h"
#include "./resource.h"
#include "./primosdk.h"
#include <strsafe.h>


DWORD BeginBurn(obj_primo *primoSDK, DWORD drive, WABURNSTRUCT *burnstruct)
{	
	burnstruct->primoSDK = primoSDK;
	burnstruct->drive	= drive;
	DWORD rc[3]	= { PRIMOSDK_ERROR, PRIMOSDK_ERROR, PRIMOSDK_ERROR};
	burnstruct->blocker = (PBYTE)("WINAMPBURNER v1.0");
	DWORD retCode = PRIMOSDK_OK;
	// init Sonic PrimoSDK 
	if (!primoSDK)
		return PRIMOSDK_NOTLOADED;
	
	// block drive
	rc[0] = primoSDK->UnitVxBlock(&burnstruct->drive, PRIMOSDK_LOCK, burnstruct->blocker);
	if (PRIMOSDK_OK != rc[0]) retCode = rc[0];
	else 
	{
		rc[1] = primoSDK->UnitAIN(&burnstruct->drive, PRIMOSDK_LOCK);
		if (PRIMOSDK_OK != rc[1]) retCode = rc[1];
		else
		{
			rc[2] = primoSDK->UnitLock(&burnstruct->drive, PRIMOSDK_LOCK);
			if (PRIMOSDK_OK != rc[2]) retCode = rc[2];
		}
	}
	if (PRIMOSDK_OK != retCode)
	{
		if (PRIMOSDK_OK == rc[2]) primoSDK->UnitLock(&burnstruct->drive, PRIMOSDK_UNLOCK);
		if (PRIMOSDK_OK == rc[1]) primoSDK->UnitAIN(&burnstruct->drive, PRIMOSDK_UNLOCK);
		if (PRIMOSDK_OK == rc[0]) primoSDK->UnitVxBlock(&burnstruct->drive, PRIMOSDK_UNLOCK, burnstruct->blocker);
		burnstruct->drive	= 0x0000;
		burnstruct->blocker = NULL;
		burnstruct->primoSDK = NULL;
	}
	return retCode;
}

DWORD EndBurn(WABURNSTRUCT *burnstruct)
{
	if (!burnstruct) return PRIMOSDK_ERROR;
	if (0x0000 == burnstruct->drive) return PRIMOSDK_OK;
	DWORD rc[4];
	rc[0] = burnstruct->primoSDK->UnitLock(&burnstruct->drive, PRIMOSDK_UNLOCK);

	rc[1] = burnstruct->primoSDK->MoveMedium(&burnstruct->drive, 
			PRIMOSDK_IMMEDIATE | ((burnstruct->eject) ? PRIMOSDK_OPENTRAYEJECT : 0)); 
	if (0 == burnstruct->eject)
		rc[1] = PRIMOSDK_OK;

	rc[2] = burnstruct->primoSDK->UnitAIN(&burnstruct->drive, PRIMOSDK_UNLOCK);
	rc[3] = burnstruct->primoSDK->UnitVxBlock(&burnstruct->drive, PRIMOSDK_UNLOCK, burnstruct->blocker);
	
	burnstruct->drive	= 0x0000;
	burnstruct->blocker = NULL;

	int len = sizeof(rc)/sizeof(DWORD);
	for (int i = 0; i < len; i++) { if (PRIMOSDK_OK != rc[i]) return rc[i]; }

	return PRIMOSDK_OK;
}

DWORD GetMediumInfo(obj_primo *primoSDK, DWORD *drive, WAMEDIUMINFO *info)
{
	DWORD retCode(PRIMOSDK_OK), erasable(0);

	if (MAXDWORD != info->mediumType || MAXDWORD != info->mediumFormat || MAXDWORD != info->erasable ||
		MAXDWORD != info->tracks || MAXDWORD != info->usedSectors || MAXDWORD != info->freeSectors || MAXDWORD != info->recordable)
	{
		retCode = primoSDK->DiscInfoEx(drive, 0,(MAXDWORD == info->mediumType && MAXDWORD == info->recordable) ? NULL : &info->mediumType, 
												(MAXDWORD == info->mediumFormat) ? NULL : &info->mediumFormat,
												&erasable,
												(MAXDWORD == info->tracks) ? NULL : &info->tracks,
												(MAXDWORD == info->usedSectors) ? NULL : &info->usedSectors,
												(MAXDWORD == info->freeSectors) ? NULL : &info->freeSectors);
		if (MAXDWORD != info->erasable) info->erasable = (TRUE == erasable);
		if (MAXDWORD != info->recordable) info->recordable = (PRIMOSDK_COMPLIANTGOLD == info->mediumType || PRIMOSDK_BLANK == info->mediumType);
		if (PRIMOSDK_OK != retCode) return retCode;
	}

	DWORD medium, rfu3;
	BOOL needMediumInfo = (MAXDWORD != info->medium || MAXDWORD != info->isCD || MAXDWORD != info->isDCD || MAXDWORD != info->isDVD || MAXDWORD != info->isDLDVD);
	if (MAXDWORD != info->protectedDVD || needMediumInfo)
	{
		retCode = primoSDK->DiscInfo2(drive, !needMediumInfo ? NULL : &medium,
											(MAXDWORD == info->protectedDVD) ? NULL : &info->protectedDVD,
											NULL,
											!needMediumInfo ? NULL : &info->medium,
											&rfu3);
	}
	
	if (MAXDWORD != info->isCD) info->isCD = (PRIMOSDK_CDROM == info->medium || PRIMOSDK_CDR == info->medium || PRIMOSDK_CDRW == info->medium);
	if (MAXDWORD != info->isDCD) info->isDCD = (PRIMOSDK_DDCDR == info->medium || PRIMOSDK_DDCDRW == info->medium);
	if (MAXDWORD != info->isDVD) info->isDVD = (PRIMOSDK_DDCDROM == info->medium || PRIMOSDK_DVDR == info->medium || PRIMOSDK_DVDROM == info->medium || PRIMOSDK_DVDRAM == info->medium ||
										 PRIMOSDK_DVDRW == info->medium || PRIMOSDK_DVDPRW == info->medium || PRIMOSDK_DVDPR == info->medium);
	if (MAXDWORD != info->isDLDVD) info->isDLDVD = (PRIMOSDK_DVDPR9 == info->medium);
	return retCode;

}


wchar_t* GetMediumText(wchar_t *buffer, unsigned int cchBuffer,DWORD medium)
{
	UINT uid = IDS_UNKNOWN;
	switch(medium)
	{
		case PRIMOSDK_CDROM:		uid = IDS_MEDIUM_CD; 	break;
		case PRIMOSDK_CDR:		uid = IDS_MEDIUM_CDR; 	break;
		case PRIMOSDK_CDRW:		uid = IDS_MEDIUM_CDRW; 	break;
		case PRIMOSDK_DVDR:		uid = IDS_MEDIUM_DVDR; 	break;
		case PRIMOSDK_DVDROM:	uid = IDS_MEDIUM_DVD; 	break;
		case PRIMOSDK_DVDRAM:	uid = IDS_MEDIUM_DVDRAM;	break;
		case PRIMOSDK_DVDRW:		uid = IDS_MEDIUM_DVDRW; 	break;
		case PRIMOSDK_DVDPRW:	uid = IDS_MEDIUM_DVDPRW;	break;
		case PRIMOSDK_DVDPR:		uid = IDS_MEDIUM_DVDPR; 	break;
		case PRIMOSDK_DDCDROM:	uid = IDS_MEDIUM_DDCD; 	break;
		case PRIMOSDK_DDCDR:		uid = IDS_MEDIUM_DDCDR;	break;
		case PRIMOSDK_DDCDRW:	uid = IDS_MEDIUM_DDCDRW;	break;
		case PRIMOSDK_DVDPR9:	uid = IDS_MEDIUM_DVDPR9;	break;
		case PRIMOSDK_DVDR9:		uid = IDS_MEDIUM_DVDR9;	break;
		case PRIMOSDK_BDR:		uid = IDS_MEDIUM_BDR;	break;
		case PRIMOSDK_BDRE:		uid = IDS_MEDIUM_BDRE;	break;
	}
	LoadStringW(hResource, uid, buffer, cchBuffer);
	return buffer;
}
wchar_t*GetMediumTypeText(wchar_t *buffer, unsigned int cchBuffer, DWORD type)
{
	UINT uid = IDS_UNKNOWN;
	switch(type)
	{
		case PRIMOSDK_SILVER:			uid = IDS_MEDIUMTYPE_SILVER; break;
		case PRIMOSDK_COMPLIANTGOLD:		uid = IDS_MEDIUMTYPE_COMPILATIONGOLD; break;
		case PRIMOSDK_OTHERGOLD:			uid = IDS_MEDIUMTYPE_OTHERGOLD; break;
		case PRIMOSDK_BLANK:				uid = IDS_MEDIUMTYPE_BLANK; break;
	}
	LoadStringW(hResource, uid, buffer, cchBuffer);
	return buffer;
}
wchar_t* GetMediumFormatText(wchar_t *buffer, unsigned int cchBuffer, DWORD format)
{
	UINT uid = IDS_UNKNOWN;
	switch(format)
	{
		case PRIMOSDK_B1:	uid = IDS_MEDIUMFORMAT_B1; break;
		case PRIMOSDK_D1:	uid = IDS_MEDIUMFORMAT_D1; break;
		case PRIMOSDK_D2:	uid = IDS_MEDIUMFORMAT_D2; break;
		case PRIMOSDK_D3:	uid = IDS_MEDIUMFORMAT_D3; break;
		case PRIMOSDK_D4:	uid = IDS_MEDIUMFORMAT_D4; break;
		case PRIMOSDK_D5:	uid = IDS_MEDIUMFORMAT_D5; break;
		case PRIMOSDK_D6:	uid = IDS_MEDIUMFORMAT_D6; break;
		case PRIMOSDK_D7:	uid = IDS_MEDIUMFORMAT_D7; break;
		case PRIMOSDK_D8:	uid = IDS_MEDIUMFORMAT_D8; break;
		case PRIMOSDK_D9:	uid = IDS_MEDIUMFORMAT_D9; break;
		case PRIMOSDK_A1:	uid = IDS_MEDIUMFORMAT_A1; break;
		case PRIMOSDK_A2:	uid = IDS_MEDIUMFORMAT_A2; break;
		case PRIMOSDK_A3:	uid = IDS_MEDIUMFORMAT_A3; break;
		case PRIMOSDK_A4:	uid = IDS_MEDIUMFORMAT_A4; break;
		case PRIMOSDK_A5:	uid = IDS_MEDIUMFORMAT_A5; break;
		case PRIMOSDK_M1:	uid = IDS_MEDIUMFORMAT_M1; break;
		case PRIMOSDK_M2:	uid = IDS_MEDIUMFORMAT_M2; break;
		case PRIMOSDK_M3:	uid = IDS_MEDIUMFORMAT_M3; break;
		case PRIMOSDK_M4:	uid = IDS_MEDIUMFORMAT_M4; break;
		case PRIMOSDK_M5:	uid = IDS_MEDIUMFORMAT_M5; break;
		case PRIMOSDK_M6:	uid = IDS_MEDIUMFORMAT_M6; break;
		case PRIMOSDK_F1:	uid = IDS_MEDIUMFORMAT_F1; break;
		case PRIMOSDK_F2:	uid = IDS_MEDIUMFORMAT_F2; break;
		case PRIMOSDK_F3:	uid = IDS_MEDIUMFORMAT_F3; break;
		case PRIMOSDK_F4:	uid = IDS_MEDIUMFORMAT_F4; break;
		case PRIMOSDK_F5:	uid = IDS_MEDIUMFORMAT_F5; break;	 
		case PRIMOSDK_F8:	uid = IDS_MEDIUMFORMAT_F8; break;
		case PRIMOSDK_FA:	uid = IDS_MEDIUMFORMAT_FA; break;
		case PRIMOSDK_GENERICCD:	 uid = IDS_MEDIUMFORMAT_GENERICCD; break;
	}
	LoadStringW(hResource, uid, buffer, cchBuffer);
	return buffer;
}
wchar_t* GetPrimoCodeText(wchar_t *buffer, unsigned int cchBuffer, DWORD primoCode)
{
	UINT uid = IDS_UNKNOWN;
	switch(primoCode)
	{
		case PRIMOSDK_OK:					uid = IDS_PRIMOCODE_OK; break;
		case PRIMOSDK_CMDSEQUENCE:			uid = IDS_PRIMOCODE_CMDSEQUENCE; break;
		case PRIMOSDK_NOASPI:				uid = IDS_PRIMOCODE_NOASPI; break; //PRIMOSDK_NO_DRIVER - is the same code
		case PRIMOSDK_INTERR:				uid = IDS_PRIMOCODE_INTERR; break;
		case PRIMOSDK_BADPARAM:				uid = IDS_PRIMOCODE_BADPARAM; break;
		case PRIMOSDK_ALREADYEXIST:			uid = IDS_PRIMOCODE_ALREADYEXIST; break;
		case PRIMOSDK_NOTREADABLE:			uid = IDS_PRIMOCODE_NOTREADABLE; break;
		case PRIMOSDK_NOSPACE:				uid = IDS_PRIMOCODE_NOSPACE; break;
		case PRIMOSDK_INVALIDMEDIUM:			uid = IDS_PRIMOCODE_INVALIDMEDIUM; break;
		case PRIMOSDK_RUNNING:				uid = IDS_PRIMOCODE_RUNNING; break;
		case PRIMOSDK_BUR:					uid = IDS_PRIMOCODE_BUR; break;
		case PRIMOSDK_SCSIERROR:				uid = IDS_PRIMOCODE_SCSIERROR; break;
		case PRIMOSDK_UNITERROR:				uid = IDS_PRIMOCODE_UNITERROR; break;
		case PRIMOSDK_NOTREADY:				uid = IDS_PRIMOCODE_NOTREADY; break;
		case PRIMOSDK_INVALIDSOURCE:			uid = IDS_PRIMOCODE_INVALIDSOURCE; break;
		case PRIMOSDK_INCOMPATIBLE:			uid = IDS_PRIMOCODE_INCOMPATIBLE; break;
		case PRIMOSDK_FILEERROR:				uid = IDS_PRIMOCODE_FILEERROR; break;
		case PRIMOSDK_ITSADEMO:				uid = IDS_PRIMOCODE_ITSADEMO; break;
		case PRIMOSDK_USERABORT:				uid = IDS_PRIMOCODE_USERABORT; break;
		case PRIMOSDK_BADHANDLE:				uid = IDS_PRIMOCODE_BADHANDLE; break;
		case PRIMOSDK_BADUNIT:				uid = IDS_PRIMOCODE_BADUNIT; break;
		case PRIMOSDK_ERRORLOADING:			uid = IDS_PRIMOCODE_ERRORLOADING; break;
		case PRIMOSDK_NOAINCONTROL:			uid = IDS_PRIMOCODE_NOAINCONTROL; break;
		case PRIMOSDK_READERROR:				uid = IDS_PRIMOCODE_READERROR; break;
		case PRIMOSDK_WRITEERROR:			uid = IDS_PRIMOCODE_WRITEERROR; break;
		case PRIMOSDK_TMPOVERFLOW:			uid = IDS_PRIMOCODE_TMPOVERFLOW; break;
		case PRIMOSDK_DVDSTRUCTERROR:		uid = IDS_PRIMOCODE_DVDSTRUCTERROR; break;
		case PRIMOSDK_FILETOOLARGE:			uid = IDS_PRIMOCODE_FILETOOLARGE; break;
		case PRIMOSDK_CACHEFULL:				uid = IDS_PRIMOCODE_CACHEFULL; break;
		case PRIMOSDK_FEATURE_NOT_SUPPORTED:	uid = IDS_PRIMOCODE_FEATURE_NOT_SUPPORTED; break;
		case PRIMOSDK_FEATURE_DISABLED:		uid = IDS_PRIMOCODE_FEATURE_DISABLED; break;
		case PRIMOSDK_CALLBACK_ERROR:		uid = IDS_PRIMOCODE_CALLBACK_ERROR; break;
		case PRIMOSDK_PROTECTEDWMA:			uid = IDS_PRIMOCODE_PROTECTEDWMA; break;
	}
	LoadStringW(hResource, uid, buffer, cchBuffer);
	return buffer;

}
wchar_t* GetBussText(wchar_t *buffer, unsigned int cchBuffer, DWORD bussType)
{
	UINT uid = IDS_UNKNOWN;
	switch(bussType)
	{
		case PRIMOSDK_ATAPI:	uid = IDS_BUSSTYPE_ATAPI; break;
		case PRIMOSDK_SCSI: 	uid = IDS_BUSSTYPE_SCSI; break;
		case PRIMOSDK_1394: 	uid = IDS_BUSSTYPE_1394; break;
		case PRIMOSDK_USB:	uid = IDS_BUSSTYPE_USB; break;
		case PRIMOSDK_USB2: 	uid = IDS_BUSSTYPE_USB2; break;
	}
	LoadStringW(hResource, uid, buffer, cchBuffer);
	return buffer;
}

wchar_t* GetTrackTypeText(wchar_t *buffer, unsigned int cchBuffer, DWORD trackType)
{
	UINT uid = IDS_UNKNOWN;
	switch(trackType)
	{
		case PRIMOSDK_AUDIO_TRACK:	uid = IDS_TRACKTYPE_AUDIO; break;
		case PRIMOSDK_MODE1_TRACK:	uid = IDS_TRACKTYPE_TRACK2; break;
		case PRIMOSDK_MODE2_TRACK:	uid = IDS_TRACKTYPE_TRACK2; break;
	}
	LoadStringW(hResource, uid, buffer, cchBuffer);
	return buffer;
}

wchar_t* GetTimeString(wchar_t *string, unsigned int cchLen, unsigned int timesec)
{
	unsigned int min = timesec / 60;
	if (min > 0) timesec = timesec % 60;
	unsigned int hour = min / 60;
	if (hour > 0) min = min % 60;
	StringCchPrintfW(string, cchLen, L"%02d:%02d:%02d", hour, min, timesec);
	return string;
}