/*
  Nullsoft Expression Evaluator Library (NS-EEL)
  Copyright (C) 1999-2003 Nullsoft, Inc.
  
  nseel-cfunc.c: assembly/C implementation of operator/function templates
  This file should be ideally compiled with optimizations towards "minimize size"

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
#include <math.h>
#include "ns-eel-int.h"


// these are used by our assembly code
static float g_cmpaddtab[2]={0.0,1.0};
static float g_signs[2]={1.0,-1.0};
static double g_closefact = 0.00001;
static float g_half=0.5;
static float negativezeropointfive=-0.5f;
static float onepointfive=1.5f;



/// functions called by built code

#define isnonzero(x) (fabs(x) > g_closefact)
#pragma optimize( "", off ) 
//---------------------------------------------------------------------------------------------------------------
static double NSEEL_CGEN_CALL _rand(double *x)
{
  if (*x < 1.0) *x=1.0;
  return (double)(rand()%(int)max(*x,1.0));
}

//---------------------------------------------------------------------------------------------------------------
static double NSEEL_CGEN_CALL _band(double *var, double *var2)
{
  return isnonzero(*var) && isnonzero(*var2) ? 1 : 0;
}

//---------------------------------------------------------------------------------------------------------------
static double NSEEL_CGEN_CALL _bor(double *var, double *var2)
{
  return isnonzero(*var) || isnonzero(*var2) ? 1 : 0;
}

//---------------------------------------------------------------------------------------------------------------
static double NSEEL_CGEN_CALL _sig(double *x, double *constraint)
{
  double t = (1+exp(-*x * (*constraint)));
  return isnonzero(t) ? 1.0/t : 0;
}


// end functions called by inline code

// these make room on the stack for local variables, but do not need to 
// worry about trashing ebp, since none of our code uses ebp and there's
// a pushad+popad surrounding the call


static double (*__asin)(double) = &asin;
//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_asin(void)
{
  FUNC1_ENTER

  *__nextBlock = __asin(*parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void nseel_asm_asin_end(void) {}

static double (*__acos)(double) = &acos;
//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_acos(void)
{
  FUNC1_ENTER

  *__nextBlock = __acos(*parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void nseel_asm_acos_end(void) {}

//---------------------------------------------------------------------------------------------------------------
static double (*__atan)(double) = &atan;
__declspec ( naked ) void nseel_asm_atan(void)
{
  FUNC1_ENTER

  *__nextBlock = __atan(*parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void nseel_asm_atan_end(void) {}

//---------------------------------------------------------------------------------------------------------------
static double (*__atan2)(double,double) = &atan2;
__declspec ( naked ) void nseel_asm_atan2(void)
{
  FUNC2_ENTER

  *__nextBlock = __atan2(*parm_b, *parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void nseel_asm_atan2_end(void) {}


//---------------------------------------------------------------------------------------------------------------
static double (NSEEL_CGEN_CALL  * __sig)(double *,double *) = &_sig;
__declspec ( naked ) void nseel_asm_sig(void)
{
  FUNC2_ENTER

  *__nextBlock = __sig(parm_b, parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void nseel_asm_sig_end(void) {}

//---------------------------------------------------------------------------------------------------------------
static double (NSEEL_CGEN_CALL *__rand)(double *) = &_rand;
__declspec ( naked ) void nseel_asm_rand(void)
{
  FUNC1_ENTER

  *__nextBlock = __rand(parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void nseel_asm_rand_end(void) {}

//---------------------------------------------------------------------------------------------------------------
static double (NSEEL_CGEN_CALL *__band)(double *,double *) = &_band;
__declspec ( naked ) void nseel_asm_band(void)
{
  FUNC2_ENTER

  *__nextBlock = __band(parm_b, parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void nseel_asm_band_end(void) {}

//---------------------------------------------------------------------------------------------------------------
static double ( NSEEL_CGEN_CALL  *__bor)(double *,double *) = &_bor;
__declspec ( naked ) void nseel_asm_bor(void)
{
  FUNC2_ENTER

  *__nextBlock = __bor(parm_b, parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void nseel_asm_bor_end(void) {}

//---------------------------------------------------------------------------------------------------------------
static double (* __pow)(double,double) = &pow;
__declspec ( naked ) void nseel_asm_pow(void)
{
  FUNC2_ENTER

  *__nextBlock = __pow(*parm_b, *parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void nseel_asm_pow_end(void) {}

//---------------------------------------------------------------------------------------------------------------
static double (*__exp)(double) = &exp;
__declspec ( naked ) void nseel_asm_exp(void)
{
  FUNC1_ENTER

  *__nextBlock = __exp(*parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void nseel_asm_exp_end(void) {}

//---------------------------------------------------------------------------------------------------------------
//static double (*__floor)(double) = &floor;
__declspec ( naked ) void nseel_asm_floor(void)
{
  FUNC1_ENTER

  *__nextBlock = __floor(*parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void nseel_asm_floor_end(void) {}


//---------------------------------------------------------------------------------------------------------------
//static double (*__ceil)(double) = &ceil;
__declspec ( naked ) void nseel_asm_ceil(void)
{
  FUNC1_ENTER

  *__nextBlock = __ceil(*parm_a);

  FUNC_LEAVE
}
__declspec ( naked ) void nseel_asm_ceil_end(void) {}

//---------------------------------------------------------------------------------------------------------------




// do nothing, eh
__declspec ( naked ) void nseel_asm_exec2(void)
{
}
__declspec ( naked ) void nseel_asm_exec2_end(void) { }



__declspec ( naked ) void nseel_asm_invsqrt(void)
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
__declspec ( naked ) void nseel_asm_invsqrt_end(void) {}


//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_sin(void)
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
__declspec ( naked ) void nseel_asm_sin_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_cos(void)
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
__declspec ( naked ) void nseel_asm_cos_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_tan(void)
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
__declspec ( naked ) void nseel_asm_tan_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_sqr(void)
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
__declspec ( naked ) void nseel_asm_sqr_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_sqrt(void)
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
__declspec ( naked ) void nseel_asm_sqrt_end(void) {}


//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_log(void)
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
__declspec ( naked ) void nseel_asm_log_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_log10(void)
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
__declspec ( naked ) void nseel_asm_log10_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_abs(void)
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
__declspec ( naked ) void nseel_asm_abs_end(void) {}


//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_assign(void)
{
  __asm 
  {
    fld qword ptr [eax]
    fstp qword ptr [ebx]
  }
}
__declspec ( naked ) void nseel_asm_assign_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_add(void)
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
__declspec ( naked ) void nseel_asm_add_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_sub(void)
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
__declspec ( naked ) void nseel_asm_sub_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_mul(void)
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
__declspec ( naked ) void nseel_asm_mul_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_div(void)
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
__declspec ( naked ) void nseel_asm_div_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_mod(void)
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
__declspec ( naked ) void nseel_asm_mod_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_or(void)
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
__declspec ( naked ) void nseel_asm_or_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_and(void)
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
__declspec ( naked ) void nseel_asm_and_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_uplus(void) // this is the same as doing nothing, it seems
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
__declspec ( naked ) void nseel_asm_uplus_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_uminus(void)
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
__declspec ( naked ) void nseel_asm_uminus_end(void) {}



//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_sign(void)
{
  __asm
  {
    mov ecx, [eax+4]
    mov edx, [eax]
    test edx, 0FFFFFFFFh
    jnz nonzero

    // high dword (minus sign bit) is zero 
    test ecx, 07FFFFFFFh
    jz zero // zero zero, return the value passed directly

nonzero:
    shr ecx, 31

    fld dword ptr [g_signs+ecx*4]

    fstp qword ptr [esi]

    mov eax, esi
    add esi, 8
zero:
  }
}
__declspec ( naked ) void nseel_asm_sign_end(void) {}



//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_bnot(void)
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
__declspec ( naked ) void nseel_asm_bnot_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_if(void)
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
__declspec ( naked ) void nseel_asm_if_end(void) {}

#ifdef NSEEL_LOOPFUNC_SUPPORT
#ifndef NSEEL_LOOPFUNC_SUPPORT_MAXLEN
#define NSEEL_LOOPFUNC_SUPPORT_MAXLEN (4096)
#endif
//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_repeat(void)
{
  __asm
  {
    fld qword ptr [eax]
    fistp dword ptr [esi]
    mov ecx, [esi]
    cmp ecx, 1
    jl skip
    cmp ecx, NSEEL_LOOPFUNC_SUPPORT_MAXLEN
    jl again
    mov ecx, NSEEL_LOOPFUNC_SUPPORT_MAXLEN
again:
      push ecx
      push esi // revert back to last temp workspace
        mov ecx, 0FFFFFFFFh
        call ecx
      pop esi
      pop ecx
    dec ecx
    jnz again
skip:
  }
}
__declspec ( naked ) void nseel_asm_repeat_end(void) {}
#endif

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_equal(void)
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
__declspec ( naked ) void nseel_asm_equal_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_below(void)
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
__declspec ( naked ) void nseel_asm_below_end(void) {}

//---------------------------------------------------------------------------------------------------------------
__declspec ( naked ) void nseel_asm_above(void)
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
__declspec ( naked ) void nseel_asm_above_end(void) {}


__declspec ( naked ) void nseel_asm_min(void)
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
__declspec ( naked ) void nseel_asm_min_end(void) {}

__declspec ( naked ) void nseel_asm_max(void)
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
__declspec ( naked ) void nseel_asm_max_end(void) {}

#pragma optimize( "", on ) 