#ifndef __DEBUGSYMBOLS_H
#define __DEBUGSYMBOLS_H

#include <bfc/ptrlist.h>
#include <bfc/string/StringW.h>
#include "disasm.h"

class SourceCodeLine;
class SourceCodeLineI;
class MakiDisassembler;

class DebugSymbols {

	public:

    DebugSymbols(int vcpuid);
    virtual ~DebugSymbols();

    virtual int getNumLines();
    virtual int findLine(int pointer);
    virtual SourceCodeLine *enumLine(int n);

  private:

    PtrList<SourceCodeLineI> lines;
    MakiDisassembler disasm;
    StringW binaryfilename;
    int gotsymbols;
    PtrList<StringW> files;
    //PtrList<MakiVariable> vars;
};






#endif