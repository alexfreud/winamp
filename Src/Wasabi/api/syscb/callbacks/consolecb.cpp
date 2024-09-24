#include <precomp.h>
#if 0
#include "consolecb.h"

using namespace ConsoleCallback;

int ConsoleCallbackI::syscb_notify(int msg, int param1, int param2) {
  switch (msg) {
    case DEBUGMESSAGE:
      return consolecb_outputString(param1, reinterpret_cast<const char *>(param2));
  }
  return 0;
}
#endif