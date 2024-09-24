#ifndef _SVC_ITEMMGR_H
#define _SVC_ITEMMGR_H

#include <bfc/dispatch.h>
#include <api/service/services.h>

class NOVTABLE svc_itemMgr : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::ITEMMANAGER; }

  int isMine(const char *playstring) { return _call(ISMINE, 0, playstring); }

  int optimizePlaystring(char *playstring) {
    return _call(OPTIMIZEPLAYSTRING, 0, playstring);
  }
  int createInitialName(const char *playstring, char *buf, int buflen) {
    return _call(CREATEINITIALNAME, 0, playstring, buf, buflen);
  }
  int optimizeFileData(const char *playstring, const char *fieldname, int datatype, char *data, int datalen) {
    return _call(OPTIMIZEFILEDATA, -1, playstring, fieldname, datatype, data, datalen);
  }

  int onDatabaseAdd(const char *playstring) {
    return _call(ONDATABASEADD, 0, playstring);
  }
  int onDatabaseDel(const char *playstring) {
    return _call(ONDATABASEDEL, 0, playstring);
  }

  //return 1 if changed
  int onTitleChange(const char *playstring, const char *newtitle) {
    return _call(ONTITLECHANGE, 0, playstring, newtitle);
  }
  int onTitle2Change(const char *playstring, const char *newtitle) {
    return _call(ONTITLE2CHANGE, 0, playstring, newtitle);
  }

  void onNextFile(const char *playstring) {
    _voidcall(ONNEXTFILE, playstring);
  }

  void onFileComplete(const char *playstring) {
    _voidcall(ONFILECOMPLETE, playstring);
  }

  int wantScanData(const char *playstring) {
    return _call(WANTSCANDATA, 1, playstring);
  }

  int getSortOrder() {
    return _call(GETSORTORDER, 0);
  }

  enum {
    ISMINE=100,
    OPTIMIZEPLAYSTRING=200,
    OPTIMIZEFILEDATA=211,	//210 retired
    CREATEINITIALNAME=300,
    ONDATABASEADD=400,
    ONDATABASEDEL=401,
    ONTITLECHANGE=600,
    ONTITLE2CHANGE=601,
    ONNEXTFILE=700,
    ONFILECOMPLETE=800,
    WANTSCANDATA=900,
    GETSORTORDER=1000,
  };
};

// derive from this one
class NOVTABLE svc_itemMgrI : public svc_itemMgr {
public:
  virtual int isMine(const char *playstring)=0;
  virtual int optimizePlaystring(char *playstring) { return 0; }
  virtual int createInitialName(const char *playstring, char *buf, int buflen) { return 0; }
  virtual int optimizeFileData(const char *playstring, const char *fieldname, int datatype, char *data, int datalen) { return -1; }
  virtual int onDatabaseAdd(const char *playstring) { return 0; }
  virtual int onDatabaseDel(const char *playstring) { return 0; }
  virtual int onTitleChange(const char *playstring, const char *newtitle) { return 0; }
  virtual int onTitle2Change(const char *playstring, const char *newtitle) { return 0; }
  virtual void onNextFile(const char *playstring) { }
  virtual void onFileComplete(const char *playstring) { }

  virtual int wantScanData(const char *playstring) { return 1; }

  virtual int getSortOrder() { return 0; }

protected:
  RECVS_DISPATCH;
};

#include <bfc/named.h>
#include <api/service/svc_enum.h>

class ItemMgrEnum : private Named, public SvcEnumT<svc_itemMgr> {
public:
  ItemMgrEnum(const char *ps) : Named(ps) { }

  void setPlaystring(const char *ps) { Named::setName(ps); }

protected:
  virtual int testService(svc_itemMgr *svc) {
    return svc->isMine(getName());
  }
};

#include <api/service/servicei.h>

template <class T>
class ItemMgrCreator : public waServiceFactoryTSingle<svc_itemMgr, T> { };

#endif
