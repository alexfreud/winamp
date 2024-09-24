#ifndef _WKC_H
#define _WKC_H

#include <bfc/dispatch.h>

/**
  A callback class for the loader so it can determine which resources wasabi.dll should load.
*/
class NOVTABLE WasabiKernelController : public Dispatchable {
public:
/**
  Check if a component should load.
*/
  int testComponent(const char *filename);
  int testScript(const char *filename, void *data, int datalen);
  int testSkin(const char *skinname);
  int testSkinFile(const char *filename);

protected:
  enum {
    TESTCOMPONENT=1000,
    TESTSCRIPT=2000,
    TESTSKIN=3000,
    TESTSKINFILE=4000,
  };
};

inline
int WasabiKernelController::testComponent(const char *filename) {
  return _call(TESTCOMPONENT, TRUE, filename);
}

inline
int WasabiKernelController::testScript(const char *filename, void *data, int datalen) {
  return _call(TESTSCRIPT, TRUE, filename, data, datalen);
}

inline
int WasabiKernelController::testSkin(const char *skinname) {
  return _call(TESTSKIN, TRUE, skinname);
}

inline
int WasabiKernelController::testSkinFile(const char *filename) {
  return _call(TESTSKINFILE, TRUE, filename);
}

// implementors derive from this one
class WasabiKernelControllerI : public WasabiKernelController {
public:
  // default to OK
  virtual int testComponent(const char *filename) { return 1; }
  virtual int testScript(const char *filename, void *data, int datalen) { return 1; }
  virtual int testSkin(const char *skinname) { return 1; }
  virtual int testSkinFile(const char *filename) { return 1; }

protected:
  RECVS_DISPATCH;
};

#endif
