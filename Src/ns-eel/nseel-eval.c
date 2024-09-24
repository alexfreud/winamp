/*
  Nullsoft Expression Evaluator Library (NS-EEL)
  Copyright (C) 1999-2003 Nullsoft, Inc.
  
  nseel-eval.c

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <windows.h>
#include "ns-eel-int.h"

#define NSEEL_VARS_PER_BLOCK 64
#define NSEEL_VARS_MALLOC_CHUNKSIZE 8
#define NSEEL_GLOBALVAR_BASE (1<<24)

#ifndef NSEEL_MAX_VARIABLE_NAMELEN
#define NSEEL_MAX_VARIABLE_NAMELEN 8
#endif


#define INTCONST 1
#define DBLCONST 2
#define HEXCONST 3
#define VARIABLE 4
#define OTHER    5

double nseel_globalregs[100];

//------------------------------------------------------------------------------
void *nseel_compileExpression(compileContext *ctx, char *exp)
{
  ctx->errVar=0;
  nseel_llinit(ctx);
  if (!nseel_yyparse(ctx,exp) && !ctx->errVar)
  {
    return (void*)ctx->result;
  }
  return 0;
}

//------------------------------------------------------------------------------
void nseel_setLastVar(compileContext *ctx)
{
  nseel_gettoken(ctx,ctx->lastVar, sizeof(ctx->lastVar));
}

static int register_var(compileContext *ctx, char *name, double **ptr)
{
  int wb;
  int ti=0;
  int i=0;
  char *nameptr;
  for (wb = 0; wb < ctx->varTable_numBlocks; wb ++)
  {
    int namepos=0;
    for (ti = 0; ti < NSEEL_VARS_PER_BLOCK; ti ++)
    {        
      if (!ctx->varTable_Names[wb][namepos] || !_strnicmp(ctx->varTable_Names[wb]+namepos,name,NSEEL_MAX_VARIABLE_NAMELEN))
      {
        break;
      }
      namepos += NSEEL_MAX_VARIABLE_NAMELEN;
      i++;
    }
    if (ti < NSEEL_VARS_PER_BLOCK)
      break;
  }
  if (wb == ctx->varTable_numBlocks)
  {
    ti=0;
    // add new block
    if (!(ctx->varTable_numBlocks&(NSEEL_VARS_MALLOC_CHUNKSIZE-1)) || !ctx->varTable_Values || !ctx->varTable_Names )
    {
      ctx->varTable_Values = (double **)realloc(ctx->varTable_Values,(ctx->varTable_numBlocks+NSEEL_VARS_MALLOC_CHUNKSIZE) * sizeof(double *));
      ctx->varTable_Names = (char **)realloc(ctx->varTable_Names,(ctx->varTable_numBlocks+NSEEL_VARS_MALLOC_CHUNKSIZE) * sizeof(char *));
    }
    ctx->varTable_numBlocks++;

    ctx->varTable_Values[wb] = (double *)calloc(sizeof(double),NSEEL_VARS_PER_BLOCK);
    ctx->varTable_Names[wb] = (char *)calloc(NSEEL_MAX_VARIABLE_NAMELEN,NSEEL_VARS_PER_BLOCK);
  }

  nameptr=ctx->varTable_Names[wb]+ti*NSEEL_MAX_VARIABLE_NAMELEN;
  if (!nameptr[0])
  {
    strncpy(nameptr,name,NSEEL_MAX_VARIABLE_NAMELEN);
  }
  if (ptr) *ptr = ctx->varTable_Values[wb] + ti;
  return i;
}

//------------------------------------------------------------------------------
int nseel_setVar(compileContext *ctx, int varNum)
{
  if (varNum < 0) // adding new var
  {
    char *var=ctx->lastVar;
    if (!_strnicmp(var,"reg",3) && strlen(var) == 5 && isdigit(var[3]) && isdigit(var[4]))
    {
      int i,x=atoi(var+3);
      if (x < 0 || x > 99) x=0;
      i=NSEEL_GLOBALVAR_BASE+x;
      return i;
    }

    return register_var(ctx,ctx->lastVar,NULL);

  }

  // setting/getting oldvar

  if (varNum >= NSEEL_GLOBALVAR_BASE && varNum < NSEEL_GLOBALVAR_BASE+100)
  {
    return varNum;
  }

  {
    int wb,ti;
    char *nameptr;
    if (varNum < 0 || varNum >= ctx->varTable_numBlocks*NSEEL_VARS_PER_BLOCK) return -1;

    wb=varNum/NSEEL_VARS_PER_BLOCK;
    ti=(varNum%NSEEL_VARS_PER_BLOCK);
    nameptr=ctx->varTable_Names[wb]+ti*NSEEL_MAX_VARIABLE_NAMELEN;
    if (!nameptr[0]) 
    {
      strncpy(nameptr,ctx->lastVar,NSEEL_MAX_VARIABLE_NAMELEN);
    }  
    return varNum;  
  }

}

//------------------------------------------------------------------------------
int nseel_getVar(compileContext *ctx, int i)
{
  if (i >= 0 && i < (NSEEL_VARS_PER_BLOCK*ctx->varTable_numBlocks))
    return nseel_createCompiledValue(ctx,0, ctx->varTable_Values[i/NSEEL_VARS_PER_BLOCK] + i%NSEEL_VARS_PER_BLOCK); 
  if (i >= NSEEL_GLOBALVAR_BASE && i < NSEEL_GLOBALVAR_BASE+100) 
    return nseel_createCompiledValue(ctx,0, nseel_globalregs+i-NSEEL_GLOBALVAR_BASE);

  return nseel_createCompiledValue(ctx,0, NULL);
}



//------------------------------------------------------------------------------
double *NSEEL_VM_regvar(NSEEL_VMCTX _ctx, char *var)
{
  compileContext *ctx = (compileContext *)_ctx;
  double *r;
  if (!ctx) return 0;

  if (!_strnicmp(var,"reg",3) && strlen(var) == 5 && isdigit(var[3]) && isdigit(var[4]))
  {
    int x=atoi(var+3);
    if (x < 0 || x > 99) x=0;
    return nseel_globalregs + x;
  }

  register_var(ctx,var,&r);

  return r;
}

//------------------------------------------------------------------------------
int nseel_translate(compileContext *ctx, int type)
{
  int v;
  int n;
  *ctx->yytext = 0;
  nseel_gettoken(ctx,ctx->yytext, sizeof(ctx->yytext));

  switch (type)
  {
    case INTCONST: return nseel_createCompiledValue(ctx,(double)atoi(ctx->yytext), NULL);
    case DBLCONST: return nseel_createCompiledValue(ctx,(double)atof(ctx->yytext), NULL);
    case HEXCONST:
      v=0;
      n=0;
      while (1)
      {
        int a=ctx->yytext[n++];
        if (a >= '0' && a <= '9') v+=a-'0';
        else if (a >= 'A' && a <= 'F') v+=10+a-'A';
        else if (a >= 'a' && a <= 'f') v+=10+a-'a';
        else break;
        v<<=4;
      }
		return nseel_createCompiledValue(ctx,(double)v, NULL);
  }
  return 0;
}

//------------------------------------------------------------------------------
int nseel_lookup(compileContext *ctx, int *typeOfObject)
{
  int i;
  nseel_gettoken(ctx,ctx->yytext, sizeof(ctx->yytext));

  if (!_strnicmp(ctx->yytext,"reg",3) && strlen(ctx->yytext) == 5 && isdigit(ctx->yytext[3]) && isdigit(ctx->yytext[4]) && (i=atoi(ctx->yytext+3))>=0 && i<100)
  {
    *typeOfObject=IDENTIFIER;
    return i+NSEEL_GLOBALVAR_BASE;
  }


  {
    int wb;
    int ti=0;
    i=0;
    for (wb = 0; wb < ctx->varTable_numBlocks; wb ++)
    {
      int namepos=0;
      for (ti = 0; ti < NSEEL_VARS_PER_BLOCK; ti ++)
      {        
        if (!ctx->varTable_Names[wb][namepos]) break;

        if (!_strnicmp(ctx->varTable_Names[wb]+namepos,ctx->yytext,NSEEL_MAX_VARIABLE_NAMELEN))
        {
          *typeOfObject = IDENTIFIER;
          return i;
        }

        namepos += NSEEL_MAX_VARIABLE_NAMELEN;
        i++;
      }
      if (ti < NSEEL_VARS_PER_BLOCK) break;
    }
  }


  for (i=0;nseel_getFunctionFromTable(i);i++)
  {
    functionType *f=nseel_getFunctionFromTable(i);
    if (!strcmpi(f->name, ctx->yytext))
    {
      switch (f->nParams)
      {
        case 1: *typeOfObject = FUNCTION1; break;
        case 2: *typeOfObject = FUNCTION2; break;
        case 3: *typeOfObject = FUNCTION3; break;
        default: *typeOfObject = IDENTIFIER; break;
      }
      return i;
    }
  }
  *typeOfObject = IDENTIFIER;
  nseel_setLastVar(ctx);
  return nseel_setVar(ctx,-1);
}



//---------------------------------------------------------------------------
void nseel_count(compileContext *ctx)
{
  nseel_gettoken(ctx,ctx->yytext, sizeof(ctx->yytext));
  ctx->colCount+=strlen(ctx->yytext);
}

//---------------------------------------------------------------------------
int nseel_yyerror(compileContext *ctx)
{
  ctx->errVar = ctx->colCount;
  return 0;
}