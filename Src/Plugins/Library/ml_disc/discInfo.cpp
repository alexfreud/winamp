#include "main.h"
#include "discinfo.h"
#include "resource.h"
#include <strsafe.h>

// indices
#define DD_PARSED				0x0000
#define DD_INDEX_MEDIUM_TYPE	0x0001
#define DD_INDEX_MEDIUM_FORMAT	0x0002
#define DD_INDEX_MEDIUM			0x0003
#define DD_INDEX_PROTECTED_DVD	0x0004
#define DD_INDEX_ERASABLE		0x0005
#define DD_INDEX_TRACKS_NUM		0x0006
#define DD_INDEX_SECTOR_USED	0x0007
#define DD_INDEX_SECTOR_FREE	0x0008

#define TEXT_UNKNOWN			L"Unknown"
#define TEXT_TRUE				L"Yes"
#define TEXT_FALSE				L"No"

// MediumType Values
#define PRIMOSDK_SILVER 		0x00000301 		/* A disc that is not recordable. It may be a stamped (silver) disc or a gold (recordable) disc that has been recorded Disc-At-Once. */
#define PRIMOSDK_COMPLIANTGOLD 	0x00000302 		/* A gold disc or rewritable disc that contains data but remains open, allowing the appending of additional data. */
#define PRIMOSDK_OTHERGOLD 		0x00000303 		/* A gold disc to which it is not possible for PrimoSDK to append additional data. */
#define PRIMOSDK_BLANK 			0x00000304		/* A blank gold disc or blank rewritable disc. */

// MediumFormat Values
#define PRIMOSDK_B1 			0x000000B1 	/* Blank disc */
#define PRIMOSDK_D1 			0x000000D1 	/* Data Mode 1 DAO (e.g. most data CD-ROMs or typical DOS games) */
#define PRIMOSDK_D2 			0x000000D2 	/* Kodak Photo CD: Data multisession Mode 2 TAO */
#define PRIMOSDK_D3 			0x000000D3 	/* Gold Data Mode 1: Data multisession Mode 1, closed */
#define PRIMOSDK_D4 			0x000000D4 	/* Gold Data Mode 2: Data multisession Mode 2, closed */
#define PRIMOSDK_D5 			0x000000D5 	/* Data Mode 2 DAO (silver mastered from Corel or Toast gold) */
#define PRIMOSDK_D6 			0x000000D6 	/* CDRFS: Fixed packet (from Sony packet-writing solution) */
#define PRIMOSDK_D7 			0x000000D7 	/* Packet writing */
#define PRIMOSDK_D8 			0x000000D8 	/* Gold Data Mode 1: Data multisession Mode 1, open */
#define PRIMOSDK_D9 			0x000000D9 	/* Gold Data Mode 2: Data multisession Mode 2, open */
#define PRIMOSDK_A1 			0x000000A1 	/* Audio DAO/SAO/TAO (like most silver music discs) or closed gold audio */
#define PRIMOSDK_A2 			0x000000A2 	/* Audio Gold disc with session not closed (TAO or SAO) */
#define PRIMOSDK_A3 			0x000000A3 	/* First type of Enhanced CD (aborted) */
#define PRIMOSDK_A4 			0x000000A4 	/* CD Extra, Blue Book standard */
#define PRIMOSDK_A5 			0x000000A5 	/* Audio TAO with session not written (in-progress compilation) */
#define PRIMOSDK_M1 			0x000000E1 	/* First track data, others audio */
#define PRIMOSDK_M2 			0x000000E2 	/* Mixed-mode made TAO */
#define PRIMOSDK_M3 			0x000000E3 	/* Kodak Portfolio (as per the Kodak standard) */
#define PRIMOSDK_M4 			0x000000E4 	/* Video CD (as the White Book standard) */
#define PRIMOSDK_M5 			0x000000E5 	/* CD-i (as the Green Book standard) */
#define PRIMOSDK_M6 			0x000000E6 	/* PlayStation (Sony games) */
#define PRIMOSDK_F1 			0x000000F1 	/* Obsolete */
#define PRIMOSDK_F2 			0x000000F2 	/* Obsolete for restricted overwrite DVD (DLA DVD-RW) */
#define PRIMOSDK_F3				0x000000F3 	/* Completed (non-appendable) DVD (DVD-ROM or closed recordable) */
#define PRIMOSDK_F4 			0x000000F4 	/* Incremental DVD with appendable zone (DLA DVD-R and DVD+RW) */
#define PRIMOSDK_F8 			0x000000F8 	/* Appendable DVD of any type (single border or multiborder) */
#define PRIMOSDK_FA 			0x000000FA 	/* DVD-RAM cartridge */
#define PRIMOSDK_GENERICCD	 	0x000000C1 	/* Other type of CD. */

// Medium and Unit Values
#define PRIMOSDK_CDROM 		0x00000201  /* CD-ROM */
#define PRIMOSDK_CDR 		0x00000202  /* CD-R   */
#define PRIMOSDK_CDRW 		0x00000203  /* CD-RW */
#define PRIMOSDK_DVDR 		0x00000204 	/* DVD-R */
#define PRIMOSDK_DVDROM	 	0x00000205 	/* DVD-ROM (any type) */
#define PRIMOSDK_DVDRAM		0x00000206 	/* DVD-RAM */
#define PRIMOSDK_DVDRW 		0x00000207 	/* DVD-RW  */
#define PRIMOSDK_DVDPRW	 	0x00000209 	/* DVD+RW */
#define PRIMOSDK_DVDPR 		0x00000210 	/* DVD+R */
#define PRIMOSDK_DDCDROM	0x00000211 	/* Double-density CD-ROM */
#define PRIMOSDK_DDCDR 		0x00000212 	/* Double-density CD-R */
#define PRIMOSDK_DDCDRW		0x00000213 	/* Double-density CD-RW */
#define PRIMOSDK_DVDPR9		0x00000214 	/* dual-layer DVD+R */
#define PRIMOSDK_OTHER 		0x00000220 	/* other types */


int mediumTypeText[] = {IDS_STAMPED_DISC_OR_RECORDABLE_THAT_HAS_BEEN_RECORDED,
						IDS_REWRITEABLE_DISC_HAS_DATA_BUT_KEPT_OPEN_FOR_APPEND,
						IDS_REWRITEABLE_DISC_NOT_POSSIBLE_TO_APPEND_DATA,
						IDS_BLANK_REWRITEABLE_DISC,
					   };

const wchar_t *mediumText[] = {L"CD-ROM", L"CD-R", L"CD-RW", L"DVD-ROM",
							   L"DVD-R",L"DVD-RW", L"DVD+R", L"DVD+RW", L"DVD-RAM",
							   L"DDCD-ROM", L"DDCD-R", L"DDCD-RW", L"DL DVD+R"};

int mediumFormatText[] = { IDS_MEDIA_BLANK_DISC,
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
						   IDS_MEDIA_CD_OTHER_TYPE
						};

DiscInfo::DiscInfo(void)
{
	serialNum = 0;
	memset(buffer, 0, sizeof(buffer));
	strData = NULL;
	ResetData();
}

DiscInfo::DiscInfo(const wchar_t *info)
{
	serialNum = 0;
	memset(buffer, 0, sizeof(buffer));
	strData = NULL;
	SetStringInfo(info);
}


DiscInfo::~DiscInfo(void)
{
	ResetData();
}

void DiscInfo::ResetData(void)
{
	if (strData) free(strData);
	strData = NULL;

	data[DD_PARSED]= FALSE;
	for (int i = 1; i < DISC_DATA_COUNT; i++) data[i] = -1;
}

BOOL DiscInfo::SetStringInfo(const wchar_t *info)
{
	ResetData();

	int strLen = lstrlenW(info) + 1;
	strData = (wchar_t*) malloc(strLen * sizeof(wchar_t));
	StringCchCopyW(strData, strLen, info);

	const wchar_t *start = info;
	const wchar_t *end  = info;
	BOOL cont;
	int i = 1;
	do
	{
		while(end[0] != L';' && end[0] != 0x00) end = CharNextW(end);
		cont = (end[0] == L';');
		
		data[i++] = _wtoi(start);
		
		if(cont)
		{
			end = CharNextW(end);
			start = end;
		}
	}
	while(cont &&  i < DISC_DATA_COUNT);
	
	data[DD_PARSED] = (i == DISC_DATA_COUNT);
	return data[DD_PARSED];
}
const wchar_t* DiscInfo::GetStringInfo(void)
{
	return strData;
}
DWORD DiscInfo::GetMedium(void)
{
	return data[DD_INDEX_MEDIUM];
}
DWORD DiscInfo::GetMediumType(void)
{
	return data[DD_INDEX_MEDIUM_TYPE];
}
DWORD DiscInfo::GetMediumFormat(void)
{
	return data[DD_INDEX_MEDIUM_FORMAT];
}
BOOL DiscInfo::GetProtectedDVD(void)
{
	return data[DD_INDEX_PROTECTED_DVD];
}
BOOL  DiscInfo::GetErasable(void)
{
	return data[DD_INDEX_ERASABLE];
}
DWORD DiscInfo::GetTracksNumber(void)
{
	return data[DD_INDEX_TRACKS_NUM];
}
DWORD DiscInfo::GetSectorsUsed(void)
{
	return data[DD_INDEX_SECTOR_USED];
}
DWORD DiscInfo::GetSectorsFree(void)
{
	return data[DD_INDEX_SECTOR_FREE];
}

BOOL DiscInfo::GetRecordable(void)
{
	return (data[DD_INDEX_MEDIUM_TYPE] == PRIMOSDK_COMPLIANTGOLD || data[DD_INDEX_MEDIUM_TYPE] == PRIMOSDK_BLANK);
}

int DiscInfo::GetSerialNumber(void)
{
	return serialNum;
}

void DiscInfo::SetSerialNumber(int serialNumber)
{
	serialNum = serialNumber;
}

const wchar_t* DiscInfo::GetMediumText(void)
{
	int index = -1;
	switch(data[DD_INDEX_MEDIUM])
	{
		case PRIMOSDK_CDROM:		index = 0; 	break;
		case PRIMOSDK_CDR:		index = 1; 	break;
		case PRIMOSDK_CDRW:		index = 2; 	break;
		case PRIMOSDK_DVDR:		index = 4; 	break;
		case PRIMOSDK_DVDROM:	index = 3; 	break;
		case PRIMOSDK_DVDRAM:	index = 8; 	break;
		case PRIMOSDK_DVDRW:		index = 5; 	break;
		case PRIMOSDK_DVDPRW:	index = 7; 	break;
		case PRIMOSDK_DVDPR:		index = 6; 	break;
		case PRIMOSDK_DDCDROM:	index = 9; 	break;
		case PRIMOSDK_DDCDR:		index = 10;	break;
		case PRIMOSDK_DDCDRW:	index = 11;	break;
		case PRIMOSDK_DVDPR9:	index = 12; 	break;
		default: return TEXT_UNKNOWN;
	}
	return mediumText[index];
}
const wchar_t* DiscInfo::GetMediumTypeText(void)
{
	static wchar_t tmp[256];
	int index = -1;
	switch(data[DD_INDEX_MEDIUM_TYPE])
	{
		case PRIMOSDK_SILVER: index = 0; break;
		case PRIMOSDK_COMPLIANTGOLD: index = 1; break;
		case PRIMOSDK_OTHERGOLD: index = 2; break;
		case PRIMOSDK_BLANK: index = 3; break;
		default: return WASABI_API_LNGSTRINGW_BUF(plugin.hDllInstance,IDS_UNKNOWN,tmp,256);
	}
	return WASABI_API_LNGSTRINGW_BUF(plugin.hDllInstance,mediumTypeText[index],tmp,256);
}
const wchar_t* DiscInfo::GetMediumFormatText(void)
{
	static wchar_t tmpM[256];
	int index = -1;
	switch(data[DD_INDEX_MEDIUM_FORMAT])
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
		default: return WASABI_API_LNGSTRINGW_BUF(plugin.hDllInstance,IDS_UNKNOWN,tmpM,256);
	}
	return WASABI_API_LNGSTRINGW_BUF(plugin.hDllInstance,mediumFormatText[index],tmpM,256);
}
const wchar_t* DiscInfo::GetProtectedDVDText(void)
{
	return data[DD_INDEX_PROTECTED_DVD] ? TEXT_TRUE : TEXT_FALSE ;
}
const wchar_t* DiscInfo::GetErasableText(void)
{
	return data[DD_INDEX_ERASABLE] ? TEXT_TRUE: TEXT_FALSE ;
}
const wchar_t* DiscInfo::GetTracksNumberText(void)
{
	StringCchPrintfW(buffer, TEXT_BUFFER_SIZE, L"%d", data[DD_INDEX_TRACKS_NUM]);
	return buffer;
}
const wchar_t* DiscInfo::GetSectorsUsedText(void)
{
	StringCchPrintfW(buffer, TEXT_BUFFER_SIZE, L"%d", data[DD_INDEX_SECTOR_USED]);
	return buffer;
}
const wchar_t* DiscInfo::GetSectorsFreeText(void)
{
	StringCchPrintfW(buffer, TEXT_BUFFER_SIZE, L"%d", data[DD_INDEX_SECTOR_FREE]);
	return buffer;
}

const wchar_t* DiscInfo::GetRecordableText(void)
{
	return GetRecordable() ? TEXT_TRUE : TEXT_FALSE;
}
