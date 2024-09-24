#ifndef _SVC_METADATA_H
#define _SVC_METADATA_H

// see helper class in common/metadatasvc.h to implement this

#include <api/service/services.h>
#include <bfc/dispatch.h>
#include <bfc/string/string.h>
#include <api/db/metatags.h>

class NOVTABLE svc_metaData : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::METADATA; }
  
  int enumMetaData(int n, char *mdName, int mdNameMaxLength, int *mdType, BOOL *mdIndexed, BOOL *mdMaintainUniques);
  int hasMetaData(const char *name, int type);
  int allowOperation(GUID whom, const char *playstring, int op, char *field, void *data, int data_len);
  GUID getGUID();
  const char *getMetaTableName();

  enum {
    HASMETADATA=10,
    ENUMMETADATA=20,
    ALLOWOPERATION=30,
    GETGUID=40,
    GETMETANAME=50,
  };

};

inline int svc_metaData::hasMetaData(const char *name, int type) {
  return _call(HASMETADATA, 0, name, type);
}

inline int svc_metaData::enumMetaData(int n, char *mdName, int mdNameMaxLength, int *mdType, BOOL *mdIndexed, BOOL *mdMaintainUniques) {
  return _call(ENUMMETADATA, 0, n, mdName, mdNameMaxLength, mdType, mdIndexed, mdMaintainUniques);
}

inline int svc_metaData::allowOperation(GUID whom, const char *playstring, int op, char *field, void *data, int data_len) {
  return _call(ALLOWOPERATION, 0, whom, playstring, op, field, data, data_len);
}

inline GUID svc_metaData::getGUID() {
  return _call(GETGUID, INVALID_GUID);
}

inline const char *svc_metaData::getMetaTableName() {
  return _call(GETMETANAME, (const char *)NULL);
}

// see helper class in common/metadatasvc.h to implement this
class NOVTABLE svc_metaDataI : public svc_metaData {
public:
  virtual int enumMetaData(int n, char *mdName, int mdNameMaxLength, int *mdType, BOOL *mdIndexed, BOOL *mdMaintainUniques)=0;
  virtual int hasMetaData(const char *name, int type)=0;
  virtual int allowOperation(GUID whom, const char *playstring, int op, char *field, void *data, int data_len)=0;
  virtual GUID getGUID()=0;
  virtual const char *getMetaTableName()=0;

protected:
  RECVS_DISPATCH;
};

#include <api/service/servicei.h>

// if you want to use class MetaDataSvc (instead of deriving from
// svc_metaDataI yourself) use class MetaDataSvcFactory in common/metadatasvc.h
template <class T>
class MetaDataSvcCreator : public waServiceFactoryTSingle<svc_metaData, T> {};

#include <api/service/svc_enum.h>

class MetaDataSvcEnum : public SvcEnumT<svc_metaData> {
public:
  MetaDataSvcEnum(const char *metadata=NULL, int mdtype=MDT_NONE) : md(metadata), type(mdtype) {}
protected:
  virtual int testService(svc_metaData *svc) {
    if (md.isempty() || type == MDT_NONE) return TRUE;
    return svc->hasMetaData(md, type);
  }
private:
  String md;
  int type;
};

class MetaDataSvcEnumGuid : public SvcEnumT<svc_metaData> {
public:
  MetaDataSvcEnumGuid(GUID g) : guid(g) {}
protected:
  virtual int testService(svc_metaData *svc) {
    return (svc->getGUID() == guid);
  }
private:
  GUID guid;
};

#endif
