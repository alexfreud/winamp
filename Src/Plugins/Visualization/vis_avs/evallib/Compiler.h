#ifndef __COMPILER_H
#define __COMPILER_H

#define FN_ASSIGN   0
#define FN_MULTIPLY 1
#define FN_DIVIDE   2
#define FN_MODULO   3
#define FN_ADD      4
#define FN_SUB      5
#define FN_AND      6
#define FN_OR       7
#define FN_UMINUS   8
#define FN_UPLUS    9

#define MATH_SIMPLE 0
#define MATH_FN     1

#ifdef __cplusplus
extern "C" {
#endif
 

int compileCode(char *exp);
void executeCode(int handle, char visdata[2][2][576]);
void freeCode(int handle);



typedef struct {
      char *name;
      void *afunc;
      void *func_e;
      int nParams;
      } functionType;

extern functionType *getFunctionFromTable(int idx);

int createCompiledValue(double value, double *addrValue);
int createCompiledFunction1(int fntype, int fn, int code);
int createCompiledFunction2(int fntype, int fn, int code1, int code2);
int createCompiledFunction3(int fntype, int fn, int code1, int code2, int code3);

#ifdef __cplusplus
}
#endif

#endif
