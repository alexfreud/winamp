#ifndef  NULLSOFT_PRIMOSDK_INTEGRATION_HEADER
#define  NULLSOFT_PRIMOSDK_INTEGRATION_HEADER

#include "./main.h"
// PrimoSDK wrapper

#define PRIMOSDK_OK					0x0000  - declared in PRIMODSK.H

#define PRIMOSDK_ERROR				0x0001
#define PRIMOSDK_ALREADYLOADED		0x1000
#define PRIMOSDK_LOAD_DLLFAILED		0x1001	
#define PRIMOSDK_LOAD_FUNCFAILED		0x1002
#define PRIMOSDK_NOTLOADED			0x1003
#define PRIMOSDK_FUNCNOTLOADED		0x1004
#define PRIMOSDK_NULLHANDLE			0x1005
#define PRIMOSDK_INTERR 0x1006
#define PRIMOSDK_UNLOCK 0
#define PRIMOSDK_WRITE 0
#include "../primo/obj_primo.h"


// MAXDWORD - for not care
typedef struct _WAMEDIUMINFO
{
	DWORD medium;
	DWORD mediumType;
	DWORD tracks;
	DWORD mediumFormat;
	DWORD freeSectors;
	DWORD usedSectors;
	DWORD protectedDVD;
	DWORD erasable;
	DWORD recordable;
	DWORD isCD;
	DWORD isDCD;
	DWORD isDVD;
	DWORD isDLDVD;
}WAMEDIUMINFO;


typedef struct _WABURNSTRUCT
{
	obj_primo		*primoSDK;	// sdk object
	DWORD		drive;		// working drive (filled by BeginBurn)
	PBYTE		blocker;	// current blocker name  (filled by BeginBurn)
	int			eject;		// if set to TRUE - endBurn will eject disc otherwise just stop motor
} WABURNSTRUCT;  // passed to the BeginBurn or EndBurn functions

BURNLIB_API DWORD GetMediumInfo(obj_primo *primoSDK, DWORD *drive, WAMEDIUMINFO *info);
BURNLIB_API DWORD BeginBurn(obj_primo *primoSDK, DWORD drive, WABURNSTRUCT *burnstruct);
BURNLIB_API DWORD EndBurn(WABURNSTRUCT *burnstruct);

#endif // NULLSOFT_PRIMOSDK_INTEGRATION_HEADER