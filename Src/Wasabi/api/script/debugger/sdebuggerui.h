#ifndef __SIMPLEDEBUGGERUI_H
#define __SIMPLEDEBUGGERUI_H

#include "debuggerui.h"
#include <api/wnd/wndclass/clickwnd.h>

class MakiDisassembler;
class EditWnd;
class String;
class SourceCodeLine;

#define SimpleDebuggerUI_PARENT ClickWnd

enum {
  DEBUG_CMD_BREAKPOINT = 0,
  DEBUG_CMD_CONTINUE,
  DEBUG_CMD_STEPINTO,
  DEBUG_CMD_STEPOVER,
  DEBUG_CMD_STEPOUT,
  DEBUG_CMD_KILL,
  DEBUG_CMD_HELP,
};


class MakiJITD;

class SimpleDebuggerUI : public SimpleDebuggerUI_PARENT, public DebuggerUII {
  public:
    SimpleDebuggerUI();
    virtual ~SimpleDebuggerUI();

    virtual int onPaint(Canvas *c);
    virtual int onLeftButtonDown(int x, int y);
    virtual int onResize();
    virtual int childNotify(ifc_window *child, int msg, intptr_t p1, intptr_t p2);
    virtual void onSetVisible(int show);

    virtual int messageLoop();
    virtual void setJITD(MakiJITD *jitd);

    virtual void disassemble(Canvas *c);
    virtual void onCommand(const wchar_t *cmd);

    virtual void addBreakPoint(const wchar_t *pointer_ascii);
    virtual void continueExecution();
    virtual void stepInto();
    virtual void stepOver();
    virtual void killScript();
    virtual void showHelp();
    virtual int onGetFocus();

    virtual int evaluate(const wchar_t *ascii);
    virtual const wchar_t *getLine(const wchar_t *filename, int fileline);

  private:
    
    int leave;
    MakiJITD *jitd;
    EditWnd *edit;
    wchar_t cmdbuf[256];
    int retcode;
    PtrList<StringW> strstack;
};

#endif