//**********************************************************************
//
// Name: 			 WINASPI.H
//
// Description:	 ASPI for Windows definitions ('C' Language)
//
//**********************************************************************

#ifndef _WINASPI_H
#define _WINASPI_H

typedef BYTE *LPSRB;
#define SENSE_LEN		14		// Default sense buffer length
#define SRB_DIR_SCSI	0x00		// Direction determined by SCSI command
#define SRB_DIR_IN		0x08		// Transfer from SCSI target to host
#define SRB_DIR_OUT	0x10		// Transfer from host to SCSI targetw
#define SRB_POSTING	0x01		// Enable ASPI posting
#define SRB_EVENT_NOTIFY 0x40	// Enable ASPI command notification
#define SRB_ENABLE_RESIDUAL_COUNT 0x04 //Enable reporting of residual byte count
#define WM_ASPIPOST	0x4D42	// ASPI Post message
#define TIMEOUT 30000  // Wait 30 seconds

//**********************************************************************
//					 %%% ASPI Command Definitions %%%
//**********************************************************************
#define SC_HA_INQUIRY		0x00		// Host adapter inquiry
#define SC_GET_DEV_TYPE	0x01		// Get device type
#define SC_EXEC_SCSI_CMD	0x02		// Execute SCSI command
#define SC_ABORT_SRB		0x03		// Abort an SRB
#define SC_RESET_DEV		0x04		// SCSI bus device reset
//**********************************************************************
//						 %%% SRB Status %%%
//**********************************************************************
#define SS_PENDING	0x00		// SRB being processed
#define SS_COMP		0x01		// SRB completed without error
#define SS_ABORTED	0x02		// SRB aborted
#define SS_ABORT_FAIL	0x03		// Unable to abort SRB
#define SS_ERR 		0x04		// SRB completed with error
#define SS_INVALID_CMD	0x80		// Invalid ASPI command
#define SS_INVALID_HA	0x81		// Invalid host adapter number
#define SS_NO_DEVICE	0x82		// SCSI device not installed
#define SS_INVALID_SRB	0xE0		// Invalid parameter set in SRB
#define SS_OLD_MANAGER  0xE1		// ASPI manager doesn't support Window
#define SS_ILLEGAL_MODE  0xE2	// Unsupported Windows mode
#define SS_NO_ASPI		0xE3		// No ASPI managers resident
#define SS_FAILED_INIT	0xE4		// ASPI for windows failed init
#define SS_ASPI_IS_BUSY	0xE5		// No resources available to execute cmd
#define SS_BUFFER_TO_BIG	0xE6	// Buffer size to big to handle!
//**********************************************************************
//						%%% Host Adapter Status %%%
//**********************************************************************
#define HASTAT_OK	0x00		// Host adapter did not detect an error
#define HASTAT_SEL_TO	0x11		// Selection Timeout
#define HASTAT_DO_DU	0x12		// Data overrun data underrun
#define HASTAT_BUS_FREE	0x13		// Unexpected bus free
#define HASTAT_PHASE_ERR	0x14		// Target bus phase sequence failure





//**********************************************************************
//			 %%% SRB - HOST ADAPTER INQUIRY - SC_HA_INQUIRY %%%
//**********************************************************************
#pragma pack(push,ASPI_Structures,1)

typedef BYTE TDriveMode[64];

struct THAUnique
{
	WORD  BufferAlignmentMask;
	BYTE  AdapterUniqueFlags;
	BYTE  MaximumSCSITargets;
	DWORD MaximumTransferLen;
	BYTE  Reserved[8];
};

typedef struct
{
	BYTE        SRB_Cmd;            // ASPI command code = SC_HA_INQUIRY
	BYTE        SRB_Status;         // ASPI command status byte
	BYTE        SRB_HaId;           // ASPI host adapter number
	BYTE        SRB_Flags;          // ASPI request flags
	DWORD       SRB_Hdr_Rsvd;       // Reserved, MUST = 0
	BYTE        HA_Count;           // Number of host adapters present
	BYTE        HA_SCSI_ID;         // SCSI ID of host adapter
	BYTE        HA_ManagerId[16];   // String describing the manager
	BYTE        HA_Identifier[16];  // String describing the host adapter
	THAUnique   HA_Unique;      // Host Adapter Unique parameters
	WORD        HA_Rsvd1;
} SRB_HAInquiry, *PSRB_HAInquiry;
//**********************************************************************
//			  %%% SRB - GET DEVICE TYPE - SC_GET_DEV_TYPE %%%
//**********************************************************************
typedef struct
{
	BYTE        SRB_Cmd;            // ASPI command code = SC_GET_DEV_TYPE
	BYTE        SRB_Status;         // ASPI command status byte
	BYTE        SRB_HaId;           // ASPI host adapter number
	BYTE        SRB_Flags;          // Reserved
	DWORD       SRB_Hdr_Rsvd;       // Reserved
	BYTE        SRB_Target;         // Target's SCSI ID
	BYTE        SRB_Lun;            // Target's LUN number
	BYTE        SRB_DeviceType;     // Target's peripheral device type
	BYTE        SRB_Rsvd1;          // Reserved for alignment
} SRB_GDEVBlock, *PSRB_GDEVBlock;
//**********************************************************************
//		  %%% SRB - EXECUTE SCSI COMMAND - SC_EXEC_SCSI_CMD %%%
//**********************************************************************
typedef struct
{
	BYTE        SRB_Cmd;            // ASPI command code = SC_EXEC_SCSI_CMD
	BYTE        SRB_Status;         // ASPI command status byte
	BYTE        SRB_HaId;           // ASPI host adapter number
	BYTE        SRB_Flags;          // ASPI request flags
	DWORD       SRB_Hdr_Rsvd;       // Reserved
	BYTE        SRB_Target;         // Target's SCSI ID
	BYTE        SRB_Lun;            // Target's LUN number
	WORD        SRB_Rsvd1;          // Reserved for Alignment
	DWORD       SRB_BufLen;         // Data Allocation Length
	BYTE        *SRB_BufPointer;    // Data Buffer Point
	BYTE        SRB_SenseLen;       // Sense Allocation Length
	BYTE        SRB_CDBLen;         // CDB Length
	BYTE        SRB_HaStat;         // Host Adapter Status
	BYTE        SRB_TargStat;       // Target Status
	void        (*SRB_PostProc)();  // Post routine
	void        *SRB_Rsvd2;         // Reserved
	BYTE        SRB_Rsvd3[16];      // Reserved for expansion
	BYTE        CDBByte[16];        // SCSI CDB
	BYTE        SenseArea[SENSE_LEN+2]; // Request Sense buffer
} SRB_ExecSCSICmd, *PSRB_ExecSCSICmd;
//**********************************************************************
//				 %%% SRB - ABORT AN SRB - SC_ABORT_SRB %%%
//**********************************************************************
typedef struct
{
	BYTE		SRB_Cmd;		// ASPI command code = SC_ABORT_SRB
	BYTE		SRB_Status;		// ASPI command status byte
	BYTE		SRB_HaId;		// ASPI host adapter number
	BYTE		SRB_Flags;		// ASPI request flags
	DWORD		SRB_Hdr_Rsvd;	// Reserved, MUST = 0
	LPSRB		SRB_ToAbort;	// Pointer to SRB to abort
} SRB_Abort;
//**********************************************************************
//			    %%% SRB - BUS DEVICE RESET - SC_RESET_DEV %%%
//**********************************************************************
typedef struct
{
	BYTE        SRB_Cmd;            // ASPI command code = SC_RESET_DEV
	BYTE        SRB_Status;         // ASPI command status byte
	BYTE        SRB_HaId;           // ASPI host adapter number
	BYTE        SRB_Flags;          // Reserved
	DWORD       SRB_Hdr_Rsvd;       // Reserved
	BYTE        SRB_Target;         // Target's SCSI ID
	BYTE        SRB_Lun;            // Target's LUN number
	BYTE        SRB_Rsvd1[12];      // Reserved for Alignment
	BYTE        SRB_HaStat;         // Host Adapter Status
	BYTE        SRB_TargStat;       // Target Status
	void        *SRB_PostProc;      // Post routine
	void        *SRB_Rsvd2;         // Reserved
	BYTE        SRB_Rsvd3[32];      // Reserved
} SRB_BusDeviceReset, *PSRB_BusDeviceReset;


//**********************************************************************
//			    %%% Header for TOC Reading %%%
//**********************************************************************
struct TTrackInfo
{
	BYTE	Reserved1;
	BYTE	AdrCtrl;
	BYTE	TrackNummer;
	BYTE	Reserved2;
	DWORD	AbsCDAdress;
};

struct TTOCHeader
{
	WORD		TOCDataLength;
	BYTE		FirstTrack;
	BYTE		LastTrack;
	TTrackInfo	Info[100];
};

//**********************************************************************
//			    %%% Structure for Read Sub-Channel %%%
//**********************************************************************
struct TQChannelInfo
{
	BYTE	Reserved1;
	BYTE	AudioStatus;
	WORD	DataLen;
	BYTE	FormatCode;
	BYTE	ADRCtrl;
	BYTE	TrackNumber;
	BYTE	IndexNumber;
	long	AbsCDAdress;
	long	RelTrackAdress;
};

//**********************************************************************
//				%%% Request Sense Data Format %%%
//**********************************************************************
typedef struct {
	BYTE		ErrorCode;		// Error Code (70H or 71H)
	BYTE		SegmentNum;		// Number of current segment descriptor
	BYTE		SenseKey;		// Sense Key(See bit definitions too)
	BYTE		InfoByte0;		// Information MSB
	BYTE		InfoByte1;		// Information MID
	BYTE		InfoByte2;		// Information MID
	BYTE		InfoByte3;		// Information LSB
	BYTE		AddSenLen;		// Additional Sense Length
	BYTE		ComSpecInf0;	// Command Specific Information MSB
	BYTE		ComSpecInf1;	// Command Specific Information MID
	BYTE		ComSpecInf2;	// Command Specific Information MID
	BYTE		ComSpecInf3;	// Command Specific Information LSB
	BYTE		AddSenseCode;	// Additional Sense Code
	BYTE		AddSenQual;		// Additional Sense Code Qualifier
	BYTE		FieldRepUCode;	// Field Replaceable Unit Code
	BYTE		SenKeySpec15;	// Sense Key Specific 15th byte
	BYTE		SenKeySpec16;	// Sense Key Specific 16th byte
	BYTE		SenKeySpec17;	// Sense Key Specific 17th byte
	BYTE		AddSenseBytes;	// Additional Sense Bytes
} TSenseInfo;

#pragma pack(pop,ASPI_Structures)


#endif //_WINASPI_H