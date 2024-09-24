#ifndef __MakiDisassembler_H
#define __MakiDisassembler_H

#include <bfc/ptrlist.h>

class SourceCodeLine;
class SourceCodeLineI;

enum {
  OPCODE_TYPE_VOID = 0,
  OPCODE_TYPE_VAR,
  OPCODE_TYPE_PTR,
  OPCODE_TYPE_DLF,
  OPCODE_TYPE_NDLF,
  OPCODE_TYPE_CLASSID,
  OPCODE_TYPE_DISCARD,
};

class MakiDisassembler {
  public:
    MakiDisassembler(int vcpuid);
    virtual ~MakiDisassembler();

    int getVCPUId();
    int getNumLines();
    SourceCodeLine *enumLine(int n);
    int findLine(int pointer);

  private:
    void disassemble();
    PtrList<SourceCodeLineI> lines;
    int vcpuid;
    static int optable[256];
    static int optableready;
};


#endif