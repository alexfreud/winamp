#include <windows.h>
#include <winioctl.h>

//functions to get the SCSI ID from a drive letter
//thanks to Microsoft for making this a nightmare...

// begin ntddscsi.h definitions

#define IOCTL_SCSI_BASE                  \
    FILE_DEVICE_CONTROLLER

#define IOCTL_SCSI_GET_ADDRESS  CTL_CODE \
    (                                    \
        IOCTL_SCSI_BASE,                 \
        0x0406,                          \
        METHOD_BUFFERED,                 \
        FILE_ANY_ACCESS                  \
    )

#define IOCTL_SCSI_GET_INQUIRY_DATA      \
    CTL_CODE(                            \
        IOCTL_SCSI_BASE,                 \
        0x0403,                          \
        METHOD_BUFFERED,                 \
        FILE_ANY_ACCESS                  \
    )

typedef struct _SCSI_ADDRESS {
    ULONG Length;
    UCHAR PortNumber;
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
}SCSI_ADDRESS, *PSCSI_ADDRESS;

//
// Define SCSI information.
// Used with the IOCTL_SCSI_GET_INQUIRY_DATA IOCTL.
//

typedef struct _SCSI_BUS_DATA {
    UCHAR NumberOfLogicalUnits;
    UCHAR InitiatorBusId;
    ULONG InquiryDataOffset;
}SCSI_BUS_DATA, *PSCSI_BUS_DATA;

//
// Define SCSI adapter bus information structure..
// Used with the IOCTL_SCSI_GET_INQUIRY_DATA IOCTL.
//

typedef struct _SCSI_ADAPTER_BUS_INFO {
    UCHAR NumberOfBuses;
    SCSI_BUS_DATA BusData[1];
} SCSI_ADAPTER_BUS_INFO, *PSCSI_ADAPTER_BUS_INFO;

//
// Define SCSI adapter bus information.
// Used with the IOCTL_SCSI_GET_INQUIRY_DATA IOCTL.
//

typedef struct _SCSI_INQUIRY_DATA {
    UCHAR PathId;
    UCHAR TargetId;
    UCHAR Lun;
    BOOLEAN DeviceClaimed;
    ULONG InquiryDataLength;
    ULONG NextInquiryDataOffset;
    UCHAR InquiryData[1];
}SCSI_INQUIRY_DATA, *PSCSI_INQUIRY_DATA;

// end ntddscsi.h definitions

int getSCSIIDFromDrive(char driveletter, int *host, int *id, int *lun)
{
  //FUCKO: only works on NT :(

  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
  wchar_t tmp[128] = {0};
  wsprintf(tmp,L"\\\\.\\%c:",driveletter);
  HANDLE device = CreateFile(
            tmp,
            0,     // no particular access necessary
                   // for the IO control we're using
            FILE_SHARE_READ
                | FILE_SHARE_WRITE
                | FILE_SHARE_DELETE,
            0,     // no security attrs - ignored anyway
            OPEN_EXISTING,
            0,     // no attributes or flags
            0      // no template
        );

  if( device == INVALID_HANDLE_VALUE ) return 0;

  SCSI_ADDRESS sa;
  ULONG bytes;

  BOOL status = DeviceIoControl(
            device,
            IOCTL_SCSI_GET_ADDRESS,
            0, 0,                    // no input buffer
            &sa, sizeof(sa),         // output args
            &bytes,                  // bytes returned
            0                        // ignored
  );

  if (!status && 50 == GetLastError())
  {
	  *host	= ((driveletter > 'a') ? (driveletter - 'a') : (driveletter - 'A'));
		*id		= 0;
		*lun	= 0;
		CloseHandle(device);
		return 1;
  }
  CloseHandle(device);

  if( !status ) return 0;

  *host=sa.PortNumber;
  *id=sa.TargetId;
  *lun=sa.Lun;
  return 1;
}
