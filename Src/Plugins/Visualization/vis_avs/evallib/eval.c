#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "cal_tab.h"
#include "compiler.h"
#include "eval.h"

#define INTCONST 1
#define DBLCONST 2
#define HEXCONST 3
#define VARIABLE 4
#define OTHER    5

int yyparse(char *exp);
void llinit(void);
int gettoken(char *lltb, int lltbsiz);
char yytext[256]="";
char lastVar[256]="";
int *errPtr;
int result;
int colCount=0;

varType *varTable;

char last_error_string[1024];
double globalregs[100];

//------------------------------------------------------------------------------
void *compileExpression(char *exp)
{
  int errv=0;
  errPtr=&errv;
  llinit();
  if (!yyparse(exp) && !*errPtr)
  {
    return (void*)result;
  }
  return 0;
}

//------------------------------------------------------------------------------
void setLastVar(void)
{
  gettoken(lastVar, sizeof lastVar);
}

//------------------------------------------------------------------------------
int setVar(int varNum, double value)
{
  int i=varNum;
  if (varNum < 0)
  {
    char *var=lastVar;
    if (!_strnicmp(var,"reg",3) && strlen(var) == 5 && isdigit(var[3]) && isdigit(var[4]))
    {
      int x=atoi(var+3);
      if (x < 0 || x > 99) x=0;
      i=EVAL_MAX_VARS+x;
    }
    else 
    {
      for (i=0;i<EVAL_MAX_VARS;i++)
      {
        if (!varTable[i].name[0] || !_strnicmp(varTable[i].name,lastVar,sizeof(varTable[i].name)))
            break;
      }
      if (i==EVAL_MAX_VARS) return -1;
    }
  }

  if (i < 0 || i >= EVAL_MAX_VARS+100) 
  {
    return -1;
  }

  if (i >= EVAL_MAX_VARS && i < EVAL_MAX_VARS+100)
  {
    globalregs[i - EVAL_MAX_VARS]=value;
  }
  else if (!varTable[i].name[0]) 
  {
    strncpy(varTable[i].name,lastVar,sizeof(varTable[i].name));
    varTable[i].value = value;
  }

  return i;
}

//------------------------------------------------------------------------------
int getVar(int varNum)
{
  if (varNum >= EVAL_MAX_VARS && varNum < EVAL_MAX_VARS+100)
    return createCompiledValue(0, globalregs + (varNum - EVAL_MAX_VARS));

  if (varNum >= 0 && varNum < EVAL_MAX_VARS)
    return createCompiledValue(0, &(varTable[varNum].value));

  return createCompiledValue(0, NULL); 
}

#if 0
//------------------------------------------------------------------------------
double *getVarPtr(char *var)
{
  int i;

  if (!_strnicmp(var,"reg",3) && strlen(var) == 5 && isdigit(var[3]) && isdigit(var[4]))
  {
    int x=atoi(var+3);
    if (x < 0 || x > 99) x=0;
    return globalregs + x;
  }

  for (i=0;i<EVAL_MAX_VARS;i++)
          if (!_strnicmp(varTable[i].name, yytext,sizeof(varTable[i].name)))
                  return &(varTable[i].value);
  return NULL;
}
#endif

//------------------------------------------------------------------------------
double *registerVar(char *var)
{
  int i;
  if (!_strnicmp(var,"reg",3) && strlen(var) == 5 && isdigit(var[3]) && isdigit(var[4]))
  {
    int x=atoi(var+3);
    if (x < 0 || x > 99) x=0;
    return globalregs + x;
  }

  for (i=0;i<EVAL_MAX_VARS;i++)
      if (!varTable[i].name[0] || 
        !_strnicmp(varTable[i].name,var,sizeof(varTable[i].name)))
          break;
  if (i==EVAL_MAX_VARS) return NULL;

  if (!varTable[i].name[0])
  {
    strncpy(varTable[i].name,var,sizeof(varTable[i].name));
    varTable[i].value = 0.0;
  }
  return &(varTable[i].value);
}

//------------------------------------------------------------------------------
int translate(int type)
{
  int v;
  int n;
  *yytext = 0;
  gettoken(yytext, sizeof yytext);

  switch (type)
  {
    case INTCONST: return createCompiledValue((double)atoi(yytext), NULL);
    case DBLCONST: return createCompiledValue((double)atof(yytext), NULL);
    case HEXCONST:
      v=0;
      n=0;
      while (1)
      {
        int a=yytext[n++];
        if (a >= '0' && a <= '9') v+=a-'0';
        else if (a >= 'A' && a <= 'F') v+=10+a-'A';
        else if (a >= 'a' && a <= 'f') v+=10+a-'a';
        else break;
        v<<=4;
      }
		return createCompiledValue((double)v, NULL);
  }
  return 0;
}

//------------------------------------------------------------------------------
int objectId(int nParams)
{
  switch (nParams)
  {
    case 1: return FUNCTION1;
    case 2: return FUNCTION2;
    case 3: return FUNCTION3;
  }
  return IDENTIFIER;
}

//------------------------------------------------------------------------------
int lookup(int *typeOfObject)
{
  int i;
  gettoken(yytext, sizeof yytext);

  if (!_strnicmp(yytext,"reg",3) && strlen(yytext) == 5 && isdigit(yytext[3]) && isdigit(yytext[4]) && (i=atoi(yytext+3))>=0 && i<100)
  {
    *typeOfObject=IDENTIFIER;
    return i+EVAL_MAX_VARS;
  }

  for (i=0;i<EVAL_MAX_VARS;i++)
    if (!_strnicmp(varTable[i].name, yytext,sizeof(varTable[i].name)))
    {
      *typeOfObject = IDENTIFIER;
      return i;
    }

  for (i=0;getFunctionFromTable(i);i++)
  {
    functionType *f=getFunctionFromTable(i);
    if (!strcmpi(f->name, yytext))
    {
      *typeOfObject = objectId(f->nParams);
      return i;
    }
  }
  *typeOfObject = IDENTIFIER;
  setLastVar();
  i = setVar(-1, 0);
  return i;
}



//---------------------------------------------------------------------------
void count(void)
{
  gettoken(yytext, sizeof yytext);
  colCount+=strlen(yytext);
}

//---------------------------------------------------------------------------
int yyerror(char *txt)
{
  *errPtr = colCount;
  return 0;
}