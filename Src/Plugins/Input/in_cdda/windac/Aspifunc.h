// ----------------------------------------------
// -           ASPIFUNC header file             -
// - Written 1996-1998 by Christoph Schmelnik   -
// ----------------------------------------------

// Version 1.40 : 24.02.1998
// Changes:
// function prototype for WaitSCSIRequest extended by immediate parameter

#ifndef _ASPIFUNC_H
#define _ASPIFUNC_H

#ifndef STRICT
#define STRICT               // Enable strict tape checking
#define WIN32_LEAN_AND_MEAN  // Include only needed header files
#endif
#include <windows.h>
#include <stdio.h>
#include <winioctl.h>

#include "winaspi.h"
#include "scsidefs.h"

/*#ifdef DLL
#define DACDLL __declspec(dllexport)
#else
#define DACDLL __declspec(dllimport)
#endif*/
#define DACDLL  

typedef DWORD (__cdecl *VOIDPROC)();
typedef DWORD (__cdecl *SRBPROC)(LPSRB);
typedef BYTE TOpcode[30];

// NT DeviceIO Structures
#define IOCTL_SCSI_BASE                 FILE_DEVICE_CONTROLLER
#define IOCTL_SCSI_GET_CAPABILITIES     CTL_CODE(IOCTL_SCSI_BASE, 0x0404, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SCSI_PASS_THROUGH_DIRECT  CTL_CODE(IOCTL_SCSI_BASE, 0x0405, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

typedef struct _IO_SCSI_CAPABILITIES {
    ULONG Length;
    ULONG MaximumTransferLength;
    ULONG MaximumPhysicalPages;
    ULONG SupportedAsynchronousEvents;
    ULONG AlignmentMask;
    BOOLEAN TaggedQueuing;
    BOOLEAN AdapterScansDown;
    BOOLEAN AdapterUsesPio;
} IO_SCSI_CAPABILITIES, *PIO_SCSI_CAPABILITIES;

typedef struct _SCSI_PASS_THROUGH_DIRECT {
    USHORT Length;
    UCHAR ScsiStatus;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    UCHAR CdbLength;
    UCHAR SenseInfoLength;
    UCHAR DataIn;
    ULONG DataTransferLength;
    ULONG TimeOutValue;
    PVOID DataBuffer;
    ULONG SenseInfoOffset;
    UCHAR Cdb[16];
} SCSI_PASS_THROUGH_DIRECT, *PSCSI_PASS_THROUGH_DIRECT;

typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER {
  SCSI_PASS_THROUGH_DIRECT sptd;
  ULONG Filler;      // realign buffer to double word boundary
  UCHAR ucSenseBuf[32];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;


extern HMODULE hDLL;
extern VOIDPROC GetASPI32SupportInfo;
extern SRBPROC SendASPI32Command;
extern int ASPIInstalled;
extern int RunningNT;
extern int NumberOfHostAdapters;

// base ASPI functions
int DACDLL GetDeviceType(int HostAdapterNumber,int TargetId,int LUN);
BOOL ExecuteSCSIRequest(int HostAdapterNumber,int TargetId,int LUN,int RequestFlags,
	TOpcode OpC, BYTE OpCLen,void *DataPtr, int DataLen,HANDLE hDriveEvent);
void ExecuteSCSIRequest(SRB_ExecSCSICmd &MySRB,HANDLE hDriveEvent);
void FillSCSIRequest(int HostAdapterNumber,int TargetId,int LUN,int RequestFlags,
	TOpcode OpC, BYTE OpCLen,void *DataPtr, int DataLen,SRB_ExecSCSICmd &MySRB,HANDLE hDriveEvent);
BYTE WaitSCSIRequest(SRB_ExecSCSICmd &MySRB,HANDLE hDriveEvent,BOOL bImmediate=FALSE);
BOOL AbortSCSIRequest(SRB_ExecSCSICmd &StuckSRB);
int GetDeviceInfo(int HostAdapterNumber,int TargetId,int LUN,BYTE &SCSIType,char *VendorID,
	char *ProductID,char *ProductRevision,HANDLE hDriveEvent);
BOOL HAInquiry(int HostAdapterNumber,char *ManagerID, char *HAID,THAUnique &HAUnique);
BOOL TestUnitReady(int HostAdapterNumber,int TargetId,int LUN,HANDLE hDriveEvent);
BOOL ModeSense(int HostAdapterNumber,int TargetId,int LUN,TDriveMode &ModeData,int Size,int PageCode,HANDLE hDriveEvent);
BOOL ATAPIModeSense(int HostAdapterNumber,int TargetId,int LUN,TDriveMode &ModeData,int Size,int PageCode,HANDLE hDriveEvent);
BOOL addModeSense(int HostAdapterNumber,int TargetId,int LUN,TDriveMode &ModeData,int Size,HANDLE hDriveEvent);
BOOL ModeSelect(int HostAdapterNumber,int TargetId,int LUN,TDriveMode &ModeData,int Size,HANDLE hDriveEvent);
BOOL addModeSelect(int HostAdapterNumber,int TargetId,int LUN,TDriveMode &ModeData,int Size,HANDLE hDriveEvent);
BOOL SCSIMaxBlocks(HANDLE fh, int *mb);

#endif //_ASPIFUNC_H