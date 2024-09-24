#ifndef __CONSOLE_H
#define __CONSOLE_H

class ConsoleEnum;

class Console {
  public:

    static void outputString(int severity, const char *string);
    static void reset();

  private:

    static ConsoleEnum *console;
    static int needscan;
    static int noconsole;
};

#endif
