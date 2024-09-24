#include <windows.h>
#include <stddef.h> // for offsetof
#include <winioctl.h>
#include <strsafe.h>

	typedef struct {
  USHORT Length;
  UCHAR  ScsiStatus;
  UCHAR  PathId;
  UCHAR  TargetId;
  UCHAR  Lun;
  UCHAR  CdbLength;
  UCHAR  SenseInfoLength;
  UCHAR  DataIn;
  ULONG  DataTransferLength;
  ULONG  TimeOutValue;
  PVOID  DataBuffer;
  ULONG  SenseInfoOffset;
  UCHAR  Cdb[16];
} SCSI_PASS_THROUGH_DIRECT, *PSCSI_PASS_THROUGH_DIRECT;


	typedef struct {
  SCSI_PASS_THROUGH_DIRECT spt;
  ULONG Filler;
  UCHAR ucSenseBuf[32];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;



	#define IOCTL_SCSI_BASE    0x00000004

/*
 * constants for DataIn member of SCSI_PASS_THROUGH* structures
 */
#define  SCSI_IOCTL_DATA_OUT          0
#define  SCSI_IOCTL_DATA_IN           1
#define  SCSI_IOCTL_DATA_UNSPECIFIED  2

/*
 * Standard IOCTL define
 */
#define CTL_CODE( DevType, Function, Method, Access ) (                 \
    ((DevType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define IOCTL_SCSI_PASS_THROUGH         CTL_CODE( IOCTL_SCSI_BASE, 0x0401, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS )
#define IOCTL_SCSI_MINIPORT             CTL_CODE( IOCTL_SCSI_BASE, 0x0402, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS )
#define IOCTL_SCSI_GET_INQUIRY_DATA     CTL_CODE( IOCTL_SCSI_BASE, 0x0403, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SCSI_GET_CAPABILITIES     CTL_CODE( IOCTL_SCSI_BASE, 0x0404, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SCSI_PASS_THROUGH_DIRECT  CTL_CODE( IOCTL_SCSI_BASE, 0x0405, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS )
#define IOCTL_SCSI_GET_ADDRESS          CTL_CODE( IOCTL_SCSI_BASE, 0x0406, METHOD_BUFFERED, FILE_ANY_ACCESS 

static bool scsi_inquiry(HANDLE deviceHandle, UCHAR page, UCHAR *inqbuf, size_t &buf_len)
{
	char buf[2048] = {0};
	BOOL status;
	PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER pswb;
	
	DWORD length=0, returned=0;

  /*
   * Get the drive inquiry data
   */
  ZeroMemory( &buf, 2048 );
  ZeroMemory( inqbuf, buf_len );
  pswb                      = (PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER)buf;
  pswb->spt.Length          = sizeof(SCSI_PASS_THROUGH_DIRECT);
  pswb->spt.CdbLength       = 6;
  pswb->spt.SenseInfoLength = 32;
  pswb->spt.DataIn          = SCSI_IOCTL_DATA_IN;
  pswb->spt.DataTransferLength = buf_len;
  pswb->spt.TimeOutValue    = 2;
  pswb->spt.DataBuffer      = inqbuf;
  pswb->spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
  pswb->spt.Cdb[0]          = 0x12;
	pswb->spt.Cdb[1]          = 0x1;
	pswb->spt.Cdb[2]					= page;
  pswb->spt.Cdb[4]          = (UCHAR)buf_len;

  length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
  status = DeviceIoControl( deviceHandle,
			    IOCTL_SCSI_PASS_THROUGH_DIRECT,
			    pswb,
			    length,
			    pswb,
			    length,
			    &returned,
			    NULL );

	
	if (status && returned >3)
	{
		buf_len=returned;
		return true;
	}
	else
	{
		buf_len=0;
		return false;
	}
}

static bool  AddPagetoXML(HANDLE dev, char *&dest, size_t &destlen, UCHAR page)
{
	UCHAR buf[256] = {0};
	size_t buflen=255;
	if (scsi_inquiry(dev, page, buf, buflen))
	{
	size_t len = buf[3];
	StringCchCopyNExA(dest, destlen, (char *)buf+4, len, &dest, &destlen, 0);
	return true;
	}
	return false;
}

static bool BuildXML(HANDLE dev, char *xml, size_t xmllen)
{
	*xml=0;
	UCHAR pages[255] = {0};
	size_t pageslen=255;
	if (scsi_inquiry(dev, 0xc0, pages, pageslen) && pageslen>3)
	{
		unsigned char numPages=pages[3];
		if (numPages+4 <= 255)
		{
			for (int i=0;i<numPages;i++)
			{
				if (!AddPagetoXML(dev, xml, xmllen, pages[i+4]))
					return false;
			}
		}
	}
	return true;
}

bool ParseSysInfoXML(wchar_t drive_letter, char * xml, int xmllen)
{
	wchar_t fn[MAX_PATH] = {0};
	StringCchPrintf(fn, MAX_PATH, L"\\\\.\\%c:", drive_letter);
	HANDLE hfile = CreateFileW(fn, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if (hfile != INVALID_HANDLE_VALUE)
	{
		bool ret = BuildXML(hfile, xml, xmllen);
		CloseHandle(hfile);
		return ret;
	}
	return false;
}
