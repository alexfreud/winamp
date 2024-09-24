#include "console.h"
#include <api/service/svcs/svc_console.h>
#include <api/service/svc_enum.h>

void Console::outputString(int severity, const char *string) {
  if (!console) {
    console = new ConsoleEnum;
  }

  if (needscan) {
    needscan=0;
    console->reset();
    svc_console *con = console->getNext();
    noconsole = (con == NULL);
  }
  if (noconsole) return;

  console->reset();
  svc_console *con = console->getNext();
  while (con) {
    con->outputString(severity, string);
    con = console->getNext();
  }
}

void Console::reset() {
  needscan=1;
}

int Console::needscan=1;
int Console::noconsole=0;

ConsoleEnum *Console::console = NULL;
