// ----------------------------------------------
// -       ASPIFUNC implementation file         -
// - Written 1996-1998 by Christoph Schmelnik   -
// ----------------------------------------------

// Version 1.40 : 24.02.1998
// Changes:
// Set correct direction flags, to work with NT device IO interface and ATAPI drives
// Added Immediate paremeter for WaitSCSIRequest to allow detection of buffer underruns

#include <stddef.h>
#include "aspifunc.h"

HMODULE hDLL=0;
VOIDPROC GetASPI32SupportInfo;
SRBPROC SendASPI32Command;
int ASPIInstalled;
int RunningNT;
int NumberOfHostAdapters;

//Implementation of base ASPI Functions
BOOL HAInquiry(int HostAdapterNumber,char *ManagerID, char *HAID,THAUnique &HAUnique)
{
    SRB_HAInquiry MySRB;
    DWORD ASPI_Status;
	memset(&MySRB,0,sizeof(SRB_HAInquiry));
    MySRB.SRB_Cmd      = SC_HA_INQUIRY;
    MySRB.SRB_HaId     = HostAdapterNumber;
    MySRB.SRB_Flags    = 0;
    MySRB.SRB_Hdr_Rsvd = 0;
    ASPI_Status = SendASPI32Command ( (LPSRB) &MySRB );
	if (ASPI_Status!=SS_COMP)
		return FALSE;
	HAUnique=MySRB.HA_Unique;
	for (int i=0; i<16; i++)
	{
		ManagerID[i]=MySRB.HA_ManagerId[i];
		HAID[i]=MySRB.HA_Identifier[i];
	}
	ManagerID[16]=0;
	HAID[16]=0;
	return TRUE;
}

int GetDeviceType(int HostAdapterNumber,int TargetId,int LUN)
{
	SRB_GDEVBlock MySRB;
	DWORD ASPI_Status;
	memset(&MySRB,0,sizeof(SRB_GDEVBlock));
	MySRB.SRB_Cmd					= SC_GET_DEV_TYPE;
	MySRB.SRB_HaId					= HostAdapterNumber;
	MySRB.SRB_Flags					= 0;
	MySRB.SRB_Hdr_Rsvd				= 0;
	MySRB.SRB_Target				= TargetId;
	MySRB.SRB_Lun					= LUN;
	ASPI_Status = SendASPI32Command ((LPSRB)&MySRB);
	/***************************************************/
	/* If ASPI_Status == SS_COMP, MySRB.SRB_DeviceType */
	/* will contain the peripheral device type.        */
	/***************************************************/
	if (ASPI_Status==SS_COMP)
		return MySRB.SRB_DeviceType;
	return DTYPE_UNKNOWN;
}

BOOL ExecuteSCSIRequest(int HostAdapterNumber,int TargetId,int LUN,int RequestFlags,
	TOpcode OpC, BYTE OpCLen,void *DataPtr, int DataLen, HANDLE hDriveEvent)
{
	if ((HostAdapterNumber>=NumberOfHostAdapters) &&
		RunningNT)
	{
		DWORD il, ol;
		SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sb;
		ZeroMemory(&sb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
		sb.sptd.Length             = sizeof(SCSI_PASS_THROUGH_DIRECT);
		sb.sptd.CdbLength          = OpCLen;
		sb.sptd.DataIn             = ((SRB_DIR_IN & RequestFlags) ? 1/*SCSI_IOCTL_DATA_IN*/ : 0/*SCSI_IOCTL_DATA_OUT*/);
		sb.sptd.SenseInfoLength    = 32;
		sb.sptd.DataTransferLength = DataLen;
		sb.sptd.TimeOutValue       = 2;
		sb.sptd.DataBuffer         = (unsigned char*) DataPtr;
		sb.sptd.SenseInfoOffset    = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf);
		for (int i=0; i<OpCLen; i++) sb.sptd.Cdb[i]=OpC[i];
		il = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
		if (DeviceIoControl(hDriveEvent, IOCTL_SCSI_PASS_THROUGH_DIRECT, &sb, il, &sb, il, &ol, FALSE)) 
		{
			if (sb.sptd.ScsiStatus==0)
				return TRUE;
		}
		return FALSE;
	}

    
	SRB_ExecSCSICmd MySRB;
	DWORD ASPI_Status;
    DWORD ASPIEventStatus;
	memset(&MySRB,0,sizeof(SRB_ExecSCSICmd));
	MySRB.SRB_Cmd					= SC_EXEC_SCSI_CMD;
	MySRB.SRB_HaId					= HostAdapterNumber;
	MySRB.SRB_Flags					= RequestFlags|SRB_EVENT_NOTIFY;
	MySRB.SRB_Hdr_Rsvd				= 0;
	MySRB.SRB_Target				= TargetId;
	MySRB.SRB_Lun					= LUN;
	MySRB.SRB_BufPointer			= (unsigned char*) DataPtr;
	MySRB.SRB_BufLen				= DataLen;
	MySRB.SRB_CDBLen				= OpCLen;
	MySRB.SRB_SenseLen				= SENSE_LEN;
    MySRB.SRB_PostProc   			= (void(__cdecl *)(void))hDriveEvent;
	for (int i=0; i<OpCLen; i++) MySRB.CDBByte[i]=OpC[i];

	ResetEvent(hDriveEvent);
	ASPI_Status = SendASPI32Command ((LPSRB)&MySRB);

    /**************************************************/
    /* Block on event till signaled                   */
    /**************************************************/

    if ( MySRB.SRB_Status == SS_PENDING )
    {
    	ASPIEventStatus = WaitForSingleObject(hDriveEvent, TIMEOUT);

    	/**************************************************/
    	/* Reset event to non-signaled state.             */
    	/**************************************************/
    	if (ASPIEventStatus == WAIT_OBJECT_0)
    		ResetEvent(hDriveEvent);
		else
		{
			OutputDebugString(L"Execute Timed out\n");
			AbortSCSIRequest(MySRB);
		}
	}
	
	if (MySRB.SRB_Status==SS_COMP)
		return TRUE;

	return FALSE;
}

void FillSCSIRequest(int HostAdapterNumber,int TargetId,int LUN,int RequestFlags,
	TOpcode OpC, BYTE OpCLen,void *DataPtr, int DataLen,SRB_ExecSCSICmd &MySRB, HANDLE hDriveEvent)
{
	memset(&MySRB,0,sizeof(SRB_ExecSCSICmd));
	MySRB.SRB_Cmd					= SC_EXEC_SCSI_CMD;
	MySRB.SRB_HaId					= HostAdapterNumber;
	if ((HostAdapterNumber>=NumberOfHostAdapters) &&
		RunningNT)
	{
		MySRB.SRB_Flags				= RequestFlags;
	}
	else
	{
		MySRB.SRB_Flags				= RequestFlags|SRB_EVENT_NOTIFY;
		MySRB.SRB_Hdr_Rsvd			= 0;
		MySRB.SRB_PostProc   		= (void(__cdecl *)(void))hDriveEvent;
	}
	MySRB.SRB_Target				= TargetId;
	MySRB.SRB_Lun					= LUN;
	MySRB.SRB_BufPointer			= (unsigned char*) DataPtr;
	MySRB.SRB_BufLen				= DataLen;
	MySRB.SRB_CDBLen				= OpCLen;
	MySRB.SRB_SenseLen				= SENSE_LEN;
	for (int i=0; i<OpCLen; i++) MySRB.CDBByte[i]=OpC[i];
}

void ExecuteSCSIRequest(SRB_ExecSCSICmd &MySRB,HANDLE hDriveEvent)
{
	if ((MySRB.SRB_HaId>=NumberOfHostAdapters) &&
		RunningNT)
	{
		DWORD il, ol;
		SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sb;
		ZeroMemory(&sb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
		sb.sptd.Length             = sizeof(SCSI_PASS_THROUGH_DIRECT);
		sb.sptd.PathId             = 0;
		sb.sptd.TargetId           = 1;
		sb.sptd.Lun                = 0;
		sb.sptd.CdbLength          = MySRB.SRB_CDBLen;
		sb.sptd.DataIn             = MySRB.SRB_Flags;
		sb.sptd.SenseInfoLength    = 32;
		sb.sptd.DataTransferLength = MySRB.SRB_BufLen;
		sb.sptd.TimeOutValue       = TIMEOUT;
		sb.sptd.DataBuffer         = MySRB.SRB_BufPointer;
		sb.sptd.SenseInfoOffset    =
			offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, ucSenseBuf);
		for (int i=0; i<MySRB.SRB_CDBLen; i++) sb.sptd.Cdb[i]=MySRB.CDBByte[i];
		il = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
		if (DeviceIoControl(hDriveEvent, IOCTL_SCSI_PASS_THROUGH_DIRECT,
                      &sb, il, &sb, il, &ol, NULL)) 
			if (sb.sptd.ScsiStatus==0)
			{
				MySRB.SRB_Status=SS_COMP;
				return;
			}
		MySRB.SRB_Status=SS_ERR;
		return;
	}
	DWORD ASPI_Status;
	ResetEvent(hDriveEvent);
	ASPI_Status = SendASPI32Command ((LPSRB)&MySRB);
}

BYTE WaitSCSIRequest(SRB_ExecSCSICmd &MySRB,HANDLE hDriveEvent,BOOL bImmediate)
{
	if ((MySRB.SRB_HaId>=NumberOfHostAdapters) &&
		RunningNT)
		return MySRB.SRB_Status;
    if ((MySRB.SRB_Status == SS_PENDING) &&
		!bImmediate)
    {
		DWORD ASPIEventStatus = WaitForSingleObject(hDriveEvent, 100);

    	if (ASPIEventStatus == WAIT_OBJECT_0)
		{
    		ResetEvent(hDriveEvent);
		}
	}
	
	return MySRB.SRB_Status;
}	

BOOL AbortSCSIRequest(SRB_ExecSCSICmd &StuckSRB)
{
    SRB_Abort AbortSRB;
    DWORD ASPIStatus;
    AbortSRB.SRB_Cmd      = SC_ABORT_SRB;
    AbortSRB.SRB_HaId     = StuckSRB.SRB_HaId;
    AbortSRB.SRB_Flags    = 0;
    AbortSRB.SRB_Hdr_Rsvd = 0;
    AbortSRB.SRB_ToAbort  = (LPSRB)&StuckSRB;
    ASPIStatus = SendASPI32Command ( (LPSRB)&AbortSRB );
	return (ASPIStatus==SS_COMP);
}


int GetDeviceInfo(int HostAdapterNumber,int TargetId,int LUN,BYTE &SCSIType,char *VendorID,
	char *ProductID,char *ProductRevision,HANDLE hDriveEvent)
{
	struct InquireFormat
	{
		BYTE ConfigPara[8];
		char VendorID[8];
		char ProductID[16];
		char ProductRevision[4];
	} DeviceInfo;
	TOpcode OpC;
	OpC[0]=0x12;
	OpC[1]=0;
	OpC[2]=0;
	OpC[3]=0;
	OpC[4]=sizeof(DeviceInfo);
	OpC[5]=0;
	memset(&DeviceInfo,0,sizeof(DeviceInfo));
	BOOL r=ExecuteSCSIRequest(HostAdapterNumber,TargetId,LUN,SRB_DIR_IN,OpC,6,(void*)&DeviceInfo,sizeof(DeviceInfo),hDriveEvent);
	if (r)
	{
		for (int i=0; i<16; i++)
		{
			if (i<8) VendorID[i]=DeviceInfo.VendorID[i];
			ProductID[i]=DeviceInfo.ProductID[i];
			if (i<4) ProductRevision[i]=DeviceInfo.ProductRevision[i];
		}
		VendorID[8]=0;
		ProductID[16]=0;
		ProductRevision[4]=0;
		SCSIType=DeviceInfo.ConfigPara[2] & 0x0f;
		return DeviceInfo.ConfigPara[0];
	}
	return DTYPE_UNKNOWN;
}

BOOL TestUnitReady(int HostAdapterNumber,int TargetId,int LUN,HANDLE hDriveEvent)
{
	TOpcode OpC;
	OpC[0]=0;
	OpC[1]=0;
	OpC[2]=0;
	OpC[3]=0;
	OpC[4]=0;
	OpC[5]=0;
	return ExecuteSCSIRequest(HostAdapterNumber,TargetId,LUN,SRB_DIR_IN,OpC,6,NULL,0,hDriveEvent);
}

BOOL ModeSense(int HostAdapterNumber,int TargetId,int LUN,TDriveMode &ModeData,int Size,int PageCode,HANDLE hDriveEvent)
{
//	while (!TestUnitReady(HostAdapterNumber,TargetId,LUN));
	TOpcode OpC;
	OpC[0]=0x1a;
	OpC[1]=0x00;
	OpC[2]=PageCode;
	OpC[3]=0x00;
	OpC[4]=Size;
	OpC[5]=0x00;
	return ExecuteSCSIRequest(HostAdapterNumber,TargetId,LUN,SRB_DIR_IN,OpC,6,(void *)&ModeData,Size,hDriveEvent);
}

BOOL ATAPIModeSense(int HostAdapterNumber,int TargetId,int LUN,TDriveMode &ModeData,int Size,int PageCode,HANDLE hDriveEvent)
{
//	while (!TestUnitReady(HostAdapterNumber,TargetId,LUN));
	TOpcode OpC;
	OpC[0]=0x5a;
	OpC[1]=0x00;
	OpC[2]=PageCode;
	OpC[3]=0x00;
	OpC[4]=0x00;
	OpC[5]=0x00;
	OpC[6]=0x00;
	OpC[7]=0x00;
	OpC[8]=Size;
	OpC[9]=0x00;
	OpC[10]=0x00;
	OpC[11]=0x00;
	return ExecuteSCSIRequest(HostAdapterNumber,TargetId,LUN,SRB_DIR_IN,OpC,12,(void *)&ModeData,Size,hDriveEvent);
}

BOOL addModeSense(int HostAdapterNumber,int TargetId,int LUN,TDriveMode &ModeData,int Size,HANDLE hDriveEvent)
{
//	while (!TestUnitReady(HostAdapterNumber,TargetId,LUN));
	TOpcode OpC;
	OpC[0]=0xca;
	OpC[1]=0x08;
	OpC[2]=0x0f;
	OpC[3]=0x00;
	OpC[4]=0x00;
	OpC[5]=0x00;
	OpC[6]=0x00;
	OpC[7]=0x00;
	OpC[8]=Size;
	OpC[9]=0x00;
	return ExecuteSCSIRequest(HostAdapterNumber,TargetId,LUN,SRB_DIR_IN,OpC,10,(void *)&ModeData,Size,hDriveEvent);
}

BOOL ModeSelect(int HostAdapterNumber,int TargetId,int LUN,TDriveMode &ModeData,int Size,HANDLE hDriveEvent)
{
//	while (!TestUnitReady(HostAdapterNumber,TargetId,LUN));
	TOpcode OpC;
	OpC[0]=0x15;
	if (Size==12)
		OpC[1]=0x00;
	else
		OpC[1]=0x10;
	OpC[2]=0x00;
	OpC[3]=0x00;
	OpC[4]=Size;
	OpC[5]=0x00;
	return ExecuteSCSIRequest(HostAdapterNumber,TargetId,LUN,SRB_DIR_OUT,OpC,6,(void *)&ModeData,Size,hDriveEvent);
}

BOOL addModeSelect(int HostAdapterNumber,int TargetId,int LUN,TDriveMode &ModeData,int Size,HANDLE hDriveEvent)
{
//	while (!TestUnitReady(HostAdapterNumber,TargetId,LUN));
	TOpcode OpC;
	OpC[0]=0xc5;
	OpC[1]=0x10;
	OpC[2]=0x00;
	OpC[3]=0x00;
	OpC[4]=0x00;
	OpC[5]=0x00;
	OpC[6]=0x00;
	OpC[7]=0x00;
	OpC[8]=Size;
	OpC[9]=0x00;
	return ExecuteSCSIRequest(HostAdapterNumber,TargetId,LUN,SRB_DIR_OUT,OpC,10,(void *)&ModeData,Size,hDriveEvent);
}

BOOL SCSIMaxBlocks(HANDLE fh, int *mb)
{
	DWORD ol;
	IO_SCSI_CAPABILITIES ca;

	if (DeviceIoControl(fh,IOCTL_SCSI_GET_CAPABILITIES,NULL,0,
						&ca,sizeof(IO_SCSI_CAPABILITIES),&ol,NULL)) 
	{
		*mb=(int)ca.MaximumTransferLength;
		return TRUE;
	}
	return FALSE;
}