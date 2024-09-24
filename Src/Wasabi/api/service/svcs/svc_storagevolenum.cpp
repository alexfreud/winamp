#include <precomp.h>
#include "svc_storagevolenum.h"

//-----------------------------------------------------------------

#define CBCLASS StorageVolumeI
START_DISPATCH;
  CB(GETVOLUMENAME, getVolumeName);
  CB(GETMOUNTPATH, getMountPath);
  CB(GETLABEL, getLabel);
  CB(GETTYPE, getType);
  CB(ISREMOVABLE, isRemovable);
  CB(ISWRITABLE, isWritable);
  CB(GETFREESPACE, getFreeSpace);
  CB(GETSIZE, getSize);
END_DISPATCH;
#undef CBCLASS

//-----------------------------------------------------------------

#define CBCLASS svc_storageVolumeEnumI
START_DISPATCH;
  CB(GETNUMVOLUMES, getNumVolumes);
  CB(ENUMVOLUME, enumVolume);
END_DISPATCH;
#undef CBCLASS

//-----------------------------------------------------------------


