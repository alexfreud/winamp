#ifndef __SCRIPTVAR_H
#define __SCRIPTVAR_H

#ifdef __cplusplus
 class ScriptObject;
#endif

#ifdef _MSC_VER
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif

typedef struct {
  int type;    // basic type, see above
  union { // union of 4 bytes of different types 
    int idata;    // Integer
    float fdata;  // Float
    double ddata; // Double
#ifdef __cplusplus
    ScriptObject *odata;  // Object
#else
    void *odata;
#endif
    const wchar_t *sdata;  // String
  } data;
} scriptVar;

#ifdef _MSC_VER
#pragma pack(pop)
#else
#pragma pack()
#endif

#endif
