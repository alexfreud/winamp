#include "./main.h"
#include "./medium.h"
#include "./drive.h"
#include "./resource.h"
//#include <primosdk.h>

static int pType[] = 
{
	IDS_STAMPED_DISC_OR_RECORDABLE_THAT_HAS_BEEN_RECORDED,
    IDS_REWRITEABLE_DISC_HAS_DATA_BUT_KEPT_OPEN_FOR_APPEND,
	IDS_REWRITEABLE_DISC_NOT_POSSIBLE_TO_APPEND_DATA,
	IDS_BLANK_REWRITEABLE_DISC,
};

static int pFormat[] = 
{ 
	IDS_MEDIA_BLANK_DISC,
	IDS_MEDIA_DATA_MODE_1_DAO,
	IDS_MEDIA_KODAK_PHOTO_CD,
	IDS_MEDIA_DATA_MULTISESSION_MODE_1_CLOSED,
	IDS_MEDIA_DATA_MULTISESSION_MODE_2_CLOSED,
	IDS_MEDIA_DATA_MODE_2_DAO,
	IDS_MEDIA_CDRFS,
	IDS_MEDIA_PACKET_WRITING,
	IDS_MEDIA_DATA_MULTISESSION_MODE_1_OPEN,
	IDS_MEDIA_DATA_MULTISESSION_MODE_2_OPEN,
	IDS_MEDIA_AUDIO_DAO_SAO_TAO,
	IDS_MEDIA_AUDIO_REWRITEABLE_DISC_WITH_SESSION_NOT_CLOSED,
	IDS_MEDIA_FIRST_TYPE_OF_ENHANCED_CD_ABORTED,
	IDS_MEDIA_CD_EXTRA,
	IDS_MEDIA_AUDIO_TAO_WITH_SESSION_NOT_WRITTEN,
	IDS_MEDIA_FIRST_TRACK_DATA_OTHERS_AUDIO,
	IDS_MEDIA_MIXED_MODE_MADE_TAO,
	IDS_MEDIA_KODAK_PORTFOLIO,
	IDS_MEDIA_VIDEO_CD,
	IDS_MEDIA_CDi,
	IDS_MEDIA_PLAYSTATION_SONY_GAMES,
	IDS_MEDIA_OBSOLETE,
	IDS_MEDIA_OBSOLETE_FOR_RESTRICTED_OVERWRITE_DVD,
	IDS_MEDIA_DVDROM_OR_CLOSED_RECORDABLE,
	IDS_MEDIA_INCREMENTAL_DVD_WITH_APPENDABLE_ZONE,
	IDS_MEDIA_APPENDABLE_DVD_OF_ANY_TYPE,
	IDS_MEDIA_DVDRAM_CARTRIDGE,
	IDS_MEDIA_CD_OTHER_TYPE,
};

static wchar_t buffer[256];

LPCWSTR Medium_GetTypeString(DWORD nType)
{
	int index = -1;
#if 0
	switch(nType)
	{
		case PRIMOSDK_SILVER:			index = 0; break;
		case PRIMOSDK_COMPLIANTGOLD:		index = 1; break;
		case PRIMOSDK_OTHERGOLD:			index = 2; break;
		case PRIMOSDK_BLANK:				index = 3; break;
	}
#endif
	return WASABI_API_LNGSTRINGW_BUF((-1 != index) ? pType[index] : IDS_UNKNOWN, buffer, 
										sizeof(buffer)/sizeof(wchar_t));
}

LPCWSTR Medium_GetPhysicalTypeString(DWORD nType)
{
	return Drive_GetTypeString(nType);
}

LPCWSTR Medium_GetFormatString(DWORD nFormat)
{
	int index = -1;
#if 0
	switch(nFormat)
	{
		case PRIMOSDK_B1: index = 0; break;
		case PRIMOSDK_D1: index = 1; break;
		case PRIMOSDK_D2: index = 2; break;
		case PRIMOSDK_D3: index = 3; break;
		case PRIMOSDK_D4: index = 4; break;
		case PRIMOSDK_D5: index = 5; break;
		case PRIMOSDK_D6: index = 6; break;
		case PRIMOSDK_D7: index = 7; break;
		case PRIMOSDK_D8: index = 8; break;
		case PRIMOSDK_D9: index = 9; break;
		case PRIMOSDK_A1: index = 10; break;
		case PRIMOSDK_A2: index = 11; break;
		case PRIMOSDK_A3: index = 12; break;
		case PRIMOSDK_A4: index = 13; break;
		case PRIMOSDK_A5: index = 14; break;
		case PRIMOSDK_M1: index = 15; break;
		case PRIMOSDK_M2: index = 16; break;
		case PRIMOSDK_M3: index = 17; break;
		case PRIMOSDK_M4: index = 18; break;
		case PRIMOSDK_M5: index = 19; break;
		case PRIMOSDK_M6: index = 20; break;
		case PRIMOSDK_F1: index = 21; break;
		case PRIMOSDK_F2: index = 22; break;
		case PRIMOSDK_F3: index = 23; break;
		case PRIMOSDK_F4: index = 24; break;
		case PRIMOSDK_F8: index = 25; break;
		case PRIMOSDK_FA: index = 26; break;
		case PRIMOSDK_GENERICCD: index = 27; break;
	}
#endif
	return WASABI_API_LNGSTRINGW_BUF((-1 != index) ? pFormat[index] : IDS_UNKNOWN, buffer, 
										sizeof(buffer)/sizeof(wchar_t));
}

BOOL Medium_IsRecordableType(DWORD nType)
{
	#if 0
	return (PRIMOSDK_COMPLIANTGOLD == nType || PRIMOSDK_BLANK == nType);
#else
	return FALSE;
#endif
}

BOOL Medium_IsRecordable(CHAR cLetter)
{
	wchar_t info[128] = {0};
	wchar_t name[] = L"cda://X.cda";
	DWORD result;
	BOOL reloaded = FALSE;

	name[6] = cLetter;
	
	for(;;)
	{
		result = getFileInfoW(name, L"cdtype", info, sizeof(info)/sizeof(wchar_t));
		if (result || reloaded || !getFileInfoW(name, L"reloadsonic", NULL, 0)) break;
		reloaded = TRUE;
	}
	
	return (result) ? (!lstrcmpW(info, L"CDR") || !lstrcmpW(info, L"CDRW")) : FALSE;
}