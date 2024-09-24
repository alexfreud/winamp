// this file almost totally copied from MSDN

#include <windows.h>
#include <stdio.h>
#include <winioctl.h>

#define LOCK_TIMEOUT  3000       // 10 Seconds
#define LOCK_RETRIES  20

static HANDLE OpenVolume(TCHAR cDriveLetter)
{
    HANDLE hVolume;
    UINT   uDriveType;
    wchar_t   szVolumeName[8] = {0};
    wchar_t   szRootName[5] = {0};
    DWORD  dwAccessFlags;
   
    wsprintf(szRootName, L"%c:\\", cDriveLetter);

    uDriveType = GetDriveType(szRootName);
    switch(uDriveType) {
      case DRIVE_REMOVABLE:
           dwAccessFlags = GENERIC_READ | GENERIC_WRITE;
           break;
      case DRIVE_CDROM:
           dwAccessFlags = GENERIC_READ;
           break;
      default:
           printf("Cannot eject.  Drive type is incorrect.\n");
           return INVALID_HANDLE_VALUE;
    }

    wsprintf(szVolumeName, L"\\\\.\\%c:", cDriveLetter);

    hVolume = CreateFile( szVolumeName,
                          dwAccessFlags,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          0,
                          NULL );

    if (hVolume == INVALID_HANDLE_VALUE)
      printf("CreateFile error %d\n", GetLastError());
    return hVolume;
}

static BOOL CloseVolume(HANDLE hVolume)
{
    return CloseHandle(hVolume);
}

static BOOL LockVolume(HANDLE hVolume)
{
    DWORD dwBytesReturned;
    DWORD dwSleepAmount;
    int nTryCount;

    dwSleepAmount = LOCK_TIMEOUT / LOCK_RETRIES;

    // Do this in a loop until a timeout period has expired
    for (nTryCount = 0; nTryCount < LOCK_RETRIES; nTryCount++) {
      if (DeviceIoControl(hVolume,
                          FSCTL_LOCK_VOLUME,
                          NULL, 0,
                          NULL, 0,
                          &dwBytesReturned,
                          NULL))
         return TRUE;

      Sleep(dwSleepAmount);
    }
    return FALSE;
}

static BOOL DismountVolume(HANDLE hVolume)
{
    DWORD dwBytesReturned;

    return DeviceIoControl( hVolume,
                            FSCTL_DISMOUNT_VOLUME,
                            NULL, 0,
                            NULL, 0,
                            &dwBytesReturned,
                            NULL);
}

static BOOL PreventRemovalOfVolume(HANDLE hVolume, BOOL fPreventRemoval)
{
    DWORD dwBytesReturned;
    PREVENT_MEDIA_REMOVAL PMRBuffer;

    PMRBuffer.PreventMediaRemoval = fPreventRemoval;

    return DeviceIoControl( hVolume,
                            IOCTL_STORAGE_MEDIA_REMOVAL,
                            &PMRBuffer, sizeof(PREVENT_MEDIA_REMOVAL),
                            NULL, 0,
                            &dwBytesReturned,
                            NULL);
}

static int AutoEjectVolume(HANDLE hVolume)
{
    DWORD dwBytesReturned;

    return DeviceIoControl( hVolume,
                            IOCTL_STORAGE_EJECT_MEDIA,
                            NULL, 0,
                            NULL, 0,
                            &dwBytesReturned,
                            NULL);
}

BOOL EjectVolume(TCHAR cDriveLetter)
{
    BOOL fAutoEject = FALSE;
    HANDLE hVolume = OpenVolume(cDriveLetter);
    if (hVolume == INVALID_HANDLE_VALUE)
      return FALSE;

    // Lock and dismount the volume.
    if (LockVolume(hVolume) && DismountVolume(hVolume)) {
      // Set prevent removal to false and eject the volume.
      if (PreventRemovalOfVolume(hVolume, FALSE) && AutoEjectVolume(hVolume))
        fAutoEject = TRUE;
    }

    // Close the volume so other processes can use the drive.
    if (!CloseVolume(hVolume))
      return FALSE;

    if (fAutoEject) return TRUE;
    else return FALSE;
}