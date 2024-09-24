#ifndef _CBMGR_H
#define _CBMGR_H

#include <bfc/std.h>

class SysCallback;
class WaComponent;

class CallbackManager {
public:
  static void registerCallback(SysCallback *cb, void *param, WaComponent *owner);
  static void deregisterCallback(SysCallback *cb, WaComponent *owner);

  static void issueCallback(int eventtype, int msg, intptr_t param1 = 0, intptr_t param2 = 0);
  static int getNumCallbacks();
};

#endif
