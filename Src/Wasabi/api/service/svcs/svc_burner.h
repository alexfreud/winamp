#ifndef _SVC_BURNER_H
#define _SVC_BURNER_H

#include <bfc/dispatch.h>
#include <bfc/depend.h>
#include <bfc/string/string.h>
#include <api/service/services.h>

namespace MediaRecorder {

class Session : public Dispatchable {
protected:
  Session() {} // protect constructor

public:
  int getSessionType() { return _call(GETSESSIONTYPE,0); }
  enum {
    Session_REDBOOK, Session_DATA, Session_ISO
  };
  int closeSession() { return _call(CLOSESESSION,0); }
  int getNumEntries() { return _call(GETNUMENTRIES,0); }
  const char *enumEntry(int n) { return _call(ENUMENTRY,(const char *)NULL,n); }
  int getTotalBytes() { return _call(GETTOTALBYTES,0); }
  int getTotalTime() { return _call(GETTOTALTIME,0); }

  enum {
    GETSESSIONTYPE=10,
    CLOSESESSION=20,
    GETNUMENTRIES=30,
    ENUMENTRY=40,
    GETTOTALBYTES=50,
    GETTOTALTIME=60,
  };
};

// this represents one session on the cd
// normal audio cds have 1 redbook session, mixed mode has 2 (or more), with
// 1 audio followed by 1 or more data
class SessionI : public Session {
public:
  virtual int getSessionType()=0;

  virtual int closeSession()=0;
  virtual int getNumEntries()=0;
  virtual const char *enumEntry(int n)=0;
  virtual int getTotalBytes()=0; //total space used in bytes
  virtual int getTotalTime()=0; //total time used in ms

protected:
  RECVS_DISPATCH;
};

class RedbookSession : public SessionI {
public:
  virtual ~RedbookSession() { m_tracks.deleteAll(); }

  int getSessionType() { return Session_REDBOOK; }

  int closeSession() { return 1; }

  void addEntry(const char *file) { m_tracks.addItem(new String(file)); }
  int getNumEntries() { return m_tracks.getNumItems(); }
  void removeEntry(int n) { m_tracks.deleteItem(n); }
  void clearEntries() { m_tracks.deleteAll(); }
 
  const char *enumEntry(int n);
  int getTotalBytes();
  int getTotalTime();

protected:
  PtrList<String> m_tracks;
};

// this is the physical device
class Device : public Dispatchable {
protected:
  Device() {} // protect constructor

public:
  static const GUID *depend_getClassGuid() {
    // {23F48039-455D-4348-86D5-0A82754678FC}
    static const GUID ret = 
    { 0x23f48039, 0x455d, 0x4348, { 0x86, 0xd5, 0xa, 0x82, 0x75, 0x46, 0x78, 0xfc } };
    return &ret;
  }

  api_dependent *getDependencyPtr() { return _call(GETDEPENDENCYPTR,(api_dependent *)NULL); }

  const char *getDeviceName() { return _call(GETDEVICENAME,(const char *)NULL); }
  const char *getDeviceType() { return _call(GETDEVICETYPE,(const char *)NULL); }
  const char *getDeviceDescription() { return _call(GETDEVICEDESCRIPTION,(const char *)NULL); }

  int enumDeviceSpeeds(int n) { return _call(ENUMDEVICESPEEDS,0,n); }

  int getMediaSize() { return _call(GETMEDIASIZE,0); }
  int getMediaFree() { return _call(GETMEDIAFREE,0); }

  void clearSessions() { _voidcall(CLEARSESSIONS); }
  int addSession(Session *session) { return _call(ADDSESSION,0,session); }
  Session *getSession(int num) { return _call(GETSESSION,(Session *)NULL,num); }

  int setRecordSpeed(int kbps) { return _call(SETRECORDSPEED,0,kbps); }
  int setTest(int testmode) { return _call(SETTEST,0,testmode); }
  int setCloseDisc(int closedisc) { return _call(SETCLOSEDISC,0,closedisc); }

  int canBurnNow() { return _call(CANBURNNOW,0); }
  int canCancel() { return _call(CANCANCEL,0); }

  int begin() { return _call(BEGIN,0); }
  int end() { return _call(END,0); }
  int cancel() { return _call(CANCEL,0); }

  int getStatus() { return _call(GETSTATUS,0); }
  enum {
    Status_IDLE, Status_PREPARING, Status_BURNING, Status_DONE,
  };

  int getProgress() { return _call(GETPROGRESS,0); }
  const char *getStatusText() { return _call(GETSTATUSTEXT,(const char *)NULL); }

  const char *getLastError() { return _call(GETLASTERROR,(const char *)NULL); }

  enum {
    Event_PREPAREBEGIN=100,
    Event_MEDIATRANSCODED=200, // params is item #
    Event_PREPAREEND=300,
    Event_BURNBEGIN=400,
    Event_ENTRYCOMPLETE=500,	// param is the position in bytes
    Event_SESSIONCOMPLETE=600,
    Event_BURNEND=700,
    Event_ERROR=800,
    Event_MEDIACHANGE=900,	// user put in a different disc
  };

  enum {
    GETDEPENDENCYPTR=10,
    GETDEVICENAME=20,
    GETDEVICETYPE=30,
    GETDEVICEDESCRIPTION=40,
    ENUMDEVICESPEEDS=50,
    GETMEDIASIZE=60,
    GETMEDIAFREE=70,
    CLEARSESSIONS=80,
    ADDSESSION=90,
    GETSESSION=100,
    SETRECORDSPEED=110,
    SETTEST=120,
    SETCLOSEDISC=130,
    CANBURNNOW=140,
    CANCANCEL=150,
    BEGIN=160,
    END=170,
    CANCEL=180,
    GETSTATUS=190,
    GETPROGRESS=200,
    GETSTATUSTEXT=210,
    GETLASTERROR=220,
  };
};

class DeviceI : public Device {
public:
  virtual api_dependent *getDependencyPtr()=0;	// for events

  virtual const char *getDeviceName()=0;	// internal device name
  virtual const char *getDeviceType()=0;	// "CD-R", "CD-RW", "DVD-R" etc
  virtual const char *getDeviceDescription()=0; // user readable string

  virtual int enumDeviceSpeeds(int n)=0;	// in kb/s

  virtual int getMediaSize()=0;			// total space in bytes
  virtual int getMediaFree()=0;			// free space in bytes

  virtual void clearSessions()=0;
  virtual int addSession(Session *session)=0;
  virtual Session *getSession(int num)=0;

  virtual int setRecordSpeed(int kbps)=0; //kbps==0 means max speed
  virtual int setTest(int testmode)=0; // if true, don't really burn
  virtual int setCloseDisc(int closedisc)=0; // if true, close entire disc at end

  virtual int canBurnNow()=0;	// return 1 if everything's ready
  virtual int canCancel()=0;	// return 1 if we can cancel (during burning)

  virtual int begin()=0;
  virtual int end()=0;
  virtual int cancel()=0;

  virtual int getStatus()=0;
  virtual int getProgress()=0;	// # of bytes written
  virtual const char *getStatusText()=0; // like "xx% complete" or something

  virtual const char *getLastError()=0;

protected:
  RECVS_DISPATCH;
};

};	// end namespace MediaRecorder

//don't override this one
class NOVTABLE svc_mediaRecorder : public Dispatchable {
protected:
  svc_mediaRecorder() {} // protect constructor

public:
  static FOURCC getServiceType() { return WaSvc::MEDIARECORDER; }

  int isSessionSupported(MediaRecorder::Session *session) { return _call(ISSESSIONSUPPORTED,0,session); }
  int isMediaSupported(const char *medianame) { return _call(ISMEDIASUPPORTED,0,medianame); }

  int getNumDevices() { return _call(GETNUMDEVICES,0); }
  MediaRecorder::Device *enumDevice(int n) { return _call(ENUMDEVICE,(MediaRecorder::Device*)NULL,n); }

  void refreshDevices() { _voidcall(REFRESHDEVICES); }

  enum {
    ISSESSIONSUPPORTED=10,
    ISMEDIASUPPORTED=20,
    GETNUMDEVICES=30,
    ENUMDEVICE=40,
    REFRESHDEVICES=50,
  };
};

// this should be implemented by a given burning lib
class NOVTABLE svc_mediaRecorderI : public svc_mediaRecorder {
public:
  static FOURCC getServiceType() { return WaSvc::MEDIARECORDER; }

  virtual int isSessionSupported(MediaRecorder::Session *session)=0;
  virtual int isMediaSupported(const char *medianame)=0;// "CD-R", "DVD-R", etc.

  virtual int getNumDevices()=0;
  virtual MediaRecorder::Device *enumDevice(int n)=0;

  virtual void refreshDevices()=0;

protected:
  RECVS_DISPATCH;
};

#endif