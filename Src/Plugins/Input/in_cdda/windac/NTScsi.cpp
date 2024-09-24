#include <stdio.h>
#include <stddef.h>
#include "NTScsi.h"

typedef struct {
	BYTE ha;
	BYTE tgt;
	BYTE lun;
	BYTE driveLetter;
	BOOL bUsed;
	HANDLE hDevice;
	BYTE inqData[36];
} NTSCSIDRIVE;

typedef struct
{
	BYTE		numAdapters;
	NTSCSIDRIVE	drive[26];
} NTSCSIDRIVES;

void GetDriveInformation( BYTE i, NTSCSIDRIVE *pDrive );

static HANDLE GetFileHandle( BYTE i );

static BOOL bNtScsiAvailable = FALSE;
static NTSCSIDRIVES NtScsiDrives;
static BOOL bUseNtScsi = FALSE;

/*
 * Initialization of SCSI Pass Through Interface code.  Responsible for
 * setting up the array of SCSI devices.  This code will be a little
 * different from the normal code -- it will query each drive letter from
 * C: through Z: to see if it is  a CD.  When we identify a CD, we then 
 * send CDB with the INQUIRY command to it -- NT will automagically fill in
 * the PathId, TargetId, and Lun for us.
 */

int NtScsiInit( void )
{
	BYTE i;
	wchar_t buf[4] = {0};
	UINT uDriveType;
	int retVal = 0;

	if ( bNtScsiAvailable )
	{
		for( i = 2; i < 26; i++ ) if ( NtScsiDrives.drive[i].bUsed ) retVal++;
		bUseNtScsi = (retVal > 0 );
		return retVal;
	}

	memset( &NtScsiDrives, 0x00, sizeof(NtScsiDrives) );

	for( i = 0; i < 26; i++ )
	{
		NtScsiDrives.drive[i].hDevice = INVALID_HANDLE_VALUE;
	}

	for( i = 2; i < 26; i++ )
	{
		wsprintf( buf, L"%c:\\", (wchar_t)('A'+i) );
		uDriveType = GetDriveType( buf );

		/* check if this is a CDROM drive */
		if ( uDriveType == DRIVE_CDROM )
		{
			GetDriveInformation( i, &NtScsiDrives.drive[i] );

			if ( NtScsiDrives.drive[i].bUsed )
				retVal++;
		}
	}

	NtScsiDrives.numAdapters = NtScsiGetNumAdapters( );

	bNtScsiAvailable = TRUE;

	if ( retVal > 0 )
	{
		bUseNtScsi = TRUE;
	}

	return retVal;
}


int NtScsiDeInit( void )
{
	BYTE i;

	if ( !bNtScsiAvailable )
		return 0;

	for( i = 2; i < 26; i++ )
	{
		if ( NtScsiDrives.drive[i].bUsed )
		{
			CloseHandle( NtScsiDrives.drive[i].hDevice );
		}
	}

	NtScsiDrives.numAdapters = NtScsiGetNumAdapters( );

	ZeroMemory( &NtScsiDrives, sizeof(NtScsiDrives) );
	bNtScsiAvailable = FALSE;
	return -1;
}


/*
 * Returns the number of "adapters" present. 
 */
BYTE NtScsiGetNumAdapters( void )
{
	BYTE buf[256] = {0};
	WORD i;
	BYTE numAdapters = 0;

	// PortNumber 0 should exist, so pre-mark it.  This avoids problems
	// when the primary IDE drives are on PortNumber 0, but can't be opened
	// because of insufficient privelege (ie. non-admin).
	buf[0] = 1;

	for( i = 0; i < 26; i++ )
	{
		if ( NtScsiDrives.drive[i].bUsed )
			buf[NtScsiDrives.drive[i].ha] = 1;
	}

	for( i = 0; i <= 255; i++ )
	{
		if ( buf[i] )
		numAdapters++;
	}

	return numAdapters;
}


/*
 * Replacement for GetASPI32SupportInfo from wnaspi32.dll
 */
DWORD NtScsiGetASPI32SupportInfo( void )
{
	DWORD retVal;


	if ( !NtScsiDrives.numAdapters )
	    retVal = (DWORD)(MAKEWORD(0,SS_NO_ADAPTERS));
	else
		retVal = (DWORD)(MAKEWORD(NtScsiDrives.numAdapters,SS_COMP));

	return retVal;
}

/*
 * Needs to call the appropriate function for the lpsrb->SRB_Cmd specified.
 * Valid types are SC_HA_INQUIRY, SC_GET_DEV_TYPE, SC_EXEC_SCSI_CMD,
 * and SC_RESET_DEV.
 */
DWORD NtScsiSendASPI32Command( LPSRB lpsrb )
{
	if ( !lpsrb )
		return SS_ERR;

	switch( lpsrb->SRB_Cmd )
    {
		case SC_HA_INQUIRY:
			return NtScsiHandleHaInquiry( (LPSRB_HAINQUIRY)lpsrb );
		break;

		case SC_GET_DEV_TYPE:
			return NtScsiGetDeviceType( (LPSRB_GDEVBLOCK)lpsrb );
		break;

		case SC_EXEC_SCSI_CMD:
		  return NtScsiExecSCSICommand( (LPSRB_EXECSCSICMD)lpsrb, FALSE );
		break;

		case SC_RESET_DEV:
		default:
		  lpsrb->SRB_Status = SS_ERR;
		  return SS_ERR;
		break;
    }

	return SS_ERR;  // should never get to here...
}


/*
 * Universal function to get a file handle to the CD device.  Since
 * NT 4.0 wants just the GENERIC_READ flag, and Win2K wants both
 * GENERIC_READ and GENERIC_WRITE (why a read-only CD device needs
 * GENERIC_WRITE access is beyond me...), the easist workaround is to just
 * try them both.
 */
static HANDLE GetFileHandle( BYTE i )
{
	wchar_t			buf[12] = {0};
	HANDLE			fh = NULL;
	OSVERSIONINFO	osver;
	DWORD			dwFlags;

	memset( &osver, 0x00, sizeof(osver) );
	osver.dwOSVersionInfoSize = sizeof(osver);
	GetVersionEx( &osver );

	// if Win2K or greater, add GENERIC_WRITE
	dwFlags = GENERIC_READ;

	if ( (osver.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osver.dwMajorVersion > 4) )
	{
      dwFlags |= GENERIC_WRITE;
	}

	wsprintf( buf, L"\\\\.\\%c:", (wchar_t)('A'+i) );
	fh = CreateFile( buf, dwFlags, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,OPEN_EXISTING, 0, NULL );

	if ( fh == INVALID_HANDLE_VALUE )
	{
		// it went foobar somewhere, so try it with the GENERIC_WRITE bit flipped
		dwFlags ^= GENERIC_WRITE;
		fh = CreateFile( buf, dwFlags, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );
    }

	if ( fh == INVALID_HANDLE_VALUE )
	{
	}
	else
	{
	}

	return fh;
}



/*
 * fills in a pDrive structure with information from a SCSI_INQUIRY
 * and obtains the ha:tgt:lun values via IOCTL_SCSI_GET_ADDRESS
 */
void GetDriveInformation( BYTE i, NTSCSIDRIVE *pDrive )
{
	HANDLE fh;
	char buf[2048] = {0};
	BOOL status;
	PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER pswb;
	PSCSI_ADDRESS pscsiAddr;
	ULONG length, returned;
	BYTE inqData[100] = {0};

	fh = GetFileHandle( i );

	if ( fh == INVALID_HANDLE_VALUE )
	{
      return;
    }

  /*
   * Get the drive inquiry data
   */
  ZeroMemory( &buf, 2048 );
  ZeroMemory( inqData, 100 );
  pswb                      = (PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER)buf;
  pswb->spt.Length          = sizeof(SCSI_PASS_THROUGH_DIRECT);
  pswb->spt.CdbLength       = 6;
  pswb->spt.SenseInfoLength = 24;
  pswb->spt.DataIn          = SCSI_IOCTL_DATA_IN;
  pswb->spt.DataTransferLength = 100;
  pswb->spt.TimeOutValue    = 2;
  pswb->spt.DataBuffer      = inqData;
  pswb->spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf );
  pswb->spt.Cdb[0]          = 0x12;
  pswb->spt.Cdb[4]          = 100;

  length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
  status = DeviceIoControl( fh,
			    IOCTL_SCSI_PASS_THROUGH_DIRECT,
			    pswb,
			    length,
			    pswb,
			    length,
			    &returned,
			    NULL );

  if ( !status )
    {
      CloseHandle( fh );
      return;
    }

  memcpy( pDrive->inqData, inqData, 36 );

  /*
   * get the address (path/tgt/lun) of the drive via IOCTL_SCSI_GET_ADDRESS
   */
  ZeroMemory( &buf, 2048 );
  pscsiAddr = (PSCSI_ADDRESS)buf;
  pscsiAddr->Length = sizeof(SCSI_ADDRESS);
  if ( DeviceIoControl( fh, IOCTL_SCSI_GET_ADDRESS, NULL, 0,
			pscsiAddr, sizeof(buf), &returned,
			NULL ) )
    {
      pDrive->bUsed     = TRUE;
      pDrive->ha        = pscsiAddr->PortNumber;
      pDrive->tgt       = pscsiAddr->TargetId;
      pDrive->lun       = pscsiAddr->Lun;
      pDrive->driveLetter = i;
      pDrive->hDevice   = INVALID_HANDLE_VALUE;
    }
  else if (50 == GetLastError()) // usb/firewire
  {
		pDrive->bUsed		= TRUE;
		pDrive->ha			= i;
		pDrive->tgt			= 0;
		pDrive->lun			= 0;
		pDrive->driveLetter	= i;
		pDrive->hDevice		= INVALID_HANDLE_VALUE;
  }
  else 
    {
      pDrive->bUsed     = FALSE;
    }

  CloseHandle( fh );
}



DWORD NtScsiHandleHaInquiry( LPSRB_HAINQUIRY lpsrb )
{
  DWORD *pMTL;

  lpsrb->HA_Count    = NtScsiDrives.numAdapters;

  if ( lpsrb->SRB_HaId >= NtScsiDrives.numAdapters )
    {
      lpsrb->SRB_Status = SS_INVALID_HA;
      return SS_INVALID_HA;
    }
  lpsrb->HA_SCSI_ID  = 7;  // who cares... we're not really an ASPI manager
  memcpy( lpsrb->HA_ManagerId,  "blahblahblahblah", 16 );
  memcpy( lpsrb->HA_Identifier, "blahblahblahblah", 16 );
  lpsrb->HA_Identifier[13] = (char)('0'+lpsrb->SRB_HaId);
  ZeroMemory( lpsrb->HA_Unique, 16 );
  lpsrb->HA_Unique[3] = 8;
  pMTL = (LPDWORD)&lpsrb->HA_Unique[4];
  *pMTL = 64 * 1024;

  lpsrb->SRB_Status = SS_COMP;
  return SS_COMP;
}


/*
 * Scans through the drive array and returns DTYPE_CDROM type for all items
 * found, and DTYPE_UNKNOWN for all others.
 */
DWORD NtScsiGetDeviceType( LPSRB_GDEVBLOCK lpsrb )
{
  lpsrb->SRB_Status = SS_NO_DEVICE;
  if ( NtScsiGetDeviceIndex( lpsrb->SRB_HaId, lpsrb->SRB_Target, lpsrb->SRB_Lun ) )
    lpsrb->SRB_Status = SS_COMP;

  if ( lpsrb->SRB_Status == SS_COMP )
    lpsrb->SRB_DeviceType = DTC_CDROM;
  else
    lpsrb->SRB_DeviceType = DTC_UNKNOWN;

  return lpsrb->SRB_Status;
}


/*
 * Looks up the index in the drive array for a given ha:tgt:lun triple
 */
BYTE NtScsiGetDeviceIndex( BYTE ha, BYTE tgt, BYTE lun )
{
	BYTE i;

	for( i = 2; i < 26; i++ )
    {
		if ( NtScsiDrives.drive[i].bUsed )
		{
			NTSCSIDRIVE *lpd;
			lpd = &NtScsiDrives.drive[i];
			if ( (lpd->ha == ha) && (lpd->tgt == tgt) && (lpd->lun == lun) )
				return i;
		}
    }
	return 0;
}

/*
 * Converts ASPI-style SRB to SCSI Pass Through IOCTL
 */
DWORD NtScsiExecSCSICommand( LPSRB_EXECSCSICMD lpsrb, BOOL bBeenHereBefore )
{
  BOOL status;
  SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER swb;
  ULONG length, returned;
  BYTE idx;

  idx = NtScsiGetDeviceIndex( lpsrb->SRB_HaId, lpsrb->SRB_Target, lpsrb->SRB_Lun );

  if ( idx == 0 )
    {
      lpsrb->SRB_Status = SS_ERR;
      return SS_ERR;
    }

  if ( lpsrb->CDBByte[0] == 0x12 ) // is it an INQUIRY?
    {
      lpsrb->SRB_Status = SS_COMP;
      memcpy( lpsrb->SRB_BufPointer, NtScsiDrives.drive[idx].inqData, 36 );
      return SS_COMP;
    }

  if ( NtScsiDrives.drive[idx].hDevice == INVALID_HANDLE_VALUE )
    NtScsiDrives.drive[idx].hDevice = GetFileHandle( NtScsiDrives.drive[idx].driveLetter );

  ZeroMemory( &swb, sizeof(swb) );
  swb.spt.Length            = sizeof(SCSI_PASS_THROUGH);
  swb.spt.CdbLength         = lpsrb->SRB_CDBLen;
  if ( lpsrb->SRB_Flags & SRB_DIR_IN )
    swb.spt.DataIn          = SCSI_IOCTL_DATA_IN;
  else if ( lpsrb->SRB_Flags & SRB_DIR_OUT )
    swb.spt.DataIn          = SCSI_IOCTL_DATA_OUT;
  else
    swb.spt.DataIn          = SCSI_IOCTL_DATA_UNSPECIFIED;
  swb.spt.DataTransferLength = lpsrb->SRB_BufLen;
  swb.spt.TimeOutValue      = 5;
  swb.spt.DataBuffer        = lpsrb->SRB_BufPointer;
  swb.spt.SenseInfoOffset   =
    offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf );
  memcpy( swb.spt.Cdb, lpsrb->CDBByte, lpsrb->SRB_CDBLen );
  length = sizeof(swb);

  status = DeviceIoControl( NtScsiDrives.drive[idx].hDevice,
			    IOCTL_SCSI_PASS_THROUGH_DIRECT,
			    &swb,
			    length,
			    &swb,
			    length,
			    &returned,
			    NULL );

  if ( status )
    {
      lpsrb->SRB_Status = SS_COMP;
    }
  else
    {
      DWORD dwErrCode;

      lpsrb->SRB_Status = SS_ERR;
      lpsrb->SRB_TargStat = 0x0004;
      dwErrCode = GetLastError();
      /*
       * KLUDGE ALERT! KLUDGE ALERT! KLUDGE ALERT!
       * Whenever a disk changer switches disks, it may render the device
       * handle invalid.  We try to catch these errors here and recover
       * from them.
       */
      if ( !bBeenHereBefore &&
	   ((dwErrCode == ERROR_MEDIA_CHANGED) || (dwErrCode == ERROR_INVALID_HANDLE)) )
	{
	  if ( dwErrCode != ERROR_INVALID_HANDLE )
	    CloseHandle( NtScsiDrives.drive[idx].hDevice );
	  GetDriveInformation( idx, &NtScsiDrives.drive[idx] );

	  return NtScsiExecSCSICommand( lpsrb, TRUE );
	}
    }

  return lpsrb->SRB_Status;
}



BOOL UsingSCSIPT( void )
{
  return bUseNtScsi;
}



/*
 * Calls GetFileHandle for the CD refered to by ha:tgt:lun to open it for
 * use
 */
void NtScsiOpenCDHandle( BYTE ha, BYTE tgt, BYTE lun )
{
  BYTE idx;

  idx = NtScsiGetDeviceIndex( ha, tgt, lun );

  if ( idx && NtScsiDrives.drive[idx].hDevice == INVALID_HANDLE_VALUE )
    NtScsiDrives.drive[idx].hDevice = GetFileHandle( NtScsiDrives.drive[idx].driveLetter );  
}