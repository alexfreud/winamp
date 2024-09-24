
#include <svc_console.h>

#define CBCLASS svc_consoleI
START_DISPATCH;
  CB(ACTIVATED, activated);
  CB(OUTPUTSTRING, outputString);
END_DISPATCH;
#undef CBCLASS

