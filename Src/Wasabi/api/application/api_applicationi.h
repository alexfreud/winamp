#ifndef __API_APPLICATIONI_IMPL_H
#define __API_APPLICATIONI_IMPL_H

/*<?<autoheader/>*/
#include "api_application.h"
#include "api_applicationx.h"
/*?>*/

/*[interface.header.h]
#include "common/nsGUID.h"
class String;
*/

class api_applicationI : public api_applicationX {
public:
  NODISPATCH api_applicationI(HINSTANCE instance, const wchar_t *_userPath);
  NODISPATCH virtual ~api_applicationI();

  DISPATCH(10) const wchar_t *main_getAppName();
  DISPATCH(20) const wchar_t *main_getVersionString();
  DISPATCH(30) unsigned int main_getBuildNumber();
  DISPATCH(40) GUID main_getGUID();
  DISPATCH(50) HANDLE main_getMainThreadHandle();
  DISPATCH(60) HINSTANCE main_gethInstance();
  DISPATCH(70) const wchar_t *main_getCommandLine();
  DISPATCH(80) void main_shutdown(int deferred = TRUE);
  DISPATCH(90) void main_cancelShutdown();
  DISPATCH(100) int main_isShuttingDown();
  DISPATCH(110) const wchar_t *path_getAppPath();
  DISPATCH(120) const wchar_t *path_getUserSettingsPath();
  DISPATCH(130) int app_getInitCount();
  DISPATCH(140) int app_messageLoopStep();

  NODISPATCH void setHInstance(HINSTANCE instance);
  NODISPATCH void setCommandLine(const wchar_t *cmdline);
  NODISPATCH void setGUID(GUID g);

protected:
    
  HINSTANCE appInstance;
  StringW cmdLine;
  StringW userPath;
  HANDLE mainthread;
  GUID guid;
  int shuttingdown;
  StringW apppath;
};

#endif // __API_APPLICATIONI_IMPL_H
