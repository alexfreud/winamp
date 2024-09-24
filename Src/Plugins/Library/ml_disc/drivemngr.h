#ifndef NULLSOFT_MLDISC_DRIVEMANAGER_HEADER
#define NULLSOFT_MLDISC_DRIVEMANAGER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>
//#include "../primo/obj_primo.h"

// drive types

#define DRIVE_TYPE_UNKNOWN		0x00000000
#define DRIVE_TYPE_CD			0x00000010
#define DRIVE_TYPE_DVD			0x00000020
#define DRIVE_CAP_UNKNOWN		0x80000000
#define DRIVE_CAP_R				0x00010000
#define DRIVE_CAP_RW				0x00020000
#define DRIVE_CDR			(DRIVE_TYPE_CD | DRIVE_CAP_R )
#define DRIVE_CDRW			(DRIVE_TYPE_CD | DRIVE_CAP_RW)
#define DRIVE_DVDR			(DRIVE_TYPE_DVD | DRIVE_CAP_R)
#define DRIVE_DVDRW			(DRIVE_TYPE_DVD | DRIVE_CAP_RW)

#define DMW_DRIVEADDED			0x0001	// param contains drive letter
#define DMW_DRIVEREMOVED			0x0002	// param contains drive letter
#define DMW_DRIVECHANGED			0x0003  // param contains drive letter
#define DMW_MEDIUMARRIVED		0x0004	// param contains drive letter
#define DMW_MEDIUMREMOVED		0x0005	// param contains drive letter
#define DMW_OPCOMPLETED			0x0006	// one of the async opetations completed

#define DMW_MODECHANGED			0x0010	// LOWORD(param) = MAKEWORD(cLetter, cMode) 

typedef void* HDRVMNGR;

typedef struct _DM_NOTIFY_PARAM DM_NOTIFY_PARAM;

typedef void (CALLBACK *DMNPROC)(WORD/*wCode*/, INT_PTR/*param*/);
typedef void (CALLBACK *DMFREEPROC)(DM_NOTIFY_PARAM *phdr);


#define DM_EJECT_REMOVE	0
#define DM_EJECT_LOAD	1
#define DM_EJECT_CHANGE	2

// valid with DM_UNITINFO_PARAM
#define DMF_DESCRIPTION		0x00000001
#define DMF_FIRMWARE		0x00000002
#define DMF_READY			0x00000004

// valid with DM_UNITINFO2_PARAM
#define DMF_TYPES			0x00000001

// valid with DM_DISCINFOEX_PARAM
#define DMF_DRIVEMODE_DAO	0x00000000
#define DMF_DRIVEMODE_TAO	0x00010000
#define DMF_MEDIUMTYPE		0x00000001
#define DMF_MEDIUMFORMAT	0x00000002
#define DMF_TRACKS			0x00000004
#define DMF_USED			0x00000008
#define DMF_FREE			0x00000010

// valid with DM_DISCINFO2_PARAM
#define DMF_MEDIUM			0x00000001
#define DMF_PROTECTEDDVD	0x00000002
#define DMF_PACKETWRITTEN	0x00000004
#define DMF_MEDIUMEX		0x00000008

// valid with DM_FANCYTITLE_PARAM
#define DMF_VOLUMELABEL		0x00000001	// volume label 
#define DMF_CDTEXT			0x00000002	// if set and medium inserted will try to get info from cdtext
#define DMF_CDDB			0x00000004  // if set and medium inserted will try to get info from gracenote
#define DMF_DRIVEDESCRIPTION	0x00000010	// will use PrimoSDK to get drive info 

// valid with DM_MCI_PARAM
#define DMF_READY			0x00000004	// 
#define DMF_MEDIUMPRESENT	0x00000002	// 
#define DMF_MODE			0x00000001  // 
#define DMF_TRACKCOUNT		0x00000008	// 
#define DMF_TRACKSINFO		0x00000010	// 
#define DMF_MEDIUMUID		0x00000020	// 
#define DMF_MEDIUMUPC		0x00000040	// 

// valid with DM_IMAPI_PARAM
#define DMF_BASEPNPID		0x00000001
#define DMF_DISPLAYNAMES	0x00000002
#define DMF_PATH			0x00000004
#define DMF_DRIVESTATE		0x00000008
#define DMF_DRIVETYPE		0x00000010
#define DMF_QUERYMEDIATYPE	0x00000020
#define DMF_QUERYMEDIAINFO	0x00000040


// Operation Codes
#define DMOP_GENERAL		0x0000
#define DMOP_UNITINFO		0x0001
#define DMOP_UNITINFO2		0x0002
#define DMOP_DISCINFO		0x0003
#define DMOP_DISCINFO2		0x0004
#define DMOP_TITLE			0x0005
#define DMOP_MCIINFO		0x0006
#define DMOP_IMAPIINFO		0x0007

// Drive modes
#define DM_MODE_ERROR		((CHAR)(0 - 1))

#define DM_MODE_READY		0x00
#define DM_MODE_BURNING		0x01
#define DM_MODE_RIPPING		0x02
#define DM_MODE_COPYING		0x03


typedef struct _DM_NOTIFY_PARAM
{
	INT_PTR		callback;	// pointer to the callback. If uMsg != 0 callback is HWND, otherwise it is DMNPROC
	UINT		uMsg;		// specify message code to post notification. if 0 callback points to DMNPROC.
	CHAR		cLetter;	// drive letter.
	UINT		fFlags;		// DMF_XXX
	DWORD		result;		// result code. Set by async func.
	WORD		opCode;		// completed opCode (DMOP_XXX). Set by async func.
	DMFREEPROC	fnFree;		// you can specify function that need to be called to free data
	HANDLE		hReserved;	// reserved;
} DM_NOTIFY_PARAM;


typedef struct _DM_UNITINFO_PARAM 
{
	DM_NOTIFY_PARAM	header;
    DWORD		dwType;		// unit type
	BOOL		bReady;		// unit ready flag
	LPSTR		pszDesc;	// pointer to the buffer with unit description.
	INT			cchDesc;	// [in] length of the decription buffer in chars. [out] number of characters written. If error value is negative and show minimum required buffer
	LPSTR		pszFirmware;	// pointer to the buffer with FirmWare ( firmware version is always 4 chars)
	INT			cchFirmware;	// [in] length of the firmware buffer in chars. [out] number of characters written. If error value is negative and show minimum required buffer
} DM_UNITINFO_PARAM;

typedef struct _DM_UNITINFO2_PARAM 
{
	DM_NOTIFY_PARAM	header;
	DWORD		*pdwTypes;	// unit types vector
	INT			nTypes;		// vector length (in DWORDS)
	DWORD		dwClassId;	// class identifier assigned to the unit.
	DWORD		dwBusType;	// type of bus to which the device is connected.
} DM_UNITINFO2_PARAM;


typedef struct _DM_DISCINFOEX_PARAM 
{
	DM_NOTIFY_PARAM	header;
	DWORD		dwMediumType;	// type of the medium.
	DWORD		dwMediumFormat;	// format of the media
	BOOL		bErasable;		// 
	DWORD		dwTracks;		// number of tracks in the disc.
	DWORD		dwUsed;			// total number of sectors used on the disc.
	DWORD		dwFree;			// total number of free sectors on the disc.

} DM_DISCINFOEX_PARAM;

typedef struct _DM_DISCINFO2_PARAM 
{
	DM_NOTIFY_PARAM	header;
	DWORD		dwMedium;		// physical type of the media.
	BOOL		bProtectedDVD;	// DVD containing copy-protected content.
	BOOL		bPacketWritten;	// if the media is formatted by packet writing software.
	DWORD		dwMediumEx;		// physical type of the medium.
} DM_DISCINFO2_PARAM;


typedef struct _DM_TITLE_PARAM
{
	DM_NOTIFY_PARAM	header;
	LPWSTR			pszTitle;
	INT				cchTitle;

} DM_TITLE_PARAM;

typedef struct _DM_MCI_PARAM
{
	DM_NOTIFY_PARAM header;
	BOOL			bReady;	
	BOOL			bMediumPresent;
	UINT			uMode;
	DWORD*			pTracks;	// this contains track info (first bit set to '1' if track is audio, other bits track length
	INT				nTracks;
	LPWSTR			pszMediumUID;
	INT				cchMediumUID;
	LPWSTR			pszMediumUPC;
	INT				cchMediumUPC;
} DM_MCI_PARAM;

// you responsible for freeing all bstrs (use SysFreeString())
typedef struct _DM_IMAPI_PARAM
{
	DM_NOTIFY_PARAM header;		// header result contains HRESULT
	BOOL	bRecorder;			// Set to TRUE if IMAPI fond drive
	BSTR	bstrBasePnPID;		// DMF_BASEPNPID
	BSTR	bstrVendorID;		// DMF_DISPLAYNAMES	
	BSTR	bstrProductID;		// DMF_DISPLAYNAMES	
	BSTR	bstrRevision;		// DMF_DISPLAYNAMES	
	BSTR	bstrPath;			// DMF_PATH
	ULONG	ulDriveState;		// DMF_DRIVESTATE
	LONG	fDriveType;			// DMF_DRIVETYPE
	LONG	fMediaType;			// DMF_QUERYMEDIATYPE
	LONG	fMediaFlags;		// DMF_QUERYMEDIATYPE
	BYTE	bSessions;			// DMF_QUERYMEDIAINFO
	BYTE	bLastTrack;			// DMF_QUERYMEDIAINFO
	ULONG	ulStartAddress;		// DMF_QUERYMEDIAINFO
	ULONG	ulNextWritable;		// DMF_QUERYMEDIAINFO
	ULONG	ulFreeBlocks;		// DMF_QUERYMEDIAINFO

} DM_IMAPI_PARAM;

BOOL DriveManager_Initialize(DMNPROC DMNProc, BOOL bSuspended);
BOOL DriveManager_Uninitialize(INT msExitWaitTime);
BOOL DriveManager_Suspend(void);
BOOL DriveManager_Resume(BOOL bUpdate);
INT DriveManager_GetDriveList(CHAR *pLetters, INT cchSize);
BOOL DriveManager_Update(BOOL bAsync);		// check all drives and discs
BOOL DriveManager_SetDriveMode(CHAR cLetter, CHAR cMode); 
CHAR DriveManager_GetDriveMode(CHAR cLetter);
DWORD DriveManager_GetDriveType(CHAR cLetter);
BOOL DriveManager_Eject(CHAR cLetter, INT nCmd); // nCmd = DM_EJECT_XXX
BOOL DriveManager_IsUnitReady(BOOL *pbReady);
BOOL DriveManager_IsMediumInserted(CHAR cLetter);
// PrimoSDK async calls
BOOL DriveManager_GetUnitInfo(DM_UNITINFO_PARAM *puip);
BOOL DriveManager_GetUnitInfo2(DM_UNITINFO2_PARAM *puip);
BOOL DriveManager_GetDiscInfoEx(DM_DISCINFOEX_PARAM *pdip);
BOOL DriveManager_GetDiscInfo2(DM_DISCINFO2_PARAM *pdip);

BOOL DriveManager_QueryTitle(DM_TITLE_PARAM *pdtp);

// MCI async
BOOL DriveManager_GetMCIInfo(DM_MCI_PARAM *pmcip);

//IMAPI async
BOOL DriveManager_GetIMAPIInfo(DM_IMAPI_PARAM *pIMAPI);


#endif //NULLSOFT_MLDISC_DRIVEMANAGER_HEADER