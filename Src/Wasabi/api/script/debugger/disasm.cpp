#include <precomp.h>
#include <api/script/debugger/disasm.h>
#include <api/script/opcodes.h>
#include <api/script/vcpu.h>
#include <api/script/debugger/sourcecodeline.h>
// ---------------------------------------------------------------------

typedef struct {
  const wchar_t *opname;
  unsigned __int8 opval;
  int type;
} opentry;

opentry _optable[] = {
 {L"nop",   OPCODE_NOP,   OPCODE_TYPE_VOID},
 {L"push",  OPCODE_PUSH,   OPCODE_TYPE_VAR},
 {L"popi",  OPCODE_POPI,   OPCODE_TYPE_VOID},
 {L"pop",   OPCODE_POP,    OPCODE_TYPE_VAR},
 {L"set ",  OPCODE_SET,    OPCODE_TYPE_VOID},
 {L"retf",  OPCODE_RETF,   OPCODE_TYPE_VOID},
 {L"call",  OPCODE_CALLC,  OPCODE_TYPE_PTR},
 {L"call",  OPCODE_CALLM,  OPCODE_TYPE_DLF},
 {L"call",  OPCODE_CALLM2, OPCODE_TYPE_NDLF},
 {L"umv",   OPCODE_UMV,    OPCODE_TYPE_DISCARD},
 {L"cmpeq", OPCODE_CMPEQ,  OPCODE_TYPE_VOID},
 {L"cmpne", OPCODE_CMPNE,  OPCODE_TYPE_VOID},
 {L"cmpa",  OPCODE_CMPA,   OPCODE_TYPE_VOID},
 {L"cmpae", OPCODE_CMPAE,  OPCODE_TYPE_VOID},
 {L"cmpb",  OPCODE_CMPB,   OPCODE_TYPE_VOID},
 {L"cmpbe", OPCODE_CMPBE,  OPCODE_TYPE_VOID},
 {L"jiz",   OPCODE_JIZ,    OPCODE_TYPE_PTR},
 {L"jnz",   OPCODE_JNZ,    OPCODE_TYPE_PTR},
 {L"jmp",   OPCODE_JMP,    OPCODE_TYPE_PTR},
 {L"incs",  OPCODE_INCS,   OPCODE_TYPE_VOID},
 {L"decs",  OPCODE_DECS,   OPCODE_TYPE_VOID},
 {L"incp",  OPCODE_INCP,   OPCODE_TYPE_VOID},
 {L"decp",  OPCODE_DECP,   OPCODE_TYPE_VOID},
 {L"add",   OPCODE_ADD,    OPCODE_TYPE_VOID},
 {L"sub",   OPCODE_SUB,    OPCODE_TYPE_VOID},
 {L"mul",   OPCODE_MUL,    OPCODE_TYPE_VOID},
 {L"div",   OPCODE_DIV,    OPCODE_TYPE_VOID},
 {L"mod",   OPCODE_MOD,    OPCODE_TYPE_VOID},
 {L"neg",   OPCODE_NEG,    OPCODE_TYPE_VOID},
 {L"shl",   OPCODE_SHL,    OPCODE_TYPE_VOID},
 {L"shr",   OPCODE_SHR,    OPCODE_TYPE_VOID},
 {L"bnot",  OPCODE_BNOT,   OPCODE_TYPE_VOID},
 {L"bxor",  OPCODE_XOR,    OPCODE_TYPE_VOID},
 {L"band",  OPCODE_AND,    OPCODE_TYPE_VOID},
 {L"bor",   OPCODE_OR,     OPCODE_TYPE_VOID},
 {L"not",   OPCODE_NOT,    OPCODE_TYPE_VOID},
 {L"and",   OPCODE_LAND,   OPCODE_TYPE_VOID},
 {L"or",    OPCODE_LOR,    OPCODE_TYPE_VOID},
 {L"del",   OPCODE_DELETE, OPCODE_TYPE_VOID},
 {L"new",   OPCODE_NEW,    OPCODE_TYPE_CLASSID},
 {L"cmpl",  OPCODE_CMPLT,  OPCODE_TYPE_VOID},
};

int MakiDisassembler::optable[256];
int MakiDisassembler::optableready = 0;


MakiDisassembler::MakiDisassembler(int _vcpuid) {
  if (!optableready) {
    MEMSET(optable, 0, sizeof(optable));
    for (int i=0;i<sizeof(_optable)/sizeof(opentry);i++) {
      opentry e = _optable[i];
      optable[e.opval] = i;
    }
    optableready = 1;
  }
  vcpuid = _vcpuid;
  disassemble();
}

MakiDisassembler::~MakiDisassembler() {
  lines.deleteAll();
}

int MakiDisassembler::getVCPUId() {
  return vcpuid;
}

int MakiDisassembler::getNumLines() {
  return lines.getNumItems();
}

SourceCodeLine *MakiDisassembler::enumLine(int n) {
  return lines.enumItem(n);
}

int MakiDisassembler::findLine(int pointer) {
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

  
void MakiDisassembler::disassemble() {
  int size;
  const char *codeblock = VCPU::getCodeBlock(vcpuid, &size);
  if (codeblock != NULL) {

    const char *p = codeblock;

    unsigned char opcode = OPCODE_NOP;

    while (p < codeblock+size) {
      const char *start_of_instruction = p;
      opcode = *p;
      p+=sizeof(opcode);

      StringW inst;
      int a = optable[opcode];
      int id;
      int size = 0;
      inst = _optable[a].opname;
      switch (_optable[a].type) {
        case OPCODE_TYPE_VOID:
          size = 1;
          break;
        case OPCODE_TYPE_VAR:
          id = *(int *)p; p+=sizeof(int);
          inst += StringPrintfW(L" var %08X", id);
          size = 5;
          break;
        case OPCODE_TYPE_PTR:
          id = *(int *)p; p+=sizeof(int);
					inst += StringPrintfW(L" %08X", id+(p-codeblock));
          size = 5;
          break;
        case OPCODE_TYPE_DLF: {
          id = *(int *)p; p+=sizeof(int);
          int np = *(int *)p;
          if ((np & 0xFFFF0000) == 0xFFFF0000) {
            p+=sizeof(int);
            np &= 0xFFFF;
          } else 
            np = -1;
          int i = VCPU::dlfBase(vcpuid)+id;
          if (i != -1) {
            VCPUdlfEntry *e = VCPU::DLFentryTable.enumItem(i);
            if (e != NULL) {
              if (np != -1) inst += StringPrintfW(L"(%d)", np);
              inst += L" ";
              inst += e->functionName;
            }
          }
          size = 5 + ((np == -1) ? 0 : 4);
          break;
        }
        case OPCODE_TYPE_NDLF: {
          id = *(int *)p; p+=sizeof(int);
          int np = *p; p++;
          int i = VCPU::dlfBase(vcpuid)+id;
          if (i != -1) {
            VCPUdlfEntry *e = VCPU::DLFentryTable.enumItem(i);
            if (e != NULL) {
              inst += StringPrintfW(L"(%d) ", np);
              inst += e->functionName;
            }
          }
          size = 6;
          break;
        }
        case OPCODE_TYPE_CLASSID: {
          id = *(int *)p; p+=sizeof(int);
          const wchar_t *cn = WASABI_API_MAKI->vcpu_getClassName(vcpuid, id);
          inst += L" ";
          inst += cn;
          size = 5;
          break;
        }
        case OPCODE_TYPE_DISCARD:
          id = *(int *)p; p+=sizeof(int);
          size = 5;
          break;
      }
      SourceCodeLineI *scl = new SourceCodeLineI();
      scl->setLine(inst),
      scl->setPointer(start_of_instruction-codeblock);
      scl->setLength(size);
      lines.addItem(scl);
    }
  }
}

