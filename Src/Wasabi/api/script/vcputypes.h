#ifndef __VCPUTYPES_H
#define __VCPUTYPES_H

#include <bfc/platform/platform.h>

// Basic types
#define SCRIPT_FIRSTTYPE        SCRIPT_VOID
#define SCRIPT_VOID             0
#define SCRIPT_EVENT            1
#define SCRIPT_INT              2
#define SCRIPT_FLOAT            3
#define SCRIPT_DOUBLE           4
#define SCRIPT_BOOLEAN          5
#define SCRIPT_STRING           6
#define SCRIPT_OBJECT           7 
#define SCRIPT_ANY              8
#define SCRIPT_LASTTYPE         SCRIPT_ANY

static const GUID guidNull = 
{ 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

#include <api/script/scriptvar.h>

#ifdef _MSC_VER
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif

typedef struct {
  scriptVar v;
  int scriptId;
  int varId;
  char transcient;
  char isaclass;
  char isstatic;
} VCPUscriptVar;

typedef struct {
  int varId;
  int pointer;
  int DLFid;
  int scriptId;
} VCPUeventEntry;

typedef struct {
  wchar_t *functionName;
  int basetype;
  int scriptId;
  int DLFid;
  void *ptr;
  int nparams;
} VCPUdlfEntry;

typedef struct {
  char *codeBlock;
  int scriptId;
  int dlfBase;
  int varBase;
  int size;
  char *debugsymbols;
  int debugsize;
} VCPUcodeBlock;

typedef struct 
{
  int cmd;
  int id;
} maki_cmd;

#ifdef _MSC_VER
#pragma pack(pop)
#else
#pragma pack()
#endif

#endif

