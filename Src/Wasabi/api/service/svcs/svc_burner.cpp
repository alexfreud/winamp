#include <precomp.h>
#include "svc_burner.h"
#include <api/api.h>

#define CBCLASS svc_mediaRecorderI
START_DISPATCH;
  CB(ISSESSIONSUPPORTED,isSessionSupported)
  CB(ISMEDIASUPPORTED,isMediaSupported)
  CB(GETNUMDEVICES,getNumDevices)
  CB(ENUMDEVICE,enumDevice)
  VCB(REFRESHDEVICES,refreshDevices)
END_DISPATCH;
#undef CBCLASS

#define CBCLASS MediaRecorder::DeviceI
START_DISPATCH;
  CB(GETDEPENDENCYPTR,getDependencyPtr)
  CB(GETDEVICENAME,getDeviceName)
  CB(GETDEVICETYPE,getDeviceType)
  CB(GETDEVICEDESCRIPTION,getDeviceDescription)
  CB(ENUMDEVICESPEEDS,enumDeviceSpeeds)
  CB(GETMEDIASIZE,getMediaSize)
  CB(GETMEDIAFREE,getMediaFree)
  VCB(CLEARSESSIONS,clearSessions)
  CB(ADDSESSION,addSession)
  CB(GETSESSION,getSession)
  CB(SETRECORDSPEED,setRecordSpeed)
  CB(SETTEST,setTest)
  CB(SETCLOSEDISC,setCloseDisc)
  CB(CANBURNNOW,canBurnNow)
  CB(CANCANCEL,canCancel)
  CB(BEGIN,begin)
  CB(END,end)
  CB(CANCEL,cancel)
  CB(GETSTATUS,getStatus)
  CB(GETPROGRESS,getProgress)
  CB(GETSTATUSTEXT,getStatusText)
  CB(GETLASTERROR,getLastError)
END_DISPATCH;
#undef CBCLASS

#define CBCLASS MediaRecorder::SessionI
START_DISPATCH;
  CB(GETSESSIONTYPE,getSessionType)
  CB(CLOSESESSION,closeSession)
  CB(GETNUMENTRIES,getNumEntries)
  CB(ENUMENTRY,enumEntry)
  CB(GETTOTALBYTES,getTotalBytes)
  CB(GETTOTALTIME,getTotalTime)
END_DISPATCH;
#undef CBCLASS

const char *MediaRecorder::RedbookSession::enumEntry(int n) {
  if( n>=getNumEntries()) return NULL;
  return m_tracks[n]->getValue(); 
}

int MediaRecorder::RedbookSession::getTotalBytes() {
  double length=(double)getTotalTime();
  return (int)(length*(44100*4)/1000); //always 44khz 16bps stereo
}

int MediaRecorder::RedbookSession::getTotalTime() {
  int total=0;
  for(int i=0;i<getNumEntries();i++) {
    int length=0;
    if((length=api->metadb_getLength(m_tracks[i]->getValue()))!=-1) total+=length;
  }
  return total;
}
