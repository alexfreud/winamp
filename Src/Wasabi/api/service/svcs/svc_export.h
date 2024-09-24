#ifndef _SVC_EXPORT_H
#define _SVC_EXPORT_H

#include <bfc/dispatch.h>
#include <api/service/services.h>

class svc_fileReader;

class svc_exporter /*: public Dispatchable*/ {
public:
  static FOURCC getServiceType() { return WaSvc::EXPORTER; }

  virtual int isMine(const char *exportid, const char *family)=0;

  virtual svc_fileReader *open()=0;
  virtual close(svc_fileReader *reader)=0;
};

class ExporterEnum : public SvcEnumT<svc_exporter> {
public:
  ExporterEnum(const char *exportid, const char *family=NULL) :
   id(exportid), fam(family) { }

  virtual int testService(svc_exporter *svc) {
    return svc->isMine(id, fam);
  }

private:
  String id, fam;
};

#endif
