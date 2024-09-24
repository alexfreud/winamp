#include "./spti.h"
#include <ntddscsi.h>


#define CDB6GENERIC_LENGTH                   6
#define CDB10GENERIC_LENGTH                  10

#define SETBITON                             1
#define SETBITOFF                            0

//
// Mode Sense/Select page constants.
//

#define MODE_PAGE_ERROR_RECOVERY        0x01
#define MODE_PAGE_DISCONNECT            0x02
#define MODE_PAGE_FORMAT_DEVICE         0x03
#define MODE_PAGE_RIGID_GEOMETRY        0x04
#define MODE_PAGE_FLEXIBILE             0x05
#define MODE_PAGE_VERIFY_ERROR          0x07
#define MODE_PAGE_CACHING               0x08
#define MODE_PAGE_PERIPHERAL            0x09
#define MODE_PAGE_CONTROL               0x0A
#define MODE_PAGE_MEDIUM_TYPES          0x0B
#define MODE_PAGE_NOTCH_PARTITION       0x0C
#define MODE_SENSE_RETURN_ALL           0x3f
#define MODE_SENSE_CURRENT_VALUES       0x00
#define MODE_SENSE_CHANGEABLE_VALUES    0x40
#define MODE_SENSE_DEFAULT_VAULES       0x80
#define MODE_SENSE_SAVED_VALUES         0xc0
#define MODE_PAGE_DEVICE_CONFIG         0x10
#define MODE_PAGE_MEDIUM_PARTITION      0x11
#define MODE_PAGE_DATA_COMPRESS         0x0f
#define MODE_PAGE_CAPABILITIES			0x2A

//
// SCSI CDB operation codes
//

#define SCSIOP_TEST_UNIT_READY     0x00
#define SCSIOP_REZERO_UNIT         0x01
#define SCSIOP_REWIND              0x01
#define SCSIOP_REQUEST_BLOCK_ADDR  0x02
#define SCSIOP_REQUEST_SENSE       0x03
#define SCSIOP_FORMAT_UNIT         0x04
#define SCSIOP_READ_BLOCK_LIMITS   0x05
#define SCSIOP_REASSIGN_BLOCKS     0x07
#define SCSIOP_READ6               0x08
#define SCSIOP_RECEIVE             0x08
#define SCSIOP_WRITE6              0x0A
#define SCSIOP_PRINT               0x0A
#define SCSIOP_SEND                0x0A
#define SCSIOP_SEEK6               0x0B
#define SCSIOP_TRACK_SELECT        0x0B
#define SCSIOP_SLEW_PRINT          0x0B
#define SCSIOP_SEEK_BLOCK          0x0C
#define SCSIOP_PARTITION           0x0D
#define SCSIOP_READ_REVERSE        0x0F
#define SCSIOP_WRITE_FILEMARKS     0x10
#define SCSIOP_FLUSH_BUFFER        0x10
#define SCSIOP_SPACE               0x11
#define SCSIOP_INQUIRY             0x12
#define SCSIOP_VERIFY6             0x13
#define SCSIOP_RECOVER_BUF_DATA    0x14
#define SCSIOP_MODE_SELECT         0x15
#define SCSIOP_RESERVE_UNIT        0x16
#define SCSIOP_RELEASE_UNIT        0x17
#define SCSIOP_COPY                0x18
#define SCSIOP_ERASE               0x19
#define SCSIOP_MODE_SENSE          0x1A
#define SCSIOP_START_STOP_UNIT     0x1B
#define SCSIOP_STOP_PRINT          0x1B
#define SCSIOP_LOAD_UNLOAD         0x1B
#define SCSIOP_RECEIVE_DIAGNOSTIC  0x1C
#define SCSIOP_SEND_DIAGNOSTIC     0x1D
#define SCSIOP_MEDIUM_REMOVAL      0x1E
#define SCSIOP_READ_CAPACITY       0x25
#define SCSIOP_READ                0x28
#define SCSIOP_WRITE               0x2A
#define SCSIOP_SEEK                0x2B
#define SCSIOP_LOCATE              0x2B
#define SCSIOP_WRITE_VERIFY        0x2E
#define SCSIOP_VERIFY              0x2F
#define SCSIOP_SEARCH_DATA_HIGH    0x30
#define SCSIOP_SEARCH_DATA_EQUAL   0x31
#define SCSIOP_SEARCH_DATA_LOW     0x32
#define SCSIOP_SET_LIMITS          0x33
#define SCSIOP_READ_POSITION       0x34
#define SCSIOP_SYNCHRONIZE_CACHE   0x35
#define SCSIOP_COMPARE             0x39
#define SCSIOP_COPY_COMPARE        0x3A
#define SCSIOP_WRITE_DATA_BUFF     0x3B
#define SCSIOP_READ_DATA_BUFF      0x3C
#define SCSIOP_CHANGE_DEFINITION   0x40
#define SCSIOP_READ_SUB_CHANNEL    0x42
#define SCSIOP_READ_TOC            0x43
#define SCSIOP_READ_HEADER         0x44
#define SCSIOP_PLAY_AUDIO          0x45
#define SCSIOP_PLAY_AUDIO_MSF      0x47
#define SCSIOP_PLAY_TRACK_INDEX    0x48
#define SCSIOP_PLAY_TRACK_RELATIVE 0x49
#define SCSIOP_PAUSE_RESUME        0x4B
#define SCSIOP_LOG_SELECT          0x4C
#define SCSIOP_LOG_SENSE           0x4D
#define SCSIOP_READ_DISC_INFORMATION    0x51

typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS 
{
    SCSI_PASS_THROUGH spt;
    DWORD	filler;
    UCHAR	ucSenseBuf[24];
	UCHAR	ucDataBuf[256];
}SCSI_PASS_THROUGH_WITH_BUFFERS;

#pragma pack(1)

typedef struct _CDB_START_STOP_UNIT 
{
	UCHAR OperationCode; // 0x1B - SCSIOP_START_STOP_UNIT
	UCHAR Immediate : 1;
	UCHAR Reserved1 : 4;
	UCHAR Lun : 3;
	UCHAR Reserved2[2];
	UCHAR Start : 1;
	UCHAR LoadEject : 1;
	UCHAR Reserved3 : 2;
	UCHAR PowerCondition : 4;
	UCHAR Control;
} CDB_START_STOP_UNIT;


typedef  struct _READ_DISC_INFORMATION 
{
	UCHAR OperationCode; // 0x51 - SCSIOP_READ_DISC_INFORMATION
	UCHAR Reserved1 : 5;
	UCHAR Lun : 3;
	UCHAR Reserved2[5];
	UCHAR AllocationLength[2];
	UCHAR Control;
} READ_DISC_INFORMATION; 

#pragma pack()


BOOL SPTI_TestUnitReady(HANDLE hDevice, BYTE *pbSC, BYTE *pbASC, BYTE *pbASCQ, INT timeOutSec)
{
	INT length;
	DWORD returned;
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	
	if (INVALID_HANDLE_VALUE == hDevice) return FALSE;
	
	ZeroMemory(&sptwb, sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.CdbLength = CDB6GENERIC_LENGTH;
	sptwb.spt.SenseInfoLength = 24;
	sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.spt.DataTransferLength = 0;
	sptwb.spt.TimeOutValue = timeOutSec; 
	sptwb.spt.DataBufferOffset = ((DWORD)(DWORD_PTR)&sptwb.ucDataBuf) - ((DWORD)(DWORD_PTR)&sptwb);
	sptwb.spt.SenseInfoOffset  = ((DWORD)(DWORD_PTR)&sptwb.ucSenseBuf) - ((DWORD)(DWORD_PTR)&sptwb);
	sptwb.spt.Cdb[0] = SCSIOP_TEST_UNIT_READY;
	length = ((DWORD)(DWORD_PTR)&sptwb.ucDataBuf) - ((DWORD)(DWORD_PTR)&sptwb);
	DeviceIoControl(hDevice, IOCTL_SCSI_PASS_THROUGH, &sptwb, sizeof(SCSI_PASS_THROUGH), &sptwb, length, &returned, FALSE);
			
	if (pbSC) *pbSC = sptwb.ucSenseBuf[2];
	if (pbASC) *pbASC = sptwb.ucSenseBuf[12];
	if (pbASCQ) *pbASCQ = sptwb.ucSenseBuf[13];
	
	return TRUE;
}
BOOL SPTI_StartStopUnit(HANDLE hDevice, BOOL bImmediate, BOOL bLoadEject, BOOL bStart, INT timeOutSec, SENSEINFO *pSense)
{
	INT length;
	BOOL status;
	DWORD returned;
	CDB_START_STOP_UNIT cmd;
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	
	UNREFERENCED_PARAMETER(pSense);

	if (INVALID_HANDLE_VALUE == hDevice) return FALSE;
	
	ZeroMemory(&sptwb, sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	ZeroMemory(&cmd, sizeof(CDB_START_STOP_UNIT));

	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.CdbLength = CDB6GENERIC_LENGTH;
	sptwb.spt.SenseInfoLength = 24;
	sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.spt.DataTransferLength = 0;
	sptwb.spt.TimeOutValue = timeOutSec; 
	sptwb.spt.DataBufferOffset = ((DWORD)(DWORD_PTR)&sptwb.ucDataBuf) - ((DWORD)(DWORD_PTR)&sptwb);
	sptwb.spt.SenseInfoOffset  = ((DWORD)(DWORD_PTR)&sptwb.ucSenseBuf) - ((DWORD)(DWORD_PTR)&sptwb);
	cmd.OperationCode = SCSIOP_START_STOP_UNIT; 
	cmd.Immediate = (bImmediate) ? 1 : 0;
	cmd.LoadEject= (bLoadEject) ? 1 : 0;
	cmd.Start = (bStart) ? 1 : 0;
	CopyMemory(sptwb.spt.Cdb, &cmd, sizeof(CDB_START_STOP_UNIT));

	length = ((DWORD)(DWORD_PTR)&sptwb.ucDataBuf) - ((DWORD)(DWORD_PTR)&sptwb);
	status = DeviceIoControl(hDevice, IOCTL_SCSI_PASS_THROUGH, &sptwb, sizeof(SCSI_PASS_THROUGH), &sptwb, length, &returned, FALSE);
		
	return status;
}
BOOL SPTI_GetCapabilities(HANDLE hDevice, DWORD *pCap)
{
	INT length = 0;
	BOOL status;
	DWORD returned = 0;
	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
	
	if (INVALID_HANDLE_VALUE == hDevice || !pCap) return FALSE;
	
	*pCap = 0;

	ZeroMemory(&sptwb, sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));
	sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.spt.CdbLength = CDB6GENERIC_LENGTH;
	sptwb.spt.SenseInfoLength = 24;
	sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.spt.DataTransferLength = 192;
	sptwb.spt.TimeOutValue = 10; //2 sec
	sptwb.spt.DataBufferOffset = ((DWORD)(DWORD_PTR)&sptwb.ucDataBuf) - ((DWORD)(DWORD_PTR)&sptwb);
	sptwb.spt.SenseInfoOffset  = ((DWORD)(DWORD_PTR)&sptwb.ucSenseBuf) - ((DWORD)(DWORD_PTR)&sptwb);
	sptwb.spt.Cdb[0] = SCSIOP_MODE_SENSE;
	sptwb.spt.Cdb[1] = 0x08;                    // target shall not return any block descriptors
	sptwb.spt.Cdb[2] = MODE_PAGE_CAPABILITIES;
	sptwb.spt.Cdb[4] = 192;

	length = ((DWORD)(DWORD_PTR)&sptwb.ucDataBuf) - ((DWORD)(DWORD_PTR)&sptwb) + sptwb.spt.DataTransferLength;;
	
	status = DeviceIoControl(hDevice, IOCTL_SCSI_PASS_THROUGH, &sptwb, sizeof(SCSI_PASS_THROUGH), &sptwb, length, &returned, FALSE);
	if (!status) return FALSE;

	*pCap = *((DWORD*)&sptwb.ucDataBuf[6]) ;
	
	return TRUE;
}
