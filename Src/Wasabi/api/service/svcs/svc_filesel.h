#ifndef _SVC_FILESEL_H
#define _SVC_FILESEL_H

#include <bfc/dispatch.h>
#include <api/service/services.h>

class ifc_window;

class NOVTABLE svc_fileSelector : public Dispatchable {
public:
  static FOURCC getServiceType() { return WaSvc::FILESELECTOR; }

  int testPrefix(const wchar_t *prefix) {
    return _call(TESTPREFIX, 0, prefix);
  }
  const wchar_t *getPrefix() {
    return _call(GETPREFIX, L"");
  }
  int setTitle(const wchar_t *title) {
    return _call(SETTITLE, 0, title);
  }
  int setExtList(const wchar_t *ext) {
    return _call(SETEXTLIST, 0, ext);
  }
  int runSelector(ifc_window *parWnd, int type, int allow_multiple, const wchar_t *ident=NULL, const wchar_t *default_dir=NULL) {
    return _call(RUNSELECTOR, 0, parWnd, type, allow_multiple, ident, default_dir);
  }
  int getNumFilesSelected() {
    return _call(GETNUMFILESSELECTED, 0);
  }
  const wchar_t *enumFilename(int n) {
    return _call(ENUMFILENAME, L"", n);
  }
  const wchar_t *getDirectory() {
    return _call(GETDIRECTORY, L"");
  }

protected:
  enum {
    TESTPREFIX=0,
    GETPREFIX=10,
    SETTITLE=20,
    SETEXTLIST=30,
    RUNSELECTOR=40,
    GETNUMFILESSELECTED=50,
    ENUMFILENAME=60,
    GETDIRECTORY=70,
  };
};

namespace FileSel {
  enum {
    OPEN=1, SAVEAS=2,
  };
};


class NOVTABLE svc_fileSelectorI : public svc_fileSelector {
public:
  virtual int testPrefix(const wchar_t *prefix)=0;
  virtual const wchar_t *getPrefix()=0;
  virtual int setTitle(const wchar_t *title) { return 0; }
  virtual int setExtList(const wchar_t *ext) { return 0; }
  virtual int runSelector(ifc_window *parWnd, int type, int allow_multiple, const wchar_t *ident=NULL, const wchar_t *default_dir=NULL)=0;
  virtual int getNumFilesSelected()=0;
  virtual const wchar_t *enumFilename(int n)=0;
  virtual const wchar_t *getDirectory()=0;

protected:
  RECVS_DISPATCH;
};

#include <api/service/svc_enum.h>
#include <bfc/string/StringW.h>

class FileSelectorEnum : public SvcEnumT<svc_fileSelector> {
public:
  FileSelectorEnum(const wchar_t *_prefix=L"files") : prefix(_prefix) { }

protected:
  virtual int testService(svc_fileSelector *svc) {
    return prefix.isempty() || svc->testPrefix(prefix);
  }

private:
  StringW prefix;
};

#endif
