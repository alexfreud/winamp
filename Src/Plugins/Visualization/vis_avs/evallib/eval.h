#ifndef __EVAL_H
#define __EVAL_H

#ifdef __cplusplus
extern "C" {
#endif

  // stuff that apps will want to use
#define EVAL_MAX_VARS 256
typedef struct 
{
  char name[8];
  double value;
} varType;

extern double globalregs[100];
extern char last_error_string[1024];

void resetVars(varType *vars);
double *getVarPtr(char *varName);
double *registerVar(char *varName);


// other shat

extern varType *varTable;
extern int *errPtr;
extern int colCount;
extern int result;

int setVar(int varNum, double value);
int getVar(int varNum);
void *compileExpression(char *txt);



#ifdef __cplusplus
}
#endif

#endif