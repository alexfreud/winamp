#ifndef NULLOSFT_MLDISC_STPI_HEADER
#define NULLOSFT_MLDISC_STPI_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>


// 02:04:01 - LOGICAL UNIT IS IN PROCESS OF BECOMING READY
// 02:3A:00 - MEDIUM NOT PRESENT
// 02:3A:01 - MEDIUM NOT PRESENT - TRAY CLOSED
// 02:3A:02 - MEDIUM NOT PRESENT - TRAY OPEN
// 06:28:00 - MEDIUM NOT PRESENT - NOT READY TO READY CHANGE, MEDIUM MAY HAVE CHANGED

typedef struct _SENSEINFO
{
	BYTE		bSense;
	BYTE		bASC;
	BYTE		bASCQ;
} SENSEINFO;
BOOL SPTI_TestUnitReady(HANDLE hDevice, BYTE *pbSC, BYTE *pbASC, BYTE *pbASCQ, INT timeOutSec); 
BOOL SPTI_StartStopUnit(HANDLE hDevice, BOOL bImmediate, BOOL bLoadEject, BOOL bStart, INT timeOutSec, SENSEINFO *pSense);
BOOL SPTI_GetCapabilities(HANDLE hDevice, DWORD *pCap);

// links to read:
// http://www.t10.org/ftp/t10/drafts/mmc3/mmc3r10g.pdf
// http://www.reactos.org/generated/doxygen/da/db8/scsi_8h-source.html
// http://www.vbarchiv.net/workshop/workshop78s4.html
// http://files.codes-sources.com/fichier.aspx?id=26141&f=Graver.h
// http://files.codes-sources.com/fichier.aspx?id=26141&f=Graver.cpp
// http://www.hackingtruths.org/files/cd_cracking/SRC/etc/DDK.SPTI/spti.c 

#endif //NULLOSFT_MLDISC_STPI_HEADER