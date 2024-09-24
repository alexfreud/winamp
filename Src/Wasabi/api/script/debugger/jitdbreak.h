#ifndef __JITDBREAKPOINT_H
#define __JITDBREAKPOINT_H

class MakiJITD;

class JITDBreakpoint {
  public:
    JITDBreakpoint(MakiJITD *jitd, int pointer);
    virtual ~JITDBreakpoint();

    virtual int getPointer();
    virtual void setPointer(int pointer);
    virtual void setEnabled(int e);
    virtual int isEnabled();

  private:
    MakiJITD *jitd;
    int pointer;
    int enabled;
};

#endif
