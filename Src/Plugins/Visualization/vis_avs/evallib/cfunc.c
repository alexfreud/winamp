#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "Compiler.h"
#include "eval.h"


// these are used by our assembly code
static float g_cmpaddtab[2]={0.0,1.0};
static float g_signs[2]={1.0,-1.0};
static double g_closefact = 0.00001;
static float g_half=0.5;
static float negativezeropointfive=-0.5f;
static float onepointfive=1.5f;



/// functions called by built code
#define SHITCALL __fastcall

#define isnonzero(x) (fabs(x) > g_closefact)

//---------------------------------------------------------------------------------------------------------------
static double SHITCALL _rand(double *x)
{
  if (*x < 1.0) *x=1.0;
  return (double)(rand()%(int)max(*x,1.0));
}

//---------------------------------------------------------------------------------------------------------------
static double SHITCALL _band(double *var, double *var2)
{
  return isnonzero(*var) && isnonzero(*var2) ? 1 : 0;
}

//---------------------------------------------------------------------------------------------------------------
static double SHITCALL _bor(double *var, double *var2)
{
  return isnonzero(*var) || isnonzero(*var2) ? 1 : 0;
}

//---------------------------------------------------------------------------------------------------------------
static double SHITCALL _sig(double *x, double *constraint)
{
  double t = (1+exp(-*x * (*constraint)));
  return isnonzero(t) ? 1.0/t : 0;
}

extern char *g_evallib_visdata;

static double SHITCALL getvis(unsigned char *visdata, int bc, int bw, int ch, int xorv)
{
  int x;
  int accum=0;
  if (ch && ch != 1 && ch != 2) return 0.0;

  if (bw < 1) bw=1;
  bc-=bw/2;
  if (bc < 0) 
  {
    bw+=bc;
    bc=0;
  }
  if (bc > 575) bc=575;
  if (bc+bw > 576) bw=576-bc;


  if (!ch)
  {
    for (x = 0; x < bw; x ++) 
    {
      accum+=(visdata[bc]^xorv)-xorv;
      accum+=(visdata[bc+576]^xorv)-xorv;
      bc++;
    }
    return (double)accum / ((double)bw*255.0);
  }
  else 
  {
    if (ch == 2) visdata+=576;
    for (x = 0; x < bw; x ++) accum+=(visdata[bc++]^xorv)-xorv;
    return (double)accum / ((double)bw*127.5);
  }
}

static double SHITCALL  getspec_(double *band, double *bandw, double *chan)
{
  if (!g_evallib_visdata) return 0.0;
  return getvis((unsigned char *)g_evallib_visdata,(int)(*band*576.0),(int)(*bandw*576.0),(int)(*chan+0.5),0)*0.5;
}

static double SHITCALL getosc_(double *band, double *bandw, double *chan)
{
  if (!g_evallib_visdata) return 0.0;
  return getvis((unsigned char *)g_evallib_visdata+576*2,(int)(*band*576.0),(int)(*bandw*576.0),(int)(*chan+0.5),128);
}

static double SHITCALL gettime_(double *sc)
{
  int ispos;
  if ((ispos=(*sc > -1.001 && *sc < -0.999)) || (*sc > -2.001 && *sc < -1.999))
  {
    int pos=0;

    extern HWND hwnd_WinampParent;
	  if (IsWindow(hwnd_WinampParent)) 
    {
      if (!SendMessageTimeout( hwnd_WinampParent, WM_USER,(WPARAM)!ispos,(LPARAM)105,SMTO_BLOCK,50,(LPDWORD)&pos)) pos=0;
    }
    if (!ispos) return (double)pos;
    return pos / 1000.0;
  }

  return GetTickCount()/1000.0 - *sc;
}

static double SHITCALL setmousepos_(double *x, double *y)
{
  //fucko: implement me
  return 0.0;
}

static double SHITCALL getmouse_(double *which)
{
  int w=(int)(*which+0.5);

  if (w > 5)
    return (GetAsyncKeyState(w)&0x8000)?1.0:0.0;

  if (w == 1 || w == 2)
  {
    double DDraw_translatePoint(POINT p, int isY);
    POINT p;
    GetCursorPos(&p);
    return DDraw_translatePoint(p,w==2);
  }
  if (w == 3) return (GetAsyncKeyState(MK_LBUTTON)&0x8000)?1.0:0.0;
  if (w == 4) return (GetAsyncKeyState(MK_RBUTTON)&0x8000)?1.0:0.0;
  if (w == 5) return (GetAsyncKeyState(MK_MBUTTON)&0x8000)?1.0:0.0;
  return 0.0;
}
// end functions called by inline code

// these make room on the stack for local variables, but do not need to 
// worry about trashing ebp, since none of our code uses ebp and there's
// a pushad+popad surrounding the call

#if 0 // dont seem to need to do this
#define CF_PUSHREGS __asm { push esi }  __asm { push edi }
#define CF_POPREGS  __asm { pop edi } __asm { pop esi }
#else
#define CF_PUSHREGS
#define CF_POPREGS
#endif

#define FUNC1_ENTER \
  double *parm_a, *__nextBlock; \
  __asm { mov ebp, esp } \
  __asm { sub esp, __LOCAL_SIZE } \
  __asm { mov dword ptr parm_a, eax } \
  __asm { mov __nextBlock, esi } \
  CF_PUSHREGS

#define FUNC2_ENTER \
  double *parm_a,*parm_b,*__nextBlock; \
  __asm { mov ebp, esp } \
  __asm { sub esp, __LOCAL_SIZE } \
  __asm { mov dword ptr parm_a, eax } \
  __asm { mov dword ptr parm_b, ebx } \
  __asm { mov __nextBlock, esi }  \
  CF_PUSHREGS

#define FUNC3_ENTER \
  double *parm_a,*parm_b,*parm_c,*__nextBlock; \
  __asm { mov ebp, esp } \
  __asm { sub esp, __LOCAL_SIZE } \
  __asm { mov dword ptr parm_a, eax } \
  __asm { mov dword ptr parm_b, ebx } \
  __asm { mov dword ptr parm_c, ecx } \
  __asm { mov __nextBlock, esi }  \
  CF_PUSHREGS

#define FUNC_LEAVE \
  __asm { mov eax, esi } \
  __asm { add esi, 8 }  \
  __asm { mov esp, ebp }  \
  CF_POPREGS


static double (*__asin)(double) = &asin;
//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_asin(void)
{
  FUNC1_ENTER

  *__nextBlock = __asin(*parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_asin_end(void) {}

static double (*__acos)(double) = &acos;
//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_acos(void)
{
  FUNC1_ENTER

  *__nextBlock = __acos(*parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_acos_end(void) {}

//---------------------------------------------------------------------------------------------------------------
static double (*__atan)(double) = &atan;
__declspec ( naked ) void _asm_atan(void)
{
  FUNC1_ENTER

  *__nextBlock = __atan(*parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_atan_end(void) {}

//---------------------------------------------------------------------------------------------------------------
static double (*__atan2)(double,double) = &atan2;
__declspec ( naked ) void _asm_atan2(void)
{
  FUNC2_ENTER

  *__nextBlock = __atan2(*parm_b, *parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_atan2_end(void) {}


//---------------------------------------------------------------------------------------------------------------
static double (SHITCALL  * __sig)(double *,double *) = &_sig;
__declspec ( naked ) void _asm_sig(void)
{
  FUNC2_ENTER

  *__nextBlock = __sig(parm_b, parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_sig_end(void) {}

//---------------------------------------------------------------------------------------------------------------
static double (SHITCALL *__rand)(double *) = &_rand;
__declspec ( naked ) void _asm_rand(void)
{
  FUNC1_ENTER

  *__nextBlock = __rand(parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_rand_end(void) {}

//---------------------------------------------------------------------------------------------------------------
static double (SHITCALL *__band)(double *,double *) = &_band;
__declspec ( naked ) void _asm_band(void)
{
  FUNC2_ENTER

  *__nextBlock = __band(parm_b, parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_band_end(void) {}

//---------------------------------------------------------------------------------------------------------------
static double ( SHITCALL  *__bor)(double *,double *) = &_bor;
__declspec ( naked ) void _asm_bor(void)
{
  FUNC2_ENTER

  *__nextBlock = __bor(parm_b, parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_bor_end(void) {}

//---------------------------------------------------------------------------------------------------------------
static double (* __pow)(double,double) = &pow;
__declspec ( naked ) void _asm_pow(void)
{
  FUNC2_ENTER

  *__nextBlock = __pow(*parm_b, *parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_pow_end(void) {}

//---------------------------------------------------------------------------------------------------------------
static double (*__exp)(double) = &exp;
__declspec ( naked ) void _asm_exp(void)
{
  FUNC1_ENTER

  *__nextBlock = __exp(*parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_exp_end(void) {}

//---------------------------------------------------------------------------------------------------------------
static double (*__floor)(double) = &floor;
__declspec ( naked ) void _asm_floor(void)
{
  FUNC1_ENTER

  *__nextBlock = __floor(*parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_floor_end(void) {}


//---------------------------------------------------------------------------------------------------------------
static double (*__ceil)(double) = &ceil;
__declspec ( naked ) void _asm_ceil(void)
{
  FUNC1_ENTER

  *__nextBlock = __ceil(*parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_ceil_end(void) {}

//---------------------------------------------------------------------------------------------------------------



static double (SHITCALL *__getosc)(double *,double *,double *) = &getosc_;
__declspec ( naked ) void _asm_getosc(void)
{
  FUNC3_ENTER

  *__nextBlock = __getosc(parm_c,parm_b,parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_getosc_end(void) {}


static double (SHITCALL *__getspec)(double *,double *,double *) = &getspec_;
__declspec ( naked ) void _asm_getspec(void)
{
  FUNC3_ENTER

  *__nextBlock = __getspec(parm_c,parm_b,parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_getspec_end(void) {}

static double (SHITCALL *__gettime)(double *) = &gettime_;
__declspec ( naked ) void _asm_gettime(void)
{
  FUNC1_ENTER
  
  *__nextBlock = __gettime(parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_gettime_end(void) {}

// do nothing, eh
__declspec ( naked ) void _asm_exec2(void)
{
}
__declspec ( naked ) void _asm_exec2_end(void) { }


static double (SHITCALL *__getmouse)(double *) = &getmouse_;
__declspec ( naked ) void _asm_getmouse(void)
{
  FUNC1_ENTER
  
  *__nextBlock = __getmouse(parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_getmouse_end(void) {}


static double (SHITCALL *__setmousepos)(double *,double *) = &setmousepos_;
__declspec ( naked ) void _asm_setmousepos(void)
{
  FUNC2_ENTER
  
  *__nextBlock = __setmousepos(parm_a,parm_b);

  FUNC_LEAVE
}
__declspec ( naked ) void _asm_setmousepos_end(void) {}



__declspec ( naked ) void _asm_invsqrt(void)
{
  __asm 
  {
    fld qword ptr [eax]

    mov edx, 0x5f3759df
    fst dword ptr [esi]
    // floating point stack has input, as does [eax]
    fmul dword ptr [negativezeropointfive]
    mov ecx, [esi]
    sar ecx, 1
    sub edx, ecx
    mov [esi], edx
    
    // st(0) = input, [eax] has x
    fmul dword ptr [esi]

    fmul dword ptr [esi]

    fadd dword ptr [onepointfive]

    fmul dword ptr [esi]
    mov eax, esi

    fstp qword ptr [esi]
  
    add esi, 8
  }
}
__declspec ( naked ) void _asm_invsqrt_end(void) {}


//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_sin(void)
{
  __asm 
  {
    fld qword ptr [eax]
    fsin
    mov eax, esi
    fstp qword ptr [esi]
    add esi, 8
  }
}
__declspec ( naked ) void _asm_sin_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_cos(void)
{
  __asm 
  {
    fld qword ptr [eax]
    fcos
    mov eax, esi
    fstp qword ptr [esi]
    add esi, 8
  }
}
__declspec ( naked ) void _asm_cos_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_tan(void)
{
  __asm 
  {
    fld qword ptr [eax]
    fsincos
    fdiv
    mov eax, esi
    fstp qword ptr [esi]
    add esi, 8
  }
}
__declspec ( naked ) void _asm_tan_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_sqr(void)
{
  __asm 
  {
    fld qword ptr [eax]
    fmul st(0), st(0)
    mov eax, esi
    fstp qword ptr [esi]
    add esi, 8
  }
}
__declspec ( naked ) void _asm_sqr_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_sqrt(void)
{
  __asm 
  {
    fld qword ptr [eax]
    fabs
    fsqrt
    mov eax, esi
    fstp qword ptr [esi]
    add esi, 8
  }
}
__declspec ( naked ) void _asm_sqrt_end(void) {}


//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_log(void)
{
  __asm 
  {
    fld1
    fldl2e
    fdiv
    fld qword ptr [eax]
    mov eax, esi
    fyl2x
    fstp qword ptr [esi]
    add esi, 8
  }
}
__declspec ( naked ) void _asm_log_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_log10(void)
{
  __asm 
  {
    fld1
    fldl2t
    fdiv
    fld qword ptr [eax]
    mov eax, esi
    fyl2x
    fstp qword ptr [esi]
    add esi, 8
  }
}
__declspec ( naked ) void _asm_log10_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_abs(void)
{
  __asm 
  {
    fld qword ptr [eax]
    fabs
    mov eax, esi
    fstp qword ptr [esi]
    add esi, 8
  }
}
__declspec ( naked ) void _asm_abs_end(void) {}


//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_assign(void)
{
  __asm 
  {
    fld qword ptr [eax]
    fstp qword ptr [ebx]
  }
}
__declspec ( naked ) void _asm_assign_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_add(void)
{
  __asm 
  {
    fld qword ptr [eax]
    fadd qword ptr [ebx]
    mov eax, esi
    fstp qword ptr [esi]
    add esi, 8
  }
}
__declspec ( naked ) void _asm_add_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_sub(void)
{
  __asm 
  {
    fld qword ptr [ebx]
    fsub qword ptr [eax]
    mov eax, esi
    fstp qword ptr [esi]
    add esi, 8
  }
}
__declspec ( naked ) void _asm_sub_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_mul(void)
{
  __asm 
  {
    fld qword ptr [ebx]
    fmul qword ptr [eax]
    mov eax, esi
    fstp qword ptr [esi]
    add esi, 8
  }
}
__declspec ( naked ) void _asm_mul_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_div(void)
{
  __asm 
  {
    fld qword ptr [ebx]
    fdiv qword ptr [eax]
    mov eax, esi
    fstp qword ptr [esi]
    add esi, 8
  }
}
__declspec ( naked ) void _asm_div_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_mod(void)
{
  __asm 
  {
    fld qword ptr [ebx]

    fld qword ptr [eax]
    fsub dword ptr [g_cmpaddtab+4]
    fabs
    fadd qword ptr [eax]
    fadd dword ptr [g_cmpaddtab+4]

    fmul dword ptr [g_half]

    fistp dword ptr [esi]
    fistp dword ptr [esi+4]
    mov eax, [esi+4]
    xor edx, edx
    div dword ptr [esi]
    mov [esi], edx
    fild dword ptr [esi]
    mov eax, esi
    fstp qword ptr [esi]
    add esi, 8

  }
}
__declspec ( naked ) void _asm_mod_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_or(void)
{
  __asm 
  {
    fld qword ptr [ebx]
    fld qword ptr [eax]
    fistp qword ptr [esi]
    fistp qword ptr [esi+8]
    mov ebx, [esi+8]
    or [esi], ebx
    mov ebx, [esi+12]
    or [esi+4], ebx
    fild qword ptr [esi]
    fstp qword ptr [esi]
    mov eax, esi
    add esi, 8
  }
}
__declspec ( naked ) void _asm_or_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_and(void)
{
  __asm 
  {
    fld qword ptr [ebx]
    fld qword ptr [eax]

    fistp qword ptr [esi]
    fistp qword ptr [esi+8]
    mov ebx, [esi+8]
    and [esi], ebx
    mov ebx, [esi+12]
    and [esi+4], ebx
    fild qword ptr [esi]
    fstp qword ptr [esi]

    mov eax, esi
    add esi, 8
  }
}
__declspec ( naked ) void _asm_and_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_uplus(void) // this is the same as doing nothing, it seems
{
#if 0
  __asm 
  {
    mov ebx, nextBlock
    mov ecx, [eax]
    mov [ebx], ecx
    mov ecx, [eax+4]
    mov [ebx+4], ecx
    mov eax, ebx
    add ebx, 8
    mov nextBlock, ebx
  }
#endif
}
__declspec ( naked ) void _asm_uplus_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_uminus(void)
{
  __asm 
  {
    mov ecx, [eax]
    mov ebx, [eax+4]
    xor ebx, 0x80000000
    mov [esi], ecx
    mov [esi+4], ebx
    mov eax, esi
    add esi, 8
  }
}
__declspec ( naked ) void _asm_uminus_end(void) {}



//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_sign(void)
{
  __asm
  {
    mov ecx, [eax+4]

    shr ecx, 31

    fld dword ptr [g_signs+ecx*4]

    fstp qword ptr [esi]
    mov eax, esi
    add esi, 8
  }
}
__declspec ( naked ) void _asm_sign_end(void) {}



//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_bnot(void)
{
  __asm
  {
    fld qword ptr [eax]
    fabs
    fcomp qword ptr [g_closefact]
    fstsw ax
    shr eax, 6
    and eax, (1<<2)
    fld dword ptr [g_cmpaddtab+eax]
    fstp qword ptr [esi]   
    mov eax, esi
    add esi, 8
  }
}
__declspec ( naked ) void _asm_bnot_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_if(void)
{
  __asm
  {
    fld qword ptr [eax]
    fabs
    fcomp qword ptr [g_closefact]
    fstsw ax

    shr eax, 6

    mov dword ptr [esi], 0FFFFFFFFh
    mov dword ptr [esi+4], 0FFFFFFFFh

    and eax, (1<<2)

    mov eax, [esi+eax]

    call eax // call the proper function

    // at this point, the return value will be in eax, as desired
  }
}
__declspec ( naked ) void _asm_if_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_equal(void)
{
  __asm
  {
    fld qword ptr [eax]
    fsub qword ptr [ebx]
    fabs
    fcomp qword ptr [g_closefact]
    fstsw ax
    shr eax, 6
    and eax, (1<<2)
    fld dword ptr [g_cmpaddtab+eax]
    fstp qword ptr [esi]   
    mov eax, esi
    add esi, 8
  }
}
__declspec ( naked ) void _asm_equal_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_below(void)
{
  __asm
  {
    fld qword ptr [ebx]
    fcomp qword ptr [eax]
    fstsw ax
    shr eax, 6
    and eax, (1<<2)
    fld dword ptr [g_cmpaddtab+eax]

    fstp qword ptr [esi]   
    mov eax, esi
    add esi, 8
  }
}
__declspec ( naked ) void _asm_below_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void _asm_above(void)
{
  __asm
  {
    fld qword ptr [eax]
    fcomp qword ptr [ebx]
    fstsw ax
    shr eax, 6
    and eax, (1<<2)
    fld dword ptr [g_cmpaddtab+eax]

    fstp qword ptr [esi]   
    mov eax, esi
    add esi, 8
  }
}
__declspec ( naked ) void _asm_above_end(void) {}


__declspec ( naked ) void _asm_min(void)
{
  __asm 
  {
    fld qword ptr [eax]
    fld qword ptr [ebx]
    fld st(1)
    fsub st(0), st(1)
    fabs  // stack contains fabs(1-2),1,2
    fchs
    fadd
    fadd
    fmul dword ptr [g_half]
    fstp qword ptr [esi]
    mov eax, esi
    add esi, 8
  }
}
__declspec ( naked ) void _asm_min_end(void) {}

__declspec ( naked ) void _asm_max(void)
{
  __asm 
  {
    fld qword ptr [eax]
    fld qword ptr [ebx]
    fld st(1)
    fsub st(0), st(1)
    fabs  // stack contains fabs(1-2),1,2
    fadd
    fadd
    fmul dword ptr [g_half]
    fstp qword ptr [esi]
    mov eax, esi
    add esi, 8
  }
}
__declspec ( naked ) void _asm_max_end(void) {}

