#ifndef _SVC_MEDIACONVERTER_H
#define _SVC_MEDIACONVERTER_H

#include <bfc/dispatch.h>

#include <api/core/chunklist.h>
#include <api/core/mediainfo.h>
#include <api/syscb/callbacks/corecbi.h>

#include <api/service/services.h>

// do NOT derive from these ones
class NOVTABLE svc_mediaConverter : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::MEDIACONVERTER; }

  // test if the converter can handle that stream, filename or chunktype:
  // if your converter can accept streams depending on the data stream you should
  // test the file thru the reader interface (like to test if the file header is ok)
  // if your converter can accept files thru a test on the filename string, you should
  // test the name string (like tone://... or *.wav)
  // if your converter can accept depending on the chunktype, you should test the chunktype
  // (like MP3, PCM, etc...)
  // returns 1 if ok
  int canConvertFrom(svc_fileReader *reader, const char *name, const char *chunktype) { return _call(CANCONVERTFROM,0,reader,name,chunktype); }

  // returns the chunk type that the converter will convert to
  // (PCM, MP3, etc...)
  const char *getConverterTo() { return _call(GETCONVERTERTO,""); }

  // override this one if your converter can decode depending on a "Content-Type:" HTTP header
  int canSupportContentType(const char *contenttype) { return _call(CANSUPPORTCONTENTTYPE,0,contenttype); }

  // fills up the infos class
  int getInfos(MediaInfo *infos) { return _call(GETINFOS,0,infos); }
  
  // writes back the infos on the file.
  // note: the reader you get in the infos class has the filename opened in read/write mode
  // return 1 if update succeeded, 0 if error
  int setInfos(MediaInfo *infos) { return _call(SETINFOS,0,infos); }

  // returns the id of the xml group the media info editor will use for displaying/editing infos
  const char *getInfosXmlGroup() { return _call(GETINFOSXMLGROUP,(const char *)NULL); }

  // process current file data
  // returns 1 if ok, 0 when file/stream has ended
  int processData(MediaInfo *infos, ChunkList *chunk_list, bool *killswitch) { return _call(PROCESSDATA,0,infos,chunk_list,killswitch); }

  // returns the current position in ms
  // usually the position is automatically calculated with the amount of PCM data converters send back to the core
  // so you don't have to override this function. however, if you want to force the position, return a value other than -1
  int getPosition(void) { return _call(GETPOSITION,-1); }

  // returns the latency of the converter in ms
  int getLatency(void) { return _call(GETLATENCY,0); }

  // sort function to be able to specify where you want your converter placed in the converter list
  // (like if you want to be before or after the crossfader for example, override this function)
  // return -1 to be placed before "otherconverter", 1 to be placed after "otherconverter", otherwise 0
  int sortPlacement(const char *otherconverter) { return _call(SORTPLACEMENT,0,otherconverter); }

  // return 1 if you want this converter to be selectable as output in prefs->audio
  int isSelectableOutput(void) { return _call(ISSELECTABLEOUTPUT,0); }

  // message received by sendConvertersMsg() in the core
  int onCoreUserMsg(const char *msg, const char *value) { return _call(ONCOREUSERMSG,0,msg,value); }

  
  // internally used by wasabi
  CoreCallback *getCoreCallback(void) { return _call(GETCORECALLBACK,(CoreCallback *)0); }
  void setCoreToken(CoreToken t) { _voidcall(SETCORETOKEN,t); }

  enum {
    CANCONVERTFROM=10,
    GETCONVERTERTO=20,
    GETINFOS=30,
    PROCESSDATA=40,
    GETPOSITION=50,
    GETLATENCY=60,
    GETCORECALLBACK=70,
    SORTPLACEMENT=80,
    CANSUPPORTCONTENTTYPE=90,
    SETCORETOKEN=100,
    SETINFOS=110,
    GETINFOSXMLGROUP=120,
    ISSELECTABLEOUTPUT=130,
    ONCOREUSERMSG=140,
  };
};

class NOVTABLE svc_mediaConverterNI : public svc_mediaConverter {
public:
  virtual int canConvertFrom(svc_fileReader *reader, const char *name, const char *chunktype)=0;
  virtual const char *getConverterTo()=0;
  virtual int canSupportContentType(const char *contenttype) { return 0; }

  virtual int getInfos(MediaInfo *infos)=0;

  virtual int setInfos(MediaInfo *infos)=0;

  virtual const char *getInfosXmlGroup()=0;

	virtual int processData(MediaInfo *infos, ChunkList *chunk_list, bool *killswitch)=0;

  virtual int getPosition(void) { return -1; }

  virtual int getLatency(void) { return 0; }

  virtual int sortPlacement(const char *otherconverter) { return 0; }

  virtual int isSelectableOutput(void) { return 0; }

  virtual int onCoreUserMsg(const char *msg, const char *value) { return 0; }

  virtual CoreCallback *getCoreCallback(void)=0;

  virtual void setCoreToken(CoreToken t)=0;
protected:
  RECVS_DISPATCH;
};

// derive from this one
class NOVTABLE svc_mediaConverterI : public svc_mediaConverterNI, public CoreCallbackI {
public:
  svc_mediaConverterI() : m_coretoken(0) { }

  virtual int canConvertFrom(svc_fileReader *reader, const char *name, const char *chunktype)=0;
  virtual const char *getConverterTo()=0;
  virtual int canSupportContentType(const char *contenttype) { return 0; }

  virtual int getInfos(MediaInfo *infos)=0;

  virtual int setInfos(MediaInfo *infos) { return 0; }

  virtual const char *getInfosXmlGroup() { return (const char *)NULL; }

	virtual int processData(MediaInfo *infos, ChunkList *chunk_list, bool *killswitch)=0;

  virtual int getPosition(void) { return -1; }

  virtual int getLatency(void) { return 0; }

  virtual int sortPlacement(const char *otherconverter) { return 0; }

  virtual int isSelectableOutput(void) { return 0; }

  virtual int onCoreUserMsg(const char *msg, const char *value) { return 0; }

  virtual CoreCallback *getCoreCallback(void) { return this; }

  virtual void setCoreToken(CoreToken t) { m_coretoken=t; }

protected:
  CoreToken m_coretoken;
};

#include <api/service/servicei.h>
template <class T>
class MediaConverterCreator : public waServiceFactoryT<svc_mediaConverter, T> { };

#endif
