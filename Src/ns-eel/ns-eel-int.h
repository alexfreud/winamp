/*
  Nullsoft Expression Evaluator Library (NS-EEL)
  Copyright (C) 1999-2003 Nullsoft, Inc.
  
  ns-eel-int.h: internal code definition header.

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

#ifndef __NS_EELINT_H__
#define __NS_EELINT_H__

#include "ns-eel.h"
#include "ns-eel-addfuncs.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#define YYSTYPE int


typedef struct
{
  double **varTable_Values;
  char   **varTable_Names;
  int varTable_numBlocks;

  int errVar;
  int colCount;
  int result;
  char last_error_string[256];
  YYSTYPE yylval;
  int	yychar;			/*  the lookahead symbol		*/
  int yynerrs;			/*  number of parse errors so far       */
  char yytext[256];
  char lastVar[256];

  char    *llsave[16];             /* Look ahead buffer            */
  char    llbuf[100];             /* work buffer                          */
  char    *llp1;//   = &llbuf[0];    /* pointer to next avail. in token      */
  char    *llp2;//   = &llbuf[0];    /* pointer to end of lookahead          */
  char    *llend;//  = &llbuf[0];    /* pointer to end of token              */
  char    *llebuf;// = &llbuf[sizeof llbuf];
  int     lleof;
  int     yyline;//  = 0;

  void *tmpblocks_head,*blocks_head;
  int computTableTop; // make it abort on potential overflow =)
  int l_stats[4]; // source bytes, static code bytes, call code bytes, data bytes

  void *userfunc_data[64];
}
compileContext;

typedef struct {
      char *name;
      void *afunc;
      void *func_e;
      int nParams;
      NSEEL_PPPROC pProc;
} functionType;


extern functionType *nseel_getFunctionFromTable(int idx);

int nseel_createCompiledValue(compileContext *ctx, double value, double *addrValue);
int nseel_createCompiledFunction1(compileContext *ctx, int fntype, int fn, int code);
int nseel_createCompiledFunction2(compileContext *ctx, int fntype, int fn, int code1, int code2);
int nseel_createCompiledFunction3(compileContext *ctx, int fntype, int fn, int code1, int code2, int code3);

extern double nseel_globalregs[100];

void nseel_resetVars(compileContext *ctx);
double *nseel_getVarPtr(compileContext *ctx, char *varName);
double *nseel_registerVar(compileContext *ctx, char *varName);


// other shat



int nseel_setVar(compileContext *ctx, int varNum);
int nseel_getVar(compileContext *ctx, int varNum);
void *nseel_compileExpression(compileContext *ctx, char *txt);

#define	VALUE	258
#define	IDENTIFIER	259
#define	FUNCTION1	260
#define	FUNCTION2	261
#define	FUNCTION3	262
#define	UMINUS	263
#define	UPLUS	264

int nseel_translate(compileContext *ctx, int type);
void nseel_count(compileContext *ctx);
void nseel_setLastVar(compileContext *ctx);
int nseel_lookup(compileContext *ctx, int *typeOfObject);
int nseel_yyerror(compileContext *ctx);
int nseel_yylex(compileContext *ctx, char **exp);
int nseel_yyparse(compileContext *ctx, char *exp);
void nseel_llinit(compileContext *ctx);
int nseel_gettoken(compileContext *ctx, char *lltb, int lltbsiz);

struct  lextab {
        int     llendst;                /* Last state number            */
        char    *lldefault;             /* Default state table          */
        char    *llnext;                /* Next state table             */
        char    *llcheck;               /* Check table                  */
        int     *llbase;                /* Base table                   */
        int     llnxtmax;               /* Last in next table           */
        int     (*llmove)();            /* Move between states          */
        char     *llfinal;               /* Final state descriptions     */
        int     (*llactr)();            /* Action routine               */
        int     *lllook;                /* Look ahead vector if != NULL */
        char    *llign;                 /* Ignore char vec if != NULL   */
        char    *llbrk;                 /* Break char vec if != NULL    */
        char    *llill;                 /* Illegal char vec if != NULL  */
};
extern struct lextab nseel_lextab;



#ifdef __cplusplus
}
#endif

#endif//__NS_EELINT_H__