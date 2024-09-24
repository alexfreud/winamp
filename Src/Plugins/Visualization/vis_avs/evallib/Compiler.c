#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "Compiler.h"
#include "eval.h"

// defining this allows code to run in different threads at the same time
// it tends however, to be slower. We leave this OFF for AVS, since most of our shit runs in one thread
// anyhow.
//#define NSEEL_REENTRANT_EXECUTION

#ifdef NSEEL_REENTRANT_EXECUTION
#include <malloc.h>
#endif
#define NSEEL_USE_CRITICAL_SECTION g_eval_cs

// note that compiling can only happen in one thread at a time, always.




int g_evallib_stats[5]; // source bytes, static code bytes, call code bytes, data bytes, segments

#ifdef NSEEL_USE_CRITICAL_SECTION
CRITICAL_SECTION NSEEL_USE_CRITICAL_SECTION;
#endif


#define LLB_DSIZE (65536-64)
typedef struct _llBlock {
	struct _llBlock *next;
  int sizeused;
	char block[LLB_DSIZE];
} llBlock;

typedef struct _startPtr {
  struct _startPtr *next;
  void *startptr;
} startPtr;

typedef struct {
  int workTablePtr_size;

  llBlock *blocks;
  void *code;
  int code_stats[4];
} codeHandleType;

static llBlock *blocks_head = NULL;
static llBlock *tmpblocks_head = NULL; // used only during compile

#define NSEEL_MAX_TEMPSPACE_ENTRIES 8192

static int g_evallib_computTableTop; // make it abort on potential overflow =)

static int l_stats[4]; // source bytes, static code bytes, call code bytes, data bytes

static void *__newBlock(llBlock **start,int size);

#define newTmpBlock(x) __newBlock(&tmpblocks_head,x)
#define newBlock(x) __newBlock(&blocks_head,x)

static void freeBlocks(llBlock *start);

char *g_evallib_visdata;

#define DECL_ASMFUNC(x)         \
  void _asm_##x##(void);        \
  void _asm_##x##_end(void);    \

  DECL_ASMFUNC(sin)
  DECL_ASMFUNC(cos)
  DECL_ASMFUNC(tan)
  DECL_ASMFUNC(asin)
  DECL_ASMFUNC(acos)
  DECL_ASMFUNC(atan)
  DECL_ASMFUNC(atan2)
  DECL_ASMFUNC(sqr)
  DECL_ASMFUNC(sqrt)
  DECL_ASMFUNC(pow)
  DECL_ASMFUNC(exp)
  DECL_ASMFUNC(log)
  DECL_ASMFUNC(log10)
  DECL_ASMFUNC(abs)
  DECL_ASMFUNC(min)
  DECL_ASMFUNC(min)
  DECL_ASMFUNC(max)
  DECL_ASMFUNC(sig)
  DECL_ASMFUNC(sign)
  DECL_ASMFUNC(rand)
  DECL_ASMFUNC(band)
  DECL_ASMFUNC(bor)
  DECL_ASMFUNC(bnot)
  DECL_ASMFUNC(if)
  DECL_ASMFUNC(equal)
  DECL_ASMFUNC(below)
  DECL_ASMFUNC(above)
  DECL_ASMFUNC(assign)
  DECL_ASMFUNC(add)
  DECL_ASMFUNC(sub)
  DECL_ASMFUNC(mul)
  DECL_ASMFUNC(div)
  DECL_ASMFUNC(mod)
  DECL_ASMFUNC(or)
  DECL_ASMFUNC(and)
  DECL_ASMFUNC(uplus)
  DECL_ASMFUNC(uminus)
  DECL_ASMFUNC(floor)
  DECL_ASMFUNC(ceil)
  DECL_ASMFUNC(invsqrt)
  DECL_ASMFUNC(exec2)

  DECL_ASMFUNC(getosc)
  DECL_ASMFUNC(getspec)
  DECL_ASMFUNC(gettime)
  DECL_ASMFUNC(getmouse)
  DECL_ASMFUNC(setmousepos)


static functionType fnTable1[36] = {
													 { "if",     _asm_if,_asm_if_end,    3 },
                           { "sin",   _asm_sin,_asm_sin_end,   1 },
                           { "cos",    _asm_cos,_asm_cos_end,   1 },
                           { "tan",    _asm_tan,_asm_tan_end,   1 },
                           { "asin",   _asm_asin,_asm_asin_end,  1 },
                           { "acos",   _asm_acos,_asm_acos_end,  1 },
                           { "atan",   _asm_atan,_asm_atan_end,  1 },
                           { "atan2",  _asm_atan2,_asm_atan2_end, 2 },
                           { "sqr",    _asm_sqr,_asm_sqr_end,   1 },
                           { "sqrt",   _asm_sqrt,_asm_sqrt_end,  1 },
                           { "pow",    _asm_pow,_asm_pow_end,   2 },
                           { "exp",    _asm_exp,_asm_exp_end,   1 },
                           { "log",    _asm_log,_asm_log_end,   1 },
                           { "log10",  _asm_log10,_asm_log10_end, 1 },
                           { "abs",    _asm_abs,_asm_abs_end,   1 },
                           { "min",    _asm_min,_asm_min_end,   2 },
                           { "max",    _asm_max,_asm_max_end,   2 },
													 { "sigmoid",_asm_sig,_asm_sig_end,   2 } ,
													 { "sign",   _asm_sign,_asm_sign_end,  1 } ,
													 { "rand",   _asm_rand,_asm_rand_end,  1 } ,
													 { "band",   _asm_band,_asm_band_end,  2 } ,
													 { "bor",    _asm_bor,_asm_bor_end,   2 } ,
													 { "bnot",   _asm_bnot,_asm_bnot_end,  1 } ,
													 { "equal",  _asm_equal,_asm_equal_end, 2 },
													 { "below",  _asm_below,_asm_below_end, 2 },
													 { "above",  _asm_above,_asm_above_end, 2 },
                           { "floor",  _asm_floor,_asm_floor_end, 1 },
                           { "ceil",   _asm_ceil,_asm_ceil_end,  1 },
                           { "invsqrt",   _asm_invsqrt,_asm_invsqrt_end,  1 },
                           { "assign",_asm_assign,_asm_assign_end,2},
                           { "exec2",_asm_exec2,_asm_exec2_end,2},
                           // these will be seperated since they are AVS specific
													 { "getosc", _asm_getosc,_asm_getosc_end,3 },
													 { "getspec",_asm_getspec,_asm_getspec_end,3 },
                           { "gettime", _asm_gettime,_asm_gettime_end,1},
                           { "getkbmouse",_asm_getmouse,_asm_getmouse_end,1},
                           { "setmousepos",_asm_setmousepos,_asm_setmousepos_end,2},
                           };


functionType *getFunctionFromTable(int idx)
{
  // todo: add API for adding functions to a seperate table :)
  if (idx<0 || idx>=sizeof(fnTable1)/sizeof(fnTable1[0])) return 0;
  return fnTable1+idx;
}

//---------------------------------------------------------------------------------------------------------------
static void *realAddress(void *fn, void *fn_e, int *size)
{
#ifdef _DEBUG
	// Debug Mode
  *siiiize=0; // fucko, need to figure this out
char *ptr = (char *)fn;
return ptr + (*(int *)(ptr+1))+5;
#else
  // Release Mode
  *size  = (int)fn_e - (int) fn;
	return fn;
#endif
}

//---------------------------------------------------------------------------------------------------------------
static void freeBlocks(llBlock *start)
{
  while (start)
	{
    llBlock *llB = start->next;
    GlobalFree(start);
	  start=llB;
	}
}

//---------------------------------------------------------------------------------------------------------------
static void *__newBlock(llBlock **start, int size)
{
  llBlock *llb;
  int alloc_size;
  if (*start && (LLB_DSIZE - (*start)->sizeused) >= size)
  {
    void *t=(*start)->block+(*start)->sizeused;
    (*start)->sizeused+=size;
    return t;
  }

  alloc_size=sizeof(llBlock);
  if ((int)size > LLB_DSIZE) alloc_size += size - LLB_DSIZE;
  llb = (llBlock *)VirtualAlloc(NULL, alloc_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  // benski> CUT: llb = (llBlock *)GlobalAlloc(GMEM_FIXED,alloc_size); // grab bigger block if absolutely necessary (heh)
  llb->sizeused=size;
  llb->next = *start;  
  *start = llb;
  return llb->block;
}


#define X86_MOV_EAX_DIRECTVALUE 0xB8
#define X86_MOV_ESI_DIRECTVALUE 0xBE
#define X86_MOV_ESI_DIRECTMEMVALUE 0x358B
#define X86_PUSH_EAX 0x50
#define X86_POP_EBX 0x5B
#define X86_POP_ECX 0x59
#define X86_MOV_ESI_EDI 0xF78B

#define X86_PUSH_ESI 0x56
#define X86_POP_ESI  0x5E

#define X86_RET 0xC3


//---------------------------------------------------------------------------------------------------------------
static int *findFBlock(char *p)
{
  while (*(int *)p != 0xFFFFFFFF) p++;
  return (int*)p;
}


//---------------------------------------------------------------------------------------------------------------
int createCompiledValue(double value, double *addrValue)
{
  unsigned char *block;
  double *dupValue;

  block=(unsigned char *)newTmpBlock(4+5);

  if (addrValue == NULL)
  {
    l_stats[3]+=sizeof(double);
	  *(dupValue = (double *)newBlock(sizeof(double))) = value;
  }
  else
	  dupValue = addrValue;

  ((int*)block)[0]=5;
  block[4]=X86_MOV_EAX_DIRECTVALUE; // mov eax, <value>
  *(int *)(block+5) = (int)dupValue;

  return ((int)(block));

}

//---------------------------------------------------------------------------------------------------------------
int getFunctionAddress(int fntype, int fn, int *size)
{
  switch (fntype)
	{
  	case MATH_SIMPLE:
	  	switch (fn)
			{
			  case FN_ASSIGN:
				  return (int)realAddress(_asm_assign,_asm_assign_end,size);
			  case FN_ADD:
				  return (int)realAddress(_asm_add,_asm_add_end,size);
			  case FN_SUB:
				  return (int)realAddress(_asm_sub,_asm_sub_end,size);
			  case FN_MULTIPLY:
				  return (int)realAddress(_asm_mul,_asm_mul_end,size);
			  case FN_DIVIDE:
				  return (int)realAddress(_asm_div,_asm_div_end,size);
			  case FN_MODULO:
				  return (int)realAddress(_asm_mod,_asm_mod_end,size);
			  case FN_AND:
				  return (int)realAddress(_asm_and,_asm_and_end,size);
			  case FN_OR:
				  return (int)realAddress(_asm_or,_asm_or_end,size);
			  case FN_UPLUS:
				  return (int)realAddress(_asm_uplus,_asm_uplus_end,size);
			  case FN_UMINUS:
				  return (int)realAddress(_asm_uminus,_asm_uminus_end,size);
			}
	  case MATH_FN:
      {
        functionType *p=getFunctionFromTable(fn);
		    if (!p) 
        {
          if (size) *size=0;
          return 0;
        }
        return (int)realAddress(p->afunc,p->func_e,size);
      }
	}		
  return 0;
}


//---------------------------------------------------------------------------------------------------------------
int createCompiledFunction3(int fntype, int fn, int code1, int code2, int code3)
{
  int sizes1=((int *)code1)[0];
  int sizes2=((int *)code2)[0];
  int sizes3=((int *)code3)[0];

  if (fntype == MATH_FN && fn == 0) // special case: IF
  {
    void *func3;
    int size;
    int *ptr;
    char *block;

    unsigned char *newblock2,*newblock3;

    newblock2=newBlock(sizes2+1);
    memcpy(newblock2,(char*)code2+4,sizes2);
    newblock2[sizes2]=X86_RET;

    newblock3=newBlock(sizes3+1);
    memcpy(newblock3,(char*)code3+4,sizes3);
    newblock3[sizes3]=X86_RET;

    l_stats[2]+=sizes2+sizes3+2;
    
    func3 = realAddress(_asm_if,_asm_if_end,&size);

    block=(char *)newTmpBlock(4+sizes1+size);
    ((int*)block)[0]=sizes1+size;
    memcpy(block+4,(char*)code1+4,sizes1);
    ptr=(int *)(block+4+sizes1);
    memcpy(ptr,func3,size);

    ptr=findFBlock((char*)ptr); *ptr++=(int)newblock2;
    ptr=findFBlock((char*)ptr); *ptr=(int)newblock3;

    return (int)block;

  }
  else
  {
    int size2;
    unsigned char *block;
    unsigned char *outp;

    int myfunc;
  
    myfunc = getFunctionAddress(fntype, fn, &size2);

    block=(unsigned char *)newTmpBlock(4+size2+sizes1+sizes2+sizes3+4);

    ((int*)block)[0]=4+size2+sizes1+sizes2+sizes3;
    outp=block+4;
    memcpy(outp,(char*)code1+4,sizes1); 
    outp+=sizes1;
    *outp++ = X86_PUSH_EAX;
    memcpy(outp,(char*)code2+4,sizes2); 
    outp+=sizes2;
    *outp++ = X86_PUSH_EAX;
    memcpy(outp,(char*)code3+4,sizes3); 
    outp+=sizes3;
    *outp++ = X86_POP_EBX;
    *outp++ = X86_POP_ECX;

    memcpy(block+4+4+sizes1+sizes2+sizes3,(void*)myfunc,size2);
    g_evallib_computTableTop++;

    return ((int)(block));  
  }
}

//---------------------------------------------------------------------------------------------------------------
int createCompiledFunction2(int fntype, int fn, int code1, int code2)
{
  int size2;
  unsigned char *block;
  unsigned char *outp;

  int myfunc;
  int sizes1=((int *)code1)[0];
  int sizes2=((int *)code2)[0];

  myfunc = getFunctionAddress(fntype, fn, &size2);

  block=(unsigned char *)newTmpBlock(2+size2+sizes1+sizes2+4);

  ((int*)block)[0]=2+size2+sizes1+sizes2;
  outp=block+4;
  memcpy(outp,(char*)code1+4,sizes1); 
  outp+=sizes1;
  *outp++ = X86_PUSH_EAX;
  memcpy(outp,(char*)code2+4,sizes2); 
  outp+=sizes2;
  *outp++ = X86_POP_EBX;

  memcpy(block+4+2+sizes1+sizes2,(void*)myfunc,size2);

  g_evallib_computTableTop++;

  return ((int)(block));

}


//---------------------------------------------------------------------------------------------------------------
int createCompiledFunction1(int fntype, int fn, int code)
{
  int size,size2;
  char *block;
  int myfunc;
  void *func1;
  
  size =((int *)code)[0];
  func1 = (void *)(code+4);

  myfunc = getFunctionAddress(fntype, fn, &size2);

  block=(char *)newTmpBlock(4+size+size2);
  ((int*)block)[0]=size+size2;

  memcpy(block+4, func1, size);
  memcpy(block+size+4,(void*)myfunc,size2);

  g_evallib_computTableTop++;

  return ((int)(block));
}

static char *preprocessCode(char *expression)
{
  int len=0;
  int alloc_len=strlen(expression)+1+64;
  char *buf=(char *)malloc(alloc_len);

  while (*expression)
  {
    if (len > alloc_len-32)
    {
      alloc_len = len+128;
      buf=(char*)realloc(buf,alloc_len);
    }

    if (expression[0] == '/')
    {
      if (expression[1] == '/')
      {
        expression+=2;
        while (expression[0] && expression[0] != '\r' && expression[0] != '\n') expression++;
      }
      else if (expression[1] == '*')
      {
        expression+=2;
        while (expression[0] && (expression[0] != '*' || expression[1] != '/')) expression++;
        if (expression[0]) expression+=2; // at this point we KNOW expression[0]=* and expression[1]=/
      }
      else 
      {
        char c=buf[len++]=*expression++;
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') l_stats[0]++;
      }
    }
    else if (expression[0] == '$')
    {
      if (toupper(expression[1]) == 'P' && toupper(expression[2]) == 'I')
      {
        static char *str="3.141592653589793";
        expression+=3;
        memcpy(buf+len,str,17);
        len+=17; //strlen(str);
        l_stats[0]+=17;
      }
      else if (toupper(expression[1]) == 'E')
      {
        static char *str="2.71828183";
        expression+=2;
        memcpy(buf+len,str,10);
        len+=10; //strlen(str);
        l_stats[0]+=10;
      }
      if (toupper(expression[1]) == 'P' && toupper(expression[2]) == 'H' && toupper(expression[3]) == 'I')
      {
        static char *str="1.61803399";
        expression+=4;
        memcpy(buf+len,str,10);
        len+=10; //strlen(str);
        l_stats[0]+=10;
      }
      else 
      {
        char c = buf[len++]=*expression++;
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') l_stats[0]++;
      }
    }
    else
    {
      char c=*expression++;
      if (c == '\r' || c == '\n' || c == '\t') c=' ';
      buf[len++]=c;
      if (c != ' ') l_stats[0]++;
    }
  }
  buf[len]=0;

  return buf;
}

int g_log_errors;

static void movestringover(char *str, int amount)
{
  char tmp[1024+8];

  int l=(int)strlen(str);
  l=min(1024-amount-1,l);

  memcpy(tmp,str,l+1);

  while (l >= 0 && tmp[l]!='\n') l--;
  l++;

  tmp[l]=0;//ensure we null terminate

  memcpy(str+amount,tmp,l+1);
}

//------------------------------------------------------------------------------
int compileCode(char *_expression)
{
  char *expression,*expression_start;
  int computable_size=0;
  codeHandleType *handle;
  startPtr *scode=NULL;
  startPtr *startpts=NULL;

  if (!_expression || !*_expression) return 0;
  if (!varTable) return 0;

  #ifdef NSEEL_USE_CRITICAL_SECTION
    EnterCriticalSection(& NSEEL_USE_CRITICAL_SECTION);
  #endif

  blocks_head=0;
  tmpblocks_head=0;
  memset(l_stats,0,sizeof(l_stats));

  handle = (codeHandleType*)newBlock(sizeof(codeHandleType));

  if (!handle) 
  {
    #ifdef NSEEL_USE_CRITICAL_SECTION
      LeaveCriticalSection(& NSEEL_USE_CRITICAL_SECTION);
    #endif
    return 0;
  }
  
  memset(handle,0,sizeof(codeHandleType));

  expression_start=expression=preprocessCode(_expression);

  while (*expression)
	{
    startPtr *tmp;
    char *expr;
    colCount=0;

    // single out segment
    while (*expression == ';' || *expression == ' ') expression++;
    if (!*expression) break;
    expr=expression;
	  while (*expression && *expression != ';') expression++;
    if (*expression) *expression++ = 0;

    // parse
    tmp=(startPtr*) newTmpBlock(sizeof(startPtr));
    if (!tmp) break;
    g_evallib_computTableTop=0;
    tmp->startptr=compileExpression(expr);
    if (computable_size < g_evallib_computTableTop)
    {
      computable_size=g_evallib_computTableTop;
    }

    if (g_evallib_computTableTop > NSEEL_MAX_TEMPSPACE_ENTRIES-32)
    {
      tmp->startptr=0; // overflow in this mode
    }   

    if (!tmp->startptr) 
    { 
      if (g_log_errors)
      {
        int l=strlen(expr);
        if (l > 512) l=512;
        movestringover(last_error_string,l+2);
        memcpy(last_error_string,expr,l);
        last_error_string[l]='\r';
        last_error_string[l+1]='\n';
      }

      scode=NULL; 
      break; 
    }
    tmp->next=NULL;
    if (!scode) scode=startpts=tmp;
    else
    {
      scode->next=tmp;
      scode=tmp;
    }
  }

  // check to see if failed on the first startingCode
  if (!scode)
  {
    freeBlocks(blocks_head);  // free blocks
    handle=NULL;              // return NULL (after resetting blocks_head)
  }
  else 
  {
    // now we build one big code segment out of our list of them, inserting a mov esi, computable before each item
    unsigned char *writeptr;
    int size=1; // for ret at end :)
    startPtr *p;
    p=startpts;
    while (p)
    {
      size+=2; // mov esi, edi
      size+=*(int *)p->startptr;
      p=p->next;
    }
    handle->code = newBlock(size);
    if (handle->code)
    {
      writeptr=(unsigned char *)handle->code;
      p=startpts;
      while (p)
      {
        int thissize=*(int *)p->startptr;
        *(unsigned short *)writeptr= X86_MOV_ESI_EDI;
        writeptr+=2;
        memcpy(writeptr,(char*)p->startptr + 4,thissize);
        writeptr += thissize;
      
        p=p->next;
      }
      *writeptr=X86_RET; // ret
      l_stats[1]=size;
    }
    handle->blocks = blocks_head;
    handle->workTablePtr_size=(computable_size+4) * sizeof(double);
  }
  freeBlocks(tmpblocks_head);  // free blocks
  tmpblocks_head=0;

  blocks_head=0;

  if (handle)
  {
    memcpy(handle->code_stats,l_stats,sizeof(l_stats));
    g_evallib_stats[0]+=l_stats[0];
    g_evallib_stats[1]+=l_stats[1];
    g_evallib_stats[2]+=l_stats[2];
    g_evallib_stats[3]+=l_stats[3];
    g_evallib_stats[4]++;
  }
  memset(l_stats,0,sizeof(l_stats));

  #ifdef NSEEL_USE_CRITICAL_SECTION
    LeaveCriticalSection(& NSEEL_USE_CRITICAL_SECTION);
  #endif

  free(expression_start);

  return (int)handle;
}

//------------------------------------------------------------------------------
void executeCode(int handle, char visdata[2][2][576])
{
#ifdef NSEEL_REENTRANT_EXECUTION
  int baseptr;
#else
  static double _tab[NSEEL_MAX_TEMPSPACE_ENTRIES];
  int baseptr = (int) _tab;
#endif
  codeHandleType *h = (codeHandleType *)handle;
  if (!h || !h->code) return;
#ifdef NSEEL_USE_CRITICAL_SECTION
  EnterCriticalSection(& NSEEL_USE_CRITICAL_SECTION);
#endif
  g_evallib_visdata=(char*)visdata;
#ifdef NSEEL_REENTRANT_EXECUTION
  baseptr = (int) alloca(h->workTablePtr_size);
  if (!baseptr) return;
#endif
  {
    int startPoint=(int)h->code;
    __asm 
    {
      mov ebx, baseptr
      mov eax, startPoint
      pushad // Lets cover our ass
      mov edi, ebx
      call eax
      popad
    }
  }
  g_evallib_visdata=NULL;
  #ifdef NSEEL_USE_CRITICAL_SECTION
    LeaveCriticalSection(& NSEEL_USE_CRITICAL_SECTION);
  #endif
}

//------------------------------------------------------------------------------
void freeCode(int handle)
{
  codeHandleType *h = (codeHandleType *)handle;
  if (h != NULL)
  {
    g_evallib_stats[0]-=h->code_stats[0];
    g_evallib_stats[1]-=h->code_stats[1];
    g_evallib_stats[2]-=h->code_stats[2];
    g_evallib_stats[3]-=h->code_stats[3];
    g_evallib_stats[4]--;
    freeBlocks(h->blocks);
  }
}


//------------------------------------------------------------------------------
void resetVars(varType *vars)
{
#ifdef NSEEL_USE_CRITICAL_SECTION
  if (vars) EnterCriticalSection(& NSEEL_USE_CRITICAL_SECTION);
#endif
  varTable=vars;
#ifdef NSEEL_USE_CRITICAL_SECTION
  if (!vars) LeaveCriticalSection(& NSEEL_USE_CRITICAL_SECTION);
#endif
}
