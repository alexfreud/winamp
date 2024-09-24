#include <precomp.h>
#include "debugsymbols.h"
#include <api/script/debugger/disasm.h>
#include <api/script/scriptmgr.h>
#include <api/script/debugger/sourcecodeline.h>
#include <api/script/vcpu.h>

DebugSymbols::DebugSymbols(int _vcpuid) : disasm(_vcpuid)
{
  gotsymbols = 0;
  SystemObject *so = SOM::getSystemObjectByScriptId(_vcpuid);
  if (so != NULL)
		binaryfilename = so->getFilename();

  VCPUcodeBlock *cb = VCPU::getCodeBlockEntry(_vcpuid);
  if (cb->debugsize != 0) {
    gotsymbols = 1;
    char *p = cb->debugsymbols;
    int n = *(int *)p; p += 4;
    while (n--) {
      int s = *(int *)p; p += 4;
      wchar_t *m = WMALLOC(s+1);
      MEMCPY(m, p, s*sizeof(wchar_t));
      m[s] = 0;
			StringW *temp = new StringW;
			temp->own(m);
      //files.addItem(new String(m));
			files.addItem(temp);
      //FREE(m);
      p+=s;
    }
    n = *(int *)p; p += 4;
    while (n--) {
      SourceCodeLineI *l = new SourceCodeLineI();
      l->setPointer(*(int *)p); p += 4;
      l->setSourceFile(files[*(int *)p]->getValue()); p += 4;
      l->setSourceFileLine(*(int *)p); p += 4;
      SourceCodeLineI *last = lines.getLast();
      if (last != NULL) last->setLength(l->getPointer()-last->getPointer());
      lines.addItem(l);
    }
    SourceCodeLineI *last = lines.getLast();
    if (last != NULL) last->setLength(cb->size - last->getPointer());
  }
}

DebugSymbols::~DebugSymbols() 
{
  files.deleteAll();
  lines.deleteAll();
}

int DebugSymbols::getNumLines() {
  if (!gotsymbols) return disasm.getNumLines();
  return lines.getNumItems();
}

int DebugSymbols::findLine(int pointer) {
  if (!gotsymbols) return disasm.findLine(pointer);
  int i;
  for (i=0;i<lines.getNumItems();i++) {
    SourceCodeLine *l = lines.enumItem(i);
    int ip = l->getPointer();
    int il = l->getLength();
    if (pointer >= ip && pointer < ip+il) {
      return i;
    }
  }
  return -1;
}

SourceCodeLine *DebugSymbols::enumLine(int n) {
  if (!gotsymbols) return disasm.enumLine(n);
  return lines.enumItem(n);  
}
